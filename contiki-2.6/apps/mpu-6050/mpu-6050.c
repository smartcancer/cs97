#include "contiki.h"
#include <stdio.h>
#include "i2c.h"
#include "mpu-6050.h"

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

  // any process must start wtih this
  PROCESS_BEGIN();
  
  // start the i2c module; must be called once before using
  i2c_enable();

  while (1) { 
    // we set the timer from here every time
    etimer_set(&timer, CLOCK_CONF_SECOND);
    
    // and wait until the vent we receive is the one we're waiting for
    PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_TIMER);
    
    printf("Data is %X\n", i2c_read_byte(MPU6050_ADDRESS, MPU6050_RA_WHO_AM_I));
      
  }
  PROCESS_END();
}
