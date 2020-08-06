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
#include <Filter.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include "Filter.h"
#include "ArduinoTimer.h"

StaticJsonDocument<250> jsonDocument;
// Pin configuration
const int TankSensorEnablepin=25;
const int WaterTempPin=4;
const int FreshWaterPin=34;
const int BlackWaterPin=35;
const int DieselSensorEnablePin=15;
const int DieselPin=33;
const int Sense12Vpin=32;

// Timers
ArduinoTimer SensorShortTimer;
ArduinoTimer SensorLongTimer;

// Setup sensor object and functions

#include <sensors.h>

// Setup network objects

long lastReconnectAttempt = 0;
//const char* mqtt_server = "192.168.72.155";
//const char* mqtt_server = "192.168.72.36";
const char* mqtt_server = "10.10.10.1";
#include <network.h>

void setup() {
  // put your setup code here, to run once:
  

  // Timers
  lastReconnectAttempt = 0;
  Serial.begin(115200);
  delay(2000);
  digitalWrite(LED,LOW);
  Serial.println("Start SETUP");
  // I/O
  pinMode(LED,OUTPUT);
  digitalWrite(LED,HIGH);
  pinMode(TankSensorEnablepin,OUTPUT);
  digitalWrite(TankSensorEnablepin,HIGH);
  pinMode(DieselSensorEnablePin,OUTPUT);
  digitalWrite(DieselSensorEnablePin,HIGH);

  // put your setup code here, to run once:
  setupSensors();
  setup_wifi();
  mqttClient.setServer(mqtt_server, 1883);
  mqttClient.setKeepAlive(60);
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
  
  setup_routing(); 	

}

void loop() {
  webserver.handleClient();	
  digitalWrite(LED,HIGH);
  ArduinoOTA.handle();
  // put your main code here, to run repeatedly:
  if(WiFi.status() != WL_CONNECTED){
    setup_wifi();
  }
  if (!mqttClient.connected()) {
    long now = millis();
    if (now - lastReconnectAttempt > 2000) {
      lastReconnectAttempt = now;
      // Attempt to reconnect
      if (reconnect()) {
        lastReconnectAttempt = 0;
      }
    }
  } else {
    // Client connected

    mqttClient.loop();
  }

if(SensorShortTimer.TimePassed_Seconds(5)){
  digitalWrite(LED,HIGH);
  SetTankLevel();
  //SetFreshWater();
  //SetBlackWater();
  SetWaterTemp();
  SetBattery();

  SendMQTT();
  digitalWrite(LED,LOW);
}

}
