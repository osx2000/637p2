
/*
#    Sfront, a SAOL to C translator    
#    This file: Code generation: user-defined opcodes
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

extern void redefglobal(void);
extern void redefnormal(void);
extern void redefstatic(int);
extern void printopcodes(tnode *);


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*                void opcodefunction()                         */
/*                                                              */
/* This is the top-level function that prints out all opcode    */
/* and oparray functions in an sa.c file. At time of writing,   */
/* called from writeorc.c, but now may be called in sfmain.c.   */
/*                                                              */
/*______________________________________________________________*/


/****************************************************************/
/*    prints out all opcode/oparray functions in the sa.c file  */
/****************************************************************/

void opcodefunctions(void)

{
  sigsym * sptr;
  tnode * tptr;
  int i;

  curroparraydepth = -1;

  if (globalopcodecalls != NULL)
    {
      currinputwidth = inchannels;
      currinstrwidth = outchannels;
      currinstancename = curropcodeprefix = "GBL";
      currinstrument = NULL;  
      currinstance = NULL;
      curropcodestack = NULL;

      redefglobal();
      printopcodes(globalopcodecalls);
      redefnormal();
    }
  
  sptr = instrnametable;
  while (sptr != NULL)
    {
      if (reachableinstrexeff(sptr))
	{
	  currinputwidth = 1;              /* was zero */
	  currinstrwidth = sptr->width;
	  currinstancename = curropcodeprefix = sptr->val;
	  currinstrument = sptr;
	  currinstance = NULL;
	  curropcodestack = NULL;

	  printopcodes(sptr->defnode->optr);
	}
      sptr = sptr->next;
    }

  /* set starting point for ninstr[] effects positions  */
  /* this must change if startup implementation changes */
  
  i = (startupinstr != NULL);

  tptr = instances;
  while (tptr != NULL)
    {
      redefstatic(i++);
      currinputwidth = tptr->inwidth;
      currinstrwidth = tptr->sptr->width;
      currinstancename = curropcodeprefix = tptr->val;
      currinstrument = tptr->sptr;
      currinstance = tptr;
      curropcodestack = NULL;

      printopcodes(tptr->sptr->defnode->optr);
      tptr = tptr->next;
    }
  redefnormal();
  curropcodestack = NULL;
  curropcodeinstance = NULL;

}


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*           functions called by opcodefunctions()              */
/*                                                              */
/* the main function, printopcodes, prints out all of the       */
/* opcodes of a single instr/instance or of the global {}.      */
/* the other functions customize NV() and friends (but note     */
/* other functions in this file customize NV() too).            */
/*                                                              */
/*______________________________________________________________*/

extern void printoparraycall(tnode *);
extern void printopcall(tnode *);

/****************************************************************/
/*                 print out opcode functions                   */
/****************************************************************/

void printopcodes(tnode * tptr)

{
  while (tptr != NULL)
    {      
      if ((tptr->ttype == S_OPARRAYCALL) && tptr->optr)
	{
	  if (tptr->special)
	    {
	      currspecialrate = KRATETYPE;
	      printoparraycall(tptr);
	      currspecialrate = ARATETYPE;
	      printoparraycall(tptr);
	    }
	  else
	    printoparraycall(tptr);
	}
      else
	{
	  if ((tptr->ttype == S_OPCALL) && tptr->optr && 
	      (!coreopcodecaninline(tptr->optr->down)))
	    {
	      if (tptr->special)
		{
		  currspecialrate = KRATETYPE;
		  printopcall(tptr);
		  currspecialrate = ARATETYPE;
		  printopcall(tptr);
		}
	      else
		printopcall(tptr);
	    }
	}
      tptr = tptr->next;
    }
}


/****************************************************************/
/*        changes ifdefs for global variables                   */
/****************************************************************/

void redefglobal(void)

{
  fprintf(outfile,"\n");
  fprintf(outfile,"#undef NS\n");
  fprintf(outfile,"#define NS(x) 0\n");
  fprintf(outfile,"#undef NSP\n");
  fprintf(outfile,"#define NSP  ENGINE_PTR_COMMA NULL\n");
  fprintf(outfile,"#undef NT\n");
  fprintf(outfile,"#define NT(x)  EV(gtables)[x]\n");
  fprintf(outfile,"#undef NV\n");
  fprintf(outfile,"#define NV(x)  EV(global)[x].f\n");
  fprintf(outfile,"#undef NVI\n");
  fprintf(outfile,"#define NVI(x)  EV(global)[x].i\n");
  fprintf(outfile,"#undef NVUI\n");
  fprintf(outfile,"#define NVUI(x)  EV(global)[x].ui\n");
  fprintf(outfile,"#undef NVPS\n");
  fprintf(outfile,"#define NVPS(x)  EV(global)[x].ps\n");
  fprintf(outfile,"#undef NVU\n");
  fprintf(outfile,"#define NVU(x)  EV(global)[x]\n");
  fprintf(outfile,"#undef NP\n");
  fprintf(outfile,"#define NP(x)  EV(global)[x].f\n");
  fprintf(outfile,"#undef NPI\n");
  fprintf(outfile,"#define NPI(x)  EV(global)[x].i\n");
  fprintf(outfile,"#undef NPUI\n");
  fprintf(outfile,"#define NPUI(x)  EV(global)[x].ui\n");
  fprintf(outfile,"#undef NPPS\n");
  fprintf(outfile,"#define NPPS(x)  EV(global)[x].ps\n");
  fprintf(outfile,"\n");
}


/****************************************************************/
/*          changes ifdefs to instr variables                   */
/****************************************************************/

void redefnormal(void)

{
  fprintf(outfile,"\n");
  fprintf(outfile,"#undef NS\n");
  fprintf(outfile,"#define NS(x) nstate->x\n");
  fprintf(outfile,"#undef NSP\n");
  fprintf(outfile,"#define NSP  ENGINE_PTR_COMMA nstate\n");
  fprintf(outfile,"#undef NT\n");
  fprintf(outfile,"#define NT(x)  nstate->t[x]\n");
  fprintf(outfile,"#undef NV\n");
  fprintf(outfile,"#define NV(x)  nstate->v[x].f\n");
  fprintf(outfile,"#undef NVI\n");
  fprintf(outfile,"#define NVI(x)  nstate->v[x].i\n");
  fprintf(outfile,"#undef NVUI\n");
  fprintf(outfile,"#define NVUI(x)  nstate->v[x].ui\n");
  fprintf(outfile,"#undef NVPS\n");
  fprintf(outfile,"#define NVPS(x)  nstate->v[x].ps\n");
  fprintf(outfile,"#undef NVU\n");
  fprintf(outfile,"#define NVU(x)  nstate->v[x]\n");
  fprintf(outfile,"#undef NP\n");
  fprintf(outfile,"#define NP(x)  nstate->v[x].f\n");
  fprintf(outfile,"#undef NPI\n");
  fprintf(outfile,"#define NPI(x)  nstate->v[x].i\n");
  fprintf(outfile,"#undef NPUI\n");
  fprintf(outfile,"#define NPUI(x)  nstate->v[x].ui\n");
  fprintf(outfile,"#undef NPPS\n");
  fprintf(outfile,"#define NPPS(x)  nstate->v[x].ps\n");
  fprintf(outfile,"\n");
}


/****************************************************************/
/*          changes ifdefs to a fixed instance position         */
/****************************************************************/

void redefstatic(int i)

