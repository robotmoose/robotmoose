#!/bin/bash

#Kill on Error
set -e

#Variables
CWD=$(pwd)

#Uninstall Packages
echo "Uninstall Packages"
sudo apt-get remove -y --purge modemmanager

#Install Categories
common="gnome-panel chromium-browser exfat-fuse flashplugin-installer flashplugin-nonfree-extrasound icedtea-plugin git libgnome-keyring-dev"
development="clang g++ gdb make minicom valgrind"
hardware="arduino avrdude binutils-avr gcc-avr avr-libc"
library="freeglut3-dev libftgl-dev libglew-dev libncurses5-dev libopencv-dev libsoil-dev"

#Updates
echo "Update Package Lists"
sudo apt-get update

echo "Upgrade"
sudo apt-get upgrade -y

#Install Packages
echo "Install Packages"
sudo apt-get install -y $common $development $hardware $library

#Remove Uneeded Packages
echo "Remove Uneeded Packages"
sudo apt-get autoremove -y

#Setup Git Keyring
echo "Setup Git Keyring"
cd /usr/share/doc/git/contrib/credential/gnome-keyring
sudo make
sudo rm *.o
sudo mv git-credential-gnome-keyring /usr/bin
git config --replace-all --global credential.helper gnome-keyring
cd $CWD

#Disable Guest Account
echo "Disable Guest Account"
echo "allow-guest=false"|sudo tee --append /usr/share/lightdm/lightdm.conf.d/50-ubuntu.conf

#Fix g++ Linker Error
echo "Fix g++ Linker Error"
GPP=$(which g++)
sudo cp $GPP $GPP-exe
sudo rm $GPP
echo "#!/bin/bash"|sudo tee --append $GPP
echo "$GPP-exe \$@ -Wl,--no-as-needed -pthread"|sudo tee --append $GPP
sudo chmod +x $GPP

#Serial Port Permissions
echo "Serial Port Permissions"
sudo usermod -a -G dialout egg

#Reboot
echo "All Done, Press [ENTER] to reboot."
read -n 1
sudo reboot
