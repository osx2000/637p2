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
#define N 9
int anread(char*, int);             /* 05/06 /96 */

/*    global variables declared as externs in monan.h need a root position  */
HEADER header;
int nhar, nhar1, nchans, npts;
float *cmag, *dfr, *phase, *br, *time, tl, dt, fa, smax, *newmag, *newfr,
  *finalfr;
double ampscale;

float *w, *a, **ampData;
int   *b, LR[2], brk, frames, **timeData;

int findBreak();
void findLR(int Nbk);
void interpolate(int L, int R);

int main(int argc, char **argv)
{
  int i,j,k,harms,Nbk;

/* read in an analysis file */
  anread(argv[1],-1);

  printf("# harmonics = %d # timepoints = %d\n",nhar, npts);
  frames = npts;
  harms = 1;

  if (argc == 3) 
    Nbk = atoi(argv[2]);
  else
    Nbk = N;

  a = (float *) calloc(frames,sizeof(float));
  w = (float *) calloc(frames,sizeof(float));
  b = (int *) calloc(Nbk,sizeof(int));
  ampData = (float **) calloc(harms,sizeof(float *));
  timeData = (int **) calloc(harms,sizeof(int *));

  //memcpy(a, DUMMY, sizeof(DUMMY));

  // generate amplitude data for each harmonic
  for (k=1;k<=harms;k++) {
 
    // set actual data
    for (i=0;i<frames;i++) {
      a[i] = cmag[k + i*nhar1] / 32768;
    }

    // set all working points to zero
    for (i=0;i<frames;i++) {
      w[i] = 0; 
    }    

    // save startpoint as the first break point
    for (i=1;i<Nbk;i++) { b[i]=-1; }
    b[0]=0; brk=0;    
    for (i=1;i<Nbk-1;i++) {
      b[i] = findBreak();
      findLR(Nbk);
      interpolate(LR[0], b[i]);
      interpolate(b[i], LR[1]);
    }
    b[Nbk-1]=frames-1; brk=0; 

    // save this harmonics arrays of data
    ampData[k] = w; // amps
    timeData[k]= b; // breakpts
  }
  P("working amps\n");
  for (i=0;i<frames;i++) {
    P("w%.2d %8.6f\n",i,w[i]);
  }
  for (i=0;i<Nbk;i++) {
    P("i%d %4d\n",i,b[i]);
  } 
  free(a); free(w); free(b);
}

int findBreak() {
  int i; float max=0;          
  
  for (i=1;i<frames;i++) {
    if (fabs(w[i]-a[i]) > max) {
      max = a[i];
      brk = i;
    }   
  }
  w[brk] = max;
  return brk;
}
 
void findLR (int Nbk) {
  int i, L=0, R=9999;
  for (i=0;i<Nbk;i++) {
    if ((i==Nbk-1) && (R==9999)) {
      R = frames-1;
    } else 
    if (b[i] < brk) {
      if (b[i] >= L) { L = b[i]; }
      //else if (b[i] < 0) { R = frames-1; }
    }
    if (b[i] > brk) {
      if (b[i] < R) { R = b[i]; }
      else if (b[i] < 0) { R = frames-1; }
    } 
  }
  LR[0] = L; LR[1] = R;
  //P("L=%d, brk=%d, R=%d\n",L,brk,R); 
  //P("brk=%d\n",b[brk]); 
}

void interpolate(int L, int R) {
  int i,x, x1=L, x2=R;
  float y1=w[x1], y2=w[x2];

  for (i=x1;i<=x2;i++) {
    x=i;
    w[i]=y1 + (y2-y1) * (x - x1)/(x2 - x1);
  }
}
