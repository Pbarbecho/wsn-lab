#!/usr/bin/env bash
# Lanza Cooja con interfaz gráfica dentro del escritorio virtual del contenedor.
# Véalo en el navegador: http://localhost:6080  (o ./scripts/vnc-viewer.sh)
# Opcionalmente abre una simulación:
#   ./scripts/cooja-gui.sh practicas/p4-rpl-udp/p4-rpl-udp.csc
cd "$(dirname "$0")/.."
docker compose up -d cooja >/dev/null
CSC=""
if [ -n "${1:-}" ]; then CSC="/home/user/wsn-lab/$1"; fi
echo "Cooja se abre en el escritorio virtual: http://localhost:6080  (VNC: localhost:5901)"
exec docker compose exec -it --user user -e DISPLAY=:1 cooja bash --login -c \
  "cooja --args='--gui --contiki=/home/user/contiki-ng $CSC'"
