#include <Arduino.h>
#include <App.hpp>
#include "Pages.h"
#include <Util.hpp>
#include <html/maintenance.h>

static const char *setupProcessor(const char *var)
{
  if (strcmp(var, "pioenv_name") == 0)
    return PIOENV;
  return nullptr;
}

void handleMaintenancePage()
{
  sendAuthentication();
  sendHeader(APP_NAME " - Maintenance", false, MAINTENANCE_STYLE );
  sendHtmlTemplate( MAINTENANCE_HTML_TEMPLATE, setupProcessor );
  sendFooter();
}
