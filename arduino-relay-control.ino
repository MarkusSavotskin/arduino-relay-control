#include "Arduino.h"
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

const char* ssid = "SSID";
const char* password = "Password";

// Relay pin
const int relay_pin = 3;
bool relay_status = false;

unsigned long previousMillis = 0;
unsigned long interval = 30000;

const char* wl_status_to_string(wl_status_t status) {
  switch (status) {
    case WL_NO_SHIELD: return "WL_NO_SHIELD";
    case WL_IDLE_STATUS: return "WL_IDLE_STATUS";
    case WL_NO_SSID_AVAIL: return "WL_NO_SSID_AVAIL";
    case WL_SCAN_COMPLETED: return "WL_SCAN_COMPLETED";
    case WL_CONNECTED: return "WL_CONNECTED";
    case WL_CONNECT_FAILED: return "WL_CONNECT_FAILED";
    case WL_CONNECTION_LOST: return "WL_CONNECTION_LOST";
    case WL_DISCONNECTED: return "WL_DISCONNECTED";
    default: return "UNKNOWN_STATUS";
  }
}

ESP8266WebServer server(80);

// Serving Hello world
void relayOn() {
    digitalWrite(relay_pin, HIGH);
    relay_status = true;
    server.send(200, "text/json", "{\"relay\": \"on\"}");
    Serial.println("Turned relay on");
}

void relayOff() {
    digitalWrite(relay_pin, LOW);
    relay_status = false;
    server.send(200, "text/json", "{\"relay\": \"off\"}");
    Serial.println("Turned relay off");
}

void getRelayStatus() {
    if (relay_status) {
      server.send(200, "text/json", "{\"relay\": \"on\"}");
      Serial.println("Relay on");
    } else {
      server.send(200, "text/json", "{\"relay\": \"off\"}");
      Serial.println("Relay off");
    }
}
 
// Define routing
void restServerRouting() {
    server.on("/", HTTP_GET, []() {
        server.send(200, F("text/html"),
            F("Hello stranger!")); 
    });
    server.on(F("/relay-on"), HTTP_GET, relayOn);
    server.on(F("/relay-off"), HTTP_GET, relayOff);
    server.on(F("/relay-status"), HTTP_GET, getRelayStatus);
}
 
// Manage not found URL
void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

void initWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.println(WiFi.localIP());
  WiFi.setAutoReconnect(true);
  WiFi.persistent(true);
}

void setup() {
  Serial.begin(115200);

  pinMode(relay_pin, OUTPUT);
  digitalWrite(relay_pin, LOW);

  initWiFi();
 
  // Set server routing
  restServerRouting();
  // Set not found response
  server.onNotFound(handleNotFound);
  // Start server
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  //print the Wi-Fi status every 30 seconds
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    Serial.print("Connection status: ");
    Serial.println(wl_status_to_string(WiFi.status()));
    previousMillis = currentMillis;
  }
  server.handleClient();
}