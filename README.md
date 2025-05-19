# ESP32-C6 Router

基于ESP32-C6开发的智能路由器项目，支持WiFi配置、系统监控和任务管理功能。

## 项目概述

本项目利用ESP32-C6的双WiFi功能实现一个简单的路由器，具有以下特点：
- 支持AP+STA双模式同时工作
- 提供Web配置界面
- 支持系统状态监控
- 实现NAT路由功能
- 使用FreeRTOS进行任务管理

## 主要功能

### 1. WiFi管理
- AP模式自动配置
- STA模式Web配置
- NVS配置存储
- NAT路由转发

### 2. Web管理界面
- WiFi配置页面
  - SSID和密码设置
  - 配置保存和加载
- 系统信息页面
  - CPU使用率监控
  - 内存使用情况
  - 系统运行时间
  - 任务数量统计
- 任务管理页面
  - 实时任务列表
  - 任务优先级显示
  - 堆栈使用情况
  - 运行核心分配

### 3. 系统功能
- FreeRTOS多任务管理
- 系统资源监控
- 配置持久化存储
- 自动重连机制

## 技术特性

- 开发平台：Arduino IDE
- 目标芯片：ESP32-C6
- 主要依赖：
  - ESP32 Arduino Core
  - WebServer
  - FreeRTOS
  - NVS Flash
  - ESP-IDF

## 目录结构

```
ESP32C6_Router/
├── ESP32C6_Router.ino    # 主程序
├── README.md             # 项目说明
├── config_manager.cpp    # 配置管理模块
├── config_manager.h
├── web_server.cpp        # Web服务器模块
├── web_server.h
├── wifi_manager.cpp      # WiFi管理模块
└── wifi_manager.h
```

## 使用方法

1. 硬件要求
   - ESP32-C6开发板
   - USB数据线
   - 5V电源供电

2. 软件环境
   - Arduino IDE 2.0+
   - ESP32-C6 Arduino Core
   - 必要的库文件

3. 编译上传
   ```bash
   # 克隆项目
   git clone [项目地址]
   
   # 使用Arduino IDE打开
   # 选择开发板：ESP32C6 Dev Module
   # 编译并上传
   ```

4. 使用说明
   - 设备上电后自动创建AP热点：ESP32-C6-Router
   - 连接热点，默认密码：12345678
   - 访问 http://192.168.4.1 进入管理界面
   - 在WiFi配置页面设置要连接的网络
   - 设备会自动保存配置并重启

## 注意事项

1. 编译相关
   - 确保Arduino IDE已安装ESP32-C6支持包
   - 检查所有依赖库是否安装
   - 编译时选择正确的开发板型号

2. 使用相关
   - 首次使用需要配置WiFi
   - 配置保存后设备会自动重启
   - 建议使用5V/2A电源供电
   - Web界面支持自动刷新功能

## 版本历史

### v1.0.00 (2025-05-19)
初始版本发布，实现基本功能：

1. 新增功能：
   - 实现AP+STA双模式
   - 添加Web配置界面
   - 支持NAT路由功能
   - 添加系统监控功能
   - 实现任务管理功能
   - NVS配置存储

2. 技术实现：
   - 使用FreeRTOS多任务管理
   - 实现模块化设计
   - 添加完整的错误处理
   - 优化内存使用

3. 界面优化：
   - 响应式Web界面设计
   - 实时数据更新
   - 用户友好的配置流程
   - 系统状态可视化

## 贡献指南

欢迎提交Issue和Pull Request来帮助改进项目。

## 许可证

本项目采用 MIT 许可证。 