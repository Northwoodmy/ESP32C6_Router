#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include <nvs_flash.h>
#include <Preferences.h>
#include <stdio.h>

// 函数声明
void initConfigManager();
void saveWiFiConfig(const char* ssid, const char* password);
bool loadWiFiConfig();

#endif 