#include "contiki.h"
#include <stdio.h>
#include "i2c.h"
#include "mpu-6050.h"

const int DATA_SIZE = 3;

// Datasheet: http://invensense.com/mems/gyro/documents/RM-MPU-6000A.pdf
//            http://invensense.com/mems/gyro/documents/PS-MPU-6000A.pdf


///////
// Reads one byte from a i2c register. 
//
// inputs: slave_addr - the 7-bit i2c address of the slave
//         register_addr - the address to read from
//
// returns: The byte read from the slave register
//////
uint8_t
i2c_read_byte(uint8_t slave_addr, uint8_t register_addr) {
  
  unsigned data; // holds the byte we have read 

  // Send start
  if (i2c_start() < 0) {
    printf("Error with start\n");
  }
  
  // Send slave write address
  if (i2c_write((slave_addr & 0x7f) << 1) != 1) {
    printf("Did not get an ack for slave write address\n");
  }
  
  // Send register address
  if (i2c_write(register_addr) != 1) {
    printf("Did not get an ack for write register address\n");
  }

  // Send start
  if (i2c_start() < 0) {
    printf("Error with second start\n");
  }
  
  // Send slave read address
  if (i2c_write((slave_addr & 0x7f) << 1 | 0x01) != 1) {
    printf("Did not get an ack for slave read address\n");
  }

  // Read data
  data = i2c_read(0); // only reading one byte so don't ack
    
  // Send stop 
  i2c_stop();
  
  return data;
}

void i2c_read_bytes(uint8_t slave_addr, uint8_t register_addr, uint8_t length, uint8_t * data) {
  
  int i=0;

  // Send start
  if (i2c_start() < 0) {
    printf("Error with start\n");
  }
  
  // Send slave write address
  if (i2c_write((slave_addr & 0x7f) << 1) != 1) {
    printf("Did not get an ack for slave write address\n");
  }
  
  // Send register address
  if (i2c_write(register_addr) != 1) {
    printf("Did not get an ack for write register address\n");
  }

  // Send start
  if (i2c_start() < 0) {
    printf("Error with second start\n");
  }
  
  // Send slave read address
  if (i2c_write((slave_addr & 0x7f) << 1 | 0x01) != 1) {
    printf("Did not get an ack for slave read address\n");
  }

  // Read data
  for(i=0;i<(length-1);i++){
    *data = i2c_read(1);
    data++;
  }
  *data = i2c_read(0); // only reading one byte so don't ack

  // Send stop 
  i2c_stop();
  
  return;
}

void i2c_write_byte(uint8_t slave_addr, uint8_t register_addr, uint8_t data) {
  //////////////////
  //Inputs:     uint8_t slave_addr      -       slave address
  //            uint8_t register_addr   -       register address
  //            uint8_t data            -       data to be written
  //
  //Outputs:    N/A
  //
  //Description: Given the slave_addr and register_addr, this function writes data to
  //             the register address.
  //////////////////

  
  // Send start
  if (i2c_start() < 0) {
    printf("Error with start\n");
  }
  
  // Send slave write address
  if (i2c_write((slave_addr & 0x7f) << 1) != 1) {
    printf("Did not get an ack for slave write address\n");
  }
  
  // Send register address
  if (i2c_write(register_addr) != 1) {
    printf("Did not get an ack for write register address\n");
  }

  //Read Data
  if (i2c_write(data) != 1){
    printf("Did not get ack for writing data");
  }

  // Send stop 
  i2c_stop();
  
}

void turn_sensor_on(){
  //////////////////
  //Inputs:     N/A
  //
  //Outputs:    N/A
  //
  //Description: Turns on the MPU6050 sensor.
  //////////////////

  uint8_t data;

  data = i2c_read_byte(MPU6050_ADDRESS, MPU6050_SLEEP);
  data = data || 0x20;
  i2c_write_byte(MPU6050_ADDRESS, MPU6050_SLEEP, data);
}

int16_t convert(uint8_t lsb, uint8_t rsb){
  int16_t bits = (lsb << 8) | (rsb);
  return bits;
}


/* We declare the process */
PROCESS(mpu6050_process, "Mpu6050 process");

/* We require the processes to be started automatically */
AUTOSTART_PROCESSES(&mpu6050_process);

/* This is the implementation of our process */
PROCESS_THREAD(mpu6050_process, ev, data)
{
  // Variables are declared static to ensure their values are kept
  // between kernel calls.
  static struct etimer timer;  // this is an event timer
  static uint8_t accel_dat[6],gyro_dat[6],temp_dat[2];
  static int16_t accel[3],gyro[3],temp;
  static int i=0,j=0;

  // any process must start wtih this
  PROCESS_BEGIN();
  
  // start the i2c module; must be called once before using
  i2c_enable();
  turn_sensor_on();

  while (1) { 
    // we set the timer from here every time
    etimer_set(&timer, CLOCK_CONF_SECOND);
    
    // and wait until the vent we receive is the one we're waiting for
    PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_TIMER);
    
    //`printf("Data is %X\n", i2c_read_byte(MPU6050_ADDRESS, MPU6050_RA_WHO_AM_I));
    
    //Read data from the accelerometer
    i2c_read_bytes(MPU6050_ADDRESS,MPU6050_ACCEL,6,accel_dat);

    //Read data from the gyroscope
    i2c_read_bytes(MPU6050_ADDRESS, MPU6050_GYRO,6,gyro_dat);

    //Read data from the temperature sensor
    i2c_read_bytes(MPU6050_ADDRESS, MPU6050_TEMP,2,temp_dat);
    
    for (i=0;i<sizeof(accel_dat);i += 2){
      accel[i] = convert(accel_dat[i],accel_dat[i+1]);
      gyro[i] = convert(gyro_dat[i],gyro_dat[i+1]);
    }
    temp = convert(temp_dat[0],temp_dat[1]);

    printf("Accelerometer Readings:");
    for(i=0;i<DATA_SIZE;i++){
      printf(" %d",accel[i]);
    }
    printf("\n");

    printf("Gyroscope Readings:");
    for(i=0;i<DATA_SIZE;i++){
      printf(" %d",gyro[i]);
    }
    printf("\n");
     
    //static double true_temp = (double) temp;
    //temp = (true_temp/340.00) + 36.53;
    printf("Temperature Reading: %d\n\n", temp);
  }
  PROCESS_END();
}
