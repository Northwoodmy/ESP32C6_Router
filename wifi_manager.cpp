#include "wifi_manager.h"
#include "esp_wifi.h"
#include "esp_mac.h"
#include "esp_netif.h"
#include "dhcpserver/dhcpserver.h"

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

void configureDHCP() {
    // 配置DHCP服务器
    dhcps_lease_t lease;
    lease.enable = true;
    lease.start_ip.addr = static_cast<uint32_t>(dhcp_pool_start);
    lease.end_ip.addr = static_cast<uint32_t>(dhcp_pool_end);
    
    // 停止DHCP服务器，设置配置，然后重启
    esp_netif_dhcps_stop(ap_netif);
    esp_netif_dhcps_option(ap_netif, ESP_NETIF_OP_SET, ESP_NETIF_REQUESTED_IP_ADDRESS, &lease, sizeof(dhcps_lease_t));
    esp_netif_dhcps_start(ap_netif);
    
    printf("DHCP服务器配置完成，IP范围: %s - %s\n", 
           dhcp_pool_start.toString().c_str(), 
           dhcp_pool_end.toString().c_str());
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
    
    // 设置DNS服务器
    esp_netif_dns_info_t dns_info = {};
    dns_info.ip.u_addr.ip4.addr = htonl(0x08080808); // 8.8.8.8
    dns_info.ip.type = ESP_IPADDR_TYPE_V4;
    esp_netif_set_dns_info(ap_netif, ESP_NETIF_DNS_MAIN, &dns_info);
    
    printf("IP配置完成\n");
}

void enableNAT() {
    // 启用IP转发
    esp_wifi_set_ps(WIFI_PS_NONE);
    printf("IP转发已启用\n");
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
    enableNAT();
    
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
                    // 连接成功后设置AP和启用NAT
                    setupAP();
                } else {
                    printf("\nWiFi连接失败，请检查配置\n");
                }
            }
        } else {
            setupAP();
            vTaskDelete(NULL);
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