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
#define P printf
int anread(char*, int);             /* 05/06 /96 */

/*    global variables declared as externs in monan.h need a root position  */
HEADER header;
int nhar, nhar1, nchans, npts;
float *cmag, *dfr, *phase, *br, *time, tl, dt, fa, smax, *newmag, *newfr,
  *finalfr;
double ampscale;

float *w, *a;
int   *b, brk, frames, nbreaks;

int envelope();
void interpolate(int brindex);
int compare_ints (const void *a, const void *b);

int main(int argc, char **argv)
{
  int i,j,k,harms;

/* read in an analysis file */
  anread(argv[1],-1);

  printf("# harmonics = %d # timepoints = %d\n",nhar, npts);
  //frames = npts;
  harms = 1;
  nbreaks = 5;
  frames = 18; 

  // dummy data for "actual" points
  static const float DUMMY[] = {0,17,15,18,8,9,16,15,12,14,11,7,9,8,4,1,2,0};

  a = (float *) calloc(frames,sizeof(float));
  w = (float *) calloc(frames,sizeof(float));
  b = (int *) calloc(nbreaks,sizeof(int));

  memcpy(a, DUMMY, sizeof(DUMMY));
  memset(b, 2000, nbreaks * sizeof(int));

  // for each harmonic
  for (k=1;k<=harms;k++) {
    // set all working points to zero
    for (i=0;i<frames-1;i++) {
      w[i] = 0;
    }
    // save startpoint frame #'s as the first break points
    b[0]=0; brk=0; 
      
    for (j=1;j<=nbreaks;j++) {
      // TODO: 2nd param to change to "frames" or "npts"
      b[j] = envelope();
      qsort(b,nbreaks,sizeof (int),compare_ints);
      //for(i=0;i<nbreaks;i++){P("%d\n",b[i]);}
      interpolate(j);
    }
  }
  // P("working amps\n");
  // for (i=0;i<frames;i++) {
  //   P("w%.2d %4.2f\n",i,w[i]);
  // }

  free(a); free(w); free(b);
}

int envelope() {
  int i; float max=0;          
  // find new brkpt w/ largest error
  for (i=1;i<frames-1;i++) {
    if (fabs(w[i]-a[i]) >= max) {
      //P("w-a=%f\n",fabs(w[i]-a[i]));
      max = a[i];
      brk = i;
    }   
  }
  w[brk] = max;
  P("working amps\n");
  for (i=0;i<frames;i++) {
    P("w%.2d %4.2f\n",i,w[i]);
  }
  return brk;
}
  
void interpolate(int brindex) {
  int x, i,j, x1, x2, temp;
  float y1, y2;

  for (i=0;i<nbreaks;i++) {
    P("\nbi=%d\n",b[i]);
    if (b[i]>1999) {
      P("\n>1999\n");
      x1=x2; y1=w[x2];
      x2=frames-1;  y2=0;
    }
    if (b[i] == 0) {
      x1=0; y1=w[x1];
      x2=b[i+1]; y2=w[x2];
    } else if (y2 != 0) {
      x1=b[i];  y1=w[x1];
      x2=b[i+1];y2=w[x2];
    }   
    for (j=x1;j<x2;j++) {
      x=j;
      if (x>=x1 && x<=x2) {
        w[j]=y1 + (y2-y1) * (x - x1)/(x2 - x1);
      }
    }
  }
 
}

int
compare_ints (const void *a, const void *b)
{
  const int *da = (const int *) a;
  const int *db = (const int *) b;

  return (*da > *db) - (*da < *db);
}