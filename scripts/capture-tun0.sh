#!/usr/bin/env bash
# Captura sin GUI el tráfico IPv6 de la red simulada que cruza tun0 (requiere connect-router.sh activo).
#   ./scripts/capture-tun0.sh resultados/p5.pcap [segundos]
# Luego: ./scripts/wireshark.sh resultados/p5.pcap  o  tshark -r ... dentro del contenedor.
cd "$(dirname "$0")/.."
OUT="${1:-resultados/tun0-$(date +%Y%m%d-%H%M%S).pcap}"
SECS="${2:-60}"
echo "Capturando tun0 durante $SECS s -> $OUT"
docker compose exec -T --user user cooja bash --login -c \
  "tshark -i tun0 -a duration:$SECS -w /home/user/wsn-lab/$OUT -q && tshark -r /home/user/wsn-lab/$OUT -q -z io,phs"
