
/*
#    Sfront, a SAOL to C translator    
#    This file: Code for table generators
#
# Copyright (c) 1999-2006, Regents of the University of California
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met:
#
#  Redistributions of source code must retain the above copyright
#  notice, this list of conditions and the following disclaimer.
#
#  Redistributions in binary form must reproduce the above copyright
#  notice, this list of conditions and the following disclaimer in the
#  documentation and/or other materials provided with the distribution.
#
#  Neither the name of the University of California, Berkeley nor the
#  names of its contributors may be used to endorse or promote products
#  derived from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
#    Maintainer: John Lazzaro, lazzaro@cs.berkeley.edu
*/


int tgen_init(ENGINE_PTR_DECLARE_COMMA tableinfo * ti, int pnum, float * p)

{
  int i;

  if (ti->llmem)
    free(ti->t); 
  ti->start = 0;
  ti->end = 0;
  ti->sr = 0;
  ti->dint = 0;
  ti->dfrac = 0;
  ti->base = 0;
  ti->stamp = EV(scorebeats);
  if ((pnum < 1) || ((i = ti->len = ROUND(p[0])) < 1))
    epr(0,"control driver","tgen_init","Table length < 1");
  ti->lenf = (float) i;
  ti->oconst = ((float) i)*EV(ATIME);
  ti->tend = i - 1;
  ti->t = (float *) calloc(i+1, sizeof(float));
  ti->llmem = 1;

#if INTERP_TYPE == INTERP_SINC  
  ti->sffl = 1.0F;
  ti->sfui = 0x00010000;
  ti->dsincr = SINC_PILEN;
#endif

  return i;
}


void tgen_buzz(ENGINE_PTR_DECLARE_COMMA tableinfo * ti, int pnum, float * p)  

{
  int size, nharm, loharm, i, j;
  float scale, base, index, acc, f;

  if (pnum < 4)
    epr(0,"control driver","tgen_buzzwave","Too few parameters");

  size = ROUND(p[0]);
  nharm = ROUND(p[1]);
  loharm = ROUND(p[2]);

  if (loharm < 0)
    epr(0,"control driver","tgen_buzzwave","Loharm must be >= 0");

  if ((size < 1) && (nharm < 1))
    epr(0,"control driver","tgen_buzzwave","Size or nharm must be given");
  if (nharm < 1)
    nharm = (int) floor(size/2.0 - loharm);
  else
    if (size < 1)
      size = 2*(loharm + nharm);

  if (((f = fabs(p[3])) < 1.0F) && (f != 0.0F))
    {
      j = -(int)(17.0F/log(f)) + loharm + 1;
      nharm = (nharm > j) ? j : nharm;
    }

  p[0] = (float) size;
  p[1] = (float) nharm;
  p[2] = (float) loharm;

  tgen_init(ENGINE_PTR_COMMA ti, pnum, p);

  if ((p[3] == 1.0F)||(p[3] == -1.0F))
    scale = 1.0F;
  else
    {
      if (p[3] == 0.0F)
	scale = 1.0F;
      else
	scale = (1.0F-(float)fabs(p[3]))/
	  (1-(float)fabs((float)pow(p[3],p[1]+1.0F)));
    }

   base = 6.283185F/p[0];
   for (i = 0; i < size; i++)
     {
       index = i*base;
       acc = 1.0F;
       j = loharm + 1;
       while (j <= (loharm + nharm))
	 {
	   ti->t[i] += acc*(float)cos(index*j);
	   acc *= p[3];
	   j++;
	 }
       ti->t[i] *= scale;
     }
   ti->t[ti->len] = ti->t[0];

}


void tgen_concat_global(ENGINE_PTR_DECLARE_COMMA tableinfo * ti, int pnum, float * p)  

{
  int i, j, k, len, tidx;

  if (pnum < 2)
    epr(0,"control driver","tgen_concat","Too few parameters");

  len = 0;
  for (j = 1; j < pnum; j++)
    {
      i = ROUND(p[j]);
      if ( (!EV(gtables)[i].t) || (EV(gtables)[i].len < 1))
	epr(0,"control driver","tgen_concat","Uninitialized/zero-lenth table");
      len += EV(gtables)[i].len;
    }

  if (p[0] <= 0.0F)
    {
      p[0] = len;
      tgen_init(ENGINE_PTR_COMMA ti, pnum, p);
    }
  else
    {
      j = tgen_init(ENGINE_PTR_COMMA ti, pnum, p);
      len = (j < len) ? j : len; 
    }

  i = 0;
  tidx = ROUND(p[k = 1]);
  j = 0;
  while ((i < len) && (k < pnum))
    {
      if (j < EV(gtables)[tidx].len)
	ti->t[i++] = EV(gtables)[tidx].t[j++];
      else
	{
	  j = 0;
	  if (++k < pnum)
	    tidx = ROUND(p[k]);
	}
    }
  ti->t[ti->len] = ti->t[0];

}

