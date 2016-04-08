
/*
#    Sfront, a SAOL to C translator    
#    This file: Variable declarations for core opcodes
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


#include "tree.h"

/******************************************************************/
/*              prints out opcode local variables                 */
/******************************************************************/

void coreparamvars(tnode * tptr)

{

  sigsym * sptr;

  sptr = tptr->sptr->defnode->sptr;
  while (sptr != NULL)
    {
      if (sptr->kind == K_PFIELD)
	{
	  if ((sptr->vartype == TABLETYPE)||(sptr->vartype == TMAPTYPE))
	    fprintf(outfile,"   PSIZE %s;\n",sptr->val);
	  else
	    fprintf(outfile,"   float %s;\n",sptr->val);
	}
      sptr = sptr->next;
    }
  fprintf(outfile,"   float ret;\n");

}

/*********************************************************/
/*      adds variable decls for code opcodes             */
/*********************************************************/

void coreopcodevars(tnode * tptr)

{

  if (!coreopcodename(tptr))
    return;

  coreparamvars(tptr);
  switch (tptr->val[0]) {
  case 'a':
    if (!(strcmp(tptr->val,"abs")))
      return;
    if (!(strcmp(tptr->val,"acos")))
      return;
    if (!(strcmp(tptr->val,"aexpon")))
      return ;
    if (!(strcmp(tptr->val,"aexprand")))
      return ;
    if (!(strcmp(tptr->val,"agaussrand")))
      return ;
    if (!(strcmp(tptr->val,"aline")))
      return ;
    if (!(strcmp(tptr->val,"alinrand")))
      {
	fprintf(outfile,"   float a,b;\n");
	return ;
      }
    if (!(strcmp(tptr->val,"allpass")))
      {
	fprintf(outfile,"   int i;\n");
	return ;
      }
    if (!(strcmp(tptr->val,"ampdb")))
      return;
    if (!(strcmp(tptr->val,"aphasor")))
      {
	fprintf(outfile,"   float index;\n");
	fprintf(outfile,"   unsigned int nint, nfrac, j;\n");
	return ;
      }
    if (!(strcmp(tptr->val,"apoissonrand")))
      return ;
    if (!(strcmp(tptr->val,"arand")))
      return ;
    if (!(strcmp(tptr->val,"asin")))
      return;
    if (!(strcmp(tptr->val,"atan")))
      return;
    return ;
  case 'b':
    if (!(strcmp(tptr->val,"balance")))
      {
	fprintf(outfile,"   int i;\n");
	return ;
      }
    if (!(strcmp(tptr->val,"bandpass")))
      {
	fprintf(outfile,"   float c,d,e;\n");
	return ;
      }
    if (!(strcmp(tptr->val,"bandstop")))
      {
	fprintf(outfile,"   float c,d,e;\n");
	return ;
      }
    if (!(strcmp(tptr->val,"biquad")))
      return ;
    if (!(strcmp(tptr->val,"buzz")))
      {
	fprintf(outfile,"   int tnharm;\n");
	fprintf(outfile,"   double dscale;\n");
	fprintf(outfile,"   float denom, q, n, rad;\n");
	fprintf(outfile,"   float c1, c2, c3, s1, s2, s3;\n");
	return ;
      }
    return ;
  case 'c':
    if (!(strcmp(tptr->val,"ceil")))
      return;
    if (!(strcmp(tptr->val,"chorus")))
      { 
	fprintf(outfile,"   int i,k,len;\n");
	fprintf(outfile,"   float index;\n");
	if (interp == INTERP_SINC)
	  {
	    fprintf(outfile,"   unsigned int fptr, rptr;\n");
	    fprintf(outfile,"   int incr;\n");
	  }
	return ;
      }
    if (!(strcmp(tptr->val,"comb")))
      {
	fprintf(outfile,"   int i;\n");
	return ;
      }
    if (!(strcmp(tptr->val,"compressor")))
      {
	fprintf(outfile,"   int i;\n");
	return ;
      }
    if (!(strcmp(tptr->val,"cos")))
      return;
    if (!(strcmp(tptr->val,"cpsmidi")))
      return;
    if (!(strcmp(tptr->val,"cpsoct")))
      return;
    if (!(strcmp(tptr->val,"cpspch")))
      return;
    return ;
  case 'd':
    if (!(strcmp(tptr->val,"dbamp")))
      return;
    if (!(strcmp(tptr->val,"decimate")))
      return ;
    if (!(strcmp(tptr->val,"delay")))
      {
	fprintf(outfile,"   int i;\n");
	return ;
      }
    if (!(strcmp(tptr->val,"delay1")))
      return ;
    if (!(strcmp(tptr->val,"doscil")))
      {
	if (interp == INTERP_LINEAR)
	  fprintf(outfile,"   unsigned int i, j, k;\n");
	if (interp == INTERP_SINC)
	  fprintf(outfile,"   unsigned int i, j, k, x0, fptr, rptr;\n");
	return ;
      }
    if (!(strcmp(tptr->val,"downsamp")))
      {
	fprintf(outfile,"   int i;\n");
	return ;
      }
    return ;
  case 'e':
    if (!(strcmp(tptr->val,"exp")))
      return;
    return;
  case 'f':
    if (!(strcmp(tptr->val,"fft")))
      {
	fprintf(outfile,"   int i,j,len,ds,dc;\n");
	fprintf(outfile,"   int bsize, bhalf, bincr;\n");
	fprintf(outfile,"   float re1,im1,re2,im2,cv,sv;\n");
	return;
      }
    if (!(strcmp(tptr->val,"fir")))
      return ;
    if (!(strcmp(tptr->val,"firt")))
      { 
	fprintf(outfile,"   int i,j,last;\n");
	return ;
      }
    if (!(strcmp(tptr->val,"flange")))
      { 
	fprintf(outfile,"   int i,k,len;\n");
	fprintf(outfile,"   float index;\n");
	if (interp == INTERP_SINC)
	  {
	    fprintf(outfile,"   unsigned int fptr, rptr;\n");
	    fprintf(outfile,"   int incr;\n");
	  }
	return ;
      }
    if (!(strcmp(tptr->val,"floor")))
      return;
    if (!(strcmp(tptr->val,"frac")))
      return;
    if (!(strcmp(tptr->val,"fracdelay")))
      { 
	fprintf(outfile,"   int i,k,len;\n");
	fprintf(outfile,"   float index;\n");
	if (interp == INTERP_SINC)
	  {
	    fprintf(outfile,"   unsigned int fptr, rptr;\n");
	    fprintf(outfile,"   int incr;\n");
	  }
	return ;
      }
    if (!(strcmp(tptr->val,"ftbasecps")))
      return;
    if (!(strcmp(tptr->val,"ftlen")))
      return;
    if (!(strcmp(tptr->val,"ftloop")))
      return;
    if (!(strcmp(tptr->val,"ftloopend")))
      return;
    if (!(strcmp(tptr->val,"ftsetbase")))
      return;
    if (!(strcmp(tptr->val,"ftsetend")))
      return;
    if (!(strcmp(tptr->val,"ftsetloop")))
      return;
    if (!(strcmp(tptr->val,"ftsetsr")))
      { 
	fprintf(outfile,"   double intdummy;\n");
	return ;
      }
      return;
    if (!(strcmp(tptr->val,"ftsr")))
      return;
    return ;
  case 'g':
    if (!(strcmp(tptr->val,"gain")))
      {
	fprintf(outfile,"   int i;\n");
	fprintf(outfile,"   float root;\n");
	return ;
      }
    if (!(strcmp(tptr->val,"gettempo")))
      return;
    if (!(strcmp(tptr->val,"gettune")))
      return;
    if (!(strcmp(tptr->val,"grain")))
      {
	fprintf(outfile,"   int i,j;\n");
	fprintf(outfile,"   float index,out,sc;\n");
	fprintf(outfile,"   float * state;\n");
	return ;
      }
    return ;
  case 'h':
    if (!(strcmp(tptr->val,"hipass")))
      {
	fprintf(outfile,"   float c,e;\n");
	return ;
      }
    return ;
  case 'i':
    if (!(strcmp(tptr->val,"iexprand")))
      return ;
    if (!(strcmp(tptr->val,"ifft")))
      {
	fprintf(outfile,"   int i,j,len,ds,dc,bsize,bhalf,bincr;\n");
	fprintf(outfile,"   float re2,im2,cv,sv;\n");
	return;
      }
    if (!(strcmp(tptr->val,"igaussrand")))
      return ;
    if (!(strcmp(tptr->val,"iir")))
      return ;
    if (!(strcmp(tptr->val,"iirt")))
      { 
	fprintf(outfile,"   int i,j;\n");
	return ;
      }
    if (!(strcmp(tptr->val,"ilinrand")))
      {
	fprintf(outfile,"   float a,b;\n");
	return ;
      }
    if (!(strcmp(tptr->val,"int")))
      return;
    if (!(strcmp(tptr->val,"irand")))
      return;
    return ;
  case 'j':
    return ;
  case 'k':
    if (!(strcmp(tptr->val,"kexpon")))
      return ;
    if (!(strcmp(tptr->val,"kexprand")))
      return ;
    if (!(strcmp(tptr->val,"kgaussrand")))
      return ;
    if (!(strcmp(tptr->val,"kline")))
      return ;
    if (!(strcmp(tptr->val,"klinrand")))
      {
	fprintf(outfile,"   float a,b;\n");
      }
    if (!(strcmp(tptr->val,"koscil")))
      {
	fprintf(outfile,"   float index;\n");
	fprintf(outfile,"   unsigned int nint, nfrac, i, j;\n");
	if (interp == INTERP_SINC)
	  {
	    fprintf(outfile,"   float sffl;\n");
	    fprintf(outfile,"   unsigned int k, fptr, rptr, sfui, kosincr;\n");
	  }
	return ;
      }
    if (!(strcmp(tptr->val,"kphasor")))
      {
	fprintf(outfile,"   float index;\n");
	fprintf(outfile,"   unsigned int nint, nfrac, j;\n");
	return ;
      }
    if (!(strcmp(tptr->val,"kpoissonrand")))
      return ;
    if (!(strcmp(tptr->val,"krand")))
      return ;
    return ;
  case 'l':
    if (!(strcmp(tptr->val,"log")))
      return;
    if (!(strcmp(tptr->val,"log10")))
      return;
    if (!(strcmp(tptr->val,"lopass")))
      {
	fprintf(outfile,"   float c,e;\n");
	return ;
      }
    if (!(strcmp(tptr->val,"loscil")))
      {
	fprintf(outfile,"   int inloop;\n");
	fprintf(outfile,"   float index;\n");
	fprintf(outfile,"   unsigned int nint, nfrac, i, j, k;\n");
	if (interp == INTERP_SINC)
	  {
	    fprintf(outfile,"   float sffl;\n");
	    fprintf(outfile,"   unsigned int fptr, rptr, sfui, losincr;\n");
	  }
	return ;
      }
    return;
  case 'm':
    if (!(strcmp(tptr->val,"max")))
      return ;
    if (!(strcmp(tptr->val,"midicps")))
      return;
    if (!(strcmp(tptr->val,"midioct")))
      return;
    if (!(strcmp(tptr->val,"midipch")))
      return;
    if (!(strcmp(tptr->val,"min")))
      return;
    return ;
  case 'n':
    return ;
  case 'o':
    if (!(strcmp(tptr->val,"octcps")))
      return ;
    if (!(strcmp(tptr->val,"octmidi")))
      return;
    if (!(strcmp(tptr->val,"octpch")))
      return;
    if (!(strcmp(tptr->val,"oscil")))
      {
	fprintf(outfile,"   float index;\n");
	fprintf(outfile,"   unsigned int nint, nfrac, i, j;\n");
	if (interp == INTERP_SINC)
	  {
	    fprintf(outfile,"   float sffl;\n");
	    fprintf(outfile,"   unsigned int k, fptr, rptr, sfui, osincr;\n");
	  }
	return ;
      }
    return ;
  case 'p':
    if (!(strcmp(tptr->val,"pchcps")))
      return ;
    if (!(strcmp(tptr->val,"pchmidi")))
      return;
    if (!(strcmp(tptr->val,"pchoct")))
      return;
    if (!(strcmp(tptr->val,"pluck")))
      {
	fprintf(outfile,"   unsigned int nint, nfrac, i, j, k;\n");
	fprintf(outfile,"   int len;\n");
	fprintf(outfile,"   float index;\n");
	fprintf(outfile,"   float * h;\n");
	if (interp == INTERP_SINC)
	  {
	    fprintf(outfile,"   float sffl;\n");
	    fprintf(outfile,"   unsigned int m, fptr, rptr, sfui, osincr;\n");
	  }
	return ;
      }
    if (!(strcmp(tptr->val,"port")))
      {
	fprintf(outfile,"   float diff;\n");
	return ;
      }
    if (!(strcmp(tptr->val,"pow")))
      return;
    return ;
  case 'q':
    return ;
  case 'r':
    if (!(strcmp(tptr->val,"reverb")))
      {
	fprintf(outfile,"   int i;\n");
	fprintf(outfile,"   float apout1,apout2,csum,fout,c,e;\n");
	return ;
      }
    if (!(strcmp(tptr->val,"rms")))
      {
	fprintf(outfile,"   int i;\n");
	return ;
      }
    return ;
  case 's':
    if (!(strcmp(tptr->val,"samphold")))
      return ;
    if (!(strcmp(tptr->val,"sblock")))
      {
	fprintf(outfile,"   int i;\n");
	return ;
      }
    if (!(strcmp(tptr->val,"settempo")))
      return;
    if (!(strcmp(tptr->val,"settune")))
      return;
    if (!(strcmp(tptr->val,"sgn")))
      return;
    if (!(strcmp(tptr->val,"sin")))
      return;
    if (!(strcmp(tptr->val,"spatialize")))
      {
	fprintf(outfile,"     int i,len;\n");
	fprintf(outfile,"     float lpc,lpe,in,phi,theta,esum,ein;\n");
	fprintf(outfile,"     float pL,pR,aleft,aright,left,right,room;\n");
	return ;
      }
    if (!(strcmp(tptr->val,"speedt")))
      return;
    if (!(strcmp(tptr->val,"sqrt")))
      return;
    return ;
  case 't':
    if (!(strcmp(tptr->val,"tableread")))
      {
	fprintf(outfile,"   int i,len;\n");
	if (interp == INTERP_SINC)
	  {
	    fprintf(outfile,"   float ksffl;\n");
	    fprintf(outfile,"   unsigned int j, k, fptr, rptr, ksfui, kdsincr;\n");
	    fprintf(outfile,"   int incr;\n");
	  }
	return ;
      }
    if (!(strcmp(tptr->val,"tablewrite")))
      {
	fprintf(outfile,"   int i;\n");
	return ;
      }
    return;
  case 'u':
    if (!(strcmp(tptr->val,"upsamp")))
      {
	fprintf(outfile,"   int i;\n");
	return ;
      }
    return ;
  case 'v':
    return ;
  case 'w':
    return ;
  case 'x':
    return ;
  case 'y':
    return ;
  case 'z':
    return ;
  default:
    return ;
 }
}


/*********************************************************/
/* returns 1 if S_IDENT is a core opcode, else 0         */
/*********************************************************/

int coreopcodename(tnode * ident)


