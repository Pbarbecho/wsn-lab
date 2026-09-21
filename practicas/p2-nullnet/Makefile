CONTIKI_PROJECT = nullnet-broadcast nullnet-unicast
all: $(CONTIKI_PROJECT)

CONTIKI = ../../contiki-ng

# Capa de red vacía (sin IPv6/6LoWPAN/RPL): solo 802.15.4 + CSMA
MAKE_MAC ?= MAKE_MAC_CSMA
MAKE_NET = MAKE_NET_NULLNET
include $(CONTIKI)/Makefile.include
