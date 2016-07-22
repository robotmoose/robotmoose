#!/usr/bin/env python3
#Mike Moss
#07/21/2016
#Copies a robot.

import getpass
import superstar
import sys

if __name__=="__main__":
	try:
		usage="Usage: "+sys.argv[0]+" SUPERSTAR FROM TO"

		#Check number of args...
		if len(sys.argv)!=4:
			print(usage)
			exit(1)

		#Valid from robot...
		if sys.argv[2].count("/")!=2:
			print("Invalid FROM robot path.")
			print(usage)
			exit(1)

		#Valid to robot...
		if sys.argv[3].count("/")!=2:
			print("Invalid TO robot path.")
			print(usage)
			exit(1)

		#Get auth...
		auth=getpass.getpass(prompt='Enter auth:  ')

		#Create superstar...
		ss=superstar.superstar_t(sys.argv[1])

		#Print errors...
		def onerror(error):
			print(str("Error("+str(error["code"])+") - "+error["message"]))
			exit(1)

		#Print success...
		def onsuccess(result):
			print("Success!")

		#Do this copy...
		def do_copy(data):
			global auth
			global ss
			ss.set("/robots/"+sys.argv[3],data,auth,onsuccess,onerror)
			ss.flush()

		#Get original...
		ss.get("/robots/"+sys.argv[2],do_copy,onerror)
		ss.flush()

	except:
		pass