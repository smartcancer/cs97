#include "contiki.h"
#include "dev/leds.h"
#include "dev/button-sensor.h"
#include <stdio.h> /* For printf() */
 
//-----------------------------------------------------------------   
PROCESS(reaction_process, "a game to test reaction time");
AUTOSTART_PROCESSES(&reaction_process);
//-----------------------------------------------------------------
PROCESS_THREAD(reaction_process, ev, data)
{   
  PROCESS_BEGIN();

  SENSORS_ACTIVATE(button_sensor);
  leds_init();
  // clock_init(); 
 
  while(1) {
    // Your code goes here 
  }
  PROCESS_END();
}
