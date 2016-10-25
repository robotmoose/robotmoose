#!/usr/bin/env python3
#Mike Moss
#10/14/2016
#Sets password for path.

import argparse
import getpass
import hashlib
import superstar
import sys
import textwrap

if __name__=="__main__":
	try:
		#Parse CLI args...
		parser=argparse.ArgumentParser(formatter_class=argparse.RawDescriptionHelpFormatter,
			description=textwrap.dedent('''\
			Sets auth codes for superstar paths.
			  You can clear a new auth code by setting it to '-'.
			  You can make a new auth code not changeable by setting it '!'.
			'''))
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
		parser.add_argument("path",
			default="/",
			nargs='?',
			help="Path to set auth for.")
		args=parser.parse_args()

		#Figure out superstar...
		superstar_url=args.superstar
		if args.dev:
			superstar_url="test.robotmoose.com"
		if args.local:
			superstar_url="127.0.0.1:8081"
		ss=superstar.superstar_t(superstar_url)

		#Get auth...
		auth=getpass.getpass(prompt='Enter admin auth:  ')

		#Print errors...
		def onerror(error):
			print(str("Error("+str(error["code"])+") - "+error["message"]))
			exit(1)

		#Print success...
		def onsuccess(result):
			if result:
				print("Success!")
			else:
				print("Failed to set auth code.")

		#Set Auth
		while True:
			robot_auth=""
			try:
				robot_auth=getpass.getpass(prompt='Enter new auth (at least 4 characters):  ')
				if len(robot_auth)<4 and robot_auth!='!' and robot_auth!='-' and len(robot_auth)!=0:
					raise Exception('Auth code must be at least 4 characters.')
				for ii in robot_auth:
					if ord(ii)<33 or ord(ii)>126:
						raise Exception('Auth code can only contain non-whitespace ascii characters.')
				robot_auth_verify=getpass.getpass(prompt='Verify new auth:  ')
				if robot_auth!=robot_auth_verify:
					raise Exception('Auth codes do not match.')
				break
			except Exception as error:
				print(error)
			except KeyboardInterrupt:
				exit(1)

		#Empty auth check
		if len(robot_auth)==0:
			answer=""
			while True:
				answer=input('Auth is blank, remove it instead of setting it to "" (y/n)?  ')
				if answer=="y" or answer=="Y":
					robot_auth="-"
					break
				if answer=="n" or answer=="N":
					break
		if robot_auth!="!" and robot_auth!="-":
			robot_auth=hashlib.sha256(bytearray(robot_auth,"utf-8")).hexdigest()
		ss.change_auth(args.path,robot_auth,auth,onsuccess,onerror)
		ss.flush()

	except Exception as error:
		print(error)
		exit(1)
	except KeyboardInterrupt:
		exit(1)