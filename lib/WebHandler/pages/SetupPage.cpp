#include "Pages.h"

#include <ConfigAttributes.hpp>
#include <WifiHandler.hpp>
#include <Util.hpp>
#include <html/setup.h>

const char *selected = "selected";
const char *checked = "checked";
const char *blank = "";

static const char *setupProcessor(const char *var)
{

  if (strcmp(var, A_admin_password) == 0)
    return appcfg.admin_password;

  // Wifi

  if (strcmp(var, "wifi_mode_ap") == 0 && appcfg.wifi_mode == WIFI_AP)
    return selected;
  if (strcmp(var, "wifi_mode_station") == 0 && appcfg.wifi_mode == WIFI_STA)
    return selected;

  if (strcmp(var, "scanned_network_options") == 0)
  {
    ListNode *node = wifiHandler.getScannedNetworks();
    String options = "";

    if ( node == NULL)
    {
      LOG0("node==NULL\n");
    }

    while (node != NULL)
    {
      options += F("<option>");
      options += String(*node->value);
      options += F("</option>");
      node = node->next;
    }

    fillBuffer( options.c_str());
    return buffer;
  }

  if (strcmp(var, A_wifi_ssid) == 0)
    return appcfg.wifi_ssid;
  if (strcmp(var, A_wifi_password) == 0)
    return appcfg.wifi_password;

  // Network
  if (strcmp(var, "net_mode_dhcp") == 0 && appcfg.net_mode == NET_MODE_DHCP)
    return selected;
  if (strcmp(var, "net_mode_static") == 0 && appcfg.net_mode == NET_MODE_STATIC)
    return selected;
  if (strcmp(var, A_net_host) == 0)
    return appcfg.net_host;
  if (strcmp(var, A_net_gateway) == 0)
    return appcfg.net_gateway;
  if (strcmp(var, A_net_mask) == 0)
    return appcfg.net_mask;
  if (strcmp(var, A_net_dns) == 0)
    return appcfg.net_dns;

  // OTA
  if (strcmp(var, A_ota_hostname) == 0)
    return appcfg.ota_hostname;
  if (strcmp(var, A_ota_password) == 0)
    return appcfg.ota_password;
  if (strcmp(var, A_ota_enabled) == 0 && appcfg.ota_enabled == true)
    return checked;

  // MQTT
  if (strcmp(var, A_mqtt_enabled) == 0 && appcfg.mqtt_enabled == true)
    return checked;
  if (strcmp(var, A_mqtt_clientid) == 0)
    return appcfg.mqtt_clientid;
  if (strcmp(var, A_mqtt_host) == 0)
    return appcfg.mqtt_host;
  if (strcmp(var, A_mqtt_port) == 0)
  {
    sprintf(buffer, "%d", appcfg.mqtt_port);
    return buffer;
  }
  if (strcmp(var, A_mqtt_useauth) == 0 && appcfg.mqtt_useauth == true)
    return checked;
  if (strcmp(var, A_mqtt_user) == 0)
    return appcfg.mqtt_user;
  if (strcmp(var, A_mqtt_password) == 0)
    return appcfg.mqtt_password;
  if (strcmp(var, A_mqtt_intopic) == 0)
    return appcfg.mqtt_intopic;
  if (strcmp(var, A_mqtt_outtopic) == 0)
    return appcfg.mqtt_outtopic;

  if (strcmp(var, A_mqtt_topic_json) == 0)
    return appcfg.mqtt_topic_json;
  if (strcmp(var, A_mqtt_sending_interval) == 0)
  {
    sprintf(buffer, "%lu", appcfg.mqtt_sending_interval);
    return buffer;
  }

  // Telnet
  if (strcmp(var, A_telnet_enabled) == 0 && appcfg.telnet_enabled == true)
    return checked;

  // NTP
  if (strcmp(var, A_ntp_enabled) == 0 && appcfg.ntp_enabled == true)
    return checked;
  if (strcmp(var, A_ntp_timezone) == 0)
    return appcfg.ntp_timezone;
  if (strcmp(var, A_ntp_server1) == 0)
    return appcfg.ntp_server1;
  if (strcmp(var, A_ntp_server2) == 0)
    return appcfg.ntp_server2;
  if (strcmp(var, A_ntp_server3) == 0)
    return appcfg.ntp_server3;

  if (strcmp(var, "millis") == 0)
  {
    sprintf(buffer, "%lu", millis());
    return buffer;
  }

  return nullptr;
}

void handleSetupPage()
{
  sendAuthentication();
  sendHeader(APP_NAME " - Setup");
  sendHtmlTemplate( SETUP_HTML_TEMPLATE, setupProcessor );
  sendFooter();
}
