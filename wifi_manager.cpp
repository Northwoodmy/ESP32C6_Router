#include "wifi_manager.h"
#include "esp_wifi.h"
#include "esp_mac.h"
#include "esp_netif.h"
#include "esp_wifi_types.h"
#include "esp_private/wifi.h"
#include "lwip/ip4.h"
#include "lwip/ip4_frag.h"
#include "lwip/dns.h"
#include "lwip/netif.h"
#include "lwip/ip_addr.h"
#include "lwip/ip.h"
#include "lwip/opt.h"
#include "lwip/err.h"
#include "lwip/inet.h"
#include "esp_netif_net_stack.h"
#include "dhcpserver/dhcpserver_options.h"

#define CONFIG_LWIP_IP4_NAPT 1
#define IP_NAPT_MAX 512
#define IP_PORTMAP_MAX 32

// DHCP选项定义
#define DHCP_OPTION_DNS 6
#define DHCP_OPTION_ROUTER 3

TaskHandle_t wifiTaskHandle = NULL;
WifiConfig wifiConfig;
esp_netif_t* ap_netif = NULL;
esp_netif_t* sta_netif = NULL;

// DHCP服务器配置
const IPAddress ap_local_ip(192, 168, 4, 1);
const IPAddress ap_gateway(192, 168, 4, 1);
const IPAddress ap_subnet(255, 255, 255, 0);
const IPAddress dhcp_pool_start(192, 168, 4, 100);  // DHCP起始地址
const IPAddress dhcp_pool_end(192, 168, 4, 150);    // DHCP结束地址

bool setupNAT() {
    // 启用DNS转发
    dns_init();
    
    // 配置DNS服务器
    esp_netif_dns_info_t dns_info = {};
    dns_info.ip.u_addr.ip4.addr = htonl(0x08080808); // 8.8.8.8
    dns_info.ip.type = ESP_IPADDR_TYPE_V4;
    
    if (esp_netif_set_dns_info(ap_netif, ESP_NETIF_DNS_MAIN, &dns_info) != ESP_OK) {
        printf("DNS配置失败\n");
        return false;
    }
    
    printf("NAT和DNS转发功能已启用\n");
    return true;
}

void configureDHCP() {
    if (ap_netif == NULL) {
        printf("警告：无法配置DHCP，AP网络接口未初始化\n");
        return;
    }
    
    // 停止DHCP服务器
    esp_netif_dhcps_stop(ap_netif);
    
    // 配置DHCP服务器
    dhcps_lease_t lease;
    lease.enable = true;
    lease.start_ip.addr = static_cast<uint32_t>(dhcp_pool_start);
    lease.end_ip.addr = static_cast<uint32_t>(dhcp_pool_end);
    
    // 设置IP地址范围
    esp_err_t result = esp_netif_dhcps_option(
        ap_netif,
        ESP_NETIF_OP_SET,
        ESP_NETIF_REQUESTED_IP_ADDRESS,
        &lease,
        sizeof(dhcps_lease_t)
    );
    printf("DHCP设置IP范围: %s\n", result == ESP_OK ? "成功" : "失败");
    
    // 设置DNS服务器
    // 使用Google DNS作为备用
    uint8_t dns_ip[4] = {8, 8, 8, 8};
    result = esp_netif_dhcps_option(
        ap_netif,
        ESP_NETIF_OP_SET,
        ESP_NETIF_DOMAIN_NAME_SERVER,
        &dns_ip,
        sizeof(dns_ip)
    );
    printf("DHCP设置DNS: %s\n", result == ESP_OK ? "成功" : "失败");
    
    // 重新启动DHCP服务器
    result = esp_netif_dhcps_start(ap_netif);
    printf("DHCP服务器启动: %s\n", result == ESP_OK ? "成功" : "失败");
    
    // 打印DHCP池信息
    printf("DHCP IP范围: %s - %s\n", dhcp_pool_start.toString().c_str(), dhcp_pool_end.toString().c_str());
}

void configureIP() {
    // 配置AP的IP地址
    esp_netif_ip_info_t ip_info;
    ip_info.ip.addr = static_cast<uint32_t>(ap_local_ip);
    ip_info.gw.addr = static_cast<uint32_t>(ap_gateway);
    ip_info.netmask.addr = static_cast<uint32_t>(ap_subnet);
    
    esp_netif_set_ip_info(ap_netif, &ip_info);
    
    // 配置DHCP服务器
    configureDHCP();
    
    printf("IP配置完成\n");
}

void enableNAT() {
    // 禁用WiFi省电模式以提高性能
    esp_wifi_set_ps(WIFI_PS_NONE);
    
    if (WiFi.status() == WL_CONNECTED) {
        // 确保获取到网络接口句柄
        if (ap_netif == NULL) {
            ap_netif = esp_netif_get_handle_from_ifkey("WIFI_AP_DEF");
        }
        if (sta_netif == NULL) {
            sta_netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
        }
        
        if (ap_netif != NULL && sta_netif != NULL) {
            // 设置默认路由
            esp_netif_set_default_netif(sta_netif);
            
            printf("NAT路由功能已启用\n");
            printf("AP网段: %s\n", ap_local_ip.toString().c_str());
            printf("STA IP: %s\n", WiFi.localIP().toString().c_str());
            printf("默认网关: %s\n", WiFi.gatewayIP().toString().c_str());
        } else {
            printf("NAT路由未启用：网络接口未初始化\n");
        }
    } else {
        printf("NAT路由未启用：STA未连接\n");
    }
}

