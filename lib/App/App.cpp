
#include "App.hpp"
#include <Arduino.h>
#include <ArduinoOTA.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <Esp.h>
#include <TelnetStream.h>

#include <OtaHandler.hpp>
#include <Util.hpp>
#include <WebHandler.hpp>
#include <WifiHandler.hpp>

#include <FS.h>
#include <LittleFS.h>

#include "ConfigAttributes.hpp"
#include "DefaultAppConfig.h"
#include <MicroJson.hpp>

time_t appStartTime = 0l;
struct tm appTimeinfo;
char appUptimeBuffer[64];
char appLocalIpBuffer[32];
char appDateTimeBuffer[32];

App app;
AppConfig appcfg;
AppConfig appcfgWR;
AppConfig appcfgRD;

const char *appDateTime()
{
  time_t now = time(nullptr);
  localtime_r(&now, &appTimeinfo);
  sprintf(appDateTimeBuffer, "%4d-%02d-%02d %02d:%02d:%02d", appTimeinfo.tm_year + 1900, appTimeinfo.tm_mon + 1,
          appTimeinfo.tm_mday, appTimeinfo.tm_hour, appTimeinfo.tm_min, appTimeinfo.tm_sec);
  return appDateTimeBuffer;
}

const char *appUptime()
{
  time_t now = time(nullptr);
  time_t uptime = now - appStartTime;
  int uptimeSeconds = uptime % 60;
  int uptimeMinutes = (uptime / 60) % 60;
  int uptimeHours = (uptime / 3600) % 24;
  time_t uptimeDays = (uptime / 86400);
  sprintf(appUptimeBuffer, "%ld days, %d hours, %d minutes, %d seconds", uptimeDays, uptimeHours, uptimeMinutes,
          uptimeSeconds);
  return appUptimeBuffer;
}

void appShowHeader(Stream &out)
{
  out.println("\n\n" APP_NAME " - " APP_VERSION " - " APP_AUTHOR);
  out.println("BUILD: " __DATE__ " " __TIME__);
  out.println("PIOENV: " PIOENV);
  out.println("PIOPLATFORM: " PIOPLATFORM);
  out.println("PIOFRAMEWORK: " PIOFRAMEWORK);
  out.printf("ESP SDK Version: %s\n", ESP.getSdkVersion());
  out.printf("ESP Core Version: %s\n\n", ESP.getCoreVersion().c_str());
}

