# Deployment Guide v2.0

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