void tgen_cubicseg(ENGINE_PTR_DECLARE_COMMA tableinfo * ti, int pnum, float * p) 

{
  int i, j, len, endpoint;
  float Q, R, S, T, a, b, c, d, xf;

  if (pnum < 7)
    epr(0,"control driver","tgen_cubicseg","Too few parameters");

  if ((pnum - 3) % 4)
    epr(0,"control driver","tgen_cubicseg",
	"Parmeter list must end with (infl, y)");
    
  if ((len = ROUND(p[1])) != 0)
    epr(0,"control driver","tgen_cubicseg","x0 != 0");
  i = 3;

  while (i < pnum)
    {
      j = ROUND(p[i]);
      if (j < len)
	epr(0,"control driver","tgen_cubicseg",
	    "Consecutive x and infl values decreasing."); 
      p[i] = (float)(len = j);
      i += 2;
    }

  if (p[0] <= 0.0F)
    {
      p[0] = len;
      tgen_init(ENGINE_PTR_COMMA ti, pnum, p);
    }
  else
    {
      j = tgen_init(ENGINE_PTR_COMMA ti, pnum, p);
      len = (j < len) ? j : len; 
    }

  i = 0;
  xf = 0.0F;

  for (j = 1; j < pnum; j += 4)
    {
      endpoint = p[j+4];

      Q = p[j]*p[j]*p[j] - p[j+2]*p[j+2]*p[j+2] 
	- 3.0F*p[j+2]*p[j+2]*(p[j]-p[j+2]);
      if (Q == 0.0F)
	epr(0,"control driver","tgen_cubicseg",
	    "No cubic solution for these parameters");
      R = p[j]*p[j] - p[j+2]*p[j+2] - 2.0F*p[j+2]*(p[j]-p[j+2]);
      if (R == 0.0F)
	epr(0,"control driver","tgen_cubicseg",
	    "No cubic solution for these parameters");
      
      S = p[j+2]*p[j+2]*p[j+2] - p[j+4]*p[j+4]*p[j+4]
	- 3.0F*p[j+2]*p[j+2]*(p[j+2]-p[j+4]);

      if (S == 0.0F)
	epr(0,"control driver","tgen_cubicseg",
	    "No cubic solution for these parameters");

      T = p[j+2]*p[j+2] - p[j+4]*p[j+4] - 2.0F*p[j+2]*(p[j+2]-p[j+4]);

      if ((T == 0.0F) || ((Q/R) == (S/T)))
	epr(0,"control driver","tgen_cubicseg",
	    "No cubic solution for these parameters");

      a = 1.0F/((Q/R) - (S/T));
      a *= (p[j+1]-p[j+3])/R - (p[j+3]-p[j+5])/T;
      b = 1.0F/((R/Q) - (T/S));
      b *= (p[j+1]-p[j+3])/Q - (p[j+3]-p[j+5])/S;
      c = - 3.0F*a*p[j+2]*p[j+2] - 2.0F*b*p[j+2];
      d = p[j+3] - a*p[j+2]*p[j+2]*p[j+2];
      d += - b*p[j+2]*p[j+2] - c*p[j+2];
    
      while (i < endpoint)
	{
	  ti->t[i] =  xf*xf*xf*a + xf*xf*b + xf*c + d;
	  xf = (++i);
	}
    }
  ti->t[ti->len] = ti->t[0];

}

void tgen_data(ENGINE_PTR_DECLARE_COMMA tableinfo * ti, int pnum, float * p)  

{  
  int len, i;
  
  if (pnum < 1)
    epr(0,"control driver","tgen_data","Too few parameters");

  if (p[0] <= 0.0F)
    p[0] = pnum - 1;

  len = tgen_init(ENGINE_PTR_COMMA ti, pnum, p);
  len = (pnum - 1 < len) ? pnum - 1 : len;

  i = 0;
  while (i < len)
    {
      ti->t[i] = p[i+1];
      i++;
    }
  ti->t[ti->len] = ti->t[0];

}