{

 switch (ident->val[0]) {
  case 'a':
    if (!(strcmp(ident->val,"abs")))
      return 1;
    if (!(strcmp(ident->val,"acos")))
      return 1;
    if (!(strcmp(ident->val,"aexpon")))
      return 1;
    if (!(strcmp(ident->val,"aexprand")))
      return 1;
    if (!(strcmp(ident->val,"agaussrand")))
      return 1;
    if (!(strcmp(ident->val,"aline")))
      return 1;
    if (!(strcmp(ident->val,"alinrand")))
      return 1;
    if (!(strcmp(ident->val,"allpass")))
      return 1;
    if (!(strcmp(ident->val,"ampdb")))
      return 1;
    if (!(strcmp(ident->val,"aphasor")))
      return 1;
    if (!(strcmp(ident->val,"apoissonrand")))
      return 1;
    if (!(strcmp(ident->val,"arand")))
      return 1;
    if (!(strcmp(ident->val,"asin")))
      return 1;
    if (!(strcmp(ident->val,"atan")))
      return 1;
    return 0;
  case 'b':
    if (!(strcmp(ident->val,"balance")))
      return 1;
    if (!(strcmp(ident->val,"bandpass")))
      return 1;
    if (!(strcmp(ident->val,"bandstop")))
      return 1;
    if (!(strcmp(ident->val,"biquad")))
      return 1;
    if (!(strcmp(ident->val,"buzz")))
      return 1;
    return 0;
  case 'c':
    if (!(strcmp(ident->val,"ceil")))
      return 1;
    if (!(strcmp(ident->val,"chorus")))
      return 1;
    if (!(strcmp(ident->val,"comb")))
      return 1;
    if (!(strcmp(ident->val,"compressor")))
      return 1;
    if (!(strcmp(ident->val,"cos")))
      return 1;
    if (!(strcmp(ident->val,"cpsmidi")))
      return 1;
    if (!(strcmp(ident->val,"cpsoct")))
      return 1;
    if (!(strcmp(ident->val,"cpspch")))
      return 1;
    return 0;
  case 'd':
    if (!(strcmp(ident->val,"dbamp")))
      return 1;
    if (!(strcmp(ident->val,"decimate")))
      return 1;
    if (!(strcmp(ident->val,"delay")))
      return 1;
    if (!(strcmp(ident->val,"delay1")))
      return 1;
    if (!(strcmp(ident->val,"doscil")))
      return 1;
    if (!(strcmp(ident->val,"downsamp")))
      return 1;
    return 0;
  case 'e':
    if (!(strcmp(ident->val,"exp")))
      return 1;
    return 0;
  case 'f':
    if (!(strcmp(ident->val,"fft")))
      return 1;
    if (!(strcmp(ident->val,"fir")))
      return 1;
    if (!(strcmp(ident->val,"firt")))
      return 1;
    if (!(strcmp(ident->val,"flange")))
      return 1;
    if (!(strcmp(ident->val,"floor")))
      return 1;
    if (!(strcmp(ident->val,"frac")))
      return 1;
    if (!(strcmp(ident->val,"fracdelay")))
      return 1;
    if (!(strcmp(ident->val,"ftbasecps")))
      return 1;
    if (!(strcmp(ident->val,"ftlen")))
      return 1;
    if (!(strcmp(ident->val,"ftloop")))
      return 1;
    if (!(strcmp(ident->val,"ftloopend")))
      return 1;
    if (!(strcmp(ident->val,"ftsetbase")))
      return 1;
    if (!(strcmp(ident->val,"ftsetend")))
      return 1;
    if (!(strcmp(ident->val,"ftsetloop")))
      return 1;
    if (!(strcmp(ident->val,"ftsetsr")))
      return 1;
    if (!(strcmp(ident->val,"ftsr")))
      return 1;
    return 0;
  case 'g':
    if (!(strcmp(ident->val,"gain")))
      return 1;
    if (!(strcmp(ident->val,"gettempo")))
      return 1;
    if (!(strcmp(ident->val,"gettune")))
      return 1;
    if (!(strcmp(ident->val,"grain")))
      return 1;
    return 0;
  case 'h':
    if (!(strcmp(ident->val,"hipass")))
      return 1;
    return 0;
  case 'i':
    if (!(strcmp(ident->val,"iexprand")))
      return 1;
    if (!(strcmp(ident->val,"ifft")))
      return 1;
    if (!(strcmp(ident->val,"igaussrand")))
      return 1;
    if (!(strcmp(ident->val,"iir")))
      return 1;
    if (!(strcmp(ident->val,"iirt")))
      return 1;
    if (!(strcmp(ident->val,"ilinrand")))
      return 1;
    if (!(strcmp(ident->val,"int")))
      return 1;
    if (!(strcmp(ident->val,"irand")))
      return 1;
    return 0;
  case 'j':
    return 0;
  case 'k':
    if (!(strcmp(ident->val,"kexpon")))
      return 1;
    if (!(strcmp(ident->val,"kexprand")))
      return 1;
    if (!(strcmp(ident->val,"kgaussrand")))
      return 1;
    if (!(strcmp(ident->val,"kline")))
      return 1;
    if (!(strcmp(ident->val,"klinrand")))
      return 1;
    if (!(strcmp(ident->val,"koscil")))
      return 1;
    if (!(strcmp(ident->val,"kphasor")))
      return 1;
    if (!(strcmp(ident->val,"kpoissonrand")))
      return 1;
    if (!(strcmp(ident->val,"krand")))
      return 1;
    return 0;
  case 'l':
    if (!(strcmp(ident->val,"log")))
      return 1;
    if (!(strcmp(ident->val,"log10")))
      return 1;
    if (!(strcmp(ident->val,"lopass")))
      return 1;
    if (!(strcmp(ident->val,"loscil")))
      return 1;
    return 0;
  case 'm':
    if (!(strcmp(ident->val,"max")))
      return 1;
    if (!(strcmp(ident->val,"midicps")))
      return 1;
    if (!(strcmp(ident->val,"midioct")))
      return 1;
    if (!(strcmp(ident->val,"midipch")))
      return 1;
    if (!(strcmp(ident->val,"min")))
      return 1;
    return 0;
  case 'n':
    return 0;
  case 'o':
    if (!(strcmp(ident->val,"octcps")))
      return 1;
    if (!(strcmp(ident->val,"octmidi")))
      return 1;
    if (!(strcmp(ident->val,"octpch")))
      return 1;
    if (!(strcmp(ident->val,"oscil")))
      return 1;
    return 0;
  case 'p':
    if (!(strcmp(ident->val,"pchcps")))
      return 1;
    if (!(strcmp(ident->val,"pchmidi")))
      return 1;
    if (!(strcmp(ident->val,"pchoct")))
      return 1;
    if (!(strcmp(ident->val,"pluck")))
      return 1;
    if (!(strcmp(ident->val,"port")))
      return 1;
    if (!(strcmp(ident->val,"pow")))
      return 1;
    return 0;
  case 'q':
    return 0;
  case 'r':
    if (!(strcmp(ident->val,"reverb")))
      return 1;
    if (!(strcmp(ident->val,"rms")))
      return 1;
    return 0;
  case 's':
    if (!(strcmp(ident->val,"samphold")))
      return 1;
    if (!(strcmp(ident->val,"sblock")))
      return 1;
    if (!(strcmp(ident->val,"settempo")))
      return 1;
    if (!(strcmp(ident->val,"settune")))
      return 1;
    if (!(strcmp(ident->val,"sgn")))
      return 1;
    if (!(strcmp(ident->val,"sin")))
      return 1;
    if (!(strcmp(ident->val,"spatialize")))
      return 1;
    if (!(strcmp(ident->val,"speedt")))
      return 1;
    if (!(strcmp(ident->val,"sqrt")))
      return 1;
    return 0;
  case 't':
    if (!(strcmp(ident->val,"tableread")))
      return 1;
    if (!(strcmp(ident->val,"tablewrite")))
      return 1;
    return 0;
  case 'u':
    if (!(strcmp(ident->val,"upsamp")))
      return 1;
    return 0;
  case 'v':
    return 0;
  case 'w':
    return 0;
  case 'x':
    return 0;
  case 'y':
    return 0;
  case 'z':
    return 0;
 }
 return 0; /* will never execute */
}


/*********************************************************/
/*  returns 1 if opcode has optional formal paramaters    */
/*********************************************************/

int coreopcodehasextras(tnode * ident)

{

 switch (ident->val[0]) {
  case 'a':
    if (!(strcmp(ident->val,"aexpon")))  
      return 1;
    if (!(strcmp(ident->val,"aline")))  
      return 1;
    return 0;
  case 'b':
    if (!(strcmp(ident->val,"balance")))  
      return 1;
    return 0;
  case 'c':
    return 0;
  case 'd':
    if (!(strcmp(ident->val,"downsamp")))  
      return 1;
    return 0;
  case 'e':
    return 0;
  case 'f':
    if (!(strcmp(ident->val,"fft")))  
      return 1;
    if (!(strcmp(ident->val,"fir")))   
      return 1;
    if (!(strcmp(ident->val,"firt")))  
      return 1;
    if (!(strcmp(ident->val,"fracdelay"))) 
      return 1;
    return 0;
  case 'g':
    if (!(strcmp(ident->val,"gain")))  
      return 1;
    if (!(strcmp(ident->val,"gettempo")))  
      return 1;
    if (!(strcmp(ident->val,"gettune")))  
      return 1;
    return 0;
  case 'h':
    return 0;
  case 'i':
    if (!(strcmp(ident->val,"ifft")))   
      return 1;
    if (!(strcmp(ident->val,"iir")))   
      return 1;
    if (!(strcmp(ident->val,"iirt")))  
      return 1;
    return 0;
  case 'j':
    return 0;
  case 'k':
    if (!(strcmp(ident->val,"kexpon")))  
      return 1;
    if (!(strcmp(ident->val,"kline")))  
      return 1;
    if (!(strcmp(ident->val,"koscil")))  
      return 1;
    return 0;
  case 'l':
    if (!(strcmp(ident->val,"loscil")))  
      return 1;
    return 0;
  case 'm':
    if (!(strcmp(ident->val,"max")))  
      return 1;
    if (!(strcmp(ident->val,"min")))  
      return 1;
    return 0;
  case 'n':
    return 0;
  case 'o':
    if (!(strcmp(ident->val,"oscil")))  
      return 1;
    return 0;
  case 'p':
    return 0;
  case 'q':
    return 0;
  case 'r':
    if (!(strcmp(ident->val,"reverb")))  
      return 1;
    if (!(strcmp(ident->val,"rms")))  
      return 1;
    return 0;
  case 's':
    return 0;
  case 't':
    return 0;
  case 'u':
    if (!(strcmp(ident->val,"upsamp"))) 
      return 1;
    return 0;
  case 'v':
    return 0;
  case 'w':
    return 0;
  case 'x':
    return 0;
  case 'y':
    return 0;
  case 'z':
    return 0;
 }
 return 0; /* will never execute */
}


/*********************************************************/
/* returns 1 if opcode is:                               */
/*                                                       */
/* [1] A krate, irate, or polymorphic opcode, that       */
/* [2] Has internal state updates that make rate         */
/*     optimization dangerous in some situations.        */
/*                                                       */
/* For now specialops not included, but remain as        */
/* commented out checks to change this.                  */
/*********************************************************/

int coreopcodespeedtrap(tnode * ident)


{

 switch (ident->val[0]) {
  case 'a':
    return 0;
  case 'b':
    return 0;
  case 'c':
    return 0;
  case 'd':
    /* 
     *if (!(strcmp(ident->val,"decimate")))
     * return 1;
     *if (!(strcmp(ident->val,"downsamp")))
     * return 1;
     */
    return 0;
  case 'e':
    return 0;
  case 'f':
    /* 
     * if (!(strcmp(ident->val,"fft")))
     *  return 1;
     */
    if (!(strcmp(ident->val,"ftsetbase")))
      return 1;
    if (!(strcmp(ident->val,"ftsetend")))
      return 1;
    if (!(strcmp(ident->val,"ftsetloop")))
      return 1;
    if (!(strcmp(ident->val,"ftsetsr")))
      return 1;
    return 0;
  case 'g':
    return 0;
  case 'h':
    return 0;
  case 'i':
    return 0;
  case 'j':
    return 0;
  case 'k':
    if (!(strcmp(ident->val,"kexpon")))
      return 1;
    if (!(strcmp(ident->val,"kline")))
      return 1;
    if (!(strcmp(ident->val,"koscil")))
      return 1;
    if (!(strcmp(ident->val,"kphasor")))
      return 1;
    return 0;
  case 'l':
    return 0;
  case 'm':
    return 0;
  case 'n':
    return 0;
  case 'o':
    return 0;
  case 'p':
    if (!(strcmp(ident->val,"port")))
      return 1;
    return 0;
  case 'q':
    return 0;
  case 'r':
    /*
     * if (!(strcmp(ident->val,"rms")))
     *  return 1;
     */
    return 0;
  case 's':
    if (!(strcmp(ident->val,"samphold")))
      return 1;
    /*
     * if (!(strcmp(ident->val,"sblock")))
     *  return 1;
     */
    if (!(strcmp(ident->val,"settempo")))
      return 1;
    if (!(strcmp(ident->val,"speedt")))
      return 1;
    return 0;
  case 't':
    if (!(strcmp(ident->val,"tablewrite")))
      return 1;
    return 0;
  case 'u':
    /*
     * if (!(strcmp(ident->val,"upsamp")))
     *  return 1;
     */
    return 0;
  case 'v':
    return 0;
  case 'w':
    return 0;
  case 'x':
    return 0;
  case 'y':
    return 0;
  case 'z':
    return 0;
 }
 return 0; /* will never execute */
}


/***********************************************************************/
/*   table core opcodes that should obey strict guard/enclosure rules  */
/***********************************************************************/

int delicatepolyops(tnode * tptr)

{
  if ((tptr->val[0] != 'f') && (tptr->val[0] != 't') && (tptr->val[0] != 'g'))
    return 0;
  
  if ((!strcmp(tptr->val,"ftlen")) ||        /* may be UNKNOWN */
      (!strcmp(tptr->val,"ftloop")) ||
      (!strcmp(tptr->val,"ftloopend")) ||
      (!strcmp(tptr->val,"ftsr")) ||
      (!strcmp(tptr->val,"gettempo")) ||
      (!strcmp(tptr->val,"gettune")) ||
      (!strcmp(tptr->val,"ftbasecps")) ||
      (!strcmp(tptr->val,"tableread")) ||    /* side-effects paranoia;   */
      (!strcmp(tptr->val,"tablewrite")))     /* optrate.c may handle it. */ 
    return 1;

  return 0;
}

/***********************************************************************/
/*    flags poly opcode exceptions to slower-rate opcall semantics     */
/***********************************************************************/

int polyopcallexcept(tnode * ident)

{    

 switch (ident->val[0]) {
  case 'a':
    if (!(strcmp(ident->val,"abs")))
      return 1;
    break;
  case 'c':
    if (!(strcmp(ident->val,"ceil")))
      return 1;
    break;
  case 'f':
    if (!(strcmp(ident->val,"floor")))
      return 1;
    if (!(strcmp(ident->val,"ftbasecps")))
      return 1;
    if (!(strcmp(ident->val,"ftlen")))
      return 1;
    if (!(strcmp(ident->val,"ftloop")))
      return 1;
    if (!(strcmp(ident->val,"ftloopend")))
      return 1;
    if (!(strcmp(ident->val,"ftsr")))
      return 1;
    break;
  case 'g':
    if (!(strcmp(ident->val,"gettempo")))
      return 1;
    if (!(strcmp(ident->val,"gettune")))
      return 1;
    break;
  case 'i':
    if (!(strcmp(ident->val,"int")))
      return 1;
    break;
  case 's':
    if (!(strcmp(ident->val,"sgn")))
      return 1;
    break;
  case 't':
    /* non-compliant, revisit later */
    if (!(strcmp(ident->val,"tableread")))
      return 1;
    if (!(strcmp(ident->val,"tablewrite")))
      return 1;
    break;
 }

 return 0; 

}


/*********************************************************/
/* returns 1 if constant-input opcode can be precomputed */
/*********************************************************/

int coreopcodeprecompute(tnode * ident)

{

 switch (ident->val[0]) {
  case 'a':
    if (!(strcmp(ident->val,"abs")))
      return 1;
    if (!(strcmp(ident->val,"acos")))
      return 1;
    if (!(strcmp(ident->val,"ampdb")))
      return 1;
    if (!(strcmp(ident->val,"asin")))
      return 1;
    if (!(strcmp(ident->val,"atan")))
      return 1;
    return 0;
  case 'b':
    return 0;
  case 'c':
    if (!(strcmp(ident->val,"ceil")))
      return 1;
    if (!(strcmp(ident->val,"cos")))
      return 1;
    if ((!(strcmp(ident->val,"cpsmidi")))||
	(!(strcmp(ident->val,"cpsoct"))) ||
	(!(strcmp(ident->val,"cpspch"))))
      {
	if (has.o_settune)
	  return 0;
	return 1;
      }
    return 0;
  case 'd':
    if (!(strcmp(ident->val,"dbamp")))
      return 1;
    return 0;
  case 'e':
    if (!(strcmp(ident->val,"exp")))
      return 1;
    return 0;
  case 'f':
    if (!(strcmp(ident->val,"floor")))
      return 1;
    if (!(strcmp(ident->val,"frac")))
      return 1;
    if (!(strcmp(ident->val,"ftbasecps"))) /* do later */
      return 0;
    if (!(strcmp(ident->val,"ftlen")))     /* do later */
      return 0;
    if (!(strcmp(ident->val,"ftloop")))    /* do later */
      return 0;
    if (!(strcmp(ident->val,"ftloopend"))) /* do later */
      return 0;
    if (!(strcmp(ident->val,"ftsetbase"))) /* do later */
      return 0;
    if (!(strcmp(ident->val,"ftsetend")))  /* do later */
      return 0;
    if (!(strcmp(ident->val,"ftsetloop"))) /* do later */
      return 0;
    if (!(strcmp(ident->val,"ftsetsr")))   /* do later */
      return 0;
    if (!(strcmp(ident->val,"ftsr")))      /* do later */
      return 0;
    return 0;
  case 'g':
    if (!(strcmp(ident->val,"gettune")))
      {
	if (has.o_settune)
	  return 0;
	return 1;
      }
    return 0;
  case 'h':
    return 0;
  case 'i':
    if (!(strcmp(ident->val,"int")))
      return 1;
    return 0;
  case 'j':
    return 0;
  case 'k':
    return 0;
  case 'l':
    if (!(strcmp(ident->val,"log")))
      return 1;
    if (!(strcmp(ident->val,"log10")))
      return 1;
    return 0;
  case 'm':
    if (!(strcmp(ident->val,"max")))
      return 1;
    if (!(strcmp(ident->val,"midicps")))
      {
	if (has.o_settune)
	  return 0;
	return 1;
      }
    if (!(strcmp(ident->val,"midioct")))
      return 1;
    if (!(strcmp(ident->val,"midipch")))
      return 1;
    if (!(strcmp(ident->val,"min")))
      return 1;
    return 0;
  case 'n':
    return 0;
  case 'o':
    if (!(strcmp(ident->val,"octcps")))
      {
	if (has.o_settune)
	  return 0;
	return 1;
      }
    if (!(strcmp(ident->val,"octmidi")))
      return 1;
    if (!(strcmp(ident->val,"octpch")))
      return 1;
    return 0;
  case 'p':
    if (!(strcmp(ident->val,"pchcps")))
      {
	if (has.o_settune)
	  return 0;
	return 1;
      }
    if (!(strcmp(ident->val,"pchmidi")))
      return 1;
    if (!(strcmp(ident->val,"pchoct")))
      return 1;
    if (!(strcmp(ident->val,"pow")))
      return 1;
    return 0;
  case 'q':
    return 0;
  case 'r':
    return 0;
  case 's':
    if (!(strcmp(ident->val,"settempo"))) /* do later */
      return 0;
    if (!(strcmp(ident->val,"settune")))  /* do later */
      return 0;
    if (!(strcmp(ident->val,"sgn")))
      return 1;
    if (!(strcmp(ident->val,"sin")))
      return 1;
    if (!(strcmp(ident->val,"speedt"))) /* do later */
      return 0;
    if (!(strcmp(ident->val,"sqrt")))
      return 1;
    return 0;
  case 't':
    if (!(strcmp(ident->val,"tableread")))  /* do later */
      return 0;
    if (!(strcmp(ident->val,"tablewrite")))
      return 0;
    return 0;
  case 'u':
    return 0;
  case 'v':
    return 0;
  case 'w':
    return 0;
  case 'x':
    return 0;
  case 'y':
    return 0;
  case 'z':
    return 0;
 }
 return 0; /* will never execute */
}

