
/*
#    Sfront, a SAOL to C translator    
#    This file: Code generator for SAOL parse tree
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

/****************************************************************/
/*             instr code generation routines                   */
/****************************************************************/


/****************************************************************/
/*             service routine: for updating nextstate          */
/*       makes sure deleted instruments aren't effects          */
/****************************************************************/

void nextstateupdate(int * lcptr)

{
  int lc;

  lc = lcptr ? (*lcptr) : 0;

  /* note a table memory leak happens if ALLDONE is set. this */
  /* can be avoided by looping through MAXTABLESTATE tables,  */
  /* or by storing table width in iline.                      */

  z[lc++]="    EV(oldstate) = EV(nextstate);";
  z[lc++]="    EV(nextstate) = (EV(nextstate)+1) % MAXSTATE;";
  z[lc++]="    while ((EV(oldstate) != EV(nextstate)) && ";
  z[lc++]="           (EV(ninstr)[EV(nextstate)].iline != NULL))";
  z[lc++]="      EV(nextstate) = (EV(nextstate)+1) % MAXSTATE;";
  z[lc++]="    if (EV(oldstate) == EV(nextstate))";
  z[lc++]="    {";
  z[lc++]="      EV(nextstate) = (EV(nextstate)+1) % MAXSTATE;";
  z[lc++]="      while ((EV(oldstate) != EV(nextstate)) && ";
  z[lc++]="          (EV(ninstr)[EV(nextstate)].iline->time == 0.0F) &&";
  z[lc++]="          (EV(ninstr)[EV(nextstate)].iline->noteon == PLAYING))";
  z[lc++]="        EV(nextstate) = (EV(nextstate)+1) % MAXSTATE;";
  z[lc++]="      EV(ninstr)[EV(nextstate)].iline->noteon = ALLDONE;";
  z[lc++]="      EV(ninstr)[EV(nextstate)].iline = NULL;";
  z[lc++]="    }";

  if (lcptr)
    *lcptr = lc;
  else
    printraw(lc);
}


/****************************************************************/
/*       prints NV/NVI/NG/NGI as appropriate for variable       */
/****************************************************************/


void macroselect(tnode * tptr)

{

  /* first check special cases */
  /* later handle currintprint */

  /* tables */
  
  if (tptr->vartype == TABLETYPE)
    {
      fprintf(outfile,"NVI(%s_%s",
	      currinstancename, tptr->val);
      return;
    }

  /* import/export signal variables */

  if (tptr->sptr && 
      (tptr->sptr->tref->mirror == GLOBALMIRROR))
    {
      switch(tptr->res) {
      case ASFLOAT:
	fprintf(outfile,"NG(GBL_%s",tptr->val);
	break;
      case ASINT:
	fprintf(outfile,"NGI(GBL_%s",tptr->val);
	break;
      }
      return;
    }

  /* user-defined opcode parameters */

  if (tptr->sptr && 
      (tptr->sptr->tref->mirror == OPCODEMIRROR))
    {
      switch(tptr->res) {
      case ASFLOAT:
	fprintf(outfile,"OSP_%s_%s(0",currinstancename,tptr->val);
	break;
      case ASINT:
	fprintf(outfile,"OSPI_%s_%s(0",currinstancename,tptr->val);
	break;
      }
      return;
    }

  /* normal case */

  switch(tptr->res) {
  case ASFLOAT:
    fprintf(outfile,"NV(%s_%s",
	    currinstancename,tptr->val);
    break;
  case ASINT:
    fprintf(outfile,"NVI(%s_%s",
	    currinstancename,tptr->val);
    break;
  }

  return;

}


/****************************************************************/
/*             output statement code generation                 */
/****************************************************************/


void outputcode(tnode * tbranch)

{                   /* OUTPUT LP exprlist RP SEM */
  tnode * tptr ;
  tnode * bptr;
  int i;
  int mono;
  int tcount = 0;  /* initialization not needed */

  currscalarflag = 0;   /* later set special cases to 1 */
  bptr = currinstrument->obus;
  while (bptr != NULL)  /* for each bus output() routes to */
    {
      tptr = tbranch->down;
      while (tptr->ttype != S_EXPRLIST)
	tptr = tptr->next;
      tptr = tptr->down;
      if ((tptr->next == NULL)&&(tptr->width == 1))
	mono = 1;
      else
	{
	  mono = 0;
	  tcount = tptr->width;
	}
      for (i=bptr->res;i < (bptr->res + bptr->width); i++)
	{
	  if ((!mono)&&(tcount==0))
	    {
	      while ((tptr->next != NULL)&&(tptr->next->ttype != S_EXPR))
		tptr = tptr->next;
	      tptr = tptr->next;
	      if (tptr == NULL)
		{
		  printf("Error: Mismatch in output statement width.\n\n");
		  showerrorplace(tbranch->down->linenum, 
				 tbranch->down->filename);
		}
	      else
		tcount = tptr->width;
	    }
	  
	  /* once an instr sent the output_bus, prevent output_bus writes */

	  if ((bptr->sptr != outputbus) || (currinstrument->outputbus))
	    fprintf(outfile,"TB(BUS_%s + %i) += ", bptr->val, i);

	  if (mono)
	    currarrayindex = i - bptr->res;
	  else
	    currarrayindex = tptr->width - tcount;
	  blocktree(tptr->down, PRINTTOKENS);
	  fprintf(outfile,";\n");
	  tcount--;
	}
      currarrayindex = 0;
      bptr = bptr->next;
    }
  currscalarflag = 1;
}

