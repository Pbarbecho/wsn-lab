# Práctica 8 – Del simulador al hardware real: nodos físicos en Node-RED

Tres rutas para llevar datos de sensores reales al mismo Mosquitto + Node-RED del laboratorio.
La guía completa está en `guias/guias-wsn.pdf` (Práctica 8).

| Ruta | Hardware | Pila | Qué se reutiliza |
|---|---|---|---|
| A · misma pila | nRF52840 dongle (Nordic), Seeed XIAO nRF54L15, nRF52840-DK | Contiki-NG: 802.15.4 + RPL + CoAP/MQTT (idéntico a P4–P6) | firmware de P5/P6 sin cambios; border router real + `tunslip6` en el contenedor |
| B · Wi-Fi/MQTT | XIAO ESP32-C3/C6/S3, Heltec WiFi LoRa 32 V3 (+ variante LoRa P2P) | Arduino: Wi-Fi + MQTT (PubSubClient); LoRa con RadioLib | mismo esquema de topics/JSON que P6; flujo Node-RED sin cambios |
| C · Zigbee | XIAO ESP32-C6 (Zigbee end device) + coordinador USB (SONOFF ZBDongle-E/P) | Zigbee 3.0 → Zigbee2MQTT (contenedor) | broker y Node-RED; pestaña "Hardware real" normaliza `zigbee2mqtt/#` |

## Archivos

```
p8-hardware/
├── README.md
└── arduino/
    ├── wifi-mqtt-node/wifi-mqtt-node.ino        ruta B: nodo ESP32 Wi-Fi → MQTT (wsn/<id>/data, wsn/<id>/cmd)
    ├── heltec-lora-node/heltec-lora-node.ino    ruta B/LoRa: nodo sensor LoRa (SX1262, RadioLib)
    └── heltec-lora-gateway/heltec-lora-gateway.ino   ruta B/LoRa: gateway LoRa → Wi-Fi/MQTT
../../docker-compose.hardware.yml                 USB al contenedor (BR real) + servicio zigbee2mqtt (perfil "zigbee")
../../services/zigbee2mqtt/configuration.yaml     configuración de Zigbee2MQTT
../../scripts/connect-router-hw.sh                tunslip6 hacia un border router real (USB o TCP)
../../services/nodered/flows.json                 pestaña "Hardware real - Práctica 8"
```

## Ruta A en tres comandos (Linux, nRF52840 dongle)

```bash
docker compose -f docker-compose.yml -f docker-compose.hardware.yml up -d
./scripts/shell.sh
#   dentro del contenedor:
make -C ~/contiki-ng/examples/rpl-border-router TARGET=nrf BOARD=nrf52840/dongle border-router.upload PORT=/dev/ttyACM0
cd practicas/p6-coap-mqtt && make TARGET=nrf BOARD=nrf52840/dongle mqtt-node.upload PORT=/dev/ttyACM1
#   en el host:
./scripts/connect-router-hw.sh            # tun0 = fd00::1; el nodo real publica en wsn/<id>/data
```

Para la XIAO nRF54L15: `TARGET=nrf BOARD=nrf54l15/xiao` y objetivo `.flash` (OpenOCD por CMSIS-DAP); solo CSMA (sin TSCH).

## Ruta B en dos pasos

1. Abra `arduino/wifi-mqtt-node/wifi-mqtt-node.ino` en el IDE de Arduino, ponga su SSID/clave y la IP de su PC en `MQTT_HOST`, y suba a la placa.
2. `mosquitto_sub -h localhost -t 'wsn/#' -v` y http://localhost:1880 (pestaña *Hardware real*).

## Ruta C

```bash
docker compose -f docker-compose.yml -f docker-compose.hardware.yml --profile zigbee up -d
# http://localhost:8080  (frontend Zigbee2MQTT) -> Permit join -> encienda el XIAO ESP32-C6 con el ejemplo
# Arduino "Zigbee > Zigbee_Temp_Sensor" (modo Zigbee End Device en Tools > Zigbee mode)
```
