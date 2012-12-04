#include "contiki.h"
#include "dev/button-sensor.h"
#include <stdio.h> //for printf.
#include "i2c.h"
#include "situp.h"

#define CYCLES 20 //Number of readings per second.
#define BUF_SIZE 1 //Number of readings to average.
#define AXIS_NUM 3

#define WAITING 0
#define RECORDING 1

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

int16_t convert(uint8_t msb, uint8_t lsb){
  int16_t bits = (msb << 8) | (lsb);
  return bits;
}

int16_t getAverage(int16_t * buffer, int size){
  int32_t total = 0;
  int i;

  for (i=0;i<size;i++){
    total += buffer[i];
  }

  return (int16_t) (total/size);
}


/* We declare the process */
PROCESS(mpu6050_process, "Situp process");

/* We require the processes to be started automatically */
AUTOSTART_PROCESSES(&mpu6050_process);

/* This is the implementation of our process */
PROCESS_THREAD(mpu6050_process, ev, data)
{
  // Variables are declared static to ensure their values are kept
  // between kernel calls.
  static struct etimer timer;  // this is an event timer
  static uint8_t accel_dat[6],gyro_dat[6];
  static uint8_t accel_fs,gyro_fs;
  static int16_t accel[AXIS_NUM],gyro[AXIS_NUM];
  static int16_t accel_x[BUF_SIZE], accel_y[BUF_SIZE], accel_z[BUF_SIZE];
  static int16_t gyro_x[BUF_SIZE], gyro_y[BUF_SIZE], gyro_z[BUF_SIZE];
  static int i=0,j=0,z=1, count=0;
  static int state = WAITING;


  // any process must start wtih this
  PROCESS_BEGIN();
  SENSORS_ACTIVATE(button_sensor);
  
  // start the i2c module; must be called once before using
  i2c_enable();
  turn_sensor_on();
  
  
  while (1) {
    //TODO: Make this if statement work.
    //Check if button was pressed during recording. 
    if (state == RECORDING){
      if ((ev==sensors_event) && (data == &button_sensor)){
        state = WAITING;
      }
    }
    
    //Wait for initial button press
    if (state == WAITING){
      PROCESS_WAIT_EVENT_UNTIL((ev == sensors_event) && (data == &button_sensor));
      state = RECORDING;
    }


    // we set the timer from here every time
    etimer_set(&timer, CLOCK_CONF_SECOND/CYCLES);
    
    // and wait until the vent we receive is the one we're waiting for
    PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_TIMER);
    
    //`printf("Data is %X\n", i2c_read_byte(MPU6050_ADDRESS, MPU6050_RA_WHO_AM_I));
    
    //Read data from the accelerometer
    i2c_read_bytes(MPU6050_ADDRESS,MPU6050_ACCEL,6,accel_dat);

    //Read data from the gyroscope
    i2c_read_bytes(MPU6050_ADDRESS, MPU6050_GYRO,6,gyro_dat);
    
    //convert the data
    j=0;
    for (i=0;i<sizeof(accel_dat);i += 2){
      accel[j] = convert(accel_dat[i],accel_dat[i+1]);
      gyro[j] = convert(gyro_dat[i],gyro_dat[i+1]);
      j++;
    }
    

    //DETECTION PHASE
    
    accel_x[count] = accel[0]; //X
    accel_y[count] = accel[1]; //Y
    accel_z[count] = accel[2]; //Z
    gyro_x[count] = gyro[0];
    gyro_y[count] = gyro[1];
    gyro_z[count] = gyro[2];


    printf("AGDATA\n");
    printf("%d,",accel[0]);
    printf("%d,",accel[1]);
    printf("%d:",accel[2]);
    printf("%d,",gyro[0]);
    printf("%d,",gyro[1]);
    printf("%d:",gyro[2]);
    accel_fs = i2c_read_byte(MPU6050_ADDRESS, MPU6050_ACCEL_FS);
    printf("%d,", ( (accel_fs & 0x18) >> 3));
    gyro_fs = i2c_read_byte(MPU6050_ADDRESS, MPU6050_GYRO_FS);
    printf("%d\n", ( (gyro_fs & 0x18) >> 3));
    
    /*
    if ( count == BUF_SIZE ){
      printf("%d,", getAverage(&accel_y,BUF_SIZE));
      printf("%d, ", getAverage(&accel_z,BUF_SIZE));
      printf("%d, ", getAverage(&gyro_x,BUF_SIZE));
      printf("%d, ", getAverage(&gyro_y,BUF_SIZE));
      printf("%d\n ", getAverage(&gyro_z,BUF_SIZE));
        
      count = 0;
      continue;
    }
    */
    count++;
  }
  PROCESS_END();
}
