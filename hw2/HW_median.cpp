#include <algorithm>
#include <vector>
#include "IP.h"
using namespace IP;

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// HW_median:
//
// Apply median filter of size sz x sz to I1.
// Clamp sz to 9.
// Output is in I2.
//

void
HW_median(ImagePtr I1, int sz, ImagePtr I2)
{
    int w,h;
    int i,j,x,y;
	int type;
    int pad, ksz;
    int bufferW;
    int src_row, dst_row;
    int temp;
    uchar* buf;
    uchar* sort;

	// declarations for image channel pointers and datatype
	ChannelPtr<uchar> p1, p2;

	// copy image header (width, height) of input image I1 to output image I2
	IP_copyImageHeader(I1, I2);

	// init var for total number of pixels
    w = I1->width();
    h = I1->height();

    // check for even filters (probs uncessary, but...)
    if (sz % 2 == 0) sz--;

    // instructions to CLAMP sz to 9. Redundant? Anyway:
    sz = CLIP(sz, 3, 9);  

    pad = sz/2;
    ksz = sz*sz;
    bufferW = w+sz-1;

    sort = new uchar[ksz];
    buf = new uchar[sz*bufferW];

	// visit all image channels and evaluate output image
	for(int ch=0; IP_getChannel(I1, ch, p1, type); ch++) {
		IP_getChannel(I2, ch, p2, type);

        // copy rows into image buffer
        for (y=0; y<pad; y++) {
            src_row = y*w;
            dst_row = (y+pad)*bufferW;

            for (x=0; x<pad; x++) buf[dst_row+x]       = p1[src_row];        // copy left pad
            for (x=0; x<w  ; x++) buf[dst_row+pad+x]   = p1[src_row+x];      // copy image
            for (x=0; x<pad; x++) buf[dst_row+pad+w+x] = p1[src_row+w-1];    // copy right pad
        }

        // copy p1[row1] into image horz pad of im buffer pad times
        for (y=0; y<pad; y++) {
            src_row = pad*bufferW;
            dst_row = y*bufferW;

            for (x=0; x<bufferW; x++) buf[dst_row+x] = buf[src_row+x];
        }

        // process image until padding border.
        /* NOTE: to avoid "if" statements, process the pad section as a seperate for condition */
        for (y=0; y<h-pad; y++) {

            // insert next row into circular buffer 
            // src_row = (y+sz-1)*w;   
            src_row = (y+pad)*w;   
            dst_row = ((y+sz-1) %sz)*bufferW;

            for (i=0; i<pad; i++) buf[dst_row+i]       = p1[src_row];        // copy left pad
            for (i=0; i<w  ; i++) buf[dst_row+pad+i]   = p1[src_row+i];      // copy image
            for (i=0; i<pad; i++) buf[dst_row+pad+w+i] = p1[src_row+w-1];    // copy right pad

            for(x=0; x<w; x++) {
                // sort
                for (i=0; i<ksz; i++) {
                    // push next value from buffer at back of sort[]
                    sort[i] = buf[x+ (i%sz)*bufferW + (i/sz)];
                    for (j=i; j>0; j--) {
                        if (sort[j] < sort[j-1]) {
                            temp = sort[j];
                            sort[j] = sort[j-1];
                            sort[j-1] = temp;
                        }
                    }
                }
                
                p2[y*w+x] = sort[ksz/2];
            }
        }

        // process bottom edge w/ paddding
        for (y=h-pad; y<h; y++) {
            src_row = ((y+sz-2) %sz)*bufferW;
            dst_row = ((y+sz-1) %sz)*bufferW;

            // copy bottom row
            for (x=0; x<bufferW; x++) buf[dst_row+x] = buf[src_row+x];

            for (x=0; x<w; x++) {
                // sort
                for (i=0; i<ksz; i++) {
                    // push next value from buffer at back of sort[]
                    sort[i] = buf[x+ (i%sz)*bufferW + (i/sz)];
                    for (j=i; j>0; j--) {
                        if (sort[j] < sort[j-1]) {
                            temp = sort[j];
                            sort[j] = sort[j-1];
                            sort[j-1] = temp;
                        }
                    }
                }
                
                p2[y*w+x] = sort[ksz/2];
            }
        }
    }

    // cleanup
    delete[] buf;
    delete[] sort;
}