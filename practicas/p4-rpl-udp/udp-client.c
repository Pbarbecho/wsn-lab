/*
 * Práctica 4 - Red multisalto con RPL y UDP: nodo sensor (cliente)
 *
 * Cuando el nodo se ha unido al DODAG (tiene ruta hacia la raíz) envía cada
 * SEND_INTERVAL una muestra "n<id>,s<seq>,t<temp>" a la dirección de la raíz.
 * Imprime periódicamente su padre preferido y su rango RPL.
 */
#include "contiki.h"
#include "net/routing/routing.h"
#include "net/netstack.h"
#include "net/ipv6/simple-udp.h"
#include "sys/node-id.h"
#include "lib/random.h"
#if ROUTING_CONF_RPL_LITE
#include "net/routing/rpl-lite/rpl.h"
#endif
#include <stdio.h>
#include <string.h>
#include <inttypes.h>

#include "sys/log.h"
#define LOG_MODULE "P4"
#define LOG_LEVEL LOG_LEVEL_INFO

#define UDP_CLIENT_PORT 8765
#define UDP_SERVER_PORT 5678

#ifndef SEND_INTERVAL
#define SEND_INTERVAL (CLOCK_SECOND * 10)
#endif

static struct simple_udp_connection udp_conn;
static uint16_t last_parent = 0;

/*---------------------------------------------------------------------------*/
/* Informa del padre preferido y del rango RPL. La línea "#L <id> 1" la
 * interpreta Cooja (View -> Mote relations) para dibujar el DODAG.          */
static void
report_parent(void)
{
#if ROUTING_CONF_RPL_LITE
  rpl_nbr_t *parent = curr_instance.dag.preferred_parent;
  uint16_t pid = 0;
  if(parent != NULL) {
    const linkaddr_t *ll = rpl_neighbor_get_lladdr(parent);
    if(ll != NULL) {
      pid = (ll->u8[LINKADDR_SIZE - 2] << 8) | ll->u8[LINKADDR_SIZE - 1];
    }
  }
  if(pid != last_parent) {
    if(last_parent != 0) {
      printf("#L %u 0\n", last_parent);
    }
    if(pid != 0) {
      printf("#L %u 1\n", pid);
    }
    last_parent = pid;
  }
  printf("RPL node=%u parent=%u rank=%u\n", node_id, pid, curr_instance.dag.rank);
#endif
}

/*---------------------------------------------------------------------------*/
static void
udp_rx_callback(struct simple_udp_connection *c,
                const uip_ipaddr_t *sender_addr, uint16_t sender_port,
                const uip_ipaddr_t *receiver_addr, uint16_t receiver_port,
                const uint8_t *data, uint16_t datalen)
{
  LOG_INFO("Respuesta '%.*s' de la raiz\n", datalen, (const char *)data);
}
/*---------------------------------------------------------------------------*/
PROCESS(udp_client_process, "P4 cliente UDP");
AUTOSTART_PROCESSES(&udp_client_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(udp_client_process, ev, data)
{
  static struct etimer periodic_timer;
  static char buf[40];
  static uint32_t seq = 0;
  static uint32_t not_reachable = 0;
  static int temp = 250;
  uip_ipaddr_t root_addr;

  PROCESS_BEGIN();

  simple_udp_register(&udp_conn, UDP_CLIENT_PORT, NULL,
                      UDP_SERVER_PORT, udp_rx_callback);

  etimer_set(&periodic_timer, random_rand() % SEND_INTERVAL);
  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));

    temp += (int)(random_rand() % 11) - 5;

    if(NETSTACK_ROUTING.node_is_reachable() &&
       NETSTACK_ROUTING.get_root_ipaddr(&root_addr)) {
      snprintf(buf, sizeof(buf), "n%u,s%" PRIu32 ",t%d", node_id, seq, temp);
      simple_udp_sendto(&udp_conn, buf, strlen(buf), &root_addr);
      printf("TX node=%u seq=%" PRIu32 "\n", node_id, seq);
      seq++;
      if(seq % 3 == 0) {
        report_parent();
      }
    } else {
      not_reachable++;
      LOG_INFO("Sin ruta a la raiz (%" PRIu32 " intentos)\n", not_reachable);
    }

    /* Jitter de +-0.5 s para evitar sincronización entre nodos */
    etimer_set(&periodic_timer, SEND_INTERVAL - CLOCK_SECOND / 2
               + (random_rand() % CLOCK_SECOND));
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
