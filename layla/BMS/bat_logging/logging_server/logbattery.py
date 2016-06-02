#!/usr/bin/env python2.7

#import os; activate_this=os.path.join(os.path.dirname(os.path.realpath(__file__)), 'activate_this.py'); exec(compile(open(activate_this).read(), activate_this, 'exec'), dict(__file__=activate_this)); del os, activate_this


#	Battery voltage monitoring via Arduino
#	Author - Clayton Auld <clayauld@gmail.com>
#	USAGE - python logbattery.py

#   This python program will log data coming from the ITEST BMS Arduino shield and
#   export the data to a CSV file. This file can then be used to plot the LiPo
#   charge curve using MATLAB or other analytical software.

import serial # Import python serial library
import csv  # Import python CSV library
import time # Import time library
import platform # Import platform library to determine system platform
import os.path

# Define loop function that will run until interrupted
def loop():
    start_time=time.time() # Log start time of loop program
    t=0
    while True:
        arduino.write('1')  # Send request for Cell0 voltage
        cell0=arduino.readline()    # Read Cell0 voltage

        arduino.write('2') # Send requenst for Cell1 voltage
        cell1=arduino.readline()    # Read Cell1 voltage
        #print 'Cell 1: ' + cell1 + ' V'
        #print('Time: %s seconds\n' %(time.time()-start_time))

        arduino.write('3')  # Send request for Cell2 voltage
        cell2=arduino.readline()    # Read Cell2 voltage
        #print 'Cell 2: ' + cell2 + ' V'
        #print('Time: %s seconds\n' %(time.time()-start_time))

        arduino.write('4')  # Send request for Cell2 voltage
        percent=arduino.readline()    # Read Cell2 voltage
        #print 'Cell 2: ' + cell2 + ' V'
        #print('Time: %s seconds\n' %(time.time()-start_time))

        current_time=time.time()-start_time # Find time since program start

        with open('./battery_data.csv', 'a') as csvfile: # Open CSV file for logging
            DWriter=csv.writer(csvfile, dialect='excel') # Define CSV writer function
            DWriter.writerow([current_time,cell0,cell1,cell2,percent]) # Write data to CSV

        print('Data Point %d\n' %t)
        print 'Cell 0: ' + cell0 + ' V' # Print Cell0 info
        print 'Cell 1: ' + cell1 + ' V' # Print Cell1 info
        print 'Cell 2: ' + cell2 + ' V' # Print Cell2 info
        print 'Percentage: ' + percent + ' %'
        print('Time: %s seconds\n' %current_time)   # Print time since program start

        print('---------------------------------\n') # Print deliminator
        t=t+1

#Set up new CSV file with data headers
with open('./battery_data.csv','w') as csvfile:
    DWriter=csv.writer(csvfile, dialect='excel') # Define CSV writer function
    DWriter.writerow(['Time (sec)','Cell0 (V)','Cell1 (V)','Cell2 (V)','Percentage'])

if platform.system()=='Darwin':
    if os.path.exists('/dev/tty.usbmodem411')==True:
        arduino=serial.Serial('/dev/tty.usbmodem411',115200,timeout=5) # Open serial port to Arduino)
        loop()
    elif (os.path.exists('/dev/tty.usbmodem621')==True):
        arduino=serial.Serial('/dev/tty.usbmodem621',115200,timeout=5)
        loop()
    else:
        print 'ERROR! DEVICE NOT FOUND!'
        exit()

elif platform.system()=='Linux':
    if (os.path.exists('/dev/ttyACM0')==True):
        arduino=serial.Serial('/dev/ttyACM0',115200,timeout=5) # Open serial port to Arduino)
        loop()
    elif (os.path.exists('/dev/ttyACM1')==True):
        arduino=serial.Serial('/dev/ttyACM1',115200,timeout=5)
        loop()
    elif (os.path.exists('/dev/ttyUSB0')==True):
        arduino=serial.Serial('/dev/ttyUSB0',115200,timeout=5)
        loop()
    elif (os.path.exists('/dev/ttyUSB1')==True):
        arduino=serial.Serial('/dev/ttyUSB1',115200,timeout=5)
        loop()
    else:
        print 'ERROR! DEVICE NOT FOUND!'
        exit()