/*********************************************************/
/*        changes fields for typical math core opcode    */
/*********************************************************/

void mathfloatop(tnode * ident, char * name)

{
  ident->val = dupval(name);
  ident->rate = IRATETYPE;
  ident->ttype = S_NUMBER;
  ident->res = ASFLOAT;
}

/*********************************************************/
/*        changes fields for typical math core opcode    */
/*********************************************************/

void mathintop(tnode * ident, char * name)

{
  ident->val = dupval(name);
  ident->rate = IRATETYPE;
  ident->ttype = S_INTGR;
  ident->res = ASINT;
}

/*********************************************************/
/*        changes fields for max and min                 */
/*********************************************************/

void mathmaxmin(tnode * ident, tnode * tptr)

{  
  float fval, rval;
  tnode * tm;
  int ismax = strcmp(ident->val,"min");

  tm = tptr;
  fval = (float)atof(tptr->down->val);
  tptr = tptr->next;
  while (tptr != NULL)
    {
      if (tptr->ttype == S_EXPR)
	{
	  rval = (float)atof(tptr->down->val);
	  if ((ismax && (rval > fval)) || (!ismax && (rval < fval)))
	    {
	      fval = rval;
	      tm = tptr;
	    }
	}
      tptr = tptr->next;
    }

  ident->val = dupval(tm->down->val);
  ident->rate = IRATETYPE;
  ident->ttype = tm->down->ttype;
  ident->res = tm->down->res;
  return;

}

/*********************************************************/
/*        collapses core opcode into constant            */
/*********************************************************/

void coreopcodecollapse(tnode * ident, tnode * tptr)

{
  char name[128];
  float fval, rval;
  int ival;
  int i;

  switch (ident->val[0]) {
  case 'a':
    if (!(strcmp(ident->val,"abs")))
      {
	if (tptr->down->val[0] == '-')
	  strcpy(name, &(tptr->down->val[1]));
	else
	  strcpy(name, tptr->down->val);
	ident->val = dupval(name);
	ident->rate = IRATETYPE;
	ident->ttype = tptr->down->ttype;
	ident->res = tptr->down->res;
	return;
      }
    if (!(strcmp(ident->val,"acos")))
      {
	sprintf(name,"%e", (float)(acos(atof(tptr->down->val))));
	mathfloatop(ident, name);
	return;
      }
    if (!(strcmp(ident->val,"ampdb")))
      {
	sprintf(name,"%e", (float)(pow(10.0F,
				       5.0e-2F*(atof(tptr->down->val)-90.0F))));
	mathfloatop(ident, name);
	return;
      }
    if (!(strcmp(ident->val,"asin")))
      {
	sprintf(name,"%e", (float)(asin(atof(tptr->down->val))));
	mathfloatop(ident, name);
	return;
      }
    if (!(strcmp(ident->val,"atan")))
      {
	sprintf(name,"%e", (float)(atan(atof(tptr->down->val))));
	mathfloatop(ident, name);
	return;
      }
    return;
  case 'b':
    return;
  case 'c':
    if (!(strcmp(ident->val,"ceil")))
      {
	sprintf(name,"%i", (int)(ceil(atof(tptr->down->val))+ 0.5F));
	mathintop(ident, name);
	return;
      }
    if (!(strcmp(ident->val,"cos")))
      {
	sprintf(name,"%e", (float)(cos(atof(tptr->down->val))));
	mathfloatop(ident, name);
	return;
      }
    if (!(strcmp(ident->val,"cpsmidi")))
      {
	fval = (float)atof(tptr->down->val);
	if (fval <= 0.0F)
	  {
	    printf("Error: x <= 0 argument to cpsmidi()\n");
	    showerrorplace(ident->linenum, ident->filename);
	  }
	sprintf(name,"%e", globaltune*(float)pow(2.0F, 8.333334e-02F*(fval-69.0F)));
	mathfloatop(ident, name);
	return;
      }
    if (!(strcmp(ident->val,"cpsoct")))
      {
	fval = (float)atof(tptr->down->val);
	if (fval <= 0.0F)
	  {
	    printf("Error: x <= 0 argument to cpsoct()\n");
	    showerrorplace(ident->linenum, ident->filename);
	  }
	sprintf(name,"%e", globaltune*(float)pow(2.0F, (fval-8.75F)));
	mathfloatop(ident, name);
	return;
      }
    if (!(strcmp(ident->val,"cpspch")))
      {	
	fval = (float)atof(tptr->down->val);
	if (fval <= 0.0F)
	  {
	    printf("Error: x <= 0 argument to cpspch()\n");
	    showerrorplace(ident->linenum, ident->filename);
	  }
	ival = (int)(0.5F + 100.0F*(fval - (int)fval));
	rval = (ival > 11.0F) ? 0.0F : 8.333334e-2F*ival;
	sprintf(name,"%e", globaltune*(float)pow(2.0F,(int)(fval)+rval-8.75F));
	mathfloatop(ident, name);
	return;
      }
    return;
  case 'd':
    if (!(strcmp(ident->val,"dbamp")))
      {
	fval = (float)atof(tptr->down->val);
	if (fval <= 0.0F)
	  {
	    printf("Error: x <= 0 argument to dbamp()\n");
	    showerrorplace(ident->linenum, ident->filename);
	  }
	sprintf(name,"%e", (float)(90.0F+20.0F*(float)log10(fval+1e-10F)));
	mathfloatop(ident, name);
	return;
      }
    return;
  case 'e':
    if (!(strcmp(ident->val,"exp")))
      {
	sprintf(name,"%e", (float)(exp(atof(tptr->down->val))));
	mathfloatop(ident, name);
	return;
      }
    return;
  case 'f':
    if (!(strcmp(ident->val,"floor")))
      {
	sprintf(name,"%i", (int)(floor(atof(tptr->down->val))));
	mathintop(ident, name);
	return;
      }
    if (!(strcmp(ident->val,"frac")))
      {
	fval = (float)atof(tptr->down->val);
	sprintf(name,"%e", fval - (int)fval);
	mathfloatop(ident, name);
	return;
      }
    if (!(strcmp(ident->val,"ftbasecps"))) /* do these later */
      return;
    if (!(strcmp(ident->val,"ftlen")))
      return;
    if (!(strcmp(ident->val,"ftloop")))
      return;
    if (!(strcmp(ident->val,"ftloopend")))
      return;
    if (!(strcmp(ident->val,"ftsetbase")))
      return;
    if (!(strcmp(ident->val,"ftsetend")))
      return;
    if (!(strcmp(ident->val,"ftsetloop")))
      return;
    if (!(strcmp(ident->val,"ftsetsr")))
      return;
    if (!(strcmp(ident->val,"ftsr")))
      return;
    return;
  case 'g':
    if (!(strcmp(ident->val,"gettune")))
      {
	sprintf(name,"%e", globaltune);
	mathfloatop(ident, name);
	return;
      }
    return;
  case 'h':
    return;
  case 'i':
    if (!(strcmp(ident->val,"int")))
      {
	sprintf(name,"%i", (int)(atof(tptr->down->val)));
	mathintop(ident, name);
	return;
      }
    return;
  case 'j':
    return;
  case 'k':
    return;
  case 'l':
    if (!(strcmp(ident->val,"log")))
      {
  	fval = (float)atof(tptr->down->val);
	if (fval <= 0.0F)
	  {
	    printf("Error: x <= 0 argument to log()\n");
	    showerrorplace(ident->linenum, ident->filename);
	  }
	sprintf(name,"%e", log(fval));
	mathfloatop(ident, name);
	return;
      }
    if (!(strcmp(ident->val,"log10")))
      {
  	fval = (float)atof(tptr->down->val);
	if (fval <= 0.0F)
	  {
	    printf("Error: x <= 0 argument to log10()\n");
	    showerrorplace(ident->linenum, ident->filename);
	  }
	sprintf(name,"%e", log10(fval));
	mathfloatop(ident, name);
	return;
      }
    return;
  case 'm':
    if (!(strcmp(ident->val,"max")))
      {
	mathmaxmin(ident, tptr);
	return;
      }
    if (!(strcmp(ident->val,"midicps")))
      {
	fval = (float)atof(tptr->down->val);
	if (fval <= 0.0F)
	  {
	    printf("Error: x <= 0 argument to midicps()\n");
	    showerrorplace(ident->linenum, ident->filename);
	  }
	sprintf(name,"%i", (int)(69.5F + 1.731234e+01F*log(fval/globaltune)));
	mathintop(ident, name);
	return;

      }
    if (!(strcmp(ident->val,"midioct")))
      {
	fval = (float)atof(tptr->down->val);
	if (fval <= 3.0F)
	  {
	    printf("Error: x <= 3.0 argument to midioct()\n");
	    showerrorplace(ident->linenum, ident->filename);
	  }
	sprintf(name,"%i", (int)(12.0F*(fval - 3.0F) + 0.5F));
	mathintop(ident, name);
	return;

      }
    if (!(strcmp(ident->val,"midipch")))
      {
	fval = (float)atof(tptr->down->val);
	if (fval <= 3.0F)
	  {
	    printf("Error: x <= 3.0 argument to midipch()\n");
	    showerrorplace(ident->linenum, ident->filename);
	  }
	ival = (int)(0.5 + 100.0F*(fval - (int)fval));
	rval = (ival > 11.0F) ? 0.0F : ival;
	sprintf(name,"%i", (int)(rval + 12.0F*(- 3 + (int) fval)));
	mathintop(ident, name);
	return;
      }
    if (!(strcmp(ident->val,"min")))
     {
       mathmaxmin(ident, tptr);
       return;
      }
    return;
  case 'n':
    return;
  case 'o':
    if (!(strcmp(ident->val,"octcps")))
      {
	fval = (float)atof(tptr->down->val);
	if (fval <= 0.0F)
	  {
	    printf("Error: x <= 0.0 argument to octcps()\n");
	    showerrorplace(ident->linenum, ident->filename);
	  }
	sprintf(name,"%e", 8.75F + 1.442695F*(float)log(fval/globaltune));
	mathfloatop(ident, name);
	return;
      }
    if (!(strcmp(ident->val,"octmidi")))
      {
	fval = (float)atof(tptr->down->val);
	if (fval <= 0.0F)
	  {
	    printf("Error: x <= 0.0 argument to octmidi()\n");
	    showerrorplace(ident->linenum, ident->filename);
	  }
	sprintf(name,"%e", 8.333334e-2F*(fval+36.0F));
	mathfloatop(ident, name);
	return;
      }
    if (!(strcmp(ident->val,"octpch")))
      {
	fval = (float)atof(tptr->down->val);
	if (fval <= 0.0F)
	  {
	    printf("Error: x <= 0.0 argument to octpch()\n");
	    showerrorplace(ident->linenum, ident->filename);
	  }
	ival = (int)(0.5 + 100.0F*(fval - (int)fval));
	rval = (ival > 11.0F) ? 0.0F : ival;
	sprintf(name,"%e", 8.333334e-2F*rval + (int) fval);
	mathfloatop(ident, name);
	return;
      }
    return;
  case 'p':
    if (!(strcmp(ident->val,"pchcps")))
      {
	fval = (float)atof(tptr->down->val);
	if (fval <= 0.0F)
	  {
	    printf("Error: x <= 0.0 argument to pchcps()\n");
	    showerrorplace(ident->linenum, ident->filename);
	  }
	rval = 8.75F +  1.442695F*(float)log(fval/globaltune);
	ival = (int)(0.5 + 12.0F*(rval - (int) rval));
	sprintf(name,"%e", 1.0e-2F*ival + (int) rval);
	mathfloatop(ident, name);
	return;
      }
    if (!(strcmp(ident->val,"pchmidi")))
      {
	fval = (float)atof(tptr->down->val);
	if (fval <= 0.0F)
	  {
	    printf("Error: x <= 0.0 argument to pchmidi()\n");
	    showerrorplace(ident->linenum, ident->filename);
	  }
	rval = 8.333334e-02F*((int)(fval + 0.5)+36.0F);
	sprintf(name,"%e", 12.0e-2F*(rval-(int)rval) + (int)rval);
	mathfloatop(ident, name);
	return;
      }
    if (!(strcmp(ident->val,"pchoct")))
      {
	fval = (float)atof(tptr->down->val);
	if (fval <= 0.0F)
	  {
	    printf("Error: x <= 0.0 argument to pchoct()\n");
	    showerrorplace(ident->linenum, ident->filename);
	  }
	ival = (int)(12.0F*(fval- (int) fval) + 0.5F);
	sprintf(name,"%e", 1.0e-2F*ival + (int) fval);
	mathfloatop(ident, name);
	return;
      }
    if (!(strcmp(ident->val,"pow")))
      {
	fval = (float)atof(tptr->down->val);
	tptr = tptr->next;
	if (tptr->ttype != S_EXPR)
	  tptr = tptr->next;
	rval = (float)pow(fval, atof(tptr->down->val));
	sprintf(name,"%e", rval);
	for (i = 0; i < strlen(name); i++)             /* Inf, Nan */
	  if ((name[i] == 'I') || (name[i] == 'N') || 
	      (name[i] == 'i') || (name[i] == 'n')) 
	    {
	      printf("Error: Inapproproate arguments to pow()\n");
	      showerrorplace(ident->linenum, ident->filename);
	    }
	mathfloatop(ident, name);
	return;
      }
    return;
  case 'q':
    return;
  case 'r':
    return;
  case 's':
    if (!(strcmp(ident->val,"settempo"))) /* do later */
      return;
    if (!(strcmp(ident->val,"settune"))) /* do later */
      return;
    if (!(strcmp(ident->val,"sgn")))
      {
	fval = (float)atof(tptr->down->val);
	if (fval == 0.0)
	  sprintf(name,"0");
	else
	  {
	    if (fval > 0)
	      sprintf(name,"1");
	    else
	      sprintf(name,"-1");
	  }
	mathintop(ident, name);
	return;
      }
    if (!(strcmp(ident->val,"sin")))
      {
	sprintf(name,"%e", (float)(sin(atof(tptr->down->val))));
	mathfloatop(ident, name);
	return;
      }
    if (!(strcmp(ident->val,"speedt"))) /* do later */
      return;
    if (!(strcmp(ident->val,"sqrt")))
      {
  	fval = (float)atof(tptr->down->val);
	if (fval < 0.0F)
	  {
	    printf("Error: x < 0 argument to sqrt()\n");
	    showerrorplace(ident->linenum, ident->filename);
	  }
	sprintf(name,"%e", sqrt(fval));
	mathfloatop(ident, name);
	return;
      }
    return;
  case 't':
    if (!(strcmp(ident->val,"tableread")))  /* do later */
      return;
    if (!(strcmp(ident->val,"tablewrite"))) /* do later */
      return;
    return;
  case 'u':
    return;
  case 'v':
    return;
  case 'w':
    return;
  case 'x':
    return;
  case 'y':
    return;
  case 'z':
    return;
  default:
    return;
 }
}

/*********************************************************/
/*       reference counts for each core opcode           */
/*********************************************************/

void hascoreopcode(tnode * ident, int incr)

