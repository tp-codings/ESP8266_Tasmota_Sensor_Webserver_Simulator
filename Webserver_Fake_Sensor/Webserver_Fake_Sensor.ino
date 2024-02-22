#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

const char* ssid = "Obi Wlan Kenobi";
const char* password = "#19-Nortestosteron";

ESP8266WebServer server(80);

float temperatur = 20.0; // Startwert für die Temperatur
float luftfeuchtigkeit = 50.0; // Startwert für die Luftfeuchtigkeit
float sensor2 = 25.0; // Startwert für den zweiten Sensor
float sensor3 = 30.0; // Startwert für den dritten Sensor

void handleRoot() {
  String html = "<html><body><h1>Sensorimitation</h1></body></html>";
  server.send(200, "text/html", html);
}

void handleCM() {
  String response;

  if(server.arg("cmnd") == "status10") {
    response = generateSensorData(temperatur, luftfeuchtigkeit);
  } else if(server.arg("cmnd") == "status11") {
    response = generateSensorData(sensor2, sensor3);
  } else {
    response = "Ungültiger Sensor-Identifier";
  }

  server.send(200, "application/json", response);
}

String generateSensorData(float value1, float value2) {
  String response = "{\"StatusSNS\":{\"Time\":\"2023-09-08T09:22:10\",\"Sensor\":{\"Value1\":";
  response += String(value1);
  response += ",\"Value2\":";
  response += String(value2);
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
  server.handleClient();

  generateRandomData(temperatur, luftfeuchtigkeit); // Aktualisiere die Daten für Sensor 1
  generateRandomData(sensor2, sensor3); // Aktualisiere die Daten für Sensor 2 und 3
  delay(1000); // Warte eine Sekunde
}
