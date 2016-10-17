#!/usr/bin/env python3
#Mike Moss
#10/14/2016
#Clones a single robot to a bunch of other robots.

import argparse
import getpass
import hashlib
import os
import superstar
import sys
import textwrap

if __name__=="__main__":
	try:
		#Robot data to clone...
		robot_clone_data=None
		auth=None

		#Next auth robot to make...
		next_robot=None

		#Parse CLI args...
		parser=argparse.ArgumentParser(formatter_class=argparse.RawDescriptionHelpFormatter,
			description=textwrap.dedent('''\
			Clones a single robot to a list of robots on superstar.
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
		parser.add_argument("-f","--force",
			action='store_true',
			help="Overwrite existing robots (prompts if flag not set).")
		parser.add_argument("ROBOT",
			help="Robot to clone.")
		parser.add_argument("FILE",
			help="File containing a robots and an authentication on per line of a file.")
		args=parser.parse_args()

		#Figure out superstar...
		superstar_url=args.superstar
		if args.dev:
			superstar_url="test.robotmoose.com"
		if args.local:
			superstar_url="127.0.0.1:8081"
		ss=superstar.superstar_t(superstar_url)

		#Validate clone robot
		if len(ss.pathify(args.ROBOT).split('/'))!=3:
			raise Exception("Robot to clone \""+args.ROBOT+"\" is not a valid robot name (expected YEAR/SCHOOL/NAME).")

		#Check robot file existance
		if not os.path.isfile(args.FILE):
			raise Exception("File \""+args.FILE+"\" is not a file.")

		#Open robot file
		robot_file=None
		try:
			robot_file=open(args.FILE)
		except Exception:
			raise Exception("File \""+args.FILE+"\" is not readble.")

		#Parse robots from file
		#  Robots contains (robot_name,password)
		robots=[]
		for line in robot_file.readlines():
			robot=line.strip().split()

			#Validate robot name
			if len(robot)==1:
				robot=(robot[0],'-')
			if len(robot)!=2:
				raise Exception("Invalid robot line \""+line+"\" (expected ROBOT AUTH).")
			if len(ss.pathify(robot[0]).split('/'))!=3:
				raise Exception("Invalid robot name \""+robot[0]+"\" (expected YEAR/SCHOOL/NAME).")

			#Validate auth code
			if len(robot[1])<4 and robot[1]!='!' and robot[1]!='-' and len(robot[1])!=0:
				raise Exception('Auth code must be at least 4 characters ("'+robot[0]+'").')
			for ii in robot[1]:
				if ord(ii)<33 or ord(ii)>126:
					raise Exception('Auth code can only contain non-whitespace ascii characters ("'+robot[0]+'").')

			#Add robot
			robots.append(robot)

		#Print errors...
		def onerror(error):
			print(str("Error("+str(error["code"])+") - "+error["message"]))
			exit(1)

		#Print success...
		def onsuccess(result):
			global next_robot
			print("Created robot \""+next_robot[0]+"\".")

		#Print auth success...
		def onauthsuccess(result):
			global next_robot
			if result:
				print("Set auth for \""+next_robot[0]+"\".")
			else:
				print("Failed to set auth code for \""+next_robot[0]+"\".")

		#Do this copy...
		def do_clone(data):
			#Robot exists, check it
			global next_robot
			if data and not args.force:
				answer=""
				while True:
					answer=input("Robot \""+next_robot[0]+"\" exists, overwrite (y/n)?  ")
					if answer=="y" or answer=="Y":
						break
					if answer=="n" or answer=="N":
						exit(1)

			#Get next robot
			robot_name=next_robot[0]
			robot_auth=next_robot[1]

			#Set robot
			global robot_clone_data
			global ss
			ss.set("/robots/"+robot_name,robot_clone_data,auth,onsuccess,onerror)
			if len(robot_auth)>1:
				robot_auth=hashlib.sha256(bytearray(robot_auth,"utf-8")).hexdigest()
			ss.change_auth("/robots/"+robot_name,robot_auth,auth,onauthsuccess,onerror)
			ss.flush()

		#Make sure clone robot exists...
		def get_clone(data):
			#I HATE PYTHON GLOBALS
			global auth
			global next_robot
			global robot_clone_data

			#Robot exists, check it
			if not data:
				raise Exception("Clone robot \""+args.ROBOT+"\" does not exist.")

			#Set robot
			robot_clone_data=data

			#Get auth one time...
			auth=getpass.getpass(prompt='Enter admin auth:  ')

			#Make robots
			for robot in robots:
				next_robot=robot
				ss.get("/robots/"+robot[0],do_clone,onerror)
				ss.flush()

		#Get clone robot
		ss.get("/robots/"+args.ROBOT,get_clone,onerror)
		ss.flush()

	#except Exception as error:
	#	print(error)
	#	exit(1)
	except KeyboardInterrupt:
		exit(1)
