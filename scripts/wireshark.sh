#!/usr/bin/env bash
# Abre Wireshark en el escritorio virtual (visor: http://localhost:6080).
#   ./scripts/wireshark.sh                              # vacío
#   ./scripts/wireshark.sh resultados/p4-cadena.pcap    # abre una captura de Cooja
#   ./scripts/wireshark.sh tun0                         # captura en vivo de la interfaz del border router
cd "$(dirname "$0")/.."
docker compose up -d cooja >/dev/null
ARG="${1:-}"
if [ -z "$ARG" ]; then CMD="wireshark"
elif [ "$ARG" = "tun0" ]; then CMD="wireshark -i tun0 -k"
else CMD="wireshark /home/user/wsn-lab/$ARG"; fi
echo "Wireshark en el escritorio virtual: http://localhost:6080"
exec docker compose exec -d --user user -e DISPLAY=:1 cooja bash --login -c "$CMD"
