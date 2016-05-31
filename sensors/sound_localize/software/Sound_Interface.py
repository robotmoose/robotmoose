#!/usr/bin/python

import serial
import time

messageSize = 32

sp = serial.Serial (
	port='/dev/ttyACM4', 
	baudrate=921600
	#parity='O',
	#stopbits=2
)
print sp.name
# code = bytes([11])
# sp.write(bytes([11]))
while True:
	# sp.write(chr(65))
	buff = sp.read(messageSize)
	for x in buff:
		print ord(x)

	if (buff[0] != 10) or (buff[1] != 13):
		print "Header was incorrect"
		#break
	else:
		print "Correct Header!"

sp.close()