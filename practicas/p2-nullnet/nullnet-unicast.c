/*
 * Práctica 2b - Comunicación de un salto sin IP (NullNet, unicast a un sumidero)
 *
 * Topología en estrella: los nodos 2..N envían sus muestras al nodo SINK_ID (1).
 * El sumidero lleva la cuenta de paquetes recibidos por nodo y calcula la
 * tasa de entrega (PDR) a partir de los números de secuencia.
 *
 * Nota sobre direcciones en Cooja (build sin IPv6): la dirección de enlace del
 * mote con id N es {N, 0, 0, 0, 0, 0, 0, 0}.
 */
#include "contiki.h"
#include "net/netstack.h"
#include "net/nullnet/nullnet.h"
#include "net/packetbuf.h"
#include "lib/random.h"
#include <string.h>

#include "sys/log.h"
#define LOG_MODULE "P2"
#define LOG_LEVEL LOG_LEVEL_INFO

#define SINK_ID       1
#define SEND_INTERVAL (CLOCK_SECOND * 4)
#define MAX_NODES     32

typedef struct __attribute__((packed)) {
  uint16_t node;
  uint16_t seq;
  int16_t  temp;
} sample_t;

static sample_t tx_sample;

/* En una compilación sin IPv6 (MAKE_NET_NULLNET) Contiki-NG no rellena node_id;
 * en Cooja la dirección de enlace del mote N es {N, 0, 0, 0, 0, 0, 0, 0}. */
static uint16_t
my_id(void)
{
  return linkaddr_node_addr.u8[0] | (linkaddr_node_addr.u8[1] << 8);
}
/*---------------------------------------------------------------------------*/
static linkaddr_t sink_addr;

/* Estadísticas del sumidero */
static uint16_t rx_count[MAX_NODES];
static uint16_t last_seq[MAX_NODES];

/*---------------------------------------------------------------------------*/
static void
input_callback(const void *data, uint16_t len,
               const linkaddr_t *src, const linkaddr_t *dest)
{
  if(len != sizeof(sample_t)) {
    return;
  }
  sample_t s;
  memcpy(&s, data, sizeof(s));

  if(s.node < MAX_NODES) {
    rx_count[s.node]++;
    last_seq[s.node] = s.seq;
    /* PDR = recibidos / (último seq + 1) */
    unsigned pdr = (100u * rx_count[s.node]) / (s.seq + 1);
    LOG_INFO("RX node=%u seq=%u temp=%d.%d rssi=%d | rx=%u pdr=%u%%\n",
             s.node, s.seq, s.temp / 10, s.temp % 10,
             (int)(int16_t)packetbuf_attr(PACKETBUF_ATTR_RSSI),
             rx_count[s.node], pdr);
  }
}
/*---------------------------------------------------------------------------*/
PROCESS(nullnet_ucast_process, "P2 NullNet unicast");
AUTOSTART_PROCESSES(&nullnet_ucast_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(nullnet_ucast_process, ev, data)
{
  static struct etimer periodic_timer;

  PROCESS_BEGIN();

  memset(&sink_addr, 0, sizeof(sink_addr));
  sink_addr.u8[0] = SINK_ID;

  nullnet_buf = (uint8_t *)&tx_sample;
  nullnet_len = sizeof(tx_sample);
  nullnet_set_input_callback(input_callback);

  tx_sample.node = my_id();
  tx_sample.temp = 250;

  if(linkaddr_cmp(&sink_addr, &linkaddr_node_addr)) {
    LOG_INFO("Soy el sumidero (nodo %u). Esperando muestras...\n", my_id());
    /* El sumidero solo recibe; el proceso termina aquí pero el callback sigue activo */
    PROCESS_EXIT();
  }

  etimer_set(&periodic_timer, random_rand() % SEND_INTERVAL);
  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));

    tx_sample.temp += (int16_t)(random_rand() % 11) - 5;
    LOG_INFO("TX unicast seq=%u -> sumidero ", tx_sample.seq);
    LOG_INFO_LLADDR(&sink_addr);
    LOG_INFO_("\n");

    nullnet_len = sizeof(tx_sample);
    NETSTACK_NETWORK.output(&sink_addr);

    tx_sample.seq++;
    etimer_set(&periodic_timer, SEND_INTERVAL);
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
