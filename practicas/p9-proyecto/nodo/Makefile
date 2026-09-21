CONTIKI_PROJECT = nodo-sensor
all: $(CONTIKI_PROJECT)

CONTIKI = ../../../contiki-ng

include $(CONTIKI)/Makefile.dir-variables
MODULES += $(CONTIKI_NG_SERVICES_DIR)/shell
# Descomente según el proyecto:
# MODULES += $(CONTIKI_NG_APP_LAYER_DIR)/mqtt
# MODULES += $(CONTIKI_NG_APP_LAYER_DIR)/coap
MAKE_MAC ?= MAKE_MAC_CSMA

include $(CONTIKI)/Makefile.include
