## Elastic ephemeral storage

Pocket is a storage system designed for ephemeral data sharing. Pocket provides fast, distributed and elastic storage for data with low durability requirements.
The key properties of Pocket are:

* high throughput and low latency for a wide range of object sizes
* automatic resource scaling and rightsizing
* intelligent data placement across multiple storage tiers
	
Pocket offers a *serverless* abstraction to storage, meaning users do not manually configure or manage storage servers. A key use-case for Pocket is ephemeral data sharing in serverless analytics, when serverless tasks need to exchange intermediate data between execution stages. 


## Running Pocket

Follow the 'Getting Started' instructions in the [deploy README](https://github.com/stanford-mast/pocket/blob/master/deploy/README.md) to get started with a VPC for Pocket on AWS. 

```
git clone https://github.com/stanford-mast/pocket
cd deploy
# follow Gettgin Started instructions in deploy README
```

Launch the VMs for your Pocket cluster:

```
# edit env.sh 
source env.sh
# generate yaml file from template by substituting values of environment variables
envsubst < pocketcluster.template.yaml > pocketcluster.k8s.local.yaml
# edit pocketcluster.k8s.local.yaml to adjust the number of metadata and storage servers

./setup_cluster.sh
kops validate cluster

# wait 3-5 minutes until the above command outputs 'Your cluster pocketcluster.k8s.local is ready'

python patch_cluster.py
./add_ip_routes.sh
```

Launch a Pocket metadata server container:

```
python deploy_pocket_namenode.py 
```

Launch the controller to autoscale the cluster as jobs register and deregister. The controller scales containers and assumes VMs are available to run these containers.

```
cd ../controller
python3 controller.py
```

You can also individually launch Pocket storage server containers instead of using the controller. For example:
```
python create_datanode_job.py dram_container 3  # this luanches 3 DRAM storage server containers
python create_reflex_job.py nvme_container 2    # this launches 2 NVMe storage server containers
python create_hdd_job.py hdd_container 1        # this launches 1 HDD storage server container
python create_ssd_job.py ssd_container 1	# this luanches 1 SSD storage server container
```

You are now ready to launch a lambda job and use Pocket. See the [microbenchmark README](https://github.com/stanford-mast/pocket/tree/master/microbenchmark) for a simple example test. 
Note: lambdas must run in the same VPC as Pocket. Ensure your security group allows traffic to/from Pocket metadata and storage server nodes. 


Tear down the Pocket cluster as follows:
```
python prep_delete_cluster.py
kops delete cluster $NAME --yes
```


## Pocket Design 

Pocket divides responsibility across a control plane, metadata plane and data plane, which can each be scaled indepdently across nodes. Pocket has a logically centralized controller which decides how to distribute data for incoming application jobs. The meatdata plane routes client requests and keeps track of data stored across nodes in the data plane.

Pocket is designed for sub-second response times. I/O operations are kept deliberatly simple and the storage tier implementation leverages low-latency, high-throughput user-level network and storage processing stacks. The controller monitors per-second cluster resource utilization across multiple dimensions (compute, network bandwidth and storage capacity) and scales cluster resource allocations to match application requirements. The controller allocates resource with the primary goal of providing high I/O performance to jobs. The secondary goals is to minimizing the resource cost. 

Pocket uses multiple storage media (DRAM, Flash and HDD) to balance performance and cost, storing an application's data on the lowest cost storage media that still satisfies the application's I/O demands.


## Implementation

We leverage several open source projects to implement Pocket. Pocket's metadata and data planes are based on [Apache Crail](http://crail.io). The NVMe Flash and block  storage tiers are based on [ReFlex](https://github.com/stanford-mast/reflex). We deploy Pocket metadata and data nodes in [Docker](https://www.docker.com/) containers. Pocket's controller uses the [Kubernetes](https://kubernetes.io) container orchestration system to elastically scale the cluster based on dynamic application demands.


## Pocket repository structure

* **deploy**: deployment scripts and instructions to run Pocket with Kubernetes on AWS EC2
* **src**: Pocket metadata and data plane implementation, based on Apache Crail and ReFlex
* **controller**: control plane logic for automatic resource scaling 
* **monitor**: source code for resource monitoring daemon that runs in each storage server container
* **clientlib**: python client library for Pocket, based on the cppcrail C++ client
* **dockerfiles**: use this to build new Pocket container images if you modify Pocket source code
* **microbenchmark**: a simple lambda latency test for Pocket


## Reference

Please refer to the Pocket OSDI'18 paper: 

Ana Klimovic, Yawen Wang, Patrick Stuedi, Animesh Trivedi, Jonas Pfefferle, and Christos Kozyrakis. **Pocket: Elastic Ephemeral Storage for Serverless Analytics.** In *Proceedings of the USENIX Symposium on Operating Systems Design and Implementation* (OSDI'18), 2018.
