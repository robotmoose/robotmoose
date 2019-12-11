### Copyright 2019 University of Alaska Fairbanks
##
### Licensed under the Apache License, Version 2.0 (the "License");
### you may not use this file except in compliance with the License.
### You may obtain a copy of the License at
##
###     http://www.apache.org/licenses/LICENSE-2.0
##
### Unless required by applicable law or agreed to in writing, software
### distributed under the License is distributed on an "AS IS" BASIS,
### WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
### See the License for the specific language governing permissions and
### limitations under the License.


import hashlib
import hmac
import json
import requests
import time

class Robot:
    def __init__(self, superstar_path, password, superstar_url="https://robotmoose.com/superstar", refresh_rate=0, verbose=False):
        self.password = password
        self.verbose = verbose
        self.path = superstar_path
        self.superstar_url = superstar_url
        self.refresh_rate = refresh_rate
        self.last_time = time.time()
        self.opts = {
            "value": self.getOpts()
        }
        self.request = {
            "jsonrpc": "2.0",
            "method": "set",
            "params": {
                "path": self.superstar_path,
                "opts": "",
                "auth": ""
            },
            "id":4
        }

    
    def getOpts(self):
        current_time = time.time()
        if current_time - self.last_time > self.refresh_rate:
            self.last_time = current_time
            return requests.get("{}/pilot".format(self.superstar_url))


    def setPassword(self, newPass):
        self.password = newPass


    def setPath(self, newPath):
        self.path = newPath


    def setOpts(self, newOpts):
        self.opts = newOpts


    def setVerbosity(self, value):
        self.verbose = value


    def getAuth(self):
        formatedPass = hashlib.sha256()
        formatedPass.update(bytearray(self.password, "utf-8"))
        formatedPass = formatedPass.hexdigest()
        
        auth = hmac.new(
            bytearray(formatedPass, "utf-8"), 
            bytearray(self.path+json.dumps(self.opts, separators=(',', ':')), "utf-8"), 
            digestmod=hashlib.sha256
        )
        
        return auth.hexdigest()


    def setMotorSpeed(self, leftMotorPower, rightMotorPower):
        self.opts["value"]["power"]["L"] = leftMotorPower
        self.opts["value"]["power"]["R"] = rightMotorPower


    def setRequestParams(self):
        self.request["params"]["path"] = str(self.path)
        self.request["params"]["opts"] = json.dumps(self.opts, separators=(',', ':'))
        self.request["params"]["auth"] = str(self.getAuth())


    def sendRequest(self):
        if self.verbose:
            print("\nRequest Data:\n", json.dumps(self.request, indent=4, sort_keys=True), "\n")
            
        data = [self.request]
        requests.post("{}/pilot".format(self.superstar_url), json=data)


    def drive(self, leftMotorPower, rightMotorPower):
        self.setMotorSpeed(leftMotorPower, rightMotorPower)
        self.setRequestParams()

        if self.verbose:
            print("Motor power set to:", "Left: {}% \nRight: {}%".format(leftMotorPower,rightMotorPower), "\n")

        self.sendRequest()


if __name__ == "__main__":
    superstar_path = "robots/2019-09/uaf/coffeebot"
    password = "example_password"
    robot = Robot(superstar_path, password, "https://robotmoose.com/superstar")