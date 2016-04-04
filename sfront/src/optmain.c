
/*
#    Sfront, a SAOL to C translator    
#    This file: main functions for line-by-line optimization
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

extern void optglobalblock(void);
extern void globalclearcount(void); 
extern void symtableinit(sigsym *);
extern void symtableparam(tnode *);
extern void tablerefer(sigsym *);
extern void refercount(sigsym *);
extern void tablepromote(sigsym *);
extern void duradd(sigsym *);
extern void optlines(sigsym *);
extern void globalrefer(sigsym *);
extern void mirrorupdate(sigsym *);


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*                       void optmain()                         */
/*                                                              */
/* This function coordinates static optimization of SAOL        */
/* code. It is called in the main() function in sfmain.c.       */
/* The second-level functions it uses are in this file.         */
/*                                                              */
/*______________________________________________________________*/

void optmain(void)

{
  sigsym * sptr = instrnametable;  /* to optimize all instrs    */
  tnode * tptr = instances;        /* to optimize instances     */

  optglobalblock();                /* optimize the global block */

  /***************************/
  /* instrument optimization */
  /***************************/

  while (sptr != NULL)
    { 
      currinputwidth = 1;           /* instr input width            */ 
      currinstrwidth = sptr->width; /* instr output width           */
      currinstancename = sptr->val; /* instr ASCII name             */
      curropcodeprefix = sptr->val; /*                              */
      currinstrument = sptr;        /* pointer to instr struct      */
      currinstance = NULL;          /* not an instance              */
      
      inrateupdate(sptr);           /* update rate and width        */
      widthupdate(sptr->defnode->down->
		  next->next->next->next->next->next->next->next->down);
      
      globalclearcount();           /* clear global reference flags  */
      symtableinit(sptr);           /* init local reference flags    */ 
      tablerefer(sptr);             /* ref-count table expressions   */
      refercount(sptr);             /* reference count instr body    */
      tablepromote(sptr);           /* change local tables to global */
      duradd(sptr);                 /* if needed, add dur variable   */
      optlines(sptr);               /* do all optimizations          */
      globalrefer(sptr);            /* update global var ref counts  */
      mirrorupdate(sptr);           /* update local mirror ref count */
      sptr = sptr->next;
    }

  /***************************/
  /* instance optimization   */
  /***************************/

  while (tptr != NULL)
    {  
      currinputwidth = tptr->inwidth;     /* instance input width      */
      currinstrwidth = tptr->sptr->width; /* instance output width     */
      currinstancename = tptr->val;       /* instance ASCII name       */
      curropcodeprefix = tptr->val;       /*                           */
      currinstrument = tptr->sptr;        /* instr struct pointer      */
      currinstance = tptr;                /* instance struct pointer   */

      inrateupdate(tptr->sptr);           /* update rate and width     */
      widthupdate(tptr->sptr->defnode->down->
		  next->next->next->next->next->next->next->next->down);

      globalclearcount();             /* clear global reference flags  */
      symtableinit(tptr->sptr);       /* init local reference flags    */ 
      symtableparam(tptr);            /* init pfields with SEND exprs  */  
      tablerefer(tptr->sptr);         /* ref-count table expressions   */
      refercount(tptr->sptr);         /* reference count instr body    */
      tablepromote(tptr->sptr);       /* change local tables to global */
      duradd(tptr->sptr);             /* if needed, add dur variable   */  
      optlines(tptr->sptr);           /* do all optimizations          */
      globalrefer(tptr->sptr);        /* update global var ref counts  */
      mirrorupdate(tptr->sptr);       /* updates loca mirror ref count */
      tptr = tptr->next;
    }
}


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*                                                              */
/* The second-level functions called by optinstr. The more      */
/* complex functions call routines in optconst.c, optrate.c,    */
/* and optrefer.c, as well as utility functions in this file.   */
/*                                                              */
/*______________________________________________________________*/



