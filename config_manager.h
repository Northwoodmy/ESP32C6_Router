#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include <nvs_flash.h>
#include <Preferences.h>
#include <stdio.h>

// 服务器配置结构体
struct ServerConfig {
    char ip[32];        // 服务器IP地址
    uint16_t port;      // 服务器端口
    char path[64];      // 服务器路径
    bool configured;    // 是否已配置
};

// 函数声明
void initConfigManager();
void saveWiFiConfig(const char* ssid, const char* password);
bool loadWiFiConfig();
void saveServerConfig(const char* ip, uint16_t port, const char* path);
bool loadServerConfig();

// 全局配置变量
extern ServerConfig serverConfig;

#endif 