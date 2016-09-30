#!/usr/bin/env python3
import getpass
import json
import superstar
import sys
import time

auth=''
ss=superstar.superstar_t('http://127.0.0.1:8081')
rx_start_time=0
rx_end_time=0

rx_avg=0
tx_avg=0
total_avg=0
times=5000

def first_set(res):
	global rx_start_time
	global ss
	rx_start_time=time.time()
	ss.get_next('/benchmarks/test',ss.sha256(json.dumps(0)),end_rx_func)

def end_rx_func(res):
	global rx_end_time
	rx_end_time=time.time()
	global auth
	global ss
	if not res or not res['value']:
		res['value']=0
	ss.set('/benchmarks/test',res['value']+1,auth,end_tx_func)
	ss.flush()

def end_tx_func(res):
	tx_end_time=time.time()
	global rx_start_time
	global rx_end_time
	global rx_avg
	global tx_avg
	global total_avg
	rx_avg+=(rx_end_time-rx_start_time)
	tx_avg+=(tx_end_time-rx_end_time)
	total_avg=(tx_end_time-rx_start_time)
	#print('RX time:    '+str(rx_end_time-rx_start_time))
	#print('TX time:    '+str(tx_end_time-rx_end_time))
	#print('Total time: '+str(tx_end_time-rx_start_time))

if __name__=='__main__':
	#auth=getpass.getpass('Auth: ')
	#for ii in range(times):
	while True:
		time.sleep(0.01)
		sys.stdout.write('\rTest '+str(ii+1)+'/'+str(times))
		sys.stdout.flush()
		ss.set('/benchmarks/test',0,auth,first_set)
		ss.flush()
	#print('')
	#print('RX avg time(ms):    '+str(rx_avg/times*1000))
	#print('TX avg time(ms):    '+str(tx_avg/times*1000))
	#print('Total avg time(ms): '+str(total_avg/times*1000))