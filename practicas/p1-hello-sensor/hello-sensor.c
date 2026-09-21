/*
 * Práctica 1 - Primer mote y ciclo de desarrollo
 *
 * Objetivos:
 *  - Entender el modelo de programación por eventos de Contiki-NG (protothreads).
 *  - Usar temporizadores (etimer, ctimer), LEDs, botón y el sistema de log.
 *  - Leer un "sensor" (simulado) periódicamente e imprimirlo por el puerto serie.
 *
 * No hay sleep() bloqueante: cada proceso cede el control con
 * PROCESS_WAIT_EVENT_UNTIL() y el planificador lo despierta cuando ocurre un evento.
 */
#include "contiki.h"
#include "sys/node-id.h"
#include "sys/ctimer.h"
#include "dev/leds.h"
#include "dev/button-hal.h"
#include "lib/random.h"

#include <stdio.h>

/* Sistema de log de Contiki-NG: LOG_INFO / LOG_DBG / LOG_ERR */
#include "sys/log.h"
#define LOG_MODULE "P1"
#define LOG_LEVEL LOG_LEVEL_INFO

/* --- Parámetros de la práctica (modifíquelos y observe el efecto) --- */
#define SAMPLE_INTERVAL   (CLOCK_SECOND * 5)   /* periodo de muestreo */
#define LED_PULSE         (CLOCK_SECOND / 4)   /* duración del pulso de LED */
#define ALARM_THRESHOLD   280                  /* décimas de grado (28.0 C) */

/*---------------------------------------------------------------------------*/
/* Sensor simulado: temperatura en décimas de grado con un paseo aleatorio.  */
static int
read_temperature(void)
{
  static int temp = 250;                       /* 25.0 C */
  temp += (int)(random_rand() % 11) - 5;       /* -0.5 .. +0.5 C */
  if(temp < 150) temp = 150;
  if(temp > 350) temp = 350;
  return temp;
}
/*---------------------------------------------------------------------------*/
static struct ctimer led_timer;

static void
led_off_callback(void *ptr)
{
  leds_off(LEDS_ALL);
}
/*---------------------------------------------------------------------------*/
PROCESS(sensor_process, "P1 proceso de muestreo");
PROCESS(button_process, "P1 proceso de boton");
AUTOSTART_PROCESSES(&sensor_process, &button_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(sensor_process, ev, data)
{
  static struct etimer periodic_timer;
  static unsigned seq = 0;

  PROCESS_BEGIN();

  LOG_INFO("Nodo %u iniciado. Muestreo cada %lu s\n",
           node_id, (unsigned long)(SAMPLE_INTERVAL / CLOCK_SECOND));

  etimer_set(&periodic_timer, SAMPLE_INTERVAL);

  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));

    int temp = read_temperature();
    LOG_INFO("seq=%u temp=%d.%d C\n", seq, temp / 10, temp % 10);

    /* Pulso de LED verde en cada muestra; rojo si hay alarma */
    if(temp > ALARM_THRESHOLD) {
      leds_on(LEDS_RED);
      LOG_WARN("ALARMA: temperatura alta (%d.%d C)\n", temp / 10, temp % 10);
    } else {
      leds_on(LEDS_GREEN);
    }
    ctimer_set(&led_timer, LED_PULSE, led_off_callback, NULL);

    seq++;
    etimer_reset(&periodic_timer);
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
/* Segundo proceso concurrente: reacciona al botón (en Cooja: clic derecho   */
/* sobre el mote -> "Click button").                                          */
PROCESS_THREAD(button_process, ev, data)
{
  PROCESS_BEGIN();

  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(ev == button_hal_press_event);
    LOG_INFO("Boton presionado en el nodo %u\n", node_id);
    leds_toggle(LEDS_YELLOW);
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
