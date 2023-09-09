#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebSrv.h>
#include <stdio.h>
#include <string.h>
#include "secrets.h"

const char* ssid = WLAN_SSID;
const char* password = WLAN_PASSWORD;

const int amountSensor = 10;

AsyncWebServer server(80);

float temperatur = 20.0; // Startwert für die Temperatur
float luftfeuchtigkeit = 50.0; // Startwert für die Luftfeuchtigkeit
float sensor2 = 25.0; // Startwert für den zweiten Sensor
float sensor3 = 30.0; // Startwert für den dritten Sensor

//dynamic sensor data
float temperatures[amountSensor];
float humidities[amountSensor];

//init dynamic sensor data
void initSensorData(){
  for(int i = 0; i < amountSensor; i ++){
    temperatures[i] = (float)random(18, 26);
    humidities[i] = (float)random(45, 70);
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

  for(int i = 0; i < amountSensor; i++){
    String i_str = String(i);
    String cmnd = "status" + i_str;
    if(request->arg("cmnd") == cmnd){
      status = i;
      i = amountSensor;
      validRequest = true;
    }
  }

  if(validRequest){
    response = generateTempHumData(temperatures[status], humidities[status]);
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

void generateRandomData(float& value1, float& value2) {
  value1 += random(-1, 2) / 10.0; 
  value2 += random(-5, 6) / 10.0; 
  
  // Begrenze die Werte auf sinnvolle Bereiche
  value1 = constrain(value1, 10, 30);
  value2 = constrain(value2, 40, 70);
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

  for(int i = 0; i < amountSensor; i++){
    generateRandomData(temperatures[i], humidities[i]);
  }

  delay(1000); // Warte eine Sekunde
}
