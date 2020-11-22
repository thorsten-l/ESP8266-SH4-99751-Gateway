#include "Pages.h"
#include <App.hpp>
#include <Esp.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <FS.h>
#include <WiFiClient.h>
#include <TelnetStream.h>

bool firmwareUploadFailed;

void handleFirmwareUpload()
{
  firmwareUploadFailed = true;
  HTTPUpload &upload = server.upload();
  if (upload.status == UPLOAD_FILE_START)
  {
    Serial.setDebugOutput(true);
    WiFiUDP::stopAll();
    Serial.printf("Update: %s\n", upload.filename.c_str());
    TLOG1( "Update: %s\n", upload.filename.c_str() );

    uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;

    if (!Update.begin(maxSketchSpace))
    {
      Update.printError(Serial);
    }

    Serial.println();
  }
  else if (upload.status == UPLOAD_FILE_WRITE)
  {
    size_t p = upload.totalSize * 100 / upload.contentLength;
    Serial.printf("\rupload progress = %u%%", p );
    Serial.flush();
    TelnetStream.printf( "\rupload progress = %u%%", p );
    TelnetStream.flush();

    if (Update.write(upload.buf, upload.currentSize) != upload.currentSize)
    {
      Update.printError(Serial);
    }
  }
  else if (upload.status == UPLOAD_FILE_END)
  {
    if (Update.end(true))
    { 
      if( upload.totalSize > 0 )
      {
        firmwareUploadFailed = false;
        Serial.printf("\n\nUpdate Success: %u\nRebooting...\n\n", upload.totalSize );
        TelnetStream.println("\n");
        TLOG1("Update Success: %u\nRebooting...\n\n", upload.totalSize );
      }
      else
      {
        Serial.println( "Firmware file size s zero.");
        TLOG0( "Firmware file size s zero.");
      }
    }
    else
    {
      Update.printError(Serial);
      TLOG0( "Firmware upload error.");
    }
    Serial.setDebugOutput(false);
  }
  yield();
}