/****************************************************************/
/*             outbus statement code generation                 */
/****************************************************************/


void outbuscode(tnode * tbranch)

{                  /* OUTBUS LP IDENT COM exprlist RP SEM  */
  tnode * tptr ;
  sigsym * busptr;
  int i;
  int mono;
  int tcount = 0;  /* initialization not needed */

  currscalarflag = 0;   /* later set special cases to 1 */

  if (outputbusinstance && currinstrument && 
      (!strcmp(outputbusinstance->sptr->val, currinstrument->val)))
    {
      printf("Error: No outbus() permitted in instr sent output_bus.\n\n");
      showerrorplace(tbranch->down->linenum, 
		     tbranch->down->filename);
    }

  busptr = getvsym(&busnametable,tbranch->down->next->next->val);
  tptr = tbranch->down->next->next->next->next->down;
  while (tptr->ttype != S_EXPR)
    tptr = tptr->next;
  if ((tptr->next == NULL)&&(tptr->width == 1))
    mono = 1;
  else
    {
      mono = 0;
      tcount = tptr->width;
    }
  for (i=0;i < busptr->width; i++)
    {
      if ((!mono)&&(tcount==0))
	{
	  while ((tptr->next != NULL)&&(tptr->next->ttype != S_EXPR))
	    tptr = tptr->next;
	  tptr = tptr->next;
	  if (tptr == NULL)
	    {
	      printf("Error: Mismatch in outbus statement width.\n\n");
	      showerrorplace(tbranch->down->linenum, 
			     tbranch->down->filename);
	    }
	  else
	    tcount = tptr->width;
	}
      fprintf(outfile,"TB(BUS_%s + %i) += ", busptr->val, i);
      if (mono)
	currarrayindex = i;
      else
	currarrayindex = tptr->width - tcount;
      blocktree(tptr->down, PRINTTOKENS);
      fprintf(outfile,";\n");
      tcount--;
    }
  currarrayindex = 0;
  currscalarflag = 1;

}


/****************************************************************/
/*             turnoff statement code generation                 */
/****************************************************************/


void turnoffcode(tnode * tbranch)

{                  /* TURNOFF SEM  */

  if (currinstance && (currinstance == outputbusinstance))
    {      
      printf("Error: turnoff statement not permitted in instr" 
	     " sent output_bus.\n\n");
      showerrorplace(tbranch->down->linenum, 
		     tbranch->down->filename);
    }

  fprintf(outfile,"  if (!NS(iline->released))\n"); 
  fprintf(outfile,"    NS(iline->turnoff) = 1;\n"); 

}


/****************************************************************/
/*             tmapidx statement code generation                */
/*                                                              */
/*                  S_TMAPIDX IDENT1 IDENT2 <expr> ;            */
/*                                                              */
/*  IDENT1 is the name of the tablemap                          */
/*  IDENT2 is the name of the new K_INTERNAL pointer            */
/*  <expr> is the tablemap index expression                     */
/****************************************************************/

void tmapidxcode(tnode * tptr)

{                  
  char tablename[STRSIZE];

  /* table name is the K_INTERNAL pointer */

  sprintf(tablename,"NVPS(%s_%s)", currinstancename,
	  tptr->down->next->next->val);

  printtmapcase(tptr->down->next, tptr->down->next->next->next, tablename);

}


/****************************************************************/
/*             spatialize statement code generation             */
/****************************************************************/

void spatializecode(tnode * tbranch)

{                  
  fprintf(outfile,"%s__sym_%s%i(NSP);\n",
	  curropcodeprefix,tbranch->down->val, 
	  tbranch->down->optr->arrayidx);
 
}

/****************************************************************/
/*             extend statement code generation                 */
/****************************************************************/

void extendcode(tnode * tbranch)

{                  /* EXTEND LP expr RP SEM */

  if (currinstance && (currinstance == outputbusinstance))
    {      
      warningmessage(tbranch->down, "extend statement not executed in instr"
		     " sent the output bus");
      return;
    }

  fprintf(outfile,"  if ((NS(iline->sdur) < 0.0F)||\n");
  fprintf(outfile,"   (NS(iline->turnoff)&&NS(iline->released)))\n");
  
  fprintf(outfile,"  {\n");
  fprintf(outfile,"    NS(iline->endtime) = EV(scorebase);\n");
  fprintf(outfile,"    NS(iline->endabs) = (EV(kbase) - 1)*EV(KTIME);\n");
  fprintf(outfile,"    NS(iline->sdur) = 0.0F;\n");
  fprintf(outfile,"  }\n");
  fprintf(outfile,"  NS(iline->abstime) += \n"); 
  blocktree(tbranch->down->next->next->down,PRINTTOKENS);
  fprintf(outfile,"  ;\n   if (NS(iline->released))\n"); 
  fprintf(outfile,"    NS(iline->turnoff) = 0;\n"); 
  printdurassign();

}

/****************************************************************/
/*             printf statement code generation                 */
/****************************************************************/

void printfcode(tnode * tbranch)

