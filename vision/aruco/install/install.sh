#!/bin/sh
cd `dirname $0`
sudo apt-get install freeglut3-dev g++ make libsoil-dev cmake libopencv-dev
tar xvf aruco-*tgz
cd aruco-*/
mkdir build
cd build
cmake .. || exit 1
make || exit 1
sudo make install
cd ../../../viewer
make
