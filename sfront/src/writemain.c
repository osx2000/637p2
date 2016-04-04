
/*
#    Sfront, a SAOL to C translator    
#    This file: Code generation: main loops
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
#include "parser.tab.h"


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*         void printmakeloops() and void makeloops(char)       */
/*                                                              */
/* The postscript() function in writeorc.c calls the top-level  */
/* function printmakeloops(), to print out the functions that   */
/* actually executes all instruments and instances. The real    */
/* top-level work is done by makeloops().                       */
/*                                                              */
/*______________________________________________________________*/


extern void makeloops(char);

/****************************************************************/
/*  top-level function for writemain.c -- called by writeorc.c  */
/****************************************************************/

void printmainloops(void)

{

  makeloops('a');
  makeloops('k');
  makeloops('i');
  makeloops('n');
}


extern void apassstartup(sigsym *);
extern void kpassstartup(sigsym *);
extern void ipassstartup(sigsym *);
extern void apasssasl(sigsym *, char, char *);
extern void kpasssasl(sigsym *, char, char *);
extern void ipasscounter(sigsym *, char, char *);
extern void ipasssasl(sigsym *, char, char *);
extern void apasscore(sigsym *, char, char *);
extern void kpasscore(sigsym *, char, char *);
extern void ipasscore(sigsym *, char, char *);
extern void midipass(sigsym *, char, int);
extern void apasseffects(sigsym *);
extern void kpasseffects(sigsym *);
extern void ipasseffects(sigsym *);
extern void ipassinitpass(sigsym *);
extern void makeinputbusread(void);
extern void makebufferinit(void);

/****************************************************************/
/*           wrapper code generator for main_ loops             */
/****************************************************************/

void makeloops(char c)

{
  int found = 0;
  sigsym * sptr;

  switch (c) {
  case 'a':
    fprintf(outfile,"void main_apass(ENGINE_PTR_DECLARE)\n");
    break;
  case 'k':
    fprintf(outfile,"int main_kpass(ENGINE_PTR_DECLARE)\n");
    break;
  case 'i':
    fprintf(outfile,"void main_ipass(ENGINE_PTR_DECLARE)\n");
    break;
  case 'n':
    fprintf(outfile,"void main_initpass(ENGINE_PTR_DECLARE)\n");
    break;
  default:
    internalerror("writemain.c","makeloops switch 1");
  }

  fprintf(outfile,"\n{\n");

  if (c == 'i')
    fprintf(outfile,"  int i;\n");

  if (c == 'a')
    fprintf(outfile,"  int busidx;\n");

  /* functions that need sysidx */

  if (((c == 'i') && (cin || session)) ||   /* makecontrolsys()     */
      ((c == 'i') && csasl))                /* makesaslcontrolsys() */
    found = 1;

  if (!found)
    {
      sptr = instrnametable;
      while ((sptr != NULL) && !found)
	{      
	  if (((sptr->startup) && (c == 'i')) ||  /* ipassstartup() */
	      ((sptr->effects) && (c == 'i')))  /* ipasseffects() */
	    found = 1;
	  
	  if ((((sptr->score) || (sptr->ascore))        /* sasl() */  ||
	      (csasl || sptr->dyn || sptr->midi || sptr->amidi || 
	       ((cmidi || session) && sptr->miditag))   /* core() */ ) 
	      && ((c == 'i') || (c == 'k') || (c == 'a')))
	    found = 1;

	  sptr = sptr->next;
	}
    }

  if (found)
    fprintf(outfile,"  instr_line * sysidx;\n");

  fprintf(outfile,"\n");

  if ((c == 'i') && (cin || session))
    makecontrolsys();

  if (c == 'a')
    makeinputbusread();

  sptr = instrnametable;
  while (sptr != NULL)
    {      
      if (sptr->startup)  /* startup instanced at start of execution */
	{
	  switch (c) {
	  case 'a':
	    apassstartup(sptr);
	    break;
	  case 'k':
	    kpassstartup(sptr);
	    break;
	  case 'i':
	    ipassstartup(sptr);
	    break;
	  case 'n':
	    break;
	  default:
	    internalerror("writemain.c","makeloops switch startup");
	  }
	}
      if (sptr->score)
	{
	  switch (c) {
	  case 'a':
	    apasssasl(sptr,'s',"s");
	    break;
	  case 'k':
	    kpasssasl(sptr,'s',"s");
	    break;
	  case 'i':
	    if (sptr->score > 1)
	      ipasscounter(sptr,'s',"s");
	    ipasssasl(sptr,'s',"s");
	    break;
	  case 'n':
	    break;
	  default:
	    internalerror("writemain.c","makeloops switch 2");
	  }
	}
      if (sptr->ascore)
	{
	  switch (c) {
	  case 'a':
	    apasssasl(sptr,'a',"sa");
	    break;
	  case 'k':
	    kpasssasl(sptr,'a',"sa");
	    break;
	  case 'i':
	    if (sptr->ascore > 1)
	      ipasscounter(sptr,'a',"sa");
	    ipasssasl(sptr,'a',"sa");
	    break;
	  case 'n':
	    break;
	  default:
	    internalerror("writemain.c","makeloops switch 2");
	  }
	}
      if (sptr->dyn)
	{
	  switch (c) {
	  case 'a':
	    apasscore(sptr,'d',"d");
	    break;
	  case 'k':
	    kpasscore(sptr,'d', "d");
	    break;
	  case 'i':
	    ipasscore(sptr,'d', "d");
	    break;
	  case 'n':
	    break;
	  default:
	    internalerror("writemain.c","makeloops switch 3");
	  }
	}

      /* midi file events */

      if (sptr->midi)          
	midipass(sptr, c, RELTSTAMP);

      if (sptr->amidi)          
	midipass(sptr, c, ABSTSTAMP);

      /* midi control device events -- only if instr has miditag */

      if ((cmidi || session) && sptr->miditag)
	{
	  switch (c) {
	  case 'a':
	    apasscore(sptr,'c',"cm");
	    break;
	  case 'k':
	    kpasscore(sptr,'c', "cm");
	    break;
	  case 'i':
	    ipasscore(sptr,'c', "cm");
	    break;
	  case 'n':
	    break;
	  default:
	    internalerror("writemain.c","makeloops switch 4");
	  }
	}

      /* sasl control device events */

      if (csasl)
	{
	  switch (c) {
	  case 'a':
	    apasscore(sptr,'l',"cs");
	    break;
	  case 'k':
	    kpasscore(sptr,'l', "cs");
	    break;
	  case 'i':
	    ipasscore(sptr,'l', "cs");
	    break;
	  case 'n':
	    break;
	  default:
	    internalerror("writemain.c","makeloops switch 4");
	  }
	}

      if (sptr->effects)
	{
	  switch (c) {
	  case 'a':
	    apasseffects(sptr);
	    break;
	  case 'k':
	    kpasseffects(sptr);
	    break;
	  case 'i':
	    ipasseffects(sptr);
	    break;
	  case 'n':
	    ipassinitpass(sptr);
	    break;
	  default:
	    internalerror("writemain.c","makeloops switch 5");
	  }
	}
      sptr = sptr->next;
    }

  if ((c == 'i') && csasl)
    makesaslcontrolsys();

  if (c == 'k')
    fprintf(outfile, "\n  return EV(graceful_exit);");
  if (c == 'n')
    makebufferinit();
      

  fprintf(outfile,"\n}\n\n");

}


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*                                                              */
/*    Second level functions for startup function creation      */
/*                                                              */
/*______________________________________________________________*/