{                  /* PRINTF LP exprstrlist RP SEM */
  tnode * tptr;

  if (wiretap_logging(aout))
    {
      fprintf(outfile, "\n#if (ASYS_AUDIOUNIT_DEBUG_LEVEL >= 1)\n");
      fprintf(outfile, 
      "  asysn_audiounit_logfile = fopen(ASYS_AUDIOUNIT_LOGFILE_NAME, \"a\");\n");
    }

  tptr = tbranch->down->next->next->down;

  if (stdoutused(aout))
    fprintf(outfile,"  fprintf(stderr, \"%s\"", tptr->val);
  else
    if (wiretap_logging(aout))
      fprintf(outfile,"  fprintf(asysn_audiounit_logfile, \"%s\"", tptr->val);
    else
      fprintf(outfile,"  fprintf(stdout, \"%s\"", tptr->val);

  tptr = tptr->next;
  while (tptr)
    {
      if (tptr->ttype == S_EXPR)
	{
	  fprintf(outfile,", ");
	  blocktree(tptr->down,PRINTTOKENS);
	}
      if (tptr->ttype == S_STRCONST)
	{
	  fprintf(outfile,", \"%s\"", tptr->val);
	}
      tptr = tptr->next;
    }
  fprintf(outfile,");\n");

  if (wiretap_logging(aout))
    {
      fprintf(outfile, "  fclose(asysn_audiounit_logfile);\n");
      fprintf(outfile, "#endif /* (ASYS_AUDIOUNIT_DEBUG_LEVEL >= 1) */\n");
    }
}

/****************************************************************/
/*             lvalue statement code generation                 */
/****************************************************************/

void lvaluecode(tnode * tbranch)


{                  
  tnode * tptr = tbranch;          /* lvalue EQ expr SEM */
  int round;

  if (truewidth(tptr->width) > 1)
    currscalarflag = 0;
  else
    currscalarflag = 1;

  round = ((tptr->down->res == ASINT) &&
	   (tptr->down->next->next->res == ASFLOAT));

  for (currarrayindex=0;currarrayindex<truewidth(tptr->width);currarrayindex++)
    {
      /* first do lval */

      blocktree(tptr->down->down,PRINTTOKENS);

      /* then do EQ */

      fprintf(outfile," = ");

      /* then do EXPR */

      /* ROUND calls expr twice, but OK for now */
      /* because this code never runs. Tmp    */
      /* variable or some other trick needed  */
      /* to make sure expr only evals once.   */

      if (round)
	fprintf(outfile," ROUND(");

      blocktree(tptr->down->next->next->down,PRINTTOKENS);

      if (round)
	fprintf(outfile,")");

      /* then do SEM */

      fprintf(outfile,";\n ");

    }

  currarrayindex = 0;
  currscalarflag = 1;

}

/****************************************************************/
/*         if-else slower-statement code generation             */
/****************************************************************/

void ifslowercode(tnode * tptr, tnode * ifptr)

{
  int currblocksafe;

  fprintf(outfile, "\n");

  if (ifptr->inwidth & KRATESECTION)
    {
      fprintf(outfile, "   if (NVI(%s__sym_if%i) != EV(kcycleidx))\n",
	      currinstancename, ifptr->arrayidx);
      fprintf(outfile, "   {\n");
    }

  if (ifptr->inwidth & IRATESECTION)
    {
      fprintf(outfile, "   if (NVI(%s__sym_if%i) == 0)\n",
	      currinstancename, ifptr->arrayidx);
      fprintf(outfile, "   {\n");

      currblocksafe = currblockrate;
      currblockrate = IRATETYPE;
      blocktree(tptr->down, PRINTIPASS);
      currblockrate = currblocksafe;

      if ((ifptr->inwidth & KRATESECTION) == 0)
	fprintf(outfile, "   NVI(%s__sym_if%i) = EV(kcycleidx);\n",
		currinstancename, ifptr->arrayidx);
      fprintf(outfile, "   }\n");
    }

  if (ifptr->inwidth & KRATESECTION)
    {
      currblocksafe = currblockrate;
      currblockrate = KRATETYPE;
      blocktree(tptr->down, PRINTKPASS); 
      currblockrate = currblocksafe;

      fprintf(outfile, "   NVI(%s__sym_if%i) = EV(kcycleidx);\n",
	      currinstancename, ifptr->arrayidx);
      fprintf(outfile, "   }\n");
    }

}


/****************************************************************/
/*             if-else statement code generation                */
/****************************************************************/

void ifcode(tnode * tbranch, int mode)


{                  
  tnode * tptr = tbranch;          
  tnode * cptr, * ifptr;

  /*  IF LP expr RP LC block RC */
  /*  IF LP expr RP LC block RC ELSE LC block RC */

  /* IRATESECTION; KRATESECTION */

  ifptr = tptr->down;

  /* do guard as PRINTTOKENS */

  cptr = tptr->down->next->next->next->next->next;
  tptr->down->next->next->next->next->next = NULL;
  blocktree(tptr->down,PRINTTOKENS); 
  tptr->down->next->next->next->next->next = cptr;

  /* do IF block as mode */

  tptr = cptr;
  cptr = tptr->next;
  tptr->next = NULL;

  if (ifptr->inwidth)
    ifslowercode(tptr, ifptr);

  blocktree(tptr->down,mode); 
  tptr->next = cptr;

  /* do upto ELSE block as PRINTTOKENS */

  tptr = cptr;
  if (tptr->next == NULL) /* no ELSE */
    blocktree(tptr,PRINTTOKENS); 
  else
    {  
      ifptr = cptr->next;

      cptr = tptr->next->next->next;
      tptr->next->next->next = NULL;
      blocktree(tptr,PRINTTOKENS); 
      tptr->next->next->next = cptr;
      tptr = cptr;
      cptr = tptr->next;
      tptr->next = NULL;

      if (ifptr->inwidth)
	ifslowercode(tptr, ifptr);

      blocktree(tptr->down,mode); 
      tptr->next = cptr;
      tptr = cptr;
      blocktree(tptr,PRINTTOKENS);
    }
  
}

