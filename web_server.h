#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <WebServer.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_system.h>
#include <esp_heap_caps.h>

void initWebServer();
void startWebServerTask(void* parameter);
void handleSystemInfo();
void handleTaskInfo();
extern TaskHandle_t webServerTaskHandle;

#endif 