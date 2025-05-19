#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <WebServer.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
<<<<<<< HEAD
#include <esp_system.h>
#include <esp_heap_caps.h>

void initWebServer();
void startWebServerTask(void* parameter);
void handleSystemInfo();
void handleTaskInfo();
=======

void initWebServer();
void startWebServerTask(void* parameter);
>>>>>>> ccfc7efee610ce0d4065fa4704320b3c64f0ff09
extern TaskHandle_t webServerTaskHandle;

#endif 