/* Recurso CoAP: /actuators/led (GET estado, PUT led=on|off) */
#include "contiki.h"
#include "coap-engine.h"
#include "dev/leds.h"
#include <stdio.h>
#include <string.h>

#include "sys/log.h"
#define LOG_MODULE "P6"
#define LOG_LEVEL LOG_LEVEL_INFO

static void res_get_handler(coap_message_t *request, coap_message_t *response,
                            uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
static void res_put_handler(coap_message_t *request, coap_message_t *response,
                            uint8_t *buffer, uint16_t preferred_size, int32_t *offset);

RESOURCE(res_led,
         "title=\"LED: PUT led=on|off\";rt=\"Control\"",
         res_get_handler, NULL, res_put_handler, NULL);

static int led_state = 0;

static void
res_get_handler(coap_message_t *request, coap_message_t *response,
                uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  coap_set_header_content_format(response, TEXT_PLAIN);
  snprintf((char *)buffer, preferred_size, "led=%s", led_state ? "on" : "off");
  coap_set_payload(response, buffer, strlen((char *)buffer));
}

static void
res_put_handler(coap_message_t *request, coap_message_t *response,
                uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  const uint8_t *payload = NULL;
  int len = coap_get_payload(request, &payload);

  if(len >= 6 && strncmp((const char *)payload, "led=on", 6) == 0) {
    leds_on(LEDS_RED);
    led_state = 1;
  } else if(len >= 7 && strncmp((const char *)payload, "led=off", 7) == 0) {
    leds_off(LEDS_RED);
    led_state = 0;
  } else {
    coap_set_status_code(response, BAD_REQUEST_4_00);
    return;
  }
  LOG_INFO("LED -> %s (orden recibida por CoAP)\n", led_state ? "on" : "off");
  coap_set_status_code(response, CHANGED_2_04);
}
