#include "http_client.h"
#include "wifi_manager.h"  // 添加wifi_manager.h以访问AP接口相关函数

TaskHandle_t httpClientTaskHandle = NULL;
HTTPClient http;
String lastMetricsData = "";
unsigned long lastRequestTime = 0;
const unsigned long REQUEST_INTERVAL = 500; // 500ms间隔

String getMetricsData() {
    return lastMetricsData;
}

void fetchMetricsData() {
    if (!serverConfig.configured || !WiFi.softAPgetStationNum()) {  // 检查是否有设备连接到AP
        return;
    }

    unsigned long currentTime = millis();
    if (currentTime - lastRequestTime < REQUEST_INTERVAL) {
        return;
    }
    lastRequestTime = currentTime;

    // 配置HTTP客户端使用AP接口
    WiFiClient wifiClient;
    http.begin(wifiClient, "http://" + String(serverConfig.ip) + ":" + String(serverConfig.port) + String(serverConfig.path));
    
    // 设置超时时间
    http.setTimeout(2000);  // 2秒超时
    
    printf("正在通过AP接口访问服务器: %s\n", serverConfig.ip);
    int httpCode = http.GET();

    if (httpCode == HTTP_CODE_OK) {
        lastMetricsData = http.getString();
        printf("成功获取metrics数据，长度：%d\n", lastMetricsData.length());
    } else {
        printf("获取metrics数据失败，错误码：%d，服务器：%s\n", httpCode, serverConfig.ip);
        lastMetricsData = "";
    }
    
    http.end();
}

void startHttpClientTask(void* parameter) {
    printf("HTTP客户端任务启动\n");
    
    while(1) {
        fetchMetricsData();
        vTaskDelay(pdMS_TO_TICKS(100)); // 给其他任务执行的时间
    }
}

void initHttpClient() {
    xTaskCreatePinnedToCore(
        startHttpClientTask,
        "HttpClientTask",
        4096,
        NULL,
        1,
        &httpClientTaskHandle,
        0
    );
    printf("HTTP客户端初始化完成\n");
} 