{
  
  switch (ident->val[0]) {
  case 'a':
    if (!(strcmp(ident->val,"abs")))
      {
	has.o_abs+= incr;
	return;
      }
    if (!(strcmp(ident->val,"acos")))
      {
	has.o_acos+= incr;
	return;
      }      
    if (!(strcmp(ident->val,"aexpon")))
      {
	has.o_aexpon+= incr;
	return;
      }      
    if (!(strcmp(ident->val,"aexprand")))
      {
	has.o_aexprand+= incr;
	return;
      }      
    if (!(strcmp(ident->val,"agaussrand")))
      {
	has.o_agaussrand+= incr;
	return;
      }      
    if (!(strcmp(ident->val,"aline")))
      {
	has.o_aline+= incr;
	return;
      }      
    if (!(strcmp(ident->val,"alinrand")))
      {
	has.o_alinrand+= incr;
	return;
      }      
    if (!(strcmp(ident->val,"allpass")))
      {
	has.o_allpass+= incr;
	return;
      }      
    if (!(strcmp(ident->val,"ampdb")))
      {
	has.o_ampdb+= incr;
	return;
      }      
    if (!(strcmp(ident->val,"aphasor")))
      {
	has.o_aphasor+= incr;
	return;
      }      
    if (!(strcmp(ident->val,"apoissonrand")))
      {
	has.o_apoissonrand+= incr;
	return;
      }      
    if (!(strcmp(ident->val,"arand")))
      {
	has.o_arand+= incr;
	return;
      }      
    if (!(strcmp(ident->val,"asin")))
      {
	has.o_asin+= incr;
	return;
      }      
    if (!(strcmp(ident->val,"atan")))
      {
	has.o_atan+= incr;
	return;
      }      
    return;
  case 'b':
    if (!(strcmp(ident->val,"balance")))
      {
	has.o_balance+= incr;
	return;
      }      
    if (!(strcmp(ident->val,"bandpass")))
      {
	has.o_bandpass+= incr;
	return;
      }      
    if (!(strcmp(ident->val,"bandstop")))
      {
	has.o_bandstop+= incr;
	return;
      }      
    if (!(strcmp(ident->val,"biquad")))
      {
	has.o_biquad+= incr;
	return;
      }      
    if (!(strcmp(ident->val,"buzz")))
      {
	has.o_buzz+= incr;
	return;
      }      
    return;
  case 'c':
    if (!(strcmp(ident->val,"ceil")))
      {
	has.o_ceil+= incr;
	return;
      }      
    if (!(strcmp(ident->val,"chorus")))
      {
	has.o_chorus+= incr;
	return;
      }      
    if (!(strcmp(ident->val,"comb")))
      {
	has.o_comb+= incr;
	return;
      }      
    if (!(strcmp(ident->val,"compressor")))
      {
	has.o_compressor+= incr;
	return;
      }      
    if (!(strcmp(ident->val,"cos")))
      {
	has.o_cos+= incr;
	return;
      }      
    if (!(strcmp(ident->val,"cpsmidi")))
      {
	has.o_cpsmidi+= incr;
	return;
      }      
    if (!(strcmp(ident->val,"cpsoct")))
      {
	has.o_cpsoct+= incr;
	return;
      }      
    if (!(strcmp(ident->val,"cpspch")))
      {
	has.o_cpspch+= incr;
	return;
      }      
    return;
  case 'd':
    if (!(strcmp(ident->val,"dbamp")))
      {
	has.o_dbamp+= incr;
	return;
      }      
    if (!(strcmp(ident->val,"decimate")))
      {
	has.o_decimate+= incr;
	return;
      }     
    if (!(strcmp(ident->val,"delay")))
      {
	has.o_delay+= incr;
	return;
      }
    if (!(strcmp(ident->val,"delay1")))
      {
	has.o_delay1+= incr;
	return;
      }      
    if (!(strcmp(ident->val,"doscil")))
      {
	has.o_doscil+= incr;
	return;
      }      
    if (!(strcmp(ident->val,"downsamp")))
      {
	has.o_downsamp+= incr;
	return;
      }     
    return;
  case 'e':
    if (!(strcmp(ident->val,"exp")))
      {
	has.o_exp+= incr;
	return;
      }     
    return;
  case 'f':
    if (!(strcmp(ident->val,"fft")))
      {
	has.o_fft+= incr;
	return;
      }     
    if (!(strcmp(ident->val,"fir")))
      {
	has.o_fir+= incr;
	return;
      }     
    if (!(strcmp(ident->val,"firt")))
      {
	has.o_firt+= incr;
	return;
      }      
    if (!(strcmp(ident->val,"flange")))
      {
	has.o_flange+= incr;
	return;
      }      
    if (!(strcmp(ident->val,"floor")))
      {
	has.o_floor+= incr;
	return;
      }      
    if (!(strcmp(ident->val,"frac")))
      {
	has.o_frac+= incr;
	return;
      }      
    if (!(strcmp(ident->val,"fracdelay")))
      {
	has.o_fracdelay+= incr;
	return;
      }      
    if (!(strcmp(ident->val,"ftbasecps")))
      {
	has.o_ftbasecps+= incr;
	return;
      }      
    if (!(strcmp(ident->val,"ftlen")))
      {
	has.o_ftlen+= incr;
	return;
      }      
    if (!(strcmp(ident->val,"ftloop")))
      {
	has.o_ftloop+= incr;
	return;
      }      
    if (!(strcmp(ident->val,"ftloopend")))
      {
	has.o_ftloopend+= incr;
	return;
      }      
    if (!(strcmp(ident->val,"ftsetbase")))
      {
	has.o_ftsetbase+= incr;
	return;
      }      
    if (!(strcmp(ident->val,"ftsetend")))
      {
	has.o_ftsetend+= incr;
	return;
      }      
    if (!(strcmp(ident->val,"ftsetloop")))
      {
	has.o_ftsetloop+= incr;
	return;
      }      
    if (!(strcmp(ident->val,"ftsetsr")))
      {
	has.o_ftsetsr+= incr;
	return;
      }      
    if (!(strcmp(ident->val,"ftsr")))
      {
	has.o_ftsr+= incr;
	return;
      }      
    return;
  case 'g':
    if (!(strcmp(ident->val,"gain")))
      {
	has.o_gain+= incr;
	return;
      }      
    if (!(strcmp(ident->val,"gettempo")))
      {
	has.o_gettempo+= incr;
	return;
      }      
    if (!(strcmp(ident->val,"gettune")))
      {
	has.o_gettune+= incr;
	return;
      }      
    if (!(strcmp(ident->val,"grain")))
      {
	has.o_grain+= incr;
	return;
      }      
    return;
  case 'h':
    if (!(strcmp(ident->val,"hipass")))
      {
	has.o_hipass+= incr;
	return;
      }    
    return;
  case 'i':
    if (!(strcmp(ident->val,"iexprand")))
      {
	has.o_iexprand+= incr;
	return;
      }      
    if (!(strcmp(ident->val,"ifft")))
      {
	has.o_ifft+= incr;
	return;
      }      
    if (!(strcmp(ident->val,"igaussrand")))
      {
	has.o_igaussrand+= incr;
	return;
      }      
    if (!(strcmp(ident->val,"iir")))
      {
	has.o_iir+= incr;
	return;
      }      
    if (!(strcmp(ident->val,"iirt")))
      {
	has.o_iirt+= incr;
	return;
      }      
    if (!(strcmp(ident->val,"ilinrand")))
      {
	has.o_ilinrand+= incr;
	return;
      }
    if (!(strcmp(ident->val,"int")))
      {
	has.o_int+= incr;
	return;
      }      
    if (!(strcmp(ident->val,"irand")))
      {
	has.o_irand+= incr;
	return;
      }     
    return;
  case 'j':
    return;
  case 'k':
    if (!(strcmp(ident->val,"kexpon")))
      {
	has.o_kexpon+= incr;
	return;
      }      
    if (!(strcmp(ident->val,"kexprand")))
      {
	has.o_kexprand+= incr;
	return;
      }      
    if (!(strcmp(ident->val,"kgaussrand")))
      {
	has.o_kgaussrand+= incr;
	return;
      }     
    if (!(strcmp(ident->val,"kline")))
      {
	has.o_kline+= incr;
	return;
      }    
    if (!(strcmp(ident->val,"klinrand")))
      {
	has.o_klinrand+= incr;
	return;
      }      
    if (!(strcmp(ident->val,"koscil")))
      {
	has.o_koscil+= incr;
	return;
      }      
    if (!(strcmp(ident->val,"kphasor")))
      {
	has.o_kphasor+= incr;
	return;
      }        
    if (!(strcmp(ident->val,"kpoissonrand")))
      {
	has.o_kpoissonrand+= incr;
	return;
      }       
    if (!(strcmp(ident->val,"krand")))
      {
	has.o_krand+= incr;
	return;
      }       
    return;
  case 'l':
    if (!(strcmp(ident->val,"log")))
      {
	has.o_log+= incr;
	return;
      }      
    if (!(strcmp(ident->val,"log10")))
      {
	has.o_log10+= incr;
	return;
      }      
    if (!(strcmp(ident->val,"lopass")))
      {
	has.o_lopass+= incr;
	return;
      }     
    if (!(strcmp(ident->val,"loscil")))
      {
	has.o_loscil+= incr;
	return;
      }    
    return;
  case 'm':
    if (!(strcmp(ident->val,"max")))
      {
	has.o_max+= incr;
	return;
      }      
    if (!(strcmp(ident->val,"midicps")))
      {
	has.o_midicps+= incr;
	return;
      }      
    if (!(strcmp(ident->val,"midioct")))
      {
	has.o_midioct+= incr;
	return;
      }      
    if (!(strcmp(ident->val,"midipch")))
      {
	has.o_midipch+= incr;
	return;
      }     
    if (!(strcmp(ident->val,"min")))
      {
	has.o_min+= incr;
	return;
      }      
    return;
  case 'n':
    return;
  case 'o':
    if (!(strcmp(ident->val,"octcps")))
      {
	has.o_octcps+= incr;
	return;
      }      
    if (!(strcmp(ident->val,"octmidi")))
      {
	has.o_octmidi+= incr;
	return;
      }    
    if (!(strcmp(ident->val,"octpch")))
      {
	has.o_octpch+= incr;
	return;
      } 
    if (!(strcmp(ident->val,"oscil")))
      {
	has.o_oscil+= incr;
	return;
      }     
    return;
  case 'p':
    if (!(strcmp(ident->val,"pchcps")))
      {
	has.o_pchcps+= incr;
	return;
      }       
    if (!(strcmp(ident->val,"pchmidi")))
      {
	has.o_pchmidi+= incr;
	return;
      }      
    if (!(strcmp(ident->val,"pchoct")))
      {
	has.o_pchoct+= incr;
	return;
      }    
    if (!(strcmp(ident->val,"pluck")))
      {
	has.o_pluck+= incr;
	return;
      }    
    if (!(strcmp(ident->val,"port")))
      {
	has.o_port+= incr;
	return;
      }   
    if (!(strcmp(ident->val,"pow")))
      {
	has.o_pow+= incr;
	return;
      }   
    return;
  case 'q':
    return;
  case 'r':
    if (!(strcmp(ident->val,"reverb")))
      {
	has.o_reverb+= incr;
	return;
      }   
    if (!(strcmp(ident->val,"rms")))
      {
	has.o_rms+= incr;
	return;
      }   
    return;
  case 's':
    if (!(strcmp(ident->val,"samphold")))
      {
	has.o_samphold+= incr;
	return;
      }       
    if (!(strcmp(ident->val,"sblock")))
      {
	has.o_sblock+= incr;
	return;
      }     
    if (!(strcmp(ident->val,"settempo")))
      {
	has.o_settempo+= incr;
	return;
      }    
    if (!(strcmp(ident->val,"settune")))
      {
	has.o_settune+= incr;
	return;
      }    
    if (!(strcmp(ident->val,"sgn")))
      {
	has.o_sgn+= incr;
	return;
      }   
    if (!(strcmp(ident->val,"sin")))
      {
	has.o_sin+= incr;
	return;
      }   
    if (!(strcmp(ident->val,"spatialize")))
      {
	has.spatialize+= incr;
	return;
      }   
    if (!(strcmp(ident->val,"speedt")))
      {
	has.o_speedt+= incr;
	return;
      }   
    if (!(strcmp(ident->val,"sqrt")))
      {
	has.o_sqrt+= incr;
	return;
      }   
    return;
  case 't':
    if (!(strcmp(ident->val,"tableread")))
      {
	has.o_tableread+= incr;
	return;
      }   
    if (!(strcmp(ident->val,"tablewrite")))
      {
	has.o_tablewrite+= incr;
	return;
      }   
    return;
  case 'u':
    if (!(strcmp(ident->val,"upsamp")))
      {
	has.o_upsamp+= incr;
	return;
      }   
    return;
  case 'v':
    return;
  case 'w':
    return;
  case 'x':
    return;
  case 'y':
    return;
  case 'z':
    return;
  default:
    return;
 }
}

/*********************************************************/
/*       optrefer reference counts for core opcodes      */
/*********************************************************/

void corerefer(sigsym * sptr, tnode * ident, tnode * tptr, int passtype)

{
  
  switch (ident->val[0]) {
  case 'f':
    if (!(strcmp(ident->val,"fft")))
      {
	/* find the re table */
	
	tptr = tptr->next;
	
	while (tptr->ttype != S_EXPR)
	  tptr = tptr->next;
	
	/* reference counts for re */
	
	(tptr->down->sptr->tref->assigntot)++;
	(tptr->down->sptr->tref->assigntval)++;
	if (whilerefdepth)
	  (tptr->down->sptr->tref->assignwhile)++;
	if (ifrefdepth)
	  (tptr->down->sptr->tref->assignif)++;
	
	/* since its a special-op */
	
	tptr->down->sptr->tref->assignrate = ARATETYPE;
	tmaprefer(tptr->down->sptr, TVALCHANGE, ARATETYPE);

	tptr = tptr->next;
	
	/* find the im table */
	
	while (tptr->ttype != S_EXPR)
	  tptr = tptr->next;
	
	/* reference counts for im */
	
	(tptr->down->sptr->tref->assigntot)++;
	(tptr->down->sptr->tref->assigntval)++;
	if (whilerefdepth)
	  (tptr->down->sptr->tref->assignwhile)++;
	if (ifrefdepth)
	  (tptr->down->sptr->tref->assignif)++;
	tptr->down->sptr->tref->assignrate = ARATETYPE;
	tmaprefer(tptr->down->sptr, TVALCHANGE, ARATETYPE);
	
	/* reference counts for instr/opcode */
	
	(sptr->cref->statewave)++;
	return;
      }     
    if ((!(strcmp(ident->val,"ftsetbase"))) ||
	(!(strcmp(ident->val,"ftsetend")))  ||
	(!(strcmp(ident->val,"ftsetloop"))) ||
	(!(strcmp(ident->val,"ftsetsr"))))
      {
	(tptr->down->sptr->tref->assigntot)++;
	if (whilerefdepth)
	  (tptr->down->sptr->tref->assignwhile)++;
	if (ifrefdepth)
	  (tptr->down->sptr->tref->assignif)++;
	if (passtype > tptr->down->sptr->tref->assignrate)
	  tptr->down->sptr->tref->assignrate = passtype;
	tmaprefer(tptr->down->sptr, TPARAMCHANGE, passtype);

	(sptr->cref->statewave)++;
	return;
      }      
    return;
  case 's':
    if (!(strcmp(ident->val,"sblock")))
      {	  
	/* find the table */

	tptr = tptr->next;
	  
	while (tptr->ttype != S_EXPR)
	  tptr = tptr->next;

	/* reference counts for table */

	(tptr->down->sptr->tref->assigntot)++;
	(tptr->down->sptr->tref->assigntval)++;
	if (whilerefdepth)
	  (tptr->down->sptr->tref->assignwhile)++;
	if (ifrefdepth)
	  (tptr->down->sptr->tref->assignif)++;

	/* since its a special-op */

	tptr->down->sptr->tref->assignrate = ARATETYPE;
	tmaprefer(tptr->down->sptr, TVALCHANGE, ARATETYPE);

	/* reference counts for instr/opcode */

	(sptr->cref->statewave)++;
	
	return;
      }     
    if (!(strcmp(ident->val,"settune")))
      {
	if (passtype > sptr->cref->settune)
	  sptr->cref->settune = passtype;
	return;
      }     
    if (!(strcmp(ident->val,"speedt")))
      {
	/* find the out table */

	tptr = tptr->next;
	  
	while (tptr->ttype != S_EXPR)
	  tptr = tptr->next;

	/* reference counts for table */

	(tptr->down->sptr->tref->assigntot)++;
	(tptr->down->sptr->tref->assigntval)++;
	if (whilerefdepth)
	  (tptr->down->sptr->tref->assignwhile)++;
	if (ifrefdepth)
	  (tptr->down->sptr->tref->assignif)++;
	if (passtype > tptr->down->sptr->tref->assignrate)
	  tptr->down->sptr->tref->assignrate = passtype;
	tmaprefer(tptr->down->sptr, TVALCHANGE, passtype);

	/* reference counts for instr/opcode */

	(sptr->cref->statewave)++;
	
	return;
      }   
    return;
  case 't':
    if (!(strcmp(ident->val,"tablewrite")))
      {
	(tptr->down->sptr->tref->assigntot)++;
	(tptr->down->sptr->tref->assigntval)++;
	if (whilerefdepth)
	  (tptr->down->sptr->tref->assignwhile)++;
	if (ifrefdepth)
	  (tptr->down->sptr->tref->assignif)++;
	if (passtype > tptr->down->sptr->tref->assignrate)
	  tptr->down->sptr->tref->assignrate = passtype;
	tmaprefer(tptr->down->sptr, TVALCHANGE, passtype);

	(sptr->cref->statewave)++;
	return;
      }   
    return;
  default:
    return;
 }
}

/*********************************************************/
/* returns 1 if S_IDENT is a  specialop, else 0 */
/*********************************************************/

int coreopcodespecial(tnode * ident)

{

  if (!(strcmp(ident->val,"fft")))
    return 1;
  if (!(strcmp(ident->val,"rms")))
    return 1;
  if (!(strcmp(ident->val,"sblock")))
    return 1;
  if (!(strcmp(ident->val,"downsamp")))
    return 1;
  if (!(strcmp(ident->val,"decimate")))
    return 1;
  return 0;

}

/*********************************************************/
/* returns ASINT if a core opcode always returns an INT  */
/* else returns ASFLOAT                                  */
/*********************************************************/

int coreopcodeasint(tnode * ident)

{
  /* only does int() right now, because there's  */
  /* never a speed hit taken. expansion to other */
  /* integral opcodes should be done with care   */

  if (!(strcmp(ident->val,"int")))
    return ASINT;

  return ASFLOAT;

}

