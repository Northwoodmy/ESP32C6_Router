#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <WiFi.h>
#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

// WiFi配置结构体
struct WifiConfig {
    char ssid[32];
    char password[64];
    bool configured;
};

// 函数声明
void initWiFiManager();
void startWiFiTask(void* parameter);
bool isWiFiConnected();
IPAddress getLocalIP();
IPAddress getAPIP();
void setupAP();

extern TaskHandle_t wifiTaskHandle;
extern WifiConfig wifiConfig;

#endif 