#!/bin/bash
#  Shell script to start code editor, and keep it running.

cd `dirname $0`

(
while [ true ]
do
	mkdir -p logs
	date >> logs/log
	echo "Running code_start.sh by "`who am i` >> logs/log
	chown no_priv:no_priv logs

	su -c ./code_editor no_priv 2>&1 >> logs/log
	echo "ERROR -- code_editor killed off somehow" >> logs/log
	sleep 10
done
) &
echo "Code running.  To see any errors, run   tail -f logs/log &"