{
  fprintf(outfile,"\n");
  fprintf(outfile,"#undef NS\n");
  fprintf(outfile,"#define NS(x)  EV(ninstr)[%i].x\n",i);
  fprintf(outfile,"#undef NSP\n");
  fprintf(outfile,"#define NSP  ENGINE_PTR \n");
  fprintf(outfile,"#undef NT\n");
  fprintf(outfile,"#define NT(x)  EV(ninstr)[%i].t[x]\n",i);
  fprintf(outfile,"#undef NV\n");
  fprintf(outfile,"#define NV(x)  EV(ninstr)[%i].v[x].f\n",i);
  fprintf(outfile,"#undef NVI\n");
  fprintf(outfile,"#define NVI(x)  EV(ninstr)[%i].v[x].i\n",i);
  fprintf(outfile,"#undef NVUI\n");
  fprintf(outfile,"#define NVUI(x)  EV(ninstr)[%i].v[x].ui\n",i);
  fprintf(outfile,"#undef NVPS\n");
  fprintf(outfile,"#define NVPS(x)  EV(ninstr)[%i].v[x].ps\n",i);
  fprintf(outfile,"#undef NVU\n");
  fprintf(outfile,"#define NVU(x)  EV(ninstr)[%i].v[x]\n",i);
  fprintf(outfile,"#undef NP\n");
  fprintf(outfile,"#define NP(x)  EV(ninstr)[%i].v[x].f\n",i);
  fprintf(outfile,"#undef NPI\n");
  fprintf(outfile,"#define NPI(x)  EV(ninstr)[%i].v[x].i\n",i);
  fprintf(outfile,"#undef NPUI\n");
  fprintf(outfile,"#define NPUI(x)  EV(ninstr)[%i].v[x].ui\n",i);
  fprintf(outfile,"#undef NPPS\n");
  fprintf(outfile,"#define NPPS(x)  EV(ninstr)[%i].v[x].ps\n",i);
  fprintf(outfile,"\n");
}


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*  Top-level functions for printing opcalls and oparray calls  */
/*                                                              */
/*           printopcall() and printoparraycall()               */
/*                                                              */
/*       The functions share many common support routines.      */
/*                                                              */
/*______________________________________________________________*/


extern void printlocals(tnode *);
extern void printopargs(tnode *, tnode *, tnode *, char *);
extern void printopimports(tnode *, char *);
extern void printopcodebody(tnode *);
extern void printoparrayconst(tnode *);
extern void printoparraygeneral(tnode *);


/****************************************************************/
/*                 print out opcall function                    */
/****************************************************************/

void printopcall(tnode * tptr)

{
  sigsym opstack;     /* element of opcode stack */
  char newifix[STRSIZE];
  char newpfix[STRSIZE];
  char newname[STRSIZE];
  char * istack, * pstack;
  char * specfix, * retvartype;
  tnode * fptr;  /* formal parameters */
  tnode * aptr;  /* actual parameters */

  specfix = "";
  retvartype = "float";

  if (tptr->special)
    {
      currblockrate = ARATETYPE;
      if (currspecialrate == ARATETYPE)
	{
	  specfix = "_spec";
	  retvartype = "void";
	}
    }
  else
    currblockrate = tptr->rate;

  istack = currinstancename;
  pstack = curropcodeprefix;

  sprintf(newifix,"%s_%s%i",currinstancename, tptr->val,tptr->arrayidx);
  sprintf(newpfix,"%s_%s%i",curropcodeprefix, tptr->val,tptr->arrayidx);

  sprintf(newname,"%s__sym_%s%i", curropcodeprefix, tptr->val,tptr->arrayidx);

  /*  function header */

  if (currinstance == NULL)
    fprintf(outfile,"\n\n%s %s%s(ENGINE_PTR_DECLARE_COMMA " 
	    "struct ninstr_types * nstate)\n{\n",
	    retvartype, newname, specfix);
  else
    fprintf(outfile,"\n\n%s %s%s(ENGINE_PTR_DECLARE)\n{\n", retvartype,newname,specfix);

  /* local variables for core opcodes and tables */

  printlocals(tptr);

  /* compute arguments into the function */
  
  fptr = tptr->sptr->defnode->down->next->next->next->down;
  aptr = tptr->optr->down->next->next->down;
  printopargs(tptr,aptr,fptr,newifix);

  /* do imports */

  currinstancename = newifix;
  curropcodeprefix = newpfix;
  curropcodeinstance = tptr;

  printopimports(tptr, newifix);

  /* statement block */

  opstack.next = curropcodestack;
  opstack.defnode = tptr;
  opstack.special = curroparraydepth;
  curropcodestack = &opstack;

  printopcodebody(tptr);

  fprintf(outfile,"\n}\n\n");
  
  /* all opcodes called by this opcode */

  printopcodes(tptr->sptr->defnode->optr);

  currinstancename = istack;
  curropcodeprefix = pstack;
  curropcodestack = opstack.next;
}


extern void printoparraychildren(tnode *, int);

/****************************************************************/
/*           print out oparray call function                    */
/****************************************************************/

void printoparraycall(tnode * tptr)

{
  char * specfix, * retvartype;
  char * istack, * pstack;
  int optype;

  istack = currinstancename;
  pstack = curropcodeprefix;

  retvartype = "float";
  specfix = "";
  if (tptr->special)
    {
      currblockrate = ARATETYPE;
      if (currspecialrate == ARATETYPE)
	{
	  retvartype = "void";
	  specfix = "_spec";
	}
    }
  else
    currblockrate = tptr->rate;

  /****************************************/
  /*  function header and local variables */
  /****************************************/

  if (currinstance == NULL)
    fprintf(outfile,
	    "\n\n%s %s__sym_%s%i%s(ENGINE_PTR_DECLARE_COMMA "
	    "struct ninstr_types * nstate)\n{\n",
	    retvartype, curropcodeprefix, tptr->val, tptr->arrayidx,specfix);
  else
    fprintf(outfile, "\n\n%s %s__sym_%s%i%s(ENGINE_PTR_DECLARE)\n{\n", 
	    retvartype, curropcodeprefix, tptr->val, tptr->arrayidx, specfix);
  printlocals(tptr);

  /*****************************************/
  /* do depth update and print opcode body */
  /*****************************************/

  if (tptr->optr->down->next->next->vol == CONSTANT)
    {
      optype = OPARRAY_CONSTANT;
      printoparrayconst(tptr);
    }
  else 
    {
      curroparraydepth++;
      optype = OPARRAY_GENERAL;
      printoparraygeneral(tptr);
    }

  fprintf(outfile,"\n}\n\n");

  currinstancename = istack;
  curropcodeprefix = pstack;

  /*********************************************/
  /* print opcode children and do depth update */
  /*********************************************/

  printoparraychildren(tptr, optype);      
  
}


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*  Second-level functions for printing opcalls/ oparray calls  */
/*                                                              */
/*                                                              */
/*  These functions handle major tasks, like printing out       */
/*       function parameters or the opcode body code.           */
/*______________________________________________________________*/



/****************************************************************/
/*   print out local variables for core opcodes and tables      */
/****************************************************************/

void printlocals(tnode * tcall)

{
  tnode * tptr;
  sigsym * sptr;
  int first = 0;

  tptr = tcall->extra;
  while (tptr != NULL)
    {
      if (tptr->ttype == S_PARAMDECL)
	{
	  if (tptr->vartype == TABLETYPE)
	    fprintf(outfile,"   int va_%s;\n",tptr->down->next->down->val);
	  else	
	    fprintf(outfile,"   float va_%s;\n",tptr->down->next->down->val);
	}
      tptr = tptr->next;
    }

  sptr = tcall->sptr->defnode->sptr;
  while (sptr != NULL)
    {
      if ((sptr->vartype == TABLETYPE) && 
	  ((sptr->kind == K_NORMAL)|| (sptr->kind == K_IMPORT)))
	{
	  if (!first)
	    {
	      fprintf(outfile,"   int i,j;\n");
	      first = 1;
	    }
	  if (sptr->kind == K_NORMAL)
	    wavegeneratorvar(sptr);
	}
      sptr = sptr->next;
    }
  coreopcodevars(tcall);
  fprintf(outfile,"\n");

}

extern int makeparamindex(tnode *, sigsym *, char **, int*);
extern void printsignaldefine(tnode *, char *, char *, int, int);
extern void callbyrefvars(tnode *, tnode *);