extern void globalreferinit(void);
extern void opcodelinecount(tnode * tptr);

/***********************************************************************/
/*                      handles global block                           */
/*                                                                     */      
/*     SEND LP IDENT SEM exprlist SEM identlist RP SEM                 */
/*        TABLE IDENT LP IDENT COM exprstrlist RP                      */
/*                                                                     */      
/***********************************************************************/

void optglobalblock(void)

{
  tnode * tptr, * pvalptr;
  sigsym * ginstr;
  int allconst;

  /*************************************************/
  /* a shortened version of reference counting and */
  /* optimizations done on instrs and instances    */
  /*************************************************/

  /* create global variable tref/cref space */

  globalreferinit();
  vmcheck(ginstr = (sigsym*) malloc(sizeof(sigsym)));
  vmcheck(ginstr->cref = (crefer *) calloc(1, sizeof(crefer)));

  /* update rate for global exprs and opcodes */

  currinstancename = "GBL";
  curropcodeprefix = "GBL";
  currinstrument = NULL;  
  currinstance = NULL;

  tptr = groot;
  while (tptr != NULL)
    {
      pvalptr = NULL;
      if (tptr->ttype == S_SENDDEF)
	pvalptr = tptr->down->next->next->next->next->down;
      if (tptr->ttype == S_TABLE)
	pvalptr = tptr->down->next->next->next->next->next->down;
      if (pvalptr)
	{
	  currinputwidth = inchannels;
	  currinstrwidth = outchannels;
	  widthupdate(pvalptr);
	  curropcoderate = ARATETYPE;
	  currtreerate = UNKNOWN;
	  rateupdate(pvalptr);
	}
      tptr = tptr->next;
    }
  opraterecurse(globalopcodecalls);

  /* initialize symbol tables for opcode calls */

  tptr = globalopcodecalls; 
  while (tptr != NULL)
    {  	  
      symtableinit(tptr->sptr);
      tablerefer(tptr->sptr);
      tptr = tptr->next;
    }

  /* create dummy instrument, do reference counting */

  /* go through each expresson in global block, do ref count */

  tptr = groot;
  while (tptr != NULL)
    {
      pvalptr = NULL;
      if (tptr->ttype == S_SENDDEF)
	pvalptr = tptr->down->next->next->next->next->down;
      if (tptr->ttype == S_TABLE)
	pvalptr = tptr->down->next->next->next->next->next->down;
      while (pvalptr)
	{
	  if (pvalptr->ttype == S_EXPR)
	    exprrefer(ginstr, pvalptr, IRATETYPE);
	  pvalptr = pvalptr->next;
	}
      tptr = tptr->next;
    }
  
  /* add duration variable if used */

  tptr = globalopcodecalls; 
  while (tptr != NULL)
    {  	  
      duradd(tptr->sptr);
      tptr = tptr->next;
    }

  /* do constant optimizaton */

  tptr = groot;
  while (constoptimize && (tptr != NULL))
    {
      allconst = 1 && constoptimize;
      pvalptr = NULL;
      if (tptr->ttype == S_SENDDEF)
	pvalptr = tptr->down->next->next->next->next->down;
      if (tptr->ttype == S_TABLE)
	pvalptr = tptr->down->next->next->next->next->next->down;
      while (pvalptr && constoptimize)
	{
	  if (pvalptr->ttype == S_EXPR)
	    {
	      exprcollapse(IRATETYPE, pvalptr);
	      allconst = allconst && (pvalptr->vol == CONSTANT);
	    }
	  pvalptr = pvalptr->next;
	}
      if (tptr->ttype == S_TABLE)
	{
	  if (allconst)
	    tptr->vol = CONSTANT;
	  tptr->down->next->sptr->consval = 
	    (char *) wavereduceconstants(tptr, NULL);
	  if (tptr->usesinput == 0)
	    haswavegenerator(tptr->down->next->next->next);
	}
      tptr = tptr->next;
    }
  
  /* finish reference counting for code calls */

  tptr = globalopcodecalls; 
  while (tptr != NULL)
    {  	 
      tablepromote(tptr->sptr);      /* change local tables to global */
      globalrefer(tptr->sptr);       /* update global var ref counts  */
      mirrorupdate(tptr->sptr);      /* updates local mirror ref count */
      tptr = tptr->next;
    }

  /* do line counts for all user-defined opcodes */

  if (globalopcodecalls)
    opcodelinecount(globalopcodecalls->sptr->defnode->optr);

  /* free space for dummy instrument */

  free(ginstr->cref);
  free(ginstr);

}


