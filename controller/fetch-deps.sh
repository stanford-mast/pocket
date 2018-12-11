#!/bin/bash

# Fetch code for standalone C++ client and compile for python3
git clone https://github.com/patrickstuedi/cppcrail
cd cppcrail
git checkout pypocket
sudo apt-get install -y cmake libboost-all-dev python3-dev
sed -i 's/python2.7/python3.5/g' pocket/CMakeLists.txt
mkdir build
cd build
cmake ..
make
cd ../..
cp cppcrail/build/pocket/libpocket.so .
cp cppcrail/build/client/libcppcrail.so .
