#!/usr/bin/env python3
#Mike Moss
#10/14/2016
#Contains client code to get requests from a superstar server.

import hashlib
import hmac
import json
import urllib.request

#Superstar object.
#  Variable self.queue to store requests in until .flush is called.
class superstar_t:
	def __init__(self,superstar):
		self.superstar=superstar
		if superstar[0:7]!="http://" and superstar[:8]!="https://":
			if superstar[0:9]=="127.0.0.1" or superstar[0:9]=="localhost":
				self.superstar="http://"+self.superstar
			else:
				self.superstar="https://"+self.superstar
		self.queue=[]

	#Helper to get the hex string sha256 sum of a string.
	def sha256(self,data):
		return hashlib.sha256(bytearray(data,"utf-8")).hexdigest()

	#Gets the value of path.
	#  Calls success_cb on success with the server response.
	#  Calls error_cb on error with the server error object (as per spec).
	def get(self,path,success_cb=None,error_cb=None):
		path=self.pathify(path)
		request=self.build_skeleton_request("get",path)
		self.add_request(request,success_cb,error_cb)

	#Sets path to the value using the given auth.
	#  Calls success_cb on success with the server response.
	#  Calls error_cb on error with the server error object (as per spec).
	def set(self,path,value,auth="",success_cb=None,error_cb=None):
		path=self.pathify(path)
		opts=json.dumps({"value":value})
		request=self.build_skeleton_request("set",path,opts)
		self.build_auth(path,request,auth)
		self.add_request(request,success_cb,error_cb)

	#Gets sub keys of the given path.
	#  Calls success_cb on success with the server response.
	#  Calls error_cb on error with the server error object (as per spec).
	def sub(self,path,success_cb=None,error_cb=None):
		path=self.pathify(path)
		request=self.build_skeleton_request("sub",path)
		self.add_request(request,success_cb,error_cb)

	#Pushes the given value onto path using the given auth.
	#  Calls success_cb on success with the server response.
	#  Calls error_cb on error with the server error object (as per spec).
	#  Note, if the path is not an array, it will be after this.
	def push(self,path,value,length,auth="",success_cb=None,error_cb=None):
		path=self.pathify(path)
		opts=json.dumps({"value":value,"length":length})
		request=self.build_skeleton_request("push",path,opts)
		self.build_auth(path,request,auth)
		self.add_request(request,success_cb,error_cb)

	#Gets the value of path when it changes.
	#  Calls success_cb on success with the server response.
	#  Calls error_cb on error with the server error object (as per spec).
	#  Note, python version is unique because it is BLOCKING.
	def get_next(self,path,last_hash=None,success_cb=None,error_cb=None):
		path=self.pathify(path)
		request=self.build_skeleton_request("get_next",path)
		request["id"]=0
		if last_hash:
			request["params"]["last_hash"]=last_hash
		request_error_handler={"error_cb":error_cb}
		try:
			#Make the request.
			data=bytes(json.dumps(request),"utf-8")
			server_response=urllib.request.urlopen(self.superstar+"/superstar/",data)

			#Parse response, call responses.
			response=json.loads(server_response.read().decode('utf-8'))

			#Got an object, must be single request...
			if isinstance(response,dict):
				#Error callback...
				if "error" in response:
					self.handle_error(request_error_handler,response["error"])

				#Success callback...
				elif "result" in response:
					if success_cb:
						success_cb(response["result"])

			#Server error...
			else:
				self.handle_error(request_error_handler,response["error"])
		except Exception as error:
			error_obj={}
			error_obj["code"]=0
			error_obj["message"]="{0}".format(error)
			self.handle_error(request_error_handler,error_obj)

	#Changes auth for the given path and auth to the given value.
	#  Calls success_cb on success with the server response.
	#  Calls error_cb on error with the server error object (as per spec).
	def change_auth(self,path,value,auth="",success_cb=None,error_cb=None):
		path=self.pathify(path)
		opts=json.dumps({"value":value})
		request=self.build_skeleton_request("change_auth",path,opts)
		self.build_auth(path,request,auth)
		self.add_request(request,success_cb,error_cb)

	#Replaces multiple slashes in path with a single slash.
	#  Removes all leading and trailing slashes.
	def pathify(self,path):
		if not path:
			return ""
		for ii in range(0,len(path)//2+1):
			path=path.replace("//","/")
		path = path.strip('/')
		return path

	#Builds a basic jsonrpc request with given method.
	#  Adds path as path and opts as opts to the params object.
	#  Note, opts is optional.
	def build_skeleton_request(self,method,path,opts=None):
		path=self.pathify(path)
		request={
			"jsonrpc":"2.0",
			"method":method,
			"params":
			{
				"path":path
			},
			"id":None
		}
		request["params"]["opts"]=opts
		return request

	#Builds HMACSHA256 for given path and request object with given auth.
	def build_auth(self,path,request,auth):
		path=self.pathify(path)
		request["params"]["auth"]=auth

	#Adds a build request into the batch queue.
	#  Note, won't be sent until .flush() is called.
	def add_request(self,request,success_cb,error_cb):
		self.queue.append({
			"request":request,
			"success_cb":success_cb,
			"error_cb":error_cb
		})

	#Builds the batch request object and clears out the current queue.
	def flush(self):
		#No requests, return.
		if len(self.queue)==0:
			return

		#Build batch of current requests.
		batch=[]
		for ii in range(0,len(self.queue)):
			request=self.queue[ii]["request"]
			path=request["params"]["path"]
			opts=request["params"]["opts"]
			request["id"]=ii
			if "auth" in request["params"]:
				auth=bytearray(hashlib.sha256(bytearray(request["params"]["auth"],"utf-8")).hexdigest(),"utf-8")
				data=bytearray(path+opts,"utf-8")
				request["params"]["auth"]=hmac.new(auth,data,digestmod=hashlib.sha256).hexdigest()
			batch.append(self.queue[ii]["request"])

		old_queue=self.queue
		self.queue=[]

		try:
			#Make the request.
			data=bytes(json.dumps(batch),"utf-8")
			server_response=urllib.request.urlopen(self.superstar+"/superstar/",data)

			#Parse response, call responses.
			response=json.loads(server_response.read().decode('utf-8'))

			#Got an array, must be batch data...
			if isinstance(response,list):
				for ii in range(0,len(response)):
					if not "id" in response[ii]:
						continue

					#Error callback...
					if "error" in response[ii]:
						response_obj=response[ii]["error"]
						self.handle_error(old_queue[response[ii]["id"]],response_obj)
						continue

					#Success callback...
					if "result" in response[ii]:
						response_obj=response[ii]["result"]
						if old_queue[response[ii]["id"]]["success_cb"]:
							old_queue[response[ii]["id"]]["success_cb"](response_obj)

			#Server error...
			else:
				for ii in range(0,len(old_queue)):
					self.handle_error(old_queue[ii],response["error"])
		except Exception as error:
			error_obj={}
			error_obj["code"]=0
			error_obj["message"]="{0}".format(error)
			for key in old_queue:
				self.handle_error(key,error_obj)

	#Function to handle errors...
	def handle_error(self,request,error):
		if request["error_cb"]:
			request["error_cb"](error)
		else:
			print("Superstar error ("+str(error["code"])+") - "+error["message"])
