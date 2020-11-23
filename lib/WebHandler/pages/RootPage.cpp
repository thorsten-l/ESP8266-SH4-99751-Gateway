#include "Pages.h"
#include <Util.hpp>

void handleRootPage()
{
  sendHeader(APP_NAME);
  sendPrint("<form class='pure-form'>");
  sendLegend("Application");
  sendPrint(
      "<p>Name: " APP_NAME "</p>"
      "<p>Version: " APP_VERSION "</p>"
      "<p>PlatformIO Environment: " PIOENV "</p>"
      "<p>Author: Dr. Thorsten Ludewig &lt;t.ludewig@gmail.com></p>");

  sendLegend("Last 10 Messages");
  sendPrint("<pre>");

  int msi = messageStartIndex;
  while (msi != messageEndIndex)
  {
    char *mb = messageBuffer + (msi * (MAX_MESSAGE_LENGTH + 1));
    sendPrint(mb);
    msi += 1;
    msi %= 11;
  }

  sendPrint("</pre></form>");
  sendFooter();
}
