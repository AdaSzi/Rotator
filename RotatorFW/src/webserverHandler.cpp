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
String domain;

void initWebServer(){  
  #ifdef DEBUG
    Serial.println("Starting Webserver");
  #endif

  server.on("/status", HTTP_GET, [](AsyncWebServerRequest * request) {
    unsigned long currentMillis = millis();
    unsigned long seconds = currentMillis / 1000;
    unsigned long minutes = seconds / 60;
    unsigned long hours = minutes / 60;
    unsigned long days = hours / 24;
    currentMillis %= 1000;
    seconds %= 60;
    minutes %= 60;
    hours %= 24;
    request->send(200, "text/plain", String("Compiled: " __DATE__ " " __TIME__"\nCPU: " + String(ESP.getChipModel()) + "-" + String(ESP.getChipRevision()) + "\nUptime: " + String(days)+ " days, " + String(hours) + " hours, " + String(minutes) + " minutes and " + String(seconds) + " seconds\n" + String("Free heap: " + String(ESP.getFreeHeap()) + " B")));
  });

  server.on("/config", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(200, "application/json", mainConfigDocString);
  });

  server.on("/calibrate", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(204, "text/plain","");
    rotator.calibrate();
  });

  server.on("/restart", HTTP_GET, [](AsyncWebServerRequest * request) {
    #ifdef DEBUG
      Serial.print("Domain: ");
      Serial.println(domain);
    #endif

    String html = F("<!DOCTYPE html><html><head><meta http-equiv=\"refresh\" content=\"30;url=http://");
    html += domain;
    html += F(".local\"></head><body><h2>Restarting</h2><p>You will be redirected to <a href='");
    html += domain;
    html += F(".local'>");
    html += domain;    
    html += F(".local</a> shortly.</p><p>Timing has been set to 30 seconds to leave enough time for your rotator controller to reboot and reconnect to wifi.</p></body></html>");
    
    request->send(200, "text/html", html);
    restart();
  });

  server.serveStatic("/", SPIFFS, "/").setDefaultFile("index.html");
  server.serveStatic("/settings", SPIFFS, "settings.html");
  server.serveStatic("/common", SPIFFS, "common.js");

  //server.serveStatic("/js/compass_degrees.js", SPIFFS, "/js/compass_degrees.js");
  //server.serveStatic("/js/main.js", SPIFFS, "/js/main.js");
  //server.serveStatic("/favicon.ico", SPIFFS, "/favicon.ico");
  server.serveStatic("/favicon.png", SPIFFS, "/favicon.png");
  server.serveStatic("/Muli-Light.ttf", SPIFFS, "/Muli-Light.ttf");
  //server.serveStatic("/HelveticaMono.ttf", SPIFFS, "/HelveticaMono.ttf");

  server.onNotFound(notFound);
  server.begin();

  ws.onEvent(onEvent);
  server.addHandler(&ws);


  #ifdef DEBUG
    Serial.println("Starting mDNS");  
  #endif
  domain = mainConfigDoc["settings"]["mDNS"]|"rotator";
  if (!MDNS.begin(domain)) {
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

#ifdef DEBUG
  uint8_t mps = 0;
#endif

void handleWebServer() {
  ws.cleanupClients();

  static unsigned long nextClientsUpdateHB = 0;
  static unsigned long nextClientsUpdateFast = 0;
  if (millis() > nextClientsUpdateHB) {    
    nextClientsUpdateHB = millis() + WEBSOCKET_UPDATE_INTERVAL;
    notifyClients();
  }

  if (lastGlobalData != globalData && millis() > nextClientsUpdateFast) {    
    nextClientsUpdateFast = millis() + 100;

    nextClientsUpdateHB = millis() + WEBSOCKET_UPDATE_INTERVAL;

    lastGlobalData = globalData;
    notifyClients();
  }

  #ifdef DEBUG
    static unsigned long mpsTimer = 0;
    if (millis() > mpsTimer) {    
      mpsTimer = millis() + 1000;
      //Serial.print("MPS: ");
      //Serial.println(mps);
      mps=0;
    }
  #endif
}

void notifyClients() {
  if(ws.availableForWriteAll() && ws.count()){
    char json[512];
    sprintf(json, "{\"current_position\": %d, \"target_position\": %d, \"current_speed\": %d}", globalData.currentAzimuth, globalData.targetAzimuth, globalData.currentSpeed);

    ws.textAll(json);
    #ifdef DEBUG
      mps++;
    #endif
  }
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len, AsyncWebSocketClient *client) {
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  #ifdef DEBUG
    Serial.println((char*)data);
  #endif
  
  StaticJsonDocument<8192> tempDoc;
  deserializeJson(tempDoc, (char*)data);
  

  #ifdef DEBUG
    Serial.println("Got data:");
    serializeJsonPretty(tempDoc, Serial);
  #endif

  if(tempDoc.containsKey("target_position")){
    rotator.setTargetPosition(tempDoc["target_position"]);
    notifyClients();
  }
  else if(tempDoc.containsKey("settings")){
    mainConfigDoc = tempDoc;
    saveConfig(mainConfigDoc, "/config.json");
    serializeJson(mainConfigDoc, mainConfigDocString);
    if (ws.availableForWriteAll() && ws.count()) {
      ws.textAll(mainConfigDocString);
#ifdef DEBUG
      mps++;
#endif
    }
    domain = mainConfigDoc["settings"]["mDNS"].as<String>();
    applyLiveSettings();
    
  #ifdef DEBUG
    Serial.print("Domain:");
    Serial.println(domain);
  #endif
  }  
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
      handleWebSocketMessage(arg, data, len, client);
      break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
      break;
  }
}





//SetupMode
DNSServer dnsServer;

class CaptiveRequestHandler : public AsyncWebHandler {
public:
  CaptiveRequestHandler() {}
  virtual ~CaptiveRequestHandler() {}

  bool canHandle(AsyncWebServerRequest *request){
    return true;
  }

  void handleRequest(AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/wifisetup.html", "text/html");
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

    ledBlink(200);
  }
}

void notifyClientsInSetupMode(){
  DynamicJsonDocument copyDoc = wifiConfig;
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

        deserializeJson(wifiConfig, (char*)data);

        #ifdef DEBUG
          Serial.println(F("\nWiFiConfig saving:"));
          serializeJsonPretty(wifiConfig, Serial);
        #endif

        saveConfig(wifiConfig, "/WiFiConfig.json");
        notifyClientsInSetupMode();
        restart();
      }
      break;
  }
}