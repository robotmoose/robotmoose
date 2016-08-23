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
sudo rm -f /etc/udev/rules.d/51-kinect.rules
sudo echo '# ATTR{product}=="Xbox NUI Motor"'                                              |tee -a /etc/udev/rules.d/51-kinect.rules > /dev/null
sudo echo 'SUBSYSTEM=="usb", ATTR{idVendor}=="045e", ATTR{idProduct}=="02b0", MODE="0666"' |tee -a /etc/udev/rules.d/51-kinect.rules > /dev/null
sudo echo '# ATTR{product}=="Xbox NUI Audio"'                                              |tee -a /etc/udev/rules.d/51-kinect.rules > /dev/null
sudo echo 'SUBSYSTEM=="usb", ATTR{idVendor}=="045e", ATTR{idProduct}=="02ad", MODE="0666"' |tee -a /etc/udev/rules.d/51-kinect.rules > /dev/null
sudo echo '# ATTR{product}=="Xbox NUI Camera"'                                             |tee -a /etc/udev/rules.d/51-kinect.rules > /dev/null
sudo echo 'SUBSYSTEM=="usb", ATTR{idVendor}=="045e", ATTR{idProduct}=="02ae", MODE="0666"' |tee -a /etc/udev/rules.d/51-kinect.rules > /dev/null
sudo echo ''                                                                               |tee -a /etc/udev/rules.d/51-kinect.rules > /dev/null
sudo echo '# Kinect for Windows'                                                           |tee -a /etc/udev/rules.d/51-kinect.rules > /dev/null
sudo echo 'SUBSYSTEM=="usb", ATTR{idVendor}=="045e", ATTR{idProduct}=="02c2", MODE="0666"' |tee -a /etc/udev/rules.d/51-kinect.rules > /dev/null
sudo echo 'SUBSYSTEM=="usb", ATTR{idVendor}=="045e", ATTR{idProduct}=="02be", MODE="0666"' |tee -a /etc/udev/rules.d/51-kinect.rules > /dev/null
sudo echo 'SUBSYSTEM=="usb", ATTR{idVendor}=="045e", ATTR{idProduct}=="02bf", MODE="0666"' |tee -a /etc/udev/rules.d/51-kinect.rules > /dev/null