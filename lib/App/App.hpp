#ifndef __APP_HPP__
#define __APP_HPP__

#include <Arduino.h>
#include <AppConfig.hpp>

#define LOG0( format ) Serial.printf( "(%lu) " format, millis())
#define LOG1( format, ... ) Serial.printf( "(%lu) " format, millis(), ##__VA_ARGS__ )
#define TLOG0( format ) TelnetStream.printf( "(%ld) " format, millis())
#define TLOG1( format, ... ) TelnetStream.printf( "(%ld) " format, millis(), ##__VA_ARGS__ )

#define APP_NAME "ESP8266 SH4-99751 Gateway"
#define APP_VERSION "0.3.1"
#define APP_AUTHOR "Dr. Thorsten Ludewig <t.ludewig@gmail.com>"
#define APP_CONFIG_FILE "/config.bin"

extern void appSetup();
extern void appLoop();
extern void appShowHeader(Stream& out);
extern const char *appUptime();
extern const char *appDateTime();

#endif
