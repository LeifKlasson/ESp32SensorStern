WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);
WebServer webserver(80);

char buffer[250];
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
  if(WiFi.status() != WL_CONNECTED){
    setup_wifi();
  }
   IPAddress ip = WiFi.localIP() ;
    char * bufIP = new char[70]();
   // sprintf(bufIP, "\"IP%d.%d.%d.%d\"", ip[0], ip[1], ip[2], ip[3]);
    sprintf(bufIP, "vessels/self/sensor/SensorStern/ip%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
    //const char ip=ipnumber.c_str();
    Serial.print("bufIP :  ");
    Serial.println(bufIP);
  if (mqttClient.connect("SensorStern")) {
    // Once connected, publish an announcement...
    mqttClient.publish(bufIP,"Active");
    Serial.println("MQTT OK");
  }
  else {
    mqttClient.disconnect();
    Serial.println("Failed, try to reset WiFiClient...");
    WiFi.disconnect();
    setup_wifi();
    //Connect MQTT again
    if (mqttClient.connect("SensorStern")) {
     Serial.println("OK with WiFi Reconnect");
    mqttClient.publish(bufIP,"Active");
    } else {
    Serial.println("Failed even with reset WiFiClient");
    }
  }
  return mqttClient.connected();
}

void SendMQTT(){
  Serial.println("Send MQTT...");

  for (size_t i = 0; i < NUM_SENSORS; i++)
  {
    Serial.print("  " + sensors[i].name + " : ");
    Serial.println(sensors[i].value);

    if(sensors[i].isActive){
      dtostrf(sensors[i].value, 1, 2, output);
      //int len = sensors[i].path.length();
      char* charpath = (char*) sensors[i].path.c_str();
      mqttClient.publish(charpath,output);
      sensors[i].oldValue=sensors[i].value ;
    }

    mqttClient.loop();
  }
}

void create_json(char *tag, float value, char *unit) {  
  jsonDocument.clear();
  jsonDocument["type"] = tag;
  jsonDocument["value"] = value;
  jsonDocument["unit"] = unit;
  serializeJson(jsonDocument, buffer);  
}

void add_json_object(char *tag, float value, char *unit) {
  JsonObject obj = jsonDocument.createNestedObject();
  obj["type"] = tag;
  obj["value"] = value;
  obj["unit"] = unit; 
}
void getTank() {
  Serial.println("Get tank");
  create_json("FreshWater", sensors[FRESHWATER].value, "ratio");
  webserver.send(200, "application/json", buffer);
}

void getEnv() {
  Serial.println("Get env");
  jsonDocument.clear(); // Clear json buffer
  add_json_object("FreshWater", sensors[FRESHWATER].value, "ratio");
  add_json_object("BLACKWATER", sensors[BLACKWATER].value, "ratio");
  add_json_object("WATERTEMP", sensors[WATERTEMP].value, "°C");
  serializeJson(jsonDocument, buffer);
  webserver.send(200, "application/json", buffer);
}

void getRoot() {
  Serial.println("Get root");
  /*String response = "<HTML><HEAD><TITLE>Sensorstern</TITLE></HEAD><BODY><H3>Sensorstern</H3>" ;
  response += "<P>FreshWater = " ; //+ sensors[FRESHWATER].value ;
  response += "<P>BlackWater = " ;//+ sensors[BLACKWATER].value ;
  response += "</BODY>";
  */
 char FreshBuffer[20];
 char BlackBuffer[20];
 char WTBuffer[20];
 dtostrf(sensors[FRESHWATER].value, 3, 0, FreshBuffer);
 dtostrf(sensors[BLACKWATER].value, 3, 0, BlackBuffer);
 dtostrf(sensors[WATERTEMP].value, 4, 1, WTBuffer);
 
  sprintf(buffer,"<HTML><BODY><P>FreshWater = %s\n<P>BackWater = %s\n<P>WaterTemp = %s\n</BODY></HTML>",FreshBuffer,BlackBuffer,WTBuffer);
  webserver.send(200, "text/html", buffer);
}

// setup API resources
void setup_routing() {
  webserver.on("/tank", getTank);
  webserver.on("/env", getEnv);
  webserver.on("/",getRoot);
 
  // start server
  webserver.begin();
}




