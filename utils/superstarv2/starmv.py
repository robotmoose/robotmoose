#!/usr/bin/env python3
#Mike Moss
#10/14/2016
#Moves a path.

import argparse
import getpass
import superstar
import sys

if __name__=="__main__":
	try:
		#Parse CLI args...
		parser=argparse.ArgumentParser(description="Moves superstar paths.")
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
			help="Name of path to move.")
		parser.add_argument("TO",
			help="New name for the path.")
		args=parser.parse_args()

		#Figure out superstar...
		superstar_url=args.superstar
		if args.dev:
			superstar_url="test.robotmoose.com"
		if args.local:
			superstar_url="127.0.0.1:8081"
		ss=superstar.superstar_t(superstar_url)

		#Print errors...
		def onerror(error):
			print(str("Error("+str(error["code"])+") - "+error["message"]))
			exit(1)

		#Print success...
		def onsuccess(result):
			print("Success!")

		#Globals...
		auth=""
		copy_data=None

		#Check if to exists...
		def do_check_to(data):
			if not data:
				print("Path \""+args.FROM+"\" does not exist!")
				exit(1)

			#Global copy data...
			global copy_data
			copy_data=data

			#Get to...
			ss.get(args.TO,do_copy,onerror)
			ss.flush()

		#Do copy...
		def do_copy(data):
			if data:
				answer=""
				while True:
					answer=input("Path \""+args.TO+"\" exists, overwrite (y/n)?  ")
					if answer=="y" or answer=="Y":
						break
					if answer=="n" or answer=="N":
						exit(1)

			#Get auth...
			global auth
			auth=getpass.getpass(prompt='Enter admin auth:  ')

			#Copy
			global ss
			global copy_data
			ss.set(args.TO,copy_data,auth,do_move,onerror)
			ss.flush()

		#Remove
		def do_move(result):
			if result:
				global auth
				ss.set(args.FROM,None,auth,onsuccess,onerror)
				ss.flush()
			else:
				print("Server returned non-true for set, aborting.")
				exit(1)

		#Check from==to...
		if ss.pathify(args.FROM)==ss.pathify(args.TO):
			raise Exception("From and to are the same!")

		#Get original...
		ss.get(args.FROM,do_check_to,onerror)
		ss.flush()

	except Exception as error:
		print(error)
		exit(1)
	except KeyboardInterrupt:
		exit(1)