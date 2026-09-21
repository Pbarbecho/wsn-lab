# Imagen del laboratorio: contiker/contiki-ng + servidor VNC + noVNC (visor en el navegador)
# + Wireshark/tshark/tcpdump para analizar las capturas .pcap de Cooja y del tun0.
# Sustituye el reenvío X11: Cooja se dibuja en un escritorio virtual dentro del contenedor
# y se ve desde http://localhost:6080 (noVNC) o con cualquier cliente VNC en localhost:5901.
FROM --platform=linux/amd64 contiker/contiki-ng:latest

USER root
ENV DEBIAN_FRONTEND=noninteractive
# wireshark-common/install-setuid: dumpcap con setuid para que "user" (grupo wireshark)
# pueda capturar en tun0 sin ser root.
RUN echo "wireshark-common wireshark-common/install-setuid boolean true" | debconf-set-selections && \
    apt-get -qq update && \
    apt-get -qq -y --no-install-recommends install \
      wireshark \
      tshark \
      tcpdump \
      tigervnc-standalone-server \
      tigervnc-common \
      openbox \
      novnc \
      websockify \
      fonts-dejavu-core \
      xterm \
      x11-utils \
      dbus-x11 > /dev/null && \
    usermod -aG wireshark user && \
    apt-get -qq clean && rm -rf /var/lib/apt/lists/*

COPY --chmod=755 start-vnc.sh /usr/local/bin/start-vnc.sh
COPY openbox-rc.xml /etc/xdg/openbox/rc.xml
COPY openbox-menu.xml /etc/xdg/openbox/menu.xml
COPY novnc-index.html /usr/share/novnc/index.html

# Escritorio virtual :1 (VNC 5901) y noVNC en 6080
ENV DISPLAY=:1
EXPOSE 5901 6080

# El ENTRYPOINT de la imagen base (remap-user.sh) sigue activo: ajusta el uid/gid
# de "user" y ejecuta el comando como ese usuario.
USER root
CMD ["/usr/local/bin/start-vnc.sh"]
