#!/usr/bin/env python3
import getpass
import json
import superstar
import time

auth=''
ss=superstar.superstar_t('http://127.0.0.1:8081')
tx_start_time=0
tx_end_time=0
old_value=None

def initial_get(res):
	global tx_start_time
	tx_start_time=time.time()
	global auth
	global ss
	global old_value
	if not res:
		res=0
	old_value=json.dumps(res+1)
	ss.set('/benchmarks/test',res+1,auth,end_tx_func)
	ss.flush()

def end_tx_func(res):
	global tx_end_time
	tx_end_time=time.time()
	global ss
	ss.get_next('/benchmarks/test',ss.sha256(old_value),end_rx_func)

def end_rx_func(res):
	rx_end_time=time.time()
	global tx_start_time
	global tx_end_time
	print('TX time:    '+str(tx_end_time-tx_start_time))
	print('RX time:    '+str(rx_end_time-tx_end_time))
	print('Total time: '+str(rx_end_time-tx_start_time))

if __name__=='__main__':
	auth=getpass.getpass('Auth: ')
	ss.get('/benchmarks/test',initial_get)
	ss.flush()