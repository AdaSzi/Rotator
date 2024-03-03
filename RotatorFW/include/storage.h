#ifndef STORAGE_H
#define STORAGE_H
#include <ArduinoJson.h>
#include "FS.h"

bool initStorage();
void saveConfig(const JsonDocument& json, char* fileName);
void deleteConfig(char* fileName);
void listDir(fs::FS &fs, const char * dirname, uint8_t levels);

#endif