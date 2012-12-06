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
from numpy import *
import math     
import datetime as dt
from peakdetect import *
from time import sleep,gmtime,strftime,time,mktime
from position import Position
import serial

def convert_to_GG(data, sens):
  conv = getGyroSensitivity(sens)
  for i in range(len(data)):
    data[i] = data[i]/conv

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
  if sens == "0":
    return 131.0 
  elif sens == "1":
    return 65.5
  elif sens == "2":
    return 32.8
  else:
    return 16.4

def getAccelSensitivity(sens):
  if sens == "0":
    return 16384.0
  elif sens == "1":
    return 8192.0
  elif sens == "2":
    return 4096.0
  else:
    return 2048.0

def parseSerialReading(s):
  """
        Description: This function parses data taken from the Serial Port
  """
  data = s.split(":")
  accel = data[0].split(",") #Accelerometer
  gyro = data[1].split(",") #Gyroscope
  sens = data[2].split(",") #Sensitivity Reading

  #convert string arrays to float arrays
  str_to_float(accel)
  str_to_float(gyro)

  #print "Gyro_x (in LSBs):",gyro[0]

  #convert to g's
  convert_to_AG(accel,sens[0])

  #convert to degrees/seconds
  convert_to_GG(gyro,sens[1])
  #print "Gyro_x (in Deg/Sec):",gyro[0], "\tSens:",sens[1]

  return accel,gyro,sens

def cal_compRateofChange(array,readings):
  end = len(array) - 1
  tot = 0

  for i in range(1,readings):
    tot += array[end] - array[end - i]

  print tot/readings

def peakDetection(array,tlookback):
  readings = array[-lookback:]

  max_x = Inf
  

def validPeak(data,comp_filt):
  peak_thres = 50.0
  rest_thres = 10.0
  gyro_x_thres = 2.0

   
  if data < gyro_x_thres and data > -gyro_x_thres:
    if comp_filt > peak_thres:
      #print "PEAK:",comp_filt
      return 1
    elif comp_filt < rest_thres:
      #print "REST:",comp_filt
      return 2
  
  #print "NOT SATISFY"
  return 0

