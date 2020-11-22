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

ADC_MODE(ADC_VCC);

// Callback function is called only when a valid code is received.
void showCode(unsigned int period, unsigned long address,
              unsigned long groupBit, unsigned long unit,
              unsigned long switchType)
{
  mqttHandler.sendCommand( address, unit, switchType == 1 );
  // TLOG1( appcfg.mqtt_outtopic, address, unit );
  TelnetStream.printf( "%s\n", switchType == 1 ? "ON" : "OFF" );

  // Print the received code.
  Serial.print("\nCode: ");
  Serial.print(address);
  Serial.print(" Period: ");
  Serial.println(period);
  Serial.print(" unit: ");
  Serial.println(unit);
  Serial.print(" groupBit: ");
  Serial.println(groupBit);
  Serial.print(" switchType: ");
  Serial.println(switchType);
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
