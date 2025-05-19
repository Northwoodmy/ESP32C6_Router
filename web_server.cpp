#include "web_server.h"
#include "wifi_manager.h"
#include "config_manager.h"
#include <esp_system.h>
#include <esp_heap_caps.h>
#include <esp_wifi.h>

TaskHandle_t webServerTaskHandle = NULL;
WebServer server(80);

void handleRoot();
void handleSave();
void handleSystemInfo();
void handleTaskInfo();
void handleWiFiInfo();
void handleClientInfo();

// HTML页面
const char* html_page = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta charset='utf-8'>
    <title>ESP32-C6 路由器</title>
    <style>
        body { font-family: Arial, sans-serif; margin: 20px; background-color: #f0f0f0; }
        .container { max-width: 800px; margin: 0 auto; background-color: white; padding: 20px; border-radius: 8px; box-shadow: 0 2px 4px rgba(0,0,0,0.1); }
        h2 { color: #333; text-align: center; margin-bottom: 20px; }
        input { width: 100%; padding: 10px; margin: 10px 0; border: 1px solid #ddd; border-radius: 4px; box-sizing: border-box; }
        button { background-color: #4CAF50; color: white; padding: 12px; border: none; width: 100%; border-radius: 4px; cursor: pointer; font-size: 16px; }
        button:hover { background-color: #45a049; }
        .status { margin-top: 20px; padding: 10px; background-color: #f8f8f8; border-radius: 4px; }
        .tabs { display: flex; margin-bottom: 20px; }
        .tab { padding: 10px 20px; cursor: pointer; border: 1px solid #ddd; background: #f8f8f8; }
        .tab.active { background: #4CAF50; color: white; border-color: #4CAF50; }
        .tab-content { display: none; }
        .tab-content.active { display: block; }
        .info-grid { display: grid; grid-template-columns: 1fr 1fr; gap: 10px; margin: 10px 0; }
        .info-item { padding: 10px; background: #f8f8f8; border-radius: 4px; }
        .task-list { margin-top: 10px; }
        .task-item { padding: 10px; background: #f8f8f8; margin: 5px 0; border-radius: 4px; }
        #refreshBtn { background-color: #2196F3; margin-top: 10px; }
        .wifi-info { 
            background: #f8f8f8; 
            padding: 15px; 
            border-radius: 4px; 
            margin-bottom: 20px; 
        }
        .wifi-info-item { 
            margin: 8px 0; 
            line-height: 1.5; 
        }
        .connected { color: #4CAF50; }
        .disconnected { color: #f44336; }
        .client-list {
            margin-top: 10px;
        }
        .client-item {
            background: #f8f8f8;
            padding: 15px;
            margin-bottom: 10px;
            border-radius: 4px;
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));
            gap: 10px;
        }
        .client-info {
            line-height: 1.5;
        }
    </style>
</head>
<body>
    <div class='container'>
        <h2>ESP32-C6 路由器</h2>
        <div class="tabs">
            <div class="tab active" onclick="showTab('wifiConfig')">WiFi 配置</div>
            <div class="tab" onclick="showTab('systemInfo')">系统信息</div>
            <div class="tab" onclick="showTab('taskInfo')">任务信息</div>
            <div class="tab" onclick="showTab('clientInfo')">已连接设备</div>
        </div>
        
        <div id="wifiConfig" class="tab-content active">
            <div class="wifi-info">
                <div class="wifi-info-item">热点名称：<span id="apName">-</span></div>
                <div class="wifi-info-item">热点IP：<span id="apIP">-</span></div>
                <div class="wifi-info-item">连接状态：<span id="wifiStatus">-</span></div>
                <div class="wifi-info-item">连接的WiFi：<span id="connectedWifi">-</span></div>
                <div class="wifi-info-item">设备IP：<span id="deviceIP">-</span></div>
            </div>
            <form action='/save' method='POST'>
                <input type='text' name='ssid' placeholder='WiFi 名称' required>
                <input type='password' name='password' placeholder='WiFi 密码' required>
                <button type='submit'>保存配置</button>
            </form>
        </div>
        
        <div id="systemInfo" class="tab-content">
            <div class="info-grid">
                <div class="info-item">CPU 使用率: <span id="cpuUsage">-</span></div>
                <div class="info-item">可用内存: <span id="freeHeap">-</span></div>
                <div class="info-item">运行时间: <span id="uptime">-</span></div>
                <div class="info-item">任务数量: <span id="taskCount">-</span></div>
            </div>
            <button id="refreshBtn" onclick="refreshInfo()">刷新</button>
        </div>
        
        <div id="taskInfo" class="tab-content">
            <div id="taskList" class="task-list"></div>
            <button id="refreshBtn" onclick="refreshInfo()">刷新</button>
        </div>

        <div id="clientInfo" class="tab-content">
            <div id="clientList" class="client-list">
                <div class="client-item">
                    <div class="client-info">正在加载设备信息...</div>
                </div>
            </div>
            <button id="refreshClientBtn" onclick="refreshClientInfo()" style="background-color: #2196F3; margin-top: 10px;">刷新</button>
        </div>
    </div>
    <script>
        function showTab(tabName) {
            var contents = document.getElementsByClassName('tab-content');
            for(var i = 0; i < contents.length; i++) {
                contents[i].classList.remove('active');
            }
            var tabs = document.getElementsByClassName('tab');
            for(var i = 0; i < tabs.length; i++) {
                tabs[i].classList.remove('active');
            }
            document.getElementById(tabName).classList.add('active');
            document.querySelector("[onclick*='" + tabName + "']").classList.add('active');
        }
        
        function updateWiFiInfo() {
            fetch('/wifi-info')
                .then(response => response.json())
                .then(data => {
                    document.getElementById('apName').textContent = data.ap_ssid;
                    document.getElementById('apIP').textContent = data.ap_ip;
                    document.getElementById('wifiStatus').textContent = data.is_connected ? '已连接' : '未连接';
                    document.getElementById('wifiStatus').className = data.is_connected ? 'connected' : 'disconnected';
                    document.getElementById('connectedWifi').textContent = data.sta_ssid || '-';
                    document.getElementById('deviceIP').textContent = data.sta_ip || '-';
                });
        }
        
        function refreshClientInfo() {
            fetch('/client-info')
                .then(response => response.json())
                .then(data => {
                    const clientList = document.getElementById('clientList');
                    clientList.innerHTML = '';
                    
                    if (data.clients.length === 0) {
                        clientList.innerHTML = '<div class="client-item"><div class="client-info">当前没有设备连接</div></div>';
                        return;
                    }
                    
                    data.clients.forEach(client => {
                        const clientItem = document.createElement('div');
                        clientItem.className = 'client-item';
                        clientItem.innerHTML = `
                            <div class="client-info">
                                <div>设备名称：${client.hostname}</div>
                                <div>IP地址：${client.ip}</div>
                                <div>MAC地址：${client.mac}</div>
                                <div>信号强度：${client.rssi}</div>
                            </div>
                        `;
                        clientList.appendChild(clientItem);
                    });
                });
        }
        
        function refreshInfo() {
            updateWiFiInfo();
            refreshClientInfo();
            fetch('/system-info')
                .then(function(response) { return response.json(); })
                .then(function(data) {
                    document.getElementById('cpuUsage').textContent = data.cpuUsage + '%';
                    document.getElementById('freeHeap').textContent = data.freeHeap + ' 字节';
                    document.getElementById('uptime').textContent = data.uptime + ' 秒';
                    document.getElementById('taskCount').textContent = data.taskCount;
                });
                
            fetch('/task-info')
                .then(function(response) { return response.json(); })
                .then(function(data) {
                    var taskList = document.getElementById('taskList');
                    taskList.innerHTML = "";
                    for(var i = 0; i < data.tasks.length; i++) {
                        var task = data.tasks[i];
                        var html = '<div class="task-item">';
                        html += '<div>任务名称: ' + task.name + '</div>';
                        html += '<div>优先级: ' + task.priority + '</div>';
                        html += '<div>剩余堆栈: ' + task.stackHigh + ' 字节</div>';
                        html += '<div>运行核心: ' + task.core + '</div>';
                        html += '</div>';
                        taskList.innerHTML += html;
                    }
                });
        }
        
        window.onload = function() {
            refreshInfo();
            setInterval(refreshInfo, 5000);
        }
    </script>
</body>
</html>
)rawliteral";

void handleRoot() {
    server.send(200, "text/html", html_page);
}

void handleSystemInfo() {
    // 获取系统信息
    uint32_t freeHeap = esp_get_free_heap_size();
    uint32_t uptime = esp_timer_get_time() / 1000000; // 转换为秒
    
    // 获取CPU使用率（简单估算）
    static uint32_t lastTotalTime = 0;
    static uint32_t lastIdleTime = 0;
    uint32_t totalTime = 0;
    uint32_t idleTime = 0;
    
    // 获取所有任务的运行时间
    UBaseType_t taskCount = uxTaskGetNumberOfTasks();
    TaskStatus_t *taskStatusArray = (TaskStatus_t *)malloc(taskCount * sizeof(TaskStatus_t));
    if (taskStatusArray != NULL) {
        taskCount = uxTaskGetSystemState(taskStatusArray, taskCount, &totalTime);
        for (UBaseType_t i = 0; i < taskCount; i++) {
            if (strcmp(taskStatusArray[i].pcTaskName, "IDLE") == 0) {
                idleTime = taskStatusArray[i].ulRunTimeCounter;
                break;
            }
        }
        free(taskStatusArray);
    }
    
    // 计算CPU使用率
    float cpuUsage = 0;
    if (lastTotalTime > 0) {
        uint32_t totalDelta = totalTime - lastTotalTime;
        uint32_t idleDelta = idleTime - lastIdleTime;
        cpuUsage = (1.0f - ((float)idleDelta / totalDelta)) * 100;
    }
    lastTotalTime = totalTime;
    lastIdleTime = idleTime;
    
    // 构建JSON响应
    String json = "{";
    json += "\"cpuUsage\":" + String(cpuUsage, 1) + ",";
    json += "\"freeHeap\":" + String(freeHeap) + ",";
    json += "\"uptime\":" + String(uptime) + ",";
    json += "\"taskCount\":" + String(taskCount);
    json += "}";
    
    server.send(200, "application/json", json);
}

void handleTaskInfo() {
    UBaseType_t taskCount = uxTaskGetNumberOfTasks();
    TaskStatus_t *taskStatusArray = (TaskStatus_t *)malloc(taskCount * sizeof(TaskStatus_t));
    uint32_t totalRunTime;
    
    String json = "{\"tasks\":[";
    
    if (taskStatusArray != NULL) {
        taskCount = uxTaskGetSystemState(taskStatusArray, taskCount, &totalRunTime);
        
        for (UBaseType_t i = 0; i < taskCount; i++) {
            if (i > 0) json += ",";
            json += "{";
            json += "\"name\":\"" + String(taskStatusArray[i].pcTaskName) + "\",";
            json += "\"priority\":" + String(taskStatusArray[i].uxCurrentPriority) + ",";
            json += "\"stackHigh\":" + String(taskStatusArray[i].usStackHighWaterMark) + ",";
            json += "\"core\":" + String(taskStatusArray[i].xCoreID);
            json += "}";
        }
        
        free(taskStatusArray);
    }
    
    json += "]}";
    server.send(200, "application/json", json);
}

void handleSave() {
    String ssid = server.arg("ssid");
    String password = server.arg("password");
    
    if (ssid.length() > 0) {
        saveWiFiConfig(ssid.c_str(), password.c_str());
        server.send(200, "text/plain", "配置已保存，设备将重启...");
        printf("收到新的WiFi配置: SSID=%s\n", ssid.c_str());
        delay(2000);
        ESP.restart();
    } else {
        server.send(400, "text/plain", "无效的配置");
        printf("收到无效的WiFi配置\n");
    }
}

void handleWiFiInfo() {
    String json = "{";
    json += "\"ap_ssid\":\"ESP32-C6-Router\",";
    json += "\"ap_ip\":\"" + WiFi.softAPIP().toString() + "\",";
    json += "\"is_connected\":" + String(WiFi.status() == WL_CONNECTED ? "true" : "false") + ",";
    json += "\"sta_ssid\":\"" + String(WiFi.SSID()) + "\",";
    json += "\"sta_ip\":\"" + WiFi.localIP().toString() + "\"";
    json += "}";
    server.send(200, "application/json", json);
}

void handleClientInfo() {
    wifi_sta_list_t sta_list;
    wifi_sta_info_t sta_info[8];
    memset(&sta_list, 0, sizeof(wifi_sta_list_t));
    esp_wifi_ap_get_sta_list(&sta_list);
    
    String json = "{\"clients\":[";
    
    for(int i = 0; i < sta_list.num; i++) {
        if(i > 0) json += ",";
        json += "{";
        
        // MAC地址
        char mac[18];
        sprintf(mac, "%02X:%02X:%02X:%02X:%02X:%02X",
                sta_list.sta[i].mac[0], sta_list.sta[i].mac[1],
                sta_list.sta[i].mac[2], sta_list.sta[i].mac[3],
                sta_list.sta[i].mac[4], sta_list.sta[i].mac[5]);
        
        // 获取IP地址
        esp_netif_ip_info_t ip_info;
        esp_netif_get_ip_info(ap_netif, &ip_info);
        
        uint8_t last_octet = 100 + i; // 简单分配IP地址
        char ip[16];
        sprintf(ip, "%d.%d.%d.%d",
                ip_info.ip.addr & 0xff,
                (ip_info.ip.addr >> 8) & 0xff,
                (ip_info.ip.addr >> 16) & 0xff,
                last_octet);
        
        json += "\"mac\":\"" + String(mac) + "\",";
        json += "\"ip\":\"" + String(ip) + "\",";
        json += "\"hostname\":\"" + String("设备 ") + String(i + 1) + "\",";
        json += "\"rssi\":\"" + String(sta_list.sta[i].rssi) + " dBm\"";
        json += "}";
    }
    
    json += "]}";
    server.send(200, "application/json", json);
}

void initWebServer() {
    server.on("/", handleRoot);
    server.on("/save", HTTP_POST, handleSave);
    server.on("/system-info", handleSystemInfo);
    server.on("/task-info", handleTaskInfo);
    server.on("/wifi-info", handleWiFiInfo);
    server.on("/client-info", handleClientInfo);
    printf("Web server routes configured\n");
}

void startWebServerTask(void* parameter) {
    server.begin();
    printf("Web server started\n");
    
    while(1) {
        server.handleClient();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
} 