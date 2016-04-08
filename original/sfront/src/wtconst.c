
/*
#    Sfront, a SAOL to C translator    
#    This file: Code generaton: core wavetables
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
/*                                                              */
/*   Top-level function for reduction of constant wavetables.   */
/*           Called in optmain.c and readscore.c                */
/*                                                              */
/*______________________________________________________________*/

extern void printtableproblem(tnode *, tnode *);

extern int  buzzconstcheck(tnode *, tnode *, int, float **);
extern int  cubicconstcheck(tnode *, tnode *, int, float **);
extern int  dataconstcheck(tnode *, tnode *, int, float **);
extern int  expsegconstcheck(tnode *, tnode *, int, float **);
extern void harmconstcheck(tnode *, tnode *, int, float **);
extern void harmphaseconstcheck(tnode *, tnode *, int, float **);
extern int  linesegconstcheck(tnode *, tnode *, int, float **);
extern void periodicconstcheck(tnode *, tnode *, int, float **);
extern void polynomialconstcheck(tnode *, tnode *, int, float **);
extern void randomconstcheck(tnode *, tnode *);
extern void sampleconstcheck(tnode *, tnode *);
extern int  splineconstcheck(tnode *, tnode *, int, float **);
extern int  stepconstcheck(tnode *, tnode *, int, float **);
extern void windowconstcheck(tnode *, tnode *, int, float **);


/*********************************************************/
/*    reduces the size parameter for constant tables     */
/*                                                       */
/*                  upon return, defnode's               */
/* arrayidx holds actual size, usesinput signals table   */
/*         precomputation can be done by sfront          */
/*         return pointer may hold computed table        */
/*********************************************************/

float * wavereduceconstants(tnode * defnode, tnode * nsl)

