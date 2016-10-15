#!/usr/bin/env python3
#Mike Moss
#10/14/2016
#Lists directories...

import argparse
import superstar
import sys

if __name__=="__main__":
	try:
		#Parse CLI args...
		parser=argparse.ArgumentParser(description="Lists superstar paths.")
		parser.add_argument("-s","--superstar",
			dest="superstar",
			default="robotmoose.com",
			help="Superstar to query (default: robotmoose.com).")
		parser.add_argument("-l","--local",
			action='store_true',
			help="Sets superstar to local superstar.")
		parser.add_argument("-d","--dev",
			action='store_true',
			help="Sets superstar to test.robotmoose.com.")
		parser.add_argument("path",
			default="/",
			nargs='?',
			help="Path to list.")
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
			for key in result:
				print(key);

		#Make request...
		ss.sub(args.path,onsuccess,onerror)
		ss.flush()

	except Exception as error:
		print(error)
		exit(1)
	except KeyboardInterrupt:
		exit(1)