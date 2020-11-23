#include "Pages.h"

void handleInfoPage()
{
  sendHeader( APP_NAME " - Info" );
  sendPrint( "<form class='pure-form'>");

  sendLegend( "Build" );
  sendPrint("<p>Date: " __DATE__ "</p>"
            "<p>Time: " __TIME__ "</p>");

  sendLegend( "RESTful API" );

  sendPrintf(
    "<p><a href='http://%s/info'>http://%s/info</a> - ESP8266 Info</p>",
    WiFi.localIP().toString().c_str(), WiFi.localIP().toString().c_str() );

  sendLegend( "Services" );
  sendPrintf("<p>OTA Enabled: %s</p>",
                     (appcfg.ota_enabled) ? "true" : "false");
  sendPrintf("<p>MQTT Enabled: %s</p>",
                     (appcfg.mqtt_enabled) ? "true" : "false");
  sendPrintf("<p>Telnet Enabled: %s</p>",
                     (appcfg.telnet_enabled) ? "true" : "false");
  sendPrintf("<p>NTP Enabled: %s</p>",
                     (appcfg.ntp_enabled) ? "true" : "false");

  sendPrint("</form>");

  sendFooter();
}
