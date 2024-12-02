#include <WiFi.h>
#include <DHT.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP085.h>
#include "UbidotsEsp32Mqtt.h"

#define DHTPIN 4       // Pin donde está conectado el DHT11
#define DHTTYPE DHT11  // Tipo de sensor DHT

#define LAB_TEMP "temperatura"
#define LAB_HUM "humedad"
#define LAB_PRES "presion"
#define LAB_DEVICE "test1"

const char* ssid = "*********";
const char* password = "*********";
const char* token = "BBUS-N50EYSK1I06Yw1nZcSFQoL2aEAFDLh";

Ubidots ubidots(token);
DHT dht(DHTPIN, DHTTYPE);
Adafruit_BMP085 bmp;

long lastMsg = 0;

void setup() {
  Serial.begin(9600);
  dht.begin();

  // Configuración Ubidots
  ubidots.connectToWifi(ssid, password);
  ubidots.setCallback(callback);
  ubidots.setup();
  ubidots.reconnect();

  while (!bmp.begin()) {
    Serial.println("No se pudo encontrar el sensor BMP180.");
    delay(1000);
  }

  Serial.println("Sensor BMP180 configurado.");
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Mensaje recibido [");
  Serial.print(topic);
  Serial.print("]: ");

  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }

  Serial.println();
}

void loop() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  float p = bmp.readPressure();

  Serial.println("Temperatura: ");
  Serial.println(String(t));
  Serial.println("Humedad: ");
  Serial.println(String(h));
  Serial.println("Presión: ");
  Serial.println(String(p));

  delay(1000);

  // Conexión y reintentos a ubidots
  if (!ubidots.connected()) {
    ubidots.reconnect();
  }

  long now = millis();
  if (now - lastMsg > 3000) {
    lastMsg = now;


    if (isnan(h) || isnan(t)) {
      Serial.println("Error al leer los sensores");

      return;
    }

    Serial.print("Enviando datos a Ubidots: ");
    ubidots.add(LAB_TEMP, t);
    ubidots.add(LAB_HUM, h);

    if (!isnan(p)) {
      pHPa = p / 100;

      ubidots.add(LAB_PRES, pHPa);
    }

    bool success = ubidots.publish(LAB_DEVICE);

    if (success) {
      Serial.println("Datos enviados");
    } else {
      Serial.println("Error Ubidots");
    }
  }
}
