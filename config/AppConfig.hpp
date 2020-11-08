#ifndef __APP_CONFIG_HPP__
#define __APP_CONFIG_HPP__

#define LOG0( format ) Serial.printf( "(%lu) " format, millis())
#define LOG1( format, ... ) Serial.printf( "(%lu) " format, millis(), ##__VA_ARGS__ )

#define OTA_HOSTNAME "esp8266-sh4-99751-gateway"
#define OTA_PASSWORD "otapass"
#define WIFI_SSID "<your wifi ssid>"
#define WIFI_PASS "<your wifi password>"

#define MQTT_ENABLED true
#define MQTT_CLIENTID "sh4-99751-gateway"
#define MQTT_HOST "192.168.1.1"
#define MQTT_PORT 1883
#define MQTT_USEAUTH true
#define MQTT_USER "<your mqtt user>"
#define MQTT_PASSWORD "<your mqtt password>"
#define MQTT_BATTERY_TOPIC "/sh4-99751-gateway/state/battery"
#define MQTT_COMMAND_TOPIC "/sh4-99751-gateway/%ld/command"

#define VCC_ADJUST 1.115449915

#endif
