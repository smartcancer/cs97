/*
 *  Authors: Brandon Snuggs and Steven Hwang
 *  Date: September 14, 2012
 *
 *
 *
 */

#include "contiki.h"
#include "dev/leds.h"
#include "lib/random.h"
#include "dev/button-sensor.h"
#include <stdio.h> /* For printf() */


#define in_milli 1000/CLOCK_SECOND
#define FALSE 0
#define TRUE 1

//-----------------------------------------------------------------   
PROCESS(reaction_process, "a game to test reaction time");
AUTOSTART_PROCESSES(&reaction_process);
//-----------------------------------------------------------------
PROCESS_THREAD(reaction_process, ev, data)
{   
  PROCESS_BEGIN();


  static unsigned short rtime;
  static clock_time_t start;
  static clock_time_t end;
  static int elapsed;
  static int is_red_on;

  SENSORS_ACTIVATE(button_sensor);
  leds_init();
  clock_init(); 
  leds_on(LEDS_GREEN); 
  
  static struct etimer wait_red;  // this is an event timer
  static struct etimer reaction_time;
  static int counter=0;

  while(1) {
    // Your code goes here
    rtime = (random_rand()%5);
    if (rtime == 0){
      rtime = 1;
    }
    //printf("Random is: %d\n",rtime);

    PROCESS_WAIT_EVENT_UNTIL((ev==sensors_event) && (data == &button_sensor));
    is_red_on = FALSE;
    leds_off(LEDS_GREEN);
 
    printf("THE GAME HAS BEGUN. SO PAY ATTENTION!\n");
    etimer_set(&wait_red, rtime*60);
    
    PROCESS_WAIT_EVENT();
    while (1){
      if ((is_red_on == FALSE) && (ev==sensors_event)){
        printf("You pressed too early!\n");
        PROCESS_WAIT_EVENT();
      }
      
      else{
        break;
      }
    }
    
      
     
    //PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_TIMER);
    leds_on(LEDS_RED);
    is_red_on = TRUE;
    //printf("GO GO GO GO GO GO GO!!!!!\n");
    start = clock_time();
    
    
    //WAIT FOR THE BUTTON AGAIN!
    PROCESS_WAIT_EVENT_UNTIL((ev==sensors_event) && (data == &button_sensor));
    end = clock_time();
    leds_off(LEDS_RED);
    

    //printf("Start: %d, End: %d\n", start, end);  
    elapsed = (end - start)*in_milli;

    printf("Wow, you got %d milliseconds!\n",elapsed);
    printf("Think your tough huh?\n");

    leds_on(LEDS_GREEN);
  }
  PROCESS_END();
}
