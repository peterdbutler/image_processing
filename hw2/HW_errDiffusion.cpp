#include "IP.h"
using namespace IP;

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// HW_errDiffusion:
//
// Apply error diffusion algorithm to image I1.
//
// This procedure produces a black-and-white dithered version of I1.
// Each pixel is visited and if it + any error that has been diffused to it
// is greater than the threshold, the output pixel is white, otherwise it is black.
// The difference between this new value of the pixel from what it used to be
// (somewhere in between black and white) is diffused to the surrounding pixel
// intensities using different weighting systems.
//
// Use Floyd-Steinberg     weights if method=0.
// Use Jarvis-Judice-Ninke weights if method=1.
//
// Use raster scan (left-to-right) if serpentine=0.
// Use serpentine order (alternating left-to-right and right-to-left) if serpentine=1.
// Serpentine scan prevents errors from always being diffused in the same direction.
//
// A circular buffer is used to pad the edges of the image.
// Since a pixel + its error can exceed the 255 limit of uchar, shorts are used.
//
// Apply gamma correction to I1 prior to error diffusion.
// Output is saved in I2.
//

void
HW_errDiffusion(ImagePtr I1, int method, bool serpentine, double gamma, ImagePtr I2)
{
    int x,y;
    int w,h,total;
    int thr;
    int type;
    int err;
    uchar thresh_lut[MXGRAY], gamma_lut[MXGRAY];
    short** buf;
    
	// declarations for image channel pointers and datatype
	ChannelPtr<uchar> p1, p2;

	// copy image header (width, height) of input image I1 to output image I2
	IP_copyImageHeader(I1, I2);

	// init var for total number of pixels
    w = I1->width();
    h = I1->height();
    total = w*h;

    //populate gamLUT for gamma correction
	for(x=0; x<MXGRAY; ++x) gamma_lut[x] = CLIP(MaxGray * pow((double) x/MaxGray, 1/gamma), 0, MaxGray); 

    // populate thrLUT for thresholding
    thr = MXGRAY/2;
    for (x=0; x<thr;    x++) thresh_lut[x] = 0;
    for (   ; x<MXGRAY; x++) thresh_lut[x] = MaxGray;

	for(int ch=0; IP_getChannel(I1, ch, p1, type); ch++) {
		IP_getChannel(I2, ch, p2, type);

        // Floyd-Steinberg
        if (!method) {
            // init buffers for padding    
            buf = new short*[2];
            for (x=0; x<2; x++) buf[x] = new short[w+2];

            // copy row into image buffer, apply gamma correction in the process. Pad l & r edges
            for (x=0; x<w; x++) buf[0][x+1] = (short) gamma_lut[p1[x]];
            buf[0][0] = buf[0][1];
            buf[0][w+1] = buf[0][w];

            for (y=0; y<h; y++) {
                // compute next row to save multiplications
                int row = w*y;

                // use total to 'wrap' indexing so as to avoid out of bounds on final call
                int next_row = (row + w) % (total); 

                // compute respective index for appropriate buffer line
                int upper_idx_offset = (y  ) %2; 
                int lower_idx_offset = (y+1) %2; 

                // copy next row p1[y+1] into buf[y%2] row
                for (x=0; x<w; x++) buf[lower_idx_offset][x+1] = (short) gamma_lut[p1[next_row+x]];

                // copy first and last val into buffer @ respective pads
                buf[lower_idx_offset][0]   = buf[lower_idx_offset][1];
                buf[lower_idx_offset][w+1] = buf[lower_idx_offset][w];

                if (serpentine && (y%2 == 1)) {
                    for (x=w; x>0; x--) {
                        p2[row+x-1] = (uchar) thresh_lut[ CLIP(buf[upper_idx_offset][x], 0, MaxGray) ];

                        // Compute local error
                        err = buf[upper_idx_offset][x] - p2[row+x-1];

                        // Distribute errors across nearby neighbors according to F.S.
                        buf[upper_idx_offset][x-1] += (err * 7/16.0);
                        buf[lower_idx_offset][x+1] += (err * 3/16.0);
                        buf[lower_idx_offset][x  ] += (err * 5/16.0);
                        buf[lower_idx_offset][x-1] += (err * 1/16.0);
                    }
                } else {
                    for (x=0; x<w; x++) {
                        p2[row+x] = (uchar) thresh_lut[ CLIP(buf[upper_idx_offset][x+1], 0, MaxGray) ];

                        // Compute local error
                        err = buf[upper_idx_offset][x+1] - p2[row+x];

                        // Distribute errors across nearby neighbors according to F.S.
                        buf[upper_idx_offset][(x+1) +1] += (err * 7/16.0);
                        buf[lower_idx_offset][(x+1) -1] += (err * 3/16.0);
                        buf[lower_idx_offset][(x+1)   ] += (err * 5/16.0);
                        buf[lower_idx_offset][(x+1) +1] += (err * 1/16.0);
                    }
                }
            }

            // cleanup
            for (x=0; x<2; x++) delete[] buf[x];
            delete[] buf;

        // Jarvis-Judice-Ninke
        } else {

            // init buffers for padding    
            buf = new short*[3];
            for (x=0; x<3; x++) buf[x] = new short[w+4];

            // copy row into image buffer, apply gamma correction in the process. Pad l & r edges
            for (x=0; x<w; x++) buf[0][x+2] = (short) gamma_lut[p1[x]];
            buf[0][0] = buf[0][1] = buf[0][2];
            buf[0][w+3] = buf[0][w+2] = buf[0][w+1];

            // copy row into image buffer, apply gamma correction in the process. Pad l & r edges
            for (x=0; x<w; x++) buf[1][x+2] = (short) gamma_lut[p1[w+x]];
            buf[1][0] = buf[1][1] = buf[1][2];
            buf[1][w+3] = buf[1][w+2] = buf[1][w+1];

            for (y=0; y<h; y++) {
                // row indices to save mults, mod total causes a wrap to avoid OOB err
                int row1 = w*y;
                int row2 = (row1 + w) % total; 
                int row3 = (row2 + w) % total;

                // compute respective index for appropriate buffer line
                int upper_idx_offset = (y  ) %3; 
                int middl_idx_offset = (y+1) %3; 
                int lower_idx_offset = (y+2) %3; 

                // copy next row p1[y+1] into buf[y%2] row
                for (x=0; x<w; x++) buf[lower_idx_offset][x+2] = (short) gamma_lut[p1[row3+x]];

                // copy first and last val into buffer @ respective pads
                buf[lower_idx_offset][0] = buf[lower_idx_offset][1] = buf[lower_idx_offset][2];
                buf[lower_idx_offset][w+3] = buf[lower_idx_offset][w+2] = buf[lower_idx_offset][w+1];

                if (serpentine && (y%2 == 1)) {
                    for (x=w+1; x>2; x--) {
                        //  threshold central kernal pixel
                        p2[row1+x-2] = (uchar) thresh_lut[ CLIP(buf[upper_idx_offset][x], 0, MaxGray) ];

                        // Compute local error
                        err = buf[upper_idx_offset][x+1] - p2[row1+x-2];

                        // Distribute errors across nearby neighbors according to J.J.N. 
                        buf[upper_idx_offset][x -1] += (err * 7/48.0);
                        buf[upper_idx_offset][x -2] += (err * 5/48.0);

                        buf[middl_idx_offset][x -2] += (err * 3/48.0);
                        buf[middl_idx_offset][x -1] += (err * 5/48.0);
                        buf[middl_idx_offset][x   ] += (err * 7/48.0);
                        buf[middl_idx_offset][x +1] += (err * 5/48.0);
                        buf[middl_idx_offset][x +2] += (err * 3/48.0);

                        buf[lower_idx_offset][x -2] += (err * 1/48.0);
                        buf[lower_idx_offset][x -1] += (err * 3/48.0);
                        buf[lower_idx_offset][x   ] += (err * 5/48.0);
                        buf[lower_idx_offset][x +1] += (err * 3/48.0);
                        buf[lower_idx_offset][x +2] += (err * 1/48.0);
                    }
                } else {
                    for (x=0; x<w; x++) {
                        //  threshold central kernal pixel
                        p2[row1+x] = (uchar) thresh_lut[ CLIP(buf[upper_idx_offset][x+2], 0, MaxGray) ];

                        // Compute local error
                        err = buf[upper_idx_offset][x+2] - p2[row1+x];

                        // Distribute errors across nearby neighbors according to J.J.N. 
                        buf[upper_idx_offset][(x+2) +1] += (err * 7/48.0);
                        buf[upper_idx_offset][(x+2) +2] += (err * 5/48.0);

                        buf[middl_idx_offset][(x+2) -2] += (err * 3/48.0);
                        buf[middl_idx_offset][(x+2) -1] += (err * 5/48.0);
                        buf[middl_idx_offset][(x+2)   ] += (err * 7/48.0);
                        buf[middl_idx_offset][(x+2) +1] += (err * 5/48.0);
                        buf[middl_idx_offset][(x+2) +2] += (err * 3/48.0);

                        buf[lower_idx_offset][(x+2) -2] += (err * 1/48.0);
                        buf[lower_idx_offset][(x+2) -1] += (err * 3/48.0);
                        buf[lower_idx_offset][(x+2)   ] += (err * 5/48.0);
                        buf[lower_idx_offset][(x+2) +1] += (err * 3/48.0);
                        buf[lower_idx_offset][(x+2) +2] += (err * 1/48.0);
                    }
                }
            }

            // cleanup
            for (x=0; x<3; x++) delete[] buf[x];
            delete[] buf;

        }
    }
}