/*********************************************************/
/*             creates an opcode declaration             */
/*  optype IDENT LP paramlist RP LC opvardecls block RC  */
/*********************************************************/

sigsym * createopcode(tnode * tptr, sigsym ** nametable, int type)

{
  sigsym * newsym;
  tnode * t_optype = NULL; /* initialization not needed */
  tnode * t_ident;
  tnode * t_lp;
  tnode * t_paramlist;
  tnode * t_rp;
  tnode * t_lc;
  tnode * t_opvardecls;
  tnode * t_block;
  tnode * t_rc;

  symcheck(addvsym(nametable, tptr->val, K_OPCODENAME), tptr);
  newsym = (*nametable);
  newsym->width = 1;
  newsym->defnode = make_tnode("<opcodedecl>",S_OPCODEDECL);

  switch (type) {
  case S_IOPCODE:
    t_optype =  make_tnode("iopcode", type);
    newsym->rate = t_optype->rate = IRATETYPE;
    break;
  case S_KOPCODE:
    t_optype =  make_tnode("kopcode", type);
    newsym->rate = t_optype->rate = KRATETYPE;
    break;
  case S_SOPCODE:
    t_optype =  make_tnode("kopcode", type);
    newsym->rate = t_optype->rate = KRATETYPE;
    newsym->special = t_optype->special = 1;
    break;
  case S_AOPCODE:
    t_optype =  make_tnode("aopcode", type);
    newsym->rate = t_optype->rate = ARATETYPE;
    break;
  case S_OPCODE:
    t_optype =  make_tnode("opcode", type);
    newsym->rate = t_optype->rate = XRATETYPE;
    break;
  }

  t_ident = make_tnode(tptr->val, S_IDENT);
  t_lp = make_tnode("(", S_LP);
  t_paramlist = make_tnode("<paramlist>",S_PARAMLIST);
  t_rp = make_tnode(")", S_RP);
  t_lc = make_tnode("{", S_LC);
  t_opvardecls = make_tnode("<opvardecls>",S_OPVARDECLS);
  t_block = make_tnode("<block>",S_BLOCK);
  t_rc = make_tnode("}", S_RC);

  newsym->defnode->down = t_optype;
  t_optype->next = t_ident;
  t_ident->next = t_lp;
  t_lp->next = t_paramlist;
  t_paramlist->next = t_rp;
  t_rp->next = t_lc;
  t_lc->next = t_opvardecls;
  t_opvardecls->next = t_block;
  t_block->next = t_rc;

  t_paramlist->rate = UNKNOWN; 
  if (type == S_SOPCODE)
    t_paramlist->special = 1;

  return newsym;

}

/*********************************************************/
/*     adds a parameter to an opcode definition          */
/*********************************************************/


void addopcodevar(sigsym * newsym, int type, char * name)

{
  tnode * paramdecl; 

  symcheck(addvsym(&(newsym->defnode->sptr), name, K_NORMAL), NULL);
  newsym->defnode->sptr->width = 1;
  paramdecl = newsym->defnode->down->next->next->next;

  switch (type) {
  case S_IVAR:
    newsym->defnode->sptr->rate = IRATETYPE;
    if (paramdecl->rate == UNKNOWN)
      paramdecl->rate = IRATETYPE;
    break;
  case S_KSIG:
    newsym->defnode->sptr->rate = KRATETYPE;
    if ( (paramdecl->rate == UNKNOWN) ||
	 (paramdecl->rate == IRATETYPE) )
      paramdecl->rate = KRATETYPE;
    break;
  case S_ASIG:
    paramdecl->rate = newsym->defnode->sptr->rate = ARATETYPE;
    break;
  case S_XSIG:
    newsym->defnode->sptr->rate = XRATETYPE;
    break;
  case S_TABLE:
    newsym->defnode->sptr->rate = IRATETYPE;
    newsym->defnode->sptr->vartype =TABLETYPE;
    newsym->defnode->sptr->kind = K_INTERNAL;
    break;
  default:
    break;
  }

  return;

}

/*********************************************************/
/*     adds a parameter to an opcode definition          */
/*********************************************************/

void addopcodepfield(sigsym * newsym, int type, char * name)


{

  tnode * tptr;
  tnode * newparam;

  tptr = newsym->defnode->down->next->next->next;
  newparam = make_tnode("<paramdecl>",S_PARAMDECL);
  switch (type) {
  case S_IVAR:
    newparam->down = make_tnode("ivar",type);
    newparam->rate = IRATETYPE;
    break;
  case S_KSIG:
    newparam->down = make_tnode("ksig",type);
    newparam->rate = KRATETYPE;
    tptr->rate = (tptr->rate == IRATETYPE) ? KRATETYPE : tptr->rate;
    break;
  case S_ASIG:
    newparam->down = make_tnode("asig",type);
    tptr->rate = newparam->rate = ARATETYPE;
    break;
  case S_XSIG:
    newparam->down = make_tnode("xsig",type);
    newparam->rate = XRATETYPE;
    break;
  case S_TABLE:
    newparam->down = make_tnode("table",type);
    newparam->vartype = TABLETYPE;
    newparam->rate = IRATETYPE;
    break;
  default:
    break;
  }

  newparam->down->next = make_tnode("<name>",S_NAME);
  newparam->down->next->down = make_tnode(name,S_IDENT);

  symcheck(addvsym(&(newsym->defnode->sptr), name, K_PFIELD), NULL);
  newparam->sptr = newparam->down->next->sptr =
    newparam->down->next->down->sptr = newsym->defnode->sptr;
  newsym->defnode->sptr->rate = newparam->down->rate =
    newparam->down->next->rate = newparam->rate;
  newsym->defnode->sptr->vartype = newparam->down->vartype =
    newparam->down->next->vartype = newparam->vartype;
  newsym->defnode->sptr->width = newparam->width
    = newparam->down->width = newparam->down->next->width = 1;

  if (tptr->down == NULL)
    tptr->down = newparam;
  else
    {
      tptr = tptr->down;
      while (tptr->next != NULL)
	tptr = tptr->next;
      tptr->next = make_tnode(",",S_COM);
      tptr->next->next = newparam;
    }

}



/*********************************************************/
/*     adds varargs parameter to an opcode call          */
/*********************************************************/


void addextraparam(tnode * tcall, int type, char * name)

{

  tnode * newparam;

  newparam = make_tnode("<paramdecl>",S_PARAMDECL);
  switch (type) {
  case S_IVAR:
    newparam->down = make_tnode("ivar",type);
    newparam->rate = IRATETYPE;
    if (tcall->extrarate == UNKNOWN)
      tcall->extrarate = IRATETYPE;
    break;
  case S_KSIG:
    newparam->down = make_tnode("ksig",type);
    newparam->rate = KRATETYPE;
    if ((tcall->extrarate == UNKNOWN)||
	(tcall->extrarate == IRATETYPE))
      tcall->extrarate = KRATETYPE;
    break;
  case S_ASIG:
    newparam->down = make_tnode("asig",type);
    tcall->extrarate = newparam->rate = ARATETYPE;
    break;
  case S_XSIG:
    newparam->down = make_tnode("xsig",type);
    newparam->rate = XRATETYPE;
    break;
  case S_TABLE:
    newparam->down = make_tnode("table",type);
    newparam->vartype = TABLETYPE;
    break;
  default:
    break;
  }

  newparam->down->next = make_tnode("<name>", S_NAME);
  newparam->down->next->down = make_tnode(name, S_IDENT);
  newparam->down->rate = newparam->down->next->rate 
    = newparam->rate;
  newparam->down->width = newparam->down->next->width 
    = newparam->width = 1;

  if (tcall->extra == NULL)
    tcall->extra = newparam;
  else
    {
      tcall = tcall->extra;
      while (tcall->next != NULL)
	tcall = tcall->next;
      tcall->next = make_tnode(",",S_COM);
      tcall->next->next = newparam;
    }
}


/*********************************************************/
/* adds one-argument, xrate opcode to nametable          */
/*********************************************************/

sigsym * oneargopcode(tnode * tptr, sigsym ** nametable)

{
  sigsym * newsym;

  newsym = createopcode(tptr, nametable, S_OPCODE);
  addopcodepfield(newsym, S_XSIG,"x");
  return newsym;
}


/*********************************************************/
/*      adds needed core opcode to nametable             */
/*********************************************************/

sigsym * coreopcodeadd(tnode * tptr, sigsym ** nametable)


