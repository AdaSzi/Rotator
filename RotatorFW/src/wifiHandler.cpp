#include "wifiHandler.h"
#include "webserverHandler.h"
#include "globals.h"

#include <Arduino.h>
#include <WiFiMulti.h>

WiFiMulti WiFiMulti;

void initWifi(){
    WiFi.mode(WIFI_STA);
    
    for (JsonObject cfg : mainConfigDoc["wifi"].as<JsonArray>()) {
        if (cfg["ssid"]) WiFiMulti.addAP(cfg["ssid"], cfg["password"]);
    }

    Serial.print("Waiting for WiFi...");
    while(WiFiMulti.run() != WL_CONNECTED) {
        delay(100);
        //Serial.print('.');
        if(millis() > 20000) setupMode();
    }
    Serial.println();
    Serial.print("Connected to: ");
    Serial.print(WiFi.SSID());
    Serial.print(" with IP address: ");
    Serial.println(WiFi.localIP());
}

void handleWifi(){
    static bool hasBeenConnected = true;
    if (WiFiMulti.run() == WL_CONNECTED) {
        if (!hasBeenConnected) {
            hasBeenConnected = true;
            Serial.print("Reconnected to: ");
            Serial.print(WiFi.SSID());
            Serial.print(" with IP address: ");
            Serial.println(WiFi.localIP());
        }
    }
    else {
        hasBeenConnected = false;
    }
}