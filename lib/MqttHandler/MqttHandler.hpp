#ifndef __MQTT_HANDLER_H__
#define __MQTT_HANDLER_H__

class MqttHandler
{
private:
  bool initialized;
  bool reconnect();
  void setup();
  unsigned long lastPublishTimestamp;

public:
  MqttHandler();
  void handle( unsigned long now );
  void sendValue( const char* outopic, const char* value );
};

extern MqttHandler mqttHandler;

#endif