{
  sigsym * newsym;

  switch (tptr->val[0]) {
  case 'a':
    if (!(strcmp(tptr->val,"abs")))
      return oneargopcode(tptr, nametable);
    if (!(strcmp(tptr->val,"acos")))
      return oneargopcode(tptr, nametable);
    if (!(strcmp(tptr->val,"aexpon")))
      {
	newsym = createopcode(tptr, nametable, S_AOPCODE);
	addopcodepfield(newsym, S_IVAR,"x1");
	addopcodepfield(newsym, S_IVAR,"dur1");
	addopcodepfield(newsym, S_IVAR,"x2");
	addopcodevar(newsym, S_ASIG,"t");
	addopcodevar(newsym, S_ASIG,"clp");
	addopcodevar(newsym, S_ASIG,"crp");
	addopcodevar(newsym, S_ASIG,"cdur");
	addopcodevar(newsym, S_ASIG,"ratio");
	addopcodevar(newsym, S_ASIG,"invcdur");
	addopcodevar(newsym, S_ASIG,"outT");
	addopcodevar(newsym, S_ASIG,"multK");
	addopcodevar(newsym, S_ASIG,"first");
	return newsym;
      }
    if (!(strcmp(tptr->val,"aexprand")))
      {
	newsym = createopcode(tptr, nametable, S_AOPCODE);
	addopcodepfield(newsym, S_ASIG,"p1");
	return newsym;
      }
    if (!(strcmp(tptr->val,"agaussrand")))
      {
	newsym = createopcode(tptr, nametable, S_AOPCODE);
	addopcodepfield(newsym, S_ASIG,"mean");
	addopcodepfield(newsym, S_ASIG,"var");
	return newsym;
      }
    if (!(strcmp(tptr->val,"aline")))
      {	
	newsym = createopcode(tptr, nametable, S_AOPCODE);
	addopcodepfield(newsym, S_IVAR,"x1");
	addopcodepfield(newsym, S_IVAR,"dur1");
	addopcodepfield(newsym, S_IVAR,"x2");
	addopcodevar(newsym, S_ASIG,"t");
	addopcodevar(newsym, S_ASIG,"clp");
	addopcodevar(newsym, S_ASIG,"crp");
	addopcodevar(newsym, S_ASIG,"cdur");
	addopcodevar(newsym, S_ASIG,"mult");
	addopcodevar(newsym, S_ASIG,"outT");
	addopcodevar(newsym, S_ASIG,"addK");
	addopcodevar(newsym, S_ASIG,"first");
	return newsym;
      }
    if (!(strcmp(tptr->val,"alinrand")))
      {
	newsym = createopcode(tptr, nametable, S_AOPCODE);
	addopcodepfield(newsym, S_ASIG,"p1");
	addopcodepfield(newsym, S_ASIG,"p2");
	return newsym;
      }
    if (!(strcmp(tptr->val,"allpass")))
      {
	newsym = createopcode(tptr, nametable, S_AOPCODE);
	addopcodepfield(newsym, S_ASIG,"input");
	addopcodepfield(newsym, S_IVAR,"time");
	addopcodepfield(newsym, S_IVAR,"gain");
	addopcodevar(newsym, S_TABLE,"dline");
	addopcodevar(newsym, S_ASIG,"kcyc");
	return newsym;
      }
    if (!(strcmp(tptr->val,"ampdb")))
      return oneargopcode(tptr, nametable);
    if (!(strcmp(tptr->val,"aphasor")))
      {
	newsym = createopcode(tptr, nametable, S_AOPCODE);
	addopcodepfield(newsym, S_ASIG,"cps");
	addopcodevar(newsym, S_ASIG,"pint");
	addopcodevar(newsym, S_ASIG,"pfrac");
	addopcodevar(newsym, S_ASIG,"kcyc");
	return newsym;
      }
    if (!(strcmp(tptr->val,"apoissonrand")))
      {
	newsym = createopcode(tptr, nametable, S_AOPCODE);
	addopcodepfield(newsym, S_ASIG,"p1");
	addopcodevar(newsym, S_ASIG,"state");
	return newsym;
      }
    if (!(strcmp(tptr->val,"arand")))
      {
	newsym = createopcode(tptr, nametable, S_AOPCODE);
	addopcodepfield(newsym, S_ASIG,"p");
	return newsym;
      }
    if (!(strcmp(tptr->val,"asin")))
      return oneargopcode(tptr, nametable);
    if (!(strcmp(tptr->val,"atan")))
      return oneargopcode(tptr, nametable);
    return NULL;
  case 'b':
    if (!(strcmp(tptr->val,"balance")))
      {
	newsym = createopcode(tptr, nametable, S_AOPCODE);
	addopcodepfield(newsym, S_ASIG,"x");
	addopcodepfield(newsym, S_ASIG,"ref");
	addopcodevar(newsym, S_KSIG,"lcount");
	addopcodevar(newsym, S_KSIG,"lval");
	addopcodevar(newsym, S_ASIG,"atten");
	addopcodevar(newsym, S_ASIG,"acc");
	addopcodevar(newsym, S_ASIG,"racc");
	return newsym;
      }
    if (!(strcmp(tptr->val,"bandpass")))
      {
	newsym = createopcode(tptr, nametable, S_AOPCODE);
	addopcodepfield(newsym, S_ASIG,"input");
	addopcodepfield(newsym, S_KSIG,"cf");
	addopcodepfield(newsym, S_KSIG,"bw");
	addopcodevar(newsym, S_ASIG,"kcyc");
	addopcodevar(newsym, S_ASIG,"ocf");
	addopcodevar(newsym, S_ASIG,"obw");
	addopcodevar(newsym, S_ASIG,"b0");
	addopcodevar(newsym, S_ASIG,"b1");
	addopcodevar(newsym, S_ASIG,"b2");
	addopcodevar(newsym, S_ASIG,"a1");
	addopcodevar(newsym, S_ASIG,"a2");
	addopcodevar(newsym, S_ASIG,"d1");
	addopcodevar(newsym, S_ASIG,"d2");
	return newsym;
      }
    if (!(strcmp(tptr->val,"bandstop")))
      {
	newsym = createopcode(tptr, nametable, S_AOPCODE);
	addopcodepfield(newsym, S_ASIG,"input");
	addopcodepfield(newsym, S_KSIG,"cf");
	addopcodepfield(newsym, S_KSIG,"bw");
	addopcodevar(newsym, S_ASIG,"kcyc");
	addopcodevar(newsym, S_ASIG,"ocf");
	addopcodevar(newsym, S_ASIG,"obw");
	addopcodevar(newsym, S_ASIG,"b0");
	addopcodevar(newsym, S_ASIG,"b1");
	addopcodevar(newsym, S_ASIG,"b2");
	addopcodevar(newsym, S_ASIG,"a1");
	addopcodevar(newsym, S_ASIG,"a2");
	addopcodevar(newsym, S_ASIG,"d1");
	addopcodevar(newsym, S_ASIG,"d2");
	return newsym;
      }
    if (!(strcmp(tptr->val,"biquad")))
      {
	newsym = createopcode(tptr, nametable, S_AOPCODE);
	addopcodepfield(newsym, S_ASIG,"input");
	addopcodepfield(newsym, S_IVAR,"b0");
	addopcodepfield(newsym, S_IVAR,"b1");
	addopcodepfield(newsym, S_IVAR,"b2");
	addopcodepfield(newsym, S_IVAR,"a1");
	addopcodepfield(newsym, S_IVAR,"a2");
	addopcodevar(newsym, S_ASIG,"d1");
	addopcodevar(newsym, S_ASIG,"d2");
	addopcodevar(newsym, S_ASIG,"first");
	return newsym;
      }
    if (!(strcmp(tptr->val,"buzz")))
      {
	newsym = createopcode(tptr, nametable, S_AOPCODE);
	addopcodepfield(newsym, S_ASIG,"cps");
	addopcodepfield(newsym, S_KSIG,"nharm");
	addopcodepfield(newsym, S_KSIG,"lowharm");
	addopcodepfield(newsym, S_KSIG,"rolloff");

	addopcodevar(newsym, S_ASIG,"p");
	addopcodevar(newsym, S_ASIG,"scale");
	addopcodevar(newsym, S_ASIG,"r");
	addopcodevar(newsym, S_ASIG,"ntab");
	addopcodevar(newsym, S_ASIG,"qtab");
	addopcodevar(newsym, S_ASIG,"d");
	addopcodevar(newsym, S_ASIG,"k1");
	addopcodevar(newsym, S_ASIG,"k2");
	addopcodevar(newsym, S_ASIG,"kcyc");

	return newsym;
      }
    return NULL;
  case 'c':
    if (!(strcmp(tptr->val,"ceil")))
      return oneargopcode(tptr, nametable);
    if (!(strcmp(tptr->val,"chorus")))
      {
	newsym = createopcode(tptr, nametable, S_AOPCODE);
	addopcodepfield(newsym, S_ASIG,"x");
	addopcodepfield(newsym, S_KSIG,"rate");
	addopcodepfield(newsym, S_KSIG,"depth");
	addopcodevar(newsym, S_ASIG,"p");
	addopcodevar(newsym, S_ASIG,"kcyc");
	addopcodevar(newsym, S_TABLE,"dline");
	addopcodevar(newsym, S_TABLE,"sweep");
	return newsym;
      }
    if (!(strcmp(tptr->val,"comb")))
      {
	newsym = createopcode(tptr, nametable, S_AOPCODE);
	addopcodepfield(newsym, S_ASIG,"input");
	addopcodepfield(newsym, S_IVAR,"time");
	addopcodepfield(newsym, S_IVAR,"gain");
	addopcodevar(newsym, S_TABLE,"dline");
	addopcodevar(newsym, S_ASIG,"kcyc");
	return newsym;
      }
    if (!(strcmp(tptr->val,"compressor")))
      {
	newsym = createopcode(tptr, nametable, S_AOPCODE);
	addopcodepfield(newsym, S_ASIG,"x");
	addopcodepfield(newsym, S_ASIG,"comp");
	addopcodepfield(newsym, S_KSIG,"nfloor");
	addopcodepfield(newsym, S_KSIG,"thresh");
	addopcodepfield(newsym, S_KSIG,"loknee");
	addopcodepfield(newsym, S_KSIG,"hiknee");
	addopcodepfield(newsym, S_KSIG,"ratio");
	addopcodepfield(newsym, S_KSIG,"att");
	addopcodepfield(newsym, S_KSIG,"rel");
	addopcodepfield(newsym, S_IVAR,"look");
	addopcodevar(newsym, S_TABLE,"xdly");
	addopcodevar(newsym, S_TABLE,"compdly");
	addopcodevar(newsym, S_ASIG,"change");
	addopcodevar(newsym, S_ASIG,"comp1");
	addopcodevar(newsym, S_ASIG,"comp2");
	addopcodevar(newsym, S_ASIG,"env");
	addopcodevar(newsym, S_ASIG,"projEnv");
	addopcodevar(newsym, S_ASIG,"oldval");
	addopcodevar(newsym, S_ASIG,"invatt");
	addopcodevar(newsym, S_ASIG,"invrel");
	addopcodevar(newsym, S_ASIG,"mult");
	addopcodevar(newsym, S_ASIG,"lval");
	addopcodevar(newsym, S_ASIG,"invr");
	addopcodevar(newsym, S_ASIG,"kcyc");
	addopcodevar(newsym, S_KSIG,"mtail");
	addopcodevar(newsym, S_KSIG,"xtail");
	addopcodevar(newsym, S_KSIG,"logmin");
	return newsym;
      }
    if (!(strcmp(tptr->val,"cos")))
      return oneargopcode(tptr, nametable);
    if (!(strcmp(tptr->val,"cpsmidi")))
      return oneargopcode(tptr, nametable);
    if (!(strcmp(tptr->val,"cpsoct")))
      return oneargopcode(tptr, nametable);
    if (!(strcmp(tptr->val,"cpspch")))
      return oneargopcode(tptr, nametable);
    return NULL;
  case 'd':
    if (!(strcmp(tptr->val,"dbamp")))
      return oneargopcode(tptr, nametable);
    if (!(strcmp(tptr->val,"decimate")))
      {
	newsym = createopcode(tptr, nametable, S_SOPCODE);
	addopcodepfield(newsym, S_ASIG,"input");
	addopcodevar(newsym, S_ASIG,"state");
	addopcodevar(newsym, S_ASIG,"krun");
	return newsym;
      }
    if (!(strcmp(tptr->val,"delay")))
      {
	newsym = createopcode(tptr, nametable, S_AOPCODE);
	addopcodepfield(newsym, S_ASIG,"x");
	addopcodepfield(newsym, S_IVAR,"t");
	addopcodevar(newsym, S_TABLE,"dline");
	return newsym;
      }
    if (!(strcmp(tptr->val,"delay1")))
      {
	newsym = createopcode(tptr, nametable, S_AOPCODE);
	addopcodepfield(newsym, S_ASIG,"x");
	addopcodevar(newsym, S_ASIG,"d");
	return newsym;
      }
    if (!(strcmp(tptr->val,"doscil")))
      {	
	newsym = createopcode(tptr, nametable, S_AOPCODE);
	addopcodepfield(newsym, S_TABLE,"t");
	addopcodevar(newsym, S_ASIG,"pint");
	addopcodevar(newsym, S_ASIG,"pfrac");
	addopcodevar(newsym, S_ASIG,"play");
	return newsym;
      }
    if (!(strcmp(tptr->val,"downsamp")))
      {
	newsym = createopcode(tptr, nametable, S_SOPCODE);
	addopcodepfield(newsym, S_ASIG,"input");
	addopcodevar(newsym, S_ASIG,"kcyc");
	addopcodevar(newsym, S_TABLE,"buffer");
	return newsym;
      }
    return NULL;
  case 'e':
    if (!(strcmp(tptr->val,"exp")))
      return oneargopcode(tptr, nametable);
    return NULL;
  case 'f':
    if (!(strcmp(tptr->val,"fft")))
      {
	newsym = createopcode(tptr, nametable, S_SOPCODE);
	addopcodepfield(newsym, S_ASIG,"input");
	addopcodepfield(newsym, S_TABLE,"re");
	addopcodepfield(newsym, S_TABLE,"im");
	addopcodevar(newsym, S_KSIG,"done");
	addopcodevar(newsym, S_TABLE,"buffer");
	addopcodevar(newsym, S_TABLE,"new");
	addopcodevar(newsym, S_TABLE,"cos");
	addopcodevar(newsym, S_TABLE,"map");
	addopcodevar(newsym, S_IVAR,"scale");
	return newsym;
      }
    if (!(strcmp(tptr->val,"fir")))
      {
	newsym = createopcode(tptr, nametable, S_AOPCODE);
	addopcodepfield(newsym, S_ASIG,"input");
	addopcodepfield(newsym, S_KSIG,"b0");
	addopcodevar(newsym, S_ASIG,"p");
	return newsym;
      }
    if (!(strcmp(tptr->val,"firt")))
      {
	newsym = createopcode(tptr, nametable, S_AOPCODE);
	addopcodepfield(newsym, S_ASIG,"input");
	addopcodepfield(newsym, S_TABLE,"t");
	addopcodevar(newsym, S_TABLE,"dline");
	addopcodevar(newsym, S_ASIG,"kcyc");
	return newsym;
      }
    if (!(strcmp(tptr->val,"flange")))
      {
	newsym = createopcode(tptr, nametable, S_AOPCODE);
	addopcodepfield(newsym, S_ASIG,"x");
	addopcodepfield(newsym, S_KSIG,"rate");
	addopcodepfield(newsym, S_KSIG,"depth");
	addopcodevar(newsym, S_ASIG,"p");
	addopcodevar(newsym, S_ASIG,"kcyc");
	addopcodevar(newsym, S_TABLE,"dline");
	addopcodevar(newsym, S_TABLE,"sweep");
	return newsym;
      }
    if (!(strcmp(tptr->val,"floor")))
      return oneargopcode(tptr, nametable);
    if (!(strcmp(tptr->val,"frac")))
      return oneargopcode(tptr, nametable);
    if (!(strcmp(tptr->val,"fracdelay")))
      {
	newsym = createopcode(tptr, nametable, S_AOPCODE);
	addopcodepfield(newsym, S_KSIG,"method");
	addopcodevar(newsym, S_TABLE,"dline");
	return newsym;
      }
    if (!(strcmp(tptr->val,"ftbasecps")))
      {
	newsym = createopcode(tptr, nametable, S_OPCODE);
	addopcodepfield(newsym, S_TABLE,"t");
	return newsym;
      }
    if (!(strcmp(tptr->val,"ftlen")))
      {
	newsym = createopcode(tptr, nametable, S_OPCODE);
	addopcodepfield(newsym, S_TABLE,"t");
	return newsym;
      }
    if (!(strcmp(tptr->val,"ftloop")))
      {
	newsym = createopcode(tptr, nametable, S_OPCODE);
	addopcodepfield(newsym, S_TABLE,"t");
	return newsym;
      }
    if (!(strcmp(tptr->val,"ftloopend")))
      {
	newsym = createopcode(tptr, nametable, S_OPCODE);
	addopcodepfield(newsym, S_TABLE,"t");
	return newsym;
      }
    if (!(strcmp(tptr->val,"ftsetbase")))
      {
	newsym = createopcode(tptr, nametable, S_KOPCODE);
	addopcodepfield(newsym, S_TABLE,"t");
	addopcodepfield(newsym, S_KSIG,"x");
	return newsym;
      }
    if (!(strcmp(tptr->val,"ftsetend")))
      {
	newsym = createopcode(tptr, nametable, S_KOPCODE);
	addopcodepfield(newsym, S_TABLE,"t");
	addopcodepfield(newsym, S_KSIG,"x");
	return newsym;
      }
    if (!(strcmp(tptr->val,"ftsetloop")))
      {
	newsym = createopcode(tptr, nametable, S_KOPCODE);
	addopcodepfield(newsym, S_TABLE,"t");
	addopcodepfield(newsym, S_KSIG,"x");
	return newsym;
      }
    if (!(strcmp(tptr->val,"ftsetsr")))
      {
	newsym = createopcode(tptr, nametable, S_KOPCODE);
	addopcodepfield(newsym, S_TABLE,"t");
	addopcodepfield(newsym, S_KSIG,"x");
	return newsym;
      }
    if (!(strcmp(tptr->val,"ftsr")))
      {
	newsym = createopcode(tptr, nametable, S_OPCODE);
	addopcodepfield(newsym, S_TABLE,"t");
	return newsym;
      }
    return NULL;
  case 'g':
    if (!(strcmp(tptr->val,"gain")))
      {
	newsym = createopcode(tptr, nametable, S_AOPCODE);
	addopcodepfield(newsym, S_ASIG,"x");
	addopcodepfield(newsym, S_KSIG,"gain");
	addopcodevar(newsym, S_KSIG,"lcount");
	addopcodevar(newsym, S_KSIG,"lval");
	addopcodevar(newsym, S_KSIG,"scale");
	addopcodevar(newsym, S_ASIG,"atten");
	addopcodevar(newsym, S_ASIG,"acc");
	return newsym;
      }
    if (!(strcmp(tptr->val,"gettempo")))
      {
	newsym = createopcode(tptr, nametable, S_OPCODE);
	return newsym;
      }
    if (!(strcmp(tptr->val,"gettune")))
      {
	newsym = createopcode(tptr, nametable, S_OPCODE);
	return newsym;
      }
    if (!(strcmp(tptr->val,"grain")))
      {
	newsym = createopcode(tptr, nametable, S_AOPCODE);
	addopcodepfield(newsym, S_TABLE,"wave");
	addopcodepfield(newsym, S_TABLE,"env");
	addopcodepfield(newsym, S_KSIG,"density");
	addopcodepfield(newsym, S_KSIG,"freq");
	addopcodepfield(newsym, S_KSIG,"amp");
	addopcodepfield(newsym, S_KSIG,"dur");
	addopcodepfield(newsym, S_KSIG,"time");
	addopcodepfield(newsym, S_KSIG,"phase");
	addopcodevar(newsym, S_TABLE,"state");
	addopcodevar(newsym, S_ASIG,"kcyc");
	addopcodevar(newsym, S_ASIG,"trip");
	addopcodevar(newsym, S_ASIG,"oscil");
	addopcodevar(newsym, S_ASIG,"dclock");
	addopcodevar(newsym, S_ASIG,"tclock");
	addopcodevar(newsym, S_ASIG,"invdens");
	addopcodevar(newsym, S_ASIG,"lconst");
	addopcodevar(newsym, S_ASIG,"dconst");
	return newsym;
      }
    return NULL;
  case 'h':
    if (!(strcmp(tptr->val,"hipass")))
      {
	newsym = createopcode(tptr, nametable, S_AOPCODE);
	addopcodepfield(newsym, S_ASIG,"input");
	addopcodepfield(newsym, S_KSIG,"cut");
	addopcodevar(newsym, S_ASIG,"kcyc");
	addopcodevar(newsym, S_ASIG,"ocut");
	addopcodevar(newsym, S_ASIG,"b0");
	addopcodevar(newsym, S_ASIG,"b1");
	addopcodevar(newsym, S_ASIG,"b2");
	addopcodevar(newsym, S_ASIG,"a1");
	addopcodevar(newsym, S_ASIG,"a2");
	addopcodevar(newsym, S_ASIG,"d1");
	addopcodevar(newsym, S_ASIG,"d2");
	return newsym;
      }
    return NULL;
  case 'i':
    if (!(strcmp(tptr->val,"iexprand")))
      {
	newsym = createopcode(tptr, nametable, S_IOPCODE);
	addopcodepfield(newsym, S_IVAR,"p1");
	return newsym;
      }
    if (!(strcmp(tptr->val,"ifft")))
      {
	newsym = createopcode(tptr, nametable, S_AOPCODE);
	addopcodepfield(newsym, S_TABLE,"re");
	addopcodepfield(newsym, S_TABLE,"im");
	addopcodevar(newsym, S_TABLE,"buffer");
	addopcodevar(newsym, S_TABLE,"new");
	addopcodevar(newsym, S_TABLE,"imnew");
	addopcodevar(newsym, S_TABLE,"cos");
	addopcodevar(newsym, S_TABLE,"map");
	addopcodevar(newsym, S_IVAR,"scale");
	return newsym;
      }
    if (!(strcmp(tptr->val,"igaussrand")))
      {
	newsym = createopcode(tptr, nametable, S_IOPCODE);
	addopcodepfield(newsym, S_IVAR,"mean");
	addopcodepfield(newsym, S_IVAR,"var");
	return newsym;
      }
    if (!(strcmp(tptr->val,"iir")))
      {
	newsym = createopcode(tptr, nametable, S_AOPCODE);
	addopcodepfield(newsym, S_ASIG,"input");
	addopcodepfield(newsym, S_KSIG,"b0");
	return newsym;
      }
    if (!(strcmp(tptr->val,"iirt")))
      {
	newsym = createopcode(tptr, nametable, S_AOPCODE);
	addopcodepfield(newsym, S_ASIG,"input");
	addopcodepfield(newsym, S_TABLE,"a");
	addopcodepfield(newsym, S_TABLE,"b");
	addopcodevar(newsym, S_TABLE,"dline");
	addopcodevar(newsym, S_ASIG,"kcyc");
	return newsym;
      }
    if (!(strcmp(tptr->val,"ilinrand")))
      {
	newsym = createopcode(tptr, nametable, S_IOPCODE);
	addopcodepfield(newsym, S_IVAR,"p1");
	addopcodepfield(newsym, S_IVAR,"p2");
	return newsym;
      }
    if (!(strcmp(tptr->val,"int")))
      return oneargopcode(tptr, nametable);
    if (!(strcmp(tptr->val,"irand")))
      {
	newsym = createopcode(tptr, nametable, S_IOPCODE);
	addopcodepfield(newsym, S_IVAR,"p");
	return newsym;
      }
    return NULL;
  case 'j':
    return NULL;
  case 'k':
    if (!(strcmp(tptr->val,"kexpon")))
      {
	newsym = createopcode(tptr, nametable, S_KOPCODE);
	addopcodepfield(newsym, S_IVAR,"x1");
	addopcodepfield(newsym, S_IVAR,"dur1");
	addopcodepfield(newsym, S_IVAR,"x2");
	addopcodevar(newsym, S_KSIG,"t");
	addopcodevar(newsym, S_KSIG,"clp");
	addopcodevar(newsym, S_KSIG,"crp");
	addopcodevar(newsym, S_KSIG,"cdur");
	addopcodevar(newsym, S_KSIG,"ratio");
	addopcodevar(newsym, S_KSIG,"invcdur");
	addopcodevar(newsym, S_KSIG,"outT");
	addopcodevar(newsym, S_KSIG,"multK");
	addopcodevar(newsym, S_KSIG,"first");
	return newsym;
      }
    if (!(strcmp(tptr->val,"kexprand")))
      {
	newsym = createopcode(tptr, nametable, S_KOPCODE);
	addopcodepfield(newsym, S_KSIG,"p1");
	return newsym;
      }
    if (!(strcmp(tptr->val,"kgaussrand")))
      {
	newsym = createopcode(tptr, nametable, S_KOPCODE);
	addopcodepfield(newsym, S_KSIG,"mean");
	addopcodepfield(newsym, S_KSIG,"var");
	return newsym;
      }
    if (!(strcmp(tptr->val,"kline")))
      {	
	newsym = createopcode(tptr, nametable, S_KOPCODE);
	addopcodepfield(newsym, S_IVAR,"x1");
	addopcodepfield(newsym, S_IVAR,"dur1");
	addopcodepfield(newsym, S_IVAR,"x2");
	addopcodevar(newsym, S_KSIG,"t");
	addopcodevar(newsym, S_KSIG,"clp");
	addopcodevar(newsym, S_KSIG,"crp");
	addopcodevar(newsym, S_KSIG,"cdur");
	addopcodevar(newsym, S_KSIG,"mult");
	addopcodevar(newsym, S_KSIG,"outT");
	addopcodevar(newsym, S_KSIG,"addK");
	addopcodevar(newsym, S_KSIG,"first");
	return newsym;
      }
    if (!(strcmp(tptr->val,"klinrand")))
      {
	newsym = createopcode(tptr, nametable, S_KOPCODE);
	addopcodepfield(newsym, S_KSIG,"p1");
	addopcodepfield(newsym, S_KSIG,"p2");
	return newsym;
      }
    if (!(strcmp(tptr->val,"koscil")))
      {	
	newsym = createopcode(tptr, nametable, S_KOPCODE);
	addopcodepfield(newsym, S_TABLE,"t");
	addopcodepfield(newsym, S_KSIG,"freq");
	addopcodevar(newsym, S_ASIG,"iloops");
	addopcodevar(newsym, S_KSIG,"first");
	addopcodevar(newsym, S_ASIG,"pint");
	addopcodevar(newsym, S_ASIG,"pfrac");
	addopcodevar(newsym, S_ASIG,"kconst");
	return newsym;
      }
    if (!(strcmp(tptr->val,"kphasor")))
      {
	newsym = createopcode(tptr, nametable, S_KOPCODE);
	addopcodepfield(newsym, S_KSIG,"cps");
	addopcodevar(newsym, S_ASIG,"pint");
	addopcodevar(newsym, S_ASIG,"pfrac");
	addopcodevar(newsym, S_ASIG,"kcyc");
	return newsym;
      }
    if (!(strcmp(tptr->val,"kpoissonrand")))
      {
	newsym = createopcode(tptr, nametable, S_KOPCODE);
	addopcodepfield(newsym, S_KSIG,"p1");
	addopcodevar(newsym, S_KSIG,"state");
	return newsym;
      }
    if (!(strcmp(tptr->val,"krand")))
      {
	newsym = createopcode(tptr, nametable, S_KOPCODE);
	addopcodepfield(newsym, S_KSIG,"p");
	return newsym;
      }
    return NULL;
  case 'l':
    if (!(strcmp(tptr->val,"log")))
      return oneargopcode(tptr, nametable);
    if (!(strcmp(tptr->val,"log10")))
      return oneargopcode(tptr, nametable);
    if (!(strcmp(tptr->val,"lopass")))
      {
	newsym = createopcode(tptr, nametable, S_AOPCODE);
	addopcodepfield(newsym, S_ASIG,"input");
	addopcodepfield(newsym, S_KSIG,"cut");
	addopcodevar(newsym, S_ASIG,"kcyc");
	addopcodevar(newsym, S_ASIG,"ocut");
	addopcodevar(newsym, S_ASIG,"b0");
	addopcodevar(newsym, S_ASIG,"b1");
	addopcodevar(newsym, S_ASIG,"b2");
	addopcodevar(newsym, S_ASIG,"a1");
	addopcodevar(newsym, S_ASIG,"a2");
	addopcodevar(newsym, S_ASIG,"d1");
	addopcodevar(newsym, S_ASIG,"d2");
	return newsym;
      }
    if (!(strcmp(tptr->val,"loscil")))
      {
	newsym = createopcode(tptr, nametable, S_AOPCODE);
	addopcodepfield(newsym, S_TABLE,"t");
	addopcodepfield(newsym, S_ASIG,"freq");
	addopcodevar(newsym, S_ASIG,"kcyc");
	addopcodevar(newsym, S_ASIG,"lconst");
	addopcodevar(newsym, S_ASIG,"pint");
	addopcodevar(newsym, S_ASIG,"pfrac");
	addopcodevar(newsym, S_ASIG,"dint");
	addopcodevar(newsym, S_ASIG,"tstartint");
	addopcodevar(newsym, S_ASIG,"tendint");
	addopcodevar(newsym, S_ASIG,"rollover");
	addopcodevar(newsym, S_ASIG,"stamp");
	if (interp == INTERP_SINC)
	  addopcodevar(newsym, S_ASIG,"second");
	return newsym;
      }
    return NULL;
  case 'm':
    if (!(strcmp(tptr->val,"max")))
      {
	newsym = createopcode(tptr, nametable, S_OPCODE);
	addopcodepfield(newsym, S_XSIG,"x1");
	return newsym;
      }
    if (!(strcmp(tptr->val,"midicps")))
      return oneargopcode(tptr, nametable);
    if (!(strcmp(tptr->val,"midioct")))
      return oneargopcode(tptr, nametable);
    if (!(strcmp(tptr->val,"midipch")))
      return oneargopcode(tptr, nametable);
    if (!(strcmp(tptr->val,"min")))
      {
	newsym = createopcode(tptr, nametable, S_OPCODE);
	addopcodepfield(newsym, S_XSIG,"x1");
	return newsym;
      }
    return NULL;
  case 'n':
    return NULL;
  case 'o':
    if (!(strcmp(tptr->val,"octcps")))
      return oneargopcode(tptr, nametable);
    if (!(strcmp(tptr->val,"octmidi")))
      return oneargopcode(tptr, nametable);
    if (!(strcmp(tptr->val,"octpch")))
      return oneargopcode(tptr, nametable);
    if (!(strcmp(tptr->val,"oscil")))
      {	
	newsym = createopcode(tptr, nametable, S_AOPCODE);
	addopcodepfield(newsym, S_TABLE,"t");
	addopcodepfield(newsym, S_ASIG,"freq");
	addopcodevar(newsym, S_ASIG,"iloops");
	addopcodevar(newsym, S_ASIG,"kcyc");
	addopcodevar(newsym, S_ASIG,"pint");
	addopcodevar(newsym, S_ASIG,"pfrac");
	addopcodevar(newsym, S_ASIG,"kint");
	addopcodevar(newsym, S_ASIG,"kfrac");
	addopcodevar(newsym, S_ASIG,"fsign");
	if (interp == INTERP_SINC)
	  {
	    addopcodevar(newsym, S_ASIG,"sffl");
	    addopcodevar(newsym, S_ASIG,"sfui");
	    addopcodevar(newsym, S_ASIG,"osincr");
	  }
	return newsym;
      }
    return NULL;
  case 'p':
    if (!(strcmp(tptr->val,"pchcps")))
     return oneargopcode(tptr, nametable);
    if (!(strcmp(tptr->val,"pchmidi")))
     return oneargopcode(tptr, nametable);
    if (!(strcmp(tptr->val,"pchoct")))
     return oneargopcode(tptr, nametable);
    if (!(strcmp(tptr->val,"pluck")))
      {	
	newsym = createopcode(tptr, nametable, S_AOPCODE);
	addopcodepfield(newsym, S_ASIG,"cps");
	addopcodepfield(newsym, S_IVAR,"buflen");
	addopcodepfield(newsym, S_TABLE,"init");
	addopcodepfield(newsym, S_KSIG,"atten");
	addopcodepfield(newsym, S_KSIG,"smoothrate");
	addopcodevar(newsym, S_ASIG,"pint");
	addopcodevar(newsym, S_ASIG,"pfrac");
	addopcodevar(newsym, S_ASIG,"oconst");
	addopcodevar(newsym, S_ASIG,"sc");
	addopcodevar(newsym, S_ASIG,"first");
	addopcodevar(newsym, S_TABLE,"t");
	addopcodevar(newsym, S_TABLE,"ts");
	return newsym;
      }
    if (!(strcmp(tptr->val,"port")))
      {
	newsym = createopcode(tptr, nametable, S_KOPCODE);
	addopcodepfield(newsym, S_KSIG,"ctrl");
	addopcodepfield(newsym, S_KSIG,"htime");
	addopcodevar(newsym, S_KSIG,"new");
	addopcodevar(newsym, S_KSIG,"curr");
	addopcodevar(newsym, S_KSIG,"int");
	addopcodevar(newsym, S_KSIG,"ohtime");
	addopcodevar(newsym, S_KSIG,"sl");
	addopcodevar(newsym, S_KSIG,"first");
	addopcodevar(newsym, S_KSIG,"done");
	return newsym;
      }
    if (!(strcmp(tptr->val,"pow")))
      {
	newsym = createopcode(tptr, nametable, S_OPCODE);
	addopcodepfield(newsym, S_XSIG,"x");
	addopcodepfield(newsym, S_XSIG,"y");
	return newsym;
      }
    return NULL;
  case 'q':
    return NULL;
  case 'r':
    if (!(strcmp(tptr->val,"reverb")))
      {
	newsym = createopcode(tptr, nametable, S_AOPCODE);
	addopcodepfield(newsym, S_ASIG,"x");
	addopcodepfield(newsym, S_IVAR,"f0");
	addopcodevar(newsym, S_TABLE,"ap1");
	addopcodevar(newsym, S_TABLE,"ap2");
	addopcodevar(newsym, S_TABLE,"dline0_0");
	addopcodevar(newsym, S_TABLE,"dline0_1");
	addopcodevar(newsym, S_TABLE,"dline0_2");
	addopcodevar(newsym, S_TABLE,"dline0_3");
	addopcodevar(newsym, S_IVAR,"g0_0");
	addopcodevar(newsym, S_IVAR,"g0_1");
	addopcodevar(newsym, S_IVAR,"g0_2");
	addopcodevar(newsym, S_IVAR,"g0_3");
	return newsym;
      }
    if (!(strcmp(tptr->val,"rms")))
      {
	newsym = createopcode(tptr, nametable, S_SOPCODE);
	addopcodepfield(newsym, S_ASIG,"x");
	addopcodevar(newsym, S_ASIG,"kcyc");
	addopcodevar(newsym, S_ASIG,"scale");
	addopcodevar(newsym, S_TABLE,"buffer");
	return newsym;
      }
    return NULL;
  case 's':
    if (!(strcmp(tptr->val,"samphold")))
      {
	newsym = createopcode(tptr, nametable, S_OPCODE);
	addopcodepfield(newsym, S_XSIG,"input");
	addopcodepfield(newsym, S_KSIG,"gate");
	addopcodevar(newsym, S_XSIG,"lpv");
	return newsym;
      }
    if (!(strcmp(tptr->val,"sblock")))
      {
	newsym = createopcode(tptr, nametable, S_SOPCODE);
	addopcodepfield(newsym, S_ASIG,"x");
	addopcodepfield(newsym, S_TABLE,"t");
	addopcodevar(newsym, S_ASIG,"idx");
	addopcodevar(newsym, S_ASIG,"kcyc");
	return newsym;
      }
    if (!(strcmp(tptr->val,"settempo")))
      {
	newsym = createopcode(tptr, nametable, S_KOPCODE);
	addopcodepfield(newsym, S_KSIG,"x");
	return newsym;
      }
    if (!(strcmp(tptr->val,"settune")))
      {
	newsym = createopcode(tptr, nametable, S_OPCODE);
	addopcodepfield(newsym, S_XSIG,"x");
	return newsym;
      }
    if (!(strcmp(tptr->val,"sgn")))
      return oneargopcode(tptr, nametable);
    if (!(strcmp(tptr->val,"sin")))
      return oneargopcode(tptr, nametable);
    if (!(strcmp(tptr->val,"spatialize")))
      {
	newsym = createopcode(tptr, nametable, S_OPCODE);
	addopcodepfield(newsym, S_ASIG,"x");
	addopcodepfield(newsym, S_KSIG,"azimuth");
	addopcodepfield(newsym, S_KSIG,"elevation");
	addopcodepfield(newsym, S_KSIG,"distance");
	addopcodevar(newsym, S_ASIG,"kcyc");
	addopcodevar(newsym, S_ASIG,"oaz");
	addopcodevar(newsym, S_ASIG,"oel");

	addopcodevar(newsym, S_ASIG,"odis");
	addopcodevar(newsym, S_ASIG,"dis_b0");
	addopcodevar(newsym, S_ASIG,"dis_b1");
	addopcodevar(newsym, S_ASIG,"dis_b2");
	addopcodevar(newsym, S_ASIG,"dis_a1");
	addopcodevar(newsym, S_ASIG,"dis_a2");
	addopcodevar(newsym, S_ASIG,"dis_d1");
	addopcodevar(newsym, S_ASIG,"dis_d2");

	addopcodevar(newsym, S_ASIG,"t0");
	addopcodevar(newsym, S_ASIG,"i0");
	addopcodevar(newsym, S_ASIG,"t1");
	addopcodevar(newsym, S_ASIG,"i1");
	addopcodevar(newsym, S_ASIG,"t2");
	addopcodevar(newsym, S_ASIG,"i2");
	addopcodevar(newsym, S_ASIG,"t3");
	addopcodevar(newsym, S_ASIG,"i3");
	addopcodevar(newsym, S_ASIG,"t4");
	addopcodevar(newsym, S_ASIG,"i4");
	addopcodevar(newsym, S_ASIG,"t5");
	addopcodevar(newsym, S_ASIG,"i5");
	addopcodevar(newsym, S_ASIG,"t6");
	addopcodevar(newsym, S_ASIG,"i6");

	addopcodevar(newsym, S_TABLE,"d0");
	addopcodevar(newsym, S_TABLE,"d1");
	addopcodevar(newsym, S_TABLE,"d2");
	addopcodevar(newsym, S_TABLE,"d3");
	addopcodevar(newsym, S_TABLE,"d4");
	addopcodevar(newsym, S_TABLE,"d5");
	addopcodevar(newsym, S_TABLE,"d6");
	addopcodevar(newsym, S_TABLE,"d7");

	addopcodevar(newsym, S_ASIG,"az_b0L");
	addopcodevar(newsym, S_ASIG,"az_b0R");
	addopcodevar(newsym, S_ASIG,"az_b1L");
	addopcodevar(newsym, S_ASIG,"az_b1R");
	addopcodevar(newsym, S_ASIG,"az_a1");
	addopcodevar(newsym, S_ASIG,"az_d1L");
	addopcodevar(newsym, S_ASIG,"az_d1R");

	return newsym;
      }
    if (!(strcmp(tptr->val,"speedt")))
      {
	newsym = createopcode(tptr, nametable, S_IOPCODE);
	addopcodepfield(newsym, S_TABLE,"in");
	addopcodepfield(newsym, S_TABLE,"out");
	addopcodepfield(newsym, S_IVAR,"factor");
	return newsym;
      }
    if (!(strcmp(tptr->val,"sqrt")))
      return oneargopcode(tptr, nametable);
    return NULL;
  case 't':
    if (!(strcmp(tptr->val,"tableread")))
      {
	newsym = createopcode(tptr, nametable, S_OPCODE);
	addopcodepfield(newsym, S_TABLE,"t");
	addopcodepfield(newsym, S_XSIG,"index");
	return newsym;
      }
    if (!(strcmp(tptr->val,"tablewrite")))
      {
	newsym = createopcode(tptr, nametable, S_OPCODE);
	addopcodepfield(newsym, S_TABLE,"t");
	addopcodepfield(newsym, S_XSIG,"index");
	addopcodepfield(newsym, S_XSIG,"val");
	return newsym;
      }
    return NULL;
  case 'u':
    if (!(strcmp(tptr->val,"upsamp")))
      {
	newsym = createopcode(tptr, nametable, S_AOPCODE);
	addopcodepfield(newsym, S_ASIG,"input");
	addopcodevar(newsym, S_TABLE,"buffer");
	addopcodevar(newsym, S_ASIG,"kcyc");
	return newsym;
      }
    return NULL;
  case 'v':
    return NULL;
  case 'w':
    return NULL;
  case 'x':
    return NULL;
  case 'y':
    return NULL;
  case 'z':
    return NULL;
 }
 return NULL;
}

