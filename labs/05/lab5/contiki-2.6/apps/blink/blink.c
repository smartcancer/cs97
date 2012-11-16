#include "contiki.h"
#include "dev/leds.h"
#include <stdio.h>

/* We declare the process */
PROCESS(blink_process, "Blink process");

/* We require the processes to be started automatically */
AUTOSTART_PROCESSES(&blink_process);

/* This is the implementation of our process */
PROCESS_THREAD(blink_process, ev, data)
{
  // Variables are declared static to ensure their values are kept
  // between kernel calls.
  static struct etimer timer;  // this is an event timer
  static uint8_t leds_state = 0;
  static clock_time_t current_time = 0;

  // any process must start wtih this
  PROCESS_BEGIN();
  
  leds_init(); // set up the LEDs on the board properly
  clock_init(); // initialize the clock library

  while (1) { 
    // we set the timer from here every time
    etimer_set(&timer, CLOCK_CONF_SECOND);
    
    // and wait until the vent we receive is the one we're waiting for
    PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_TIMER);
    current_time = clock_time();
    
    printf("Time is %d\n", current_time);

    // update the LEDs
    leds_off(0xFF);
    leds_on(leds_state);
    leds_state += 1;
  }
  PROCESS_END();
}
