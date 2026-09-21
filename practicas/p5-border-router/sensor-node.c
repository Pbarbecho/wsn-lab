/*
 * Práctica 5 - Border router: nodo sensor accesible desde el exterior
 *
 * Nodo RPL "pasivo": se une al DODAG que anuncia el border router y, cuando
 * obtiene una dirección global (prefijo fd00::/64 anunciado por el BR),
 * la imprime para que pueda hacerle ping6 desde el contenedor.
 * Responde a ICMPv6 echo (ping) y a los comandos del shell (rpl-status, routes...).
 *
 * El border router se toma tal cual de contiki-ng/examples/rpl-border-router.
 */
#include "contiki.h"
#include "net/routing/routing.h"
#include "net/ipv6/uip-ds6.h"
#include "sys/node-id.h"

#include "sys/log.h"
#define LOG_MODULE "P5"
#define LOG_LEVEL LOG_LEVEL_INFO

PROCESS(sensor_node_process, "P5 nodo sensor");
AUTOSTART_PROCESSES(&sensor_node_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(sensor_node_process, ev, data)
{
  static struct etimer timer;
  static int announced = 0;

  PROCESS_BEGIN();

  LOG_INFO("Nodo %u esperando unirse a la red del border router...\n", node_id);
  etimer_set(&timer, CLOCK_SECOND * 5);

  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));
    if(!announced && NETSTACK_ROUTING.node_is_reachable()) {
      uip_ds6_addr_t *global = uip_ds6_get_global(ADDR_PREFERRED);
      if(global != NULL) {
        LOG_INFO("Unido a la red. Direccion global: ");
        LOG_INFO_6ADDR(&global->ipaddr);
        LOG_INFO_("  -> pruebe: ping6 <esta direccion>\n");
        announced = 1;
      }
    }
    etimer_reset(&timer);
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
