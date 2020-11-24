#include <App.hpp>
#include <Arduino.h>
#include <ArduinoOTA.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <NewRemoteReceiver.h>
#include <OtaHandler.hpp>
#include <Util.hpp>
#include <WifiHandler.hpp>
#include <MqttHandler.hpp>
#include <TelnetStream.h>
#include <LinkedList.hpp>

ADC_MODE(ADC_VCC);

// Callback function is called only when a valid code is received.
void showCode(unsigned int period, unsigned long address,
              unsigned long groupBit, unsigned long unit,
              unsigned long switchType)
{
  static bool messageStartRollover = false;

  mqttHandler.sendCommand( address, unit, switchType == 1 );
  sprintf( buffer2, appcfg.mqtt_outtopic, address, unit );
  sprintf( buffer, "(%s) %s %s", appDateTime(), buffer2, 
           switchType == 1 ? "ON" : "OFF" );
  TelnetStream.println();
  TelnetStream.println( buffer );

// put
  char *mb = messageBuffer + (messageEndIndex*(MAX_MESSAGE_LENGTH+1));
  
  strncpy( mb, buffer, MAX_MESSAGE_LENGTH );
  mb[MAX_MESSAGE_LENGTH] = 0;

  messageEndIndex += 1;
  messageEndIndex %= MESSAGE_BUFFER_LINES;

  if ( messageEndIndex == 0 )
  {
    messageStartRollover = true;
  }

  if ( messageStartRollover )
  {
    messageStartIndex += 1;
    messageStartIndex %= MESSAGE_BUFFER_LINES;
  }

  // Print the received code.
  Serial.print("Code: ");
  Serial.print(address);
  Serial.print(" unit: ");
  Serial.print(unit);
  Serial.print(" Period: ");
  Serial.print(period);
  Serial.print(" groupBit: ");
  Serial.print(groupBit);
  Serial.print(" switchType: ");
  Serial.println(switchType);
  Serial.println();

  // Telnet the received code.
  TelnetStream.print("Code: ");
  TelnetStream.print(address);
  TelnetStream.print(" unit: ");
  TelnetStream.print(unit);
  TelnetStream.print(" Period: ");
  TelnetStream.print(period);
  TelnetStream.print(" groupBit: ");
  TelnetStream.print(groupBit);
  TelnetStream.print(" switchType: ");
  TelnetStream.println(switchType);
  TelnetStream.println();
}

void setup()
{
  NewRemoteReceiver::init(4, 2, showCode);
  app.setup();
  LOG0("Receiver initialized\n");
}

void loop()
{
  app.loop();

  if (otaHandler.inProgress == false)
  {
    mqttHandler.handle(millis());
  }

  delay(20);
}
