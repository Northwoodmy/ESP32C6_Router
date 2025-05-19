#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <WiFi.h>
#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_wifi.h>
#include <esp_netif.h>
#include <lwip/dns.h>
#include <lwip/ip4.h>
#include <lwip/ip4_frag.h>
#include <lwip/netif.h>
#include <lwip/ip_addr.h>
#include <lwip/ip.h>
#include <lwip/opt.h>
#include <lwip/err.h>
#include <lwip/inet.h>
#include <lwip/ip4_napt.h>
#include <dhcpserver/dhcpserver.h>

// WiFi配置结构体
struct WifiConfig {
    char ssid[32];
    char password[64];
    bool configured;
};

// DHCP配置变量声明
extern const IPAddress ap_local_ip;
extern const IPAddress ap_gateway;
extern const IPAddress ap_subnet;
extern const IPAddress dhcp_pool_start;
extern const IPAddress dhcp_pool_end;

// 函数声明
void initWiFiManager();
void startWiFiTask(void* parameter);
bool isWiFiConnected();
IPAddress getLocalIP();
IPAddress getAPIP();
void setupAP();
void enableNAT();
void configureIP();
bool setupNAT();
void checkAndRestartNetworking();

extern TaskHandle_t wifiTaskHandle;
extern WifiConfig wifiConfig;
extern esp_netif_t* ap_netif;
extern esp_netif_t* sta_netif;

#endif 