/****************************************************************/
/*           writes core ipass code for sstartup instr          */
/****************************************************************/

void ipassstartup(sigsym * sptr)

{
  if (sptr->cref->conlines)
    {
      fprintf(outfile,"  sysidx = &EV(u_startup)[0];\n");
      fprintf(outfile,"  if (sysidx->noteon == PLAYING) {\n");
      fprintf(outfile,"   if (sysidx->released)\n");
      fprintf(outfile,"    {\n");
      fprintf(outfile,"     if (sysidx->turnoff)\n");
      fprintf(outfile,"      {\n");
      fprintf(outfile,"        sysidx->noteon = ALLDONE;\n");
      fprintf(outfile,"        for (i = 0; i < %s_ENDTBL; i++)\n", sptr->val);
      fprintf(outfile,"         if (sysidx->nstate->t[i].llmem)\n");
      fprintf(outfile,"           free(sysidx->nstate->t[i].t);\n");
      fprintf(outfile,"        sysidx->nstate->iline = NULL;\n"); 
      fprintf(outfile,"      }\n");
      fprintf(outfile,"     else\n");
      fprintf(outfile,"      {\n");
      fprintf(outfile,"        sysidx->abstime -= EV(KTIME);\n");
      fprintf(outfile,"        if (sysidx->abstime < 0.0F)\n");
      fprintf(outfile,"         {\n");
      fprintf(outfile,"           sysidx->noteon = ALLDONE;\n");
      fprintf(outfile,"           for (i = 0; i < %s_ENDTBL; i++)\n",
	    sptr->val);
      fprintf(outfile,"            if (sysidx->nstate->t[i].llmem)\n");
      fprintf(outfile,"              free(sysidx->nstate->t[i].t);\n"); 
      fprintf(outfile,"           sysidx->nstate->iline = NULL;\n"); 
      fprintf(outfile,"         }\n");
      fprintf(outfile,"        else\n");
      fprintf(outfile,"         sysidx->turnoff = sysidx->released = 0;\n");
      fprintf(outfile,"      }\n");
      fprintf(outfile,"    }\n");
      fprintf(outfile,"   else\n");
      fprintf(outfile,"    {\n");
      fprintf(outfile,"     if (sysidx->turnoff)\n");
      fprintf(outfile,"      {\n");
      fprintf(outfile,"       sysidx->released = 1;\n");
      fprintf(outfile,"      }\n");
      fprintf(outfile,"     else\n");
      fprintf(outfile,"      {\n");
      fprintf(outfile,"        if (sysidx->endtime <= EV(scorebeats))\n");
      fprintf(outfile,"         {\n");
      fprintf(outfile,"           if (sysidx->abstime <= 0.0F)\n");
      fprintf(outfile,"             sysidx->turnoff = sysidx->released = 1;\n");
      fprintf(outfile,"           else\n");
      fprintf(outfile,"           {\n");
      fprintf(outfile,"             sysidx->abstime -= EV(KTIME);\n");
      fprintf(outfile,"             if (sysidx->abstime < 0.0F)\n");
      fprintf(outfile,"               sysidx->turnoff = sysidx->released = 1;\n");
      fprintf(outfile,"           }\n");
      fprintf(outfile,"         }\n");
      fprintf(outfile,"        else\n");
      fprintf(outfile,"          if ((sysidx->abstime < 0.0F) &&\n");  
      fprintf(outfile,"           (1.666667e-2F*EV(tempo)*sysidx->abstime + \n");
      fprintf(outfile,"               sysidx->endtime <= EV(scorebeats)))\n");
      fprintf(outfile,"            sysidx->turnoff = sysidx->released = 1;\n");
      fprintf(outfile,"      }\n");
      fprintf(outfile,"    }\n");
      fprintf(outfile,"   }\n");
    }
}


/****************************************************************/
/*           writes kpass code for startup instrs               */
/****************************************************************/

void kpassstartup(sigsym * sptr)

{
  if (sptr->cref->conlines)
    fprintf(outfile,"  if (EV(u_startup)[0].noteon == PLAYING)\n");
  
  /* needs separate copy for u_startup to eliminate argument */
  
  fprintf(outfile,"   startup_kpass(ENGINE_PTR_COMMA &(EV(ninstr)[0]));\n");
  
}


