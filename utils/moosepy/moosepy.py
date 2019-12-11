# robot.py
# Robot class controlling drive communication
# 10/15/2019
# Tristan Van Cise
# Modified Dain Harmon 12/10/2019

import hashlib
import hmac
import json
import requests

class Robot:

    def __init__(self, password, verbose=False):
        self.password = password
        self.verbose = verbose
        self.path = "robots/2019-09/uaf/coffeebot/pilot"
        self.opts = {
            "value": {
                "power": {
                    "L": 10,
                    "R": 10
                },
                "trim": 0,
                "time": 0,
                "cmd": {
                    "run": "",
                    "arg": ""
                }
            }
        }
        self.request = {
            "jsonrpc": "2.0",
            "method": "set",
            "params": {
                "path":"robots/2019-09/uaf/coffeebot/pilot",
                "opts":"{\"value\":{\"power\":{\"L\":10,\"R\":10},\"trim\":0,\"time\":0,\"cmd\":{\"run\":\"\",\"arg\":\"\"}}}",
                "auth":"46b641297dcf6facbcc8d976825ba794ce2b277a2d6b31294f2bd72c0d329faf"
            },
            "id":4
        }

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
            print("\nRequest Data:")
            print(json.dumps(self.request, indent=4, sort_keys=True))
            print("\n")
            
        data = [self.request]
        requests.post("https://robotmoose.com/superstar", json=data)

    def drive(self, leftMotorPower, rightMotorPower):
        self.setMotorSpeed(leftMotorPower, rightMotorPower)
        self.setRequestParams()

        if self.verbose:
            print("Motor power set to:")
            print("Left: {}% \nRight: {}%".format(leftMotorPower,rightMotorPower))
            print("\n")

        self.sendRequest()