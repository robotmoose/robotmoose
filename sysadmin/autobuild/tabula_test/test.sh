#!/bin/sh
# Automated clean build and test for Tabula Rasa robot control system

# Make sure the machine configuration is correct
#  sudo apt-get install git make g++ freeglut3-dev arduino

# Kill any running servers
`dirname $0`/stoptest.sh

# Check out a clean copy of the code
cd /tmp
rm -fr robotmoose
git clone https://github.com/robotmoose/robotmoose

# Run a local superstar
(
	cd robotmoose/superstar
	make || exit 1
	killall superstar
	./superstar &
) || exit 1

# Flash arduino firmware (if command line avr-gcc is available)
if which avr-gcc
then
  (
	cd robotmoose/tabula_rasa/arduino
	make upload
  ) || exit 1
fi

# Start backend
(
	cd robotmoose/tabula_rasa
	make
	./backend --robot test/autotest1 --superstar http://localhost:8081 &
) || exit 1
sleep 4 # wait for firmware to start

# Start web front end and reconfigure backend
# Use selenium or other browser automation here?
# Or use manual wget hack?
wsup="wget -nv -O out http://127.0.0.1:8081/superstar/test/autotest1"

$wsup/config?set='{"counter":10,"configs":["analog(A0);","heartbeat();","pwm(5);"]}'
sleep 3

$wsup/pilot?set='{"power":{"L":0,"R":0,"pwm":[255]},"time":0,"cmd":{"run":"","arg":""}}'
sleep 1

$wsup/sensors?get





