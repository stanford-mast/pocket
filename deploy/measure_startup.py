from subprocess import Popen, PIPE
import time

namenode_pull = "sudo docker pull anakli/pocket-namenode"
dram_pull     = "sudo docker pull anakli/pocket-datanode-dram"
reflex_pull   = "sudo docker pull yawenw/pocket-reflex"
test_pull     = "sudo docker pull ubuntu"

namenode_deploy = "python deploy_pocket_namenode.py"
dram_deploy     = "python deploy_pocket_datanode.py"
reflex_deploy   = "python deploy_pocket_reflex.py"

# measurse the time it takes to pull the Docker image
def image_pull_time(cmd):
    t1 = time.time()
    p = Popen(cmd.split(), stdout=PIPE)
    result = p.communicate()[0]
    t2 = time.time()
    print result
    print "========================"
    print cmd
    print str(t2-t1) + " sec"
    print "========================"

# measures time it takes to add a node (VM) to the cluster
def vm_start_time(node_type):
    cmd = "kops update cluster pocketcluster.k8s.local --yes"
    p = Popen(cmd.split(), stdout=PIPE, stderr=PIPE)
    result = p.communicate()[0]
    #print result

    cmd = "kops validate cluster"
    t1 = time.time()
    while True:
        p = Popen(cmd.split(), stdout=PIPE, stderr=PIPE)
        result = p.communicate()[0]
        if "is ready" in result:
            break
    t2 = time.time()
    print result
    print "========================"
    print cmd
    print node_type
    print str(t2-t1) + " sec"
    print "========================"

# measures time it takes to deploy a container on a node
def container_start_time(cmd):
    #cmd = "python deploy_pocket_datanode.py"   
    p = Popen(cmd.split(), stdout=PIPE, stderr=PIPE)
    result = p.communicate()[0]
    #print result

    cmd = "kubectl get pod -o wide"
    t1 = time.time()
    while True:
        p = Popen(cmd.split(), stdout=PIPE, stderr=PIPE)
        result = p.communicate()[0]
        if "Running" in result:
            break
    t2 = time.time()
    print result
    print "========================"
    print cmd
    print str(t2-t1) + " sec"
    print "========================"


if __name__ == '__main__':
    #image_pull_time(test_pull)
    node_type = "reflex datanode"
    vm_start_time(node_type)
    #container_start_time(reflex_deploy)

