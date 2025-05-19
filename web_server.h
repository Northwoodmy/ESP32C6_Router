#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <WebServer.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

void initWebServer();
void startWebServerTask(void* parameter);
extern TaskHandle_t webServerTaskHandle;

#endif 