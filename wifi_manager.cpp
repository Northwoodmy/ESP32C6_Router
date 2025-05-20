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
    // 配置DHCP服务器
    dhcps_lease_t lease;
    lease.enable = true;
    lease.start_ip.addr = static_cast<uint32_t>(dhcp_pool_start);
    lease.end_ip.addr = static_cast<uint32_t>(dhcp_pool_end);
    
    // 停止DHCP服务器，设置配置，然后重启
    esp_netif_dhcps_stop(ap_netif);
    
    // 设置IP地址范围
    esp_netif_dhcps_option(ap_netif, ESP_NETIF_OP_SET, ESP_NETIF_REQUESTED_IP_ADDRESS, &lease, sizeof(dhcps_lease_t));
    
    // 设置IP地址租约时间（默认7200秒）
    uint32_t lease_time = 7200;
    esp_netif_dhcps_option(ap_netif, ESP_NETIF_OP_SET, ESP_NETIF_IP_ADDRESS_LEASE_TIME, &lease_time, sizeof(lease_time));
    
    // 设置网络参数
    esp_netif_ip_info_t ip_info;
    ip_info.ip.addr = static_cast<uint32_t>(ap_local_ip);
    ip_info.gw.addr = static_cast<uint32_t>(ap_gateway);
    ip_info.netmask.addr = static_cast<uint32_t>(ap_subnet);
    esp_netif_set_ip_info(ap_netif, &ip_info);
    
    // 启动DHCP服务器
    esp_netif_dhcps_start(ap_netif);
    
    printf("DHCP服务器配置完成，IP范围: %s - %s\n", 
           dhcp_pool_start.toString().c_str(), 
           dhcp_pool_end.toString().c_str());
    printf("IP地址租约时间: %d秒\n", lease_time);
}

void configureIP() {
    // 配置AP的IP地址
    esp_netif_ip_info_t ip_info = {};
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
    
    if (millis() - lastCheck > 30000) {  // 每30秒检查一次
        lastCheck = millis();
        
        if (WiFi.status() != WL_CONNECTED) {
            failCount++;
            printf("检测到网络断开，尝试重连...\n");
            
            if (failCount >= 3) {  // 连续失败3次后重启网络
                printf("网络多次连接失败，重启网络服务...\n");
                WiFi.disconnect(true);
                delay(1000);
                ESP.restart();
            }
        } else {
            failCount = 0;
        }
    }
}

void initWiFiManager() {
    // 初始化WiFi
    WiFi.mode(WIFI_MODE_APSTA);
    
    // 获取网络接口引用
    ap_netif = esp_netif_get_handle_from_ifkey("WIFI_AP_DEF");
    sta_netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
    
    // 配置AP
    WiFi.softAP("ESP32-C6-Router", "12345678");
    
    // 配置IP地址
    WiFi.softAPConfig(ap_local_ip, ap_gateway, ap_subnet);
    
    // 启用DNS和NAT功能
    setupNAT();
    
    wifiConfig.configured = false;
    printf("WiFi管理器初始化完成\n");
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
    while(1) {
        if (wifiConfig.configured) {
            if (WiFi.status() != WL_CONNECTED) {
                // 连接到配置的WiFi
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
                    
                    // 启用NAT路由
                    enableNAT();
                } else {
                    printf("\nWiFi连接失败，请检查配置\n");
                }
            }
            
            // 检查网络状态
            checkAndRestartNetworking();
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