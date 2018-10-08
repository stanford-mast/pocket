#!/usr/bin/env python3
import asyncio
import struct
import math
import pandas as pd
from random import randint
import socket
import pocket
import pocket_metadata_cmds as ioctlcmd
from os import path
from subprocess import Popen, PIPE
import sys
import yaml
from kubernetes import client, config
import time

CONTROLLER_IP = "10.1.47.178"    # set this to the controller IP address
NAMENODE_IP = "10.1.0.10"	     # set this to the metadata server IP
NAMENODE_PORT = 9070

DRAM = 0
NVME = 1
STORAGE_TIERS = [DRAM, NVME]     # define tiers in order of lowest to highest latency       

WAIT_FOR_DRAM_STARTUP = 10       # how long it takes for DRAM container to startup, in seconds
WAIT_FOR_FLASH_STARTUP = 60      # how long it takes ReFlex container to startup, in seconds
FRAC_DRAM_ALLOCATION = 0.2       # fraction of dataset that will go to dram vs. flash,
                                 # used if need more nodes for capacity than for throughput

# TODO: tune these parameters empirically!
UTIL_DRAM_LOWER_LIMIT = 60
UTIL_DRAM_UPPER_LIMIT = 80
UTIL_FLASH_LOWER_LIMIT = 60
UTIL_FLASH_UPPER_LIMIT = 80
UTIL_CPU_LOWER_LIMIT = 60
UTIL_CPU_UPPER_LIMIT = 80 
UTIL_NET_LOWER_LIMIT = 60
UTIL_NET_UPPER_LIMIT = 80
ALLOC_NET_UPPER_LIMIT = 1.0      # autoscaling for network is currently based on allocated network bandwidth 
ALLOC_NET_LOWER_LIMIT = 0.5

MIN_NUM_DATANODES = 2		
MAX_NUM_DATANODES = 8


BLOCKSIZE = 65536		 # this should match setting in Pocket metadata and storage servers
MAX_DIR_DEPTH = 16
TICKET = 1000

DRAM_NODE_GB = 60		 # instance properties
FLASH_NODE_GB = 2000
NODE_Mbps = 8000

DEFAULT_NUM_NODES = 50		 # default number of nodes to use when no hint given for job 
PER_LAMBDA_MAX_Mbps = 600	 # AWS Lambda approximate per-lambda network limit in Mb/s

GET_CAPACITY_STATS_INTERVAL = 1  # how often get stats from metadata server, in seconds
AUTOSCALE_INTERVAL = 1           # how often try to adjust cluster size, in seconds

# define RPC_CMDs and CMD_TYPEs
RPC_IOCTL_CMD = 13
NN_IOCTL_CMD = 13
RPC_JOB_CMD = 14
JOB_CMD = 14
UTIL_STAT_CMD = 15

CMD_DEL = 2
CMD_CREATE_DIR = 3
CMD_CLOSE = 4

# define IOCTL OPCODES
REGISTER_OPCODE = 0
DEREGISTER_OPCODE = 1
DN_REMOVE_OPCODE = 2
GET_CLASS_STATS_OPCODE = 3
NN_SET_WMASK_OPCODE = 4


INT = 4
LONG = 8
FLOAT = 4
SHORT = 2
BYTE = 1

REQ_STRUCT_FORMAT = "!iqhhi" # msg_len (INT), ticket (LONG LONG), cmd (SHORT), cmd_type (SHORT), register_type (INT)
REQ_LEN_HDR = SHORT + SHORT + BYTE # CMD, CMD_TYPE, IOCTL_OPCODE (note: doesn't include msg_len or ticket from NaRPC hdr)
MSG_LEN_HDR = INT + LONG + SHORT + SHORT + INT # MSG_LEN + TICKET + CMD, CMD_TYPE, OPCODE
DN_LEN_HDR = INT + LONG + SHORT + 5*(INT)

RESP_STRUCT_FORMAT = "!iqhhi" # msg_len (INT), ticket (LONG LONG), cmd (SHORT), error (SHORT), register_opcode (INT)
RESP_LEN_BYTES = INT + LONG + SHORT + SHORT + INT # MSG_LEN, TICKET, CMD, ERROR, REGISTER_OPCODE 
NN_RESPONSE_BYTES = 4 + 8 + 2 # INT (msg_len) + LONG (ticket) + SHORT (OK or ERROR)

RESP_OK = 0
RESP_ERR = 1

