#include "IP.h"
using namespace IP;

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// HW_convolve:
//
// Convolve I1 with filter kernel in Ikernel.
// Output is in I2.
//

void
HW_convolve(ImagePtr I1, ImagePtr Ikernel, ImagePtr I2)
{
    int w,h;
	int kw,kh;
	int pw,ph;	
	int src_row, dst_row;
    int bufferW;
	int x,y,i,j;
	int type;
    float sum;
	uchar* buf;

	// declarations for image channel pointers and datatype
	ChannelPtr<uchar> p1, p2;
    ChannelPtr<float> ker;

	// copy image header (width, height) of input image I1 to output image I2
	IP_copyImageHeader(I1, I2);

	// init var for total number of pixels
    w = I1->width();
    h = I1->height();

	// kernal dims
	kw = Ikernel->width();
	kh = Ikernel->height();

	// pad dims
	pw = (int) kw/2;
	ph = (int) kh/2;

    // buffer width for indexing (increases readability)
    bufferW = w+kw-1;

	buf = new uchar[(w+ 2*pw)*(h+ 2*kw)];

	for(int ch=0; IP_getChannel(I1, ch, p1, type); ch++) {
		IP_getChannel(I2, ch, p2, type);
		IP_getChannel(Ikernel, ch, ker, type);

		// top edge padding
		for(y=0; y<ph; y++) {
			// save mults, pre-compute
			dst_row = y*bufferW;
			
			for (x=0; x<pw	   ; x++) buf[dst_row+x] = p1[0];			// left pad edge
			for (   ; x<pw+w   ; x++) buf[dst_row+x] = p1[x-pw];		// image row
			for (	; x<pw+w+pw; x++) buf[dst_row+x] = p1[w-1];			// right pad edge
		}

		// center horizontal strip
		for (y=0; y<h; y++) {
			// save mults, pre-compute
			dst_row = (y+ph)*bufferW;
			src_row  = y*w;

			for (x=0; x<pw	   ; x++) buf[dst_row+x] = p1[src_row];		// left pad edge
			for (   ; x<pw+w   ; x++) buf[dst_row+x] = p1[src_row+x-pw];// image row
			for (	; x<pw+w+pw; x++) buf[dst_row+x] = p1[src_row+w-1];	// right pad edge
		}
		
		//	bottom edge padding
		for(y=0; y<ph; y++) {
			// save mults, pre-compute
			src_row = (  h-1+ph)*bufferW;
			dst_row = (y+h  +ph)*bufferW;
			
			// duplicate bottom row 
			for (x=0; x<pw+w+pw; x++) buf[dst_row+x] = buf[src_row+x];
		}

        for (y=0; y<h; y++) {
            for (x=0; x<w; x++) {
                src_row = y*bufferW+x;
                sum = 0;

                for (j=0; j<kh; j++) {
                    dst_row = j*kw;
                    for (i=0; i<kw; i++) sum += ker[dst_row+i] * (float) buf[src_row +j*bufferW+i];
                }

                p2[x+y*w] = (uchar) CLIP(sum, 0, MaxGray);
            }
        }
	}

    // cleanup
	delete[] buf;
}