/****************************************************************/
/*           writes apass code for startup instrs               */
/****************************************************************/

void apassstartup(sigsym * sptr)

{
  if (sptr->cref->alines)
    {
      if (sptr->cref->conlines)
	fprintf(outfile,"  if (EV(u_startup)[0].noteon == PLAYING)\n");

      /* needs separate copy for u_startup to eliminate argument */

      fprintf(outfile,"   startup_apass(ENGINE_PTR_COMMA &(EV(ninstr)[0]));\n");
    }
}



/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*                                                              */
/*  Second level functions for absolute-numbered SASL instrs    */
/*                                                              */
/*______________________________________________________________*/

/****************************************************************/
/*           writes core ipass code for sasl instr              */
/****************************************************************/

void ipasssasl(sigsym * sptr, char c, char * prefix)

{  
  char * val;

  val = dupunderscore(sptr->val);

  if ( ((c == 's') && (sptr->score > 1)) ||
       ((c == 'a') && (sptr->ascore > 1)))
    {
      fprintf(outfile,"  EV(beginflag) = 0;\n");
      fprintf(outfile," for (sysidx=EV(%s_%sfirst);sysidx<=EV(%s_%slast);sysidx++)\n"
	      ,prefix,val,prefix,val);
      fprintf(outfile,"  {\n");
    }
  else
    fprintf(outfile,"  sysidx = &EV(%s_%s)[0];\n", prefix, sptr->val);

  fprintf(outfile,"  switch(sysidx->noteon) {\n");
  fprintf(outfile,"   case PLAYING:\n");
  fprintf(outfile,"   if (sysidx->released)\n");
  fprintf(outfile,"    {\n");
  fprintf(outfile,"     if (sysidx->turnoff)\n");
  fprintf(outfile,"      {\n");
  fprintf(outfile,"        sysidx->noteon = ALLDONE;\n");
  fprintf(outfile,"        for (i = 0; i < %s_ENDTBL; i++)\n", sptr->val);
  fprintf(outfile,"         if (sysidx->nstate->t[i].llmem)\n");
  fprintf(outfile,"           free(sysidx->nstate->t[i].t);\n");
  fprintf(outfile,"        sysidx->nstate->iline = NULL;\n"); 
  fprintf(outfile,"      }\n");
  fprintf(outfile,"     else\n");
  fprintf(outfile,"      {\n");
  fprintf(outfile,"        sysidx->abstime -= EV(KTIME);\n");
  fprintf(outfile,"        if (sysidx->abstime < 0.0F)\n");
  fprintf(outfile,"         {\n");
  fprintf(outfile,"           sysidx->noteon = ALLDONE;\n");
  fprintf(outfile,"           for (i = 0; i < %s_ENDTBL; i++)\n", sptr->val);
  fprintf(outfile,"            if (sysidx->nstate->t[i].llmem)\n");
  fprintf(outfile,"             free(sysidx->nstate->t[i].t);\n");
  fprintf(outfile,"           sysidx->nstate->iline = NULL;\n"); 
  fprintf(outfile,"         }\n");
  fprintf(outfile,"        else\n");
  fprintf(outfile,"         sysidx->turnoff = sysidx->released = 0;\n");
  fprintf(outfile,"      }\n");
  fprintf(outfile,"    }\n");
  fprintf(outfile,"   else\n");
  fprintf(outfile,"    {\n");
  fprintf(outfile,"     if (sysidx->turnoff)\n");
  fprintf(outfile,"      {\n");
  fprintf(outfile,"       sysidx->released = 1;\n");
  fprintf(outfile,"      }\n");
  fprintf(outfile,"     else\n");
  fprintf(outfile,"      {\n");
  fprintf(outfile,"        if (sysidx->endtime <= EV(scorebeats))\n");
  fprintf(outfile,"         {\n");
  fprintf(outfile,"           if (sysidx->abstime <= 0.0F)\n");
  fprintf(outfile,"             sysidx->turnoff = sysidx->released = 1;\n");
  fprintf(outfile,"           else\n");
  fprintf(outfile,"           {\n");
  fprintf(outfile,"             sysidx->abstime -= EV(KTIME);\n");
  fprintf(outfile,"             if (sysidx->abstime < 0.0F)\n");
  fprintf(outfile,"               sysidx->turnoff = sysidx->released = 1;\n");
  fprintf(outfile,"           }\n");
  fprintf(outfile,"         }\n");
  fprintf(outfile,"        else\n");  
  fprintf(outfile,"          if ((sysidx->abstime < 0.0F) &&\n");  
  fprintf(outfile,"           (1.666667e-2F*EV(tempo)*sysidx->abstime + \n");
  fprintf(outfile,"               sysidx->endtime <= EV(scorebeats)))\n");
  fprintf(outfile,"            sysidx->turnoff = sysidx->released = 1;\n");
  fprintf(outfile,"      }\n");
  fprintf(outfile,"    }\n");
  fprintf(outfile,"   break;\n");
  fprintf(outfile,"   case TOBEPLAYED:\n");  
  if (c == 'a')
    fprintf(outfile,"    if (sysidx->startabs <= EV(absolutetime))\n");
  else
    fprintf(outfile,"    if (sysidx->starttime <= EV(scorebeats))\n");
  fprintf(outfile,"     {\n");
  fprintf(outfile,"      sysidx->noteon = PLAYING;\n");
  fprintf(outfile,"      sysidx->notestate = EV(nextstate);\n");
  fprintf(outfile,"      sysidx->nstate = &(EV(ninstr)[EV(nextstate)]);\n");

  if (totmidichan)
    {  
      fprintf(outfile,"      sysidx->numchan = EV(midimasterchannel);\n");
    }

  fprintf(outfile,"      if (sysidx->sdur >= 0.0F)\n");
  fprintf(outfile,"        sysidx->endtime = EV(scorebeats) + sysidx->sdur;\n");
  fprintf(outfile,"      sysidx->kbirth = EV(kcycleidx);\n");
  fprintf(outfile,"      EV(ninstr)[EV(nextstate)].iline = sysidx;\n");
  fprintf(outfile,"      sysidx->time = (EV(kcycleidx)-1)*EV(KTIME);\n");
  nextstateupdate(NULL);
  fprintf(outfile,"      %s_ipass(ENGINE_PTR_COMMA sysidx->nstate);\n",sptr->val);
  fprintf(outfile,"    }\n");
  fprintf(outfile,"   break;\n");
  fprintf(outfile,"   }\n");
  if ( ((c == 's') && (sptr->score > 1)) ||
       ((c == 'a') && (sptr->ascore > 1)))
    {
      fprintf(outfile,"   if ((!EV(beginflag)) && (sysidx->noteon == ALLDONE))\n");
      fprintf(outfile,"     EV(%s_%sfirst) = sysidx+1;\n",prefix,val);
      fprintf(outfile,"   else\n");
      fprintf(outfile,"     EV(beginflag) = 1;\n");
      fprintf(outfile," }\n");
    }

  free(val);
}

