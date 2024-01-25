#include "storage.h"
#include "main.h"
#include "globals.h"
#include <Arduino.h>
#include <ArduinoJson.h>
#include <SPIFFS.h>
#include "FS.h"

bool initStorage(){    
  if(!SPIFFS.begin()){
    Serial.println("SPIFFS Mount Failed");
    restart();
  }
  Serial.println("SPIFFS Mount Successful");

  listDir(SPIFFS, "/", 0);
  Serial.flush();

  if(SPIFFS.exists("/config.json")) {
    Serial.println("Saves.json found! - loading config...");
    File file = SPIFFS.open("/config.json", FILE_READ);
    DeserializationError err = deserializeJson(mainConfigDoc, file);
    if (err) {
      Serial.print(F("deserializeJson() failed with code "));
      Serial.println(err.c_str());

      Serial.println("File Content:");
      while (file.available()) Serial.write(file.read());
      file.close();
      Serial.println();

      return false;
    };

    file.close();
    Serial.println(F("\nConfig loaded:"));
    serializeJsonPretty(mainConfigDoc, Serial);
    Serial.flush();
    return true;
  }

  Serial.println("config.json not found!");
  return false;
}


void saveConfig() {
  deleteConfig();
  File configFile = SPIFFS.open("/config.json", FILE_WRITE);
  if (!configFile) {
    Serial.println("Failed to open file for writing");
    return;
  }
  if (serializeJsonPretty(mainConfigDoc, configFile) == 0) {
    Serial.println(F("Failed to write to file"));
  }
  configFile.close();
  Serial.println(F("Config saved"));
}


void deleteConfig(){
  SPIFFS.remove("/config.json");
}

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
