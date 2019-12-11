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
        self.pilot_path = "{}/pilot".format(self.path)
        self.superstar_url = superstar_url
        self.robot_url = "{}/{}".format(superstar_url, superstar_path)
        self.refresh_rate = refresh_rate
        self.last_time = time.time()
        self.sensors = {}
        self.opts = {
            "value": {}
        }
        self.getPilot()
        self.request = {
            "jsonrpc": "2.0",
            "method": "set",
            "params": {
                "path": self.path,
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
            url = "{}/{}".format(self.robot_url, path)
            return requests.get(url).json()
        return False

    
    def getPilot(self):
        opts = self.getData("pilot")
        if opts:
            self.opts["value"] = opts
            return opts
        else:
            return self.opts["value"]


    def recursivelySetOpt(self, level, opt_name, opt_value):
        if not isinstance(level, dict):
            return

        if isinstance(opt_value, dict):
            for key, val in opt_value.items():
                if key in level[opt_name]:
                    self.recursivelySetOpt(level[opt_name], key, val)
                else:
                    raise ValueError("Opt key '{key}' does not exist.".format(key))
        else:
            level[opt_name] = opt_value

    def setOpt(self, opt_name, opt_value):
        if opt_name in self.getPilot():
            self.recursivelySetOpt(self.getPilot(), opt_name, opt_value)
        else:
            raise ValueError("Opt name '{}' does not exist.".format(opt_name))


    def getSensors(self):
        sensors = self.getData("sensors")
        if sensors:
            self.sensors = sensors
            return sensors
        else:
            return self.sensors


    def getAuth(self):
        formatedPass = hashlib.sha256()
        formatedPass.update(bytearray(self.password, "utf-8"))
        formatedPass = formatedPass.hexdigest()
        auth = hmac.new(
            bytearray(formatedPass, "utf-8"), 
            bytearray(self.pilot_path+json.dumps(self.opts, separators=(',', ':')), "utf-8"), 
            digestmod=hashlib.sha256
        )
        
        return auth.hexdigest()

    def setLeftPower(self, leftMotorPower):
        self.setOpt("power", {"L": leftMotorPower})

    def setRightPower(self, rightMotorPower):
        self.setOpt("power", {"R": rightMotorPower})


    def setMotorPower(self, leftMotorPower, rightMotorPower):
        self.setOpt("power", {"L": leftMotorPower, "R": rightMotorPower})


    def setRequestParams(self):
        self.request["params"]["path"] = str(self.pilot_path)
        self.request["params"]["opts"] = str(json.dumps(self.opts, separators=(',', ':')))
        self.request["params"]["auth"] = str(self.getAuth())


    def sendRequest(self):
        data = [self.request]
        if data:
            requests.post(self.superstar_url, json=data)


    def drive(self, leftMotorPower=None, rightMotorPower=None):
        if leftMotorPower is not None:
            self.setLeftPower(leftMotorPower)
        if rightMotorPower is not None:
            self.setLeftPower(rightMotorPower)
        self.setRequestParams()
        self.sendRequest()
