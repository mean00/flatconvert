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
#include "flatconvert.h"
#include "cxxopts.hpp"

 
 /**
  * 
  * @return 
  */
 bool FontConverter::convert()
 {
     int err;
     FT_Glyph glyph;
      GFXglyph zeroGlyph= (GFXglyph){0,0,0,0,0,0};
     for(int i=first;i<=last;i++)
     {
       
        // MONO renderer provides clean image with perfect crop
        // (no wasted pixels) via bitmap struct.
        bool renderingOk=true;
        if ((err = FT_Load_Char(face, i, FT_LOAD_TARGET_NORMAL))) {     fprintf(stderr, "Error %d loading char '%c'\n", err, i); renderingOk=false;   }
        if ((err = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL))) {      fprintf(stderr, "Error %d rendering char '%c'\n", err, i);     renderingOk=false;  }
        if ((err = FT_Get_Glyph(face->glyph, &glyph))) {      fprintf(stderr, "Error %d getting glyph '%c'\n", err, i);    renderingOk=false;    }

        if(!renderingOk)
        {
            listOfGlyphs.push_back(zeroGlyph);  
            continue;
        }
        FT_Bitmap *bitmap = &face->glyph->bitmap;
        FT_BitmapGlyphRec *g= (FT_BitmapGlyphRec *)glyph;

        // Minimal font and per-glyph information is stored to
        // reduce flash space requirements.  Glyph bitmaps are
        // fully bit-packed; no per-scanline pad, though end of
        // each character may be padded to next byte boundary
        // when needed.  16-bit offset means 64K max for bitmaps,
        // code currently doesn't check for overflow.  (Doesn't
        // check that size & offsets are within bounds either for
        // that matter...please convert fonts responsibly.)
        bitPusher.align();
         GFXglyph thisGlyph;
        thisGlyph.bitmapOffset = bitPusher.offset();
        thisGlyph.width = bitmap->width;
        thisGlyph.height = bitmap->rows;
        thisGlyph.xAdvance = face->glyph->advance.x >> 6;
        thisGlyph.xOffset = g->left;
        thisGlyph.yOffset = 1 - g->top;
        listOfGlyphs.push_back(thisGlyph);

        for (int y = 0; y < bitmap->rows; y++) 
        {
          const uint8_t *line=bitmap->buffer+y * bitmap->pitch;
          for (int x = 0; x < bitmap->width; x++) 
          {
            bitPusher.add2Bits(line[x] >>6);
          }
        }
    
    }     
    face_height= face->size->metrics.height >> 6;
    FT_Done_Glyph(glyph);  
    return true;
 }
 
/**
 * 
 */
void usage()
{
    
    exit(1);
}

/**
 * 
 * @param argc
 * @param argv
 * @return 
 */
int main(int argc, char *argv[]) 
{
    printf("Usage:  flatconver -s size -f fontToUse (-o output file] [-b first char] [-e last char]\n");
    
    cxxopts::Options options("fatconvert", "cleaner version of adafruit fontconvert, ttf to GFXfont");
  
  options.add_options()
    ("f,font",          "font to use",  cxxopts::value<std::string>()) // a bool parameter
    ("s,size",          "font size",    cxxopts::value<int>())
    ("b,begin_char",    "first glyph",  cxxopts::value<int>()->default_value("32"))
    ("e,end_char",      "last glyph",   cxxopts::value<int>()->default_value("0x7e")) // ~
    ("o,output_file",   "output file",  cxxopts::value<std::string>())
    ("m,bitmap_file",   "bitmap binaryfile",  cxxopts::value<std::string>()->default_value(""))
    ;
   cxxopts::ParseResult result;
   result = options.parse(argc, argv);
   
   int first = result["begin_char"].as<int>();
   int last = result["end_char"].as<int>();
   int size=result["size"].as<int>();
   std::string fontFile=result["font"].as<std::string>();
   std::string outputFile=result["output_file"].as<std::string>();
   std::string bitmapFile=result["bitmap_file"].as<std::string>();
  
      
  std::string fileName = fontFile.substr(fontFile.find_last_of("/\\") + 1);
  fileName= std::regex_replace(fileName, std::regex(" "), "_");  
  std::string::size_type const p(fileName.find_last_of('.'));
  fileName = fileName.substr(0, p);
  
  
  char ext[16];
  sprintf(ext, "%dpt%db", size, (last > 127) ? 8 : 7);
  
  // full var name
  std::string symbolName=fileName+std::string(ext)+std::string("2b");

  if(!outputFile.size())
  {
      outputFile=symbolName+std::string(".h");
  }
  
  
  printf("Processing font %s\n",fontFile.c_str());
  printf("Generating symbol %s\n",symbolName.c_str());
  printf("Writing file %s\n",outputFile.c_str());
  printf("First glyph  : %d '%c'\n",first,FontConverter::printable(first));
  printf("Last glyph   : %d '%c'\n",last,FontConverter::printable(last));

  FontConverter *converter=new FontConverter(fontFile,symbolName,outputFile);
  
  if(!converter->init(size,first,last))
  {
      printf("Failed to init converter\n");
      exit(1);
  }
  if(!converter->convert())
  {
      printf("Failed to convert\n");
      exit(1);
  }
  converter->printHeader();
  converter->printBitmap();
  converter->printIndex();
  converter->printFooter();
  
  if(bitmapFile.size())
  {
      converter->saveBitmap(bitmapFile.c_str());
  }
  
  delete converter;
  converter=NULL;  
  printf("Done.\n");
  return 0;
}

/* -------------------------------------------------------------------------

Character metrics are slightly different from classic GFX & ftGFX.
In classic GFX: cursor position is the upper-left pixel of each 5x7
character; lower extent of most glyphs (except those w/descenders)
is +6 pixels in Y direction.
W/new GFX fonts: cursor position is on baseline, where baseline is
'inclusive' (containing the bottom-most row of pixels in most symbols,
except those with descenders; ftGFX is one pixel lower).

Cursor Y will be moved automatically when switching between classic
and new fonts.  If you switch fonts, any print() calls will continue
along the same baseline.

                    ...........#####.. -- yOffset
                    ..........######..
                    ..........######..
                    .........#######..
                    ........#########.
   * = Cursor pos.  ........#########.
                    .......##########.
                    ......#####..####.
                    ......#####..####.
       *.#..        .....#####...####.
       .#.#.        ....##############
       #...#        ...###############
       #...#        ...###############
       #####        ..#####......#####
       #...#        .#####.......#####
====== #...# ====== #*###.........#### ======= Baseline
                    || xOffset

glyph->xOffset and yOffset are pixel offsets, in GFX coordinate space
(+Y is down), from the cursor position to the top-left pixel of the
glyph bitmap.  i.e. yOffset is typically negative, xOffset is typically
zero but a few glyphs will have other values (even negative xOffsets
sometimes, totally normal).  glyph->xAdvance is the distance to move
the cursor on the X axis after drawing the corresponding symbol.

There's also some changes with regard to 'background' color and new GFX
fonts (classic fonts unchanged).  See Adafruit_GFX.cpp for explanation.
*/