/***********************************************************************/
/*      clears count variables global variable symbol table            */
/***********************************************************************/

void globalclearcount(void)
     
{
  sigsym * sptr;

  sptr = globalsymtable;
  while (sptr != NULL)
    {
      sptr->tref->totimport = 0;
      sptr->tref->totexport = 0;
      sptr->tref->dynaccess = 0;
      sptr = sptr->next;
    }
}


/***********************************************************************/
/*    initializes fields in symbol table for optimization pass         */
/***********************************************************************/

void symtableinit(sigsym * iptr)

{
  sigsym * sptr;
  tnode * tptr;

  vmcheck(iptr->cref = (crefer *) calloc(1, sizeof(crefer)));

  sptr = iptr->defnode->sptr;
  while (sptr != NULL)
    {

      vmcheck(sptr->tref = (trefer *) calloc(1, sizeof(trefer)));

      switch(sptr->kind) {
      case K_PFIELD: 
	/* later special case send statements */
	sptr->vol = VARIABLE;
	sptr->res = ASFLOAT;
	break;
      case K_EXPORT:
      case K_NORMAL:
	if ((currinstrument == iptr) && 
	    (sptr->vartype == SCALARTYPE) &&
	    (sptr->rate == IRATETYPE))
	  {
	    sptr->vol = CONSTANT;
	    sptr->res = ASINT;
	    sptr->consval = dupval("0");
	  }
	else
	  {
	    sptr->vol = VARIABLE;
	    sptr->res = ASFLOAT;
	  }
	break;
      case K_IMPORTEXPORT:
      case K_IMPORT:
	/* later special case easy cases */
	sptr->vol = VARIABLE;
	sptr->res = ASFLOAT;
	break;
      }
      sptr = sptr->next;
    }

  tptr = iptr->defnode->optr;
  while (tptr != NULL)
    {
      symtableinit(tptr->sptr);  
      tptr = tptr->next;
    }

}


/***********************************************************************/
/*    initializes K_PFIELDs in symbol table for effects instruments    */
/*          SEND LP IDENT SEM exprlist SEM identlist RP SEM            */
/***********************************************************************/

void symtableparam(tnode * tptr)

{
  tnode * iptr, * optr;
  tnode * eptr;
  sigsym * sptr;

  eptr = tptr->down->down->next->next->next->next->down;
  sptr = tptr->sptr->defnode->sptr;

  while ((sptr != NULL) && (sptr->kind == K_PFIELD))
    {
      while ((eptr != NULL) && (eptr->ttype != S_EXPR))
	eptr = eptr->next;
      if (eptr == NULL)
	{
	  sptr->vol = CONSTANT;
	  sptr->res = ASINT;
	  sptr->consval = dupval("0");
	}
      else
	{
	  if (eptr->vol == CONSTANT)
	    {
	      sptr->vol = CONSTANT;
	      sptr->res = eptr->res;
	      sptr->consval = dupval(eptr->down->val);
	    }
	  eptr = eptr->next;
	}
      sptr = sptr->next;
    }

  /* mirror bus if instance is sent the output bus */

  tptr->sptr->cref->inmirror |= (outputbusinstance == tptr);

  /* mirror bus if instance is routed to a bus it is sent */

  iptr = tptr->ibus;   /* cycle through input[] buses */
  while (iptr)
    {
      optr = tptr->sptr->obus;   /* cycle through output() buses */
      while (optr)
	{
	  tptr->sptr->cref->inmirror |= (optr->sptr == iptr->sptr);
	  optr = optr->next;
	}
      iptr = iptr->next;
    }

  /* update global flag for shadowbus usage */

  useshadowbus |= tptr->sptr->cref->inmirror;
}


