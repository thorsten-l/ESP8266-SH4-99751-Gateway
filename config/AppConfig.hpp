#ifndef __APP_CONFIG_HPP__
#define __APP_CONFIG_HPP__

#define LOG0( format ) Serial.printf( "(%lu) " format, millis())
#define LOG1( format, ... ) Serial.printf( "(%lu) " format, millis(), ##__VA_ARGS__ )

#define OTA_HOSTNAME "esp8266-smartwares-gateway"
#define OTA_PASSWORD "otapass"
#define WIFI_SSID "<your wifi ssid>"
#define WIFI_PASS "<your wifi password>"

#define TIMEZONE "CET-1CEST-2,M3.5.0/02:00:00,M10.5.0/03:00:00"
#define NTP_SERVER1 "ptbtime1.ptb.de"
#define NTP_SERVER2 "ptbtime2.ptb.de"
#define NTP_SERVER3 "de.pool.ntp.org"

#define MQTT_ENABLED true
#define MQTT_CLIENTID "smartwares-gateway"
#define MQTT_HOST "192.168.1.1"
#define MQTT_PORT 1883
#define MQTT_USEAUTH true
#define MQTT_USER "<your mqtt user>"
#define MQTT_PASSWORD "<your mqtt password>"
#define MQTT_BATTERY_TOPIC "/smartwares-gateway/state/battery"
#define MQTT_COMMAND_TOPIC "/smartwares-gateway/%ld/%ld/command"

#define VCC_ADJUST 1.115449915

#endif
