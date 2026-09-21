#!/usr/bin/env bash
# Conecta la red simulada con el "mundo exterior" (contenedor) mediante tunslip6.
# Requiere: simulación en Cooja con el plugin "Serial Socket (SERVER)" en el
# border router, escuchando en el puerto 60001, y la simulación en ejecución.
# Crea la interfaz tun0 con la dirección fd00::1/64 dentro del contenedor.
cd "$(dirname "$0")/.."
exec docker compose exec -it --user user cooja bash --login -c \
  "make -C /home/user/contiki-ng/examples/rpl-border-router TARGET=cooja connect-router-cooja"
