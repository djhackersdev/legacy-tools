#!/bin/sh
gcc -o extract-png -ggdb -Wall -std=c99 extract-png.c
mkdir pngs
for i in $(find -type f | grep -v extract-png | grep -v .m2v | grep -v ssq); do
  dir="$(echo "$i" | cut -b 3- | sed -e 's#/#__#g')"
  dir="pngs/${dir%.*}"
  mkdir -p "$dir"
  ./extract-png "$i"
  mv *.png "$dir"
  rm -f *.png
done
