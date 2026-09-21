#!/usr/bin/env bash
# Preparación del laboratorio WSN (ejecutar una sola vez en el host).
#  1. Clona Contiki-NG con los submódulos necesarios (Cooja, etc.)
#  2. Crea el archivo .env con tu UID/GID y DISPLAY
#  3. Descarga las imágenes Docker y construye la imagen con VNC/noVNC
set -euo pipefail
cd "$(dirname "$0")"

CNG_TAG="${CNG_TAG:-develop}"   # rama/etiqueta de Contiki-NG (p.ej. release/v4.9)

echo "== [1/3] Contiki-NG =="
if [ ! -d contiki-ng/.git ]; then
  git clone --branch "$CNG_TAG" https://github.com/contiki-ng/contiki-ng.git contiki-ng
fi
git -C contiki-ng submodule update --init --recursive \
  tools/cooja tools/motelist tools/cc2538-bsl os/net/security/mbedtls os/net/security/micro-ecc

echo "== [2/3] .env =="
if [ ! -f .env ]; then
  cat > .env <<ENV
LOCAL_UID=$(id -u)
LOCAL_GID=$(id -g)
VNC_GEOMETRY=1600x900
VNC_PASSWORD=
COOJA_AUTOSTART=0
ENV
fi
mkdir -p .cache/gradle services/nodered resultados

echo "== [3/3] Imágenes Docker (pull + build de la imagen con VNC) =="
docker compose pull mosquitto nodered
docker compose build cooja

echo
echo "Listo. Ahora:  docker compose up -d   &&   ./scripts/vnc-viewer.sh   &&   ./scripts/cooja-gui.sh"
