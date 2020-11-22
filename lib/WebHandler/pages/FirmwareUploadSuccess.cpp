#include "pages/Pages.h"
#include <FS.h>
#include <LittleFS.h>

extern bool firmwareUploadFailed;

void handleFirmwareUploadSuccess()
{
  sendAuthentication();
  sendHeader(APP_NAME " - Firmware upload", true );
  sendPrint("<form class='pure-form'>");

  if(firmwareUploadFailed)
  {
    sendLegend("Firmware upload FAILED!");
    sendFooter();
  }
  else
  {
    sendLegend("Firmware successfully uploaded.");
    sendPrint("<h4 style='color: red'>Restarting System ... takes about 30s</h4></form>");
    sendFooter();
    delay(1000);
    ESP.restart();
  }
}
