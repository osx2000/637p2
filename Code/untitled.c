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
#include <ctype.h>
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
int   *b, LR[2], brk, frames, harms, **timeData;

int  findBreak();
void findLR(int Nbk);
void interpolate(int L, int R);
void makeSAOL(char *fname);
void makeSASL(char *fname); 
int compare_ints (const void *a, const void *b);

int main(int argc, char **argv)
{
  int i,j,k,Nbk;

  // read in an analysis file
  anread(argv[1],-1);

  printf("# harmonics = %d # timepoints = %d\n",nhar, npts);
  frames = npts;
  harms = nhar;

  if (argc == 3) 
    Nbk = atoi(argv[2]);
  else
    Nbk = N;

  a = (float *) calloc(frames,sizeof(float));
  w = (float *) calloc(frames,sizeof(float));
  b = (int *) calloc(Nbk,sizeof(int));
  ampData = (float **) calloc(harms,sizeof(float *));
  timeData = (float **) calloc(harms,sizeof(float *));

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
    // sort, then set x=frames-1 as the last break point
    qsort(b,nbreaks,sizeof (int),compare_ints);
    b[Nbk-1]=frames-1; brk=Nbk-1; 
    // save this harmonics arrays of data
    ampData[k] = w; // amps
    timeData[k]= b; // breakpts
  }
  makeSAOL(argv[1]);

  free(a); free(w); free(b); free(ampData); free(timeData);
}

void makeSAOL (char *filename) {
  int k;
  FILE *fp;
  char env[harms][6], y[harms][4];
  char instr[6]; char fname[11];

  // create instr name & output file name
  memcpy(instr,filename,5); instr[5] = '\0';
  memcpy(fname,filename,5);
  strcat(fname,".saol"); fname[10] = '\0';
  fname[0] = tolower(fname[0]); instr[0] = tolower(instr[0]);
 
  // create variable declaration strings
  for (k=0;k<harms;k++) {
    snprintf(env[k], 6, "%s%.2d", "env", k+1);
    snprintf(y[k], 4, "%s%.2d", "y", k+1);
    //P("e: %s, y: %s\n",env[k],y[k]);
  }

  // open/create output file
  if (!(fp = fopen(fname,"w+"))) {
    P("Error opening/creating SAOL file\n");
    exit(1);
  }

  // write formatted text and data to file
  fprintf(fp,"global {\n table cyc(harm,128,1);\n srate 44100;\n}\n");
  fprintf(fp,"instr %s (fr) {\n imports exports table cyc;\n ivar scalar;\n ksig ",instr);
  for (k=0;k<harms;k++) {
    if (k==harms-1)
      fprintf(fp, "%s;\n asig ",env[k]);
    else
      fprintf(fp,"%s,",env[k]);
  }
  for (k=0;k<harms;k++) {
    if (k==harms-1)
      fprintf(fp, "%s;\n",y[k]);
    else
      fprintf(fp,"%s,",y[k]);
  }
  fprintf(fp," scalar = 32768;\n// **********************\n// computed during k-pass\n");




  fclose(fp);
}

void makeSASL (char *fname) {
  FILE *fp;

  // creates output filename
  char instr[11]; memcpy(instr,fname,5); 
  strcat(instr,".sasl"); instr[10] = '\0';
  instr[0] = tolower(instr[0]);

}

// finds breaks based on largest margin of error
int findBreak () {
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
 
// finds breakpoints to the left & right of current breakpoint 
void findLR (int Nbk) {
  int i, L=0, R=9999;
  for (i=0;i<Nbk;i++) {
    if ((i==Nbk-1) && (R==9999)) {
      R = frames-1;
    } else 
    if (b[i] < brk) {
      if (b[i] >= L) { L = b[i]; }
    }
    if (b[i] > brk) {
      if (b[i] < R) { R = b[i]; }
      else if (b[i] < 0) { R = frames-1; }
    } 
  }
  LR[0] = L; LR[1] = R;
}

// uses linear interpolation to approximate points between breaks
void interpolate (int L, int R) {
  int i,x, x1=L, x2=R;
  float y1=w[x1], y2=w[x2];
  for (i=x1;i<=x2;i++) {
    x=i;
    w[i]=y1 + (y2-y1) * (x - x1)/(x2 - x1);
  }
}

int compare_ints (const void *a, const void *b) {
  const int *da = (const int *) a;
  const int *db = (const int *) b;

  return (*da > *db) - (*da < *db);
}