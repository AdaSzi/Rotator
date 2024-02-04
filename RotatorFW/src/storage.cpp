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
      Serial.println("Saves.json found! - loading config...");
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

      return false;
    };

    file.close();

    #ifdef DEBUG
      Serial.println(F("\nConfig loaded:"));
      serializeJsonPretty(mainConfigDoc, Serial);
      Serial.flush();
    #endif

    
    DynamicJsonDocument staticConfigDoc = mainConfigDoc;
    staticConfigDoc.remove("wifi");    

    serializeJson(staticConfigDoc, sharedConfigJson);


    #ifdef DEBUG
      Serial.println(F("\nStaticConfig created:"));
      serializeJsonPretty(staticConfigDoc, Serial);
      Serial.flush();
      Serial.println();
    #endif
    
    
    return true;
  }

  #ifdef DEBUG
    Serial.println("config.json not found!");
  #endif

  return false;
}


void saveConfig() {
  deleteConfig();
  File configFile = SPIFFS.open("/config.json", FILE_WRITE);
  if (!configFile) {
    #ifdef DEBUG
      Serial.println("Failed to open file for writing");
    #endif
    return;
  }
  if (serializeJsonPretty(mainConfigDoc, configFile) == 0) {
    #ifdef DEBUG
      Serial.println(F("Failed to write to file"));
    #endif
  }
  configFile.close();
  #ifdef DEBUG
    Serial.println(F("Config saved"));
  #endif
}


void deleteConfig(){
  SPIFFS.remove("/config.json");
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