hdr_req_packer = struct.Struct(REQ_STRUCT_FORMAT)
hdr_resp_packer = struct.Struct(RESP_STRUCT_FORMAT)
dn_req_packer = struct.Struct("!iqhiiiii")

dram_launch_num = 0
flash_launch_num = 0
waitnodes = 0

# store job ids and their associated capacity & throughput allocation and weight map
job_table = pd.DataFrame(columns=['jobid', 
				  'GB', 
				  'Mbps', 
				  'wmask', 
				  'wmask_str']).set_index('jobid')

# keep track of each storage server's CPU and network utilization
datanode_usage = pd.DataFrame(columns=['datanodeip_port',
				       'cpu', 
				       'net',
				       'blacklisted']).set_index(['datanodeip_port'])
datanode_usage.loc[:, ('cpu', 'net','blacklisted')] = datanode_usage.loc[:, ('cpu', 'net','blacklisted')].astype(int) 

# keep track of each storage server node's allocated resources for running the Pocket storage container
datanode_alloc = pd.DataFrame(columns=['datanodeip_port', 
				       'cpu', 
				       'net', 
				       'DRAM_GB', 
				       'Flash_GB',
				       'blacklisted', 
				       'reserved']).set_index(['datanodeip_port'])
datanode_alloc.loc[:,('cpu', 'net', 'DRAM_GB', 'Flash_GB')] = datanode_alloc.loc[:,('cpu', 'net', 'DRAM_GB', 'Flash_GB')].astype(float) 

# keep track of each storage server node's provisioned resources on the VM 
datanode_provisioned = pd.DataFrame(columns=['datanodeip_port', 
					     'cpu_num', 
					     'net_Mbps', 
					     'DRAM_GB', 
					     'Flash_GB', 
					     'blacklisted']).set_index(['datanodeip_port'])

# total cluster utilization across multiple resource dimensions
avg_util = {'cpu': 0, 'net': 0, 'dram': 0, 'flash': 0, 'net_aggr':0, 'dram_totalGB': 0, 'dram_usedGB':0}


# NOTE: assuming i3 and r4 2xlarge instances
def add_datanode_provisioned(datanodeip, port, num_cpu):
  datanode = datanodeip + ":" + str(port)
  if datanode in datanode_provisioned.index.values:
    #print("Datanode {}:{} is already in table".format(datanodeip, port))
    return 1
  if port == 50030:
    datanode_provisioned.loc[datanode,:] = dict(cpu_num=num_cpu, net_Mbps=8000, DRAM_GB=60, Flash_GB=0, blacklisted=0)
  elif port == 1234:
    datanode_provisioned.loc[datanode,:] = dict(cpu_num=num_cpu, net_Mbps=8000, DRAM_GB=0, Flash_GB=2000, blacklisted=0)
  else:
    print("ERROR: unrecognized port! assuming 50030 for dram, 1234 for Flash/ReFlex")

def add_datanode_alloc(datanodeip, port):
  datanode = datanodeip + ":" + str(port)
  if datanode in datanode_alloc.index.values: 
    return 1
  global waitnodes
  if waitnodes > 0:
    datanode_alloc.loc[datanode,:] = dict(cpu=0.0, net=0.0, DRAM_GB=0.0, Flash_GB=0.0, blacklisted=0, reserved=1)
    waitnodes = waitnodes -1
  else:
    print("index values:", datanode_usage.index.values)
    #print(datanode_usage)
    datanode_alloc.loc[datanode,:] = dict(cpu=0.0, net=0.0, DRAM_GB=0.0, Flash_GB=0.0, blacklisted=0, reserved=0)

def add_datanode_usage(datanodeip, port, cpu, net):
  datanode = datanodeip + ":" + str(port)
  if datanode not in datanode_usage.index.values:
    print("datanode usage table got new datanode {}.".format(datanode))
    datanode_usage.loc[datanode,:] = dict(cpu=cpu, net=net, blacklisted=0) #FIXME: put waitnodes logic here when util based scaling
  else:
    datanode_usage.at[datanode ,'cpu'] = cpu 
    datanode_usage.at[datanode, 'net'] = net


def add_job(jobid, GB, Mbps, wmask, wmask_str):
  if jobid in job_table.index.values:
    print("ERROR: jobid {} already exists!".format(jobid))
    return 1
  job_table.loc[jobid,:] = dict(GB=GB, Mbps=Mbps, wmask=wmask, wmask_str=wmask_str) 
  print("Adding job " + jobid)
  print(job_table)
  return 0

