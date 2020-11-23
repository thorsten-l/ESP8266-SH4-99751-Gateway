#ifndef __APP_H__
#define __APP_H__

#include <Arduino.h>

#define LOG0( format ) Serial.printf( "(%ld) " format, millis())
#define LOG1( format, ... ) Serial.printf( "(%lu) " format, millis(), ##__VA_ARGS__ )
#define TLOG0( format ) TelnetStream.printf( "(%ld) " format, millis())
#define TLOG1( format, ... ) TelnetStream.printf( "(%ld) " format, millis(), ##__VA_ARGS__ )

#define APP_NAME "Smartwares Gateway"
#define APP_VERSION "0.4.4"
#define APP_AUTHOR "Dr. Thorsten Ludewig <t.ludewig@gmail.com>"
#define APP_CONFIG_FILE_JSON "/config.json"

#define WIFI_LED_ON   0
#define WIFI_LED_OFF  1

extern void appShowHeader(Stream& out);
extern const char *appUptime();
extern const char *appDateTime();

// Network mode
#define NET_MODE_STATIC 1
#define NET_MODE_DHCP 2

typedef struct appconfig
{
  char wifi_ssid[64];
  char wifi_password[64];
  int  wifi_mode;

  int net_mode;
  char net_host[64];
  char net_mask[64];
  char net_gateway[64];
  char net_dns[64];

  char ota_hostname[64];
  char ota_password[64];

  char admin_password[64];

  bool mqtt_enabled;
  char mqtt_clientid[64];
  char mqtt_host[64];
  int  mqtt_port;
  bool mqtt_useauth;
  char mqtt_user[64];
  char mqtt_password[64];
  char mqtt_intopic[64];
  char mqtt_outtopic[64];
  char mqtt_topic_json[64];
  unsigned long mqtt_sending_interval;
  
  bool telnet_enabled;
  bool ota_enabled;

  bool ntp_enabled;
  char ntp_timezone[64];
  char ntp_server1[64];
  char ntp_server2[64];
  char ntp_server3[64];

} AppConfig;

class App
{
private:
  char initFilename[32];
  bool initialized = false;
  bool doSystemRestart;
  unsigned long systemRestartTimestamp;
  unsigned int systemRestartCounter;
  unsigned long ledStateTimestamp;
  int wifiLedState;

  void formatFS();
  void restartSystem();
  void updateLedStates( unsigned long timestamp );

public:
  size_t fsTotalBytes;
  size_t fsUsedBytes;

  App();

  void setup();
  void firmwareReset();
  void defaultConfig();
  void loadConfig();
  void writeConfig();
  bool loadJsonConfig( const char *filename );
  void printConfig(AppConfig ac);
  void delayedSystemRestart();
  void loop();
  //
  void wifiLedOn();
  void wifiLedOff();
  void wifiLedToggle();
};

extern App app;
extern AppConfig appcfg;
extern AppConfig appcfgWR;
extern AppConfig appcfgRD;

#endif
