#!/bin/sh
cd ..

docker run -it --rm \
  -v `pwd`:/workdir:rw \
  --name platformio platformio \
  /usr/local/bin/platformio run -e d1_mini -e d1_mini_lite

docker run -it --rm \
  -v `pwd`:/workdir:rw \
  --name platformio platformio \
  /usr/local/bin/platformio run -e d1_mini -e d1_mini_lite

mkdir -p private/firmware
cp .pio/build/d1_mini/firmware.bin private/firmware/d1_mini.bin
cp .pio/build/d1_mini_lite/firmware.bin private/firmware/d1_mini_lite.bin
