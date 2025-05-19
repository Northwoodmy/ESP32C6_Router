#include "config_manager.h"
#include "wifi_manager.h"

Preferences preferences;

void initConfigManager() {
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        nvs_flash_erase();
        nvs_flash_init();
    }
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