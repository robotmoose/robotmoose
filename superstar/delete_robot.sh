#!/bin/sh
# Script to clean a bad robot off the server.

if [ $# -ne 1 ]
then
	echo "Usage: delete_robot.sh uaf/dermo"
	exit 1
fi

robot="$1"

mt() {
	while [ $# -gt 0 ]
	do
		wget -O - http://robotmoose.com/superstar/"$robot/$1"?set= || exit 1
		shift
	done
}

mt config gui options pilot sensors

