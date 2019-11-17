#ifndef __WIFI_HANDLER_H__
#define __WIFI_HANDLER_H__

#include <Arduino.h>

class WifiHandler
{
private:
  bool connected = false;
  int connectCounter = 0;
  byte mac[6];
  char macAddress[20];
  char hostname[64];

public:
  void wifiInitStationMode();
  int getConnectCounter();
  const char* getMacAddress();
  const char* getPhyMode();
  const char* getHostname();
};

extern WifiHandler wifiHandler;

#endif
