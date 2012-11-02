#include <stdio.h>
#include "contiki.h"
#include "dev/leds.h"

/* We declare the process */
PROCESS(green_process, "Green process");
PROCESS(red_process, "Red process");
static uint8_t leds_state = 1;
//static struct etimer timer;
/* We require the processes to be started automatically */
AUTOSTART_PROCESSES(&green_process, &red_process);

/* This is the implementation of our process */
PROCESS_THREAD(green_process, ev, data)
{
  // Variables are declared static to ensure their values are kept
  // between kernel calls.
  static struct etimer timer;  // this is an event timer
  //static uint8_t leds_state = 1;

  // any process must start wtih this
  PROCESS_BEGIN();
  
  leds_init(); // set up the LEDs on the board properly

  while (1) { 
    // we set the timer from here every time
    etimer_set(&timer, 30);

    // and wait until the event we receive is the one we're waiting for
    PROCESS_WAIT_EVENT();
    
    
    printf("  green process timer event signaled\n");
    // update the LEDs
    if (leds_state%2 == 0){
      leds_off(1);
    }
    else{
      leds_on(1);
    }
    
    leds_state += 1;
    printf("green updated led_state: %i\n", leds_state);

  }
  PROCESS_END();

}

PROCESS_THREAD(red_process, ev, data)
{
  // Variables are declared static to ensure their values are kept
  // between kernel calls.
  static struct etimer timer;  // this is an event timer
  //static uint8_t leds_state = 1;

  // any process must start wtih this
  PROCESS_BEGIN();
  
  leds_init(); // set up the LEDs on the board properly

  while (1) { 
    // we set the timer from here every time
    etimer_set(&timer, CLOCK_CONF_SECOND);

    // and wait until the event we receive is the one we're waiting for
    PROCESS_WAIT_EVENT();
    
    printf("  red process timer event signaled\n");
    // update the LEDs
    //if (leds_state%2 == 0){
    leds_off(4);
      //}
      //else{
    leds_on(4);
    //leds_off(4);
      //}
    
    
      //leds_state += 1;
      //printf("Red updated led_state(RED): %i\n", leds_state);

  }
  PROCESS_END();

}
