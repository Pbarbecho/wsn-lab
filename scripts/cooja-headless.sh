#!/usr/bin/env bash
# Ejecuta una simulación .csc sin GUI (requiere un ScriptRunner con TIMEOUT()).
#   ./scripts/cooja-headless.sh practicas/p7-escalabilidad/sim/n20.csc [semilla] [dir_logs]
# Los resultados quedan en <dir_logs>/COOJA.testlog
set -euo pipefail
cd "$(dirname "$0")/.."
CSC="${1:?uso: $0 archivo.csc [semilla] [dir_logs]}"
SEED="${2:-1}"
LOGDIR="${3:-resultados/$(basename "${CSC%.csc}")_s${SEED}}"
mkdir -p "$LOGDIR"
docker compose up -d cooja >/dev/null
docker compose exec -T --user user cooja bash --login -c \
  "cooja --args='--no-gui --contiki=/home/user/contiki-ng --random-seed=$SEED --logdir=/home/user/wsn-lab/$LOGDIR /home/user/wsn-lab/$CSC'"
echo "Log: $LOGDIR/COOJA.testlog"