/****************************************************************/
/*           writes kpass code for sasl instrs                  */
/****************************************************************/

void kpasssasl(sigsym * sptr, char c, char * prefix)

{
  char * val;

  val = dupunderscore(sptr->val);

  if ( ((c == 's') && (sptr->score == 1)) ||
       ((c == 'a') && (sptr->ascore == 1)))
    {
      fprintf(outfile,"  if (EV(%s_%s)[0].noteon == PLAYING)\n", prefix, 
	      sptr->val);
      fprintf(outfile,"   %s_kpass(ENGINE_PTR_COMMA EV(%s_%s)[0].nstate);\n",sptr->val,
	      prefix, sptr->val);
    }
  else
    {
      fprintf(outfile," for (sysidx=EV(%s_%sfirst);sysidx<=EV(%s_%slast);sysidx++)\n"
	      ,prefix,val,prefix,val);
      fprintf(outfile,"  if (sysidx->noteon == PLAYING)\n");
      fprintf(outfile,"   %s_kpass(ENGINE_PTR_COMMA sysidx->nstate);\n",sptr->val);
    }
  
  free(val);

}

/****************************************************************/
/*           writes apass code for sasl instrs                  */
/****************************************************************/

void apasssasl(sigsym * sptr, char c, char * prefix)

{
  char * val;

  val = dupunderscore(sptr->val);

  if (sptr->cref->alines)
    {
      if ( ((c == 's') && (sptr->score == 1)) ||
	   ((c == 'a') && (sptr->ascore == 1)))
	{
	  fprintf(outfile,"  if (EV(%s_%s)[0].noteon == PLAYING)\n", 
		  prefix, sptr->val);
	  fprintf(outfile,"   %s_apass(ENGINE_PTR_COMMA EV(%s_%s)[0].nstate);\n",sptr->val,
		  prefix, sptr->val);
	}
      else
	{
	  fprintf(outfile,
		  " for (sysidx=EV(%s_%sfirst);sysidx<=EV(%s_%slast);sysidx++)\n"
		  ,prefix, val,prefix,val);
	  fprintf(outfile,"  if (sysidx->noteon == PLAYING)\n");
	  fprintf(outfile,"   %s_apass(ENGINE_PTR_COMMA sysidx->nstate);\n",sptr->val);
	}
    }
  free(val);

}



/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*                                                              */
/*  Second level functions for scorebeats-numbered SASL instrs  */
/*                                                              */
/*______________________________________________________________*/

/****************************************************************/
/*           writes ipass counter update code                  */
/****************************************************************/

void ipasscounter(sigsym * sptr, char c, char * s)


{
  char * val;

  val = dupunderscore(sptr->val);

  fprintf(outfile,"    sysidx = EV(%s_%slast);\n",s,val);
  fprintf(outfile,"    while ((sysidx <= EV(%s_%send)) && \n",s,val);
  if (c == 'a')
    fprintf(outfile,"      (sysidx->startabs <= EV(absolutetime)))\n");
  else
    fprintf(outfile,"      (sysidx->starttime <= EV(scorebeats)))\n");
  fprintf(outfile,"      {\n");
  fprintf(outfile,"       EV(%s_%slast) = sysidx;\n",s,val);
  fprintf(outfile,"       sysidx++;\n");
  fprintf(outfile,"      }\n");

  free(val);
}

/****************************************************************/
/*           writes core ipass code for instrs                  */
/****************************************************************/

void ipasscore(sigsym * sptr, char c, char * s)

