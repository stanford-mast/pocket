# Pocket: Elastic ephemeral storage

Pocket is an elastic, distributed storage system for ephemeral data sharing. The key properties of Pocket are high throughput and low latency for a wide range of object sizesand automatic, fine-grain resource scaling to rightsize cluster resources for high performance at low cost. Pocket offers a 'serverless' abstraction to storage, so application users do not need to manually configure and manage storage servers. Thus, a key use-case for Pocket is ephemeral data sharing in serverless analytics applications, in which serverless tasks (i.e., lambdas) need to exchange short-lived intermediate data between execution stages. Pocket provides fast, distributed and elastic storage for data with low durability requirements.

## Design: 

Pocket divides responsibility across a control plane, metadata plane and data plane, which can each be scaled indepdently across nodes. Pocket has a logically centralized controller which decides how to distribute data for incoming application jobs. The controller monitors per-second cluster resource utilization across multiple dimensions (compute, network bandwidth and storage capacity) and scales cluster resource allocations to match application requirements. Pocket's primary goal is to provide high I/O performance to applications. The secondary goal is to minimize the resource allocation cost for storage cluster resources. The system is designed for sub-second response times. Pocket's object store client API is kept deliberatly simple and the storage tier implementation is designed to leverage low-latency, high-throughput user-level network and storage processing stacks. Pocket uses multiple storage media (DRAM, Flash and HDD) to balance performance and cost, storing an application's data on the lowest cost storage media that still satisfies the application's I/O demands.


## Implementation:

We leverage several open source projects to implement Pocket. Pocket's metadata and data planes are based on [Apache Crail](http://crail.io). The NVMe Flash and block  storage tiers are based on [ReFlex](https://github.com/stanford-mast/reflex). We deploy Pocket metadata and data nodes in [Docker](https://www.docker.com/) containers. Pocket's controller uses the [Kubernetes](https://kubernetes.io) container orchestration system to elastically scale the cluster based on dynamic application demands.


## Pocket repository structure: 

* pocket-core: Pocket metadata and data plane implementation, based on Apache Crail and ReFlex
* controller: control plane logic for automatic resource scaling 
* monitor: storage server resource monitoring daemon 
* clientlib: python client library for Pocket, based on the cppcrail C++ client
* dokcerfiles: Dockerfiles for building Pocket metadata and storage server container images 
* deploy: deployment scripts and instructions to run Pocket with Kubernetes on AWS EC2

