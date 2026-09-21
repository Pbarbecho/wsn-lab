#ifndef PROJECT_CONF_H_
#define PROJECT_CONF_H_

/* Habilita el módulo Energest (contadores de tiempo por componente) */
#define ENERGEST_CONF_ON 1

/* Menos ruido en el log de las capas inferiores */
#define LOG_CONF_LEVEL_RPL      LOG_LEVEL_WARN
#define LOG_CONF_LEVEL_TCPIP    LOG_LEVEL_WARN
#define LOG_CONF_LEVEL_IPV6     LOG_LEVEL_WARN
#define LOG_CONF_LEVEL_MAC      LOG_LEVEL_WARN

#if MAC_CONF_WITH_TSCH
/* TSCH: arranque automático; el nodo raíz RPL se vuelve coordinador */
#define TSCH_CONF_AUTOSTART 1
#define TSCH_CONF_AUTOSELECT_TIME_SOURCE 1
/* Slotframe pequeño = más ranuras activas = menos latencia, más consumo.
 * Pruebe 7, 17 y 101 y compare Energest. */
#define TSCH_SCHEDULE_CONF_DEFAULT_LENGTH 7
#endif

#endif /* PROJECT_CONF_H_ */
