#include "IP.h"
using namespace IP;

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// HW_sharpen:
//
// Sharpen image I1. Output is in I2.
//
void
HW_sharpen(ImagePtr I1, int size, double factor, ImagePtr I2)
{
    int i, total, type;

	// declarations for image channel pointers and datatype
	ChannelPtr<uchar> p1, p2;

	// copy image header (width, height) of input image I1 to output image I2
	IP_copyImageHeader(I1, I2);

	// init var for total number of pixels
    total = I1->width()*I1->height();

    // blur input image, store in p2
    IP_blur(I1, size, size, I2);

	for(int ch=0; IP_getChannel(I1, ch, p1, type); ch++) {
		IP_getChannel(I2, ch, p2, type);
        for (i=0; i<total; i++)  p2[i] = CLIP(p1[i]+factor*(p1[i]-p2[i]), 0, MaxGray);
    }
}
