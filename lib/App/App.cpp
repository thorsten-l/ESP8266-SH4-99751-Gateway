
#include "App.hpp"
#include <Arduino.h>
#include <ArduinoOTA.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <Esp.h>
#include <TelnetStream.h>

#include <WifiHandler.hpp>
#include <OtaHandler.hpp>
#include <Util.hpp>

#include <FS.h>
#include <LittleFS.h>

time_t appStartTime = 0l;
struct tm appTimeinfo;
char appUptimeBuffer[64];
char appLocalIpBuffer[32];
char appDateTimeBuffer[32];

const char *appDateTime()
{
  time_t now = time(nullptr);
  localtime_r( &now, &appTimeinfo );
  sprintf( appDateTimeBuffer, "%4d-%02d-%02d %02d:%02d:%02d", 
  appTimeinfo.tm_year+1900, appTimeinfo.tm_mon+1, appTimeinfo.tm_mday,
  appTimeinfo.tm_hour, appTimeinfo.tm_min, appTimeinfo.tm_sec );
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
  sprintf( appUptimeBuffer, 
    "%ld days, %d hours, %d minutes, %d seconds",
    uptimeDays, uptimeHours, uptimeMinutes, uptimeSeconds );
  return appUptimeBuffer;
}

void appShowHeader(Stream& out)
{
  out.println( "\n\n" APP_NAME " - " APP_VERSION " - " APP_AUTHOR);
  out.println("BUILD: " __DATE__ " " __TIME__);
  out.println("PIOENV: " PIOENV );
  out.println("PIOPLATFORM: " PIOPLATFORM );
  out.println("PIOFRAMEWORK: " PIOFRAMEWORK );
  out.printf( "ESP SDK Version: %s\n", ESP.getSdkVersion() );
  out.printf( "ESP Core Version: %s\n\n", ESP.getCoreVersion().c_str() );
}

