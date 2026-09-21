/*
 * Práctica 8 (ruta B, variante LoRa) - Nodo sensor LoRa punto a punto (Heltec WiFi LoRa 32 V3)
 * Librería: RadioLib (Jan Gromeš). Placa en el IDE: "Heltec WiFi LoRa 32(V3)".
 *
 * Envía cada TX_INTERVAL_MS una línea de texto   "n<id>,s<seq>,t<temp>"   (mismo formato que la
 * práctica 4) al gateway (heltec-lora-gateway.ino), que la reenvía por MQTT a wsn/<id>/data.
 * Es el equivalente "de verdad" de la estrella de un salto de la práctica 2, con LoRa en vez de 802.15.4.
 *
 * Pines SX1262 en Heltec V3: NSS 8, DIO1 14, RST 12, BUSY 13; SPI SCK 9, MISO 11, MOSI 10.
 * Frecuencia: 915 MHz (banda 902-928 MHz, Ecuador/América). Ajuste a 868 MHz en Europa.
 */
#include <RadioLib.h>

#define LORA_FREQ_MHZ   915.0
#define LORA_BW_KHZ     125.0
#define LORA_SF         9
#define LORA_CR         7
#define LORA_SYNC       0x12
#define LORA_POWER_DBM  14
#define NODE_ID         201
#define TX_INTERVAL_MS  15000

SX1262 radio = new Module(8, 14, 12, 13);
uint32_t seq = 0;

void setup() {
  Serial.begin(115200);
  delay(500);
  SPI.begin(9, 11, 10, 8);
  Serial.print("SX1262 init... ");
  // begin(freq, bw, sf, cr, syncWord, power, preambleLength, tcxoVoltage, useRegulatorLDO)
  int st = radio.begin(LORA_FREQ_MHZ, LORA_BW_KHZ, LORA_SF, LORA_CR, LORA_SYNC, LORA_POWER_DBM, 8, 1.8, false);
  if (st != RADIOLIB_ERR_NONE) { Serial.printf("error %d\n", st); while (true) delay(1000); }
  radio.setDio2AsRfSwitch(true);          // el Heltec V3 conmuta la antena con DIO2
  Serial.println("OK");
}

void loop() {
  float temp = 25.0 + (random(-30, 30) / 10.0);   // sustituya por su sensor real
  char msg[48];
  snprintf(msg, sizeof(msg), "n%u,s%lu,t%d", NODE_ID, (unsigned long)seq, (int)(temp * 10));
  int st = radio.transmit(msg);
  if (st == RADIOLIB_ERR_NONE) Serial.printf("TX %s (%.1f ms en el aire)\n", msg, radio.getTimeOnAir(strlen(msg)) / 1000.0);
  else Serial.printf("TX error %d\n", st);
  seq++;
  delay(TX_INTERVAL_MS);
}
