#!/usr/bin/env bash
# Abre el visor noVNC del escritorio de Cooja en el navegador predeterminado.
cd "$(dirname "$0")/.."
docker compose up -d cooja >/dev/null
URL="http://localhost:6080/"
echo "Visor de Cooja (noVNC): $URL"
echo "Cliente VNC nativo:     localhost:5901"
if command -v xdg-open >/dev/null 2>&1; then xdg-open "$URL" >/dev/null 2>&1 &
elif command -v open >/dev/null 2>&1; then open "$URL"
elif command -v powershell.exe >/dev/null 2>&1; then powershell.exe -c "Start-Process '$URL'"
else echo "Abra la URL manualmente en su navegador."; fi
