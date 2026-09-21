#ifndef PROJECT_CONF_H_
#define PROJECT_CONF_H_

/* Modo de RPL Lite: non-storing (por defecto) vs storing.
 * Descomente para probar el modo storing y compare las tablas de rutas.  */
/* #define RPL_CONF_MOP RPL_MOP_STORING_NO_MULTICAST */

/* Tamaño de las tablas para redes de hasta ~100 nodos (prácticas 4 y 7) */
#define NETSTACK_MAX_ROUTE_ENTRIES 128
#define NBR_TABLE_CONF_MAX_NEIGHBORS 32

/* Menos ruido en el log */
#define LOG_CONF_LEVEL_RPL      LOG_LEVEL_WARN
#define LOG_CONF_LEVEL_TCPIP    LOG_LEVEL_WARN
#define LOG_CONF_LEVEL_IPV6     LOG_LEVEL_WARN
#define LOG_CONF_LEVEL_6LOWPAN  LOG_LEVEL_WARN
#define LOG_CONF_LEVEL_MAC      LOG_LEVEL_WARN

#endif /* PROJECT_CONF_H_ */
