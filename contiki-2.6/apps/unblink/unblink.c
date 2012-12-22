#include "contiki.h"
#include "net/rime.h"
#include "dev/leds.h"
#include <stdio.h>
#include "random.h"

#include "dev/button-sensor.h"

#include "dev/leds.h"

#include <stdio.h>
/*---------------------------------------------------------------------------*/
PROCESS(example_broadcast_process, "Broadcast example");
PROCESS(blink_process, "Blink process");
AUTOSTART_PROCESSES(&example_broadcast_process, &blink_process);
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
PROCESS_THREAD(example_broadcast_process, ev, data)
{
  static struct etimer et;
  static rimeaddr_t addr;

  //addr.u8[0] = 241; //hardcode for now
  //addr.u8[1] = 136;
  addr.u8[0] = 100; //hardcode for now
  addr.u8[1] = 255;
  
  PROCESS_EXITHANDLER(broadcast_close(&broadcast);)

  PROCESS_BEGIN();

  broadcast_open(&broadcast, 128, &broadcast_call);
  unicast_open(&uc, 146, &unicast_callbacks);

  while(1) {

    /* Delay 2-4 seconds */
    etimer_set(&et, CLOCK_SECOND * 4 + random_rand() % (CLOCK_SECOND * 4));

    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

    packetbuf_copyfrom("Cool man", 9);
    broadcast_send(&broadcast);
    printf("broadcast message sent\n");

    if(!rimeaddr_cmp(&addr, &rimeaddr_node_addr) && random_rand() % 2) {
      packetbuf_copyfrom("Uni-hello!!", 16);
      unicast_send(&uc, &addr);
    printf("unicast message sent\n");
    }

  }

  PROCESS_END();
}

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
