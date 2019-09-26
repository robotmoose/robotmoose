#!/bin/bash

if [ $# -ne 1 ]
then
	echo "Usage: ./run_backend.sh superstar"
	exit 1
fi

mkdir -p bench
./getset.py "$1" 128
./getset.py "$1" 1024
./getset.py "$1" 10240
./getset.py "$1" 102400
./getset.py "$1" 1024000