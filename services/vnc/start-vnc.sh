#!/bin/bash
# Arranca el escritorio virtual del laboratorio:
#   Xtigervnc :1  -> servidor X + VNC (puerto 5901)
#   openbox       -> gestor de ventanas ligero (mover/redimensionar ventanas de Cooja)
#   websockify    -> noVNC en el puerto 6080 (visor en el navegador)
# Variables: VNC_GEOMETRY (1600x900), VNC_PASSWORD (vacío = sin contraseña)
set -u
GEOM="${VNC_GEOMETRY:-1600x900}"
DISPLAY_NUM="${DISPLAY#:}"
DISPLAY_NUM="${DISPLAY_NUM:-1}"
VNC_PORT=$((5900 + DISPLAY_NUM))
NOVNC_PORT="${NOVNC_PORT:-6080}"

mkdir -p "$HOME/.vnc"
rm -f "/tmp/.X${DISPLAY_NUM}-lock" "/tmp/.X11-unix/X${DISPLAY_NUM}"

SECURITY=(-SecurityTypes None)
if [ -n "${VNC_PASSWORD:-}" ]; then
  printf '%s\n' "$VNC_PASSWORD" | tigervncpasswd -f > "$HOME/.vnc/passwd"
  chmod 600 "$HOME/.vnc/passwd"
  SECURITY=(-SecurityTypes VncAuth -PasswordFile "$HOME/.vnc/passwd")
fi

echo "[vnc] Xtigervnc :$DISPLAY_NUM geometria=$GEOM puerto=$VNC_PORT"
Xtigervnc ":$DISPLAY_NUM" -geometry "$GEOM" -depth 24 -rfbport "$VNC_PORT" \
  -localhost no -AlwaysShared -desktop "WSN-lab Cooja" "${SECURITY[@]}" \
  > "$HOME/.vnc/Xtigervnc.log" 2>&1 &
XPID=$!

# esperar a que el display exista
for _ in $(seq 1 50); do
  [ -e "/tmp/.X11-unix/X${DISPLAY_NUM}" ] && break
  sleep 0.2
done

xsetroot -solid "#2b3a42" 2>/dev/null || true
openbox > "$HOME/.vnc/openbox.log" 2>&1 &

echo "[vnc] noVNC en http://localhost:${NOVNC_PORT}/  (VNC nativo: localhost:${VNC_PORT})"
websockify --web=/usr/share/novnc "$NOVNC_PORT" "localhost:${VNC_PORT}" \
  > "$HOME/.vnc/websockify.log" 2>&1 &

# Lanzar Cooja automáticamente si se pide (COOJA_AUTOSTART=1)
if [ "${COOJA_AUTOSTART:-0}" = "1" ]; then
  (sleep 2; cd "$HOME/contiki-ng/tools/cooja" && \
    "$HOME/.local/bin/cooja" --args="--gui --contiki=$HOME/contiki-ng" \
    > "$HOME/.vnc/cooja.log" 2>&1) &
fi

trap 'kill $XPID 2>/dev/null; exit 0' INT TERM
wait $XPID
