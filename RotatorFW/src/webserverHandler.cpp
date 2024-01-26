#include "webserverHandler.h"
#include "globals.h"
#include "storage.h"
#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <WiFi.h>
#include <SPIFFS.h>
#include <ESPmDNS.h>
#include <DNSServer.h>
#include <ArduinoJson.h>

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");
GlobalData lastGlobalData;

void initWebServer(){  
  #ifdef DEBUG
    Serial.println("Starting Webserver");
  #endif
  
  server.on("/heap", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(200, "text/plain", String("Free heap: " + String(ESP.getFreeHeap()) + " B"));
  });

  server.on("/info", HTTP_GET, [](AsyncWebServerRequest * request) {
    unsigned long currentMillis = millis();
    unsigned long seconds = currentMillis / 1000;
    unsigned long minutes = seconds / 60;
    unsigned long hours = minutes / 60;
    unsigned long days = hours / 24;
    currentMillis %= 1000;
    seconds %= 60;
    minutes %= 60;
    hours %= 24;
    request->send(200, "text/plain", String("Compiled: " __DATE__ " " __TIME__"\nCPU: " + String(ESP.getChipModel()) + "-" + String(ESP.getChipRevision()) + "\nUptime: " + String(days)+ " days, " + String(hours) + " hours, "+ String(minutes)+ " minutes and " + String(seconds)+ " seconds"));
  });

  server.serveStatic("/", SPIFFS, "/").setDefaultFile("index.html");

  server.serveStatic("/js/compass_degrees.js", SPIFFS, "/js/compass_degrees.js");
  server.serveStatic("/js/main.js", SPIFFS, "/js/main.js");
  
  //server.serveStatic("/favicon.ico", SPIFFS, "/favicon.ico");
  server.serveStatic("/favicon.png", SPIFFS, "/favicon.png");

  server.serveStatic("/Muli-Light.ttf", SPIFFS, "/Muli-Light.ttf");
  //server.serveStatic("/HelveticaMono.ttf", SPIFFS, "/HelveticaMono.ttf");
  server.serveStatic("/styles.css", SPIFFS, "/styles.css");

  server.onNotFound(notFound);
  server.begin();

  ws.onEvent(onEvent);
  server.addHandler(&ws);


  #ifdef DEBUG
    Serial.println("Starting mDNS");  
  #endif
  if (!MDNS.begin("rotator")) {
    #ifdef DEBUG
      Serial.println("Error setting up MDNS responder!");
    #endif
  }
  else {
    #ifdef DEBUG
      Serial.println("mDNS started");  
    #endif  
    MDNS.addService("http", "tcp", 80);
  }
}

void notFound(AsyncWebServerRequest *request) {
  //request->send(404, "text/plain", "404 Not found");
  request->redirect("/");
}


unsigned long nextClientsUpdate = 0;
void handleWebServer() {
  ws.cleanupClients();

  if (/*lastGlobalData != globalData ||*/ millis() > nextClientsUpdate) {    
    nextClientsUpdate = millis() + WEBSOCKET_UPDATE_INTERVAL;

    lastGlobalData = globalData;
    notifyClients();
  }
}

void notifyClients() {
  char json[512];
  sprintf(json, "{\"current_position\": %d, \"target_position\": %d, \"current_speed\": %d}", globalData.currentAzimuth, globalData.targetAzimuth, globalData.currentSpeed);

  ws.textAll(json);
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  #ifdef DEBUG
    //Serial.println((char*)data);
  #endif
  
  StaticJsonDocument<128> tempDoc;
  deserializeJson(tempDoc, (char*)data);

  globalData.targetAzimuth = tempDoc["target_position"];
  notifyClients();
  
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
  switch (type) {
    case WS_EVT_CONNECT:
    #ifdef DEBUG
        Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
    #endif
      notifyClients();
      break;
    case WS_EVT_DISCONNECT:
      #ifdef DEBUG
          Serial.printf("WebSocket client #%u disconnected\n", client->id());
      #endif
      break;
    case WS_EVT_DATA:
      handleWebSocketMessage(arg, data, len);
      break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
      break;
  }
}





DNSServer dnsServer;

class CaptiveRequestHandler : public AsyncWebHandler {
public:
  CaptiveRequestHandler() {}
  virtual ~CaptiveRequestHandler() {}

  bool canHandle(AsyncWebServerRequest *request){
    return true;
  }

  void handleRequest(AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/setup.html", "text/html");
    #ifdef DEBUG
      Serial.println("Served Setup");
    #endif
  }  
};

void setupMode(){
  #ifdef DEBUG
    Serial.println("Entered setup mode");
  #endif

  WiFi.softAP("Rotator_Setup");

  dnsServer.start(53, "*", WiFi.softAPIP());

  server.begin();

  ws.onEvent(onEventInSetupMode);
  server.addHandler(&ws);
  server.addHandler(new CaptiveRequestHandler()).setFilter(ON_AP_FILTER);

  while(true){
    dnsServer.processNextRequest();
    ws.cleanupClients();

    ledBlink(250);
  }
}

void notifyClientsInSetupMode(){
  DynamicJsonDocument copyDoc = mainConfigDoc;
  for (JsonObject wifi : copyDoc["wifi"].as<JsonArray>()) {
    wifi.remove("password");
  }

  String temp;
  serializeJson(copyDoc, temp);
  ws.textAll(temp);
}

 void onEventInSetupMode(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
  switch (type) {
    case WS_EVT_CONNECT:{
        #ifdef DEBUG
              Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
        #endif
        notifyClientsInSetupMode();
      }
      break;
    case WS_EVT_DISCONNECT:
      #ifdef DEBUG
          Serial.printf("WebSocket client #%u disconnected\n", client->id());
      #endif
      break;
    case WS_EVT_DATA:{
        AwsFrameInfo *info = (AwsFrameInfo*)arg;

        deserializeJson(mainConfigDoc, (char*)data);

        #ifdef DEBUG
          Serial.println(F("\nConfig saving:"));
          serializeJsonPretty(mainConfigDoc, Serial);
        #endif

        saveConfig();
        notifyClientsInSetupMode();
        restart();
      }
      break;
  }
}