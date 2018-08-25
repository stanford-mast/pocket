# Run setup script for DRAM datanode 

hugepages=`cat /proc/meminfo | grep Huge | grep Free | awk '{$1=""; print $0}'`
echo $hugepages
if [ $hugepages -lt 25000 ]
then
    ./setup_hugepages.sh
fi

rm -rf /dev/hugepages/cache/*
rm -rf /dev/hugepages/data/*

# Start datanode utilization tracking in background
python datanode.py &> /dev/null &

# Start datanode
./bin/crail datanode
