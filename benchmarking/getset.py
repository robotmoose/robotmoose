#!/usr/bin/env python3
import getpass
import json
import superstar
import time

auth=''
ss=superstar.superstar_t('http://127.0.0.1:8081')
rx_start_time=0
rx_end_time=0

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
	print('RX time:    '+str(rx_end_time-rx_start_time))
	print('TX time:    '+str(tx_end_time-rx_end_time))
	print('Total time: '+str(tx_end_time-rx_start_time))

if __name__=='__main__':
	auth=getpass.getpass('Auth: ')
	ss.set('/benchmarks/test',0,auth,first_set)
	ss.flush()
