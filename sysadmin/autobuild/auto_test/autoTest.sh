#autoTest.sh
#Shell script to automatically pul new chnages and test them, runs on a
#dedicated box as a cron job.
#TO DO: Email errors to predefined people

#! /bin/bash

checkMake()
	{
		if [ $? -ne 0 ]
		then
		echo "Failed to Compile $2"
		fi
	}
pull=$(git pull 2>&1)
if [ "$pull" == "Already up-to-date." ]; then
	echo "Nothing to pull"

else
	cd ../../../tabula_rasa/arduino
	if [ $# -lt 1 ]
	then
	echo "Usage is ./autotest.sh uno/mega"
	elif [ "$1" == "uno" ] #We have an Uno connected
	then
		echo "Compiling firmware and flashing uno"
		result=$(make uno 2>&1)
		checkMake result uno
	elif [ "$1" ==  "mega" ] #We have a Mega connected 
	then
		echo "Compiling firmware and flashing mega"
		result=$(make mega 2>&1)
		checkMake result mega
	fi
	
	cd ../ #Change directory to tabula_rasa
	echo "Compiling backend"
	result=$(make clean all) # Compile backend 
	checkMake result backend
	
	cd ../superstar #Change directory to superstar 
	echo "Compiling Superstar"
	result=$(make clean all) # Compile backend 
	checkMake result superstar
	
	
fi
	