void App::setup()
{
  int bootDevice = getBootDevice();

#ifdef WIFI_LED
  pinMode(WIFI_LED, OUTPUT);
  wifiLedOff();
#endif

#ifdef BUTTON_PIN
  pinMode(BUTTON_PIN, INPUT_PULLUP);
#endif

  Serial.begin(115200);

  for (int i = 0; i < 5; i++)
  {
    wifiLedOn();
    delay(500);
    wifiLedOff();
    delay(500);
    Serial.println();
  }

  appShowHeader(Serial);
  Serial.printf("\nBoot device = %d\n", bootDevice);
  Serial.printf("Boot mode = %d\n", ESP.getBootMode());
  Serial.printf("Boot version = %d\n", ESP.getBootVersion());

  if (bootDevice == 1)
  {
    Serial.println(F("\nThis sketch has just been uploaded over the UART.\n"
                     "The ESP8266 will freeze on the first restart.\n"
                     "Press the reset button or power cycle the ESP\n"
                     "and operation will be resumed thereafter."));
  }

  Serial.println();

  if (LittleFS.begin())
  {
    if (!LittleFS.exists(initFilename))
    {
      LOG1("WARNING: Init file <%s> does not exist.\n", initFilename);
      LOG0("Formatting LittleFS... ");
      LittleFS.format();
      Serial.println("done.\n");
      File config = LittleFS.open(initFilename, "w");
      config.write(0);
      config.close();
      WiFi.persistent(false);
      WiFi.disconnect(true);
      WiFi.mode(WIFI_OFF);
      ESP.eraseConfig();
      ESP.reset();
    }
    else
    {
      LOG1("Init file <%s> found.\n\n", initFilename);
    }

    LittleFS.end();
  }

  showChipInfo();

  memcpy(&appcfgRD, &appcfg, sizeof(appcfg));
  loadConfig();
  memcpy(&appcfgWR, &appcfg, sizeof(appcfg));

  writeConfig();
  printConfig(appcfg);

  configTime(appcfg.ntp_timezone, appcfg.ntp_server1, appcfg.ntp_server2, appcfg.ntp_server3);

  wifiHandler.setup();
  strcpy(appLocalIpBuffer, WiFi.localIP().toString().c_str());

  if (appcfg.telnet_enabled)
  {
    TelnetStream.begin();
  }

  if (appcfg.ntp_enabled)
  {
    bool timeNotSet = true;
    struct tm timeinfo;

    for (int i = 0; i < 15; i++)
    {
      time_t now = time(nullptr);
      localtime_r(&now, &timeinfo);
      if (timeinfo.tm_year >= 120)
      {
        timeNotSet = false;
        appStartTime = now - (millis() / 1000);
        break;
      }
      else
      {
        delay(1000);
      }
    }

    Serial.printf("Time              : %s\n", appDateTime());
    Serial.print("Timezone          : ");
    Serial.println(getenv("TZ"));

    if (timeNotSet)
    {
      Serial.println("Time not set");
    }
  }

  Serial.println();
  otaHandler.setup();
  webHandler.setup();

  LOG0( "Setup done.\n" );
}

App::App()
{
  sprintf(initFilename, "/%08X.ini", ESP.getChipId());
  defaultConfig();
  initialized = true;
  doSystemRestart = false;
}

void App::defaultConfig()
{
  strncpy(appcfg.wifi_ssid, DEFAULT_WIFI_SSID, 63);
  strncpy(appcfg.wifi_password, DEFAULT_WIFI_PASSWORD, 63);
  appcfg.wifi_mode = DEFAULT_WIFI_MODE;

  appcfg.net_mode = DEFAULT_NET_MODE;
  strncpy(appcfg.net_host, DEFAULT_NET_HOST, 63);
  strncpy(appcfg.net_mask, DEFAULT_NET_MASK, 63);
  strncpy(appcfg.net_gateway, DEFAULT_NET_GATEWAY, 63);
  strncpy(appcfg.net_dns, DEFAULT_NET_DNS, 63);

  appcfg.ota_enabled = DEFAULT_OTA_ENABLED;
  strncpy(appcfg.ota_hostname, DEFAULT_OTA_HOSTNAME, 63);
  strncpy(appcfg.ota_password, DEFAULT_OTA_PASSWORD, 63);

  strncpy(appcfg.admin_password, DEFAULT_ADMIN_PASSWORD, 63);

  appcfg.mqtt_enabled = DEFAULT_MQTT_ENABLED;
  strncpy(appcfg.mqtt_clientid, DEFAULT_MQTT_CLIENTID, 63);
  strncpy(appcfg.mqtt_host, DEFAULT_MQTT_HOST, 63);
  appcfg.mqtt_port = DEFAULT_MQTT_PORT;
  appcfg.mqtt_useauth = DEFAULT_MQTT_USEAUTH;
  strncpy(appcfg.mqtt_user, DEFAULT_MQTT_USER, 63);
  strncpy(appcfg.mqtt_password, DEFAULT_MQTT_PASSWORD, 63);
  strncpy(appcfg.mqtt_intopic, DEFAULT_MQTT_INTOPIC, 63);
  strncpy(appcfg.mqtt_outtopic, DEFAULT_MQTT_OUTTOPIC, 63);

  appcfg.telnet_enabled = DEFAULT_TELNET_ENABLED;

  strncpy(appcfg.mqtt_topic_json, DEFAULT_MQTT_TOPIC_JSON, 63);
  appcfg.mqtt_sending_interval = DEFAULT_MQTT_SENDING_INTERVAL;

  appcfg.telnet_enabled = DEFAULT_TELNET_ENABLED;

  appcfg.ntp_enabled = DEFAULT_NTP_ENABLED;
  strncpy(appcfg.ntp_timezone, DEFAULT_NTP_TIMEZONE, 63);
  strncpy(appcfg.ntp_server1, DEFAULT_NTP_SERVER1, 63);
  strncpy(appcfg.ntp_server2, DEFAULT_NTP_SERVER2, 63);
  strncpy(appcfg.ntp_server3, DEFAULT_NTP_SERVER3, 63);

  memcpy(&appcfgWR, &appcfg, sizeof(appcfg));
  memcpy(&appcfgRD, &appcfg, sizeof(appcfg));
}

