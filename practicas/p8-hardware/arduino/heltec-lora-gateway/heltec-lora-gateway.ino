/*
 * Práctica 8 (ruta B, variante LoRa) - Gateway LoRa -> Wi-Fi/MQTT (Heltec WiFi LoRa 32 V3)
 * Librerías: RadioLib, PubSubClient.
 *
 * Recibe las tramas LoRa de heltec-lora-node.ino ("n<id>,s<seq>,t<temp>"), añade RSSI/SNR y
 * publica en wsn/<id>/data el mismo JSON de la práctica 6 hacia el Mosquitto del laboratorio.
 * Cumple el papel del border router + tunslip6 de la práctica 5, pero en la capa de aplicación.
 */
#include <RadioLib.h>
#include <WiFi.h>
#include <PubSubClient.h>

const char *WIFI_SSID = "MI_RED";
const char *WIFI_PASS = "MI_CLAVE";
const char *MQTT_HOST = "192.168.1.50";   // IP del PC con docker compose
const uint16_t MQTT_PORT = 1883;

#define LORA_FREQ_MHZ 915.0
#define LORA_BW_KHZ   125.0
#define LORA_SF       9
#define LORA_CR       7
#define LORA_SYNC     0x12

SX1262 radio = new Module(8, 14, 12, 13);
WiFiClient net;
PubSubClient mqtt(net);
volatile bool rxFlag = false;
uint32_t rxCount = 0;

void IRAM_ATTR onReceive() { rxFlag = true; }

void connectAll() {
  if (WiFi.status() != WL_CONNECTED) {
    WiFi.mode(WIFI_STA); WiFi.begin(WIFI_SSID, WIFI_PASS);
    Serial.print("WiFi...");
    while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
    Serial.printf(" %s\n", WiFi.localIP().toString().c_str());
  }
  while (!mqtt.connected()) {
    Serial.print("MQTT...");
    if (mqtt.connect("wsn-lora-gateway")) Serial.println(" conectado");
    else { Serial.printf(" rc=%d\n", mqtt.state()); delay(3000); }
  }
}

void setup() {
  Serial.begin(115200);
  delay(500);
  SPI.begin(9, 11, 10, 8);
  int st = radio.begin(LORA_FREQ_MHZ, LORA_BW_KHZ, LORA_SF, LORA_CR, LORA_SYNC, 14, 8, 1.8, false);
  if (st != RADIOLIB_ERR_NONE) { Serial.printf("SX1262 error %d\n", st); while (true) delay(1000); }
  radio.setDio2AsRfSwitch(true);
  radio.setPacketReceivedAction(onReceive);
  radio.startReceive();
  mqtt.setServer(MQTT_HOST, MQTT_PORT);
  connectAll();
  Serial.println("Gateway LoRa listo");
}

void loop() {
  connectAll();
  mqtt.loop();
  if (!rxFlag) return;
  rxFlag = false;

  String msg;
  int st = radio.readData(msg);
  float rssi = radio.getRSSI(), snr = radio.getSNR();
  radio.startReceive();
  if (st != RADIOLIB_ERR_NONE) { Serial.printf("RX error %d\n", st); return; }
  rxCount++;

  unsigned id = 0, seq = 0; int temp10 = 0;
  if (sscanf(msg.c_str(), "n%u,s%u,t%d", &id, &seq, &temp10) != 3) {
    Serial.printf("RX trama desconocida: %s\n", msg.c_str());
    return;
  }
  char topic[32], payload[128];
  snprintf(topic, sizeof(topic), "wsn/%u/data", id);
  snprintf(payload, sizeof(payload),
           "{\"id\":%u,\"seq\":%u,\"temp\":%d.%d,\"rssi\":%.0f,\"snr\":%.1f,\"src\":\"lora\"}",
           id, seq, temp10 / 10, abs(temp10 % 10), rssi, snr);
  mqtt.publish(topic, payload);
  Serial.printf("RX #%lu %s -> %s %s\n", (unsigned long)rxCount, msg.c_str(), topic, payload);
}