extern int maketableindex(tnode *, sigsym *, char **, int *);
extern void printtabledefine(int, char *, int, int);
extern void printtmapcase(tnode *, tnode *, char *);


/****************************************************************/
/*            print out actual args for opcode calls            */
/****************************************************************/

void printopargs(tnode * tcall, tnode * aptr, tnode * fptr, 
		 char * newfix)

{
  int core, specialop, skip;
  int tablenum, tabletype, signaltype, callbyrefidx;
  tnode * tptr;
  char tablename[STRSIZE];
  char * idxstr;
  int depth;

  tptr = fptr;
  if (!(core = coreopcodename(tcall)))
    callbyrefvars(aptr, fptr);

  specialop = coreopcodespecial(tcall);
  callbyrefidx = tablenum = 1;
  while (fptr != NULL)
    {
      if ((fptr->ttype == S_PARAMDECL) && 
	  !( specialop && (fptr->rate == ARATETYPE)
	     && (currspecialrate == KRATETYPE)))
	{	
	  if (fptr->vartype == TABLETYPE)
	    {
	      if (core)
		sprintf(tablename,"%s",fptr->down->next->down->val);
	      else
		if (aptr->down->vartype == TMAPTYPE)
		  sprintf(tablename,"NPPS(%s_%s)", newfix,
			  fptr->down->next->down->val);
		else
		  sprintf(tablename,"NPI(%s_%s)", newfix,
			  fptr->down->next->down->val);

	      if (aptr->down->vartype == TMAPTYPE)
		{
		  /* tablemaps */
		  
		  printtmapcase(aptr->down, aptr->down->next->next, tablename);

		  if (core)
		    printtabledefine(tablenum++, tablename, S_TABLEMAP, -1);

		}
	      else
		{
		  /* tables */
		  
		  currarrayindex = 0;
		  currscalarflag = 1;

		  if (core)
		    {
		      fprintf(outfile,"   %s = ", tablename);
		      tabletype = maketableindex(aptr->down, curropcodestack,
						 &idxstr, &depth);
		      fprintf(outfile,"%s;\n",idxstr);
		      free(idxstr);
		      printtabledefine(tablenum++,tablename,tabletype,depth);
		    }
		}	
	    }
	  else  
	    {
	      /* pass-by-reference for scalars and arrays */

	      if ((!core) && ((fptr->sptr->tref->mirror == OPCODEMIRROR) ||
			      stname_cbr(aptr) || 
			      (indexed_cbr(aptr) && 
			       (fptr->sptr->tref->assigntot))))
		{
		  if (aptr->down->next == NULL)
		    {
		      if (aptr->down->sptr)
			{
			  /* scalars and unindexed arrays */

			  signaltype = makeparamindex(aptr->down,
						      curropcodestack, 
						      &idxstr, &depth);
			  printsignaldefine(fptr->down->next->down, idxstr, 
					    newfix, signaltype, depth);
			  free(idxstr);
			}
		      else
			{
			  /* params and MIDIctrl -- also non-call-by-ref */

			  currarrayindex = 0;
			  currscalarflag = 0;

			  fprintf(outfile,"   for(stidx = 0; stidx < 128; stidx++)\n");
			  fprintf(outfile,"     NP(%s_%s + stidx) = *(&(",
				  newfix, fptr->down->next->down->val);
			  blocktree(aptr->down,PRINTTOKENS);
			  fprintf(outfile,") + stidx);\n");

			  currarrayindex = 0;
			  currscalarflag = 1;
			}
		    }
		  else
		    {
		      /* indexed arrays */

		      fprintf(outfile,"   NP(%s_%s) = *(cbr%i = &(",
			      newfix, fptr->down->next->down->val, 
			      callbyrefidx++);
		      blocktree(aptr->down,PRINTTOKENS);
		      fprintf(outfile,"));\n");
		    }
		}
	      else
		{
		  /* signal variables */

		  currarrayindex = 0;
		  if (truewidth(fptr->width) > 1)
		    currscalarflag = 0;
		  else
		    currscalarflag = 1;
		  while (currarrayindex < truewidth(fptr->width))
		    {
		      if (truewidth(fptr->width) == 1)
			{
			  if (core)
			    {
			      fprintf(outfile,"   %s = ",
				      fptr->down->next->down->val);
			      blocktree(aptr->down,PRINTTOKENS);
			      fprintf(outfile,";\n");
			    }
			  else
			    {
			      /* if expression to be assigned is constant, */
			      /* the opcode isn't in the global block,     */
			      /* user-defined opcode parameter is scalar,  */
			      /* parameter not used in while or if block,  */
			      /* and const optimization is on, assignment  */
			      /* is not needed. Later extend to VECTORTYPE */
			      
			      skip = constoptimize && currinstrument &&
				(aptr->vol == CONSTANT) &&
				(fptr->sptr->vartype ==
				 SCALARTYPE) && 
				!(fptr->sptr->tref->assignif) &&
				!(fptr->sptr->tref->assignwhile);
			      
			      if (!skip)
				{
				  fprintf(outfile,"   NP(%s_%s) = ",
					  newfix, fptr->down->next->down->val);
				  blocktree(aptr->down,PRINTTOKENS);
				  fprintf(outfile,";\n");
				}
			    }
			}
		      else
			{
			  /* no core opcode array arguments */
			  
			  fprintf(outfile,"   NP(%s_%s + %i) = ", newfix, 
				  fptr->down->next->down->val,currarrayindex);
			  blocktree(aptr->down,PRINTTOKENS);
			  fprintf(outfile,";\n");
			}
		      
		      currarrayindex++;
		    }
		}
	    }
	}

      aptr = aptr->next;
      fptr = fptr->next;
    }

  fptr = tcall->extra; 
  if ((aptr != NULL) && (tptr != NULL))
    aptr = aptr->next;   /* comma skip */

  while (fptr != NULL)
    {
      if (fptr->ttype == S_PARAMDECL)
	{	 
	  if (aptr->vartype == TABLETYPE)
	    {
	      if (aptr->down->vartype == TMAPTYPE)
		{
		  /* tablemaps */
		  
		  sprintf(tablename,"va_%s",fptr->down->next->down->val);
		  printtmapcase(aptr->down, aptr->down->next->next, tablename);
		  printtabledefine(tablenum++, tablename, S_TABLEMAP, -1);
		}
	      else
		{
		  /* tables */

		  currarrayindex = 0;
		  currscalarflag = 1;
		  sprintf(tablename,"va_%s", fptr->down->next->down->val);
		  fprintf(outfile,"   %s = ", tablename);
		  tabletype = maketableindex(aptr->down, curropcodestack,
					     &idxstr, &depth);
		  fprintf(outfile,"%s;\n", idxstr);
		  free(idxstr);
		  printtabledefine(tablenum++, tablename, tabletype, depth);
		}
	    }
	  else
	    {
	      /* signal variables */

	      currarrayindex = 0;
	      currscalarflag = 1;
	      fprintf(outfile,"   va_%s = ", fptr->down->next->down->val);
	      blocktree(aptr->down,PRINTTOKENS);
	      fprintf(outfile," ;\n");
	    }
	}
      aptr = aptr->next;
      fptr = fptr->next;
    }

  currarrayindex = 0;
  currscalarflag = 1;
}


/****************************************************************/
/*   print out global variable importation for opcode args      */
/****************************************************************/

void printopimports(tnode * tptr, char * newfix)


