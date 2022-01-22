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
extern "C"
{
#include "heatshrink_encoder.h"
}
/**
 *
 * @return
 */
 bool FontConverter::compressInPlace(uint8_t *in, int &inoutSize)
 {
     uint8_t tmp[FC_BUFFER_SIZE];
     //printf("Compressing...\n");
     bitPusher.align();
     const uint8_t *src=in;
     int     size=inoutSize;

     heatshrink_encoder *hse = heatshrink_encoder_alloc(8, 4);
     if(!hse)
     {
         printf("Cannot initialize heatshrink\n");
         exit(-1);
     }
    size_t sunk = 0;
    size_t count=0;
    size_t polled = 0;
    size_t comp_sz=FC_BUFFER_SIZE;
    while (sunk < size)
    {
        HSE_sink_res esres = heatshrink_encoder_sink(hse, (uint8_t *)(src+sunk), size - sunk, &count);
        if(esres <0)
        {
            printf("sink fail\n");
            exit(-1);
        }
        sunk += count;

        if (sunk == size)
        {
            if(HSER_FINISH_MORE!= heatshrink_encoder_finish(hse))
            {
                printf("Finish fail\n");
                exit(-1);
            }
        }

        HSE_poll_res pres;
        do
        {
            pres = heatshrink_encoder_poll(hse, &tmp[polled], comp_sz - polled, &count);
            polled += count;
        } while (pres == HSER_POLL_MORE);
        if(HSER_POLL_EMPTY!= pres)
        {
            printf("Poll fail\n");
            exit(-1);
        }
        if (polled >= comp_sz)
        {
            printf("compression overflow\n");
            exit(-1);
        }

        if (sunk == size)
        {
            if(HSER_FINISH_DONE!= heatshrink_encoder_finish(hse))
             {
                printf("done state failed\n");
                exit(-1);
            }
        }
    }
    //printf("Input size =%d, output size=%d\n",(int)size,(int)polled);
    //printf("Shrinked to %d %%\n",(int)((polled*100)/size));    
    // copy tmp to bitPusher
    heatshrink_encoder_free(hse);
    hse=NULL;
    memcpy(in,tmp,polled);
    inoutSize=polled;
    return true;
 }
