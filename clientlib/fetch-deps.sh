#!/bin/bash

# Fetch code for standalone C++ client and compile
git clone https://github.com/patrickstuedi/cppcrail
cd cppcrail
git checkout pypocket
sudo apt-get install -y cmake libboost-all-dev python-dev
mkdir build
cd build
cmake ..
make
cd ../..