/***********************************************************************/
/*        reference counts expressions in table declarations           */
/***********************************************************************/

void tablerefer(sigsym * iptr)

{
  sigsym * sptr;
  tnode * tptr;

  sptr = iptr->defnode->sptr;
  while (sptr != NULL)
    {
      if ((sptr->vartype == TABLETYPE) && (sptr->kind == K_NORMAL))
        {
          tptr = sptr->defnode->down->next->next->next->next->next->down;
          while (tptr != NULL)
            {
              if (tptr->ttype == S_EXPR)
		exprrefer(iptr, tptr, IRATETYPE);
              tptr = tptr->next;
            }
        }
      sptr = sptr->next;
    }

  tptr = iptr->defnode->optr;
  while (tptr != NULL)
    {
      if (tptr->ttype != S_OPARRAYDECL)
	{
	  tablerefer(tptr->sptr);
	  iptr->cref->MIDIctrl += tptr->sptr->cref->MIDIctrl;
	  iptr->cref->params += tptr->sptr->cref->params;
	  iptr->cref->settune += tptr->sptr->cref->settune;
	  iptr->cref->kadur += tptr->sptr->cref->kadur;
	  iptr->cref->idur += tptr->sptr->cref->idur;
	  iptr->cref->itime += tptr->sptr->cref->itime;
	  iptr->cref->statevars += tptr->sptr->cref->statevars;
	  iptr->cref->statewave += tptr->sptr->cref->statewave;
	  iptr->cref->syslines += tptr->sptr->cref->syslines;
	  iptr->cref->conlines += tptr->sptr->cref->conlines;
	}
      tptr = tptr->next;
    }
}


/***********************************************************************/
/*             reference counts for an instr and its opcodes           */
/*                                                                     */
/* instrdecl: INSTR IDENT LP identlist RP miditag LC vardecls block RC */
/***********************************************************************/

void refercount(sigsym * sptr)

{
  int passtype;        
  tnode * tptr;

  for (passtype = IRATETYPE; passtype <= ARATETYPE; passtype++)
    {
      tptr = sptr->defnode->down->next->next->next
      ->next->next->next->next->next->down;
      while (tptr != NULL)
	{
	  if (tptr->rate == passtype)
	    {
	      ifrefdepth = 0;
	      whilerefdepth = 0;
	      ifrefglobaldepth = 0;
	      whilerefglobaldepth = 0;
	      staterefer(sptr, tptr, passtype);
	    }
	  tptr = tptr->next;
	}
    }

}


extern void dotablepromote(sigsym *);
extern void globalcountupdate(sigsym *, sigsym *);


/***********************************************************************/
/* checks if K_NORMAL tables can become global tables, also updates    */
/* totimport and totexport reference counts for signal variables.      */
/***********************************************************************/

void tablepromote(sigsym * iptr)

