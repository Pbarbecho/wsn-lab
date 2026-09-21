#ifndef PROJECT_CONF_H_
#define PROJECT_CONF_H_

/* MQTT usa TCP (desactivado por defecto en Contiki-NG) */
#define UIP_CONF_TCP 1

/* CoAP: tamaño máximo de bloque de respuesta */
#define COAP_CONF_MAX_CHUNK_SIZE 64

#define LOG_CONF_LEVEL_RPL      LOG_LEVEL_WARN
#define LOG_CONF_LEVEL_TCPIP    LOG_LEVEL_WARN
#define LOG_CONF_LEVEL_IPV6     LOG_LEVEL_WARN
#define LOG_CONF_LEVEL_MAC      LOG_LEVEL_WARN
#define LOG_CONF_LEVEL_COAP     LOG_LEVEL_INFO

#endif /* PROJECT_CONF_H_ */
