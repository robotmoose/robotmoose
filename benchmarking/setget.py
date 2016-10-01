#!/usr/bin/env python3
import json
import superstar
import sys
import time

#Experiment variables (you should change only these really...)
auth=''
experiment_path='/test'
payload_length=5#10**6
ss=superstar.superstar_t('http://127.0.0.1:8081')
#ss=superstar.superstar_t('http://137.229.25.252:443')
total_runs=10000

#Meh...
old_value=''

#Important time points
tx_start_time=0
tx_end_time=0

#Total averages
rx_avg=0
tx_avg=0
total_avg=0

#Called on error, print and die...
def err_func(res):
	sys.stderr.write('\nError - '+str(res)+'\n')
	sys.stderr.flush()
	exit(1)

#Do a get just to get the old value...not really needed...but it's here...
def initial_get(res):

	#Globals in python be lame...
	global auth
	global experiment_path
	global old_value
	global ss
	global tx_start_time

	#Figure out what to write (write a's if b's or b's if a's)
	if res=='a'*payload_length:
		old_value='b'*payload_length
	else:
		old_value='a'*payload_length

	#Start the clock and the experiment (set)
	tx_start_time=time.time()
	ss.set(experiment_path,old_value,auth,end_tx_func,err_func)
	ss.flush()

#Called when set is done
def end_tx_func(res):

	#Globals in python be lame...
	global experiment_path
	global old_value
	global ss
	global tx_end_time

	#End the tx clock, start the rx clock!
	old_hash=ss.sha256(json.dumps(old_value))
	tx_end_time=time.time()
	ss.get_next(experiment_path,old_hash,end_rx_func,err_func)

#Called when data is confirmed changed
def end_rx_func(res):

	#Globals in python be lame...
	global tx_start_time
	global tx_end_time
	global rx_avg
	global tx_avg
	global total_avg

	#Calculate time diffs
	rx_end_time=time.time()
	rx_avg+=(rx_end_time-tx_end_time)*1000
	tx_avg+=(tx_end_time-tx_start_time)*1000
	total_avg+=(rx_end_time-tx_start_time)*1000

	#Print data to stdout (redirect to a file)
	sys.stdout.write(str((rx_end_time-tx_end_time)*1000)+',')
	sys.stdout.write(str((tx_end_time-tx_start_time)*1000)+',')
	sys.stdout.write(str((rx_end_time-tx_start_time)*1000)+'\n')
	sys.stdout.flush()

if __name__=='__main__':
	try:
		#Print file header
		sys.stdout.write('rx,tx,total\n')
		sys.stdout.flush()

		#Run experiments
		for ii in range(total_runs):

			#DDOS timeout (note this isn't counted in experiment data)
			time.sleep(0.01)

			#Debug output (note this isn't counted in experiment data)
			sys.stderr.write('\rRunning test '+str(ii+1)+'/'+str(total_runs))
			sys.stderr.flush()

			#Run experiment
			ss.get(experiment_path,initial_get,err_func)
			ss.flush()

		#Debug output
		sys.stderr.write('\n')
		sys.stderr.write('RX avg time(ms):    '+str(rx_avg/total_runs)+'\n')
		sys.stderr.write('TX avg time(ms):    '+str(tx_avg/total_runs)+'\n')
		sys.stderr.write('Total avg time(ms): '+str(total_avg/total_runs)+'\n')
		sys.stderr.flush();

	#Kill on ctrl+c
	except KeyboardInterrupt:
		exit(1)

	#Error, time to die...
	except Exception as error:
		sys.stderr.write('\nError - '+str(error)+'\n')
		sys.stderr.flush()
		exit(1)