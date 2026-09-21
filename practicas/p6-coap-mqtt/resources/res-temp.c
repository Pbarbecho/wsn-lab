/* Recurso CoAP: /sensors/temp (GET). Negociación de formato: text/plain o JSON */
#include "contiki.h"
#include "coap-engine.h"
#include "sys/node-id.h"
#include "lib/random.h"
#include <stdio.h>
#include <string.h>

static void res_get_handler(coap_message_t *request, coap_message_t *response,
                            uint8_t *buffer, uint16_t preferred_size, int32_t *offset);

RESOURCE(res_temp,
         "title=\"Temperatura\";rt=\"Temperature\";ct=\"0 50\"",
         res_get_handler, NULL, NULL, NULL);

static int
read_temperature(void)
{
  static int temp = 250;
  temp += (int)(random_rand() % 11) - 5;
  return temp;
}

static void
res_get_handler(coap_message_t *request, coap_message_t *response,
                uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  int temp = read_temperature();
  unsigned int accept = -1;
  coap_get_header_accept(request, &accept);

  if(accept == APPLICATION_JSON) {
    coap_set_header_content_format(response, APPLICATION_JSON);
    snprintf((char *)buffer, preferred_size,
             "{\"id\":%u,\"temp\":%d.%d}", node_id, temp / 10, temp % 10);
  } else {
    coap_set_header_content_format(response, TEXT_PLAIN);
    snprintf((char *)buffer, preferred_size, "%d.%d", temp / 10, temp % 10);
  }
  coap_set_payload(response, buffer, strlen((char *)buffer));
}
