![logo](docs/moosepy.png)
# MoosePy
A python robotics library for interfacing with [RobotMoose](https://github.com/robotmoose/robotmoose) 

## Summary
MoosePy offers functionality for driving and retreiving sensors values from robots deployed using RobotMoose. It supports arbitrary robot configurations using the JSON structure found on the Superstar server.

## Installation
1. Download the latest supported version of Python3

2. Install the library with following pip command:
   ``` shell
    pip3 install moosepy
   ```

   MoosePy uses the `requests` library, which can be installed using the following command if you are experiencing dependency issues:
   ``` shell
   pip3 install requests
   ```

## Setup

Instantiating a robot object is dependent on two parameters:

| Required Parameter |      Description     |
|:------------------:|:--------------------:|
|  `superstar_path`  |The path after https://robotmoose.com/superstar/ For example, if the path to your robot is https://robotmoose.com/superstar/robots/20xx-yy/uaf/robotName then `superstar_path = robots/20xx-yy/uaf/robotName`|
|      `password`    | The robot's password |

### Usage

``` python
import moosepy

# The path after https://robotmoose.com/superstar/
superstar_path = "robots/20xx-yy/uaf/robotName"
password = "password"

robot = moosepy.Robot(superstar_path, password) 
```

## Examples

### Drive Forward
``` python
import moosepy
import time

robot = moosepy.Robot("superstar_path", "password")

# Drive forward with left and right motors at 10% speed for 1 second
robot.drive(10,10)
time.sleep(1)

# Stop the robot
robot.drive(0,0)
```

### Drive Forward, Backward, Left, and Right
``` python
import moosepy
import time

robot = moosepy.Robot("superstar_path", "password")

# Forward
robot.drive(10,10)
time.sleep(1)

# Backward
robot.drive(-10,-10)
time.sleep(1)

# Left
robot.drive(-10, 10)
time.sleep(1)

# Right
robot.drive(10, -10)
time.sleep(1)

# Stop
robot.drive(0,0)
```

### Get Sensor Data JSON
``` python
import moosepy

robot = moosepy.Robot("superstar_path", "password")
sensor_data = robot.getSensors()

print(json.dumps(sensor_data, indent=4))
```

Example Output:
``` json
{
    "backend": {
        "battery": {
            "is_charging": false,
            "percent": "2.00%",
            "time_till_discharged": "7.63 minutes"
        },
        "name": "RobotMoose",
        "version": "2017.3.6.0"
    },
    "battery": {
        "charge": 43,
        "state": 16
    },
    "heartbeats": 188,
    "power": {
        "L": 0,
        "R": 0
    }
}
```

### Control a Servo
``` python
import moosepy

robot = moosepy.Robot("superstar_path", "password")
robot.setOpt("servo", {"degree": 70})
robot.sendRequest()
```