def remove_job(jobid):
  if jobid not in job_table.index.values:
    print("ERROR: jobid {} not found!".format(jobid))
    return 1
  job_table.drop(jobid, inplace=True)
  print("Removed job " + jobid)
  print(job_table)
  return 0

@asyncio.coroutine
def launch_dram_datanode(parallelism):
  print("KUBERNETES: launch dram datanode........")
  global dram_launch_num
  kubernetes_job_name = "pocket-datanode-dram-job" + str(dram_launch_num)
  yaml_file = "kubernetes/pocket-datanode-dram-job.yaml"
  cmd = ["./kubernetes/update_datanode_yaml.sh", kubernetes_job_name, str(parallelism), yaml_file] 
  Popen(cmd, stdout=PIPE).wait()

  config.load_kube_config()
  
  with open(path.join(path.dirname(__file__), yaml_file)) as f:
    job = yaml.load(f)
    k8s_beta = client.BatchV1Api()
    resp = k8s_beta.create_namespaced_job(
           body=job, namespace="default")
    print("Job created. status='%s'" % str(resp.status))
  dram_launch_num = dram_launch_num+1
  print("Wait for DRAM datanode to start...")
  yield from asyncio.sleep(WAIT_FOR_DRAM_STARTUP)
  print("Done waiting for DRAM datanode to start.")
  return

@asyncio.coroutine
def launch_flash_datanode(parallelism):
  print("KUBERNETES: launch flash datanode........")
  global flash_launch_num
  kubernetes_job_name = "pocket-datanode-nvme-job" + str(flash_launch_num)
  yaml_file = "kubernetes/pocket-datanode-nvme-job.yaml"
  cmd = ["./kubernetes/update_datanode_yaml.sh", kubernetes_job_name, str(parallelism), yaml_file] 
  Popen(cmd, stdout=PIPE).wait()

  config.load_kube_config()
  
  with open(path.join(path.dirname(__file__), yaml_file)) as f:
    job = yaml.load(f)
    k8s_beta = client.BatchV1Api()
    resp = k8s_beta.create_namespaced_job(
           body=job, namespace="default")
    print("Job created. status='%s'" % str(resp.status))
  flash_launch_num = flash_launch_num+1
  print("Wait for flash datanode to start...")
  yield from asyncio.sleep(WAIT_FOR_FLASH_STARTUP)
  print("Done waiting for flash datanode to start.")
  return

# wait until "paralellism" number of nodes have registered 
# i.e. they have started sending util stats and are therefore in datanode_alloc table
@asyncio.coroutine
def wait_for_datanodes_to_join(datanode_alloc_prelaunch, parallelism):
  while True:
    yield from asyncio.sleep(1)
    new_datanodes = datanode_alloc.index[datanode_alloc.index.isin(datanode_alloc_prelaunch.index)==False].values.tolist()
    if len(new_datanodes) == parallelism:
      return new_datanodes

