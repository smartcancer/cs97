#include <stdio.h>
#include "contiki.h"

/* We declare the process */
PROCESS(hello_world_process, "Hello world process");

/* We require the processes to be started automatically */
AUTOSTART_PROCESSES(&hello_world_process);

/* This is the implementation of our process */
PROCESS_THREAD(hello_world_process, ev, data)
{
  // Variables are declared static to ensure their values are kept
  // between kernel calls.
  static struct etimer timer;  // this is an event timer
  static int count = 0; 

  // any process must start wtih this
  PROCESS_BEGIN();

  // set the etimer module to generate an event in one second
  etimer_set(&timer, CLOCK_CONF_SECOND);
 
  while (1) { 
    // Wait here for an even to happen
    PROCESS_WAIT_EVENT();
    
    // if the event is the timer event as expected...
    if (ev == PROCESS_EVENT_TIMER) { 
      // to the process work
      printf("Hello, world #%i\n", count);
      count++;

      // reset the timer so it will generate another event
      // the exact same time after it expired (periodicity quaranteed)
      etimer_reset(&timer);
    }
  } // end loop
    
  // every process must end with this, even if it is never reached
  PROCESS_END();
}
