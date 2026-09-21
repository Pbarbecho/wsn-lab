CONTIKI_PROJECT = udp-client udp-server
all: $(CONTIKI_PROJECT)

CONTIKI = ../../contiki-ng

# Shell serie: comandos rpl-status, routes, ip-nbr, ping, etc. en "Mote output"
include $(CONTIKI)/Makefile.dir-variables
MODULES += $(CONTIKI_NG_SERVICES_DIR)/shell

# Periodo de envío configurable desde la línea de comandos (segundos):
#   make TARGET=cooja SEND_INTERVAL_S=5
ifdef SEND_INTERVAL_S
CFLAGS += -DSEND_INTERVAL="(CLOCK_SECOND*$(SEND_INTERVAL_S))"
endif

include $(CONTIKI)/Makefile.include
