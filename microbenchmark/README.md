## Latency Microbenchmark for Pocket

### Instructions

1. Modify `aws-credentials.txt` to contain your IAM role, Pocket VPC private subnet, and Pocket VPC security group. Then load your environment variables:
```
source aws-credentials.txt
```

2. Compile Pocket client:
```
cd ../../clientlib
./fetch-deps.sh
cp pocket.py ../microbenchmark/latency/
cp cppcrail/build/client/libcppcrail.so ../microbenchmark/latency/
cp cppcrail/build/pocket/libpocket.so ../microbenchmark/latency/
cd ../microbenchmark/latency
```

3. Make sure you have launched a Pocket metadata server and that `latency.py` connects to the right metadata server IP address (default is 10.1.0.10). Also ensure you have at least one Pocket storage server (any storage technology tier) in the cluster. For a simple microbenchmark test, you can launch the storage nodes manually rather than using the Pocket controller.
 
4. To run the test:

Option 1: Run `python invoke_lambda.py` Check the ouput in AWS CloudWatch logs.

Option 2: In the AWS lambda console, open the function `pocket_latency_test`, then run it with the test button. The output will print in the console.


### Notes

* To re-run the latency microbenchamrk, either change `jobid` in `latency.py` or restart Pocket namenode & datanode (since Pocket doesn't support overwriting of the same file) 
* Use the `update_lambda.sh` script to update the AWS deployment package if you make changes to any of the files.
