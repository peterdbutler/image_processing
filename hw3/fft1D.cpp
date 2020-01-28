#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define PI 3.141592653589793

struct complex {
    int len;
    double *re;
    double *im;
};

static void fft1D(struct complex *coeff_i, int dir, struct complex *coeff_o);

int main(int argc, char** argv) {

    if (argc != 4) {
        printf("ERROR: df1D usage:\n\tdft1D in dir out\n");
        return -1;
    }

    int w,h;
    int dir;
    int u,x;
    double c, s, re, im;
    FILE *fin, *fout;
    struct complex f, F;

    dir = atoi(argv[2]);
    if ( (dir < 0) || (dir > 1) ) {
        printf("ERROR: fft1D usage: dir must be 1 for forward FFT or 2 "
               "for inverse FFT\n"); 
        return -1;
    }

    if ((fin = fopen(argv[1], "r")) == NULL)  {
        printf("ERROR: Unable to open file: %s\nHINT: Does it exist?\n", argv[1]);
        return -1;
    }

    if (fscanf(fin, "%i %i", &w, &h) ==  0) {
        printf("ERROR: unable to determine w,h of data in input file\n"); 
        fclose(fin);
        return -1;
    }

    if ((fout = fopen(argv[3], "w")) == NULL) {
        printf("ERROR: Unable to open file: %s", argv[3]);
        fclose(fin);
        return -1;
    }

    // ensure length of f,F is a power of 2:
    x = (int) exp2( ceil(log2(h)) );
    f.len = x;
    f.re = new double[x]{0.};
    f.im = new double[x]{0.};

    F.len = x;
    F.re = new double[x]{0.};
    F.im = new double[x]{0.};

    if (!dir) {
        // fft1D time -> freq   FORWARD

        // read data in to f
        if (w == 1) {
            // real data as input:
            for (x = 0; x<h; x++) {
                fscanf(fin, "%lf", &f.re[x]);
            }
        } else {
            // complex data as input:
            for (x = 0; x<h; x++) {
                fscanf(fin, "%lf %lf", &f.re[x], &f.im[x]);
            }
        }
        fclose(fin);

        // compute fft1D
        fft1D(&f, dir, &F);
    
        // write data out
        fprintf(fout, "2 %i\n", h);
        for (x = 0; x<h; x++) fprintf(fout, "%lf %lf\n", F.re[x], F.im[x]);
        fclose(fout);

    } else {
        // fft1D freq -> time   INVERSE

        // read data in to F
        for (x = 0; x<h; x++) fscanf(fin, "%lf %lf", &F.re[x], &F.im[x]);
        fclose(fin);

        // compute fft1D
        fft1D(&F, dir, &f);

        // write data out
        fprintf(fout, "2 %i\n",  h);
        for (x = 0; x<h; x++) fprintf(fout, "%lf %lf\n", f.re[x], f.im[x]);
        fclose(fout);
    }

    // cleanup F
    delete[] F.re;
    delete[] F.im;

    // cleanup f
    delete[] f.im;
    delete[] f.re;
    
    return 0;
}


static void fft1D(struct complex *coeff_i, int dir, struct complex *coeff_o) {
    int i, j, N, N2;
    double arg, arg_inc, t1, t2, c, s;
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
        even.re = new double[N2];
        even.im = new double[N2];

        odd.len = N2;
        odd.re = new double[N2];
        odd.im = new double[N2];

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

        for (i = arg = 0.; i < N2; i++, arg += arg_inc) {
            c = cos(arg);
            s = sin(arg);

            /* W = (c + i*s) * (o.re + i*o.im)                                      */
            /* W = (c * o.re) + (i*s * o.re) + (c * i*o.im) + (i^2 * s * o.im)      */
            /* W_re = c * o.re - s * o.im                                           */
            /* W_im = s * o.re + c * o.im)                                          */

            t1 = c * odd.re[i] - s * odd.im[i];
            coeff_o->re[i   ] = even.re[i] + t1;
            coeff_o->re[i+N2] = even.re[i] - t1;
            
            t1 = s * odd.re[i] + c * odd.im[i];
            coeff_o->im[i   ] = even.im[i] + t1;
            coeff_o->im[i+N2] = even.im[i] - t1;
        }

        delete[] even.re;
        delete[] even.im;
        delete[] odd.re;
        delete[] odd.im;
    }

    if (!dir) {
        for (i=0; i<N; i++) {
            coeff_o->re[i] *= 0.5; 
            coeff_o->im[i] *= 0.5; 
        }
    }
}