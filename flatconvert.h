/*
TrueType to Adafruit_GFX font converter.  Derived from Peter Jakobs'
Adafruit_ftGFX fork & makefont tool, and Paul Kourany's Adafruit_mfGFX.

NOT AN ARDUINO SKETCH.  This is a command-line tool for preprocessing
fonts to be used with the Adafruit_GFX Arduino library.

For UNIX-like systems.  Outputs to stdout; redirect to header file, e.g.:
  ./fontconvert ~/Library/Fonts/FreeSans.ttf 18 > FreeSans18pt7b.h

REQUIRES FREETYPE LIBRARY.  www.freetype.org

Currently this only extracts the printable 7-bit ASCII chars of a font.
Will eventually extend with some int'l chars a la ftGFX, not there yet.
Keep 7-bit fonts around as an option in that case, more compact.

See notes at end for glyph nomenclature & other tidbits.
*/
#pragma once
#include <ctype.h>
#include <ft2build.h>
#include <stdint.h>
#include <stdio.h>
#include FT_GLYPH_H
#include FT_MODULE_H
#include FT_TRUETYPE_DRIVER_H
#include "pfxfont.h" // Adafruit_GFX font structures
#include "string"
#include "regex"
#include "vector"
#define FC_BUFFER_SIZE (256*1024)
#define DPI 141 // Approximate res. of Adafruit 2.8" TFT
/**
 * 
 */
class BitPusher
{
public:
    BitPusher()
    {
        acc=0;
        bit=7;
        cur=buffer;
    }
    const uint8_t *data() {return buffer;}
    void swallow(int nb, const uint8_t *d)
    {
        memcpy(buffer,d,nb);
        cur=buffer+nb;
        bit=7;
        acc=0;
    }
    void add8Bits(int val)
    {
       *cur++=val;
    }
    void add4Bits(int val)
    {
        if(bit==7)
        {
            acc|=(val<<4);
            bit-=4;
        }else
        {
            acc|=val&0xf;
            align();
        }
    }
    void add2Bits(int val)
    {
        val=val&3;
        acc|=(val<<(bit-1));
        bit-=2;
        if(bit<0)
        {
            align();
        }
    }
    
    void addBit(bool onoff)
    {
        if(onoff) acc|=(1<<bit);
        bit--;
        if(bit<0)
        {
            align();
        }
    }
    void    align()
    {
        if(bit==7) return;
        *cur=acc;
        acc=0;
        cur++;
        bit=7;
    }
    int  offset()
    {
        return (int)(cur-buffer);
    }
    void setOffset(int of)
    {
        cur=buffer+of;
    }
    int    bit;    
    int    acc;
    uint8_t *cur;
    uint8_t buffer[FC_BUFFER_SIZE];
};

/**
 * 
 * @param fontFile
 * @param symbolName
 */

class FontConverter
{
public:
                        FontConverter(const std::string &fontFile, const std::string &symbolName, const std::string &outputFile);
                        ~FontConverter();
        bool           enableCompression() {compressed=true;return true;}
        bool           init(int size,int bpp, int first, int last,int *mapp);
        bool           convert();
        void           printHeader();
        void           printIndex();
        void           printFooter();
        void           printBitmap();
        bool           compressInPlace(uint8_t *in, int &inoutSize);
 static char           printable(int c);
        bool           saveBitmap(const char *bitmap);
        
protected:
    bool                initFreeType(int size);
    bool                convert1bit();
    bool                convertNbit(int n);
    FT_Library          library;
    FT_Face             face;
    std::string         fontFile,symbolName,outputFile;
    bool                ftInited;
    int                 first,last, bpp;  
    bool                compressed;
    std::vector<PFXglyph > listOfGlyphs;
    BitPusher           bitPusher;
    int                 face_height;
    FILE                *output;
    int                 _totalUncompressedSize;
    const int           *_mapp; 
};
