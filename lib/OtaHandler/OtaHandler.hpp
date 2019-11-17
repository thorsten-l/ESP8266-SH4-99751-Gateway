#ifndef __OTA_HANDLER_HPP__
#define __OTA_HANDLER_HPP__

class OtaHandler
{
public:
  bool inProgress;
  void setup();
};

extern OtaHandler otaHandler;

#endif