void App::firmwareReset()
{
  if (LittleFS.begin())
  {
    LOG0("Removing init file\n");
    LittleFS.remove(initFilename);
    LittleFS.end();
  }
  delayedSystemRestart();
}

void App::formatFS()
{
  wifiLedOn();
  ESP.eraseConfig();

  if (LittleFS.begin())
  {
    LOG0("File system format started...\n");
    LittleFS.format();
    LOG0("File system format finished.\n");
    LittleFS.end();
  }
  else
  {
    LOG0("\nERROR: format filesystem.\n");
  }
  wifiLedOff();
}

void App::restartSystem()
{
  // watchdogTicker.detach();
  ESP.eraseConfig();
  LOG0("*** restarting system ***\n");
  delay(2000);
  ESP.restart();
  delay(2000);
  ESP.reset();
}

void App::loadConfig()
{
  if (!LittleFS.begin())
  {
    LOG0("ERROR: Failed to mount file system");
  }
  else
  {
    if (LittleFS.exists(APP_CONFIG_FILE_JSON))
    {
      if (loadJsonConfig(APP_CONFIG_FILE_JSON) == false)
      {
        memcpy(&appcfg, &appcfgRD, sizeof(appcfg));
      }
      else
      {
        LOG0("ERROR: loading config.json file. Using default config.\n");
      }
    }
    else
    {
      LOG0("WARNING: appcfg file " APP_CONFIG_FILE_JSON " does not exist. Using default appcfg.\n");
    }
    LittleFS.end();
  }

  //------------------------------------------------------------
  sprintf(buffer, appcfg.wifi_ssid, ESP.getChipId());
  strcpy(appcfg.wifi_ssid, buffer);
  sprintf(buffer, appcfg.ota_hostname, ESP.getChipId());
  strcpy(appcfg.ota_hostname, buffer);
  sprintf(buffer, appcfg.mqtt_clientid, ESP.getChipId());
  strcpy(appcfg.mqtt_clientid, buffer);
  //------------------------------------------------------------
}

