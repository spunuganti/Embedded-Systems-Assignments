#include "contiki.h"
#include "contiki-lib.h"
#include "sys/compower.h"
#include "sys/energest.h"
#include "powertrace.h"
#include <stdio.h>
#include <string.h>

PROCESS(powertrace_process, "Periodic power output");
/*---------------------------------------------------------------------------*/
void
powertrace_print(char *str)
{
  static uint32_t last_cpu, last_lpm, last_transmit, last_listen;
  uint32_t all_cpu, all_lpm, all_transmit, all_listen;
  uint32_t cpu, lpm, transmit, listen;

  energest_flush();

  all_cpu = energest_type_time(ENERGEST_TYPE_CPU);
  all_lpm = energest_type_time(ENERGEST_TYPE_LPM);
  all_transmit = energest_type_time(ENERGEST_TYPE_TRANSMIT);
  all_listen = energest_type_time(ENERGEST_TYPE_LISTEN);

  cpu = all_cpu - last_cpu;
  lpm = all_lpm - last_lpm;
  transmit = all_transmit - last_transmit;
  listen = all_listen - last_listen;

  energest_flush();

  last_cpu = energest_type_time(ENERGEST_TYPE_CPU);
  last_lpm = energest_type_time(ENERGEST_TYPE_LPM);
  last_transmit = energest_type_time(ENERGEST_TYPE_TRANSMIT);
  last_listen = energest_type_time(ENERGEST_TYPE_LISTEN);

  printf("%lu %lu %lu %lu %lu %lu %lu %lu\n",
	 all_cpu, all_lpm, all_transmit, all_listen, cpu, lpm, transmit, listen);
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(powertrace_process, ev, data)
{
  static struct etimer periodic;
  clock_time_t *period;
  PROCESS_BEGIN();

  period = data;

  if(period == NULL) {
    PROCESS_EXIT();
  }
  etimer_set(&periodic, *period);

  while(1) {
    PROCESS_WAIT_UNTIL(etimer_expired(&periodic));
    etimer_reset(&periodic);
    powertrace_print("");
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
void
powertrace_start(clock_time_t period)
{
  process_start(&powertrace_process, (void *)&period);
}
/*---------------------------------------------------------------------------*/
void
powertrace_stop(void)
{
  process_exit(&powertrace_process);
}
/*---------------------------------------------------------------------------*/


#include "net/rime/rime.h"
#include "random.h"

#include "powertrace.h"

#include "dev/button-sensor.h"

#include "dev/leds.h"

#include <stdio.h>
/*---------------------------------------------------------------------------*/
PROCESS(example_broadcast_process, "BROADCAST example");
AUTOSTART_PROCESSES(&example_broadcast_process,&powertrace_process);
/*---------------------------------------------------------------------------*/
static void
broadcast_recv(struct broadcast_conn *c, const linkaddr_t *from)
{
  printf("broadcast message received from %d.%d: '%s'\n",
         from->u8[0], from->u8[1], (char *)packetbuf_dataptr());
}
static const struct broadcast_callbacks broadcast_call = {broadcast_recv};
static struct broadcast_conn broadcast;
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(example_broadcast_process, ev, data)
{
  static struct etimer et;

  PROCESS_EXITHANDLER(broadcast_close(&broadcast);)

  PROCESS_BEGIN();


  /* Start powertracing, once every two seconds. */
  powertrace_start(CLOCK_SECOND * 2);
  
  broadcast_open(&broadcast, 129, &broadcast_call);

  while(1) {

    /* Delay 2-4 seconds */
    etimer_set(&et, CLOCK_SECOND * 4 + random_rand() % (CLOCK_SECOND * 4));

    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

    packetbuf_copyfrom("Hello", 6);
    broadcast_send(&broadcast);
    printf("broadcast message sent\n");
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
