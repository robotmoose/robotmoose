#!/usr/bin/env python3
#Mike Moss
#07/22/2016
#Removes a path.

import argparse
import getpass
import superstar
import sys

if __name__=="__main__":
	try:
		#Parse CLI args...
		parser=argparse.ArgumentParser(description="Removes superstar paths.")
		parser.add_argument("-s","--superstar",
			dest="superstar",
			default="https://robotmoose.com",
			help="Superstar to use (default: robotmoose.com).")
		parser.add_argument("-l","--local",
			action='store_true',
			help="Sets superstar to local superstar.")
		parser.add_argument("-d","--dev",
			action='store_true',
			help="Sets superstar to test.robotmoose.com.")
		parser.add_argument("PATH",
			help="Name of path to remove.")
		args=parser.parse_args()

		#Figure out superstar...
		superstar_url=args.superstar
		if args.dev:
			superstar_url="https://test.robotmoose.com"
		if args.local:
			superstar_url="127.0.0.1:8081"
		ss=superstar.superstar_t(superstar_url)

		#Get auth...
		auth=getpass.getpass(prompt='Enter auth:  ')

		#Print errors...
		def onerror(error):
			print(str("Error("+str(error["code"])+") - "+error["message"]))
			exit(1)

		#Print success...
		def onsuccess(result):
			print("Success!")

		#Remove
		def remove(data):
			if not data:
				print("Path \""+args.PATH+"\" does not exist!")
				exit(1)
			ss.set(args.PATH,None,auth,onsuccess,onerror)
			ss.flush()

		#Get original...
		ss.get(args.PATH,remove,onerror)
		ss.flush()

	except:
		pass