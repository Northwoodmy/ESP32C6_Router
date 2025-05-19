#include "web_server.h"
#include "wifi_manager.h"
#include "config_manager.h"

TaskHandle_t webServerTaskHandle = NULL;
WebServer server(80);

// HTML页面
const char* html_page = R"(
<!DOCTYPE html>
<html>
<head>
    <meta charset='utf-8'>
    <title>ESP32-C6 WiFi配置</title>
    <style>
        body { font-family: Arial, sans-serif; margin: 20px; background-color: #f0f0f0; }
        .container { max-width: 400px; margin: 0 auto; background-color: white; padding: 20px; border-radius: 8px; box-shadow: 0 2px 4px rgba(0,0,0,0.1); }
        h2 { color: #333; text-align: center; margin-bottom: 20px; }
        input { width: 100%; padding: 10px; margin: 10px 0; border: 1px solid #ddd; border-radius: 4px; box-sizing: border-box; }
        button { background-color: #4CAF50; color: white; padding: 12px; border: none; width: 100%; border-radius: 4px; cursor: pointer; font-size: 16px; }
        button:hover { background-color: #45a049; }
        .status { margin-top: 20px; padding: 10px; background-color: #f8f8f8; border-radius: 4px; }
    </style>
</head>
<body>
    <div class='container'>
        <h2>ESP32-C6 WiFi配置</h2>
        <form action='/save' method='POST'>
            <input type='text' name='ssid' placeholder='WiFi名称' required>
            <input type='password' name='password' placeholder='WiFi密码' required>
            <button type='submit'>保存配置</button>
        </form>
        <div class='status' id='status'></div>
    </div>
</body>
</html>
)";

void handleRoot() {
    server.send(200, "text/html", html_page);
}

void handleSave() {
    String ssid = server.arg("ssid");
    String password = server.arg("password");
    
    if (ssid.length() > 0) {
        saveWiFiConfig(ssid.c_str(), password.c_str());
        server.send(200, "text/plain", "配置已保存，设备将重启...");
        printf("收到新的WiFi配置：SSID=%s\n", ssid.c_str());
        delay(2000);
        ESP.restart();
    } else {
        server.send(400, "text/plain", "配置无效");
        printf("收到无效的WiFi配置\n");
    }
}

void initWebServer() {
    server.on("/", handleRoot);
    server.on("/save", HTTP_POST, handleSave);
    printf("Web服务器路由配置完成\n");
}

void startWebServerTask(void* parameter) {
    server.begin();
    printf("Web服务器已启动\n");
    
    while(1) {
        server.handleClient();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
} 