{
  sigsym * sptr;
  sigsym * gptr;
  char name[STRSIZE];

  sptr = tptr->sptr->defnode->sptr;
  while (sptr != NULL)
    {
      gptr = getvsym(&globalsymtable,sptr->val);
      if (sptr->vartype == TABLETYPE)
	{
	  if ((sptr->kind == K_NORMAL) || 
	      ((sptr->kind == K_IMPORT) && ((sptr->tref->assigntot != 0) || 
					    (gptr->tref->assigntot != 0))))
	    {
	      sprintf(name,"TBL_%s",newfix);
	      fprintf(outfile, "   if (NT(%s_%s).t == NULL)\n",
		      name,sptr->val);
	      fprintf(outfile, "   {\n");
	      createtable(sptr,name, S_OPCODE);
	      fprintf(outfile, "   }\n");
	    }

	  if ((sptr->kind == K_IMPORTEXPORT) && startupinstr && 
	      (currinstrument == startupinstr))
	    tablestartupcheck(sptr, S_OPCODE, K_IMPORTEXPORT); 
	}
      else
	{
	  if ((sptr->kind == K_IMPORT)|| (sptr->kind == K_IMPORTEXPORT))
	    {

	      /* note this implies no imports w/o global match for opcodes */

	      if (gptr == NULL)
		{
		  printf("Error: No global variable matches import.\n\n");
		  showerrorplace(sptr->defnode->linenum, 
				 sptr->defnode->filename);
		}
	      if ((gptr->vartype != sptr->vartype)||
		  (gptr->width != truewidth(sptr->width))||
		  (gptr->rate != sptr->rate))
		{
		  printf("Error: Mismatch in global import.\n\n");
		  showerrorplace(sptr->defnode->linenum, 
				 sptr->defnode->filename);
		}

	      /* semantics problem for ivar, performance problem for ksig */
	      /* want to add an if statement here of some sort. Note      */
	      /* sptr->rate == tptr->staterate was the previous guard     */

	      if (sptr->tref->mirror != GLOBALMIRROR)
		{
		  fprintf(outfile,
	  "   memcpy(&(NVU(%s_%s)), &(NGU(GBL_%s)), %i*sizeof(NGU(0)));\n",
			  newfix,sptr->val,sptr->val,
			  truewidth(sptr->width));
		}
	    }
	  if (sptr->kind == K_EXPORT)
	    {
	      if (gptr == NULL)
		{
		  printf("Error: No global variable matches export.\n\n");
		  showerrorplace(sptr->defnode->linenum, 
				 sptr->defnode->filename);
		}
	      if ((gptr->vartype != sptr->vartype)||
		  (gptr->width != truewidth(sptr->width))||
		  (gptr->rate != sptr->rate))
		{
		  printf("Error: Mismatch in global export.\n\n");
		  showerrorplace(sptr->defnode->linenum, 
				 sptr->defnode->filename);
		}
	      
	      /* semantics problem for k/a opcalls, needs an if statement */
	      
	      if ((tptr->staterate == IRATETYPE) && 
		  (sptr->tref->mirror == GLOBALMIRROR) &&
		  ((sptr->tref->assigntot == 0) ||
		   (sptr->tref->varstate)))
		{
		  fprintf(outfile,
			  "   memset(&(NGU(GBL_%s)), 0, %i*sizeof(NGU(0)));\n",
			  sptr->val, truewidth(sptr->width));
		}
	    }
	}
      sptr = sptr->next;
    }

}


extern void printopbody_ki(tnode * tptr);
extern void printopbody_aki(tnode * tptr);

/****************************************************************/
/*                 print out opcall body                        */
/****************************************************************/

void printopcodebody(tnode * tptr)

{

  if (coreopcodename(tptr))
    coreopcodebody(tptr);
  else
    {
      switch (tptr->rate) {
      case IRATETYPE:
	blocktree(tptr->sptr->defnode->down
		  ->next->next->next->next->next->next->next->down,
		  PRINTIPASS);
	break;
      case KRATETYPE:
	if (tptr->sptr->cref->ilines)
	  printopbody_ki(tptr);
	else
	  blocktree(tptr->sptr->defnode->down
		    ->next->next->next->next->next->next->next->down,
		    PRINTKPASS);
	break;
      case ARATETYPE:
	if (tptr->sptr->cref->ilines || tptr->sptr->cref->klines)
	  printopbody_aki(tptr);
	else
	  blocktree(tptr->sptr->defnode->down
		    ->next->next->next->next->next->next->next->down,
		    PRINTAPASS);
	break;
      }
    }
}

/****************************************************************/
/*          print out kopcodes with i-rate lines                */
/****************************************************************/

void printopbody_ki(tnode * tptr)

{
  int currblocksafe;

  fprintf(outfile, "   do {\n");
  fprintf(outfile, "    if (NVI(%s__sym_opstate))\n", currinstancename);
  fprintf(outfile, "     {\n");

  blocktree(tptr->sptr->defnode->down
	    ->next->next->next->next->next->next->next->down,
	    PRINTKPASS);

  fprintf(outfile, "     }\n");

  currblocksafe = currblockrate;
  currblockrate = IRATETYPE;

  blocktree(tptr->sptr->defnode->down
	    ->next->next->next->next->next->next->next->down,
	    PRINTIPASS);

  currblockrate = currblocksafe;
  
  fprintf(outfile, "  NVI(%s__sym_opstate) = EV(kcycleidx);\n", currinstancename);
  fprintf(outfile, "   } while (1);\n\n");

}



/****************************************************************/
/*          print out aopcodes with i/k-rate lines              */
/****************************************************************/

void printopbody_aki(tnode * tptr)

{
  int currblocksafe;

  fprintf(outfile, "   do {\n");

  if (tptr->sptr->cref->klines)
    fprintf(outfile, "    if (NVI(%s__sym_opstate) == EV(kcycleidx))\n",
	    currinstancename);
  else
    fprintf(outfile, "    if (NVI(%s__sym_opstate))\n", currinstancename);

  fprintf(outfile, "     {\n");

  blocktree(tptr->sptr->defnode->down
	    ->next->next->next->next->next->next->next->down,
	    PRINTAPASS);

  fprintf(outfile, "     }\n");

  if (tptr->sptr->cref->ilines)
    {
      if (tptr->sptr->cref->klines)
	{
	  fprintf(outfile, "    if (NVI(%s__sym_opstate) == 0)\n",
		  currinstancename);
	  fprintf(outfile, "     {\n");
	}

      currblocksafe = currblockrate;
      currblockrate = IRATETYPE;

      blocktree(tptr->sptr->defnode->down
		->next->next->next->next->next->next->next->down,
		PRINTIPASS);

      currblockrate = currblocksafe;

      if (tptr->sptr->cref->klines)
	{
	  fprintf(outfile, "     }\n");
	}
    }

  if (tptr->sptr->cref->klines)
    {

      currblocksafe = currblockrate;
      currblockrate = KRATETYPE;

      blocktree(tptr->sptr->defnode->down
		->next->next->next->next->next->next->next->down,
		PRINTKPASS);

      currblockrate = currblocksafe;
    }

  fprintf(outfile, "  NVI(%s__sym_opstate) = EV(kcycleidx);\n", 
	  currinstancename);
  fprintf(outfile, "   } while (1);\n\n");

}


extern void redefoparrayargs(void);
extern void redefoparraybody(void);

/****************************************************************/
/*         print out oparray code: the general case             */
/****************************************************************/

void printoparraygeneral(tnode * tptr)

