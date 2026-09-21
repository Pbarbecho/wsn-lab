CONTIKI_PROJECT = energy-node
all: $(CONTIKI_PROJECT)

CONTIKI = ../../contiki-ng

# Elija la capa MAC en la línea de comandos:
#   make TARGET=cooja MAKE_MAC=MAKE_MAC_CSMA
#   make TARGET=cooja MAKE_MAC=MAKE_MAC_TSCH
MAKE_MAC ?= MAKE_MAC_CSMA

include $(CONTIKI)/Makefile.include