void tgen_destroy(ENGINE_PTR_DECLARE_COMMA tableinfo * ti, int pnum, float * p)  

{  
  ti->len = ti->start = ti->end = ti->tend = 0;
  ti->lenf = ti->oconst = ti->sr = ti->base = 0.0F;
  ti->dint = ti->dfrac = 0;
  ti->sffl = 0;
  ti->sfui = 0;
  ti->dsincr = 0;
  if (ti->llmem)
    free(ti->t); 
  ti->llmem = 0;
  ti->stamp = EV(scorebeats);
}

void tgen_empty(ENGINE_PTR_DECLARE_COMMA tableinfo * ti, int pnum, float * p)  

{
  tgen_init(ENGINE_PTR_COMMA ti, pnum, p);
}


void tgen_expseg(ENGINE_PTR_DECLARE_COMMA tableinfo * ti, int pnum, float * p) 

{
  int i, j, len, dx, nx, sgn;
  float diff;

  if (pnum < 5)
    epr(0,"control driver","tgen_expseg","Too few parameters");

  if (!(pnum % 2))
    epr(0,"control driver","tgen_expseg","Last parameter is an xval");
  
  if ((len = ROUND(p[1])) != 0)
    epr(0,"control driver","tgen_expseg","x0 != 0");

  if (p[2] == 0.0F)
    epr(0,"control driver","tgen_expseg","illegal yval");
  sgn = (p[2] > 0);

  i = 3;

  while (i < pnum)
    {
      if ((p[i+1] == 0.0F) || ((p[i-1] > 0) != sgn))
	epr(0,"control driver","tgen_expseg","illegal yval");

      j = ROUND(p[i]);
      if (j < len)
	epr(0,"control driver","tgen_expseg","Decreasing xval in sequence");
      len = j;
      i += 2;
    }

  if (p[0] <= 0.0F)
    {
      p[0] = len;
      tgen_init(ENGINE_PTR_COMMA ti, pnum, p);
    }
  else
    {
      j = tgen_init(ENGINE_PTR_COMMA ti, pnum, p);
      len = (j < len) ? j : len; 
    }

  i = j = 1;
  while ((j + 2) < pnum)
    if (!(dx = ((nx = ROUND(p[j+2])) - ROUND(p[j]))))
      j += 2;
    else
      {
	diff = (float)pow(p[j+3]/p[j+1], 1.0F/dx);
	ti->t[ti->len] = ti->t[0] = p[j+1];
	break;
      }

  while ((i < len) && ((j + 2) < pnum))
    {
      if (nx > i)
	{
	  ti->t[i] = ti->t[i-1]*diff;
	  i++;
	}
      else
	{
	  j += 2;
	  while ((j + 2) < pnum)
	    if (!(dx = ((nx =ROUND(p[j+2])) - ROUND(p[j]))))
	      j += 2;
	    else
	      {
		diff = (float)pow(p[j+3]/p[j+1], 1.0F/dx);
		ti->t[i++] = p[j+1];
		break;
	      }
	}
    }

}


void tgen_harm(ENGINE_PTR_DECLARE_COMMA tableinfo * ti, int pnum, float * p)

{
  int i, j;
  float base, index;

  i = tgen_init(ENGINE_PTR_COMMA ti, pnum, p);
  base = 6.283185F/i;
  i--;

  while (i >= 0)
    {
      j = 0;
      index = i*base;
      while (++j < pnum)
	ti->t[i] += p[j]*(float)sin(j*index);
      i--;
    }
  ti->t[ti->len] = ti->t[0];

}

void tgen_harm_phase(ENGINE_PTR_DECLARE_COMMA tableinfo * ti, int pnum, float * p)

{
  int i, j;
  float base, index, k;

  i = tgen_init(ENGINE_PTR_COMMA ti, pnum, p);
  base = 6.283185F/i;
  i--;

  if (!(pnum % 2))
    epr(0,"control driver","tgen_harm_phase","Must have (f1, ph1) pairs.");

  while (i >= 0)
    {
      j = -1;
      k = 1.0F;
      index = i*base;
      while ((j += 2) < pnum)
	ti->t[i] += p[j]*(float)sin((k++)*index + p[j+1]);
      i--;
    }
  ti->t[ti->len] = ti->t[0];

}


