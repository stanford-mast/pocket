# Pocket Kubernetes cluster setup on AWS

Instructions for seting up Pocket with kubernetes.

## Getting started

Create a pocket-kubernetes VPC, pocket-kubernetes subnet, pocket-kubernetes-open security group and internet gateway. 
Create a s3 bucket for your kops cluster state store. 
Choose a name for your pocket cluster that ends with '.k8s.local', e.g., pocketcluster.k8s.local.

Create and ssh into an instance in pocket-kubernetes VPC from which you will launch kubernetes. This instance will serve as the Pocket controller. Make sure to create the VM in the private subnet of the pocket-kubernetes VPC and assign the VM's (private) IP address to be `10.1.47.178`. This is because the default container images for Pocket storage and metadata nodes assume this IP address for the controller. If you use a different IP address for the controller, you will need to update the Pocket storage and metadata Docker images accordingly (see the `dockerfiles` directory of the pocket repo).

Install [kops](https://github.com/kubernetes/kops):

```
wget -O kops https://github.com/kubernetes/kops/releases/download/$(curl -s https://api.github.com/repos/kubernetes/kops/releases/latest | grep tag_name | cut -d '"' -f 4)/kops-linux-amd64
chmod +x ./kops
sudo mv ./kops /usr/local/bin/

wget -O kubectl https://storage.googleapis.com/kubernetes-release/release/$(curl -s https://storage.googleapis.com/kubernetes-release/release/stable.txt)/bin/linux/amd64/kubectl
chmod +x ./kubectl
sudo mv ./kubectl /usr/local/bin/kubectl

```

Install [kubernetes python client](https://github.com/kubernetes-client/python): 

```
pip install kubernetes
```

Install aws-cli. And run `aws configure` to setup your credentials.

```
pip install awscli --upgrade --user
aws configure
```

Ensure your AWS IAM user has the following permissions (see [here](https://github.com/kubernetes/kops/blob/master/docs/aws.md) for more details):

```
AmazonEC2FullAccess
AmazonRoute53FullAccess
AmazonS3FullAccess
IAMFullAccess
AmazonVPCFullAccess
```

The above steps only need to be done once. The steps to follow are needed each time you want to launch a Pocket cluster.

## Creating a cluster with kops

Set environment variables in env.sh and generate the cluster config file from template:

```
# edit env.sh 
source env.sh
# generate yaml file from template by substituting values of environment variables
envsubst < pocketcluster.template.yaml > pocketcluster.k8s.local.yaml
```

Start cluster (launch instances with kubernetes services running, e.g., kubernetes API, master with kube ctrlr, nodes with kubelet agents). 
The cluster config is defined in `pocketcluster.k8s.local.yaml`, generated in the previous step.

```
./setup_cluster.sh
```

Now wait for cluster instances to start. This usually takes about 5-10 minutes. Check the cluster is valid with:

```
kops validate cluster
```

You should see output like:
```
$ kops validate cluster
Using cluster from kubectl context: pocketcluster.k8s.local

Validating cluster pocketcluster.k8s.local

INSTANCE GROUPS
NAME			ROLE	MACHINETYPE	MIN	MAX	SUBNETS
dram-nodes		Node	r4.2xlarge	2	2	pocket-kube-private
hdd-nodes		Node	h1.2xlarge	0	0	pocket-kube-private
master-us-west-2c	Master	m3.medium	1	1	pocket-kube-private
metadata-nodes		Node	m5.xlarge	1	1	pocket-kube-private
nvme-nodes		Node	i3.2xlarge	0	0	pocket-kube-private
ssd-nodes		Node	i2.2xlarge	0	0	pocket-kube-private

NODE STATUS
NAME						ROLE	READY
ip-10-1-88-112.us-west-2.compute.internal	node	True
ip-10-1-100-102.us-west-2.compute.internal	node	True
ip-10-1-56-16.us-west-2.compute.internal	node	True
ip-10-1-63-38.us-west-2.compute.internal	node	True
ip-10-1-90-254.us-west-2.compute.internal	master	True

Your cluster pocketcluster.k8s.local is ready
```


If you see an `EOF` error, wait a few more minutes. If the cluster does not enter ready state, check your configuration. Make sure the nodes are running on instances with sufficient resources (avoid t2.micro instances, for example) and the master needs to run on a node with local SSD, e.g. m3 family.

When the cluster is in ready state, patch the cluster to edit the node ingress security group rule and create & attach a secondary interface on the metadata server: 

```
python patch_cluster.py
```

Next, edit IP route table on metadata server:

```
./add_ip_routes.sh
```


## Launching pods

Launch a metadata (namenode) deployemnt. The deployment is defined in `pocket-namenode-deployment.yaml`.

```
python deploy_pocket_namenode.py
```

Now launch the controller. The controller automatically launches datanodes with the spec defined in `pocket-datanode-dram-job.yaml`. It assumes there are sufficient VMs in the cluster for the containers it attempts to launch. Note that Pocket storage server containers have affinity to particular VM types to ensure they have the right type of storage technology available. `*-job.yaml` files in the deploy directory use the Kubernetes nodeSelector key to specify their pocketnodetype which corresponds to the nodeLabels key in `pocketcluster.k8x.local.yaml`.

To launch the controller:

```
cd ../controller
python3 controller.py
```

Alternatively, you can manually launch Pocket storage server containers (make sure you have enough storage VMs in your cluster to run the containers you launch):

```
python create_datanode_job.py dram 3  # this luanches 3 DRAM storage server containers
python create_reflex_job.py nvme 2    # this launches 2 NVMe storage server containers
python create_hdd_job.py hdd 1        # this launches 1 HDD storage server container
python create_ssd_job.py ssd 1	      # this luanches 1 SSD storage server container
```

Checking the status of your containers and some other useful commands:

```
kubectl get deploy -o wide
kubectl get pod -o wide

kubectl exec -it pocket-namenode-deployment-xxxxx -- /bin/bash
kubectl delete deploy --all
```

## Testing from client container

```
docker run -it --net=host --privileged anakli/pocket-shell

# mkdir /tmp/hugepages
# mkdir /tmp/hugepages/cache
# mkdir /tmp/hugepages/data
# ./bin/crail fs -ls /
# ./bin/crail iobench -t write -f /test.data -s 4096 -k 100 -w 0
```

## Tearing down the kubernetes cluster

Kops will delete all the resources it created (security groups, autoscaling groups, volumes, instances, etc). However, we first need to delete the secondary interface we created for the metadata node (run the python script). 

```
python prep_delete_cluster.py
kops delete cluster $NAME --yes
```
