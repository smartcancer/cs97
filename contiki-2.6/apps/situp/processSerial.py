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
from pylab import *
from time import sleep,gmtime,strftime
from position import Position
import serial

def convert_to_GG(data, sens):
  for i in range(len(data)):
    data[i] = data[i]/getGyroSensitivity(sens)

def convert_to_AG(data, sens):
  for i in range(len(data)):
    data[i] = data[i]/getAccelSensitivity(sens)

def str_to_float(s_array):
  for i in range(len(s_array)):
    #This ensures that the data gets converted. Negative numbers aren't
    #recognized by float(var).
    s_array[i] = int(s_array[i]) 
    s_array[i] = float(s_array[i])


def getGyroSensitivity(sens):
  if sens == 0:
    return 131.0 
  elif sens == 1:
    return 65.5
  elif sens == 2:
    return 32.8
  else:
    return 16.4

def getAccelSensitivity(sens):
  if sens == 0:
    return 16384.0
  elif sens == 1:
    return 8192.0
  elif sens == 2:
    return 4096.0
  else:
    return 2048.0

def main():
  time = strftime("%b-%d-%Y-%H-%M-%S", gmtime())
  fname = 'logs/%s.txt'%(time)
  file = open(fname,'w')
  pos = Position(0,0,0)
  cycle_time = .10 #This is a tenth of a second.  Which means 10 hz.

  
  ser = serial.Serial('/dev/ttyUSB0', 115200, timeout=10)
  #ser.close()

  accel_z_rate = 0
  z_rate_array = []
  cnt = 0
  
  new_accel_z = 0
  while(1):
    s = ser.readline()
    old_accel_z = new_accel_z
    #data[0] = accel_x
    #data[1] = accel_y
    #data[2] = accel_z
    
    if s == "AGDATA\n":
      s = ser.readline()
      while (s == "AGDATA\n"):
        s = ser.readline()
      s = s.strip("\n")
      data = s.split(":")

      accel = data[0].split(",") #Accelerometer
      gyro = data[1].split(",") #Gyroscope
      sens = data[2].split(",") #Sensitivity Reading

      #convert string arrays to float arrays
      str_to_float(accel)
      str_to_float(gyro)

      #convert to g's
      convert_to_AG(accel,sens[0])
      convert_to_GG(gyro,sens[1])
      
      #Print out data
      #print "Accel: ", accel, "Gyro: ", gyro
      new_accel_z = accel[2]
    else:
      print s
    


    
    if cnt != 0 and cnt != 120:
      accel_z_rate = new_accel_z - old_accel_z
      print "[%d]Z: %f" % (cnt,accel_z_rate)
      z_rate_array.append(accel_z_rate)
    elif cnt == 120:
      print "PLOTTING!"
      plot(array(arange(0,len(z_rate_array))),array(z_rate_array), marker='o', linestyle='--',color='r')

      xlabel("Time")
      ylabel("Z")
      title("Accelerometer Z")
      show()
      ser.close()
      break
    else:
      print

    cnt += 1
    sleep(cycle_time)

  print "DONE."
main()
