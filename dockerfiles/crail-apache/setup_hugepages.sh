mkdir -p /dev/hugepages
mount -t hugetlbfs nodev /dev/hugepages
chmod a+rw /dev/hugepages
mkdir /dev/hugepages/cache
mkdir /dev/hugepages/data

# 30000 * 2048 * 1024 = 62GB
sh -c 'for i in /sys/devices/system/node/node*/hugepages/hugepages-2048kB/nr_hugepages; do echo 30000 > $i; done' 
