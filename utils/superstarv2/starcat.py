#!/usr/bin/env python3
#Mike Moss
#10/14/2016
#Shows values.

import argparse
import json
import superstar
import sys

if __name__=="__main__":
	try:
		#Parse CLI args...
		parser=argparse.ArgumentParser(description="Shows contents of superstar paths.")
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
			print(json.dumps(result,indent=4,sort_keys=True))

		#Make request...
		ss.get(args.path,onsuccess,onerror)
		ss.flush()

	except Exception as error:
		print(error)
		exit(1)
	except KeyboardInterrupt:
		exit(1)