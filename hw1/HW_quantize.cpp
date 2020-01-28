#include "IP.h"
using namespace IP;

#include <stdio.h>

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// HW_quantize:
//
// Quantize I1 to specified number of levels. Apply dither if flag is set.
// Output is in I2.
//
void
HW_quantize(ImagePtr I1, int levels, bool dither, ImagePtr I2)
{
	// copy image header (width, height) of input image I1 to output image I2
	IP_copyImageHeader(I1, I2);

	// init vars for width, height, and total number of pixels
	int w = I1->width ();
	int h = I1->height();
	int total = w * h;

	// init lookup table
	int i, scale, bias, lut[MXGRAY];
    scale = MXGRAY / levels;
    bias = scale/2;
	for(i=0; i<MXGRAY; ++i) lut[i] = scale * (int) (i/scale) + bias; 

	// declarations for image channel pointers and datatype
	ChannelPtr<uchar> p1, p2;
	int type;

    if (dither != 0) {
        int sign, max_val; 
        double dither_val;

        max_val = MaxGray - bias;

        for (int y = 0; y < h; y++) {
            sign = (y % 2 == 0) ? 1 : -1;

            for (int x=0; x < w; x++) {
                dither_val = (double) bias * rand() / (double) RAND_MAX;

                for(int ch=0; IP_getChannel(I1, ch, p1, type); ch++) {
                    IP_getChannel(I2, ch, p2, type);
                    p2[y*w + x] = lut[ CLIP((p1[y*w + x] + sign * (int) dither_val), bias, max_val) ]; 
                }
                sign = -sign;
            }
        }
    } else {

        for(int ch=0; IP_getChannel(I1, ch, p1, type); ch++) {
            IP_getChannel(I2, ch, p2, type);
            for(i=0; i<total; i++) *p2++ = lut[*p1++];
        }
    }
}
