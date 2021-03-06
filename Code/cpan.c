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
#define NBREAKS 8
#define NHARMS 30
int anread(char*, int);             /* 05/06 /96 */

/*    global variables declared as externs in monan.h need a root position  */
HEADER header;
int nhar, nhar1, nchans, npts;
float *cmag, *dfr, *phase, *br, *time, tl, dt, fa, smax, *newmag, *newfr,
  *finalfr;
double ampscale;

float *w, *a, **ampData, **timeData;
int   *b, LR[2], brk, frames, harms, Nbk;

int  findBreak();
void findLR(int Nbk);
void interpolate(int L, int R);
void makeSAOL(char *filename);
void makeSASL(char *filename); 
int compare_ints (const void *a, const void *b);

int main(int argc, char **argv)
{
  int i,j,k;
  
  // read in an analysis file
  anread(argv[1],-1);

  frames = npts;
  harms = NHARMS;

  if (argc == 3) 
    Nbk = atoi(argv[2]) * tl + 2;
  else
    Nbk = NBREAKS * tl + 2;

  // allocate space for the final data structures
  ampData = (float **)malloc(harms*sizeof(float*));
  timeData = (float **)malloc(harms*sizeof(float*));
  for (i=0;i<harms;i++) {
    ampData[i] = (float *)malloc(Nbk*sizeof(float));
    timeData[i] = (float *)malloc(Nbk*sizeof(float));
  }

  a = (float *) calloc(frames,sizeof(float));
  w = (float *) calloc(frames,sizeof(float));
  b = (int *) calloc(Nbk,sizeof(int));

  // generate amplitude/time data for each harmonic
  for (k=1;k<=harms;k++) { 
    
    // copy actual data
    for (i=0;i<frames;i++) {
      a[i] = cmag[k + i*nhar1];
    }
    // set all working points to zero
    for (i=0;i<frames;i++) {
      w[i] = 0.0; 
    }    
    // reset brkpts & set x=0 as first break point
    for (i=0;i<Nbk;i++) { b[i]=-1; }
    b[0]=0; 
    brk=0; 

    // find break point, interpolate segments from L -> brk -> R   
    for (i=0;i<Nbk;i++) {
      b[i] = findBreak();               //P("k:%d\tbrk:%d\tb[%d]=%d\n",k,brk,i,b[i]);
      findLR(Nbk);
      // if (LR[0]!=b[i] && brk!=LR[1]) {
      interpolate(LR[0], b[i]);
      interpolate(b[i], LR[1]);
      // } else if (LR[0]==b[i]) {
      //   interpolate(b[i],LR[1]);
      // }    
    }   
 
    // set x=frames-1 as the last break point, then sort
    b[Nbk-1]=frames-1; brk=Nbk-1;
    qsort(b,Nbk,sizeof (int),compare_ints);

    // copy amps & time deltas
    for (i=0;i<Nbk;i++) {
      if (i==0) {
        timeData[k-1][i] = b[i]*dt;
      } else {
        timeData[k-1][i] = (b[i] - b[i-1])*dt; 
      }    
      if (i == Nbk-1) {
        ampData[k-1][i]  = 0; 
      } else {
        ampData[k-1][i]  = w[b[i]] / 32768; // *32768 is scalar*
      }
    }    
  }

  makeSAOL(argv[1]);
  makeSASL(argv[1]);
  free(a); free(w); free(b); 
  free(ampData); free(timeData);
}

// finds breaks based on largest margin of error
int findBreak () {
  int i; float max=0;          
  for (i=0;i<frames;i++) {
    if (fabs(w[i]-a[i]) > max) {
      max = fabs(w[i]-a[i]);
      brk = i;
    }   
  }
  w[brk] = a[brk];          //P("brk=%d\tw=%f\n",brk,w[brk]);
  
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
  LR[0] = L; LR[1] = R;     //P("L=%d   b=%d   R=%d\n",LR[0],brk,LR[1]);
}

