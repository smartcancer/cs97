#include "contiki.h"
#include "dev/button-sensor.h"
#include "dev/leds.h"
#include "stdio.h" //for printf.
#include "stdlib.h"
#include "i2c.h"
#include "situp.h"
#include <string.h>
#include "net/rime.h"
#include "dev/leds.h"
#include "random.h"

#define CYCLES 50 //Number of readings per second.
#define BUF_SIZE 1 //Number of readings to average.
#define AXIS_NUM 3
#define PI 3.14159f

#define WAITING 0
#define RECORDING 1
#define FALSE 0
#define TRUE 1






const int DATA_SIZE = 3;
float REST_THRES = 0.0;
float PEAK_THRES = 0.0;

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


////////////////////////////
//  ValidPeak checks that a given gyro value is a peak
//  given a threshold
//
//
///////////////////////////

int validPeak(float data, float comp_filt){
  //float peak_thres = 50.0;
  //float rest_thres = 10.0;
  float gyro_x_thres = 5.0;

  if (data < gyro_x_thres && data > -gyro_x_thres){
    if (comp_filt > PEAK_THRES){
      //printf("I HAZ A PEAK\n");
      return 1;
    }
    else if (comp_filt < REST_THRES){
      //printf("I HAZ A REST\n");
      return 2;
    }
  }
  return 0;
}

void printFloat(float data){
  printf("Angle: %i.%i\n", (int)data, (int) ((data*100.0)-( ((int) data) * 100.0 ) ));
}

float getGyroSensitivity(uint8_t sens){
  if (sens == 0){
    return 131.0;
  }else if (sens == 1){
    return 65.5;
  }else if (sens == 2){
    return 32.8;
  }else{
    return 16.4;
  }
}

