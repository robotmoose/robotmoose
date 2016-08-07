#!/usr/bin/env python3
#Mike Moss
#07/22/2016
#Copies a path.

import argparse
import getpass
import superstar
import sys

if __name__=="__main__":
	try:
		#Parse CLI args...
		parser=argparse.ArgumentParser(description="Copies superstar paths.")
		parser.add_argument("-s","--superstar",
			dest="superstar",
			default="robotmoose.com",
			help="Superstar to use (default: robotmoose.com).")
		parser.add_argument("-l","--local",
			action='store_true',
			help="Sets superstar to local superstar.")
		parser.add_argument("-d","--dev",
			action='store_true',
			help="Sets superstar to test.robotmoose.com.")
		parser.add_argument("FROM",
			help="Name of path to copy.")
		parser.add_argument("TO",
			help="New name for the copied path.")
		args=parser.parse_args()

		#Figure out superstar...
		superstar_url=args.superstar
		if args.dev:
			superstar_url="test.robotmoose.com"
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

		#Do this copy...
		def do_copy(data):
			if not data:
				print("Path \""+args.FROM+"\" does not exist!")
				exit(1)
			global auth
			global ss
			ss.set(args.TO,data,auth,onsuccess,onerror)
			ss.flush()

		#Get original...
		ss.get(args.FROM,do_copy,onerror)
		ss.flush()

	except Exception as error:
		print(error)
		pass