#include "storage.h"
#include "main.h"
#include "globals.h"
#include <Arduino.h>
#include <ArduinoJson.h>
#include <SPIFFS.h>
#include "FS.h"

bool initStorage(){    
  if(!SPIFFS.begin()){
    #ifdef DEBUG
      Serial.println("SPIFFS Mount Failed");
    #endif
    restart();
  }
  #ifdef DEBUG
    Serial.println("SPIFFS Mount Successful");
    listDir(SPIFFS, "/", 0);
    Serial.flush();
  #endif

  if(SPIFFS.exists("/config.json")) {
    #ifdef DEBUG
      Serial.println("config.json found! - loading config...");
    #endif
    File file = SPIFFS.open("/config.json", FILE_READ);
    DeserializationError err = deserializeJson(mainConfigDoc, file);
    if (err) {
      #ifdef DEBUG
        Serial.print(F("deserializeJson() failed with code "));
        Serial.println(err.c_str());

        Serial.println("File Content:");
        while (file.available()) Serial.write(file.read());
        file.close();
        Serial.println();
      #endif
    }
    else {

    file.close();
    
    serializeJson(mainConfigDoc, mainConfigDocString);


    #ifdef DEBUG
      Serial.println(F("\nMainConfig loaded:"));
      Serial.println(mainConfigDocString);
    #endif

    }
  }

  if(SPIFFS.exists("/WiFiConfig.json")) {
    #ifdef DEBUG
      Serial.println("WiFiConfig.json found! - loading config...");
    #endif
    File file = SPIFFS.open("/WiFiConfig.json", FILE_READ);
    DeserializationError err = deserializeJson(wifiConfig, file);
    if (err) {
      #ifdef DEBUG
        Serial.print(F("deserializeJson() failed with code "));
        Serial.println(err.c_str());

        Serial.println("File Content:");
        while (file.available()) Serial.write(file.read());
        file.close();
        Serial.println();
      #endif

      return false;
    }

    file.close();

    #ifdef DEBUG
      Serial.println(F("\nWiFionfig loaded:"));
      serializeJsonPretty(wifiConfig, Serial);
      Serial.flush();
    #endif

    return true;
  }

  #ifdef DEBUG
    Serial.println("WiFiConfig.json not found!");
  #endif

  return false;
}


void saveConfig(const JsonDocument& json, char* fileName) {
  deleteConfig(fileName);
  File configFile = SPIFFS.open(fileName, FILE_WRITE);
  if (!configFile) {
    #ifdef DEBUG
      Serial.println("Failed to open file for writing");
    #endif
    return;
  }
  if (serializeJsonPretty(json, configFile) == 0) {
    #ifdef DEBUG
      Serial.println(F("Failed to write to file"));
    #endif
  }
  configFile.close();
  #ifdef DEBUG
    Serial.print(fileName);
    Serial.print(F(" saved"));
  #endif
}


void deleteConfig(char* fileName){
  SPIFFS.remove(fileName);
}

#ifdef DEBUG
void listDir(fs::FS &fs, const char * dirname, uint8_t levels) {
  Serial.printf("Listing directory: %s\n", dirname);

  File root = fs.open(dirname);
  if (!root) {
      Serial.println("Failed to open directory");
    return;
  }
  if (!root.isDirectory()) {
      Serial.println("Not a directory");
    return;
  }

  File file = root.openNextFile();
  while (file) {
    if (file.isDirectory()) {
      Serial.print("  DIR : ");
      Serial.println(file.name());
      if (levels) {
        listDir(fs, file.name(), levels - 1);
      }
    } else {
      Serial.print("  FILE: ");
      Serial.print(file.name());
      Serial.print("  SIZE: ");
      Serial.println(file.size());
    }
    file = root.openNextFile();
  }
}
#endif