#!/bin/bash
cd `dirname $0`

echo "Running code_stop by "`id` >> logs/log
killall code_editor code_start.sh
