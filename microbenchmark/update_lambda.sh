#!/bin/bash -e

rm deploy.zip

zip -r deploy.zip latency.py pocket.py libc.so.6 libstdc++.so.6 libpocket.so libcppcrail.so libboost_python-py35.so.1.58.0 

aws lambda update-function-code \
    --function-name pocket_latency_test \
    --zip-file fileb://deploy.zip \



