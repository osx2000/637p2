/*
  Sample code for CSc 635 Project 2

  compile:
  gcc -O cpan.c header.c anread.c -o cpan -lm

  run:
  cpan [.an] 

*/
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "header.h"
// #include "macro.h"
// #include "monan.h"
// #include "g_raph.h"
#define P printf
int anread(char*, int);             /* 05/06 /96 */

/*    global variables declared as externs in monan.h need a root position  */
HEADER header;
int nhar, nhar1, nchans, npts;
float *cmag, *dfr, *phase, *br, *time, tl, dt, fa, smax, *newmag, *newfr,
  *finalfr;
double ampscale;

void envelope(float *a,int aSize, float *w,int wSize, int *brkpts);

int main(int argc, char **argv)
{
  int i,j,k,frames;
  float **working, *actual;

/* read in an analysis file */
  anread(argv[1],-1);

  printf("# harmonics = %d # timepoints = %d\n",nhar, npts);
  frames = npts;
  
// set number of breakpoints & set number of harmonics
  harms = 1;
  nbreaks = 9;

// brkpts 2D array to save the frame# of each break, for each harmonic
  int brkpts[harms][nbreaks];

// dummy data for "actual" points
  actual = {0,18,16,14,8,9,16,15,12,14,11,7,9,8,4,1,2,0};

// for each harmonic
  for (k=1;k<=harms;k++) {

  // generate amplitude data and save frame indeces
    for (j=0;j<nbreaks;j++) {
      // TODO: 2nd param to change to "frames" or "npts"
      envelope(actual,sizeof(actual)/sizeof(float), &working[k],nbreaks, &brkpts[k]);
    }
  }
}

// @return the frame index of most recent break
void envelope(float *a,int aSize, float *w,int wSize, int *brkpts) {
  int i,j;
  float x,x1,y1,x2,y2; // values for interpolation

// set all working points to zero
  for (i=0;i<wSize-1;i++) {
    w[i] = 0;
  }
// trivial, but save endpoints as individual break points
  brkpts[0]=0; brkpts[aSize-1]=aSize-1;

// find new brkpt w/ largest error


}
  
