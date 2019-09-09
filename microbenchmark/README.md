## Latency Microbenchmark for Pocket

### Instructions

1. Modify `aws-credentials.txt` to contain your IAM role, Pocket VPC private subnet, and Pocket VPC security group. Note that lambdas must run in the same VPC as Pocket. Ensure your security group allows traffic to/from Pocket metadata and storage server nodes. Load your environment variables: 
```
source aws-credentials.txt
```

2. Compile the Pocket client library (if you have not already) and copy the shared library and pocket.py file to the microbenchmark directory:
```
# compile Pocket client source
cd ../crailclient
./build.sh

# copy client files to microbenchmark directory
cp build/client/libcppcrail.so ../microbenchmark/
cp build/pocket/libpocket.so ../microbenchmark/
cp ../clientlib/pocket.py ../microbenchmark/

cd ../microbenchmark/
```

3. Register the lambda function called `pocket_latency_test`:
```
./create_lambda.sh
```

4. Make sure you have launched a Pocket metadata server and that `latency.py` connects to the right metadata server IP address (default is 10.1.0.10). Also ensure you have at least one Pocket storage server (any storage technology tier) in the cluster. For a simple microbenchmark test, you can launch the storage nodes manually rather than using the Pocket controller.
 
5. To run the test:

Option 1: Run `python invoke_lambda.py` Check the ouput in AWS CloudWatch logs.

Option 2: In the AWS lambda console, open the function `pocket_latency_test`, then run it with the test button. The output will print in the console.


### Notes

* To re-run the latency microbenchamrk, either change `jobid` in `latency.py` or restart Pocket namenode & datanode (since Pocket doesn't support overwriting of the same file) 
* Use the `update_lambda.sh` script to update the AWS deployment package if you make changes to any of the files.
