#include "IP.h"
using namespace IP;
#include <math.h>

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// HW_swapPhase:
//
// Swap the phase channels of I1 and I2.
// Output is in II1 and II2.
//

struct complex {
    int len;
    float *re;
    float *im;
};

static void swapXY(float **arr, int dim);
static void fft1D(struct complex *coeff_i, int dir, struct complex *coeff_o);

extern void HW_fft2MagPhase(ImagePtr Ifft, ImagePtr Imag, ImagePtr Iphase);
extern void HW_MagPhase2fft(ImagePtr Imag, ImagePtr Iphase, ImagePtr Ifft);

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// HW_swapPhase:
//
// Swap phase of I1 with I2.
// (I1_mag, I2_phase) -> II1
// (I1_phase, I2_mag) -> II2
//
void
HW_swapPhase(ImagePtr I1, ImagePtr I2, ImagePtr II1, ImagePtr II2)
{
	struct complex *f[2];
	int i, j, x, y, d, row;
	int w1, h1, w2, h2;
	int padW, padH;
	int type;
	ChannelPtr<uchar> p1, p2;
	// ImagePtr Ifft1, Ifft2, IinvFFT1, IinvFFT2;
	// ImagePtr Imag1, Iphase1, Imag2, Iphase2;

	if(I1->imageType() != BW_IMAGE) {
		IP_castImage(I1, BW_IMAGE, II1);
		IP_copyImage(II1, I1);
	}

	w1 = I1->width();
	h1 = I1->height();
	w2 = I2->width();
	h2 = I2->height();

	// compute FFT of I1 and I2
	// TODO: PUT YOUR CODE HERE...
	// ensure w,h for each image is equal && a power of 2 for fft
	d = (int) exp2( ceil(log2 (MAX(MAX(w1,h1), MAX(w2, h2)))) );

	for (i=0; i<2; i++) {
		f[i] = new struct complex[d];
		f[i] = new struct complex[d];
	}

	for (i=0; i<d; i++) {
		f[0][i].re = new float[d]{ };
		f[0][i].im = new float[d]{ };

		f[1][i].re = new float[d]{ };
		f[1][i].im = new float[d]{ };
	}

	IP_getChannel(I1, 0, p1, type);
	IP_getChannel(I2, 0, p2, type);

	/*
	// copy image 1 into f[0]
	padW = (int) (d-w1)/2;
	padH = (int) (d-h1)/2;
	for (y=padH, j=0; y<h1+padH; y++, j++) {
		row = j*w1;
		for (x=padW, i=0; x<w1+padW; x++, i++) f[0][y].re[x] = (float) p1[row+i];
	}

	// copy image 2 into f[0];
	padW = (int) (d-w2)/2;
	padH = (int) (d-h2)/2;
	for (y=padH, j=0; y<h2+padH; y++, j++) {
		row = j*w2;
		for (x=padW, i=0; x<w2+padW; x++, i++) f[1][y].re[x] = (float) p2[row+i];
	}
	*/

	#if DEBUG
	printf("\np1:\n\t");
	for (y=0; y<h1; y++) {
		row = y*w1;
		for (x=0; x<w1; x++) printf("%i ", p1[row+x]);
		printf("\n\t");
	}

	printf("\nf[0].re\n\t");
	for (y=0; y<d; y++) {
		row = y*d;
		for (x=0; x<d; x++) printf("%5.2f ", f[0][y].re[x]);
		printf("\n\t");
	}
	printf("\n");
	#endif

	// compute magnitude and phase from real and imaginary FFT channels
	for (i=0; i<d; i++) {
		fft1D(&f[0][i], 0, &f[0][i]);
		fft1D(&f[1][i], 0, &f[1][i]);
	}
	
	// TODO: PUT YOUR CODE HERE...

	// swap phases and convert back to FFT images
	
	// TODO: PUT YOUR CODE HERE...

	// compute inverse FFT
	
	// TODO: PUT YOUR CODE HERE...

	// extract magnitude from resulting images
	
	// TODO: PUT YOUR CODE HERE...

	// allocate uchar image and cast float channel to uchar for mag1

	// TODO: PUT YOUR CODE HERE...

	// allocate uchar image and cast float channel to uchar for mag2

	// TODO: PUT YOUR CODE HERE...

	// cleanup...
	/*
	for (i=0; i<d; i++) {
		delete[] f[0][i].re;
		delete[] f[0][i].im;

		delete[] f[1][i].re;
		delete[] f[1][i].im;
	}

	for (i=0; i<2; i++) {
		delete[] f[i];
		delete[] f[i];
	}
	*/

	printf("swapPhase Exit success\n");
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