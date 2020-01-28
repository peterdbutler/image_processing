#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define PI 3.1415927
#define MAG(i)      sqrt(Fr[i]*Fr[i] + Fi[i]*Fi[i])
#define PHASE(i)    atan2(Fi[i], Fr[i])

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
    double *fr, *fi;
    double *Fr, *Fi;

    dir = atoi(argv[2]);
    if ( (dir < 0) || (dir > 1) ) {
        printf("ERROR: dft1D usage: dir must be 0 for forward DFT or 1 "
               "for inverse DFT\n"); 
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

    fr = new double[h]{0.};
    fi = new double[h]{0.};

    Fr = new double[h]{0.};
    Fi = new double[h]{0.};

    if (!dir) {
        // dft1D time -> freq

        if (w == 1) {
            for (x = 0; x<h; x++) fscanf(fin, "%lf", &fr[x]);
        } else {
            for (x = 0; x<h; x++) fscanf(fin, "%lf %lf", &fr[x], &fi[x]);
        }
        fclose(fin);

        for (u=0; u<h; u++) {
            re = im = 0;
            for(x=0; x<h; x++) {
                c =  cos(2.*PI*u*x/h);
                s = -sin(2.*PI*u*x/h);
                re += fr[x]*c - fi[x]*s;
                im += fr[x]*s + fi[x]*c;
            }
            Fr[u] = re / h;
            Fi[u] = im / h;
        }
    
        fprintf(fout, "2 %i\n", h);
        for (x = 0; x<h; x++) fprintf(fout, "%lf %lf\n", Fr[x], Fi[x]);
        fclose(fout);

    } else {
        // dft1D freq -> time    INVERSE

        for (x = 0; x<h; x++) fscanf(fin, "%lf %lf", &Fr[x], &Fi[x]);
        fclose(fin);

        for (x=0; x<h; x++) {
            re = im = 0;
            for(u=0; u<h; u++) {
                c =  cos(2.*PI*u*x/h);
                s = -sin(2.*PI*u*x/h);
                re += Fr[u]*c + Fi[u]*s;
                im += Fr[u]*s - Fi[u]*c;
            }
            fr[x] = re;
            fi[x] = im;
        }

        fprintf(fout, "2 %i\n", h);
        for (x = 0; x<h; x++) fprintf(fout, "%lf %lf\n", fr[x], fi[x]);
        fclose(fout);
    }

    delete[] Fi;
    delete[] Fr;

    delete[] fi;
    delete[] fr;
    
    return 0;
}