#!/usr/bin/env bash
# Práctica 8 (ruta A): tunslip6 hacia un border router REAL (nRF52840 dongle, XIAO nRF54L15...).
#
#   Linux (USB pasado al contenedor con docker-compose.hardware.yml):
#       ./scripts/connect-router-hw.sh                 # usa /dev/ttyACM0 dentro del contenedor
#       DEVICE=/dev/ttyACM1 ./scripts/connect-router-hw.sh
#
#   Windows/macOS (Docker Desktop no pasa USB): exponga el puerto serie del host por TCP y
#   tunslip6 se conecta como cliente:
#       host$ python3 scripts/serial-tcp-bridge.py /dev/tty.usbmodem1101 60002   (Mac; Windows: COM5)
#       (o bien: socat TCP-LISTEN:60002,reuseaddr,fork /dev/tty.usbmodem1101,raw,b115200,echo=0)
#       (Windows/WSL2: usbipd-win para pasar el USB a WSL2 y usar la variante Linux)
#       HOST=host.docker.internal PORT=60002 ./scripts/connect-router-hw.sh
cd "$(dirname "$0")/.."
BR_DIR=/home/user/contiki-ng/examples/rpl-border-router
if [ -n "${HOST:-}" ]; then
  PORT="${PORT:-60002}"
  echo "tunslip6 -> TCP $HOST:$PORT (puerto serie expuesto desde el host)"
  exec docker compose exec -it --user user cooja bash --login -c \
    "make -C /home/user/contiki-ng/tools/serial-io tunslip6 >/dev/null && \
     sudo /home/user/contiki-ng/tools/serial-io/tunslip6 -a $HOST -p $PORT fd00::1/64"
else
  DEVICE="${DEVICE:-/dev/ttyACM0}"
  echo "tunslip6 -> $DEVICE (USB dentro del contenedor)"
  exec docker compose exec -it --user user cooja bash --login -c \
    "make -C $BR_DIR TARGET=nrf BOARD=nrf52840/dongle connect-router PORT=$DEVICE"
fi