{
  tnode * fptr;            /* formal parameters    */
  tnode * aptr;            /* actual parameters    */
  char newifix[STRSIZE];   /* replace w/ char *    */
  char newpfix[STRSIZE];   /* replace w/ char *    */
  sigsym opstack;          /* opcode stack element */
  int currintstack;        /* for printing opindex */

  fprintf(outfile,"\n");
  if (tptr->optr->down->next->next->res != ASINT)
    fprintf(outfile,"   float arrayround;\n");
  fprintf(outfile,"   int arrayswitch;\n");
  fprintf(outfile,"\n");

  /******************************/
  /* compute which state to use */
  /******************************/

  currintstack = currintprint;
  if (tptr->optr->down->next->next->res == ASINT)
    {
      fprintf(outfile,"\n   arrayswitch = ");
      currarrayindex = 0;
      currscalarflag = 1;
      currintprint = ASINT;
      blocktree(tptr->optr->down->next->next->down,PRINTTOKENS);
      fprintf(outfile," ;\n");
    }
  else
    {
      fprintf(outfile,"\n   arrayround = ");
      currarrayindex = 0;
      currscalarflag = 1;
      currintprint = ASFLOAT;
      blocktree(tptr->optr->down->next->next->down,PRINTTOKENS);
      fprintf(outfile," ;\n");
      fprintf(outfile,"   arrayswitch = ROUND(arrayround);\n");
    }
  currintprint = currintstack;

  /****************************************/
  /* set base value for vstate and nstate */
  /****************************************/

  if (currinstrument)
    fprintf(outfile, "   NS(vstate[%i]) ", curroparraydepth);
  else
    fprintf(outfile, "   EV(globalvstate)[%i] ", curroparraydepth);

  fprintf(outfile, "= &(NVU(%s_%sopbase + arrayswitch*%s_%sopsize));\n",
	  currinstancename, tptr->val, currinstancename, tptr->val);

  fprintf(outfile, "   if (TBL_%s_%sopsize)\n", currinstancename, tptr->val);

  if (currinstrument)
    fprintf(outfile, "   NS(tstate[%i]) ", curroparraydepth);
  else
    fprintf(outfile, "   EV(globaltstate)[%i] ", curroparraydepth);

  fprintf(outfile, "= &(NT(TBL_%s_%sopbase + arrayswitch*TBL_%s_%sopsize));\n",
	  currinstancename, tptr->val, currinstancename, tptr->val);

  /**********************************************************/
  /* print opcode args: compute arguments into the function */
  /**********************************************************/

  redefoparrayargs();

  sprintf(newifix,"%s_%soparray0",currinstancename, tptr->val);
  sprintf(newpfix,"%s_%s%i",curropcodeprefix, tptr->val, tptr->arrayidx);

  fptr = tptr->sptr->defnode->down->next->next->next->down;
  aptr = tptr->optr->down->next->next->next->next->next->down;
  printopargs(tptr, aptr, fptr, newifix);

  /**************/
  /* do imports */
  /**************/

  redefoparraybody();

  currinstancename = newifix;
  curropcodeprefix = newpfix;

  curropcodeinstance = tptr;
  printopimports(tptr, newifix);

  /***************************************/
  /* opstack used by inlined table calls */
  /***************************************/

  opstack.next = curropcodestack;
  opstack.defnode = tptr;
  opstack.special = curroparraydepth;
  opstack.numinst = 0;
  curropcodestack = &opstack;

  /*******************/
  /* statement block */
  /*******************/

  printopcodebody(tptr);

  /*************/
  /* pop stack */
  /*************/

  curropcodestack = opstack.next;

}

/****************************************************************/
/*     print out oparray code when index is constant            */
/****************************************************************/

void printoparrayconst(tnode * tptr)

{
  tnode * fptr;            /* formal parameters    */
  tnode * aptr;            /* actual parameters    */
  int opnum;               /* oparray slot to use  */
  char newifix[STRSIZE];   /* replace w/ char *    */
  char newpfix[STRSIZE];   /* replace w/ char *    */
  sigsym opstack;          /* opcode stack element */

  opnum = make_int(tptr->optr->down->next->next->down);

  if ((opnum < 0) || (opnum >= truewidth(tptr->ibus->opwidth)))
    internalerror("writeop.c", "oparray range error -- printoparrayconst()");
  else
    {
      sprintf(newifix,"%s_%soparray%i",currinstancename, tptr->val, (opnum+1));
      sprintf(newpfix,"%s_%s%i",curropcodeprefix, tptr->val, tptr->arrayidx);

      /***************************************/
      /* compute arguments into the function */
      /***************************************/
  
      fptr = tptr->sptr->defnode->down->next->next->next->down;
      aptr = tptr->optr->down->next->next->next->next->next->down;
      printopargs(tptr, aptr, fptr, newifix);

      /**************/
      /* do imports */
      /**************/

      currinstancename = newifix;
      curropcodeprefix = newpfix;

      curropcodeinstance = tptr;
      printopimports(tptr, newifix);

      /***************************************/
      /* opstack used by inlined table calls */
      /***************************************/

      opstack.next = curropcodestack;
      opstack.defnode = tptr;
      opstack.special = curroparraydepth;
      curropcodestack = &opstack;
      opstack.numinst = opnum + 1;

      /*******************/
      /* statement block */
      /*******************/

      printopcodebody(tptr);

      /*************/
      /* pop stack */
      /*************/

      curropcodestack = opstack.next;
    }

}

/****************************************************************/
/*         print out opcalls made in an oparray call            */
/****************************************************************/

void printoparraychildren(tnode * tptr, int optype)

{  
  char newifix[STRSIZE];
  char newpfix[STRSIZE];
  sigsym opstack;       /* element of opcode stack */
  char * istack, * pstack;
  int opnum;


  istack = currinstancename;
  pstack = curropcodeprefix;

  sprintf(newpfix,"%s_%s%i",curropcodeprefix, tptr->val, tptr->arrayidx);
  curropcodeprefix = newpfix;

  opstack.next = curropcodestack;
  opstack.defnode = tptr;
  opstack.special = curroparraydepth;
  curropcodestack = &opstack;

  if (optype == OPARRAY_CONSTANT)
    {
      opnum = make_int(tptr->optr->down->next->next->down);

      opstack.numinst = opnum + 1;
      sprintf(newifix,"%s_%soparray%i", currinstancename, tptr->val, (opnum+1));

      currinstancename = newifix;
      printopcodes(tptr->sptr->defnode->optr);

    }
  else
    {
      opstack.numinst = 0;
      sprintf(newifix,"%s_%soparray0", currinstancename, tptr->val);
      currinstancename = newifix;
      printopcodes(tptr->sptr->defnode->optr);
      curroparraydepth--;
      redefoparrayargs();
      redefoparraybody();
    }

  currinstancename = istack;
  curropcodeprefix = pstack;

  curropcodestack = opstack.next;

}

extern char * stackstring(int, int, char * idxstr);
 
/****************************************************************/
/*   changes variable macros for oparray parameters arguments   */
/****************************************************************/

void redefoparrayargs(void)

{
  char * pstr;

  fprintf(outfile,"\n");
  fprintf(outfile,"#undef NP\n");
  fprintf(outfile,"#undef NPI\n");
  fprintf(outfile,"#undef NPUI\n");
  fprintf(outfile,"#undef NPPS\n");

  fprintf(outfile,"#define NP(x) %s \n", 
	  (pstr = stackstring(S_NUMBER, curroparraydepth, "x")));
  free(pstr);

  fprintf(outfile,"#define NPI(x) %s \n\n", 
	  (pstr = stackstring(S_INTGR, curroparraydepth, "x")));
  free(pstr);

  fprintf(outfile,"#define NPUI(x) %s \n\n", 
	  (pstr = stackstring(S_FREE1, curroparraydepth, "x")));
  free(pstr);

  fprintf(outfile,"#define NPPS(x) %s \n\n", 
	  (pstr = stackstring(S_FREE2, curroparraydepth, "x")));
  free(pstr);
}


/****************************************************************/
/*        changes variable macros for oparray body code         */
/****************************************************************/

void redefoparraybody(void)

