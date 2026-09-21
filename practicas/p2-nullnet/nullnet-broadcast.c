/*
 * Práctica 2a - Comunicación de un salto sin IP (NullNet, broadcast)
 *
 * Cada nodo difunde periódicamente una muestra {seq, temp} a todos sus vecinos
 * en alcance radio. Todo nodo que la recibe imprime el emisor, el RSSI y el LQI.
 *
 * Pila de red: NullNet (capa de red vacía) sobre CSMA/802.15.4.
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

#define SEND_INTERVAL (CLOCK_SECOND * 4)

/* Formato de la trama de aplicación (empaquetado para que el tamaño sea fijo) */
typedef struct __attribute__((packed)) {
  uint16_t node;   /* id del emisor */
  uint16_t seq;    /* número de secuencia */
  int16_t  temp;   /* décimas de grado */
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

/*---------------------------------------------------------------------------*/
static void
input_callback(const void *data, uint16_t len,
               const linkaddr_t *src, const linkaddr_t *dest)
{
  if(len == sizeof(sample_t)) {
    sample_t s;
    memcpy(&s, data, sizeof(s));
    LOG_INFO("RX de ");
    LOG_INFO_LLADDR(src);
    LOG_INFO_(" node=%u seq=%u temp=%d.%d rssi=%d lqi=%u\n",
              s.node, s.seq, s.temp / 10, s.temp % 10,
              (int)(int16_t)packetbuf_attr(PACKETBUF_ATTR_RSSI),
              (unsigned)packetbuf_attr(PACKETBUF_ATTR_LINK_QUALITY));
  }
}
/*---------------------------------------------------------------------------*/
PROCESS(nullnet_bcast_process, "P2 NullNet broadcast");
AUTOSTART_PROCESSES(&nullnet_bcast_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(nullnet_bcast_process, ev, data)
{
  static struct etimer periodic_timer;

  PROCESS_BEGIN();

  /* NullNet: el buffer de salida es un puntero a nuestros datos */
  nullnet_buf = (uint8_t *)&tx_sample;
  nullnet_len = sizeof(tx_sample);
  nullnet_set_input_callback(input_callback);

  tx_sample.node = my_id();
  tx_sample.seq = 0;
  tx_sample.temp = 250;

  /* Arranque aleatorio para evitar que todos transmitan a la vez */
  etimer_set(&periodic_timer, random_rand() % SEND_INTERVAL);
  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));

    tx_sample.temp += (int16_t)(random_rand() % 11) - 5;
    LOG_INFO("TX broadcast seq=%u temp=%d.%d\n",
             tx_sample.seq, tx_sample.temp / 10, tx_sample.temp % 10);

    nullnet_len = sizeof(tx_sample);
    NETSTACK_NETWORK.output(NULL);   /* NULL = broadcast */

    tx_sample.seq++;
    etimer_set(&periodic_timer, SEND_INTERVAL);
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
