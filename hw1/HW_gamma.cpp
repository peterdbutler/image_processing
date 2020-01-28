#include "IP.h"
using namespace IP;

#include <math.h>
#include <stdio.h>

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// HW_gammaCorrect:
//
// Gamma correct image I1. Output is in I2.
//
void
HW_gammaCorrect(ImagePtr I1, double gamma, ImagePtr I2)
{
	// copy image header (width, height) of input image I1 to output image I2
	IP_copyImageHeader(I1, I2);

	// init var for total number of pixels
	int total = I1->width() * I1->height();

	// init lookup table
	int i, lut[MXGRAY];
	for(i=0; i<MXGRAY; ++i) {
		lut[i] = CLIP(MaxGray * pow((double) i/MaxGray, 1/gamma), 0, MaxGray);
	} 

	// declarations for image channel pointers and datatype
	ChannelPtr<uchar> p1, p2;
	int type;

	// visit all image channels and evaluate output image
	for(int ch=0; IP_getChannel(I1, ch, p1, type); ch++) {
		IP_getChannel(I2, ch, p2, type);
		for(i=0; i<total; i++) *p2++ = lut[*p1++];
	}
}
