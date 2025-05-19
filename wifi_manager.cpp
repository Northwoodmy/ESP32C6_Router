#include "wifi_manager.h"

TaskHandle_t wifiTaskHandle = NULL;
WifiConfig wifiConfig;

void initWiFiManager() {
    wifiConfig.configured = false;
}

void startWiFiTask(void* parameter) {
    while(1) {
        if (wifiConfig.configured) {
            if (WiFi.status() != WL_CONNECTED) {
                WiFi.begin(wifiConfig.ssid, wifiConfig.password);
                printf("正在连接WiFi...\n");
                int attempts = 0;
                while (WiFi.status() != WL_CONNECTED && attempts < 20) {
                    delay(500);
                    printf(".");
                    fflush(stdout);
                    attempts++;
                }
                if (WiFi.status() == WL_CONNECTED) {
                    printf("\nWiFi连接成功！\n");
                    printf("IP地址: %s\n", WiFi.localIP().toString().c_str());
                } else {
                    printf("\nWiFi连接失败，请检查配置\n");
                }
            }
        } else {
            setupAP();
            vTaskDelete(NULL);
        }
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

bool isWiFiConnected() {
    return WiFi.status() == WL_CONNECTED;
}

IPAddress getLocalIP() {
    return WiFi.localIP();
}

IPAddress getAPIP() {
    return WiFi.softAPIP();
}

void setupAP() {
    WiFi.mode(WIFI_AP);
    WiFi.softAP("ESP32-C6-Config", "12345678");
    printf("AP模式已启动\n");
    printf("AP IP地址: %s\n", WiFi.softAPIP().toString().c_str());
} 