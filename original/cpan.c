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
#include "header.h"
#define P printf
int anread(char*, int);					    /* 05/06 /96 */

/*    global variables declared as externs in monan.h need a root position  */
HEADER header;
int nhar, nhar1, nchans, npts;
float *cmag, *dfr, *phase, *br, *time, tl, dt, fa, smax, *newmag, *newfr,
  *finalfr;
double ampscale;

int main(int argc, char **argv)
{
  int i,k;
/* read in an analysis file */

  anread(argv[1],10);

  printf("# harmonics = %d # timepoints = %d\n",nhar, npts);

/* example code to illustrate data layout */

  printf("Amplitude data:\n");
  printf("harmonic        1       2       3       4       5\n\n");
  for (i=0;i<10;i++) {
    printf("frame %d %f ",i,i*dt);
    for (k=1;k<=5;k++)
      printf("%8.2f ",cmag[k + i*nhar1]);
    printf("\n");
  }
  printf("\nFrequency deviation data:\n");
  printf("harmonic        1       2       3       4       5\n\n");
  for (i=0;i<10;i++) {
    printf("frame %d ",i);
    for (k=1;k<=5;k++)
      printf("%8.2f ",dfr[k + i*nhar1]);
    printf("\n");
  }

}
  
