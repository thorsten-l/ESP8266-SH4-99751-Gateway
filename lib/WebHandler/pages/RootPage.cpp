#include "Pages.h"

void handleRootPage()
{
  sendHeader( APP_NAME );
  sendPrint( "<form class='pure-form'>");
  sendLegend( "Application" );
  sendPrint(
    "<p>Name: " APP_NAME "</p>"
    "<p>Version: " APP_VERSION "</p>"
    "<p>PlatformIO Environment: " PIOENV "</p>"
    "<p>Author: Dr. Thorsten Ludewig &lt;t.ludewig@gmail.com></p>" );

  sendLegend( "Build" );
  sendPrint("<p>Date: " __DATE__ "</p>"
            "<p>Time: " __TIME__ "</p>");

  sendPrint("</form>");
  sendFooter();
}
