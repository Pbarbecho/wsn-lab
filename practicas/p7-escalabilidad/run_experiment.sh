#!/usr/bin/env bash
# Práctica 7 - Barrido de escalabilidad sin GUI.
# Genera una simulación por tamaño de red y la ejecuta con varias semillas.
#
#   ./run_experiment.sh                    # N = 10 20 40 80, semillas 1 2 3
#   NODES="10 50 100" SEEDS="1" ./run_experiment.sh
#   AREA=200 RANGE=50 INTERVAL=5 ./run_experiment.sh
#
# Resultados: ../../resultados/p7_n<N>_s<seed>/COOJA.testlog
# Luego:      python3 parse_results.py
set -euo pipefail
cd "$(dirname "$0")"
LAB_ROOT="$(cd ../.. && pwd)"

NODES="${NODES:-10 20 40 80}"
SEEDS="${SEEDS:-1 2 3}"
AREA="${AREA:-150}"          # lado del área (m)
RANGE="${RANGE:-50}"         # alcance radio (m)
INTERVAL="${INTERVAL:-10}"   # segundos entre envíos (SEND_INTERVAL)
LAYOUT="${LAYOUT:-random}"

mkdir -p sim
for n in $NODES; do
  clients=$((n - 1))
  csc="sim/n${n}.csc"
  python3 "$LAB_ROOT/scripts/gen_csc.py" -o "$csc" --title "P7 escalabilidad N=$n" \
    --type "raiz:../../p4-rpl-udp/udp-server.c:1" \
    --type "sensor:../../p4-rpl-udp/udp-client.c:$clients" \
    --layout "$LAYOUT" --area "$AREA" --range "$RANGE" --pos-seed "$n" \
    --script ../metrics.js \
    --make-args "SEND_INTERVAL_S=${INTERVAL}" --clean
  for s in $SEEDS; do
    echo "=== N=$n semilla=$s ==="
    "$LAB_ROOT/scripts/cooja-headless.sh" "practicas/p7-escalabilidad/$csc" "$s" "resultados/p7_n${n}_s${s}" \
      | tail -3
  done
done
echo "Listo. Ejecute: python3 parse_results.py"
