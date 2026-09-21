#!/usr/bin/env bash
# Abre una terminal dentro del contenedor de Contiki-NG (usuario "user").
cd "$(dirname "$0")/.."
docker compose up -d cooja >/dev/null
exec docker compose exec -it --user user cooja bash --login
