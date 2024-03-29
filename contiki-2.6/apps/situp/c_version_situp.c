#include "contiki.h"
#include "dev/button-sensor.h"
#include "stdio.h" //for printf.
#include "stdlib.h"
#include "i2c.h"
#include "situp.h"
#include <string.h>

#define CYCLES 20 //Number of readings per second.
#define BUF_SIZE 1 //Number of readings to average.
#define AXIS_NUM 3
#define PI 3.14159f

#define WAITING 0
#define RECORDING 1
#define FALSE 0
#define TRUE 1

const int DATA_SIZE = 3;

// Datasheet: http://invensense.com/mems/gyro/documents/RM-MPU-6000A.pdf
//            http://invensense.com/mems/gyro/documents/PS-MPU-6000A.pdf



/////////////////
//  Queue struct implementation
//    included pointers to head, tail and size of queue
//
//    The queue will concatenate the elements in the list as a string in FIFO
//    order. The MACRO append will delimit the current queue with the next 
//    element with a '&'.
//    
//
/////////////////
/*
struct light_queue{

  char * queue_as_string;
  char * head; 
  char * tail;
  char * prev_tail;
  int size;

} light_queue;
*/

//////////////////////////
//  Function declarations
//
//
//
//////////////////////////

//void enqueue(struct light_queue*, float new_element);
//void dequeue(struct light_queue*);
int validPeak(float data, float comp_filt);

////////////////////////////
//
//   Queue functions:
//     Enqueue: This will append the "new node" to be added at end of queue 
//              string.
//     Dequeue: This will remove the first element from the queue string.
//     Size: returns the value of the queue's size variable.
//
//
///////////////////////////

#define QUEUE_SIZE(q) q.size
//#define ENQUEUE(q,n)  q=q##&##n

/*  
void enqueue(struct light_queue * lq, float new_element){
   
  if (lq->size == 0){
    
    char new[100];

    int d1 = new_element;            // Get the integer part (678).
    float f2 = new_element - d1;     // Get fractional part (678.0123 - 678 = 0.0123).
    int d2 = (int)(f2 * 10000);   // Turn into integer (123).

    sprintf(new, "%d.%04d", d1,d2);

    lq->head = new;
    lq->tail = new;
    lq->prev_tail = new;
    lq->queue_as_string = new;
  }
  else{
    char next[100];

    int d1 = new_element;            // Get the integer part (678).
    float f2 = new_element - d1;     // Get fractional part (678.0123 - 678 = 0.0123).
    int d2 = (int)(f2 * 10000);   // Turn into integer (123).
    
    sprintf(next, "%d.%04d", d1,d2);
    lq->prev_tail = lq->tail;
    lq->tail = next;
    lq->queue_as_string = strcat(lq->queue_as_string, next);

  }
  (lq->size)++;
}
*/
/*
void dequeue(struct light_queue * lq){

  if(lq->size == 0){
    printf("No more elements");
    exit(-1);
  }
  
  else{ //hacky way to take off the first element
    
    char * pch;
    pch = strtok(lq->queue_as_string, "&");
    lq->queue_as_string = "";
    
    while (pch != NULL)
    {
      pch = strtok (NULL, "&");
      lq->queue_as_string = strcat(lq->queue_as_string, pch);
    }

  }
  (lq->size)--;

}
*/
////////////////////////////
// arrshift
//   shift all array elements over 
//
//
//
////////////////////////////





////////////////////////////
//  ValidPeak checks that a given gyro value is a peak
//  given a threshold
//
//
///////////////////////////

int validPeak(float data, float comp_filt){
  float peak_thres = 50.0;
  float rest_thres = 10.0;
  float gyro_x_thres = 2.0;

   
  if (data < gyro_x_thres && data > -gyro_x_thres){
    if (comp_filt > peak_thres){
      //print "PEAK:",comp_filt
      return 1;
    }
    else if (comp_filt < rest_thres){
      //print "REST:",comp_filt
      return 2;
    }
  }
  //#print "NOT SATISFY"
  return 0;
}

