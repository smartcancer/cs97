#include "contiki.h"

#include "utils.h"
#include "dev/leds.h"
#include "adc.h"
#include <stdio.h> /* For printf() */

static struct etimer adc_timer;

PROCESS(adc_process, "adc process");

PROCESS_NAME(adc_process);

AUTOSTART_PROCESSES(&adc_process);

PROCESS_THREAD(adc_process, ev, data)
{

    PROCESS_BEGIN();

    leds_init();
    etimer_set(&adc_timer, CLOCK_SECOND/100);

    adc_init();

    while(1) {
      PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_TIMER);
      
      if(data == &adc_timer) {
        leds_toggle(LEDS_RED);
        adc_service();
        printf("%u \n", adc_reading[5]);
        etimer_reset(&adc_timer);
      }
    }

    PROCESS_END();
}