def main():

  #Get the time of creation for this 
  log_time = strftime("%b-%d-%Y-%H-%M-%S", gmtime())
  time_array = []
  readings_cnt = 0
  fname = 'logs/%s.txt'%(log_time)
  f = open(fname,'w')
  pos = Position(0,0,0)
  cycle_time = .10 #This is a tenth of a second.  Which means 10 hz.
  gotOrigin = False
  gyro_x_gotOrigin = False
  
  
  ser = serial.Serial('/dev/ttyUSB0', 115200, timeout=10)
  #ser.close()

  accel_z_rate = 0
  z_rate_array = []
  cnt = 0
  
  new_accel_z = 0
  
  #Initialize a reading counter
  r_cnt = 0

  #Initialize plotter
  ion()
  x_graph=arange(0,100,1)
  y_graph=array([0]*100)
  fig=figure()
  ax=fig.add_subplot(111)
  ax.axis([0,100,-180,180])
  line1 = ax.plot(x_graph,y_graph,'r-')
  line2 = ax.plot(x_graph,y_graph,'bo')
  line3 = ax.plot(x_graph,y_graph,'go')

  comp_array = [0]*100
  local_min = [0]*100
  graph_time_array = [0]*100
  gyro_x_array = [0]*100
  gyro_x_peaks = [0]*100
  gyro_x_rests = [0]*100
  
  local_peak_count = 0
  local_rest_count = 0
  PEAK = 0
  REST = 0
  situp_cnt = 0
  max_readings = 10

  print("Finish plotter")
  
  while(1):
    
    s = ser.readline()
    old_accel_z = new_accel_z

    """
    while (s == ""):
      print "READING..."
      s = ser.readline()
      print "READING DONE"
    """

    if s == "AGDATA\n":
      while (s == "AGDATA\n"):
        s = ser.readline()
        now = dt.datetime.now()
        timestamp = round(float(now.strftime('%s.%f')),3)
        t = timestamp#This is the time since the epoch in milliseconds.
      
      
      #Strip the newline before starting
      s = s.strip("\n")
      accel,gyro,sens = parseSerialReading(s)

            
      #Print out data
      #print "Accel: ", accel, "Gyro: ", gyro
      accel_z = accel[2]
      gyro_x = gyro[0]

      if gyro_x_gotOrigin == False:
        gyro_x_origin = gyro[0]
        gyro_x_gotOrigin = True

      gyro_x -= gyro_x_origin
      
      gyro_x_array.append(gyro_x)
      time_array.append(t)
      graph_time_array.append(t)
      if len(time_array) > 1:
        R = math.sqrt( (accel[0]**2)+(accel[1]**2)+(accel[2]**2) )
        #roll = math.degrees(math.acos(accel[0]/R))
        #pitch = math.degrees(math.acos(accel[1]/R))
        #yaw = math.degrees(math.acos(accel[2]/R))
        roll = math.degrees(math.atan2(accel[0],accel[2]) + math.pi)
        pitch = math.degrees(math.atan2(accel[1],accel[2]) + math.pi)

        #gyro_z*dt/1000 converts the gyro data into degrees.
        comp_part = (gyro_x)*(time_array[r_cnt]-time_array[r_cnt-1])/1000
        
        comp_filt = ( (0.92)*(roll+comp_part) ) + ( (.08)*(accel[0]) )

        if gotOrigin == False:
          origin = comp_filt
          gotOrigin = True

        comp_filt = comp_filt - origin
        
        #print "Comp Filter: %f" % (comp_filt)
        comp_array.append(comp_filt)
        
        #Write data to log file for future usage.
        txt = "%f,%f,%f\n" % (t-time_array[0],roll,pitch)
        f.write(txt)
        
        #plot complementary filter results!
        CurrentXAxis=arange(len(comp_array)-100,len(comp_array),1)
        line1[0].set_data(CurrentXAxis,array(comp_array[-100:]))
        ax.axis([CurrentXAxis.min(),CurrentXAxis.max(),min(comp_array),max(comp_array)])

        #find local minimum
        
        #gyro_x_array.append(gyro
        #if r_cnt % 50 == 0 and r_cnt > 0:
        result = validPeak(gyro_x,comp_filt)
        if (result == 1):

          gyro_x_peaks.append(comp_filt)
          gyro_x_rests.append(-800) #This will represent a null number that is impossible 
          line2[0].set_data(CurrentXAxis,array(gyro_x_peaks[-100:]))

          if REST == 1:
            local_peak_count += 1
        elif (result == 2):

          gyro_x_rests.append(comp_filt)
          gyro_x_peaks.append(-800) #This will represent a null number that is impossible 
          line3[0].set_data(CurrentXAxis,array(gyro_x_rests[-100:]))
          local_rest_count += 1
        else:
          gyro_x_peaks.append(-800) #This will represent a null number that is impossible 
          gyro_x_rests.append(-800) #This will represent a null number that is impossible


        if PEAK == 1 and REST == 1:
          situp_cnt += 1
          local_rest_count = 0 #We should see any counts while in the peak threshold zoen
          local_peak_count = 0
          print "You have done %d situps!" % (situp_cnt)
          PEAK = 0
          REST = 0

        if PEAK == 0 and local_peak_count >= max_readings and REST == 1:
          PEAK = 1
          print "ITS A PEAK!", local_peak_count
        
        if REST == 0 and local_rest_count >= max_readings:
          REST = 1
          print "ITS A REST!", local_rest_count

        



          

        
          #ax.axis([CurrentXAxis.min(),CurrentXAxis.max(),min(gyro_x_array),max(gyro_x_array)])
        
        """
        now = len(comp_array) - 1
        if r_cnt > 10:
          if comp_array[now-1] < comp_array[now] and comp_array[now-1] < comp_array[now-2]:
            local_min.append(comp_array[now-1])
            line2[0].set_data(CurrentXAxis,array(local_min[-100:]))
        """

        #line1.set_ydata(comp_array)

        comp_filt_prev = comp_filt
        fig.canvas.draw()
        

        #print "Roll: %f\tPitch: %f\tYaw: %f\tCOMP_FILT: %f" %(roll,pitch,yaw,comp_filt)
      #print "ACCEL_Z: %f\tGYRO_Z: %f" % (accel_z, gyro_z)
      r_cnt += 1
    else:
      print s
    

    
    cnt += 1
    #sleep(cycle_time)
    

main()