/////////////////
// Calculates atan2 with max |error| > 0.01
//
// directly copied from website: 
//    http://lists.apple.com/archives/perfoptimization-dev/2005/Jan/msg00051.html
////////////////
float arctan2( float y, float x )
{
   const float ONEQTR_PI = PI / 4.0;
   const float THRQTR_PI = 3.0 * PI / 4.0;
   float r, angle;
   float abs_y = ((y > 0.0f) ? y : -y) + 0.0000001f;      // kludge to prevent 0/0 condition
   if ( x < 0.0f )
   {
     r = (x + abs_y) / (abs_y - x);
     angle = THRQTR_PI;
   }
   else
   {
     r = (x - abs_y) / (x + abs_y);
     angle = ONEQTR_PI;
   }
   angle += (0.1963f * r * r - 0.9817f) * r;
   if ( y < 0.0f )
     return( -angle );     // negate if in quad III or IV
   else
     return( angle );
}


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
  //static uint8_t accel_fs,gyro_fs;
  static int16_t accel[AXIS_NUM],gyro[AXIS_NUM];
  //static int16_t accel_x[BUF_SIZE], accel_y[BUF_SIZE], accel_z[BUF_SIZE];
  //static int16_t gyro_x[BUF_SIZE], gyro_y[BUF_SIZE], gyro_z[BUF_SIZE];
  static int16_t accel_x, accel_y, accel_z;
  static int16_t gyro_x, gyro_y, gyro_z;
 
  static int i=0,j=0, count=0;
  //static int z = 0;
  static int state = WAITING;
  static clock_time_t current_time_step = 0;
  static clock_time_t prev_time_step = 0;


  /* measurement variables */
  static float roll;
  static float pitch;
  static float comp_filt_prev,comp_part, comp_filt;
  static float origin;
  static int local_peak_count = 0;
  /*
  static int time_array[10];
  static float comp_array[100];
  static float local_min[100];
  static float graph_time_array[100];
  static float gyro_x_array[100];
  static float gyro_x_peaks[100];
  static float gyro_x_rests[100];
  */
  /*
  static struct light_queue time_array;
  static struct light_queue comp_array;
  //static struct light_queue local_min;
  //static struct light_queue graph_time_array;
  //static struct light_queue gyro_x_array;
  static struct light_queue gyro_x_peaks;
  static struct light_queue gyro_x_rests;
  */

  static int local_rest_count = 0;
  static int PEAK = 0;
  static int REST = 0;
  static int situp_cnt = 0;
  static int max_readings = 10;
  static int result = 0;
  //static int r_cnt = 0;
  static int gotOrigin = FALSE;  //FALSE stands for int value 0.


  /************************/

  // any process must start wtih this
  PROCESS_BEGIN();
  SENSORS_ACTIVATE(button_sensor);
  
  // start the i2c module; must be called once before using
  i2c_enable();
  turn_sensor_on();
  clock_init(); 
  
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
    
    // get clock time
    prev_time_step = current_time_step;
    current_time_step = clock_time();
    //enqueue(&time_array, current_time);


    //DETECTION PHASE

    /* 
    accel_x[count] = accel[0]; //X
    accel_y[count] = accel[1]; //Y
    accel_z[count] = accel[2]; //Z
    gyro_x[count] = gyro[0];
    gyro_y[count] = gyro[1];
    gyro_z[count] = gyro[2];
    */
    
    
    accel_x = accel[0]; //X
    accel_y = accel[1]; //Y
    accel_z = accel[2]; //Z
    gyro_x = gyro[0];
    gyro_y = gyro[1];
    gyro_z = gyro[2];
    
    
    //Euclidean directions
    roll = (360.0f/PI) * ( arctan2( accel[0] , accel[2] )   + PI );
    pitch = (360.0f/PI) * ( arctan2( accel[1],accel[2]) + PI);

    //gyro_z*dt/1000 converts the gyro data into degrees.
    //static float t1 = strtod(time_array.tail); 
    //static float t2 = strtod(time_array.prev_tail); 

    comp_part = (gyro_x)*( current_time_step - prev_time_step)/1000;
    
    comp_part = 0.0;
    comp_filt = ( (0.92)*(roll+comp_part) ) + ( (.08)*(accel[0]) );

    if (gotOrigin == FALSE) {
       origin = comp_filt;
       gotOrigin = TRUE;
    }
    
    //comp filter!
    comp_filt = comp_filt - origin;

    //TO DO: make something for append stuff at end of arrays
    //enqueue(&comp_array ,comp_filt);
    
    //log files
    //txt = sprintf("%f,%f,%f\n", t-time_array[0],roll,pitch);
    //f.write(txt);
    
    
    result = validPeak(gyro_x,comp_filt);
    
    if (result == 1) {
      //*** TO DO: make a thing for appending
      //enqueue(&gyro_x_peaks, comp_filt);
      //enqueue(&gyro_x_rests, -800); //This will represent a null number that is impossible 

      if (REST == 1) {
        local_peak_count += 1;
      }
    }
    
    else if (result == 2) {

      //enqueue(&gyro_x_rests, comp_filt);
      //enqueue(&gyro_x_peaks, -800); //This will represent a null number that is impossible ;
      local_rest_count += 1;
    }
    
    else{
      //enqueue(&gyro_x_peaks, -800); //This will represent a null number that is impossible 
      //enqueue(&gyro_x_rests, -800); //This will represent a null number that is impossible
    }

    if (PEAK == 1 && REST == 1){
      situp_cnt += 1;
      local_rest_count = 0; //We should see any counts while in the peak threshold zoen
      local_peak_count = 0;
      PEAK = 0;
      REST = 0;
    }
    
    if (PEAK == 0 && local_peak_count >= max_readings && REST == 1){
      PEAK = 1;
    } 
    if (REST == 0 && local_rest_count >= max_readings) {
      REST = 1;
    }
    comp_filt_prev = comp_filt;

    /*
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
    */


    count++;
  }
  PROCESS_END();
}
