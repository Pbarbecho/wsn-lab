CONTIKI_PROJECT = coap-server mqtt-node
all: $(CONTIKI_PROJECT)

CONTIKI = ../../contiki-ng

include $(CONTIKI)/Makefile.dir-variables
# Pila de aplicación: CoAP (UDP) y MQTT (TCP)
MODULES += $(CONTIKI_NG_APP_LAYER_DIR)/coap
MODULES += $(CONTIKI_NG_APP_LAYER_DIR)/mqtt
# Recursos CoAP de esta práctica
MODULES_REL += ./resources

include $(CONTIKI)/Makefile.include