/****************************************************************/
/*             if-else statement code generation                */
/****************************************************************/

void whilecode(tnode * tbranch, int mode)


{                  
  tnode * tptr = tbranch;          
  tnode * cptr;

  /*  WHILE LP expr RP LC block RC  */

  /* do guard as PRINTTOKENS */

  cptr = tptr->down->next->next->next->next->next;
  tptr->down->next->next->next->next->next = NULL;
  blocktree(tptr->down,PRINTTOKENS); 
  tptr->down->next->next->next->next->next = cptr;

  /* do WHILE block as mode */

  tptr = cptr;
  cptr = tptr->next;
  tptr->next = NULL;
  blocktree(tptr->down,mode); 
  tptr->next = cptr;

  /* do rest of block as PRINTTOKENS */

  tptr = cptr;
  blocktree(tptr,PRINTTOKENS); 
  
}

/****************************************************************/
/*             return statement code generation                 */
/****************************************************************/


void returncode(tnode * tbranch)


{   
  char * retstr, * prefix, * astr;
  tnode * tptr = tbranch;          /* RETURN LP exprlist RP SEM */
  sigsym * sptr;
  tnode * fptr;  /* formal parameters */
  tnode * aptr;  /* actual parameters */
  int i;

  if (curropcodeinstance->ttype == S_OPARRAYCALL)
    {
      prefix = namingprefix(curropcodestack->next, "");
      vmcheck(retstr = calloc(strlen(prefix) + strlen("__return") + 
			      strlen(curropcodestack->defnode->val) + 
			      21, sizeof(char)));
      sprintf(retstr, "%s_%s%i_return", prefix, 
	      curropcodestack->defnode->val, 
	      curropcodestack->defnode->arrayidx);
      free(prefix);
    }
  else
    {
      vmcheck(retstr = calloc(strlen(currinstancename) + strlen("_return") +
			      1, sizeof(char)));
      sprintf(retstr, "%s_return", currinstancename); 
    }

  /* compute all return values */
  
  aptr = tptr->down->next->next->down;

  currarrayindex=0;
  currscalarflag=0; /* later do special cases */

  while (aptr != NULL)
    {
      if (aptr->ttype == S_EXPR)
	{
	  for (i=0;i<truewidth(aptr->width);i++)
	    {
	      fprintf(outfile,"   ");
	      if (curropcodeinstance->ttype == S_OPARRAYCALL)
		{
		  vmcheck(prefix = calloc(strlen(retstr) + 
					  strlen(" + ") + 21,
					  sizeof(char)));
		  sprintf(prefix, "%s + %i", retstr, currarrayindex);
		  astr = stackstring(S_NUMBER, curropcodestack->next ? 
				     curropcodestack->next->special : -1, 
				     prefix);
		  free(prefix);
		  fprintf(outfile, "%s", astr);
		  free(astr);
		}
	      else
		fprintf(outfile,"NV(%s + %i)", retstr, currarrayindex);
	      fprintf(outfile," = ");
	      blocktree(aptr->down,PRINTTOKENS);
	      fprintf(outfile,";\n");
	      currarrayindex++;
	    }
	}
      aptr = aptr->next;
    }
  currarrayindex = 0;

  /* handle exports */

  sptr = curropcodeinstance->sptr->defnode->sptr;
  while (sptr != NULL)
    {
      if ((sptr->kind == K_EXPORT)|| (sptr->kind == K_IMPORTEXPORT))
	{
	  /* semantics problem for ivar, performance problem for ksig */
	  /* want to add an if statement here of some sort            */

	  if ((sptr->vartype != TABLETYPE) && 
	      (sptr->tref->mirror != GLOBALMIRROR))
	    fprintf(outfile,
		    "   memcpy(&(NGU(GBL_%s)), &(NVU(%s_%s)), %i*sizeof(NGU(0)));\n",
		    sptr->val,currinstancename,sptr->val,
		    truewidth(sptr->width));
	}
      sptr = sptr->next;
    }

  /* handle call by references */

  fptr = curropcodeinstance->sptr->defnode->down->next->next->next->down;
  if (curropcodeinstance->ttype == S_OPARRAYCALL)
    {
      aptr = curropcodeinstance->optr->down->next->next->next->next->next->down;
    }
  else
    {
      aptr = curropcodeinstance->optr->down->next->next->down;
    }
  i = 1;
  while (aptr != NULL)
    {
      if ((aptr->ttype == S_EXPR) && (aptr->vartype != TABLETYPE) &&
	  (fptr->sptr->tref->assigntot))
	{

	  /* for indexed array call-by-reference */
	 
	  if (indexed_cbr(aptr))
	    fprintf(outfile, "   *(cbr%i) = NV(%s_%s);\n",
		    i++, currinstancename, fptr->down->next->down->val);

	  if (stname_cbr(aptr))
	    {
	      fprintf(outfile,"   for(stidx = 0; stidx < 128; stidx++)\n");
	      fprintf(outfile,"     *(&(");
	      blocktree(aptr->down,PRINTTOKENS);
	      fprintf(outfile,") + stidx) = NP(%s_%s + stidx);\n",
		      currinstancename, fptr->down->next->down->val);
	    }

	}
      aptr = aptr->next;
      fptr = fptr->next;
    }

  currarrayindex = 0;
  currscalarflag = 1;

  if (curropcodeinstance->ttype == S_OPARRAYCALL)
    {
      astr = stackstring(S_NUMBER, curropcodestack->next ? 
			 curropcodestack->next->special : -1, 
			 retstr);
      fprintf(outfile, "   return(%s);", astr);
      free(astr);
    }
  else
    fprintf(outfile,"   return(NV(%s));", retstr);
    
  free(retstr);
 
}

