#!/bin/bash
sudo apt-get remove --purge libfreenect-dev -y --force-yes
sudo apt-get install git cmake build-essential libusb-1.0-0-dev freeglut3-dev libxmu-dev libxi-dev libssl-dev libglib2.0-dev -y --force-yes
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
echo '# ATTR{product}=="Xbox NUI Motor"'                                              |sudo tee -a /etc/udev/rules.d/51-kinect.rules > /dev/null
echo 'SUBSYSTEM=="usb", ATTR{idVendor}=="045e", ATTR{idProduct}=="02b0", MODE="0666"' |sudo tee -a /etc/udev/rules.d/51-kinect.rules > /dev/null
echo '# ATTR{product}=="Xbox NUI Audio"'                                              |sudo tee -a /etc/udev/rules.d/51-kinect.rules > /dev/null
echo 'SUBSYSTEM=="usb", ATTR{idVendor}=="045e", ATTR{idProduct}=="02ad", MODE="0666"' |sudo tee -a /etc/udev/rules.d/51-kinect.rules > /dev/null
echo '# ATTR{product}=="Xbox NUI Camera"'                                             |sudo tee -a /etc/udev/rules.d/51-kinect.rules > /dev/null
echo 'SUBSYSTEM=="usb", ATTR{idVendor}=="045e", ATTR{idProduct}=="02ae", MODE="0666"' |sudo tee -a /etc/udev/rules.d/51-kinect.rules > /dev/null
echo ''                                                                               |sudo tee -a /etc/udev/rules.d/51-kinect.rules > /dev/null
echo '# Kinect for Windows'                                                           |sudo tee -a /etc/udev/rules.d/51-kinect.rules > /dev/null
echo 'SUBSYSTEM=="usb", ATTR{idVendor}=="045e", ATTR{idProduct}=="02c2", MODE="0666"' |sudo tee -a /etc/udev/rules.d/51-kinect.rules > /dev/null
echo 'SUBSYSTEM=="usb", ATTR{idVendor}=="045e", ATTR{idProduct}=="02be", MODE="0666"' |sudo tee -a /etc/udev/rules.d/51-kinect.rules > /dev/null
echo 'SUBSYSTEM=="usb", ATTR{idVendor}=="045e", ATTR{idProduct}=="02bf", MODE="0666"' |sudo tee -a /etc/udev/rules.d/51-kinect.rules > /dev/null