// uses linear interpolation to approximate points between breaks
void interpolate (int L, int R) {
  int i,x, x1=L, x2=R;
  float y1=w[x1], y2=w[x2];
  for (i=x1;i<=x2;i++) {
    x=i;
    if (x2-x1==0) {
      x2++;
    }
    w[i]=y1 + (y2-y1) * (x - x1)/(x2 - x1);
  }
}


void makeSAOL (char *filename) {
  int i,k;
  FILE *fp;
  char env[harms][6], y[harms][4];
  char instr[6]; char fname[11];

  // create instr name & output file name
  memcpy(instr,filename,5); instr[5] = '\0';
  memcpy(fname,filename,5); fname[5] = '\0';
  strcat(fname,".saol"); fname[10] = '\0';
  fname[0] = tolower(fname[0]); instr[0] = tolower(instr[0]);
 
  // create variable declaration strings
  for (k=0;k<harms;k++) {
    snprintf(env[k], 6, "%s%.2d", "env", k+1);
    snprintf(y[k], 4, "%s%.2d", "y", k+1);
  }

  // open/create output file
  if (!(fp = fopen(fname,"w+"))) {
    P("Error opening/creating SAOL file\n");
    exit(1);
  }

  // write formatted text and data to file
  fprintf(fp,"global {\n table cyc(harm,128,1);\n srate %.1f;\n}\n\n",header.sr);
  fprintf(fp,"instr %s (fr) {\n imports exports table cyc;\n\n ksig ",instr);
  for (k=0;k<harms;k++) {
    if (k==harms-1)
      fprintf(fp, "%s;\n\n asig ",env[k]);
    else
      fprintf(fp, "%s,",env[k]);
  }
  for (k=0;k<harms;k++) {
    if (k==harms-1)
      fprintf(fp, "%s;\n\n",y[k]);
    else
      fprintf(fp, "%s,",y[k]);
  }

  fprintf(fp,"\n\n// **********************\n// computed during k-pass\n\n");
  for (k=0;k<harms;k++) {
    for (i=0;i<Nbk;i++) {
      if (i==0) 
        fprintf(fp, "%s = kline(0,\n",env[k]);      
      else if (i==Nbk-1)
        fprintf(fp, "\t%f, %f);\n\n",timeData[k][i], ampData[k][i]);
      else 
        fprintf(fp, "\t%f, %f,\n",timeData[k][i], ampData[k][i]);
    }         
  }

  fprintf(fp,"\n\n// **********************\n// computed during a-pass\n\n");
  for (k=0;k<harms;k++) {
    fprintf(fp, "%s = %s * oscil(cyc, fr * %d);\n",y[k],env[k],k+1);       
  }  
  fprintf(fp,"\noutput(");
  for (k=0;k<harms;k++) {
    if (k==harms-1) 
      fprintf(fp,"%s);\n\n}", y[k]);
    else 
      fprintf(fp,"%s+", y[k]);
  }  

  P("%s file created\n",fname);
  fclose(fp);
}

/*
  SASL file format:
  0.0 <instr name> <endtime> <frequency>
  <endtime> end
*/
void makeSASL (char *filename) {
  FILE *fp;
  char env[harms][6], y[harms][4];
  char instr[6]; char fname[11];

  // create instr name & output file name
  memcpy(instr,filename,5); instr[5] = '\0';
  memcpy(fname,filename,5); fname[5] = '\0';
  strcat(fname,".sasl"); fname[10] = '\0';
  fname[0] = tolower(fname[0]); instr[0] = tolower(instr[0]);

  // open/create output file
  if (!(fp = fopen(fname,"w+"))) {
    P("Error opening/creating SASL file\n");
    exit(1);
  }
  fprintf(fp, "0.0 %s %.2f %.0f\n%.2f end\n", instr, tl, fa, tl);

  P("%s file created\n",fname);
  fclose(fp);
}

// compare_function for qsort()
int compare_ints (const void *a, const void *b) {
  const int *da = (const int *) a;
  const int *db = (const int *) b;

  return (*da > *db) - (*da < *db);
}