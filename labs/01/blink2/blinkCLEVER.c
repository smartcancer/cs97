#include <stdio.h>
#include "contiki.h"
#include "dev/leds.h"

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
  static uint8_t leds_state = 1;

  // any process must start wtih this
  PROCESS_BEGIN();
  
  leds_init(); // set up the LEDs on the board properly

  while (1) { 
    // we set the timer from here every time
    etimer_set(&timer, 15);

    // and wait until the event we receive is the one we're waiting for
    PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_TIMER);
    
    // update the LEDs
    leds_off(0xFF);
    if ( leds_state%4 == 0){
      leds_on(4);
    }
    
    if ( leds_state%2 == 0){
      leds_on(1);
    }

    
    leds_state += 1;
    printf("WTH is led_state: %i\n", leds_state);

  }
  PROCESS_END();

}
