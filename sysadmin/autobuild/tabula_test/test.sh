#!/bin/sh
# Automated clean build and test for Tabula Rasa robot control system

# Make sure the machine configuration is correct
sudo apt-get install git make g++ freeglut3-dev arduino

# Kill any running servers
`dirname $0`/stoptest.sh

# Check out a clean copy of the code
cd /tmp
rm -fr robotmoose
git clone https://github.com/robotmoose/main
mv main robotmoose

# Run a local superstar
(
	cd robotmoose/superstar
	make || exit 1
	killall superstar
	./superstar &
) || exit 1

# Flash arduino firmware
(
	cd robotmoose/tabula_rasa/arduino
	make upload
) || exit 1

# Start backend
(
	cd robotmoose/tabula_rasa
	make
	./backend --robot autotest1 --superstar http://localhost:8081 &
	sleep 4 # wait for firmware to start
) || exit 1

# Start web front end and reconfigure backend
# Use selenium?


