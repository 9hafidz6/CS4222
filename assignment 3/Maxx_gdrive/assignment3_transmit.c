/*---------------------------------------------------------------------------*/
#include "contiki.h"
#include "board-peripherals.h"
#include <stdio.h>
#include "net/rime/rime.h"
#include <stdio.h>
#include "stdlib.h"
/*---------------------------------------------------------------------------*/


static char message[50];
static void send(char message[], int size);

PROCESS(transmit_process, "unicasting...");
AUTOSTART_PROCESSES(&transmit_process);

static const struct unicast_callbacks unicast_callbacks = {};
static struct unicast_conn uc;

//set timer to send 4 packets per second, for 10 seconds
int size;
int packets_per_second = 4;
int packet_num = 0; //Which packet is being transferred now?

int iterations = 0;

static void send(char message[], int size){
	linkaddr_t addr;
	//printf("%s\n",message);
   packetbuf_copyfrom(message, strlen(message));

   // COMPUTE THE ADDRESS OF THE RECEIVER FROM ITS NODE ID, FOR EXAMPLE NODEID 0xBA04 MAPS TO 0xBA AND 0x04 RESPECTIVELY
   // In decimal, if node ID is 47620, this maps to 186 (higher byte) AND 4 (lower byte)
   //Rui Huan: 8264, 0x2048 // HIGH: 32; LOW: 72
   // YuXuan: 55552 // HIGH: D9 = 217 ; LOW = 00 = 0
   addr.u8[0] = 217;
   addr.u8[1] = 0;
   if(!linkaddr_cmp(&addr, &linkaddr_node_addr)) {
         unicast_send(&uc, &addr);
   }

}

PROCESS_THREAD(transmit_process, ev, data) {
	static struct etimer timer_etimer;
 
	int seconds = packets_per_second * 10;//Number of seconds to iterate through 
	
	PROCESS_EXITHANDLER(unicast_close(&uc);)
	PROCESS_BEGIN();
	unicast_open(&uc, 146, &unicast_callbacks);

	 
	//could seconds be replaced by a more robust timer instead of relying on the rough amount of loops in 30seconds?
	while(iterations < seconds) { 
	//	sprintf(message, "%d", packet_num); //message = packet_num
		size = 50; //not used, but we'll set it as size of message which is 50 for now
		
		etimer_set(&timer_etimer, CLOCK_SECOND/packets_per_second);  //4 times per second
		PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_TIMER);	
		itoa(packet_num, message, 10);
		printf("message sent: %s\n", message);
		printf("Packet_num: %d\n", packet_num);
		send(message, size);
		packet_num = packet_num + 1;
		iterations = iterations + 1;
	}	
	printf("Stopped transmitting, reached end"); 

   PROCESS_END();
}

/*---------------------------------------------------------------------------*/