{
  sigsym * sptr;
  tnode * tptr;
  tnode * wgen;
  int allconst;

  sptr = iptr->defnode->sptr;
  while (sptr != NULL)
    {
      if ((sptr->vartype == TABLETYPE) && (sptr->kind == K_NORMAL))
        {
	  allconst = 1 && constoptimize;
	  wgen = sptr->defnode->down->next->next->next;
	  tptr = sptr->defnode->down->next->next->next->next->next->down;
	  while ((tptr != NULL) && constoptimize)
            {
	      if (tptr->ttype == S_EXPR)
		{
		  exprcollapse(IRATETYPE, tptr);
		  allconst = allconst && (tptr->vol == CONSTANT);
		}
	      tptr = tptr->next;
            }

	  sptr->defnode->vol = allconst ?  CONSTANT : VARIABLE;
	  sptr->consval = (char *) wavereduceconstants(sptr->defnode,NULL);

	  if (allconst)
	    {
	      if (sptr->defnode->usesinput == 0)
		haswavegenerator(wgen);

	      if ((sptr->tref->assigntot == 0))
		{
		  if (strcmp(wgen->val,"random") && 
		      strcmp(wgen->val,"concat") && 
		      (iptr != startupinstr))
		    {
		      dotablepromote(sptr);
		    }
		}
            }
	  else
	    haswavegenerator(wgen);
        }

      if ((sptr->kind == K_EXPORT) || (sptr->kind == K_IMPORT) ||
	  (sptr->kind == K_IMPORTEXPORT))
	globalcountupdate(sptr, iptr);

      sptr = sptr->next;
    }

  tptr = iptr->defnode->optr;
  while (tptr != NULL)
    {
      if ((tptr->ttype == S_OPCALL) || (tptr->ttype == S_OPARRAYCALL))
	tablepromote(tptr->sptr);
      tptr = tptr->next;
    }
}


/***********************************************************************/
/*       adds duration variable to instr symbol table only             */
/***********************************************************************/

void duradd(sigsym * iptr)

{
  sigsym * sptr;

  if (iptr->cref->idur || iptr->cref->kadur)
    {
      sptr = addvsymend(&(iptr->defnode->sptr), "_dur" , K_NORMAL);
      vmcheck(sptr->tref = (trefer *) calloc(1, sizeof(trefer)));

      sptr->rate = IRATETYPE;
      sptr->res = ASFLOAT;
      sptr->vol = VARIABLE;
    }

}


/***********************************************************************/
/*                optimizes all lines of an instr                      */
/*                                                                     */
/* instrdecl: INSTR IDENT LP identlist RP miditag LC vardecls block RC */
/***********************************************************************/

void optlines(sigsym * iptr)

{
  int num = 0;           /* temporary variable index */
  tnode * xtra;          /* holds new statements */
  tnode * xtop;          /* dummy new statement */
  tnode ** start;        /* first line in instr  */
  int passtype;          /* indexes ...          */
  sigsym * sptr;
  tnode ** tptr;
  tnode * before;


  xtop = xtra = make_tnode("<statement>",S_STATEMENT); 

  start = &(iptr->defnode->down->next->next->next
    ->next->next->next->next->next->down);
  if (!(*start))
    return;    /* no statements to process */

  locsymtable = NULL;

  /* optimize all statements */

  for (passtype = IRATETYPE; passtype <= ARATETYPE; passtype++)
    {
      tptr = start;
      while ((*tptr) != NULL)
	{
	  if ((*tptr)->rate == passtype)
	    {
	      before = *tptr;
	      if (constoptimize)
		{
		  currconstoptif = 0;
		  currconstoptwhile = 0;
		  stateoptconst(passtype, tptr);
		}
	      if (before == (*tptr))
		{
		  if (rateoptimize)
		    stateoptrate(passtype, *tptr, &xtra, &num);
		  tptr = &((*tptr)->next);
		}
	    }
	  else
	    tptr = &((*tptr)->next);
	}
    }

  /* add new statements and symbols */

  (*tptr) = xtop->next;
  if (locsymtable)
    {
      if (!(iptr->defnode->sptr))
	iptr->defnode->sptr = locsymtable;
      else
	{
	  sptr = iptr->defnode->sptr;
	  while (sptr->next != NULL)
	    sptr = sptr->next;
	  sptr->next = locsymtable;
	}
    }

  /* do top-level statement count */

  tptr = start;
  while ((*tptr) != NULL)
    {
      switch ((*tptr)->rate) {
      case IRATETYPE:
	iptr->cref->ilines++;
	break;
      case KRATETYPE:
	iptr->cref->klines++;
	if ((*tptr)->special)
	  iptr->cref->alines++;
	break;
      case ARATETYPE:
	iptr->cref->alines++;
	break;
      }
      tptr = &((*tptr)->next);
    }

  /* do line counts for all user-defined opcodes */

  opcodelinecount(iptr->defnode->optr);

}  


