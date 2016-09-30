#!/usr/bin/env python3
import getpass
import json
import superstar
import sys
import time

auth=''
ss=superstar.superstar_t('http://137.229.25.252:443')
tx_start_time=0
tx_end_time=0
old_value=None

rx_avg=0
tx_avg=0
total_avg=0
times=2000

def err_func(res):
	print(res)
	exit(1)

def initial_get(res):
	global tx_start_time
	tx_start_time=time.time()
	global auth
	global ss
	global old_value
	if not res:
		res=0
	old_value=json.dumps(res+1)
	ss.set('/benchmarks/test',res+1,auth,end_tx_func,err_func)
	ss.flush()

def end_tx_func(res):
	global tx_end_time
	tx_end_time=time.time()
	global ss
	ss.get_next('/benchmarks/test',ss.sha256(old_value),end_rx_func,err_func)

def end_rx_func(res):
	rx_end_time=time.time()
	global tx_start_time
	global tx_end_time
	global rx_avg
	global tx_avg
	global total_avg
	rx_avg+=(rx_end_time-tx_end_time)
	tx_avg+=(tx_end_time-tx_start_time)
	total_avg=(rx_end_time-tx_start_time)
	#print('RX time:    '+str(rx_end_time-tx_end_time))
	#print('TX time:    '+str(tx_end_time-tx_start_time))
	#print('Total time: '+str(rx_end_time-tx_start_time))

if __name__=='__main__':
	try:
		if len(sys.argv)>1:
			url=sys.argv[1]
			ss.superstar=url
		#auth=getpass.getpass('Auth: ')
		for ii in range(times):
			time.sleep(0.01)
			sys.stdout.write('\rTest '+str(ii+1)+'/'+str(times))
			sys.stdout.flush()
			ss.get('/benchmarks/test',initial_get,err_func)
			ss.flush()
		print('')
		print('RX avg time(ms):    '+str(rx_avg/times*1000))
		print('TX avg time(ms):    '+str(tx_avg/times*1000))
		print('Total avg time(ms): '+str(total_avg/times*1000))
	except KeyboardInterrupt:
		exit(1)
	except Exception as error:
		print(error)
		exit(1)