#include "IP.h"
using namespace IP;

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// HW_histoStretch:
//
// Apply histogram stretching to I1. Output is in I2.
// Stretch intensity values between t1 and t2 to fill the range [0,255].
//
void
HW_histoStretch(ImagePtr I1, int t1, int t2, ImagePtr I2)
{
	// copy image header (width, height) of input image I1 to output image I2
	IP_copyImageHeader(I1, I2);

	// init var for total number of pixels
	int total = I1->width () * I1->height();

	// init lookup table
	int i, lut[MXGRAY];
	
	// catch division by zero exception if t1 == t2 (it's possible!)
	if (t1 == t2) {
		for(i=0; i<MXGRAY; ++i) lut[i] = MaxGray;
	} else {
		for(i=0; i<MXGRAY; ++i) lut[i] = CLIP(MaxGray * (i - t1) / (t2 - t1), 0, MaxGray);
	}

	// declarations for image channel pointers and datatype
	ChannelPtr<uchar> p1, p2;
	int type;

    // visit all image channels and vealuate output image
	for(int ch=0; IP_getChannel(I1, ch, p1, type); ch++) {
		IP_getChannel(I2, ch, p2, type);
		for(i=0; i<total; i++) *p2++ = lut[*p1++];
	}
}