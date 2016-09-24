#!/usr/bin/env python3
import getpass
import superstar
import time

auth=''
ss=superstar.superstar_t('http://test.robotmoose.com')
rx_start_time=0
rx_end_time=0

def end_rx_func(res):
	global rx_end_time
	rx_end_time=time.time()
	global auth
	global ss
	ss.set('/benchmarks/test',res+1,auth,end_tx_func)
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
	rx_start_time=time.time()
	ss.get_next('/benchmarks/test',end_rx_func)