void checkAndRestartNetworking() {
    static uint32_t lastCheck = 0;
    static uint8_t failCount = 0;
    
    uint32_t currentTime = millis();
    if (currentTime - lastCheck > 30000) {  // 每30秒检查一次
        lastCheck = currentTime;
        
        if (WiFi.status() != WL_CONNECTED && wifiConfig.configured) {
            failCount++;
            printf("检测到网络断开，失败计数: %d/3\n", failCount);
            
            if (failCount >= 3) {  // 连续失败3次后重置网络
                printf("网络多次连接失败，重置网络服务...\n");
                
                // 重置网络，但不重启设备
                failCount = 0;
                WiFi.disconnect();
                
                // 不使用delay，在WiFi任务中重新连接
            }
        } else if (WiFi.status() == WL_CONNECTED) {
            failCount = 0;
        }
    }
}

void initWiFiManager() {
    // 确保WiFi已经初始化
    WiFi.disconnect(true);
    vTaskDelay(pdMS_TO_TICKS(1000));
    
    // 设置WiFi模式
    WiFi.mode(WIFI_MODE_APSTA);
    vTaskDelay(pdMS_TO_TICKS(500));
    
    // 配置AP
    bool apStarted = WiFi.softAP("ESP32-C6-Router", "12345678", 1, false, 10);
    if (!apStarted) {
        printf("启动AP失败！尝试重置...\n");
        WiFi.disconnect(true);
        vTaskDelay(pdMS_TO_TICKS(1000));
        WiFi.mode(WIFI_MODE_APSTA);
        vTaskDelay(pdMS_TO_TICKS(500));
        apStarted = WiFi.softAP("ESP32-C6-Router", "12345678");
    }
    
    // 配置IP地址
    bool ipConfigured = WiFi.softAPConfig(ap_local_ip, ap_gateway, ap_subnet);
    printf("AP IP配置%s\n", ipConfigured ? "成功" : "失败");
    
    // 获取网络接口引用
    ap_netif = esp_netif_get_handle_from_ifkey("WIFI_AP_DEF");
    sta_netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
    
    // 重新配置DHCP服务器
    configureDHCP();
    
    // 启用DNS和NAT功能
    setupNAT();
    
    wifiConfig.configured = false;
    printf("WiFi管理器初始化完成\n");
    printf("AP状态: %s, IP: %s\n", apStarted ? "已启动" : "启动失败", WiFi.softAPIP().toString().c_str());
}

void setupAP() {
    // 配置AP
    WiFi.softAP("ESP32-C6-Router", "12345678", 1, false, 10);
    
    // 配置IP地址
    WiFi.softAPConfig(ap_local_ip, ap_gateway, ap_subnet);
    
    // 配置DHCP
    configureDHCP();
    
    printf("AP模式已启动\n");
    printf("AP IP地址: %s\n", WiFi.softAPIP().toString().c_str());
    printf("最大连接数: 10\n");
}

void startWiFiTask(void* parameter) {
    // WiFi 连接状态和计时器
    static uint32_t reconnectTimer = 0;
    static uint32_t connectionStartTime = 0;
    static bool connectionInProgress = false;
    static uint8_t connectionAttempts = 0;
    const uint8_t MAX_CONNECTION_ATTEMPTS = 3;
    const uint32_t CONNECTION_TIMEOUT = 15000; // 15秒超时
    const uint32_t RECONNECT_INTERVAL = 30000; // 30秒重连间隔
    
    while(1) {
        uint32_t currentTime = millis();
        
        if (wifiConfig.configured) {
            // 检查是否需要开始连接过程
            if (!connectionInProgress && WiFi.status() != WL_CONNECTED) {
                // 检查重连定时器
                if (currentTime - reconnectTimer >= RECONNECT_INTERVAL) {
                    // 开始连接过程
                    connectionInProgress = true;
                    connectionStartTime = currentTime;
                    connectionAttempts++;
                    
                    printf("正在连接WiFi... (尝试 %d/%d)\n", connectionAttempts, MAX_CONNECTION_ATTEMPTS);
                    WiFi.begin(wifiConfig.ssid, wifiConfig.password);
                }
            }
            
            // 检查连接状态
            if (connectionInProgress) {
                // 检查是否连接成功
                if (WiFi.status() == WL_CONNECTED) {
                    printf("\nWiFi连接成功！\n");
                    printf("IP地址: %s\n", WiFi.localIP().toString().c_str());
                    
                    // 重置连接变量
                    connectionInProgress = false;
                    connectionAttempts = 0;
                    
                    // 启用NAT路由
                    enableNAT();
                }
                // 检查是否超时
                else if (currentTime - connectionStartTime >= CONNECTION_TIMEOUT) {
                    printf("\nWiFi连接超时\n");
                    connectionInProgress = false;
                    reconnectTimer = currentTime;
                    
                    // 检查是否达到最大尝试次数
                    if (connectionAttempts >= MAX_CONNECTION_ATTEMPTS) {
                        printf("达到最大尝试次数，将在 %d 秒后重试\n", RECONNECT_INTERVAL / 1000);
                        connectionAttempts = 0;
                    }
                }
                // 连接中，打印进度
                else if ((currentTime - connectionStartTime) % 1000 < 20) {
                    // 每秒打印一个点
                    printf(".");
                    fflush(stdout);
                }
            }
            
            // 检查网络状态
            checkAndRestartNetworking();
        }
        
        // 短暂延迟不会阻塞其他任务
        vTaskDelay(pdMS_TO_TICKS(100));
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