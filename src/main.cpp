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

static time_t checkWifiTimestamp = 0;
static time_t sendBatteryTimestamp = 0;

ADC_MODE(ADC_VCC);

// Callback function is called only when a valid code is received.
void showCode(unsigned int period, unsigned long address,
              unsigned long groupBit, unsigned long unit,
              unsigned long switchType)
{
  mqttHandler.sendCommand( address, switchType == 1 );
  TLOG1( MQTT_COMMAND_TOPIC " ", address );
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
  int bootDevice = getBootDevice();
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);

  NewRemoteReceiver::init(4, 2, showCode);

  digitalWrite(LED_BUILTIN, LOW);
  delay(4000);
  digitalWrite(LED_BUILTIN, HIGH);

  Serial.println("\n\n\n" APP_NAME " - Version " APP_VERSION
                 " by Dr. Thorsten Ludewig");
  Serial.println("Build date: " __DATE__ " " __TIME__ "\n");

  Serial.printf("Boot device = %d\n", bootDevice);
  Serial.printf("Boot mode = %d\n", ESP.getBootMode());
  Serial.printf("Boot version = %d\n", ESP.getBootVersion());

  if (bootDevice == 1)
  {
    Serial.println(F("\nThis sketch has just been uploaded over the UART.\n"
                     "The ESP8266 will freeze on the first restart.\n"
                     "Press the reset button or power cycle the ESP\n"
                     "and operation will be resumed thereafter."));
  }

  Serial.println();

  showChipInfo();

  configTime(0, 0, "pool.ntp.org");

  wifiHandler.wifiInitStationMode();
  TelnetStream.begin();
  otaHandler.setup();

  LOG0("Receiver initialized\n");
}

void loop()
{
  ArduinoOTA.handle();
  TelnetStream.handle();

  if (otaHandler.inProgress == false)
  {
    time_t currentTimestamp = millis();

    if ((currentTimestamp - checkWifiTimestamp) >= 2000)
    {
      checkWifiTimestamp = currentTimestamp;

      if (WiFi.status() != WL_CONNECTED)
      {
        LOG0("*** WiFi connection lost\n");
        wifiHandler.wifiInitStationMode();
      }
    }

    if ((currentTimestamp - sendBatteryTimestamp) >= ( 1000 * 300 ))
    {
      char buffer[16];

      sendBatteryTimestamp = currentTimestamp;
      uint16_t vcc = (uint16_t)(((double)ESP.getVcc()) * VCC_ADJUST);
      sprintf( buffer, "%u", vcc );

      mqttHandler.sendValue( MQTT_BATTERY_TOPIC, buffer );
    }

    // handle MQTT connection
    mqttHandler.handle(currentTimestamp);
  }

  delay(20);
}
