#include "Pages.h"
#include <Util.hpp>

const char last10Script[] PROGMEM =
  "<script>\n"
    "function loadLast10(){\n"
      "var e = document.getElementById(\"last10\");\n"
      "fetch(\"/last10\").then((resp) => "
      "resp.json()).then(function(data){\n"
        "var text=\"\";\n"
        "var i;\n"
        "for ( i = 0; i < data.last10.length; i++) {\n"
          "text += data.last10[i] + \"\\n\";\n"
        "}\n"
        "e.textContent=text;\n"
      "});\n"
    "}\n"
    "setInterval(loadLast10,10000);\n"
    "document.onload=loadLast10();\n"
  "</script>\n";

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
  sendPrint("<pre><div id='last10'></div></pre></form>\n");

/*  
  int msi = messageStartIndex;
  while (msi != messageEndIndex)
  {
    char *mb = messageBuffer + (msi * (MAX_MESSAGE_LENGTH + 1));
    sendPrintf( "%s\n", mb );
    msi += 1;
    msi %= MESSAGE_BUFFER_LINES;
  }
*/
  sendPrint(last10Script);
  sendFooter();
}