/***********************************************************************/
/*           updates global variable symbol table                      */
/***********************************************************************/

void globalrefer(sigsym * iptr)
     
{
  sigsym * gptr;
  sigsym * sptr = iptr->defnode->sptr;
  tnode * optr = iptr->defnode->optr;
  int skip;

  /* later consider adding currinstrument check, and removing */
  /* currinstrument check below, for faster table accesses.   */

  skip = !strcmp(iptr->val,"startup");
  if (skip && reachableinstrexstart(iptr))
    {
      skip = 0;
    }
  
  while (sptr != NULL)
    {

      if (((sptr->kind == K_EXPORT)||(sptr->kind == K_IMPORTEXPORT)||
	   (sptr->kind == K_IMPORT)) &&
	  (gptr = getvsym(&globalsymtable, sptr->val)))
	{

	  /* update global variable writeback info */

	  if ((sptr->kind == K_EXPORT) || 
	      ((sptr->kind == K_IMPORTEXPORT) && sptr->tref->assigntot))
	    {
	      if ( (!skip) || (sptr->tref->assignrate == KRATETYPE) ||
		   (sptr->tref->assignrate == ARATETYPE))
		{
		  gptr->tref->assigntot++;
		  if (sptr->tref->assigntval)
		    gptr->tref->assigntval++;
		  if (currinstrument &&
		      (currinstrument->vol > gptr->tref->finalinstr))
		    gptr->tref->finalinstr = currinstrument->vol;
		}
	    }

	  /* update global variable access info */

	  if (((sptr->kind == K_IMPORT) || (sptr->kind == K_IMPORTEXPORT)) 
	      && sptr->tref->accesstot && 
	      ((!skip) || (sptr->tref->accessrate == KRATETYPE) ||
	       (sptr->tref->accessrate == ARATETYPE)))
	      gptr->tref->accesstot++;

	  /* update local variable status */

	  sptr->tref->totimport = gptr->tref->totimport;
	  sptr->tref->totexport = gptr->tref->totexport;
	  sptr->tref->dynaccess = gptr->tref->dynaccess;
	}
	
      sptr = sptr->next;
    }

  while (optr != NULL)
    {
      if (optr->ttype != S_OPARRAYDECL)
	globalrefer(optr->sptr);
      optr = optr->next;
    }
}


/***********************************************************************/
/*                  sets mirror flag for variables                     */
/***********************************************************************/

void mirrorupdate(sigsym * iptr)
     
{
  sigsym * sptr = iptr->defnode->sptr;
  tnode * optr;

  while (sptr != NULL)
    {
      refermirror(sptr);
      sptr = sptr->next;
    }

  optr = iptr->defnode->optr;
  while (optr != NULL)
    {
      if (optr->ttype != S_OPARRAYDECL)
	mirrorupdate(optr->sptr);
      optr = optr->next;
    }
}


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*                                                              */
/* Utility functions, used by second level preamble() calls     */
/*                                                              */
/*______________________________________________________________*/


/***********************************************************************/
/* initializes global variable symbol table, used by optglobalblock()  */
/***********************************************************************/

void globalreferinit(void)
     
{
  sigsym * sptr;

  sptr = globalsymtable;
  while (sptr != NULL)
    {
      /* currently being used for tables only      */
      /* to expand to variables see symtableinit() */

      vmcheck(sptr->tref = (trefer *) calloc(1, sizeof(trefer)));

      /* -1 codes "no instrument last touched this variable. if */
      /* it remains after reference count, either its 0 or the  */
      /* startup instrument sets it to a constant value         */
      
      sptr->tref->finalinstr = -1; 

      sptr = sptr->next;
    }
}


