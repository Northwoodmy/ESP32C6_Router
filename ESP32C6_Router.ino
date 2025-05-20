#include "wifi_manager.h"
#include "web_server.h"
#include "config_manager.h"
#include "http_client.h"

void setup() {
    // 初始化各个模块
    initConfigManager();
    initWiFiManager();
    initWebServer();
    //initHttpClient();
    
    // 加载WiFi配置
    loadWiFiConfig();
    
    // 创建任务
    xTaskCreate(
        startWiFiTask,
        "WiFiTask",
        8192,
        NULL,
        1,
        &wifiTaskHandle
    );
    
    xTaskCreate(
        startWebServerTask,
        "WebServerTask",
        8192,
        NULL,
        1,
        &webServerTaskHandle
    );
}

void loop() {
    // 主循环为空，所有功能由FreeRTOS任务处理
    vTaskDelay(pdMS_TO_TICKS(1000));
} 