// #include <WebSocketClient.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoWebsockets.h>

#include <WiFiClient.h>
#include <Arduino_JSON.h>

WiFiClient client;
HTTPClient http;
// WebSocketClient ws(false);
using namespace websockets;

WebsocketsClient wsclient;

const char* ssid = "Fest";
const char* password = "12345678";
int Led = 16;
int GroundLight = 4;  //D2
int Level1Light = 5;  //D1
int Socket = 0;       //D3

String fetchAppliancesState = "https://autochalitbackend.onrender.com/appliances";

bool fetchedState = false;

void setup() {
  Serial.begin(9600);
  WiFi.begin(ssid, password);

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  bool connected = wsclient.connect("wss://autochalitbackend.onrender.com");

  while (!connected) {
    wsclient.connect("wss://autochalitbackend.onrender.com");
    Serial.println("connecting to websocket");
    delay(500);
  }

  pinMode(Led, OUTPUT);
  pinMode(GroundLight, OUTPUT);
  pinMode(Level1Light, OUTPUT);
  pinMode(Socket, OUTPUT);
  delay(100);
  digitalWrite(GroundLight, LOW);
  digitalWrite(Level1Light, LOW);
  digitalWrite(Socket, LOW);
  fetchInitialState();
  // connectWebSocket();
}

void loop() {

  if (WiFi.status() == WL_CONNECTED) {
    digitalWrite(Led, HIGH);
  } else {
    digitalWrite(Led, LOW);
  }

  if (wsclient.available()) {
    wsclient.poll();
    // Serial.println("connected");
  }

  wsclient.onMessage(onMessageCallback);
}

void fetchInitialState() {
  http.begin(client, fetchAppliancesState);

  int responseCode = http.GET();

  if (responseCode > 0) {
    Serial.println("Success! Fetched data:");
    String payload = http.getString();
    Serial.println(payload);
    fetchedState = true;

    // Parse the JSON data
    JSONVar responseObject = JSON.parse(payload);

    if (JSON.typeof(responseObject) == "undefined") {
      Serial.println("Parsing input failed!");
      fetchedState = false;
    } else {
      bool groundLightState = false;
      bool level1LightState = false;
      bool socketState = false;
      JSONVar appliances = responseObject["appliances"];
      for (int i = 0; i < appliances.length(); i++) {
        if (String(appliances[i]["name"]) == "groundLight") {
          groundLightState = (bool)appliances[i]["state"];
          digitalWrite(GroundLight, groundLightState ? HIGH : LOW);
          Serial.print("GroundLight state set to: ");
          Serial.println(groundLightState ? "ON" : "OFF");
        } else if (String(appliances[i]["name"]) == "level1Light") {
          level1LightState = (bool)appliances[i]["state"];
          digitalWrite(Level1Light, level1LightState ? HIGH : LOW);
          Serial.print("level 1 light state set to: ");
          Serial.println(level1LightState ? "ON" : "OFF");
        }

        else if (String(appliances[i]["name"]) == "powerSocket") {

          socketState = (bool)appliances[i]["state"];
          digitalWrite(Socket, socketState ? HIGH : LOW);
          Serial.print("socket state set to: ");
          Serial.println(socketState ? "ON" : "OFF");
        }
      }
    }

  } else {
    Serial.print("Error on HTTP request, response code: ");
    Serial.println(responseCode);
    fetchedState = false;
  }

  http.end();
}

// void connectWebSocket() {
//   while (!ws.isConnected()) {
//     Serial.println("Connecting to WebSocket...");
//     ws.connect("192.168.137.82", "/", 4000);
//     delay(500);
//   }
//   Serial.println("Socket Connected");
// }

void onMessageCallback(WebsocketsMessage message) {
  Serial.println("Message received:");
  Serial.println(message.data());
  processWebSocketMessage(message.data());
}

void processWebSocketMessage(String message) {
  JSONVar responseObject = JSON.parse(message);
  if (JSON.typeof(responseObject) == "undefined") {
    Serial.println("Parsing input failed!");
    return;
  } else {
    String name = String(responseObject["name"]);
    if (name == "groundLight") {
      bool groundLightState = (bool)responseObject["state"];
      digitalWrite(GroundLight, groundLightState ? HIGH : LOW);
      Serial.print("GroundLight state set to: ");
      Serial.println(groundLightState ? "ON" : "OFF");
    } else if (name == "level1Light") {
      bool level1LightState = (bool)responseObject["state"];
      digitalWrite(Level1Light, level1LightState ? HIGH : LOW);
      Serial.print("Level1Light state set to: ");
      Serial.println(level1LightState ? "ON" : "OFF");
    } else if (name == "powerSocket") {
      bool socketState = (bool)responseObject["state"];
      digitalWrite(Socket, socketState ? HIGH : LOW);
      Serial.print("socket state set to: ");
      Serial.println(socketState ? "ON" : "OFF");
    }
  }
}
