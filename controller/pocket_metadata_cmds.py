import socket
import os
import struct
import errno
import asyncio
import numpy as np
from functools import reduce

TICKET = 100
RPC_IOCTL_CMD = 13
NN_IOCTL_CMD = 13

# IOCTL OPCODES
NOP = 1
DN_REMOVE_OPCODE = 2
GET_CLASS_STATS_OPCODE = 3
NN_SET_WMASK_OPCODE = 4

MAX_DIR_DEPTH = 16

INT = 4
LONG = 8
FLOAT = 4
SHORT = 2
BYTE = 1

REQ_STRUCT_FORMAT_IOCTL = "!iqhhb" # msg_len (INT), ticket (LONG LONG), cmd (SHORT), cmd_type (SHORT), ioctl_cmd (BYTE)
REQ_LEN_IOCTL_HDR = SHORT + SHORT + BYTE # CMD, CMD_TYPE, IOCTL_OPCODE (note: doesn't include msg_len or ticket from NaRPC hdr)

RESP_STRUCT_FORMAT_IOCTL = "!iqhhb" # msg_len (INT), ticket (LONG LONG), cmd (SHORT), error (SHORT), ioctl_cmd (BYTE)
RESP_LEN_IOCTL_BYTES = INT + LONG + SHORT + SHORT + BYTE # MSG_LEN, TICKET, CMD, ERROR, IOCTL_OPCODE 

def ip2long(ip):
  """
  Convert an IP string to long
  """
  packedIP = socket.inet_aton(ip)
  return struct.unpack("!L", packedIP)[0]

def java_string_hashcode(fileComponent):
  h = 0
  for c in fileComponent:
    h = (31 * h + ord(c)) & 0xFFFFFFFF
  return ((h + 0x80000000) & 0xFFFFFFFF) - 0x80000000

# we don't need this 
def calculate_datanode_hash_string(ipaddr, port):
  hashcode = java_string_hashcode(ipaddr)
  hash = ((hashcode << 32) | (port & 0xffffffff))
  #print("Hash of ", ipaddr, ":", port, " is ", hash)
  return hash


def java_array_hash(array):
  if array is None:
    return 0;
  result = 1
  for a in array:
    result = 31 * result + np.int8(a)
  return result

def calculate_datanode_hash(ipaddr, port):
  iparray = ipaddr.split(".")
  iparray = [int(i) for i in iparray]
  hashcode = java_array_hash(iparray)
  hashnum = ((hashcode << 32) | (port & 0xffffffff))
  #print("Hash of ", ipaddr, ":", port, " is ", hashnum)
  return hashnum
  
# tokenize filename into FileName representation for Pocket
# e.g., "/abc/def" --> "/1234/2313/0/0/0/0/0/0/0/0/0/0/0/0/0
def create_filename_repr(filename):
  # tokenize
  components = filename.split("/")  
  num = len(components)
  if num > MAX_DIR_DEPTH:
    print("ERROR: exceeded max directory depth for filename ", filename)
  result = ""
  for component in components:
    if component is "":
      continue
    result = result + "/" + str(java_string_hashcode(component)) 
  while (MAX_DIR_DEPTH - num + 1) > 0:
    result = result + "/0" 
    num = num + 1
  result = result + "/"
  #print("filename repr for ", filename, " is: ", result)
  return result, len(components)-1
    

def get_components(fileName):
  components = fileName.split("/")  
  result = ()
  for c in components: 
    if c is not "":
      result = result + (int(c),)
  #print(result)
  return result
 
