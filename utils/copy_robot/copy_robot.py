#!/usr/bin/env python2

# Arsh Chauhan
# 06/20/2016
# Last Edited: 07/04/2016
# copy_robot.py: Copy an exiting robot to another robot
# Public Domain


import json
import urllib2
import urllib
import sys


def check_sub(url):
	try:
		response = urllib2.urlopen(url)
		sub_list = response.read()
	except Exception as error:
		print ("Error "+str(error) +" in ?sub for " + url)
		sys.exit(1)
	return sub_list;


#Pre: src and dst must be valid robots
#		superstar must be valid FQDN running a superstar 
#Post: src == dst 
 
def copy(src,dst,key,superstar):
	
	robots = superstar + "/superstar/robots/"
	url  =  robots + src + "/" + key + "?get"

	try:
		response = urllib2.urlopen(url)
	except urllib2.HTTPError as error:
		print (url + str(error))
		sys.exit(1)

	res_data = response.read()
	
	if len(res_data)>0:
		set_data = urllib.urlencode({"set":res_data})
		
		#set the data 
		try:
			send = urllib2.urlopen(url_dst + "?" + set_data)
	
		except Exception as error:
			print(url_dst)
			print (url_dst +": "+ str(error))
			sys.exit(1)
		sent_data = send.read()

		#read data to make sure it was set correctly
		try:
			response = urllib2.urlopen(url_dst + "?get")
			if not response.read() == res_data:
				raise Exception("Data read does not match data wrote.\n")
		except Exception as error:
			print (url_src +": "+ str(error))
			sys.exit(1)

#Pre: src must be an exiting robot on superstar
# 		supersyar must be a valid FQDN running a superstar
#Post: Calls copy on all robots in filename
def file_handler(filename,src,superstar):
	input_file=open(filename,'r')
	for robot in input_file.read().splitlines(): # Strip trailing newlines
		if robot[0] is not "#":
			print(robot)
			copy(src,robot,superstar)

		

if __name__ == '__main__':

	if len(sys.argv) < 3  :
		print("Usage: python copy_robot.py source --file <filename>/singleDest superstar (optional)")
		sys.exit(1)
	else:
		robot_file=None
		src_robot = sys.argv[1]
		if sys.argv[2] == '--file': # We have file with a list of robots
			robot_file = sys.argv[3]
			if(len(sys.argv)) == 5: #We have a superstar
				superstar = sys.argv[4]
			else:
				superstar = "http://robotmoose.com"
		else:
			dst_robot=sys.argv[2]
			if(len(sys.argv)) == 4: #We have a superstar
				superstar = sys.argv[3]
			else:
				superstar = "http://robotmoose.com"
	if robot_file is not None:
		file_handler(robot_file,src_robot,superstar)
	else:
		copy(src_robot, dst_robot ,superstar)
	
