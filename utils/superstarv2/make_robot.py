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
		parser=argparse.ArgumentParser(description="Makes a robot on superstar.")
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

		#Get auth...
		auth=getpass.getpass(prompt='Enter auth:  ')

		#Print errors...
		def onerror(error):
			print(str("Error("+str(error["code"])+") - "+error["message"]))
			exit(1)

		#Print success...
		def onsuccess(result):
			print("Created robot.")

		#Print auth success...
		def onauthsuccess(result):
			print("Set auth for robot.")

		#Do this copy...
		def do_make(data):
			if data:
				print("Robot \""+args.ROBOT+"\" exists!")
				exit(1)
			global auth
			global ss
			robot_auth=getpass.getpass(prompt='Enter new auth:  ')
			robot_auth_verify=getpass.getpass(prompt='Verify new auth:  ')
			if robot_auth!=robot_auth_verify:
				raise Exception('Auth codes do not match.')
			robot={"authtest":"","config":{"configs":[],"counter":1},"experiments":{"HelloWorld":{"code":[{"code":"// JavaScript code","name":"start","time":""}]}},"options":["ultrasonic_sensor PP","wheel_encoder PPC","encoder P","neato SP","latency ","heartbeat ","bms ","analog P","pwm P","neopixel PC","servo P","create2 S","sabertooth2 S","sabertooth1 S","bts PPPP"],"sensors":{"heartbeats":0}}
			ss.set("/robots/"+args.ROBOT,robot,auth,onsuccess,onerror)
			ss.change_auth("/robots/"+args.ROBOT,robot_auth,auth,onauthsuccess,onerror)
			ss.flush()

		#Get original...
		ss.get("/robots/"+args.ROBOT,do_make,onerror)
		ss.flush()

	except Exception as error:
		print(error)
		pass