{
  char * pstr;

  fprintf(outfile,"\n");
  fprintf(outfile,"#undef NT\n");
  fprintf(outfile,"#undef NV\n");
  fprintf(outfile,"#undef NVI\n");
  fprintf(outfile,"#undef NVUI\n");
  fprintf(outfile,"#undef NVPS\n");
  fprintf(outfile,"#undef NVU\n");

  fprintf(outfile,"#define NV(x) %s \n", 
	  (pstr = stackstring(S_NUMBER, curroparraydepth, "x")));
  free(pstr);

  fprintf(outfile,"#define NVI(x) %s \n", 
	  (pstr = stackstring(S_INTGR, curroparraydepth, "x")));
  free(pstr);

  fprintf(outfile,"#define NVUI(x) %s \n", 
	  (pstr = stackstring(S_FREE1, curroparraydepth, "x")));
  free(pstr);

  fprintf(outfile,"#define NVPS(x) %s \n", 
	  (pstr = stackstring(S_FREE2, curroparraydepth, "x")));
  free(pstr);

  fprintf(outfile,"#define NVU(x) %s \n", 
	  (pstr = stackstring(S_UNION, curroparraydepth, "x")));
  free(pstr);

  fprintf(outfile,"#define NT(x) %s \n\n", 
	  (pstr = stackstring(S_TABLE, curroparraydepth, "x")));
  free(pstr);

}


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*         Utility functions for writing opcall/oparrays        */
/*                                                              */
/*                                                              */
/*    These functions involve handling call-by-reference and    */
/*    global variable addressing for tables and signal vars.    */
/*______________________________________________________________*/


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/* Tables and signal variables need to backtrack to do naming.  */
/* This recursive function does the job for both var types.     */
/*______________________________________________________________*/

 
/****************************************************************/
/*         forms full variable name by stack backtracking       */
/****************************************************************/

char * namingprefix(sigsym * opstack, char * prefix)

{
  char * in;
  char * ret;
  char num[32];

  if (opstack != NULL)
    {
      in = namingprefix(opstack->next, prefix);
      if (opstack->defnode->ttype == S_OPARRAYCALL)
	{
	  sprintf(num,"%i", opstack->numinst);
	  vmcheck(ret = (char *) calloc((strlen(in) +
					 strlen(opstack->defnode->val) 
					 + strlen(num) + 9), sizeof(char)));
	  sprintf(ret,"%s_%soparray%s", in, opstack->defnode->val, num);
	}
      else
	{
	  sprintf(num,"%i", opstack->defnode->arrayidx);
	  vmcheck(ret = (char *) calloc((strlen(in) +
					 strlen(opstack->defnode->val) 
					 + strlen(num) + 2), sizeof(char)));
	  sprintf(ret,"%s_%s%s", in, opstack->defnode->val, num);
	}
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
	      vmcheck(ret = calloc((strlen("GBL") 
				    + strlen(prefix) + 1),
				   sizeof(char)));
	      strcpy(ret, prefix);
	      strcat(ret, "GBL");
	      return ret;
	    }
	}
    }
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/* These functions handle signal parameters for imports and     */
/* call by reference and ksig flags.                            */
/*______________________________________________________________*/


/****************************************************************/
/*        declare extra variables for call-by-reference         */
/****************************************************************/

void callbyrefvars(tnode * aptr, tnode * fptr)

{
  int i = 1;
  int needs_stidx = 0;

  while (aptr)
    {
      if (indexed_cbr(aptr) && fptr->sptr->tref->assigntot)
	fprintf(outfile,"   float * cbr%i;\n",i++);
      needs_stidx |= (stname_cbr(aptr) || ((aptr->down != NULL) && 
					 (aptr->down->ttype == S_IDENT) && 
					 (aptr->down->next == NULL) &&
					 (aptr->down->sptr == NULL)));
      aptr = aptr->next;
    }

  if (needs_stidx)
    fprintf(outfile,"   int stidx;\n");

  if ((i > 1) || needs_stidx)
    fprintf(outfile,"\n");
}


/****************************************************************/
/*            print out signal parameter OSP defines            */
/****************************************************************/

void printsignaldefine(tnode * fptr, char * idxstr, 
		       char * newfix, int type, int depth)

{
  char * sumstr;
  char * pstr;

  vmcheck(sumstr = calloc(strlen(idxstr) + strlen(" + x") + 1,
			  sizeof(char)));

  if (fptr->sptr->vartype == SCALARTYPE)
    strcpy(sumstr, idxstr);
  else
    {
      strcpy(sumstr, idxstr);
      strcat(sumstr, " + x");
    }

  fprintf(outfile, "\n#undef OSP_%s_%s\n",newfix,fptr->val);
  fprintf(outfile, "#undef OSPI_%s_%s\n\n",newfix,fptr->val);

  /* first the imports/exports case */

  if (type == S_GLOBAL)
    {
      fprintf(outfile, "#define OSP_%s_%s(x) NG(%s)\n",
	      newfix, fptr->val, sumstr);
      fprintf(outfile, "#define OSPI_%s_%s(x) NGI(%s)\n\n",
	      newfix, fptr->val, sumstr);
      free(sumstr);
      return;
    }
  
  /* the rest, for: instance, instrument, and global-block opcalls */
  
  fprintf(outfile, "#define OSP_%s_%s(x) %s\n",
	  newfix, fptr->val, (pstr = stackstring(S_NUMBER, depth, sumstr)));
  free(pstr);

  fprintf(outfile, "#define OSPI_%s_%s(x) %s\n\n",
	  newfix, fptr->val, (pstr = stackstring(S_INTGR, depth, sumstr)));

  free(pstr);
  free(sumstr);

}


/****************************************************************/
/*            make string that indexes opcall pfield            */
/****************************************************************/

int makeparamindex(tnode * tptr, sigsym * opstack, char ** idxstr, 
		   int * depth)

{
  char * prefix;
  int ret;
  tnode * aptr;
  tnode * fptr;

  switch (tptr->sptr->kind) {
  case K_PFIELD:
    if (opstack && (tptr->sptr->tref->mirror == OPCODEMIRROR))
      {
	/* the difficult case: an opcode K_PFIELD that */
	/* is also passed by reference from above      */

	fptr = opstack->defnode->sptr->defnode->down->next->next->next->down;

	if (opstack->defnode->ttype == S_OPARRAYCALL)
	  aptr = opstack->defnode->optr->down->next->next->next->next->next->down;
	else
	  aptr = opstack->defnode->optr->down->next->next->down;

	while (fptr != NULL)
	  {
	    if ((fptr->ttype == S_PARAMDECL) &&
		(tptr->sptr == fptr->sptr))
	      break;
	    fptr = fptr->next;
	    aptr = aptr->next;
	  }

	if (fptr == NULL)
	  internalerror("writeop.c", "makeparmindex backtrack problem");

	return(makeparamindex(aptr->down, opstack->next, idxstr, depth));
      }
    else
      {
	/* simpler cases */

	if (opstack == curropcodestack)
	  prefix = dupval(currinstancename); 
	else
	  prefix = namingprefix(opstack, "");
	ret = S_INSTR;
      }
    break;
  case K_IMPORT:
  case K_EXPORT:
  case K_IMPORTEXPORT:
    if (tptr->sptr->tref->mirror == GLOBALMIRROR)
      {
	prefix = dupval("GBL");
	ret = S_GLOBAL;
	break;
      }

    /* falls through */

  default:
    {
      if (opstack == curropcodestack)
        prefix = dupval(currinstancename); 
      else
	prefix = namingprefix(opstack, "");
      ret = S_INSTR;
    }
  }

  /* make return string */

  vmcheck(*idxstr = calloc(2 + strlen(prefix) + strlen(tptr->val), 1));
				    
  strcat(*idxstr, prefix);
  strcat(*idxstr, "_");
  strcat(*idxstr, tptr->val);

  free(prefix);

  /* set depth */

  if (opstack)
    *depth = opstack->special;
  else
    *depth = -1;

  return ret;

}


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*    These functions handle table and tablemap parameters.     */
/*______________________________________________________________*/


/****************************************************************/
/*            print out table parameter AP define               */
/****************************************************************/

void printtabledefine(int num, char * tablename, int type, int depth)

{
  char * pstr;

  switch(type) {
  case S_TABLE:
    fprintf(outfile, "\n#undef AP%i\n",num);
    fprintf(outfile, "#define AP%i %s\n\n", num,
	    (pstr = stackstring(S_TABLE, depth, tablename)));
    free(pstr);
    break;
  case S_IMPORTS:
    fprintf(outfile, "\n#undef AP%i\n",num);
    fprintf(outfile, "#define AP%i EV(gtables)[%s]\n\n",num,tablename);
    break;
  case S_TABLEMAP:
    fprintf(outfile, "\n#undef AP%i\n",num);
    fprintf(outfile, "#define AP%i (*((tableinfo *)(%s)))\n\n",
	    num, tablename);
    break;
  }

}

