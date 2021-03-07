#include "contiki.h"
#include "net/rime/rime.h"
#include <stdio.h>
#include <stdlib.h>
#include "net/netstack.h"

/*---------------------------------------------------------------------------*/
PROCESS(example_unicast_process, "unicast receiver");
AUTOSTART_PROCESSES(&example_unicast_process);
/*---------------------------------------------------------------------------*/

int count = 0;
int total_rssi = 0;
static void recv_uc(struct unicast_conn *c, const linkaddr_t *from){
  char message[50];
  int rssi; 
  strcpy(message,(char *)packetbuf_dataptr());
  message[packetbuf_datalen()]='\0';
  rssi = (signed short)packetbuf_attr(PACKETBUF_ATTR_RSSI);
  count += 1;
  total_rssi += rssi; 
  printf("message: %s, This RSSI: %d, Total RSSI = %d, Count = %d\n", message, rssi, total_rssi, count);
 
  // //Save to a text file
  //char data[60];
  // FILE *f = fopen("data.txt","a");
  // if (f == NULL) {
  //   printf("Error opening file");
  // } else {
  //   sprintf(data, "message: %s, RSSI: %d\n", message, rssi);
  //   fputs(data, f);
  //   fclose(f);
  // }
}

static const struct unicast_callbacks unicast_callbacks = {recv_uc};
static struct unicast_conn uc;
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(example_unicast_process, ev, data){
  PROCESS_EXITHANDLER(unicast_close(&uc);)

  PROCESS_BEGIN();

  unicast_open(&uc, 146, &unicast_callbacks);

  while(1) {
    static struct etimer et;
    etimer_set(&et, CLOCK_SECOND);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