/****************************************************************/
/*              instr statement code generation                 */
/****************************************************************/

void instrcode(tnode * tbranch)

{                  
  tnode * tptr = tbranch;          /* INSTR IDENT LP exprlist RP SEM */
  tnode * dptr;                    /* formal parameters */
  tnode * aptr;                    /* actual parameters */
  tnode * lptr;                    /* instr list for order check */
  sigsym * sptr;
  int before = 0;                  /* is instr before currinstrument */
  char * origval = tbranch->down->next->val;
  char * val;

  val = dupunderscore(tbranch->down->next->val);

  currarrayindex = 0;
  currscalarflag = 1;

  /* see if instr runs before currinstrument, set before flag */

  lptr = make_tnode(origval,S_IDENT);
  sptr = getsym(&instrnametable,lptr);
  if (sptr != currinstrument)
    {
      lptr->next = make_tnode(currinstrument->val,S_IDENT);
      if (findlast(&instrnametable,lptr) == currinstrument)
	before = 1;
    }
  
  /* code generation begins */

  fprintf(outfile,"   if ( EV(d_%snext) == NULL)\n",val);
  fprintf(outfile,"     EV(d_%snext) = EV(d_%sfirst) = EV(d_%slast);\n",val,val,val);
  fprintf(outfile,"   else\n");
  fprintf(outfile,"   {\n");
  fprintf(outfile,"    EV(d_%snext) = EV(d_%sfirst);\n",val,val);
  fprintf(outfile,"    while (EV(d_%snext) < EV(d_%send))\n",val,val);
  fprintf(outfile,"     {\n");
  fprintf(outfile,"       if ((EV(d_%snext)->noteon == ALLDONE)||\n",val);
  fprintf(outfile,"           (EV(d_%snext)->noteon == NOTUSEDYET))\n",val); 
  fprintf(outfile,"        break;\n");
  fprintf(outfile,"       EV(d_%snext)++;\n",val);
  fprintf(outfile,"     }\n");
  fprintf(outfile,"    if ((EV(d_%snext) != EV(d_%send)) && (EV(d_%snext) > EV(d_%slast)))\n",val,val,val,val);
  fprintf(outfile,"      EV(d_%slast) = EV(d_%snext);\n",val,val);
  fprintf(outfile,"   }\n");
  fprintf(outfile,"   if (EV(d_%snext) != EV(d_%send))\n",val,val);
  fprintf(outfile,"    {\n");

  /* compute all return values */

  aptr = tptr->down->next->next->next->down;
  dptr = tptr->dptr->sptr->defnode->down->next->next->next->down;

  /* first, set start time  */

  fprintf(outfile,"   EV(d_%snext)->starttime = EV(scorebeats) + (",val);
  blocktree(aptr->down,PRINTTOKENS);
  fprintf(outfile,");\n");
  aptr = aptr->next->next;

  /* then, set duration */

  fprintf(outfile,"   EV(d_%snext)->sdur = ",val);
  blocktree(aptr->down,PRINTTOKENS);
  fprintf(outfile,";\n");
  fprintf(outfile,"   if (EV(d_%snext)->sdur < 0.0F)\n",val);
  fprintf(outfile,"     EV(d_%snext)->sdur = -1.0F;\n",val);
  
  aptr = aptr->next;
  if (aptr != NULL)
    aptr = aptr->next;

  while (aptr != NULL)
    {
      if (aptr->ttype == S_EXPR)
	{
	  fprintf(outfile,"   EV(d_%snext)->p[%s_%s] = ",
		  val,origval,dptr->val);
	  blocktree(aptr->down,PRINTTOKENS);
	  fprintf(outfile,";\n");
	}
      aptr = aptr->next;
      dptr = dptr->next;
    }

  fprintf(outfile,"   EV(d_%snext)->abstime = 0.0F;\n",val);
  fprintf(outfile,"   EV(d_%snext)->endtime = MAXENDTIME;\n",val);
  fprintf(outfile,"   EV(d_%snext)->released = 0;\n",val);
  fprintf(outfile,"   EV(d_%snext)->turnoff = 0;\n",val);
  fprintf(outfile,"   EV(d_%snext)->launch = NOTLAUNCHED;\n",val);
  fprintf(outfile,"   EV(d_%snext)->numchan = NS(iline->numchan);\n",val);
  fprintf(outfile,"   EV(d_%snext)->notenum = 255 & (NS(iline->notenum));\n",val);
  fprintf(outfile,"   EV(d_%snext)->preset  = NS(iline->preset);\n",val);

  fprintf(outfile,"   if (EV(d_%snext)->starttime <= (EV(scorebeats)+EV(scoremult)))\n",val);
  fprintf(outfile,"   {\n");
  fprintf(outfile,"    EV(d_%snext)->notestate = EV(nextstate);\n",val);
  fprintf(outfile,"    EV(d_%snext)->nstate = &(EV(ninstr)[EV(nextstate)]);\n",val);

  fprintf(outfile,"    EV(ninstr)[EV(nextstate)].iline = EV(d_%snext);\n",val);
  fprintf(outfile,"    EV(d_%snext)->time = (EV(kcycleidx)-1)*EV(KTIME);\n",val);
  fprintf(outfile,"    if (EV(d_%snext)->sdur >= 0.0F)\n",val);
  fprintf(outfile,"      EV(d_%snext)->endtime = EV(scorebeats) + EV(d_%snext)->sdur;\n"
	  ,val,val);
  fprintf(outfile,"    EV(d_%snext)->kbirth = EV(kcycleidx);\n",val);
  fprintf(outfile,"    EV(d_%snext)->noteon = PLAYING;\n",val);
  nextstateupdate(NULL);
  fprintf(outfile,"    if (EV(pass) == KPASS)\n");
  fprintf(outfile,"     {\n");
  fprintf(outfile,"      EV(pass) = IPASS;\n");
  fprintf(outfile,"      %s_ipass(ENGINE_PTR_COMMA EV(d_%snext)->nstate);\n",origval,val);
  fprintf(outfile,"      EV(pass) = KPASS;\n");
  if (before)
    {
      fprintf(outfile,"    EV(d_%snext)->noteon = PAUSED;\n",val);
    }
  fprintf(outfile,"     }\n");
  fprintf(outfile,"    else\n");
  fprintf(outfile,"     {\n");
  fprintf(outfile,"     %s_ipass(ENGINE_PTR_COMMA EV(d_%snext)->nstate);\n",origval,val);
  if (before)
    {
      fprintf(outfile,"    EV(d_%snext)->noteon = PAUSED;\n",val);
    }
  fprintf(outfile,"     }\n");

  fprintf(outfile,"   }\n");
  fprintf(outfile,"   else\n");
  fprintf(outfile,"    EV(d_%snext)->noteon = TOBEPLAYED;\n",val);
  fprintf(outfile,"  }\n");

  free(val);

}

