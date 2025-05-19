#include "config_manager.h"
#include "wifi_manager.h"

Preferences preferences;
ServerConfig serverConfig;

void initConfigManager() {
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        nvs_flash_erase();
        nvs_flash_init();
    }
    // 初始化服务器配置
    memset(&serverConfig, 0, sizeof(ServerConfig));
    loadServerConfig();
    printf("NVS初始化完成\n");
}

void saveWiFiConfig(const char* ssid, const char* password) {
    preferences.begin("wifi", false);
    preferences.putString("ssid", ssid);
    preferences.putString("password", password);
    preferences.putBool("configured", true);
    preferences.end();
    
    strncpy(wifiConfig.ssid, ssid, sizeof(wifiConfig.ssid));
    strncpy(wifiConfig.password, password, sizeof(wifiConfig.password));
    wifiConfig.configured = true;
    
    printf("WiFi配置已保存到NVS\n");
}

bool loadWiFiConfig() {
    preferences.begin("wifi", true);
    String saved_ssid = preferences.getString("ssid", "");
    String saved_password = preferences.getString("password", "");
    wifiConfig.configured = preferences.getBool("configured", false);
    preferences.end();
    
    if (wifiConfig.configured) {
        strncpy(wifiConfig.ssid, saved_ssid.c_str(), sizeof(wifiConfig.ssid));
        strncpy(wifiConfig.password, saved_password.c_str(), sizeof(wifiConfig.password));
        printf("已从NVS加载WiFi配置\n");
        return true;
    }
    printf("未找到已保存的WiFi配置\n");
    return false;
}

void saveServerConfig(const char* ip, uint16_t port, const char* path) {
    preferences.begin("server", false);
    preferences.putString("ip", ip);
    preferences.putUInt("port", port);
    preferences.putString("path", path);
    preferences.putBool("configured", true);
    preferences.end();
    
    strncpy(serverConfig.ip, ip, sizeof(serverConfig.ip));
    serverConfig.port = port;
    strncpy(serverConfig.path, path, sizeof(serverConfig.path));
    serverConfig.configured = true;
    
    printf("服务器配置已保存到NVS\n");
}

bool loadServerConfig() {
    preferences.begin("server", true);
    String saved_ip = preferences.getString("ip", "");
    uint16_t saved_port = preferences.getUInt("port", 80);
    String saved_path = preferences.getString("path", "");
    serverConfig.configured = preferences.getBool("configured", false);
    preferences.end();
    
    if (serverConfig.configured) {
        strncpy(serverConfig.ip, saved_ip.c_str(), sizeof(serverConfig.ip));
        serverConfig.port = saved_port;
        strncpy(serverConfig.path, saved_path.c_str(), sizeof(serverConfig.path));
        printf("已从NVS加载服务器配置\n");
        return true;
    }
    printf("未找到已保存的服务器配置\n");
    return false;
} 