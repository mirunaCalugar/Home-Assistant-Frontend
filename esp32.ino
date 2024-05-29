#include <WiFi.h>
#include <WebServer.h>

const char* ssid = "DUBA_7_FILAJ_DIICOT";
const char* password = "parola1998";

WebServer server(80);

String temperature = "";
String humidity = "";

void handleRoot() {
  String html = "<html><body><h1>Sensor Data</h1>";
  html += "<p>Temperature: " + temperature + " C</p>";
  html += "<p>Humidity: " + humidity + " %</p>";
  html += "<button onclick=\"toggleLED('A')\">Turn ON LED</button>";
  html += "<button onclick=\"toggleLED('S')\">Turn OFF LED</button>";
  html += "<button onclick=\"updateData()\">Update Data</button>";
  html += "<script>";
  html += "function toggleLED(state) {";
  html += "  var xhr = new XMLHttpRequest();";
  html += "  xhr.open('GET', '/' + state, true);";
  html += "  xhr.send();";
  html += "}";
  html += "function updateData() {";
  html += "  var xhr = new XMLHttpRequest();";
  html += "  xhr.open('GET', '/T', true);";
  html += "  xhr.send();";
  html += "}";
  html += "</script>";
  html += "</body></html>";
  server.send(200, "text/html", html);
}

void handleLEDOn() {
  Serial.print('A');
  server.send(200, "text/plain", "LED turned ON");
}

void handleLEDOff() {
  Serial.print('S');
  server.send(200, "text/plain", "LED turned OFF");
}

void handleTemperatureAndHumidity() {
  Serial.print('T');
  delay(1100);
  if (Serial.available()) {
    String data = Serial.readStringUntil('\n');
    int tempIndex = data.indexOf("Temperature: ");
    int humIndex = data.indexOf("Humidity: ");

    if (tempIndex != -1 && humIndex != -1) {
      int tempEnd = data.indexOf(" C", tempIndex);
      int humEnd = data.indexOf(" %", humIndex);
      
      if (tempEnd != -1 && humEnd != -1) {
        temperature = data.substring(tempIndex + 13, tempEnd);
        humidity = data.substring(humIndex + 10, humEnd);
      }
    }
  }
  server.send(200, "text/plain", "Temperature and Humidity Updated");
}

void setup() {
  Serial.begin(9600);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }

  Serial.println("Connected to WiFi");
  server.on("/", handleRoot);
  server.on("/A", handleLEDOn);
  server.on("/S", handleLEDOff);
  server.on("/T", handleTemperatureAndHumidity);
  server.begin();
}

void loop() {
  server.handleClient();
}
