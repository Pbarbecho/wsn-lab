/*
 * Práctica 4 - Red multisalto con RPL y UDP: nodo raíz (sumidero)
 *
 * El servidor inicia el DODAG RPL (NETSTACK_ROUTING.root_start) y recibe las
 * muestras UDP de los clientes. Por cada paquete imprime el número de saltos
 * (a partir del hop limit IPv6) y mantiene la tasa de entrega por nodo.
 *
 * Incluye el shell serie de Contiki-NG: escriba en la consola del mote
 *   help, rpl-status, routes, ip-nbr, ping <addr>
 */
#include "contiki.h"
#include "net/routing/routing.h"
#include "net/netstack.h"
#include "net/ipv6/simple-udp.h"
#include "net/ipv6/uip.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sys/log.h"
#define LOG_MODULE "P4"
#define LOG_LEVEL LOG_LEVEL_INFO

#define UDP_CLIENT_PORT 8765
#define UDP_SERVER_PORT 5678
#define MAX_NODES       128
#define INITIAL_HOP_LIMIT 64   /* uip_ds6 usa 64 como hop limit inicial */

static struct simple_udp_connection udp_conn;
static uint32_t rx_count[MAX_NODES];
static uint32_t last_seq[MAX_NODES];

/*---------------------------------------------------------------------------*/
static void
udp_rx_callback(struct simple_udp_connection *c,
                const uip_ipaddr_t *sender_addr, uint16_t sender_port,
                const uip_ipaddr_t *receiver_addr, uint16_t receiver_port,
                const uint8_t *data, uint16_t datalen)
{
  char buf[48];
  unsigned node = 0, seq = 0;
  int temp = 0;
  /* Número de saltos: cada router decrementa el hop limit en 1 */
  int hops = INITIAL_HOP_LIMIT - UIP_IP_BUF->ttl + 1;  /* vecino directo = 1 salto */

  int n = datalen < sizeof(buf) - 1 ? datalen : sizeof(buf) - 1;
  memcpy(buf, data, n);
  buf[n] = '\0';

  if(sscanf(buf, "n%u,s%u,t%d", &node, &seq, &temp) == 3 && node < MAX_NODES) {
    rx_count[node]++;
    last_seq[node] = seq;
    unsigned pdr = (unsigned)((100u * rx_count[node]) / (seq + 1));
    /* Línea parseable por los scripts de la práctica 7 */
    printf("RX node=%u seq=%u temp=%d hops=%d rx=%lu pdr=%u\n",
           node, seq, temp, hops, (unsigned long)rx_count[node], pdr);
  } else {
    LOG_INFO("RX '%s' (%d saltos) de ", buf, hops);
    LOG_INFO_6ADDR(sender_addr);
    LOG_INFO_("\n");
  }
}
/*---------------------------------------------------------------------------*/
PROCESS(udp_server_process, "P4 servidor UDP / raiz RPL");
AUTOSTART_PROCESSES(&udp_server_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(udp_server_process, ev, data)
{
  PROCESS_BEGIN();

  NETSTACK_ROUTING.root_start();
  simple_udp_register(&udp_conn, UDP_SERVER_PORT, NULL,
                      UDP_CLIENT_PORT, udp_rx_callback);
  LOG_INFO("Raiz RPL y servidor UDP iniciados (puerto %u)\n", UDP_SERVER_PORT);

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