# Algorithm to generate weightmask
# The weightmask defines the list of datanodes (including tier type) 
# to spread this job's data across, along with an associated weight for each node
# the weight is used for weighted random datanode block selection at the metdata server
@asyncio.coroutine
def generate_weightmask(jobid, jobGB, jobMbps, latency_sensitive):
  print("generate weightmask for ", jobid, jobGB, jobMbps, latency_sensitive)
  wmask = []
  # Step 1: determine if capacity or throughput bound
  if latency_sensitive:  
    num_nodes_for_capacity = math.ceil(jobGB / DRAM_NODE_GB)
  else:
    num_nodes_for_capacity = math.ceil(jobGB / FLASH_NODE_GB)
   
  num_nodes_for_throughput = math.ceil(jobMbps / NODE_Mbps)

  if num_nodes_for_throughput >= num_nodes_for_capacity:
    print("jobid {} is throughput-bound".format(jobid))
    throughput_bound = 1
  else:
    print("jobid {} is capacity-bound".format(jobid))
    throughput_bound = 0
  
  # Step 2: check available resources in cluster
  # If throughput bound, will allocate nodes based on CPU and network demand 
  if throughput_bound:
    # find all nodes that have spare Mbps 
    #print(datanode_alloc)
    spare_throughput = datanode_alloc.loc[(datanode_alloc['net'] < 1.0) & (datanode_alloc['blacklisted'] == 0)]
    candidate_nodes_net = spare_throughput.sort_values(by='net', ascending=False).loc[:, 'net']
    spare_net_weight_alloc = 0
    job_net_weight_req = jobMbps * 1.0 / NODE_Mbps
    #print(candidate_nodes_net)

    print("job net weight req {}, this is {} Mbps".format(job_net_weight_req, jobMbps))
    # smallest fit first algorithm: fill in smallest gap first
    for node in candidate_nodes_net.index: 
      net = candidate_nodes_net[node]
      #print("net for node {} is {}".format(node, net))
      if net == 1.0:
        continue
      #TODO: before decide to use a node, also much check weight*capacity satisfies capacity constraint!
      if job_net_weight_req - spare_net_weight_alloc >= 1 - net: 
        spare_net_weight_alloc += 1 - net
        print("setting datanode_alloc to 1 for datanode {}".format(node))
        datanode_alloc.at[node,'net'] = 1.0
        wmask.append((node, 1- net))
      elif job_net_weight_req - spare_net_weight_alloc < 1 - net:
        print("setting datanode_alloc to {} for datanode {}".format(net + (job_net_weight_req - spare_net_weight_alloc),node))
        datanode_alloc.at[node,'net'] = float(net + (job_net_weight_req - spare_net_weight_alloc))
        wmask.append((node, job_net_weight_req - spare_net_weight_alloc))
        spare_net_weight_alloc += job_net_weight_req - spare_net_weight_alloc

      if spare_net_weight_alloc == job_net_weight_req:
        break
    
      if spare_net_weight_alloc > job_net_weight_req:
        print("ERROR: shouldn't be allocating more than job needs! something went wrong...")
        break
    
    if spare_net_weight_alloc == job_net_weight_req:
      print("Satisfied job without needing to launch new nodes :)")
      #print(datanode_alloc)
    else:
      datanode_alloc_prelaunch = datanode_alloc.copy()
      extra_nodes_needed = (job_net_weight_req - spare_net_weight_alloc)
      last_weight = extra_nodes_needed - int(extra_nodes_needed)
      if last_weight == 0:
        new_node_weights = [1.0 for i in range(0, int(extra_nodes_needed))]
      else:
        new_node_weights = [1.0 for i in range(0, int(extra_nodes_needed))]
        new_node_weights.append(last_weight)
      parallelism = math.ceil(extra_nodes_needed)
      global waitnodes
      waitnodes = parallelism
      print("KUBERNETES: launch {} extra nodes, wait for them to come up and assing proper weights {}"\
              .format(parallelism, new_node_weights))
      # decide which kind of nodes to launch
      if latency_sensitive and jobGB <= parallelism*DRAM_NODE_GB:
        yield from launch_dram_datanode(parallelism)
      elif latency_sensitive: # but capacity doesn't fit in paralellism*DRAM nodes
        print("app is latency sensitive but high capacity, so we put {} in DRAM, rest in flash".format(FRAC_DRAM_ALLOCATION))
        print("WARNING: check logic. we should not reach this case since then app would be capacity bound!")
        num_dram_nodes = int((jobGB * FRAC_DRAM_ALLOCATION)/ DRAM_NODE_GB)
        num_flash_nodes = parallelism - num_dram_nodes
        yield from launch_flash_datanode(num_dram_nodes)
        yield from launch_flash_datanode(num_flash_nodes)
      else:
        yield from launch_flash_datanode(parallelism)
      
      # wait for new nodes to start sending stats and add themselves to the datanode_alloc table,
      # then assign them the proper weights
      new_datanodes = yield from wait_for_datanodes_to_join(datanode_alloc_prelaunch, parallelism)
      print("datanodes {} have joined!".format(new_datanodes))
      i = 0
      for n in new_datanodes:
        wmask.append((n, new_node_weights[i]))
        i = i + 1
      print("wmask:", wmask)
  
  # TODO: If capacity bound, will allocate nodes based on DRAM or Flash capacity (depending on latency sensitivity)
  else:
    # find all nodes that have spare capacity
    print("NOTICE: the app is capacity-bound. \
           TODO: need to implement weightmask generation for this case. Not yet supported.\n")
    # should be similar to sizing based on capacity
    # but need to decide whether to use DRAM or Flash capacity based on latency sensitivity
    # skipping this for now since all our apps are throughput-bound

  # convert weightmask to proper format
  job_wmask = []
  for (datanodeip_port, weight) in wmask:
    datanode_ip = datanodeip_port.split(":")[0]
    datanode_port = int(datanodeip_port.split(":")[1])
    datanode_hash = ioctlcmd.calculate_datanode_hash(datanode_ip, datanode_port) 
    job_wmask.append((datanode_hash, float("{0:.2f}".format(weight))))
    datanode_alloc.at[datanodeip_port,'net'] = weight
    datanode_alloc.at[datanodeip_port,'reserved'] = 0

  print("job_wmask is:", wmask) 
  print(datanode_alloc)
  return job_wmask, wmask