void App::writeConfig()
{
  if (!LittleFS.begin())
  {
    LOG0("ERROR: Failed to mount file system");
  }
  else
  {
    LOG0("Writing " APP_CONFIG_FILE_JSON " file.\n");
    File configJson = LittleFS.open(APP_CONFIG_FILE_JSON, "w");
    uJson j = uJson(configJson);

    j.writeHeader();
    j.writeEntry(A_wifi_ssid, appcfgWR.wifi_ssid);
    j.writeEntry(A_wifi_password, appcfgWR.wifi_password);
    j.writeEntry(A_wifi_mode, appcfgWR.wifi_mode);

    j.writeEntry(A_net_mode, appcfgWR.net_mode);
    j.writeEntry(A_net_host, appcfgWR.net_host);
    j.writeEntry(A_net_mask, appcfgWR.net_mask);
    j.writeEntry(A_net_gateway, appcfgWR.net_gateway);
    j.writeEntry(A_net_dns, appcfgWR.net_dns);

    j.writeEntry(A_ota_enabled, appcfgWR.ota_enabled);
    j.writeEntry(A_ota_hostname, appcfgWR.ota_hostname);
    j.writeEntry(A_ota_password, appcfgWR.ota_password);

    j.writeEntry(A_admin_password, appcfgWR.admin_password);

    j.writeEntry(A_mqtt_enabled, appcfgWR.mqtt_enabled);
    j.writeEntry(A_mqtt_clientid, appcfgWR.mqtt_clientid);
    j.writeEntry(A_mqtt_host, appcfgWR.mqtt_host);
    j.writeEntry(A_mqtt_port, appcfgWR.mqtt_port);
    j.writeEntry(A_mqtt_intopic, appcfgWR.mqtt_intopic);
    j.writeEntry(A_mqtt_outtopic, appcfgWR.mqtt_outtopic);
    j.writeEntry(A_mqtt_useauth, appcfgWR.mqtt_useauth);
    j.writeEntry(A_mqtt_user, appcfgWR.mqtt_user);
    j.writeEntry(A_mqtt_password, appcfgWR.mqtt_password);

    j.writeEntry(A_mqtt_topic_json, appcfgWR.mqtt_topic_json);
    j.writeEntry(A_mqtt_sending_interval, appcfgWR.mqtt_sending_interval);

    j.writeEntry(A_telnet_enabled, appcfgWR.telnet_enabled);

    j.writeEntry(A_ntp_enabled, appcfgWR.ntp_enabled);
    j.writeEntry(A_ntp_timezone, appcfgWR.ntp_timezone);
    j.writeEntry(A_ntp_server1, appcfgWR.ntp_server1);
    j.writeEntry(A_ntp_server2, appcfgWR.ntp_server2);
    j.writeEntry(A_ntp_server3, appcfgWR.ntp_server3);

    j.writeFooter();
    configJson.close();

    FSInfo fs_info;
    LittleFS.info(fs_info);

    fsTotalBytes = fs_info.totalBytes;
    fsUsedBytes = fs_info.usedBytes;

    Serial.printf("\n--- SPIFFS Info ---\ntotal bytes = %d\n", fs_info.totalBytes);
    Serial.printf("used bytes = %d\n", fs_info.usedBytes);
    Serial.printf("block size = %d\n", fs_info.blockSize);
    Serial.printf("page size = %d\n", fs_info.pageSize);
    Serial.printf("max open files = %d\n", fs_info.maxOpenFiles);
    Serial.printf("max path length = %d\n", fs_info.maxPathLength);

    LittleFS.end();
  }
}

