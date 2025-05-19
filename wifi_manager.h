#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <WiFi.h>
#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_wifi.h>
#include <esp_netif.h>
#include <lwip/dns.h>
#include <dhcpserver/dhcpserver.h>

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
void enableNAT();
void configureIP();

extern TaskHandle_t wifiTaskHandle;
extern WifiConfig wifiConfig;
extern esp_netif_t* ap_netif;
extern esp_netif_t* sta_netif;

#endif 