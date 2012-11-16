#!/usr/bin/env python

import serial

# name of the serial port
ser = serial.Serial('/dev/ttyUSB0', 15200, timeout=0)

# file to store the raw data
f = open('raw_data/data.txt', 'w+')

# run forever until we cntl-C
while(1):
   line = ser.readline()
   if(line != NULL):
     f.write(line)
     print 'wrote to file'
   else:
     print 'lookin for data'
   
# close serial port 
ser.close()


