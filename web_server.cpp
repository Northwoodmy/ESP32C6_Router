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
void handleServerConfig();
void handleGetServerConfig();

// HTML页面
const char* html_page = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta charset='utf-8'>
    <title>ESP32-C6 智能路由器</title>
    <style>
        body { font-family: Arial, sans-serif; margin: 20px; background-color: #f0f0f0; }
        .container { max-width: 800px; margin: 0 auto; background-color: white; padding: 20px; border-radius: 8px; box-shadow: 0 2px 4px rgba(0,0,0,0.1); }
        h2 { color: #333; text-align: center; margin-bottom: 20px; }
        input { width: 100%; padding: 10px; margin: 10px 0; border: 1px solid #ddd; border-radius: 4px; box-sizing: border-box; }
        button { background-color: #4CAF50; color: white; padding: 12px; border: none; width: 100%; border-radius: 4px; cursor: pointer; font-size: 16px; }
        button:hover { background-color: #45a049; }
        .status { margin-top: 20px; padding: 10px; background-color: #f8f8f8; border-radius: 4px; }
        .tabs { display: flex; margin-bottom: 20px; border-radius: 4px; overflow: hidden; }
        .tab { padding: 12px 24px; cursor: pointer; border: 1px solid #ddd; background: #f8f8f8; flex: 1; text-align: center; transition: all 0.3s; }
        .tab.active { background: #4CAF50; color: white; border-color: #4CAF50; }
        .tab-content { display: none; animation: fadeIn 0.3s; }
        .tab-content.active { display: block; }
        .info-grid { display: grid; grid-template-columns: 1fr 1fr; gap: 15px; margin: 15px 0; }
        .info-item { 
            padding: 15px; 
            background: #f8f8f8; 
            border-radius: 4px; 
            box-shadow: 0 1px 3px rgba(0,0,0,0.1);
        }
        .info-item .title {
            font-size: 14px;
            color: #666;
            margin-bottom: 10px;
        }
        .info-item .value {
            font-size: 18px;
            font-weight: bold;
            color: #333;
            margin-bottom: 10px;
        }
        .progress-bar {
            width: 100%;
            height: 8px;
            background: #e0e0e0;
            border-radius: 4px;
            overflow: hidden;
            margin-top: 8px;
        }
        .progress-bar .bar {
            height: 100%;
            background: linear-gradient(90deg, #4CAF50, #45a049);
            transition: width 0.3s ease;
        }
        .progress-bar.memory .bar {
            background: linear-gradient(90deg, #2196F3, #1976D2);
        }
        .progress-bar.uptime .bar {
            background: linear-gradient(90deg, #FF9800, #F57C00);
        }
        .progress-bar.tasks .bar {
            background: linear-gradient(90deg, #9C27B0, #7B1FA2);
        }
        .task-list { margin-top: 15px; }
        .task-item { padding: 15px; background: #f8f8f8; margin: 8px 0; border-radius: 4px; box-shadow: 0 1px 3px rgba(0,0,0,0.1); }
        #refreshBtn { background-color: #2196F3; margin-top: 15px; }
        .wifi-info { 
            background: #f8f8f8; 
            padding: 20px; 
            border-radius: 4px; 
            margin-bottom: 25px;
            box-shadow: 0 1px 3px rgba(0,0,0,0.1);
        }
        .wifi-info-item { 
            margin: 10px 0; 
            line-height: 1.6;
            display: flex;
            justify-content: space-between;
        }
        .connected { 
            color: #4CAF50; 
            font-weight: bold;
            padding: 3px 8px;
            background: #e8f5e9;
            border-radius: 4px;
        }
        .disconnected { 
            color: #f44336;
            font-weight: bold;
            padding: 3px 8px;
            background: #ffebee;
            border-radius: 4px;
        }
        .client-list {
            margin-top: 15px;
        }
        .client-item {
            background: #f8f8f8;
            padding: 20px;
            margin-bottom: 15px;
            border-radius: 4px;
            box-shadow: 0 1px 3px rgba(0,0,0,0.1);
        }
        .client-info {
            line-height: 1.6;
        }
        .client-info div {
            margin: 5px 0;
            display: flex;
            justify-content: space-between;
        }
        .signal-strength {
            display: inline-block;
            width: 60px;
            height: 8px;
            background: #ddd;
            border-radius: 4px;
            overflow: hidden;
        }
        .signal-bar {
            height: 100%;
            background: #4CAF50;
            transition: width 0.3s;
        }
        @keyframes fadeIn {
            from { opacity: 0; }
            to { opacity: 1; }
        }
        .refresh-timer {
            text-align: center;
            color: #666;
            margin-top: 10px;
            font-size: 0.9em;
        }
        .server-config {
            background: #f8f8f8;
            padding: 20px;
            border-radius: 4px;
            margin-bottom: 25px;
            box-shadow: 0 1px 3px rgba(0,0,0,0.1);
        }
        .server-info {
            margin: 10px 0;
            line-height: 1.6;
        }
        .action-button {
            background-color: #2196F3;
            color: white;
            padding: 5px 10px;
            border: none;
            border-radius: 4px;
            cursor: pointer;
            font-size: 14px;
            margin-top: 10px;
        }
        .action-button:hover {
            background-color: #1976D2;
        }
        .success-message {
            color: #4CAF50;
            margin-top: 10px;
            display: none;
        }
    </style>
</head>
<body>
    <div class='container'>
        <h2>ESP32-C6 智能路由器</h2>
        <div class="tabs">
            <div class="tab active" onclick="showTab('wifiConfig')">WiFi 配置</div>
            <div class="tab" onclick="showTab('systemInfo')">系统状态</div>
            <div class="tab" onclick="showTab('taskInfo')">任务管理</div>
            <div class="tab" onclick="showTab('clientInfo')">已连接设备</div>
            <div class="tab" onclick="showTab('serverConfig')">服务器配置</div>
        </div>
        
        <div id="wifiConfig" class="tab-content active">
            <div class="wifi-info">
                <div class="wifi-info-item">
                    <span>热点名称：</span>
                    <span id="apName">-</span>
                </div>
                <div class="wifi-info-item">
                    <span>热点IP地址：</span>
                    <span id="apIP">-</span>
                </div>
                <div class="wifi-info-item">
                    <span>连接状态：</span>
                    <span id="wifiStatus">-</span>
                </div>
                <div class="wifi-info-item">
                    <span>已连接WiFi：</span>
                    <span id="connectedWifi">-</span>
                </div>
                <div class="wifi-info-item">
                    <span>设备IP地址：</span>
                    <span id="deviceIP">-</span>
                </div>
            </div>
            <form action='/save' method='POST'>
                <input type='text' name='ssid' placeholder='输入要连接的WiFi名称' required>
                <input type='password' name='password' placeholder='输入WiFi密码' required>
                <button type='submit'>保存并连接</button>
            </form>
        </div>
        
        <div id="systemInfo" class="tab-content">
            <div class="info-grid">
                <div class="info-item">
                    <div class="title">CPU 使用率</div>
                    <div class="value" id="cpuUsage">-</div>
                    <div class="progress-bar">
                        <div class="bar" id="cpuBar" style="width: 0%"></div>
                    </div>
                </div>
                <div class="info-item">
                    <div class="title">可用内存</div>
                    <div class="value" id="freeHeap">-</div>
                    <div class="progress-bar memory">
                        <div class="bar" id="memoryBar" style="width: 0%"></div>
                    </div>
                </div>
                <div class="info-item">
                    <div class="title">活动任务数</div>
                    <div class="value" id="taskCount">-</div>
                    <div class="progress-bar tasks">
                        <div class="bar" id="taskBar" style="width: 0%"></div>
                    </div>
                </div>
                <div class="info-item">
                    <div class="title">运行时间</div>
                    <div class="value" id="uptime" style="margin-bottom: 0">-</div>
                </div>
            </div>
            <button id="refreshBtn" onclick="refreshInfo()">刷新系统信息</button>
            <div class="refresh-timer">自动刷新倒计时：<span id="systemRefreshTimer">10</span>秒</div>
        </div>
        
        <div id="taskInfo" class="tab-content">
            <div id="taskList" class="task-list"></div>
            <button id="refreshTaskBtn" onclick="refreshTaskInfo()">刷新任务信息</button>
            <div class="refresh-timer">自动刷新倒计时：<span id="taskRefreshTimer">10</span>秒</div>
        </div>

        <div id="clientInfo" class="tab-content">
            <div id="clientList" class="client-list">
                <div class="client-item">
                    <div class="client-info">正在加载设备信息...</div>
                </div>
            </div>
            <button id="refreshClientBtn" onclick="refreshClientInfo()">刷新设备列表</button>
            <div class="refresh-timer">自动刷新倒计时：<span id="clientRefreshTimer">10</span>秒</div>
        </div>

        <div id="serverConfig" class="tab-content">
            <div class="server-config">
                <div class="server-info">
                    <div>当前服务器配置状态：<span id="serverStatus">未配置</span></div>
                    <div>服务器地址：<span id="currentServerIP">-</span></div>
                    <div>服务器端口：<span id="currentServerPort">-</span></div>
                    <div>服务器路径：<span id="currentServerPath">-</span></div>
                </div>
            </div>
            <form id="serverConfigForm" onsubmit="saveServerConfig(event)">
                <input type="text" name="ip" placeholder="服务器IP地址" required>
                <input type="number" name="port" placeholder="服务器端口" value="80" required>
                <input type="text" name="path" placeholder="服务器路径" value="/metrics">
                <button type="submit">保存服务器配置</button>
            </form>
        </div>
    </div>
    <script>
        let systemRefreshInterval;
        let taskRefreshInterval;
        let clientRefreshInterval;
        let systemTimer = 10;
        let taskTimer = 10;
        let clientTimer = 10;

        function showTab(tabName) {
            document.querySelectorAll('.tab-content').forEach(content => content.classList.remove('active'));
            document.querySelectorAll('.tab').forEach(tab => tab.classList.remove('active'));
            document.getElementById(tabName).classList.add('active');
            document.querySelector(`[onclick*='${tabName}']`).classList.add('active');
            
            if (tabName === 'metricsData') {
                updateMetricsData();
            }
        }
        
        function updateWiFiInfo() {
            fetch('/wifi-info')
                .then(response => response.json())
                .then(data => {
                    document.getElementById('apName').textContent = data.ap_ssid || '-';
                    document.getElementById('apIP').textContent = data.ap_ip || '-';
                    const status = document.getElementById('wifiStatus');
                    status.textContent = data.is_connected ? '已连接' : '未连接';
                    status.className = data.is_connected ? 'connected' : 'disconnected';
                    document.getElementById('connectedWifi').textContent = data.sta_ssid || '-';
                    document.getElementById('deviceIP').textContent = data.sta_ip || '-';
                });
        }
        
        function setAsServer(ip) {
            showTab('serverConfig');
            
            const form = document.getElementById('serverConfigForm');
            form.querySelector('input[name="ip"]').value = ip;
            form.querySelector('input[name="port"]').value = "80";
            form.querySelector('input[name="path"]').value = "/metrics";
            
            const formData = new FormData(form);
            fetch('/server-config', {
                method: 'POST',
                body: formData
            })
            .then(response => response.text())
            .then(result => {
                updateServerConfig();
                alert('已成功将设备设置为服务器');
            })
            .catch(error => {
                alert('设置失败：' + error);
            });
        }
        
        function refreshClientInfo() {
            fetch('/client-info')
                .then(response => response.json())
                .then(data => {
                    const clientList = document.getElementById('clientList');
                    clientList.innerHTML = '';
                    
                    if (!data.clients || data.clients.length === 0) {
                        clientList.innerHTML = '<div class="client-item"><div class="client-info">当前没有设备连接</div></div>';
                        return;
                    }
                    
                    data.clients.forEach(client => {
                        const rssiPercentage = Math.min(100, Math.max(0, (client.rssi + 100) * 2));
                        const clientItem = document.createElement('div');
                        clientItem.className = 'client-item';
                        clientItem.innerHTML = `
                            <div class="client-info">
                                <div>
                                    <span>设备名称</span>
                                    <span>${client.hostname || '未知设备'}</span>
                                </div>
                                <div>
                                    <span>IP地址</span>
                                    <span>${client.ip}</span>
                                </div>
                                <div>
                                    <span>MAC地址</span>
                                    <span>${client.mac}</span>
                                </div>
                                <div>
                                    <span>信号强度</span>
                                    <div class="signal-strength">
                                        <div class="signal-bar" style="width: ${rssiPercentage}%"></div>
                                    </div>
                                </div>
                                <div>
                                    <span>连接时长</span>
                                    <span>${client.connected_time || '0'} 分钟</span>
                                </div>
                                <button class="action-button" onclick="setAsServer('${client.ip}')">设为服务器</button>
                            </div>
                        `;
                        clientList.appendChild(clientItem);
                    });
                });
            resetClientTimer();
        }
        
        function refreshSystemInfo() {
            fetch('/system-info')
                .then(response => response.json())
                .then(data => {
                    // 更新CPU使用率
                    document.getElementById('cpuUsage').textContent = data.cpuUsage.toFixed(1) + '%';
                    document.getElementById('cpuBar').style.width = data.cpuUsage + '%';
                    
                    // 更新内存使用情况
                    const totalHeap = 512 * 1024; // 假设总内存为512KB
                    const memoryUsage = ((totalHeap - data.freeHeap) / totalHeap * 100).toFixed(1);
                    document.getElementById('freeHeap').textContent = formatBytes(data.freeHeap) + ` (${memoryUsage}% 已用)`;
                    document.getElementById('memoryBar').style.width = memoryUsage + '%';
                    
                    // 更新运行时间
                    document.getElementById('uptime').textContent = formatUptime(data.uptime);
                    
                    // 更新任务数
                    const maxTasks = 20; // 假设最大任务数为20
                    const taskPercentage = Math.min(100, (data.taskCount / maxTasks * 100)).toFixed(1);
                    document.getElementById('taskCount').textContent = data.taskCount + ` (${taskPercentage}%)`;
                    document.getElementById('taskBar').style.width = taskPercentage + '%';
                });
            resetSystemTimer();
        }

        function refreshTaskInfo() {
            fetch('/task-info')
                .then(response => response.json())
                .then(data => {
                    const taskList = document.getElementById('taskList');
                    taskList.innerHTML = "";
                    data.tasks.forEach(task => {
                        const taskItem = document.createElement('div');
                        taskItem.className = 'task-item';
                        taskItem.innerHTML = `
                            <div class="client-info">
                                <div>
                                    <span>任务名称</span>
                                    <span>${task.name}</span>
                                </div>
                                <div>
                                    <span>优先级</span>
                                    <span>${task.priority}</span>
                                </div>
                                <div>
                                    <span>剩余堆栈</span>
                                    <span>${formatBytes(task.stackHigh)}</span>
                                </div>
                            </div>
                        `;
                        taskList.appendChild(taskItem);
                    });
                });
            resetTaskTimer();
        }

        function formatBytes(bytes) {
            if (bytes < 1024) return bytes + " B";
            else if (bytes < 1048576) return (bytes / 1024).toFixed(1) + " KB";
            else return (bytes / 1048576).toFixed(1) + " MB";
        }

        function formatUptime(seconds) {
            const days = Math.floor(seconds / 86400);
            const hours = Math.floor((seconds % 86400) / 3600);
            const minutes = Math.floor((seconds % 3600) / 60);
            const secs = seconds % 60;
            return `${days}天 ${hours}时 ${minutes}分 ${secs}秒`;
        }

        function resetSystemTimer() {
            systemTimer = 10;
            document.getElementById('systemRefreshTimer').textContent = systemTimer;
        }

        function resetTaskTimer() {
            taskTimer = 10;
            document.getElementById('taskRefreshTimer').textContent = taskTimer;
        }

        function resetClientTimer() {
            clientTimer = 10;
            document.getElementById('clientRefreshTimer').textContent = clientTimer;
        }

        function startTimers() {
            clearInterval(systemRefreshInterval);
            clearInterval(taskRefreshInterval);
            clearInterval(clientRefreshInterval);

            systemRefreshInterval = setInterval(() => {
                systemTimer--;
                document.getElementById('systemRefreshTimer').textContent = systemTimer;
                if (systemTimer <= 0) {
                    refreshSystemInfo();
                }
            }, 1000);

            taskRefreshInterval = setInterval(() => {
                taskTimer--;
                document.getElementById('taskRefreshTimer').textContent = taskTimer;
                if (taskTimer <= 0) {
                    refreshTaskInfo();
                }
            }, 1000);

            clientRefreshInterval = setInterval(() => {
                clientTimer--;
                document.getElementById('clientRefreshTimer').textContent = clientTimer;
                if (clientTimer <= 0) {
                    refreshClientInfo();
                }
            }, 1000);
        }

        function updateServerConfig() {
            fetch('/get-server-config')
                .then(response => response.json())
                .then(data => {
                    document.getElementById('serverStatus').textContent = data.configured ? '已配置' : '未配置';
                    document.getElementById('currentServerIP').textContent = data.ip || '-';
                    document.getElementById('currentServerPort').textContent = data.port || '-';
                    document.getElementById('currentServerPath').textContent = data.path || '-';
                });
        }

        function saveServerConfig(event) {
            event.preventDefault();
            const form = document.getElementById('serverConfigForm');
            const formData = new FormData(form);
            
            fetch('/server-config', {
                method: 'POST',
                body: formData
            })
            .then(response => response.text())
            .then(result => {
                alert(result);
                updateServerConfig();
            })
            .catch(error => {
                alert('保存失败：' + error);
            });
        }

        function updateMetricsData() {
            fetch('/metrics-data')
                .then(response => response.text())
                .then(data => {
                    const content = document.getElementById('metricsContent');
                    content.textContent = data || '暂无数据';
                })
                .catch(error => {
                    console.error('获取数据失败:', error);
                });
        }

        function startMetricsTimer() {
            clearInterval(metricsRefreshInterval);
            metricsRefreshInterval = setInterval(updateMetricsData, 500);
        }

        document.addEventListener('DOMContentLoaded', () => {
            updateWiFiInfo();
            refreshSystemInfo();
            refreshTaskInfo();
            refreshClientInfo();
            updateServerConfig();
            updateMetricsData();
            startTimers();
            startMetricsTimer();
        });
    </script>
</body>
</html>
)rawliteral";

void handleRoot() {
    server.send(200, "text/html", html_page);
}

void handleSystemInfo() {
    uint32_t freeHeap = esp_get_free_heap_size();
    uint32_t uptime = esp_timer_get_time() / 1000000;
    
    static uint32_t lastTotalTime = 0;
    static uint32_t lastIdleTime = 0;
    uint32_t totalTime = 0;
    uint32_t idleTime = 0;
    
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
    
    float cpuUsage = 0;
    if (lastTotalTime > 0) {
        uint32_t totalDelta = totalTime - lastTotalTime;
        uint32_t idleDelta = idleTime - lastIdleTime;
        cpuUsage = (1.0f - ((float)idleDelta / totalDelta)) * 100;
    }
    lastTotalTime = totalTime;
    lastIdleTime = idleTime;
    
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
            json += "\"stackHigh\":" + String(taskStatusArray[i].usStackHighWaterMark);
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
    memset(&sta_list, 0, sizeof(wifi_sta_list_t));
    esp_wifi_ap_get_sta_list(&sta_list);
    
    esp_netif_dhcp_status_t status;
    esp_netif_dhcps_get_status(ap_netif, &status);
    
    String json = "{\"clients\":[";
    
    for(int i = 0; i < sta_list.num; i++) {
        if(i > 0) json += ",";
        json += "{";
        
        char mac[18];
        sprintf(mac, "%02X:%02X:%02X:%02X:%02X:%02X",
                sta_list.sta[i].mac[0], sta_list.sta[i].mac[1],
                sta_list.sta[i].mac[2], sta_list.sta[i].mac[3],
                sta_list.sta[i].mac[4], sta_list.sta[i].mac[5]);
        
        uint32_t base_ip = static_cast<uint32_t>(ap_local_ip);
        uint32_t offset = 100 + i;
        uint32_t client_ip = (base_ip & 0xFFFFFF00) | offset;
        
        char ip[16];
        sprintf(ip, "%d.%d.%d.%d",
                client_ip & 0xff,
                (client_ip >> 8) & 0xff,
                (client_ip >> 16) & 0xff,
                (client_ip >> 24));
        
        json += "\"mac\":\"" + String(mac) + "\",";
        json += "\"ip\":\"" + String(ip) + "\",";
        json += "\"hostname\":\"" + String("设备 ") + String(i + 1) + "\",";
        json += "\"rssi\":\"" + String(sta_list.sta[i].rssi) + " dBm\",";
        json += "\"dhcp_status\":\"" + String(status == ESP_NETIF_DHCP_STARTED ? "已分配" : "未分配") + "\"";
        json += "}";
    }
    
    json += "]}";
    server.send(200, "application/json", json);
}

void handleServerConfig() {
    String ip = server.arg("ip");
    String port = server.arg("port");
    String path = server.arg("path");
    
    if (ip.length() > 0) {
        saveServerConfig(ip.c_str(), port.toInt(), path.c_str());
        server.send(200, "text/plain", "服务器配置已保存");
        printf("收到新的服务器配置: IP=%s, Port=%s\n", ip.c_str(), port.c_str());
    } else {
        server.send(400, "text/plain", "无效的配置");
        printf("收到无效的服务器配置\n");
    }
}

void handleGetServerConfig() {
    String json = "{";
    json += "\"ip\":\"" + String(serverConfig.ip) + "\",";
    json += "\"port\":" + String(serverConfig.port) + ",";
    json += "\"path\":\"" + String(serverConfig.path) + "\",";
    json += "\"configured\":" + String(serverConfig.configured ? "true" : "false");
    json += "}";
    server.send(200, "application/json", json);
}

void initWebServer() {
    server.on("/", handleRoot);
    server.on("/save", HTTP_POST, handleSave);
    server.on("/system-info", handleSystemInfo);
    server.on("/task-info", handleTaskInfo);
    server.on("/wifi-info", handleWiFiInfo);
    server.on("/client-info", handleClientInfo);
    server.on("/server-config", HTTP_POST, handleServerConfig);
    server.on("/get-server-config", handleGetServerConfig);
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