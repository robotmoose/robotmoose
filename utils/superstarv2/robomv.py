#!/usr/bin/env python3
#Mike Moss
#07/22/2016
#Moves a robot.

import argparse
import getpass
import superstar
import sys

if __name__=="__main__":
	try:
		#Parse CLI args...
		parser=argparse.ArgumentParser(description="Moves robotmoose robots.")
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
		parser.add_argument("FROM",
			help="Name of robot to move.")
		parser.add_argument("TO",
			help="New name of the robot.")
		args=parser.parse_args()

		#Figure out superstar...
		superstar_url=args.superstar
		if args.dev:
			superstar_url="https://test.robotmoose.com"
		if args.local:
			superstar_url="127.0.0.1:8081"
		ss=superstar.superstar_t(superstar_url)

		#Valid from robot...
		if ss.pathify(args.FROM).count("/")!=2:
			print("Invalid robot path \""+args.FROM+"\".")
			exit(1)

		#Valid to robot...
		if ss.pathify(args.TO).count("/")!=2:
			print("Invalid robot path \""+args.TO+"\".")
			exit(1)

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
		def remove(result):
			if result:
				ss.set("/robots/"+args.FROM,None,auth,onsuccess,onerror)
				ss.flush()
			else:
				print("Server returned non-true for set, aborting.")
				exit(1)

		#Do this copy...
		def do_copy(data):
			if not data:
				print("Robot \""+args.ROBOT+"\" does not exist!")
				exit(1)
			global auth
			global ss
			ss.set("/robots/"+args.TO,data,auth,remove,onerror)
			ss.flush()

		#Get original...
		ss.get("/robots/"+args.FROM,do_copy,onerror)
		ss.flush()

	except:
		pass