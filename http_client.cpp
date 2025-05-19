#include "http_client.h"

TaskHandle_t httpClientTaskHandle = NULL;
HTTPClient http;
String lastMetricsData = "";
unsigned long lastRequestTime = 0;
const unsigned long REQUEST_INTERVAL = 500; // 500ms间隔

String getMetricsData() {
    return lastMetricsData;
}

void fetchMetricsData() {
    if (!serverConfig.configured || !WiFi.isConnected()) {
        return;
    }

    unsigned long currentTime = millis();
    if (currentTime - lastRequestTime < REQUEST_INTERVAL) {
        return;
    }
    lastRequestTime = currentTime;

    String url = "http://" + String(serverConfig.ip) + ":" + String(serverConfig.port) + String(serverConfig.path);
    
    http.begin(url);
    int httpCode = http.GET();

    if (httpCode == HTTP_CODE_OK) {
        lastMetricsData = http.getString();
        printf("成功获取metrics数据，长度：%d\n", lastMetricsData.length());
    } else {
        printf("获取metrics数据失败，错误码：%d\n", httpCode);
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