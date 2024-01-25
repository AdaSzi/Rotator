#ifndef WEBSERVERHANDLER_H
#define WEBSERVERHANDLER_H
#include <ESPAsyncWebServer.h>



void initWebServer();
void notFound(AsyncWebServerRequest *request);
void handleWebServer();
void handleWebSocketMessage(void *arg, uint8_t *data, size_t len);
void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len);
void notifyClients();

void setupMode();
void onEventInSetupMode(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len);
#endif