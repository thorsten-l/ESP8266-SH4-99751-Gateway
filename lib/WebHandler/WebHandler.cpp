#include "WebHandler.hpp"
#include <App.hpp>
#include <Arduino.h>
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <Esp.h>
#include <FS.h>
#include <LittleFS.h>
#include <OtaHandler.hpp>
#include <Util.hpp>
#include <WifiHandler.hpp>

#include "pages/Pages.h"

#include <html/footer.h>
#include <html/header.h>
#include <html/header2.h>
#include <html/header3.h>
#include <html/setup.h>

WebHandler webHandler;
ESP8266WebServer server(80);
size_t fsTotalBytes;
size_t fsUsedBytes;

const char *getJsonStatus(WiFiClient *client)
{
  time_t now = time(nullptr);

  int remotePort = 0;
  char remoteAddress[32] = {0};

  if (client != NULL)
  {
    remotePort = client->remotePort();
    strncpy(remoteAddress, client->remoteIP().toString().c_str(), 31);
  }
  else
  {
    remotePort = server.client().remotePort();
    strncpy(remoteAddress, server.client().remoteIP().toString().c_str(), 31);
  }

  sprintf(buffer,
          "{"
          "\"millis\":%lu,"
          "\"utc\":%lu,"
          "\"ctime\":\"%s\","
          "\"timezone\":\"%s\","
          "\"uptime\":\"%s\","
          "\"host_name\":\"%s.local\","
          "\"esp_full_version\":\"%s\","
          "\"esp_core_version\":\"%s\","
          "\"esp_sdk_version\":\"%s\","
          "\"platformio_env\":\"%s\","
          "\"platformio_platform\":\"%s\","
          "\"platformio_framework\":\"%s\","
          "\"arduino_board\":\"%s\","
          "\"chip_id\":\"%08X\","
          "\"cpu_freq\":\"%dMhz\","
          "\"flash_size\":%u,"
          "\"flash_speed\":%u,"
          "\"ide_size\":%u,"
          "\"fw_name\":\"%s\","
          "\"fw_version\":\"%s\","
          "\"build_date\":\"%s\","
          "\"build_time\":\"%s\","
          "\"wifi_ssid\":\"%s\","
          "\"wifi_reconnect_counter\":%d,"
          "\"wifi_channel\":%d,"
          "\"wifi_phy_mode\":\"%s\","
          "\"wifi_mac_address\":\"%s\","
          "\"wifi_hostname\":\"%s\","
          "\"wifi_ip_address\":\"%s\","
          "\"wifi_gateway_ip\":\"%s\","
          "\"wifi_subnet_mask\":\"%s\","
          "\"wifi_dns_ip\":\"%s\","
          "\"spiffs_total\":%u,"
          "\"spiffs_used\":%u,"
          "\"free_heap\":%u,"
          "\"sketch_size\":%u,"
          "\"free_sketch_space\":%u,"
          "\"remote_client_ip\":\"%s\","
          "\"remote_client_port\":%u"
          "}",
          millis(), now, strtok(ctime(&now), "\n"), appcfg.ntp_timezone, appUptime(), wifiHandler.getHostname(),
          ESP.getFullVersion().c_str(), ESP.getCoreVersion().c_str(), ESP.getSdkVersion(), PIOENV, PIOPLATFORM,
          PIOFRAMEWORK, ARDUINO_BOARD, ESP.getChipId(), ESP.getCpuFreqMHz(), ESP.getFlashChipRealSize(),
          ESP.getFlashChipSpeed(), ESP.getFlashChipSize(), APP_NAME, APP_VERSION, __DATE__, __TIME__, appcfg.wifi_ssid,
          wifiHandler.getConnectCounter(), WiFi.channel(), wifiHandler.getPhyMode(), wifiHandler.getMacAddress(),
          WiFi.hostname().c_str(), WiFi.localIP().toString().c_str(), WiFi.gatewayIP().toString().c_str(),
          WiFi.subnetMask().toString().c_str(), WiFi.dnsIP().toString().c_str(), fsTotalBytes, fsUsedBytes,
          ESP.getFreeHeap(), ESP.getSketchSize(), ESP.getFreeSketchSpace(), remoteAddress, remotePort);

  return buffer;
}

