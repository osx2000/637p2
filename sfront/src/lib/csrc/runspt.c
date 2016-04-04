
/*
#    Sfront, a SAOL to C translator    
#    This file: Runtime for speedt core opcode
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


/* for speedt core opcode */

void picola(struct tableinfo * in, struct tableinfo * out,
	    float factor)

{
  int i,j, p0, lw, l, p1, over, pmin, pmax, cyc, cidx;
  float minerr, newerr, awin, bwin, scale;
  float window[2048]; 

  if (factor == 1.0F) 
    {
      i = 0;
      while (i <= in->len)
	{
	  out->t[i] = in->t[i];
	  i++;
	}
      out->len = in->len;
      out->lenf = in->lenf;
      out->start = in->start;
      out->end = in->end;
      out->sr = in->sr;
      out->dint = in->dint;
      out->dfrac = in->dfrac;
      out->sffl = in->sffl;
      out->sfui = in->sfui;
      out->dsincr = in->dsincr;
      out->base = in->base;
      out->stamp = in->stamp;
      out->oconst = in->oconst;
      out->tend = in->tend;
      return;
    }

  if (factor <= 0.0F)
    factor = 0.001F;
  cyc = 1;
  while (factor > 2.0F)
    {
      factor--;
      cyc++;
    }
  factor = 1/factor;

  over = p1 = p0 = 0;
  pmin = 5.0e-3F*EV(ARATE);
  pmax = 2.0e-2F*EV(ARATE);
  while (!over)
  {
    i = pmin;
    while ((i<= pmax) && (p0 + 2*i < in->len))
      {
	newerr = 0;
	for (j = p0; j < p0 + i; j++)
	  newerr += (in->t[j]-in->t[j+i])*(in->t[j]-in->t[j+i]);
	newerr /= (float) lw;
	if ((i == pmin) || (newerr < minerr))
	  {
	    minerr = newerr;
	    lw = i;
	  }
	i++;
      }
    if (i == pmin)
      {
	over = 1;
	lw = 0;
      }
    else
      {
	newerr = 0;
	for (i = p0; i < p0 + lw; i++)
	  newerr += in->t[i]*in->t[i+lw];
	if (newerr < 0)
	  lw = pmin - 1;
	awin = 1.0F;
	bwin = 0.0F;
	scale = 1.0F/(lw-1.0F);
	for (i = 0; i < lw; i++)
	  {
	    if (factor > 1)
	      window[i] = awin*in->t[p0+i] + 
		bwin*in->t[p0+i+lw];
	    else
	      window[i] = bwin*in->t[p0+i] + 
		awin*in->t[p0+i+lw];
	    awin -= scale;
	    bwin += scale;
	  }
      }

    if (factor > 1)
      {
	p0 += lw;
	l = ((int)((float)lw/(factor - 1)));
	i = 0;
	while ((l>0) && (i<lw) && !(over))
	  {
	    out->t[p1] = window[i];
	    p0++;
	    p1++;
	    if (p1 == out->len)
	      over = 1;
	    i++;
	    l--;
	  }
	while ((l>0) && !(over))
	  {
	    if (p0 == in->len)
	      out->t[p1] = 0.0F;
	    else
	      {
		out->t[p1] = in->t[p0];
		p0++;
	      }
	    p1++;
	    if (p1 == out->len)
	      over = 1;
	    l--;
	  }
      }
    else
      {
	i = 0;
	while ((i < lw) && !(over))
	  {
	    if (p0 == in->len)
	      out->t[p1] = 0.0F;
	    else
	      {
		out->t[p1] = in->t[p0];
		p0++;
	      }
	    i++;
	    p1++;
	    if (p1 == out->len)
	      over = 1;
	  }

	l = (int)(((float)lw*factor)/(1 - factor));
	cidx = cyc;
	while ((cidx > 0) && (!over))
	  {
	    i = 0;
	    while ((l>0) && (i<lw) && (!over))
	      {
		out->t[p1] = window[i];
		p1++;
		if (p1 == out->len)
		  over = 1;
		i++;
		if (cidx == 1)
		  l--;
	      }
	    cidx--;
	  }
	while ((l>0) && (!over))
	  {	    
	    if (p0 == in->len)
	      out->t[p1] = 0.0F;
	    else
	      {
		out->t[p1] = in->t[p0];
		p0++;
	      }
	    p1++;
	    if (p1 == out->len)
	      over = 1;
	    l--;
	  }
      }
  }

  if (p1 < out->len)
    {
      out->len = p1;
      out->lenf = (float)p1;
      out->oconst = ((float)p1)*EV(ATIME);
    }
  out->t[out->len] = out->t[0]; 
  out->base = in->base;
  out->stamp = in->stamp;
  out->sr = in->sr;
  out->start = in->start/factor;
  if (in->end == 0)
    {
      out->end = 0;
      out->tend = out->len - 1;
    }
  else
    {
      out->tend = out->end = in->end/factor;
    }
  out->dint = in->dint;
  out->dfrac = in->dfrac;
  out->sffl = in->sffl;
  out->sfui = in->sfui;
  out->dsincr = in->dsincr;
  return;
}