{
  char * val;

  val = dupunderscore(sptr->val);

  if ((c != 'd') && (c != 'c') && (c != 'l'))
    fprintf(outfile,"  EV(beginflag) = 0;\n");
  fprintf(outfile," for (sysidx=EV(%s_%sfirst);sysidx<=EV(%s_%slast);sysidx++)\n"
	  ,s,val,s,val);
  fprintf(outfile,"  {\n");
  fprintf(outfile,"  switch(sysidx->noteon) {\n");
  fprintf(outfile,"   case PLAYING:\n");
  fprintf(outfile,"   if (sysidx->released)\n");
  fprintf(outfile,"    {\n");
  fprintf(outfile,"     if (sysidx->turnoff)\n");
  fprintf(outfile,"      {\n");
  fprintf(outfile,"        sysidx->noteon = ALLDONE;\n");
  fprintf(outfile,"        for (i = 0; i < %s_ENDTBL; i++)\n", sptr->val);
  fprintf(outfile,"         if (sysidx->nstate->t[i].llmem)\n");
  fprintf(outfile,"           free(sysidx->nstate->t[i].t);\n");
  fprintf(outfile,"        sysidx->nstate->iline = NULL;\n"); 
  fprintf(outfile,"      }\n");
  fprintf(outfile,"     else\n");
  fprintf(outfile,"      {\n");
  fprintf(outfile,"        sysidx->abstime -= EV(KTIME);\n");
  fprintf(outfile,"        if (sysidx->abstime < 0.0F)\n");
  fprintf(outfile,"         {\n");
  fprintf(outfile,"           sysidx->noteon = ALLDONE;\n"); 
  fprintf(outfile,"           for (i = 0; i < %s_ENDTBL; i++)\n", sptr->val);
  fprintf(outfile,"            if (sysidx->nstate->t[i].llmem)\n");
  fprintf(outfile,"             free(sysidx->nstate->t[i].t);\n");
  fprintf(outfile,"           sysidx->nstate->iline = NULL;\n"); 
  fprintf(outfile,"         }\n");
  fprintf(outfile,"        else\n");
  fprintf(outfile,"         sysidx->turnoff = sysidx->released = 0;\n");
  fprintf(outfile,"      }\n");
  fprintf(outfile,"    }\n");
  fprintf(outfile,"   else\n");
  fprintf(outfile,"    {\n");
  fprintf(outfile,"     if (sysidx->turnoff)\n");
  fprintf(outfile,"      {\n");
  fprintf(outfile,"       sysidx->released = 1;\n");
  fprintf(outfile,"      }\n");
  fprintf(outfile,"     else\n");
  fprintf(outfile,"      {\n");
  if ((c == 'a')||(c == 'm')||(c == 'c'))
    {
      if (c == 'a')
	fprintf(outfile,"        if ((sysidx->endabs <= EV(absolutetime)) &&\n");
      else
	fprintf(outfile,"        if ((sysidx->endtime <= EV(scorebeats)) &&\n");
      fprintf(outfile,"        ");

      if (((c == 'm')||(c == 'a')) && midiallsoundsoff)
	fprintf(outfile,"(sysidx->label || ");

      fprintf(outfile, "((NG(%i*sysidx->nstate->iline->numchan+%i) == 0.0F)",
	    MIDIFRAMELEN, MIDICTRLPOS + 64);

      fprintf(outfile, "|| (sysidx->sdur == 0.0F)))");

      if (((c == 'm')||(c == 'a')) && midiallsoundsoff)
	fprintf(outfile,")");

      fprintf(outfile,"\n");
    }
  else
    fprintf(outfile,"        if (sysidx->endtime <= EV(scorebeats))\n");

  fprintf(outfile,"         {\n");
  fprintf(outfile,"           if (sysidx->abstime <= 0.0F)\n");
  fprintf(outfile,"             sysidx->turnoff = sysidx->released = 1;\n");
  fprintf(outfile,"           else\n");
  fprintf(outfile,"           {\n");
  fprintf(outfile,"             sysidx->abstime -= EV(KTIME);\n");
  fprintf(outfile,"             if (sysidx->abstime < 0.0F)\n");
  fprintf(outfile,"               sysidx->turnoff = sysidx->released = 1;\n");
  fprintf(outfile,"           }\n");

  if (((c == 'm')||(c == 'a')) && midiallsoundsoff)
    {
      fprintf(outfile,"           if (sysidx->label)\n");
      fprintf(outfile,"           {\n");
      fprintf(outfile,"             sysidx->noteon = ALLDONE;\n");
      fprintf(outfile,"             for (i = 0; i < %s_ENDTBL; i++)\n", sptr->val);
      fprintf(outfile,"              if (sysidx->nstate->t[i].llmem)\n");
      fprintf(outfile,"                free(sysidx->nstate->t[i].t);\n");
      fprintf(outfile,"             sysidx->nstate->iline = NULL;\n"); 
      fprintf(outfile,"           }\n");
    }

  fprintf(outfile,"         }\n");
  fprintf(outfile,"        else\n");
  if (c == 'a')
    {
      fprintf(outfile,"          if ((sysidx->abstime < 0.0F) &&\n");  
      fprintf(outfile,"           (sysidx->abstime + sysidx->endabs\n");
      fprintf(outfile,"               <= EV(absolutetime)))\n");
    }
  else
    {
      fprintf(outfile,"          if ((sysidx->abstime < 0.0F) &&\n");  
      fprintf(outfile,"           (1.666667e-2F*EV(tempo)*sysidx->abstime + \n");
      fprintf(outfile,"               sysidx->endtime <= EV(scorebeats)))\n");
    }
  fprintf(outfile,"            sysidx->turnoff = sysidx->released = 1;\n");
  fprintf(outfile,"      }\n");
  fprintf(outfile,"    }\n");
  fprintf(outfile,"   break;\n");
  fprintf(outfile,"   case TOBEPLAYED:\n");
  if (c == 'a')
    fprintf(outfile,"    if (sysidx->startabs <= EV(absolutetime))\n");
  else
    fprintf(outfile,"    if (sysidx->starttime <= EV(scorebeats))\n");
  fprintf(outfile,"     {\n");
  fprintf(outfile,"      sysidx->noteon = PLAYING;\n");
  fprintf(outfile,"      sysidx->notestate = EV(nextstate);\n");
  fprintf(outfile,"      sysidx->nstate = &(EV(ninstr)[EV(nextstate)]);\n");

  if (totmidichan && ((c == 'd') || (c == 'l')))
    fprintf(outfile,"      sysidx->numchan = EV(midimasterchannel);\n");

  fprintf(outfile,"      if (sysidx->sdur >= 0.0F)\n");
  fprintf(outfile,"        sysidx->endtime = EV(scorebeats) + sysidx->sdur;\n");
  fprintf(outfile,"      sysidx->kbirth = EV(kcycleidx);\n");
  fprintf(outfile,"      EV(ninstr)[EV(nextstate)].iline = sysidx;\n");
  fprintf(outfile,"      sysidx->time = (EV(kcycleidx)-1)*EV(KTIME);\n");
  nextstateupdate(NULL);
  fprintf(outfile,"      %s_ipass(ENGINE_PTR_COMMA sysidx->nstate);\n",sptr->val);
  fprintf(outfile,"    }\n");
  fprintf(outfile,"   break;\n");
  if (c == 'd')
    {
      fprintf(outfile,"   case PAUSED:\n");
      fprintf(outfile,"      sysidx->noteon = PLAYING;\n");
      fprintf(outfile,"      sysidx->kbirth = EV(kcycleidx);\n");
      fprintf(outfile,"   break;\n");
    }
  fprintf(outfile,"   default:\n");
  fprintf(outfile,"   break;\n");
  fprintf(outfile,"   }\n");
  if ((c != 'd')&&(c != 'c')&&(c != 'l'))
    {
      fprintf(outfile,"   if ((!EV(beginflag)) && (sysidx->noteon == ALLDONE))\n");
      fprintf(outfile,"     EV(%s_%sfirst) = sysidx+1;\n",s,val);
      fprintf(outfile,"   else\n");
      fprintf(outfile,"     EV(beginflag) = 1;\n");
    }
  fprintf(outfile," }\n");
  if ((c == 'd')||(c == 'c')||(c == 'l'))
    {
      fprintf(outfile,"  while (EV(%s_%slast)->noteon == ALLDONE)\n",s,val);
      fprintf(outfile,"   if (EV(%s_%slast) == EV(%s_%sfirst))\n",s,val,s,val);
      fprintf(outfile,"    {\n");
      fprintf(outfile,"     EV(%s_%sfirst) = &EV(%s_%s)[1];\n",s,val,s,sptr->val);
      fprintf(outfile,"     EV(%s_%slast) =  &EV(%s_%s)[0];\n",s,val,s,sptr->val);
      fprintf(outfile,"     EV(%s_%snext) =  NULL;\n",s,val);
      fprintf(outfile,"     EV(%s_%slast)->noteon = TOBEPLAYED;\n",s,val);
      fprintf(outfile,"     break;\n");
      fprintf(outfile,"    }\n");
      fprintf(outfile,"   else\n");
      fprintf(outfile,"    EV(%s_%slast)--;\n",s,val);
    }

  free(val);
}

