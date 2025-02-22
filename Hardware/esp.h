
#ifndef __ESP_H
#define __ESP_H

// OneNET平台配置信息
#define ONENET_DEVICE_ID    "SmFONzEyRDhZNnBRd3lzUkc4ZG5hUW1nejVOOGZlRjE="    // OneNET设备ID
#define ONENET_API_KEY      "tcWzS6+3L48hbxc/BT3dmytPeVs9Q/fVMbkLzSezQTU="    // API密钥
#define ONENET_PRODUCT_ID   "Ub99ifb8x7"     // 产品ID
// OneNET平台配置信息
#define ONENET_SERVER       "183.230.40.39"  // OneNET服务器地址
#define ONENET_PORT        "6002"           // OneNET服务器端口
// WiFi配置
#define WIFI_SSID          "1234"           // WiFi名称
#define WIFI_PASSWORD      "123456789"      // WiFi密码

// 函数声明
void ESP8266_Init(void);
void ESP8266_SendData(void);
void ESP8266_MQTTPublish(void);
void ESP8266_ReceiveData(void);

#endif
