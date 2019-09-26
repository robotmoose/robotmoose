#!/usr/bin/env python3
import json
import superstar
import sys
total_runs=1000

#Experiment variables (you should change only these really...)
auth=''
experiment_path='/test'
payload_length=128
ss=superstar.superstar_t('http://198.199.89.187:8081')
total_runs=1000

#Called on error, print and die...
def err_func(res):
	sys.stderr.write('\nError - '+str(res)+'\n')
	sys.stderr.flush()
	exit(1)

#Run when set of initial known value is done...
def first_set(res):

	#Globals in python be lame...
	global experiment_path
	global ss

	#Wait for a change...
	ss.get_next(experiment_path,ss.sha256(json.dumps('a'*payload_length)),end_rx_func,err_func)

#On change, write it (reply if you will)
def end_rx_func(res):

	#Globals in python be lame...
	global auth
	global experiment_path
	global ss

	#Figure out what to write (write a's if b's or b's if a's)
	new_value='a'*payload_length
	if 'value' not in res or (len(res['value'])>0 and res['value'][0]=='a'):
		new_value='b'*payload_length

	#Do the "reply"
	ss.set(experiment_path,new_value,auth,None,err_func)
	ss.flush()

if __name__=='__main__':
	try:
		#Print Usage
		sys.stderr.write('Usage: ./getset.py superstar payload_size\n')
		sys.stderr.flush()

		#Parse Settings
		if len(sys.argv)>1:
			ss=superstar.superstar_t(sys.argv[1])
		if len(sys.argv)>2:
			payload_length=int(sys.argv[2])

		#Print Settings
		sys.stderr.write('Superstar:      '+sys.argv[1]+'\n')
		sys.stderr.write('Payload Length: '+str(payload_length)+'\n')
		sys.stderr.flush()

		#Run experiments...forever...
		for ii in range(0,total_runs):

			#But give us debug while you do it...
			sys.stderr.write('\rRunning test '+str(ii+1)+'/'+str(total_runs))
			sys.stdout.flush()

			#And do experiment stuff...
			ss.set(experiment_path,'a'*payload_length,auth,first_set,err_func)
			ss.flush()

		sys.stderr.write('\n')
		sys.stderr.flush()

	#Kill on ctrl+c
	except KeyboardInterrupt:
		sys.stderr.write('\n')
		sys.stderr.flush()
		exit(1)

	#Error, time to die...
	except Exception as error:
		sys.stderr.write('\nError - '+str(error)+'\n')
		sys.stderr.flush()
		exit(1)
