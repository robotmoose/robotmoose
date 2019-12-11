# drive.py - Debug tool
# Set motor speeds directly from the command line.
# 10/15/2019
# Tristan Van Cise

import robot
import getpass
import argparse
import time

parser = argparse.ArgumentParser(
    prog='drive.py', 
    usage='python3 drive.py [options] leftMotorPower rightMotorPower',
    description='Control the coffee bot from the command line',
    epilog='Plz drive responsibly :D'
)

# Required
parser.add_argument('leftMotorPower', type=int, default=0, choices=range(0,101), help='Left motor power as integer in the range [0,100]', metavar='Left motor power')
parser.add_argument('rightMotorPower', type=int, default=0, choices=range(0,101), help='Right motor power as integer in the range [0,100]', metavar='Right motor power')

# Optional
parser.add_argument('-p', '--password', help='Robot Moose password authentication for associated robot')
parser.add_argument('-v', '--verbose', action='store_true', help='Show request data being sent to superstar server and some extra robot info')
parser.add_argument('-t', '--time', type=float, default=0.0, help='Time in seconds before robot stops (automatically stops after time limit)')
parser.add_argument('-k', '--kill', action='store_true', help='Stops the robot by settings motor values to 0')

args = parser.parse_args()

# Set motor power values
leftMotorPower = args.leftMotorPower
rightMotorPower = args.rightMotorPower

# Authentication
def handlePassword():
    if args.password != None:
        return args.password
    else:
        return getpass.getpass()

# Drive logic
if __name__ == "__main__":
    password = handlePassword()
    robot = robot.Robot(password, args.verbose)
    
    if args.kill:
        leftMotorPower = 0
        rightMotorPower = 0

    robot.drive(leftMotorPower, rightMotorPower)
    
    if args.time >= 0.01:
        time.sleep(args.time)
        robot.drive(0,0)
        if args.verbose:
            print("Halted robot driving after {} seconds".format(args.time))
