#include "IP.h"
using namespace IP;
#include <math.h>

#define DEBUG 1

struct complex {
    int len;
    float *re;
    float *im;
};

static void swapXY(float **arr, int dim);
static void fft1D(struct complex *coeff_i, int dir, struct complex *coeff_o);

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// HW_spectrum:
//
// Compute magnitude and phase spectrum from input image I1.
// Save results in Imag and Iphase.
//
void
HW_spectrum(ImagePtr I1, ImagePtr Imag, ImagePtr Iphase)
{
    float **re, **im;
    float real, imag, maxM, minM, maxP, minP;
    int i, j, x, y, d, row;
    int padW, padH;
    float scaleM, scaleP;
    int type;
    struct complex f;

	int w = I1->width();
	int h = I1->height();

	// compute FFT of the input image
    d = (int) exp2 (ceil ( log2 (MAX(w, h))));
    padW = (int) (d-w)/2;
    padH = (int) (d-h)/2;

    // create re, im arrays for FFT
    re = new float*[d];
    im = new float*[d];
    for (x=0; x<d; x++) {
        re[x] = new float[d]{ 0., };
        im[x] = new float[d]{ 0., };
    }

    ChannelPtr<uchar> p1;
    IP_getChannel(I1, 0, p1, type);
    
    // copy image into re, im arrays
    for (y=padH, j=0; y<h+padH; y++, j++) {
        row = j*w;
        for (x=padW, i=0; x<w+padW; x++, i++) re[y][x] = (float) p1[row+i]; 
    }

    // row FFTD
    f.len = d;
    for (y=0; y<d; y++) {
        f.re = &re[y][0];
        f.im = &im[y][0];

        fft1D(&f, 0, &f);
    }

    // swap row, columns for 
    swapXY(re, d);
    swapXY(im, d);

    // col FFT
    for (y=0; y<d; y++) {
        f.re = &re[y][0];
        f.im = &im[y][0];

        fft1D(&f, 0, &f);
    }

    // swap row, columns for 
    swapXY(re, d);
    swapXY(im, d);

	// scale magnitude and phase to fit between [0, 255]
	Imag  ->allocImage(d, d, BW_TYPE);
	Iphase->allocImage(d, d, BW_TYPE);

	ChannelPtr<uchar> mag    = Imag  [0];
	ChannelPtr<uchar> phase  = Iphase[0];

    maxM = minM = sqrt(re[0][0]*re[0][0] + im[0][0]*im[0][0]);
    maxP = minP = (double) atan2((double) im[0][0], (double) re[0][0]);
    for (y=0; y<d; y++) {
        for (x=0; x<d; x++) {
            real = re[y][x];
            imag = im[y][x];

            re[y][x] = sqrt(real*real + imag*imag);
            im[y][x] = (float) atan2((double) imag, (double) real);

            if (re[y][x] > maxM) {
                maxM = re[y][x];
            } else if (re[y][x] < minM) {
                minM = re[y][x];
            }

            if (im[y][x] > maxP) {
                maxP = im[y][x];
            } else if (im[y][x] < minP) {
                minP = im[y][x];
            }
        }
    }

    // determine scaling factors 
    scaleM = maxM - minM; 
    scaleP = maxP - minP;
   
    if (minP > 0) minP = -minP; 

    for (y=0; y<d; y++) {
        row = y*d;
        for (x=0; x<d; x++) {
            mag[row+x]   = (uchar) (MaxGray * (float) (re[y][x] - minM) / scaleM);
            phase[row+x] = (uchar) (MaxGray * (float) (im[y][x] - minP) / scaleP);
        }
    }

    // clean-up re,im
    for (x=0; x<d; x++) {
        delete[] re[x];
        delete[] im[x]; 
    }
    delete[] re;
    delete[] im;
}


static void swapXY(float **arr, int dim) {
    int i, j;
    float t;

    for (i=0; i<dim; i++) {
        for (j=i+1; j<dim; j++) {
            t = arr[i][j];
            arr[i][j] = arr[j][i];
            arr[j][i] = t; 
        }
    }
}


static void fft1D(struct complex *coeff_i, int dir, struct complex *coeff_o) {
    int i, j, N, N2;
    float arg, arg_inc, t1, t2, c, s;
    struct complex even, odd;

    // get len of inputs, compute half
    N = coeff_i->len;
    N2 = (int) N / 2;

    // base case:
    if (N == 2) {
        t1 = coeff_i->re[0] + coeff_i->re[1];
        t2 = coeff_i->im[0] + coeff_i->im[1];
        coeff_o->re[1] = coeff_i->re[0] - coeff_i->re[1];
        coeff_o->im[1] = coeff_i->im[0] - coeff_i->im[1];
        coeff_o->re[0] = t1;
        coeff_o->im[0] = t2;

    } else {
        // create even and odd structs, to recursively call onto 
        even.len = N2;
        even.re = new float[N2];
        even.im = new float[N2];

        odd.len = N2;
        odd.re = new float[N2];
        odd.im = new float[N2];

        // split input coeffecients into even, odd indices
        for (i=j=0; i<N; i++, j++) {
            even.re[j] = coeff_i->re[i];
            even.im[j] = coeff_i->im[i];

            i++;
            odd.re[j] = coeff_i->re[i];
            odd.im[j] = coeff_i->im[i]; 
        }

        // recursively call on even, odd splits
        fft1D(&even, dir, &even);
        fft1D(&odd,  dir, &odd);

        // compute arguments for trig functions
        if (!dir) {
            arg_inc = -2 * PI / N;
        } else {
            arg_inc =  2 * PI / N;
        }

        // compute outgoing coeffecients
        for (i = arg = 0.; i < N2; i++, arg += arg_inc) {
            c = cos(arg);
            s = sin(arg);

            t1 = c * odd.re[i] - s * odd.im[i];
            coeff_o->re[i   ] = even.re[i] + t1;
            coeff_o->re[i+N2] = even.re[i] - t1;
            
            t1 = s * odd.re[i] + c * odd.im[i];
            coeff_o->im[i   ] = even.im[i] + t1;
            coeff_o->im[i+N2] = even.im[i] - t1;
        }

        // clean-up
        delete[] even.re;
        delete[] even.im;
        delete[] odd.re;
        delete[] odd.im;
    }

    // divide by 2 for logN division
    if (!dir) {
        for (i=0; i<N; i++) {
            coeff_o->re[i] *= 0.5; 
            coeff_o->im[i] *= 0.5; 
        }
    }
}