/****************************************************************/
/*             instr statement code generation                 */
/****************************************************************/

void instrstubcode(tnode * tbranch)


{                  
  tnode * tptr = tbranch;          /* INSTR IDENT LP exprlist RP SEM */
  tnode * aptr;                    /* actual parameters */

  currarrayindex = 0;
  currscalarflag = 1;

  aptr = tptr->down->next->next->next->down;
  fprintf(outfile,"   ");
  blocktree(aptr->down,PRINTTOKENS);
  fprintf(outfile,";\n");
  aptr = aptr->next;
  if (aptr != NULL)
    aptr = aptr->next;

  while (aptr != NULL)
    {
      if (aptr->ttype == S_EXPR)
	{
	  fprintf(outfile,"   ");
	  blocktree(aptr->down,PRINTTOKENS);
	  fprintf(outfile,";\n");
	}
      aptr = aptr->next;
    }

}

/****************************************************************/
/*                 prints actual call to opcode                 */
/****************************************************************/

void opcallprint(tnode * tptr, int opcoderate)

{
  int currintstack; 

  if ((tptr->optr->ttype == S_OPCALL) && 
      coreopcodecaninline(tptr))
    {
      currintstack = currintprint;
      currintprint = coreopcodeasint(tptr);
      fprintf(outfile,"(");
      if ((!currscalarflag) || (opcoderate == SLOWRATETYPE))
	{
	  if (currintprint == ASINT)
	    fprintf(outfile,"(int)(");
	  fprintf(outfile,"NV(%s_%s%i_return) = ",
		  currinstancename, tptr->val,
		  tptr->optr->arrayidx);
	  
	}
      coreopcodedoinline(tptr);
      if (((!currscalarflag) || (opcoderate == SLOWRATETYPE)) &&
	  (currintprint == ASINT))
	fprintf(outfile,")");
      currintprint = currintstack;
      fprintf(outfile,")");
    }
  else
    {
      fprintf(outfile,"%s__sym_%s%i(NSP)",
	      curropcodeprefix,tptr->val, 
	      tptr->optr->arrayidx);
    }

}

/****************************************************************/
/*                 opcall code generation                       */
/****************************************************************/


void opcallcode(tnode * tptr)

