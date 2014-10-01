#!/bin/bash
echo "Building software"
make clean all || exit 1

echo "Starting server"
./demo > log_demo &

echo "Starting clients.  Press ctrl-C to exit test."
sleep 1
cd latency_analysis/benchmark
	rm copy_*
export TIMEFORMAT="%4R"
      ./bench.sh 2> copy_1 &
      ./bench.sh 2> copy_2 &
      ./bench.sh 2> copy_3