# param jobdir: directory named after jobID which we are setting the weightmask for
# param wmasklist: list of <long, float> tuples defining <datanodeHash, weight>
@asyncio.coroutine
def send_weightmask(socket, jobdir, wmasklist):
  masklen = len(wmasklist)
  if not jobdir.startswith("/"): 
    jobdir = "/" + jobdir
  maskstruct = "qf" * masklen # make repeating string
  fileName, num_levels = create_filename_repr(jobdir)
  msg_packer = struct.Struct(REQ_STRUCT_FORMAT_IOCTL + "i" + "i"*MAX_DIR_DEPTH + "i" + maskstruct) # filename len (INT) + fileName repr of filename + num_pairs + num_pairs*<long, float>
  msgLen = REQ_LEN_IOCTL_HDR + INT + (MAX_DIR_DEPTH * INT) + INT + masklen*(LONG + FLOAT)
  ticket = TICKET
  cmd = RPC_IOCTL_CMD
  cmdType = NN_IOCTL_CMD
  ioctlOpcode = NN_SET_WMASK_OPCODE
  sampleMsg = (msgLen, ticket, cmd, cmdType, ioctlOpcode, num_levels, get_components(fileName), masklen) + sum(wmasklist, ())  # note: sum(wmasklist,()) flattens list of tuples to tuple
  flatten = lambda lst: reduce(lambda l, i: l + flatten(i) if isinstance(i, (list, tuple)) else l + [i], lst, [])   
  sampleMsg = tuple(flatten(sampleMsg))
 
  pkt = msg_packer.pack(*sampleMsg)
  socket.sendall(pkt)
  data = socket.recv(RESP_LEN_IOCTL_BYTES + INT) 
  resp_packer = struct.Struct(RESP_STRUCT_FORMAT_IOCTL + "i")
  [length, ticket, type_, err, opcode, ecode] = resp_packer.unpack(data)
  if err != 0 and ecode !=0:
    print("Error setting weightmask: ", err, ecode)
  else:
    print("Set weightmask for dir ", jobdir)
  return


# assume datanodeIp is string (we convert it to long in here)
@asyncio.coroutine
def dn_remove(socket, datanodeIp, datanodePort):
  datanodeIp = ip2long(datanodeIp)
  msg_packer = struct.Struct(REQ_STRUCT_FORMAT_IOCTL + "ii") # ipaddr (INT) + port (INT)
  msgLen = REQ_LEN_IOCTL_HDR + INT + INT
  ticket = TICKET
  cmd = RPC_IOCTL_CMD
  cmdType = NN_IOCTL_CMD
  ioctlOpcode = DN_REMOVE_OPCODE
  sampleMsg = (msgLen, ticket, cmd, cmdType, ioctlOpcode, datanodeIp, datanodePort)
  pkt = msg_packer.pack(*sampleMsg)
  socket.sendall(pkt)
  data = socket.recv(RESP_LEN_IOCTL_BYTES) 
  resp_packer = struct.Struct(RESP_STRUCT_FORMAT_IOCTL)
  [length, ticket, type_, err, opcode] = resp_packer.unpack(data)
  if err != 0:
    print("Error removing datanode: ", err)
  else:
    print("Removed datanode ", datanodeIp, ":", datanodePort)
  return

@asyncio.coroutine
def get_class_stats(socket, storageClass):
  msg_packer = struct.Struct(REQ_STRUCT_FORMAT_IOCTL + "i") # storageClass (INT)
  msgLen = REQ_LEN_IOCTL_HDR + INT 
  ticket = TICKET
  cmd = RPC_IOCTL_CMD
  cmdType = NN_IOCTL_CMD
  ioctlOpcode = GET_CLASS_STATS_OPCODE
  sampleMsg = (msgLen, ticket, cmd, cmdType, ioctlOpcode, storageClass)
  pkt = msg_packer.pack(*sampleMsg)
  socket.sendall(pkt)
  data = socket.recv(RESP_LEN_IOCTL_BYTES) # first receive header and check if error (e.g. no such storage class)
  resp_packer = struct.Struct(RESP_STRUCT_FORMAT_IOCTL)
  [length, ticket, type_, err, opcode] = resp_packer.unpack(data)
  if err != 0:
    #print("Error getting capacity stats: ", err)
    return (0, 0)
  else:
    data = socket.recv(LONG + LONG) # if no error, get stats
    resp_packer = struct.Struct("!qq")
    [allBlocks, freeBlocks] = resp_packer.unpack(data)
    #print("allBlocks: ",allBlocks," freeBlocks: ",freeBlocks)
    return (allBlocks, freeBlocks)
  

def connect_until_succeed(HOSTNAME, PORT):
  connected = 1
  print("Connecting to metadata server....")
  while connected != 0:
    try:
      s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
      connected = s.connect_ex((HOSTNAME, PORT))
    except:
      print("Connection to metadata server refused...trying again")
      continue;
  return s


@asyncio.coroutine
def connect(HOSTNAME, PORT):
  try:
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    connected = s.connect_ex((HOSTNAME, PORT))
  except:
    print("Connection to metadata server refused.")
    return None
  return s
