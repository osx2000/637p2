
/*
#    Sfront, a SAOL to C translator    
#    This file: Core opcode library
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

/*********************************************************/
/*       checks for virtual memory exhaustion            */
/*********************************************************/

void vmcheck(void * p)

{
  if (p)
    return;

  printf("Sfront Error: Virtual memory space exhausted.\n\n");
  printf("If processing an .mp4 file with a large streaming\n");
  printf("section, use -bitc file.mp4 -cin fstr sfront flags.\n");
  printf("Otherwise, increase swap or memory space on your OS.\n");
  noerrorplace();
}

/*********************************************************/
/*              mallocs space for  z[]                   */
/*********************************************************/

void mz(int lc)

{
  znode * zptr;

  vmcheck(z[lc] = (char *) malloc(1024*sizeof(char)));

  /* add to list of objects to free after printing */

  vmcheck(zptr = malloc(sizeof(znode)));
  zptr->zchar = z[lc];
  zptr->next = zlist;
  zlist = zptr;
  
}


/*********************************************************/
/*              frees dynamic space for z[]              */
/*********************************************************/

void freezlist(void)

{
  znode * zptr;
  znode * ztmp;

  zptr = zlist;
  while (zptr)
    {
      free(zptr->zchar);
      ztmp = zptr->next;
      free(zptr);
      zptr = ztmp;
    }
  zlist = NULL;

}

/*********************************************************/
/*          returns ptr to first opcode/oparray arg      */
/*********************************************************/

tnode * firstopcodearg(tnode * tptr)

{
  tptr = tptr->optr->down;
  while ((tptr != NULL) && (tptr->ttype != S_EXPRLIST))
    tptr= tptr->next;
  if (tptr == NULL)
    internalerror("corecode.c","firstopcodearg");
  return (tptr->down);

}


/*********************************************************/
/*          prints out z[]: for core code body           */
/*********************************************************/

void printbody(int num)

{
   int i, j;

   for (i=0;i<num;i++)
     {
       fputs("   ", outfile);
       j = 0;
       while (z[i][j] != '\0')
	 {
	   if ((z[i][j] == '%')&&(z[i][j+1] == '1') && 
	       (z[i][j+2] == '$')&&(z[i][j+3] == 's'))
	     {
	       fputs(currinstancename, outfile);
	       j += 4;
	     }
	   else
	     fputc(z[i][j++], outfile);
	 }
       fputs("\n", outfile);
     }
   freezlist();

}


/*********************************************************/
/*     prints out correct return variable                */
/*********************************************************/

char * makereturn(void)

{
  char * astr, * prefix, * retstr;

  if (curropcodeinstance->ttype == S_OPARRAYCALL)
    {
      prefix = namingprefix(curropcodestack->next, "");
      vmcheck(astr = calloc(strlen(prefix) + strlen("__return") + 
			      strlen(curropcodestack->defnode->val) + 
			      21, sizeof(char)));
      sprintf(astr, "%s_%s%i_return", prefix, 
	      curropcodestack->defnode->val, 
	      curropcodestack->defnode->arrayidx);
      free(prefix);
      retstr = stackstring(S_NUMBER,
			 curropcodestack->next ? 
			 curropcodestack->next->special : -1, astr);
      free(astr);
    }
  else
    {
      vmcheck(retstr = calloc(strlen(currinstancename) + 
			      strlen("NV(_return)") + 1,
			      sizeof(char)));
      sprintf(retstr, "NV(%s_return)", currinstancename);
    }
  return(retstr);
}


/*********************************************************/
/*          prints out z[]: for core code                */
/*********************************************************/

void printblock(int num)

{
  char * retstr;

  printbody(num);
  fprintf(outfile,"   return((%s = ret));\n", 
	  (retstr = makereturn()));
  free(retstr);
}


/*********************************************************/
/*          prints out z[]: w/o substitutions            */
/*********************************************************/

void printraw(int num)

{
   int i;

   for (i=0;i<num;i++)
     {
       fprintf(outfile, "   ");
       fputs(z[i], outfile);
       fprintf(outfile, "\n");
     }
   freezlist();

}


/*********************************************************/
/*          prints out z[]: w/o leading spaces           */
/*********************************************************/

void printlib(int num)

{
   int i;

   for (i=0;i<num;i++)
     {
       fputs(z[i], outfile);
       fprintf(outfile, "\n");
     }
   freezlist();

}


/*********************************************************/
/*               generates exmessage                     */
/*********************************************************/

void genex(int * lcptr, tnode * tptr, char * message)

{
  int lc = *lcptr;
 
  mz(lc); 
  if (tptr != NULL)
    {
      if (tptr->filename != NULL)
	sprintf(z[lc++],"epr(%i,\"%s\",\"%s\",\"%s\");",tptr->linenum,
		tptr->filename, tptr->val, message);
      else
	sprintf(z[lc++],"epr(0,NULL,\"%s\",\"%s\");",
		tptr->val, message);
    }
  else
    {
	sprintf(z[lc++],"epr(0,NULL,NULL,\"%s\");",
		message);
    }
  *lcptr = lc;

}

/*********************************************************/
/*           generates exmessage directly                */
/*********************************************************/

void gened(tnode * tptr, char * message)

{
 
  if (tptr != NULL)
    {
      if (tptr->filename != NULL)
	fprintf(outfile,"   epr(%i,\"%s\",\"%s\",\"%s\");\n",tptr->linenum,
		tptr->filename, tptr->val, message);
      else
	fprintf(outfile,"   epr(0,NULL,\"%s\",\"%s\");\n",
		tptr->val, message);
    }
  else
    {
	fprintf(outfile,"   epr(0,NULL,NULL,\"%s\");\n",
		message);
    }

}

/*********************************************************/
/*          prints out z[]: for wavetables               */
/*********************************************************/

void printwaveblock(int num, sigsym * ident, char * prefix)

{
   int i, j;
   char name[STRSIZE];

   sprintf(name,"%s_%s",prefix,ident->val);
   for (i=0;i<num;i++)
     {
       fputs("   ", outfile);
       j = 0;
       while (z[i][j] != '\0')
	 {
	   if ((z[i][j] == '%')&&(z[i][j+1] == '1') && 
	       (z[i][j+2] == '$')&&(z[i][j+3] == 's'))
	     {
	       fputs(name, outfile);
	       j += 4;
	     }
	   else
	     {
	       if ((z[i][j] == '%')&&(z[i][j+1] == '2') && 
		   (z[i][j+2] == '$')&&(z[i][j+3] == 's'))
		 {
		   fputs(ident->val, outfile);
		   j += 4;
		 }
	       else
		 fputc(z[i][j++], outfile);
	     }
	 }
       fputs("\n", outfile);
     }
   freezlist();

}

/*********************************************************/
/*   prints out z[]: for writeorc.c control loops        */
/*********************************************************/

void printcontrolblock(int num, char * prefix)

{
   int i, j;

   for (i=0;i<num;i++)
     {
       fputs("   ", outfile);
       j = 0;
       while (z[i][j] != '\0')
	 {
	   if ((z[i][j] == '%')&&(z[i][j+1] == 's'))
	     {
	       fputs(prefix, outfile);
	       j += 2;
	     }
	   else
	     fputc(z[i][j++], outfile);
	 }
       fputs("\n", outfile);
     }
   freezlist();

}

/*********************************************************/
/*          prints out z[]: for wavetables               */
/*********************************************************/

void printwaveblock2(int num, tnode * ident)

{
   int i, j;

   for (i=0;i<num;i++)
     {
       fputs("   ", outfile);
       j = 0;
       while (z[i][j] != '\0')
	 {
	   if ((z[i][j] == '%')&&(z[i][j+1] == '2') && 
	       (z[i][j+2] == '$')&&(z[i][j+3] == 's'))
	     {
	       fputs(ident->val, outfile);
	       j += 4;
	     }
	   else
	     fputc(z[i][j++], outfile);
	 }
       fputs("\n", outfile);
     }
   freezlist();

}

/*********************************************************/
/*  prints out z[]: for wavetables, with alias string    */
/*********************************************************/

void printwavesymblock(int num, sigsym * ident, char * prefix)

{
   int i, j;
   char name[STRSIZE];

   sprintf(name,"%s_%s",prefix,ident->val);
   for (i=0;i<num;i++)
     {
       fputs("   ", outfile);
       j = 0;
       while (z[i][j] != '\0')
	 {
	   if ((z[i][j] == '%')&&(z[i][j+1] == '1') && 
	       (z[i][j+2] == '$')&&(z[i][j+3] == 's'))
	     {
	       fputs(name, outfile);
	       j += 4;
	     }
	   else
	     {
	       if ((z[i][j] == '%')&&(z[i][j+1] == '2') && 
		   (z[i][j+2] == '$')&&(z[i][j+3] == 's'))
		 {
		   fputs(ident->val, outfile);
		   fputs("__sym", outfile);
		   j += 4;
		 }
	       else
		 fputc(z[i][j++], outfile);
	     }
	 }
       fputs("\n", outfile);
     }
   freezlist();

}

/*********************************************************/
/*  prints out z[]: for wavetables, with alias string    */
/*********************************************************/

void printwavesymblock2(int num, tnode * ident)

{
   int i, j;

   for (i=0;i<num;i++)
     {
       fputs("   ", outfile);
       j = 0;
       while (z[i][j] != '\0')
	 {
	   if ((z[i][j] == '%')&&(z[i][j+1] == '2') && 
	       (z[i][j+2] == '$')&&(z[i][j+3] == 's'))
	     {
	       fputs(ident->val, outfile);
	       fputs("__sym", outfile);
	       j += 4;
	     }
	   else
	     fputc(z[i][j++], outfile);
	 }
       fputs("\n", outfile);
     }
   freezlist();

}

/*********************************************************/
/*     prints out "do if not first apass in acycle" if   */
/*********************************************************/

void acycleguard(tnode * tptr, int * lcptr)

{
  sigsym * sptr = tptr->optr->sptr;
  int lc = *lcptr;

  if ((sptr->cref->callif == 0) &&
      (sptr->cref->callwhile == 0))
    {
      z[lc++]="if (EV(acycleidx))"; 
    }
  else
    {
      z[lc++]="if (NVI(%1$s_kcyc) == EV(kcycleidx))";
    }
  *lcptr = lc;
  return;

}

/*********************************************************/
/*     prints out "do if first apass in acycle" if       */
/*********************************************************/

void acycleguard2(tnode * tptr, int * lcptr)

{
  sigsym * sptr = tptr->optr->sptr;
  int lc = *lcptr;

  if ((sptr->cref->callif == 0) &&
      (sptr->cref->callwhile == 0))
    {
      z[lc++]="if (!EV(acycleidx))"; 
    }
  else
    {
      z[lc++]="if (NVI(%1$s_kcyc) != EV(kcycleidx))";
    }
  *lcptr = lc;
  return;

}

/*********************************************************/
/*     prints out most efficient NVI((%1$s_kcyc) assign  */
/*********************************************************/

void kcycassign(tnode * tptr, int * lcptr)

{
  sigsym * sptr = tptr->optr->sptr;
  int lc = *lcptr;

  if ((sptr->cref->callif == 0) &&
      (sptr->cref->callwhile == 0))
    {
      z[lc++]="    NVI(%1$s_kcyc) = 1;";
    }
  else
    {
      z[lc++]="     NVI(%1$s_kcyc) = EV(kcycleidx);";
    }
  *lcptr = lc;
  return;

}

/*********************************************************/
/*     prints out most efficient NVI((%1$s_kcyc) assign  */
/*********************************************************/

void kcycassign2(tnode * tptr, int * lcptr)

{
  sigsym * sptr = tptr->optr->sptr;
  int lc = *lcptr;

  if ((sptr->cref->callif != 0) ||
      (sptr->cref->callwhile != 0))
    {
      z[lc++]="     NVI(%1$s_kcyc) = EV(kcycleidx);";
    }
  *lcptr = lc;
  return;

}


/*********************************************************/
/*      prints out simple math op code                   */
/*********************************************************/

void mathopscode(char * opname)

{
  char * retstr;

  fprintf(outfile,"   %s = ret = (float)%s((double)x);\n",
	  (retstr = makereturn()), opname);
  fprintf(outfile,"   return(ret);\n");
  free(retstr);

}

/*********************************************************/
/*      prints out min or max op code                    */
/*********************************************************/

void minmaxcode(tnode * tptr, char c)

{
  char * retstr;

  tptr = tptr->extra;
  while (tptr != NULL)
    {
      if (tptr->ttype == S_PARAMDECL)
	fprintf(outfile, "   x1 = (x1 %c va_%s) ? x1 : va_%s;\n",
		c,tptr->down->next->down->val,tptr->down->next->down->val);
      tptr = tptr->next;
    }
  fprintf(outfile,"   return(%s = x1);\n", (retstr = makereturn()));
  free(retstr);

}


/*********************************************************/
/*        code for koscil opcode (w/o loops)             */
/*********************************************************/

void noloopkoscil_sinc(tnode * tptr)

{
  int lc = 0;

  z[lc++]="if (NVI(%1$s_first))";
  z[lc++]="{";
  z[lc++]="  if (freq >= 0)";  /* positive frequency */
  z[lc++]="   {";
  z[lc++]="    nint = (unsigned int)(index = freq*NV(%1$s_kconst));";
  z[lc++]="    while (index >= AP1.lenf)";
  z[lc++]="      nint = (unsigned int)(index -= AP1.lenf);";
  z[lc++]="    nfrac = (unsigned int)(4294967296.0F*(index - nint));";
  z[lc++]="    NVUI(%1$s_pfrac) = (j = NVUI(%1$s_pfrac)) + nfrac;";
  z[lc++]="    NVUI(%1$s_pint) += nint + (NVUI(%1$s_pfrac) < j);";
  z[lc++]="    if ((i = NVUI(%1$s_pint)) >= AP1.len)";
  z[lc++]="      i = (NVUI(%1$s_pint) -= AP1.len);";
  z[lc++]="   }";
  z[lc++]="  else";   /* negative frequency */
  z[lc++]="   {";
  z[lc++]="    nint = (unsigned int)(index = -freq*NV(%1$s_kconst));";
  z[lc++]="    while (index >= AP1.lenf)";
  z[lc++]="      nint = (unsigned int)(index -= AP1.lenf);";
  z[lc++]="    nfrac = (unsigned int)(4294967296.0F*(index - nint));";
  z[lc++]="    NVUI(%1$s_pfrac) = (j = NVUI(%1$s_pfrac)) - nfrac;";
  z[lc++]="    NVUI(%1$s_pint) -= nint + (NVUI(%1$s_pfrac) > j);";
  z[lc++]="    if ((i = NVUI(%1$s_pint)) >= AP1.len)";
  z[lc++]="      i = (NVUI(%1$s_pint) += AP1.len);";
  z[lc++]="   }";
  z[lc++]="  if ((nint == 1) && (nfrac == 0))";
  z[lc++]="    ret = AP1.t[i];";
  z[lc++]="  else";
  z[lc++]="   {";
  z[lc++]="    if (index <= 1.0F)";
  z[lc++]="     {";
  z[lc++]="      sffl = 1.0F;";
  z[lc++]="      sfui = 0x00010000;";
  z[lc++]="      kosincr = SINC_PILEN;";
  z[lc++]="     }";
  z[lc++]="    else";
  z[lc++]="     {";
  z[lc++]="       if (index < SINC_UPMAX)";
  z[lc++]="         sffl = 1.0F/index;";
  z[lc++]="       else";
  z[lc++]="         sffl = 1.0F/SINC_UPMAX;";
  z[lc++]="      sfui = ((float)(pow(2,16)))*sffl + 0.5F;";
  z[lc++]="      kosincr = (SINC_PILEN*sfui) >> 16;";
  z[lc++]="     }";
  z[lc++]="    if (sfui == 0x00010000)";
  z[lc++]="     fptr = (NVUI(%1$s_pfrac)) >> (32 - SINC_LOG_PILEN);";
  z[lc++]="    else";
  z[lc++]="     fptr = (sfui*((NVUI(%1$s_pfrac)) >> 16)) >> (32 - SINC_LOG_PILEN);";
  z[lc++]="    ret = sinc[fptr]*AP1.t[i];";
  z[lc++]="    rptr = kosincr - fptr;";
  z[lc++]="    fptr += kosincr;";
  z[lc++]="    k = 0;";
  z[lc++]="    while (rptr < SINC_SIZE)";
  z[lc++]="     {";
  z[lc++]="       if ((i + (++k)) < AP1.len)";
  z[lc++]="         ret += sinc[rptr]*AP1.t[i + k];";
  z[lc++]="       else";
  z[lc++]="         ret += sinc[rptr]*AP1.t[i + k - AP1.len];";
  z[lc++]="       rptr += kosincr;";
  z[lc++]="       if (fptr < SINC_SIZE)";
  z[lc++]="        {";
  z[lc++]="          if (i >= k)";
  z[lc++]="            ret += sinc[fptr]*AP1.t[i - k];";
  z[lc++]="          else";
  z[lc++]="            ret += sinc[fptr]*AP1.t[AP1.len + i - k];";
  z[lc++]="          fptr += kosincr;";
  z[lc++]="        }";
  z[lc++]="     }";
  z[lc++]="    if (sfui != 0x00010000)";
  z[lc++]="      ret *= sffl;";
  z[lc++]="   }";
  z[lc++]="}";
  z[lc++]="else";
  z[lc++]="{";
  z[lc++]="   NVI(%1$s_first) = 1;";
  z[lc++]="   NV(%1$s_kconst) = AP1.len*EV(KTIME);";
  z[lc++]="   if (freq >= 0)";  /* positive frequency */
  z[lc++]="    {";
  z[lc++]="     nint = (unsigned int)(index = freq*NV(%1$s_kconst));";
  z[lc++]="     while (index >= AP1.lenf)";
  z[lc++]="       nint = (unsigned int)(index -= AP1.lenf);";
  z[lc++]="     nfrac = (unsigned int)(4294967296.0F*(index - nint));";
  z[lc++]="    }";
  z[lc++]="   else";   /* negative frequency */
  z[lc++]="    {";
  z[lc++]="     nint = (unsigned int)(index = -freq*NV(%1$s_kconst));";
  z[lc++]="     while (index >= AP1.lenf)";
  z[lc++]="       nint = (unsigned int)(index -= AP1.lenf);";
  z[lc++]="     nfrac = (unsigned int)(4294967296.0F*(index - nint));";
  z[lc++]="    }";
  z[lc++]="   ret = AP1.t[0];";
  z[lc++]="   if (index > 1.0F)";
  z[lc++]="    {";
  z[lc++]="     if (index < SINC_UPMAX)";
  z[lc++]="       sffl = 1.0F/index;";
  z[lc++]="     else";
  z[lc++]="       sffl = 1.0F/SINC_UPMAX;";
  z[lc++]="    sfui = ((float)(pow(2,16)))*sffl + 0.5F;";
  z[lc++]="    kosincr = (SINC_PILEN*sfui) >> 16;";
  z[lc++]="    fptr = rptr = kosincr;";
  z[lc++]="    k = 0;";
  z[lc++]="    while (rptr < SINC_SIZE)";
  z[lc++]="     {";
  z[lc++]="       if ((++k) < AP1.len)";
  z[lc++]="         ret += sinc[rptr]*AP1.t[k];";
  z[lc++]="       else";
  z[lc++]="         ret += sinc[rptr]*AP1.t[k - AP1.len];";
  z[lc++]="       rptr += kosincr;";
  z[lc++]="       if (fptr < SINC_SIZE)";
  z[lc++]="        {";
  z[lc++]="          ret += sinc[fptr]*AP1.t[AP1.len - k];";
  z[lc++]="          fptr += kosincr;"; 
  z[lc++]="        }";
  z[lc++]="     }";
  z[lc++]="    ret *= sffl;";
  z[lc++]="   }";
  z[lc++]="}";
  printblock(lc);
}

/*********************************************************/
/*        code for koscil opcode (w/o loops)             */
/*********************************************************/

void noloopkoscil(tnode * tptr)

{
  int lc = 0;

  z[lc++]="if (NVI(%1$s_first))";
  z[lc++]="{";
  z[lc++]="  if (freq >= 0)";  /* positive frequency */
  z[lc++]="   {";
  z[lc++]="    nint = (unsigned int)(index = freq*NV(%1$s_kconst));";
  z[lc++]="    while (index >= AP1.lenf)";
  z[lc++]="      nint = (unsigned int)(index -= AP1.lenf);";
  z[lc++]="    nfrac = (unsigned int)(4294967296.0F*(index - nint));";
  z[lc++]="    NVUI(%1$s_pfrac) = (j = NVUI(%1$s_pfrac)) + nfrac;";
  z[lc++]="    NVUI(%1$s_pint) += nint + (NVUI(%1$s_pfrac) < j);";
  z[lc++]="    if ((i = NVUI(%1$s_pint)) >= AP1.len)";
  z[lc++]="      i = (NVUI(%1$s_pint) -= AP1.len);";
  z[lc++]="   }";
  z[lc++]="  else";   /* negative frequency */
  z[lc++]="   {";
  z[lc++]="    nint = (unsigned int)(index = -freq*NV(%1$s_kconst));";
  z[lc++]="    while (index >= AP1.lenf)";
  z[lc++]="      nint = (unsigned int)(index -= AP1.lenf);";
  z[lc++]="    nfrac = (unsigned int)(4294967296.0F*(index - nint));";
  z[lc++]="    NVUI(%1$s_pfrac) = (j = NVUI(%1$s_pfrac)) - nfrac;";
  z[lc++]="    NVUI(%1$s_pint) -= nint + (NVUI(%1$s_pfrac) > j);";
  z[lc++]="    if ((i = NVUI(%1$s_pint)) >= AP1.len)";
  z[lc++]="      i = (NVUI(%1$s_pint) += AP1.len);";
  z[lc++]="   }";
  z[lc++]="  ret = AP1.t[i] + NVUI(%1$s_pfrac)*";
  z[lc++]="        ((float)(1.0/4294967296.0))*(AP1.t[i+1] - AP1.t[i]);";
  z[lc++]="}";
  z[lc++]="else";
  z[lc++]=" {";
  z[lc++]="   NVI(%1$s_first) = 1;";
  z[lc++]="   NV(%1$s_kconst) = AP1.len*EV(KTIME);";
  z[lc++]="   ret = AP1.t[0];";
  z[lc++]="  }";
  printblock(lc);
}

/*********************************************************/
/*        code for koscil opcode -- with loop count      */
/*********************************************************/

void loopkoscil_sinc(tnode * tptr)

{
  int lc = 0;

  z[lc++]="if (NVI(%1$s_first))";   /* fast path */
  z[lc++]="{";
  z[lc++]="  if (freq >= 0)";  /* positive frequency */
  z[lc++]="   {";
  z[lc++]="    nint = (unsigned int)(index = freq*NV(%1$s_kconst));";
  z[lc++]="    while (index >= AP1.lenf)";
  z[lc++]="      nint = (unsigned int)(index -= AP1.lenf);";
  z[lc++]="    nfrac = (unsigned int)(4294967296.0F*(index - nint));";
  z[lc++]="    NVUI(%1$s_pfrac) = (j = NVUI(%1$s_pfrac)) + nfrac;";
  z[lc++]="    NVUI(%1$s_pint) += nint + (NVUI(%1$s_pfrac) < j);";
  z[lc++]="    if ((i = NVUI(%1$s_pint)) >= AP1.len)";
  z[lc++]="     {";
  z[lc++]="      i = (NVUI(%1$s_pint) -= AP1.len);";
  z[lc++]="      if (NVI(%1$s_iloops) > 0)";
  z[lc++]="        NVI(%1$s_iloops)--;";
  z[lc++]="     }";
  z[lc++]="   }";
  z[lc++]="  else";   /* negative frequency */
  z[lc++]="   {";
  z[lc++]="    nint = (unsigned int)(index = -freq*NV(%1$s_kconst));";
  z[lc++]="    while (index >= AP1.lenf)";
  z[lc++]="      nint = (unsigned int)(index -= AP1.lenf);";
  z[lc++]="    nfrac = (unsigned int)(4294967296.0F*(index - nint));";
  z[lc++]="    NVUI(%1$s_pfrac) = (j = NVUI(%1$s_pfrac)) - nfrac;";
  z[lc++]="    NVUI(%1$s_pint) -= nint + (NVUI(%1$s_pfrac) > j);";
  z[lc++]="    if ((i = NVUI(%1$s_pint)) >= AP1.len)";
  z[lc++]="     {";
  z[lc++]="       i = (NVUI(%1$s_pint) += AP1.len);";
  z[lc++]="       if (NVI(%1$s_iloops) > 0)";
  z[lc++]="         NVI(%1$s_iloops)++;";
  z[lc++]="     }";
  z[lc++]="   }";
  z[lc++]="  if (NVI(%1$s_iloops) == 0)";
  z[lc++]="    ret = 0.0F;";
  z[lc++]="  else";
  z[lc++]="   {";
  z[lc++]="    if ((nint == 1) && (nfrac == 0))";
  z[lc++]="      ret = AP1.t[i];";
  z[lc++]="    else";
  z[lc++]="     {";
  z[lc++]="      if (index <= 1.0F)";
  z[lc++]="       {";
  z[lc++]="        sffl = 1.0F;";
  z[lc++]="        sfui = 0x00010000;";
  z[lc++]="        kosincr = SINC_PILEN;";
  z[lc++]="       }";
  z[lc++]="      else";
  z[lc++]="       {";
  z[lc++]="         if (index < SINC_UPMAX)";
  z[lc++]="           sffl = 1.0F/index;";
  z[lc++]="         else";
  z[lc++]="           sffl = 1.0F/SINC_UPMAX;";
  z[lc++]="        sfui = ((float)(pow(2,16)))*sffl + 0.5F;";
  z[lc++]="        kosincr = (SINC_PILEN*sfui) >> 16;";
  z[lc++]="       }";
  z[lc++]="      if (sfui == 0x00010000)";
  z[lc++]="       fptr = (NVUI(%1$s_pfrac)) >> (32 - SINC_LOG_PILEN);";
  z[lc++]="      else";
  z[lc++]="       fptr = (sfui*((NVUI(%1$s_pfrac)) >> 16)) >> (32 - SINC_LOG_PILEN);";
  z[lc++]="      ret = sinc[fptr]*AP1.t[i];";
  z[lc++]="      rptr = kosincr - fptr;";
  z[lc++]="      fptr += kosincr;";
  z[lc++]="      k = 0;";
  z[lc++]="      while (rptr < SINC_SIZE)";
  z[lc++]="       {";
  z[lc++]="         if ((i + (++k)) < AP1.len)";
  z[lc++]="           ret += sinc[rptr]*AP1.t[i + k];";
  z[lc++]="         else";
  z[lc++]="           ret += sinc[rptr]*AP1.t[i + k - AP1.len];";
  z[lc++]="         rptr += kosincr;";
  z[lc++]="         if (fptr < SINC_SIZE)";
  z[lc++]="          {";
  z[lc++]="            if (i >= k)";
  z[lc++]="              ret += sinc[fptr]*AP1.t[i - k];";
  z[lc++]="            else";
  z[lc++]="              ret += sinc[fptr]*AP1.t[AP1.len + i - k];";
  z[lc++]="            fptr += kosincr;";
  z[lc++]="          }";
  z[lc++]="       }";
  z[lc++]="      if (sfui != 0x00010000)";
  z[lc++]="        ret *= sffl;";
  z[lc++]="     }";
  z[lc++]="   }";
  z[lc++]="}";
  z[lc++]="else";             /* for first time through */ 
  z[lc++]="{";
  z[lc++]="  NVI(%1$s_first) = 1;";
  z[lc++]="  NVI(%1$s_iloops)= (int)(ROUND(va_loops));";
  z[lc++]="  NV(%1$s_kconst) = AP1.len*EV(KTIME);";
  z[lc++]="  if (freq >= 0)";  /* positive frequency */
  z[lc++]="   {";
  z[lc++]="    nint = (unsigned int)(index = freq*NV(%1$s_kconst));";
  z[lc++]="    while (index >= AP1.lenf)";
  z[lc++]="      nint = (unsigned int)(index -= AP1.lenf);";
  z[lc++]="    nfrac = (unsigned int)(4294967296.0F*(index - nint));";
  z[lc++]="   }";
  z[lc++]="  else";   /* negative frequency */
  z[lc++]="   {";
  z[lc++]="    nint = (unsigned int)(index = -freq*NV(%1$s_kconst));";
  z[lc++]="    while (index >= AP1.lenf)";
  z[lc++]="      nint = (unsigned int)(index -= AP1.lenf);";
  z[lc++]="    nfrac = (unsigned int)(4294967296.0F*(index - nint));";
  z[lc++]="   }";
  z[lc++]="  ret = AP1.t[0];";
  z[lc++]="  if (index > 1.0F))";
  z[lc++]="   {";
  z[lc++]="     if (index < SINC_UPMAX)";
  z[lc++]="       sffl = 1.0F/index;";
  z[lc++]="     else";
  z[lc++]="       sffl = 1.0F/SINC_UPMAX;";
  z[lc++]="    sfui = ((float)(pow(2,16)))*sffl + 0.5F;";
  z[lc++]="    kosincr = (SINC_PILEN*sfui) >> 16;";
  z[lc++]="    fptr = rptr = kosincr;";
  z[lc++]="    k = 0;";
  z[lc++]="    while (rptr < SINC_SIZE)";
  z[lc++]="     {";
  z[lc++]="       if ((++k) < AP1.len)";
  z[lc++]="         ret += sinc[rptr]*AP1.t[k];";
  z[lc++]="       else";
  z[lc++]="         ret += sinc[rptr]*AP1.t[k - AP1.len];";
  z[lc++]="       rptr += kosincr;";
  z[lc++]="       if (fptr < SINC_SIZE)";
  z[lc++]="        {";
  z[lc++]="          ret += sinc[fptr]*AP1.t[AP1.len - k];";
  z[lc++]="          fptr += kosincr;";  
  z[lc++]="        }";
  z[lc++]="     }";
  z[lc++]="    ret *= sffl;";
  z[lc++]="   }";
  z[lc++]="}";
  printblock(lc);
}

/*********************************************************/
/*        code for koscil opcode -- with loop count      */
/*********************************************************/

void loopkoscil(tnode * tptr)

{
  int lc = 0;

  z[lc++]="if (NVI(%1$s_first))";   /* fast path */
  z[lc++]="{";
  z[lc++]="  if (freq >= 0)";  /* positive frequency */
  z[lc++]="   {";
  z[lc++]="    nint = (unsigned int)(index = freq*NV(%1$s_kconst));";
  z[lc++]="    while (index >= AP1.lenf)";
  z[lc++]="      nint = (unsigned int)(index -= AP1.lenf);";
  z[lc++]="    nfrac = (unsigned int)(4294967296.0F*(index - nint));";
  z[lc++]="    NVUI(%1$s_pfrac) = (j = NVUI(%1$s_pfrac)) + nfrac;";
  z[lc++]="    NVUI(%1$s_pint) += nint + (NVUI(%1$s_pfrac) < j);";
  z[lc++]="    if ((i = NVUI(%1$s_pint)) >= AP1.len)";
  z[lc++]="     {";
  z[lc++]="      i = (NVUI(%1$s_pint) -= AP1.len);";
  z[lc++]="      if (NVI(%1$s_iloops) > 0)";
  z[lc++]="        NVI(%1$s_iloops)--;";
  z[lc++]="     }";
  z[lc++]="   }";
  z[lc++]="  else";   /* negative frequency */
  z[lc++]="   {";
  z[lc++]="    nint = (unsigned int)(index = -freq*NV(%1$s_kconst));";
  z[lc++]="    while (index >= AP1.lenf)";
  z[lc++]="      nint = (unsigned int)(index -= AP1.lenf);";
  z[lc++]="    nfrac = (unsigned int)(4294967296.0F*(index - nint));";
  z[lc++]="    NVUI(%1$s_pfrac) = (j = NVUI(%1$s_pfrac)) - nfrac;";
  z[lc++]="    NVUI(%1$s_pint) -= nint + (NVUI(%1$s_pfrac) > j);";
  z[lc++]="    if ((i = NVUI(%1$s_pint)) >= AP1.len)";
  z[lc++]="     {";
  z[lc++]="       i = (NVUI(%1$s_pint) += AP1.len);";
  z[lc++]="       if (NVI(%1$s_iloops) > 0)";
  z[lc++]="         NVI(%1$s_iloops)++;";
  z[lc++]="     }";
  z[lc++]="   }";
  z[lc++]="  if (NVI(%1$s_iloops) == 0)";
  z[lc++]="    ret = 0.0F;";
  z[lc++]="  else";
  z[lc++]="    ret = AP1.t[i] + NVUI(%1$s_pfrac)*";
  z[lc++]="        ((float)(1.0/4294967296.0))*(AP1.t[i+1] - AP1.t[i]);";
  z[lc++]="}";
  z[lc++]="else";             /* for first time through */ 
  z[lc++]="{";
  z[lc++]="  NVI(%1$s_first) = 1;";
  z[lc++]="  NVI(%1$s_iloops)= (int)(ROUND(va_loops));";
  z[lc++]="  NV(%1$s_kconst) = AP1.len*EV(KTIME);";
  z[lc++]="  ret = AP1.t[0];";
  z[lc++]="}";
  printblock(lc);
}


/*********************************************************/
/*  oscil() code for special case of no loops, a-rate f  */
/*********************************************************/

void nolooposcilafreq_sinc(tnode * tptr)

{
  int lc = 0;

  z[lc++]="if (NVI(%1$s_kcyc))";   /* fast path */
  z[lc++]="{";
  z[lc++]="  if (freq >= 0)";  /* positive frequency */
  z[lc++]="   {";
  z[lc++]="    nint = (unsigned int)(index = freq*AP1.oconst);";
  z[lc++]="    while (index >= AP1.lenf)";
  z[lc++]="      nint = (unsigned int)(index -= AP1.lenf);";
  z[lc++]="    nfrac = (unsigned int)(4294967296.0F*(index - nint));";
  z[lc++]="    NVUI(%1$s_pfrac) = (j = NVUI(%1$s_pfrac)) + nfrac;";
  z[lc++]="    NVUI(%1$s_pint) += nint + (NVUI(%1$s_pfrac) < j);";
  z[lc++]="    if ((i = NVUI(%1$s_pint)) >= AP1.len)";
  z[lc++]="      i = (NVUI(%1$s_pint) -= AP1.len);";
  z[lc++]="   }";
  z[lc++]="  else";   /* negative frequency */
  z[lc++]="   {";
  z[lc++]="    nint = (unsigned int)(index = -freq*AP1.oconst);";
  z[lc++]="    while (index >= AP1.lenf)";
  z[lc++]="      nint = (unsigned int)(index -= AP1.lenf);";
  z[lc++]="    nfrac = (unsigned int)(4294967296.0F*(index - nint));";
  z[lc++]="    NVUI(%1$s_pfrac) = (j = NVUI(%1$s_pfrac)) - nfrac;";
  z[lc++]="    NVUI(%1$s_pint) -= nint + (NVUI(%1$s_pfrac) > j);";
  z[lc++]="    if ((i = NVUI(%1$s_pint)) >= AP1.len)";
  z[lc++]="      i = (NVUI(%1$s_pint) += AP1.len);";
  z[lc++]="   }";
  z[lc++]="  if ((nint == 1) && (nfrac == 0))";
  z[lc++]="    ret = AP1.t[i];";
  z[lc++]="  else";
  z[lc++]="   {";
  z[lc++]="    if (index <= 1.0F)";
  z[lc++]="     {";
  z[lc++]="      sffl = 1.0F;";
  z[lc++]="      sfui = 0x00010000;";
  z[lc++]="      osincr = SINC_PILEN;";
  z[lc++]="     }";
  z[lc++]="    else";
  z[lc++]="     {";
  z[lc++]="       if (index < SINC_UPMAX)";
  z[lc++]="         sffl = 1.0F/index;";
  z[lc++]="       else";
  z[lc++]="         sffl = 1.0F/SINC_UPMAX;";
  z[lc++]="      sfui = ((float)(pow(2,16)))*sffl + 0.5F;";
  z[lc++]="      osincr = (SINC_PILEN*sfui) >> 16;";
  z[lc++]="     }";
  z[lc++]="    if (sfui == 0x00010000)";
  z[lc++]="     fptr = (NVUI(%1$s_pfrac)) >> (32 - SINC_LOG_PILEN);";
  z[lc++]="    else";
  z[lc++]="     fptr = (sfui*((NVUI(%1$s_pfrac)) >> 16)) >> (32 - SINC_LOG_PILEN);";
  z[lc++]="    ret = sinc[fptr]*AP1.t[i];";
  z[lc++]="    rptr = osincr - fptr;";
  z[lc++]="    fptr += osincr;";
  z[lc++]="    k = 0;";
  z[lc++]="    while (rptr < SINC_SIZE)";
  z[lc++]="     {";
  z[lc++]="       if ((i + (++k)) < AP1.len)";
  z[lc++]="         ret += sinc[rptr]*AP1.t[i + k];";
  z[lc++]="       else";
  z[lc++]="         ret += sinc[rptr]*AP1.t[i + k - AP1.len];";
  z[lc++]="       rptr += osincr;";
  z[lc++]="       if (fptr < SINC_SIZE)";
  z[lc++]="        {";
  z[lc++]="          if (i >= k)";
  z[lc++]="            ret += sinc[fptr]*AP1.t[i - k];";
  z[lc++]="          else";
  z[lc++]="            ret += sinc[fptr]*AP1.t[AP1.len + i - k];";
  z[lc++]="          fptr += osincr;";
  z[lc++]="        }";
  z[lc++]="     }";
  z[lc++]="    if (sfui != 0x00010000)";
  z[lc++]="      ret *= sffl;";
  z[lc++]="   }";
  z[lc++]="}";
  z[lc++]="else";             /* for first time through */ 
  z[lc++]="{";
  z[lc++]="  if (freq >= 0)";  /* positive frequency */
  z[lc++]="   {";
  z[lc++]="    nint = (unsigned int)(index = freq*AP1.oconst);";
  z[lc++]="    while (index >= AP1.lenf)";
  z[lc++]="      nint = (unsigned int)(index -= AP1.lenf);";
  z[lc++]="    nfrac = (unsigned int)(4294967296.0F*(index - nint));";
  z[lc++]="   }";
  z[lc++]="  else";   /* negative frequency */
  z[lc++]="   {";
  z[lc++]="    nint = (unsigned int)(index = -freq*AP1.oconst);";
  z[lc++]="    while (index >= AP1.lenf)";
  z[lc++]="      nint = (unsigned int)(index -= AP1.lenf);";
  z[lc++]="    nfrac = (unsigned int)(4294967296.0F*(index - nint));";
  z[lc++]="   }";
  z[lc++]="  ret = AP1.t[0];";
  z[lc++]="  if (index > 1.0F)";
  z[lc++]="   {";
  z[lc++]="     if (index < SINC_UPMAX)";
  z[lc++]="       sffl = 1.0F/index;";
  z[lc++]="     else";
  z[lc++]="       sffl = 1.0F/SINC_UPMAX;";
  z[lc++]="    sfui = ((float)(pow(2,16)))*sffl + 0.5F;";
  z[lc++]="    osincr = (SINC_PILEN*sfui) >> 16;";
  z[lc++]="    fptr = rptr = osincr;";
  z[lc++]="    k = 0;";
  z[lc++]="    while (rptr < SINC_SIZE)";
  z[lc++]="     {";
  z[lc++]="       if ((++k) < AP1.len)";
  z[lc++]="         ret += sinc[rptr]*AP1.t[k];";
  z[lc++]="       else";
  z[lc++]="         ret += sinc[rptr]*AP1.t[k - AP1.len];";
  z[lc++]="       rptr += osincr;";
  z[lc++]="       if (fptr < SINC_SIZE)";
  z[lc++]="        {";
  z[lc++]="          ret += sinc[fptr]*AP1.t[AP1.len - k];";
  z[lc++]="          fptr += osincr;";
  z[lc++]="        }";
  z[lc++]="     }";
  z[lc++]="    ret *= sffl;";
  z[lc++]="   }";
  z[lc++]="  NVI(%1$s_kcyc) = 1;";
  z[lc++]="}";
  printblock(lc);
}


/*********************************************************/
/*  oscil() code for special case of no loops, a-rate f  */
/*********************************************************/

void nolooposcilafreq(tnode * tptr)

{
  int lc = 0;

  z[lc++]="if (NVI(%1$s_kcyc))";   /* fast path */
  z[lc++]="{";
  z[lc++]="  if (freq >= 0)";  /* positive frequency */
  z[lc++]="   {";
  z[lc++]="    nint = (unsigned int)(index = freq*AP1.oconst);";
  z[lc++]="    while (index >= AP1.lenf)";
  z[lc++]="      nint = (unsigned int)(index -= AP1.lenf);";
  z[lc++]="    nfrac = (unsigned int)(4294967296.0F*(index - nint));";
  z[lc++]="    NVUI(%1$s_pfrac) = (j = NVUI(%1$s_pfrac)) + nfrac;";
  z[lc++]="    NVUI(%1$s_pint) += nint + (NVUI(%1$s_pfrac) < j);";
  z[lc++]="    if ((i = NVUI(%1$s_pint)) >= AP1.len)";
  z[lc++]="      i = (NVUI(%1$s_pint) -= AP1.len);";
  z[lc++]="   }";
  z[lc++]="  else";   /* negative frequency */
  z[lc++]="   {";
  z[lc++]="    nint = (unsigned int)(index = -freq*AP1.oconst);";
  z[lc++]="    while (index >= AP1.lenf)";
  z[lc++]="      nint = (unsigned int)(index -= AP1.lenf);";
  z[lc++]="    nfrac = (unsigned int)(4294967296.0F*(index - nint));";
  z[lc++]="    NVUI(%1$s_pfrac) = (j = NVUI(%1$s_pfrac)) - nfrac;";
  z[lc++]="    NVUI(%1$s_pint) -= nint + (NVUI(%1$s_pfrac) > j);";
  z[lc++]="    if ((i = NVUI(%1$s_pint)) >= AP1.len)";
  z[lc++]="      i = (NVUI(%1$s_pint) += AP1.len);";
  z[lc++]="   }";
  z[lc++]="  ret = AP1.t[i] + NVUI(%1$s_pfrac)*";
  z[lc++]="        ((float)(1.0/4294967296.0))*(AP1.t[i+1] - AP1.t[i]);";
  z[lc++]="}";
  z[lc++]="else";             /* for first time through */ 
  z[lc++]="{";
  z[lc++]="  NVI(%1$s_kcyc) = 1;";
  z[lc++]="  ret = AP1.t[0];";
  z[lc++]="}";
  printblock(lc);
}


/*********************************************************/
/*  oscil() code for special case of no loops, k-rate f  */
/*********************************************************/

void nolooposcilkfreq_sinc(tnode * tptr)

{
  int lc = 0;

  acycleguard(tptr, &lc);    /* fast path for most acycles */
  z[lc++]="{";
  z[lc++]="  if (NVI(%1$s_fsign))";  /* positive frequency */
  z[lc++]="   {";
  z[lc++]="    NVUI(%1$s_pfrac) = (j = NVUI(%1$s_pfrac)) + NVUI(%1$s_kfrac);";
  z[lc++]="    NVUI(%1$s_pint) += NVUI(%1$s_kint) + (NVUI(%1$s_pfrac) < j);";
  z[lc++]="    if ((i = NVUI(%1$s_pint)) >= AP1.len)";
  z[lc++]="      i = (NVUI(%1$s_pint) -= AP1.len);";
  z[lc++]="   }";
  z[lc++]="  else";   /* negative frequency */
  z[lc++]="   {";
  z[lc++]="    NVUI(%1$s_pfrac) = (j = NVUI(%1$s_pfrac)) - NVUI(%1$s_kfrac);";
  z[lc++]="    NVUI(%1$s_pint) -= NVUI(%1$s_kint) + (NVUI(%1$s_pfrac) > j);";
  z[lc++]="    if ((i = NVUI(%1$s_pint)) >= AP1.len)";
  z[lc++]="      i = (NVUI(%1$s_pint) += AP1.len);";
  z[lc++]="   }";
  z[lc++]="  if ((NVUI(%1$s_kint) == 1) && (NVUI(%1$s_kfrac) == 0))";
  z[lc++]="    ret = AP1.t[i];";
  z[lc++]="  else";
  z[lc++]="   {";
  z[lc++]="    if (NVUI(%1$s_sfui) == 0x00010000)";
  z[lc++]="     fptr = (NVUI(%1$s_pfrac)) >> (32 - SINC_LOG_PILEN);";
  z[lc++]="    else";
  z[lc++]="     fptr = (NVUI(%1$s_sfui)*((NVUI(%1$s_pfrac)) >> 16)) >> (32 - SINC_LOG_PILEN);";
  z[lc++]="    ret = sinc[fptr]*AP1.t[i];";
  z[lc++]="    rptr = NVUI(%1$s_osincr) - fptr;";
  z[lc++]="    fptr += NVUI(%1$s_osincr);";
  z[lc++]="    k = 0;";
  z[lc++]="    while (rptr < SINC_SIZE)";
  z[lc++]="     {";
  z[lc++]="       if ((i + (++k)) < AP1.len)";
  z[lc++]="         ret += sinc[rptr]*AP1.t[i + k];";
  z[lc++]="       else";
  z[lc++]="         ret += sinc[rptr]*AP1.t[i + k - AP1.len];";
  z[lc++]="       rptr += NVUI(%1$s_osincr);";
  z[lc++]="       if (fptr < SINC_SIZE)";
  z[lc++]="        {";
  z[lc++]="          if (i >= k)";
  z[lc++]="            ret += sinc[fptr]*AP1.t[i - k];";
  z[lc++]="          else";
  z[lc++]="            ret += sinc[fptr]*AP1.t[AP1.len + i - k];";
  z[lc++]="          fptr += NVUI(%1$s_osincr);";
  z[lc++]="        }";
  z[lc++]="     }";
  z[lc++]="    if (NVUI(%1$s_sfui) != 0x00010000)";
  z[lc++]="      ret *= NV(%1$s_sffl);";
  z[lc++]="   }";
  z[lc++]="}";
  z[lc++]="else";             /* slow path for first time of each acycle */ 
  z[lc++]="{";
  z[lc++]="  if ((NVI(%1$s_fsign) = (freq >= 0)))";
  z[lc++]="   {";
  z[lc++]="    nint = (unsigned int)(index = freq*AP1.oconst);";
  z[lc++]="    while (index >= AP1.lenf)";
  z[lc++]="      nint = (unsigned int)(index -= AP1.lenf);";
  z[lc++]="    nfrac = (unsigned int)(4294967296.0F*(index - nint));";  
  z[lc++]="   }";
  z[lc++]="  else";
  z[lc++]="   {";
  z[lc++]="    nint = (unsigned int)(index = -freq*AP1.oconst);";
  z[lc++]="    while (index >= AP1.lenf)";
  z[lc++]="      nint = (unsigned int)(index -= AP1.lenf);";
  z[lc++]="    nfrac = (unsigned int)(4294967296.0F*(index - nint));";
  z[lc++]="   }";
  z[lc++]="  NVUI(%1$s_kint) = nint;";
  z[lc++]="  NVUI(%1$s_kfrac) = nfrac;";
  z[lc++]="  if (index <= 1.0F)";     /* set interpolation values */
  z[lc++]="   {";
  z[lc++]="    NV(%1$s_sffl) = 1.0F;";
  z[lc++]="    NVUI(%1$s_sfui) = 0x00010000;";
  z[lc++]="    NVUI(%1$s_osincr) = SINC_PILEN;";
  z[lc++]="   }";
  z[lc++]="  else";
  z[lc++]="   {";
  z[lc++]="    if (index < SINC_UPMAX)";
  z[lc++]="      NV(%1$s_sffl) = 1.0F/index;";
  z[lc++]="    else";
  z[lc++]="      NV(%1$s_sffl) = 1.0F/SINC_UPMAX;";
  z[lc++]="    NVUI(%1$s_sfui) = ((float)(pow(2,16)))*NV(%1$s_sffl) + 0.5F;";
  z[lc++]="    NVUI(%1$s_osincr) = (SINC_PILEN*NVUI(%1$s_sfui)) >> 16;";
  z[lc++]="   }";
  z[lc++]="  if (NVI(%1$s_kcyc) != 0)";     /* after first time through ... */
  z[lc++]="   {";
  z[lc++]="    if (NVI(%1$s_fsign))";  /* positive frequency */
  z[lc++]="     {";
  z[lc++]="      NVUI(%1$s_pfrac) = (j = NVUI(%1$s_pfrac)) + nfrac;";
  z[lc++]="      NVUI(%1$s_pint) += nint + (NVUI(%1$s_pfrac) < j);";
  z[lc++]="      if ((i = NVUI(%1$s_pint)) >= AP1.len)";
  z[lc++]="        i = (NVUI(%1$s_pint) -= AP1.len);";
  z[lc++]="     }";
  z[lc++]="    else";   /* negative frequency */
  z[lc++]="     {";
  z[lc++]="      NVUI(%1$s_pfrac) = (j = NVUI(%1$s_pfrac)) - nfrac;";
  z[lc++]="      NVUI(%1$s_pint) -= nint + (NVUI(%1$s_pfrac) > j);";
  z[lc++]="      if ((i = NVUI(%1$s_pint)) >= AP1.len)";
  z[lc++]="        i = (NVUI(%1$s_pint) += AP1.len);";
  z[lc++]="     }";
  z[lc++]="   }";
  z[lc++]="  else";                      /* first time through */
  z[lc++]="   {";
  z[lc++]="    NVI(%1$s_kcyc) = 1;";
  z[lc++]="    i = 0;";
  z[lc++]="   }";
  z[lc++]="  if ((NVUI(%1$s_kint) == 1) && (NVUI(%1$s_kfrac) == 0))";
  z[lc++]="    ret = AP1.t[i];";
  z[lc++]="  else";
  z[lc++]="   {";
  z[lc++]="    if (NVUI(%1$s_sfui) == 0x00010000)";
  z[lc++]="     fptr = (NVUI(%1$s_pfrac)) >> (32 - SINC_LOG_PILEN);";
  z[lc++]="    else";
  z[lc++]="     fptr = (NVUI(%1$s_sfui)*((NVUI(%1$s_pfrac)) >> 16)) >> (32 - SINC_LOG_PILEN);";
  z[lc++]="    ret = sinc[fptr]*AP1.t[i];";
  z[lc++]="    rptr = NVUI(%1$s_osincr) - fptr;";
  z[lc++]="    fptr += NVUI(%1$s_osincr);";
  z[lc++]="    k = 0;";
  z[lc++]="    while (rptr < SINC_SIZE)";
  z[lc++]="     {";
  z[lc++]="       if ((i + (++k)) < AP1.len)";
  z[lc++]="         ret += sinc[rptr]*AP1.t[i + k];";
  z[lc++]="       else";
  z[lc++]="         ret += sinc[rptr]*AP1.t[i + k - AP1.len];";
  z[lc++]="       rptr += NVUI(%1$s_osincr);";
  z[lc++]="       if (fptr < SINC_SIZE)";
  z[lc++]="        {";
  z[lc++]="          if (i >= k)";
  z[lc++]="            ret += sinc[fptr]*AP1.t[i - k];";
  z[lc++]="          else";
  z[lc++]="            ret += sinc[fptr]*AP1.t[AP1.len + i - k];";
  z[lc++]="          fptr += NVUI(%1$s_osincr);";
  z[lc++]="        }";
  z[lc++]="     }";
  z[lc++]="    if (NVUI(%1$s_sfui) != 0x00010000)";
  z[lc++]="      ret *= NV(%1$s_sffl);";
  z[lc++]="   }";
  kcycassign(tptr, &lc);
  z[lc++]="}";
  printblock(lc);
}


/*********************************************************/
/*  oscil() code for special case of no loops, k-rate f  */
/*********************************************************/

void nolooposcilkfreq(tnode * tptr)

{
  int lc = 0;

  acycleguard(tptr, &lc);    /* fast path for most acycles */
  z[lc++]="{";
  z[lc++]="  if (NVI(%1$s_fsign))";  /* positive frequency */
  z[lc++]="   {";
  z[lc++]="    NVUI(%1$s_pfrac) = (j = NVUI(%1$s_pfrac)) + NVUI(%1$s_kfrac);";
  z[lc++]="    NVUI(%1$s_pint) += NVUI(%1$s_kint) + (NVUI(%1$s_pfrac) < j);";
  z[lc++]="    if ((i = NVUI(%1$s_pint)) >= AP1.len)";
  z[lc++]="      i = (NVUI(%1$s_pint) -= AP1.len);";
  z[lc++]="   }";
  z[lc++]="  else";   /* negative frequency */
  z[lc++]="   {";
  z[lc++]="    NVUI(%1$s_pfrac) = (j = NVUI(%1$s_pfrac)) - NVUI(%1$s_kfrac);";
  z[lc++]="    NVUI(%1$s_pint) -= NVUI(%1$s_kint) + (NVUI(%1$s_pfrac) > j);";
  z[lc++]="    if ((i = NVUI(%1$s_pint)) >= AP1.len)";
  z[lc++]="      i = (NVUI(%1$s_pint) += AP1.len);";
  z[lc++]="   }";
  z[lc++]="  ret = AP1.t[i] + NVUI(%1$s_pfrac)*";
  z[lc++]="        ((float)(1.0/4294967296.0))*(AP1.t[i+1] - AP1.t[i]);";  
  z[lc++]="}";
  z[lc++]="else";             /* slow path for first time of each acycle */ 
  z[lc++]="{";
  z[lc++]="  if ((NVI(%1$s_fsign) = (freq >= 0)))";
  z[lc++]="   {";
  z[lc++]="    nint = (unsigned int)(index = freq*AP1.oconst);";
  z[lc++]="    while (index >= AP1.lenf)";
  z[lc++]="      nint = (unsigned int)(index -= AP1.lenf);";
  z[lc++]="    nfrac = (unsigned int)(4294967296.0F*(index - nint));";  
  z[lc++]="   }";
  z[lc++]="  else";
  z[lc++]="   {";
  z[lc++]="    nint = (unsigned int)(index = -freq*AP1.oconst);";
  z[lc++]="    while (index >= AP1.lenf)";
  z[lc++]="      nint = (unsigned int)(index -= AP1.lenf);";
  z[lc++]="    nfrac = (unsigned int)(4294967296.0F*(index - nint));";
  z[lc++]="   }";
  z[lc++]="  NVUI(%1$s_kint) = nint;";
  z[lc++]="  NVUI(%1$s_kfrac) = nfrac;";
  z[lc++]="  if (NVI(%1$s_kcyc) != 0)";     /* after first time through ... */
  z[lc++]="   {";
  z[lc++]="    if (NVI(%1$s_fsign))";  /* positive frequency */
  z[lc++]="     {";
  z[lc++]="      NVUI(%1$s_pfrac) = (j = NVUI(%1$s_pfrac)) + nfrac;";
  z[lc++]="      NVUI(%1$s_pint) += nint + (NVUI(%1$s_pfrac) < j);";
  z[lc++]="      if ((i = NVUI(%1$s_pint)) >= AP1.len)";
  z[lc++]="        i = (NVUI(%1$s_pint) -= AP1.len);";
  z[lc++]="     }";
  z[lc++]="    else";   /* negative frequency */
  z[lc++]="     {";
  z[lc++]="      NVUI(%1$s_pfrac) = (j = NVUI(%1$s_pfrac)) - nfrac;";
  z[lc++]="      NVUI(%1$s_pint) -= nint + (NVUI(%1$s_pfrac) > j);";
  z[lc++]="      if ((i = NVUI(%1$s_pint)) >= AP1.len)";
  z[lc++]="        i = (NVUI(%1$s_pint) += AP1.len);";
  z[lc++]="     }";
  z[lc++]="    ret = AP1.t[i] + NVUI(%1$s_pfrac)*";
  z[lc++]="          ((float)(1.0/4294967296.0))*(AP1.t[i+1] - AP1.t[i]);";  
  z[lc++]="   }";
  z[lc++]="  else";                      /* first time through */
  z[lc++]="   {";
  z[lc++]="    NVI(%1$s_kcyc) = 1;";
  z[lc++]="    ret = AP1.t[0];";
  z[lc++]="   }";
  kcycassign(tptr, &lc);
  z[lc++]="}";
  printblock(lc);
}




/*********************************************************/
/*  oscil() code for special case of no loops, i-rate f  */
/*********************************************************/

void nolooposcilifreq_sinc(tnode * tptr)

{
  int lc = 0;

  z[lc++]="if (NVI(%1$s_kcyc))";   /* fast path */
  z[lc++]="{";
  z[lc++]="  if (NVI(%1$s_fsign))";  /* positive frequency */
  z[lc++]="   {";
  z[lc++]="    NVUI(%1$s_pfrac) = (j = NVUI(%1$s_pfrac)) + NVUI(%1$s_kfrac);";
  z[lc++]="    NVUI(%1$s_pint) += NVUI(%1$s_kint) + (NVUI(%1$s_pfrac) < j);";
  z[lc++]="    if ((i = NVUI(%1$s_pint)) >= AP1.len)";
  z[lc++]="      i = (NVUI(%1$s_pint) -= AP1.len);";
  z[lc++]="   }";
  z[lc++]="  else";   /* negative frequency */
  z[lc++]="   {";
  z[lc++]="    NVUI(%1$s_pfrac) = (j = NVUI(%1$s_pfrac)) - NVUI(%1$s_kfrac);";
  z[lc++]="    NVUI(%1$s_pint) -= NVUI(%1$s_kint) + (NVUI(%1$s_pfrac) > j);";
  z[lc++]="    if ((i = NVUI(%1$s_pint)) >= AP1.len)";
  z[lc++]="      i = (NVUI(%1$s_pint) += AP1.len);";
  z[lc++]="   }";
  z[lc++]="  if ((NVUI(%1$s_kint) == 1) && (NVUI(%1$s_kfrac) == 0))";
  z[lc++]="    ret = AP1.t[i];";
  z[lc++]="  else";
  z[lc++]="   {";
  z[lc++]="    if (NVUI(%1$s_sfui) == 0x00010000)";
  z[lc++]="     fptr = (NVUI(%1$s_pfrac)) >> (32 - SINC_LOG_PILEN);";
  z[lc++]="    else";
  z[lc++]="     fptr = (NVUI(%1$s_sfui)*((NVUI(%1$s_pfrac)) >> 16)) >> (32 - SINC_LOG_PILEN);";
  z[lc++]="    ret = sinc[fptr]*AP1.t[i];";
  z[lc++]="    rptr = NVUI(%1$s_osincr) - fptr;";
  z[lc++]="    fptr += NVUI(%1$s_osincr);";
  z[lc++]="    k = 0;";
  z[lc++]="    while (rptr < SINC_SIZE)";
  z[lc++]="     {";
  z[lc++]="       if ((i + (++k)) < AP1.len)";
  z[lc++]="         ret += sinc[rptr]*AP1.t[i + k];";
  z[lc++]="       else";
  z[lc++]="         ret += sinc[rptr]*AP1.t[i + k - AP1.len];";
  z[lc++]="       rptr += NVUI(%1$s_osincr);";
  z[lc++]="       if (fptr < SINC_SIZE)";
  z[lc++]="        {";
  z[lc++]="          if (i >= k)";
  z[lc++]="            ret += sinc[fptr]*AP1.t[i - k];";
  z[lc++]="          else";
  z[lc++]="            ret += sinc[fptr]*AP1.t[AP1.len + i - k];";
  z[lc++]="          fptr += NVUI(%1$s_osincr);";
  z[lc++]="        }";
  z[lc++]="     }";
  z[lc++]="    if (NVUI(%1$s_sfui) != 0x00010000)";
  z[lc++]="      ret *= NV(%1$s_sffl);";
  z[lc++]="   }";
  z[lc++]="}";
  z[lc++]="else";             /* for first time through */ 
  z[lc++]="{";
  z[lc++]="  if ((NVI(%1$s_fsign) = (freq >= 0)))";
  z[lc++]="   {";
  z[lc++]="    nint = (unsigned int)(index = freq*AP1.oconst);";
  z[lc++]="    while (index >= AP1.lenf)";
  z[lc++]="      nint = (unsigned int)(index -= AP1.lenf);";
  z[lc++]="    nfrac = (unsigned int)(4294967296.0F*(index - nint));";  
  z[lc++]="   }";
  z[lc++]="  else";
  z[lc++]="   {";
  z[lc++]="    nint = (unsigned int)(index = -freq*AP1.oconst);";
  z[lc++]="    while (index >= AP1.lenf)";
  z[lc++]="      nint = (unsigned int)(index -= AP1.lenf);";
  z[lc++]="    nfrac = (unsigned int)(4294967296.0F*(index - nint));";
  z[lc++]="   }";
  z[lc++]="  NVUI(%1$s_kint) = nint;";
  z[lc++]="  NVUI(%1$s_kfrac) = nfrac;";
  z[lc++]="  NVI(%1$s_kcyc) = 1;";
  z[lc++]="  if (index <= 1.0F)";     /* set interpolation values */
  z[lc++]="   {";
  z[lc++]="    NV(%1$s_sffl) = 1.0F;";
  z[lc++]="    NVUI(%1$s_sfui) = 0x00010000;";
  z[lc++]="    NVUI(%1$s_osincr) = SINC_PILEN;";
  z[lc++]="   }";
  z[lc++]="  else";
  z[lc++]="   {";
  z[lc++]="    if (index < SINC_UPMAX)";
  z[lc++]="      NV(%1$s_sffl) = 1.0F/index;";
  z[lc++]="    else";
  z[lc++]="      NV(%1$s_sffl) = 1.0F/SINC_UPMAX;";
  z[lc++]="    NVUI(%1$s_sfui) = ((float)(pow(2,16)))*NV(%1$s_sffl) + 0.5F;";
  z[lc++]="    NVUI(%1$s_osincr) = (SINC_PILEN*NVUI(%1$s_sfui)) >> 16;";
  z[lc++]="   }";
  z[lc++]="  ret = AP1.t[0];";
  z[lc++]="  if (index > 1.0F)";
  z[lc++]="   {";
  z[lc++]="    fptr = rptr = NVUI(%1$s_osincr);";
  z[lc++]="    k = 0;";
  z[lc++]="    while (rptr < SINC_SIZE)";
  z[lc++]="     {";
  z[lc++]="       if ((++k) < AP1.len)";
  z[lc++]="         ret += sinc[rptr]*AP1.t[k];";
  z[lc++]="       else";
  z[lc++]="         ret += sinc[rptr]*AP1.t[k - AP1.len];";
  z[lc++]="       rptr += NVUI(%1$s_osincr);";
  z[lc++]="       if (fptr < SINC_SIZE)";
  z[lc++]="        {";
  z[lc++]="          ret += sinc[fptr]*AP1.t[AP1.len - k];";
  z[lc++]="          fptr += NVUI(%1$s_osincr);";
  z[lc++]="        }";
  z[lc++]="     }";
  z[lc++]="    ret *= NV(%1$s_sffl);";
  z[lc++]="   }";
  z[lc++]="}";
  printblock(lc);
}

/*********************************************************/
/*  oscil() code for special case of no loops, i-rate f  */
/*********************************************************/

void nolooposcilifreq(tnode * tptr)

{
  int lc = 0;

  z[lc++]="if (NVI(%1$s_kcyc))";   /* fast path */
  z[lc++]="{";
  z[lc++]="  if (NVI(%1$s_fsign))";  /* positive frequency */
  z[lc++]="   {";
  z[lc++]="    NVUI(%1$s_pfrac) = (j = NVUI(%1$s_pfrac)) + NVUI(%1$s_kfrac);";
  z[lc++]="    NVUI(%1$s_pint) += NVUI(%1$s_kint) + (NVUI(%1$s_pfrac) < j);";
  z[lc++]="    if ((i = NVUI(%1$s_pint)) >= AP1.len)";
  z[lc++]="      i = (NVUI(%1$s_pint) -= AP1.len);";
  z[lc++]="   }";
  z[lc++]="  else";   /* negative frequency */
  z[lc++]="   {";
  z[lc++]="    NVUI(%1$s_pfrac) = (j = NVUI(%1$s_pfrac)) - NVUI(%1$s_kfrac);";
  z[lc++]="    NVUI(%1$s_pint) -= NVUI(%1$s_kint) + (NVUI(%1$s_pfrac) > j);";
  z[lc++]="    if ((i = NVUI(%1$s_pint)) >= AP1.len)";
  z[lc++]="      i = (NVUI(%1$s_pint) += AP1.len);";
  z[lc++]="   }";
  z[lc++]="  ret = AP1.t[i] + NVUI(%1$s_pfrac)*";
  z[lc++]="        ((float)(1.0/4294967296.0))*(AP1.t[i+1] - AP1.t[i]);";  
  z[lc++]="}";
  z[lc++]="else";             /* for first time through */ 
  z[lc++]="{";
  z[lc++]="  if ((NVI(%1$s_fsign) = (freq >= 0)))";
  z[lc++]="   {";
  z[lc++]="    nint = (unsigned int)(index = freq*AP1.oconst);";
  z[lc++]="    while (index >= AP1.lenf)";
  z[lc++]="      nint = (unsigned int)(index -= AP1.lenf);";
  z[lc++]="    nfrac = (unsigned int)(4294967296.0F*(index - nint));";  
  z[lc++]="   }";
  z[lc++]="  else";
  z[lc++]="   {";
  z[lc++]="    nint = (unsigned int)(index = -freq*AP1.oconst);";
  z[lc++]="    while (index >= AP1.lenf)";
  z[lc++]="      nint = (unsigned int)(index -= AP1.lenf);";
  z[lc++]="    nfrac = (unsigned int)(4294967296.0F*(index - nint));";
  z[lc++]="   }";
  z[lc++]="  NVUI(%1$s_kint) = nint;";
  z[lc++]="  NVUI(%1$s_kfrac) = nfrac;";
  z[lc++]="  NVI(%1$s_kcyc) = 1;";
  z[lc++]="  ret = AP1.t[0];";
  z[lc++]="}";
  printblock(lc);
}


/*********************************************************/
/*      oscil() code for loop case -- a-rate freq        */
/*********************************************************/

void looposcilafreq_sinc(tnode * tptr)

{
  int lc = 0;

  z[lc++]="if (NVI(%1$s_kcyc))";   /* fast path */
  z[lc++]="{";
  z[lc++]="  if (freq >= 0)";  /* positive frequency */
  z[lc++]="   {";
  z[lc++]="    nint = (unsigned int)(index = freq*AP1.oconst);";
  z[lc++]="    while (index >= AP1.lenf)";
  z[lc++]="      nint = (unsigned int)(index -= AP1.lenf);";
  z[lc++]="    nfrac = (unsigned int)(4294967296.0F*(index - nint));";
  z[lc++]="    NVUI(%1$s_pfrac) = (j = NVUI(%1$s_pfrac)) + nfrac;";
  z[lc++]="    NVUI(%1$s_pint) += nint + (NVUI(%1$s_pfrac) < j);";
  z[lc++]="    if ((i = NVUI(%1$s_pint)) >= AP1.len)";
  z[lc++]="     {";
  z[lc++]="      i = (NVUI(%1$s_pint) -= AP1.len);";
  z[lc++]="      if (NVI(%1$s_iloops) > 0)";
  z[lc++]="        NVI(%1$s_iloops)--;";
  z[lc++]="     }";
  z[lc++]="   }";
  z[lc++]="  else";   /* negative frequency */
  z[lc++]="   {";
  z[lc++]="    nint = (unsigned int)(index = -freq*AP1.oconst);";
  z[lc++]="    while (index >= AP1.lenf)";
  z[lc++]="      nint = (unsigned int)(index -= AP1.lenf);";
  z[lc++]="    nfrac = (unsigned int)(4294967296.0F*(index - nint));";
  z[lc++]="    NVUI(%1$s_pfrac) = (j = NVUI(%1$s_pfrac)) - nfrac;";
  z[lc++]="    NVUI(%1$s_pint) -= nint + (NVUI(%1$s_pfrac) > j);";
  z[lc++]="    if ((i = NVUI(%1$s_pint)) >= AP1.len)";
  z[lc++]="     {";
  z[lc++]="       i = (NVUI(%1$s_pint) += AP1.len);";
  z[lc++]="       if (NVI(%1$s_iloops) > 0)";
  z[lc++]="         NVI(%1$s_iloops)++;";
  z[lc++]="     }";
  z[lc++]="   }";
  z[lc++]="  if (NVI(%1$s_iloops) == 0)";
  z[lc++]="    ret = 0.0F;";
  z[lc++]="  else";
  z[lc++]="   {";
  z[lc++]="    if ((nint == 1) && (nfrac == 0))";
  z[lc++]="      ret = AP1.t[i];";
  z[lc++]="    else";
  z[lc++]="     {";
  z[lc++]="      if (index <= 1.0F)";
  z[lc++]="       {";
  z[lc++]="        sffl = 1.0F;";
  z[lc++]="        sfui = 0x00010000;";
  z[lc++]="        osincr = SINC_PILEN;";
  z[lc++]="       }";
  z[lc++]="      else";
  z[lc++]="       {";
  z[lc++]="         if (index < SINC_UPMAX)";
  z[lc++]="           sffl = 1.0F/index;";
  z[lc++]="         else";
  z[lc++]="           sffl = 1.0F/SINC_UPMAX;";
  z[lc++]="        sfui = ((float)(pow(2,16)))*sffl + 0.5F;";
  z[lc++]="        osincr = (SINC_PILEN*sfui) >> 16;";
  z[lc++]="       }";
  z[lc++]="      if (sfui == 0x00010000)";
  z[lc++]="       fptr = (NVUI(%1$s_pfrac)) >> (32 - SINC_LOG_PILEN);";
  z[lc++]="      else";
  z[lc++]="       fptr = (sfui*((NVUI(%1$s_pfrac)) >> 16)) >> (32 - SINC_LOG_PILEN);";
  z[lc++]="      ret = sinc[fptr]*AP1.t[i];";
  z[lc++]="      rptr = osincr - fptr;";
  z[lc++]="      fptr += osincr;";
  z[lc++]="      k = 0;";
  z[lc++]="      while (rptr < SINC_SIZE)";
  z[lc++]="       {";
  z[lc++]="         if ((i + (++k)) < AP1.len)";
  z[lc++]="           ret += sinc[rptr]*AP1.t[i + k];";
  z[lc++]="         else";
  z[lc++]="           ret += sinc[rptr]*AP1.t[i + k - AP1.len];";
  z[lc++]="         rptr += osincr;";
  z[lc++]="         if (fptr < SINC_SIZE)";
  z[lc++]="          {";
  z[lc++]="            if (i >= k)";
  z[lc++]="              ret += sinc[fptr]*AP1.t[i - k];";
  z[lc++]="            else";
  z[lc++]="              ret += sinc[fptr]*AP1.t[AP1.len + i - k];";
  z[lc++]="            fptr += osincr;";
  z[lc++]="          }";
  z[lc++]="       }";
  z[lc++]="      if (sfui != 0x00010000)";
  z[lc++]="        ret *= sffl;";
  z[lc++]="     }";
  z[lc++]="   }";
  z[lc++]="}";
  z[lc++]="else";             /* for first time through */ 
  z[lc++]="{";
  z[lc++]="  NVI(%1$s_kcyc) = 1;";
  z[lc++]="  NVI(%1$s_iloops)= (int)(ROUND(va_loops));";
  z[lc++]="  if (freq >= 0)";  /* positive frequency */
  z[lc++]="   {";
  z[lc++]="    nint = (unsigned int)(index = freq*AP1.oconst);";
  z[lc++]="    while (index >= AP1.lenf)";
  z[lc++]="      nint = (unsigned int)(index -= AP1.lenf);";
  z[lc++]="    nfrac = (unsigned int)(4294967296.0F*(index - nint));";
  z[lc++]="   }";
  z[lc++]="  else";   /* negative frequency */
  z[lc++]="   {";
  z[lc++]="    nint = (unsigned int)(index = -freq*AP1.oconst);";
  z[lc++]="    while (index >= AP1.lenf)";
  z[lc++]="      nint = (unsigned int)(index -= AP1.lenf);";
  z[lc++]="    nfrac = (unsigned int)(4294967296.0F*(index - nint));";
  z[lc++]="   }";
  z[lc++]="  ret = AP1.t[0];";
  z[lc++]="  if (index > 1.0F)";
  z[lc++]="   {";
  z[lc++]="     if (index < SINC_UPMAX)";
  z[lc++]="       sffl = 1.0F/index;";
  z[lc++]="     else";
  z[lc++]="       sffl = 1.0F/SINC_UPMAX;";
  z[lc++]="    sfui = ((float)(pow(2,16)))*sffl + 0.5F;";
  z[lc++]="    osincr = (SINC_PILEN*sfui) >> 16;";
  z[lc++]="    fptr = rptr = osincr;";
  z[lc++]="    k = 0;";
  z[lc++]="    while (rptr < SINC_SIZE)";
  z[lc++]="     {";
  z[lc++]="       if ((++k) < AP1.len)";
  z[lc++]="         ret += sinc[rptr]*AP1.t[k];";
  z[lc++]="       else";
  z[lc++]="         ret += sinc[rptr]*AP1.t[k - AP1.len];";
  z[lc++]="       rptr += osincr;";
  z[lc++]="       if (fptr < SINC_SIZE)";
  z[lc++]="        {";
  z[lc++]="          ret += sinc[fptr]*AP1.t[AP1.len - k];";
  z[lc++]="          fptr += osincr;";
  z[lc++]="        }";
  z[lc++]="     }";
  z[lc++]="    ret *= sffl;";
  z[lc++]="   }";
  z[lc++]="}";
  printblock(lc);
}

/*********************************************************/
/*      oscil() code for loop case -- a-rate freq        */
/*********************************************************/

void looposcilafreq(tnode * tptr)

{
  int lc = 0;

  z[lc++]="if (NVI(%1$s_kcyc))";   /* fast path */
  z[lc++]="{";
  z[lc++]="  if (freq >= 0)";  /* positive frequency */
  z[lc++]="   {";
  z[lc++]="    nint = (unsigned int)(index = freq*AP1.oconst);";
  z[lc++]="    while (index >= AP1.lenf)";
  z[lc++]="      nint = (unsigned int)(index -= AP1.lenf);";
  z[lc++]="    nfrac = (unsigned int)(4294967296.0F*(index - nint));";
  z[lc++]="    NVUI(%1$s_pfrac) = (j = NVUI(%1$s_pfrac)) + nfrac;";
  z[lc++]="    NVUI(%1$s_pint) += nint + (NVUI(%1$s_pfrac) < j);";
  z[lc++]="    if ((i = NVUI(%1$s_pint)) >= AP1.len)";
  z[lc++]="     {";
  z[lc++]="      i = (NVUI(%1$s_pint) -= AP1.len);";
  z[lc++]="      if (NVI(%1$s_iloops) > 0)";
  z[lc++]="        NVI(%1$s_iloops)--;";
  z[lc++]="     }";
  z[lc++]="   }";
  z[lc++]="  else";   /* negative frequency */
  z[lc++]="   {";
  z[lc++]="    nint = (unsigned int)(index = -freq*AP1.oconst);";
  z[lc++]="    while (index >= AP1.lenf)";
  z[lc++]="      nint = (unsigned int)(index -= AP1.lenf);";
  z[lc++]="    nfrac = (unsigned int)(4294967296.0F*(index - nint));";
  z[lc++]="    NVUI(%1$s_pfrac) = (j = NVUI(%1$s_pfrac)) - nfrac;";
  z[lc++]="    NVUI(%1$s_pint) -= nint + (NVUI(%1$s_pfrac) > j);";
  z[lc++]="    if ((i = NVUI(%1$s_pint)) >= AP1.len)";
  z[lc++]="     {";
  z[lc++]="       i = (NVUI(%1$s_pint) += AP1.len);";
  z[lc++]="       if (NVI(%1$s_iloops) > 0)";
  z[lc++]="         NVI(%1$s_iloops)++;";
  z[lc++]="     }";
  z[lc++]="   }";
  z[lc++]="  if (NVI(%1$s_iloops) == 0)";
  z[lc++]="    ret = 0.0F;";
  z[lc++]="  else";
  z[lc++]="    ret = AP1.t[i] + NVUI(%1$s_pfrac)*";
  z[lc++]="        ((float)(1.0/4294967296.0))*(AP1.t[i+1] - AP1.t[i]);";
  z[lc++]="}";
  z[lc++]="else";             /* for first time through */ 
  z[lc++]="{";
  z[lc++]="  NVI(%1$s_kcyc) = 1;";
  z[lc++]="  NVI(%1$s_iloops)= (int)(ROUND(va_loops));";
  z[lc++]="  ret = AP1.t[0];";
  z[lc++]="}";
  printblock(lc);
}

/*********************************************************/
/*      oscil() code for loop case -- k-rate freq        */
/*********************************************************/

void looposcilkfreq_sinc(tnode * tptr)

{
  int lc = 0;

  acycleguard(tptr, &lc);    /* fast path for most acycles */
  z[lc++]="{";
  z[lc++]="  if (NVI(%1$s_fsign))";  /* positive frequency */
  z[lc++]="   {";
  z[lc++]="    NVUI(%1$s_pfrac) = (j = NVUI(%1$s_pfrac)) + NVUI(%1$s_kfrac);";
  z[lc++]="    NVUI(%1$s_pint) += NVUI(%1$s_kint) + (NVUI(%1$s_pfrac) < j);";
  z[lc++]="    if ((i = NVUI(%1$s_pint)) >= AP1.len)";
  z[lc++]="     {";
  z[lc++]="      i = (NVUI(%1$s_pint) -= AP1.len);";
  z[lc++]="      if (NVI(%1$s_iloops) > 0)";
  z[lc++]="        NVI(%1$s_iloops)--;";
  z[lc++]="     }";
  z[lc++]="   }";
  z[lc++]="  else";   /* negative frequency */
  z[lc++]="   {";
  z[lc++]="    NVUI(%1$s_pfrac) = (j = NVUI(%1$s_pfrac)) - NVUI(%1$s_kfrac);";
  z[lc++]="    NVUI(%1$s_pint) -= NVUI(%1$s_kint) + (NVUI(%1$s_pfrac) > j);";
  z[lc++]="    if ((i = NVUI(%1$s_pint)) >= AP1.len)";
  z[lc++]="     {";
  z[lc++]="       i = (NVUI(%1$s_pint) += AP1.len);";
  z[lc++]="       if (NVI(%1$s_iloops) > 0)";
  z[lc++]="         NVI(%1$s_iloops)++;";
  z[lc++]="     }";
  z[lc++]="   }";
  z[lc++]="  if (NVI(%1$s_iloops) == 0)";
  z[lc++]="    ret = 0.0F;";
  z[lc++]="  else";
  z[lc++]="   {";
  z[lc++]="    if ((NVUI(%1$s_kint) == 1) && (NVUI(%1$s_kfrac) == 0))";
  z[lc++]="      ret = AP1.t[i];";
  z[lc++]="    else";
  z[lc++]="     {";
  z[lc++]="      if (NVUI(%1$s_sfui) == 0x00010000)";
  z[lc++]="       fptr = (NVUI(%1$s_pfrac)) >> (32 - SINC_LOG_PILEN);";
  z[lc++]="      else";
  z[lc++]="       fptr = (NVUI(%1$s_sfui)*((NVUI(%1$s_pfrac)) >> 16)) >> (32 - SINC_LOG_PILEN);";
  z[lc++]="      ret = sinc[fptr]*AP1.t[i];";
  z[lc++]="      rptr = NVUI(%1$s_osincr) - fptr;";
  z[lc++]="      fptr += NVUI(%1$s_osincr);";
  z[lc++]="      k = 0;";
  z[lc++]="      while (rptr < SINC_SIZE)";
  z[lc++]="       {";
  z[lc++]="         if ((i + (++k)) < AP1.len)";
  z[lc++]="           ret += sinc[rptr]*AP1.t[i + k];";
  z[lc++]="         else";
  z[lc++]="           ret += sinc[rptr]*AP1.t[i + k - AP1.len];";
  z[lc++]="         rptr += NVUI(%1$s_osincr);";
  z[lc++]="         if (fptr < SINC_SIZE)";
  z[lc++]="          {";
  z[lc++]="            if (i >= k)";
  z[lc++]="              ret += sinc[fptr]*AP1.t[i - k];";
  z[lc++]="            else";
  z[lc++]="              ret += sinc[fptr]*AP1.t[AP1.len + i - k];";
  z[lc++]="            fptr += NVUI(%1$s_osincr);";
  z[lc++]="          }";
  z[lc++]="       }";
  z[lc++]="      if (NVUI(%1$s_sfui) != 0x00010000)";
  z[lc++]="        ret *= NV(%1$s_sffl);";
  z[lc++]="     }";
  z[lc++]="   }";
  z[lc++]="}";
  z[lc++]="else";             /* slow path for first time of each acycle */ 
  z[lc++]="{";
  z[lc++]="  if ((NVI(%1$s_fsign) = (freq >= 0)))";
  z[lc++]="   {";
  z[lc++]="    nint = (unsigned int)(index = freq*AP1.oconst);";
  z[lc++]="    while (index >= AP1.lenf)";
  z[lc++]="      nint = (unsigned int)(index -= AP1.lenf);";
  z[lc++]="    nfrac = (unsigned int)(4294967296.0F*(index - nint));";  
  z[lc++]="   }";
  z[lc++]="  else";
  z[lc++]="   {";
  z[lc++]="    nint = (unsigned int)(index = -freq*AP1.oconst);";
  z[lc++]="    while (index >= AP1.lenf)";
  z[lc++]="      nint = (unsigned int)(index -= AP1.lenf);";
  z[lc++]="    nfrac = (unsigned int)(4294967296.0F*(index - nint));";
  z[lc++]="   }";
  z[lc++]="  NVUI(%1$s_kint) = nint;";
  z[lc++]="  NVUI(%1$s_kfrac) = nfrac;";
  z[lc++]="  if (index <= 1.0F)";     /* set interpolation values */
  z[lc++]="   {";
  z[lc++]="    NV(%1$s_sffl) = 1.0F;";
  z[lc++]="    NVUI(%1$s_sfui) = 0x00010000;";
  z[lc++]="    NVUI(%1$s_osincr) = SINC_PILEN;";
  z[lc++]="   }";
  z[lc++]="  else";
  z[lc++]="   {";
  z[lc++]="    if (index < SINC_UPMAX)";
  z[lc++]="      NV(%1$s_sffl) = 1.0F/index;";
  z[lc++]="    else";
  z[lc++]="      NV(%1$s_sffl) = 1.0F/SINC_UPMAX;";
  z[lc++]="    NVUI(%1$s_sfui) = ((float)(pow(2,16)))*NV(%1$s_sffl) + 0.5F;";
  z[lc++]="    NVUI(%1$s_osincr) = (SINC_PILEN*NVUI(%1$s_sfui)) >> 16;";
  z[lc++]="   }";
  z[lc++]="  if (NVI(%1$s_kcyc) != 0)";     /* after first time through ... */
  z[lc++]="   {";
  z[lc++]="    if (NVI(%1$s_fsign))";  /* positive frequency */
  z[lc++]="     {";
  z[lc++]="      NVUI(%1$s_pfrac) = (j = NVUI(%1$s_pfrac)) + nfrac;";
  z[lc++]="      NVUI(%1$s_pint) += nint + (NVUI(%1$s_pfrac) < j);";
  z[lc++]="      if ((i = NVUI(%1$s_pint)) >= AP1.len)";
  z[lc++]="       {";
  z[lc++]="        i = (NVUI(%1$s_pint) -= AP1.len);";
  z[lc++]="        if (NVI(%1$s_iloops) > 0)";
  z[lc++]="          NVI(%1$s_iloops)--;";
  z[lc++]="       }";
  z[lc++]="     }";
  z[lc++]="    else";   /* negative frequency */
  z[lc++]="     {";
  z[lc++]="      NVUI(%1$s_pfrac) = (j = NVUI(%1$s_pfrac)) - nfrac;";
  z[lc++]="      NVUI(%1$s_pint) -= nint + (NVUI(%1$s_pfrac) > j);";
  z[lc++]="      if ((i = NVUI(%1$s_pint)) >= AP1.len)";
  z[lc++]="       {";
  z[lc++]="         i = (NVUI(%1$s_pint) += AP1.len);";
  z[lc++]="         if (NVI(%1$s_iloops) > 0)";
  z[lc++]="           NVI(%1$s_iloops)++;";
  z[lc++]="       }";
  z[lc++]="     }";
  z[lc++]="   }";
  z[lc++]="  else";                      /* first time through */
  z[lc++]="   {";
  z[lc++]="    NVI(%1$s_kcyc) = 1;";
  z[lc++]="    NVI(%1$s_iloops)= (int)(ROUND(va_loops));";
  z[lc++]="    i = 0;";
  z[lc++]="   }";
  z[lc++]="  if (NVI(%1$s_iloops) == 0)";
  z[lc++]="    ret = 0.0F;";
  z[lc++]="  else";
  z[lc++]="   {";
  z[lc++]="    if ((NVUI(%1$s_kint) == 1) && (NVUI(%1$s_kfrac) == 0))";
  z[lc++]="      ret = AP1.t[i];";
  z[lc++]="    else";
  z[lc++]="     {";
  z[lc++]="       if (NVUI(%1$s_sfui) == 0x00010000)";
  z[lc++]="        fptr = (NVUI(%1$s_pfrac)) >> (32 - SINC_LOG_PILEN);";
  z[lc++]="       else";
  z[lc++]="        fptr = (NVUI(%1$s_sfui)*((NVUI(%1$s_pfrac)) >> 16)) >> (32 - SINC_LOG_PILEN);";
  z[lc++]="       ret = sinc[fptr]*AP1.t[i];";
  z[lc++]="       rptr = NVUI(%1$s_osincr) - fptr;";
  z[lc++]="       fptr += NVUI(%1$s_osincr);";
  z[lc++]="       k = 0;";
  z[lc++]="       while (rptr < SINC_SIZE)";
  z[lc++]="        {";
  z[lc++]="          if ((i + (++k)) < AP1.len)";
  z[lc++]="            ret += sinc[rptr]*AP1.t[i + k];";
  z[lc++]="          else";
  z[lc++]="            ret += sinc[rptr]*AP1.t[i + k - AP1.len];";
  z[lc++]="          rptr += NVUI(%1$s_osincr);";
  z[lc++]="          if (fptr < SINC_SIZE)";
  z[lc++]="           {";
  z[lc++]="             if (i >= k)";
  z[lc++]="               ret += sinc[fptr]*AP1.t[i - k];";
  z[lc++]="             else";
  z[lc++]="               ret += sinc[fptr]*AP1.t[AP1.len + i - k];";
  z[lc++]="             fptr += NVUI(%1$s_osincr);";
  z[lc++]="           }";
  z[lc++]="        }";
  z[lc++]="       if (NVUI(%1$s_sfui) != 0x00010000)";
  z[lc++]="         ret *= NV(%1$s_sffl);";
  z[lc++]="      }";
  z[lc++]="     }";
  kcycassign(tptr, &lc);
  z[lc++]="}";
  printblock(lc);
}

/*********************************************************/
/*      oscil() code for loop case -- k-rate freq        */
/*********************************************************/

void looposcilkfreq(tnode * tptr)

{
  int lc = 0;

  acycleguard(tptr, &lc);    /* fast path for most acycles */
  z[lc++]="{";
  z[lc++]="  if (NVI(%1$s_fsign))";  /* positive frequency */
  z[lc++]="   {";
  z[lc++]="    NVUI(%1$s_pfrac) = (j = NVUI(%1$s_pfrac)) + NVUI(%1$s_kfrac);";
  z[lc++]="    NVUI(%1$s_pint) += NVUI(%1$s_kint) + (NVUI(%1$s_pfrac) < j);";
  z[lc++]="    if ((i = NVUI(%1$s_pint)) >= AP1.len)";
  z[lc++]="     {";
  z[lc++]="      i = (NVUI(%1$s_pint) -= AP1.len);";
  z[lc++]="      if (NVI(%1$s_iloops) > 0)";
  z[lc++]="        NVI(%1$s_iloops)--;";
  z[lc++]="     }";
  z[lc++]="   }";
  z[lc++]="  else";   /* negative frequency */
  z[lc++]="   {";
  z[lc++]="    NVUI(%1$s_pfrac) = (j = NVUI(%1$s_pfrac)) - NVUI(%1$s_kfrac);";
  z[lc++]="    NVUI(%1$s_pint) -= NVUI(%1$s_kint) + (NVUI(%1$s_pfrac) > j);";
  z[lc++]="    if ((i = NVUI(%1$s_pint)) >= AP1.len)";
  z[lc++]="     {";
  z[lc++]="       i = (NVUI(%1$s_pint) += AP1.len);";
  z[lc++]="       if (NVI(%1$s_iloops) > 0)";
  z[lc++]="         NVI(%1$s_iloops)++;";
  z[lc++]="     }";
  z[lc++]="   }";
  z[lc++]="  if (NVI(%1$s_iloops) == 0)";
  z[lc++]="    ret = 0.0F;";
  z[lc++]="  else";
  z[lc++]="    ret = AP1.t[i] + NVUI(%1$s_pfrac)*";
  z[lc++]="        ((float)(1.0/4294967296.0))*(AP1.t[i+1] - AP1.t[i]);";  
  z[lc++]="}";
  z[lc++]="else";             /* slow path for first time of each acycle */ 
  z[lc++]="{";
  z[lc++]="  if ((NVI(%1$s_fsign) = (freq >= 0)))";
  z[lc++]="   {";
  z[lc++]="    nint = (unsigned int)(index = freq*AP1.oconst);";
  z[lc++]="    while (index >= AP1.lenf)";
  z[lc++]="      nint = (unsigned int)(index -= AP1.lenf);";
  z[lc++]="    nfrac = (unsigned int)(4294967296.0F*(index - nint));";  
  z[lc++]="   }";
  z[lc++]="  else";
  z[lc++]="   {";
  z[lc++]="    nint = (unsigned int)(index = -freq*AP1.oconst);";
  z[lc++]="    while (index >= AP1.lenf)";
  z[lc++]="      nint = (unsigned int)(index -= AP1.lenf);";
  z[lc++]="    nfrac = (unsigned int)(4294967296.0F*(index - nint));";
  z[lc++]="   }";
  z[lc++]="  NVUI(%1$s_kint) = nint;";
  z[lc++]="  NVUI(%1$s_kfrac) = nfrac;";
  z[lc++]="  if (NVI(%1$s_kcyc) != 0)";     /* after first time through ... */
  z[lc++]="   {";
  z[lc++]="    if (NVI(%1$s_fsign))";  /* positive frequency */
  z[lc++]="     {";
  z[lc++]="      NVUI(%1$s_pfrac) = (j = NVUI(%1$s_pfrac)) + nfrac;";
  z[lc++]="      NVUI(%1$s_pint) += nint + (NVUI(%1$s_pfrac) < j);";
  z[lc++]="      if ((i = NVUI(%1$s_pint)) >= AP1.len)";
  z[lc++]="       {";
  z[lc++]="        i = (NVUI(%1$s_pint) -= AP1.len);";
  z[lc++]="        if (NVI(%1$s_iloops) > 0)";
  z[lc++]="          NVI(%1$s_iloops)--;";
  z[lc++]="       }";
  z[lc++]="     }";
  z[lc++]="    else";   /* negative frequency */
  z[lc++]="     {";
  z[lc++]="      NVUI(%1$s_pfrac) = (j = NVUI(%1$s_pfrac)) - nfrac;";
  z[lc++]="      NVUI(%1$s_pint) -= nint + (NVUI(%1$s_pfrac) > j);";
  z[lc++]="      if ((i = NVUI(%1$s_pint)) >= AP1.len)";
  z[lc++]="       {";
  z[lc++]="         i = (NVUI(%1$s_pint) += AP1.len);";
  z[lc++]="         if (NVI(%1$s_iloops) > 0)";
  z[lc++]="           NVI(%1$s_iloops)++;";
  z[lc++]="       }";
  z[lc++]="     }";
  z[lc++]="    if (NVI(%1$s_iloops) == 0)";
  z[lc++]="      ret = 0.0F;";
  z[lc++]="    else";
  z[lc++]="      ret = AP1.t[i] + NVUI(%1$s_pfrac)*";
  z[lc++]="          ((float)(1.0/4294967296.0))*(AP1.t[i+1] - AP1.t[i]);";  
  z[lc++]="   }";
  z[lc++]="  else";                      /* first time through */
  z[lc++]="   {";
  z[lc++]="    NVI(%1$s_kcyc) = 1;";
  z[lc++]="    NVI(%1$s_iloops)= (int)(ROUND(va_loops));";
  z[lc++]="    ret = AP1.t[0];";
  z[lc++]="   }";
  kcycassign(tptr, &lc);
  z[lc++]="}";
  printblock(lc);
}

/*********************************************************/
/*      oscil() code for loop case -- i-rate freq        */
/*********************************************************/

void looposcilifreq_sinc(tnode * tptr)

{
  int lc = 0;

  z[lc++]="if (NVI(%1$s_kcyc))";   /* fast path */
  z[lc++]="{";
  z[lc++]="  if (NVI(%1$s_fsign))";  /* positive frequency */
  z[lc++]="   {";
  z[lc++]="    NVUI(%1$s_pfrac) = (j = NVUI(%1$s_pfrac)) + NVUI(%1$s_kfrac);";
  z[lc++]="    NVUI(%1$s_pint) += NVUI(%1$s_kint) + (NVUI(%1$s_pfrac) < j);";
  z[lc++]="    if ((i = NVUI(%1$s_pint)) >= AP1.len)";
  z[lc++]="     {";
  z[lc++]="      i = (NVUI(%1$s_pint) -= AP1.len);";
  z[lc++]="      if (NVI(%1$s_iloops) > 0)";
  z[lc++]="        NVI(%1$s_iloops)--;";
  z[lc++]="     }";
  z[lc++]="   }";
  z[lc++]="  else";   /* negative frequency */
  z[lc++]="   {";
  z[lc++]="    NVUI(%1$s_pfrac) = (j = NVUI(%1$s_pfrac)) - NVUI(%1$s_kfrac);";
  z[lc++]="    NVUI(%1$s_pint) -= NVUI(%1$s_kint) + (NVUI(%1$s_pfrac) > j);";
  z[lc++]="    if ((i = NVUI(%1$s_pint)) >= AP1.len)";
  z[lc++]="     {";
  z[lc++]="       i = (NVUI(%1$s_pint) += AP1.len);";
  z[lc++]="       if (NVI(%1$s_iloops) > 0)";
  z[lc++]="         NVI(%1$s_iloops)++;";
  z[lc++]="     }";
  z[lc++]="   }";
  z[lc++]="  if (NVI(%1$s_iloops) == 0)";
  z[lc++]="    ret = 0.0F;";
  z[lc++]="  else";
  z[lc++]="   {";
  z[lc++]="    if ((NVUI(%1$s_kint) == 1) && (NVUI(%1$s_kfrac) == 0))";
  z[lc++]="      ret = AP1.t[i];";
  z[lc++]="    else";
  z[lc++]="     {";
  z[lc++]="      if (NVUI(%1$s_sfui) == 0x00010000)";
  z[lc++]="       fptr = (NVUI(%1$s_pfrac)) >> (32 - SINC_LOG_PILEN);";
  z[lc++]="      else";
  z[lc++]="       fptr = (NVUI(%1$s_sfui)*((NVUI(%1$s_pfrac)) >> 16)) >> (32 - SINC_LOG_PILEN);";
  z[lc++]="      ret = sinc[fptr]*AP1.t[i];";
  z[lc++]="      rptr = NVUI(%1$s_osincr) - fptr;";
  z[lc++]="      fptr += NVUI(%1$s_osincr);";
  z[lc++]="      k = 0;";
  z[lc++]="      while (rptr < SINC_SIZE)";
  z[lc++]="       {";
  z[lc++]="         if ((i + (++k)) < AP1.len)";
  z[lc++]="           ret += sinc[rptr]*AP1.t[i + k];";
  z[lc++]="         else";
  z[lc++]="           ret += sinc[rptr]*AP1.t[i + k - AP1.len];";
  z[lc++]="         rptr += NVUI(%1$s_osincr);";
  z[lc++]="         if (fptr < SINC_SIZE)";
  z[lc++]="          {";
  z[lc++]="            if (i >= k)";
  z[lc++]="              ret += sinc[fptr]*AP1.t[i - k];";
  z[lc++]="            else";
  z[lc++]="              ret += sinc[fptr]*AP1.t[AP1.len + i - k];";
  z[lc++]="            fptr += NVUI(%1$s_osincr);";
  z[lc++]="          }";
  z[lc++]="       }";
  z[lc++]="      if (NVUI(%1$s_sfui) != 0x00010000)";
  z[lc++]="        ret *= NV(%1$s_sffl);";
  z[lc++]="     }";
  z[lc++]="   }";
  z[lc++]="}";
  z[lc++]="else";             /* for first time through */ 
  z[lc++]="{";
  z[lc++]="  if ((NVI(%1$s_fsign) = (freq >= 0)))";
  z[lc++]="   {";
  z[lc++]="    nint = (unsigned int)(index = freq*AP1.oconst);";
  z[lc++]="    while (index >= AP1.lenf)";
  z[lc++]="      nint = (unsigned int)(index -= AP1.lenf);";
  z[lc++]="    nfrac = (unsigned int)(4294967296.0F*(index - nint));";  
  z[lc++]="   }";
  z[lc++]="  else";
  z[lc++]="   {";
  z[lc++]="    nint = (unsigned int)(index = -freq*AP1.oconst);";
  z[lc++]="    while (index >= AP1.lenf)";
  z[lc++]="      nint = (unsigned int)(index -= AP1.lenf);";
  z[lc++]="    nfrac = (unsigned int)(4294967296.0F*(index - nint));";
  z[lc++]="   }";
  z[lc++]="  NVUI(%1$s_kint) = nint;";
  z[lc++]="  NVUI(%1$s_kfrac) = nfrac;";
  z[lc++]="  NVI(%1$s_kcyc) = 1;";
  z[lc++]="  NVI(%1$s_iloops)= (int)(ROUND(va_loops));";
  z[lc++]="  if (index <= 1.0F)";     /* set interpolation values */
  z[lc++]="   {";
  z[lc++]="    NV(%1$s_sffl) = 1.0F;";
  z[lc++]="    NVUI(%1$s_sfui) = 0x00010000;";
  z[lc++]="    NVUI(%1$s_osincr) = SINC_PILEN;";
  z[lc++]="   }";
  z[lc++]="  else";
  z[lc++]="   {";
  z[lc++]="    if (index < SINC_UPMAX)";
  z[lc++]="      NV(%1$s_sffl) = 1.0F/index;";
  z[lc++]="    else";
  z[lc++]="      NV(%1$s_sffl) = 1.0F/SINC_UPMAX;";
  z[lc++]="    NVUI(%1$s_sfui) = ((float)(pow(2,16)))*NV(%1$s_sffl) + 0.5F;";
  z[lc++]="    NVUI(%1$s_osincr) = (SINC_PILEN*NVUI(%1$s_sfui)) >> 16;";
  z[lc++]="   }";
  z[lc++]="  ret = AP1.t[0];";
  z[lc++]="  if (index > 1.0F)";
  z[lc++]="   {";
  z[lc++]="    fptr = rptr = NVUI(%1$s_osincr);";
  z[lc++]="    k = 0;";
  z[lc++]="    while (rptr < SINC_SIZE)";
  z[lc++]="     {";
  z[lc++]="       if ((++k) < AP1.len)";
  z[lc++]="         ret += sinc[rptr]*AP1.t[k];";
  z[lc++]="       else";
  z[lc++]="         ret += sinc[rptr]*AP1.t[k - AP1.len];";
  z[lc++]="       rptr += NVUI(%1$s_osincr);";
  z[lc++]="       if (fptr < SINC_SIZE)";
  z[lc++]="        {";
  z[lc++]="          ret += sinc[fptr]*AP1.t[AP1.len - k];";
  z[lc++]="          fptr += NVUI(%1$s_osincr);";
  z[lc++]="        }";
  z[lc++]="     }";
  z[lc++]="    ret *= NV(%1$s_sffl);";
  z[lc++]="   }";
  z[lc++]="}";
  printblock(lc);
}

/*********************************************************/
/*      oscil() code for loop case -- i-rate freq        */
/*********************************************************/

void looposcilifreq(tnode * tptr)

{
  int lc = 0;

  z[lc++]="if (NVI(%1$s_kcyc))";   /* fast path */
  z[lc++]="{";
  z[lc++]="  if (NVI(%1$s_fsign))";  /* positive frequency */
  z[lc++]="   {";
  z[lc++]="    NVUI(%1$s_pfrac) = (j = NVUI(%1$s_pfrac)) + NVUI(%1$s_kfrac);";
  z[lc++]="    NVUI(%1$s_pint) += NVUI(%1$s_kint) + (NVUI(%1$s_pfrac) < j);";
  z[lc++]="    if ((i = NVUI(%1$s_pint)) >= AP1.len)";
  z[lc++]="     {";
  z[lc++]="      i = (NVUI(%1$s_pint) -= AP1.len);";
  z[lc++]="      if (NVI(%1$s_iloops) > 0)";
  z[lc++]="        NVI(%1$s_iloops)--;";
  z[lc++]="     }";
  z[lc++]="   }";
  z[lc++]="  else";   /* negative frequency */
  z[lc++]="   {";
  z[lc++]="    NVUI(%1$s_pfrac) = (j = NVUI(%1$s_pfrac)) - NVUI(%1$s_kfrac);";
  z[lc++]="    NVUI(%1$s_pint) -= NVUI(%1$s_kint) + (NVUI(%1$s_pfrac) > j);";
  z[lc++]="    if ((i = NVUI(%1$s_pint)) >= AP1.len)";
  z[lc++]="     {";
  z[lc++]="       i = (NVUI(%1$s_pint) += AP1.len);";
  z[lc++]="       if (NVI(%1$s_iloops) > 0)";
  z[lc++]="         NVI(%1$s_iloops)++;";
  z[lc++]="     }";
  z[lc++]="   }";
  z[lc++]="  if (NVI(%1$s_iloops) == 0)";
  z[lc++]="    ret = 0.0F;";
  z[lc++]="  else";
  z[lc++]="    ret = AP1.t[i] + NVUI(%1$s_pfrac)*";
  z[lc++]="        ((float)(1.0/4294967296.0))*(AP1.t[i+1] - AP1.t[i]);";  
  z[lc++]="}";
  z[lc++]="else";             /* for first time through */ 
  z[lc++]="{";
  z[lc++]="  if ((NVI(%1$s_fsign) = (freq >= 0)))";
  z[lc++]="   {";
  z[lc++]="    nint = (unsigned int)(index = freq*AP1.oconst);";
  z[lc++]="    while (index >= AP1.lenf)";
  z[lc++]="      nint = (unsigned int)(index -= AP1.lenf);";
  z[lc++]="    nfrac = (unsigned int)(4294967296.0F*(index - nint));";  
  z[lc++]="   }";
  z[lc++]="  else";
  z[lc++]="   {";
  z[lc++]="    nint = (unsigned int)(index = -freq*AP1.oconst);";
  z[lc++]="    while (index >= AP1.lenf)";
  z[lc++]="      nint = (unsigned int)(index -= AP1.lenf);";
  z[lc++]="    nfrac = (unsigned int)(4294967296.0F*(index - nint));";
  z[lc++]="   }";
  z[lc++]="  NVUI(%1$s_kint) = nint;";
  z[lc++]="  NVUI(%1$s_kfrac) = nfrac;";
  z[lc++]="  NVI(%1$s_kcyc) = 1;";
  z[lc++]="  NVI(%1$s_iloops)= (int)(ROUND(va_loops));";
  z[lc++]="  ret = AP1.t[0];";
  z[lc++]="}";
  printblock(lc);
}


/*********************************************************/
/*       determines if oscil is finite-loop              */
/*********************************************************/

int finitelooposcil(tnode * tptr)

{
  tptr = firstopcodearg(tptr); /* first arg, table t*/
  tptr = tptr->next->next;     /* second arg, freq */

  if (tptr->next == NULL)  /* if no loops specified, infinite */
    return 0;

  tptr = tptr->next->next;   /* third argument, loops */
  if (tptr->vol == VARIABLE) /* if loops not constant, can't tell */
    return 1;

  if (tptr->res == ASINT)
    {
      if (atoi(tptr->down->val) <= 0)
	return 0;
      else
	return 1;
    }
  else
    {
      if (atof(tptr->down->val) <= 0.0F)
	return 0;
      else
	return 1;
    }
}

/*********************************************************/
/*       determines rate of oscil frequency              */
/*********************************************************/

int looposcilrate(tnode * tptr)

{

  tptr = firstopcodearg(tptr);   /* first arg, table t*/
  return tptr->next->next->rate; /* second arg, freq */

}

/* to do: change for new constants */

/*********************************************************/
/*       sets up constants for loscil() code             */
/*********************************************************/

void loscilsetup(tnode * tptr, int * lcptr, int passtype)

{
  int lc;

  lc = * lcptr;

  if (tptr->extra == NULL)
    {
      if (passtype == KRATETYPE)
	{
	  z[lc++]="     if (NV(%1$s_stamp) != AP1.stamp)";
	  z[lc++]="     {";
	}

      z[lc++]="     if (AP1.base <= 0.0F) ";
      genex(&lc,tptr->optr->down,"Basefreq <= 0");
      z[lc++]="     NV(%1$s_lconst) = EV(ATIME)*AP1.sr/AP1.base;";

      z[lc++]="     if (AP1.start >= AP1.tend) ";
      genex(&lc,tptr->optr->down,"Loopstart > loopend");

      z[lc++]="     NVUI(%1$s_tstartint) = AP1.start;";
      z[lc++]="     NVUI(%1$s_tendint) = AP1.tend;";   
      z[lc++]="     NVUI(%1$s_dint) = AP1.tend - AP1.start;";
      z[lc++]="     NV(%1$s_rollover) = 4294967291U - NVUI(%1$s_tendint);";
      z[lc++]="     NV(%1$s_stamp) = AP1.stamp;";

      if (passtype == KRATETYPE)
	{
	  z[lc++]="     }";
	}
      *lcptr = lc;
      return;
    }

  if (tptr->extra->next == NULL)
    {
      if (passtype == IRATETYPE)
	{
	  z[lc++]="     if (va_basefreq <= 0.0F) ";
	  genex(&lc,tptr->optr->down,"Basefreq <= 0");
	}
      else
	{
	  z[lc++]="     if (NV(%1$s_stamp) != AP1.stamp)";
	  z[lc++]="     {";
	}

      z[lc++]="     NV(%1$s_lconst) = EV(ATIME)*AP1.sr/va_basefreq;";
      z[lc++]="     if (AP1.start >= AP1.tend) ";
      genex(&lc,tptr->optr->down,"Loopstart > loopend");
      z[lc++]="     NVUI(%1$s_tstartint) = AP1.start;";
      z[lc++]="     NVUI(%1$s_tendint) = AP1.tend;";   
      z[lc++]="     NVUI(%1$s_dint) = AP1.tend - AP1.start;"; 
      z[lc++]="     NV(%1$s_rollover) = 4294967291U - NVUI(%1$s_tendint);";
      z[lc++]="     NV(%1$s_stamp) = AP1.stamp;";

      if (passtype == KRATETYPE)
	{
	  z[lc++]="     }";
	}
      *lcptr = lc;
      return;
    }

  if (tptr->extra->next->next->next == NULL)
    {
      if (passtype == IRATETYPE)
	{
	  z[lc++]="     if (va_loopstart < 0.0F) ";
	  genex(&lc,tptr->optr->down,"Loopstart < 0");
	  z[lc++]="     if (va_basefreq <= 0.0F) ";
	  genex(&lc,tptr->optr->down,"Basefreq <= 0");
	}
      else
	{
	  z[lc++]="     if (NV(%1$s_stamp) != AP1.stamp)";
	  z[lc++]="     {";
	}


      z[lc++]="     NV(%1$s_lconst) = EV(ATIME)*AP1.sr/va_basefreq;";
      z[lc++]="     if (va_loopstart >= AP1.tend) ";
      genex(&lc,tptr->optr->down,"Loopstart > loopend");
      z[lc++]="     NVUI(%1$s_tstartint) = (int)(va_loopstart);";
      z[lc++]="     NVUI(%1$s_tendint) = AP1.tend;";    
      z[lc++]="     NVUI(%1$s_dint) = AP1.tend - va_loopstart;";
      z[lc++]="     NV(%1$s_rollover) = 4294967291U - NVUI(%1$s_tendint);";
      z[lc++]="     NV(%1$s_stamp) = AP1.stamp;";

      if (passtype == KRATETYPE)
	{
	  z[lc++]="     }";
	}
      *lcptr = lc;
      return;
    }


  if (passtype == IRATETYPE)
    {
      z[lc++]="     if (va_loopstart < 0.0F) ";
      genex(&lc,tptr->optr->down,"Loopstart < 0");
      z[lc++]="     if (va_loopend < 0.0F) ";
      genex(&lc,tptr->optr->down,"Loopend < 0");
      z[lc++]="     if (va_basefreq <= 0.0F) ";
      genex(&lc,tptr->optr->down,"Basefreq <= 0");
    }
  else
    {
      z[lc++]="     if (NV(%1$s_stamp) != AP1.stamp)";
      z[lc++]="     {";
    }

  z[lc++]="     NV(%1$s_lconst) = EV(ATIME)*AP1.sr/va_basefreq;";
  z[lc++]="     if (va_loopstart >= va_loopend) ";
  genex(&lc,tptr->optr->down,"Loopstart > loopend");
  z[lc++]="     NVUI(%1$s_tstartint) = (int)(va_loopstart);";
  z[lc++]="     NVUI(%1$s_tendint) = (int)(va_loopend);";    
  z[lc++]="     NVUI(%1$s_dint) = va_loopend - va_loopstart;"; 
  z[lc++]="     NV(%1$s_rollover) = 4294967291U - NVUI(%1$s_tendint);";
  z[lc++]="     NV(%1$s_stamp) = AP1.stamp;";
  
  if (passtype == KRATETYPE)
    {
      z[lc++]="     }";
    }
  *lcptr = lc;
  return;
  
}

/*********************************************************/
/*               loscil() code in progress               */
/*********************************************************/

void loscil_sinc(tnode * tptr)

{
  int lc = 0;

  acycleguard(tptr, &lc);/* fast loop taken most of the time */
  z[lc++]="{";
  z[lc++]="  if (freq >= 0)";  /* positive frequency */
  z[lc++]="   {";
  z[lc++]="    if ((index = freq*NV(%1$s_lconst)) >= NV(%1$s_rollover))";
  z[lc++]="     index = fmod(index, NVUI(%1$s_dint));";
  z[lc++]="    nint = (unsigned int)(index);";
  z[lc++]="    nfrac = (unsigned int)(4294967296.0F*(index - nint));";
  z[lc++]="    NVUI(%1$s_pfrac) = (j = NVUI(%1$s_pfrac)) + nfrac;";
  z[lc++]="    NVUI(%1$s_pint) += nint + (NVUI(%1$s_pfrac) < j);";
  z[lc++]="    while ((i = NVUI(%1$s_pint)) > NVUI(%1$s_tendint))";
  z[lc++]="     {";
  z[lc++]="        NVUI(%1$s_pint) += - NVUI(%1$s_dint);";
  z[lc++]="        NVUI(%1$s_second) = 1;";
  z[lc++]="     }";
  z[lc++]="   }";
  z[lc++]="  else";   /* negative frequency */
  z[lc++]="   {";
  z[lc++]="    inloop = (NVUI(%1$s_pint) > NVUI(%1$s_tstartint));";
  z[lc++]="    index = -freq*NV(%1$s_lconst);";
  z[lc++]="    if (inloop && (index > NVUI(%1$s_dint) + 1))";
  z[lc++]="     index = fmod(index, NVUI(%1$s_dint));";
  z[lc++]="    if (!inloop && (index > NVUI(%1$s_tstartint) + 1))";
  z[lc++]="     index = 0.0F;";
  z[lc++]="    nint = (unsigned int)(index);";
  z[lc++]="    nfrac = (unsigned int)(4294967296.0F*(index - nint));";
  z[lc++]="    NVUI(%1$s_pfrac) = (j = NVUI(%1$s_pfrac)) - nfrac;";
  z[lc++]="    k = NVUI(%1$s_pint);";
  z[lc++]="    NVUI(%1$s_pint) -= nint + (NVUI(%1$s_pfrac) > j);";
  z[lc++]="    if (k < NVUI(%1$s_pint))";
  z[lc++]="      NVUI(%1$s_pint) = NVUI(%1$s_pfrac) = 0;";
  z[lc++]="    while (inloop && ((i = NVUI(%1$s_pint)) < NVUI(%1$s_tstartint)))";
  z[lc++]="       NVUI(%1$s_pint) += NVUI(%1$s_dint);";
  z[lc++]="   }";
  z[lc++]="  if ((nint == 1) && (nfrac == 0))";
  z[lc++]="    ret = AP1.t[i];";
  z[lc++]="  else";
  z[lc++]="   {";
  z[lc++]="    if (index <= 1.0F)";
  z[lc++]="     {";
  z[lc++]="      sffl = 1.0F;";
  z[lc++]="      sfui = 0x00010000;";
  z[lc++]="      losincr = SINC_PILEN;";
  z[lc++]="     }";
  z[lc++]="    else";
  z[lc++]="     {";
  z[lc++]="       if (index < SINC_UPMAX)";
  z[lc++]="         sffl = 1.0F/index;";
  z[lc++]="       else";
  z[lc++]="         sffl = 1.0F/SINC_UPMAX;";
  z[lc++]="      sfui = ((float)(pow(2,16)))*sffl + 0.5F;";
  z[lc++]="      losincr = (SINC_PILEN*sfui) >> 16;";
  z[lc++]="     }";
  z[lc++]="    if (sfui == 0x00010000)";
  z[lc++]="     fptr = (NVUI(%1$s_pfrac)) >> (32 - SINC_LOG_PILEN);";
  z[lc++]="    else";
  z[lc++]="     fptr = (sfui*((NVUI(%1$s_pfrac)) >> 16)) >> (32 - SINC_LOG_PILEN);";
  z[lc++]="    ret = sinc[fptr]*AP1.t[i];";
  z[lc++]="    rptr = losincr - fptr;";
  z[lc++]="    fptr += losincr;";
  z[lc++]="    k = 0;";
  z[lc++]="    while (rptr < SINC_SIZE)";
  z[lc++]="     {";
  z[lc++]="       if ((i + (++k)) <= NVUI(%1$s_tendint))";
  z[lc++]="         ret += sinc[rptr]*AP1.t[i + k];";
  z[lc++]="       else";
  z[lc++]="         ret += sinc[rptr]*AP1.t[NVUI(%1$s_tstartint) +";
  z[lc++]="                                 i + k - NVUI(%1$s_tendint) - 1];";
  z[lc++]="       rptr += losincr;";
  z[lc++]="       if (fptr < SINC_SIZE)";
  z[lc++]="        {";
  z[lc++]="          if (NVUI(%1$s_second))";
  z[lc++]="           {";
  z[lc++]="             if (i >= NVUI(%1$s_tstartint) + k)";
  z[lc++]="               ret += sinc[fptr]*AP1.t[i - k];";
  z[lc++]="             else";
  z[lc++]="               ret += sinc[fptr]*AP1.t[NVUI(%1$s_tendint)";
  z[lc++]="                         - NVUI(%1$s_tstartint) + i - k + 1];";
  z[lc++]="           }";
  z[lc++]="          else";
  z[lc++]="           if (i >= k)";
  z[lc++]="             ret += sinc[fptr]*AP1.t[i - k];";
  z[lc++]="          fptr += losincr;";
  z[lc++]="        }";
  z[lc++]="     }";
  z[lc++]="    if (sfui != 0x00010000)";
  z[lc++]="      ret *= sffl;";
  z[lc++]="   }";
  z[lc++]="}";
  z[lc++]="else";             /* for first time through */ 
  z[lc++]=" {";
  z[lc++]="  if (NVI(%1$s_kcyc)==0)";
  z[lc++]="   {";
  kcycassign(tptr, &lc);
  loscilsetup(tptr,&lc,IRATETYPE);
  z[lc++]="    if (freq >= 0)";  /* positive frequency */
  z[lc++]="     {";
  z[lc++]="      if ((index = freq*NV(%1$s_lconst)) >= NV(%1$s_rollover))";
  z[lc++]="       index = fmod(index, NVUI(%1$s_dint));";
  z[lc++]="      nint = (unsigned int)(index);";
  z[lc++]="      nfrac = (unsigned int)(4294967296.0F*(index - nint));";
  z[lc++]="     }";
  z[lc++]="    else";   /* negative frequency */
  z[lc++]="     {";
  z[lc++]="      inloop = (NVUI(%1$s_pint) > NVUI(%1$s_tstartint));";
  z[lc++]="      index = -freq*NV(%1$s_lconst);";
  z[lc++]="      if (inloop && (index > NVUI(%1$s_dint) + 1))";
  z[lc++]="       index = fmod(index, NVUI(%1$s_dint));";
  z[lc++]="      if (!inloop && (index > NVUI(%1$s_tstartint) + 1))";
  z[lc++]="       index = 0.0F;";
  z[lc++]="      nint = (unsigned int)(index);";
  z[lc++]="      nfrac = (unsigned int)(4294967296.0F*(index - nint));";
  z[lc++]="     }";
  z[lc++]="    ret = AP1.t[0];";
  z[lc++]="    if (index > 1.0F)";
  z[lc++]="     {";
  z[lc++]="       if (index < SINC_UPMAX)";
  z[lc++]="         sffl = 1.0F/index;";
  z[lc++]="       else";
  z[lc++]="         sffl = 1.0F/SINC_UPMAX;";
  z[lc++]="      sfui = ((float)(pow(2,16)))*sffl + 0.5F;";
  z[lc++]="      rptr = losincr = (SINC_PILEN*sfui) >> 16;";
  z[lc++]="      k = 0;";
  z[lc++]="      while (rptr < SINC_SIZE)";
  z[lc++]="       {";
  z[lc++]="         if ((++k) <= NVUI(%1$s_tendint))";
  z[lc++]="           ret += sinc[rptr]*AP1.t[k];";
  z[lc++]="         else";
  z[lc++]="           ret += sinc[rptr]*AP1.t[NVUI(%1$s_tstartint) +";
  z[lc++]="                                   k - NVUI(%1$s_tendint) - 1];";
  z[lc++]="         rptr += losincr;";
  z[lc++]="       }";
  z[lc++]="     ret *= sffl;";
  z[lc++]="    }";
  z[lc++]="   }";
  z[lc++]="  else";
  z[lc++]="   {"; 
  kcycassign2(tptr, &lc);
  loscilsetup(tptr,&lc,KRATETYPE); /* repeated for SASL tables */
  z[lc++]="    if (freq >= 0)";  /* positive frequency */
  z[lc++]="     {";
  z[lc++]="      if ((index = freq*NV(%1$s_lconst)) >= NV(%1$s_rollover))";
  z[lc++]="       index = fmod(index, NVUI(%1$s_dint));";
  z[lc++]="      nint = (unsigned int)(index);";
  z[lc++]="      nfrac = (unsigned int)(4294967296.0F*(index - nint));";
  z[lc++]="      NVUI(%1$s_pfrac) = (j = NVUI(%1$s_pfrac)) + nfrac;";
  z[lc++]="      NVUI(%1$s_pint) += nint + (NVUI(%1$s_pfrac) < j);";
  z[lc++]="      while ((i = NVUI(%1$s_pint)) > NVUI(%1$s_tendint))";
  z[lc++]="       {";
  z[lc++]="          NVUI(%1$s_pint) += - NVUI(%1$s_dint);";
  z[lc++]="          NVUI(%1$s_second) = 1;";
  z[lc++]="       }";
  z[lc++]="     }";
  z[lc++]="    else";   /* negative frequency */
  z[lc++]="     {";
  z[lc++]="      inloop = (NVUI(%1$s_pint) > NVUI(%1$s_tstartint));";
  z[lc++]="      index = -freq*NV(%1$s_lconst);";
  z[lc++]="      if (inloop && (index > NVUI(%1$s_dint) + 1))";
  z[lc++]="       index = fmod(index, NVUI(%1$s_dint));";
  z[lc++]="      if (!inloop && (index > NVUI(%1$s_tstartint) + 1))";
  z[lc++]="       index = 0.0F;";
  z[lc++]="      nint = (unsigned int)(index);";
  z[lc++]="      nfrac = (unsigned int)(4294967296.0F*(index - nint));";
  z[lc++]="      NVUI(%1$s_pfrac) = (j = NVUI(%1$s_pfrac)) - nfrac;";
  z[lc++]="      k = NVUI(%1$s_pint);";
  z[lc++]="      NVUI(%1$s_pint) -= nint + (NVUI(%1$s_pfrac) > j);";
  z[lc++]="      if (k < NVUI(%1$s_pint))";
  z[lc++]="        NVUI(%1$s_pint) = NVUI(%1$s_pfrac) = 0;";
  z[lc++]="      while (inloop && ((i = NVUI(%1$s_pint)) < NVUI(%1$s_tstartint)))";
  z[lc++]="         NVUI(%1$s_pint) += NVUI(%1$s_dint);";
  z[lc++]="     }";
  z[lc++]="    if ((nint == 1) && (nfrac == 0))";
  z[lc++]="      ret = AP1.t[i];";
  z[lc++]="    else";
  z[lc++]="     {";
  z[lc++]="      if (index <= 1.0F)";
  z[lc++]="       {";
  z[lc++]="        sffl = 1.0F;";
  z[lc++]="        sfui = 0x00010000;";
  z[lc++]="        losincr = SINC_PILEN;";
  z[lc++]="       }";
  z[lc++]="      else";
  z[lc++]="       {";
  z[lc++]="         if (index < SINC_UPMAX)";
  z[lc++]="           sffl = 1.0F/index;";
  z[lc++]="         else";
  z[lc++]="           sffl = 1.0F/SINC_UPMAX;";
  z[lc++]="        sfui = ((float)(pow(2,16)))*sffl + 0.5F;";
  z[lc++]="        losincr = (SINC_PILEN*sfui) >> 16;";
  z[lc++]="       }";
  z[lc++]="      if (sfui == 0x00010000)";
  z[lc++]="       fptr = (NVUI(%1$s_pfrac)) >> (32 - SINC_LOG_PILEN);";
  z[lc++]="      else";
  z[lc++]="       fptr = (sfui*((NVUI(%1$s_pfrac)) >> 16)) >> (32 - SINC_LOG_PILEN);";
  z[lc++]="      ret = sinc[fptr]*AP1.t[i];";
  z[lc++]="      rptr = losincr - fptr;";
  z[lc++]="      fptr += losincr;";
  z[lc++]="      k = 0;";
  z[lc++]="      while (rptr < SINC_SIZE)";
  z[lc++]="       {";
  z[lc++]="         if ((i + (++k)) <= NVUI(%1$s_tendint))";
  z[lc++]="           ret += sinc[rptr]*AP1.t[i + k];";
  z[lc++]="         else";
  z[lc++]="           ret += sinc[rptr]*AP1.t[NVUI(%1$s_tstartint) +";
  z[lc++]="                                   i + k - NVUI(%1$s_tendint) - 1];";
  z[lc++]="         rptr += losincr;";
  z[lc++]="         if (fptr < SINC_SIZE)";
  z[lc++]="          {";
  z[lc++]="            if (NVUI(%1$s_second))";
  z[lc++]="             {";
  z[lc++]="               if (i >= NVUI(%1$s_tstartint) + k)";
  z[lc++]="                 ret += sinc[fptr]*AP1.t[i - k];";
  z[lc++]="               else";
  z[lc++]="                 ret += sinc[fptr]*AP1.t[NVUI(%1$s_tendint)";
  z[lc++]="                           - NVUI(%1$s_tstartint) + i - k + 1];";
  z[lc++]="             }";
  z[lc++]="            else";
  z[lc++]="             if (i >= k)";
  z[lc++]="               ret += sinc[fptr]*AP1.t[i - k];";
  z[lc++]="            fptr += losincr;";
  z[lc++]="          }";
  z[lc++]="       }";
  z[lc++]="      if (sfui != 0x00010000)";
  z[lc++]="        ret *= sffl;";
  z[lc++]="     }";
  z[lc++]="   }";
  z[lc++]=" }";
  printblock(lc);
}


/*********************************************************/
/*               loscil() code in progress               */
/*********************************************************/

void loscil(tnode * tptr)

{
  int lc = 0;

  acycleguard(tptr, &lc);/* fast loop taken most of the time */
  z[lc++]="{";
  z[lc++]="  if (freq >= 0)";  /* positive frequency */
  z[lc++]="   {";
  z[lc++]="    if ((index = freq*NV(%1$s_lconst)) >= NV(%1$s_rollover))";
  z[lc++]="     index = fmod(index, NVUI(%1$s_dint));";
  z[lc++]="    nint = (unsigned int)(index);";
  z[lc++]="    nfrac = (unsigned int)(4294967296.0F*(index - nint));";
  z[lc++]="    NVUI(%1$s_pfrac) = (j = NVUI(%1$s_pfrac)) + nfrac;";
  z[lc++]="    NVUI(%1$s_pint) += nint + (NVUI(%1$s_pfrac) < j);";
  z[lc++]="    while ((i = NVUI(%1$s_pint)) > NVUI(%1$s_tendint))";
  z[lc++]="      NVUI(%1$s_pint) += - NVUI(%1$s_dint);";
  z[lc++]="   }";
  z[lc++]="  else";   /* negative frequency */
  z[lc++]="   {";
  z[lc++]="    inloop = (NVUI(%1$s_pint) > NVUI(%1$s_tstartint));";
  z[lc++]="    index = -freq*NV(%1$s_lconst);";
  z[lc++]="    if (inloop && (index > NVUI(%1$s_dint) + 1))";
  z[lc++]="     index = fmod(index, NVUI(%1$s_dint));";
  z[lc++]="    if (!inloop && (index > NVUI(%1$s_tstartint) + 1))";
  z[lc++]="     index = 0.0F;";
  z[lc++]="    nint = (unsigned int)(index);";
  z[lc++]="    nfrac = (unsigned int)(4294967296.0F*(index - nint));";
  z[lc++]="    NVUI(%1$s_pfrac) = (j = NVUI(%1$s_pfrac)) - nfrac;";
  z[lc++]="    k = NVUI(%1$s_pint);";
  z[lc++]="    NVUI(%1$s_pint) -= nint + (NVUI(%1$s_pfrac) > j);";
  z[lc++]="    if (k < NVUI(%1$s_pint))";
  z[lc++]="      NVUI(%1$s_pint) = NVUI(%1$s_pfrac) = 0;";
  z[lc++]="    while (inloop && ((i = NVUI(%1$s_pint)) < NVUI(%1$s_tstartint)))";
  z[lc++]="       NVUI(%1$s_pint) += NVUI(%1$s_dint);";
  z[lc++]="   }";
  z[lc++]="  ret = AP1.t[i];";
  z[lc++]="  if (i < NVUI(%1$s_tendint))";
  z[lc++]="    ret += NVUI(%1$s_pfrac)*((float)(1.0/4294967296.0))";
  z[lc++]="           *(AP1.t[i+1] - AP1.t[i]);";
  z[lc++]="  else";
  z[lc++]="    ret += NVUI(%1$s_pfrac)*((float)(1.0/4294967296.0))";
  z[lc++]="           *(AP1.t[NVUI(%1$s_tstartint)] - AP1.t[i]);";
  z[lc++]="}";
  z[lc++]="else";             /* for first time through */ 
  z[lc++]=" {";
  z[lc++]="  if (NVI(%1$s_kcyc)==0)";
  z[lc++]="   {";
  kcycassign(tptr, &lc);
  loscilsetup(tptr,&lc,IRATETYPE);
  z[lc++]="     ret = AP1.t[0];";
  z[lc++]="   }";
  z[lc++]="  else";
  z[lc++]="   {"; 
  kcycassign2(tptr, &lc);
  loscilsetup(tptr,&lc,KRATETYPE); /* repeated for SASL tables */
  z[lc++]="  if (freq >= 0)";  /* positive frequency */
  z[lc++]="   {";
  z[lc++]="    if ((index = freq*NV(%1$s_lconst)) >= NV(%1$s_rollover))";
  z[lc++]="     index = fmod(index, NVUI(%1$s_dint));";
  z[lc++]="    nint = (unsigned int)(index);";
  z[lc++]="    nfrac = (unsigned int)(4294967296.0F*(index - nint));";
  z[lc++]="    NVUI(%1$s_pfrac) = (j = NVUI(%1$s_pfrac)) + nfrac;";
  z[lc++]="    NVUI(%1$s_pint) += nint + (NVUI(%1$s_pfrac) < j);";
  z[lc++]="    while ((i = NVUI(%1$s_pint)) > NVUI(%1$s_tendint))";
  z[lc++]="      NVUI(%1$s_pint) += - NVUI(%1$s_dint);";
  z[lc++]="   }";
  z[lc++]="  else";   /* negative frequency */
  z[lc++]="   {";
  z[lc++]="    inloop = (NVUI(%1$s_pint) > NVUI(%1$s_tstartint));";
  z[lc++]="    index = -freq*NV(%1$s_lconst);";
  z[lc++]="    if (inloop && (index > NVUI(%1$s_dint) + 1))";
  z[lc++]="     index = fmod(index, NVUI(%1$s_dint));";
  z[lc++]="    if (!inloop && (index > NVUI(%1$s_tstartint) + 1))";
  z[lc++]="     index = 0.0F;";
  z[lc++]="    nint = (unsigned int)(index);";
  z[lc++]="    nfrac = (unsigned int)(4294967296.0F*(index - nint));";
  z[lc++]="    NVUI(%1$s_pfrac) = (j = NVUI(%1$s_pfrac)) - nfrac;";
  z[lc++]="    k = NVUI(%1$s_pint);";
  z[lc++]="    NVUI(%1$s_pint) -= nint + (NVUI(%1$s_pfrac) > j);";
  z[lc++]="    if (k < NVUI(%1$s_pint))";
  z[lc++]="      NVUI(%1$s_pint) = NVUI(%1$s_pfrac) = 0;";
  z[lc++]="    while (inloop && ((i = NVUI(%1$s_pint)) < NVUI(%1$s_tstartint)))";
  z[lc++]="       NVUI(%1$s_pint) += NVUI(%1$s_dint);";
  z[lc++]="   }";
  z[lc++]="  ret = AP1.t[i];";
  z[lc++]="  if (i < NVUI(%1$s_tendint))";
  z[lc++]="    ret += NVUI(%1$s_pfrac)*((float)(1.0/4294967296.0))";
  z[lc++]="           *(AP1.t[i+1] - AP1.t[i]);";
  z[lc++]="  else";
  z[lc++]="    ret += NVUI(%1$s_pfrac)*((float)(1.0/4294967296.0))";
  z[lc++]="           *(AP1.t[NVUI(%1$s_tstartint)] - AP1.t[i]);";
  z[lc++]="  }";
  z[lc++]=" }";
  printblock(lc);
}


/*********************************************************/
/*                  code for method 1                    */
/*********************************************************/


void fracdelaym1(tnode * tptr, int * lcptr)

{
  int lc;

  lc = * lcptr;

  if (tptr->extra == NULL)
    {
      genex(&lc,tptr->optr->down,"Delay length unspecified (method 1)");
    }
  else
    {
      z[lc++]= " ret = 0;";
      z[lc++]= " i = (int)(va_p1*EV(ARATE));";
      z[lc++]= " if (i<=0)";
      genex(&lc,tptr->optr->down,"Negative p1 (method 1)");
      z[lc++]= " if (NT(TBL_%1$s_dline).len != i)";
      z[lc++]= " {";
      z[lc++]= "   if (NT(TBL_%1$s_dline).t)";
      z[lc++]= "     free(NT(TBL_%1$s_dline).t);";
      z[lc++]= "   NT(TBL_%1$s_dline).len = i;";
      z[lc++]= "   NT(TBL_%1$s_dline).tend = i - 1;";
      z[lc++]= "   NT(TBL_%1$s_dline).t=(float *)calloc(i,sizeof(float)); ";
      z[lc++]= "   NT(TBL_%1$s_dline).llmem=1; ";
      z[lc++]= " }";
      z[lc++]= " else";
      z[lc++]= "   while (i > 0) ";
      z[lc++]= "    NT(TBL_%1$s_dline).t[--i] = 0.0F;";
    }

  *lcptr = lc;
  return;
}

/*********************************************************/
/*                  code for method 2                    */
/*********************************************************/

void fracdelaym2(tnode * tptr, int * lcptr)

{
  int lc;

  lc = * lcptr;

  if (tptr->extra == NULL)
    {
      genex(&lc,tptr->optr->down,"Tap position unspecified (method 2)");
    }
  else
    {
      if (isocheck)
	{
	  z[lc++]= " if (NT(TBL_%1$s_dline).t == NULL)";
	  genex(&lc,tptr->optr->down,"Delay line uninitialized (method 2)");
	  z[lc++]= " index = va_p1*EV(ARATE);";
	  z[lc++]= " len = NT(TBL_%1$s_dline).len;";
	  z[lc++]= " if (index < 0.0F)";
	  z[lc++]= "  index = 0.0F;";
	  z[lc++]= " if (index > len - 1)";
	  z[lc++]= "  index = len - 1;";
	  z[lc++]= " i = (int) index;";
	  z[lc++]= " index = index - i;";
	  z[lc++]= " i = (i + NT(TBL_%1$s_dline).tend) % len;";
	}
      else
	{
	  z[lc++]= " i = (int)(index = va_p1*EV(ARATE));";
	  z[lc++]= " index -= i;";
	  z[lc++]= " len = NT(TBL_%1$s_dline).len;";
	  z[lc++]= " if ((i+= NT(TBL_%1$s_dline).tend) >= len)";
	  z[lc++]= "   i -= len;";
	}

      if (interp == INTERP_LINEAR)
	{
	  z[lc++]= " ret = NT(TBL_%1$s_dline).t[i];";
	  z[lc++]= " k = i + 1;";
	  z[lc++]= " if ((k < len) && (k != NT(TBL_%1$s_dline).tend))";
	  z[lc++]= "  ret+= index*(NT(TBL_%1$s_dline).t[k] - ret);";
	  z[lc++]= " else";
	  z[lc++]= "  {";
	  z[lc++]= "     if (k == len)";
	  z[lc++]= "      {";
	  z[lc++]= "        if (NT(TBL_%1$s_dline).tend)";
	  z[lc++]= "          ret+= index*(NT(TBL_%1$s_dline).t[0] - ret);";
	  z[lc++]= "        else";
	  z[lc++]= "          ret+= index*(NT(TBL_%1$s_dline).t[len-2] - ret);";
	  z[lc++]= "      }";
	  z[lc++]= "     else";
	  z[lc++]= "      {";
	  z[lc++]= "        if (k >= 2)";
	  z[lc++]= "          ret+= index*(NT(TBL_%1$s_dline).t[k-2] - ret);";
	  z[lc++]= "        else";
	  z[lc++]= "          ret+= index*(NT(TBL_%1$s_dline).t[len-1] - ret);";
	  z[lc++]= "      }";
	  z[lc++]= "  }";
	}
      
      if (interp == INTERP_SINC)
	{
	  z[lc++]="  if (index == 0.0F)";
	  z[lc++]="    ret = NT(TBL_%1$s_dline).t[i];";
	  z[lc++]="  else";
	  z[lc++]="   {";
	  z[lc++]="    fptr = ((unsigned int)(4294967296.0F*index)) >> (32 - SINC_LOG_PILEN);";
	  z[lc++]="    ret = sinc[fptr]*NT(TBL_%1$s_dline).t[i];";
	  z[lc++]="    rptr = SINC_PILEN - fptr;";
	  z[lc++]="    fptr += SINC_PILEN;";
	  z[lc++]="    k = i;";
	  z[lc++]="    incr = 1;";
	  z[lc++]="    while (rptr < SINC_SIZE)";
	  z[lc++]="     {";
	  z[lc++]="       k = k + incr;";
	  z[lc++]="       if (incr == 1)";
	  z[lc++]="         {";
	  z[lc++]="           if (k == len)";
	  z[lc++]="             k = 0;";
	  z[lc++]="           if (k == NT(TBL_%1$s_dline).tend)";
	  z[lc++]="            {";
	  z[lc++]="              incr = -1;";
	  z[lc++]="              if (k >= 2)";
	  z[lc++]="                k -= 2;";
	  z[lc++]="              else";
	  z[lc++]="                k = ((k == 1) ? (len - 1) : (len - 2));";
	  z[lc++]="            }";
	  z[lc++]="         }";
	  z[lc++]="       else";
	  z[lc++]="         {";
	  z[lc++]="           if (k < 0)";
	  z[lc++]="             k = len - 1;";
	  z[lc++]="           if (k == NT(TBL_%1$s_dline).tend)";
	  z[lc++]="             incr = 1;";
	  z[lc++]="         }";
	  z[lc++]="       ret += sinc[rptr]*NT(TBL_%1$s_dline).t[k];";
	  z[lc++]="       rptr += SINC_PILEN;";
	  z[lc++]="     }";
	  z[lc++]="    k = i;";
	  z[lc++]="    incr = -1;";
	  z[lc++]="    while (fptr < SINC_SIZE)";
	  z[lc++]="     {";
	  z[lc++]="       k = k + incr;";
	  z[lc++]="       if (incr == -1)";
	  z[lc++]="         {";
	  z[lc++]="           if (k < 0)";
	  z[lc++]="             k = len - 1;";
	  z[lc++]="           if (k == NT(TBL_%1$s_dline).tend)";
	  z[lc++]="             incr = 1;";
	  z[lc++]="         }";
	  z[lc++]="       else";
	  z[lc++]="         {";
	  z[lc++]="           if (k == len)";
	  z[lc++]="             k = 0;";
	  z[lc++]="           if (k == NT(TBL_%1$s_dline).tend)";
	  z[lc++]="            {";
	  z[lc++]="              incr = -1;";
	  z[lc++]="              if (k >= 2)";
	  z[lc++]="                k -= 2;";
	  z[lc++]="              else";
	  z[lc++]="                k = ((k == 1) ? (len - 1) : (len - 2));";
	  z[lc++]="            }";
	  z[lc++]="         }";
	  z[lc++]="       ret += sinc[fptr]*NT(TBL_%1$s_dline).t[k];";
	  z[lc++]="       fptr += SINC_PILEN;";
	  z[lc++]="     }";
	  z[lc++]="   }";
	}
    }

  *lcptr = lc;
  return;
}

/*********************************************************/
/*                  code for method 3                    */
/*********************************************************/

void fracdelaym3(tnode * tptr, int * lcptr)

{
  int lc;

  lc = * lcptr;

  if ((tptr->extra == NULL)||(tptr->extra->next == NULL))
    {
      genex(&lc,tptr->optr->down,"P1 and/or p2 unspecificed (method 3)");
    }
  else
    {
      if (isocheck)
	{
	  z[lc++]= " if (NT(TBL_%1$s_dline).t == NULL)";
	  genex(&lc,tptr->optr->down,"Delay line uninitialized (method 3)");
	  z[lc++]= " len = NT(TBL_%1$s_dline).len;";
	  z[lc++]= " i = (int) (va_p1*EV(ARATE));";
	  z[lc++]= " if (i < 0)";
	  z[lc++]= "  i = 0;";
	  z[lc++]= " if (i > len - 1)";
	  z[lc++]= "  i = len - 1;";
	  z[lc++]= " i = (i + NT(TBL_%1$s_dline).tend) % len;";
	  z[lc++]= " NT(TBL_%1$s_dline).t[i] = va_p2;";
	  z[lc++]= " ret = 0.0F;";
	}
      else
	{
	  z[lc++]= " i = NT(TBL_%1$s_dline).tend + (int)(va_p1*EV(ARATE));";
	  z[lc++]= " if (i >= NT(TBL_%1$s_dline).len)";
	  z[lc++]= "   i -= NT(TBL_%1$s_dline).len;";
	  z[lc++]= " NT(TBL_%1$s_dline).t[i] = va_p2;";
	  z[lc++]= " ret = 0.0F;";
	}
    }

  *lcptr = lc;
  return;
}

/*********************************************************/
/*                  code for method 4                    */
/*********************************************************/

void fracdelaym4(tnode * tptr, int * lcptr)

{
  int lc;

  lc = * lcptr;

  if ((tptr->extra == NULL)||(tptr->extra->next == NULL))
    {
      genex(&lc,tptr->optr->down,"P1 and/or p2 unspecified (method 4)");
    }
  else
    {
      if (isocheck)
	{
	  z[lc++]= " if (NT(TBL_%1$s_dline).t == NULL)";
	  genex(&lc,tptr->optr->down,"Delay line uninitialized (method 4)");
	  z[lc++]= " len = NT(TBL_%1$s_dline).len;";
	  z[lc++]= " i = (int) (va_p1*EV(ARATE));";
	  z[lc++]= " if (i < 0)";
	  z[lc++]= "  i = 0;";
	  z[lc++]= " if (i > len - 1)";
	  z[lc++]= "  i = len - 1;";
	  z[lc++]= " i = (i + NT(TBL_%1$s_dline).tend) % len;";
	  z[lc++]= " ret = NT(TBL_%1$s_dline).t[i]+= va_p2;";
	}
      else
	{
	  z[lc++]= " i = NT(TBL_%1$s_dline).tend + (int)(va_p1*EV(ARATE));";
	  z[lc++]= " if (i >= NT(TBL_%1$s_dline).len)";
	  z[lc++]= "   i -= NT(TBL_%1$s_dline).len;";
	  z[lc++]= " ret = NT(TBL_%1$s_dline).t[i]+= va_p2;";
	} 
    }

  *lcptr = lc;
  return;
}

/*********************************************************/
/*                  code for method 5                    */
/*********************************************************/

void fracdelaym5(tnode * tptr, int * lcptr)

{
  int lc;

  lc = * lcptr;

  if (isocheck)
    {
      z[lc++]= "if (NT(TBL_%1$s_dline).t == NULL)";
      genex(&lc,tptr->optr->down,"Delay line uninitialized (method 5)");
    }
  z[lc++]= "i = --NT(TBL_%1$s_dline).tend;";
  z[lc++]= "if (i < 0)";
  z[lc++]= " i = NT(TBL_%1$s_dline).tend = NT(TBL_%1$s_dline).len - 1;";
  z[lc++]= "ret = NT(TBL_%1$s_dline).t[i];";
  z[lc++]= "NT(TBL_%1$s_dline).t[i] = 0.0F;";

  *lcptr = lc;
  return;
}

/*********************************************************/
/*       code for general-method fracdelay               */
/*********************************************************/


void fracdelaygeneral(tnode * tptr)

{
  int lc = 0;

  z[lc++]= "switch ((int)(method+0.5F)) {";
  z[lc++]= "case 1:";
  fracdelaym1(tptr, &lc);
  z[lc++]= "break;";
  z[lc++]= "case 2:";
  fracdelaym2(tptr, &lc);
  z[lc++]= "break;";
  z[lc++]= "case 3:";
  fracdelaym3(tptr, &lc);
  z[lc++]= "break;";
  z[lc++]= "case 4:";
  fracdelaym4(tptr, &lc);
  z[lc++]= "break;";
  z[lc++]= "case 5:";
  fracdelaym5(tptr, &lc);
  z[lc++]= "break;";
  z[lc++]= "default:";
  genex(&lc,tptr->optr->down,"Illegal method specified (not 1,2,3,4,5)");
  z[lc++]= "}";
  printblock(lc);
  return;

}

/*********************************************************/
/*           global routine for fracdelay                */
/*********************************************************/

void fracdelay(tnode * tptr)

{

  int lc = 0;
  int method;
  tnode * aptr;

  aptr = firstopcodearg(tptr);
  if (aptr->vol == VARIABLE)
    fracdelaygeneral(tptr);
  else
    {
      method = make_int(aptr->down);
      switch(method) 
	{
	case 1:
	  fracdelaym1(tptr, &lc);
	  break;
	case 2:
	  fracdelaym2(tptr, &lc);
	  break;
	case 3:
	  fracdelaym3(tptr, &lc);
	  break;
	case 4:
	  fracdelaym4(tptr, &lc);
	  break;
	case 5:
	  fracdelaym5(tptr, &lc);
	  break;
	default:
	  genex(&lc,tptr->optr->down,"Illegal method specified (not 1,2,3,4,5)");
	}
    }
    printblock(lc);

}

/*********************************************************/
/*       code for flange opcode                          */
/*********************************************************/


void flangecode(tnode * tptr)

{
  int lc = 0;

  acycleguard2(tptr, &lc);
  z[lc++]= " {";
  z[lc++]= "  if (NV(%1$s_kcyc))";
  z[lc++]= "  {";
  z[lc++]= "   NV(%1$s_rate) = EV(ATIME)*rate;";
  z[lc++]= "   depth = (depth<0) ? -depth : depth;";
  z[lc++]= "   NV(%1$s_depth) = (depth > 100.0F) ? FLNOFF :";
  z[lc++]= "                    FLNOFF*0.01F*depth;";
  kcycassign2(tptr, &lc);
  z[lc++]= "  }";
  z[lc++]= "  else";
  z[lc++]= "  {";
  z[lc++]= "   i = ((int)(2.0F*FLNOFF*EV(ARATE)))+1;";
  z[lc++]= "   NT(TBL_%1$s_dline).len = i;";
  z[lc++]= "   NT(TBL_%1$s_dline).tend = i;";
  z[lc++]= "   NT(TBL_%1$s_dline).t = (float *) malloc(i*sizeof(float)); ";
  z[lc++]= "   NT(TBL_%1$s_dline).llmem = 1; ";
  z[lc++]= "   while (i > 0) ";
  z[lc++]= "     NT(TBL_%1$s_dline).t[--i] = 0.0F;";
  z[lc++]= "   i = 64;";
  z[lc++]= "   NT(TBL_%1$s_sweep).len = i;";
  z[lc++]= "   NT(TBL_%1$s_sweep).lenf = i;";
  z[lc++]= "   NT(TBL_%1$s_sweep).tend = i;";
  z[lc++]= "   NT(TBL_%1$s_sweep).t = (float *) malloc((++i)*sizeof(float)); ";
  z[lc++]= "   NT(TBL_%1$s_sweep).llmem = 1; ";
  z[lc++]= "   while ((i--) > 0) ";  /* const is 2.0*M_PI/64.0 */
  z[lc++]= "     NT(TBL_%1$s_sweep).t[i] = (float)sin(9.817477e-02F*i);";
  z[lc++]= "  NV(%1$s_rate) = EV(ATIME)*rate;";
  z[lc++]= "  depth = (depth<0) ? -depth : depth;";
  z[lc++]= "  NV(%1$s_depth) = (depth > 100.0F) ? FLNOFF :";
  z[lc++]= "                   FLNOFF*0.01F*depth;";
  kcycassign(tptr, &lc);
  z[lc++]= "  }";
  z[lc++]= " }";

  z[lc++]= " if ((NV(%1$s_p) += NV(%1$s_rate)) > 1.0F)";
  z[lc++]= "  NV(%1$s_p) -= (int)NV(%1$s_p);";
  z[lc++]= " if (NV(%1$s_p) < 0.0F)";
  z[lc++]= "  NV(%1$s_p) += 1 - (int)NV(%1$s_p);";
  z[lc++]= " i = (int) (index = NV(%1$s_p)*NT(TBL_%1$s_sweep).lenf);";
  z[lc++]= " ret = NT(TBL_%1$s_sweep).t[i] + (index - i)*";
  z[lc++]= "       (NT(TBL_%1$s_sweep).t[i+1] - NT(TBL_%1$s_sweep).t[i]);";
  z[lc++]= " i = (int)(index = (FLNOFF+ret*NV(%1$s_depth))*EV(ARATE));";
  z[lc++]= " index -= i;";
  z[lc++]= " len = NT(TBL_%1$s_dline).len;";
  z[lc++]= " if ((i+= NT(TBL_%1$s_dline).tend) >= len)";
  z[lc++]= "   i -= len;";


  if (interp == INTERP_LINEAR)
    {
      z[lc++]= " ret = NT(TBL_%1$s_dline).t[i];";
      z[lc++]= " k = i + 1;";
      z[lc++]= " if ((k < len) && (k != NT(TBL_%1$s_dline).tend))";
      z[lc++]= "  ret+= index*(NT(TBL_%1$s_dline).t[k] - ret);";
      z[lc++]= " else";
      z[lc++]= "  {";
      z[lc++]= "     if (k == len)";
      z[lc++]= "      {";
      z[lc++]= "        if (NT(TBL_%1$s_dline).tend)";
      z[lc++]= "          ret+= index*(NT(TBL_%1$s_dline).t[0] - ret);";
      z[lc++]= "        else";
      z[lc++]= "          ret+= index*(NT(TBL_%1$s_dline).t[len-2] - ret);";
      z[lc++]= "      }";
      z[lc++]= "     else";
      z[lc++]= "      {";
      z[lc++]= "        if (k >= 2)";
      z[lc++]= "          ret+= index*(NT(TBL_%1$s_dline).t[k-2] - ret);";
      z[lc++]= "        else";
      z[lc++]= "          ret+= index*(NT(TBL_%1$s_dline).t[len-1] - ret);";
      z[lc++]= "      }";
      z[lc++]= "  }";
    }

  if (interp == INTERP_SINC)
    {
      z[lc++]="  if (index == 0.0F)";
      z[lc++]="    ret = NT(TBL_%1$s_dline).t[i];";
      z[lc++]="  else";
      z[lc++]="   {";
      z[lc++]="    fptr = ((unsigned int)(4294967296.0F*index)) >> (32 - SINC_LOG_PILEN);";
      z[lc++]="    ret = sinc[fptr]*NT(TBL_%1$s_dline).t[i];";
      z[lc++]="    rptr = SINC_PILEN - fptr;";
      z[lc++]="    fptr += SINC_PILEN;";
      z[lc++]="    k = i;";
      z[lc++]="    incr = 1;";
      z[lc++]="    while (rptr < SINC_SIZE)";
      z[lc++]="     {";
      z[lc++]="       k = k + incr;";
      z[lc++]="       if (incr == 1)";
      z[lc++]="         {";
      z[lc++]="           if (k == len)";
      z[lc++]="             k = 0;";
      z[lc++]="           if (k == NT(TBL_%1$s_dline).tend)";
      z[lc++]="            {";
      z[lc++]="              incr = -1;";
      z[lc++]="              if (k >= 2)";
      z[lc++]="                k -= 2;";
      z[lc++]="              else";
      z[lc++]="                k = ((k == 1) ? (len - 1) : (len - 2));";
      z[lc++]="            }";
      z[lc++]="         }";
      z[lc++]="       else";
      z[lc++]="         {";
      z[lc++]="           if (k < 0)";
      z[lc++]="             k = len - 1;";
      z[lc++]="           if (k == NT(TBL_%1$s_dline).tend)";
      z[lc++]="             incr = 1;";
      z[lc++]="         }";
      z[lc++]="       ret += sinc[rptr]*NT(TBL_%1$s_dline).t[k];";
      z[lc++]="       rptr += SINC_PILEN;";
      z[lc++]="     }";
      z[lc++]="    k = i;";
      z[lc++]="    incr = -1;";
      z[lc++]="    while (fptr < SINC_SIZE)";
      z[lc++]="     {";
      z[lc++]="       k = k + incr;";
      z[lc++]="       if (incr == -1)";
      z[lc++]="         {";
      z[lc++]="           if (k < 0)";
      z[lc++]="             k = len - 1;";
      z[lc++]="           if (k == NT(TBL_%1$s_dline).tend)";
      z[lc++]="             incr = 1;";
      z[lc++]="         }";
      z[lc++]="       else";
      z[lc++]="         {";
      z[lc++]="           if (k == len)";
      z[lc++]="             k = 0;";
      z[lc++]="           if (k == NT(TBL_%1$s_dline).tend)";
      z[lc++]="            {";
      z[lc++]="              incr = -1;";
      z[lc++]="              if (k >= 2)";
      z[lc++]="                k -= 2;";
      z[lc++]="              else";
      z[lc++]="                k = ((k == 1) ? (len - 1) : (len - 2));";
      z[lc++]="            }";
      z[lc++]="         }";
      z[lc++]="       ret += sinc[fptr]*NT(TBL_%1$s_dline).t[k];";
      z[lc++]="       fptr += SINC_PILEN;";
      z[lc++]="     }";
      z[lc++]="   }";
    }

  z[lc++]= " ret = (ret+x)*0.7F;";
  z[lc++]= " i = --NT(TBL_%1$s_dline).tend;";
  z[lc++]= " if (i < 0)";
  z[lc++]= "   i = NT(TBL_%1$s_dline).tend = NT(TBL_%1$s_dline).len - 1;";
  z[lc++]= " NT(TBL_%1$s_dline).t[i] = x;";
  printblock(lc);

}


/*********************************************************/
/*       code for chorus opcode                          */
/*********************************************************/


void choruscode(tnode * tptr)

{
  int lc = 0;

  acycleguard2(tptr, &lc);
  z[lc++]= " {";
  z[lc++]= "  if (NV(%1$s_kcyc))";
  z[lc++]= "  {";
  z[lc++]= "   NV(%1$s_rate) = EV(ATIME)*rate;";
  z[lc++]= "   depth = (depth<0) ? -depth : depth;";
  z[lc++]= "   NV(%1$s_depth) = (depth > 100.0F) ? CHROFF :";
  z[lc++]= "                    CHROFF*0.01F*depth;";
  kcycassign2(tptr, &lc);
  z[lc++]= "  }";
  z[lc++]= "  else";
  z[lc++]= "  {";
  z[lc++]= "   i = ((int)(2.0F*CHROFF*EV(ARATE)))+1;";
  z[lc++]= "   NT(TBL_%1$s_dline).len = i;";
  z[lc++]= "   NT(TBL_%1$s_dline).tend = i;";
  z[lc++]= "   NT(TBL_%1$s_dline).t = (float *) malloc(i*sizeof(float)); ";
  z[lc++]= "   NT(TBL_%1$s_dline).llmem = 1; ";
  z[lc++]= "   while (i > 0) ";
  z[lc++]= "     NT(TBL_%1$s_dline).t[--i] = 0.0F;";
  z[lc++]= "   i = 64;";
  z[lc++]= "   NT(TBL_%1$s_sweep).len = i;";
  z[lc++]= "   NT(TBL_%1$s_sweep).lenf = i;";
  z[lc++]= "   NT(TBL_%1$s_sweep).tend = i;";
  z[lc++]= "   NT(TBL_%1$s_sweep).t = (float *) malloc((++i)*sizeof(float)); ";
  z[lc++]= "   NT(TBL_%1$s_sweep).llmem = 1; ";
  z[lc++]= "   while ((i--) > 0) ";  /* const is 2.0*M_PI/64.0 */
  z[lc++]= "     NT(TBL_%1$s_sweep).t[i] = (float)sin(9.817477e-02F*i);";
  z[lc++]= "  NV(%1$s_rate) = EV(ATIME)*rate;";
  z[lc++]= "  depth = (depth<0) ? -depth : depth;";
  z[lc++]= "  NV(%1$s_depth) = (depth > 100.0F) ? CHROFF :";
  z[lc++]= "                   CHROFF*0.01F*depth;";
  kcycassign(tptr, &lc);
  z[lc++]= "  }";
  z[lc++]= " }";

  z[lc++]= " if ((NV(%1$s_p) += NV(%1$s_rate)) > 1.0F)";
  z[lc++]= "  NV(%1$s_p) -= (int)NV(%1$s_p);";
  z[lc++]= " if (NV(%1$s_p) < 0.0F)";
  z[lc++]= "  NV(%1$s_p) += 1 - (int)NV(%1$s_p);";
  z[lc++]= " i = (int) (index = NV(%1$s_p)*NT(TBL_%1$s_sweep).lenf);";
  z[lc++]= " ret = NT(TBL_%1$s_sweep).t[i] + (index - i)*";
  z[lc++]= "       (NT(TBL_%1$s_sweep).t[i+1] - NT(TBL_%1$s_sweep).t[i]);";
  z[lc++]= " i = (int)(index = (CHROFF + ret*NV(%1$s_depth))*EV(ARATE));";
  z[lc++]= " index -= i;";
  z[lc++]= " len = NT(TBL_%1$s_dline).len;";
  z[lc++]= " if ((i+= NT(TBL_%1$s_dline).tend) >= len)";
  z[lc++]= "   i -= len;";

  if (interp == INTERP_LINEAR)
    {
      z[lc++]= " ret = NT(TBL_%1$s_dline).t[i];";
      z[lc++]= " k = i + 1;";
      z[lc++]= " if ((k < len) && (k != NT(TBL_%1$s_dline).tend))";
      z[lc++]= "  ret+= index*(NT(TBL_%1$s_dline).t[k] - ret);";
      z[lc++]= " else";
      z[lc++]= "  {";
      z[lc++]= "     if (k == len)";
      z[lc++]= "      {";
      z[lc++]= "        if (NT(TBL_%1$s_dline).tend)";
      z[lc++]= "          ret+= index*(NT(TBL_%1$s_dline).t[0] - ret);";
      z[lc++]= "        else";
      z[lc++]= "          ret+= index*(NT(TBL_%1$s_dline).t[len-2] - ret);";
      z[lc++]= "      }";
      z[lc++]= "     else";
      z[lc++]= "      {";
      z[lc++]= "        if (k >= 2)";
      z[lc++]= "          ret+= index*(NT(TBL_%1$s_dline).t[k-2] - ret);";
      z[lc++]= "        else";
      z[lc++]= "          ret+= index*(NT(TBL_%1$s_dline).t[len-1] - ret);";
      z[lc++]= "      }";
      z[lc++]= "  }";
    }

  if (interp == INTERP_SINC)
    {
      z[lc++]="  if (index == 0.0F)";
      z[lc++]="    ret = NT(TBL_%1$s_dline).t[i];";
      z[lc++]="  else";
      z[lc++]="   {";
      z[lc++]="    fptr = ((unsigned int)(4294967296.0F*index)) >> (32 - SINC_LOG_PILEN);";
      z[lc++]="    ret = sinc[fptr]*NT(TBL_%1$s_dline).t[i];";
      z[lc++]="    rptr = SINC_PILEN - fptr;";
      z[lc++]="    fptr += SINC_PILEN;";
      z[lc++]="    k = i;";
      z[lc++]="    incr = 1;";
      z[lc++]="    while (rptr < SINC_SIZE)";
      z[lc++]="     {";
      z[lc++]="       k = k + incr;";
      z[lc++]="       if (incr == 1)";
      z[lc++]="         {";
      z[lc++]="           if (k == len)";
      z[lc++]="             k = 0;";
      z[lc++]="           if (k == NT(TBL_%1$s_dline).tend)";
      z[lc++]="            {";
      z[lc++]="              incr = -1;";
      z[lc++]="              if (k >= 2)";
      z[lc++]="                k -= 2;";
      z[lc++]="              else";
      z[lc++]="                k = ((k == 1) ? (len - 1) : (len - 2));";
      z[lc++]="            }";
      z[lc++]="         }";
      z[lc++]="       else";
      z[lc++]="         {";
      z[lc++]="           if (k < 0)";
      z[lc++]="             k = len - 1;";
      z[lc++]="           if (k == NT(TBL_%1$s_dline).tend)";
      z[lc++]="             incr = 1;";
      z[lc++]="         }";
      z[lc++]="       ret += sinc[rptr]*NT(TBL_%1$s_dline).t[k];";
      z[lc++]="       rptr += SINC_PILEN;";
      z[lc++]="     }";
      z[lc++]="    k = i;";
      z[lc++]="    incr = -1;";
      z[lc++]="    while (fptr < SINC_SIZE)";
      z[lc++]="     {";
      z[lc++]="       k = k + incr;";
      z[lc++]="       if (incr == -1)";
      z[lc++]="         {";
      z[lc++]="           if (k < 0)";
      z[lc++]="             k = len - 1;";
      z[lc++]="           if (k == NT(TBL_%1$s_dline).tend)";
      z[lc++]="             incr = 1;";
      z[lc++]="         }";
      z[lc++]="       else";
      z[lc++]="         {";
      z[lc++]="           if (k == len)";
      z[lc++]="             k = 0;";
      z[lc++]="           if (k == NT(TBL_%1$s_dline).tend)";
      z[lc++]="            {";
      z[lc++]="              incr = -1;";
      z[lc++]="              if (k >= 2)";
      z[lc++]="                k -= 2;";
      z[lc++]="              else";
      z[lc++]="                k = ((k == 1) ? (len - 1) : (len - 2));";
      z[lc++]="            }";
      z[lc++]="         }";
      z[lc++]="       ret += sinc[fptr]*NT(TBL_%1$s_dline).t[k];";
      z[lc++]="       fptr += SINC_PILEN;";
      z[lc++]="     }";
      z[lc++]="   }";
    }

  z[lc++]= " ret = (ret+x)*0.7F;";
  z[lc++]= " i = --NT(TBL_%1$s_dline).tend;";
  z[lc++]= " if (i < 0)";
  z[lc++]= "   i = NT(TBL_%1$s_dline).tend = NT(TBL_%1$s_dline).len - 1;";
  z[lc++]= " NT(TBL_%1$s_dline).t[i] = x;";
  printblock(lc);

}


/*********************************************************/
/* initializes comb set for reverb opcode                */
/*********************************************************/

void revcombinit(int * lcptr, int num, tnode * tptr)

{
  char tname[STRSIZE];
  char gname[STRSIZE];
  char rname[STRSIZE];
  char fname[STRSIZE];
  int i ;
  int lc;
  float dtime[4] = {0.030F,0.0343F,0.0393F,0.045F};

  lc = * lcptr;
  if (num <=0)
    {
      sprintf(tname,"TBL_%s_dline0_",currinstancename);
      sprintf(gname,"%s_g0_",currinstancename);
      if (num < 0)
	sprintf(rname,"f0");
      else
	{
	  sprintf(rname,"va_r0");
	  sprintf(fname,"f0");
	}
    }
  else
    {
      sprintf(tname,"TBL_%s_dline%i_",currinstancename,num);
      sprintf(gname,"%s_g%i_",currinstancename,num);
      sprintf(rname,"va_r%i",num);
      sprintf(fname,"va_f%i",num);
    }
  for (i=0;i<4;i++)
    {
      mz(lc); sprintf(z[lc++], "i = NT(%s%i).len = ROUND(%f*EV(ARATE));",
		      tname,i,dtime[i]);
      mz(lc); sprintf(z[lc++], 
		      "NT(%s%i).t = (float *) calloc(i,sizeof(float));",tname,i);
      mz(lc); sprintf(z[lc++], 
		      "NT(%s%i).llmem = 1;",tname,i);
      mz(lc); sprintf(z[lc++], "if (%s <= 0)", rname); /* const was -3*log(10) */
      genex(&lc,tptr->optr->down,"Negative reverberation time");
      mz(lc); sprintf(z[lc++], "NV(%s%i) = (float)exp(%fF/%s);\n",
		      gname,i, -6.907755F*dtime[i],rname);

    }
  if (num >= 0)
    {
      mz(lc); sprintf(z[lc++], "e = 3.141593F*%s*EV(ATIME);",fname);
      z[lc++]= "  c = 0.0031416F;";
      z[lc++]= "  if (e < 3.27249e-05F)";
      z[lc++]= "    c = 30557.8F;";
      z[lc++]= "  else";
      z[lc++]= "   if (e < 1.56765F)";
      z[lc++]= "     c = 1.0F/(float)tan((double)e);";
      mz(lc); sprintf(z[lc++], "  NV(%s_b0_%i)= 1.0F/(1.0F + 1.414214F*c + c*c);",
		      currinstancename,num);
      mz(lc); sprintf(z[lc++], "  NV(%s_b1_%i)= 2.0F*NV(%s_b0_%i);",
		      currinstancename,num,currinstancename,num);
      mz(lc); sprintf(z[lc++],"  NV(%s_b2_%i)= NV(%s_b0_%i);",
		      currinstancename,num,currinstancename,num);

      mz(lc); sprintf(z[lc++],"  NV(%s_a1_%i)= 2.0F*NV(%s_b0_%i)*(1.0F - c*c);",
		      currinstancename,num,currinstancename,num);
      mz(lc); sprintf(z[lc++],
		      "  NV(%s_a2_%i)= NV(%s_b0_%i)*(1.0F - 1.414214F*c + c*c);\n",
		      currinstancename,num,currinstancename,num);

    }
  *lcptr = lc;
  return;
}

/******************************************************/
/* executes comb set for reverb opcode                */
/******************************************************/

void revcombcode(int * lcptr, int num)

{
  char tname[STRSIZE];
  char gname[STRSIZE];
  int i ;
  int lc;

  lc = * lcptr;
  if (num <=0)
    {
      sprintf(tname,"TBL_%s_dline0_",currinstancename);
      sprintf(gname,"%s_g0_",currinstancename);
    }
  else
    {
      sprintf(tname,"TBL_%s_dline%i_",currinstancename,num);
      sprintf(gname,"%s_g%i_",currinstancename,num);
    }
  z[lc++]= "csum = 0.0F;";
  for (i=0;i<4;i++)
    {
      mz(lc); sprintf(z[lc++],"i = NT(%s%i).tend;", tname,i);
      mz(lc); sprintf(z[lc++], "csum += NT(%s%i).t[i];", tname,i);
      mz(lc); sprintf(z[lc++], 
		      "NT(%s%i).t[i] = NT(%s%i).t[i]*NV(%s%i)+apout2;",
		      tname,i,tname,i,gname,i);
      mz(lc); sprintf(z[lc++], "NT(%s%i).tend = ++i;", tname,i);
      mz(lc); sprintf(z[lc++], "if (i==NT(%s%i).len)", tname,i);
      mz(lc); sprintf(z[lc++], "NT(%s%i).tend=0;\n", tname,i);

    }
  z[lc++]= "csum *= 0.25F;";
  if (num >= 0)
    {
      mz(lc); sprintf(z[lc++], "fout = NV(%s_b0_%i)*csum + NV(%s_d2_%i);",
		      currinstancename,num,currinstancename,num);    
      mz(lc); sprintf(z[lc++], 
    "NV(%s_d2_%i)=NV(%s_d1_%i)-NV(%s_a1_%i)*fout+NV(%s_b1_%i)*csum;",
		      currinstancename,num,currinstancename,num,
		      currinstancename,num,currinstancename,num);    
      mz(lc); sprintf(z[lc++], 
       "NV(%s_d1_%i) = -NV(%s_a2_%i)*fout+NV(%s_b2_%i)*csum;",
		      currinstancename,num,currinstancename,num,
		      currinstancename,num);
      z[lc++]= "ret += fout;";
    }
  else
    z[lc++]= "ret += csum;";

  *lcptr = lc;
  return;
}

/*********************************************************/
/* code for reverb opcode                                  */
/*********************************************************/

void reverbcode(tnode * tcall)

{
  int lc = 0;
  int i;
  tnode * tptr = tcall->extra;

  z[lc++]= "if (NT(TBL_%1$s_ap1).t == NULL)"; /* initialize all tables */
  z[lc++]= "{";

  z[lc++]= " i = NT(TBL_%1$s_ap1).len = ROUND(0.005F*EV(ARATE));";
  z[lc++]= " if (i<=0)";
  genex(&lc,tcall->optr->down,"Library error -- allpass 1");
  z[lc++]= " NT(TBL_%1$s_ap1).t = (float *) calloc(i,sizeof(float)); ";
  z[lc++]= " NT(TBL_%1$s_ap1).llmem = 1; ";

  z[lc++]= " i = NT(TBL_%1$s_ap2).len = ROUND(0.0017F*EV(ARATE));";
  z[lc++]= " if (i<=0)";
  genex(&lc,tcall->optr->down,"Library error -- allpass 2");
  z[lc++]= " NT(TBL_%1$s_ap2).t = (float *) calloc(i,sizeof(float)); ";
  z[lc++]= " NT(TBL_%1$s_ap2).llmem = 1; ";

  if (tptr == NULL)
    revcombinit(&lc, -1, tcall);
  else
    {
      i = 0;
      while (tptr != NULL)
	{
	  revcombinit(&lc, i, tcall);
	  tptr = tptr->next;
	  if (tptr != NULL)
	    tptr = tptr->next;
	  if (tptr != NULL)
	    tptr = tptr->next;
	  if (tptr != NULL)
	    tptr = tptr->next;
	  i++;
	}
    }

  z[lc++]= "}";

  z[lc++]= "i = NT(TBL_%1$s_ap1).tend;";
  z[lc++]= "apout1 = NT(TBL_%1$s_ap1).t[i] - x*0.7F;";
  z[lc++]= "NT(TBL_%1$s_ap1).t[i] = apout1*0.7F + x;";
  z[lc++]= "NT(TBL_%1$s_ap1).tend= ++i;";
  z[lc++]= "if (i==NT(TBL_%1$s_ap1).len)";
  z[lc++]= " NT(TBL_%1$s_ap1).tend=0;";


  z[lc++]= "i = NT(TBL_%1$s_ap2).tend;";
  z[lc++]= "apout2 = NT(TBL_%1$s_ap2).t[i] - apout1*0.7F;";
  z[lc++]= "NT(TBL_%1$s_ap2).t[i] = apout2*0.7F + apout1;";
  z[lc++]= "NT(TBL_%1$s_ap2).tend= ++i;";
  z[lc++]= "if (i==NT(TBL_%1$s_ap2).len)";
  z[lc++]= " NT(TBL_%1$s_ap2).tend=0;\n";

  z[lc++]= "ret = 0.0F;";

  tptr = tcall->extra;
  if (tptr == NULL)
    revcombcode(&lc, -1);
  else
    {
      i = 0;
      while (tptr != NULL)
	{
	  revcombcode(&lc, i);
	  tptr = tptr->next;
	  if (tptr != NULL)
	    tptr = tptr->next;
	  if (tptr != NULL)
	    tptr = tptr->next;
	  if (tptr != NULL)
	    tptr = tptr->next;
	  i++;
	}
    }
  printblock(lc);
}


/*********************************************************/
/* code for spatialize command                           */
/*********************************************************/

void spatialcode(tnode * tcall)

{
  int lc = 0;
  int i;
  char * side;

  float dtime[8] =  {3.0F*(1.0F/44100.0F),
		     9.0F*(1.0F/44100.0F),
		     12.0F*(1.0F/44100.0F),
		     16.0F*(1.0F/44100.0F),
                     18.0F*(1.0F/44100.0F), 
		     (HEADSIZE/SPEEDSOUND)*(1.0F + 1.570796F),
		     (HEADSIZE/SPEEDSOUND)*(1.0F + 1.570796F),
		     0.02F};
  float aval[5] =   {1.0F*(1.0F/44100.0F),
		     5.0F*(1.0F/44100.0F),
		     5.0F*(1.0F/44100.0F),
		     5.0F*(1.0F/44100.0F),
		     5.0F*(1.0F/44100.0F)};
  float bval[5] =   {2.0F*(1.0F/44100.0F),
		     4.0F*(1.0F/44100.0F),
		     7.0F*(1.0F/44100.0F),
		     11.0F*(1.0F/44100.0F),
		     13.0F*(1.0F/44100.0F)};
  float dval[5] = {DZERO, DZERO - 0.5F, DZERO - 0.5F, 
		   DZERO - 0.5F, DZERO - 0.5F};
  float pval[5] = {0.5F, -1.0F, 0.5F, -0.25F, 0.25F};

  z[lc++]="if (NVI(%1$s_kcyc) != EV(kcycleidx))";
  z[lc++]= "{";
  z[lc++]= " if (!NVI(%1$s_kcyc))";
  z[lc++]= " {";
  for (i = 0; i < 8; i++)
    {
      mz(lc); sprintf(z[lc++], "   i = NT(TBL_%s_d%i).len = 1+(ROUND(%1.3eF*EV(ARATE)));",
		      currinstancename,i,dtime[i]);
      mz(lc); sprintf(z[lc++], 
		      "   NT(TBL_%s_d%i).t = (float *) calloc(i,sizeof(float));",
		      currinstancename,i);
      mz(lc); sprintf(z[lc++], 
		      "   NT(TBL_%s_d%i).llmem = 1;",
		      currinstancename,i);
    }
  z[lc++]= " }";
  z[lc++]= " if ((!NVI(%1$s_kcyc))||(NV(%1$s_odis)!=distance))";
  z[lc++]= "  {";
  z[lc++]= "   NV(%1$s_odis) = distance;";
  z[lc++]= "   lpe = 2.199115e+5F*EV(ATIME)*(float)pow((distance>0.3F)?distance:0.3F,-0.666667F);";
  z[lc++]= "   lpc = 0.0031416F;";
  z[lc++]= "   if (lpe < 3.27249e-05F)";
  z[lc++]= "     lpc = 30557.8F;";
  z[lc++]= "   else";
  z[lc++]= "    if (lpe < 1.56765F)";
  z[lc++]= "      lpc = 1.0F/(float)tan(lpe);";
  z[lc++]= "   NV(%1$s_dis_b0)= 1.0F/(1.0F + 1.414214F*lpc + lpc*lpc);";
  z[lc++]= "   NV(%1$s_dis_b1)= 2.0F*NV(%1$s_dis_b0);";
  z[lc++]= "   NV(%1$s_dis_b2)= NV(%1$s_dis_b0);";
  z[lc++]= "   NV(%1$s_dis_a1)= 2.0F*NV(%1$s_dis_b0)*(1.0F - lpc*lpc);";
  z[lc++]= "   NV(%1$s_dis_a2)= NV(%1$s_dis_b0)*(1.0F - 1.414214F*lpc + lpc*lpc);";
  z[lc++]= "  }";
  z[lc++]= " if ((!NVI(%1$s_kcyc))||(NV(%1$s_oaz)!=azimuth)";
  z[lc++]= "                   || (NV(%1$s_oel)!=elevation))";
  z[lc++]= "  {";
  z[lc++]= "   NV(%1$s_oel) = elevation;";
  z[lc++]= "   NV(%1$s_oaz) = azimuth;";
  z[lc++]= "   theta = (float)asin(cos(elevation)*sin(azimuth));";
  z[lc++]= "   phi = (float)atan2(sin(elevation),cos(elevation)*cos(azimuth));";
  for (i = 0; i < 5; i++)
    {
      mz(lc); sprintf(z[lc++], 
     "   NV(%s_t%i) = EV(ARATE)*(%1.3eF*(float)cos(theta/2.0F)*(float)sin(%1.3eF*(1.570796F-phi))+%1.3eF);", 
		      currinstancename,i,aval[i],dval[i],bval[i]);
      mz(lc); sprintf(z[lc++], "   NVI(%s_i%i) = (int)NV(%s_t%i);",
		      currinstancename,i,currinstancename,i);
      mz(lc); sprintf(z[lc++], "   NV(%s_t%i) -= NVI(%s_i%i);",
		      currinstancename,i,currinstancename,i);

    }
  if (outputbus->width == 2)
    {
      z[lc++]= "   pL = 1.0F - (float)sin(theta);";
      z[lc++]= "   pR = 1.0F + (float)sin(theta);";
      if (reentrant)
	{
	  mz(lc); sprintf(z[lc++], 
			  "   NV(%s_az_b0L) = (%1.3eF + pL*(EV(ARATE) + EV(ARATE)))/"
			  "(%1.3eF + EV(ARATE) + EV(ARATE));", currinstancename,
			  (2*SPEEDSOUND/HEADSIZE), (2*SPEEDSOUND/HEADSIZE));
	  mz(lc); sprintf(z[lc++], 
			  "   NV(%s_az_b0R) = (%1.3eF + pR*(EV(ARATE) + EV(ARATE)))/"
			  "(%1.3eF + EV(ARATE) + EV(ARATE));", currinstancename,
			  (2*SPEEDSOUND/HEADSIZE), (2*SPEEDSOUND/HEADSIZE));
	  mz(lc); sprintf(z[lc++], 
			  "   NV(%s_az_b1L) = (%1.3eF - pL*(EV(ARATE) + EV(ARATE)))/"
			  "(%1.3eF + EV(ARATE) + EV(ARATE));", currinstancename,
			  (2*SPEEDSOUND/HEADSIZE), (2*SPEEDSOUND/HEADSIZE));
	  mz(lc); sprintf(z[lc++], 
			  "   NV(%s_az_b1R) = (%1.3eF - pR*(EV(ARATE) + EV(ARATE)))/"
			  "(%1.3eF + EV(ARATE) + EV(ARATE));", currinstancename,
			  (2*SPEEDSOUND/HEADSIZE), (2*SPEEDSOUND/HEADSIZE));
	  mz(lc); sprintf(z[lc++], 
			  "   NV(%s_az_a1) = (%1.3eF -(EV(ARATE) + EV(ARATE)))/"
			  "(%1.3eF + EV(ARATE) + EV(ARATE));", currinstancename,
			  (2*SPEEDSOUND/HEADSIZE), (2*SPEEDSOUND/HEADSIZE));
	}
      else
	{
	  mz(lc); sprintf(z[lc++], 
			  "   NV(%s_az_b0L) = %1.3eF*(%1.3eF*pL + %1.3eF);", 
			  currinstancename,1/((2*SPEEDSOUND/HEADSIZE)+(2.0*srate)),
			  (2.0*srate),(2*SPEEDSOUND/HEADSIZE));
	  mz(lc); sprintf(z[lc++], 
			  "   NV(%s_az_b0R) = %1.3eF*(%1.3eF*pR + %1.3eF);", 
			  currinstancename,1/((2*SPEEDSOUND/HEADSIZE)+(2.0*srate)),
			  (2.0*srate),(2*SPEEDSOUND/HEADSIZE));
	  mz(lc); sprintf(z[lc++], 
			  "   NV(%s_az_b1L) = %1.3eF*(- %1.3eF*pL + %1.3eF);", 
			  currinstancename,1/((2*SPEEDSOUND/HEADSIZE)+(2.0*srate)),
			  (2.0*srate),(2*SPEEDSOUND/HEADSIZE));
	  mz(lc); sprintf(z[lc++], 
			  "   NV(%s_az_b1R) = %1.3eF*(- %1.3eF*pR + %1.3eF);", 
			  currinstancename,1/((2*SPEEDSOUND/HEADSIZE)+(2.0*srate)),
			  (2.0*srate),(2*SPEEDSOUND/HEADSIZE));
	  mz(lc); sprintf(z[lc++], 
			  "   NV(%s_az_a1) = %1.3eF;", 
			  currinstancename,((2*SPEEDSOUND/HEADSIZE)-(2.0*srate))
			  /((2*SPEEDSOUND/HEADSIZE)+(2.0*srate)));
	}
      z[lc++]= "   if (theta > 0.0F)";
      z[lc++]= "   {";
      mz(lc); sprintf(z[lc++], 
		      "   NV(%s_t5) = EV(ARATE)*%1.3eF*(1.0F + theta);",
		       currinstancename,HEADSIZE/SPEEDSOUND);
      mz(lc); sprintf(z[lc++], "  NVI(%s_i5) = (int)NV(%s_t5);",
		      currinstancename,currinstancename);
      mz(lc); sprintf(z[lc++], "  NV(%s_t5) -= NVI(%s_i5);",
		      currinstancename,currinstancename);
      mz(lc); sprintf(z[lc++], 
		      "NV(%s_t6) = EV(ARATE)*%1.3eF*(1.0F - (float)sin(theta));",
		       currinstancename,HEADSIZE/SPEEDSOUND);
      mz(lc); sprintf(z[lc++], "  NVI(%s_i6) = (int)NV(%s_t6);",
		      currinstancename,currinstancename);
      mz(lc); sprintf(z[lc++], "  NV(%s_t6) -= NVI(%s_i6);",
		      currinstancename,currinstancename);
      z[lc++]= "   }";
      z[lc++]= "   else";
      z[lc++]= "   {";
      mz(lc); sprintf(z[lc++], 
		      "   NV(%s_t5) = EV(ARATE)*%1.3eF*(1.0F - theta);",
		      currinstancename,HEADSIZE/SPEEDSOUND);
      mz(lc); sprintf(z[lc++], "  NVI(%s_i5) = (int)NV(%s_t5);",
		      currinstancename,currinstancename);
      mz(lc); sprintf(z[lc++], "  NV(%s_t5) -= NVI(%s_i5);",
		      currinstancename,currinstancename);
      mz(lc); sprintf(z[lc++], 
		      "NV(%s_t6) = EV(ARATE)*%1.3eF*(1.0F + (float)sin(theta));",
		      currinstancename,HEADSIZE/SPEEDSOUND);
      mz(lc); sprintf(z[lc++], "  NVI(%s_i6) = (int)NV(%s_t6);",
		      currinstancename,currinstancename);
      mz(lc); sprintf(z[lc++], "  NV(%s_t6) -= NVI(%s_i6);",
		      currinstancename,currinstancename);
      z[lc++]= "  }";
    }
  z[lc++]= "   }";
  z[lc++]="   NVI(%1$s_kcyc) = EV(kcycleidx);";
  z[lc++]= " }";

  z[lc++]= "in = NV(%1$s_dis_b0)*x + NV(%1$s_dis_d2);";
  z[lc++]= "NV(%1$s_dis_d2)=NV(%1$s_dis_d1)-NV(%1$s_dis_a1)*in+NV(%1$s_dis_b1)*x;";
  z[lc++]= "NV(%1$s_dis_d1) =            -NV(%1$s_dis_a2)*in+NV(%1$s_dis_b2)*x;";

  z[lc++]= "i = NT(TBL_%1$s_d7).tend;";
  mz(lc); sprintf(z[lc++], 
       "room = %1.3eF*NT(TBL_%s_d7).t[i];", ROOMGAIN,currinstancename);
  z[lc++]= "NT(TBL_%1$s_d7).t[i] = in;\n";
  z[lc++]= "if ((++NT(TBL_%1$s_d7).tend) == NT(TBL_%1$s_d7).len)";
  z[lc++]= " NT(TBL_%1$s_d7).tend = 0;";
  
  z[lc++]= "ein = in;\n";
  for (i = 0; i < 5; i++)
    {
      mz(lc); sprintf(z[lc++], 
      "len = NT(TBL_%s_d%i).len;", currinstancename,i);
      mz(lc); sprintf(z[lc++], 
	    "if ((i = NVI(%s_i%i) + NT(TBL_%s_d%i).tend) >=len) ",
		      currinstancename,i,currinstancename,i);
      z[lc++]= " i -= len;";
      mz(lc); sprintf(z[lc++], 
		      "ret = NT(TBL_%s_d%i).t[i];",currinstancename,i);
 
      z[lc++]= " if ((i+1) < len)";
      mz(lc);
      sprintf(z[lc++], "   ret+= NV(%s_t%i)*(NT(TBL_%s_d%i).t[i+1] - ret);",
	      currinstancename,i,currinstancename,i);
      z[lc++]= " else";
      mz(lc); 
      sprintf(z[lc++],"   ret+= NV(%s_t%i)*(NT(TBL_%s_d%i).t[0] - ret);",
	      currinstancename,i,currinstancename,i);

      mz(lc); sprintf(z[lc++], 
	      "ein += %1.3eF*ret;",pval[i]);
      mz(lc); sprintf(z[lc++], 
              "if ((i = --NT(TBL_%s_d%i).tend) < 0)",currinstancename,i);
      mz(lc); sprintf(z[lc++], 
      " i = NT(TBL_%s_d%i).tend = len - 1;", currinstancename,i);
      mz(lc); sprintf(z[lc++], 
       "NT(TBL_%s_d%i).t[i] = in;\n",currinstancename,i);
    }
  
  if (outputbus->width != 2)
    {
      z[lc++]="for (i=BUS_output_bus; i<ENDBUS_output_bus;i++)";
      z[lc++]="{";
      z[lc++]=" TB(i) += ein + room;";
      z[lc++]="}";
      printblock(lc);
      return;
    }
  z[lc++]= "aleft = NV(%1$s_az_b0L)*ein + NV(%1$s_az_d1L);";
  z[lc++]= "NV(%1$s_az_d1L)= -NV(%1$s_az_a1)*aleft+NV(%1$s_az_b1L)*ein;";
  z[lc++]= "aright = NV(%1$s_az_b0R)*ein + NV(%1$s_az_d1R);";
  z[lc++]= "NV(%1$s_az_d1R)= -NV(%1$s_az_a1)*aright+NV(%1$s_az_b1R)*ein;";
  for (i = 5; i < 7; i++)
    {
      if (i == 5)
	side = "left";
      else
	side = "right";
      mz(lc); sprintf(z[lc++], 
      "len = NT(TBL_%s_d%i).len;", currinstancename,i);

      mz(lc); sprintf(z[lc++], 
		      "if ((i = NVI(%s_i%i) + NT(TBL_%s_d%i).tend) >=len) ",
		      currinstancename,i,currinstancename,i);
      z[lc++]= " i -= len;";
      
      mz(lc); sprintf(z[lc++], 
		      "ret = NT(TBL_%s_d%i).t[i];",currinstancename,i);
      z[lc++]= " if ((i+1) < len)";
      mz(lc);
      sprintf(z[lc++], "   ret+= NV(%s_t%i)*(NT(TBL_%s_d%i).t[i+1] - ret);",
	      currinstancename,i,currinstancename,i);
      z[lc++]= " else";
      mz(lc); 
      sprintf(z[lc++],"   ret+= NV(%s_t%i)*(NT(TBL_%s_d%i).t[0] - ret);",
	      currinstancename,i,currinstancename,i);
      
      mz(lc); sprintf(z[lc++],"%s = ret;",side);
      mz(lc); sprintf(z[lc++], 
              "if ((i = --NT(TBL_%s_d%i).tend) < 0)",currinstancename,i);
      mz(lc); sprintf(z[lc++], 
      " i = NT(TBL_%s_d%i).tend = len - 1;", currinstancename,i);
      mz(lc); sprintf(z[lc++], 
       "NT(TBL_%s_d%i).t[i] = a%s;\n",currinstancename,i,side);
    }
  z[lc++]=" TB(0) += left + room;";
  z[lc++]=" TB(1) += right + room;";
  
  printblock(lc);
  return;

}


/*********************************************************/
/* code for phasor opcode                                  */
/*********************************************************/

void aphasorcode(tnode * tptr)

{
  int lc = 0;

  z[lc++]="if (NVI(%1$s_kcyc))";   /* fast path */
  z[lc++]="{";
  z[lc++]="  if (cps >= 0)";  /* positive frequency */
  z[lc++]="   {";
  z[lc++]="    nint = (unsigned int)(index = cps*((float)(4294967296.0/EV(ARATE))));";
  z[lc++]="    while (index >= 4294967296.0F)";
  z[lc++]="      nint = (unsigned int)(index -= 4294967296.0F);";
  z[lc++]="    nfrac = (unsigned int)(4294967296.0F*(index - nint));";
  z[lc++]="    NVUI(%1$s_pfrac) = (j = NVUI(%1$s_pfrac)) + nfrac;";
  z[lc++]="    NVUI(%1$s_pint) += nint + (NVUI(%1$s_pfrac) < j);";
  z[lc++]="   }";
  z[lc++]="  else";   /* negative frequency */
  z[lc++]="   {";
  z[lc++]="    nint = (unsigned int)(index = -cps*((float)(4294967296.0/EV(ARATE))));";
  z[lc++]="    while (index >= 4294967296.0F)";
  z[lc++]="      nint = (unsigned int)(index -= 4294967296.0F);";
  z[lc++]="    nfrac = (unsigned int)(4294967296.0F*(index - nint));";
  z[lc++]="    NVUI(%1$s_pfrac) = (j = NVUI(%1$s_pfrac)) - nfrac;";
  z[lc++]="    NVUI(%1$s_pint) -= nint + (NVUI(%1$s_pfrac) > j);";
  z[lc++]="   }";
  z[lc++]="}";
  z[lc++]="else";             /* for first time through */ 
  z[lc++]="  NVI(%1$s_kcyc) = 1;";
  z[lc++]="ret = ((float)(1.0/4294967296.0))*NVUI(%1$s_pint) +";
  z[lc++]="      ((float)((1.0/4294967296.0)*(1.0/4294967296.0)))*NVUI(%1$s_pfrac);";
  printblock(lc);
}

/*********************************************************/
/* code for phasor opcode                                  */
/*********************************************************/

void kphasorcode(tnode * tptr)

{
  int lc = 0;

  z[lc++]="if (NVI(%1$s_kcyc))";   /* fast path */
  z[lc++]="{";
  z[lc++]="  if (cps >= 0)";  /* positive frequency */
  z[lc++]="   {";
  z[lc++]="    nint = (unsigned int)(index = cps*((float)(4294967296.0/EV(KRATE))));";
  z[lc++]="    while (index >= 4294967296.0F)";
  z[lc++]="      nint = (unsigned int)(index -= 4294967296.0F);";
  z[lc++]="    nfrac = (unsigned int)(4294967296.0F*(index - nint));";
  z[lc++]="    NVUI(%1$s_pfrac) = (j = NVUI(%1$s_pfrac)) + nfrac;";
  z[lc++]="    NVUI(%1$s_pint) += nint + (NVUI(%1$s_pfrac) < j);";
  z[lc++]="   }";
  z[lc++]="  else";   /* negative frequency */
  z[lc++]="   {";
  z[lc++]="    nint = (unsigned int)(index = -cps*((float)(4294967296.0/EV(KRATE))));";
  z[lc++]="    while (index >= 4294967296.0F)";
  z[lc++]="      nint = (unsigned int)(index -= 4294967296.0F);";
  z[lc++]="    nfrac = (unsigned int)(4294967296.0F*(index - nint));";
  z[lc++]="    NVUI(%1$s_pfrac) = (j = NVUI(%1$s_pfrac)) - nfrac;";
  z[lc++]="    NVUI(%1$s_pint) -= nint + (NVUI(%1$s_pfrac) > j);";
  z[lc++]="   }";
  z[lc++]="}";
  z[lc++]="else";             /* for first time through */ 
  z[lc++]="  NVI(%1$s_kcyc) = 1;";
  z[lc++]="ret = ((float)(1.0/4294967296.0))*NVUI(%1$s_pint) +";
  z[lc++]="      ((float)((1.0/4294967296.0)*(1.0/4294967296.0)))*NVUI(%1$s_pfrac);";
  printblock(lc);
}


/*********************************************************/
/*         customizes buzz phase increment               */
/*********************************************************/

void buzzphaseincr(int * lcptr, tnode * cps)

{
  int lc = *lcptr;
 
  switch(cps->rate) {
  case ARATETYPE:
    z[lc++]="if ((NV(%1$s_p) += EV(ATIME)*cps) >= 1.0F)";
    z[lc++]=" NV(%1$s_p) -= (int)NV(%1$s_p);";
    break;
  case KRATETYPE:
    z[lc++]="if ((NV(%1$s_p) += NV(%1$s_d)) >= 1.0F)";
    z[lc++]=" NV(%1$s_p) -= (int)NV(%1$s_p);";
    break;
  case IRATETYPE:
    if (cps->vol == CONSTANT)
      {
	z[lc++]="if ((NV(%1$s_p) += EV(ATIME)*cps) >= 1.0F)";
	z[lc++]=" NV(%1$s_p) -= (int)NV(%1$s_p);";
      }
    else
      {
	z[lc++]="if ((NV(%1$s_p) += NV(%1$s_d)) >= 1.0F)";
	z[lc++]=" NV(%1$s_p) -= (int)NV(%1$s_p);";
      }
    break;
  }

  *lcptr = lc;

}

/*********************************************************/
/*         initialize buzz phase increment               */
/*********************************************************/

void buzzphaseinit(int * lcptr, tnode * cps)

{
  int lc = *lcptr;
 
  if ((cps->rate == KRATETYPE)||
      ((cps->rate == IRATETYPE)&&(cps->vol != CONSTANT)))
    z[lc++]="  NV(%1$s_d) = EV(ATIME)*cps;";

  *lcptr = lc;

}


/*********************************************************/
/* code for buzz opcode                                 */
/*********************************************************/

void buzzcode(tnode * tptr)

{
  int lc = 0;
  tnode * cps, * nharm, * lowharm, * rolloff;

  cps = firstopcodearg(tptr);
  nharm = cps->next->next;
  lowharm = nharm->next->next;
  rolloff = lowharm->next->next;

  /* optimizations left to do
   *
   * [1] rate-optimize first-pass block 
   * [2] table replacement when rolloff/nharm/lowharm i-rate 
   * [3] special code for lowharm=0,rolloff= +/- 1
   *
   */

  acycleguard(tptr, &lc);
  z[lc++]="{";
  buzzphaseincr(&lc, cps);
  z[lc++]="  if (NV(%1$s_p) < 0.0F)";
  z[lc++]="   NV(%1$s_p) += 1 - (int)NV(%1$s_p);";
  z[lc++]="  if ((q = NV(%1$s_p)*NV(%1$s_qtab)) >= 1.0F)";
  z[lc++]="   q -= (int)q;";
  z[lc++]="  if ((n = NV(%1$s_p)*NV(%1$s_ntab)) >= 1.0F)";
  z[lc++]="   n -= (int)n;";
  z[lc++]="  c1 = NV(%1$s_r)*TCOS(rad = TRIGSIZEF*NV(%1$s_p));";
  z[lc++]="  s1 = NV(%1$s_r)*TSIN(rad);";
  z[lc++]="  c2 = NV(%1$s_k2)*TCOS(rad = TRIGSIZEF*n);";
  z[lc++]="  s2 = NV(%1$s_k2)*TSIN(rad);";
  z[lc++]="  c3 = NV(%1$s_scale)*TCOS(rad = TRIGSIZEF*q);";
  z[lc++]="  s3 = NV(%1$s_scale)*TSIN(rad);";
  z[lc++]="  if ((denom = (NV(%1$s_k1)-c1-c1)) > BUZZDIVISOR)";
  z[lc++]="   {";
  z[lc++]="     ret = ((1.0F-c1)*(c3-c2) + s1*(s2-s3))/denom;";
  z[lc++]="   }";
  z[lc++]="  else";
  z[lc++]="   {";
  z[lc++]="    c1 = NV(%1$s_r)*cos(rad = 6.283185F*NV(%1$s_p));";
  z[lc++]="    s1 = NV(%1$s_r)*sin(rad);";
  z[lc++]="    c2 = NV(%1$s_k2)*cos(rad = 6.283185F*n);";
  z[lc++]="    s2 = NV(%1$s_k2)*sin(rad);";
  z[lc++]="    c3 = NV(%1$s_scale)*cos(rad = 6.283185F*q);";
  z[lc++]="    s3 = NV(%1$s_scale)*sin(rad);";
  z[lc++]="    denom = NV(%1$s_k1)-c1-c1;";
  z[lc++]="    ret = ((1.0F-c1)*(c3-c2) + s1*(s2-s3))/denom;";
  z[lc++]="   }";
  z[lc++]="}";
  z[lc++]="else";     /* first apass each cycle */
  z[lc++]="{";
  z[lc++]="  if (lowharm < 0.0F)";
  genex(&lc,tptr->optr->down,"Lowharm < 0");
  z[lc++]="  tnharm = (int)((0.5F*EV(ARATE)/((cps>1.0F)?cps:1.0F))-lowharm-1.0F);";
  z[lc++]="  if ((nharm > 0.0F) && (nharm < tnharm))";
  z[lc++]="    tnharm = (int)nharm;";
  z[lc++]="  NV(%1$s_qtab)=(float)(1+(int)lowharm);";
  z[lc++]="  NV(%1$s_ntab)=(float)(2+tnharm+(int)lowharm);";
  z[lc++]="  NV(%1$s_r) = rolloff;";
  z[lc++]="  if ((rolloff > BUZZMINVAL) && (rolloff < BUZZMAXVAL))"; 
  z[lc++]="   rolloff = NV(%1$s_r) = BUZZMINVAL;";
  z[lc++]="  if ((rolloff < - BUZZMINVAL) && (rolloff > - BUZZMAXVAL))"; 
  z[lc++]="   rolloff = NV(%1$s_r) = - BUZZMINVAL;";
  z[lc++]="  NV(%1$s_k1)= 1 + rolloff*rolloff;";
  z[lc++]="  if (rolloff)";
  z[lc++]="   {";
  z[lc++]="    dscale=pow((double)rolloff,(double)(tnharm+1));";
  z[lc++]="    if (rolloff > 0.0F)";
  z[lc++]="     {";  
  z[lc++]="       NV(%1$s_k2) = (1-rolloff)/((1.0/dscale)-1.0);";
  z[lc++]="       NV(%1$s_scale) = (1.0-rolloff)/(1.0-dscale);";
  z[lc++]="     }";
  z[lc++]="    else";
  z[lc++]="     {";  
  z[lc++]="       NV(%1$s_k2) = (1.0+rolloff)/((1.0/dscale)+((tnharm&1)?-1.0:1.0));";
  z[lc++]="       NV(%1$s_scale) = (1.0+rolloff)/(1.0-fabs(dscale));";
  z[lc++]="     }";
  z[lc++]="   }";
  z[lc++]="  else";
  z[lc++]="   {";
  z[lc++]="      NV(%1$s_scale)= 1.0F;";
  z[lc++]="      NV(%1$s_k2)=0.0F;";
  z[lc++]="   }";
  buzzphaseinit(&lc, cps);
  z[lc++]="  if (NVI(%1$s_kcyc) != 0)";
  z[lc++]="   {";
  buzzphaseincr(&lc, cps);
  z[lc++]="    if (NV(%1$s_p) < 0.0F)";
  z[lc++]="     NV(%1$s_p) += 1 - (int)NV(%1$s_p);";
  z[lc++]="    if ((q = NV(%1$s_p)*NV(%1$s_qtab)) >= 1.0F)";
  z[lc++]="     q -= (int)q;";
  z[lc++]="    if ((n = NV(%1$s_p)*NV(%1$s_ntab)) >= 1.0F)";
  z[lc++]="     n -= (int)n;";
  z[lc++]="   }";
  z[lc++]="  else";
  z[lc++]="     n = q = 0.0F;";
  kcycassign(tptr, &lc);
  z[lc++]="  c1 = rolloff*TCOS(rad = TRIGSIZEF*NV(%1$s_p));";
  z[lc++]="  s1 = rolloff*TSIN(rad);";
  z[lc++]="  c2 = NV(%1$s_k2)*TCOS(rad = TRIGSIZEF*n);";
  z[lc++]="  s2 = NV(%1$s_k2)*TSIN(rad);";
  z[lc++]="  c3 = NV(%1$s_scale)*TCOS(rad = TRIGSIZEF*q);";
  z[lc++]="  s3 = NV(%1$s_scale)*TSIN(rad);";
  z[lc++]="  if ((denom = (NV(%1$s_k1)-c1-c1)) > BUZZDIVISOR)";
  z[lc++]="   {";
  z[lc++]="     ret = ((1.0F-c1)*(c3-c2) + s1*(s2-s3))/denom;";
  z[lc++]="   }";
  z[lc++]="  else";
  z[lc++]="   {";
  z[lc++]="    c1 = rolloff*cos(rad = 6.283185F*NV(%1$s_p));";
  z[lc++]="    s1 = rolloff*sin(rad);";
  z[lc++]="    c2 = NV(%1$s_k2)*cos(rad = 6.283185F*n);";
  z[lc++]="    s2 = NV(%1$s_k2)*sin(rad);";
  z[lc++]="    c3 = NV(%1$s_scale)*cos(rad = 6.283185F*q);";
  z[lc++]="    s3 = NV(%1$s_scale)*sin(rad);";
  z[lc++]="    denom = NV(%1$s_k1)-c1-c1;";
  z[lc++]="    ret = ((1.0F-c1)*(c3-c2) + s1*(s2-s3))/denom;";
  z[lc++]="   }";
  z[lc++]="}";

  /* fast path code */

  printblock(lc);
}


/*********************************************************/
/* code for pluck opcode                                 */
/*********************************************************/

void pluckcode_sinc(tnode * tptr)

{
  int lc = 0;

  z[lc++]="if (NVI(%1$s_first))";
  z[lc++]="{";
  z[lc++]="  if (cps >= 0)";  /* positive frequency */
  z[lc++]="   {";
  z[lc++]="    nint = (unsigned int)(index = cps*NV(%1$s_oconst));";
  z[lc++]="    while (index >= NT(TBL_%1$s_t).lenf)";
  z[lc++]="      nint = (unsigned int)(index -= NT(TBL_%1$s_t).lenf);";
  z[lc++]="    nfrac = (unsigned int)(4294967296.0F*(index - nint));";
  z[lc++]="    NVUI(%1$s_pfrac) = (j = NVUI(%1$s_pfrac)) + nfrac;";
  z[lc++]="    NVUI(%1$s_pint) += nint + (NVUI(%1$s_pfrac) < j);";
  z[lc++]="    if ((k = NVUI(%1$s_pint)) >= NT(TBL_%1$s_t).len)";
  z[lc++]="      k = (NVUI(%1$s_pint) -= NT(TBL_%1$s_t).len);";
  z[lc++]="   }";
  z[lc++]="  else";   /* negative frequency */
  z[lc++]="   {";
  z[lc++]="    nint = (unsigned int)(index = -cps*NV(%1$s_oconst));";
  z[lc++]="    while (index >= NT(TBL_%1$s_t).lenf)";
  z[lc++]="      nint = (unsigned int)(index -= NT(TBL_%1$s_t).lenf);";
  z[lc++]="    nfrac = (unsigned int)(4294967296.0F*(index - nint));";
  z[lc++]="    NVUI(%1$s_pfrac) = (j = NVUI(%1$s_pfrac)) - nfrac;";
  z[lc++]="    NVUI(%1$s_pint) -= nint + (NVUI(%1$s_pfrac) > j);";
  z[lc++]="    if ((k = NVUI(%1$s_pint)) >= NT(TBL_%1$s_t).len)";
  z[lc++]="      k = (NVUI(%1$s_pint) += NT(TBL_%1$s_t).len);";
  z[lc++]="   }";
  z[lc++]=" if ((++(NVI(%1$s_sc))) >= smoothrate)";
  z[lc++]=" {";
  z[lc++]="   NVI(%1$s_sc) = 0;";
  z[lc++]="   i = 2;";
  z[lc++]="   len = NT(TBL_%1$s_t).len;";
  z[lc++]="   atten *= 0.2F;";
  z[lc++]="   while (i <  (len + 2))";
  z[lc++]="   {";
  z[lc++]="     NT(TBL_%1$s_ts).t[i] = atten*(";
  z[lc++]="        NT(TBL_%1$s_t).t[(i - 2)] +";
  z[lc++]="        NT(TBL_%1$s_t).t[(i - 1)] +";
  z[lc++]="        NT(TBL_%1$s_t).t[(i    )] +";
  z[lc++]="        NT(TBL_%1$s_t).t[(i + 1)] +";
  z[lc++]="        NT(TBL_%1$s_t).t[(i + 2)]);";
  z[lc++]="     i++;";
  z[lc++]="    }";
  z[lc++]="   NT(TBL_%1$s_ts).t[0] = NT(TBL_%1$s_ts).t[len];";
  z[lc++]="   NT(TBL_%1$s_ts).t[1] = NT(TBL_%1$s_ts).t[len+1];";
  z[lc++]="   NT(TBL_%1$s_ts).t[len+2] = NT(TBL_%1$s_ts).t[2];";
  z[lc++]="   NT(TBL_%1$s_ts).t[len+3] = NT(TBL_%1$s_ts).t[3];";
  z[lc++]="   h = NT(TBL_%1$s_t).t;";
  z[lc++]="   NT(TBL_%1$s_t).t = NT(TBL_%1$s_ts).t;";
  z[lc++]="   NT(TBL_%1$s_ts).t = h;";
  z[lc++]="  }";
  z[lc++]=" if ((nint == 1) && (nfrac == 0))";
  z[lc++]="   ret = NT(TBL_%1$s_t).t[k];";
  z[lc++]=" else";
  z[lc++]="  {";
  z[lc++]="   if (index <= 1.0F)";
  z[lc++]="    {";
  z[lc++]="     sffl = 1.0F;";
  z[lc++]="     sfui = 0x00010000;";
  z[lc++]="     osincr = SINC_PILEN;";
  z[lc++]="    }";
  z[lc++]="   else";
  z[lc++]="    {";
  z[lc++]="      if (index < SINC_UPMAX)";
  z[lc++]="        sffl = 1.0F/index;";
  z[lc++]="      else";
  z[lc++]="        sffl = 1.0F/SINC_UPMAX;";
  z[lc++]="     sfui = ((float)(pow(2,16)))*sffl + 0.5F;";
  z[lc++]="     osincr = (SINC_PILEN*sfui) >> 16;";
  z[lc++]="    }";
  z[lc++]="   if (sfui == 0x00010000)";
  z[lc++]="    fptr = (NVUI(%1$s_pfrac)) >> (32 - SINC_LOG_PILEN);";
  z[lc++]="   else";
  z[lc++]="    fptr = (sfui*((NVUI(%1$s_pfrac)) >> 16)) >> (32 - SINC_LOG_PILEN);";
  z[lc++]="   ret = sinc[fptr]*NT(TBL_%1$s_t).t[k];";
  z[lc++]="   rptr = osincr - fptr;";
  z[lc++]="   fptr += osincr;";
  z[lc++]="   m = 0;";
  z[lc++]="   while (rptr < SINC_SIZE)";
  z[lc++]="    {";
  z[lc++]="      if ((k + (++m)) < NT(TBL_%1$s_t).len)";
  z[lc++]="        ret += sinc[rptr]*NT(TBL_%1$s_t).t[k + m];";
  z[lc++]="      else";
  z[lc++]="        ret += sinc[rptr]*NT(TBL_%1$s_t).t[k + m - NT(TBL_%1$s_t).len];";
  z[lc++]="      rptr += osincr;";
  z[lc++]="      if (fptr < SINC_SIZE)";
  z[lc++]="       {";
  z[lc++]="         if (k >= m)";
  z[lc++]="           ret += sinc[fptr]*NT(TBL_%1$s_t).t[k - m];";
  z[lc++]="         else";
  z[lc++]="           ret += sinc[fptr]*NT(TBL_%1$s_t).t[NT(TBL_%1$s_t).len + k - m];";
  z[lc++]="         fptr += osincr;";
  z[lc++]="       }";
  z[lc++]="    }";
  z[lc++]="   if (sfui != 0x00010000)";
  z[lc++]="     ret *= sffl;";
  z[lc++]="  }";
  z[lc++]="}";
  z[lc++]="else";
  z[lc++]="{";
  z[lc++]="  NVI(%1$s_first) = 1;";
  z[lc++]="  if (buflen<=0.0F)";
  genex(&lc,tptr->optr->down,"Buflen <= 0");
  z[lc++]="  len = i = NT(TBL_%1$s_t).len = NT(TBL_%1$s_ts).len =(int)buflen;";
  z[lc++]="  NT(TBL_%1$s_t).lenf = NT(TBL_%1$s_ts).lenf = (float) i;";
  z[lc++]="  NT(TBL_%1$s_ts).t = (float *) malloc((i+4)*sizeof(float));";
  z[lc++]="  NT(TBL_%1$s_ts).llmem = 1;";
  z[lc++]="  NT(TBL_%1$s_t).t = (float *) malloc((i+4)*sizeof(float));";
  z[lc++]="  NT(TBL_%1$s_t).llmem = 1;";
  z[lc++]="  NV(%1$s_oconst) = EV(ATIME)*len;";
  z[lc++]="  i = 0;";
  z[lc++]="  while (i < NT(TBL_%1$s_t).len)";
  z[lc++]="  {";
  z[lc++]="    NT(TBL_%1$s_t).t[i] = AP1.t[i % AP1.len];";
  z[lc++]="    i++;";
  z[lc++]="  }";
  z[lc++]="  NT(TBL_%1$s_t).t[len] = NT(TBL_%1$s_t).t[0];";
  z[lc++]="  NT(TBL_%1$s_t).t[len+1] = NT(TBL_%1$s_t).t[1];";
  z[lc++]="  NT(TBL_%1$s_t).t[len+2] = NT(TBL_%1$s_t).t[2];";
  z[lc++]="  NT(TBL_%1$s_t).t[len+3] = NT(TBL_%1$s_t).t[3];";
  z[lc++]="  if (cps >= 0)";  /* positive frequency */
  z[lc++]="   {";
  z[lc++]="    nint = (unsigned int)(index = cps*NV(%1$s_oconst));";
  z[lc++]="    while (index >= NT(TBL_%1$s_t).lenf)";
  z[lc++]="      nint = (unsigned int)(index -= NT(TBL_%1$s_t).lenf);";
  z[lc++]="    nfrac = (unsigned int)(4294967296.0F*(index - nint));";
  z[lc++]="   }";
  z[lc++]="  else";   /* negative frequency */
  z[lc++]="   {";
  z[lc++]="    nint = (unsigned int)(index = -cps*NV(%1$s_oconst));";
  z[lc++]="    while (index >= NT(TBL_%1$s_t).lenf)";
  z[lc++]="      nint = (unsigned int)(index -= NT(TBL_%1$s_t).lenf);";
  z[lc++]="    nfrac = (unsigned int)(4294967296.0F*(index - nint));";
  z[lc++]="   }";
  z[lc++]="  ret = NT(TBL_%1$s_t).t[0];";
  z[lc++]="  if (index > 1.0F)";
  z[lc++]="   {";
  z[lc++]="    if (index < SINC_UPMAX)";
  z[lc++]="      sffl = 1.0F/index;";
  z[lc++]="    else";
  z[lc++]="      sffl = 1.0F/SINC_UPMAX;";
  z[lc++]="    sfui = ((float)(pow(2,16)))*sffl + 0.5F;";
  z[lc++]="    osincr = (SINC_PILEN*sfui) >> 16;";
  z[lc++]="    fptr = rptr = osincr;";
  z[lc++]="    m = 0;";
  z[lc++]="    while (rptr < SINC_SIZE)";
  z[lc++]="     {";
  z[lc++]="       if ((++m) < NT(TBL_%1$s_t).len)";
  z[lc++]="         ret += sinc[rptr]*NT(TBL_%1$s_t).t[m];";
  z[lc++]="       else";
  z[lc++]="         ret += sinc[rptr]*NT(TBL_%1$s_t).t[m - NT(TBL_%1$s_t).len];";
  z[lc++]="       rptr += osincr;";
  z[lc++]="       if (fptr < SINC_SIZE)";
  z[lc++]="        {";
  z[lc++]="          ret += sinc[fptr]*NT(TBL_%1$s_t).t[NT(TBL_%1$s_t).len - m];";
  z[lc++]="          fptr += osincr;";
  z[lc++]="        }";
  z[lc++]="     }";
  z[lc++]="   ret *= sffl;";
  z[lc++]="  }";
  z[lc++]="}";
  printblock(lc);
}


/*********************************************************/
/* code for pluck opcode                                 */
/*********************************************************/

void pluckcode(tnode * tptr)

{
  int lc = 0;

  z[lc++]="if (NVI(%1$s_first))";
  z[lc++]="{";
  z[lc++]="  if (cps >= 0)";  /* positive frequency */
  z[lc++]="   {";
  z[lc++]="    nint = (unsigned int)(index = cps*NV(%1$s_oconst));";
  z[lc++]="    while (index >= NT(TBL_%1$s_t).lenf)";
  z[lc++]="      nint = (unsigned int)(index -= NT(TBL_%1$s_t).lenf);";
  z[lc++]="    nfrac = (unsigned int)(4294967296.0F*(index - nint));";
  z[lc++]="    NVUI(%1$s_pfrac) = (j = NVUI(%1$s_pfrac)) + nfrac;";
  z[lc++]="    NVUI(%1$s_pint) += nint + (NVUI(%1$s_pfrac) < j);";
  z[lc++]="    if ((k = NVUI(%1$s_pint)) >= NT(TBL_%1$s_t).len)";
  z[lc++]="      k = (NVUI(%1$s_pint) -= NT(TBL_%1$s_t).len);";
  z[lc++]="   }";
  z[lc++]="  else";   /* negative frequency */
  z[lc++]="   {";
  z[lc++]="    nint = (unsigned int)(index = -cps*NV(%1$s_oconst));";
  z[lc++]="    while (index >= NT(TBL_%1$s_t).lenf)";
  z[lc++]="      nint = (unsigned int)(index -= NT(TBL_%1$s_t).lenf);";
  z[lc++]="    nfrac = (unsigned int)(4294967296.0F*(index - nint));";
  z[lc++]="    NVUI(%1$s_pfrac) = (j = NVUI(%1$s_pfrac)) - nfrac;";
  z[lc++]="    NVUI(%1$s_pint) -= nint + (NVUI(%1$s_pfrac) > j);";
  z[lc++]="    if ((k = NVUI(%1$s_pint)) >= NT(TBL_%1$s_t).len)";
  z[lc++]="      k = (NVUI(%1$s_pint) += NT(TBL_%1$s_t).len);";
  z[lc++]="   }";
  z[lc++]=" if ((++(NVI(%1$s_sc))) < smoothrate)";
  z[lc++]="  {";
  z[lc++]="    ret = NT(TBL_%1$s_t).t[k] + NVUI(%1$s_pfrac)*";
  z[lc++]="          ((float)(1.0/4294967296.0))*";
  z[lc++]="          (NT(TBL_%1$s_t).t[k+1] - NT(TBL_%1$s_t).t[k]);";
  z[lc++]="  }";
  z[lc++]=" else";
  z[lc++]=" {";
  z[lc++]="   NVI(%1$s_sc) = 0;";
  z[lc++]="   i = 2;";
  z[lc++]="   len = NT(TBL_%1$s_t).len;";
  z[lc++]="   atten *= 0.2F;";
  z[lc++]="   while (i <  (len + 2))";
  z[lc++]="   {";
  z[lc++]="     NT(TBL_%1$s_ts).t[i] = atten*(";
  z[lc++]="        NT(TBL_%1$s_t).t[(i - 2)] +";
  z[lc++]="        NT(TBL_%1$s_t).t[(i - 1)] +";
  z[lc++]="        NT(TBL_%1$s_t).t[(i    )] +";
  z[lc++]="        NT(TBL_%1$s_t).t[(i + 1)] +";
  z[lc++]="        NT(TBL_%1$s_t).t[(i + 2)]);";
  z[lc++]="     i++;";
  z[lc++]="    }";
  z[lc++]="   NT(TBL_%1$s_ts).t[0] = NT(TBL_%1$s_ts).t[len];";
  z[lc++]="   NT(TBL_%1$s_ts).t[1] = NT(TBL_%1$s_ts).t[len+1];";
  z[lc++]="   NT(TBL_%1$s_ts).t[len+2] = NT(TBL_%1$s_ts).t[2];";
  z[lc++]="   NT(TBL_%1$s_ts).t[len+3] = NT(TBL_%1$s_ts).t[3];";
  z[lc++]="   h = NT(TBL_%1$s_t).t;";
  z[lc++]="   NT(TBL_%1$s_t).t = NT(TBL_%1$s_ts).t;";
  z[lc++]="   NT(TBL_%1$s_ts).t = h;";
  z[lc++]="   ret = NT(TBL_%1$s_t).t[k] + NVUI(%1$s_pfrac)*";
  z[lc++]="         ((float)(1.0/4294967296.0))*";
  z[lc++]="         (NT(TBL_%1$s_t).t[k+1] - NT(TBL_%1$s_t).t[k]);";
  z[lc++]="  }";
  z[lc++]="}";
  z[lc++]="else";
  z[lc++]="{";
  z[lc++]="  NVI(%1$s_first) = 1;";
  z[lc++]="  if (buflen<=0.0F)";
  genex(&lc,tptr->optr->down,"Buflen <= 0");
  z[lc++]="  len = i = NT(TBL_%1$s_t).len = NT(TBL_%1$s_ts).len =(int)buflen;";
  z[lc++]="  NT(TBL_%1$s_t).lenf = NT(TBL_%1$s_ts).lenf = (float) i;";
  z[lc++]="  NT(TBL_%1$s_ts).t = (float *) malloc((i+4)*sizeof(float));";
  z[lc++]="  NT(TBL_%1$s_ts).llmem = 1;";
  z[lc++]="  NT(TBL_%1$s_t).t = (float *) malloc((i+4)*sizeof(float));";
  z[lc++]="  NT(TBL_%1$s_t).llmem = 1;";
  z[lc++]="  NV(%1$s_oconst) = EV(ATIME)*len;";
  z[lc++]="  i = 0;";
  z[lc++]="  while (i < NT(TBL_%1$s_t).len)";
  z[lc++]="  {";
  z[lc++]="    NT(TBL_%1$s_t).t[i] = AP1.t[i % AP1.len];";
  z[lc++]="    i++;";
  z[lc++]="  }";
  z[lc++]="  NT(TBL_%1$s_t).t[len] = NT(TBL_%1$s_t).t[0];";
  z[lc++]="  NT(TBL_%1$s_t).t[len+1] = NT(TBL_%1$s_t).t[1];";
  z[lc++]="  NT(TBL_%1$s_t).t[len+2] = NT(TBL_%1$s_t).t[2];";
  z[lc++]="  NT(TBL_%1$s_t).t[len+3] = NT(TBL_%1$s_t).t[3];";
  z[lc++]="  ret = NT(TBL_%1$s_t).t[0];";
  z[lc++]="}";
  printblock(lc);
}


/*********************************************************/
/* krate code for fft opcode                             */
/*********************************************************/

void fftkrate(tnode * tptr)

{
  int lc = 0;


  z[lc++]= "ret = NVI(%1$s_done);";
  z[lc++]= "NVI(%1$s_done)=0;";

  printblock(lc);
}


/*********************************************************/
/*           parse fft length parameter                  */
/*********************************************************/

int newfftlen(tnode * aptr)

{
  int newlen;

  if (aptr->vol != CONSTANT)
    return 0;

  newlen = make_int(aptr->down);

  if (!newlen)
    newlen = twocycle;

  return newlen;

}

/*********************************************************/
/*       parse fft size parameter                        */
/*********************************************************/

int newfftsize(tnode * aptr, tnode * eptr, int fftlen)

{
  int i;
  int newsize;

  if (aptr)
    {
      if (aptr->vol != CONSTANT)
	return 0;

      newsize = make_int(aptr->down);
      newsize = (newsize <= 0) ? fftlen : newsize;
    }
  else
    {
      newsize = fftlen;
    }

  /* check if too big, too small, not a power of 2 */

  i = 8192;

  if (newsize > i)
    {	      
      printf("Error: FFT size > 8192 selected.\n\n");
      showerrorplace(eptr->down->linenum, eptr->down->filename);
    }
  else
    {
      while (i >= 2)
	{
	  if (newsize == i)
	    break;
	  i >>= 1;
	}
      if ((i == 1) && newsize)
	{
	  printf("Error: FFT size %i not a power of 2.\n\n", newsize);
	  showerrorplace(eptr->down->linenum, eptr->down->filename);
	}
    }
  return newsize;

}

/*********************************************************/
/*             parse fft shift parameter                 */
/*********************************************************/

int newfftshift(tnode * aptr, int fftlen)

{
  int newshift;

  if (aptr->vol != CONSTANT)
    return 0;

  newshift = make_int(aptr->down);
  newshift = (newshift <= 0) ? fftlen : newshift;

  return newshift;

}


/*********************************************************/
/* checks for legal fft parameters                       */
/*********************************************************/

void fftparams(tnode * aptr, tnode * eptr, int * haslen, int * hasshift, 
	       int * hassize, int * haswin, int * fftlen, 
	       int * fftshift, int * fftsize)

{

  if (aptr)
    {
      aptr = aptr->next;
      *haslen = 1;
      *fftlen = newfftlen(aptr);
      aptr = aptr->next;
      if (aptr)
	{
	  aptr = aptr->next;
	  *hasshift = 1;    
	  *fftshift = newfftshift(aptr,*fftlen);
	  aptr = aptr->next;
	  if (aptr)
	    {
	      *hassize = 1;    
	      aptr = aptr->next;
	      *fftsize = newfftsize(aptr, eptr, *fftlen);
	      if (aptr->next != NULL)
		*haswin = 1;
	    }
	  else
	    *fftsize = newfftsize(NULL, eptr, *fftlen);
	}
      else
	{
	  *fftshift = *fftlen;
	  *fftsize = newfftsize(NULL, eptr, *fftlen);
	}
    }
  else
    {
      *fftsize = twocycle;
      *fftshift = twocycle;
      *fftlen = twocycle;
    }
  
}



/*********************************************************/
/* requests tables in sa.c file                          */
/*********************************************************/

void ffttableset(int fftsize)

{
  int i, j;
  
  j = 4;
  for (i=2;i<FFTTABSIZE;i++)
    {
      if (!fftsize || (j == fftsize))
	{
	  ffttables[i]++;
	}
      j *= 2;
    }
}


/*********************************************************/
/* arate code for fft opcode                             */
/*********************************************************/

void fftarate(tnode * tptr)

{
  int lc = 0;
  int haslen = 0;
  int hasshift = 0;
  int hassize = 0;
  int haswin = 0;
  int fftlen, fftshift, fftsize;
  tnode * aptr;

  aptr = firstopcodearg(tptr);

  fftparams(aptr->next->next->next->next->next, aptr,
	    &haslen, &hasshift, &hassize, &haswin, 
	    &fftlen, &fftshift, &fftsize);

  ffttableset(fftsize);

  fprintf(outfile,"\n#undef FFTTAB\n");
  fprintf(outfile,"\n#undef FFTMAP\n");
  fprintf(outfile,"\n#undef FFTSCALE\n");
  
  if (fftsize)
    {
      fprintf(outfile,"#define FFTTAB(x) fft%itab[x]\n",fftsize);
      fprintf(outfile,"#define FFTMAP(x) fft%imap[x]\n",fftsize);
      fprintf(outfile,"#define FFTSCALE %eF\n\n",
	      1.0F/sqrt((double)fftsize));
    }
  else
    {
      fprintf(outfile,"#define FFTTAB(x) NT(TBL_%s_cos).t[x]\n",
	      currinstancename);
      fprintf(outfile,"#define FFTMAP(x) ((int *)NT(TBL_%s_map).t)[x]\n\n",
	      currinstancename);
      fprintf(outfile,"#define FFTSCALE NV(%s_scale)\n\n",
	      currinstancename);
    }

  /* generate code for bounds-checking */

  z[lc++]= "if (NT(TBL_%1$s_buffer).t == NULL)";
  z[lc++]= "{";
  mz(lc); sprintf(z[lc++], "i = %i;",twocycle);

  if (!haslen)
    {
      z[lc++]= " NT(TBL_%1$s_buffer).len = i;";
    }
  else
    {
      z[lc++]= " if (va_len <= 0.0F)";
      z[lc++]= "   NT(TBL_%1$s_buffer).len = i;";
      z[lc++]= " else";
      z[lc++]= "   i = NT(TBL_%1$s_buffer).len = (int)(va_len+0.5F);";
    }
  z[lc++]= " NT(TBL_%1$s_buffer).t = (float *) calloc(i,sizeof(float));";
  z[lc++]= " NT(TBL_%1$s_buffer).llmem = 1;";
  if (!hassize)
    z[lc++]= " i = NT(TBL_%1$s_new).len = NT(TBL_%1$s_buffer).len;";
  else
    {
      z[lc++]= " if (va_size <= 0.0F)";
      z[lc++]= "   i = NT(TBL_%1$s_new).len = NT(TBL_%1$s_buffer).len;";
      z[lc++]= " else";
      z[lc++]= "   i = NT(TBL_%1$s_new).len = (int)(va_size+0.5F);";
    }
  z[lc++]= "j = 0;";
  z[lc++]= "i >>= 1;";
  z[lc++]= "while (i > 0)";
  z[lc++]= "{";
  z[lc++]= "  if (i&1)";
  z[lc++]= "   j++;";
  z[lc++]= "  i >>= 1;";
  z[lc++]= "}";
  z[lc++]= "if (j != 1)";
  genex(&lc,tptr->optr->down,"Size not a factor of two");
  z[lc++]= "if ((i = NT(TBL_%1$s_new).len) > 8192)";
  genex(&lc,tptr->optr->down,"Size > 8192");
  z[lc++]= " NT(TBL_%1$s_new).t = (float *) calloc(i,sizeof(float));";
  z[lc++]= " NT(TBL_%1$s_new).llmem = 1;";
  if (!fftsize)
    {
      z[lc++]= " switch(i) {";
      z[lc++]= " case 4:";
      z[lc++]= " NT(TBL_%1$s_cos).t = &fft4tab[0];";
      z[lc++]= " NT(TBL_%1$s_map).t = (float *)(&fft4map[0]);";
      z[lc++]= " break;";
      z[lc++]= " case 8:";
      z[lc++]= " NT(TBL_%1$s_cos).t = &fft8tab[0];";
      z[lc++]= " NT(TBL_%1$s_map).t = (float *)(&fft8map[0]);";
      z[lc++]= " break;";
      z[lc++]= " case 16:";
      z[lc++]= " NT(TBL_%1$s_cos).t = &fft16tab[0];";
      z[lc++]= " NT(TBL_%1$s_map).t = (float *)(&fft16map[0]);";
      z[lc++]= " break;";
      z[lc++]= " case 32:";
      z[lc++]= " NT(TBL_%1$s_cos).t = &fft32tab[0];";
      z[lc++]= " NT(TBL_%1$s_map).t = (float *)(&fft32map[0]);";
      z[lc++]= " break;";
      z[lc++]= " case 64:";
      z[lc++]= " NT(TBL_%1$s_cos).t = &fft64tab[0];";
      z[lc++]= " NT(TBL_%1$s_map).t = (float *)(&fft64map[0]);";
      z[lc++]= " break;";
      z[lc++]= " case 128:";
      z[lc++]= " NT(TBL_%1$s_cos).t = &fft128tab[0];";
      z[lc++]= " NT(TBL_%1$s_map).t = (float *)(&fft128map[0]);";
      z[lc++]= " break;";
      z[lc++]= " case 256:";
      z[lc++]= " NT(TBL_%1$s_cos).t = &fft256tab[0];";
      z[lc++]= " NT(TBL_%1$s_map).t = (float *)(&fft256map[0]);";
      z[lc++]= " break;";
      z[lc++]= " case 512:";
      z[lc++]= " NT(TBL_%1$s_cos).t = &fft512tab[0];";
      z[lc++]= " NT(TBL_%1$s_map).t = (float *)(&fft512map[0]);";
      z[lc++]= " break;";
      z[lc++]= " case 1024:";
      z[lc++]= " NT(TBL_%1$s_cos).t = &fft1024tab[0];";
      z[lc++]= " NT(TBL_%1$s_map).t = (float *)(&fft1024map[0]);";
      z[lc++]= " break;";
      z[lc++]= " case 2048:";
      z[lc++]= " NT(TBL_%1$s_cos).t = &fft2048tab[0];";
      z[lc++]= " NT(TBL_%1$s_map).t = (float *)(&fft2048map[0]);";
      z[lc++]= " break;";
      z[lc++]= " case 4096:";
      z[lc++]= " NT(TBL_%1$s_cos).t = &fft4096tab[0];";
      z[lc++]= " NT(TBL_%1$s_map).t = (float *)(&fft4096map[0]);";
      z[lc++]= " break;";
      z[lc++]= " case 8192:";
      z[lc++]= " NT(TBL_%1$s_cos).t = &fft8192tab[0];";
      z[lc++]= " NT(TBL_%1$s_map).t = (float *)(&fft8192map[0]);";
      z[lc++]= " break;";
      z[lc++]= " }";
    }

  if (!hasshift)
    z[lc++]= "  NT(TBL_%1$s_buffer).start = NT(TBL_%1$s_buffer).len;";
  else
    {      
      z[lc++]= " NT(TBL_%1$s_buffer).start = (int)(va_shift+0.5F);";
      z[lc++]= " if ((NT(TBL_%1$s_buffer).start <= 0)||";
      z[lc++]= "     (NT(TBL_%1$s_buffer).start>NT(TBL_%1$s_buffer).len))";
      z[lc++]= "   NT(TBL_%1$s_buffer).start = NT(TBL_%1$s_buffer).len;";
      z[lc++]= " else";
      z[lc++]= "   NT(TBL_%1$s_buffer).tend = NT(TBL_%1$s_buffer).len - ";
      z[lc++]= "                              NT(TBL_%1$s_buffer).start;";
    }
  if (haswin)
    {
      z[lc++]= " if (AP3.len < NT(TBL_%1$s_buffer).len)";
      genex(&lc,tptr->optr->down,"Window length < buffer length");
    }
  z[lc++]= " if (AP1.len < NT(TBL_%1$s_new).len)";
  genex(&lc,tptr->optr->down,"Re[] table length < FFT length");
  z[lc++]= " if (AP2.len < NT(TBL_%1$s_new).len)";
  genex(&lc,tptr->optr->down,"Im[] table length < FFT length");
  if (!fftsize)
    z[lc++]= " NV(%1$s_scale) = (float)sqrt(1.0F/NT(TBL_%1$s_new).len);";
  z[lc++]= "}";


  z[lc++]= "  i = NT(TBL_%1$s_buffer).tend++;";
  z[lc++]= "  NT(TBL_%1$s_buffer).t[i] = input;";
  z[lc++]= "  if (NT(TBL_%1$s_buffer).tend == NT(TBL_%1$s_buffer).len)";
  z[lc++]= "    NT(TBL_%1$s_buffer).tend = 0;";
  z[lc++]= "  if (NT(TBL_%1$s_buffer).tend == NT(TBL_%1$s_buffer).end)";
  z[lc++]= "   {";
  if (!haswin)
    {
      z[lc++]= "     if ((i=NT(TBL_%1$s_new).len-NT(TBL_%1$s_buffer).len)>0)";
      z[lc++]= "       memset(&(NT(TBL_%1$s_new).t[NT(TBL_%1$s_buffer).len]),";
      z[lc++]= "              0, i*sizeof(NT(0)));";
      z[lc++]= "     i = NT(TBL_%1$s_buffer).len - 1;";
      z[lc++]= "     if ((j = NT(TBL_%1$s_buffer).end - 1) < 0)";
      z[lc++]= "         j = NT(TBL_%1$s_buffer).len - 1;";
      z[lc++]= "     while (i>=0)";
      z[lc++]= "      {";
      z[lc++]= "       NT(TBL_%1$s_new).t[i--] = NT(TBL_%1$s_buffer).t[j--];";
      z[lc++]= "       if (j < 0)";
      z[lc++]= "         j = NT(TBL_%1$s_buffer).len - 1;";
      z[lc++]= "      }";
    }
  else
    {
      z[lc++]= "     if ((i=NT(TBL_%1$s_new).len-NT(TBL_%1$s_buffer).len)>0)";
      z[lc++]= "       memset(&(NT(TBL_%1$s_new).t[NT(TBL_%1$s_buffer).len]),";
      z[lc++]= "              0, i*sizeof(NT(0)));";
      z[lc++]= "     i = NT(TBL_%1$s_buffer).len - 1;";
      z[lc++]= "     if ((j = NT(TBL_%1$s_buffer).end - 1) < 0)";
      z[lc++]= "         j = NT(TBL_%1$s_buffer).len - 1;";
      z[lc++]= "     while (i>=0)";
      z[lc++]= "      {";
      z[lc++]= "       NT(TBL_%1$s_new).t[i] = NT(TBL_%1$s_buffer).t[j--]";
      z[lc++]= "                                *AP3.t[i];";
      z[lc++]= "       i--;";
      z[lc++]= "       if (j < 0)";
      z[lc++]= "         j = NT(TBL_%1$s_buffer).len - 1;";
      z[lc++]= "      }";
    }
  z[lc++]= "     if (NT(TBL_%1$s_buffer).start <NT(TBL_%1$s_buffer).len)";
  z[lc++]= "      {";
  z[lc++]= "        NT(TBL_%1$s_buffer).end += NT(TBL_%1$s_buffer).start;";
  z[lc++]= "        if (NT(TBL_%1$s_buffer).end >= NT(TBL_%1$s_buffer).len)";
  z[lc++]= "            NT(TBL_%1$s_buffer).end -= NT(TBL_%1$s_buffer).len;";
  z[lc++]= "      }";
  z[lc++]= "     NVI(%1$s_done)=1;";
  z[lc++]= "     len = NT(TBL_%1$s_new).len;";
  z[lc++]= "      bincr = len >> 2;";
  z[lc++]= "     for (i=0; i < len; i += 2)";
  z[lc++]= "       AP1.t[i] = AP1.t[i+1] = NT(TBL_%1$s_new).t[FFTMAP(i)];";
  z[lc++]= "     for (i=1; i < len; i += 2)";
  z[lc++]= "      {";
  z[lc++]= "       j = FFTMAP(i);";
  z[lc++]= "       AP1.t[i-1] +=  NT(TBL_%1$s_new).t[j];";
  z[lc++]= "       AP1.t[i] -=  NT(TBL_%1$s_new).t[j];";
  z[lc++]= "      }";
  z[lc++]= "     if (4 <= len)";
  z[lc++]= "      {";
  z[lc++]= "        for (i=0; i < len; i += 4)";
  z[lc++]= "        {";
  z[lc++]= "          re1 = AP1.t[i];";
  z[lc++]= "          AP1.t[i] += AP1.t[i+2];";
  z[lc++]= "          AP1.t[i+2] = - AP1.t[i+2] + re1;";
  z[lc++]= "          AP2.t[i+1] = - AP1.t[i+3];";
  z[lc++]= "          AP2.t[i+3] =   AP1.t[i+3];";
  z[lc++]= "          AP1.t[i+3] =   AP1.t[i+1];";
  z[lc++]= "        }";
  z[lc++]= "        bincr >>= 1;";
  z[lc++]= "      }";
  z[lc++]= "     if (8 <= len)";
  z[lc++]= "      {";
  z[lc++]= "        for (i=0; i < len; i += 8)";
  z[lc++]= "        {";
  z[lc++]= "          re1 = AP1.t[i];";
  z[lc++]= "          AP1.t[i] += AP1.t[i+4];";
  z[lc++]= "          AP1.t[i+4] = - AP1.t[i+4] + re1;";
  z[lc++]= "          AP2.t[i] = AP2.t[i+4] = 0.0F;";
  z[lc++]= "          re1 = AP1.t[i+1];";
  z[lc++]= "          im1 = AP2.t[i+1];";
  z[lc++]= "          re2 = AP1.t[i+5];";
  z[lc++]= "          im2 = AP2.t[i+5];";
  z[lc++]= "          AP1.t[i+5] = 0.707107F*(re2 + im2);";
  z[lc++]= "          AP2.t[i+5] = 0.707107F*(im2 - re2);";
  z[lc++]= "          AP1.t[i+1] += AP1.t[i+5];";
  z[lc++]= "          AP2.t[i+1] += AP2.t[i+5];";
  z[lc++]= "          AP1.t[i+5] = - AP1.t[i+5] + re1;";
  z[lc++]= "          AP2.t[i+5] = - AP2.t[i+5] + im1;";
  z[lc++]= "          re1 = AP1.t[i+2];";
  z[lc++]= "          AP2.t[i+6] = AP1.t[i+6];";
  z[lc++]= "          AP2.t[i+2] = -AP1.t[i+6];";
  z[lc++]= "          AP1.t[i+6] = + re1;";
  z[lc++]= "          re1 = AP1.t[i+3];";
  z[lc++]= "          im1 = AP2.t[i+3];";
  z[lc++]= "          re2 = AP1.t[i+7];";
  z[lc++]= "          im2 = AP2.t[i+7];";
  z[lc++]= "          AP1.t[i+7] = 0.707107F*(im2 - re2);";
  z[lc++]= "          AP2.t[i+7] = -0.707107F*(re2+ im2);";
  z[lc++]= "          AP1.t[i+3] += AP1.t[i+7];";
  z[lc++]= "          AP2.t[i+3] += AP2.t[i+7];";
  z[lc++]= "          AP1.t[i+7] = - AP1.t[i+7] + re1;";
  z[lc++]= "          AP2.t[i+7] = - AP2.t[i+7] + im1;";
  z[lc++]= "        }";
  z[lc++]= "        bsize = 16;";
  z[lc++]= "        bhalf = 8;";
  z[lc++]= "        bincr >>= 1;";
  z[lc++]= "      }";
  z[lc++]= "     while (bsize <= len)";
  z[lc++]= "      {";
  z[lc++]= "        for (i=0; i < len; i += bsize)";
  z[lc++]= "        {";
  z[lc++]= "          ds = (len>>2);";
  z[lc++]= "          dc = 0;";
  z[lc++]= "          for (j=i; j < bhalf+i; j++)";
  z[lc++]= "           {";
  z[lc++]= "              re1 = AP1.t[j];";
  z[lc++]= "              im1 = AP2.t[j];";
  z[lc++]= "              re2 = AP1.t[j+bhalf];";
  z[lc++]= "              im2 = AP2.t[j+bhalf];";
  z[lc++]= "              cv = FFTTAB(dc);";
  z[lc++]= "              sv = FFTTAB(ds);";
  z[lc++]= "              AP1.t[j+bhalf] = re2*cv - im2*sv;";
  z[lc++]= "              AP2.t[j+bhalf] = re2*sv + im2*cv;";
  z[lc++]= "              AP1.t[j] += AP1.t[j+bhalf];";
  z[lc++]= "              AP2.t[j] += AP2.t[j+bhalf];";
  z[lc++]= "              AP1.t[j+bhalf] = - AP1.t[j+bhalf] + re1;";
  z[lc++]= "              AP2.t[j+bhalf] = - AP2.t[j+bhalf] + im1;";
  z[lc++]= "              dc += bincr;";
  z[lc++]= "              ds += bincr;";
  z[lc++]= "           }";
  z[lc++]= "        }";
  z[lc++]= "        bsize = bsize << 1;";
  z[lc++]= "        bhalf = bhalf << 1;";
  z[lc++]= "        bincr = bincr >> 1;";
  z[lc++]= "      }";
  z[lc++]= "     for (i=0; i < len; i++)";
  z[lc++]= "      {";
  z[lc++]= "       AP1.t[i] *= FFTSCALE;";
  z[lc++]= "       AP2.t[i] *= FFTSCALE;";
  z[lc++]= "      }";
  z[lc++]= "     AP1.t[AP1.len] = AP1.t[0];";
  z[lc++]= "     AP2.t[AP2.len] = AP2.t[0];";
  z[lc++]= "   }";
  printbody(lc);
}


/*********************************************************/
/* code for fft opcode                                   */
/*********************************************************/

void fftcode(tnode * tptr)


{  
  if (currspecialrate == KRATETYPE)
    fftkrate(tptr);
  else
    fftarate(tptr);
}


/*********************************************************/
/*             core code of ifft                         */
/*********************************************************/

void ifftcore(tnode * tptr, int * lcptr)

{
  int lc = *lcptr;

  z[lc++]= " len = NT(TBL_%1$s_new).len;";
  z[lc++]= " bsize = len;";
  z[lc++]= " bhalf = len>>1;";
  z[lc++]= " ds = (len>>2);";
  z[lc++]= " dc = 0;";
  z[lc++]= " for (i=0; i < bhalf; i++)";
  z[lc++]= "  {";
  z[lc++]= "   NT(TBL_%1$s_new).t[i] = AP1.t[i] + AP1.t[i+bhalf];";
  z[lc++]= "   NT(TBL_%1$s_imnew).t[i] = AP2.t[i] + AP2.t[i+bhalf];";
  z[lc++]= "   re2 = AP1.t[i] - AP1.t[i+bhalf];";
  z[lc++]= "   im2 = AP2.t[i] - AP2.t[i+bhalf];";
  z[lc++]= "   cv = FFTTAB(dc++);";
  z[lc++]= "   sv = - FFTTAB(ds++);";
  z[lc++]= "   NT(TBL_%1$s_new).t[i+bhalf] = re2*cv - im2*sv;";
  z[lc++]= "   NT(TBL_%1$s_imnew).t[i+bhalf] = re2*sv + im2*cv;";
  z[lc++]= "  }";
  z[lc++]= " bhalf >>= 1;";
  z[lc++]= " bsize >>= 1;";
  z[lc++]= " bincr = 2;";
  z[lc++]= " while (bsize > 8)";
  z[lc++]= "  {";
  z[lc++]= "    for (i=0; i < len; i += bsize)";
  z[lc++]= "    {";
  z[lc++]= "     ds = (len>>2);";
  z[lc++]= "     dc = 0;";
  z[lc++]= "     for (j=i; j < bhalf+i; j++)";
  z[lc++]= "      {";
  z[lc++]= "       re2=NT(TBL_%1$s_new).t[j]-NT(TBL_%1$s_new).t[j+bhalf];";
  z[lc++]= "       im2=NT(TBL_%1$s_imnew).t[j]-NT(TBL_%1$s_imnew).t[j+bhalf];";
  z[lc++]= "       NT(TBL_%1$s_new).t[j] += NT(TBL_%1$s_new).t[j+bhalf];";
  z[lc++]= "       NT(TBL_%1$s_imnew).t[j] += NT(TBL_%1$s_imnew).t[j+bhalf];";
  z[lc++]= "       cv = FFTTAB(dc);";
  z[lc++]= "       sv = -FFTTAB(ds);";
  z[lc++]= "       NT(TBL_%1$s_new).t[j+bhalf] = re2*cv - im2*sv;";
  z[lc++]= "       NT(TBL_%1$s_imnew).t[j+bhalf] = re2*sv + im2*cv;";
  z[lc++]= "       dc += bincr;";
  z[lc++]= "       ds += bincr;";
  z[lc++]= "      }";
  z[lc++]= "     }";
  z[lc++]= "     bsize >>= 1;";
  z[lc++]= "     bhalf >>= 1;";
  z[lc++]= "     bincr <<= 1;";
  z[lc++]= "  }";
  z[lc++]= "  if (bsize == 8)";
  z[lc++]= "  {";
  z[lc++]= "    for (i=0; i < len; i += 8)";
  z[lc++]= "    {";
  z[lc++]= "     re2=NT(TBL_%1$s_new).t[i]-NT(TBL_%1$s_new).t[i+4];";
  z[lc++]= "     NT(TBL_%1$s_new).t[i] += NT(TBL_%1$s_new).t[i+4];";
  z[lc++]= "     NT(TBL_%1$s_new).t[i+4] = re2;";

  z[lc++]= "     re2=0.707107F*";
  z[lc++]= "           (NT(TBL_%1$s_new).t[i+1]-NT(TBL_%1$s_new).t[i+5]);";
  z[lc++]= "     im2=0.707107F*";
  z[lc++]= "           (NT(TBL_%1$s_imnew).t[i+1]-NT(TBL_%1$s_imnew).t[i+5]);";
  z[lc++]= "     NT(TBL_%1$s_new).t[i+1] += NT(TBL_%1$s_new).t[i+5];";
  z[lc++]= "     NT(TBL_%1$s_imnew).t[i+1] += NT(TBL_%1$s_imnew).t[i+5];";
  z[lc++]= "     NT(TBL_%1$s_new).t[i+5] = re2 - im2;";
  z[lc++]= "     NT(TBL_%1$s_imnew).t[i+5] = re2 + im2;";

  z[lc++]= "     im2=NT(TBL_%1$s_imnew).t[i+6]-NT(TBL_%1$s_imnew).t[i+2];";
  z[lc++]= "     NT(TBL_%1$s_new).t[i+2] += NT(TBL_%1$s_new).t[i+6];";
  z[lc++]= "     NT(TBL_%1$s_new).t[i+6] = im2;";

  z[lc++]= "     re2=0.707107F*";
  z[lc++]= "           (NT(TBL_%1$s_new).t[i+3]-NT(TBL_%1$s_new).t[i+7]);";
  z[lc++]= "     im2=0.707107F*";
  z[lc++]= "           (NT(TBL_%1$s_imnew).t[i+3]-NT(TBL_%1$s_imnew).t[i+7]);";
  z[lc++]= "     NT(TBL_%1$s_new).t[i+3] += NT(TBL_%1$s_new).t[i+7];";
  z[lc++]= "     NT(TBL_%1$s_imnew).t[i+3] += NT(TBL_%1$s_imnew).t[i+7];";
  z[lc++]= "     NT(TBL_%1$s_new).t[i+7] = -re2 - im2;";
  z[lc++]= "     NT(TBL_%1$s_imnew).t[i+7] = re2 - im2;";
  z[lc++]= "     }";
  z[lc++]= "     bsize = 4;";
  z[lc++]= "  }";
  z[lc++]= "  if (bsize == 4)";
  z[lc++]= "  {";
  z[lc++]= "    for (i=0; i < len; i += 4)";
  z[lc++]= "    {";
  z[lc++]= "     re2=NT(TBL_%1$s_new).t[i]-NT(TBL_%1$s_new).t[i+2];";
  z[lc++]= "     NT(TBL_%1$s_new).t[i] += NT(TBL_%1$s_new).t[i+2];";
  z[lc++]= "     NT(TBL_%1$s_new).t[i+2] = re2;";
  z[lc++]= "     im2= NT(TBL_%1$s_imnew).t[i+3]-NT(TBL_%1$s_imnew).t[i+1];";
  z[lc++]= "     NT(TBL_%1$s_new).t[i+1] += NT(TBL_%1$s_new).t[i+3];";
  z[lc++]= "     NT(TBL_%1$s_new).t[i+3] =  im2;";
  z[lc++]= "    }";
  z[lc++]= "    bsize = 2;";
  z[lc++]= "  }";
  z[lc++]= "  if (bsize == 2)";
  z[lc++]= "   for (i=0; i < len; i += 2)";
  z[lc++]= "    {";
  z[lc++]= "     re2=NT(TBL_%1$s_new).t[i]-NT(TBL_%1$s_new).t[i+1];";
  z[lc++]= "     NT(TBL_%1$s_new).t[i] += NT(TBL_%1$s_new).t[i+1];";
  z[lc++]= "     NT(TBL_%1$s_new).t[i] *= FFTSCALE;";
  z[lc++]= "     NT(TBL_%1$s_new).t[i+1] = re2*FFTSCALE;";
  z[lc++]= "     }";
  z[lc++]= "  else";
  z[lc++]= "   for (i=0; i < len; i++)";
  z[lc++]= "     NT(TBL_%1$s_new).t[i] *= FFTSCALE;";

  *lcptr = lc;
  return;
}

/*********************************************************/
/* code for ifft opcode                                 */
/*********************************************************/

void ifftcode(tnode * tptr)

{
  int lc = 0;
  int haslen = 0;
  int hasshift = 0;
  int hassize = 0;
  int haswin = 0;  
  int fftlen, fftshift, fftsize;
  int constscale;
  tnode * aptr;

  aptr = firstopcodearg(tptr);

  fftparams(aptr->next->next->next, aptr, 
	    &haslen, &hasshift, &hassize, &haswin, 
	    &fftlen, &fftshift, &fftsize);

  /* fftlen, fftsize, and fftshift are positive if known */

  ffttableset(fftsize);

  fprintf(outfile,"\n#undef FFTTAB\n");
  fprintf(outfile,"\n#undef FFTMAP\n");  
  fprintf(outfile,"\n#undef FFTSCALE\n");
  
  if (fftsize)
    {
      fprintf(outfile,"#define FFTTAB(x) fft%itab[x]\n",fftsize);
      fprintf(outfile,"#define FFTMAP(x) fft%imap[x]\n\n",fftsize);
    }
  else
    {
      fprintf(outfile,"#define FFTTAB(x) NT(TBL_%s_cos).t[x]\n",
	      currinstancename);
      fprintf(outfile,"#define FFTMAP(x) ((int *)NT(TBL_%s_map).t)[x]\n\n",
	      currinstancename);     
    }

  if ((constscale = (fftsize && (!hasshift || (fftshift && fftlen)))))
    {
      fprintf(outfile,"#define FFTSCALE %eF\n\n",
	      (hasshift ? ((float)fftshift)/fftlen : 1.0F)
	      /sqrt((double)fftsize));
    }
  else
    {
      fprintf(outfile,"#define FFTSCALE NV(%s_scale)\n\n",
	      currinstancename);
    }


  z[lc++]= "if (NT(TBL_%1$s_buffer).t == NULL)";
  z[lc++]= "{";
  mz(lc); sprintf(z[lc++], "i = %i;",twocycle);
  if (!haslen)
    {
      z[lc++]= " NT(TBL_%1$s_buffer).len = i;";
    }
  else
    {
      z[lc++]= " if (va_len <= 0.0F)";
      z[lc++]= "   NT(TBL_%1$s_buffer).len = i;";
      z[lc++]= " else";
      z[lc++]= "   i = NT(TBL_%1$s_buffer).len = (int)(va_len+0.5F);";
    }
  z[lc++]= " NT(TBL_%1$s_buffer).t = (float *) calloc(i,sizeof(float));";
  z[lc++]= " NT(TBL_%1$s_buffer).llmem = 1;";
  if (!hassize)
    z[lc++]= " i = NT(TBL_%1$s_new).len = NT(TBL_%1$s_buffer).len;";
  else
    {
      z[lc++]= " if (va_size <= 0.0F)";
      z[lc++]= "   i = NT(TBL_%1$s_new).len = NT(TBL_%1$s_buffer).len;";
      z[lc++]= " else";
      z[lc++]= "   i = NT(TBL_%1$s_new).len = (int)(va_size+0.5F);";
    }
  z[lc++]= "j = 0;";
  z[lc++]= "i >>= 1;";
  z[lc++]= "while (i > 0)";
  z[lc++]= "{";
  z[lc++]= "  if (i&1)";
  z[lc++]= "   j++;";
  z[lc++]= "  i >>= 1;";
  z[lc++]= "}";
  z[lc++]= "if (j != 1)";
  genex(&lc,tptr->optr->down,"Size not a factor of two");
  z[lc++]= "if ((i = NT(TBL_%1$s_new).len) > 8192)";
  genex(&lc,tptr->optr->down,"Size > 8192");
  z[lc++]= " NT(TBL_%1$s_new).t = (float *) calloc(i,sizeof(float));";
  z[lc++]= " NT(TBL_%1$s_new).llmem = 1;";
  z[lc++]= " NT(TBL_%1$s_imnew).t = (float *) calloc(i,sizeof(float));";
  z[lc++]= " NT(TBL_%1$s_imnew).llmem = 1;";
  if (!fftsize)
    {
      z[lc++]= " switch(i) {";
      z[lc++]= " case 4:";
      z[lc++]= " NT(TBL_%1$s_cos).t = &fft4tab[0];";
      z[lc++]= " NT(TBL_%1$s_map).t = (float *)(&fft4map[0]);";
      z[lc++]= " break;";
      z[lc++]= " case 8:";
      z[lc++]= " NT(TBL_%1$s_cos).t = &fft8tab[0];";
      z[lc++]= " NT(TBL_%1$s_map).t = (float *)(&fft8map[0]);";
      z[lc++]= " break;";
      z[lc++]= " case 16:";
      z[lc++]= " NT(TBL_%1$s_cos).t = &fft16tab[0];";
      z[lc++]= " NT(TBL_%1$s_map).t = (float *)(&fft16map[0]);";
      z[lc++]= " break;";
      z[lc++]= " case 32:";
      z[lc++]= " NT(TBL_%1$s_cos).t = &fft32tab[0];";
      z[lc++]= " NT(TBL_%1$s_map).t = (float *)(&fft32map[0]);";
      z[lc++]= " break;";
      z[lc++]= " case 64:";
      z[lc++]= " NT(TBL_%1$s_cos).t = &fft64tab[0];";
      z[lc++]= " NT(TBL_%1$s_map).t = (float *)(&fft64map[0]);";
      z[lc++]= " break;";
      z[lc++]= " case 128:";
      z[lc++]= " NT(TBL_%1$s_cos).t = &fft128tab[0];";
      z[lc++]= " NT(TBL_%1$s_map).t = (float *)(&fft128map[0]);";
      z[lc++]= " break;";
      z[lc++]= " case 256:";
      z[lc++]= " NT(TBL_%1$s_cos).t = &fft256tab[0];";
      z[lc++]= " NT(TBL_%1$s_map).t = (float *)(&fft256map[0]);";
      z[lc++]= " break;";
      z[lc++]= " case 512:";
      z[lc++]= " NT(TBL_%1$s_cos).t = &fft512tab[0];";
      z[lc++]= " NT(TBL_%1$s_map).t = (float *)(&fft512map[0]);";
      z[lc++]= " break;";
      z[lc++]= " case 1024:";
      z[lc++]= " NT(TBL_%1$s_cos).t = &fft1024tab[0];";
      z[lc++]= " NT(TBL_%1$s_map).t = (float *)(&fft1024map[0]);";
      z[lc++]= " break;";
      z[lc++]= " case 2048:";
      z[lc++]= " NT(TBL_%1$s_cos).t = &fft2048tab[0];";
      z[lc++]= " NT(TBL_%1$s_map).t = (float *)(&fft2048map[0]);";
      z[lc++]= " break;";
      z[lc++]= " case 4096:";
      z[lc++]= " NT(TBL_%1$s_cos).t = &fft4096tab[0];";
      z[lc++]= " NT(TBL_%1$s_map).t = (float *)(&fft4096map[0]);";
      z[lc++]= " break;";
      z[lc++]= " case 8192:";
      z[lc++]= " NT(TBL_%1$s_cos).t = &fft8192tab[0];";
      z[lc++]= " NT(TBL_%1$s_map).t = (float *)(&fft8192map[0]);";
      z[lc++]= " break;";
      z[lc++]= " }";
    }

  if (!hasshift)
    z[lc++]= "  NT(TBL_%1$s_buffer).start = NT(TBL_%1$s_buffer).len;";
  else
    {      
      z[lc++]= " NT(TBL_%1$s_buffer).start = (int)(va_shift+0.5F);";
      z[lc++]= " if ((NT(TBL_%1$s_buffer).start <= 0)||";
      z[lc++]= "     (NT(TBL_%1$s_buffer).start>NT(TBL_%1$s_buffer).len))";
      z[lc++]= "   NT(TBL_%1$s_buffer).start = NT(TBL_%1$s_buffer).len;";
    }
  z[lc++]= " NT(TBL_%1$s_buffer).end = NT(TBL_%1$s_buffer).start;";     
  z[lc++]= " if (NT(TBL_%1$s_buffer).end == NT(TBL_%1$s_buffer).len)";
  z[lc++]= "   NT(TBL_%1$s_buffer).end = 0;";
  if (haswin)
    {
      z[lc++]= " if (AP3.len < NT(TBL_%1$s_buffer).len)";
      genex(&lc,tptr->optr->down,"Window length < buffer length");
    }
  z[lc++]= " if (AP1.len < NT(TBL_%1$s_new).len)";
  genex(&lc,tptr->optr->down,"Re[] table length < IFFT output length");
  z[lc++]= " if (AP2.len < NT(TBL_%1$s_new).len)";
  genex(&lc,tptr->optr->down,"Im[] table length < IFFT output length");

  if (!constscale)
    {
      z[lc++]= "NV(%1$s_scale) = (float)sqrt(1.0F/NT(TBL_%1$s_new).len);";
      if (hasshift)
	{
	  z[lc++]= "NV(%1$s_scale) *= ((float)NT(TBL_%1$s_buffer).start)/";
	  z[lc++]= "                             NT(TBL_%1$s_buffer).len;";
	}
    }

  ifftcore(tptr, &lc);
  
  z[lc++]= " j = NT(TBL_%1$s_buffer).len;";
  z[lc++]= " if (j > NT(TBL_%1$s_new).len)";
  z[lc++]= "  j = NT(TBL_%1$s_new).len;";
  z[lc++]= " for (i = 0; i < j; i++)";
  if (!haswin)
    {
      z[lc++]= "   NT(TBL_%1$s_buffer).t[i] = ";
      z[lc++]= "      NT(TBL_%1$s_new).t[FFTMAP(i)];";
    }
  else
    {
      z[lc++]= "   NT(TBL_%1$s_buffer).t[i] = AP3.t[i]*";
      z[lc++]= "      NT(TBL_%1$s_new).t[FFTMAP(i)];";
    }
  z[lc++]= "}";

  z[lc++]= "  i = NT(TBL_%1$s_buffer).tend++;";
  z[lc++]= "  ret = NT(TBL_%1$s_buffer).t[i];";
  z[lc++]= "  if (NT(TBL_%1$s_buffer).tend >= NT(TBL_%1$s_buffer).len)";
  z[lc++]= "    NT(TBL_%1$s_buffer).tend -= NT(TBL_%1$s_buffer).len;";

  z[lc++]= "  if (NT(TBL_%1$s_buffer).tend == NT(TBL_%1$s_buffer).end)";
  z[lc++]= "   {";
  z[lc++]= "     if (NT(TBL_%1$s_buffer).start < NT(TBL_%1$s_buffer).len)";
  z[lc++]= "      {";
  z[lc++]= "        i = NT(TBL_%1$s_buffer).tend ";
  z[lc++]= "              - NT(TBL_%1$s_buffer).start;";
  z[lc++]= "        if (i < 0)";
  z[lc++]= "           i += NT(TBL_%1$s_buffer).len;";
  z[lc++]= "        while (i != NT(TBL_%1$s_buffer).tend)";
  z[lc++]= "         {";
  z[lc++]= "           NT(TBL_%1$s_buffer).t[i]=0.0F;";
  z[lc++]= "           if ((++i) == NT(TBL_%1$s_buffer).len)";
  z[lc++]= "             i = 0;";
  z[lc++]= "         }";
  z[lc++]= "       }";
  z[lc++]= "     else";
  z[lc++]= "      {";      
  z[lc++]= "       memset(&(NT(TBL_%1$s_buffer).t[0]),";
  z[lc++]= "              0, NT(TBL_%1$s_buffer).len*sizeof(NT(0)));";
  z[lc++]= "      }";
  z[lc++]= "     if (NT(TBL_%1$s_buffer).start < NT(TBL_%1$s_buffer).len)";
  z[lc++]= "      {";
  z[lc++]= "        NT(TBL_%1$s_buffer).end += NT(TBL_%1$s_buffer).start;";
  z[lc++]= "        if (NT(TBL_%1$s_buffer).end >= NT(TBL_%1$s_buffer).len)";
  z[lc++]= "          NT(TBL_%1$s_buffer).end -= NT(TBL_%1$s_buffer).len;";
  z[lc++]= "      }";

  ifftcore(tptr, &lc);

  z[lc++]= " len = NT(TBL_%1$s_buffer).len;";
  z[lc++]= " if (len > NT(TBL_%1$s_new).len)";
  z[lc++]= "  len = NT(TBL_%1$s_new).len;";
  z[lc++]= " j = NT(TBL_%1$s_buffer).tend;";
  z[lc++]= " for(i = 0; i < len; i++)";
  z[lc++]= "  {";  
  if (!haswin)
    {
      z[lc++]= "   NT(TBL_%1$s_buffer).t[j] += ";
      z[lc++]= "      NT(TBL_%1$s_new).t[FFTMAP(i)];";
    }
  else
    {
      z[lc++]= "   NT(TBL_%1$s_buffer).t[j] += AP3.t[i]*";
      z[lc++]= "      NT(TBL_%1$s_new).t[FFTMAP(i)];";
    }
  z[lc++]= "    j++;";
  z[lc++]= "    if (j == NT(TBL_%1$s_buffer).len)";
  z[lc++]= "      j = 0;";
  z[lc++]= "  }";
  z[lc++]= "}";
  printblock(lc);
}


/*********************************************************/
/*          code for aline opcode                        */
/*********************************************************/

void alinecode(tnode * tptr)

{
  int i = 1;
  int lc = 0;
  tnode * xptr = tptr->extra;
  
  z[lc++]="if (NVI(%1$s_first)>0)";
  z[lc++]="{";
  z[lc++]="  if ((NV(%1$s_t) += EV(ATIME)) <= NV(%1$s_cdur))";
  z[lc++]="    ret = (NV(%1$s_outT) += NV(%1$s_addK));";
  z[lc++]="  else";
  z[lc++]="  {";
  z[lc++]="   while (NV(%1$s_t) > NV(%1$s_cdur))";
  z[lc++]="    {";
  z[lc++]="      NV(%1$s_t) -= NV(%1$s_cdur);";
  z[lc++]="      switch(NVI(%1$s_first))\n      {";

  while (xptr != NULL)
    {
      mz(lc); sprintf(z[lc++],"    case %i:",i++);
      mz(lc); sprintf(z[lc++],"    NV(%s_cdur) = va_dur%i;",
	      currinstancename,i);

      if (isocheck)
	{
	  z[lc++]="    if (NV(%1$s_cdur) < 0.0F)";
	  genex(&lc,tptr->optr->down,"A duration < 0");
	}

      z[lc++]="      NV(%1$s_clp) = NV(%1$s_crp);";
      mz(lc); sprintf(z[lc++],"      NV(%s_crp) = va_x%i;",
	      currinstancename,i+1);
      z[lc++]="      break;";
      xptr = xptr->next->next->next;
      if (xptr != NULL)
	xptr = xptr->next;
    }
  z[lc++]="      default:";
  z[lc++]="      NVI(%1$s_first) = -100;"; 
  z[lc++]="      NV(%1$s_cdur) = NV(%1$s_t) + 10000.0F;";
  z[lc++]="      break;";
  z[lc++]="      }";
  z[lc++]="      NVI(%1$s_first)++;";
  z[lc++]="     }";
  z[lc++]="     NV(%1$s_mult)=(NV(%1$s_crp) - NV(%1$s_clp))/NV(%1$s_cdur);";
  z[lc++]="     ret = NV(%1$s_outT) = NV(%1$s_clp)+NV(%1$s_mult)*NV(%1$s_t);";
  z[lc++]="     NV(%1$s_addK) = NV(%1$s_mult)*EV(ATIME);";
  z[lc++]="     if (NVI(%1$s_first)<0)";
  z[lc++]="      ret = 0.0F;";
  z[lc++]="  }";
  z[lc++]="}";
  z[lc++]="else";
  z[lc++]="{";
  z[lc++]=" if (NVI(%1$s_first)<0)";
  z[lc++]="  ret = 0.0F;";
  z[lc++]=" else";
  z[lc++]="  {";
  z[lc++]="    NVI(%1$s_first) = 1;";
  z[lc++]="    ret = NV(%1$s_outT) = NV(%1$s_clp) = x1;";
  z[lc++]="    NV(%1$s_crp) = x2;";
  z[lc++]="    NV(%1$s_cdur) = dur1;";
  if (isocheck)
    {
      z[lc++]="    if (dur1 < 0.0F)";
      genex(&lc,tptr->optr->down,"A duration < 0");
    }
  z[lc++]="    if (dur1 > 0.0F)";
  z[lc++]="    NV(%1$s_addK) = EV(ATIME)*((x2 - x1)/dur1);";
  z[lc++]="  }";
  z[lc++]="}";
  printblock(lc);

}

/*********************************************************/
/*          code for kline opcode                        */
/*********************************************************/

void klinecode(tnode * tptr)

{
  int i = 1;
  int lc = 0;
  tnode * xptr = tptr->extra;

  z[lc++]="ret = 0.0F;";
  z[lc++]="if (NVI(%1$s_first)>0)";
  z[lc++]="{";
  z[lc++]="   NV(%1$s_t) += EV(KTIME);";
  z[lc++]="   ret = (NV(%1$s_outT) += NV(%1$s_addK));";
  z[lc++]="   if (NV(%1$s_t) > NV(%1$s_cdur))";
  z[lc++]="    {";
  z[lc++]="     while (NV(%1$s_t) > NV(%1$s_cdur))";
  z[lc++]="      {";
  z[lc++]="        NV(%1$s_t) -= NV(%1$s_cdur);";
  z[lc++]="        switch(NVI(%1$s_first))\n      {";

  while (xptr != NULL)
    {
      mz(lc); sprintf(z[lc++],"      case %i:",i++);
      mz(lc); sprintf(z[lc++],"      NV(%s_cdur) = va_dur%i;",
	      currinstancename,i);
      
      if (isocheck)
	{
	  z[lc++]="      if (NV(%1$s_cdur) < 0.0F)";
	  genex(&lc,tptr->optr->down,"A duration < 0");
	}

      z[lc++]="        NV(%1$s_clp) = NV(%1$s_crp);";
      mz(lc); sprintf(z[lc++],"        NV(%s_crp) = va_x%i;",
	      currinstancename,i+1);
      z[lc++]="        break;";
      xptr = xptr->next->next->next;
      if (xptr != NULL)
	xptr = xptr->next;
    }
  z[lc++]="        default:";
  z[lc++]="        NVI(%1$s_first) = -100;"; 
  z[lc++]="        NV(%1$s_cdur) = NV(%1$s_t) + 10000.0F;";
  z[lc++]="        break;";
  z[lc++]="        }";
  z[lc++]="      NVI(%1$s_first)++;";
  z[lc++]="     }";
  z[lc++]="     NV(%1$s_mult)=(NV(%1$s_crp) - NV(%1$s_clp))/NV(%1$s_cdur);";
  z[lc++]="     ret = NV(%1$s_outT) = NV(%1$s_clp)+NV(%1$s_mult)*NV(%1$s_t);";
  z[lc++]="     NV(%1$s_addK) = NV(%1$s_mult)*EV(KTIME);";
  z[lc++]="     if (NVI(%1$s_first)<0)";
  z[lc++]="       ret = 0.0F;";
  z[lc++]="   }";
  z[lc++]="}";
  z[lc++]="if (NVI(%1$s_first)==0)";
  z[lc++]="  {";
  z[lc++]="    NVI(%1$s_first) = 1;";
  z[lc++]="    ret = NV(%1$s_outT) = NV(%1$s_clp) = x1;";
  z[lc++]="    NV(%1$s_crp) = x2;";
  z[lc++]="    NV(%1$s_cdur) = dur1;";
  if (isocheck)
    { 
      z[lc++]="    if (dur1 < 0.0F)";
      genex(&lc,tptr->optr->down,"A duration < 0");
    } 
  z[lc++]="    if (dur1 > 0.0F)";
  z[lc++]="      NV(%1$s_addK) = EV(KTIME)*((x2 - x1)/dur1);";
  z[lc++]="  }";
  printblock(lc);

}

/*********************************************************/
/*         code for aexpon opcode                        */
/*********************************************************/

void aexpcode(tnode * tptr)

{
  int i = 1;
  int lc = 0;
  tnode * xptr = tptr->extra;

  z[lc++]="if (NVI(%1$s_first)>0)";
  z[lc++]="{";
  z[lc++]=" if ((NV(%1$s_t) += EV(ATIME)) <= NV(%1$s_cdur))";
  z[lc++]="  ret = (NV(%1$s_outT) *= NV(%1$s_multK));";
  z[lc++]=" else";
  z[lc++]="  {";
  z[lc++]="   while (NV(%1$s_t) > NV(%1$s_cdur))";
  z[lc++]="    {";
  z[lc++]="      NV(%1$s_t) -= NV(%1$s_cdur);";
  z[lc++]="      switch(NVI(%1$s_first))\n      {";

  while (xptr != NULL)
    {
      mz(lc); sprintf(z[lc++],"    case %i:",i++);
      mz(lc); sprintf(z[lc++],"    NV(%s_cdur) = va_dur%i;",
	      currinstancename,i);

      if (isocheck)
	{
	  z[lc++]="    if (NV(%1$s_cdur) < 0.0F)";
	  genex(&lc,tptr->optr->down,"A duration < 0");
	}

      z[lc++]="      NV(%1$s_clp) = NV(%1$s_crp);";
      mz(lc); sprintf(z[lc++],"      NV(%s_crp) = va_x%i;",
	      currinstancename,i+1);

      if (isocheck)
	{
	  z[lc++]="      if (NV(%1$s_crp) == 0.0F)";
	  genex(&lc,tptr->optr->down,"An xval == 0");
	  z[lc++]="      if ((NV(%1$s_crp) > 0.0F) != (NV(%1$s_clp) > 0.0F))";
	  genex(&lc,tptr->optr->down,"Mix of + and - xval signs");
	}

      z[lc++]="      break;";
      xptr = xptr->next->next->next;
      if (xptr != NULL)
	xptr = xptr->next;
    }
  z[lc++]="      default:";
  z[lc++]="      NVI(%1$s_first) = -100;"; 
  z[lc++]="      NV(%1$s_cdur) = NV(%1$s_t) + 10000.0F;";
  z[lc++]="      break;";
  z[lc++]="      }";
  z[lc++]="      NVI(%1$s_first)++;";
  z[lc++]="    }";
  z[lc++]="    NV(%1$s_ratio) = NV(%1$s_crp)/NV(%1$s_clp);";
  z[lc++]="    NV(%1$s_invcdur) = 1.0F/NV(%1$s_cdur);";
  z[lc++]="    NV(%1$s_outT) = NV(%1$s_clp)*(float)pow(NV(%1$s_ratio),NV(%1$s_invcdur)*NV(%1$s_t));";
  z[lc++]="    NV(%1$s_multK) = (float)pow(NV(%1$s_ratio),NV(%1$s_invcdur)*EV(ATIME));";
  z[lc++]="    if (NVI(%1$s_first)>0)";
  z[lc++]="     ret=NV(%1$s_outT);";
  z[lc++]="    else";
  z[lc++]="     ret = 0.0F;";
  z[lc++]="  }";
  z[lc++]="}";
  z[lc++]="else";
  z[lc++]=" {";
  z[lc++]="  if (NVI(%1$s_first)<0)";
  z[lc++]="    ret = 0.0F;";
  z[lc++]="  else";
  z[lc++]="   {";
  z[lc++]="     NVI(%1$s_first) = 1;";
  z[lc++]="     ret = NV(%1$s_outT) = NV(%1$s_clp) = x1;";
  z[lc++]="     NV(%1$s_crp) = x2;";
  z[lc++]="     NV(%1$s_cdur) = dur1;";
  if (isocheck)
    {
      z[lc++]="     if (dur1 < 0.0F)";
      genex(&lc,tptr->optr->down,"A duration < 0");
      z[lc++]="     if ((x1 == 0.0F)||(x2 == 0.0F))";
      genex(&lc,tptr->optr->down,"An xval == 0");
      z[lc++]="     if ((x1 > 0.0F) != (x2 > 0.0F))";
      genex(&lc,tptr->optr->down,"Mix of + and - xval signs");
    }
  z[lc++]="     NV(%1$s_ratio) = x2/x1;";
  z[lc++]="     if (dur1 > 0.0F)";
  z[lc++]="       NV(%1$s_invcdur) = 1.0F/dur1;";
  z[lc++]="     NV(%1$s_multK) = (float)pow(NV(%1$s_ratio),NV(%1$s_invcdur)*EV(ATIME));";
  z[lc++]="  }";
  z[lc++]=" }";
  printblock(lc);

}

/*********************************************************/
/*         code for  kexpon opcode                       */
/*********************************************************/

void kexpcode(tnode * tptr)

{
  int i = 1;
  int lc = 0;
  tnode * xptr = tptr->extra;
  
  z[lc++]="ret = 0.0F;";
  z[lc++]="if (NVI(%1$s_first)>0)";
  z[lc++]="{";
  z[lc++]="   NV(%1$s_t) += EV(KTIME);";
  z[lc++]="   ret = (NV(%1$s_outT) *= NV(%1$s_multK));";
  z[lc++]="   if (NV(%1$s_t) > NV(%1$s_cdur))";
  z[lc++]="   {";
  z[lc++]="    while (NV(%1$s_t) > NV(%1$s_cdur))";
  z[lc++]="     {";
  z[lc++]="       NV(%1$s_t) -= NV(%1$s_cdur);";
  z[lc++]="       switch(NVI(%1$s_first))\n      {";

  while (xptr != NULL)
    {
      mz(lc); sprintf(z[lc++],"     case %i:",i++);
      mz(lc); sprintf(z[lc++],"     NV(%s_cdur) = va_dur%i;",
	      currinstancename,i);
      if (isocheck)
	{
	  z[lc++]="     if (NV(%1$s_cdur) < 0.0F)";
	  genex(&lc,tptr->optr->down,"A duration < 0");
	}

      z[lc++]="       NV(%1$s_clp) = NV(%1$s_crp);";
      mz(lc); sprintf(z[lc++],"       NV(%s_crp) = va_x%i;",
	      currinstancename,i+1);
      if (isocheck)
	{
	  z[lc++]="       if (NV(%1$s_crp) == 0.0F)";
	  genex(&lc,tptr->optr->down,"An xval == 0");
	  z[lc++]="       if ((NV(%1$s_crp) > 0.0F) != (NV(%1$s_clp) > 0.0F))";
	  genex(&lc,tptr->optr->down,"Mix of + and - xval signs");
	}
      z[lc++]="       break;";
      xptr = xptr->next->next->next;
      if (xptr != NULL)
	xptr = xptr->next;
    }
  z[lc++]="       default:";
  z[lc++]="       NVI(%1$s_first) = -100;"; 
  z[lc++]="       NV(%1$s_cdur) = NV(%1$s_t) + 10000.0F;";
  z[lc++]="       break;";
  z[lc++]="       }";
  z[lc++]="      NVI(%1$s_first)++;";
  z[lc++]="     }";
  z[lc++]="     NV(%1$s_ratio) = NV(%1$s_crp)/NV(%1$s_clp);";
  z[lc++]="     NV(%1$s_invcdur) = 1.0F/NV(%1$s_cdur);";
  z[lc++]="     ret = NV(%1$s_outT) = NV(%1$s_clp)*(float)pow(NV(%1$s_ratio),NV(%1$s_invcdur)*NV(%1$s_t));";
  z[lc++]="     NV(%1$s_multK) = (float)pow(NV(%1$s_ratio),NV(%1$s_invcdur)*EV(KTIME));";
  z[lc++]="     if (NVI(%1$s_first) < 0)";
  z[lc++]="        ret = 0.0F;";
  z[lc++]="   }";
  z[lc++]="}";
  z[lc++]="if (NVI(%1$s_first)==0)";
  z[lc++]="  {";
  z[lc++]="    NVI(%1$s_first) = 1;";
  z[lc++]="    ret = NV(%1$s_outT) = NV(%1$s_clp) = x1;";
  z[lc++]="    NV(%1$s_crp) = x2;";
  z[lc++]="    NV(%1$s_cdur) = dur1;";

  if (isocheck)
    {
      z[lc++]="    if (dur1 < 0.0F)";
      genex(&lc,tptr->optr->down,"A duration < 0");
      z[lc++]="    if ((x1 == 0.0F)||(x2 == 0.0F))";
      genex(&lc,tptr->optr->down,"An xval == 0");
      z[lc++]="    if ((x1 > 0.0F) != (x2 > 0.0F))";
      genex(&lc,tptr->optr->down,"Mix of + and - xval signs");
    }

  z[lc++]="    NV(%1$s_ratio) = x2/x1;";
  z[lc++]="    if (dur1 > 0.0F)";
  z[lc++]="      NV(%1$s_invcdur) = 1.0F/dur1;";
  z[lc++]="    NV(%1$s_multK) = (float)pow(NV(%1$s_ratio),NV(%1$s_invcdur)*EV(KTIME));";
  z[lc++]="  }";
  printblock(lc);

}


/*********************************************************/
/*         sets oscil flag in grain in correct place     */
/*********************************************************/

void grainoscilflag(tnode * tptr, int * lcptr, int ratetype)

{
  int lc = *lcptr;
  tnode * aptr;

  aptr = firstopcodearg(tptr); /* first arg, table t*/

  /* easiest to cull out places where code doesn't go */

  if ((ratetype == KRATETYPE) && (aptr->down->vartype == TMAPTYPE) &&
      (aptr->down->next->next->rate == ARATETYPE))
    return;

  if ((ratetype == ARATETYPE) && (aptr->down->vartype == TABLETYPE))
    return;

  if ((ratetype == ARATETYPE) && (aptr->down->vartype == TMAPTYPE) &&
      (aptr->down->next->next->rate != ARATETYPE))
    return;
  
  z[lc++]= "  if (AP1.sr)";
  z[lc++]= "   {";
  z[lc++]= "    if ((NVI(%1$s_oscil) = !(AP1.base)) && ";
  z[lc++]="         (AP1.start >= AP1.tend)) ";
  genex(&lc, tptr->optr->down,"End loop point < start loop point");
  z[lc++]= "   }";
  z[lc++]= "  else";
  z[lc++]= "    NVI(%1$s_oscil) = 2;";

  *lcptr = lc;			  
}


/*********************************************************/
/*         code for grain opcode                         */
/*********************************************************/

void graincode(tnode * tptr)

{
  int lc = 0;

  z[lc++]= "if (NVI(%1$s_kcyc) != EV(kcycleidx))";
  z[lc++]= "{";
  z[lc++]= "  if (!(NVI(%1$s_kcyc)))";
  z[lc++]= "  {";
  z[lc++]= "   i = NT(TBL_%1$s_state).len = GRNUM*GRSTATE;";
  z[lc++]= "   NT(TBL_%1$s_state).start = 0;";
  z[lc++]= "   NT(TBL_%1$s_state).end = 0;";
  z[lc++]= "   NT(TBL_%1$s_state).t = (float *) malloc(i*sizeof(float)); ";
  z[lc++]= "   NT(TBL_%1$s_state).llmem = 1; ";
  z[lc++]= "   while (i > 0) ";
  z[lc++]= "    NT(TBL_%1$s_state).t[--i] = 0.0F;";
  z[lc++]= "   NV(%1$s_dclock)=0.0F;";
  z[lc++]= "   NV(%1$s_tclock)=0.0F;";
  z[lc++]= "  }";
  z[lc++]= "  NVI(%1$s_kcyc) = EV(kcycleidx);";
  z[lc++]= "  if (density <= 0.0F)";
  genex(&lc,tptr->optr->down,"Density <= 0");
  z[lc++]= "  if (dur < 0.0F)";
  genex(&lc,tptr->optr->down,"Duration < 0");
  z[lc++]= "  if (time < 0.0F)";
  genex(&lc,tptr->optr->down,"Time < 0");
  z[lc++]= "  if ((phase < 0.0F) || (phase > 1.0F))";
  genex(&lc,tptr->optr->down,"Phase outside [0,1]");
  z[lc++]= "  NV(%1$s_invdens)= 1.0F/density;";
  grainoscilflag(tptr, &lc, KRATETYPE);
  z[lc++]= "  switch (NVI(%1$s_oscil)) {";
  z[lc++]= "    case GRLOSCIL:";
  z[lc++]= "      NV(%1$s_lconst)= AP1.sr*EV(ATIME)/AP1.base;";
  z[lc++]= "      break;";
  z[lc++]= "    case GRDOSCIL:";
  z[lc++]= "      NV(%1$s_dconst)= AP1.sr*EV(ATIME)/AP1.lenf;";
  z[lc++]= "      break;";
  z[lc++]= "    case GROSCIL:";
  z[lc++]= "      break;";
  z[lc++]= "   }";
  z[lc++]= "}";
  z[lc++]= "if ((NV(%1$s_dclock)+= EV(ATIME)) >= NV(%1$s_invdens))";
  z[lc++]= " {";
  z[lc++]= "   NV(%1$s_dclock) = 0.0F;";
  z[lc++]= "   if (time < NV(%1$s_invdens))";
  z[lc++]= "     NV(%1$s_tclock) = time;";
  z[lc++]= " }";
  z[lc++]= "if (NV(%1$s_tclock) >= 0.0F)";
  z[lc++]= " {";
  z[lc++]= "   if ((NV(%1$s_tclock)-= EV(ATIME))<=0.0F)";
  z[lc++]= "    {";
  z[lc++]= "      NV(%1$s_tclock)= -1.0F;";
  z[lc++]= "      if (dur > 0.0F)";
  z[lc++]= "      {";
  z[lc++]= "        i = NT(TBL_%1$s_state).end;";
  z[lc++]= "        NT(TBL_%1$s_state).t[i+GRTIME] = 0.0F;";
  z[lc++]= "        NT(TBL_%1$s_state).t[i+GRPHASE] = phase;";
  z[lc++]= "        NT(TBL_%1$s_state).t[i+GRFREQ] = freq;";
  z[lc++]= "        NT(TBL_%1$s_state).t[i+GRAMP] = amp;";
  z[lc++]= "        NT(TBL_%1$s_state).t[i+GRDUR] = dur;";
  z[lc++]= "        NT(TBL_%1$s_state).t[i+GRINVDUR] = 1.0F/dur;";
  z[lc++]= "        if ((i += GRSTATE) == NT(TBL_%1$s_state).len)";
  z[lc++]= "          i = 0;";
  z[lc++]= "        NT(TBL_%1$s_state).end = i;";
  z[lc++]= "        if (i == NT(TBL_%1$s_state).start)";
  z[lc++]= "         {";
  z[lc++]= "           NT(TBL_%1$s_state).t[i+GRTIME] = 0.0F;";
  z[lc++]= "           if ((i += GRSTATE) == NT(TBL_%1$s_state).len)";
  z[lc++]= "             i = 0;";
  z[lc++]= "           NT(TBL_%1$s_state).start = i;";
  z[lc++]= "         }";
  z[lc++]= "      }";
  z[lc++]= "    }";
  z[lc++]= " }";
  z[lc++]= " j = NT(TBL_%1$s_state).start;";
  z[lc++]= " out = 0.0F;";
  grainoscilflag(tptr, &lc, ARATETYPE);
  z[lc++]= " while (j != NT(TBL_%1$s_state).end)";
  z[lc++]= "  {";
  z[lc++]= "    state = &(NT(TBL_%1$s_state).t[j]);";
  z[lc++]= "    if ((state[GRTIME] += EV(ATIME)) <= state[GRDUR])";
  z[lc++]= "     {";
  z[lc++]= "       i = (int)(index = state[GRTIME]*state[GRINVDUR]";
  z[lc++]= "                          *AP2.lenf);";
  z[lc++]= "       if (i < (AP2.len - 1))";
  z[lc++]= "         sc = state[GRAMP]*";
  z[lc++]= "               (AP2.t[i]+(index-i)*(AP2.t[i+1]-AP2.t[i]));";
  z[lc++]= "       else";
  z[lc++]= "         sc = state[GRAMP]*";
  z[lc++]= "               (AP2.t[i]-(index - i)*AP2.t[AP2.len-1]);";
  z[lc++]= "       switch (NVI(%1$s_oscil)) {";
  z[lc++]= "       case GROSCIL:";
  z[lc++]= "         i = (int)(index = state[GRPHASE]*AP1.lenf);";
  z[lc++]= "         ret = AP1.t[i] + (index-i)*(AP1.t[i+1]-AP1.t[i]);";
  z[lc++]= "         if ((state[GRPHASE] += EV(ATIME)*state[GRFREQ])>1.0F)";
  z[lc++]= "           state[GRPHASE] -= (int)state[GRPHASE];";
  z[lc++]= "         if (state[GRPHASE] < 0.0F)";
  z[lc++]= "           state[GRPHASE] += 1 - (int)state[GRPHASE];";
  z[lc++]= "         break;";
  z[lc++]= "       case GRLOSCIL:";
  z[lc++]= "         i = (int)(index = AP1.start +";
  z[lc++]= "             (AP1.tend - AP1.start)*state[GRPHASE]);";
  z[lc++]= "         ret = AP1.t[i] + (index-i)*(AP1.t[i+1]-AP1.t[i]);";
  z[lc++]= "         if ((state[GRPHASE] += NV(%1$s_lconst)*state[GRFREQ])";
  z[lc++]= "               > 1.0F)";
  z[lc++]= "           state[GRPHASE] -= (int)state[GRPHASE];";
  z[lc++]= "         if (state[GRPHASE] < 0.0F)";
  z[lc++]= "           state[GRPHASE] += 1 - (int)state[GRPHASE];";
  z[lc++]= "         break;";
  z[lc++]= "       case GRDOSCIL:";
  z[lc++]= "        i = (int) (index = state[GRPHASE]*AP1.lenf);";
  z[lc++]= "        if (i < (AP1.len - 1))";
  z[lc++]= "          ret = AP1.t[i]+(index-i)*(AP1.t[i+1]-AP1.t[i]);";
  z[lc++]= "        else";
  z[lc++]= "          ret = AP1.t[i] - (index - i)*AP1.t[AP1.len-1];";
  z[lc++]= "        if ((state[GRPHASE] += NV(%1$s_dconst)) > 1.0F)";
  z[lc++]= "            state[GRTIME] = state[GRDUR];";
  z[lc++]= "         break;";
  z[lc++]= "       }";
  z[lc++]= "       out += sc*ret;";
  z[lc++]= "       if ((j += GRSTATE) == NT(TBL_%1$s_state).len)";
  z[lc++]= "         j = 0;";
  z[lc++]= "     }";
  z[lc++]= "    else";
  z[lc++]= "     {";
  z[lc++]= "      if (j == NT(TBL_%1$s_state).start)";
  z[lc++]= "       {";
  z[lc++]= "         while ((NT(TBL_%1$s_state).t[j+GRTIME] >";
  z[lc++]= "                 NT(TBL_%1$s_state).t[j+GRDUR])&&";
  z[lc++]= "                 (j != NT(TBL_%1$s_state).end))";
  z[lc++]= "          if ((j += GRSTATE) == NT(TBL_%1$s_state).len)";
  z[lc++]= "             j  = 0;";
  z[lc++]= "          NT(TBL_%1$s_state).start = j;";
  z[lc++]= "       }";
  z[lc++]= "      else";
  z[lc++]= "       {";
  z[lc++]= "          if ((j += GRSTATE) == NT(TBL_%1$s_state).len)";
  z[lc++]= "            j = 0;";
  z[lc++]= "       }";
  z[lc++]= "     }";
  z[lc++]= "  }";
  z[lc++]= "ret = out;";
  printblock(lc);

}


/*********************************************************/
/* simple filters, const cutoff                           */
/*********************************************************/

void filterconst(tnode * tptr, int ttype)

{
  int lc = 0;
  double cut, cf, bw, e, c, d, b0, b1, b2, a1, a2;
  tnode * cptr = firstopcodearg(tptr); 
  
  cut = cf = bw = e = c = d = b0 = b1 = b2 = a1 = a2 = 0.0;

  switch (ttype) {
  case S_BANDPASS:
  case S_BANDSTOP:
    cf = atof(cptr->next->next->down->val);
    bw = atof(cptr->next->next->next->next->down->val);
    break;
  case S_LOPASS:
  case S_HIPASS:
    cut = atof(cptr->next->next->down->val);
    break;
  }

  switch (ttype) {
  case S_BANDPASS:
    e = 3.141593*bw/srate;
    c = 0.0031416;
    if (e < 3.27249e-05)
      c = 30557.8;
    else
      if (e < 1.56765)
	c = 1.0/tan(e);
    e = 6.283185*cf/srate;
    d = -1.996053;
    if (e < 6.283185e-03)
      d = 1.999961;
    else
      if (e < 3.078761)
	d = 2.0*cos(e);
    b0= 1.0/(1.0 + c);
    b1= 0.0;
    b2= -b0;
    a1= -c*d*b0;
    a2= (c - 1.0)*b0;
    break;
  case S_BANDSTOP:
    e = 3.141593*bw/srate;
    c = 318.309;
    if (e < 3.27249e-05)
      c = 3.27249e-05;
    else
      if (e < 1.56765)
	c = tan(e);
    e = 6.283185*cf/srate;
    d = -1.996053;
    if (e < 6.283185e-03)
      d = 1.999961;
    else
      if (e < 3.078761)
	d = 2.0*cos(e);
    b0= 1.0/(1.0 + c);
    b1= -d*b0;
    b2= b0;
    a1= b1;
    a2= (1.0 - c)*b0;
    break;
  case S_LOPASS:
    e = 3.141593*cut/srate;
    c = 0.0031416;
    if (e < 3.27249e-05)
      c = 30557.8;
    else
      if (e < 1.56765)
	c = 1.0/tan(e);
    b0= 1.0/(1.0 + 1.414214*c + c*c);
    b1= 2.0*b0;
    b2 = b0;
    a1 = 2.0*b0*(1.0 - c*c);
    a2 = b0*(1.0 - 1.414214*c + c*c);
    break;
  case S_HIPASS:
    e = 3.141593*cut/srate;
    c = 318.309;
    if (e < 3.27249e-05)
      c = 3.27249e-05;
    else
      if (e < 1.56765)
	c = tan(e);
    b0= 1.0/(1.0 + 1.414214*c + c*c);
    b1= -2.0*b0;
    b2= b0;
    a1= 2.0*b0*(c*c - 1.0);
    a2= b0*(1.0 - 1.414214*c + c*c);
    break;
  }


  mz(lc);
  sprintf(z[lc++], "  ret = (%10eF)*input  ",b0);
  z[lc++]= "    + NV(%1$s_d2);";

  z[lc++]= "   NV(%1$s_d2) = NV(%1$s_d1)";

  mz(lc);
  sprintf(z[lc++], "-(%10eF)*ret+(%10eF)*input;   ",a1, b1);

  z[lc++]= "   NV(%1$s_d1) = ";

  mz(lc);
  sprintf(z[lc++], "-(%10eF)*ret+(%10eF)*input;   ",a2, b2);

  printblock(lc);
  return;

}


/*********************************************************/
/* lopass filter, default case (krate cut)               */
/*********************************************************/

void lopassk(tnode * tptr)

{
  int lc = 0;

  acycleguard(tptr, &lc);
  z[lc++]= " {";
  z[lc++]= "   ret = NV(%1$s_b0)*input + NV(%1$s_d2);";
  z[lc++]= "   NV(%1$s_d2) = NV(%1$s_d1)-NV(%1$s_a1)*ret+NV(%1$s_b1)*input;";
  z[lc++]= "   NV(%1$s_d1) =            -NV(%1$s_a2)*ret+NV(%1$s_b2)*input;";
  z[lc++]= " }";
  z[lc++]= "else";
  z[lc++]= " {";
  z[lc++]= "  if ((NVI(%1$s_kcyc)==0)||(NV(%1$s_ocut)!=cut))";
  z[lc++]= "   {";
  kcycassign(tptr,&lc);
  z[lc++]= "    NV(%1$s_ocut) = cut;";
  z[lc++]= "    e = 3.141593F*cut*EV(ATIME);";
  z[lc++]= "    c = 0.0031416F;";
  z[lc++]= "    if (e < 3.27249e-05F)";
  z[lc++]= "      c = 30557.8F;";
  z[lc++]= "    else";
  z[lc++]= "     if (e < 1.56765F)";
  z[lc++]= "       c = 1.0F/(float)tan(e);";
  z[lc++]= "    NV(%1$s_b0)= 1.0F/(1.0F + 1.414214F*c + c*c);";
  z[lc++]= "    NV(%1$s_b1)= 2.0F*NV(%1$s_b0);";
  z[lc++]= "    NV(%1$s_b2)= NV(%1$s_b0);";
  z[lc++]= "    NV(%1$s_a1)= 2.0F*NV(%1$s_b0)*(1.0F - c*c);";
  z[lc++]= "    NV(%1$s_a2)= NV(%1$s_b0)*(1.0F - 1.414214F*c + c*c);";
  z[lc++]= "   }";
  kcycassign2(tptr,&lc);
  z[lc++]= "  ret = NV(%1$s_b0)*input + NV(%1$s_d2);";
  z[lc++]= "  NV(%1$s_d2) = NV(%1$s_d1)-NV(%1$s_a1)*ret+NV(%1$s_b1)*input;";
  z[lc++]= "  NV(%1$s_d1) =            -NV(%1$s_a2)*ret+NV(%1$s_b2)*input;";
  z[lc++]= " }";
  printblock(lc);
  return;

}


/*********************************************************/
/* lopass filter                                         */
/*********************************************************/

void lopass(tnode * tptr)

{
  tnode * cptr = firstopcodearg(tptr); 

  cptr = cptr->next->next;
  if ((cptr->vol == CONSTANT) && !reentrant)
    filterconst(tptr, S_LOPASS);
  else
    lopassk(tptr);
  return;

}


/*********************************************************/
/* hipass filter, default case (krate cut)               */
/*********************************************************/

void hipassk(tnode * tptr)

{
  int lc = 0;

  acycleguard(tptr, &lc);
  z[lc++]= " {";
  z[lc++]= "   ret = NV(%1$s_b0)*input + NV(%1$s_d2);";
  z[lc++]= "   NV(%1$s_d2) = NV(%1$s_d1)-NV(%1$s_a1)*ret+NV(%1$s_b1)*input;";
  z[lc++]= "   NV(%1$s_d1) =            -NV(%1$s_a2)*ret+NV(%1$s_b2)*input;";
  z[lc++]= " }";
  z[lc++]= "else";
  z[lc++]= " {";
  z[lc++]= "  if ((NVI(%1$s_kcyc)==0)||(NV(%1$s_ocut)!=cut))";
  z[lc++]= "   {";
  kcycassign(tptr,&lc);
  z[lc++]= "     NV(%1$s_ocut) = cut;";
  z[lc++]= "     e = 3.141593F*cut*EV(ATIME);";
  z[lc++]= "     c = 318.309F;";
  z[lc++]= "     if (e < 3.27249e-05F)";
  z[lc++]= "       c = 3.27249e-05F;";
  z[lc++]= "     else";
  z[lc++]= "      if (e < 1.56765F)";
  z[lc++]= "        c = (float)tan(e);";
  z[lc++]= "     NV(%1$s_b0)= 1.0F/(1.0F + 1.414214F*c + c*c);";
  z[lc++]= "     NV(%1$s_b1)= -2.0F*NV(%1$s_b0);";
  z[lc++]= "     NV(%1$s_b2)= NV(%1$s_b0);";
  z[lc++]= "     NV(%1$s_a1)= 2.0F*NV(%1$s_b0)*(c*c - 1.0F);";
  z[lc++]= "     NV(%1$s_a2)= NV(%1$s_b0)*(1.0F - 1.414214F*c + c*c);";
  z[lc++]= "   }";
  kcycassign2(tptr,&lc);
  z[lc++]= "  ret = NV(%1$s_b0)*input + NV(%1$s_d2);";
  z[lc++]= "  NV(%1$s_d2) = NV(%1$s_d1)-NV(%1$s_a1)*ret+NV(%1$s_b1)*input;";
  z[lc++]= "  NV(%1$s_d1) =            -NV(%1$s_a2)*ret+NV(%1$s_b2)*input;";
  z[lc++]= " }";
  printblock(lc);
  return;

}


/*********************************************************/
/* hipass filter                                         */
/*********************************************************/

void hipass(tnode * tptr)

{
  tnode * cptr = firstopcodearg(tptr); 

  cptr = cptr->next->next;

  if ((cptr->vol == CONSTANT) && !reentrant)
    filterconst(tptr, S_HIPASS);
  else
    hipassk(tptr);
  return;

}

/*********************************************************/
/* bandpass filter, default case (krate cut)               */
/*********************************************************/

void bandpassk(tnode * tptr)

{
  int lc = 0;

  acycleguard(tptr, &lc);
  z[lc++]= " {";
  z[lc++]= "   ret = NV(%1$s_b0)*input + NV(%1$s_d2);";
  z[lc++]= "   NV(%1$s_d2) = NV(%1$s_d1)-NV(%1$s_a1)*ret+NV(%1$s_b1)*input;";
  z[lc++]= "   NV(%1$s_d1) =            -NV(%1$s_a2)*ret+NV(%1$s_b2)*input;";
  z[lc++]= " }";
  z[lc++]= "else";
  z[lc++]= " {";
  z[lc++]= "  if ((NVI(%1$s_kcyc)==0)||(NV(%1$s_obw)!=bw)||";
  z[lc++]= "      (NV(%1$s_ocf)!=cf))";
  z[lc++]= "   {";
  kcycassign(tptr,&lc);
  z[lc++]= "    NV(%1$s_obw) = bw;";
  z[lc++]= "    NV(%1$s_ocf) = cf;";
  z[lc++]= "    e = 3.141593F*bw*EV(ATIME);";
  z[lc++]= "    c = 0.0031416F;";
  z[lc++]= "    if (e < 3.27249e-05F)";
  z[lc++]= "      c = 30557.8F;";
  z[lc++]= "    else";
  z[lc++]= "     if (e < 1.56765F)";
  z[lc++]= "       c = 1.0F/(float)tan(e);";
  z[lc++]= "    e = 6.283185F*cf*EV(ATIME);";
  z[lc++]= "    d = -1.996053F;";
  z[lc++]= "    if (e < 6.283185e-03F)";
  z[lc++]= "      d = 1.999961F;";
  z[lc++]= "    else";
  z[lc++]= "      if (e < 3.078761F)";
  z[lc++]= "        d = 2.0F*(float)cos(e);";
  z[lc++]= "    NV(%1$s_b0)= 1.0F/(1.0F + c);";
  z[lc++]= "    NV(%1$s_b1)= 0.0F;";
  z[lc++]= "    NV(%1$s_b2)= -NV(%1$s_b0);";
  z[lc++]= "    NV(%1$s_a1)= -c*d*NV(%1$s_b0);";
  z[lc++]= "    NV(%1$s_a2)= (c - 1.0F)*NV(%1$s_b0);";
  z[lc++]= "   }";
  kcycassign2(tptr,&lc);
  z[lc++]= "  ret = NV(%1$s_b0)*input + NV(%1$s_d2);";
  z[lc++]= "  NV(%1$s_d2) = NV(%1$s_d1)-NV(%1$s_a1)*ret+NV(%1$s_b1)*input;";
  z[lc++]= "  NV(%1$s_d1) =            -NV(%1$s_a2)*ret+NV(%1$s_b2)*input;";
  z[lc++]= " }";
  printblock(lc);
  return;

}


/*********************************************************/
/* bandpass filter                                         */
/*********************************************************/

void bandpass(tnode * tptr)

{
  tnode * cptr = firstopcodearg(tptr); 

  cptr = cptr->next->next;

  if ((cptr->vol == CONSTANT) &&
      (cptr->next->next->vol == CONSTANT) && !reentrant)
    filterconst(tptr, S_BANDPASS);
  else
    bandpassk(tptr);
  return;

}




/*********************************************************/
/* bandstop filter, default case (krate cut)               */
/*********************************************************/

void bandstopk(tnode * tptr)

{
  int lc = 0;

  acycleguard(tptr, &lc);
  z[lc++]= " {";
  z[lc++]= "   ret = NV(%1$s_b0)*input + NV(%1$s_d2);";
  z[lc++]= "   NV(%1$s_d2) = NV(%1$s_d1)-NV(%1$s_a1)*ret+NV(%1$s_b1)*input;";
  z[lc++]= "   NV(%1$s_d1) =            -NV(%1$s_a2)*ret+NV(%1$s_b2)*input;";
  z[lc++]= " }";
  z[lc++]= "else";
  z[lc++]= " {";
  z[lc++]= "  if ((NVI(%1$s_kcyc)==0)||(NV(%1$s_obw)!=bw)||";
  z[lc++]= "      (NV(%1$s_ocf)!=cf))";
  z[lc++]= "   {";
  kcycassign(tptr,&lc);
  z[lc++]= "    NV(%1$s_obw) = bw;";
  z[lc++]= "    NV(%1$s_ocf) = cf;";
  z[lc++]= "    e = 3.141593F*bw*EV(ATIME);";
  z[lc++]= "    c = 318.309F;";
  z[lc++]= "    if (e < 3.27249e-05F)";
  z[lc++]= "      c = 3.27249e-05F;";
  z[lc++]= "    else";
  z[lc++]= "     if (e < 1.56765F)";
  z[lc++]= "       c = (float)tan(e);";
  z[lc++]= "    e = 6.283185F*cf*EV(ATIME);";
  z[lc++]= "    d = -1.996053F;";
  z[lc++]= "    if (e < 6.283185e-03F)";
  z[lc++]= "      d = 1.999961F;";
  z[lc++]= "    else";
  z[lc++]= "     if (e < 3.078761F)";
  z[lc++]= "      d = 2.0F*(float)cos(e);";
  z[lc++]= "    NV(%1$s_b0)= 1.0F/(1.0F + c);";
  z[lc++]= "    NV(%1$s_b1)= -d*NV(%1$s_b0);";
  z[lc++]= "    NV(%1$s_b2)= NV(%1$s_b0);";
  z[lc++]= "    NV(%1$s_a1)= NV(%1$s_b1);";
  z[lc++]= "    NV(%1$s_a2)= (1.0F - c)*NV(%1$s_b0);";
  z[lc++]= "   }";
  kcycassign2(tptr,&lc);
  z[lc++]= "  ret = NV(%1$s_b0)*input + NV(%1$s_d2);";
  z[lc++]= "  NV(%1$s_d2) = NV(%1$s_d1)-NV(%1$s_a1)*ret+NV(%1$s_b1)*input;";
  z[lc++]= "  NV(%1$s_d1) =            -NV(%1$s_a2)*ret+NV(%1$s_b2)*input;";
  z[lc++]= " }";
  printblock(lc);
  return;

}


/*********************************************************/
/* bandstop filter                                         */
/*********************************************************/

void bandstop(tnode * tptr)

{
  tnode * cptr = firstopcodearg(tptr); 

  cptr = cptr->next->next;

  if ((cptr->vol == CONSTANT) &&
      (cptr->next->next->vol == CONSTANT) && !reentrant)
    filterconst(tptr, S_BANDSTOP);
  else
    bandstopk(tptr);
  return;

}

/*********************************************************/
/* portamento filter                                     */
/*********************************************************/

void portamento(tnode * tptr)

{
  int lc = 0;

  z[lc++]="if (NVI(%1$s_first))";
  z[lc++]="{";
  z[lc++]="  if (!NVI(%1$s_done) || (NV(%1$s_new) != ctrl))";
  z[lc++]="   {";
  z[lc++]="     if (htime != NV(%1$s_ohtime))";
  z[lc++]="      {";
  z[lc++]="        NV(%1$s_ohtime) = htime;";
  z[lc++]="        if (htime != 0.0F)";
  z[lc++]="        {";
  z[lc++]="          NV(%1$s_sl)=(float)pow(2.0F,-EV(KTIME)/htime);";
  z[lc++]="          NV(%1$s_int)=NV(%1$s_new)*(1.0F-NV(%1$s_sl));";
  z[lc++]="        }";
  z[lc++]="      }";
  z[lc++]="    if ((NV(%1$s_new) != ctrl)||(htime == 0.0F))";
  z[lc++]="     {";
  z[lc++]="       NV(%1$s_new)=ctrl;";
  z[lc++]="       if (htime == 0.0F)";
  z[lc++]="         NV(%1$s_curr) = ctrl;";
  z[lc++]="       else";
  z[lc++]="        {";
  z[lc++]="         NVI(%1$s_done) = 0;";
  z[lc++]="         NV(%1$s_int)=NV(%1$s_new)*(1.0F-NV(%1$s_sl));";
  z[lc++]="        }";
  z[lc++]="     }";
  z[lc++]="    diff = NV(%1$s_new)-NV(%1$s_curr);";
  z[lc++]="    if ( (diff > 1e-10F) || (diff < -1e-10F))";
  z[lc++]="      NV(%1$s_curr)=NV(%1$s_int)+NV(%1$s_curr)*NV(%1$s_sl);";
  z[lc++]="    else" ;
  z[lc++]="     {";
  z[lc++]="       NV(%1$s_curr) = NV(%1$s_new);";
  z[lc++]="       NVI(%1$s_done) = 1;";
  z[lc++]="     }";
  z[lc++]="  }";
  z[lc++]="}";
  z[lc++]="else ";
  z[lc++]="{";
  z[lc++]="  NVI(%1$s_done) = NVI(%1$s_first) = 1;";
  z[lc++]="  NV(%1$s_new)=NV(%1$s_curr)=ctrl;";
  z[lc++]="  NV(%1$s_ohtime)=0.0F;";
  z[lc++]="}";
  z[lc++]="ret = NV(%1$s_curr);";
  printblock(lc);

}


/*********************************************************/
/* firt filter -- no order parameter                     */
/*********************************************************/

void firtordernone(tnode * tptr)

{
  int lc = 0;

  z[lc++]= "if (NVI(%1$s_kcyc))";
  z[lc++]= "{";
  z[lc++]= " ret = AP1.t[0]*input;";
  z[lc++]= " j = NT(TBL_%1$s_dline).len;";
  z[lc++]= " if (j>0)";
  z[lc++]= " {";
  z[lc++]= "  i = NT(TBL_%1$s_dline).tend;";
  z[lc++]= "  while (j>0)";
  z[lc++]= "   {";
  z[lc++]= "     ret += AP1.t[j]*NT(TBL_%1$s_dline).t[i];";
  z[lc++]= "     i--;j--;";
  z[lc++]= "     if (i<0)";
  z[lc++]= "       i = AP1.len-2;";
  z[lc++]= "   }";
  z[lc++]= "  NT(TBL_%1$s_dline).t[NT(TBL_%1$s_dline).tend--]= input;";
  z[lc++]= "  if (NT(TBL_%1$s_dline).tend<0)";
  z[lc++]= "   NT(TBL_%1$s_dline).tend=AP1.len-2;";
  z[lc++]= " }";
  z[lc++]= "}";
  z[lc++]= "else";
  z[lc++]= "{";
  z[lc++]= "  NT(TBL_%1$s_dline).len = AP1.len-1;";
  z[lc++]= "  NT(TBL_%1$s_dline).tend = 0;";
  z[lc++]= "  if (AP1.len > 1)";
  z[lc++]= "   {";
  z[lc++]= "    NT(TBL_%1$s_dline).t=(float*)calloc(AP1.len-1,sizeof(float));";
  z[lc++]= "    NT(TBL_%1$s_dline).llmem=1;";
  z[lc++]= "    NT(TBL_%1$s_dline).t[0]= input;";
  z[lc++]= "    NT(TBL_%1$s_dline).tend = AP1.len-2;";
  z[lc++]= "   }";
  z[lc++]= "  ret = AP1.t[0]*input;";
  z[lc++]= "  NVI(%1$s_kcyc) = 1;";
  z[lc++]= "}";
  printblock(lc);

}

/*********************************************************/
/* firt filter -- irate order parameter                  */
/*********************************************************/

void firtorderirate(tnode * tptr)

{
  int lc = 0;

  z[lc++]= "if (NVI(%1$s_kcyc))";
  z[lc++]= "{";
  z[lc++]= " ret = AP1.t[0]*input;";
  z[lc++]= " j = NT(TBL_%1$s_dline).len;";
  z[lc++]= " if (j>0)";
  z[lc++]= " {";
  z[lc++]= "  i = NT(TBL_%1$s_dline).tend;";
  z[lc++]= "  while (j>0)";
  z[lc++]= "   {";
  z[lc++]= "     ret += AP1.t[j]*NT(TBL_%1$s_dline).t[i];";
  z[lc++]= "     i--;j--;";
  z[lc++]= "     if (i<0)";
  z[lc++]= "       i = AP1.len-2;";
  z[lc++]= "   }";
  z[lc++]= "  NT(TBL_%1$s_dline).t[NT(TBL_%1$s_dline).tend--]= input;";
  z[lc++]= "  if (NT(TBL_%1$s_dline).tend<0)";
  z[lc++]= "   NT(TBL_%1$s_dline).tend=AP1.len-2;";
  z[lc++]= " }";
  z[lc++]= "}";
  z[lc++]= "else";
  z[lc++]= "{";
  z[lc++]= "  j = (int) (va_order + 0.5F);";
  z[lc++]= "  if (j <= 0)";
  genex(&lc,tptr->optr->down,"Order <= 0");
  z[lc++]= "  j = (j > AP1.len) ? AP1.len : j;";
  z[lc++]= "  NT(TBL_%1$s_dline).len = j-1;";
  z[lc++]= "  NT(TBL_%1$s_dline).tend = 0;";
  z[lc++]= "  if (j > 1)";
  z[lc++]= "   {";
  z[lc++]= "    NT(TBL_%1$s_dline).t=(float*)calloc(j-1,sizeof(float));";
  z[lc++]= "    NT(TBL_%1$s_dline).llmem=1;";
  z[lc++]= "    NT(TBL_%1$s_dline).t[0]= input;";
  z[lc++]= "    NT(TBL_%1$s_dline).tend = j-2;";
  z[lc++]= "   }";
  z[lc++]= "  ret = AP1.t[0]*input;";
  z[lc++]= "  NVI(%1$s_kcyc) = 1;";
  z[lc++]= "}";
  printblock(lc);
  
}


/*********************************************************/
/* firt filter -- krate order parameter                  */
/*********************************************************/

void firtorderkrate(tnode * tptr)

{
  int lc = 0;

  z[lc++]= "if (NVI(%1$s_kcyc))";
  z[lc++]= "{";
  z[lc++]= "  if (NVI(%1$s_kcyc) != EV(kcycleidx))";
  z[lc++]= "  {";
  z[lc++]= "   NVI(%1$s_kcyc) = EV(kcycleidx);";
  z[lc++]= "   j = (int) (va_order + 0.5F);";
  z[lc++]= "   j = ((j <= 0)||(j > AP1.len)) ? AP1.len : j;";
  z[lc++]= "   if (NT(TBL_%1$s_dline).len != (j-1))";
  z[lc++]= "    {";
  z[lc++]= "      i = NT(TBL_%1$s_dline).len = j-1;";
  z[lc++]= "      NT(TBL_%1$s_dline).tend = 0;";
  z[lc++]= "      while (i > 0)";
  z[lc++]= "        NT(TBL_%1$s_dline).t[--i] = 0.0F;";
  z[lc++]= "    }";
  z[lc++]= " }";
  z[lc++]= " ret = AP1.t[0]*input;";
  z[lc++]= " j = NT(TBL_%1$s_dline).len;";
  z[lc++]= " if (j>0)";
  z[lc++]= " {";
  z[lc++]= "  i = NT(TBL_%1$s_dline).tend;";
  z[lc++]= "  while (j>0)";
  z[lc++]= "   {";
  z[lc++]= "     ret += AP1.t[j]*NT(TBL_%1$s_dline).t[i];";
  z[lc++]= "     i--;j--;";
  z[lc++]= "     if (i<0)";
  z[lc++]= "       i = AP1.len-2;";
  z[lc++]= "   }";
  z[lc++]= "  NT(TBL_%1$s_dline).t[NT(TBL_%1$s_dline).tend--]= input;";
  z[lc++]= "  if (NT(TBL_%1$s_dline).tend<0)";
  z[lc++]= "   NT(TBL_%1$s_dline).tend=AP1.len-2;";
  z[lc++]= " }";
  z[lc++]= "}";
  z[lc++]= "else";
  z[lc++]= "{";
  z[lc++]= "  j = (int) (va_order + 0.5F);";
  z[lc++]= "  j = ((j <= 0)||(j > AP1.len)) ? AP1.len : j;";
  z[lc++]= "  NT(TBL_%1$s_dline).len = j-1;";
  z[lc++]= "  NT(TBL_%1$s_dline).tend = 0;";
  z[lc++]= "  if (AP1.len > 1)";
  z[lc++]= "   {";
  z[lc++]= "    NT(TBL_%1$s_dline).t=(float*)calloc(AP1.len-1,sizeof(float));";
  z[lc++]= "    NT(TBL_%1$s_dline).llmem=1;";
  z[lc++]= "    NT(TBL_%1$s_dline).t[0]= input;";
  z[lc++]= "    NT(TBL_%1$s_dline).tend = j-2;";
  z[lc++]= "   }";
  z[lc++]= "  ret = AP1.t[0]*input;";
  z[lc++]= "  NVI(%1$s_kcyc) = EV(kcycleidx);";
  z[lc++]= "}";

  printblock(lc);

}


/*********************************************************/
/* firt filter                                           */
/*********************************************************/

void firtcode(tnode * tptr)

{
  tnode * aptr;

  if (tptr->extra == NULL)
    firtordernone(tptr);
  else
    {
      aptr = firstopcodearg(tptr);
      aptr = aptr->next->next->next->next;
      if (aptr->rate == KRATETYPE)
	firtorderkrate(tptr);
      else
	firtorderirate(tptr);
    }
}




/*********************************************************/
/* iirt filter -- no order parameter                     */
/*********************************************************/

void iirtordernone(tnode * tptr)

{
  int lc = 0;

  z[lc++]= "if (NVI(%1$s_kcyc))";
  z[lc++]= "{";
  z[lc++]= "  i =  NT(TBL_%1$s_dline).len;";
  z[lc++]= "  j = 1;";
  z[lc++]= "  ret =  AP2.t[0]*input;";
  z[lc++]= "  if (i>=0)";
  z[lc++]= "  {";
  z[lc++]= "    ret +=  NT(TBL_%1$s_dline).t[i];";
  z[lc++]= "    while (i>0)";
  z[lc++]= "     {";
  z[lc++]= "      NT(TBL_%1$s_dline).t[i] = NT(TBL_%1$s_dline).t[i-1];";
  z[lc++]= "      if (j < AP1.len)";
  z[lc++]= "        NT(TBL_%1$s_dline).t[i] -= AP1.t[j]*ret;";
  z[lc++]= "      if (j < AP2.len)";
  z[lc++]= "        NT(TBL_%1$s_dline).t[i] += AP2.t[j]*input;";
  z[lc++]= "      j++;i--;";
  z[lc++]= "     }";
  z[lc++]= "    NT(TBL_%1$s_dline).t[0] = 0.0F;";
  z[lc++]= "    if (j < AP1.len)";
  z[lc++]= "      NT(TBL_%1$s_dline).t[0] -= AP1.t[j]*ret;";
  z[lc++]= "    if (j < AP2.len)";
  z[lc++]= "      NT(TBL_%1$s_dline).t[0] += AP2.t[j]*input;";
  z[lc++]= "  }";
  z[lc++]= "}";
  z[lc++]= "else";
  z[lc++]= "{";
  z[lc++]= "  NVI(%1$s_kcyc) = 1;";
  z[lc++]= "  if (AP1.len > AP2.len)";
  z[lc++]= "    i = NT(TBL_%1$s_dline).len = AP1.len-2;";
  z[lc++]= "  else";
  z[lc++]= "    i = NT(TBL_%1$s_dline).len = AP2.len-2;";
  z[lc++]= "  if (i>=0) {";
  z[lc++]= "    NT(TBL_%1$s_dline).t = (float *) calloc(i+1,sizeof(float)); ";
  z[lc++]= "    NT(TBL_%1$s_dline).llmem = 1; ";
  z[lc++]= "  }";
  z[lc++]= "  j = 1;";
  z[lc++]= "  ret =  AP2.t[0]*input;";
  z[lc++]= "  if (i>=0)";
  z[lc++]= "  {";
  z[lc++]= "    while (i>0)";
  z[lc++]= "     {";
  z[lc++]= "      if (j < AP1.len)";
  z[lc++]= "        NT(TBL_%1$s_dline).t[i] -= AP1.t[j]*ret;";
  z[lc++]= "      if (j < AP2.len)";
  z[lc++]= "        NT(TBL_%1$s_dline).t[i] += AP2.t[j]*input;";
  z[lc++]= "      j++;i--;";
  z[lc++]= "     }";
  z[lc++]= "    if (j < AP1.len)";
  z[lc++]= "      NT(TBL_%1$s_dline).t[0] -= AP1.t[j]*ret;";
  z[lc++]= "    if (j < AP2.len)";
  z[lc++]= "      NT(TBL_%1$s_dline).t[0] += AP2.t[j]*input;";
  z[lc++]= "  }";
  z[lc++]= "}";


  printblock(lc);

}

/*********************************************************/
/* iirt filter -- irate order parameter                  */
/*********************************************************/

void iirtorderirate(tnode * tptr)

{
  int lc = 0;

  z[lc++]= "if (NVI(%1$s_kcyc))";
  z[lc++]= "{";
  z[lc++]= "  i =  NT(TBL_%1$s_dline).len;";
  z[lc++]= "  j = 1;";
  z[lc++]= "  ret =  AP2.t[0]*input;";
  z[lc++]= "  if (i>=0)";
  z[lc++]= "  {";
  z[lc++]= "    ret +=  NT(TBL_%1$s_dline).t[i];";
  z[lc++]= "    while (i>0)";
  z[lc++]= "     {";
  z[lc++]= "      NT(TBL_%1$s_dline).t[i] = NT(TBL_%1$s_dline).t[i-1];";
  z[lc++]= "      if (j < AP1.len)";
  z[lc++]= "        NT(TBL_%1$s_dline).t[i] -= AP1.t[j]*ret;";
  z[lc++]= "      if (j < AP2.len)";
  z[lc++]= "        NT(TBL_%1$s_dline).t[i] += AP2.t[j]*input;";
  z[lc++]= "      j++;i--;";
  z[lc++]= "     }";
  z[lc++]= "    NT(TBL_%1$s_dline).t[0] = 0.0F;";
  z[lc++]= "    if (j < AP1.len)";
  z[lc++]= "      NT(TBL_%1$s_dline).t[0] -= AP1.t[j]*ret;";
  z[lc++]= "    if (j < AP2.len)";
  z[lc++]= "      NT(TBL_%1$s_dline).t[0] += AP2.t[j]*input;";
  z[lc++]= "  }";
  z[lc++]= "}";
  z[lc++]= "else";
  z[lc++]= "{";
  z[lc++]= "  NVI(%1$s_kcyc) = 1;";
  z[lc++]= "  if (AP1.len > AP2.len)";
  z[lc++]= "    i = AP1.len;";
  z[lc++]= "  else";
  z[lc++]= "    i = AP2.len;";
  z[lc++]= "  j = ((int) (va_order + 0.5F));";
  z[lc++]= "  i = ((j > 0) && (j < i)) ? j - 2: i - 2;";
  z[lc++]= "  if (i>=0) {";
  z[lc++]= "    NT(TBL_%1$s_dline).t = (float *) calloc(i+1,sizeof(float));";
  z[lc++]= "    NT(TBL_%1$s_dline).llmem = 1;";
  z[lc++]= "  }";
  z[lc++]= "  NT(TBL_%1$s_dline).len = i;";
  z[lc++]= "  j = 1;";
  z[lc++]= "  ret =  AP2.t[0]*input;";
  z[lc++]= "  if (i>=0)";
  z[lc++]= "  {";
  z[lc++]= "    while (i>0)";
  z[lc++]= "     {";
  z[lc++]= "      if (j < AP1.len)";
  z[lc++]= "        NT(TBL_%1$s_dline).t[i] -= AP1.t[j]*ret;";
  z[lc++]= "      if (j < AP2.len)";
  z[lc++]= "        NT(TBL_%1$s_dline).t[i] += AP2.t[j]*input;";
  z[lc++]= "      j++;i--;";
  z[lc++]= "     }";
  z[lc++]= "    if (j < AP1.len)";
  z[lc++]= "      NT(TBL_%1$s_dline).t[0] -= AP1.t[j]*ret;";
  z[lc++]= "    if (j < AP2.len)";
  z[lc++]= "      NT(TBL_%1$s_dline).t[0] += AP2.t[j]*input;";
  z[lc++]= "  }";
  z[lc++]= "}";

  printblock(lc);
  
}


/*********************************************************/
/* iirt filter -- krate order parameter                  */
/*********************************************************/

void iirtorderkrate(tnode * tptr)

{
  int lc = 0;

  z[lc++]= "if (NVI(%1$s_kcyc))";
  z[lc++]= "{";
  z[lc++]= "  if (NVI(%1$s_kcyc) != EV(kcycleidx))";
  z[lc++]= "  {";
  z[lc++]= "     j = ((int) (va_order + 0.5F));";
  z[lc++]= "     if ((j<=0)||((j > AP1.len)&&(j > AP2.len)))";
  z[lc++]= "      j=(AP1.len>AP2.len) ? AP1.len : AP2.len;";
  z[lc++]= "     if (NT(TBL_%1$s_dline).len != (j-2))";
  z[lc++]= "      {";
  z[lc++]= "        i = NT(TBL_%1$s_dline).len = j-2;";
  z[lc++]= "        while (i >= 0) ";
  z[lc++]= "          NT(TBL_%1$s_dline).t[i--] = 0.0F;";
  z[lc++]= "      }";
  z[lc++]= "     NVI(%1$s_kcyc) = EV(kcycleidx);";
  z[lc++]= "  }";
  z[lc++]= "  i =  NT(TBL_%1$s_dline).len;";
  z[lc++]= "  j = 1;";
  z[lc++]= "  ret =  AP2.t[0]*input;";
  z[lc++]= "  if (i>=0)";
  z[lc++]= "  {";
  z[lc++]= "    ret +=  NT(TBL_%1$s_dline).t[i];";
  z[lc++]= "    while (i>0)";
  z[lc++]= "     {";
  z[lc++]= "      NT(TBL_%1$s_dline).t[i] = NT(TBL_%1$s_dline).t[i-1];";
  z[lc++]= "      if (j < AP1.len)";
  z[lc++]= "        NT(TBL_%1$s_dline).t[i] -= AP1.t[j]*ret;";
  z[lc++]= "      if (j < AP2.len)";
  z[lc++]= "        NT(TBL_%1$s_dline).t[i] += AP2.t[j]*input;";
  z[lc++]= "      j++;i--;";
  z[lc++]= "     }";
  z[lc++]= "    NT(TBL_%1$s_dline).t[0] = 0.0F;";
  z[lc++]= "    if (j < AP1.len)";
  z[lc++]= "      NT(TBL_%1$s_dline).t[0] -= AP1.t[j]*ret;";
  z[lc++]= "    if (j < AP2.len)";
  z[lc++]= "      NT(TBL_%1$s_dline).t[0] += AP2.t[j]*input;";
  z[lc++]= "  }";
  z[lc++]= "}";
  z[lc++]= "else";
  z[lc++]= "{";
  z[lc++]= "  NVI(%1$s_kcyc) = EV(kcycleidx);";
  z[lc++]= "  if (AP1.len > AP2.len)";
  z[lc++]= "    i = AP1.len;";
  z[lc++]= "  else";
  z[lc++]= "    i = AP2.len;";
  z[lc++]= "  if (i>1) {";
  z[lc++]= "    NT(TBL_%1$s_dline).t = (float *) calloc(i-1,sizeof(float));";
  z[lc++]= "    NT(TBL_%1$s_dline).llmem = 1;";
  z[lc++]= "  }";
  z[lc++]= "  j = ((int) (va_order + 0.5F));";
  z[lc++]= "  i = ((j > 0) && (j < i)) ? j - 2: i - 2;";
  z[lc++]= "  NT(TBL_%1$s_dline).len = i;";
  z[lc++]= "  j = 1;";
  z[lc++]= "  ret =  AP2.t[0]*input;";
  z[lc++]= "  if (i>=0)";
  z[lc++]= "  {";
  z[lc++]= "    while (i>0)";
  z[lc++]= "     {";
  z[lc++]= "      if (j < AP1.len)";
  z[lc++]= "        NT(TBL_%1$s_dline).t[i] -= AP1.t[j]*ret;";
  z[lc++]= "      if (j < AP2.len)";
  z[lc++]= "        NT(TBL_%1$s_dline).t[i] += AP2.t[j]*input;";
  z[lc++]= "      j++;i--;";
  z[lc++]= "     }";
  z[lc++]= "    if (j < AP1.len)";
  z[lc++]= "      NT(TBL_%1$s_dline).t[0] -= AP1.t[j]*ret;";
  z[lc++]= "    if (j < AP2.len)";
  z[lc++]= "      NT(TBL_%1$s_dline).t[0] += AP2.t[j]*input;";
  z[lc++]= "  }";
  z[lc++]= "}";

  printblock(lc);

}



/*********************************************************/
/* iirt filter                                           */
/*********************************************************/

void iirtcode(tnode * tptr)

{
  tnode * aptr;

  if (tptr->extra == NULL)
    iirtordernone(tptr);
  else
    {
      aptr = firstopcodearg(tptr);
      aptr = aptr->next->next->next->next->next->next;
      if (aptr->rate == KRATETYPE)
	iirtorderkrate(tptr);
      else
	iirtorderirate(tptr);
    }
}


/*********************************************************/
/*   checks if any compressor controls are k-rate */
/*********************************************************/

int compresskcheck(tnode * tptr)

{
  tnode * aptr = firstopcodearg(tptr);

  aptr = aptr->next->next->next->next;
  if (aptr->rate == KRATETYPE)
    return 1;

  aptr = aptr->next->next;
  if (aptr->rate == KRATETYPE)
    return 1;

  aptr = aptr->next->next;
  if (aptr->rate == KRATETYPE)
    return 1;

  aptr = aptr->next->next;
  if (aptr->rate == KRATETYPE)
    return 1;

  aptr = aptr->next->next;
  if (aptr->rate == KRATETYPE)
    return 1;

  aptr = aptr->next->next;
  if (aptr->rate == KRATETYPE)
    return 1;

  aptr = aptr->next->next;
  if (aptr->rate == KRATETYPE)
    return 1;

  return 0;

}

/*********************************************************/
/*   evaluates compress parameters that change on k-rate */
/*********************************************************/

void compresskrate(tnode * tptr, int * lcptr)

{
  int lc = *lcptr;
  tnode * aptr = firstopcodearg(tptr);
  int knfloor, kthresh, kloknee, khiknee, kratio, katt, krel;

  aptr = aptr->next->next->next->next;
  knfloor = (aptr->rate == KRATETYPE);

  aptr = aptr->next->next;
  kthresh = (aptr->rate == KRATETYPE);
  aptr = aptr->next->next;
  kloknee = (aptr->rate == KRATETYPE);
  aptr = aptr->next->next;
  khiknee = (aptr->rate == KRATETYPE);
  aptr = aptr->next->next;
  kratio = (aptr->rate == KRATETYPE);
  aptr = aptr->next->next;
  katt = (aptr->rate == KRATETYPE);
  aptr = aptr->next->next;
  krel = (aptr->rate == KRATETYPE);

  if (knfloor)
    {
      z[lc++]= "  NV(%1$s_logmin) = 0.1F*pow(10.0F,-0.05F*(90 - nfloor));";
    }

  if (kthresh || knfloor)
    {
      z[lc++]= "  if (thresh < nfloor)";
      genex(&lc,tptr->optr->down,"Thresh < nfloor");
      z[lc++]= "  if (thresh > nfloor)";
      z[lc++]= "   {";
      z[lc++]= "    NV(%1$s_mtail) = 1.0F/(thresh-nfloor);";
      z[lc++]= "    NV(%1$s_xtail) = nfloor/(thresh-nfloor);";
      z[lc++]= "   }";
      z[lc++]= "  else";
      z[lc++]= "    NV(%1$s_mtail) = NV(%1$s_xtail) = 0.0F;";
    }

  if (kthresh || kloknee)
    {
      z[lc++]= "  if (loknee < thresh)";
      genex(&lc,tptr->optr->down,"Loknee < thresh");
    }

  if (kloknee || khiknee)
    {
      z[lc++]= "  if (hiknee < loknee)";
      genex(&lc,tptr->optr->down,"Kiknee < loknee");
    }

  if (kratio)
    {
      z[lc++]= "  if (ratio <= 0.0F)";
      genex(&lc,tptr->optr->down,"Ratio <= 0");
      z[lc++]= "  NV(%1$s_invr) = (float) pow(10.0F, 0.05F*(1.0F-ratio));";
    }

  if (katt)
    {  
      z[lc++]= "  if (att < 0.0F)";
      genex(&lc,tptr->optr->down,"Att < 0");
      z[lc++]= "  NV(%1$s_invatt) = EV(ATIME)/att;";
    }

  if (krel)
    {   
      z[lc++]= "  if (rel < 0.0F)";
      genex(&lc,tptr->optr->down,"Rel < 0");
      z[lc++]= "  NV(%1$s_invrel) =EV(ATIME)/rel;";
    }

  if (kratio || khiknee || kloknee)
    { 
      z[lc++]= "  if (hiknee != loknee)";
      z[lc++]= "    NV(%1$s_mult)=(NV(%1$s_invr)-1.0F)";
      z[lc++]= "                            /(hiknee - loknee);";
    }

  *lcptr = lc;
  return;

}


/*********************************************************/
/*   evaluates compress parameters that change on i-rate */
/*********************************************************/

void compressirate(tnode * tptr, int * lcptr)

{
  int lc = *lcptr;
  tnode * aptr = firstopcodearg(tptr);
  int infloor, ithresh, iloknee, ihiknee, iratio, iatt, irel;

  aptr = aptr->next->next->next->next;
  infloor = (aptr->rate == IRATETYPE);

  aptr = aptr->next->next;
  ithresh = (aptr->rate == IRATETYPE);
  aptr = aptr->next->next;
  iloknee = (aptr->rate == IRATETYPE);
  aptr = aptr->next->next;
  ihiknee = (aptr->rate == IRATETYPE);
  aptr = aptr->next->next;
  iratio = (aptr->rate == IRATETYPE);
  aptr = aptr->next->next;
  iatt = (aptr->rate == IRATETYPE);
  aptr = aptr->next->next;
  irel = (aptr->rate == IRATETYPE);

  if (ithresh)
    {
      z[lc++]= "  NV(%1$s_logmin) = 0.1F*pow(10.0F,-0.05F*(90 - nfloor));";
    }

  if (ithresh && infloor)
    {
      z[lc++]= "  if (thresh < nfloor)";
      genex(&lc,tptr->optr->down,"Thresh < nfloor");
      z[lc++]= "  if (thresh > nfloor)";
      z[lc++]= "   {";
      z[lc++]= "    NV(%1$s_mtail) = 1.0F/(thresh-nfloor);";
      z[lc++]= "    NV(%1$s_xtail) = nfloor/(thresh-nfloor);";
      z[lc++]= "   }";
    }

  if (ithresh && iloknee)
    {
      z[lc++]= "  if (loknee < thresh)";
      genex(&lc,tptr->optr->down,"Loknee < thresh");
    }

  if (iloknee && ihiknee)
    {
      z[lc++]= "  if (hiknee < loknee)";
      genex(&lc,tptr->optr->down,"Kiknee < loknee");
    }

  if (iratio)
    {
      z[lc++]= "  if (ratio <= 0.0F)";
      genex(&lc,tptr->optr->down,"Ratio <= 0");
      z[lc++]= "  NV(%1$s_invr) = (float) pow(10.0F, 0.05F*(1.0F - ratio));";
    }

  if (iatt)
    {  
      z[lc++]= "  if (att < 0.0F)";
      genex(&lc,tptr->optr->down,"Att < 0");
      z[lc++]= "  NV(%1$s_invatt) = EV(ATIME)/att;";
    }

  if (irel)
    {   
      z[lc++]= "  if (rel < 0.0F)";
      genex(&lc,tptr->optr->down,"Rel < 0");
      z[lc++]= "  NV(%1$s_invrel) =EV(ATIME)/rel;";
    }

  if (iratio && ihiknee && iloknee)
    { 
      z[lc++]= "  if (hiknee != loknee)";
      z[lc++]= "    NV(%1$s_mult)=(NV(%1$s_invr)-1.0F)";
      z[lc++]= "                            /(hiknee - loknee);";
    }

  *lcptr = lc;
  return;

}

/*********************************************************/
/* compressor code                                       */
/*********************************************************/

void compresscode(tnode * tptr)

{
  int lc = 0;
  int onek = compresskcheck(tptr);


  if (onek)
    {
      acycleguard2(tptr,&lc);
      z[lc++]= "{";
    }

  z[lc++]= "  if (!NVI(%1$s_kcyc))";
  z[lc++]= "  {";
  z[lc++]= "   i = NT(TBL_%1$s_xdly).len = (int) (look*EV(ARATE));";
  z[lc++]= "   if (i<=0)";
  genex(&lc,tptr->optr->down,"Lookahead buffer < 1 sample in length");
  z[lc++]= "   NT(TBL_%1$s_compdly).len = i;";
  z[lc++]= "   NT(TBL_%1$s_xdly).t = (float *)calloc(i,sizeof(float));";
  z[lc++]= "   NT(TBL_%1$s_xdly).llmem = 1;";
  z[lc++]= "   NT(TBL_%1$s_compdly).t=(float *)calloc(i,sizeof(float));";
  z[lc++]= "   NT(TBL_%1$s_compdly).llmem=1;";
  z[lc++]= "   NV(%1$s_lval)= look*EV(ARATE);";
  compressirate(tptr,&lc);
  kcycassign(tptr, &lc);
  z[lc++]= "  }";
  if (onek)
    {
      compresskrate(tptr,&lc);
      kcycassign2(tptr, &lc);
      z[lc++]= "}";
    }

  z[lc++]= " ";
  
  z[lc++]= "i = NT(TBL_%1$s_xdly).tend;";
  z[lc++]= "i--;";
  z[lc++]= "if (i<0)";
  z[lc++]= " i = NT(TBL_%1$s_xdly).len-1;";
  z[lc++]= "NT(TBL_%1$s_xdly).tend = i;";
  z[lc++]= "NV(%1$s_oldval) = NT(TBL_%1$s_xdly).t[i];";
  z[lc++]= "NT(TBL_%1$s_xdly).t[i] = x;";

  z[lc++]= "NV(%1$s_comp1) = 90.0F + 20.0F*(float)log10";
  z[lc++]= "              (NV(%1$s_logmin) + (float)fabs(comp));";
  z[lc++]= "i = NT(TBL_%1$s_compdly).tend;";
  z[lc++]= "i--;";
  z[lc++]= "if (i<0)";
  z[lc++]= " i = NT(TBL_%1$s_compdly).len-1;";
  z[lc++]= "NT(TBL_%1$s_compdly).tend = i;";
  z[lc++]= "NV(%1$s_comp2) = NT(TBL_%1$s_compdly).t[i];";
  z[lc++]= "NT(TBL_%1$s_compdly).t[i] = NV(%1$s_comp1);";
  z[lc++]= " ";
  
  z[lc++]= "if (NV(%1$s_comp2) > NV(%1$s_env))";
  z[lc++]= "  NV(%1$s_change) = NV(%1$s_invatt)*";
  z[lc++]= "                   (NV(%1$s_comp2)-NV(%1$s_env));";
  z[lc++]= "else";
  z[lc++]= "{";
  z[lc++]= "  NV(%1$s_projEnv) = NV(%1$s_change)*NV(%1$s_lval);";
  z[lc++]= "  if (NV(%1$s_projEnv) < nfloor)";
  z[lc++]= "    NV(%1$s_projEnv) = nfloor;";
  z[lc++]= "  if ( (NV(%1$s_comp1) > NV(%1$s_projEnv)) && ";
  z[lc++]= "       (NV(%1$s_comp2) > NV(%1$s_comp1)) )";
  z[lc++]= "    NV(%1$s_change) = NV(%1$s_invrel)*";
  z[lc++]= "                     (NV(%1$s_comp1)-NV(%1$s_comp2));";
  z[lc++]= "  else";
  z[lc++]= "    NV(%1$s_change) = 0.0F;";
  z[lc++]= "}";
  z[lc++]= "if ((NV(%1$s_env)+= NV(%1$s_change)) < nfloor)";
  z[lc++]= "  NV(%1$s_env) = nfloor;";
  z[lc++]= " ";
  
  z[lc++]= "if (NV(%1$s_env) < thresh)";
  z[lc++]= " {";
  z[lc++]= "  if (NV(%1$s_env) > nfloor)";
  z[lc++]= "    ret = NV(%1$s_oldval)*(NV(%1$s_mtail)*NV(%1$s_env)";
  z[lc++]= "                           - NV(%1$s_xtail));";
  z[lc++]= "  else";
  z[lc++]= "    ret = 0.0F;";
  z[lc++]= " }";
  z[lc++]= "else";
  z[lc++]= " {";
  z[lc++]= "  if (NV(%1$s_env) < loknee)";
  z[lc++]= "    ret = NV(%1$s_oldval);";
  z[lc++]= "  else";  
  z[lc++]= "   {";
  z[lc++]= "    if (NV(%1$s_env) >= hiknee)";
  z[lc++]= "      ret = NV(%1$s_oldval)*NV(%1$s_invr);";
  z[lc++]= "    else";
  z[lc++]= "      ret = NV(%1$s_oldval)*(1.0F + ";
  z[lc++]= "                      (NV(%1$s_env)-loknee)*NV(%1$s_mult));";
  z[lc++]= "    }";
  z[lc++]= " }";

  printblock(lc);

}

/*********************************************************/
/* length check code for rms                              */
/*********************************************************/

void rmslengthset(int * lcptr, tnode * tptr)

{
  int lc = *lcptr;

  if (tptr->extra == NULL)
    z[lc++]= "    i = NT(TBL_%1$s_buffer).len = EV(ACYCLE);";
  else
    {
      z[lc++]="    i = 0;";
      z[lc++]="    if (va_length > 0.0F)";
      z[lc++]="      i = NT(TBL_%1$s_buffer).len = va_length*EV(ARATE);";
      z[lc++]="    if (i == 0)";
      genex(&lc,tptr->optr->down, "Buffer time yields zero-len buffer");
    }

  z[lc++]= " NT(TBL_%1$s_buffer).t=(float *)calloc(i,sizeof(float));";
  z[lc++]= " NT(TBL_%1$s_buffer).llmem=1;";
  z[lc++]= " NV(%1$s_scale) = 1.0F/NT(TBL_%1$s_buffer).len;";
  z[lc++]= " NVI(%1$s_kcyc) = EV(kcycleidx);";

  *lcptr = lc;
}

/*********************************************************/
/* krate code for rms                                    */
/*********************************************************/

void rmskrate(tnode * tptr)

{
  int lc = 0;

  z[lc++]= "if (NVI(%1$s_kcyc))";
  z[lc++]= "{";
  z[lc++]= "  ret = 0.0F;";
  z[lc++]= "  for (i=0; i < NT(TBL_%1$s_buffer).len;i++)";
  z[lc++]= "   ret +=  NT(TBL_%1$s_buffer).t[i]*NT(TBL_%1$s_buffer).t[i];";
  z[lc++]= "  ret = (float)sqrt(ret*NV(%1$s_scale));";
  z[lc++]= "}";
  z[lc++]= "else";
  z[lc++]= "{";
  rmslengthset(&lc, tptr);
  z[lc++]= " ret = 0.0F;";
  z[lc++]= "}";

  printblock(lc);

}

/*********************************************************/
/* arate code for rms                                    */
/*********************************************************/

void rmsarate(tnode * tptr)

{
  int lc = 0;
  sigsym * sptr = tptr->optr->sptr;

  if (sptr->cref->callif)
    {
      z[lc++]= " if (!NVI(%1$s_kcyc))";
      z[lc++]= "  {";
      rmslengthset(&lc, tptr);
      z[lc++]= "  }";
    }

  z[lc++]= "  i = NT(TBL_%1$s_buffer).tend++;";
  z[lc++]= "  if (NT(TBL_%1$s_buffer).tend == NT(TBL_%1$s_buffer).len)";
  z[lc++]= "    NT(TBL_%1$s_buffer).tend = 0;";
  z[lc++]= "  NT(TBL_%1$s_buffer).t[i] = x;";

  printbody(lc);
}


/*********************************************************/
/* rms specialopcode                                     */
/*********************************************************/

void rmscode(tnode * tptr)

{

  if (currspecialrate == KRATETYPE)
    rmskrate(tptr);
  else
    rmsarate(tptr);
}



/*********************************************************/
/* decimate specialopcode                                */
/*********************************************************/

void decimatecode(tnode * tptr)

{
  int lc = 0;

  if (currspecialrate == KRATETYPE)
    {  
      z[lc++]= "  ret = NV(%1$s_state);";
      printblock(lc);
    }
  else
    {  
      z[lc++]= "  NV(%1$s_state) = input;";
      printbody(lc);
    }
}


/*********************************************************/
/* krate code for downsamp                               */
/*********************************************************/

void downsampkrate(tnode * tptr)

{
  int lc = 0;
  sigsym * sptr = tptr->optr->sptr;


  z[lc++]= "if (NVI(%1$s_kcyc))";
  z[lc++]= "{";
  z[lc++]= "  ret = 0.0F;";
  z[lc++]= "  for (i = 0; i < NT(TBL_%1$s_buffer).len;i++)";
  if (tptr->extra == NULL)
    {
      z[lc++]= "  ret += NT(TBL_%1$s_buffer).t[i];";
    }
  else
    {
      z[lc++]= "  ret += NT(TBL_%1$s_buffer).t[i]*AP1.t[i];";
    }
  if (tptr->extra == NULL)
    z[lc++]= "  ret *=  NT(TBL_%1$s_buffer).oconst;"; 
  if (sptr->cref->callif)
    {
      z[lc++]= " NVI(%1$s_kcyc) = EV(kcycleidx);";
    }
  z[lc++]= "}";
  z[lc++]= "else";
  z[lc++]= "{";
  z[lc++]= " NVI(%1$s_kcyc) = EV(kcycleidx);";
  z[lc++]= " i = NT(TBL_%1$s_buffer).len = EV(ACYCLE);";
  z[lc++]= " NT(TBL_%1$s_buffer).oconst = 1.0F/EV(ACYCLE);";
  if (tptr->extra != NULL)
    {
      z[lc++]= " if (AP1.len < NT(TBL_%1$s_buffer).len)";
      genex(&lc,tptr->optr->down,"Window length < buffer length");
    }
  z[lc++]= " NT(TBL_%1$s_buffer).t = (float *)calloc(i,sizeof(float));";
  z[lc++]= " NT(TBL_%1$s_buffer).llmem = 1;";
  z[lc++]= " ret = 0.0F;";
  z[lc++]= "}";

  printblock(lc);

}

/*********************************************************/
/* arate code for downsamp                               */
/*********************************************************/

void downsamparate(tnode * tptr)

{
  int lc = 0;
  sigsym * sptr = tptr->optr->sptr;

  if (sptr->cref->callif)
    {

      z[lc++]= " if (!NVI(%1$s_kcyc))";
      z[lc++]= "  {";
      z[lc++]= "    i = NT(TBL_%1$s_buffer).len = EV(ACYCLE);";
      if (tptr->extra != NULL)
	{
	  z[lc++]="    if (va_length > 0.0F);";
	  z[lc++]="     i = NT(TBL_%1$s_buffer).len = va_length*EV(ARATE);";
	}
      z[lc++]= "   NT(TBL_%1$s_buffer).t=(float *)calloc(i,sizeof(float));";
      z[lc++]= "   NT(TBL_%1$s_buffer).llmem=1;";
      z[lc++]= "   NV(%1$s_scale) = 1.0F/NT(TBL_%1$s_buffer).len;";
      z[lc++]= "   NVI(%1$s_kcyc) = EV(kcycleidx);";
      z[lc++]= "  }";
      z[lc++]= " else";
      z[lc++]= "  if (NVI(%1$s_kcyc) != EV(kcycleidx))";
      z[lc++]= "   {";
      z[lc++]= "     NVI(%1$s_kcyc) = EV(kcycleidx);";
      z[lc++]= "     NT(TBL_%1$s_buffer).tend = 0;";
      z[lc++]= "   }";
    }
  
  z[lc++]= "    i = NT(TBL_%1$s_buffer).tend++;";
  z[lc++]= "    if (NT(TBL_%1$s_buffer).tend==NT(TBL_%1$s_buffer).len)";
  z[lc++]= "      NT(TBL_%1$s_buffer).tend = 0;";
  z[lc++]= "    NT(TBL_%1$s_buffer).t[i] = input;";

  printbody(lc);
}


/*********************************************************/
/* downsamp specialopcode                                */
/*********************************************************/

void downsampcode(tnode * tptr)

{

  if (currspecialrate == KRATETYPE)
    downsampkrate(tptr);
  else
    downsamparate(tptr);
}


/*********************************************************/
/* krate code for sblock                               */
/*********************************************************/

void sblockkrate(tnode * tptr)

{
  int lc = 0;
  sigsym * sptr = tptr->optr->sptr;

  if (sptr->cref->callif)
    {
      z[lc++]= " NVI(%1$s_kcyc) = EV(kcycleidx);";
    }
  z[lc++]= " NVI(%1$s_idx) = 0;";
  z[lc++]= " ret = 0.0F;";
  printblock(lc);
}


/*********************************************************/
/* arate code for sblock                               */
/*********************************************************/

void sblockarate(tnode * tptr)

{
  int lc = 0;
  sigsym * sptr = tptr->optr->sptr;


  if (sptr->cref->callif)
    {
      z[lc++]= " if (NVI(%1$s_kcyc) != EV(kcycleidx))";
      z[lc++]= "  {";
      z[lc++]= "    NVI(%1$s_kcyc) = EV(kcycleidx);";
      z[lc++]= "    NVI(%1$s_idx) = 0;";
      z[lc++]= "  }";
    }

  z[lc++]= " AP1.t[NVI(%1$s_idx)++] = x;";
  z[lc++]= " if (NVI(%1$s_idx) == AP1.len)";
  z[lc++]= "   NVI(%1$s_idx) = 0;";

  printbody(lc);

}


/*********************************************************/
/* sblock specialopcode                                  */
/*********************************************************/

void sblockcode(tnode * tptr)

{

  if (currspecialrate == KRATETYPE)
    sblockkrate(tptr);
  else
    sblockarate(tptr);
}


/*********************************************************/
/* computes length of extra arguments                    */
/*********************************************************/

int extralength(tnode * tptr)

{
  int j=0;

  tptr = tptr->extra;
  while (tptr != NULL)
    {
      j++;
      tptr = tptr->next;
      if (tptr != NULL)
	tptr = tptr->next;
    }
  return j;
}

/*********************************************************/
/*           generates core opcode code block            */
/*********************************************************/

void coreopcodebody(tnode * tptr)

{

int lc = 0;
int i,j,k,l;

 switch (tptr->val[0]) {
  case 'a':
    if (!(strcmp(tptr->val,"abs")))  /* inline */
      {
	mathopscode("fabs");
        return;
      }
    if (!(strcmp(tptr->val,"acos"))) /* inline */
      {
	mathopscode(tptr->val);
        return;
      }
    if (!(strcmp(tptr->val,"aexpon"))) 
      {
	aexpcode(tptr);
	return;
      }
    if (!(strcmp(tptr->val,"aexprand"))) /* inline */
      {
	z[lc++]= "if (p1 <= 0.0F)";
	genex(&lc,tptr->optr->down,"p1 < 0");
	z[lc++]= "ret = -p1*(float)log(RMULT*((float)rand())+1e-45F);";
	printblock(lc);
        return;
      }
    if (!(strcmp(tptr->val,"agaussrand"))) /* inline */
      {
	z[lc++]= "if (var <= 0.0F)";
	genex(&lc,tptr->optr->down,"var <= 0");
	z[lc++]= "ret = mean + (float)sqrt(var)*(float)cos(6.283185F*RMULT*((float)rand()))*";
	z[lc++]= "      (float)sqrt(-2.0F*(float)log(RMULT*((float)rand())+1e-45F));";
	printblock(lc);
        return;
      }
    if (!(strcmp(tptr->val,"aline")))
      {
	alinecode(tptr);
	return;
      }
    if (!(strcmp(tptr->val,"alinrand")))
      {
	z[lc++]= "a = RMULT*((float)rand());";
	z[lc++]= "b = RMULT*((float)rand());";
	z[lc++]= "if (a>b)";
	z[lc++]= "   ret = a*(p2-p1) + p1;";
	z[lc++]= "else";
	z[lc++]= "   ret = b*(p2-p1) + p1;";
	printblock(lc);
        return;
      }
    if (!(strcmp(tptr->val,"allpass")))
      {
	acycleguard(tptr, &lc);
	z[lc++]= "{";
	z[lc++]= "  i = NT(TBL_%1$s_dline).tend;";
	z[lc++]= "  ret = NT(TBL_%1$s_dline).t[i] - input*gain;";
	z[lc++]= "  NT(TBL_%1$s_dline).t[i] = ret*gain + input;";
	z[lc++]= "  if ((++NT(TBL_%1$s_dline).tend)==NT(TBL_%1$s_dline).len)";
	z[lc++]= "    NT(TBL_%1$s_dline).tend=0;";
	z[lc++]= "}";
	z[lc++]= "else";
	z[lc++]= "{";
	z[lc++]= "  if (!NVI(%1$s_kcyc))";
	z[lc++]= "  {";
	z[lc++]= "   i = NT(TBL_%1$s_dline).len = (int)(time*EV(ARATE));";
	z[lc++]= "   NT(TBL_%1$s_dline).tend = 0;";
	z[lc++]= "   if (i<=0)";
	genex(&lc,tptr->optr->down,"Allpass buffer < 1 sample in length");
	z[lc++]= "   NT(TBL_%1$s_dline).t = (float *)malloc(i*sizeof(float));";
	z[lc++]= "   NT(TBL_%1$s_dline).llmem = 1;";
	z[lc++]= "   while (i > 0) ";
	z[lc++]= "    NT(TBL_%1$s_dline).t[--i] = 0.0F;";
	kcycassign(tptr, &lc);
	z[lc++]= "  }";
	kcycassign2(tptr, &lc);
	z[lc++]= "  i = NT(TBL_%1$s_dline).tend;";
	z[lc++]= "  ret = NT(TBL_%1$s_dline).t[i] - input*gain;";
	z[lc++]= "  NT(TBL_%1$s_dline).t[i] = ret*gain + input;";
	z[lc++]= "  if ((++NT(TBL_%1$s_dline).tend)==NT(TBL_%1$s_dline).len)";
	z[lc++]= "    NT(TBL_%1$s_dline).tend=0;";
	z[lc++]= "}";
	printblock(lc);
        return;
      }
    if (!(strcmp(tptr->val,"ampdb"))) /* inline */
      {
	z[lc++]= "ret = (float)pow(10.0F, 5.0e-2F*(x-90.0F));";
	printblock(lc);
        return;
      }
    if (!(strcmp(tptr->val,"aphasor")))
      {
	aphasorcode(tptr);
	return ;
      }
    if (!(strcmp(tptr->val,"apoissonrand")))
      {
	if (isocheck)
	  {
	    z[lc++]= "if (p1 <= 0.0F)";
	    genex(&lc,tptr->optr->down,"p1 < 0");
	  }
	z[lc++]= "ret = 0.0F;";
	z[lc++]= "if (--NVI(%1$s_state) < 1)";
	z[lc++]= "{";
	z[lc++]= "  if (NVI(%1$s_state) == 0)";
	z[lc++]= "    ret = 1.0F;";
	z[lc++]= "  NVI(%1$s_state) = 2 + ";
	z[lc++]= "   (int)floor(-EV(ARATE)*p1*(float)log(RMULT*((float)rand())+1e-45F));";
	z[lc++]= "}";
	printblock(lc);
        return;
      }
    if (!(strcmp(tptr->val,"arand"))) /* inline */
      {
	z[lc++]= "ret = 2.0F*p*(RMULT*((float)rand()) - 0.5F);";
	printblock(lc);
        return;
      }
    if (!(strcmp(tptr->val,"asin"))) /* inline */
      {
	mathopscode(tptr->val);
        return;
      }
    if (!(strcmp(tptr->val,"atan"))) /* inline */
      {
	mathopscode(tptr->val);
        return;
      }
    return ;
  case 'b':
    if (!(strcmp(tptr->val,"balance")))
      {
	z[lc++]= "if (NVI(%1$s_lcount))";
	z[lc++]= " {";
	z[lc++]= "   NV(%1$s_acc) +=  x*x;";
	z[lc++]= "   NV(%1$s_racc) +=  ref*ref;";
	z[lc++]= "   NVI(%1$s_lcount)--;";
	z[lc++]= "   ret =  NV(%1$s_atten)*x;";
	z[lc++]= " }";
	z[lc++]= "else";
	z[lc++]= " {";
	z[lc++]= "  if (NVI(%1$s_lval))";
	z[lc++]= "   {";
	z[lc++]= "    NVI(%1$s_lcount) = NVI(%1$s_lval);";
	z[lc++]= "    if ((NV(%1$s_acc) > 1e-19F)&&(NV(%1$s_racc) < 1e19F))";
	z[lc++]= "      NV(%1$s_atten)=";
	z[lc++]= "        (float)sqrt(NV(%1$s_racc)/NV(%1$s_acc));";
	z[lc++]= "    NV(%1$s_acc) =  x*x;";
	z[lc++]= "    NV(%1$s_racc) =  ref*ref;";
	z[lc++]= "    ret =  NV(%1$s_atten)*x;";
	z[lc++]= "   }";
	z[lc++]= "  else";
	z[lc++]= "   {";
	z[lc++]= "     NVI(%1$s_lval) = EV(ACYCLE) - 1;";
	if (tptr->extra != NULL)
	  {
	    z[lc++]="  if (va_length*EV(ARATE) > 0.5F);";
	    z[lc++]="   NVI(%1$s_lval)=((int)(0.5F + va_length*EV(ARATE)))-1;";
	  }
	z[lc++]= "    NVI(%1$s_lcount)=NVI(%1$s_lval);";
	z[lc++]= "    NV(%1$s_atten)= 1.0F;";
	z[lc++]= "    NV(%1$s_acc) =  x*x;";
	z[lc++]= "    NV(%1$s_racc) =  ref*ref;";
	z[lc++]= "    ret =  x;";
	z[lc++]= "   }";
	z[lc++]= " }";
	printblock(lc);
        return;
      }
    if (!(strcmp(tptr->val,"bandpass")))
      {
	bandpass(tptr);
        return;
      }
    if (!(strcmp(tptr->val,"bandstop")))
      {
	bandstop(tptr);
        return;
      }
    if (!(strcmp(tptr->val,"biquad")))
      {
	z[lc++]= "ret =         NV(%1$s_d2)          + b0*input;";
	z[lc++]= "NV(%1$s_d2) = NV(%1$s_d1) - a1*ret + b1*input;";
	z[lc++]= "NV(%1$s_d1) =             - a2*ret + b2*input;;";
	printblock(lc);
        return;
      }
    if (!(strcmp(tptr->val,"buzz")))
      {
	buzzcode(tptr);
	return ;
      }
    return ;
  case 'c':
    if (!(strcmp(tptr->val,"ceil"))) /* inline */
      {
	mathopscode(tptr->val);
        return;
      }
    if (!(strcmp(tptr->val,"chorus")))
      { 
	choruscode(tptr);
        return;
      }
    if (!(strcmp(tptr->val,"comb")))
      {
	acycleguard(tptr, &lc);
	z[lc++]= "{";
	z[lc++]= "  i = NT(TBL_%1$s_dline).tend;";
	z[lc++]= "  ret = NT(TBL_%1$s_dline).t[i];";
	z[lc++]= "  NT(TBL_%1$s_dline).t[i] *= gain;";
	z[lc++]= "  NT(TBL_%1$s_dline).t[i] += input;";
	z[lc++]= "  if ((++NT(TBL_%1$s_dline).tend)==NT(TBL_%1$s_dline).len)";
	z[lc++]= "    NT(TBL_%1$s_dline).tend=0;";
	z[lc++]= "}";
	z[lc++]= "else";
	z[lc++]= "{";
	z[lc++]= "  if (!NVI(%1$s_kcyc))";
	z[lc++]= "  {";
	z[lc++]= "   i = NT(TBL_%1$s_dline).len = (int)(time*EV(ARATE));";
	z[lc++]= "   NT(TBL_%1$s_dline).tend = 0;";
	z[lc++]= "   if (i<=0)";
	genex(&lc,tptr->optr->down,"Comb buffer less than 1 sample in length");
	z[lc++]= "   NT(TBL_%1$s_dline).t = (float *)malloc(i*sizeof(float));";
	z[lc++]= "   NT(TBL_%1$s_dline).llmem = 1;";
	z[lc++]= "   while (i > 0) ";
	z[lc++]= "    NT(TBL_%1$s_dline).t[--i] = 0.0F;";
	kcycassign(tptr, &lc);
	z[lc++]= "  }";
	kcycassign2(tptr, &lc);
	z[lc++]= "  i = NT(TBL_%1$s_dline).tend;";
	z[lc++]= "  ret = NT(TBL_%1$s_dline).t[i];";
	z[lc++]= "  NT(TBL_%1$s_dline).t[i] *= gain;";
	z[lc++]= "  NT(TBL_%1$s_dline).t[i] += input;";
	z[lc++]= "  if ((++NT(TBL_%1$s_dline).tend)==NT(TBL_%1$s_dline).len)";
	z[lc++]= "    NT(TBL_%1$s_dline).tend=0;";
	z[lc++]= "}";
	  
	printblock(lc);
        return;
      }
    if (!(strcmp(tptr->val,"compressor")))
      {
	compresscode(tptr);
        return;
      }
    if (!(strcmp(tptr->val,"cos")))     /* inline */
      {
	mathopscode(tptr->val);
        return;
      }
    if (!(strcmp(tptr->val,"cpsmidi"))) /* inline */
      {
	z[lc++]="if (x <= 0.0F)";
	genex(&lc,tptr->optr->down,"x <= 0");
	z[lc++]="ret = EV(globaltune)*(float)pow(2.0F, 8.333334e-02F*(x-69.0F));";
	printblock(lc);
        return;
      }
    if (!(strcmp(tptr->val,"cpsoct"))) /* inline */
      {
	z[lc++]="if (x <= 0.0F)";
	genex(&lc,tptr->optr->down,"x <= 0");
	z[lc++]="ret = EV(globaltune)*(float)pow(2.0F,x-8.75F);";
	printblock(lc);
        return;
      }
    if (!(strcmp(tptr->val,"cpspch")))
      {
	z[lc++]="if (x <= 0.0F)";
	genex(&lc,tptr->optr->down,"x <= 0");
	z[lc++]="ret = (float)(ROUND(100.0F*(x - (int)x)));";
	z[lc++]="ret = (ret > 11.0F) ? 0.0F : 8.333334e-2F*ret;";
	z[lc++]="ret = EV(globaltune)*(float)pow(2.0F,(int)x+ret-8.75F);";
	printblock(lc);
        return;
      }
    return ;
  case 'd':
    if (!(strcmp(tptr->val,"dbamp"))) /* inline */
      {
	z[lc++]="if (x <= 0.0F)";
	genex(&lc,tptr->optr->down,"x <= 0");
	z[lc++]="ret = 90.0F+20.0F*(float)log10(x+1e-10F);";
	printblock(lc);
        return;
      }
    if (!(strcmp(tptr->val,"decimate")))
      {
	decimatecode(tptr);
        return;
      }
    if (!(strcmp(tptr->val,"delay")))
      {
	z[lc++]= "if (NT(TBL_%1$s_dline).t)";
	z[lc++]= "{";
	z[lc++]= "  if (NT(TBL_%1$s_dline).len)";
	z[lc++]= "    {";
	z[lc++]= "     i = NT(TBL_%1$s_dline).tend;";
	z[lc++]= "     ret = NT(TBL_%1$s_dline).t[i];";
	z[lc++]= "     NT(TBL_%1$s_dline).t[i] = x;";
	z[lc++]= "     NT(TBL_%1$s_dline).tend= ++i;";
	z[lc++]= "     if (i==NT(TBL_%1$s_dline).len)";
	z[lc++]= "      NT(TBL_%1$s_dline).tend=0;";
	z[lc++]= "    }";
	z[lc++]= "  else";
	z[lc++]= "    ret = x;";
	z[lc++]= "}";
	z[lc++]= "else";
	z[lc++]= "{";
	z[lc++]= " i = NT(TBL_%1$s_dline).len = (int) (t*EV(ARATE));";
	z[lc++]= " NT(TBL_%1$s_dline).tend = ((i == 1) ? 0 : 1);";
	z[lc++]= " if (i<0)";
	genex(&lc,tptr->optr->down,"Delay < 1 sample in length");
	z[lc++]= " if (i>0) {";
	z[lc++]= "   NT(TBL_%1$s_dline).t=(float *)calloc(i,sizeof(float));";
	z[lc++]= "   NT(TBL_%1$s_dline).llmem=1;";
	z[lc++]= " }";
	z[lc++]= " if (NT(TBL_%1$s_dline).len)";
	z[lc++]= "  {";
	z[lc++]= "    ret = 0.0F;";
	z[lc++]= "    NT(TBL_%1$s_dline).t[0] = x;";
	z[lc++]= "  }";
	z[lc++]= " else";
	z[lc++]= "   ret = x;";
	z[lc++]= "}";

	printblock(lc);
        return;
      }
    if (!(strcmp(tptr->val,"delay1")))
      {
	z[lc++]="ret = NV(%1$s_d);";
	z[lc++]="NV(%1$s_d)= x;";
	printblock(lc);
        return;
      }
    if ((!strcmp(tptr->val,"doscil")) && (interp == INTERP_SINC))
      {
	z[lc++]="if (NVI(%1$s_play) && (NVUI(%1$s_pint) < AP1.len))";
	z[lc++]=" {";
	z[lc++]="   i = NVUI(%1$s_pfrac);";
	z[lc++]="   j = (NVUI(%1$s_pfrac) += AP1.dfrac);";
	z[lc++]="   NVUI(%1$s_pint) += (j < i) + AP1.dint;";
	z[lc++]="   if ((x0 = NVUI(%1$s_pint)) < AP1.len)";
	z[lc++]="    {";
	z[lc++]="      if ((AP1.dint == 1) && (AP1.dfrac == 0))";
	z[lc++]="        ret = AP1.t[x0];";
	z[lc++]="      else";
	z[lc++]="       {";
	z[lc++]="         if (AP1.sfui == 0x00010000)";
	z[lc++]="          fptr = j >> (32 - SINC_LOG_PILEN);";
	z[lc++]="         else";
	z[lc++]="          fptr = (AP1.sfui*(j >> 16)) >> (32 - SINC_LOG_PILEN);";
	z[lc++]="        ret = sinc[fptr]*AP1.t[x0];";
	z[lc++]="        rptr = AP1.dsincr - fptr;";
	z[lc++]="        fptr += AP1.dsincr;";
	z[lc++]="        k = 0;";
	z[lc++]="        while (rptr < SINC_SIZE)";
	z[lc++]="         {";
	z[lc++]="            if ((x0 + (++k)) < AP1.len)";
	z[lc++]="              ret += sinc[rptr]*AP1.t[x0 + k];";
	z[lc++]="            rptr += AP1.dsincr;";
	z[lc++]="            if ((fptr < SINC_SIZE) && (x0 >= k))";
	z[lc++]="             {";
	z[lc++]="               ret += sinc[fptr]*AP1.t[x0 - k];";
	z[lc++]="               fptr += AP1.dsincr;";
	z[lc++]="             }";
	z[lc++]="         }";
	z[lc++]="        if (AP1.sfui != 0x00010000)";
	z[lc++]="          ret *= AP1.sffl;";
	z[lc++]="       }";
	z[lc++]="    }";
	z[lc++]="   else";
	z[lc++]="    ret = 0.0F;";
        z[lc++]=" }";
	z[lc++]="else";
	z[lc++]=" {";
	z[lc++]="   if (NVUI(%1$s_pint))";
	z[lc++]="    {";
	z[lc++]="     NVI(%1$s_play) = 0;";
	z[lc++]="     ret = 0.0F;";
	z[lc++]="    }";
	z[lc++]="  else";
	z[lc++]="   {";
	z[lc++]="     NVI(%1$s_play) = 1;";
	z[lc++]="     ret = AP1.t[0];";
	z[lc++]="     if (AP1.sfui != 0x00010000)";
	z[lc++]="       {";
	z[lc++]="        rptr = AP1.dsincr;";
	z[lc++]="        k = 1;";
	z[lc++]="        while (rptr < SINC_SIZE)";
	z[lc++]="         {";
	z[lc++]="           if (k < AP1.len)";
	z[lc++]="             ret += sinc[rptr]*AP1.t[k++];";
	z[lc++]="           rptr += AP1.dsincr;";
	z[lc++]="         }";
	z[lc++]="        ret *= AP1.sffl;";
	z[lc++]="      }";
	z[lc++]="   }";
	z[lc++]=" }";
	printblock(lc);
	return;
      }
    if ((!strcmp(tptr->val,"doscil")) && (interp == INTERP_LINEAR))
      {
	z[lc++]="if (NVI(%1$s_play) && (NVUI(%1$s_pint) < AP1.len))";
	z[lc++]=" {";
	z[lc++]="   i = NVUI(%1$s_pfrac);";
	z[lc++]="   j = (NVUI(%1$s_pfrac) += AP1.dfrac);";
	z[lc++]="   NVUI(%1$s_pint) += (j < i) + AP1.dint;";
	z[lc++]="   if ((k = NVUI(%1$s_pint)) < AP1.len)";
	z[lc++]="    {";
	z[lc++]="       ret = AP1.t[k];";
	z[lc++]="       if (k < (AP1.len - 1))";
	z[lc++]="         ret += j*((float)(1.0/4294967296.0))*(AP1.t[k+1] - ret);";
	z[lc++]="       else";
	z[lc++]="         ret -= j*((float)(1.0/4294967296.0))*AP1.t[AP1.len-1];";
	z[lc++]="    }";
	z[lc++]="   else";
	z[lc++]="    ret = 0.0F;";
        z[lc++]=" }";
	z[lc++]="else";
	z[lc++]=" {";
	z[lc++]="   ret = 0.0F;";
	z[lc++]="   if (NVUI(%1$s_pint) == 0)";
	z[lc++]="   {";
	z[lc++]="     NVI(%1$s_play) = 1;";
	z[lc++]="     ret = AP1.t[0];";
	z[lc++]="   }";
	z[lc++]="   else";
	z[lc++]="    NVI(%1$s_play) = 0;";
	z[lc++]=" }";
	printblock(lc);
	return;
      }
    if (!(strcmp(tptr->val,"downsamp")))
      {
	downsampcode(tptr);
        return;
      }
    return ;
  case 'e':
    if (!(strcmp(tptr->val,"exp"))) /* inline */
      {
	mathopscode(tptr->val);
        return;
      }
    return;
  case 'f':
    if (!(strcmp(tptr->val,"fft")))
      {
	fftcode(tptr);
	return;
      }
    if (!(strcmp(tptr->val,"fir")))
      {
	z[lc++]="ret = input*b0;";
	k = extralength(tptr);
	if (k > 1)
	  {
	    i = 1;
	    z[lc++]="switch (NVI(%1$s_p)) {";
	    z[lc++]="  case 0:";
	    mz(lc); sprintf(z[lc++],"  NVI(%s_p) = %i;",
			    currinstancename,k);
	    mz(lc); sprintf(z[lc++],"  NV(%s_z1) = input;",
			    currinstancename);
	    z[lc++]="  break;";

	    while (i<=k)
	      {
		mz(lc); sprintf(z[lc++],"  case %i:",i);
		j = i;
		l = 1;
		while (l<=k)
		  {
		    mz(lc); sprintf(z[lc++],"  ret += NV(%s_z%i)*va_b%i;",
				    currinstancename,l,j);
		    l++;
		    if (l<=k)
		      {
			j--;
			if (j == 0)
			  j=k;
		      }
		  }
		mz(lc); sprintf(z[lc++],"  NV(%s_z%i) = input;",
				currinstancename,j);
		z[lc++]="  break;";
		i++;
	      }
	    z[lc++]="}   ";
	    
	    mz(lc); sprintf(z[lc++],"if ((++NVI(%s_p))>%i)",
			    currinstancename,k);
	    z[lc++]="  NVI(%1$s_p)=1;";
	  }
	else
	  {
	    if (k == 1)
	      {
		mz(lc); sprintf(z[lc++],"  ret += NV(%s_z1)*va_b1;",
				currinstancename);
		mz(lc); sprintf(z[lc++],"  NV(%s_z1) = input;",
				currinstancename);
	      }
	  }
	printblock(lc);
        return;
      }
    if (!(strcmp(tptr->val,"firt")))
      {
	firtcode(tptr);
        return;
      }
    if (!(strcmp(tptr->val,"flange")))
      { 
	flangecode(tptr);
        return;
      }
    if (!(strcmp(tptr->val,"floor"))) /* inline */
      {
	mathopscode(tptr->val);
        return;
      }
    if (!(strcmp(tptr->val,"frac"))) /* inline */
      {
	z[lc++]="ret = x - ((int) x);";
	printblock(lc);
        return;
      }
    if (!(strcmp(tptr->val,"fracdelay")))
      { 
	fracdelay(tptr);
        return;
      }
    if (!(strcmp(tptr->val,"ftbasecps"))) /* sometimes inline */
      {
	z[lc++]="ret = AP1.base;";
	printblock(lc);
        return;
      }
    if (!(strcmp(tptr->val,"ftlen")))    /* sometimes inline */
      {
	z[lc++]="ret = AP1.len;";
	printblock(lc);
        return;
      }
    if (!(strcmp(tptr->val,"ftloop")))   /* sometimes inline */
      {
	z[lc++]="ret = AP1.start;";
	printblock(lc);
        return;
      }
    if (!(strcmp(tptr->val,"ftloopend"))) /* sometimes inline */
      {
	z[lc++]="ret = AP1.end;";
	printblock(lc);
        return;
      }
    if (!(strcmp(tptr->val,"ftsetbase")))
      {
	z[lc++]="if (x>0.0F)";
	z[lc++]="  AP1.base = x;";
	z[lc++]="ret = x;";
	printblock(lc);
        return;
      }
    if (!(strcmp(tptr->val,"ftsetend")))
      {
	z[lc++]="if ((x>=0.0F)&&(x<AP1.len))";
	z[lc++]= "{";
	z[lc++]="   AP1.tend = AP1.end = (int) x;";
	z[lc++]="   if (AP1.end==0)";
        z[lc++]="    AP1.tend = AP1.len - 1;";
	z[lc++]= "}";
	z[lc++]="ret = x;";
	printblock(lc);
        return;
      }
    if (!(strcmp(tptr->val,"ftsetloop")))
      {
	z[lc++]="if ((x>=0)&&(x<AP1.len))";
	z[lc++]="  AP1.start = (int) x;";
	z[lc++]="ret = x;";
	printblock(lc);
        return;
      }
    if (!(strcmp(tptr->val,"ftsetsr")))
      {
	z[lc++]="if (x>0.0F)";
	z[lc++]= "{";
	z[lc++]="   AP1.sr = x;";
	z[lc++]="   if (x == EV(ARATE))";
	z[lc++]= "   {";
	z[lc++]="     AP1.dint = 1;";
	z[lc++]="     AP1.dfrac = 0;";
	z[lc++]= "   }";
	z[lc++]= "  else";
	z[lc++]= "   {";
	z[lc++]="     AP1.dfrac = 4294967296.0*";
	z[lc++]="                 modf(((double)x)/((double)EV(ARATE)), &intdummy);";
	z[lc++]="     AP1.dint = intdummy;";
	z[lc++]= "   }";
	if (interp == INTERP_SINC)
	  {
	    z[lc++]="   if (EV(ARATE) >= x)";
	    z[lc++]="   {";
	    z[lc++]="     AP1.sffl = 1.0F;";
	    z[lc++]="     AP1.sfui = 0x00010000;";
	    z[lc++]="     AP1.dsincr = SINC_PILEN;";
	    z[lc++]="   }";
	    z[lc++]="   else";
	    z[lc++]="   {";
	    z[lc++]="     if ((EV(ARATE)*SINC_UPMAX) > x)";
	    z[lc++]="       AP1.sffl = (EV(ARATE)/x);";
	    z[lc++]="     else";
	    z[lc++]="       AP1.sffl = (1.0F/SINC_UPMAX);";
	    z[lc++]="     AP1.sfui = ((float)(pow(2,16)))*AP1.sffl + 0.5F;";
	    z[lc++]="     AP1.dsincr = (SINC_PILEN*AP1.sfui) >> 16;";
	    z[lc++]="   }";
	  }
	z[lc++]="ret = x;";
	z[lc++]= "}";
	z[lc++]="else";
	genex(&lc,tptr->optr->down,"Sample Rate <= 0");
	printblock(lc);
        return;
      }
    if (!(strcmp(tptr->val,"ftsr"))) /* sometimes inline */
      {
	z[lc++]="ret = AP1.sr;";
	printblock(lc);
        return;
      }
    return ;
  case 'g':
    if (!(strcmp(tptr->val,"gain")))
      {
	z[lc++]= "if (NVI(%1$s_lcount))";
	z[lc++]= " {";
	z[lc++]= "   NV(%1$s_acc) +=  x*x;";
	z[lc++]= "   NVI(%1$s_lcount)--;";
	z[lc++]= "   ret = NV(%1$s_atten)*x;";
	z[lc++]= " }";
	z[lc++]= "else";
	z[lc++]= " {";
	z[lc++]= "   if (NVI(%1$s_lval))";
	z[lc++]= "    {";
	z[lc++]= "     NVI(%1$s_lcount) = NVI(%1$s_lval);";
	z[lc++]= "     root = (float)sqrt(NV(%1$s_acc)*NV(%1$s_scale));";
	z[lc++]= "     if (root*100000.0F > gain)";
	z[lc++]= "      NV(%1$s_atten)= gain/root;";
	z[lc++]= "     else";
	z[lc++]= "      NV(%1$s_atten)=100000.0F;";
	z[lc++]= "     NV(%1$s_acc) =  x*x;";
	z[lc++]= "     ret = NV(%1$s_atten)*x;";
	z[lc++]= "    }";
	z[lc++]= "   else";
	z[lc++]= "    {";
	z[lc++]= "      NVI(%1$s_lval) = EV(ACYCLE) - 1;";
	if (tptr->extra != NULL)
	  {
	    z[lc++]="      if (va_length*EV(ARATE) > 0.5F)";
	    z[lc++]="        NVI(%1$s_lval)=(int)(0.5F + va_length*EV(ARATE)) - 1;";
	    z[lc++]="      else";
	    genex(&lc,tptr->optr->down, "Buffer time yields zero-len buffer");
	  }
	z[lc++]= "      NVI(%1$s_lcount) = NVI(%1$s_lval);";
	z[lc++]= "      NV(%1$s_scale) = 1.0F/(NVI(%1$s_lval)+1);";
	z[lc++]= "      NV(%1$s_atten)= 1.0F;";
	z[lc++]= "      NV(%1$s_acc) =  x*x;";
	z[lc++]= "      ret = x;";
	z[lc++]= "    }";
	z[lc++]= " }";
	printblock(lc);
        return;
      }
    if (!(strcmp(tptr->val,"gettempo"))) /* inline */
      {
	z[lc++]="ret = EV(tempo);";
	printblock(lc);
        return;
      }
    if (!(strcmp(tptr->val,"gettune"))) /* inline */
      {
	z[lc++]="ret = EV(globaltune);";
	printblock(lc);
        return;
      }
    if (!(strcmp(tptr->val,"grain")))
      {      	
	graincode(tptr);
        return;
      }
    return ;
  case 'h':
    if (!(strcmp(tptr->val,"hipass")))
      {
	hipass(tptr);
        return;
      }
    return ;
  case 'i':
    if (!(strcmp(tptr->val,"iexprand")))  /* inline */
      {
	z[lc++]= "if (p1 <= 0.0F)";
	genex(&lc,tptr->optr->down,"p1 <= 0");
	z[lc++]= "ret = -p1*(float)log(RMULT*((float)rand())+1e-45F);";
	printblock(lc);
        return;
      }
    if (!(strcmp(tptr->val,"ifft")))
      {
	ifftcode(tptr);
	return;
      }
    if (!(strcmp(tptr->val,"igaussrand"))) /* inline */
      {
	z[lc++]= "if (var <= 0.0F)";
	genex(&lc,tptr->optr->down,"var <= 0");
	z[lc++]= "ret = mean + (float)sqrt(var)*(float)cos(6.283185F*RMULT*((float)rand()))*";
	z[lc++]= "      (float)sqrt(-2.0F*(float)log(RMULT*((float)rand())+1e-45F));";
	printblock(lc);
        return;
      }
    if (!(strcmp(tptr->val,"iir")))
      {
	k = extralength(tptr);
	if (k==0)
	  z[lc++]= "ret =b0*input;";
	else
	  {
	    mz(lc); sprintf(z[lc++],
			    "ret =  NV(%s_d%i)         + b0*input;",
			    currinstancename,((k+1)/2));
	    i = ((k+1)/2);
	    j = 1;
	    while (i>1)
	      {
		mz(lc); sprintf(z[lc++],
	 "NV(%s_d%i) = NV(%s_d%i) - va_a%i*ret + va_b%i*input;",
			currinstancename,i,
				currinstancename,i-1,j,j);
		i--; 
		j++;
	      }
	    if (k%2 == 0)
	      {
		mz(lc); sprintf(z[lc++],
			"NV(%s_d1) =  - va_a%i*ret + va_b%i*input;",
			currinstancename,j,j);
	      }
	    else
	      {
		mz(lc); sprintf(z[lc++],
				"NV(%s_d1) =  - va_a%i*ret;",
				currinstancename,j);
		
	      }
	  }
	printblock(lc);
        return;
      }
    if (!(strcmp(tptr->val,"iirt")))
      {	
	iirtcode(tptr);
        return;
      }
    if (!(strcmp(tptr->val,"ilinrand")))
      {
	z[lc++]= "a = RMULT*((float)rand());";
	z[lc++]= "b = RMULT*((float)rand());";
	z[lc++]= "if (a>b)";
	z[lc++]= "   ret = a*(p2-p1) + p1;";
	z[lc++]= "else";
	z[lc++]= "   ret = b*(p2-p1) + p1;";
	printblock(lc);
        return;
      }
    if (!(strcmp(tptr->val,"int")))   /* inline */
      {
	z[lc++]="ret = (int) x;";
	printblock(lc);
	return;
      }
    if (!(strcmp(tptr->val,"irand"))) /* inline */
      {
	z[lc++]= "ret = 2.0F*p*(RMULT*((float)rand()) - 0.5F);";
	printblock(lc);
        return;
      }
    return ;
  case 'j':
    return ;
  case 'k':
    if (!(strcmp(tptr->val,"kexpon")))
      {
	kexpcode(tptr);
	return;
      }
    if (!(strcmp(tptr->val,"kexprand")))  /* inline */
      {
	z[lc++]= "if (p1 <= 0.0F)";
	genex(&lc,tptr->optr->down,"p1 <= 0");
	z[lc++]= "ret = -p1*(float)log(RMULT*((float)rand())+1e-45F);";
	printblock(lc);
        return;
      }
    if (!(strcmp(tptr->val,"kgaussrand")))  /* inline */
      {
	z[lc++]= "if (var <= 0.0F)";
	genex(&lc,tptr->optr->down,"var <= 0");
	z[lc++]= "ret = mean + (float)sqrt(var)*(float)cos(6.283185F*RMULT*((float)rand()))*";
	z[lc++]= "      (float)sqrt(-2.0F*(float)log(RMULT*((float)rand())+1e-45F));";
	printblock(lc);
        return;
      }
    if (!(strcmp(tptr->val,"kline")))
      {
	klinecode(tptr);
	return;
      }
    if (!(strcmp(tptr->val,"klinrand")))
      {
	z[lc++]= "a = RMULT*((float)rand());";
	z[lc++]= "b = RMULT*((float)rand());";
	z[lc++]= "if (a>b)";
	z[lc++]= "   ret = a*(p2-p1) + p1;";
	z[lc++]= "else";
	z[lc++]= "   ret = b*(p2-p1) + p1;";
	printblock(lc);
        return;
      }
    if (!(strcmp(tptr->val,"koscil")))
      {
	if (finitelooposcil(tptr))
	  {
	    if (interp == INTERP_LINEAR)
	      loopkoscil(tptr);
	    if (interp == INTERP_SINC)
	      loopkoscil_sinc(tptr);
	  }
	else
	  {
	    if (interp == INTERP_LINEAR)
	      noloopkoscil(tptr);
	    if (interp == INTERP_SINC)
	      noloopkoscil_sinc(tptr);
	  }
	return;
      }
    if (!(strcmp(tptr->val,"kphasor")))
      {
	kphasorcode(tptr);
	return ;
      }
    if (!(strcmp(tptr->val,"kpoissonrand")))
      {

	if (isocheck)
	  {
	    z[lc++]= "if (p1 <= 0.0F)";
	    genex(&lc,tptr->optr->down,"p1 < 0");
	  }

	z[lc++]= "ret = 0.0F;";
	z[lc++]= "if (--NVI(%1$s_state) < 1)";
	z[lc++]= "{";
	z[lc++]= "  if (NVI(%1$s_state) == 0)";
	z[lc++]= "    ret = 1.0F;";
	z[lc++]= "  NVI(%1$s_state) = 2 + ";
	z[lc++]= "   (int)floor(-p1*(float)log(RMULT*((float)rand())+1e-45F)*EV(KRATE));";
	z[lc++]= "}";
	printblock(lc);
        return;
      }
    if (!(strcmp(tptr->val,"krand")))  /* inline */
      {

	z[lc++]= "ret = 2.0F*p*(RMULT*((float)rand()) - 0.5F);";
	printblock(lc);
        return;
      }
    return ;
  case 'l':
    if (!(strcmp(tptr->val,"log")))/* inline */
      {
	z[lc++]="if (x <= 0.0F)";
	genex(&lc,tptr->optr->down,"x <= 0");
	mathopscode(tptr->val);
        return;
      }
    if (!(strcmp(tptr->val,"log10")))/* inline */
      {
	z[lc++]="if (x <= 0.0F)";
	genex(&lc,tptr->optr->down,"x <= 0");
	mathopscode(tptr->val);
        return;
      }
    if (!(strcmp(tptr->val,"lopass")))
      {
	lopass(tptr);
        return;
      }
    if (!(strcmp(tptr->val,"loscil")))
      {
	if (interp == INTERP_LINEAR)
	  loscil(tptr);
	if (interp == INTERP_SINC)
	  loscil_sinc(tptr);
	return;
      }
    return ;
  case 'm':
    if (!(strcmp(tptr->val,"max")))
      {
	minmaxcode(tptr,'>');
	return ;
      }
    if (!(strcmp(tptr->val,"midicps")))  /* inline */
      {
	z[lc++]="if (x <= 0.0F)";
	genex(&lc,tptr->optr->down,"x <= 0");
	z[lc++]="ret= (int)(69.5F + 1.731234e+01F*(float)log(x*EV(invglobaltune)));";
	printblock(lc);
        return;
      }
    if (!(strcmp(tptr->val,"midioct"))) /* inline */
      {
	z[lc++]="if (x <= 3.0F)";
	genex(&lc,tptr->optr->down,"x <= 3");
	z[lc++]="ret = (int)(12.0F*(x - 3.0) + 0.5F);";
	printblock(lc);
        return;
      }
    if (!(strcmp(tptr->val,"midipch")))
      {
	z[lc++]="if (x <= 3.0F)";
	genex(&lc,tptr->optr->down,"x <= 3");
	z[lc++]="ret = (float)(ROUND(100.0F*(x - (int) x)));";
	z[lc++]="ret = (ret > 11.0F) ? 0.0F : ret;";
	z[lc++]="ret = ret + 12.0F*(- 3 + (int) x);";
	printblock(lc);
        return;
      }
    if (!(strcmp(tptr->val,"min")))
      {
	minmaxcode(tptr,'<');
	return;
      }
    return ;
  case 'n':
    return ;
  case 'o':
    if (!(strcmp(tptr->val,"octcps")))/* inline */
      {
	z[lc++]="if (x <= 0.0F)";
	genex(&lc,tptr->optr->down,"x <= 0");
	z[lc++]="ret = 8.75F + 1.442695F*(float)log(x*EV(invglobaltune));";
	printblock(lc);
	return ;
      }
    if (!(strcmp(tptr->val,"octmidi")))/* inline */
      {
	z[lc++]="if (x <= 0.0F)";
	genex(&lc,tptr->optr->down,"x <= 0");
	z[lc++]="ret = 8.333334e-2F*(x+36.0F);";
	printblock(lc);
        return;
      }
    if (!(strcmp(tptr->val,"octpch")))
      {
	z[lc++]="if (x <= 0.0F)";
	genex(&lc,tptr->optr->down,"x <= 0");
	z[lc++]="ret = (float)(ROUND(100.0F*(x - (int)x)));";
	z[lc++]="ret = (ret > 11.0F) ? 0.0F : ret;";
	z[lc++]="ret = 8.333334e-2F*ret + (int) x;";
	printblock(lc);
        return;
      }
    if (!(strcmp(tptr->val,"oscil")))
      {
	if (finitelooposcil(tptr))
	  switch (looposcilrate(tptr)) {
	  case ARATETYPE:
	    if (interp == INTERP_LINEAR)
	      looposcilafreq(tptr);  
	    if (interp == INTERP_SINC)
	      looposcilafreq_sinc(tptr);  
	    break;
	  case KRATETYPE:
	    if (interp == INTERP_LINEAR)
	      looposcilkfreq(tptr);  
	    if (interp == INTERP_SINC)
	      looposcilkfreq_sinc(tptr);  
	    break;
	  case IRATETYPE:
	    if (interp == INTERP_LINEAR)
	      looposcilifreq(tptr);  
	    if (interp == INTERP_SINC)
	      looposcilifreq_sinc(tptr);  
	    break;
	  }
	else
	  switch (looposcilrate(tptr)) {
	  case ARATETYPE:
	    if (interp == INTERP_LINEAR)
	      nolooposcilafreq(tptr);  
	    if (interp == INTERP_SINC)
	      nolooposcilafreq_sinc(tptr);  
	    break;
	  case KRATETYPE:
	    if (interp == INTERP_LINEAR)
	      nolooposcilkfreq(tptr); 
	    if (interp == INTERP_SINC)
	      nolooposcilkfreq_sinc(tptr);
	    break;
	  case IRATETYPE:
	    if (interp == INTERP_LINEAR)
	      nolooposcilifreq(tptr);   
	    if (interp == INTERP_SINC)
	      nolooposcilifreq_sinc(tptr);   
	    break;
	  }
	return;
      }
    return ;
  case 'p':
    if (!(strcmp(tptr->val,"pchcps")))
      {
	z[lc++]="if (x <= 0.0F)";
	genex(&lc,tptr->optr->down,"x <= 0");
	z[lc++]="x = 8.75F +  1.442695F*(float)log(x*EV(invglobaltune));";
	z[lc++]="ret = (float)(ROUND(12.0F*(x - (int) x)));";
	z[lc++]="ret = 1.0e-2F*ret + (int) x;";
	printblock(lc);
	return ;
      }
    if (!(strcmp(tptr->val,"pchmidi")))
      {
	z[lc++]="if (x <= 0.0F)";
	genex(&lc,tptr->optr->down,"x <= 0");
	z[lc++]="ret = 8.333334e-02F*((float)(ROUND(x))+36.0F);";
	z[lc++]="ret = 12.0e-2F*(ret-(int)ret) + (int)ret;";
	printblock(lc);
        return;
      }
    if (!(strcmp(tptr->val,"pchoct")))
      {
	z[lc++]="if (x <= 0.0F)";
	genex(&lc,tptr->optr->down,"x <= 0");
	z[lc++]="ret = (float)(ROUND(12.0F*(x- (int) x)));";
	z[lc++]="ret = 1.0e-2F*ret + (int) x;";
	printblock(lc);
        return;
      }
    if (!(strcmp(tptr->val,"pluck")))
      {
	if (interp == INTERP_LINEAR)
	  pluckcode(tptr);
	if (interp == INTERP_SINC)
	  pluckcode_sinc(tptr);
	return;
      }
    if (!(strcmp(tptr->val,"port")))
      {
	portamento(tptr);
	return;
      }
    if (!(strcmp(tptr->val,"pow")))/* inline */
      {
	z[lc++]="ret = (float)pow(x,y);";
	printblock(lc);
        return;
      }
    return ;
  case 'q':
    return ;
  case 'r':
    if (!(strcmp(tptr->val,"reverb")))
      {
	reverbcode(tptr);
	return;
      }
    if (!(strcmp(tptr->val,"rms")))
      {
	rmscode(tptr);
        return;
      }
    return ;
  case 's':
    if (!(strcmp(tptr->val,"samphold")))
      {
	z[lc++]="if (gate != 0.0F)";
	z[lc++]="  NV(%1$s_lpv)= input;";
	z[lc++]="ret = NV(%1$s_lpv);";
	printblock(lc);
        return;
      }
    if (!(strcmp(tptr->val,"sblock")))
      {
	sblockcode(tptr);
        return;
      }
    if (!(strcmp(tptr->val,"settempo")))
      {
	z[lc++]="if (x <= 0.0F)";
	genex(&lc,tptr->optr->down,"x <= 0");
	z[lc++]="ret = EV(tempo) = x;";
	z[lc++]="EV(kbase) = EV(kcycleidx);";
	z[lc++]="EV(scorebase) = EV(scorebeats);";
	z[lc++]="EV(scoremult) = 1.666667e-2F*EV(KTIME)*EV(tempo);";
	z[lc++]="EV(endkcycle) = EV(kbase) + ";
	z[lc++]="   (int) (EV(KRATE)*(EV(endtime) - EV(scorebase))*(60.0F/EV(tempo)));";
	printbody(lc);
	printdurassign();
	printblock(0);
        return;
      }
    if (!(strcmp(tptr->val,"settune")))
      {
	z[lc++]="if (x <= 0.0F)";
	genex(&lc,tptr->optr->down,"x <= 0");
	z[lc++]="EV(invglobaltune) = 1/x;";
	z[lc++]="ret = EV(globaltune) = x;";
	printblock(lc);
        return;
      }
    if (!(strcmp(tptr->val,"sgn")))
      {
	z[lc++]="ret = (x == 0.0F) ? 0.0F : ((x>0.0F) ? 1.0F : -1.0F);";
	printblock(lc);
        return;
      }
    if (!(strcmp(tptr->val,"sin")))/* inline */
      {
	mathopscode(tptr->val);
        return;
      }
    if (!(strcmp(tptr->val,"spatialize")))
      {
	spatialcode(tptr);
        return;
      }
    if (!(strcmp(tptr->val,"speedt")))
      {
	z[lc++]="if (factor <= 0.0F)";
	genex(&lc,tptr->optr->down,"Factor <= 0");
	z[lc++]="if ((int)(factor*AP1.len) > AP2.len)";
	genex(&lc,tptr->optr->down,"Out table too small for factor");
	z[lc++]="picola(&(AP1),&(AP2),factor);";
	printblock(lc);
        return;
      }
    if (!(strcmp(tptr->val,"sqrt")))/* inline */
      {
	z[lc++]="if (x < 0.0F)";
	genex(&lc,tptr->optr->down,"x < 0");
	mathopscode(tptr->val);
        return;
      }
    return ;
  case 't':
    if (!(strcmp(tptr->val,"tableread")))  
      {
	if ((interp == INTERP_LINEAR) || /* integer index is inlined */
	    ((tptr->rate != ARATETYPE) && (tptr->rate != KRATETYPE))) 
	  {
	    if (isocheck)
	      {
		z[lc++]="i = (int)index;";
		z[lc++]="if ((i>=0)&&(i<AP1.len))";
		z[lc++]="  ret = AP1.t[i] + (index - i)*(AP1.t[i+1] - AP1.t[i]);";
		z[lc++]="else";
		genex(&lc,tptr->optr->down,"Tableread index out of range");
	      }
	    else
	      { 
		z[lc++]="i = (int)index;";
		z[lc++]="ret = AP1.t[i] + (index - i)*(AP1.t[i+1] - AP1.t[i]);";
	      }
	  }
	
	if ((interp == INTERP_SINC) && (tptr->rate == ARATETYPE))
	  {
	    z[lc++]="i = (int)index;";
	    z[lc++]="index -= i;";
	    z[lc++]="if ((i<0)||(i>=AP1.len))";
	    z[lc++]="  i = (i < 0) ? 0 : AP1.len - 1;";
	    z[lc++]="if ((index == 0.0F) && (AP1.sfui == 0x00010000))";
	    z[lc++]="  ret = AP1.t[i];";
	    z[lc++]="else";
	    z[lc++]=" {";
	    z[lc++]="   j = (unsigned int)(4294967296.0F*index);";
	    z[lc++]="   if (AP1.sfui == 0x00010000)";
	    z[lc++]="     fptr = j >> (32 - SINC_LOG_PILEN);";
	    z[lc++]="   else";
	    z[lc++]="     fptr = (AP1.sfui*(j >> 16)) >> (32 - SINC_LOG_PILEN);";
	    z[lc++]="  ret = sinc[fptr]*AP1.t[i];";
	    z[lc++]="  rptr = AP1.dsincr - fptr;";
	    z[lc++]="  fptr += AP1.dsincr;";  
	    z[lc++]="  k = i;";
	    z[lc++]="  incr = 1;";
	    z[lc++]="  while (rptr < SINC_SIZE)";
	    z[lc++]="   {";
	    z[lc++]="     if (incr == 1)";
	    z[lc++]="       {";
	    z[lc++]="         if (k < AP1.len - 1)";
	    z[lc++]="           k = k + incr;";
	    z[lc++]="          else";
	    z[lc++]="          {";
	    z[lc++]="            k = AP1.len - 2;";
	    z[lc++]="            incr = - 1;";
	    z[lc++]="          }";
	    z[lc++]="       }";
	    z[lc++]="     else";
	    z[lc++]="       {";
	    z[lc++]="         if (k > 0)";
	    z[lc++]="           k = k + incr;";
	    z[lc++]="          else";
	    z[lc++]="          {";
	    z[lc++]="            k = 1;";
	    z[lc++]="            incr = 1;";
	    z[lc++]="          }";
	    z[lc++]="       }";
	    z[lc++]="     ret += sinc[rptr]*AP1.t[k];";
	    z[lc++]="     rptr += AP1.dsincr;";
	    z[lc++]="   }";
	    z[lc++]="  k = i;";
	    z[lc++]="  incr = -1;";
	    z[lc++]="  while (fptr < SINC_SIZE)";
	    z[lc++]="   {";
	    z[lc++]="     if (incr == 1)";
	    z[lc++]="       {";
	    z[lc++]="         if (k < AP1.len - 1)";
	    z[lc++]="           k = k + incr;";
	    z[lc++]="          else";
	    z[lc++]="          {";
	    z[lc++]="            k = AP1.len - 2;";
	    z[lc++]="            incr = - 1;";
	    z[lc++]="          }";
	    z[lc++]="       }";
	    z[lc++]="     else";
	    z[lc++]="       {";
	    z[lc++]="         if (k > 0)";
	    z[lc++]="           k = k + incr;";
	    z[lc++]="          else";
	    z[lc++]="          {";
	    z[lc++]="            k = 1;";
	    z[lc++]="            incr = 1;";
	    z[lc++]="          }";
	    z[lc++]="       }";
	    z[lc++]="     ret += sinc[fptr]*AP1.t[k];";
	    z[lc++]="     fptr += AP1.dsincr;";
	    z[lc++]="   }";
	    z[lc++]="  if (AP1.sfui != 0x00010000)";
	    z[lc++]="   ret *= AP1.sffl;";
	    z[lc++]=" }";
	  }

	if ((interp == INTERP_SINC) && (tptr->rate == KRATETYPE))
	  {
	    z[lc++]="if (EV(KRATE) >= AP1.sr)";
	    z[lc++]=" {";
	    z[lc++]="   ksffl = 1.0F;";
	    z[lc++]="   ksfui = 0x00010000;";
	    z[lc++]="   kdsincr = SINC_PILEN;";
	    z[lc++]=" }";
	    z[lc++]="else";
	    z[lc++]=" {";
	    z[lc++]="  if ((EV(KRATE)*SINC_UPMAX) > AP1.sr)";
	    z[lc++]="    ksffl = (EV(KRATE)/AP1.sr);";
	    z[lc++]="  else";
	    z[lc++]="    ksffl = (1.0F/SINC_UPMAX);";
	    z[lc++]="  ksfui = ((float)(pow(2,16)))*ksffl + 0.5F;";
	    z[lc++]="  kdsincr = (SINC_PILEN*ksfui) >> 16;";
	    z[lc++]=" }";	    
	    z[lc++]="i = (int)index;";
	    z[lc++]="index -= i;";
	    z[lc++]="if ((i<0)||(i>=AP1.len))";
	    z[lc++]="  i = (i < 0) ? 0 : AP1.len - 1;";
	    z[lc++]="if ((index == 0.0F) && (ksfui == 0x00010000))";
	    z[lc++]="  ret = AP1.t[i];";
	    z[lc++]="else";
	    z[lc++]=" {";
	    z[lc++]="   j = (unsigned int)(4294967296.0F*index);";
	    z[lc++]="   if (ksfui == 0x00010000)";
	    z[lc++]="     fptr = j >> (32 - SINC_LOG_PILEN);";
	    z[lc++]="   else";
	    z[lc++]="     fptr = (ksfui*(j >> 16)) >> (32 - SINC_LOG_PILEN);";
	    z[lc++]="  ret = sinc[fptr]*AP1.t[i];";
	    z[lc++]="  rptr = kdsincr - fptr;";
	    z[lc++]="  fptr += kdsincr;";  
	    z[lc++]="  k = i;";
	    z[lc++]="  incr = 1;";
	    z[lc++]="  while (rptr < SINC_SIZE)";
	    z[lc++]="   {";
	    z[lc++]="     if (incr == 1)";
	    z[lc++]="       {";
	    z[lc++]="         if (k < AP1.len - 1)";
	    z[lc++]="           k = k + incr;";
	    z[lc++]="          else";
	    z[lc++]="          {";
	    z[lc++]="            k = AP1.len - 2;";
	    z[lc++]="            incr = - 1;";
	    z[lc++]="          }";
	    z[lc++]="       }";
	    z[lc++]="     else";
	    z[lc++]="       {";
	    z[lc++]="         if (k > 0)";
	    z[lc++]="           k = k + incr;";
	    z[lc++]="          else";
	    z[lc++]="          {";
	    z[lc++]="            k = 1;";
	    z[lc++]="            incr = 1;";
	    z[lc++]="          }";
	    z[lc++]="       }";
	    z[lc++]="     ret += sinc[rptr]*AP1.t[k];";
	    z[lc++]="     rptr += kdsincr;";
	    z[lc++]="   }";
	    z[lc++]="  k = i;";
	    z[lc++]="  incr = -1;";
	    z[lc++]="  while (fptr < SINC_SIZE)";
	    z[lc++]="   {";
	    z[lc++]="     if (incr == 1)";
	    z[lc++]="       {";
	    z[lc++]="         if (k < AP1.len - 1)";
	    z[lc++]="           k = k + incr;";
	    z[lc++]="          else";
	    z[lc++]="          {";
	    z[lc++]="            k = AP1.len - 2;";
	    z[lc++]="            incr = - 1;";
	    z[lc++]="          }";
	    z[lc++]="       }";
	    z[lc++]="     else";
	    z[lc++]="       {";
	    z[lc++]="         if (k > 0)";
	    z[lc++]="           k = k + incr;";
	    z[lc++]="          else";
	    z[lc++]="          {";
	    z[lc++]="            k = 1;";
	    z[lc++]="            incr = 1;";
	    z[lc++]="          }";
	    z[lc++]="       }";
	    z[lc++]="     ret += sinc[fptr]*AP1.t[k];";
	    z[lc++]="     fptr += kdsincr;";
	    z[lc++]="   }";
	    z[lc++]="  if (ksfui != 0x00010000)";
	    z[lc++]="   ret *= ksffl;";
	    z[lc++]=" }";
	  }
        printblock(lc);
	return ;
      }
    if (!(strcmp(tptr->val,"tablewrite"))) /* not inlined anymore */
      { 
	if (isocheck)
	  {
	    z[lc++]="i = ROUND(index);";
	    z[lc++]="if (!i)";
	    z[lc++]="  AP1.t[AP1.len] = val;";
	    z[lc++]="if ((i>=0)&&(i<AP1.len))";
	    z[lc++]="   AP1.t[i] = val;";
	    z[lc++]="else";
	    genex(&lc,tptr->optr->down,"Tablewrite index out of range");
	    z[lc++]="ret = val;";
	  }
	else
	  {
	    z[lc++]="i = ROUND(index);";
	    z[lc++]="ret = AP1.t[i] = val;";
	    z[lc++]="if (!i)";
	    z[lc++]="  AP1.t[AP1.len] = AP1.t[i];";
	  }
	printblock(lc);
	return ;
      }
  case 'u':
    if (!(strcmp(tptr->val,"upsamp")))
      {
	acycleguard(tptr, &lc);
	z[lc++]= " {";
	z[lc++]="   i = NT(TBL_%1$s_buffer).tend++;";
	z[lc++]="   ret = 0.0F;";
	z[lc++]="   if (i < NT(TBL_%1$s_buffer).len)";
	z[lc++]="     ret = NT(TBL_%1$s_buffer).t[i];";
	z[lc++]= " }";
	z[lc++]= "else";
	z[lc++]= " {";
	z[lc++]= "  if (NT(TBL_%1$s_buffer).t == NULL)";
	z[lc++]= "   {";
	z[lc++]= "    i = NT(TBL_%1$s_buffer).len = EV(ACYCLE);";
	if (tptr->extra != NULL)
	  {
	    z[lc++]= "   if (AP1.len > NT(TBL_%1$s_buffer).len)";
	    z[lc++]= "    i = NT(TBL_%1$s_buffer).len = AP1.len;";
	  }
	z[lc++]= "    NT(TBL_%1$s_buffer).t = (float *) calloc(i,sizeof(float));";
	z[lc++]= "    NT(TBL_%1$s_buffer).llmem = 1;";
	z[lc++]= "   }";
	z[lc++]="   NT(TBL_%1$s_buffer).tend = 1;";
	z[lc++]="   for (i=0; i < (NT(TBL_%1$s_buffer).len - EV(ACYCLE)); i++)";
	z[lc++]="     NT(TBL_%1$s_buffer).t[i]=NT(TBL_%1$s_buffer).t[i+EV(ACYCLE)];";
	z[lc++]="   while (i < NT(TBL_%1$s_buffer).len)";
	z[lc++]="     NT(TBL_%1$s_buffer).t[i++]=0.0F;";
	z[lc++]="   for (i=0;i < NT(TBL_%1$s_buffer).len;i++)";
	if (tptr->extra == NULL)
	  {
	    z[lc++]="     NT(TBL_%1$s_buffer).t[i]+= input;";
	  }
	else
	  {
	    z[lc++]="     if (i<AP1.len)";
	    z[lc++]="       NT(TBL_%1$s_buffer).t[i]+= input*AP1.t[i];";
	  }
	z[lc++]="   ret = NT(TBL_%1$s_buffer).t[0];";
	kcycassign2(tptr,&lc);
	z[lc++]="}";
	printblock(lc);
        return;
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