/****************************************************************/
/*           writes core kpass code for instrs                  */
/****************************************************************/

void kpasscore(sigsym * sptr, char c, char * s)

{
  char * val;

  val = dupunderscore(sptr->val);

  fprintf(outfile," for (sysidx=EV(%s_%sfirst);sysidx<=EV(%s_%slast);sysidx++)\n"
	  ,s,val,s,val);
  fprintf(outfile,"  if (sysidx->noteon == PLAYING)\n");
  fprintf(outfile,"  {\n");
  if (c == 'd')
    fprintf(outfile,"   sysidx->launch = LAUNCHED;\n");
  fprintf(outfile,"   %s_kpass(ENGINE_PTR_COMMA sysidx->nstate);\n",sptr->val);
  fprintf(outfile,"  }\n");
 
  free(val);
}

/****************************************************************/
/*           writes core apass code for instrs                  */
/****************************************************************/

void apasscore(sigsym * sptr, char c, char * s)

{
  char * val;

  val = dupunderscore(sptr->val);

  if (sptr->cref->alines)
    {

      fprintf(outfile," for (sysidx=EV(%s_%sfirst);sysidx<=EV(%s_%slast);sysidx++)\n"
	      ,s,val,s,val);
      if (c == 'd')
	{
	  fprintf(outfile,
       "  if ((sysidx->noteon == PLAYING) &&(sysidx->launch == LAUNCHED))\n");
	}
      else
	fprintf(outfile,"  if (sysidx->noteon == PLAYING)\n");

      fprintf(outfile,"   %s_apass(ENGINE_PTR_COMMA sysidx->nstate);\n",sptr->val);

    }
  free(val);
}



/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*                                                              */
/*         Second level functions for MIDI instrs               */
/*                                                              */
/*______________________________________________________________*/

/****************************************************************/
/*       writes core routines for midi-triggered instrs         */
/****************************************************************/

void midipass(sigsym * sptr, char c, int type)

{
  char * prefix;
  tnode * tptr;
  int i;
  char iname[32];
  char ccode;

  if (type == RELTSTAMP)
    {
      tptr = confmidi->imidiroot;
      i = sptr->midi;
      prefix = "m";
      ccode = 'm';
    }
  else
    {
      tptr = sstrmidi->imidiroot;
      i = sptr->amidi;
      prefix = "ma";
      ccode = 'a';
    }

  while ((tptr != NULL)&&(i > 0))
    {
      if (tptr->sptr != sptr)
        {
          tptr = tptr->next;
          continue;
        }
      sprintf(iname,"%s%i",prefix,tptr->special);
      switch (c) {
      case 'a':
	apasscore(sptr, 'm',iname);
	break;
      case 'k':
	kpasscore(sptr,'m', iname);
	break;
      case 'i':
	ipasscounter(sptr,ccode, iname);
	ipasscore(sptr,ccode, iname);
	break;
      default:
	break;
      }
      i--;
      tptr = tptr->next;
    }
}



/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*                                                              */
/*         Second level functions for effects instrs            */
/*                                                              */
/*______________________________________________________________*/

/****************************************************************/
/*           writes initpass code for effects instrs            */
/****************************************************************/

void ipassinitpass(sigsym * sptr)