void tgen_lineseg(ENGINE_PTR_DECLARE_COMMA tableinfo * ti, int pnum, float * p) 

{
  int i, j, len, dx, nx;
  float diff;

  if (pnum < 5)
    epr(0,"control driver","tgen_lineseg","Too few parameters");

  if (!(pnum % 2))
    epr(0,"control driver","tgen_lineseg","Last parameter is an xval");
  
  if ((len = ROUND(p[1])) != 0)
    epr(0,"control driver","tgen_lineseg","x0 != 0");
  i = 3;

  while (i < pnum)
    {
      j = ROUND(p[i]);
      if (j < len)
	epr(0,"control driver","tgen_lineseg","Decreasing xval in sequence");
      len = j;
      i += 2;
    }

  if (p[0] <= 0.0F)
    {
      p[0] = len;
      tgen_init(ENGINE_PTR_COMMA ti, pnum, p);
    }
  else
    {
      j = tgen_init(ENGINE_PTR_COMMA ti, pnum, p);
      len = (j < len) ? j : len; 
    }

  i = j = 1;
  while ((j + 2) < pnum)
    if (!(dx = ((nx = ROUND(p[j+2])) - ROUND(p[j]))))
      j += 2;
    else
      {
	diff = (p[j+3]-p[j+1])/dx;
	ti->t[ti->len] = ti->t[0] = p[j+1];
	break;
      }

  while ((i < len) && ((j + 2) < pnum))
    {
      if (nx > i)
	{
	  ti->t[i] = ti->t[i-1] + diff;
	  i++;
	}
      else
	{
	  j += 2;
	  while ((j + 2) < pnum)
	    if (!(dx = ((nx =ROUND(p[j+2])) - ROUND(p[j]))))
	      j += 2;
	    else
	      {
		diff = (p[j+3]-p[j+1])/dx;
		ti->t[i++] = p[j+1];
		break;
	      }
	}
    }

}

void tgen_periodic(ENGINE_PTR_DECLARE_COMMA tableinfo * ti, int pnum, float * p)  

{
  int i, j;
  float base, index;

  i = tgen_init(ENGINE_PTR_COMMA ti, pnum, p);
  base = 6.283185F/i;
  i--;

  if ((pnum-1) % 3)
    epr(0,"control driver","tgen_periodic","Must have (p1, f1, ph1) triplets.");

  while (i >= 0)
    {
      j = -2;
      index = i*base;
      while ((j += 3) < pnum)
	ti->t[i] += p[j+1]*(float)sin(p[j]*index + p[j+2]);
      i--;
    }
  ti->t[ti->len] = ti->t[0];

}


void tgen_polynomial(ENGINE_PTR_DECLARE_COMMA tableinfo * ti, int pnum, float * p) 

{
  float scale, yval, ynth;
  int i, j;

  if (pnum < 4)
    epr(0,"control driver","tgen_polynomial","Too few parameters");

  tgen_init(ENGINE_PTR_COMMA ti, pnum, p);
    
  scale = (1.0F/p[0])*(p[2]-p[1]);

  for (i=0; i < ti->len; i++)
    {
      yval = p[1] + scale*(p[0] - i);
      ynth = 1.0F;
      ti->t[i] = p[3];
      j = 3;
      while (++j < pnum)
	{
	  ynth *= yval;
	  ti->t[i] += p[j]*ynth;
	}
    }

  ti->t[ti->len] = ti->t[0];

}

#define RAN_UNIFORM 1
#define RAN_LINRAMP 2
#define RAN_EXPO    3
#define RAN_GAUSS   4
#define RAN_PPROC   5

void tgen_random(ENGINE_PTR_DECLARE_COMMA tableinfo * ti, int pnum, float * p)  