{			     
  sigsym * sptr;

  /* revisit how polyopcallexcept() works */

  if (currarrayindex == 0)
    {
      if ((tptr->optr->rate == currblockrate) || polyopcallexcept(tptr))
	{
	  opcallprint(tptr, SAMERATETYPE);
	}
      else
	{
	  /* opcalls at slower rate   */
	  /* left to do: currintprint */

	  sptr = tptr->optr->sptr;

	  if (tptr->optr->rate == IRATETYPE)
	    {
	      fprintf(outfile, "(NVI(%s_%s%i__sym_ocstate) ? ",
		      currinstancename, tptr->val, tptr->optr->arrayidx);
	      fprintf(outfile, "%sNV(%s_%s%i_return) : (",
		      (currintprint == ASINT) ? "(int)" : "",
		      currinstancename, tptr->val, tptr->optr->arrayidx);
	      fprintf(outfile, "(NVI(%s_%s%i__sym_ocstate) = EV(kcycleidx)), ",
		      currinstancename, tptr->val, tptr->optr->arrayidx);
	      opcallprint(tptr, SLOWRATETYPE);
	      fprintf(outfile, "))");
	    }
	  if (tptr->optr->rate == KRATETYPE)
	    {
	      if (sptr->cref->callswitch == 0)
		{
		  fprintf(outfile, "(EV(acycleidx) ? %sNV(%s_%s%i_return) : ",
			  (currintprint == ASINT) ? "(int)" : "",
			  currinstancename, tptr->val, tptr->optr->arrayidx);
		  opcallprint(tptr, SLOWRATETYPE);
		  fprintf(outfile, ")");
		}
	      else
		{
		  fprintf(outfile, "( (NVI(%s_%s%i__sym_ocstate) == EV(kcycleidx)) ? ",
			  currinstancename, tptr->val, tptr->optr->arrayidx);
		  fprintf(outfile, "%sNV(%s_%s%i_return) : ",
			  (currintprint == ASINT) ? "(int)" : "",
			  currinstancename, tptr->val, tptr->optr->arrayidx);
		  fprintf(outfile, "((NVI(%s_%s%i__sym_ocstate) = EV(kcycleidx)), ",
			  currinstancename, tptr->val, tptr->optr->arrayidx);
		  opcallprint(tptr, SLOWRATETYPE);
		  fprintf(outfile, "))");
		}
	    }
	}
    }
  else
    {
      /* any time an ASINT is required, the context is */
      /* a scalar context (so far). So, (int) casts    */
      /* are not needed or checked.                    */
      
      if (truewidth(tptr->optr->width)>1)
	fprintf(outfile,
		"NV(%s_%s%i_return + %i)",
		currinstancename, tptr->val, 
		tptr->optr->arrayidx,
		currarrayindex);
      else
	fprintf(outfile,
		"NV(%s_%s%i_return)",
		currinstancename, tptr->val, 
		tptr->optr->arrayidx);
    }
  
}


/****************************************************************/
/*             special expression code generation               */
/****************************************************************/

void specialexpr(tnode * tptr)

{
  while (tptr)
    {
      if (tptr->down)
	specialexpr(tptr->down);
      else
	{
	  /* for now this only triggers specialops           */
	  /* later amend to trigger UDO's w/ specialop calls */

	  if ((tptr->ttype == S_IDENT) && (tptr->optr) &&
	      (tptr->special))
 	    fprintf(outfile,"%s__sym_%s%i_spec(NSP);\n",
		    curropcodeprefix,tptr->val, 
		    tptr->optr->arrayidx); 
	}
      tptr = tptr->next;
    }
}

/****************************************************************/
/*             special sematics  code generation                */
/****************************************************************/

void specialcode(tnode * tbranch)


{
  tnode * tptr;

  switch(tbranch->down->ttype) {
  case S_LVALUE:    /* lvalue EQ expr SEM */
    tptr = tbranch->down->down;
    if ((tptr->next != NULL) && (tptr->next->ttype == S_LB) && 
	(tptr->next->next->next->next == NULL) && /* an array index */
	(tptr->next->next->special))
      {
	specialexpr(tptr->next->next->down);
	fprintf(outfile,";\n");
      }
    tptr = tbranch->down->next->next;
    if (tptr->special)
      {
	specialexpr(tptr->down);
	fprintf(outfile,";\n");
      }
    break;          
  case S_EXPR:      /* expr SEM */
    specialexpr(tbranch->down);
    break;
  case S_IF:        /* IF LP expr RP LC block RC [ELSE LC block RC] */
    tptr = tbranch->down->next->next;
    if (tptr->special)
      {
	specialexpr(tptr->down);
	fprintf(outfile,";\n");
      }
    tptr = tbranch->down->next->next->next->next->next->down;
    while (tptr != NULL)
      {
	if (tptr->special)
	  specialcode(tptr);
	tptr = tptr->next;
      }
    tptr = tbranch->down->next->next->next->next->next->next->next;
    if (tptr != NULL)
      {
	tptr = tptr->next->next->down;
	while (tptr != NULL)
	  {
	    if (tptr->special)
	      specialcode(tptr);
	    tptr = tptr->next;
	  }
      }
    break;
  case S_INSTR:     /* INSTR IDENT LP exprlist RP SEM */
    tptr = tbranch->down->next->next->next->down;
    while (tptr != NULL)
      {
	if ((tptr->ttype == S_EXPR) && tptr->special)
	  {
	    specialexpr(tptr->down);
	    fprintf(outfile,";\n");
	  }
	tptr = tptr->next;
      }
    break;
  case S_EXTEND:     /* EXTEND LP expr RP SEM */
    tptr = tbranch->down->next->next;
    if (tptr->special)
      {
	specialexpr(tptr->down);
	fprintf(outfile,";\n");
      }
    break;
  case S_RETURN:    /* RETURN LP exprlist RP SEM */
    tptr = tbranch->down->next->next->down;
    while (tptr != NULL)
      {
	if ((tptr->ttype == S_EXPR) && tptr->special)
	  {
	    specialexpr(tptr->down);
	    fprintf(outfile,";\n");
	  }
	tptr = tptr->next;
      }
    break;
  case S_PRINTF:    /* PRINTF LP exprstrlist RP SEM */
    tptr = tbranch->down->next->next->down;
    while (tptr != NULL)
      {
	if ((tptr->ttype == S_EXPR) && tptr->special)
	  {
	    specialexpr(tptr->down);
	    fprintf(outfile,";\n");
	  }
	tptr = tptr->next;
      }
    break;
  default:
    break;
  }

}

