/*
 * Práctica 8 (ruta B) - Nodo real por Wi-Fi + MQTT
 * Placas: Seeed XIAO ESP32-C3/C6/S3, Heltec WiFi LoRa 32 V3, cualquier ESP32 (core arduino-esp32 >= 3.0).
 * Librerías (Gestor de librerías de Arduino): PubSubClient (Nick O'Leary).
 *
 * Publica cada PUB_INTERVAL_MS en   wsn/<id>/data   el MISMO JSON que el mote simulado de la práctica 6:
 *   {"id":101,"seq":12,"temp":27.4,"uptime":240,"src":"esp32"}
 * y obedece órdenes en             wsn/<id>/cmd    ("led=on" / "led=off").
 * Así el flujo de Node-RED del laboratorio muestra motes simulados y reales sin ningún cambio.
 */
#include <WiFi.h>
#include <PubSubClient.h>

// ---------- Ajuste estos valores ----------
const char *WIFI_SSID = "MI_RED";
const char *WIFI_PASS = "MI_CLAVE";
const char *MQTT_HOST = "192.168.1.50";   // IP del PC que ejecuta docker compose (puerto 1883 publicado)
const uint16_t MQTT_PORT = 1883;
const uint16_t NODE_ID = 101;             // use 100+ para no chocar con los ids de Cooja (1..99)
const uint32_t PUB_INTERVAL_MS = 20000;
#ifndef LED_BUILTIN
#define LED_BUILTIN 2
#endif
// -------------------------------------------

WiFiClient net;
PubSubClient mqtt(net);
char topicData[32], topicCmd[32], clientId[32];
uint32_t seq = 0, lastPub = 0;

float readTemperature() {
#if defined(SOC_TEMP_SENSOR_SUPPORTED) || defined(CONFIG_IDF_TARGET_ESP32S3) || defined(CONFIG_IDF_TARGET_ESP32C3) || defined(CONFIG_IDF_TARGET_ESP32C6)
  return temperatureRead();               // sensor interno del SoC (aproximado). Sustituya por su sensor (DHT22, BME280...)
#else
  return 25.0 + (random(-50, 50) / 10.0);
#endif
}

void onMessage(char *topic, byte *payload, unsigned int len) {
  String msg;
  for (unsigned int i = 0; i < len; i++) msg += (char)payload[i];
  Serial.printf("CMD %s: %s\n", topic, msg.c_str());
  if (msg == "led=on") digitalWrite(LED_BUILTIN, HIGH);
  else if (msg == "led=off") digitalWrite(LED_BUILTIN, LOW);
}

void connectWiFi() {
  if (WiFi.status() == WL_CONNECTED) return;
  Serial.printf("WiFi %s ...", WIFI_SSID);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  Serial.printf(" OK %s\n", WiFi.localIP().toString().c_str());
}

void connectMQTT() {
  while (!mqtt.connected()) {
    Serial.printf("MQTT %s:%u ...", MQTT_HOST, MQTT_PORT);
    if (mqtt.connect(clientId)) {
      Serial.println(" conectado");
      mqtt.subscribe(topicCmd);
    } else {
      Serial.printf(" fallo rc=%d, reintento en 5 s\n", mqtt.state());
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  snprintf(clientId, sizeof(clientId), "wsn-esp32-%u", NODE_ID);
  snprintf(topicData, sizeof(topicData), "wsn/%u/data", NODE_ID);
  snprintf(topicCmd, sizeof(topicCmd), "wsn/%u/cmd", NODE_ID);
  connectWiFi();
  mqtt.setServer(MQTT_HOST, MQTT_PORT);
  mqtt.setCallback(onMessage);
}

void loop() {
  connectWiFi();
  connectMQTT();
  mqtt.loop();
  if (millis() - lastPub >= PUB_INTERVAL_MS) {
    lastPub = millis();
    char payload[96];
    snprintf(payload, sizeof(payload), "{\"id\":%u,\"seq\":%lu,\"temp\":%.1f,\"uptime\":%lu,\"src\":\"esp32\"}",
             NODE_ID, (unsigned long)seq, readTemperature(), (unsigned long)(millis() / 1000));
    mqtt.publish(topicData, payload);
    Serial.printf("PUB %s %s\n", topicData, payload);
    seq++;
  }
}
