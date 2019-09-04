#!/bin/bash
sudo apt-get install -y cmake libboost-all-dev python-dev
sed -i 's/python2.7/python3.5/g' pocket/CMakeLists.txt
mkdir build
cd build
cmake ..
make
