#ifndef HTTP_CLIENT_H
#define HTTP_CLIENT_H

#include <Arduino.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include "config_manager.h"

extern TaskHandle_t httpClientTaskHandle;

void initHttpClient();
void startHttpClientTask(void* parameter);
String getMetricsData();

#endif 