#!/bin/bash -e

zip -r deploy.zip latency.py pocket.py libc.so.6 libstdc++.so.6 libpocket.so libcppcrail.so libboost_python-py27.so.1.58.0

aws lambda create-function \
    --function-name pocket_latency_test \
    --region us-west-2 \
    --zip-file fileb://deploy.zip \
    --role arn:aws:iam::$AWS_ACCOUNT_NUMBER:role/$IAM_ROLE_NAME \
    --handler latency.lambda_handler \
    --runtime python2.7 \
    --timeout 120 \
    --memory-size 3008 \
    --vpc-config SubnetIds=$POCKET_VPC_PRIVATE_SUBNET_ID,SecurityGroupIds=$POCKET_VPC_SECURITY_GROUP_ID



