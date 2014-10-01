#!/bin/bash
echo "Building software"
make clean all || exit 1

echo "Starting server"
./demo > log_demo &

echo "Starting clients.  We'll run for 10 seconds."
cd latency_analysis/benchmark
	rm copy_*
export TIMEFORMAT="%5R"
      ./bench.sh 2> copy_1 &
      ./bench.sh 2> copy_2 &
      ./bench.sh 2> copy_3 &
cd ../..

sleep 10
killall demo
killall bench.sh
./bench_dump.sh

