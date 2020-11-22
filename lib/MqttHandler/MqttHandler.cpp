#include <App.hpp>
#include <Arduino.h>
#include <PubSubClient.h>
#include <WiFiClient.h>
#include "MqttHandler.hpp"

MqttHandler mqttHandler;

static WiFiClient wifiClient;
static PubSubClient client(wifiClient);
static long lastReconnectAttempt = 0;

MqttHandler::MqttHandler() { initialized = false; }

bool MqttHandler::reconnect()
{
  if (( appcfg.mqtt_useauth &&
       client.connect(appcfg.mqtt_clientid, appcfg.mqtt_user,
                      appcfg.mqtt_password)) ||
      (!appcfg.mqtt_useauth && client.connect(appcfg.mqtt_clientid)))
  {
    LOG0("mqtt broker connected\n");
  }

  return client.connected();
}

void MqttHandler::setup()
{
  LOG0("MQTT Setup...\n");
  client.setServer(appcfg.mqtt_host, appcfg.mqtt_port);
  initialized = true;
}

void MqttHandler::handle(unsigned long now)
{
  if (appcfg.mqtt_enabled)
  {
    if (initialized == false)
    {
      setup();
    }

    if (!client.connected())
    {
      if (now - lastReconnectAttempt > 5000)
      {
        lastReconnectAttempt = now;

        if (reconnect())
        {
          lastReconnectAttempt = 0;
        }
      }
    }
    else
    {
      client.loop();
    }
  }
}

void MqttHandler::sendValue(const char *outtopic, const char *value)
{
  if (appcfg.mqtt_enabled && client.connected())
  {
    client.publish(outtopic, value);
    LOG1( "MQTT publish outtopic=%s value=%s\n", outtopic, value );
  }
}

static char topicBuffer[255];

void MqttHandler::sendCommand( unsigned long address, unsigned long unit, const bool value)
{
  if (appcfg.mqtt_enabled && client.connected())
  {
    sprintf( topicBuffer, appcfg.mqtt_outtopic, address, unit );    

    if ( value )
    {
      client.publish(topicBuffer, "ON" );
      LOG1( "MQTT publish outtopic=%s value=%s\n", topicBuffer, "ON" );
    }
    else
    {
      client.publish(topicBuffer, "OFF" );
      LOG1( "MQTT publish outtopic=%s value=%s\n", topicBuffer, "OFF" );
    }
  }
}
