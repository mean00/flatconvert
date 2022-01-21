This is a fork of https://github.com/charles-haynes/fontconvert
which is itself a fork of adafruit font convert

The code has been cleaned up 

flatconvert -f fontfile -s size -o outputfile [[-b firstChar]] [[-e lastChar]]  [[-m bitmapfile]]

to build:

mkdir build
cmake ..
make
