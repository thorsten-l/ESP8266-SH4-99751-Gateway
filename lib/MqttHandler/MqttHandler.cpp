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
  if ((MQTT_USEAUTH &&
       client.connect(MQTT_CLIENTID, MQTT_USER,
                      MQTT_PASSWORD)) ||
      (!MQTT_USEAUTH && client.connect(MQTT_CLIENTID)))
  {
    LOG0("mqtt broker connected\n");
  }

  return client.connected();
}

void MqttHandler::setup()
{
  LOG0("MQTT Setup...\n");
  client.setServer(MQTT_HOST, MQTT_PORT);
  initialized = true;
}

void MqttHandler::handle(unsigned long now)
{
  if (MQTT_ENABLED)
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
  if (MQTT_ENABLED && client.connected())
  {
    client.publish(outtopic, value);
    LOG1( "MQTT publish outtopic=%s value=%s\n", outtopic, value );
  }
}

static char topicBuffer[255];

void MqttHandler::sendCommand( unsigned long address, const bool value)
{
  if (MQTT_ENABLED && client.connected())
  {
    sprintf( topicBuffer, MQTT_COMMAND_TOPIC, address );    

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
