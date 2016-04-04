
/*
#    Sfront, a SAOL to C translator    
#    This file: Inlined version of selected core opcodes
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

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*       coreopcodecaninline() and coreopcodedoinline()         */
/*                                                              */
/* These top-level functions are called in blocktree.c,         */
/* writeop.c, and writepre.c to detect and implement opcall     */
/* inlining. Oparrays aren't inlined presently (see below).     */
/*                                                              */
/*______________________________________________________________*/

extern int coreopcodetabletype(tnode *);
extern int coretablereadinline(tnode *);

/*********************************************************/
/*   returns 1 if constant-input opcode can be inlined   */
/*                                                       */
/* if tptr->ttype is S_EXPR of opcall, pass this routine */
/*        tptr->down. Do not pass it tptr->optr.         */
/*********************************************************/

int coreopcodecaninline(tnode * ident)

{

  if (isocheck)    /* bounds-checking issues */
    return 0;

  switch (ident->val[0]) {
  case 'a':
    if (!(strcmp(ident->val,"abs")))
      return 1;
    if (!(strcmp(ident->val,"acos")))
      return 1;
    if (!(strcmp(ident->val,"aexpon")))
      return 0;
    if (!(strcmp(ident->val,"aexprand")))
      return 1;
    if (!(strcmp(ident->val,"agaussrand")))
      return 1;
    if (!(strcmp(ident->val,"aline")))
      return 0;
    if (!(strcmp(ident->val,"alinrand")))
      return 0;
    if (!(strcmp(ident->val,"allpass")))
      return 0;
    if (!(strcmp(ident->val,"ampdb")))
      return 1;
    if (!(strcmp(ident->val,"aphasor")))
      return 0;
    if (!(strcmp(ident->val,"apoissonrand")))
      return 0;
    if (!(strcmp(ident->val,"arand")))
      return 1;
    if (!(strcmp(ident->val,"asin")))
      return 1;
    if (!(strcmp(ident->val,"atan")))
      return 1;
    return 0;
  case 'b':
    if (!(strcmp(ident->val,"balance")))
      return 0;
    if (!(strcmp(ident->val,"bandpass")))
      return 0;
    if (!(strcmp(ident->val,"bandstop")))
      return 0;
    if (!(strcmp(ident->val,"biquad")))
      return 0;
    if (!(strcmp(ident->val,"buzz")))
      return 0;
    return 0;
  case 'c':
    if (!(strcmp(ident->val,"ceil")))
      return 1;
    if (!(strcmp(ident->val,"chorus")))
      return 0;
    if (!(strcmp(ident->val,"comb")))
      return 0;
    if (!(strcmp(ident->val,"compressor")))
      return 0;
    if (!(strcmp(ident->val,"cos")))
      return 1;
    if (!(strcmp(ident->val,"cpsmidi")))
      return 1;
    if (!(strcmp(ident->val,"cpsoct")))
      return 1;
    if (!(strcmp(ident->val,"cpspch")))
      return 0;
    return 0;
  case 'd':
    if (!(strcmp(ident->val,"dbamp")))
      return 1;
    if (!(strcmp(ident->val,"decimate")))
      return 0;
    if (!(strcmp(ident->val,"delay")))
      return 0;
    if (!(strcmp(ident->val,"delay1")))
      return 0;
    if (!(strcmp(ident->val,"doscil")))
      return 0;
    if (!(strcmp(ident->val,"downsamp")))
      return 0;
    return 0;
  case 'e':
    if (!(strcmp(ident->val,"exp")))
      return 1;
    return 0;
  case 'f':
    if (!(strcmp(ident->val,"fft")))
      return 0;
    if (!(strcmp(ident->val,"fir")))
      return 0;
    if (!(strcmp(ident->val,"firt")))
      return 0;
    if (!(strcmp(ident->val,"flange")))
      return 0;
    if (!(strcmp(ident->val,"floor")))
      return 1;
    if (!(strcmp(ident->val,"frac")))
      return 1;
    if (!(strcmp(ident->val,"fracdelay")))
      return 0;
    if (!(strcmp(ident->val,"ftbasecps")))
      return coreopcodetabletype(ident);
    if (!(strcmp(ident->val,"ftlen")))
      return coreopcodetabletype(ident);
    if (!(strcmp(ident->val,"ftloop")))
      return coreopcodetabletype(ident);
    if (!(strcmp(ident->val,"ftloopend")))
      return coreopcodetabletype(ident);
    if (!(strcmp(ident->val,"ftsetbase")))
      return 0;
    if (!(strcmp(ident->val,"ftsetend")))
      return 0;
    if (!(strcmp(ident->val,"ftsetloop")))
      return 0;
    if (!(strcmp(ident->val,"ftsetsr")))
      return 0;
    if (!(strcmp(ident->val,"ftsr")))
      return coreopcodetabletype(ident);
    return 0;
  case 'g':
    if (!(strcmp(ident->val,"gain")))
      return 0;
    if (!(strcmp(ident->val,"gettempo")))
      return 1;
    if (!(strcmp(ident->val,"gettune")))
      return 1;
    if (!(strcmp(ident->val,"grain")))
      return 0;
    return 0;
  case 'h':
    if (!(strcmp(ident->val,"hipass")))
      return 0;
    return 0;
  case 'i':
    if (!(strcmp(ident->val,"iexprand")))
      return 1;
    if (!(strcmp(ident->val,"ifft")))
      return 0;
    if (!(strcmp(ident->val,"igaussrand")))
      return 1;
    if (!(strcmp(ident->val,"iir")))
      return 0;
    if (!(strcmp(ident->val,"iirt")))
      return 0;
    if (!(strcmp(ident->val,"ilinrand")))
      return 0;
    if (!(strcmp(ident->val,"int")))
      return 1;
    if (!(strcmp(ident->val,"irand")))
      return 1;
    return 0;
  case 'j':
    return 0;
  case 'k':
    if (!(strcmp(ident->val,"kexpon")))
      return 0;
    if (!(strcmp(ident->val,"kexprand")))
      return 1;
    if (!(strcmp(ident->val,"kgaussrand")))
      return 1;
    if (!(strcmp(ident->val,"kline")))
      return 0;
    if (!(strcmp(ident->val,"klinrand")))
      return 0;
    if (!(strcmp(ident->val,"koscil")))
      return 0;
    if (!(strcmp(ident->val,"kphasor")))
      return 0;
    if (!(strcmp(ident->val,"kpoissonrand")))
      return 0;
    if (!(strcmp(ident->val,"krand")))
      return 1;
    return 0;
  case 'l':
    if (!(strcmp(ident->val,"log")))
      return 1;
    if (!(strcmp(ident->val,"log10")))
      return 1;
    if (!(strcmp(ident->val,"lopass")))
      return 0;
    if (!(strcmp(ident->val,"loscil")))
      return 0;
    return 0;
  case 'm':
    if (!(strcmp(ident->val,"max")))
      return 0;
    if (!(strcmp(ident->val,"midicps")))
      return 1;
    if (!(strcmp(ident->val,"midioct")))
      return 1;
    if (!(strcmp(ident->val,"midipch")))
      return 0;
    if (!(strcmp(ident->val,"min")))
      return 0;
    return 0;
  case 'n':
    return 0;
  case 'o':
    if (!(strcmp(ident->val,"octcps")))
      return 1;
    if (!(strcmp(ident->val,"octmidi")))
      return 1;
    if (!(strcmp(ident->val,"octpch")))
      return 0;
    if (!(strcmp(ident->val,"oscil")))
      return 0;
    return 0;
  case 'p':
    if (!(strcmp(ident->val,"pchcps")))
      return 0;
    if (!(strcmp(ident->val,"pchmidi")))
      return 0;
    if (!(strcmp(ident->val,"pchoct")))
      return 0;
    if (!(strcmp(ident->val,"pluck")))
      return 0;
    if (!(strcmp(ident->val,"port")))
      return 0;
    if (!(strcmp(ident->val,"pow")))
      return 1;
    return 0;
  case 'q':
    return 0;
  case 'r':
    if (!(strcmp(ident->val,"reverb")))
      return 0;
    if (!(strcmp(ident->val,"rms")))
      return 0;
    return 0;
  case 's':
    if (!(strcmp(ident->val,"samphold")))
      return 0;
    if (!(strcmp(ident->val,"sblock")))
      return 0;
    if (!(strcmp(ident->val,"settempo")))
      return 0;
    if (!(strcmp(ident->val,"settune")))
      return 0;
    if (!(strcmp(ident->val,"sgn")))
      return 0;
    if (!(strcmp(ident->val,"sin")))
      return 1;
    if (!(strcmp(ident->val,"spatialize")))
      return 0;
    if (!(strcmp(ident->val,"speedt")))
      return 0;
    if (!(strcmp(ident->val,"sqrt")))
      return 1;
    return 0;
  case 't':
    if (!(strcmp(ident->val,"tableread")))
      return coretablereadinline(ident);
    if (!(strcmp(ident->val,"tablewrite")))
      return 0;  
    return 0; 
  case 'u':
    if (!(strcmp(ident->val,"upsamp")))
      return 0;
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
  default:
    return 0;
 }
}