{
  int dist, i, j;
  float c1, x, y;

  if (pnum < 3)
    epr(0,"control driver","tgen_random","Too few parameters");

  tgen_init(ENGINE_PTR_COMMA ti, pnum, p);

  dist = ROUND(p[1]);
    
  switch(dist) {
  case RAN_UNIFORM:
  case RAN_LINRAMP:
    if (pnum < 4)
      epr(0,"control driver","tgen_random","Too few parameters");
    c1 = p[3] - p[2];
    if ((dist == 2) && !(c1))
      epr(0,"control driver","tgen_random","p1 == p2 (dist 2)");
    break;
  case RAN_GAUSS:
    if (pnum < 4)
      epr(0,"control driver","tgen_random","Too few parameters");
    if (p[3] <= 0.0F)
      epr(0,"control driver","tgen_random","p2 <= 0 (dist 4)");
    c1 = (float)sqrt(2.0*p[3]);
    break;
  case RAN_EXPO:
  case RAN_PPROC:
    break;
  default:
    epr(0,"control driver","tgen_random","Distribution not 1,2,3,4 or 5.");
  }

  j = 0; i = 0;
  while (i < ti->len)
    {
      switch(dist) {
      case RAN_UNIFORM:
        ti->t[i] = c1*RMULT*((float)rand()) + p[2];
        break;
      case RAN_LINRAMP:
        x = RMULT*((float)rand());
        y = RMULT*((float)rand());
        if (x > y)
          ti->t[i] =  c1*x + p[2];
        else
          ti->t[i] =  c1*y + p[2];
        break;
      case RAN_EXPO:
        ti->t[i] = -p[2]*(float)log(RMULT*((float)rand())+1e-45F);
        break;
      case RAN_GAUSS:
        ti->t[i] = p[2]+c1*
	  (float)sqrt(-2.0F*(float)log(RMULT*((float)rand())+1e-45F))
	  *(float)cos(6.283185F*RMULT*((float)rand()));
        break;
      case RAN_PPROC:
        ti->t[i] = 0;
        if (j == 0)
	  {
	    j = ROUND(-p[2]*(float)log(RMULT*((float)rand())+1e-45F))+1;
	    if (i != 0)
	      ti->t[i] = 1.0F;
	  }
	j--;
        break;
      }
      i++;
    }
  ti->t[ti->len] = ti->t[0];

}

#undef RAN_UNIFORM 
#undef RAN_LINRAMP 
#undef RAN_EXPO    
#undef RAN_GAUSS   
#undef RAN_PPROC   


#define SAMP_SR        5
#define SAMP_LOOPSTART 4
#define SAMP_LOOPEND   3
#define SAMP_BASEFREQ  2
#define SAMP_LLMEM     1
#define SAMP_DATABLOCK 5

void tgen_sample(ENGINE_PTR_DECLARE_COMMA tableinfo * ti, int pnum, float * p)  

{
  int i;
  double intdummy;

  if (pnum < SAMP_DATABLOCK + 1)
    epr(0,"control driver","tgen_sample","Zero-length sample");

  ti->len = pnum - SAMP_DATABLOCK;
  ti->lenf = (float)(pnum - SAMP_DATABLOCK);
  ti->oconst = ti->lenf*EV(ATIME);

  if (ti->llmem)
    free(ti->t); 
  ti->t = p;

  ti->stamp = EV(scorebeats);

  if (p[pnum - SAMP_SR] > 0)
    {
      ti->sr = p[pnum - SAMP_SR];
      if (p[pnum - SAMP_SR] == EV(ARATE))
	{
	  ti->dint = 1;
	  ti->dfrac = 0;
	}
      else
	{
	  ti->dfrac = 4294967296.0*
	    modf(((double)p[pnum - SAMP_SR])/((double)EV(ARATE)), &intdummy);
	  ti->dint = intdummy;
	}
    }
  else
    {
      ti->sr = EV(ARATE);
      ti->dint = 1;
      ti->dfrac = 0;
    }

#if (INTERP_TYPE == INTERP_SINC)

  if (p[pnum - SAMP_SR] > 0)
    {
      if (EV(ARATE) >= p[pnum - SAMP_SR])
	{
	  ti->sffl = 1.0F;
	  ti->sfui = 0x00010000;
	  ti->dsincr = SINC_PILEN;
	}
      else
	{
	  if ((EV(ARATE)*SINC_UPMAX) >= p[pnum - SAMP_SR])
	    ti->sffl = (EV(ARATE)/ti->sr);
	  else
	    ti->sffl = (1.0F/SINC_UPMAX);
	  ti->sfui = ((float)(pow(2,16)))*ti->sffl + 0.5F;
	  ti->dsincr = (SINC_PILEN*ti->sfui) >> 16;
	}
    }
  else
    {
      ti->sffl = 1.0F;
      ti->sfui = 0x00010000;
      ti->dsincr = SINC_PILEN;
    }

#endif

  if (p[pnum - SAMP_BASEFREQ] > 0)
    {
      ti->base = p[pnum - SAMP_BASEFREQ];
    }
  else
    {  
      ti->base = 0;
    }

  if (p[pnum - SAMP_LOOPSTART] > 0)
    {
      ti->start = p[pnum - SAMP_LOOPSTART];
    }
  else
    {
      ti->start = 0;
    }

  if (p[pnum - SAMP_LOOPEND] > 0)
    {
      ti->tend = ti->end = p[pnum - SAMP_LOOPEND];
      if (ti->end==0)
	{
	  ti->tend = ti->len - 1;
	}
    }
  else
    {
      ti->end = 0;
      ti->tend = i - 1;
    }
  ti->llmem = (int)(p[pnum - SAMP_LLMEM]);
  ti->t[ti->len] = ti->t[0];
}

