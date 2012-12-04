//Authors: Brandon Snuggs and Steven Hwang
//Date: 10/5/2012


#include "contiki.h"

#include "utils.h"
#include "dev/leds.h"
#include "adc.h"
#include "dev/button-sensor.h"
#include <stdio.h> /* For printf() */

#define in_milli 1000/CLOCK_SECOND
#define FALSE 0
#define TRUE 1


static struct etimer adc_timer;

PROCESS(adc_process, "adc process");

PROCESS_NAME(adc_process);

AUTOSTART_PROCESSES(&adc_process);

//-----------------------------------------------------------------
PROCESS_THREAD(adc_process, ev, data)
{   
  PROCESS_BEGIN();


  static unsigned short rtime;
  static clock_time_t start;
  static clock_time_t end;
  static int elapsed;
  static int is_red_on, player1_win;
  static unsigned int thresh = 500;
  static int p1,p2;

  SENSORS_ACTIVATE(button_sensor);
  adc_init();
  leds_init();
  clock_init(); 
  leds_on(LEDS_GREEN); 
  
  static struct etimer wait_red;  // this is an event timer
  static struct etimer reaction_time;
  static struct etimer adc_timer;
  static int counter=0;

  while(1) {
    rtime = (random_rand()%5)+5;

    PROCESS_WAIT_EVENT_UNTIL((ev==sensors_event) && (data == &button_sensor));
    is_red_on = FALSE;
    leds_off(LEDS_GREEN);
 
    printf("THE GAME HAS BEGUN. SO PAY ATTENTION!\n");
    etimer_set(&wait_red, rtime*60);  //The timer for the red light to show starts.
    etimer_set(&adc_timer, CLOCK_SECOND/100);

    
    p1 = 0;
    p2 = 0;
    while(1){
      PROCESS_WAIT_EVENT();
      if (ev == PROCESS_EVENT_TIMER && data == &wait_red){
        printf("We're breaking this loop!\n");
        break;
      }
      //Check to see if sensors were pressed way too early!
      if ((is_red_on == FALSE) && ( (p1==5) || (p2==5) )){
        printf("You pressed too early!\n");
        if (p1==5){
          printf("Player 1 you lose, SUCKAH!!\n");
        }else if (p2==5){
          printf("Player 2 you lose, SUCKAH!!\n");
        }
        PROCESS_EXIT();
      }

      adc_service();
      if ((adc_reading[5] < thresh) && (p1 < 5)){
        p1++;
      }else{
        p1=0;
      }
      if ((adc_reading[6] < thresh) && (p2 < 5)){
        p2++;
      }else{
        p2=0;
      }
      etimer_reset(&adc_timer);

    }
  
         
    //PROCESS_WAIT_EVENT(ev == PROCESS_EVENT_TIMER);
    leds_on(LEDS_RED);
    is_red_on = TRUE;
    //printf("GO GO GO GO GO GO GO!!!!!\n");
    start = clock_time();


    
    //WAIT FOR THE BUTTON AGAIN!
    p1 = 0;
    p2 = 0;
    while( (p1 != 5) && (p2 != 5) ){
      adc_service();
      if ((adc_reading[5] < thresh) && (p1 < 5)){
        player1_win = TRUE;
        p1++;
      }else{
        p1=0;
      }
      if ((adc_reading[6] < thresh) && (p2 < 5)){
        player1_win = FALSE;
        p2++;
      }else{
        p2=0;
      }
    }
    end = clock_time();
    leds_off(LEDS_RED);
    

    //printf("Start: %d, End: %d\n", start, end);  
    elapsed = (end - start)*in_milli;
    
    if (player1_win == TRUE){
      printf("Congratulations player 1!\n");
    }else{
      printf("Congratulations player 2!\n");
    }
    printf("Wow, you got %d milliseconds!\n",elapsed);
    printf("Think your tough huh?\n");

    leds_on(LEDS_GREEN);
  }
  PROCESS_END();
}
