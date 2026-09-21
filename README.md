# WSN Lab · Redes de Sensores Inalámbricos con Contiki-NG y Cooja

Entorno de laboratorio reproducible en **Docker** para el curso de Redes de Sensores Inalámbricos (Universidad de Cuenca). Programa motes en C con **Contiki-NG**, simúlalos en **Cooja** desde el navegador, conecta la red simulada con un **border router** y publica datos por **CoAP/MQTT** hacia **Mosquitto** y **Node-RED**. Incluye nueve prácticas, de un "hola mundo" hasta hardware real y un proyecto integrador.

![Contiki-NG](https://img.shields.io/badge/Contiki--NG-develop-blue)
![Cooja](https://img.shields.io/badge/simulador-Cooja-orange)
![Docker](https://img.shields.io/badge/Docker-Compose-2496ED?logo=docker&logoColor=white)
![MQTT](https://img.shields.io/badge/MQTT-Mosquitto-660066?logo=eclipsemosquitto&logoColor=white)
![Node-RED](https://img.shields.io/badge/Node--RED-dashboard-8F0000?logo=nodered&logoColor=white)

## Características

- **Sin instalar nada más que Docker**: Contiki-NG, Cooja, Wireshark, tshark y tcpdump viven dentro del contenedor.
- **Cooja en el navegador**: escritorio virtual con noVNC en `http://localhost:6080` (o VNC nativo en `localhost:5901`). No necesitas X11, XQuartz ni WSLg.
- **Igual en Linux, macOS y Windows** (Docker Desktop o WSL2).
- **Red simulada conectada al host**: `tunslip6` expone la red 6LoWPAN/RPL como `fd00::/64`.
- **Pila IoT completa**: broker Mosquitto y flujo Node-RED de ejemplo suscrito a `wsn/#`.
- **Capturas .pcap automáticas** de la radio 802.15.4 y de la interfaz `tun0`.
- **Modo sin GUI** para barridos de experimentos y análisis por scripts.
- **Hardware real** (opcional): nRF52840, XIAO nRF54L15, ESP32 (Wi-Fi/LoRa) y Zigbee2MQTT.

## Arquitectura

```
┌─────────────────────────────── docker compose ───────────────────────────────┐
│  wsn-cooja  (contiker/contiki-ng + Xtigervnc + openbox + noVNC + Wireshark)  │
│    noVNC :6080 / VNC :5901 ── escritorio virtual ── Cooja (motes simulados)  │
│                                                       │ Serial Socket :60001 │
│    tunslip6 ── tun0  fd00::1  ◄──►  pila IPv6 del contenedor                 │
│                                        ▲               ▲                     │
│  wsn-mosquitto (red compartida) ───────┘               │                     │
│  wsn-nodered   (red compartida) ───────────────────────┘                     │
└──────────────┬──────────────────────────┬────────────────────────────────────┘
     localhost:1883 (MQTT)       localhost:1880 (Node-RED)
```

Mosquitto y Node-RED comparten la red del contenedor `cooja`, por eso los motes simulados alcanzan el broker en `fd00::1` a través del border router.

## Requisitos

- Docker Engine 24+ (o Docker Desktop) con `docker compose`.
- Unos 3 GB de disco (imagen de Contiki-NG, código fuente y caché de Gradle).
- Git.

## Inicio rápido

```bash
git clone https://github.com/Pbarbecho/wsn-lab.git
cd wsn-lab
./setup.sh               # clona Contiki-NG, crea .env y construye las imágenes (una sola vez)
docker compose up -d     # levanta cooja + mosquitto + node-red
./scripts/vnc-viewer.sh  # abre http://localhost:6080
./scripts/cooja-gui.sh   # lanza Cooja (la primera vez Gradle tarda 2-5 min)
```

| Servicio | URL / puerto |
|---|---|
| Cooja (noVNC, navegador) | <http://localhost:6080> |
| Cooja (cliente VNC) | `localhost:5901` |
| Node-RED | <http://localhost:1880> |
| Mosquitto (MQTT) | `localhost:1883` |

Para fijar una versión de Contiki-NG: `CNG_TAG=release/v4.9 ./setup.sh`.

### Configuración (`.env`)

`setup.sh` crea `.env` a partir de tus datos; puedes partir también de [`.env.example`](.env.example).

| Variable | Descripción | Valor por defecto |
|---|---|---|
| `VNC_GEOMETRY` | Resolución del escritorio virtual | `1600x900` |
| `VNC_PASSWORD` | Contraseña VNC (vacío = sin contraseña) | vacío |
| `COOJA_AUTOSTART` | `1` arranca Cooja con el contenedor | `0` |
| `LOCAL_UID` / `LOCAL_GID` | Tu usuario del host, para que los archivos no queden como root | `1000` |

> Si expones los puertos fuera de tu máquina, define `VNC_PASSWORD`.

## Uso diario

```bash
docker compose up -d                                          # arrancar
./scripts/shell.sh                                            # terminal en el contenedor
./scripts/cooja-gui.sh practicas/p4-rpl-udp/p4-rpl-udp.csc    # abrir una simulación
./scripts/cooja-headless.sh practicas/p4-rpl-udp/p4-rpl-udp.csc 1   # sin GUI, semilla 1
./scripts/connect-router.sh                                   # tunslip6 (prácticas 5, 6 y 8)
./scripts/wireshark.sh tun0                                   # captura en vivo del border router
./scripts/capture-tun0.sh resultados/p5.pcap 60               # captura con tshark sin GUI
docker compose down                                           # detener
```

Dentro del contenedor el repositorio está en `/home/user/wsn-lab` y Contiki-NG en `/home/user/contiki-ng`. Para compilar un firmware a mano:

```bash
cd practicas/p1-hello-sensor && make TARGET=cooja
```

## Prácticas

| # | Tema | Carpeta |
|---|---|---|
| 1 | Primer mote: protothreads, timers, LEDs y log | [`p1-hello-sensor`](practicas/p1-hello-sensor) |
| 2 | Un salto sin IP (NullNet): broadcast, unicast, RSSI/LQI y PDR | [`p2-nullnet`](practicas/p2-nullnet) |
| 3 | MAC y energía: CSMA vs TSCH con Energest | [`p3-energia-mac`](practicas/p3-energia-mac) |
| 4 | Multisalto con RPL + UDP | [`p4-rpl-udp`](practicas/p4-rpl-udp) |
| 5 | Border router, `tunslip6`, `ping6` y Wireshark | [`p5-border-router`](practicas/p5-border-router) |
| 6 | Protocolos IoT: CoAP y MQTT hacia Mosquitto y Node-RED | [`p6-coap-mqtt`](practicas/p6-coap-mqtt) |
| 7 | Escalabilidad: barrido de N nodos sin GUI | [`p7-escalabilidad`](practicas/p7-escalabilidad) |
| 8 | Hardware real: nRF52840/nRF54L15, ESP32 Wi-Fi/LoRa, Zigbee | [`p8-hardware`](practicas/p8-hardware) |
| 9 | Proyecto integrador: plantilla de nodo e informe | [`p9-proyecto`](practicas/p9-proyecto) |

## Estructura del repositorio

```
wsn-lab/
├── docker-compose.yml            cooja + mosquitto + node-red
├── docker-compose.hardware.yml   override para hardware real (USB + Zigbee2MQTT)
├── setup.sh                      preparación inicial
├── .env.example                  plantilla de configuración
├── scripts/                      shell, visor VNC, Cooja GUI/headless, tunslip6, Wireshark, gen_csc.py
├── services/
│   ├── vnc/                      Dockerfile del escritorio virtual (Xtigervnc + openbox + noVNC)
│   ├── mosquitto/                configuración del broker
│   ├── nodered/                  flujo de ejemplo (wsn/#)
│   └── zigbee2mqtt/              configuración del coordinador Zigbee
├── practicas/                    p1 … p9
└── resultados/                   logs y capturas generados por las simulaciones
```

`contiki-ng/` y `.cache/` los crea `setup.sh` y no se versionan.

## Direcciones IPv6 en Cooja

El mote *N* tiene dirección de enlace `000N.000N.000N.000N` y, con el prefijo del border router, la dirección global `fd00::20N:N:N:N`. Por ejemplo, el mote 2 es `fd00::202:2:2:2` y el mote 10 es `fd00::20a:a:a:a`. La interfaz `tun0` del contenedor es `fd00::1`, que es también la dirección del broker MQTT vista desde los motes.

## Capturas de tráfico

| Punto de captura | Qué se ve | Cómo |
|---|---|---|
| Radio de Cooja | Tramas IEEE 802.15.4 con 6LoWPAN, RPL, CoAP y MQTT tal como viajan por el aire | Automático en `resultados/<simulación>.pcap` (modo GUI) |
| `tun0` (prácticas 5+) | IPv6 ya descomprimido entre el border router y el host | `./scripts/wireshark.sh tun0` o `./scripts/capture-tun0.sh` |

Hay una captura de ejemplo en [`practicas/p4-rpl-udp/ejemplo-p4-cadena.pcap`](practicas/p4-rpl-udp/ejemplo-p4-cadena.pcap). Filtros útiles: `icmpv6.type == 155` (RPL), `coap`, `mqtt`, `wpan.frame_type == 0x2` (ACK).

## Hardware real (práctica 8)

```bash
docker compose -f docker-compose.yml -f docker-compose.hardware.yml up -d
docker compose -f docker-compose.yml -f docker-compose.hardware.yml --profile zigbee up -d   # con Zigbee2MQTT
```

- **Ruta A, Contiki-NG en placa**: nRF52840 dongle/DK o Seeed XIAO nRF54L15 como border router real (`./scripts/connect-router-hw.sh`). El paso de USB al contenedor solo funciona en Linux; en macOS y Windows expón el puerto serie por TCP con `socat` (ver el script).
- **Ruta B, ESP32 por Wi-Fi + MQTT**: sketches de Arduino en [`practicas/p8-hardware/arduino`](practicas/p8-hardware/arduino), incluida una pareja LoRa nodo → gateway (Heltec WiFi LoRa 32 V3).
- **Ruta C, Zigbee**: Zigbee2MQTT con un coordinador USB (SONOFF ZBDongle-E/P, etc.).

Más detalles en [`practicas/p8-hardware/README.md`](practicas/p8-hardware/README.md).

## Solución de problemas

- **El visor muestra "Failed to connect"**: el escritorio aún está arrancando. Revisa `docker compose logs cooja`. Si cambiaste `VNC_PASSWORD`, recrea el contenedor con `docker compose up -d --force-recreate cooja`.
- **Cooja no aparece**: la primera compilación con Gradle tarda de 2 a 5 minutos; revisa la salida de `cooja-gui.sh`.
- **Pantalla pequeña o borrosa**: ajusta `VNC_GEOMETRY` en `.env` y en noVNC usa *Settings → Scaling mode: Remote resizing*.
- **`tunslip6` falla con "Operation not permitted"**: añade `privileged: true` al servicio `cooja` en `docker-compose.yml`.
- **Los motes no alcanzan `fd00::1`**: la simulación debe estar corriendo, con el plugin *Serial Socket (SERVER)* en el border router y `./scripts/connect-router.sh` activo.
- **Archivos creados como root**: revisa `LOCAL_UID`/`LOCAL_GID` en `.env` y recrea el contenedor.
- **"Nothing to be done" al cambiar `MAKE_MAC`**: ejecuta `make TARGET=cooja clean` antes de recompilar.

## Referencias

- Contiki-NG: <https://github.com/contiki-ng/contiki-ng> · Documentación: <https://docs.contiki-ng.org>
- Imagen Docker oficial: <https://hub.docker.com/r/contiker/contiki-ng>

## Autor

Pablo Barbecho · Universidad de Cuenca · [pbarbecho.com](https://pbarbecho.com)