/****************************************************************/
/*   print out table specifier for inline opcodes               */
/****************************************************************/

void printinlinetable(tnode * tptr)

{
  char * idxstr;
  int depth;
  char * pstr;

  switch(maketableindex(tptr->down, curropcodestack, &idxstr, &depth)) {
  case S_TABLE:
    fprintf(outfile, "%s", (pstr = stackstring(S_TABLE, depth, idxstr)));
    free(pstr);
    break;
  case S_IMPORTS:
    fprintf(outfile, "EV(gtables)[%s]", idxstr);
    break;
  case S_TABLEMAP:
    fprintf(outfile, "(*((tableinfo *)(%s)))",idxstr);
    break;
  }

  free(idxstr);
}


/****************************************************************/
/*   print out table specifier for inline opcodes               */
/****************************************************************/

void printtableindirect(tnode * tptr, char * tablename)

{
  char * idxstr;
  int depth;
  char * pstr;

  switch(maketableindex(tptr, curropcodestack, &idxstr, &depth)) {
  case S_TABLE:
    pstr = stackstring(S_TABLE, depth, idxstr);
    fprintf(outfile,"     %s = (PSIZE)(&(%s));\n", tablename, pstr);
    free(pstr);
    break;
  case S_IMPORTS:
    fprintf(outfile,"     %s = (PSIZE)(&(EV(gtables)[%s]));\n", tablename, idxstr);
    break;
  case S_TABLEMAP:
    fprintf(outfile,"     %s = %s;\n", tablename, idxstr);
    break;
  }
  free(idxstr);
}


/****************************************************************/
/*        print out case statement for tmap indexing            */
/****************************************************************/

void printtmapcase(tnode * tmapptr, tnode * eptr, char * tablename)

{
  tnode * pptr;
  int j;
  int currintstack;

  /* print out case statement */
		  
  currarrayindex = 0;
  currscalarflag = 1;
  currintstack = currintprint;
  fprintf(outfile,"   switch(");

  if (eptr->res == ASINT)
    {
      currintprint = ASINT;
      blocktree(eptr->down, PRINTTOKENS);
    }
  else
    {
      currintprint = ASFLOAT;
      fprintf(outfile,"(int) (0.5F + ");
      blocktree(eptr->down, PRINTTOKENS);
      fprintf(outfile," )");
    }
  currintprint = currintstack;
  fprintf(outfile,") { \n");

  pptr = tmapptr->sptr->defnode->down->next->next->next->down;
  j = 0;
  while (pptr != NULL)
    {
      if (pptr->ttype == S_IDENT)
	{
	  fprintf(outfile,"   case %i:\n",j);
	  printtableindirect(pptr, tablename);
	  j++;
	  fprintf(outfile,"     break;\n");
	}
      pptr = pptr->next;
    }
  fprintf(outfile,"   default:\n");
  gened(tmapptr->sptr->defnode->down, "Tablemap index out of range");
  fprintf(outfile,"   }\n\n");

}


/****************************************************************/
/*            make string that indexes into table array         */
/****************************************************************/

int maketableindex(tnode * aptr, sigsym * opstack, 
		   char ** retstr, int * depth)

{
  int pfield;
  sigsym * sptr, * gptr;
  tnode * tptr;
  char * prefix;
  char * astr;

  gptr = getvsym(&globalsymtable, aptr->sptr->val);

  /* default depth behavior, overriden in special cases */

  if (opstack)
    *depth = opstack->special;
  else
    *depth = -1;

  /* handles K_NORMAL, K_IMPORTEXPORT, and K_IMPORT, and K_INTERNAL */

  switch (aptr->sptr->kind) {
  case K_NORMAL:
    if (opstack == curropcodestack)
      {
	vmcheck(*retstr = calloc(1 + strlen(" TBL__ ") +
			strlen(currinstancename) 
				 + strlen(aptr->sptr->val),
				 sizeof(char)));
	sprintf(*retstr, " TBL_%s_%s ", currinstancename, aptr->sptr->val);
      }
    else
      {
	prefix = namingprefix(opstack, "TBL_");
	vmcheck(*retstr = calloc(1 + strlen(" _ ") + strlen(prefix) 
			 + strlen(aptr->sptr->val), sizeof(char)));
	sprintf(*retstr, " %s_%s ", prefix, aptr->sptr->val);
	free(prefix);
      }
    return S_TABLE;
    break;
  case K_IMPORTEXPORT:
    vmcheck(*retstr = calloc(1 + strlen(" TBL_GBL_ ") 
			     + strlen(aptr->sptr->val),
			     sizeof(char)));
    sprintf(*retstr, " TBL_GBL_%s ", aptr->sptr->val);
    *depth = -1;
    return S_IMPORTS;
    break;
  case K_IMPORT:
    if ((aptr->sptr->tref->assigntot == 0) &&
	(gptr->tref->assigntot == 0))
      {
	vmcheck(*retstr = calloc(1 + strlen(" TBL_GBL_ ") 
				 + strlen(aptr->sptr->val),
				 sizeof(char)));
	sprintf(*retstr, " TBL_GBL_%s ", aptr->sptr->val);
	*depth = -1;
	return S_IMPORTS;
      }
    else
      {
	if (opstack == curropcodestack)
	  {
	    vmcheck(*retstr = calloc(1 + strlen(" TBL__ ") +
			    strlen(currinstancename) 
				     + strlen(aptr->sptr->val),
				     sizeof(char)));
	    sprintf(*retstr, " TBL_%s_%s ", currinstancename, aptr->sptr->val);
	  }
	else
	  {
	    prefix = namingprefix(opstack, "TBL_");
	    vmcheck(*retstr = calloc(1 + strlen(" _ ") + strlen(prefix)
			    + strlen(aptr->sptr->val), sizeof(char)));
	    sprintf(*retstr, " %s_%s ", prefix, aptr->sptr->val);
	    free(prefix);
	  }
	return S_TABLE;
      }
    break;
  case K_INTERNAL:
    if (opstack == curropcodestack)
      prefix = currinstancename;
    else
      prefix = namingprefix(opstack, "");

    vmcheck(astr = calloc(1 + strlen("_") + strlen(prefix)
			  + strlen(aptr->sptr->val), sizeof(char)));
    sprintf(astr, "%s_%s", prefix, aptr->sptr->val);
    *retstr = stackstring(S_FREE2, *depth, astr);
    free(astr);

    if (opstack != curropcodestack)
      free(prefix);
    return S_TABLEMAP;
  default:
    break;
  }

  /* rest of code traces K_PFIELD back up the stack */

  if (aptr->sptr->kind != K_PFIELD)
    internalerror("writeop.c","in maketableindex() -- bad parameter");

  if (opstack == NULL)
    internalerror("writeop.c","in maketableindex() -- corrupt stack");

  /**************************************/
  /* first find opcode parameter number */
  /**************************************/

  pfield = 0;
  sptr = opstack->defnode->sptr->defnode->sptr;
  while (sptr != aptr->sptr)
    {
      pfield++;
      sptr = sptr->next;
      if ((sptr == NULL) || (sptr->kind != K_PFIELD))
	internalerror("writeop.c","in maketableindex() -- symbol table");
    }


  /************************************************/
  /* now, find the opcode call's actual arguments */
  /************************************************/

  tptr = opstack->defnode->optr->down;

  /* find list of expressions */
  while (tptr->ttype != S_EXPRLIST)
    tptr = tptr->next;

  /* find the nth expression */

  tptr = tptr->down;
  while (pfield != 0)
    {
      tptr = tptr->next->next;
      pfield--;
    }
  tptr = tptr->down;

  /* test for table map */

  if (tptr->next != NULL)
    {
      if (opstack && opstack->next)
	*depth = opstack->special;
      else
	*depth = -1;
      prefix = namingprefix(opstack, "");

      vmcheck(astr = calloc(1 + strlen("_") + strlen(prefix)
			    + strlen(aptr->sptr->val), sizeof(char)));
      sprintf(astr, "%s_%s", prefix, aptr->sptr->val);
      *retstr = stackstring(S_FREE2, *depth, astr);

      free(astr);
      free(prefix);
      return S_TABLEMAP;
    }
  else
    {
      return maketableindex(tptr, opstack->next, retstr, depth);
    }

  /* should never happen */

  return S_TABLE;
}