/*********************************************************/
/*      checks that extra params are scalar non-tables   */
/*********************************************************/

void extracheckargs(tnode * tcall, tnode * tptr)

{
  tablecheck(tptr);
  if (truewidth(tptr->width) != 1)
    {
      printf("Error: Opcode (varargs) call width mismatch.\n");
      showerrorplace(tcall->optr->down->linenum, 
		     tcall->optr->down->filename);
    }
}

/*********************************************************/
/*      identifies opcodes with variable arguments       */
/*********************************************************/

int coreopcodeargs(tnode * tcall, tnode * textra)


{
  tnode * tptr;
  int i,j;
  char name[32];

  if (tcall->extra != NULL)   /* earlier call created variables, */
    return 1;                 /* did arg count, */

  if ((!(strcmp(tcall->val,"max")))||(!(strcmp(tcall->val,"min"))))
    {
      tptr = textra;
      i = 2;
      while (tptr != NULL)
	{
	  if (tptr->ttype == S_EXPR)
	    {
	      extracheckargs(tcall, tptr);
	      sprintf(name,"x%i",i);
	      addextraparam(tcall, S_XSIG, dupval(name));
	      i++;
	    }
	  tptr = tptr->next;
	}
      return 1;
    }
     
  if ((!(strcmp(tcall->val,"gettune")))||(!(strcmp(tcall->val,"gettempo"))))
    {
      tptr = textra;
      i = 1;
      while (tptr != NULL)
	{
	  if (tptr->ttype == S_EXPR)
	    {
	      if (i>1)
		{
		  printf("Error: Too many parameters.\n");
		  showerrorplace(tcall->optr->down->linenum,
				 tcall->optr->down->filename);
		}
	      extracheckargs(tcall, tptr);
	      sprintf(name,"dummy");
	      addextraparam(tcall, S_XSIG, dupval(name));
	      i++;
	    }
	  tptr = tptr->next;
	}
      return 1;
    }
     
  if ((!(strcmp(tcall->val,"firt")))||(!(strcmp(tcall->val,"iirt"))))
    {
      tptr = textra;
      i = 1;
      while (tptr != NULL)
	{
	  if (tptr->ttype == S_EXPR)
	    {
	      if (i>1)
		{
		  printf("Error: Too many parameters.\n");
		  showerrorplace(tcall->optr->down->linenum,
				 tcall->optr->down->filename);
		}
	      extracheckargs(tcall, tptr);
	      sprintf(name,"order");
	      addextraparam(tcall, S_KSIG, dupval(name));
	      i++;
	    }
	  tptr = tptr->next;
	}
      return 1;
    }
     
  if ((!(strcmp(tcall->val,"gain")))||(!(strcmp(tcall->val,"balance")))
      ||(!(strcmp(tcall->val,"rms"))))
    {
      tptr = textra;
      i = 1;
      while (tptr != NULL)
	{
	  if (tptr->ttype == S_EXPR)
	    {
	      if (i>1)
		{
		  printf("Error: Too many parameters.\n");
		  showerrorplace(tcall->optr->down->linenum,
				 tcall->optr->down->filename);
		}
	      extracheckargs(tcall, tptr);
	      sprintf(name,"length");
	      addextraparam(tcall, S_IVAR, dupval(name));
	      i++;
	    }
	  tptr = tptr->next;
	}
      return 1;
    }
     
  if ((!(strcmp(tcall->val,"upsamp")))||(!(strcmp(tcall->val,"downsamp"))))
    {
      tptr = textra;
      i = 1;
      while (tptr != NULL)
	{
	  if (tptr->ttype == S_EXPR)
	    {
	      if (i>1)
		{
		  printf("Error: Too many parameters.\n");
		  showerrorplace(tcall->optr->down->linenum,
				 tcall->optr->down->filename);
		}
	      if (tptr->vartype != TABLETYPE)
		{
		  printf("Error: Parameter must be a table.\n");
		  showerrorplace(tcall->optr->down->linenum,
				 tcall->optr->down->filename);
		}
	      sprintf(name,"win");
	      addextraparam(tcall, S_TABLE, dupval(name));
	      i++;
	    }
	  tptr = tptr->next;
	}
      return 1;
    }
     
  if ((!(strcmp(tcall->val,"oscil")))||(!(strcmp(tcall->val,"koscil"))) )
    {
      tptr = textra;
      i = 1;
      while (tptr != NULL)
	{
	  if (tptr->ttype == S_EXPR)
	    {
	      if (i>1)
		{
		  printf("Error: Too many parameters.\n");
		  showerrorplace(tcall->optr->down->linenum,
				 tcall->optr->down->filename);
		}
	      extracheckargs(tcall, tptr);
	      sprintf(name,"loops");
	      addextraparam(tcall, S_IVAR, dupval(name));
	      i++;
	    }
	  tptr = tptr->next;
	}
      return 1;
    }
     
  if (!(strcmp(tcall->val,"loscil")))
    {
      tptr = textra;
      i = 1;
      while (tptr != NULL)
	{
	  if (tptr->ttype == S_EXPR)
	    {
	      if (i>3)
		{
		  printf("Error: Too many parameters.\n");
		  showerrorplace(tcall->optr->down->linenum,
				 tcall->optr->down->filename);
		}
	      extracheckargs(tcall, tptr);
	      switch (i) {
	      case 1:
		sprintf(name,"basefreq");
		break;
	      case 2:
		sprintf(name,"loopstart");
		break;
	      case 3:
		sprintf(name,"loopend");
		break;
	      default:
		internalerror("corevars.c","case default");
	      }
	      addextraparam(tcall, S_IVAR, dupval(name));
	      i++;
	    }
	  tptr = tptr->next;
	}
      return 1;
    }
     
  if (!(strcmp(tcall->val,"fracdelay")))
    {
      tptr = textra;
      i = 1;
      while (tptr != NULL)
	{
	  if (tptr->ttype == S_EXPR)
	    {
	      if (i>2)
		{
		  printf("Error: Too many parameters.\n");
		  showerrorplace(tcall->optr->down->linenum,
				 tcall->optr->down->filename);
		}
	      extracheckargs(tcall, tptr);
	      switch (i) {
	      case 1:
		addextraparam(tcall, S_XSIG, "p1");
		break;
	      case 2:
		addextraparam(tcall, S_XSIG, "p2");
		break;
	      default:
		internalerror("corevars.c","case default");
	      }
	      i++;
	    }
	  tptr = tptr->next;
	}
      return 1;
    }
     
  if ((!(strcmp(tcall->val,"fft")))||(!(strcmp(tcall->val,"ifft"))))
    {
      tptr = textra;
      i = 1;
      while (tptr != NULL)
	{
	  if (tptr->ttype == S_EXPR)
	    {
	      switch (i) {
	      case 1:
		extracheckargs(tcall, tptr);
		addextraparam(tcall, S_IVAR, "len");
		break;
	      case 2:
		extracheckargs(tcall, tptr);
		addextraparam(tcall, S_IVAR, "shift");
		break;
	      case 3:
		extracheckargs(tcall, tptr);
		addextraparam(tcall, S_IVAR, "size");
		break;
	      case 4:	      
		if (tptr->vartype != TABLETYPE)
		  {
		    printf("Error: Parameter must be a table.\n");
		    showerrorplace(tcall->optr->down->linenum,
				   tcall->optr->down->filename);
		  }
		addextraparam(tcall, S_TABLE, "win");
		break;
	      default:
		{
		  printf("Error: Too many parameters.\n");
		  showerrorplace(tcall->optr->down->linenum,
				 tcall->optr->down->filename);
		}
	      }
	      i++;
	    }
	  tptr = tptr->next;
	}
      return 1;
    }
     
  if ((!(strcmp(tcall->val,"aline")))||(!(strcmp(tcall->val,"kline")))||
      (!(strcmp(tcall->val,"aexpon")))||(!(strcmp(tcall->val,"kexpon"))))
    {
      tptr = textra;
      i = 2; j = 0;
      while (tptr != NULL)
	{
	  if (tptr->ttype == S_EXPR)
	    {
	      extracheckargs(tcall, tptr);
	      switch (j) {
	      case 0:
		j = 1;
		sprintf(name,"dur%i",i);
		break;
	      case 1:
		j = 0; i++;
		sprintf(name,"x%i",i);
		break;
	      default:
		internalerror("corevars.c","case default");
	      }
	      addextraparam(tcall, S_IVAR, dupval(name));
	    }
	  tptr = tptr->next;
	}
      if (j == 1)
	{
	  printf("Error: Even number of parameters.\n");
	  showerrorplace(tcall->optr->down->linenum,
			 tcall->optr->down->filename);
	}
      return 1;
    }

  if (!(strcmp(tcall->val,"fir")))
    {
      tptr = textra;
      i = 1;
      while (tptr != NULL)
        {
          if (tptr->ttype == S_EXPR)
            {
	      extracheckargs(tcall, tptr);
              sprintf(name,"b%i",i);
              addextraparam(tcall, S_KSIG, dupval(name));
              i++;
            }
          tptr = tptr->next;
        }
      j = 1;
      while (j<i)
        {
          sprintf(name,"z%i",j);
          addvsym(&(tcall->optr->sptr->defnode->sptr),name,K_NORMAL);
          tcall->optr->sptr->defnode->sptr->width = 1;
          tcall->optr->sptr->defnode->sptr->rate = ARATETYPE;
          j++;
        }
      return 1;
    }

  if (!(strcmp(tcall->val,"iir")))
    {
      tptr = textra;
      i = 1;
      j = 0;
      while (tptr != NULL)
	{
	  if (tptr->ttype == S_EXPR)
	    {
	      extracheckargs(tcall, tptr);
	      if ((i % 2) == 1)
		{
		  sprintf(name,"a%i",(i/2)+1);
		  j++;
		}
	      else
		sprintf(name,"b%i",(i/2));
	      addextraparam(tcall, S_KSIG, dupval(name));
	      i++;
	    }
	  tptr = tptr->next;
	}
      while (j>0)
	{
	  sprintf(name,"d%i",j);
	  addvsym(&(tcall->optr->sptr->defnode->sptr),name,K_NORMAL);
	  tcall->optr->sptr->defnode->sptr->width = 1;
	  tcall->optr->sptr->defnode->sptr->rate = ARATETYPE;
	  j--;
	}
      return 1;
    }
   
  if (!(strcmp(tcall->val,"reverb")))
    {
      tptr = textra;
      i = 1; j = 0;
      while (tptr != NULL)
	{
	  if (tptr->ttype == S_EXPR)
	    {
	      extracheckargs(tcall, tptr);
	      if ((i % 2) == 1)
		{
		  sprintf(name,"r%i",(i/2));
		}
	      else
		{
		  sprintf(name,"f%i",(i/2));
		  j++;
		}
	      addextraparam(tcall, S_IVAR, dupval(name));
	      i++;
	    }
	  tptr = tptr->next;
	}
      if ((i % 2) == 1)
	{
	  printf("Error: F without an R pair.\n");
	  showerrorplace(tcall->optr->down->linenum,
			 tcall->optr->down->filename);
	}
      if (i >= 2)
	{
	  sprintf(name,"d2_0");
	  addvsym(&(tcall->optr->sptr->defnode->sptr),name,K_NORMAL);
	  tcall->optr->sptr->defnode->sptr->width = 1;
	  tcall->optr->sptr->defnode->sptr->rate = IRATETYPE;

	  sprintf(name,"d1_0");
	  addvsym(&(tcall->optr->sptr->defnode->sptr),name,K_NORMAL);
	  tcall->optr->sptr->defnode->sptr->width = 1;
	  tcall->optr->sptr->defnode->sptr->rate = IRATETYPE;

	  sprintf(name,"b0_0");
	  addvsym(&(tcall->optr->sptr->defnode->sptr),name,K_NORMAL);
	  tcall->optr->sptr->defnode->sptr->width = 1;
	  tcall->optr->sptr->defnode->sptr->rate = IRATETYPE;

	  sprintf(name,"b1_0");
	  addvsym(&(tcall->optr->sptr->defnode->sptr),name,K_NORMAL);
	  tcall->optr->sptr->defnode->sptr->width = 1;
	  tcall->optr->sptr->defnode->sptr->rate = IRATETYPE;

	  sprintf(name,"b2_0");
	  addvsym(&(tcall->optr->sptr->defnode->sptr),name,K_NORMAL);
	  tcall->optr->sptr->defnode->sptr->width = 1;
	  tcall->optr->sptr->defnode->sptr->rate = IRATETYPE;

	  sprintf(name,"a1_0");
	  addvsym(&(tcall->optr->sptr->defnode->sptr),name,K_NORMAL);
	  tcall->optr->sptr->defnode->sptr->width = 1;
	  tcall->optr->sptr->defnode->sptr->rate = IRATETYPE;

	  sprintf(name,"a2_0");
	  addvsym(&(tcall->optr->sptr->defnode->sptr),name,K_NORMAL);
	  tcall->optr->sptr->defnode->sptr->width = 1;
	  tcall->optr->sptr->defnode->sptr->rate = IRATETYPE;

	}
      while (j>0)
	{
	  sprintf(name,"dline%i_0",j);
	  addvsym(&(tcall->optr->sptr->defnode->sptr),name,K_NORMAL);
	  tcall->optr->sptr->defnode->sptr->width = 1;
	  tcall->optr->sptr->defnode->sptr->rate = IRATETYPE;
	  tcall->optr->sptr->defnode->sptr->vartype = TABLETYPE;
	  tcall->optr->sptr->defnode->sptr->kind = K_INTERNAL;

	  sprintf(name,"dline%i_1",j);
	  addvsym(&(tcall->optr->sptr->defnode->sptr),name,K_NORMAL);
	  tcall->optr->sptr->defnode->sptr->width = 1;
	  tcall->optr->sptr->defnode->sptr->rate = IRATETYPE;
	  tcall->optr->sptr->defnode->sptr->vartype = TABLETYPE;
	  tcall->optr->sptr->defnode->sptr->kind = K_INTERNAL;

	  sprintf(name,"dline%i_2",j);
	  addvsym(&(tcall->optr->sptr->defnode->sptr),name,K_NORMAL);
	  tcall->optr->sptr->defnode->sptr->width = 1;
	  tcall->optr->sptr->defnode->sptr->rate = IRATETYPE;
	  tcall->optr->sptr->defnode->sptr->vartype = TABLETYPE;
	  tcall->optr->sptr->defnode->sptr->kind = K_INTERNAL;

	  sprintf(name,"dline%i_3",j);
	  addvsym(&(tcall->optr->sptr->defnode->sptr),name,K_NORMAL);
	  tcall->optr->sptr->defnode->sptr->width = 1;
	  tcall->optr->sptr->defnode->sptr->rate = IRATETYPE;
	  tcall->optr->sptr->defnode->sptr->vartype = TABLETYPE;
	  tcall->optr->sptr->defnode->sptr->kind = K_INTERNAL;

	  sprintf(name,"g%i_0",j);
	  addvsym(&(tcall->optr->sptr->defnode->sptr),name,K_NORMAL);
	  tcall->optr->sptr->defnode->sptr->width = 1;
	  tcall->optr->sptr->defnode->sptr->rate = IRATETYPE;

	  sprintf(name,"g%i_1",j);
	  addvsym(&(tcall->optr->sptr->defnode->sptr),name,K_NORMAL);
	  tcall->optr->sptr->defnode->sptr->width = 1;
	  tcall->optr->sptr->defnode->sptr->rate = IRATETYPE;

	  sprintf(name,"g%i_2",j);
	  addvsym(&(tcall->optr->sptr->defnode->sptr),name,K_NORMAL);
	  tcall->optr->sptr->defnode->sptr->width = 1;
	  tcall->optr->sptr->defnode->sptr->rate = IRATETYPE;

	  sprintf(name,"g%i_3",j);
	  addvsym(&(tcall->optr->sptr->defnode->sptr),name,K_NORMAL);
	  tcall->optr->sptr->defnode->sptr->width = 1;
	  tcall->optr->sptr->defnode->sptr->rate = IRATETYPE;

	  sprintf(name,"d2_%i",j);
	  addvsym(&(tcall->optr->sptr->defnode->sptr),name,K_NORMAL);
	  tcall->optr->sptr->defnode->sptr->width = 1;
	  tcall->optr->sptr->defnode->sptr->rate = IRATETYPE;

	  sprintf(name,"d1_%i",j);
	  addvsym(&(tcall->optr->sptr->defnode->sptr),name,K_NORMAL);
	  tcall->optr->sptr->defnode->sptr->width = 1;
	  tcall->optr->sptr->defnode->sptr->rate = IRATETYPE;

	  sprintf(name,"b0_%i",j);
	  addvsym(&(tcall->optr->sptr->defnode->sptr),name,K_NORMAL);
	  tcall->optr->sptr->defnode->sptr->width = 1;
	  tcall->optr->sptr->defnode->sptr->rate = IRATETYPE;

	  sprintf(name,"b1_%i",j);
	  addvsym(&(tcall->optr->sptr->defnode->sptr),name,K_NORMAL);
	  tcall->optr->sptr->defnode->sptr->width = 1;
	  tcall->optr->sptr->defnode->sptr->rate = IRATETYPE;

	  sprintf(name,"b2_%i",j);
	  addvsym(&(tcall->optr->sptr->defnode->sptr),name,K_NORMAL);
	  tcall->optr->sptr->defnode->sptr->width = 1;
	  tcall->optr->sptr->defnode->sptr->rate = IRATETYPE;

	  sprintf(name,"a1_%i",j);
	  addvsym(&(tcall->optr->sptr->defnode->sptr),name,K_NORMAL);
	  tcall->optr->sptr->defnode->sptr->width = 1;
	  tcall->optr->sptr->defnode->sptr->rate = IRATETYPE;

	  sprintf(name,"a2_%i",j);
	  addvsym(&(tcall->optr->sptr->defnode->sptr),name,K_NORMAL);
	  tcall->optr->sptr->defnode->sptr->width = 1;
	  tcall->optr->sptr->defnode->sptr->rate = IRATETYPE;


	  j--;
	}
      return 1;
    }
   
  return 0;
     
}