def compute_GB_Mbps_with_hints(num_lambdas, jobGB, peakMbps, latency_sensitive):

  # determine jobGB and peakMbps based on provided hints (0 means not provided)
  if num_lambdas == 0 and jobGB == 0 and peakMbps == 0: 
    if latency_sensitive: 
      jobGB = DEFAULT_NUM_NODES * (DRAM_NODE_GB + FLASH_NODE_GB)   
    else:
      jobGB = DEFAULT_NUM_NODES * (FLASH_NODE_GB)   
    peakMbps = DEFAULT_NUM_NODES * NODE_Mbps  
  elif num_lambdas != 0 and jobGB == 0 and peakMbps == 0:
    num_nodes = num_lambdas * PER_LAMBDA_MAX_Mbps / NODE_Mbps
    if latency_sensitive: 
      jobGB = num_nodes * (DRAM_NODE_GB + FLASH_NODE_GB)   
    else:
      jobGB = num_nodes * (FLASH_NODE_GB)   
    peakMbps = num_nodes * NODE_Mbps
  elif num_lambdas != 0 and jobGB == 0: # only capacity unknown
    num_nodes = num_lambdas * PER_LAMBDA_MAX_Mbps / NODE_Mbps
    if latency_sensitive: 
      jobGB = num_nodes * (DRAM_NODE_GB + FLASH_NODE_GB)   
    else:
      jobGB = num_nodes * (FLASH_NODE_GB)   
  elif num_lambdas != 0 and peakMbps == 0: # only Mbps unknown
    num_nodes = num_lambdas * PER_LAMBDA_MAX_Mbps / NODE_Mbps
    peakMbps = num_nodes * NODE_Mbps
  elif num_lambdas == 0 and jobGB == 0: # capacity and lambdas unknown
    # use peakMbps hint to estimate number of nodes needed
    num_nodes = peakMbps / NODE_Mbps
    if latency_sensitive: 
      jobGB = num_nodes * (DRAM_NODE_GB + FLASH_NODE_GB)   
    else:
      jobGB = num_nodes * (FLASH_NODE_GB)   
  elif num_lambdas == 0 and peakMbps == 0: # Mbps and lambdas unknown
    # use capacity hint to estimate number of nodes needed
    if latency_sensitive:
      num_nodes = jobGB / DRAM_NODE_GB
    else:
      num_nodes = jobGB / FLASH_NODE_GB
    peakMbps = num_nodes * NODE_Mbps
  if jobGB == 0:
    jobGB = 1

  return jobGB, peakMbps


@asyncio.coroutine
def handle_register_job(reader, writer):
  print("-------------------------- REGISTER JOB --------------------------------", time.time())
  jobname_len = yield from reader.read(INT)
  jobname_len, = struct.Struct("!i").unpack(jobname_len)
  jobname = yield from reader.read(jobname_len + 3*INT + SHORT)
  jobname, num_lambdas, jobGB, peakMbps, latency_sensitive = struct.Struct("!" + str(jobname_len) + "siiih").unpack(jobname)
  jobname = jobname.decode('utf-8')
  
  # generate jobid
  if 'gg' in jobname:
    jobid = jobname + '-1234'
    jobid_int = 1234
  else:
    jobid_int = randint(0,1000000)
    jobid = jobname + "-" + str(jobid_int)

  print("received hints ", jobid, num_lambdas, jobGB, peakMbps, latency_sensitive) 
  # create dir named jobid
  createdirsock = pocket.connect(NAMENODE_IP, NAMENODE_PORT)
  if createdirsock is None:
    return
  pocket.create_dir(createdirsock, None, jobid)
  #pocket.close(createdirsock)

  if jobGB == 0 or peakMbps == 0: 
    jobGB, peakMbps = compute_GB_Mbps_with_hints(num_lambdas, jobGB, peakMbps, latency_sensitive)

  # generate weightmask 
  wmask, wmask_str = yield from generate_weightmask(jobid, jobGB, peakMbps, latency_sensitive)
 # wmask = [(ioctlcmd.calculate_datanode_hash("10.1.88.82", 50030), 1)]

  # register job in table
  err = add_job(jobid, jobGB, peakMbps, wmask, wmask_str)

  # send wmask to metadata server   
  ioctlsock = yield from ioctlcmd.connect(NAMENODE_IP, NAMENODE_PORT)
  if ioctlsock is None:
    return
  yield from ioctlcmd.send_weightmask(ioctlsock, jobid, wmask) 

  # reply to client with jobid int
  resp_packer = struct.Struct(RESP_STRUCT_FORMAT + "i")
  resp = (RESP_LEN_BYTES + INT, TICKET, JOB_CMD, err, REGISTER_OPCODE, jobid_int)
  pkt = resp_packer.pack(*resp)
  writer.write(pkt)
  print("-------------------------- REGISTERED JOB --------------------------------")

  return
  

