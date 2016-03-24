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
int anread(char*, int);					    /* 05/06 /96 */

/*    global variables declared as externs in monan.h need a root position  */
HEADER header;
int nhar, nhar1, nchans, npts;
float *cmag, *dfr, *phase, *br, *time, tl, dt, fa, smax, *newmag, *newfr,
  *finalfr;
double ampscale;

int main(int argc, char **argv)
{
  int i,k,harms;
/* read in an analysis file */
  if (argc == 3) {
    if (atoi(argv[2]) == 1) { 
      printf("argv2: %d\n",atoi(argv[2]));
      anread(argv[1],1); 
    } else { 
      printf("argv2!: %d\n",atoi(argv[2]));
      anread(argv[1],atoi(argv[2])); 
    }
  } else {
    anread(argv[1],-1);
  }

  printf("# harmonics = %d # timepoints = %d\n",nhar, npts);
  harms = 8;
/* example code to illustrate data layout */

  printf("Amplitude data:\n");
  printf("harmonic ------>");
  for (i=1;i<=harms;i++) {
    printf("%9d",i);
  } printf("\n\t      ∆t\n");
  for (i=0;i<10;i++) {
    printf("frame %d %f ",i,i*dt);
    for (k=1;k<=harms;k++)
      printf("%8.2f ",cmag[k + i*nhar1]);
    printf("\n");
  }
  printf("\nFrequency deviation data:\n");
  printf("harmonic ->    ");
  for (i=1;i<=harms;i++) {
    printf("%-9d",i);
  } printf("\n\n");
  for (i=0;i<10;i++) {
    printf("frame %d ",i);
    for (k=1;k<=8;k++)
      printf("%8.2f ",dfr[k + i*nhar1]);
    printf("\n");
  }

}
  
