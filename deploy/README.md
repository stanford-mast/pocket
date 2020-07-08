# Deployment Guide v2.0

This guide runs thruough the steps for setting up a Pocket kubernetes cluster running on AWS. 

- [The first section](#setup-aws-resources) deals with initial setup and creating AWS resources. 
- [The second section](#create-a-pocket-cluster) details how to create a new Pocket cluster with those resources. 
- The third section shows how to use that cluster to run and use Pocket.

## Setup AWS Resources

1. Create an AWS IAM user with an access key ID and secret access key that will be used to access and allocate AWS resources programatically via the AWS CLI. Ensure your AWS IAM user has the following permissions:

    ```
    AmazonEC2FullAccess
    AmazonRoute53FullAccess
    AmazonS3FullAccess
    IAMFullAccess
    AmazonVPCFullAccess
    ```

2. The initial setup can be run on any computer with the AWS CLI installed. To install the CLI you can use pip. Use the AWS credentials for the IAM user created above. Install aws-cli then run `aws configure` to setup your credentials:

    ```
    pip install awscli --upgrade --user
    aws configure
    ```

3. Run [create_pocket_vpc.sh](./create_pocket_vpc.sh) to setup the requisite AWS VPC with a public and private subnet, as well as a NAT gateway with a static IP address. The cluster will run in the private subnet. The VPC must be created in the `us-west-2` region in order to have acces to the correct AMI for the pocket node instances.

    ```
    ./create_pocket_vpc.sh
    ```

4. Note down the following values and save them in [env.sh](./env.sh) to be reused later to access the VPC resources:

    ```
    POCKET_VPC_ID
    POCKET_VPC_PRIVATE_SUBNET_ID
    POCKET_VPC_PUBLIC_SUBNET_ID
    POCKET_NAT_ID
    POCKET_INTERNET_GATEWAY_ID
    POCKET_NAT_ELASTIC_IP_ID
    POCKET_ROUTE_TABLE_ID
    ```

5. Create a S3 bucket for your kops cluster state store and note the name under `KOPS_STATE_STORE` in [env.sh](./env.sh).

6. Create an EC2 instance in the private subnet of the VPC that will serve as the controller. It must have a private IP address of `10.1.47.178`. This is because the default container images for Pocket storage and metadata nodes assume this IP address for the controller. If you use a different IP address for the controller, you will need to update the Pocket storage and metadata Docker images accordingly (see the `dockerfiles` directory of the pocket repo). You must also create an EC2 instance in the public subnet to act as a bastion since you cannot directly ssh into the private subnet. Make sure both instances use the same ssh key so that you can use ssh forwarding.

7. SSH into the pocket controller instance using the bastion and ssh agent forwarding and run the following commands to setup the necessary resources.

    Install [kops](https://github.com/kubernetes/kops):

    ```
    wget -O kops https://github.com/kubernetes/kops/releases/download/$(curl -s https://api.github.com/repos/kubernetes/kops/releases/latest | grep tag_name | cut -d '"' -f 4)/kops-linux-amd64
    chmod +x ./kops
    sudo mv ./kops /usr/local/bin/

    wget -O kubectl https://storage.googleapis.com/kubernetes-release/release/$(curl -s https://storage.googleapis.com/kubernetes-release/release/stable.txt)/bin/linux/amd64/kubectl
    chmod +x ./kubectl
    sudo mv ./kubectl /usr/local/bin/kubectl
    ```

    Install the [kubernetes python client](https://github.com/kubernetes-client/python) using pip: 

    ```
    pip install kubernetes
    ```
    
    Install the AWS CLI and login using the same IAM credentials as before:

    ```
    pip install awscli --upgrade --user
    aws configure
    ```

    Clone the git repository and ensure the resources referenced in your [env.sh](./env.sh) match the same ones created earlier.

The above steps only need to be done once. The steps to follow are needed each time you want to launch a Pocket cluster.

## Create a Pocket cluster

Set environment variables in [env.sh](./env.sh) and generate the cluster config file from template:

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