#include "contiki.h"
#include "dev/leds.h"
#include "dev/button-sensor.h"
#include <stdio.h> /* For printf() */
 
//-----------------------------------------------------------------   
PROCESS(button_process, "button example");
AUTOSTART_PROCESSES(&button_process);
//-----------------------------------------------------------------
PROCESS_THREAD(button_process, ev, data)
{   
  static uint8_t push = 0;	// number of times the user pushes the button sensor
  
  PROCESS_BEGIN();

  SENSORS_ACTIVATE(button_sensor);
  leds_init();

  leds_on(LEDS_GREEN);
  printf("+     All leds are on     +\n\n");   
  printf("Press the user button to begin\n\n");
 
  while(1) {
	PROCESS_WAIT_EVENT_UNTIL((ev==sensors_event) && (data == &button_sensor));
    if (push % 2 == 0) { 
	  leds_toggle(LEDS_GREEN);
	  printf("[%d] TURNING OFF LED ... [DONE]\n", push);
	  push++;
	} else {
	  leds_toggle(LEDS_GREEN);
	  printf("[%d] TURNING ON LED ...   [DONE]\n", push);
	  push++;
	}
	if (push == 255) { 
      push = 0;
    } // Prevents overflowing		 
  }
  PROCESS_END();
}
