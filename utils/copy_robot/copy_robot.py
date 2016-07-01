#!/usr/bin/env python2

# Arsh Chauhan
# 06/20/2016
# copy_robot.py: Copy a key from a src robot to a dst robot
# Public Domain


import urllib2
import urllib
import sys

def check(dst,key,superstar,expected):
	url = superstar + "/superstar/robots/" + dst + "/" + key + "?get"

	try:
		response = urllib2.urlopen(url)
	except urllib2.HTTPError as error:
		print (url + str(error))
		sys.exit(1)

	received_data = response.read()

	if(expected == received_data):
		print("Succesfully copied")
	else:
		print("COPY UNSUCESSFULL")
		print(expected)
		print(received_data)


#Pre: src and dst must be valid robots
#Post: src/key == dst/key 
def copy(src,dst,key,superstar):
	
	robots = superstar + "/superstar/robots/"
	url  =  robots + src + "/" + key + "?get"

	try:
		response = urllib2.urlopen(url)
	except urllib2.HTTPError as error:
		print (url + str(error))
		sys.exit(1)

	res_data = response.read()
	
	set_data = urllib.urlencode({"set":res_data})
	
	#set the data 
	set_url = robots + dst + "/" + key + "?" + set_data
	try:
		send = urllib2.urlopen(set_url)
	except urllib2.HTTPError as error:
		print (set_url + str(error))
		sys.exit(1)
	sent_data = send.read()

	check(dst,key,superstar,res_data)


if __name__ == '__main__':

	if len(sys.argv) < 4  :
		print("Usage: python copy_robot.py source dest key superstar (optional)")
		sys.exit(1)
	else:
		src_robot = sys.argv[1]
		dst_robot = sys.argv[2]
		key = sys.argv[3]
	if(len(sys.argv)) == 5: #We have a superstar
		superstar = sys.argv[4]
	else:
		superstar = "http://robotmoose.com"

	copy(src_robot, dst_robot, key ,superstar)
	
