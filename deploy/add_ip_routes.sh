#/bin/bash 

ssh -t admin@`kubectl get nodes --show-labels | grep metadata | awk '{print $1}'` "sudo ip route add default via 10.1.0.1 dev eth1 tab 2"
ssh -t admin@`kubectl get nodes --show-labels | grep metadata | awk '{print $1}'` "sudo ip rule add from 10.1.0.10/32 tab 2 priority 700"
