#!/usr/bin/env python3
import json
import superstar
import sys

#Experiment variables (you should change only these really...)
auth=''
experiment_path='/test'
payload_length=5#10**6
ss=superstar.superstar_t('http://127.0.0.1:8081')
#ss=superstar.superstar_t('http://137.229.25.252:443')

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
	if 'value' not in res or res['value']=='a'*payload_length:
		new_value='b'*payload_length

	#Do the "reply"
	ss.set(experiment_path,new_value,auth,None,err_func)
	ss.flush()

if __name__=='__main__':
	try:
		#Run experiments...forever...
		ii=0
		while True:

			#But give us debug while you do it...
			ii+=1
			sys.stdout.write('\rRunning test '+str(ii))
			sys.stdout.flush()

			#And do experiment stuff...
			ss.set(experiment_path,'a'*payload_length,auth,first_set,err_func)
			ss.flush()

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