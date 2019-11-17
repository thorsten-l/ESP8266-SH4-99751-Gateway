#include "WifiHandler.hpp"
#include <App.hpp>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <Util.hpp>

WifiHandler wifiHandler;

const char *phyModes[] = {"11B", "11G", "11N"};

void WifiHandler::wifiInitStationMode() {

  int bestChannel = 0;
  int bestRSSI = -100;

  Serial.println( "\nScanning WiFi networks...");
  int n = WiFi.scanNetworks();
  Serial.println( "done.");

  if (n == 0)
  {
    Serial.println("no networks found");
  }
  else
  {
    char scannedSSID[256];
    Serial.print(n);
    Serial.println(" networks found");
    
    for (int i = 0; i < n; ++i)
    {
      strncpy( scannedSSID, WiFi.SSID(i).c_str(), 255 );
      scannedSSID[255] = 0;

      int scannedRSSI = WiFi.RSSI(i);

      Serial.printf( "%2d: %s (%d,%d)%s\n", i+1,
                    scannedSSID, WiFi.channel(i), scannedRSSI,
                    (WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " " : "*");

      if ( strcmp(WIFI_SSID, scannedSSID) == 0 && scannedRSSI > bestRSSI )
      {
        bestChannel = WiFi.channel(i);
        bestRSSI = scannedRSSI;
      }

      delay( 5 );
    }
  }

  Serial.printf( "\nBest Channel = %d\n", bestChannel );
  Serial.printf( "Best RSSI = %d\n\n", bestRSSI );

  LOG0("Starting Wifi in Station Mode\n");
  sprintf( hostname, "%s", OTA_HOSTNAME );
  WiFi.persistent(false);
  WiFi.disconnect(true);
  delay(200);
  WiFi.mode(WIFI_STA);
  WiFi.hostname(hostname);
  WiFi.setSleepMode(WIFI_NONE_SLEEP);
  WiFi.begin(WIFI_SSID, WIFI_PASS, bestChannel );

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    alterPin(LED_BUILTIN);
    Serial.print(".");
  }
  Serial.println("\n");
  connectCounter++;

  Serial.println("WiFi connected to : " WIFI_SSID);
  Serial.printf("WiFi connection # : %d\n", connectCounter );
  Serial.printf("WiFi Channel      : %d\n", WiFi.channel());
  Serial.printf("WiFi phy mode     : %s\n", getPhyMode() );

  WiFi.macAddress(mac);
  sprintf( macAddress, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0],
                mac[1], mac[2], mac[3], mac[4], mac[5]);

  Serial.printf("WiFi MAC Address  : %s\n", macAddress);

  Serial.printf("WiFi Hostname     : %s\n", WiFi.hostname().c_str());
  Serial.print("WiFi IP-Address   : ");
  Serial.println(WiFi.localIP());
  Serial.print("WiFi Gateway-IP   : ");
  Serial.println(WiFi.gatewayIP());
  Serial.print("WiFi Subnetmask   : ");
  Serial.println(WiFi.subnetMask());
  Serial.print("WiFi DNS Server   : ");
  Serial.println(WiFi.dnsIP());
  Serial.println();
  digitalWrite(LED_BUILTIN, LOW);
}

int WifiHandler::getConnectCounter() {
  return connectCounter;
}

const char* WifiHandler::getMacAddress()
{
  return macAddress;
}

const char* WifiHandler::getPhyMode()
{
  return phyModes[WiFi.getPhyMode() - 1];
}

const char* WifiHandler::getHostname()
{
  return hostname;
}