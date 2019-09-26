#!/bin/bash
if [ $# -ne 3 ]
then
	echo "Usage: ./run_robot.sh superstar quiet|real testnum"
	exit 1
fi

mkdir -p bench
./setget.py "$1" 128     > "bench/$2_128b_$3"
./setget.py "$1" 1024    > "bench/$2_1kb_$3"
./setget.py "$1" 10240   > "bench/$2_10kb_$3"
./setget.py "$1" 102400  > "bench/$2_100kb_$3"
./setget.py "$1" 1024000 > "bench/$2_1000kb_$3"