{

  tnode * tptr = instances;
  int i = 0;
  
  while (tptr != NULL)
    {
      if (!strcmp(tptr->sptr->val, sptr->val))
	{
	  fprintf(outfile,"  EV(e_%s)[%i].noteon = PLAYING;\n", sptr->val,i);
	  fprintf(outfile,"  EV(e_%s)[%i].notestate = EV(nextstate);\n", sptr->val,i);
	  fprintf(outfile,"  EV(e_%s)[%i].endtime = MAXENDTIME;\n", sptr->val,i);
	  fprintf(outfile,"  EV(e_%s)[%i].nstate = &(EV(ninstr)[EV(nextstate)]);\n",
		  sptr->val,i);
	  fprintf(outfile,"  EV(ninstr)[EV(nextstate)].iline = &EV(e_%s)[%i];\n",
		  sptr->val,i);
	  nextstateupdate(NULL);
	  fprintf(outfile,"   %s_ipass(ENGINE_PTR);\n", tptr->val);
	  fprintf(outfile,"\n");
	  i++;
	}
      tptr = tptr->next;
    }

}

/****************************************************************/
/*           writes core ipass code for effects instrs          */
/****************************************************************/

void ipasseffects(sigsym * sptr)
     
{
  
  tnode * tptr = instances;
  int i = 0;
  
  while (tptr != NULL)
    {
      if (!strcmp(tptr->sptr->val, sptr->val))
	{
	  if (tptr->sptr->cref->conlines)
	    {
	      fprintf(outfile,"  sysidx = &EV(e_%s)[%i];\n", sptr->val,i);
	      fprintf(outfile,"  if (sysidx->noteon == PLAYING) {\n");
	      fprintf(outfile,"   if (sysidx->released)\n");
	      fprintf(outfile,"    {\n");
	      fprintf(outfile,"     if (sysidx->turnoff)\n");
	      fprintf(outfile,"      {\n");
	      fprintf(outfile,"        sysidx->noteon = ALLDONE;\n");
	      fprintf(outfile,"        sysidx->nstate->iline = NULL;\n"); 
	      fprintf(outfile,"      }\n");
	      fprintf(outfile,"     else\n");
	      fprintf(outfile,"      {\n");
	      fprintf(outfile,"        sysidx->abstime -= EV(KTIME);\n");
	      fprintf(outfile,"        if (sysidx->abstime < 0.0F)\n");
	      fprintf(outfile,"         {\n");
	      fprintf(outfile,"           sysidx->noteon = ALLDONE;\n"); 
	      fprintf(outfile,"           sysidx->nstate->iline = NULL;\n"); 
	      fprintf(outfile,"         }\n");
	      fprintf(outfile,"        else\n");
	      fprintf(outfile,"         sysidx->turnoff = sysidx->released = 0;\n");
	      fprintf(outfile,"      }\n");
	      fprintf(outfile,"    }\n");
	      fprintf(outfile,"   else\n");
	      fprintf(outfile,"    {\n");
	      fprintf(outfile,"     if (sysidx->turnoff)\n");
	      fprintf(outfile,"      {\n");
	      fprintf(outfile,"       sysidx->released = 1;\n");
	      fprintf(outfile,"      }\n");
	      fprintf(outfile,"     else\n");
	      fprintf(outfile,"      {\n");
	      fprintf(outfile,"        if (sysidx->endtime <= EV(scorebeats))\n");
	      fprintf(outfile,"         {\n");
	      fprintf(outfile,"           if (sysidx->abstime <= 0.0F)\n");
	      fprintf(outfile,"             sysidx->turnoff = sysidx->released = 1;\n");
	      fprintf(outfile,"           else\n");
	      fprintf(outfile,"           {\n");
	      fprintf(outfile,"             sysidx->abstime -= EV(KTIME);\n");
	      fprintf(outfile,"             if (sysidx->abstime < 0.0F)\n");
	      fprintf(outfile,"               sysidx->turnoff = sysidx->released = 1;\n");
	      fprintf(outfile,"           }\n");
	      fprintf(outfile,"         }\n");
	      fprintf(outfile,"        else\n");
	      fprintf(outfile,"          if ((sysidx->abstime < 0.0F) &&\n");  
	      fprintf(outfile,"           (1.666667e-2F*EV(tempo)*sysidx->abstime + \n");
	      fprintf(outfile,"               sysidx->endtime <= EV(scorebeats)))\n");
	      fprintf(outfile,"            sysidx->turnoff = sysidx->released = 1;\n");
	      fprintf(outfile,"      }\n");
	      fprintf(outfile,"    }\n");
	      fprintf(outfile,"   }\n");
	    }
	  i++;
	}
      tptr = tptr->next;
    }

}

/****************************************************************/
/*           writes core kpass code for effects instrs          */
/****************************************************************/

void kpasseffects(sigsym * sptr)

{

  tnode * tptr = instances;
  int i = 0;
  
  while (tptr != NULL)
    {
      if (!strcmp(tptr->sptr->val, sptr->val))
	{
	  if (tptr->sptr->cref->conlines)
	    fprintf(outfile,"  if (EV(e_%s)[%i].noteon == PLAYING)\n",
		    sptr->val,i);
	  fprintf(outfile,"   %s_kpass(ENGINE_PTR);\n", tptr->val);
	  i++;
	}
      tptr = tptr->next;
    }

}

/****************************************************************/
/*           writes core apass code for effects instrs          */
/****************************************************************/

void apasseffects(sigsym * sptr)

{

  tnode * tptr = instances;
  int i = 0;
  
  while (tptr != NULL)
    {
      if (!strcmp(tptr->sptr->val, sptr->val))
	{
	  if (tptr->sptr->cref->alines)
	    {
	      if (tptr->sptr->cref->conlines)
		fprintf(outfile,"  if (EV(e_%s)[%i].noteon == PLAYING)\n",
			sptr->val,i);
	      fprintf(outfile,"   %s_apass(ENGINE_PTR);\n", tptr->val);
	    }
	  i++;
	}
      tptr = tptr->next;
    }

}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*                                                              */
/*            Miscellaneous Second level functions.             */
/*                                                              */
/*______________________________________________________________*/

/****************************************************************/
/*       reads stdin for latest input_bus data                  */
/****************************************************************/

void makeinputbusread(void)

