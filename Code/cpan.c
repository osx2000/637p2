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
#define N 18
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

  // printf("Amplitude data:\n");
  // printf("harmonic        1       2       3       4       5\n\n");
  // for (i=0;i<10;i++) {
  //   printf("frame %d %f ",i,i*dt);
  //   for (k=1;k<=5;k++)
  //     printf("%8.2f ",cmag[k + i*nhar1]);
  //   printf("\n");
  // }
  // printf("\nFrequency deviation data:\n");
  // printf("harmonic        1       2       3       4       5\n\n");
  // for (i=0;i<frames;i++) {
  //   printf("frame %d ",i);
  //   for (k=1;k<=5;k++)
  //     printf("%8.2f ",dfr[k + i*nhar1]);
  //   printf("\n");
  // }

  // generate amplitude data for each harmonic
  for (k=1;k<=harms;k++) {
 
    // copy actual data
    for (i=0;i<frames;i++) {
      a[i] = cmag[k + i*nhar1];
    }

    // set all working points to zero
    for (i=0;i<frames;i++) {
      w[i] = 0; 
    }    

    // reset brkpts & set x=0 as first break point
    for (i=1;i<Nbk;i++) { b[i]=-1; }
    b[0]=0; brk=0;    
    for (i=1;i<Nbk-1;i++) {
      b[i] = findBreak();
      findLR(Nbk);
      interpolate(LR[0], b[i]);
      interpolate(b[i], LR[1]);
    }
    // set x=frames-1 as the last break point
    b[Nbk-1]=frames-1; brk=Nbk-1; 

    // save this harmonics arrays of data
    ampData[k] = w; // amps
    timeData[k]= b; // breakpts
  }
  // P("working amps\n");
  // for (i=0;i<frames;i++) {
  //   P("w%.2d %8.6f\n",i,w[i]);
  // }
  // P("1d\n");
  // for (i=0;i<Nbk;i++) {
  //   P("i%d %4d\n",i,b[i]);
  // } 
  P("generated amps\t\tactual amps\n");
  P("--------------\t\t-----------\n");
  for (i=0;i<frames;i++) {
    P("w%.2d %8.2f\t\t",i,ampData[1][i]);
    P("w%.2d %8.2f\n",i,cmag[1 + i*nhar1]);
  }
  // for (i=0;i<10;i++) {
  //   printf("frame %d %f ",i,i*dt);
  //   for (k=1;k<=5;k++)
  //     printf("%8.2f ",cmag[k + i*nhar1]);
  //   printf("\n");
  // }  
  // P("break point\n");
  // for (k=1;k<=harms;k++) {
  //   P("harmonic %d\n",k);
  //   for (i=0;i<Nbk;i++) {
  //     P("i%d %4d\n",i,timeData[k][i]);
  //   }     
  // }

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