bool App::loadJsonConfig(const char *filename)
{
  bool readError = false;
  char attributeName[128];

  File tmpConfig = LittleFS.open(filename, "r");

  uJson j = uJson(tmpConfig);

  if (j.readHeader())
  {
    memcpy(&appcfgRD, &appcfg, sizeof(appcfg));

    while (readError == false && j.readAttributeName(attributeName) == true)
    {
      readError |= j.readEntryChars(attributeName, A_wifi_ssid, appcfgRD.wifi_ssid);
      readError |= j.readEntryChars(attributeName, A_wifi_password, appcfgRD.wifi_password);
      readError |= j.readEntryInteger(attributeName, A_wifi_mode, &appcfgRD.wifi_mode);

      readError |= j.readEntryInteger(attributeName, A_net_mode, &appcfgRD.net_mode);
      readError |= j.readEntryChars(attributeName, A_net_host, appcfgRD.net_host);
      readError |= j.readEntryChars(attributeName, A_net_mask, appcfgRD.net_mask);
      readError |= j.readEntryChars(attributeName, A_net_gateway, appcfgRD.net_gateway);
      readError |= j.readEntryChars(attributeName, A_net_dns, appcfgRD.net_dns);

      readError |= j.readEntryBoolean(attributeName, A_ota_enabled, &appcfgRD.ota_enabled);
      readError |= j.readEntryChars(attributeName, A_ota_hostname, appcfgRD.ota_hostname);
      readError |= j.readEntryChars(attributeName, A_ota_password, appcfgRD.ota_password);

      readError |= j.readEntryChars(attributeName, A_admin_password, appcfgRD.admin_password);

      readError |= j.readEntryBoolean(attributeName, A_mqtt_enabled, &appcfgRD.mqtt_enabled);
      readError |= j.readEntryChars(attributeName, A_mqtt_clientid, appcfgRD.mqtt_clientid);
      readError |= j.readEntryChars(attributeName, A_mqtt_host, appcfgRD.mqtt_host);
      readError |= j.readEntryInteger(attributeName, A_mqtt_port, &appcfgRD.mqtt_port);
      readError |= j.readEntryChars(attributeName, A_mqtt_intopic, appcfgRD.mqtt_intopic);
      readError |= j.readEntryChars(attributeName, A_mqtt_outtopic, appcfgRD.mqtt_outtopic);
      readError |= j.readEntryBoolean(attributeName, A_mqtt_useauth, &appcfgRD.mqtt_useauth);
      readError |= j.readEntryChars(attributeName, A_mqtt_user, appcfgRD.mqtt_user);
      readError |= j.readEntryChars(attributeName, A_mqtt_password, appcfgRD.mqtt_password);

      readError |= j.readEntryChars(attributeName, A_mqtt_topic_json, appcfgRD.mqtt_topic_json);
      readError |= j.readEntryULong(attributeName, A_mqtt_sending_interval, &appcfgRD.mqtt_sending_interval);

      readError |= j.readEntryBoolean(attributeName, A_telnet_enabled, &appcfgRD.telnet_enabled);

      readError |= j.readEntryBoolean(attributeName, A_ntp_enabled, &appcfgRD.ntp_enabled);
      readError |= j.readEntryChars(attributeName, A_ntp_timezone, appcfgRD.ntp_timezone);
      readError |= j.readEntryChars(attributeName, A_ntp_server1, appcfgRD.ntp_server1);
      readError |= j.readEntryChars(attributeName, A_ntp_server2, appcfgRD.ntp_server2);
      readError |= j.readEntryChars(attributeName, A_ntp_server3, appcfgRD.ntp_server3);
    }
  }

  tmpConfig.close();
  return readError;
}

void App::wifiLedOn()
{
#ifdef WIFI_LED
  wifiLedState = 1;
  digitalWrite(WIFI_LED, WIFI_LED_ON);
  ledStateTimestamp = millis();
#endif
}

void App::wifiLedOff()
{
#ifdef WIFI_LED
  wifiLedState = 0;
  digitalWrite(WIFI_LED, WIFI_LED_OFF);
  ledStateTimestamp = millis();
#endif
}

void App::wifiLedToggle()
{
#ifdef WIFI_LED
  if (wifiLedState == 1)
  {
    wifiLedOff();
  }
  else
  {
    wifiLedOn();
  }
#endif
}

void App::delayedSystemRestart()
{
  doSystemRestart = true;
  systemRestartTimestamp = millis();
  systemRestartCounter = 6;
  LOG0("*** delayedSystemRestart ***\n");
}