#undef SAMP_SR 
#undef SAMP_LOOPSTART 
#undef SAMP_LOOPEND 
#undef SAMP_BASEFREQ 
#undef SAMP_DATABLOCK 


void tgen_spline(ENGINE_PTR_DECLARE_COMMA tableinfo * ti, int pnum, float * p)  

{
  int i, j, len, endpoint, newpoint;
  float Q, R, S, a, b, c, d, k1, k2, xf;

  if (pnum < 6)
    epr(0,"control driver","tgen_spline","Too few parameters");

  if ((pnum - 3) % 3)
    epr(0,"control driver","tgen_spline",
	"Parmeter list must end with (x, y)");
    
  if ((len = ROUND(p[1])) != 0)
    epr(0,"control driver","tgen_cubicseg","x0 != 0");
  i = 4;

  while (i < pnum)
    {
      j = ROUND(p[i]);
      if (j < len)
	epr(0,"control driver","tgen_spline",
	    "Consecutive x values decreasing.");
      p[i] = (float)(len = j);
      i += 3;
    }

  if (p[0] <= 0.0F)
    {
      p[0] = len;
      tgen_init(ENGINE_PTR_COMMA ti, pnum, p);
    }
  else
    {
      j = tgen_init(ENGINE_PTR_COMMA ti, pnum, p);
      len = (j < len) ? j : len; 
    }

  endpoint = i = 0;
  xf = 0.0F;

  for (j = 1; (j+4) < pnum; j += 3)
    {

      if ((newpoint = p[j+3]) != endpoint)
	{
	  endpoint = newpoint;

	  if (j == 1)
	    k1 = 0.0F;
	  else
	    k1 = p[j-1];
	  if ((j+5) == pnum)
	    k2 = 0;
	  else
	    k2 = p[j+2];

	  Q = 1.0F/(p[j] - p[j+3]);
	  R = Q*(p[j]*p[j] - p[j+3]*p[j+3]) - 2.0F*p[j];
	  if (R == 0.0F)
	    epr(0,"control driver","tgen_splice",
		"No spline solution for these parameters");
	  R =1.0F/R;
	  S = Q*(p[j]*p[j] - p[j+3]*p[j+3]) - 2.0F*p[j+3];
	  if (S == 0.0F)
	    epr(0,"control driver","tgen_splice",
		"No spline solution for these parameters");
	  S =1.0F/S;
	  
	  a = Q*(p[j]*p[j]*p[j]-p[j+3]*p[j+3]*p[j+3])*(R-S);
	  a += -3.0F*(R*p[j]*p[j] - S*p[j+3]*p[j+3]);
	  if (a==0.0F)
	    epr(0,"control driver","tgen_spline",
		"No spline solution for these parameters");
	  a =1.0F/a;
	  a *= Q*(p[j+1]-p[j+4])*(R-S) - R*k1 + S*k2;
	  b = Q*(k1-k2)*0.5F;
	  b += - 1.5F*a*Q*(p[j]*p[j] - p[j+3]*p[j+3]);
	  c = k1 - 3.0F*a*p[j]*p[j] -2.0F*b*p[j];
	  d=p[j+1]-a*p[j]*p[j]*p[j]-b*p[j]*p[j]-c*p[j];

	  while (i < endpoint)
	    {
	      ti->t[i] =  xf*xf*xf*a + xf*xf*b + xf*c + d;
	      xf = (++i);
	    }
	}
    }
  ti->t[ti->len] = ti->t[0];

}

void tgen_step(ENGINE_PTR_DECLARE_COMMA tableinfo * ti, int pnum, float * p)  

