WiFiClient espClient;
PubSubClient client(espClient);
char output[8];
//char realpath;

// ***  Functions for network *****
void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, ssidpwd);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

boolean reconnect() {
  Serial.print("CONNECT MQTT...");
  if (client.connect("SensorStern")) {
    // Once connected, publish an announcement...
    //client.publish("outTopic","hello world");
    Serial.println("OK");
  }
  else {
    Serial.println("Failed");    
  }
  return client.connected();
}

void SendMQTT(){
  Serial.println("Send MQTT...");

  for (size_t i = 0; i < NUM_SENSORS; i++)
  {
    Serial.print("  " + sensors[i].name + " : ");
    Serial.println(sensors[i].value);

    dtostrf(sensors[i].value, 1, 2, output);
    //int len = sensors[i].path.length();
    char* charpath = (char*) sensors[i].path.c_str();
    client.publish(charpath,output);
  }
}
