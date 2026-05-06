#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DHT.h>
#include <ESP8266HTTPClient.h>

const char* ssid = "SSID name";
const char* password = "SSID password";

// Change this to your Phone's Local IP
const char* phoneServer = "http://192.168.1.XXX:3000/data";   // ←←← UPDATE THIS

#define DHT_PIN   4
#define DHT_TYPE  DHT11
#define LDR_PIN   A0

DHT dht(DHT_PIN, DHT_TYPE);
ESP8266WebServer server(80);

float temperature = 0;
float humidity = 0;
int lightLevel = 0;

void setup() {
  Serial.begin(115200);
  dht.begin();

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi Connected!");
  Serial.print("ESP8266 IP Address: http://");
  Serial.println(WiFi.localIP());

  server.on("/", handleRoot);
  server.on("/data", handleData);
  server.begin();

  Serial.println("Web Server Started!");
}

void loop() {
  server.handleClient();

  static unsigned long last = 0;
  if (millis() - last > 10000) {        // Read & Send every 10 seconds
    temperature = dht.readTemperature();
    humidity = dht.readHumidity();
    lightLevel = analogRead(LDR_PIN);

    if (!isnan(temperature) && !isnan(humidity)) {
      sendDataToPhone(temperature, humidity, lightLevel);
    }
    last = millis();
  }
}

// =============== Local Webpage ===============
void handleRoot() {
  String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Weather Station</title>
  <style>
    body { font-family: Arial; background:#0f172a; color:white; text-align:center; padding:15px;}
    h1 { color:#60a5fa; }
    .card { background:#1e2937; margin:15px auto; padding:25px; border-radius:20px; max-width:350px; }
    .value { font-size:2.6rem; font-weight:bold; }
  </style>
</head>
<body>
  <h1>🌤️ Weather Station</h1>
  <div class="card"><div class="value" id="temp">--°C</div>Temperature</div>
  <div class="card"><div class="value" id="hum">--%</div>Humidity</div>
  <div class="card"><div class="value" id="light">--</div>Light Level</div>

  <script>
    setInterval(() => {
      fetch('/data').then(r => r.json()).then(d => {
        document.getElementById('temp').textContent = d.temperature + '°C';
        document.getElementById('hum').textContent = d.humidity + '%';
        document.getElementById('light').textContent = d.light;
      });
    }, 2000);
  </script>
</body>
</html>
  )rawliteral";

  server.send(200, "text/html", html);
}

void handleData() {
  String json = "{\"temperature\":" + String(temperature,1) + 
                ",\"humidity\":" + String(humidity,1) + 
                ",\"light\":" + String(lightLevel) + "}";
  server.send(200, "application/json", json);
}

void sendDataToPhone(float temp, float hum, int light) {
  WiFiClient client;
  HTTPClient http;
  http.begin(client, phoneServer);
  http.addHeader("Content-Type", "application/json");

  String json = "{\"temperature\":" + String(temp,1) + 
                ",\"humidity\":" + String(hum,1) + 
                ",\"light\":" + String(light) + "}";

  http.POST(json);
  http.end();
}