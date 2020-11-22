#!/bin/bash

DESTDIR="../include/html"

mkdir -p "$DESTDIR"

for f in *.html; do
  NAME=`basename $f .html`
  DEFNAME="__`echo $NAME | tr a-z A-Z`_H__"
  echo $NAME
  xxd -i $f > tmp1.file
  sed -e 's/\ =\ {/ PROGMEM = {/' tmp1.file | \
  sed -e 's/unsigned char/const char/' | \
  sed -e 's/};/, 0x00 };/' > tmp2.file

  cat > tmp1.file <<EOT
#ifndef $DEFNAME
#define $DEFNAME

#include <Arduino.h>

EOT

  cat > tmp3.file <<EOT

#endif
EOT

  cat tmp1.file tmp2.file tmp3.file > "$DESTDIR/$NAME.h"
  rm -f tmp?.file
done