@asyncio.coroutine
def handle_deregister_job(reader, writer):
  jobid_len = yield from reader.read(INT)
  jobid_len, = struct.Struct("!i").unpack(jobid_len)
  jobid = yield from reader.read(jobid_len)
  jobid, = struct.Struct("!" + str(jobid_len) + "s").unpack(jobid)
  jobid = jobid.decode('utf-8')
  
  print("------------------------- DEREGISTER JOB --------------------------------", time.time())
  # clear weight of this job
  for (datanodeip_port, weight) in job_table.loc[jobid,'wmask_str']:
    datanode_alloc.at[datanodeip_port, 'net'] =  datanode_alloc.at[datanodeip_port, 'net'] - weight
  # delete job from table
  err = remove_job(jobid)
  if err == 0:
    # delete dir named jobid
    createdirsock = pocket.connect(NAMENODE_IP, NAMENODE_PORT)
    if createdirsock is None:
      return
    pocket.delete(createdirsock, None, "/" + jobid)
    #pocket.close(createdirsock)
 
 
  # reply to client with jobid int
  resp_packer = struct.Struct(RESP_STRUCT_FORMAT)
  resp = (RESP_LEN_BYTES + INT, TICKET, JOB_CMD, err, DEREGISTER_OPCODE)
  pkt = resp_packer.pack(*resp)
  writer.write(pkt)
  print("------------------------- DEREGISTERED JOB --------------------------------")
  return


@asyncio.coroutine
def handle_jobs(reader, writer):
  address = writer.get_extra_info('peername')
  print('Accepted job connection from {}'.format(address))
  hdr = yield from reader.read(MSG_LEN_HDR) 
  [msg_len, ticket, cmd, cmd_type, opcode] = hdr_req_packer.unpack(hdr)
  print(msg_len, ticket, cmd, cmd_type, opcode)
  if cmd != cmd_type:
    print("ERROR: expected CMD_TYPE == CMD")

  if cmd == RPC_JOB_CMD:
    if opcode == REGISTER_OPCODE:
      yield from handle_register_job(reader, writer)
    elif opcode == DEREGISTER_OPCODE:
      print("Deregister job...");
      yield from handle_deregister_job(reader, writer)
    else:
      print("ERROR: unknown JOB_CMD opcode ", opcode);

  return      

@asyncio.coroutine
def handle_datanodes(reader, writer):
  address = writer.get_extra_info('peername')
  print('Accepted datanode connection from {}'.format(address))
  while True:
    hdr = yield from reader.read(DN_LEN_HDR) 
    [msg_len, ticket, cmd, datanode_int, port, rx_util, tx_util, num_cores] = dn_req_packer.unpack(hdr)
    if cmd != UTIL_STAT_CMD:
      print("ERROR: unknown datanode opcode ", opcode);
    cpu_util = yield from reader.read(num_cores * INT)
    cpu_util = struct.Struct("!" + "i"*num_cores).unpack(cpu_util)
    if len(cpu_util) == 0:
      avg_cpu = 0
    else:
      #avg_cpu = math.ceil(sum(cpu_util)/len(cpu_util)) # FIXME: calculate CPU utilization with multiple cores
      avg_cpu = math.ceil(cpu_util[0])  
    peak_net = int(max(rx_util, tx_util)*1.0/NODE_Mbps * 100)
    # add datanode to tables
    datanode_ip = socket.inet_ntoa(struct.pack('!L', datanode_int))
    add_datanode_provisioned(datanode_ip, port, len(cpu_util))
    add_datanode_alloc(datanode_ip, port) 
    add_datanode_usage(datanode_ip, port, avg_cpu, peak_net) 