{
  tnode * ident = defnode->down->next->next->next;
  tnode * sizeptr = defnode->down->next->next->next->next->next->down;
  char newval[STRSIZE];
  tnode * tptr;
  int size = 0;
  int serr = 0;
  float * ret = NULL;

  defnode->usesinput = 0;

  /***********************************/
  /* first regularize size parameter */
  /***********************************/

  if (sizeptr == NULL)
    {
      printf("Error: Generator needs size parameter.\n");
      printtableproblem(sizeptr, nsl);
    }

  if (sizeptr->vol == CONSTANT)
    {
      if ((size = make_int(sizeptr->down)) < 0)
	size = -1;

      if (sizeptr->down->ttype == S_NUMBER)
	{
	  sizeptr->res = sizeptr->down->res = ASINT;
	  sizeptr->down->ttype = S_INTGR;
	}

      if (size == 0)
	{
	  printf("Error: Generator size rounds to zero.\n");
	  printtableproblem(sizeptr, nsl);
	}
    }

  tptr = sizeptr->next ? sizeptr->next->next : NULL;

  /********************************************/
  /* syntax-check and update table parameters */
  /********************************************/

  if (!(strcmp(ident->val,"sample")))
    {
      sampleconstcheck(tptr, nsl);
    }
  else
    if (defnode->vol == CONSTANT) 
      switch (ident->val[0]) {
      case 'b':
	if (!(strcmp(ident->val,"buzz")))
	  {
	    size = buzzconstcheck(tptr, nsl, size, &ret);
	    break;
	  }
	break;
      case 'c':
	if (!(strcmp(ident->val,"concat")))
	  {
	    /* size calculation must wait until runtime */
	    break;
	  }
	if (!(strcmp(ident->val,"cubicseg")))
	  {
	    size = cubicconstcheck(tptr, nsl, size, &ret);
	    break;
	  }
	break;
      case 'd':
	if (!(strcmp(ident->val,"data")))
	  {
	    size = dataconstcheck(tptr, nsl, size, &ret);
	    break;
	  }
	break;
      case 'e':
	if (!(strcmp(ident->val,"empty")))
	  {
	    if (size <= 0)
	      serr = 1;
	    else
	      defnode->usesinput = 1;
	    break;
	  }
	if (!(strcmp(ident->val,"expseg")))
	  {
	    size = expsegconstcheck(tptr, nsl, size, &ret);
	    break;
	  }
	break;
      case 'h':
	if (!(strcmp(ident->val,"harm")))
	  {
	    if (size <= 0)
	      serr = 1;
	    else
	      harmconstcheck(tptr, nsl, size, &ret);
	    break;
	  }
	if (!(strcmp(ident->val,"harm_phase")))
	  {
	    if (size <= 0)
	      serr = 1;
	    else
	      harmphaseconstcheck(tptr, nsl, size, &ret);
	    break;
	  }
	break;
      case 'l':
	if (!(strcmp(ident->val,"lineseg")))
	  {
	    size = linesegconstcheck(tptr, nsl, size, &ret);
	    break;
	  }
	break;
      case 'p':
	if (!(strcmp(ident->val,"periodic")))
	  {
	    if (size <= 0)
	      serr = 1;
	    else
	      periodicconstcheck(tptr, nsl, size, &ret);
	    break;
	  }
	if (!(strcmp(ident->val,"polynomial")))
	  {
	    if (size <= 0)
	      serr = 1;
	    else
	      polynomialconstcheck(tptr, nsl, size, &ret);
	    break;
	  }
	break;
      case 'r':
	if (!(strcmp(ident->val,"random")))
	  {
	    if (size <= 0)
	      serr = 1;
	    else
	      randomconstcheck(tptr, nsl);
	    break;
	  }
	break;
      case 's':
	if (!(strcmp(ident->val,"spline")))
	  {
	    size = splineconstcheck(tptr, nsl, size, &ret);
	    break;
	  }
	if (!(strcmp(ident->val,"step")))
	  {
	    size = stepconstcheck(tptr, nsl, size, &ret);
	    break;
	  }
	break;
      case 'w':
	if (!(strcmp(ident->val,"window")))
	  {
	    if (size <= 0)
	      serr = 1;
	    else
	      windowconstcheck(tptr, nsl, size, &ret);
	    break;
	  }
	break;
      default:
	break;
      }

  if (serr)
    {
      printf("Error: Size of -1 illegal for %s wavetable generator.\n",
	     ident->val);
      printtableproblem(sizeptr, nsl);
    }

  /*********************/
  /* update and return */
  /*********************/

  if (sizeptr->vol == CONSTANT)
    {
      sprintf(newval,"%i",size);
      free(sizeptr->down->val);
      sizeptr->down->val = dupval(newval);
    }

  if (defnode->vol == CONSTANT)
    {
      defnode->arrayidx = size;
      defnode->usesinput |= (ret != NULL);
    }

  return ret;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*                                                              */
/*    Top-level code-generation for SAOL constant wavetables.   */
/*    See printtableloop() in writeorc.c for SASL equivalent.   */
/*                                                              */
/*______________________________________________________________*/

extern char * wavetablenameprefix(sigsym *, char *);

/*********************************************************/
/*           creates code for constant tables            */
/*          called in createtable in wtparse.c           */
/*                                                       */
/*     TABLE IDENT LP IDENT COM exprstrlist RP           */
/*********************************************************/

void createconstanttable(sigsym * ident, char * prefix, int mode)

{
  int lc = 0;
  int size = ident->defnode->arrayidx;

  mz(lc); sprintf(z[lc++], 
		  "NT(%s_%s).lenf = (float)(NT(%s_%s).len = %i);",
		   prefix,ident->val, prefix, ident->val, size);

  mz(lc); sprintf(z[lc++], "NT(%s_%s).tend = %i;",
		  prefix,ident->val, size -1);

  mz(lc); sprintf(z[lc++], 
		  "NT(%s_%s).oconst = %i.0F*EV(ATIME);",
		   prefix,ident->val, size);

  if (interp == INTERP_SINC)
    {
      mz(lc); sprintf(z[lc++], 
		      "NT(%s_%s).sffl = 1.0F;", prefix,ident->val);
      
      mz(lc); sprintf(z[lc++], 
		      "NT(%s_%s).sfui = 0x00010000;", prefix,ident->val);
      
      mz(lc); sprintf(z[lc++], 
		      "NT(%s_%s).dsincr = SINC_PILEN;", prefix,ident->val);
    }

  switch (mode) {
  case S_GLOBAL:
    mz(lc); sprintf(z[lc++],
		    "NT(%s_%s).t = (float *)(table_global_%s);",
		    prefix, ident->val, ident->val);
    break;
  case S_INSTR:
    mz(lc); sprintf(z[lc++],
		    "NT(%s_%s).t = (float *)(memcpy"
		    "(malloc(%i), table_%s_%s, %i));",
		    prefix, ident->val, (int)(sizeof(float)*(size+1)),
		    currinstancename, ident->val, 
		    (int)(sizeof(float)*(size+1)));
    mz(lc); sprintf(z[lc++], "NT(%s_%s).llmem = 1;", prefix, ident->val);
    break;
  case S_OPCODE:
    mz(lc); sprintf(z[lc++],
		    "NT(%s_%s).t = (float *)(memcpy"
		    "(malloc(%i), %s_%s%i_%s, %i));",
		    prefix, ident->val, (int)(sizeof(float)*(size+1)),
		    wavetablenameprefix(curropcodestack, "table_"),
		    curropcodeinstance->val, curropcodeinstance->arrayidx,
		    ident->val, (int)(sizeof(float)*(size+1)));
    mz(lc); sprintf(z[lc++], "NT(%s_%s).llmem = 1;", prefix, ident->val);
    break;
  default:
    internalerror("wtparse","createconstanttable -- mode");
  }
  printraw(lc);

}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*                                                              */
/*       Top-level data-printout for constant wavetables.       */
/*         Used in printtablefunctions() in readscore.c,        */
/*             and printsaoltables() in writepre.c              */
/*                                                              */
/*______________________________________________________________*/

/****************************************************************/
/*      prints out a table as a hexadecimal char array          */
/****************************************************************/

void printtablestring(sigsym * sptr, char * varname)

{
  char newval[STRSIZE];
  unsigned char * endc;
  unsigned char * endl;
  unsigned char * c;
  float * f;
  float * endf;
  float * fline;

  if (hexstrings)
    {
      fprintf(outfile,"char %s[%i]",
	      varname, (int)(sizeof(float)*(sptr->defnode->arrayidx + 1) + 1));

      if (sptr->consval == NULL)
	fprintf(outfile,";\n\n");
      else
	{
	  fprintf(outfile," = \n\"");
	  
	  c = (unsigned char *) (sptr->consval);
	  endc = c + sizeof(float)*(sptr->defnode->arrayidx + 1);
	  endl = c + 15;
	  while (c < endc)
	    {
	      fprintf(outfile,
		      "\\x%02x\\x%02x\\x%02x\\x%02x", 
		      c[0], c[1], c[2], c[3]); 
	      if ((c += 4) > endl)
		{
		  fprintf(outfile,"\"\n\"");
		  endl = c + 15;
		}
	    }
	  fprintf(outfile,"\";\n\n");
	}
    }
  else
    {
      fprintf(outfile,"float %s[%i]",
	      varname, sptr->defnode->arrayidx + 1);

      if (sptr->consval == NULL)
	fprintf(outfile,";\n\n");
      else
	{
	  fprintf(outfile," = { \n");
	  
	  f = (float *) (sptr->consval);
	  endf = f + sptr->defnode->arrayidx + 1;
	  fline= f + 8;
	  while (f < endf)
	    {
	      fprintf(outfile, "%s", compactfloat(newval, *f));
	      if ((++f) == endf)
		fprintf(outfile,"};\n\n");
	      else
		{
		  fprintf(outfile,",");
		  if (fline == f)
		    {
		      fprintf(outfile,"\n");
		      fline= f + 8;
		    }
		}
	    }
	}
    }
}


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*                                                              */
/* Second-level functions for reducing constant wavetables.     */
/*                                                              */
/*______________________________________________________________*/

/***************************************************************/
/*    prints correct table problem, depending on source        */
/***************************************************************/

void printtableproblem(tnode * tptr, tnode * nsl)

{

  printf("\n");

  if (nsl)
    showbadline(nsl);
  else
    {
      while (tptr->down)
	tptr = tptr->down;
      showerrorplace(tptr->linenum, tptr->filename);
    }
}


/***************************************************************/
/*  syntax checks and updates constants for buzz wt generator  */
/***************************************************************/

int buzzconstcheck(tnode * tptr, tnode  *nsl, int size, float ** ret)

{
  int i = 1;
  int j;
  tnode * nptr = tptr;
  int loharm = 0;
  int nharm = 0;
  float rolloff = 0.0F;
  float scale, idx, f, base, acc, index;
  char newval[STRSIZE];
  float * fptr;
  float * endptr;

  while (tptr != NULL)
    {
      switch (i) {
      case 1:
	if ((nharm = make_int(tptr->down)) < 0)
	  nharm = -1;
	if (tptr->down->ttype == S_NUMBER)
	  {
	    tptr->res = tptr->down->res = ASINT;
	    tptr->down->ttype = S_INTGR;
	  }
	break;
      case 2:
	if ((loharm = make_int(tptr->down)) < 0)
	  loharm = -1;
	if (tptr->down->ttype == S_NUMBER)
	  {
	    tptr->res = tptr->down->res = ASINT;
	    tptr->down->ttype = S_INTGR;
	    sprintf(newval,"%i",loharm);
	    free(tptr->down->val);
	    tptr->down->val = dupval(newval);
	  }
	tptr->down->arrayidx = loharm;
	break;
      case 3:
	rolloff = (float)atof(tptr->down->val);
	break;
      default:
	printf("Error: Too many parameters for buzz generator.\n");
	printtableproblem(nptr, nsl);
	break;
      }
      i++;
      tptr = tptr->next ? tptr->next->next : NULL;
    }

  if (i<4)
    {
      printf("Error: Too few parameters for buzz generator.\n");
      printtableproblem(nptr, nsl);
    }

  if ((size < 1) && (nharm < 1))
    {
      printf("Error: Both size and nharm non-positive for buzz generator.\n");
      printtableproblem(nptr, nsl);
    }

  if (loharm < 0)
    {
      printf("Error: Loharm must be >= 0 for buzz generator.\n");
      printtableproblem(nptr, nsl);
    }

  if (size < 1)
    size = 2*(loharm + nharm);
  else
    {
      if (nharm < 1)
	nharm = (int)(floor((size/2.0F) - loharm));
    }

  sprintf(newval,"%i",nharm);
  free(nptr->down->val);
  nptr->down->val = dupval(newval);
  nptr->down->arrayidx = nharm;

  if (((f = (float)fabs(rolloff)) < 1.0F) && (f != 0.0F))
    {
      j = -(int)(17.0F/log(f)) + loharm + 1;
      nharm = (nharm > j) ? j : nharm;
    }

  if (size > MAXTABLECONSTSIZE)
    return(size);

  vmcheck(*ret = (fptr = (float *) calloc((size+1), sizeof(float))));
  
  base = 6.283185F/size;
  if ((rolloff == 1.0F)||(rolloff == -1.0F))
    {
      scale = 1.0F/(1 + nharm);
    }
  else
    {
      if (rolloff == 0.0F)
	scale = 1.0F;
      else
	scale = (1.0F-(float)fabs(rolloff))/
	  (1-(float)fabs((float)pow(rolloff, nharm + 1)));
    }

  idx = 0.0F;
  endptr = fptr + size;
  while (fptr < endptr)
    {
      index = (idx++)*base;
      acc = 1.0F;
      j = loharm + 1;
      while (j <= (loharm + nharm))
	{
	  *fptr += acc*(float)cos(index*(j++));
	  acc *= rolloff;
	}
      *(fptr++) *= scale;
    }

  *fptr = (*ret)[0];
  return(size);
}


/***************************************************************/
/*  syntax checks and updates constants for cubic wt generator  */
/***************************************************************/

int cubicconstcheck(tnode * tptr, tnode * nsl, int size, float ** ret)

{
  char newval[STRSIZE];
  int i = 1;
  int x, oldx, infl, oldinfl;
  float x_f, oldx_f, infl_f, oldinfl_f;
  float y, oldy, oldery;
  tnode * nptr = tptr;
  float * fptr;
  float * endptr;
  float * segptr;
  float Q, R, S, T, a, b, c, d;

  x = oldx = infl = oldinfl = -1;
  x_f = oldx_f = infl_f = oldinfl_f = -1.0F;

  while (tptr != NULL)
    {
      switch (i & 3) {
      case 1:
	oldinfl = infl;
	oldinfl_f = (float) oldinfl;
	infl = make_int(tptr->down);
	infl_f = (float) infl;

	if (tptr->down->ttype == S_NUMBER)
	  {
	    tptr->res = tptr->down->res = ASINT;
	    tptr->down->ttype = S_INTGR;
	    sprintf(newval,"%i",infl);
	    free(tptr->down->val);
	    tptr->down->val = dupval(newval);
	  }
	if ((i == 1) && (infl != 0))
	  {     
	    printf("Error: Cubic generator first infl value is not 0.\n");
	    printtableproblem(tptr, nsl);
	  }	
	if (i > 1)
	  {
	    Q = (oldinfl_f*oldinfl_f*oldinfl_f - x_f*x_f*x_f 
		 - 3.0F*x_f*x_f*(oldinfl_f - x_f));
	    if (Q == 0.0F)
	      {     
		printf("Error: No cubic solution for these parameters.\n");
		printtableproblem(tptr, nsl);
	      }

	    R = oldinfl_f*oldinfl_f - x_f*x_f - 2.0F*x_f*(oldinfl_f - x_f);
	    if (R == 0.0F)
	      {     
		printf("Error: No cubic solution for these parameters.\n");
		printtableproblem(tptr, nsl);
	      }
      
	    S = x_f*x_f*x_f - infl_f*infl_f*infl_f - 3.0F*x_f*x_f*(x_f - infl_f);
	    if (S == 0.0F)
	      {     
		printf("Error: No cubic solution for these parameters.\n");
		printtableproblem(tptr, nsl);
	      }
	    
	    T = x_f*x_f - infl_f*infl_f - 2.0F*x_f*(x_f - infl_f);
	    if ((T == 0.0F) || ((Q/R) == (S/T)))
	      {     
		printf("Error: No cubic solution for these parameters.\n");
		printtableproblem(tptr, nsl);
	      }
	  }
	tptr->down->arrayidx = infl;
	break;
      case 3:
	oldx = x;
	oldx_f = (float) oldx;
	x = make_int(tptr->down);
	x_f = (float) x;
	if (tptr->down->ttype == S_NUMBER)
	  {
	    tptr->res = tptr->down->res = ASINT;
	    tptr->down->ttype = S_INTGR;
	    sprintf(newval,"%i",x);
	    free(tptr->down->val);
	    tptr->down->val = dupval(newval);
	  }
	if ((x != oldx) && 
	    ((x < oldx) || (infl >= x) || (infl <= oldx)))
	  {     
	    printf("Error: Cubic generator x/infl values decreasing.\n");
	    printtableproblem(tptr, nsl);
	  }	    
	tptr->down->arrayidx = x;
	break;
      case 0:
      case 2:
	break;
      }
      i++;
      tptr = tptr->next ? tptr->next->next : NULL;
    }

  if ((i & 1) == 0)
    {
      printf("Error: Cubic generator data doesn't end with a y value.\n");
      printtableproblem(nptr, nsl);
    }

  if (i < 9)
    {
      printf("Error: Cubic generator needs at least 2 x values.\n");
      printtableproblem(nptr, nsl);
    }

  if (size < 0)
    {
      /* later change to be size=x */
      size = ( ((i-2)&3) == 1) ? infl : x; 
      if (size < 1)
	{     
	  printf("Error: Cubic generator has zero-sized table.\n");
	  printtableproblem(nptr, nsl);
	}
    }

  if (size > MAXTABLECONSTSIZE)
    return(size);

  vmcheck(*ret = (fptr = (float *) calloc((size+1), sizeof(float))));

  endptr = fptr + size;

  x = oldx = infl = oldinfl = -1;
  y = oldy = oldery = -1.0F;
  x_f = oldx_f = infl_f = oldinfl_f = -1.0F;

  i = 1;
  tptr = nptr;
  while (tptr != NULL)
    {
      switch (i & 3) {
      case 1:
	oldinfl = infl;
	oldinfl_f = (float) oldinfl;
	infl = tptr->down->arrayidx;
	infl_f = (float) infl;
	if (tptr->next && tptr->next->next)
	  {
	    oldery = oldy;
	    oldy = y;
	    y = (float)atof(tptr->next->next->down->val);
	  }
	if (i > 1)
	  {
	    Q = (oldinfl_f*oldinfl_f*oldinfl_f - x_f*x_f*x_f
		 - 3.0F*x_f*x_f*(oldinfl_f-x_f));
	    R = oldinfl_f*oldinfl_f - x_f*x_f - 2.0F*x_f*(oldinfl_f-x_f);
	    S = x_f*x_f*x_f - infl_f*infl_f*infl_f - 3.0F*x_f*x_f*(x_f-infl_f);
	    T = x_f*x_f - infl_f*infl_f - 2.0F*x_f*(x_f-infl_f);

	    a = 1.0F/((Q/R) - (S/T));
	    a *= (oldery-oldy)/R - (oldy-y)/T;
	    b = 1.0F/((R/Q) - (T/S));
	    b *= (oldery-oldy)/Q - (oldy-y)/S;
	    c = - 3.0F*a*x_f*x_f - 2.0F*b*x_f;
	    d = oldy - a*x_f*x_f*x_f;
	    d += - b*x_f*x_f - c*x_f;
    
	    segptr = fptr + (infl - oldinfl);
	    while ((fptr < endptr) && (fptr < segptr))
	      {
		*(fptr++) = oldinfl_f*oldinfl_f*oldinfl_f*a +
		  oldinfl_f*oldinfl_f*b + oldinfl_f*c + d;
		oldinfl_f = (float) (++oldinfl);
	      }
	  }
	break;
      case 3:
	oldx = x;
	oldx_f = (float) oldx;
	x = tptr->down->arrayidx;
	x_f = (float) x;
	break;
      case 0:
	oldery = oldy;
	oldy = y;
	y = (float)atof(tptr->down->val);
	break;
      case 2:
	break;
      }
      i++;
      tptr = tptr->next ? tptr->next->next : NULL;
    }

  (*ret)[size] = (*ret)[0];

  return size;
}


/***************************************************************/
/*   syntax checks and updates constants for data wtgen        */
/***************************************************************/

int dataconstcheck(tnode * tptr, tnode  *nsl, int size, float ** ret)

{
  tnode * data = tptr;
  float * fptr;
  float * endptr;
  int numpoints = 0;
  int nosize;

  if ((nosize = (size <= 0)))
    size = 0;

  while (tptr)
    {
      if (nosize)
	size++;
      numpoints++;
      tptr = tptr->next ? tptr->next->next : NULL;
    }

  if (nosize && (!size))
    {      
      printf("Error: No data values in a `size = -1' data generator.\n");
      printtableproblem(data, nsl);
    }

  /* handles giant table with a few data points case well */

  if ((size > MAXTABLECONSTSIZE) && (numpoints < MAXTABLECONSTSIZE))
    return size;

  vmcheck(*ret = (fptr = (float *) calloc((size+1), sizeof(float))));
  endptr = fptr + size;

  tptr = data;

  while (tptr && (fptr < endptr))
    {
      *(fptr++) = (float)atof(tptr->down->val);
      tptr = tptr->next ? tptr->next->next : NULL;
    }

  (*ret)[size] = (*ret)[0];
  return size;

}

/***************************************************************/
/*  syntax checks and updates constants for expseg generator   */
/***************************************************************/

int expsegconstcheck(tnode * tptr, tnode * nsl, int size, float ** ret)

{
  int i = 1;
  int x = -1;
  int y = -1;
  int oldx;
  float f, oldf, diff;
  tnode * nptr = tptr;
  char newval[STRSIZE];
  float * fptr;
  float * endptr;
  float * segptr;

  while (tptr != NULL)
    {
      switch (i & 1) {
      case 1:
	oldx = x;
	x = make_int(tptr->down);
	if (tptr->down->ttype == S_NUMBER)
	  {
	    tptr->res = tptr->down->res = ASINT;
	    tptr->down->ttype = S_INTGR;
	    sprintf(newval,"%i",x);
	    free(tptr->down->val);
	    tptr->down->val = dupval(newval);
	  }
	if (x < oldx)
	  {     
	    printf("Error: Expseg generator x values decreasing.\n");
	    printtableproblem(tptr, nsl);
	  }	    
	if ((i == 1) && (x != 0))
	  {
	    printf("Error: Expseg generator first x value is not 0.\n");
	    printtableproblem(tptr, nsl);
	  }	    
	tptr->down->arrayidx = x;
	break;
      case 0:
	if ((f = (float)atof(tptr->down->val)) == 0.0F)
	  {
	    printf("Error: Expseg generator y value is 0.\n");
	    printtableproblem(tptr, nsl);
	  }
	if (y == -1)
	  y = (f > 0.0F);
	else
	  if ((f > 0.0F) != y)
	    {
	      printf("Error: Expseg generator y values must all ");
	      printf("have the same sign.\n");
	      printtableproblem(tptr, nsl);
	    }
	break;
      }
      i++;
      tptr = tptr->next ? tptr->next->next : NULL;
    }

  if (size < 0)
    {
      size = x;
      if (size < 1)
	{     
	  printf("Error: Expseg generator has zero-sized table.\n");
	  printtableproblem(nptr, nsl);
	}
    }

  if ((i & 1) == 0)
    {
      printf("Error: Expseg generator data ends with a x value.\n");
      printtableproblem(nptr, nsl);
    }

  if (size > MAXTABLECONSTSIZE)
    return(size);

  vmcheck(*ret = (fptr = (float *) calloc((size+1), sizeof(float))));

  endptr = fptr + size;
  i = 1;
  tptr = nptr;
  oldx = x = -1;
  f = oldf = 0.0F;
  while (tptr != NULL)
    {
      switch (i & 1) {
      case 1:
	oldx = x;
	x = tptr->down->arrayidx;
	break;
      case 0:
	oldf = f;
	f = (float)atof(tptr->down->val);
	if ((i > 2) && (oldx != x))
	  {
	    segptr = fptr + (x - oldx);
	    *(fptr++) = oldf;
	    diff = (float)pow(f/oldf, 1.0F/(x - oldx));
	    while ((fptr < endptr) && (fptr < segptr))
	      {
		*fptr = (*(fptr - 1))*diff;
		fptr++;
	      }
	  }
	break;
      }
      i++;
      tptr = tptr->next ? tptr->next->next : NULL;
    }

  (*ret)[size] = (*ret)[0];

  return size;
}

/***************************************************************/
/*   syntax checks and updates constants for harm wtgen        */
/***************************************************************/

void harmconstcheck(tnode * tptr, tnode  *nsl, int size, float ** ret)

{
  float base = (6.283185F/size);
  float amp, idx, harm;
  float * fptr = NULL;
  float * endptr = NULL;

  if (size > MAXTABLECONSTSIZE)
    return;
  
  vmcheck(*ret = (float *) calloc((size+1), sizeof(float)));

  harm = 0.0;
  while (tptr != NULL)
    {      
      amp = (float)atof(tptr->down->val);
      idx = 0.0F;
      endptr = (fptr = *ret) + size;
      harm++;
      while (fptr < endptr)
	*(fptr++) += amp*((float)sin(harm*(idx++)*base));
      tptr = tptr->next ? tptr->next->next : NULL;
    }

  if (fptr)
    *fptr = (*ret)[0];

  return;

}

/***************************************************************/
/*   syntax checks and updates constants for harm_phase wtgen  */
/***************************************************************/

void harmphaseconstcheck(tnode * tptr, tnode  *nsl, int size, float ** ret)

{
  int state;
  tnode * first = tptr;
  float base, amp, phase, idx, harm;
  float * fptr = NULL; 
  float * endptr;

  if (size > MAXTABLECONSTSIZE)
    return;
  
  vmcheck((*ret = (float *) calloc((size+1), sizeof(float))));

  state = 0;
  amp = phase = harm = 0.0F;
  base = (6.283185F/size);
  
  while (tptr != NULL)
    {      
      switch (state) {
      case 0:
	amp = (float)atof(tptr->down->val);
	state = 1;
	break;
      case 1:
	phase = (float)atof(tptr->down->val);
	idx = 0.0F;
	endptr = (fptr = *ret) + size;
	harm++;
	while (fptr < endptr)
	  *(fptr++) += amp*((float)sin((harm*(idx++)*base) + phase));
	state = 0;
	break;
      }
      tptr = tptr->next ? tptr->next->next : NULL;
    }

  if (state && first)
    {
      printf("Error: harm_phase data values not in (f, ph) pairs.\n");
      printtableproblem(first, nsl);
    }
    
  if (fptr)
    *fptr = (*ret)[0];

  return;
}


/***************************************************************/
/*  syntax checks and updates constants for lineseg generator  */
/***************************************************************/

int linesegconstcheck(tnode * tptr, tnode * nsl, int size, float ** ret)

{
  int i = 1;
  int x = -1;
  int oldx;
  float f, oldf, diff;
  tnode * nptr = tptr;
  char newval[STRSIZE];
  float * fptr;
  float * endptr;
  float * segptr;

  while (tptr != NULL)
    {
      switch (i & 1) {
      case 1:
	oldx = x;
	x = make_int(tptr->down);
	if (tptr->down->ttype == S_NUMBER)
	  {
	    tptr->res = tptr->down->res = ASINT;
	    tptr->down->ttype = S_INTGR;
	    sprintf(newval,"%i",x);
	    free(tptr->down->val);
	    tptr->down->val = dupval(newval);
	  }
	if (x < oldx)
	  {     
	    printf("Error: Lineseg generator x values decreasing.\n");
	    printtableproblem(tptr, nsl);
	  }	    
	if ((i == 1) && (x != 0))
	  {     
	    printf("Error: Lineseg generator first x value is not 0.\n");
	    printtableproblem(tptr, nsl);
	  }	    
	tptr->down->arrayidx = x;
	break;
      case 0:
	break;
      }
      i++;
      tptr = tptr->next ? tptr->next->next : NULL;
    }

  if (size < 0)
    {
      size = x;
      if (size < 1)
	{     
	  printf("Error: Lineseg generator has zero-sized table.\n");
	  printtableproblem(nptr, nsl);
	}
    }

  if ((i & 1) == 0)
    {
      printf("Error: Lineseg generator data ends with a x value.\n");
      printtableproblem(nptr, nsl);
    }

  if (size > MAXTABLECONSTSIZE)
    return(size);

  vmcheck(*ret = (fptr = (float *) calloc((size+1), sizeof(float))));

  endptr = fptr + size;
  i = 1;
  tptr = nptr;
  oldx = x = -1;
  f = oldf = 0.0F;
  while (tptr != NULL)
    {
      switch (i & 1) {
      case 1:
	oldx = x;
	x = tptr->down->arrayidx;
	break;
      case 0:
	oldf = f;
	f = (float)atof(tptr->down->val);
	if ((i > 2) && (oldx != x))
	  {
	    segptr = fptr + (x - oldx);
	    *(fptr++) = oldf;
	    diff = (f - oldf)/(x - oldx);
	    while ((fptr < endptr) && (fptr < segptr))
	      {
		*fptr = *(fptr - 1) + diff;
		fptr++;
	      }
	  }
	break;
      }
      i++;
      tptr = tptr->next ? tptr->next->next : NULL;
    }

  (*ret)[size] = (*ret)[0];

  return size;
}

/***************************************************************/
/*   syntax checks and updates constants for periodic wtgen   */
/***************************************************************/

void periodicconstcheck(tnode * tptr, tnode  *nsl, int size, float ** ret)

{
  int state;
  tnode * first = tptr;
  float base, amp, phase, freq, idx;
  float * fptr = NULL; 
  float * endptr;

  if (size > MAXTABLECONSTSIZE)
    return;
  
  vmcheck((*ret = (float *) calloc((size+1), sizeof(float))));

  state = 0;
  freq = amp = phase = 0.0F;
  base = (6.283185F/size);
  
  while (tptr != NULL)
    {      
      switch (state) {
      case 0:
	freq = base*((float)atof(tptr->down->val));
	state = 1;
	break;
      case 1:
	amp = (float)atof(tptr->down->val);
	state = 2;
	break;
      case 2:
	phase = (float)atof(tptr->down->val);
	idx = 0.0F;
	endptr = (fptr = *ret) + size;
	while (fptr < endptr)
	  *(fptr++) += amp*((float)sin((freq*(idx++)) + phase));
	state = 0;
	break;
      }
      tptr = tptr->next ? tptr->next->next : NULL;
    }

  if (state && first)
    {
      printf("Error: periodic data values not in (p, f, ph) triples.\n");
      printtableproblem(first, nsl);
    }
    
  if (fptr)
    *fptr = (*ret)[0];

  return;
}

/***************************************************************/
/*   syntax checks and updates constants for polynomial wtgen  */
/***************************************************************/


void polynomialconstcheck(tnode * tptr, tnode * nsl, int size, float ** ret)

{
  int i = 1;
  float scale, yval, ynth;
  float xmin = 0.0F;
  float xmax = 0.0F;
  float * aval, * aptr, * enda;
  tnode * first = tptr;
  float * fptr, * endptr; 

  /* syntax-check generator values */

  while (tptr != NULL)
    {
      switch (i) {
      case 1:
	xmin = (float)atof(tptr->down->val);
	break;
      case 2:
	xmax = (float)atof(tptr->down->val);
	if (xmin == xmax)
	  {
	    printf("Error: xmin == xmax in polynomial generator.\n");
	    printtableproblem(tptr, nsl);
	  }
	break;
      case 3:
	first = tptr;
	break;
      default:
	break;
      }
      i++;
      tptr = tptr->next ? tptr->next->next : NULL;
    }

  if (i < 4)
    {
      printf("Error: Too few parameters for polynomial generator.\n");
      printtableproblem(first, nsl);
    }

  if (size > MAXTABLECONSTSIZE)
    return;

  /* fill aval with polynomial coefficients */

  vmcheck((aptr = (aval = (float *) malloc((i-3)*sizeof(float)))));
  tptr = first;
  while (tptr != NULL)
    {
      if (tptr->ttype == S_EXPR)
	*(aptr++) = (float)atof(tptr->down->val);
      tptr = tptr->next;
    }

  /* create table */

  vmcheck((fptr = (*ret = (float *) calloc((size+1), sizeof(float)))));

  endptr = fptr + size;
  scale = (1.0F/size)*(xmax - xmin);
  while (fptr < endptr)
    {
      aptr = aval;
      *fptr = *(aptr++);
      yval = xmin + scale*(endptr - fptr);
      ynth = 1.0F;
      enda = aptr + (i-4);
      while (aptr < enda)
	*fptr += (*(aptr++))*(ynth *= yval);
      fptr++;
    }

  *fptr = (*ret)[0];
  free(aval);
}


/***************************************************************/
/*    syntax checks and updates constants for random wtgen     */
/***************************************************************/

void randomconstcheck(tnode * tptr, tnode * nsl)

{  
  char newval[STRSIZE];
  int i = 1;
  int dist = 0;
  float p1 = 0.0F;
  float p2;
  tnode * nptr = tptr;

  while (tptr != NULL)
    {
      switch (i) {
      case 1:
	dist = make_int(tptr->down);
	if (tptr->down->ttype == S_NUMBER)
	  {
	    tptr->res = tptr->down->res = ASINT;
	    tptr->down->ttype = S_INTGR;
	    sprintf(newval,"%i",dist);
	    free(tptr->down->val);
	    tptr->down->val = dupval(newval);
	  }
	tptr->down->arrayidx = dist;
	if ((dist < 1) || (dist > 5))
	  {		
	    printf("Error: Illegal dist %i in random generator.\n",
		   dist);
	    printtableproblem(tptr, nsl);
	  }
	break;
      case 2:
	if ((dist == 3) || (dist == 5))
	  return;
	if (dist == 2)
	  p1 = (float)atof(tptr->down->val);
	break;
      case 3:
	if (dist == 1)
	  return;
	p2 = (float)atof(tptr->down->val);
	if ((dist == 2) && (p1 == p2))
	  {		
	    printf("Error: p1 == p2 for random dist 2.\n");
	    printtableproblem(tptr, nsl);
	  }
	if ((dist == 4) && (p2 <= 0.0F))
	  {		
	    printf("Error:  p2 <= 0.0 for random dist 4.\n");
	    printtableproblem(tptr, nsl);
	  }
	return;
	break;
      }
      i++;
      tptr = tptr->next ? tptr->next->next : NULL;
    }

  if (i > 1)
    printf("Error: Too few parameters for random generator dist %i.\n", dist);
  else
    printf("Error: Too few parameters for random generator.\n");

  printtableproblem(nptr, nsl);

}


extern void wavefileconstcheck(tnode *, tnode *);
extern void aiffileconstcheck(tnode *, tnode *);

/***************************************************************/
/*  syntax checks and updates constants for sample generator  */
/***************************************************************/

void sampleconstcheck(tnode * tptr, tnode * nsl)

{
  char * suffix;
  int wav, skip;
  char newval[STRSIZE];

  /*************************/
  /* check filename string */
  /*************************/

  if ((tptr == NULL) || (tptr->ttype != S_STRCONST))
    {      
      printf("Error: Sample generator requires filename.\n");
      printtableproblem(tptr, nsl);
    }

  suffix = strrchr(tptr->val, '.');

  if ((suffix == NULL) ||
      !(strstr(suffix, ".wav") || strstr(suffix, ".WAV") || 
	strstr(suffix, ".aif") || strstr(suffix, ".AIF") || 
	strstr(suffix, ".aiff") || strstr(suffix, ".AIFF")))
    {
      printf("Error: Sample generator requires "
	     ".wav/.WAV/.aif/.AIF/.aiff/.AIFF file name extension.\n");
      printtableproblem(tptr, nsl);
    }

  wav = (strstr(suffix, ".wav") != NULL) || (strstr(suffix, ".WAV") != NULL);

  /*************/
  /* read file */
  /*************/

  if (wav)
    wavefileconstcheck(tptr, nsl);
  else
    aiffileconstcheck(tptr, nsl);

  /*************************/
  /* handle skip parameter */
  /*************************/

  tptr = tptr->next ? tptr->next->next : NULL;

  if (tptr)
    {

      if (tptr->ttype != S_EXPR)
	{      
	  printf("Error: Sample generator skip parameter not"
		 "an expression.\n");
	  printtableproblem(tptr, nsl);
	}
      if (tptr->next)
	{      
	  printf("Error: Too many parameters for sample generator.\n");
	  printtableproblem(tptr, nsl);
	}

      if (tptr->vol == CONSTANT)
	{
	  if ((skip = make_int(tptr->down)) < 0)
	    skip = 0;
	  if (tptr->down->ttype == S_NUMBER)
	    {	  
	      tptr->res = tptr->down->res = ASINT;
	      tptr->down->ttype = S_INTGR;
	    }
	  sprintf(newval,"%i",skip);
	  free(tptr->down->val);
	  tptr->down->val = dupval(newval);
	}
    }

}

/***************************************************************/
/*  syntax checks and updates constants for spline generator  */
/***************************************************************/

int splineconstcheck(tnode * tptr, tnode * nsl, int size, float ** ret)

{
  int i = 1;
  int x = -1;
  int oldx = -1;
  float y, k, oldy, oldk, xf, oldxf;
  tnode * nptr = tptr;
  char newval[STRSIZE];
  float Q, R, S, a, b, c, d;
  float * fptr;
  float * endptr;
  float * segptr;

  y = oldy = k = oldk = 0.0F;

  while (tptr != NULL)
    {
      switch (i % 3) {
      case 1:
	oldx = x;
	x = make_int(tptr->down);
	if (tptr->down->ttype == S_NUMBER)
	  {
	    tptr->res = tptr->down->res = ASINT;
	    tptr->down->ttype = S_INTGR;
	    sprintf(newval,"%i",x);
	    free(tptr->down->val);
	    tptr->down->val = dupval(newval);
	  }
	if (x < oldx)
	  {     
	    printf("Error: Spline generator x values decreasing.\n");
	    printtableproblem(tptr, nsl);
	  }	    
	if ((i == 1) && (x != 0))
	  {     
	    printf("Error: Spline generator first x value is not 0.\n");
	    printtableproblem(tptr, nsl);
	  }	 
	tptr->down->arrayidx = x;
	if ((i != 1) && (oldx != x))
	  {
	    oldxf = (float) oldx;
	    xf = (float) x;

	    Q = 1.0F/(oldxf - xf);
	    R = Q*(oldxf*oldxf - xf*xf) - 2.0F*oldxf;
	    if (R == 0.0F)
	      {     
		printf("Error: No spline solution for these parameters.\n");
		printtableproblem(tptr, nsl);
	      }	 
	    R =1.0F/R;
	    S = Q*(oldxf*oldxf - xf*xf) - 2.0F*xf;
	    if (S == 0.0F)
	      {     
		printf("Error: No spline solution for these parameters.\n");
		printtableproblem(tptr, nsl);
	      }	 
	    S =1.0F/S;
	    
	    a = Q*(oldxf*oldxf*oldxf-xf*xf*xf)*(R-S);
	    a += -3.0F*(R*oldxf*oldxf - S*xf*xf);
	    if (a==0.0F)
	      {     
		printf("Error: No spline solution for these parameters.\n");
		printtableproblem(tptr, nsl);
	      }
	  }
	break;
      case 2:
	break;
      case 0:
	break;
      }
      i++;
      tptr = tptr->next ? tptr->next->next : NULL;
    }

  if (size < 0)
    {
      size = x;
      if (size < 1)
	{     
	  printf("Error: Spline generator has zero-sized table.\n");
	  printtableproblem(nptr, nsl);
	}
    }
  if (i < 6)
    {
      printf("Error: Spline generator needs two data points.\n");
      printtableproblem(nptr, nsl);
    }
  if ((i % 3) == 2)
    {
      printf("Error: Spline generator ends on an x value.\n");
      printtableproblem(nptr, nsl);    
    }

  if (size > MAXTABLECONSTSIZE)
    return(size);

  vmcheck(*ret = (fptr = (float *) calloc((size+1), sizeof(float))));

  endptr = fptr + size;
  i = 1;
  tptr = nptr;
  oldx = x = -1;
  k = 0;
  while (tptr != NULL)
    {
      switch (i % 3) {
      case 1:
	oldx = x;
	x = tptr->down->arrayidx;
	break;
      case 2:
	if (tptr->next == NULL)
	  k = 0.0F;
	oldy = y;
	y = (float)atof(tptr->down->val);
	if ((i > 2) && (oldx != x))
	  {
	    xf = (float) x;
	    oldxf = (float) oldx;

	    /* compute spline parameters */

	    Q = 1.0F/(oldxf - xf);
	    R = Q*(oldxf*oldxf - xf*xf) - 2.0F*oldxf;
	    R =1.0F/R;
	    S = Q*(oldxf*oldxf - xf*xf) - 2.0F*xf;
	    S =1.0F/S;
	    a = Q*(oldxf*oldxf*oldxf-xf*xf*xf)*(R-S);
	    a += -3.0F*(R*oldxf*oldxf - S*xf*xf);
	    a =1.0F/a;
	    a *= Q*(oldy-y)*(R-S) - R*oldk + S*k;
	    b = Q*(oldk-k)*0.5F;
	    b += - 1.5F*a*Q*(oldxf*oldxf - xf*xf);
	    c = oldk - 3.0F*a*oldxf*oldxf -2.0F*b*oldxf;
	    d = oldy-a*oldxf*oldxf*oldxf-b*oldxf*oldxf-c*oldxf;

	    /* fill table */

	    segptr = fptr + (x - oldx);
	    while ((fptr < endptr) && (fptr < segptr))
	      {
		*(fptr++) = oldxf*oldxf*oldxf*a + oldxf*oldxf*b + oldxf*c + d;
		oldxf = (float) (++oldx);
	      }
	  }
	break;
      case 0:
	oldk = k;
	k = (float)atof(tptr->down->val);
	break;
      }
      i++;
      tptr = tptr->next ? tptr->next->next : NULL;
    }

  (*ret)[size] = (*ret)[0];

  return size;
}


/***************************************************************/
/*  syntax checks and updates constants for step wt generator  */
/***************************************************************/

int stepconstcheck(tnode * tptr, tnode * nsl, int size, float ** ret)

{
  char newval[STRSIZE];
  int i = 1;
  int x = -1;
  int oldx;
  float f = 0.0F;
  tnode * nptr = tptr;
  float * fptr;
  float * endptr;
  float * segptr;

  while (tptr != NULL)
    {
      switch (i & 1) {
      case 1:
	oldx = x;
	x = make_int(tptr->down);
	if (tptr->down->ttype == S_NUMBER)
	  {
	    tptr->res = tptr->down->res = ASINT;
	    tptr->down->ttype = S_INTGR;
	    sprintf(newval,"%i",x);
	    free(tptr->down->val);
	    tptr->down->val = dupval(newval);
	  }
	if (x < oldx)
	  {     
	    printf("Error: Step generator x values decreasing.\n");
	    printtableproblem(tptr, nsl);
	  }	    
	if ((i == 1) && (x != 0))
	  {     
	    printf("Error: Step generator first x value is not 0.\n");
	    printtableproblem(tptr, nsl);
	  }	    
	tptr->down->arrayidx = x;
	break;
      case 0:
	break;
      }
      i++;
      tptr = tptr->next ? tptr->next->next : NULL;
    }

  if (size < 0)
    {
      size = x;
      if (size < 1)
	{     
	  printf("Error: Step generator has zero-sized table.\n");
	  printtableproblem(nptr, nsl);
	}
    }

  if (i & 1)
    {
      printf("Error: Step generator data ends with a y value.\n");
      printtableproblem(nptr, nsl);
    }

  if (size > MAXTABLECONSTSIZE)
    return(size);

  vmcheck(*ret = (fptr = (float *) calloc((size+1), sizeof(float))));

  endptr = fptr + size;
  i = 1;
  tptr = nptr;
  while (tptr != NULL)
    {
      switch (i & 1) {
      case 1:
	if (i > 1)
	  {
	    segptr = *ret + tptr->down->arrayidx;
	    while ((fptr < endptr) && (fptr < segptr))
	      *(fptr++) = f;
	  }
	break;
      case 0:
	f = (float)atof(tptr->down->val);
	break;
      }
      i++;
      tptr = tptr->next ? tptr->next->next : NULL;
    }

  (*ret)[size] = (*ret)[0];

  return size;
}


extern double besselval(double);

/***************************************************************/
/*    syntax checks and updates constants for window wtgen     */
/***************************************************************/

void windowconstcheck(tnode * tptr, tnode * nsl, int size, float ** ret)

{  
  char newval[STRSIZE];
  int i = 1;
  int type = 0;
  float p = 0.0F;
  tnode * nptr = tptr;
  float c1, c2, idx;
  double d1, d2, d3, didx;
  float * fptr, * endptr; 

  while (tptr != NULL)
    {
      switch (i) {
      case 1:
	type = make_int(tptr->down);
	if (tptr->down->ttype == S_NUMBER)
	  {
	    tptr->res = tptr->down->res = ASINT;
	    tptr->down->ttype = S_INTGR;
	    sprintf(newval,"%i",type);
	    free(tptr->down->val);
	    tptr->down->val = dupval(newval);
	  }
	tptr->down->arrayidx = type;
	if ((type < 1) || (type > 6))
	  {		
	    printf("Error: Illegal type %s in window generator.\n", 
		   tptr->down->val);
	    printtableproblem(tptr, nsl);
	  }
	break;
      case 2:
	p = (float)atof(tptr->down->val);
	break;
      default:
	printf("Error: Too many parameters for window generator.\n");
	printtableproblem(tptr, nsl);
	break;
      }
      i++;
      tptr = tptr->next ? tptr->next->next : NULL;
    }

  if (i == 1)
    {
      printf("Error: No type provided for window generator.\n");
      printtableproblem(nptr, nsl);
    }

  if ((i == 2) && (type == 5))
    {
      printf("Error: Window generator type 5 needs p parameter.\n");
      printtableproblem(nptr, nsl);
    }

  if (size > MAXTABLECONSTSIZE)
    return;

  vmcheck((fptr = (*ret = (float *) calloc((size+1), sizeof(float)))));
  endptr = fptr + size;
  idx = 0.0F;

  switch(type) {
  case 1:   /* hamming */
    c1 = 6.283185F/(size-1.0F);
    while (fptr < endptr)
      *(fptr++) = 0.54F - 0.46F*(float)cos(c1*(idx++));
    break;
  case 2:   /* hanning */
    c1 = 6.283185F/(size-1.0F);
    while (fptr < endptr)
      *(fptr++) = 0.5F*(1.0F - (float)cos(c1*(idx++)));
    break;
  case 3:  /* bartlett */
    c1 = 2.0F/(size-1.0F);
    c2 = 1.0F/c1;
    while (fptr < endptr)
      *(fptr++) = 1.0F - c1*(float)fabs((idx++) - c2);
    break;
  case 4:  /* gaussian */
    c1 = - 18.0F/(size*size);
    c2 = - 0.5F*size;
    while (fptr < endptr)
      {
	*(fptr++) = (float)exp(c1*(c2+idx)*(c2+idx));
	idx++;
      }
    break;
  case 5: /* kaiser */
    d1 = (size-1.0)/2.0;
    d2 = d1*d1;
    d3 = 1.0/besselval(p*d1);
    didx = 0.0;
    while (fptr < endptr)
      {
	*(fptr++) = (float)(d3*besselval(p*sqrt(d2-(didx-d1)*(didx-d1))));
	didx++;
      }
    break;
  case 6: /* boxcar */
    while (fptr < endptr)
      *(fptr++) = 1.0F;
    break;
  }

  *fptr = (*ret)[0];

}



/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*                                                              */
/* Second-level functions for constant wavetables code-gen.     */
/*                                                              */
/*______________________________________________________________*/

/****************************************************************/
/* forms full variable name for constant wavetable arrays       */
/****************************************************************/

char * wavetablenameprefix(sigsym * opstack, char * prefix)

{
  char * in;
  char * ret;
  char num[32];

  if (opstack != NULL)
    {
      in = wavetablenameprefix(opstack->next, prefix);
      sprintf(num,"%i", opstack->defnode->arrayidx);
      vmcheck(ret = (char *) calloc((strlen(in) +
					 strlen(opstack->defnode->val) 
					 + strlen(num) + 2), sizeof(char)));
      sprintf(ret,"%s_%s%s", in, opstack->defnode->val, num);
      free(in);
      return ret;
    }
  else
    {
      if (currinstance != NULL)
	{
	  vmcheck(ret = calloc((strlen(currinstance->val) 
				+ strlen(prefix) + 1),
			       sizeof(char)));
	  strcpy(ret, prefix);
	  strcat(ret, currinstance->val);
	  return ret;
	}
      else
	{
	  if (currinstrument != NULL)
	    {
	      vmcheck(ret = calloc((strlen(currinstrument->val) 
				    + strlen(prefix) + 1),
				   sizeof(char)));
	      strcpy(ret, prefix);
	      strcat(ret, currinstrument->val);
	      return ret;
	    }
	  else
	    {
	      vmcheck(ret = calloc((strlen("global") 
				    + strlen(prefix) + 1),
				   sizeof(char)));
	      strcpy(ret, prefix);
	      strcat(ret, "global");
	      return ret;
	    }
	}
    }
}


extern void wt_bitsounderror(tnode *, tnode *);
extern void wt_getsoundbytes(unsigned char *, int, unsigned int *,
			     tnode *, tnode *);
extern void wt_getsoundsbytes(char *, int, unsigned int *,
			      tnode *, tnode *);
extern int wt_getnextcookie(unsigned char *, unsigned int *);
extern void wt_flushsoundbytes(int, unsigned int *, tnode *, tnode * );
extern unsigned int wt_wavefileintval(int, unsigned int *, tnode *, tnode *);
extern unsigned int wt_aiffileintval(int, unsigned int *, tnode *, tnode *);
extern int wt_soundtypecheck(char *, unsigned int *);
 


/****************************************************************/
/*         reads wav file, extracts sample info                 */
/****************************************************************/

void wavefileconstcheck(tnode * tptr, tnode * nsl)

{
  int i;
  sampleinfo * sinfo;
  unsigned int count = 0;
  unsigned char cookie[4];
  int unity = 0;
  int fract = 0;
  int dataflag = 0;
  int channels = 0;
  char * suffix;
  char at_n = -1;
  int ntotal = 0;

  vmcheck(sinfo = (sampleinfo *) calloc(1, sizeof(sampleinfo)));
  tptr->ibus = (tnode *) sinfo;

  sinfo->wav = 1;

  suffix = strrchr(tptr->val, '.');  /* prechecked to exist */

  if (strlen(suffix) <= 4)          /* if true, prechecked == 4 */
    soundfile = fopen(tptr->val,"rb");
  else
    {
      if ((sscanf(&(suffix[4]), "@%hhd%n", &at_n, &ntotal) < 1) || 
	  (strlen(suffix) != 4 + ntotal) || (at_n < 0))
	{
	  printf("Error: Syntax error in sample filename (or in @N specifier).\n");
	  printtableproblem(tptr, nsl);
	}
      suffix[4]= '\0';
      soundfile = fopen(tptr->val,"rb");
      suffix[4]= '@';
    }

  /*******************/
  /* read WAV header */
  /*******************/

  if (soundfile == NULL)
    wt_bitsounderror(tptr, nsl);
  if (wt_soundtypecheck("RIFF", &count))
    wt_bitsounderror(tptr, nsl);
  wt_flushsoundbytes(4, &count, tptr, nsl);
  if (wt_soundtypecheck("WAVE", &count))
    wt_bitsounderror(tptr, nsl);
  while (wt_soundtypecheck("fmt ", &count))
    wt_flushsoundbytes(wt_wavefileintval(4, &count, tptr, nsl),
		       &count, tptr, nsl);
  if ((i = wt_wavefileintval(4, &count, tptr, nsl) - 16) < 0)
    {
      /* format chunk size corrupt */
      wt_bitsounderror(tptr, nsl);
    }
  if (wt_wavefileintval(2, &count, tptr, nsl)!= 1) /* PCM wave files = 1 */
    {
      printf("Error: Sfront only handles PCM-format WAV files\n");
      printtableproblem(tptr, nsl);
    }
  channels = wt_wavefileintval(2, &count, tptr, nsl);  /* 1 = mono, 2 = stereo */
  sinfo->srate = wt_wavefileintval(4, &count, tptr, nsl);
  wt_flushsoundbytes(6, &count, tptr, nsl);
  sinfo->numbytes = (char)((wt_wavefileintval(2, &count, tptr, nsl)+7)/8);  
  if ((sinfo->numbytes <= 0) || (sinfo->numbytes > 3)) 
    {
      printf("Error: Sample generator only handles 8/16/24-bit WAV files.\n");
      printtableproblem(tptr, nsl);
    }
  sinfo->framebytes = channels * sinfo->numbytes;

  if (channels == 1)
    sinfo->chanpoint = 0;
  else
    {
      if (at_n >= 0)
	{
	  if (at_n > channels - 1)
	    {
	      printf("Error: Sample generator @%i filename syntax requests\n"
		     "       the %i'th channel of an %i channel WAV file.\n", 
		     at_n, at_n+1, channels);
	      printtableproblem(tptr, nsl);
	    }
	  sinfo->chanpoint = at_n*sinfo->numbytes;
	}
      else
	sinfo->chanpoint = -1;
    }

  if (i)
    wt_flushsoundbytes(i + (i % 2), &count, tptr, nsl);


  /************************************************/
  /* read through chunks, look for data and smpl  */
  /************************************************/


  sinfo->start = sinfo->end = sinfo->hasbase = sinfo->hasloop = 0;
  while (!wt_getnextcookie(cookie, &count))
    {
      if (!strncmp((char *) cookie,"data",4))
	{
	  /*******************************/
	  /* save seek point, flush data */
	  /*******************************/

	  sinfo->len = wt_wavefileintval(4, &count, tptr, nsl);
	  sinfo->point = count;   /* where sa.c file should seek to */
	  wt_flushsoundbytes(sinfo->len + (sinfo->len % 2), &count, tptr, nsl);
	  sinfo->len /= sinfo->framebytes;
	  dataflag = 1;
	}
      else
	{
	  if (!strncmp((char *) cookie,"smpl",4))
	    {

	      /***********************/
	      /* save loop/base info */
	      /***********************/

	      sinfo->hasbase = 1;
	      i = wt_wavefileintval(4, &count, tptr, nsl);

	      /* flush Manu, Prod, Period */ 

	      wt_flushsoundbytes(12, &count, tptr, nsl); 

	      /* save base frequency*/

	      unity = wt_wavefileintval(4, &count, tptr, nsl);
	      fract = wt_wavefileintval(4, &count, tptr, nsl);
	      sinfo->base = (float)(440.0*pow(2.0, 
				((double)unity +
				 ((double)fract/4.295e9) - 69.0)/12.0));

	      /* flush SMPTEs, Loops, Data */ 

	      wt_flushsoundbytes(16, &count, tptr, nsl);   

	      i -= 36;
	      if (i > 0)
		{
		  /* save loop start and end */

		  sinfo->hasloop = 1;
		  wt_flushsoundbytes(8, &count, tptr, nsl); /* ID, Type */ 
		  sinfo->start = wt_wavefileintval(4, &count, tptr, nsl);
		  sinfo->end = wt_wavefileintval(4, &count, tptr, nsl);
		  sinfo->start /= sinfo->framebytes;
		  sinfo->end /= sinfo->framebytes;
		  wt_flushsoundbytes(8, &count, tptr, nsl); /* Frac, count */
		  i -= 24;
		  wt_flushsoundbytes(i + (i % 2), &count, tptr, nsl); /* other loops */ 
		}
	    }
	  else
	    {
	      i = wt_wavefileintval(4, &count, tptr, nsl);
	      wt_flushsoundbytes(i + (i % 2), &count, tptr, nsl);
	    }
	}
    }

  if (dataflag == 0)
    wt_bitsounderror(tptr, nsl);

  fclose(soundfile);
}

/****************************************************************/
/*         reads aif file, extracts sample info                 */
/****************************************************************/

void aiffileconstcheck(tnode * tptr, tnode * nsl)

{
  sampleinfo * sinfo;
  unsigned int i, j, m, e;
  unsigned char cookie[4];  
  int dataflag = 0;
  unsigned int count = 0;
  unsigned int chunksize;
  int unity = 0;
  int fract = 0;
  char c;
  struct aiffmark * mptr = NULL;
  struct aiffmark * mhead = NULL;
  int channels = 0;
  char * suffix;
  char at_n = -1;
  int ntotal = 0;
  int extlen;

  vmcheck(sinfo = (sampleinfo *) calloc(1, sizeof(sampleinfo)));
  tptr->ibus = (tnode *) sinfo;

  suffix = strrchr(tptr->val, '.');  /* prechecked to exist */

  if (strstr(suffix, ".aiff") || strstr(suffix, ".AIFF"))
    extlen = 5;
  else
    extlen = 4;

  if (strlen(suffix) <= extlen)          /* if true, prechecked == 4 */
    soundfile = fopen(tptr->val,"rb");
  else
    {
      if ((sscanf(&(suffix[extlen]), "@%hhd%n", &at_n, &ntotal) < 1) || 
	  (strlen(suffix) != extlen + ntotal) || (at_n < 0))
	{
	  printf("Error: Syntax error in sample filename (or in @N specifier).\n");
	  printtableproblem(tptr, nsl);
	}
      suffix[extlen]= '\0';
      soundfile = fopen(tptr->val,"rb");
      suffix[extlen]= '@';
    }

  /* AIFF preamble */

  if (soundfile == NULL)
    wt_bitsounderror(tptr, nsl);
  if (wt_soundtypecheck("FORM", &count))
    wt_bitsounderror(tptr, nsl);
  wt_flushsoundbytes(4, &count, tptr, nsl);
  if (wt_soundtypecheck("AIFF", &count))
    wt_bitsounderror(tptr, nsl);
  
  /* extract data from COMM, SSND, INST, MARK, skip the rest */
  
  while (!wt_getnextcookie(cookie, &count))
    {
      if (!strncmp((char *) cookie,"COMM",4))
	{
	  chunksize = wt_aiffileintval(4, &count, tptr, nsl);  /* length */
	  channels = wt_aiffileintval(2, &count, tptr, nsl);  /* 1 = mono, 2 = stereo */
	  wt_flushsoundbytes(4, &count, tptr, nsl);       /* frames */

	  sinfo->numbytes = (char)((wt_aiffileintval(2, &count, tptr, nsl)+7)/8);
	  if ((sinfo->numbytes <= 0) || (sinfo->numbytes > 3)) 
	    {
	      printf("Error: Sample generator only handles 8/16/24-bit AIF files.\n");
	      printtableproblem(tptr, nsl);
	    }
	  sinfo->framebytes = channels * sinfo->numbytes;

	  if (channels == 1)
	    sinfo->chanpoint = 0;
	  else
	    {
	      if (at_n >= 0)
		{
		  if (at_n > channels - 1)
		    {
		      printf("Error: Sample generator @%i filename syntax requests\n"
			     "       the %i'th channel of an %i channel AIF file.\n", 
			     at_n, at_n+1, channels);
		      printtableproblem(tptr, nsl);
		    }
		  sinfo->chanpoint = at_n*sinfo->numbytes;
		}
	      else
		sinfo->chanpoint = -1;
	    }

	  e = ((short)wt_aiffileintval(2, &count, tptr, nsl))&0x7FFF;
	  m = wt_aiffileintval(4, &count, tptr, nsl);
	  wt_flushsoundbytes(4, &count, tptr, nsl);
	  sinfo->srate = (unsigned int) (0.5+(m*exp(log(2)*(e - 16414.0F))));
	  if (chunksize > 18)  /* '% 2' flushes pad byte */
	    wt_flushsoundbytes(chunksize - 18 + (chunksize % 2), &count, tptr, nsl);
	  continue;
      }

      if (!strncmp((char *) cookie,"SSND",4))
	{
	  /* sinfo->len scaled by framebytes later */
	  
	  sinfo->len = (wt_aiffileintval(4, &count, tptr, nsl) - 8);
	  wt_flushsoundbytes(8, &count, tptr, nsl);
	  sinfo->point = count;   /* where sa.c file should fseek() to */
	  wt_flushsoundbytes(sinfo->len + (sinfo->len % 2), &count, tptr, nsl);
	  dataflag = 1;
	  continue;
	}

      if (!strncmp((char *) cookie,"INST",4))
	{
	  sinfo->hasbase = 1;
	  chunksize = wt_aiffileintval(4, &count, tptr, nsl);  /* size */
	  unity = wt_aiffileintval(1, &count, tptr, nsl); /* baseNote */
	  wt_getsoundsbytes(&c,1,&count, tptr, nsl); /* sbytes -> signed */
	  fract = c;                                      /* detune */
	  
	  /* flush {lo,hi}{note,vel},gain */
	  
	  wt_flushsoundbytes(6, &count, tptr, nsl);   
	  if (wt_aiffileintval(2, &count, tptr, nsl))
	    sinfo->hasloop = 1;
	  
	  /* start and end ID pointers into MARK chunk */
	  
	  sinfo->start = wt_aiffileintval(2, &count, tptr, nsl); 
	  sinfo->end = wt_aiffileintval(2, &count, tptr, nsl);   
	  wt_flushsoundbytes(6, &count, tptr, nsl);   /* release loop */
	  if (chunksize > 20)
	    wt_flushsoundbytes(chunksize - 20 + (chunksize % 2), &count, tptr, nsl);
	  continue;
	}

      if (!strncmp((char *) cookie,"MARK",4))
	{
	  chunksize = wt_aiffileintval(4, &count, tptr, nsl);  /* size */
	  i = wt_aiffileintval(2, &count, tptr, nsl); /* num markers */
	  chunksize = ((chunksize - 2) >= 0) ? (chunksize - 2) : 0;

	  /* inserts marker positions into mhead list */
	  
	  while (i--)
	    {
	      vmcheck(mptr = (aiffmark *)malloc(sizeof(aiffmark)));
	      mptr->id = (short) 
		wt_aiffileintval(2, &count, tptr, nsl);
	      mptr->position = wt_aiffileintval(4, &count, tptr, nsl);
	      mptr->next = mhead;
	      mhead = mptr;
	      j = wt_aiffileintval(1, &count, tptr, nsl); /* pstring */
	      if (!(j&0x0001)) /* pad byte if even */
		j++;
	      wt_flushsoundbytes(j, &count, tptr, nsl);
	      chunksize = ((chunksize - 7 - j) >= 0) ? (chunksize - 7 - j) : 0;
	    }
	  if (chunksize)
	    wt_flushsoundbytes(chunksize + (chunksize % 2), &count, tptr, nsl);
	  continue;
	}

      chunksize = wt_aiffileintval(4, &count, tptr, nsl);  /* size */
      wt_flushsoundbytes(chunksize + (chunksize % 2), &count, tptr, nsl);
    }

  sinfo->len /= sinfo->framebytes;

  if (sinfo->hasloop)
    {
      j = 0;
      mptr = mhead;
      while (mptr != NULL)
	{
	  if (mptr->id == ((short)(sinfo->start)))
	    {
	      j = 1;
	      sinfo->start = mptr->position;
	      if ((unsigned int)(sinfo->start) == sinfo->len)
		sinfo->start=0;
	      break;
	    }
	  mptr = mptr->next;
	}
      if (!j)
	wt_bitsounderror(tptr, nsl);      /* can't find start */
      j = 0;
      mptr = mhead;
      while (mptr != NULL)
	{
	  if (mptr->id == ((short)(sinfo->end)))
	    {
	      j = 1;
	      sinfo->end = mptr->position;
	      if ((unsigned int)(sinfo->end) == sinfo->len)
		sinfo->end=0;
	      break;
	    }
	  mptr = mptr->next;
	}
      if (!j)
	wt_bitsounderror(tptr, nsl);      /* can't find end */
      if (sinfo->start >= sinfo->end)      /* behavior in AIFF spec   */
	sinfo->hasloop = 0;
    }
  sinfo->base = (float)(440.0*pow(2.0, ((double)unity +
					((double)fract/100.0) - 69.0)/12.0));

  if (dataflag == 0)
    wt_bitsounderror(tptr, nsl);

  mptr = mhead;
  while (mptr)
    {
      mhead = mptr->next;
      free(mptr);
      mptr = mhead;
    }
  
  fclose(soundfile);
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*                                                              */
/*                Utility functions                             */
/*                                                              */
/*______________________________________________________________*/


/*********************************************************/
/*       computes a Bessel function value                */
/*********************************************************/

double besselval(double x)

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


/*********************************************************/
/*              reports sample error                  */
/*********************************************************/

void wt_bitsounderror(tnode * tptr, tnode * nsl)

{
  printf("Error: Problem reading sample generator file.\n");
  printtableproblem(tptr, nsl);
}


/*********************************************************/
/*    gets next block of WAV/AIFF bytes -- unsigned      */
/*********************************************************/

void wt_getsoundbytes(unsigned char * c, int numbytes, 
		      unsigned int * count, tnode * tptr, tnode * nsl)

{
  (*count) += numbytes;

  if ((int)fread(c, sizeof(char), numbytes, soundfile) != numbytes)
    wt_bitsounderror(tptr, nsl);

}

/*********************************************************/
/*    gets next block of WAV/AIFF bytes -- signed        */
/*********************************************************/

void wt_getsoundsbytes(char * c, int numbytes, unsigned int * count,
		       tnode * tptr, tnode * nsl)

{

  (*count) += numbytes;

  if ((int)fread(c, sizeof(char), numbytes, soundfile) != numbytes)
    wt_bitsounderror(tptr, nsl);

}

/*********************************************************/
/*           gets next WAV/AIFF cookie -- 1 if EOF       */
/*********************************************************/

int wt_getnextcookie(unsigned char * c, unsigned int * count)

{
  (*count) += 4;

  if ((int)fread(c, sizeof(char), 4, soundfile) == 4)
    return 0;
  else
    return 1;

}


/*********************************************************/
/*        flushes next block of WAV/AIFF bytes           */
/*********************************************************/

void wt_flushsoundbytes(int numbytes, unsigned int * count,
			tnode * tptr, tnode * nsl)

{
  
  (*count) += numbytes; 

  if (fseek(soundfile, numbytes , SEEK_CUR))
    wt_bitsounderror(tptr, nsl);

}

/*********************************************************/
/*     converts byte stream to an unsigned int          */
/*********************************************************/

unsigned int wt_wavefileintval(int numbytes, unsigned int * count, 
				tnode * tptr, tnode * nsl)

{
  unsigned int ret = 0;
  unsigned char c[4];

  if (numbytes > 4) 
    wt_bitsounderror(tptr, nsl);

  wt_getsoundbytes(&c[0],numbytes, count, tptr, nsl);
  switch (numbytes) {
  case 4:
    ret  =  (unsigned int)c[0];
    ret |=  (unsigned int)c[1] << 8;
    ret |=  (unsigned int)c[2] << 16;
    ret |=  (unsigned int)c[3] << 24;
    break;
  case 3:
    ret  =  (unsigned int)c[0];
    ret |=  (unsigned int)c[1] << 8;
    ret |=  (unsigned int)c[2] << 16;
    break;
  case 2:
    ret  =  (unsigned int)c[0];
    ret |=  (unsigned int)c[1] << 8;
    break;
  case 1:
    ret = ((unsigned int)c[0]);
    break;
  default:
    break;
  }

  return ret;
}
  
/*********************************************************/
/*     converts byte stream to an unsigned int          */
/*********************************************************/

unsigned int wt_aiffileintval(int numbytes, unsigned int * count, 
			       tnode * tptr, tnode * nsl)

{
  unsigned int ret = 0;
  unsigned char c[4];

  if (numbytes > 4)
    wt_bitsounderror(tptr, nsl);
 
  wt_getsoundbytes(&c[0],numbytes, count, tptr, nsl);
  switch (numbytes) {
  case 4:
    ret  =  (unsigned int)c[3];
    ret |=  (unsigned int)c[2] << 8;
    ret |=  (unsigned int)c[1] << 16;
    ret |=  (unsigned int)c[0] << 24;
    break;
  case 3:
    ret  =  (unsigned int)c[2];
    ret |=  (unsigned int)c[1] << 8;
    ret |=  (unsigned int)c[0] << 16;
    break;
  case 2:
    ret  =  (unsigned int)c[1];
    ret |=  (unsigned int)c[0] << 8;
    break;
  case 1:
    ret = ((unsigned int)c[0]);
    break;
  default:
    break;
  }
  return ret;
}
  
/***********************************************************/
/*  checks byte stream for AIFF/WAV cookie --              */
/*    return 0 for found cookie                            */
/*    return 1 for different cookie                        */
/*    return 2 for EOF                                     */
/***********************************************************/

int wt_soundtypecheck(char * d, unsigned int * count) 

{
  int ret = 0;
  char c[4];

  *count += 4;

  if (fread(c, sizeof(char), 4, soundfile) != 4)
    ret = 2;
  else
    {
      if (strncmp(c,d,4))
	ret = 1;
    }
  return ret;
}
  

/*********************************************************/
/*       compactly converts a float to a string          */
/*********************************************************/

char * compactfloat(char * ret, float f)

{
  float fl;

  if ((fl = ((float)floor(f))) == f)
    sprintf(ret,"%i.0F", (int)fl);
  else
    sprintf(ret,"%eF", (double)(f));
  return ret;

}
