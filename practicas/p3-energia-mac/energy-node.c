/*
 * Práctica 3 - Capa MAC y consumo de energía (CSMA vs TSCH)
 *
 * Red IPv6/RPL con un nodo raíz (id 1) y nodos sensores que envían una muestra
 * UDP cada SEND_INTERVAL. Cada nodo imprime periódicamente los contadores de
 * Energest (tiempo de radio en escucha, transmisión y apagada), con los que se
 * estima el consumo y la vida útil de la batería.
 *
 * Compilar con distintas capas MAC:
 *   make TARGET=cooja MAKE_MAC=MAKE_MAC_CSMA   (radio siempre encendida)
 *   make TARGET=cooja MAKE_MAC=MAKE_MAC_TSCH   (acceso por ranuras de tiempo, radio dormida)
 */
#include "contiki.h"
#include "net/routing/routing.h"
#include "net/netstack.h"
#include "net/ipv6/simple-udp.h"
#include "sys/energest.h"
#include "sys/node-id.h"
#include "lib/random.h"
#include <stdio.h>
#include <string.h>
#include <inttypes.h>

#include "sys/log.h"
#define LOG_MODULE "P3"
#define LOG_LEVEL LOG_LEVEL_INFO

#define ROOT_ID          1
#define UDP_PORT         5678
#define SEND_INTERVAL    (CLOCK_SECOND * 10)
#define REPORT_INTERVAL  (CLOCK_SECOND * 60)

static struct simple_udp_connection udp_conn;

/*---------------------------------------------------------------------------*/
static void
udp_rx_callback(struct simple_udp_connection *c,
                const uip_ipaddr_t *sender_addr, uint16_t sender_port,
                const uip_ipaddr_t *receiver_addr, uint16_t receiver_port,
                const uint8_t *data, uint16_t datalen)
{
  LOG_INFO("RX '%.*s' de ", datalen, (const char *)data);
  LOG_INFO_6ADDR(sender_addr);
  LOG_INFO_("\n");
}
/*---------------------------------------------------------------------------*/
/* Imprime los contadores acumulados de Energest (en ticks de ENERGEST_SECOND) */
static void
print_energest(void)
{
  energest_flush();
  uint64_t total = ENERGEST_GET_TOTAL_TIME();
  uint64_t cpu = energest_type_time(ENERGEST_TYPE_CPU);
  uint64_t lpm = energest_type_time(ENERGEST_TYPE_LPM);
  uint64_t tx = energest_type_time(ENERGEST_TYPE_TRANSMIT);
  uint64_t rx = energest_type_time(ENERGEST_TYPE_LISTEN);
  uint64_t off = total - tx - rx;

  /* Línea de una sola pieza, fácil de parsear con scripts (ver energy_calc.py) */
  printf("ENERGEST node=%u total=%" PRIu64 " cpu=%" PRIu64 " lpm=%" PRIu64
         " tx=%" PRIu64 " rx=%" PRIu64 " off=%" PRIu64 " tick_hz=%lu\n",
         node_id, total, cpu, lpm, tx, rx, off, (unsigned long)ENERGEST_SECOND);
  LOG_INFO("Radio: escucha %lu%% transmision %lu%% apagada %lu%%\n",
           (unsigned long)(100 * rx / (total ? total : 1)),
           (unsigned long)(100 * tx / (total ? total : 1)),
           (unsigned long)(100 * off / (total ? total : 1)));
}
/*---------------------------------------------------------------------------*/
PROCESS(energy_process, "P3 energia");
AUTOSTART_PROCESSES(&energy_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(energy_process, ev, data)
{
  static struct etimer send_timer;
  static struct etimer report_timer;
  static char buf[32];
  static uint32_t seq = 0;
  uip_ipaddr_t root_addr;

  PROCESS_BEGIN();

  if(node_id == ROOT_ID) {
    NETSTACK_ROUTING.root_start();     /* raiz del DODAG RPL (y coordinador TSCH) */
    LOG_INFO("Nodo raiz iniciado\n");
  }
  simple_udp_register(&udp_conn, UDP_PORT, NULL, UDP_PORT, udp_rx_callback);

  etimer_set(&send_timer, SEND_INTERVAL + (random_rand() % SEND_INTERVAL));
  etimer_set(&report_timer, REPORT_INTERVAL);

  while(1) {
    PROCESS_WAIT_EVENT();

    if(etimer_expired(&report_timer)) {
      print_energest();
      etimer_reset(&report_timer);
    }

    if(etimer_expired(&send_timer)) {
      if(node_id != ROOT_ID) {
        if(NETSTACK_ROUTING.node_is_reachable() &&
           NETSTACK_ROUTING.get_root_ipaddr(&root_addr)) {
          snprintf(buf, sizeof(buf), "n%u,s%" PRIu32, node_id, seq);
          simple_udp_sendto(&udp_conn, buf, strlen(buf), &root_addr);
          LOG_INFO("TX seq=%" PRIu32 " hacia la raiz\n", seq);
          seq++;
        } else {
          LOG_INFO("Aun sin ruta a la raiz\n");
        }
      }
      etimer_set(&send_timer, SEND_INTERVAL - CLOCK_SECOND / 2
                 + (random_rand() % CLOCK_SECOND));
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
