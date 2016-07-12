#1/usr/bin/env python3
#Mike Moss
#07/11/2016
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
        self.queue=[]

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
    #  Note, if the path is not an array, it will be after self.
    def push(self,path,value,length,auth="",success_cb=None,error_cb=None):
        path=self.pathify(path)
        opts=json.dumps({"value":value,"length":length})
        request=self.build_skeleton_request("push",path,opts)
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
        path=self.pathify(path);
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
                tmp = bytearray(request["params"]["auth"], 'utf-8')
                request["params"]["auth"]=hmac.new(tmp,digestmod=hashlib.sha256).hexdigest()
            batch.append(self.queue[ii]["request"])

        old_queue=self.queue
        self.queue=[]

        try:
            data=bytes(json.dumps(batch), 'utf-8')
            server_response=urllib.request.urlopen("http://"+self.superstar+"/superstar/", data)
            #server_response=urllib.urlopen(server_request)

            #Make the request.

            #Parse response, call responses.
            response=json.loads(server_response.read().decode('utf-8'))

            #Got an array, must be batch data...
            if isinstance(response,list):
                for key in range(0,len(response)):
                    if not "id" in response[key]:
                        continue

                    #Error callback...
                    if "error" in response[key]:
                        response_obj=response[key]["error"]
                        self.handle_error(old_queue[response[key]["id"]],response_obj)
                        continue

                    #Success callback...
                    if "result" in response[key]:
                        response_obj=response[key]["result"]
                        if old_queue[response[key]["id"]]["success_cb"]:
                            old_queue[response[key]["id"]]["success_cb"](response_obj);

            #Server error...
            else:
                for key in old_queue:
                    self.handle_error(old_queue[key],response["error"])
                    continue
        except Exception as error:
                print("Superstar error (unknown) "+str(error))

    #Function to handle errors...
    def handle_error(self,request,error):
            if request["error_cb"]:
                    request["error_cb"](error)
            else:
                    print("Superstar error ("+str(error["code"])+") "+error["message"])

if __name__=="__main__":
        def getprint(result):
                print(result)

        def subprint(result):
                print(result)

        ss=superstar_t("127.0.0.1:8081")

        ss.get("/blarg",getprint)
        ss.set("/blarg",4,"123")
        ss.get("/blarg",getprint)
        ss.sub("/",subprint)
        ss.push("/blarg2",2,3,"123")
        ss.get("/blarg2",getprint)
        ss.flush()
