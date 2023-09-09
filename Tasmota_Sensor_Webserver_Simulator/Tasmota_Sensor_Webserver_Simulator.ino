#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebSrv.h>
#include <stdio.h>
#include <string.h>
#include "secrets.h"

const char* ssid = WLAN_SSID;
const char* password = WLAN_PASSWORD;

const int amountTempHumSensors = 10;
const int amountEnergySensors = 10;

AsyncWebServer server(80);

//dynamic sensor data
float temperatures[amountTempHumSensors];
float humidities[amountTempHumSensors];

float voltages[amountEnergySensors];
float currents[amountEnergySensors];

//init dynamic sensor data
void initSensorData(){
  for(int i = 0; i < amountTempHumSensors; i ++){
    temperatures[i] = (float)random(18, 26);
    humidities[i] = (float)random(45, 70);
  }

  for(int i = 0; i < amountEnergySensors; i ++){
    voltages[i] = (float)random(220, 240);
    currents[i] = (float)random(1, 50) / 1000.0;
  }
}


void handleRoot(AsyncWebServerRequest *request) {
  String html = "<html><body><h1>Sensorimitation</h1></body></html>";
  request->send(200, "text/html", html);
}

void handleCM(AsyncWebServerRequest *request) {
  String response;
  bool validRequest = false;
  int status = -1;
  int type = -1;
  for(int i = 0; i < amountTempHumSensors; i++){
    String i_str = String(i);
    String cmnd = "status00" + i_str;
    if(request->arg("cmnd") == cmnd){
      status = i;
      type = 0;
      i = amountTempHumSensors;
      validRequest = true;
    }
  }
  for(int i = 0; i < amountEnergySensors; i++){
    String i_str = String(i);
    String cmnd = "status10" + i_str;
    if(request->arg("cmnd") == cmnd){
      status = i;
      type = 1;
      i = amountEnergySensors;
      validRequest = true;
    }
  }

  if(validRequest){
    switch(type){
      case 0:
        response = generateTempHumData(temperatures[status], humidities[status]);
        break;
      case 1:
        response = generateEnergyData(voltages[status], currents[status]);
        break;
    }
  } else {
    response = "Ungültiger Sensor-Identifier";
  }

  Serial.println(response);
  
  request->send(200, "application/json", response);
}

float calculateDewPoint(float temperature, float humidity){
    float sdd = 6.112 * exp((17.67 * temperature) / (temperature + 243.5));
    float dd = (humidity / 100) * sdd;
    float v = log(dd / 6.112);
    float dewPoint = (243.5 * v) / (17.67 - v);
    return dewPoint;
}

String generateTempHumData(float temperature, float humidity) {
  String response = "{\"StatusSNS\":{\"Time\":\"2023-09-08T09:22:10\",\"DHT11\":{\"Temperature\":";
  response += String(temperature);
  response += ",\"Humidity\":";
  response += String(humidity);
  response += ",\"DewPoint\":";
  response += String(calculateDewPoint(temperature, humidity));
  response += "}}}";

  return response;
}

String generateEnergyData(float voltage, float current){
  String response = "{\"StatusSNS\":{\"Time\":\"2023-09-08T09:22:10\",\"Energy\":{\"Voltage\":";
  response += String(voltage);
  response += ",\"Current\":";
  response += String(current);
  response += ",\"Power\":";
  response += String(voltage*current);
  response += "}}}";

  return response;
}

void generateRandomData(float& value1, float& value2, int type) {
  
  // Temp Hum
  if(type == 0){
    value1 += random(-1, 2) / 10.0; 
    value2 += random(-5, 6) / 10.0; 
    value1 = constrain(value1, 10, 30);
    value2 = constrain(value2, 40, 70);
  }
  // Energy
  else if(type == 1){
    value1 += random(-1, 2); 
    value2 += random(-5, 6) / 1000.0; 
    value1 = constrain(value1, 200, 270);
    value2 = constrain(value2, 0.0, 1.0);
  }
}

void setup() {
  Serial.begin(115200);

  initSensorData();

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }

  Serial.println("Connected to WiFi");

  server.on("/", handleRoot);
  server.on("/cm", handleCM);

  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  //server.handleClient();

  for(int i = 0; i < amountTempHumSensors; i++){
    generateRandomData(temperatures[i], humidities[i], 0);
  }
  for(int i = 0; i < amountEnergySensors; i++){
    generateRandomData(voltages[i], currents[i], 1);
  }

  delay(1000); // Warte eine Sekunde
}