void sendHeader(const char *title, bool sendMetaRefresh, const char *style)
{
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.chunkedResponseModeStart(200, "text/html");
  server.sendContent_P(header_html);

  if (sendMetaRefresh)
  {
    server.sendContent_P(META_REFRESH30);
  }

  if (style != nullptr)
  {
    server.sendContent_P(style);
  }

  sprintf(buffer, "<title>%s</title>\n", title);
  server.sendContent(buffer);
  server.sendContent_P(header2_html);
  server.sendContent(title);
  server.sendContent_P(header3_html);
}

void sendHeader(const char *title, bool sendMetaRefresh)
{
  sendHeader(title, sendMetaRefresh, nullptr);
}

void sendHeader(const char *title)
{
  sendHeader(title, false);
}

void sendHeaderNoCache()
{
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "0");
}

void sendAuthentication()
{
  if (!server.authenticate("admin", appcfg.admin_password))
  {
    return server.requestAuthentication();
  }
  sendHeaderNoCache();
}

void sendFooter()
{
  server.sendContent_P(footer_html);
  server.chunkedResponseFinalize();
  server.client().stop();
}

void sendPrintf(const char *format, ...)
{
  va_list myargs;
  va_start(myargs, format);
  vsprintf(buffer, format, myargs);
  va_end(myargs);
  server.sendContent(buffer);
}

void sendPrint(const char *message)
{
  server.sendContent(message);
}

void sendLegend(const char *name)
{
  sendPrintf("<legend>%s</legend>", name);
}

WebHandler::WebHandler()
{
  initialized = false;
}

void WebHandler::setup()
{
  LOG0("HTTP server setup...\n");

  fsTotalBytes = 0;
  fsUsedBytes = 0;

  if (LittleFS.begin())
  {
    FSInfo fs_info;
    LittleFS.info(fs_info);
    fsTotalBytes = fs_info.totalBytes;
    fsUsedBytes = fs_info.usedBytes;
    LittleFS.end();
  }

  server.on("/", handleRootPage);
  server.on("/info.html", handleInfoPage);
  server.on("/setup.html", handleSetupPage);
  server.on("/maintenance.html", handleMaintenancePage);
  server.on("/savecfg.html", handleSaveConfigPage);
  server.on("/config-backup", handleBackupConfiguration);
  server.on("/config-restore", HTTP_POST, handleRestoreConfiguration, handleConfigFileUpload);
  server.on("/update-firmware", HTTP_POST, handleFirmwareUploadSuccess, handleFirmwareUpload);
  server.on("/reset-firmware", handleResetFirmware);

  server.on("/info", []() {
    server.sendHeader("Access-Control-Allow-Origin", "*");
    sendHeaderNoCache();
    String message(getJsonStatus(NULL));
    server.send(200, "application/json", message);
  });

  server.on("/last10", []() {
    server.sendHeader("Access-Control-Allow-Origin", "*");
    sendHeaderNoCache();
    server.chunkedResponseModeStart(200, "application/json");
    sendPrint("{ \"last10\" : [ ");

    int msi = messageStartIndex;
    bool firstMessage = true;

    while (msi != messageEndIndex)
    {
      char *mb = messageBuffer + (msi * (MAX_MESSAGE_LENGTH + 1));
      if ( firstMessage == false )
      {
        sendPrint(",");
      }
      else 
      {
        firstMessage = false;
      }
      sendPrint("\"");
      sendPrint(mb);
      sendPrint("\"");
      msi += 1;
      msi %= MESSAGE_BUFFER_LINES;
    }

    sendPrint("] }");
    server.chunkedResponseFinalize();
    server.client().stop();
  });

  if (appcfg.ota_enabled == false)
  {
    MDNS.begin(appcfg.ota_hostname);
  }

  MDNS.addService("http", "tcp", 80);
  MDNS.addServiceTxt("http", "tcp", "path", "/");
  MDNS.addServiceTxt("http", "tcp", "fw_name", APP_NAME);
  MDNS.addServiceTxt("http", "tcp", "fw_version", APP_VERSION);

  if (appcfg.ota_enabled == false)
  {
    MDNS.update();
  }

  server.begin();

  LOG0("HTTP server started\n");
  initialized = true;
}

void WebHandler::handle()
{
  server.handleClient();
}
