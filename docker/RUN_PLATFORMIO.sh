#!/bin/sh
cd ..

docker run -it --rm \
  -v `pwd`:/workdir:rw \
  --name platformio platformio \
  /usr/local/bin/platformio run -e d1_mini -e d1_mini_lite

VERSION="`awk '/APP_VERSION/{ print $3 }' lib/App/App.hpp | sed -e 's/"//g'`"
echo $VERSION

mkdir -p firmware
cp .pio/build/d1_mini/firmware.bin firmware/$VERSION-d1_mini.bin
cp .pio/build/d1_mini_lite/firmware.bin firmware/$VERSION-d1_mini_lite.bin