/***********************************************************************/
/*  actually promote table to a global -- called in tablepromote()     */
/***********************************************************************/

void dotablepromote(sigsym * sptr)

{
  char * newname;
  int i = 0;

  vmcheck(newname = (char *)calloc(strlen(sptr->val) + 10, sizeof(char)));
  sprintf(newname, "_sym_%s_%i", sptr->val, i++);
  while (getvsym(&globalsymtable, newname) != NULL)
    {
      vmcheck(newname = (char *)calloc(strlen(sptr->val) + 10, sizeof(char)));
      sprintf(newname, "_sym_%s_%i", sptr->val, i++);
    }

  if (addvsym(&globalsymtable, newname, K_NORMAL) != INSTALLED)
    internalerror("optmain.c","dotablepromote addvsym");
  globalsymtable->vartype = TABLETYPE;
  globalsymtable->rate = IRATETYPE;
  globalsymtable->width = 1;
  globalsymtable->defnode = sptr->defnode;
  globalsymtable->consval = sptr->consval;
  vmcheck(globalsymtable->tref = (trefer *) calloc(1, sizeof(trefer)));

  sptr->defnode->down->next->val = newname;

  /* convert sptr entry to an IMPORT */

  sptr->kind = K_IMPORT;
  sptr->val = dupval(newname);
  sptr->consval = NULL;

  /* for now these are for show, could be useful later on */

  sptr->defnode = make_tnode("<vardecl>" , S_VARDECL);  
  sptr->defnode->down = make_tnode("<taglist>", S_TAGLIST);
  sptr->defnode->down->down = make_tnode("IMPORTS", S_IDENT);
  sptr->defnode->down->next = make_tnode("TABLE", S_IDENT);
  sptr->defnode->down->next->next = make_tnode(dupval(newname),
					       S_IDENT);
  sptr->defnode->down->next->next->next = make_tnode(";", S_SEM);

}

/***********************************************************************/
/*         update count variables, used in tablepromote()              */
/***********************************************************************/

void globalcountupdate(sigsym * sptr, sigsym * iptr)

{
  sigsym * gptr;

  if (((sptr->vartype == SCALARTYPE) || (sptr->vartype == VECTORTYPE))
      && (gptr = getvsym(&globalsymtable, sptr->val)))
    {
      if ((sptr->kind == K_EXPORT) || 
	  ((sptr->kind == K_IMPORTEXPORT) && sptr->tref->assigntot))
	gptr->tref->totexport++;
      if (((sptr->kind == K_IMPORT) || (sptr->kind == K_IMPORTEXPORT))
	  && sptr->tref->accesstot)
	gptr->tref->totimport++;
    }
}


/***********************************************************************/
/*     count number of aline/kline/iline in user-defined opcodes       */
/***********************************************************************/

void opcodelinecount(tnode * optr)

{
  tnode * tptr;

  while (optr)
    {
      if ((optr->ttype != S_OPARRAYDECL) && !coreopcodename(optr))
	{
	  tptr = optr->sptr->defnode->down
	    ->next->next->next->next->next->next->next->down;

	  while (tptr)
	    {
	      switch (tptr->rate) {
	      case IRATETYPE:
		optr->sptr->cref->ilines++;
		break;
	      case KRATETYPE:
		optr->sptr->cref->klines++;
		if (tptr->special)
		  optr->sptr->cref->alines++;
		break;
	      case ARATETYPE:
		optr->sptr->cref->alines++;
		break;
	      } 
	      tptr = tptr->next;
	    }

	  /* count all children */

	  opcodelinecount(optr->sptr->defnode->optr);
	}
      optr = optr->next;
    }
}
