#include <Arduino.h>
#include <credentials.h>
#include <WiFi.h>
#include <SPI.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Adafruit_ADS1015.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

// Setup sensor object and functions

#include <sensors.h>

// Setup network objects
long lastReconnectAttempt = 0;
const char* mqtt_server = "192.168.72.155";
#include <network.h>

void setup() {
  // put your setup code here, to run once:
  lastReconnectAttempt = 0;
  Serial.begin(115200);
  delay(2000);
  Serial.println("Start SETUP");
  // put your setup code here, to run once:
  tempsensors.begin();
  adsbattery.begin();
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  if (reconnect()){
  }
  
  ArduinoOTA.setHostname("SensorStern");

    ArduinoOTA
    .onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH)
        type = "sketch";
      else // U_SPIFFS
        type = "filesystem";

      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
      Serial.println("Start updating " + type);
    })
    .onEnd([]() {
      Serial.println("\nEnd");
    })
    .onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    })
    .onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });

  ArduinoOTA.begin();
  
}

void loop() {
  ArduinoOTA.handle();
  // put your main code here, to run repeatedly:
  if (!client.connected()) {
    long now = millis();
    if (now - lastReconnectAttempt > 5000) {
      lastReconnectAttempt = now;
      // Attempt to reconnect
      if (reconnect()) {
        lastReconnectAttempt = 0;
      }
    }
  } else {
    // Client connected

    client.loop();
  }

  SetFreshWater();
  SetBlackWater();
  SetWaterTemp();
  SetBattery();

  SendMQTT();

 // Gör en korrekt delay!!
  delay(2000);
}
