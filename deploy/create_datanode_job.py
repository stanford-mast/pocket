from os import path
from subprocess import Popen, PIPE
import sys

import yaml

from kubernetes import client, config


def main():
    job_id = sys.argv[1]
    parallelism = sys.argv[2]
    job_name = "pocket-datanode-dram-job" + job_id  
    yaml_file = "pocket-datanode-dram-job.yaml"
    cmd = ["./update_datanode_yaml.sh", job_name, parallelism, yaml_file] 
    Popen(cmd, stdout=PIPE).wait()

    config.load_kube_config()
    
    with open(path.join(path.dirname(__file__), yaml_file)) as f:
        job = yaml.load(f)
        k8s_beta = client.BatchV1Api()
        resp = k8s_beta.create_namespaced_job(
            body=job, namespace="default")
        print("Job created. status='%s'" % str(resp.status))


if __name__ == '__main__':
    main()