float getAccelSensitivity(uint8_t sens){
  if (sens == 0){
    return 16384.0;
  }
  else if (sens == 1){
    return 8192.0;
  }
  else if (sens == 2){
    return 4096.0;
  }
  else{
    return 2048.0;
  }
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


//WIRELESS COMMUNICATION FUNCTIONS
/*---------------------------------------------------------------------------*/
//PROCESS(example_broadcast_process, "Broadcast example");
//PROCESS(blink_process, "Blink process");
//AUTOSTART_PROCESSES(&example_broadcast_process, &blink_process);
/*---------------------------------------------------------------------------*/
static void
broadcast_recv(struct broadcast_conn *c, const rimeaddr_t *from)
{
  printf("broadcast message received from %d.%d: '%s'\n",
         from->u8[0], from->u8[1], (char *)packetbuf_dataptr());
}
static const struct broadcast_callbacks broadcast_call = {broadcast_recv};
static struct broadcast_conn broadcast;
/*---------------------------------------------------------------------------*/
static void
recv_uc(struct unicast_conn *c, const rimeaddr_t *from)
{
  printf("unicast message of size %d received from %d.%d: '%s'\n",
         packetbuf_datalen(), from->u8[0], from->u8[1], (char*)packetbuf_dataptr());
}
static const struct unicast_callbacks unicast_callbacks = {recv_uc};
static struct unicast_conn uc;
/*---------------------------------------------------------------------------*/




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
  static float accel_sens,gyro_sens;
  static int16_t accel[AXIS_NUM],gyro[AXIS_NUM];
  //static int16_t accel_x[BUF_SIZE], accel_y[BUF_SIZE], accel_z[BUF_SIZE];
  //static int16_t gyro_x[BUF_SIZE], gyro_y[BUF_SIZE], gyro_z[BUF_SIZE];
  static float accel_x, accel_y, accel_z;
  static float gyro_x, gyro_y, gyro_z;
 
  static int i=0,j=0, count=0;
  //static int z = 0;
  static int state = WAITING;
  static clock_time_t current_time_step = 0;
  static clock_time_t prev_time_step = 0;


  /* measurement variables */
  static float accel_angle_y, accel_angle_x;
  static int angle_y,angle_x;
  static float comp_filt_prev,comp_part, comp_filt;
  static float origin;
  static int local_peak_count = 0;

  static int local_rest_count = 0;
  static int PEAK = 0;
  static int REST = 0;
  static int situp_cnt = 0;
  static int max_readings = 2;
  static int result = 0;
  static int gotOrigin = FALSE;  //FALSE stands for int value 0.

  /* Clocks for various time tracking*/
  static clock_time_t situp_time, prev_time, average_time;
 
  //broadcasting messages
  static struct etimer et;
  static rimeaddr_t addr;
  static uint8_t leds_state = 0;

  addr.u8[0] = 241; //hardcode for now
  addr.u8[1] = 136;


  /************************/

  // any process must start wtih this
  PROCESS_EXITHANDLER(broadcast_close(&broadcast);)
  PROCESS_BEGIN();
  SENSORS_ACTIVATE(button_sensor);
  
  broadcast_open(&broadcast, 128, &broadcast_call);
  unicast_open(&uc, 146, &unicast_callbacks);

  
  // start the i2c module; must be called once before using
  i2c_enable();
  turn_sensor_on();
  clock_init();
  leds_init();
  prev_time = clock_time();
  
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
    
    //Read Accelerometer Sensitivity
    accel_fs = i2c_read_byte(MPU6050_ADDRESS, MPU6050_ACCEL_FS);
    accel_fs = ( (accel_fs & 0x18) >> 3);
    gyro_fs = i2c_read_byte(MPU6050_ADDRESS, MPU6050_GYRO_FS);
    gyro_fs = ( (gyro_fs & 0x18) >> 3);
    gyro_sens = getGyroSensitivity(gyro_fs);
    accel_sens = getAccelSensitivity(accel_fs);

    //printf("gryo");
    //printFloat(gyro_sens);
    //printf("accel");
    //printFloat(accel_sens);
    
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
    //printf("Accel Reading: %i\n",accel[0]);
    accel_x = ((float) accel[0])/accel_sens; //X
    accel_y = ((float) accel[1])/accel_sens; //Y
    accel_z = ((float) accel[2])/accel_sens; //Z
    gyro_x = ((float) gyro[0])/gyro_sens;
    gyro_y = ((float) gyro[1])/gyro_sens;
    gyro_z = ((float) gyro[2])/gyro_sens;
    //printFloat(accel_z);
    
    
    //Euclidean directions
    accel_angle_x = (180.0f/PI) * ( arctan2(accel_z, accel_x ) + PI);
    accel_angle_y = ((180.0f/PI) * ( arctan2(accel_z, accel_y ) + PI)) - 270;
    if (accel_angle_y < -90){
      accel_angle_y += 360;
    }
    angle_y = ( ((int)(360-accel_angle_y)) % 360);  //Come back to this!
    
    //printFloat(accel_angle_y);
    //printf("Pitch: %i.%i\n", (int)accel_angle_y, (int) ((accel_angle_y*100.0)-( ((int) accel_angle_y) * 100.0 ) ));

    //gyro_z*dt/1000 converts the gyro data into degrees.
    //static float t1 = strtod(time_array.tail); 
    //static float t2 = strtod(time_array.prev_tail); 
    

    //The gyro's origin is around -34 degrees unfortunately. So we must offset that amount.
    gyro_x -= 34.0;
    comp_part = (gyro_x)*(1.0/((float) CYCLES));
    
    //printFloat(gyro_x);
    //printFloat(comp_part);
    
    //comp_part = 0.0;
    //comp_filt = ( (0.92)*(accel_angle_y+comp_part) ) + ( (.08)*(accel[0]) );
    comp_filt = ( (0.80)*(comp_filt+comp_part) ) + ( (.20)*(accel_angle_y));


    if (gotOrigin == FALSE) {
       origin = comp_filt;
       gotOrigin = TRUE;
       PEAK_THRES = (comp_filt - origin) + 50.0;
       REST_THRES = (comp_filt - origin) + 10.0;
    }
    
    //comp filter!
    comp_filt = comp_filt - origin;

    //TO DO: make something for append stuff at end of arrays
    //enqueue(&comp_array ,comp_filt);
    
    //log files
    //txt = sprintf("%f,%f,%f\n", t-time_array[0],roll,pitch);
    //f.write(txt);
    //printf("Angle: %i,",(int) comp_filt);
    //printFloat(comp_filt);
    //printf("Pitch: %i\n",angle_y);
    
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
      
      //log the amount of time it took to get a situp.
      situp_time = clock_time();
      average_time += situp_time - prev_time;
      

      local_rest_count = 0; //We should see any counts while in the peak threshold zoen
      local_peak_count = 0;
      PEAK = 0;
      REST = 0;
      printf(" \n\nYou have completed: %i situps\n",situp_cnt);
      printf("This situp took %lu seconds to complete a situp. YAY!\n", ((unsigned long) (situp_time - prev_time)/CLOCK_SECOND));
      printf("Current situp average time: %lu", ((unsigned long) ((unsigned long)average_time)/((unsigned long)situp_cnt)/CLOCK_SECOND) );
      
      
      packetbuf_copyfrom("Situp Complete", 13);
      broadcast_send(&broadcast);
      printf("broadcast message sent\n");


      leds_off(0xFF);
      leds_on(leds_state);
      leds_state += 1;

      prev_time = situp_time;
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
