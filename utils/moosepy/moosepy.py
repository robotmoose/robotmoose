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
    def __init__(self, superstar_path, password, superstar_url="https://robotmoose.com/superstar", refresh_rate=0):
        self.password = password
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

    
    def hasRefreshed(self):
        return time.time() - self.last_time > self.refresh_rate


    def getData(self, path):
        if self.hasRefreshed():
            self.last_time = time.time()
            return requests.get("{superstar_url}/{path}".format(self.superstar_url, path))

    
    def getOpts(self):
        self.getData("pilot")


    def setOpt(self, opt_name, opt_value):
        if opt_name in self.opts["value"]:
            self.opts["value"][opt_name] = opt_value
        else:
            raise ValueError("Opt name '{opt_name}' does not exist.".format(opt_name))


    def getSensors(self):
        self.getData("sensors")


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
        data = [self.request]
        requests.post("{superstar_url}/pilot".format(self.superstar_url), json=data)


    def drive(self, leftMotorPower, rightMotorPower):
        self.setMotorSpeed(leftMotorPower, rightMotorPower)
        self.setRequestParams()
        self.sendRequest()


if __name__ == "__main__":
    superstar_path = "robots/2019-09/uaf/coffeebot"
    password = "example_password"
    robot = Robot(superstar_path, password, "https://robotmoose.com/superstar")