#include "wifi_manager.h"
#include "web_server.h"
#include "config_manager.h"
#include "http_client.h"

void setup() {
    // 初始化串口
    Serial.begin(115200);
    
    // 打印启动信息
    Serial.println("\n\n初始化开始...");
    
    // 先初始化配置管理器
    initConfigManager();
    
    // 初始化WiFi管理器
    initWiFiManager();
    
    // 加载WiFi配置
    loadWiFiConfig();
    
    // 初始化并启动Web服务器
    initWebServer();
    
    // 创建任务，确保WiFi任务优先级高于Web服务器任务
    xTaskCreate(
        startWiFiTask,
        "WiFiTask",
        8192,
        NULL,
        2,  // 更高的优先级
        &wifiTaskHandle
    );
    
    // 创建Web服务器任务
    xTaskCreate(
        startWebServerTask,
        "WebServerTask",
        8192,
        NULL,
        1,
        &webServerTaskHandle
    );
    
    Serial.println("所有任务已启动");
}

void loop() {
    // 主循环为空，所有功能由FreeRTOS任务处理
    vTaskDelay(pdMS_TO_TICKS(1000));
} 