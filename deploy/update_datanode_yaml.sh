#!/bin/bash

# ./update_datanode_yaml.sh [job_name [parallelism] [yaml_file]

# update job name
echo $1 | sed -i "4s@.*@$(awk '{print "   name: "$1}')@g" $3
# update job parallelism
echo $2 | sed -i "6s@.*@$(awk '{print "  parallelism: "$1}')@g" $3
