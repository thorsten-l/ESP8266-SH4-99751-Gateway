#include <Arduino.h>
#include "Util.hpp"

/*
Find the description about the boot device code here:
https://www.sigmdel.ca/michel/program/esp8266/arduino/watchdogs2_en.html
*/
int getBootDevice(void) {
  int bootmode = 0;
  asm("movi %0, 0x60000200\n\t"
      "l32i %0, %0, 0x118\n\t"
      : "+r"(bootmode) /* Output */
      :                /* Inputs (none) */
      : "memory"       /* Clobbered */
  );
  return ((bootmode >> 0x10) & 0x7);
}

void alterPin(int pin) { digitalWrite(pin, 1 ^ digitalRead(pin)); }

void showChipInfo() {
  Serial.println("-- CHIPINFO --");
  Serial.printf("Chip Id = %08X\n", ESP.getChipId());

  Serial.printf("CPU Frequency = %dMHz\n", ESP.getCpuFreqMHz());

  uint32_t realSize = ESP.getFlashChipRealSize();
  uint32_t ideSize = ESP.getFlashChipSize();
  FlashMode_t ideMode = ESP.getFlashChipMode();

  Serial.printf("\nFlash real id:   %08X\n", ESP.getFlashChipId());
  Serial.printf("Flash real size: %u\n", realSize);
  Serial.printf("Flash ide  size: %u\n", ideSize);
  Serial.printf("Flash chip speed: %u\n", ESP.getFlashChipSpeed());
  Serial.printf("Flash ide mode:  %s\n",
                (ideMode == FM_QIO
                     ? "QIO"
                     : ideMode == FM_QOUT
                           ? "QOUT"
                           : ideMode == FM_DIO
                                 ? "DIO"
                                 : ideMode == FM_DOUT ? "DOUT" : "UNKNOWN"));

  if (ideSize != realSize) {
    Serial.println("Flash Chip configuration wrong!\n");
  } else {
    Serial.println("Flash Chip configuration ok.\n");
  }

  Serial.printf("Free Heap         : %u\n", ESP.getFreeHeap());
  Serial.printf("Sketch Size       : %u\n", ESP.getSketchSize());
  Serial.printf("Free Sketch Space : %u\n", ESP.getFreeSketchSpace());

  Serial.println();
}