/****************************************************************/
/*       add variable argument structures to opcode calls       */
/****************************************************************/

void coreopcodevarargs(tnode * tcall)

{
  tnode * tptr;
  tnode * dptr;

  /* tptr holds actual arguments */

  switch(tcall->ttype) {
  case S_OPCALL:
    tptr = tcall->optr->down->next->next->down;
    break;
  case S_OPARRAYCALL:
    tptr = tcall->optr->down->next->next->next->next->next->down;
    break;
  default:
    return;
  }

  /* dptr holds formal arguments */

  dptr = tcall->sptr->defnode->down->next->next->next->down;
  
  if ((dptr != NULL) && (tptr == NULL))
    {
      printf("Error: Opcode call argument mismatch (%s).\n",
	     tcall->optr->down->val);
      showerrorplace(tcall->optr->down->linenum,
		     tcall->optr->down->filename);
    }

  while (dptr != NULL)
    {
      if (dptr->ttype == S_PARAMDECL)
	{
	  if (((tptr->vartype != TABLETYPE) && (dptr->vartype == TABLETYPE))||
	      ((tptr->vartype == TABLETYPE) && (dptr->vartype != TABLETYPE))||
	      (tptr->vartype == TMAPTYPE))
	    {
	      printf("Error: Opcode table parameter mismatch.\n");
	      showerrorplace(tcall->optr->down->linenum,
			     tcall->optr->down->filename);
	    }
	}
      dptr = dptr->next;
      if (tptr == NULL)
	{
	  printf("Error: Opcode call argument mismatch (%s).\n",
		 tcall->optr->down->val);
	  showerrorplace(tcall->optr->down->linenum,
			 tcall->optr->down->filename);
	}
      tptr = tptr->next;
    }

  if ((tptr != NULL) && (!coreopcodeargs(tcall,tptr)))
    {
      printf("Error: Opcode call argument mismatch (%s).\n",
	     tcall->optr->down->val);
      showerrorplace(tcall->optr->down->linenum,
		     tcall->optr->down->filename);
    }

}





