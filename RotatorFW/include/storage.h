#ifndef STORAGE_H
#define STORAGE_H
#include <ArduinoJson.h>
#include "FS.h"

bool initStorage();
void saveConfig();
void deleteConfig();
void listDir(fs::FS &fs, const char * dirname, uint8_t levels);

#endif