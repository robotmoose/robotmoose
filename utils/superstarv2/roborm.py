#!/usr/bin/env python3
#Mike Moss
#07/21/2016
#Removes a robot.

import getpass
import superstar
import sys

if __name__=="__main__":
	try:
		usage="Usage: "+sys.argv[0]+" SUPERSTAR ROBOT"

		#Check number of args...
		if len(sys.argv)!=3:
			print(usage)
			exit(1)

		#Valid robot...
		if sys.argv[2].count("/")!=2:
			print("Invalid robot path.")
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

		#Remove robot...
		ss.set("/robots/"+sys.argv[2],None,auth,onsuccess,onerror)
		ss.flush()

	except:
		pass