#!/bin/bash

# Script to delete Pocket VPC resources
# Assumes you've sourced create_pocket_vpc.sh
# Otherwise set vars in and source env.sh

#==================================================

# Delete NAT Gateway
echo "Deleting NAT..."
aws ec2 delete-nat-gateway \
  --nat-gateway-id $POCKET_NAT_ID 
echo "  Deleted NAT '$POCKET_NAT_ID'."

echo "Release Elastic IP address for NAT..."
aws ec2 release-address \
  --allocation-id $POCKET_NAT_ELASTIC_IP_ID
echo "  Elastic IP address ID '$POCKET_NAT_ELASTIC_IP_ID' RELEASED."

# Delete Route Table
echo "Deleting Route Table..."
aws ec2 delete-route-table \
  --route-table-id $POCKET_ROUTE_TABLE_ID 
echo "  Route Table ID '$POCKET_ROUTE_TABLE_ID' DELETED."

# Detach Internet gateway to your VPC
echo "Detaching Internet Gateway..."
aws ec2 detach-internet-gateway \
  --vpc-id $POCKET_VPC_ID \
  --internet-gateway-id $POCKET_INTERNET_GATEWAY_ID \
echo "  Internet Gateway ID '$POCKET_INTERNET_GATEWAY_ID' DETACHED."

# Delete Internet gateway
echo "Deleting Internet Gateway..."
aws ec2 delete-internet-gateway \
  --internet-gateway-id $POCKET_INTERNET_GATEWAY_ID
echo "  Internet Gateway ID '$POCKET_INTERNET_GATEWAY_ID' DELETED."


# DELETE Public Subnet
echo "Deleting Public Subnet..."
aws ec2 delete-subnet \
  --subnet-id $POCKET_VPC_PUBLIC_SUBNET_ID
echo "  Subnet ID '$POCKET_VPC_PUBLIC_SUBNET_ID' DELETED."

# DELETE Private Subnet
echo "Deleting Private Subnet..."
aws ec2 delete-subnet \
  --subnet-id $POCKET_VPC_PRIVE_SUBNET_ID
echo "  Subnet ID '$POCKET_VPC_PRIVATE_SUBNET_ID' DELETED."

# Delete VPC
echo "Deleting VPC '$POCKET_VPC_ID'."
aws ec2 delete-vpc \
  --vpc-id $POCKET_VPC_ID 
echo "  VPC ID '$POCKET_VPC_ID' DELETED."