/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*    This functions gets the right syntax for stack access     */
/*______________________________________________________________*/


/****************************************************************/
/*     returns correct syntax to access a variable/table        */
/****************************************************************/

char * stackstring(int type, int depth, char * idxstr) 

{
  char retstr[STRSIZE];   
  int i;
  
  if (currinstrument && currinstance)
    {
      i = (startupinstr != NULL) + currinstance->arrayidx;
      if (depth < 0)
	{
	  switch (type) {
	  case S_NUMBER:
	    sprintf(retstr, "EV(ninstr)[%i].v[%s].f", i, idxstr);
	    break;
	  case S_INTGR:
	    sprintf(retstr, "EV(ninstr)[%i].v[%s].i", i, idxstr);
	    break;
	  case S_FREE1:
	    sprintf(retstr, "EV(ninstr)[%i].v[%s].ui", i, idxstr);
	    break;
	  case S_FREE2:
	    sprintf(retstr, "EV(ninstr)[%i].v[%s].ps", i, idxstr);
	    break;
	  case S_UNION:
	    sprintf(retstr, "EV(ninstr)[%i].v[%s]", i, idxstr);
	    break;
	  case S_TABLE:
	    sprintf(retstr, "EV(ninstr)[%i].t[%s]", i, idxstr);
	    break;
	  default:
	    internalerror("writeop.c", "bad type given to stackstring 1");
	  }
	}
      else
	{
	  switch (type) {
	  case S_NUMBER:
	    sprintf(retstr, "(EV(ninstr)[%i].vstate)[%i][%s].f", i, depth, idxstr);
	    break;
	  case S_INTGR:
	    sprintf(retstr, "(EV(ninstr)[%i].vstate)[%i][%s].i", i, depth, idxstr);
	    break;
	  case S_FREE1:
	    sprintf(retstr, "(EV(ninstr)[%i].vstate)[%i][%s].ui", i, depth, idxstr);
	    break;
	  case S_FREE2:
	    sprintf(retstr, "(EV(ninstr)[%i].vstate)[%i][%s].ps", i, depth, idxstr);
	    break;
	  case S_UNION:
	    sprintf(retstr, "(EV(ninstr)[%i].vstate)[%i][%s]", i, depth, idxstr);
	    break;
	  case S_TABLE:
	    sprintf(retstr, "(EV(ninstr)[%i].tstate)[%i][%s]", i, depth, idxstr);
	    break;
	  default:
	    internalerror("writeop.c", "bad type given to stackstring 2");
	  }
	}
    }
  else
    {
      if (currinstrument)
	{
	  if (depth < 0)
	    {
	      switch (type) {
	      case S_NUMBER:
		sprintf(retstr, "nstate->v[%s].f", idxstr);
		break;
	      case S_INTGR:
		sprintf(retstr, "nstate->v[%s].i", idxstr);
		break;
	      case S_FREE1:
		sprintf(retstr, "nstate->v[%s].ui", idxstr);
		break;
	      case S_FREE2:
		sprintf(retstr, "nstate->v[%s].ps", idxstr);
		break;
	      case S_UNION:
		sprintf(retstr, "nstate->v[%s]", idxstr);
		break;
	      case S_TABLE:
		sprintf(retstr, "nstate->t[%s]", idxstr);
		break;
	      default:
		internalerror("writeop.c", "bad type given to stackstring 3");
	      }
	    }
	  else
	    {	
	      switch (type) {
	      case S_NUMBER:
		sprintf(retstr, "(nstate->vstate)[%i][%s].f", depth, idxstr);
		break;
	      case S_INTGR:
		sprintf(retstr, "(nstate->vstate)[%i][%s].i", depth, idxstr);
		break;
	      case S_FREE1:
		sprintf(retstr, "(nstate->vstate)[%i][%s].ui", depth, idxstr);
		break;
	      case S_FREE2:
		sprintf(retstr, "(nstate->vstate)[%i][%s].ps", depth, idxstr);
		break;
	      case S_UNION:
		sprintf(retstr, "(nstate->vstate)[%i][%s]", depth, idxstr);
		break;
	      case S_TABLE:
		sprintf(retstr, "(nstate->tstate)[%i][%s]", depth, idxstr);
		break;
	      default:
		internalerror("writeop.c", "bad type given to stackstring 4");
	      }
	    }
	}
      else
	{
	  if (depth < 0)
	    {
	      switch (type) {
	      case S_NUMBER:
		sprintf(retstr, "EV(global)[%s].f", idxstr);
		break;
	      case S_INTGR:
		sprintf(retstr, "EV(global)[%s].i", idxstr);
		break;
	      case S_FREE1:
		sprintf(retstr, "EV(global)[%s].ui", idxstr);
		break;
	      case S_FREE2:
		sprintf(retstr, "EV(global)[%s].ps", idxstr);
		break;
	      case S_UNION:
		sprintf(retstr, "EV(global)[%s]", idxstr);
		break;
	      case S_TABLE:
		sprintf(retstr, "EV(gtables)[%s]", idxstr);
		break;
	      default:
		internalerror("writeop.c", "bad type given to stackstring 5");
	      }
	    }
	  else
	    {
	      switch (type) {
	      case S_NUMBER:
		sprintf(retstr, "EV(globalvstate)[%i][%s].f", depth, idxstr);
		break;
	      case S_INTGR:
		sprintf(retstr, "EV(globalvstate)[%i][%s].i", depth, idxstr);
		break;
	      case S_FREE1:
		sprintf(retstr, "EV(globalvstate)[%i][%s].ui", depth, idxstr);
		break;
	      case S_FREE2:
		sprintf(retstr, "EV(globalvstate)[%i][%s].ps", depth, idxstr);
		break;
	      case S_UNION:
		sprintf(retstr, "EV(globalvstate)[%i][%s]", depth, idxstr);
		break;
	      case S_TABLE:
		sprintf(retstr, "EV(globaltstate)[%i][%s]", depth, idxstr);
		break;
	      default:
		internalerror("writeop.c", "bad type given to stackstring 6");
	      }
	    }
	}
    }
  return(dupval(retstr));
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*    These functions detect certain types of expressions       */
/*______________________________________________________________*/


/****************************************************************/
/*    detects indexed arrays that could be call-by-reference    */
/****************************************************************/

int indexed_cbr(tnode * aptr)

{
  int ret;

  ret = ((aptr->ttype == S_EXPR) && (aptr->vartype != TABLETYPE) &&
	 (aptr->down != NULL) && (aptr->down->ttype == S_IDENT) &&
	 (aptr->down->sptr || 
	  (!strcmp(aptr->down->val, "MIDIctrl")) ||
	  (!strcmp(aptr->down->val, "params"))) &&  
	 (aptr->down->next != NULL) &&
	 (aptr->down->next->ttype == S_LB) && 
	 (aptr->down->next->next->next->next == NULL));

  return ret;

}

/****************************************************************/
/*    detects unindexed standard names for call-by-reference    */
/****************************************************************/

int stname_cbr(tnode * aptr)

{
  int ret;

  ret = ((aptr->ttype == S_EXPR) && (aptr->down != NULL) &&
	 (aptr->down->ttype == S_IDENT) && (aptr->down->next == NULL) &&
	 (aptr->down->sptr == NULL) &&  
	 ((!strcmp(aptr->down->val, "MIDIctrl")) ||
	  (!strcmp(aptr->down->val, "params"))));

  return ret;

}