@asyncio.coroutine
def get_capacity_stats_periodically(sock):
  while True:
    yield from asyncio.sleep(GET_CAPACITY_STATS_INTERVAL)
    for tier in STORAGE_TIERS: 
      all_blocks, free_blocks = yield from ioctlcmd.get_class_stats(sock, tier)
      if all_blocks:
        avg_usage = (all_blocks-free_blocks)*100.0/all_blocks
        print("Capacity usage for Tier", tier, ":", free_blocks, "free blocks out of", \
               all_blocks, "(", avg_usage, "% )")
      else:
        avg_usage = -1
      # update global avg_util dictionary
      if tier == 0:
        avg_util['dram'] = avg_usage 
        avg_util['dram_totalGB'] = all_blocks*BLOCKSIZE *1.0 /1e9 
        avg_util['dram_usedGB'] = (all_blocks - free_blocks)*BLOCKSIZE * 1.0 / 1e9
      elif tier == 1:
        avg_util['flash'] = avg_usage 


@asyncio.coroutine
def autoscale_cluster(logfile):
  while True:
    yield from asyncio.sleep(AUTOSCALE_INTERVAL)
    avg_util['cpu'] = datanode_usage.loc[:,'cpu'].mean()  
    avg_util['net_aggr'] = datanode_usage.loc[:,'net'].sum() * NODE_Mbps / 100
    num_nodes_active = 0
    if len(datanode_usage.index) > 0:
      num_nodes_active = int(datanode_alloc.loc[datanode_alloc['blacklisted'] == 0].count()[0]) 
    allocated_net = num_nodes_active * NODE_Mbps
    if num_nodes_active == 0:
      avg_util['net'] = 0
    else:
      avg_util['net'] = avg_util['net_aggr'] / allocated_net * 100
    print(datanode_alloc)
    if num_nodes_active > 0:
      print(time.time(), avg_util['net_aggr'], avg_util['cpu'], avg_util['dram_usedGB'], allocated_net, avg_util['dram_totalGB'], file=logfile) 

    # check datanode resource utilization and add/remove nodes as necessary
    # TODO: do the same for metadata server utilization and metadata node scaling
    
    # Logic to add node... (if any resource util is above upper limit)
    if num_nodes_active < MIN_NUM_DATANODES:
      print("INSUFFICIENT DATANODES IN CLUSTER! Launching {} datanodes...".format(MIN_NUM_DATANODES-num_nodes_active))
      #yield from launch_flash_datanode(MIN_NUM_DATANODES-num_nodes_active)
      yield from launch_dram_datanode(MIN_NUM_DATANODES-num_nodes_active)
      print("Launched datanode. Should now have min size cluster.")
    datanode_net_alloc = datanode_alloc[datanode_alloc['reserved'] == 0].loc[:,'net'].mean()
    if datanode_net_alloc > ALLOC_NET_UPPER_LIMIT and num_nodes_active < MAX_NUM_DATANODES:
      print("add a dram datanode. net alloc is {}".format(datanode_net_alloc))
      yield from launch_dram_datanode(1)
    if datanode_net_alloc < ALLOC_NET_LOWER_LIMIT and num_nodes_active > MIN_NUM_DATANODES:
      print("REMOVE NODE, avg datanode_net_alloc is low:", datanode_net_alloc)
      num_eligible_datanodes = int(datanode_alloc.loc[(datanode_alloc['blacklisted'] == 0) & (datanode_alloc['reserved'] == 0)].count()[0])
      print("eligible_datanodes: ", num_eligible_datanodes)
      if num_eligible_datanodes == 0:
        continue
      eligible_datanodes = datanode_alloc.loc[(datanode_alloc['blacklisted'] == 0) & (datanode_alloc['reserved'] == 0)]
      print("eligible_datanodes: ", eligible_datanodes)
      datanodeip_port = eligible_datanodes['net'].idxmin()
      print("Datanode with lowest net alloc is: ", datanodeip_port)
      datanode_alloc.at[datanodeip_port, 'blacklisted'] = 1
      datanode_usage.at[datanodeip_port, 'blacklisted'] = 1
      datanode_provisioned.at[datanodeip_port, 'blacklisted'] = 1
    continue  

