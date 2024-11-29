#include <WiFi.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <Adafruit_BMP085.h>
#include "UbidotsEsp32Mqtt.h"

#define DHTPIN 23      // Pin donde está conectado el DHT11
#define DHTTYPE DHT11  // Tipo de sensor DHT

#define LAB_TEMP "temperatura";
#define LAB_HUM "humedad";
#define LAB_PRES "presion";
#define LAB_DEVICE "test1";

const char* ssid = "Velocity_DarkWeb";
const char* password = "*********************";
const char* token = "BBUS-N50EYSK1I06Yw1nZcSFQoL2aEAFDLh";

Ubidots ubidots(token);
DHT dht(DHTPIN, DHTTYPE);
Adafruit_BMP085 bmp;

long lastMsg = 0;

void setup() {
  Serial.begin(9600);
  setup_wifi();
  dht.begin();

  // Configuración Ubidots
  ubidots.connectToWifi(ssid, password);
  ubidots.setCallback(callback);
  ubidots.setup();
  ubidots.reconnect();

  if (!bmp.begin()) {
    Serial.println("No se pudo encontrar el sensor BMP180.");
    while (1)
      ;
  }
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

void setup_wifi() {
  delay(10);

  Serial.println();
  Serial.print("Conectando a red ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi conectado");
  Serial.println(WiFi.localIP());
}

void loop() {
  // Conexiòn y reintentos a ubidots
  if (!ubidots.connected()) {
    ubidots.reconnect();
  }

  long now = millis();
  if (now - lastMsg > 3000) {
    lastMsg = now;

    float h = dht.readHumidity();
    float t = dht.readTemperature();
    float p = bmp.readPressure();

    if (isnan(h) || isnan(t) || isnan(p)) {
      Serial.println("Error al leer los sensores");

      return;
    }

    Serial.print("Enviando datos a Ubidots: ");
    ubidots.add(VARIABLE_LABEL_1, temperature);
    ubidots.add(VARIABLE_LABEL_2, humidity);

    bool success = ubidots.publish(DEVICE_LABEL);

    if (success) {
      Serial.println("Datos enviados");
    } else {
      Serial.println("Error Ubidots");
    }
  }
}
