#!/usr/bin/env python3
#Mike Moss
#10/14/2016
#Makes robots.

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
			Makes a robot on superstar.
			  You can clear a robot auth code by setting it to '-'.
			  You can make a robot auth code not changeable by setting it '!'.
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
		parser.add_argument("ROBOT",
			help="Full path of robot.")
		args=parser.parse_args()

		#Figure out superstar...
		superstar_url=args.superstar
		if args.dev:
			superstar_url="test.robotmoose.com"
		if args.local:
			superstar_url="127.0.0.1:8081"
		ss=superstar.superstar_t(superstar_url)

		#Check robot
		if len(ss.pathify(args.ROBOT).split('/'))!=3:
			raise Exception("Invalid robot name (expect YEAR/SCHOOL/NAME).")

		#Print errors...
		def onerror(error):
			print(str("Error("+str(error["code"])+") - "+error["message"]))
			exit(1)

		#Print success...
		def onsuccess(result):
			print("Created robot.")

		#Print auth success...
		def onauthsuccess(result):
			if result:
				print("Set auth for robot.")
			else:
				print("Failed to set auth code.")

		#Do this copy...
		def do_make(data):
			#Robot exists, exit
			if data:
				answer=""
				while True:
					answer=input("Robot \""+args.ROBOT+"\" exists, overwrite (y/n)?  ")
					if answer=="y" or answer=="Y":
						break
					if answer=="n" or answer=="N":
						exit(1)

			#Get auth...
			auth=getpass.getpass(prompt='Enter admin auth:  ')

			#Get new auth
			robot_auth=""
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

			#Set robot
			global ss
			robot={"authtest":"","config":{"configs":[],"counter":1},"experiments":{"HelloWorld":{"code":[{"code":"// JavaScript code","name":"start","time":""}]}},
				"options":["ultrasonic_sensor PP","wheel_encoder PPC","encoder P","neato SP","latency ","heartbeat ","bms ","analog P","pwm P","neopixel PC","servo P",
				"create2 S","sabertooth2 S","sabertooth1 S","bts PPPP"],"sensors":{"heartbeats":0},"run":{"options":["Squeak","eagle","bike horn"],"play":False,"sound":"eagle"}}
			ss.set("/robots/"+args.ROBOT,robot,auth,onsuccess,onerror)
			if len(robot_auth)>1:
				robot_auth=hashlib.sha256(bytearray(robot_auth,"utf-8")).hexdigest()
			ss.change_auth("/robots/"+args.ROBOT,robot_auth,auth,onauthsuccess,onerror)
			ss.flush()

		#Get original...
		ss.get("/robots/"+args.ROBOT,do_make,onerror)
		ss.flush()

	except Exception as error:
		print(error)
		exit(1)
	except KeyboardInterrupt:
		exit(1)
