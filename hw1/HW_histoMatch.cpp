#include "IP.h"
using namespace IP;

void histoMatchApprox(ImagePtr, ImagePtr, ImagePtr);

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// HW_histoMatch:
//
// Apply histogram matching to I1. Output is in I2.
//

void
HW_histoMatch(ImagePtr I1, ImagePtr targetHisto, bool approxAlg, ImagePtr I2)
{
	if(approxAlg) {
		histoMatchApprox(I1, targetHisto, I2);
		return;
	}

	int i, p, R;							// i: index, p: pixel, R: Remaining difference between Href[i] && H[i]				
	int left[MXGRAY], right[MXGRAY];		// right and left range bufs for i index 
	int s_cursor, save[MXGRAY];
	int total, Hsum, Havg, h1[MXGRAY];		
	ChannelPtr<uchar> p1, p2;				// input & output image buffers
	int type, ch;							// type, ch vars for IP_getChannel
	double scale;							// scale factor for differing input/reference image sizes
	ChannelPtr<int> h2;

	/* total number of pixels in image */ 
	total = I1->width() * I1->height();

	/* init I2 dimensions and buffer */
	IP_copyImageHeader(I1, I2);

	// get reference Histogram
	IP_getChannel(targetHisto, 0, h2, type);

	for (ch=0; IP_getChannel(I1, ch, p1, type); ch++) {
		IP_getChannel(I2, ch, p2, type);

		for(i=0; i<MXGRAY; i++) h1[i] = 0; 		// clear histogram
		for(i=0; i<total; i++) h1[p1[i]]++; 	// eval histogram 

		for(i=Havg=0; i<MXGRAY; i++) Havg += h2[i]; 
		scale = (double) total / Havg;
		if(scale != 1) for(i=0; i<MXGRAY; i++) h2[i] *= scale;

		R = 0;									// Remainder
		Hsum = 0;								// current h2 Sum
		/* evaluate remapping of all input gray levels;
		Each input gray value maps to an interval of valid output values.
		The endpoints of the intervals are left[] and right[] */
		for(i=0; i<MXGRAY; i++) {
			left[i] = R; 						// left end of interval
			Hsum += h1[i];						// cumulative value for interval
			while(Hsum>h2[R] && R<MaxGray) { 	// compute width of interval
				Hsum -= h2[R];					// adjust Hsum as interval widens
				R++;							// update
			}
			right[i] = R;						// init right end of interval
			save[i] = Hsum;
		}

		// clear h1 and reuse it below
		for(i=0; i<MXGRAY; i++) h1[i] = 0;

		// visit all input pixels
		for(i=0; i<total; i++) {
			// get pixel
			p = left[p1[i]];

			// set cursor to index in save[]
			s_cursor = p1[i];
			while (p == right[s_cursor]) save[s_cursor++]--;

			if ((p1[i] == 0 || left[p1[i]] != right[p1[i]-1]) && h1[p] < h2[p])
				p2[i] = p;	
			else if ((h1[p] + save[p1[i]-1]) < h2[p])
				p2[i] = p;						// mapping satisfies h2	
			else
				p2[i] = p = left[p1[i]] = MIN(p+1, right[p1[i]]);

			h1[p]++;
		}
	}
}

void
histoMatchApprox(ImagePtr I1, ImagePtr targetHisto, ImagePtr I2)
{
	int i, p, R;
	int left[MXGRAY], right[MXGRAY];
	int total, Hsum, Havg, h1[MXGRAY]; //, h2[MXGRAY];
	double scale;
	ChannelPtr<uchar> p1, p2;	// input & output buffers
	ChannelPtr<int> h2;
	int type, ch;

	/* total number of pixels in image */ 
	total = I1->width() * I1->height();

	/* init I2 dimensions and buffer */
	IP_copyImageHeader(I1, I2);
	IP_getChannel(targetHisto, 0, h2, type);

	for (ch=0; IP_getChannel(I1, ch, p1, type); ch++) {
		IP_getChannel(I2, ch, p2, type);

		for(i=0; i<MXGRAY; i++) h1[i] = 0; /* clear histogram */
		for(i=0; i<total; i++) h1[p1[i]]++; /* eval histogram */

		for(i=Havg=0; i<MXGRAY; i++) Havg += h2[i]; // NOTE: HAVG misleading var name. H2_Count better
		scale = (double) total / Havg;
		if(scale != 1) for(i=0; i<MXGRAY; i++) h2[i] *= scale;

		R = 0;									// Remainder
		Hsum = 0;								// current h2 Sum
		/* evaluate remapping of all input gray levels;
		Each input gray value maps to an interval of valid output values.
		The endpoints of the intervals are left[] and right[] */
		for(i=0; i<MXGRAY; i++) {
			left[i] = R; 						// left end of interval
			Hsum += h1[i];						// cumulative value for interval
			while(Hsum>h2[R] && R<MaxGray) { 	// compute width of interval
				Hsum -= h2[R];					// adjust Hsum as interval widens
				R++;							// update
			}
			right[i] = R;						// init right end of interval
		}

		// clear h1 and reuse it below
		for(i=0; i<MXGRAY; i++) h1[i] = 0;

		// visit all input pixels
		for(i=0; i<total; i++) {
			p = left[p1[i]];

			if(h1[p] < h2[p])
				p2[i] = p;						// mapping satisfies h2	
			else
				p2[i] = p = left[p1[i]] = MIN(p+1, right[p1[i]]);

			h1[p]++;
		}
	}
}