'''
    if avg_util['dram'] > UTIL_DRAM_UPPER_LIMIT and num_nodes_active < MAX_NUM_DATANODES:
      # add a DRAM datanode
      print("add a dram datanode. dram util is {}".format(avg_util['dram']))
      yield from launch_dram_datanode(1)
    elif avg_util['flash'] > UTIL_FLASH_UPPER_LIMIT and num_nodes_active < MAX_NUM_DATANODES:
      # add a Flash datanode
      print("add a reflex datanode. flash util is {}".format(avg_util['flash']))
      yield from launch_flash_datanode(1)
    elif avg_util['cpu'] > UTIL_CPU_UPPER_LIMIT and num_nodes_active < MAX_NUM_DATANODES:
      # add a node to cluster
      # need to decide between DRAM or Flash node
      # check if CPU is higher on DRAM or Flash nodes 
      cpu_util_dram = datanode_usage.filter(like='50030', axis=0).loc[:, 'cpu'].mean()
      cpu_util_flash = datanode_usage.filter(like='1234', axis=0).loc[:, 'cpu'].mean()
      if cpu_util_dram > cpu_util_flash:
        print("add a dram datanode, cpu util is high")
        yield from launch_dram_datanode(1)
      else:
        print("add a dram datanode, cpu util is high")
        yield from launch_flash_datanode(1)
    elif avg_util['net'] > UTIL_NET_UPPER_LIMIT and num_nodes_active < MAX_NUM_DATANODES:
      # add a node to cluster
      # need to decide between DRAM or Flash node
      # check if network is higher on DRAM or Flash nodes
      net_util_dram = datanode_usage.filter(like='50030', axis=0).loc[:, 'net'].mean()
      net_util_flash = datanode_usage.filter(like='1234', axis=0).loc[:, 'net'].mean()
      if net_util_dram > net_util_flash:
        print("add a dram datanode, net util is high")
        yield from launch_dram_datanode(1)
      else:
        print("add a dram datanode, net util is high")
        yield from launch_flash_datanode(1)
    
    # Logic to remove node... (if all resources are below lower limit)
    # also we want to keep at least MIN_NUM_DATANODES active in case of new jobs
    elif num_nodes_active > MIN_NUM_DATANODES and \
       avg_util['cpu'] < UTIL_CPU_LOWER_LIMIT and \
       avg_util['net'] < UTIL_NET_LOWER_LIMIT and \
       avg_util['dram'] < UTIL_DRAM_LOWER_LIMIT and \
       avg_util['flash'] < UTIL_FLASH_LOWER_LIMIT: 
      # remove a node with the lowest network utilization 
      datanodeip_port = datanode_usage['net'].idxmin()
      print("Datanode with lowest net or cpu usage is: ", datanodeip_port)
      datanode_alloc.at[datanodeip_port, 'blacklisted'] = 1
      datanode_usage.at[datanodeip_port, 'blacklisted'] = 1
      datanode_provisioned.at[datanodeip_port, 'blacklisted'] = 1
'''
      
    


if __name__ == '__main__':
  
  loop = asyncio.get_event_loop()
  # Start server listening for register/deregister job connections
  #address = ("localhost", 4321) 
  address = (CONTROLLER_IP, 4321) 
  coro = asyncio.start_server(handle_jobs, *address)
  server = loop.run_until_complete(coro)
  print('Listening at {}'.format(address))
 
  # Initialize routine to periodically send
  metadata_socket = ioctlcmd.connect_until_succeed(NAMENODE_IP, NAMENODE_PORT)
  asyncio.async(get_capacity_stats_periodically(metadata_socket))

  # Periodically check avg utilization and run autoscale algorithm
  logfile = open("resource_util.log", "w+")
  print("time", "net_usedMbps", "avg_cpu", "dram_usedGB", "net_allocMbps", "dram_allocGB", file=logfile)  
  asyncio.async(autoscale_cluster(logfile))
 
  # Start server listening for datanode util info
  address = (CONTROLLER_IP, 2345) 
  coro = asyncio.start_server(handle_datanodes, *address)
  server = loop.run_until_complete(coro)
  print('Listening at {}'.format(address))
  print("Start loop...") 


  # test with dummy datanodes
  #add_datanode_alloc("10.1.88.82", 50030)
  #add_datanode_alloc("10.1.88.83", 1234)
  #add_datanode_alloc("10.1.88.84", 50030)
  #add_datanode_alloc("10.1.88.85", 1234)
  #add_datanode_usage("10.1.88.82", 50030, cpu=50, net=70)
  #add_datanode_usage("10.1.88.83", 1234, cpu=80, net=22)
  #add_datanode_usage("10.1.88.84", 50030, cpu=90, net=55)
  #add_datanode_usage("10.1.88.85", 1234, cpu=20, net=50)

  try:
    loop.run_forever()
  finally:
    server.close()
    loop.close()
