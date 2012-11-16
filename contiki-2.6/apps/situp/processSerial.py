"""
       Authors:        Brandon Snuggs
                       Steven Hwang

        Purpose:
                processSerial was created to remove information
                from the TELOSmote.  Once this program receives
                information from the TELOSmote, it processes the
                data into a format that is readable by the human
                eye.  This program also allows us to write data
                to files easily.
"""
from time import sleep
import serial

def main():
  cycle_time = .10 #This is a tenth of a second.  Which means 10 hz.

  ser = serial.Serial('/dev/ttyUSB0', 115200, timeout=1)
  print ser
  #ser.close()

  
  while(1):
    s = ser.readline()
    print s
    sleep(cycle_time)


main()
