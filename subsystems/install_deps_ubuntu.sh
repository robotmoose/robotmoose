#!/bin/bash
sudo apt-get install git cmake build-essential libusb-1.0-0-dev freeglut3-dev libxmu-dev libxi-dev libssl-dev libglib2.0-dev
cd /opt
sudo rm -rf libfreenect
sudo git clone https://github.com/OpenKinect/libfreenect.git
cd libfreenect
sudo mkdir -p build
cd build
sudo cmake ..
sudo make install
sudo ldconfig
