#include "wifiHandler.h"
#include "webserverHandler.h"
#include "globals.h"

#include <Arduino.h>
#include <WiFiMulti.h>

WiFiMulti WiFiMulti;

void initWifi(){
    WiFi.mode(WIFI_STA);
    
    for (JsonObject cfg : wifiConfig["wifi"].as<JsonArray>()) {
        if (cfg["ssid"]) WiFiMulti.addAP(cfg["ssid"], cfg["password"]);
    }

    #ifdef DEBUG
        Serial.print("Waiting for WiFi...");
    #endif
    while(WiFiMulti.run() != WL_CONNECTED) {
        delay(100);
        if(millis() > 20000) {
            //setupMode();
            restart();
        }
    }
    #ifdef DEBUG
        Serial.println();
        Serial.print("Connected to: ");
        Serial.print(WiFi.SSID());
        Serial.print(" with IP address: ");
        Serial.println(WiFi.localIP());
    #endif
}

void handleWifi(){
    static bool hasBeenConnected = true;
    if (WiFiMulti.run() == WL_CONNECTED) {
        if (!hasBeenConnected) {
            hasBeenConnected = true;

            #ifdef DEBUG
                Serial.print("Reconnected to: ");
                Serial.print(WiFi.SSID());
                Serial.print(" with IP address: ");
                Serial.println(WiFi.localIP());
            #endif
        }
    }
    else {
        static unsigned long disconectionTime = 0;
        if(hasBeenConnected) disconectionTime = millis();
        
        if(disconectionTime + 30000 < millis()) restart();

        hasBeenConnected = false;
    }
}