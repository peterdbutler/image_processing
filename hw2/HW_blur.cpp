#include "IP.h"
using namespace IP;

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// HW_blur:
//
// Blur image I1 with a box filter (unweighted averaging).
// The filter has width filterW and height filterH.
// We force the kernel dimensions to be odd.
// Output is in I2.
//
void
HW_blur(ImagePtr I1, int filterW, int filterH, ImagePtr I2)
{
	// copy image header (width, height) of input image I1 to output image I2
	IP_copyImageHeader(I1, I2);

	// init var for total number of pixels
    int w = I1->width();
    int h = I1->height();
    uchar* buf;

	// declarations for image channel pointers and datatype
	ChannelPtr<uchar> p1, p2;
	int type;

    // check for kernal filter size mismatch 
    // (will return out if index exception for case where kernal is > image w,h)
    if (filterH > h/2) filterH = h/2;
    if (filterW > w/2) filterW = w/2;

    // check for even filters
    if (filterH % 2 == 0) filterH--;
    if (filterW % 2 == 0) filterW--;

	// visit all image channels and evaluate output image
	for(int ch=0; IP_getChannel(I1, ch, p1, type); ch++) {
		IP_getChannel(I2, ch, p2, type);

        int x, y, sum;
        int row;
        int kernalW = filterW/2;
        int kernalH = filterH/2;

        // Two implementations provided: 
        // first with a buffer copying each row by row into buffer to pad and operate on.
        // second without a buffer, using some for loop bounds to
        // iterate over the the input image w.o and if condition checks,

        // change below 1 to 0 to see second iteration. 
        #if 1
        buf = new uchar[MAX(w+filterW, h+filterH)];

        for (y=0; y<h; y++) {
            // precomute row, saving multiplications at each for call later
            row = y*w;

            // pad top edge
            for (x=0; x<kernalW; x++) buf[x] = p1[row];

            // copy row into padded buffer
            for (x=0; x<w; x++) buf[kernalW+x] = p1[row+x];
            
            // pad bottom edge
            for (x=0; x<kernalW; x++) buf[kernalW+w+x] = p1[row+w-1];

            // compute sum
            sum = 0;
            for (x=0; x<filterW; x++) sum+= buf[x];
            
            // populate p2 with blurred pixel vals, update sum
            for (x=0; x<w; x++) {
                p2[row+x] = sum/filterW;
                sum += buf[x+filterW] - buf[x];
            }
        }

        #else
        for (y=0; y<h; y++) {
            // save multiplications
            row = y*w;

            // left pad of row, duplicates row[0] filterW/2+1 times for initial removal
            // (we offset sum s.t. it's ready for kernal shift BEFORE we calc the new pixel val
            // this helps ensure no out of bounds indexing exceptions)
            sum = (kernalW+1)*p1[row];

            // rows filterW/2+1 number of pixels ()
            for (x=0; x<kernalW; x++) sum += p1[row + x];

            // left pad kernal range
            for (x=0; x<=kernalW; x++)  {
                // remove left padding of row[0] from sum 
                sum += p1[row+x+kernalW] - p1[row];
                p2[row + x] = sum / filterW;
            }

            // center of image (no padding)
            for (   ; x < w-1 - kernalW; x++) {
                sum += (p1[x +row +kernalW] - p1[row +x -1 -kernalW]);
                p2[row +x] = sum / filterW;
            }

            // right pad kernal range
            for (   ; x < w; x++) {
                sum += (p1[row +w -1] - p1[row + x -1-kernalW]);
                p2[row +x] = sum / filterW;
            }
        }

        buf = new uchar[h+filterH];
        #endif

        // NOTE: we cannot use a horizontal blur version of the second version above as we
        //      operate on the same image, we would be using the new avg pixel to update the next
        //      (we need a memory store to prevent these overwrites, so the buffer is req'd)

        for (x=0; x<w; x++) {
            // pad top edge
            for (y=0; y<kernalH; y++) buf[y] = p2[x];

            // copy row into padded buffer
            for (y=0; y<h; y++) buf[kernalH+y] = p2[x+y*w];
            
            // pad bottom edge
            for (y=0; y<kernalH; y++) buf[kernalH+h+y] = p2[x+(h-1)*w];

            sum = 0;
            for (y=0; y<filterH; y++) sum+= buf[y];
            
            for (y=0; y<h; y++) {
                p2[x+y*w] = sum/filterH;
                sum += buf[y+filterH] - buf[y];
            }
        }

        delete[] buf;
    }
}