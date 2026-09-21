/*
 * Práctica 6b - Nodo publicador MQTT
 *
 * El mote se conecta por TCP/IPv6 (a través del border router y tunslip6) al
 * broker Mosquitto en fd00::1:1883 y publica cada PUB_INTERVAL un JSON en
 *   wsn/<id>/data      {"id":2,"seq":10,"temp":23.5,"rssi":-70}
 * y se suscribe a
 *   wsn/<id>/cmd       payload "led=on" | "led=off"
 *
 * Desde el contenedor:
 *   mosquitto_sub -h localhost -t 'wsn/#' -v
 *   mosquitto_pub -h localhost -t 'wsn/2/cmd' -m 'led=on'
 */
#include "contiki.h"
#include "net/routing/routing.h"
#include "net/ipv6/uip.h"
#include "net/ipv6/uip-ds6.h"
#include "mqtt.h"
#include "dev/leds.h"
#include "sys/node-id.h"
#include "lib/random.h"
#include <stdio.h>
#include <string.h>

#include "sys/log.h"
#define LOG_MODULE "P6"
#define LOG_LEVEL LOG_LEVEL_INFO

#define BROKER_IP            "fd00::1"    /* dirección de tun0 (tunslip6) */
#define BROKER_PORT          1883
#define PUB_INTERVAL         (CLOCK_SECOND * 20)
#define NET_CHECK_INTERVAL   (CLOCK_SECOND * 5)
#define RECONNECT_INTERVAL   (CLOCK_SECOND * 10)
#define KEEP_ALIVE_SECONDS   ((3 * PUB_INTERVAL) / CLOCK_SECOND)
#define MAX_TCP_SEGMENT_SIZE 32

static struct mqtt_connection conn;
static char client_id[24];
static char pub_topic[24];
static char sub_topic[24];
static char app_buffer[96];
static uint32_t seq = 0;
static int temp = 250;

static struct etimer timer;
static enum {
  STATE_WAIT_NET, STATE_CONNECTING, STATE_CONNECTED, STATE_DISCONNECTED
} state = STATE_WAIT_NET;

PROCESS(mqtt_node_process, "P6 nodo MQTT");
AUTOSTART_PROCESSES(&mqtt_node_process);
/*---------------------------------------------------------------------------*/
static int
have_connectivity(void)
{
  return uip_ds6_get_global(ADDR_PREFERRED) != NULL &&
         uip_ds6_defrt_choose() != NULL;
}
/*---------------------------------------------------------------------------*/
static void
handle_command(const char *topic, const uint8_t *payload, uint16_t len)
{
  if(len >= 6 && strncmp((const char *)payload, "led=on", 6) == 0) {
    leds_on(LEDS_RED);
    LOG_INFO("Comando: LED encendido\n");
  } else if(len >= 7 && strncmp((const char *)payload, "led=off", 7) == 0) {
    leds_off(LEDS_RED);
    LOG_INFO("Comando: LED apagado\n");
  } else {
    LOG_INFO("Comando desconocido en %s: '%.*s'\n", topic, len, (const char *)payload);
  }
}
/*---------------------------------------------------------------------------*/
static void
mqtt_event(struct mqtt_connection *m, mqtt_event_t event, void *data)
{
  switch(event) {
  case MQTT_EVENT_CONNECTED:
    LOG_INFO("Conectado al broker %s\n", BROKER_IP);
    state = STATE_CONNECTED;
    mqtt_subscribe(&conn, NULL, sub_topic, MQTT_QOS_LEVEL_0);
    process_poll(&mqtt_node_process);
    break;
  case MQTT_EVENT_DISCONNECTED:
    LOG_WARN("Desconectado del broker (motivo %u)\n", *((mqtt_event_t *)data));
    state = STATE_DISCONNECTED;
    process_poll(&mqtt_node_process);
    break;
  case MQTT_EVENT_PUBLISH: {
    struct mqtt_message *msg = data;
    handle_command(msg->topic, msg->payload_chunk, msg->payload_chunk_length);
    break;
  }
  case MQTT_EVENT_SUBACK:
    LOG_INFO("Suscrito a %s\n", sub_topic);
    break;
  case MQTT_EVENT_PUBACK:
    LOG_DBG("PUBACK recibido\n");
    break;
  default:
    LOG_DBG("Evento MQTT %i no manejado\n", event);
    break;
  }
}
/*---------------------------------------------------------------------------*/
static void
publish_sample(void)
{
  temp += (int)(random_rand() % 11) - 5;
  int len = snprintf(app_buffer, sizeof(app_buffer),
                     "{\"id\":%u,\"seq\":%lu,\"temp\":%d.%d,\"uptime\":%lu}",
                     node_id, (unsigned long)seq, temp / 10, temp % 10,
                     (unsigned long)clock_seconds());
  mqtt_status_t st = mqtt_publish(&conn, NULL, pub_topic, (uint8_t *)app_buffer,
                                  len, MQTT_QOS_LEVEL_0, MQTT_RETAIN_OFF);
  if(st == MQTT_STATUS_OK) {
    LOG_INFO("PUB %s %s\n", pub_topic, app_buffer);
    seq++;
  } else {
    LOG_WARN("Fallo al publicar (estado %u)\n", st);
  }
}
/*---------------------------------------------------------------------------*/
static void
state_machine(void)
{
  switch(state) {
  case STATE_WAIT_NET:
    if(have_connectivity()) {
      LOG_INFO("Red lista. Conectando a [%s]:%u ...\n", BROKER_IP, BROKER_PORT);
      state = STATE_CONNECTING;
      mqtt_connect(&conn, BROKER_IP, BROKER_PORT, KEEP_ALIVE_SECONDS,
                   MQTT_CLEAN_SESSION_ON);
      etimer_set(&timer, RECONNECT_INTERVAL);
    } else {
      LOG_INFO("Esperando prefijo global y ruta por defecto (RPL)...\n");
      etimer_set(&timer, NET_CHECK_INTERVAL);
    }
    break;
  case STATE_CONNECTING:
    /* Esperando MQTT_EVENT_CONNECTED; si expira el timer, reintentar */
    if(etimer_expired(&timer)) {
      LOG_WARN("Tiempo de conexion agotado, reintentando\n");
      mqtt_disconnect(&conn);
      state = STATE_WAIT_NET;
      etimer_set(&timer, NET_CHECK_INTERVAL);
    }
    break;
  case STATE_CONNECTED:
    if(mqtt_ready(&conn) && conn.out_buffer_sent) {
      publish_sample();
    }
    etimer_set(&timer, PUB_INTERVAL);
    break;
  case STATE_DISCONNECTED:
    state = STATE_WAIT_NET;
    etimer_set(&timer, RECONNECT_INTERVAL);
    break;
  }
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(mqtt_node_process, ev, data)
{
  PROCESS_BEGIN();

  snprintf(client_id, sizeof(client_id), "wsn-node-%u", node_id);
  snprintf(pub_topic, sizeof(pub_topic), "wsn/%u/data", node_id);
  snprintf(sub_topic, sizeof(sub_topic), "wsn/%u/cmd", node_id);

  mqtt_register(&conn, &mqtt_node_process, client_id, mqtt_event,
                MAX_TCP_SEGMENT_SIZE);

  etimer_set(&timer, NET_CHECK_INTERVAL);

  while(1) {
    PROCESS_WAIT_EVENT();
    if((ev == PROCESS_EVENT_TIMER && data == &timer) || ev == PROCESS_EVENT_POLL) {
      state_machine();
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