/****************************************************************/
/*             main generator routines for code blocks          */
/****************************************************************/

void blocktree(tnode * tbranch, int mode)

{
  tnode * tptr = tbranch;
  int currintstack; 

  while (tptr != NULL)
    if ((tptr->ttype == S_STATEMENT))
      {
	if ((tptr->down->ttype == S_INSTR) && (mode == PRINTIPASS)
	    && (tptr->rate == KRATETYPE))
	  instrstubcode(tptr);
	if ( ((mode == PRINTKPASS) && (tptr->rate == KRATETYPE)) ||
	     ((mode == PRINTAPASS) && (tptr->rate == ARATETYPE)) ||
	     ((mode == PRINTIPASS) && (tptr->rate == IRATETYPE)) ||
	     (mode == PRINTTOKENS))
	  switch (tptr->down->ttype) {
	  case S_OUTPUT:
	    outputcode(tptr);
	    break; 
	  case S_OUTBUS:
	    outbuscode(tptr);
	    break; 
	  case S_TURNOFF:
	    turnoffcode(tptr);
	    break; 
	  case S_EXTEND:
	    extendcode(tptr);
	    break; 
	  case S_PRINTF:
	    printfcode(tptr);
	    break; 
	  case S_LVALUE:
	    lvaluecode(tptr);
	    break; 
	  case S_RETURN:
	    returncode(tptr);
	    break;
	  case S_TMAPIDX :
	    tmapidxcode(tptr);
	    break;
	  case S_INSTR:
	    instrcode(tptr);
	    break;
	  case S_SPATIALIZE:
	    spatializecode(tptr);
	    break;
	  case S_IF: 
	    ifcode(tptr,mode);
	    break;
	  case S_WHILE: 
	    whilecode(tptr,mode);
	    break;
	  default :
	    blocktree(tptr->down,PRINTTOKENS);
	    break;
	  }
	if ((mode == PRINTAPASS) && (tptr->special == 1))
	  specialcode(tptr);
	tptr = tptr->next;
      }
    else
      {
	if ((tptr->down == NULL))
	  {
	    if (mode == PRINTTOKENS)
	      {
		switch (tptr->ttype) {
		case S_IDENT:
		  if (standardname(tptr))
		    {
		      printstandardname(&tptr);
		    }
		  else
		    {
		      if (tptr->next != NULL) 
			{
			  if ((tptr->next->ttype == S_LB) && /* an array index */
			      (tptr->next->next->next->next == NULL))
			    {
			      macroselect(tptr);
			      currintstack = currintprint;
			      fprintf(outfile," + ");
			      if (tptr->next->next->res == ASINT)
				{
				  currintprint = ASINT;
				  blocktree(tptr->next->next->down, PRINTTOKENS);
				  fprintf(outfile,")");
				}
			      else
				{
				  fprintf(outfile,"((int)(0.5F + (");
				  currintprint = ASFLOAT;
				  blocktree(tptr->next->next->down, PRINTTOKENS);
				  fprintf(outfile," )))) ");
				}
			      
			      tptr = tptr->next->next->next;  /* skip to end  */
			      currintprint = currintstack;
			      
			    }
			  if (tptr->optr != NULL)       /* opcode/oparray call */
			    {
			      opcallcode(tptr);
			      if (tptr->optr->ttype == S_OPCALL)
				tptr = tptr->next->next->next; 
			      else
				tptr = tptr->next->next->next->next->next->next; 
			    }
			}
		      else
			{
			  macroselect(tptr);
			  if (truewidth(tptr->width)>1)
			    {
			      /* an array */
			      
			      fprintf(outfile," + %i)",
				      currarrayindex);
			    }
			  else                          
			    {
			      /* scalar or table */
			      
			      fprintf(outfile,")");
			    }
			}
		    }
		  break;
		case S_RB:
		  internalerror("blocktree.c", "S_RB encountered");
		  break;
		case S_SEM:
		case S_RP:
		case S_RC:
		  fprintf(outfile,"%s\n",tptr->val);
		  break;
		case S_BLOCK:
		case S_EXPR:
		  break;
		case S_NUMBER:
		  if (atof(tptr->val) >= 0)
		    fprintf(outfile," %sF ",tptr->val);
		  else
		    fprintf(outfile," (%sF) ",tptr->val);
		  break;
		case S_INTGR:
		  if (atoi(tptr->val) >= 0)
		    {
		      if (currintprint == ASFLOAT)
			fprintf(outfile," %s.0F ",tptr->val);
		      else
			fprintf(outfile," %s ",tptr->val);
		    }
		  else
		    {
		      if (currintprint == ASFLOAT)
			fprintf(outfile," (%s.0F) ",tptr->val);
		      else
			fprintf(outfile," (%s) ",tptr->val);
		    }
		  break;
		default:
		  fprintf(outfile," %s ",tptr->val);
		  break;
		}
	      }
	  }
	else
	  blocktree(tptr->down,mode);
	tptr = tptr->next;
      }

}



