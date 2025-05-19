#include "web_server.h"
#include "wifi_manager.h"
#include "config_manager.h"
<<<<<<< HEAD
#include <esp_system.h>
#include <esp_heap_caps.h>
=======
>>>>>>> ccfc7efee610ce0d4065fa4704320b3c64f0ff09

TaskHandle_t webServerTaskHandle = NULL;
WebServer server(80);

<<<<<<< HEAD
void handleRoot();
void handleSave();
void handleSystemInfo();
void handleTaskInfo();

// HTML页面
const char* html_page = R"rawliteral(
=======
// HTML页面
const char* html_page = R"(
>>>>>>> ccfc7efee610ce0d4065fa4704320b3c64f0ff09
<!DOCTYPE html>
<html>
<head>
    <meta charset='utf-8'>
<<<<<<< HEAD
    <title>ESP32-C6 Router</title>
    <style>
        body { font-family: Arial, sans-serif; margin: 20px; background-color: #f0f0f0; }
        .container { max-width: 800px; margin: 0 auto; background-color: white; padding: 20px; border-radius: 8px; box-shadow: 0 2px 4px rgba(0,0,0,0.1); }
=======
    <title>ESP32-C6 WiFi配置</title>
    <style>
        body { font-family: Arial, sans-serif; margin: 20px; background-color: #f0f0f0; }
        .container { max-width: 400px; margin: 0 auto; background-color: white; padding: 20px; border-radius: 8px; box-shadow: 0 2px 4px rgba(0,0,0,0.1); }
>>>>>>> ccfc7efee610ce0d4065fa4704320b3c64f0ff09
        h2 { color: #333; text-align: center; margin-bottom: 20px; }
        input { width: 100%; padding: 10px; margin: 10px 0; border: 1px solid #ddd; border-radius: 4px; box-sizing: border-box; }
        button { background-color: #4CAF50; color: white; padding: 12px; border: none; width: 100%; border-radius: 4px; cursor: pointer; font-size: 16px; }
        button:hover { background-color: #45a049; }
        .status { margin-top: 20px; padding: 10px; background-color: #f8f8f8; border-radius: 4px; }
<<<<<<< HEAD
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
=======
>>>>>>> ccfc7efee610ce0d4065fa4704320b3c64f0ff09
    </style>
</head>
<body>
    <div class='container'>
<<<<<<< HEAD
        <h2>ESP32-C6 Router</h2>
        <div class="tabs">
            <div class="tab active" onclick="showTab('wifiConfig')">WiFi Config</div>
            <div class="tab" onclick="showTab('systemInfo')">System Info</div>
            <div class="tab" onclick="showTab('taskInfo')">Task Info</div>
        </div>
        
        <div id="wifiConfig" class="tab-content active">
            <form action='/save' method='POST'>
                <input type='text' name='ssid' placeholder='WiFi SSID' required>
                <input type='password' name='password' placeholder='WiFi Password' required>
                <button type='submit'>Save Config</button>
            </form>
        </div>
        
        <div id="systemInfo" class="tab-content">
            <div class="info-grid">
                <div class="info-item">CPU Usage: <span id="cpuUsage">-</span></div>
                <div class="info-item">Free Memory: <span id="freeHeap">-</span></div>
                <div class="info-item">Uptime: <span id="uptime">-</span></div>
                <div class="info-item">Task Count: <span id="taskCount">-</span></div>
            </div>
            <button id="refreshBtn" onclick="refreshInfo()">Refresh</button>
        </div>
        
        <div id="taskInfo" class="tab-content">
            <div id="taskList" class="task-list"></div>
            <button id="refreshBtn" onclick="refreshInfo()">Refresh</button>
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
        
        function refreshInfo() {
            fetch('/system-info')
                .then(function(response) { return response.json(); })
                .then(function(data) {
                    document.getElementById('cpuUsage').textContent = data.cpuUsage + '%';
                    document.getElementById('freeHeap').textContent = data.freeHeap + ' bytes';
                    document.getElementById('uptime').textContent = data.uptime + ' sec';
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
                        html += '<div>Task Name: ' + task.name + '</div>';
                        html += '<div>Priority: ' + task.priority + '</div>';
                        html += '<div>Stack Free: ' + task.stackHigh + ' bytes</div>';
                        html += '<div>Core: ' + task.core + '</div>';
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
=======
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
>>>>>>> ccfc7efee610ce0d4065fa4704320b3c64f0ff09

void handleRoot() {
    server.send(200, "text/html", html_page);
}

<<<<<<< HEAD
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

=======
>>>>>>> ccfc7efee610ce0d4065fa4704320b3c64f0ff09
void handleSave() {
    String ssid = server.arg("ssid");
    String password = server.arg("password");
    
    if (ssid.length() > 0) {
        saveWiFiConfig(ssid.c_str(), password.c_str());
<<<<<<< HEAD
        server.send(200, "text/plain", "Configuration saved, device will restart...");
        printf("Received new WiFi config: SSID=%s\n", ssid.c_str());
        delay(2000);
        ESP.restart();
    } else {
        server.send(400, "text/plain", "Invalid configuration");
        printf("Received invalid WiFi config\n");
=======
        server.send(200, "text/plain", "配置已保存，设备将重启...");
        printf("收到新的WiFi配置：SSID=%s\n", ssid.c_str());
        delay(2000);
        ESP.restart();
    } else {
        server.send(400, "text/plain", "配置无效");
        printf("收到无效的WiFi配置\n");
>>>>>>> ccfc7efee610ce0d4065fa4704320b3c64f0ff09
    }
}

void initWebServer() {
    server.on("/", handleRoot);
    server.on("/save", HTTP_POST, handleSave);
<<<<<<< HEAD
    server.on("/system-info", handleSystemInfo);
    server.on("/task-info", handleTaskInfo);
    printf("Web server routes configured\n");
=======
    printf("Web服务器路由配置完成\n");
>>>>>>> ccfc7efee610ce0d4065fa4704320b3c64f0ff09
}

void startWebServerTask(void* parameter) {
    server.begin();
<<<<<<< HEAD
    printf("Web server started\n");
=======
    printf("Web服务器已启动\n");
>>>>>>> ccfc7efee610ce0d4065fa4704320b3c64f0ff09
    
    while(1) {
        server.handleClient();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
} 