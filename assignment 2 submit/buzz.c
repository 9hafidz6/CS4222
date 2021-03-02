#include <stdio.h>
#include <stdlib.h>
#include "contiki.h"
//#include "sys/rtimer.h"
#include "board-peripherals.h"
#include <stdint.h>
#include "sys/etimer.h"
#include "buzzer.h"

PROCESS(process_buzz, "buzz");
AUTOSTART_PROCESSES(&process_buzz);

//======================================================================

//from etimer
static int counter_etimer;
int buzzerFrequency = 3156; // hgh notes on a piano

int state = 0; // 0 is idle, 1 is countdown, 2 is buzz
int trans_state = 1; // 1 is from 0-1, 2 is from 2-1

int final_value = 0;
int light_difference = 0;
int prev_value = 0; //store the previous value of the light sensor

static void init_opt_reading(void);
static void get_light_reading(void);

//======================================================================
//codes from etimer

void do_etimer_timeout()
{
	clock_time_t t;
 	int s, ms1,ms2,ms3;
 	t = clock_time();
  	s = t / CLOCK_SECOND;
  	ms1 = (t% CLOCK_SECOND)*10/CLOCK_SECOND;
  	ms2 = ((t% CLOCK_SECOND)*100/CLOCK_SECOND)%10;
  	ms3 = ((t% CLOCK_SECOND)*1000/CLOCK_SECOND)%10;

  	counter_etimer++;
  	printf("ETIMER: Time(E): %d (cnt) %d (ticks) %d.%d%d%d (sec) \n\n\n",counter_etimer,t, s, ms1,ms2,ms3); 

}

//======================================================================
//codes from rtimer

static void get_light_reading()
{
	int value;

	value = opt_3001_sensor.value(0);
	if(value != CC26XX_SENSOR_READING_ERROR) {
  	printf("OPT: Light=%d.%02d lux\n", value / 100, value % 100);
	} else {
  	printf("OPT: Light Sensor's Warming Up\n\n");
	}
	init_opt_reading();

	final_value = value / 100;
	light_difference = abs(final_value - prev_value);

	printf("final value: %d \nprevious_value: %d\nlightdifference: %d \n", final_value, prev_value, light_difference);

	if(light_difference >= 300 && state == 0)
	{
		printf("LUX more than 300, changing state from 0-1... \n");
		state = 1; //from idle to countdown
		trans_state = 1; // transition form 0-1
	}
	else if(light_difference >= 300 && state == 2)
	{
		printf("LUX more than 300, changing state from 2-1... \n");
		state = 1; //from buzz to countdown
		trans_state = 2; //transition from 2-1
	}
	else if(state == 1)
	{
		//i dont think suppose to be here
		state = 1;
	}
	else
	{
		printf("waiting for state transition \n");
	}

	prev_value = final_value;

}

static void init_opt_reading(void)
{
	SENSORS_ACTIVATE(opt_3001_sensor);
}

//======================================================================

PROCESS_THREAD(process_buzz, ev, data)
{
	PROCESS_BEGIN();
	
	buzzer_init();
	init_opt_reading();
	static struct etimer timer_etimer;

	while(true)
	{		
		if(state == 0) //idle
		{
			printf("STATE 0 \n");
			etimer_set(&timer_etimer, CLOCK_SECOND);  //1s timer
    		PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_TIMER);
			get_light_reading();
		}
		else if(state == 1) //countdown
		{
			printf("STATE 1 \n");
			if(trans_state == 1) // trans state from 0-1
			{
				printf("3 seconds \n");
				do_etimer_timeout();
				etimer_set(&timer_etimer, 3*CLOCK_SECOND);  //3s timer
				PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_TIMER);
				do_etimer_timeout();
				buzzer_start(buzzerFrequency);
				state = 2;
				printf("changed state to 2 \n");
			}
			else if(trans_state == 2) // trans state from 2-1
			{
				buzzer_stop();
				printf("3 seconds \n");
				do_etimer_timeout();
				etimer_set(&timer_etimer, 3*CLOCK_SECOND);  //3s timer
				PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_TIMER);
				do_etimer_timeout();
				state = 0;
				printf("changed state to 0 \n");
			}
			else
			{
				printf("error in transition");
			}
		}
		else if(state == 2) //buzz
		{
			printf("STATE 2 \n");
			etimer_set(&timer_etimer, CLOCK_SECOND/2);  //0.5s timer
    		PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_TIMER);
			get_light_reading();
		}
		else
		{
			//not supposed to be here
			printf("ERROR!! the current state value: %d", state);
		}
		PROCESS_PAUSE();
	}
	PROCESS_END();
}