{
  int i, j, len, nx;

  if (pnum < 4)
    epr(0,"control driver","tgen_step","Too few parameters");

  if (pnum % 2)
    epr(0,"control driver","tgen_step","Last parameter is a yval");
  
  if ((len = ROUND(p[1])) != 0)
    epr(0,"control driver","tgen_step","x0 != 0");
  i = 3;

  while (i < pnum)
    {
      j = ROUND(p[i]);
      if (j < len)
	epr(0,"control driver","tgen_step","Decreasing xval in sequence");
      len = j;
      i += 2;
    }

  if (p[0] <= 0.0F)
    {
      p[0] = len;
      tgen_init(ENGINE_PTR_COMMA ti, pnum, p);
    }
  else
    {
      j = tgen_init(ENGINE_PTR_COMMA ti, pnum, p);
      len = (j < len) ? j : len; 
    }

  i = 0; 
  j = 2;
  nx = ROUND(p[3]);
  while (i < len)
    {
      if (nx > i)
	{
	  ti->t[i] = p[j];
	  i++;
	}
      else
	{
	  if ((j += 2) == pnum)
	    break;
	  nx = ROUND(p[j+1]);
	}
    }
  ti->t[ti->len] = ti->t[0];
}


double tgen_bessel(double x)

{
  double sum,a;

  sum = a = 1.0;
  a *= x*x; sum += a*2.5e-1;
  a *= x*x; sum += a*1.5625e-2;
  a *= x*x; sum += a*4.340278e-4;
  a *= x*x; sum += a*6.781684e-6;
  a *= x*x; sum += a*6.781684e-8; 
  a *= x*x; sum += a*4.709503e-10;
  a *= x*x; sum += a*2.402808e-12; 
  a *= x*x; sum += a*9.385967e-15;
  a *= x*x; sum += a*2.896903e-17;
  a *= x*x; sum += a*7.242258e-20;

  return sum;
}


#define WIN_HAMMING  1
#define WIN_HANNING  2
#define WIN_BARTLETT 3
#define WIN_GAUSSIAN 4
#define WIN_KAISER   5
#define WIN_BOXCAR   6

void tgen_window(ENGINE_PTR_DECLARE_COMMA tableinfo * ti, int pnum, float * p)  

{
  int type,i;
  float c1, c2;
  double d1, d2, d3;

  if (pnum < 2)
    epr(0,"control driver","tgen_window","Too few parameters");

  tgen_init(ENGINE_PTR_COMMA ti, pnum, p);

  type = ROUND(p[1]);
    
  switch(type) {
  case WIN_HAMMING:
  case WIN_HANNING:
    c1 = 6.283185F/(p[0]-1.0F);
    break;
  case WIN_BARTLETT:
    c1 = 2.0F/(p[0]-1.0F);
    c2 = 1.0F/c1;
    break;
  case WIN_GAUSSIAN:
    c1 = - 18.0F/(p[0]*p[0]);
    c2 = - 0.5F*p[0];
    break;
  case WIN_KAISER:     
    if (pnum < 3)
      epr(0,"control driver","tgen_window","Parameter p needed (type 5)");
    d1 = (p[0]-1.0)/2.0;
    d2 = d1*d1;
    d3 = 1.0/tgen_bessel(p[2]*d1);
    break;
  case WIN_BOXCAR:
    break;
  default:
    epr(0,"control driver","tgen_window","Window type not 1,2,3,4,5 or 6");
  }

  for (i=0; i < ti->len; i++)
    switch(type) {
    case WIN_HAMMING:
      ti->t[i] = 0.54F - 0.46F*(float)cos(c1*i);
      break;
    case WIN_HANNING:
      ti->t[i] = 0.5F*(1.0F - (float)cos(c1*i));
      break;
    case WIN_BARTLETT:
      ti->t[i] = 1.0F - c1*(float)fabs(i - c2);
      break;
    case WIN_GAUSSIAN:
      ti->t[i] = 
	(float)exp(c1*(c2+i)*(c2+i));
      break;
    case WIN_KAISER:
      ti->t[i] = (float)(d3*tgen_bessel(p[2]*sqrt(d2 - (i-d1)*(i-d1))));
      break;
    case WIN_BOXCAR:
      ti->t[i] = 1.0F;
      break;
    }
  ti->t[ti->len] = ti->t[0];

}

#undef WIN_HAMMING  
#undef WIN_HANNING  
#undef WIN_BARTLETT 
#undef WIN_GAUSSIAN 
#undef WIN_KAISER   
#undef WIN_BOXCAR   

/* ************************* */
/*  end of table generators  */
/* ************************* */
