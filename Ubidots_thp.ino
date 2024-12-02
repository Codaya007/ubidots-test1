#include <WiFi.h>
#include <DHT.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP085.h>
#include "UbidotsEsp32Mqtt.h"

#define DHTPIN 4       // Pin donde está conectado el DHT11
#define DHTTYPE DHT11  // Tipo de sensor DHT
#define LED_PIN 5      // Pin donde está conectado el LED

#define LAB_TEMP "temperatura"
#define LAB_HUM "humedad"
#define LAB_PRES "presion"
#define LAB_DEVICE "test1"

const char* ssid = "Internet_UNL";
const char* password = "UNL1859WiFi";
const char* token = "BBUS-N50EYSK1I06Yw1nZcSFQoL2aEAFDLh";

Ubidots ubidots(token);
DHT dht(DHTPIN, DHTTYPE);
Adafruit_BMP085 bmp;

long lastMsg = 0;

void setup() {
  Serial.begin(9600);
  dht.begin();

  // Configuración del LED
  pinMode(LED_PIN, OUTPUT);  // Configuramos el pin del LED como salida
  digitalWrite(LED_PIN, LOW); // Apagamos el LED inicialmente

  // Configuración Ubidots
  ubidots.connectToWifi(ssid, password);
  ubidots.setCallback(callback); // Callback para controlar el LED
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

  String message;
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  Serial.println(message);

  // Control del LED desde Ubidots
  if (message == "1") {
    digitalWrite(LED_PIN, HIGH);  // Encender el LED
    Serial.println("LED Encendido");
  } else if (message == "0") {
    digitalWrite(LED_PIN, LOW);   // Apagar el LED
    Serial.println("LED Apagado");
  }
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

  // Conexión y reintentos a Ubidots
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
      float pHPa = p / 100; // Convertir presión a hPa
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
