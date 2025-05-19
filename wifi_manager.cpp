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
        // 获取STA接口的IP信息
        esp_netif_ip_info_t ip_info;
        esp_netif_get_ip_info(sta_netif, &ip_info);
        
        // 配置路由
        esp_netif_ip_info_t ap_ip_info;
        esp_netif_get_ip_info(ap_netif, &ap_ip_info);
        
        // 设置默认路由
        esp_netif_set_default_netif(sta_netif);
        
        printf("NAT路由功能已启用\n");
        printf("AP网段: %s\n", ap_local_ip.toString().c_str());
        printf("STA IP: %s\n", WiFi.localIP().toString().c_str());
        printf("默认网关: %s\n", WiFi.gatewayIP().toString().c_str());
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
    // 初始化网络接口
    esp_netif_init();
    
    // 创建默认事件循环
    esp_event_loop_create_default();
    
    // 创建AP和STA接口
    ap_netif = esp_netif_create_default_wifi_ap();
    sta_netif = esp_netif_create_default_wifi_sta();
    
    // 初始化WiFi
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);
    
    // 设置WiFi模式为AP+STA
    esp_wifi_set_mode(WIFI_MODE_APSTA);
    
    // 初始化NAT功能
    setupNAT();
    
    wifiConfig.configured = false;
    printf("WiFi管理器初始化完成\n");
}

void setupAP() {
    // 配置AP
    wifi_config_t ap_config = {};
    strcpy((char*)ap_config.ap.ssid, "ESP32-C6-Router");
    strcpy((char*)ap_config.ap.password, "12345678");
    ap_config.ap.ssid_len = strlen("ESP32-C6-Router");
    ap_config.ap.channel = 1;
    ap_config.ap.authmode = WIFI_AUTH_WPA2_PSK;
    ap_config.ap.max_connection = 10;  // 增加最大连接数
    ap_config.ap.beacon_interval = 100;  // 设置信标间隔
    
    esp_wifi_set_config(WIFI_IF_AP, &ap_config);
    
    // 配置IP和启用NAT
    configureIP();
    
    printf("AP模式已启动\n");
    printf("AP IP地址: %s\n", WiFi.softAPIP().toString().c_str());
    printf("最大连接数: %d\n", ap_config.ap.max_connection);
}

void startWiFiTask(void* parameter) {
    // 启动WiFi
    esp_wifi_start();
    
    while(1) {
        if (wifiConfig.configured) {
            if (WiFi.status() != WL_CONNECTED) {
                // 配置STA
                wifi_config_t sta_config = {};
                strcpy((char*)sta_config.sta.ssid, wifiConfig.ssid);
                strcpy((char*)sta_config.sta.password, wifiConfig.password);
                esp_wifi_set_config(WIFI_IF_STA, &sta_config);
                
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
                    
                    // 先设置AP模式
                    setupAP();
                    // 然后启用NAT路由
                    enableNAT();
                } else {
                    printf("\nWiFi连接失败，请检查配置\n");
                    // 如果连接失败，仍然启动AP模式
                    setupAP();
                }
            }
            
            // 检查网络状态
            checkAndRestartNetworking();
        } else {
            setupAP();
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