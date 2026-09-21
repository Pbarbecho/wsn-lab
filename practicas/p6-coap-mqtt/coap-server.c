/*
 * Práctica 6a - Servidor CoAP en el mote
 *
 * Expone dos recursos:
 *   GET  coap://[addr]/sensors/temp   -> temperatura (texto o JSON)
 *   GET/PUT coap://[addr]/actuators/led  -> estado del LED (PUT led=on|off)
 * Y el descubrimiento estándar: GET coap://[addr]/.well-known/core
 *
 * Desde el contenedor (con tunslip6 activo):
 *   coap-client -m get coap://[fd00::202:2:2:2]/.well-known/core
 *   coap-client -m get coap://[fd00::202:2:2:2]/sensors/temp
 *   coap-client -m put -e "led=on" coap://[fd00::202:2:2:2]/actuators/led
 */
#include "contiki.h"
#include "coap-engine.h"
#include "sys/node-id.h"

#include "sys/log.h"
#define LOG_MODULE "P6"
#define LOG_LEVEL LOG_LEVEL_INFO

extern coap_resource_t res_temp;
extern coap_resource_t res_led;

PROCESS(coap_server_process, "P6 servidor CoAP");
AUTOSTART_PROCESSES(&coap_server_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(coap_server_process, ev, data)
{
  PROCESS_BEGIN();

  LOG_INFO("Servidor CoAP en el nodo %u\n", node_id);
  coap_activate_resource(&res_temp, "sensors/temp");
  coap_activate_resource(&res_led, "actuators/led");

  while(1) {
    PROCESS_WAIT_EVENT();
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