extern void coremathinline(char *, tnode *);

/*********************************************************/
/*           inlines selected core opcodes               */
/*                                                       */
/* now only called for S_OPCALLS, because:               */
/*    [1] no place for computing side-effect producing   */
/*        oparray index.                                 */
/*    [2] return symbol isn't created correctly.         */
/*                                                       */
/*********************************************************/

void coreopcodedoinline(tnode * ident)

{
  tnode * tptr;
  int currintstack, currscalarstack;

  if (ident->optr->ttype == S_OPCALL)
    tptr = ident->next->next->down; 
  else
    tptr = ident->next->next->next->next->next->down; 

  switch (ident->val[0]) {
  case 'a':
    if (!(strcmp(ident->val,"abs")))
      {
	coremathinline("fabs", tptr);
	return;
      }
    if (!(strcmp(ident->val,"acos")))
      {
	coremathinline("acos", tptr);
	return;
      }
    if (!(strcmp(ident->val,"aexprand")))
      {
	fprintf(outfile,"-((float)log(RMULT*((float)rand())+1e-45F))*(");
	blocktree(tptr->down,PRINTTOKENS);
	fprintf(outfile,")");
        return;
      }
    if (!(strcmp(ident->val,"agaussrand")))
      {
	blocktree(tptr->down,PRINTTOKENS);
	fprintf(outfile,"+ (float)(sqrt( ");
	tptr = tptr->next;
	while (tptr->ttype != S_EXPR)
	  tptr = tptr->next;
	blocktree(tptr->down,PRINTTOKENS);
	fprintf(outfile,")*(float)cos(6.283185F*RMULT*((float)rand()))*\n");
	fprintf(outfile,"(float)sqrt(-2.0F*(float)log(RMULT*((float)rand())+1e-45F)))\n");
	return;
      }
    if (!(strcmp(ident->val,"ampdb")))
      {
	fprintf(outfile,"(float)(pow(10.0F, 5.0e-2F*(-90.0F + ");
	blocktree(tptr->down,PRINTTOKENS);
	fprintf(outfile,")))");
	return;
      }
    if (!(strcmp(ident->val,"arand")))
      {
	fprintf(outfile,"2.0F*(RMULT*((float)rand()) - 0.5F)*(");
	blocktree(tptr->down,PRINTTOKENS);
	fprintf(outfile,")");
        return;
      }
    if (!(strcmp(ident->val,"asin")))
      {
	coremathinline("asin", tptr);
	return;
      }
    if (!(strcmp(ident->val,"atan")))
      {
	coremathinline("atan", tptr);
	return;
      }
    return;
  case 'b':
    return;
  case 'c':
    if (!(strcmp(ident->val,"ceil")))
      {
	coremathinline("ceil", tptr);
	return;
      }
    if (!(strcmp(ident->val,"cos")))
      {
	coremathinline("cos", tptr);
	return;
      }
    if (!(strcmp(ident->val,"cpsmidi")))
      {
	fprintf(outfile,"EV(globaltune)*(float)(pow(2.0F, 8.333334e-02F*(-69.0F + ");
	blocktree(tptr->down,PRINTTOKENS);
	fprintf(outfile,")))");
	return;
      }
    if (!(strcmp(ident->val,"cpsoct")))
      {	
	fprintf(outfile,"EV(globaltune)*(float)pow(2.0F, (-8.75F + ");
	blocktree(tptr->down,PRINTTOKENS);
	fprintf(outfile,"))");
	return;
      }
    return;
  case 'd':
    if (!(strcmp(ident->val,"dbamp")))
      {
	fprintf(outfile,"(float)(90.0F+20.0F*(float)log10(1e-10F + ");
	blocktree(tptr->down,PRINTTOKENS);
	fprintf(outfile,"))");
	return;
      }
    return;
  case 'e':
    if (!(strcmp(ident->val,"exp")))
      {
	coremathinline("exp", tptr);
	return;
      }
    return;
  case 'f':
    if (!(strcmp(ident->val,"floor")))
      {
	coremathinline("floor", tptr);
	return;
      }
    if (!(strcmp(ident->val,"frac")))
      {
	fprintf(outfile,"(float)(modf(");
	blocktree(tptr->down,PRINTTOKENS);
	fprintf(outfile,", &EV(fracdummy)))");
	return;
      }
    if (!(strcmp(ident->val,"ftbasecps"))) 
      {
	printinlinetable(tptr);
	fprintf(outfile,".base");
      }
    if (!(strcmp(ident->val,"ftlen")))
      {
	printinlinetable(tptr);
	fprintf(outfile,".lenf");
      }
    if (!(strcmp(ident->val,"ftloop")))
      {
	fprintf(outfile,"((float)(");
	printinlinetable(tptr);
	fprintf(outfile,".start))");
      }
    if (!(strcmp(ident->val,"ftloopend"))) 
      {
	fprintf(outfile,"((float)(");
	printinlinetable(tptr);
	fprintf(outfile,".end))");
      }
    if (!(strcmp(ident->val,"ftsetbase")))
      return;
    if (!(strcmp(ident->val,"ftsetend")))
      return;
    if (!(strcmp(ident->val,"ftsetloop")))
      return;
    if (!(strcmp(ident->val,"ftsetsr")))
      return;
    if (!(strcmp(ident->val,"ftsr")))
      {
	printinlinetable(tptr);
	fprintf(outfile,".sr");
      }
    return;
  case 'g':
    if (!(strcmp(ident->val,"gettune")))
      {
	fprintf(outfile,"EV(globaltune)");
	return;
      }
    if (!(strcmp(ident->val,"gettempo")))
      {
	fprintf(outfile,"EV(tempo)");
	return;
      }
    return;
  case 'h':
    return;
  case 'i':
    if (!(strcmp(ident->val,"iexprand")))
      {
	fprintf(outfile,"-((float)log(RMULT*((float)rand())+1e-45F))*(");
	blocktree(tptr->down,PRINTTOKENS);
	fprintf(outfile,")");
        return;
      }
    if (!(strcmp(ident->val,"igaussrand")))
      {
	blocktree(tptr->down,PRINTTOKENS);
	fprintf(outfile,"+ (float)(sqrt( ");
	tptr = tptr->next;
	while (tptr->ttype != S_EXPR)
	  tptr = tptr->next;
	blocktree(tptr->down,PRINTTOKENS);
	fprintf(outfile,")*(float)cos(6.283185F*RMULT*((float)rand()))*\n");
	fprintf(outfile,"(float)sqrt(-2.0F*(float)log(RMULT*((float)rand())+1e-45F)))\n");
	return;
      }
    if (!(strcmp(ident->val,"int")))
      {
	currintstack = currintprint;
	currintprint = ASFLOAT;
	fprintf(outfile,"(int)(");
	blocktree(tptr->down,PRINTTOKENS);
	fprintf(outfile,")\n");
	currintprint = currintstack;
	return;
      }
    if (!(strcmp(ident->val,"irand")))
      {
	fprintf(outfile,"2.0F*(RMULT*((float)rand()) - 0.5F)*(");
	blocktree(tptr->down,PRINTTOKENS);
	fprintf(outfile,")");
        return;
      }
    return;
  case 'j':
    return;
  case 'k':
    if (!(strcmp(ident->val,"kexprand")))
      {
	fprintf(outfile,"-((float)log(RMULT*((float)rand())+1e-45F))*(");
	blocktree(tptr->down,PRINTTOKENS);
	fprintf(outfile,")");
        return;
      }
    if (!(strcmp(ident->val,"kgaussrand")))
      {
	blocktree(tptr->down,PRINTTOKENS);
	fprintf(outfile,"+ (float)(sqrt( ");
	tptr = tptr->next;
	while (tptr->ttype != S_EXPR)
	  tptr = tptr->next;
	blocktree(tptr->down,PRINTTOKENS);
	fprintf(outfile,")*(float)cos(6.283185F*RMULT*((float)rand()))*\n");
	fprintf(outfile,"(float)sqrt(-2.0F*(float)log(RMULT*((float)rand())+1e-45F)))\n");
	return;
      }
    if (!(strcmp(ident->val,"krand")))
      {
	fprintf(outfile,"2.0F*(RMULT*((float)rand()) - 0.5F)*(");
	blocktree(tptr->down,PRINTTOKENS);
	fprintf(outfile,")");
        return;
      }
    return;
  case 'l':
    if (!(strcmp(ident->val,"log")))
      {
	coremathinline("log", tptr);
	return;
      }
    if (!(strcmp(ident->val,"log10")))
      {
	coremathinline("log10", tptr);
	return;
      }
    return;
  case 'm':
    if (!(strcmp(ident->val,"midicps")))
      {	
	fprintf(outfile,"(int)(69.5F + 1.731234e+01F*log(EV(invglobaltune)*");
	blocktree(tptr->down,PRINTTOKENS);
	fprintf(outfile,"))");
	return;
      }
    if (!(strcmp(ident->val,"midioct")))
      {
	fprintf(outfile,"(int)(0.5F + 12.0F*(-3.0F + ");
	blocktree(tptr->down,PRINTTOKENS);
	fprintf(outfile,"))");
	return;
      }
    return;
  case 'n':
    return;
  case 'o':
    if (!(strcmp(ident->val,"octcps")))
      {
	fprintf(outfile,"8.75F + 1.442695F*(float)log(EV(invglobaltune)*");
	blocktree(tptr->down, PRINTTOKENS);
	fprintf(outfile,")");
	return;
      }
    if (!(strcmp(ident->val,"octmidi")))
      {
	fprintf(outfile,"8.333334e-2F*(36.0F + ");
	blocktree(tptr->down,PRINTTOKENS);
	fprintf(outfile,")");
	return;
      }
    return;
  case 'p':
    if (!(strcmp(ident->val,"pow")))
      {	
	fprintf(outfile,"(float)(pow(");
	blocktree(tptr->down,PRINTTOKENS);
	fprintf(outfile,", ");
	tptr = tptr->next;
	while (tptr->ttype != S_EXPR)
	  tptr = tptr->next;
	blocktree(tptr->down,PRINTTOKENS);
	fprintf(outfile,"))");
	return;
      }
    return;
  case 'q':
    return;
  case 'r':
    return;
  case 's':
    if (!(strcmp(ident->val,"sin")))
      {
	coremathinline("sin", tptr);
	return;
      }
    if (!(strcmp(ident->val,"speedt"))) /* do later */
      return;
    if (!(strcmp(ident->val,"sqrt")))
      {
	coremathinline("sqrt", tptr);
	return;
      }
    return;
  case 't':
    if (!(strcmp(ident->val,"tableread")))  
      {
	fprintf(outfile,"(");
	printinlinetable(tptr);
	fprintf(outfile,".t[");
	tptr = tptr->next->next;

	currintstack = currintprint;
	currscalarstack = currscalarflag;
	currintprint = ASINT;              /* only inlined if ASINT */
	currscalarflag = 1;                /* must always be width 1 */
	blocktree(tptr->down,PRINTTOKENS);
	currintprint = currintstack;
	currscalarflag = currscalarstack;

	fprintf(outfile,"])");
	return;
      }
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


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*                                                        */
/*   Utility functions for top-level inline functions.    */
/*                                                        */
/*________________________________________________________*/


/*********************************************************/
/* returns 1 if first argument of opcode is a TABLETYPE  */
/*********************************************************/

int coreopcodetabletype(tnode * ident)

{  
  tnode * tptr;

  /* disable inlining of tables for now */

  if (ident->optr->ttype == S_OPCALL)
    tptr = ident->next->next->down; 
  else
    tptr = ident->next->next->next->next->next->down; 

  if (tptr->down->vartype == TABLETYPE)
    return 1;
  else
    return 0;
}

/*********************************************************/
/* returns 1 if tableread can be inlined                 */
/*********************************************************/

int coretablereadinline(tnode * ident)

{  
  tnode * tptr;
  int ret = 0;

  if ((interp == INTERP_SINC) && 
      ((ident->optr->rate == ARATETYPE) || 
       (ident->optr->rate == KRATETYPE)))
    return ret;

  if (ident->optr->ttype == S_OPCALL)
    tptr = ident->next->next->down; 
  else
    tptr = ident->next->next->next->next->next->down; 
  
  if (tptr->down->vartype == TABLETYPE)
    {
      tptr = tptr->next->next;
      if (tptr->res == ASINT)
	ret = 1;
    }

  return ret;
}


/*********************************************************/
/*    inline code for simple one-op math opcodes         */
/*********************************************************/

void coremathinline(char * name, tnode * tptr)

{
  fprintf(outfile,"(float)%s(", name);
  blocktree(tptr->down,PRINTTOKENS);
  fprintf(outfile,")\n");
}