void App::loop()
{
  static unsigned int counter = 0;
  static time_t timestamp = 0;

  if (appcfg.ota_enabled == true)
  {
    ArduinoOTA.handle();
  }

  wifiHandler.handle();

  if ( otaHandler.inProgress == false)
  {
    time_t currentTimestamp = millis();
    webHandler.handle();

    if ( appcfg.telnet_enabled )
    {
      TelnetStream.handle();
    }

    if ((currentTimestamp - timestamp) >= 2000)
    {
      Serial.printf("\r%d ", counter++);
      timestamp = currentTimestamp;

      char currentLocalIp[32];
      strcpy(currentLocalIp, WiFi.localIP().toString().c_str());
      if (strcmp(appLocalIpBuffer, currentLocalIp) != 0)
      {
        Serial.printf("\n[%s] Local IP-Address has changed to: %s\n", appDateTime(), currentLocalIp);
        strcpy(appLocalIpBuffer, currentLocalIp);
      }
    }

    if (doSystemRestart && ((currentTimestamp - systemRestartTimestamp) > 1000))
    {
      systemRestartCounter--;

      if (systemRestartCounter > 0)
      {
        LOG1("*** system restart in %us ***\n", systemRestartCounter);
      }
      else
      {
        LOG1("*** do system restart *** (%lu)\n", (currentTimestamp - systemRestartTimestamp));
        writeConfig();
        restartSystem();
      }

      systemRestartTimestamp = millis();
    }
  }
}

void App::printConfig(AppConfig ac)
{
  Serial.println();
  Serial.println("--- App appcfguration -----------------------------------");
  Serial.println("  Security:");
  Serial.printf("    Admin password: %s\n", ac.admin_password);
  Serial.println("\n  WiFi:");
  Serial.printf("    SSID: %s\n", ac.wifi_ssid);
  Serial.printf("    Password: %s\n", ac.wifi_password);
  Serial.printf("    Mode: %s\n", (ac.wifi_mode == 1) ? "Station" : "Access Point");
  Serial.println("\n  Network:");
  Serial.printf("    Mode: %s\n", (ac.net_mode == NET_MODE_DHCP) ? "DHCP" : "Static");
  Serial.printf("    host address: %s\n", ac.net_host);
  Serial.printf("    gateway: %s\n", ac.net_gateway);
  Serial.printf("    netmask: %s\n", ac.net_mask);
  Serial.printf("    dns server: %s\n", ac.net_dns);
  Serial.println("\n  OTA:");
  Serial.printf("    Enabled: %s\n", (ac.ota_enabled ? "true" : "false"));
  Serial.printf("    Hostname: %s\n", ac.ota_hostname);
  Serial.printf("    Password: %s\n", ac.ota_password);

  Serial.println("\n  MQTT:");
  Serial.printf("    Enabled: %s\n", (ac.mqtt_enabled ? "true" : "false"));
  Serial.printf("    Client ID: %s\n", ac.mqtt_clientid);
  Serial.printf("    Host: %s\n", ac.mqtt_host);
  Serial.printf("    Port: %d\n", ac.mqtt_port);
  Serial.printf("    Use Auth: %s\n", (ac.mqtt_useauth ? "true" : "false"));
  Serial.printf("    User: %s\n", ac.mqtt_user);
  Serial.printf("    Password: %s\n", ac.mqtt_password);
  Serial.printf("    In Topic: %s\n", ac.mqtt_intopic);
  Serial.printf("    Out Topic: %s\n", ac.mqtt_outtopic);
  Serial.printf("    Topic JSON: %s\n", ac.mqtt_topic_json);
  Serial.printf("    Sending Interval: %ld\n", ac.mqtt_sending_interval);

  Serial.println("\n  Telnet:");
  Serial.printf("    Enabled: %s\n", (ac.telnet_enabled ? "true" : "false"));

  Serial.println("\n  NTP:");
  Serial.printf("    Enabled: %s\n", (ac.ntp_enabled ? "true" : "false"));
  Serial.printf("    Timezone: %s\n", ac.ntp_timezone);
  Serial.printf("    NTP Server 1: %s\n", ac.ntp_server1);
  Serial.printf("    NTP Server 2: %s\n", ac.ntp_server2);
  Serial.printf("    NTP Server 3: %s\n", ac.ntp_server3);

  Serial.println("---------------------------------------------------------");
  Serial.println();
}
