**flatconvert : Tweakable embedded font conversion**
This is a fork of https://github.com/charles-haynes/fontconvert, which is itself a fork of adafruit font convert

The code has been cleaned up and supports 1/2/4 bit per pixel fonts + heatshrink compression.

flatconvert -f fontfile -s size -o outputfile [-b firstChar] [-e lastChar]  [-m bitmapfile] [-p bitperpixel (1 or 4)] [-c compressed] [-k "abcde"]

The -k allows you to pick only the glyphs you really need. That helps a lot size-wise when dealing with large fonts.

to build:

   mkdir build
   cmake ..
   make