{

  if ((getvsym(&busnametable,"input_bus") != NULL) && (inchannels > 0) && 
      (ainflow == PASSIVE_FLOW) && (aoutflow == PASSIVE_FLOW))
    {
      fprintf(outfile,"   if (EV(ibusidx) < EV(asys_isize))\n");
      fprintf(outfile,
	      "    for(busidx=BUS_input_bus;busidx<BUS_input_bus+%i;busidx++)\n",
	      inchannels);
      switch (makeaudiotypein(ain)) {
      case SAMPLE_SHORT:
	fprintf(outfile,
		"      TB(busidx) = 3.051851e-5F*EV(asys_ibuf)[EV(ibusidx)++];\n");
	break;
      case SAMPLE_FLOAT:
	fprintf(outfile,
		"      TB(busidx) = EV(asys_ibuf)[EV(ibusidx)++];\n");
	break;
      }
      fprintf(outfile,"   else\n");
      fprintf(outfile,"   {\n");
      fprintf(outfile,"    EV(ibusidx) = 0;\n");
      fprintf(outfile,"    if (asys_getbuf(&EV(asys_ibuf), &EV(asys_isize))"
	                       "||(!EV(asys_isize)))\n");
      fprintf(outfile,"     {\n");
      fprintf(outfile,"       EV(asys_isize) = 0;\n");
      fprintf(outfile,
	      "       for(busidx=BUS_input_bus;busidx<BUS_input_bus+%i;busidx++)\n",
	      inchannels);
      fprintf(outfile,"       TB(busidx) = 0.0F;\n");
      fprintf(outfile,"       EV(kcycleidx) = EV(endkcycle);\n");
      fprintf(outfile,"      }\n");
      fprintf(outfile,"    else\n");
      fprintf(outfile,
	      "      for(busidx=BUS_input_bus;busidx<BUS_input_bus+%i;busidx++)\n",
	      inchannels);
      switch (makeaudiotypein(ain)) {
      case SAMPLE_SHORT:
	fprintf(outfile,
		"      TB(busidx) = 3.051851e-5F*EV(asys_ibuf)[EV(ibusidx)++];\n");
	break;
      case SAMPLE_FLOAT:
	fprintf(outfile,
		"      TB(busidx) = EV(asys_ibuf)[EV(ibusidx)++];\n");
	break;
      }
      fprintf(outfile,"   }\n");
    }

}

/****************************************************************/
/*       does initialization for buffer and synchronication     */
/****************************************************************/

void makebufferinit(void)

{
  sigsym * iptr;

  fprintf(outfile, "\n");
  iptr = getvsym(&busnametable,"input_bus");

  if (session)
    {
      fprintf(outfile, "   if (nsys_setup(NSYS_NONBLOCK) == NSYS_ERROR)\n");
      gened(NULL,"network setup failure");
      fprintf(outfile, "\n");
    }

  if (cin)
    {
      fprintf(outfile, "   if (csys_setup(ENGINE_PTR) != CSYS_DONE)\n");
      gened(NULL,"control input device unavailable");
      fprintf(outfile, "\n");
    }

  if ((ain == aout) && (iptr != NULL) && (inchannels > 0))
    {
      fprintf(outfile, "   if (asys_iosetup((int) EV(ARATE), ASYS_ICHAN, ");
      fprintf(outfile, "ASYS_OCHAN, ASYS_ITYPENAME, ASYS_OTYPENAME,");
      fprintf(outfile, "\"%s\", \"%s\",", ainname, aoutname);
      fprintf(outfile, "ASYS_TIMEOPTION) != ASYS_DONE)\n");
      gened(NULL,"audio i/o device unavailable");
    }
  else
    {
      fprintf(outfile, "   if (asys_osetup((int) EV(ARATE), ASYS_OCHAN, ");
      fprintf(outfile, "ASYS_OTYPENAME, ");
      fprintf(outfile, " \"%s\",\n", aoutname);
      fprintf(outfile, "ASYS_TIMEOPTION) != ASYS_DONE)\n");
      gened(NULL,"audio output device unavailable");
      if ((iptr != NULL) && (inchannels > 0))
	{
	  fprintf(outfile, "   if (asys_isetup((int) EV(ARATE), ASYS_ICHAN, ");
	  fprintf(outfile, "ASYS_ITYPENAME, ");
	  fprintf(outfile, " \"%s\",", ainname);
	  fprintf(outfile, "ASYS_TIMEOPTION) != ASYS_DONE)\n");
	  gened(NULL,"audio input device unavailable");
	}
    }

  if (allsasl->endtimeval)
    {
      fprintf(outfile, "   EV(endkcycle) = EV(kbase) + (int) \n");
      fprintf(outfile, 
	      "      (EV(KRATE)*(EV(endtime) - EV(scorebase))*(60.0F/EV(tempo)));\n\n");
      if (abssasl->endtimeval)
	{
	  fprintf(outfile, 
		  "   EV(endkcycle) = (EV(endkinit) > EV(endkcycle)) ?\n");
	  fprintf(outfile, 
		  "                EV(endkinit) : EV(endkcycle);\n\n");
	}
    }
  else
    {
      fprintf(outfile, "   EV(endkcycle) = EV(endkinit);\n\n");
    }

  if ((ainflow == PASSIVE_FLOW) && (aoutflow == PASSIVE_FLOW))
    {

      /* ordering tentative, pending testing */

      if ((iptr != NULL) && (inchannels > 0))
	{
	  fprintf(outfile, " if "
		  "((asys_getbuf(&EV(asys_ibuf), &EV(asys_isize)) != ASYS_DONE)\n");
	  fprintf(outfile,
		  "     || (!EV(asys_isize)))\n");
	  gened(NULL,"audio input device unavailable");
	}

      fprintf(outfile, 
	      "   if (asys_preamble(&EV(asys_obuf), &EV(asys_osize)) != ASYS_DONE)\n");
      gened(NULL,"audio output device unavailable");
    }

  fprintf(outfile, "   ksyncinit();\n\n");

}

