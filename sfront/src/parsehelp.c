
/*
#    Sfront, a SAOL to C translator    
#    This file: Works with bison's parser.y SAOL file
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
/*  The parser.y file holds the SAOL grammer sfront uses. This  */
/*  parsehelp.c file contains the functions parser.y rules      */
/*  use to build the initial parse tree.                        */
/*                                                              */
/*  parsehelp.c begins with a set of sections, for the top      */
/*  level complicated parser.y rules, scanning the parser.y     */
/*  file from top to bottom. Following these sections are       */
/*  sections devoted to simpler generic functions used by       */
/*  by many parser.y rules, and finally utility functions.      */
/*                                                              */
/*______________________________________________________________*/

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*                                                              */
/*      Top-level functions for instrument declarations.        */
/*                                                              */
/*______________________________________________________________*/

extern void tableexprcheck(void);

/***********************************************************************/
/* instrdecl: INSTR IDENT LP identlist RP miditag LC vardecls block RC */
/***********************************************************************/

tnode * make_instrdecl(tnode * t_instr, tnode * t_ident, tnode * t_lp,
		       tnode * t_identlist, tnode * t_rp,
                       tnode * t_miditag, tnode * t_lc, 
		       tnode * t_vardecls, tnode * t_block, 
		       tnode * t_rc)

{
  tnode * retptr;
  int symstate;
  tnode * tptr;
  sigsym * sptr;

  retptr = make_tnode("<instrdecl>",S_INSTRDECL);

  /* make sure name is unique over instr and opcodes */

  if ((getsym(&opcodenametable, t_ident) != NULL) ||
       wavegeneratorname(t_ident) || coreopcodename(t_ident))
    {
      printf("Error: Instr name %s not unique.\n\n",
	     t_ident->val);
      showerrorplace(t_ident->linenum, t_ident->filename);
    }
  symstate = addsym(&instrnametable, t_ident);
  symcheck(symstate, t_ident);

  instrnametable->kind = K_INSTRNAME;
  instrnametable->obus = NULL;
  instrnametable->defnode = retptr;
  instrnametable->width = OUTCHANNELSWIDTH;
  instrnametable->maxifstate = conditionalblocks;

  /* add nodes */
  retptr->down = t_instr;

  t_instr->next = t_ident;
  t_ident->next = t_lp;

  t_lp->next = make_tnode("<identlist>",S_IDENTLIST);
  t_lp->next->down = t_identlist;
  t_lp->next->next = t_rp;

  if (t_miditag == NULL)
    {
      t_rp->next = make_tnode("<miditag>",S_MIDITAG);
      t_rp->next->next = t_lc;
    }
  else
    {
      instrnametable->miditag = 1;
      t_rp->next = t_miditag;
      t_miditag->next = t_lc;
      tptr = t_miditag->down->next->down;
      while (tptr != NULL)
	{
	  addvsymsort(&instrpresets, tptr->val, K_PRESET);
	  sptr = getvsym(&instrpresets, tptr->val);

	  /* addvsymsort sets sptr->width to atoi(tptr->val) */

	  if (sptr->width > maxmidipreset)
	    maxmidipreset = sptr->width;
	  sptr->defnode = make_tnode(t_ident->val, S_INSTR);
	  sptr->defnode->sptr = instrnametable;
	  tptr = tptr->next;
	}
    }

  t_lc->next = make_tnode("<vardecls>",S_VARDECLS);
  t_lc->next->down = t_vardecls;
  t_lc->next->next = make_tnode("<block>",S_BLOCK);

  t_lc->next->next->down = t_block;
  t_lc->next->next->next = t_rc;

  tableexprcheck();

  retptr->sptr = reversetable(locsymtable);
  locsymtable = NULL;

  retptr->optr = locopcodecalls;
  locopcodecalls = NULL;

  retptr->dptr = locdyncalls;
  locdyncalls = NULL;

  isaninstr = 0;
  conditionalblocks = 0;

  return retptr;
}


extern void reservednames(tnode *);

/***********************************************************************/
/* adds parameter fields into symbol table before instr parse begins   */
/***********************************************************************/

void make_instrpfields(tnode * t_identlist)

{
  int symstate;
  int i = 0;
  tnode * tptr = t_identlist;

  while (tptr != NULL)
    if (tptr->ttype == S_IDENT)
      {
	i++;
	reservednames(tptr);
	symstate = addsym(&locsymtable, tptr);
	symcheck(symstate,tptr);
	locsymtable->kind = K_PFIELD;
	tptr->sptr = locsymtable;
	tptr->width = locsymtable->width = 1;
	tptr->rate = locsymtable->rate = IRATETYPE;
	tptr = tptr->next;
      }
    else
      tptr = tptr->next;

  if (i > 255)
    {
      printf("Error: Instr or template has more than 255 parameters.\n\n");
      showerrorplace(t_identlist->linenum, t_identlist->filename);
    }

  if (i > numpfields)
    numpfields = i;

  isaninstr = 1;
  conditionalblocks = 0;
  
}


/***********************************************************************/
/* make_miditag:    part of instr definition                           */
/***********************************************************************/

tnode * make_miditag(tnode * t_preset, tnode * t_intlist)

{
  tnode * retptr;

  retptr = make_tnode("<miditag>",S_MIDITAG);
  retptr->down = t_preset;
  if (strcmp("preset",t_preset->val))
    {
      printf("Error: Parser expected token preset.\n\n");
      showerrorplace(t_preset->linenum, t_preset->filename);
    }
  t_preset->next = make_tnode("<intlist>",S_INTLIST);
  t_preset->next->down = t_intlist;

  return retptr;
}


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*                                                              */
/*      Top-level functions for opcode declarations.            */
/*                                                              */
/*______________________________________________________________*/

/***********************************************************************/
/*  optype IDENT LP paramlist RP LC opvardecls block RC                */
/***********************************************************************/

tnode * make_opcodedecl(tnode * t_optype, tnode * t_ident, tnode * t_lp,
			tnode * t_paramlist, tnode * t_rp, tnode * t_lc,
			tnode * t_opvardecls, tnode *  t_block, tnode * t_rc)
{

  tnode * retptr;
  tnode * tptr;
  tnode ** rstat;

  retptr = make_tnode("<opcodedecl>",S_OPCODEDECL);
  opcodenametable->defnode = retptr;
  opcodenametable->maxifstate = conditionalblocks;

  /* add nodes */

  retptr->down = t_optype;
 
  t_optype->next = t_ident;
  t_ident->next = t_lp;

  t_lp->next = make_tnode("<paramlist>",S_PARAMLIST);
  t_lp->next->down = t_paramlist;
  t_lp->next->next = t_rp;

  t_lp->next->rate = UNKNOWN;
  tptr = t_paramlist;
  while (tptr != NULL)
    {
      if ( (tptr->ttype == S_PARAMDECL) && 
	   (tptr->rate < XRATETYPE)     &&
	   (tptr->rate > t_lp->next->rate) &&
	   ((tptr->vartype == SCALARTYPE) || (tptr->vartype == VECTORTYPE)))
	t_lp->next->rate = tptr->rate;
      tptr = tptr->next;
    }

  t_rp->next = t_lc;

  t_lc->next = make_tnode("<opvardecls>",S_OPVARDECLS);
  t_lc->next->down = t_opvardecls;
  t_lc->next->next = make_tnode("<block>",S_BLOCK);

  /* add return statement on end it needed */

  if (t_block)
    {
      tptr = t_block;
      while (tptr->next != NULL)
	tptr = tptr->next;
      rstat = &(tptr->next);
    }
  else
    rstat = &t_block;

  if ((t_block == NULL) || (tptr->down->ttype != S_RETURN))
    {
      *rstat = make_tnode("<statement>",S_STATEMENT);
      (*rstat)->down = make_tnode("return",S_RETURN);
      (*rstat)->down->next = make_tnode("(",S_LP);
      (*rstat)->down->next->next = make_tnode("<exprlist>",S_EXPRLIST);
      (*rstat)->down->next->next->next = make_tnode(")",S_RP);
      tptr = (*rstat)->down->next->next;
      tptr->down = make_tnode("<expr>",S_EXPR);
      tptr->down->rate = IRATETYPE;
      tptr->down->vol = CONSTANT;
      tptr->down->down = make_tnode("0.0",S_NUMBER);
      tptr->down->down->rate = IRATETYPE;
      tptr->down->down->vol = CONSTANT;
    }

  t_lc->next->next->down = t_block;
  t_lc->next->next->next = t_rc;

  tableexprcheck();

  retptr->sptr = reversetable(locsymtable);
  locsymtable = NULL;

  if (t_optype->ttype != S_AOPCODE)
    {
      tptr = locopcodecalls;
      while (tptr != NULL)
	{
	  if (coreopcodespecial(tptr))
	    {
	      printf("Error: Specialop %s may not appear in ", tptr->val);

	      switch (t_optype->ttype) { 
	      case S_IOPCODE:
		printf("an iopcode definition.\n");
		break;
	      case S_KOPCODE:
		printf("a kopcode definition.\n");
		break;
	      case S_OPCODE:
		printf("a polymorphic opcode definition.\n");
		break;
	      }
	      showerrorplace(tptr->optr->down->linenum, 
			     tptr->optr->down->filename);
	    }
	  tptr = tptr->next;
	}
    }

  retptr->optr = locopcodecalls;
  locopcodecalls = NULL;

  retptr->dptr = locdyncalls;
  locdyncalls = NULL;

  return retptr;

}

/***********************************************************************/
/*  optype IDENT {make_opcodetype();} ...                              */
/***********************************************************************/

void make_opcodetype(tnode * t_optype, tnode * t_ident)

{

  int symstate;

  /* make sure name is unique over instr and opcodes */

  if ((getsym(&instrnametable, t_ident) != NULL) ||
      wavegeneratorname(t_ident) || coreopcodename(t_ident))
    {
      printf("Error: Opcode name %s not unique.\n\n",
	     t_ident->val);
      showerrorplace(t_ident->linenum, t_ident->filename);
    }
  symstate = addsym(&opcodenametable, t_ident);
  symcheck(symstate, t_ident);
  opcodenametable->rate = t_optype->rate;
  opcodenametable->kind = K_OPCODENAME;
  opcodenametable->width = UNKNOWN;

  isaninstr = 0;
  nonpolyparams = 0;
  conditionalblocks = 0;

}


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*                                                              */
/*      Top-level function for the global block.                */
/*                                                              */
/*______________________________________________________________*/

/***********************************************************************/
/* globaldecl:  GLOBAL LC globalblock RC                               */
/***********************************************************************/

tnode * make_globaldecl(tnode * t_global, tnode * t_lc,
			tnode * t_globalblock, tnode * t_rc)


{
  tnode * retptr;
  sigsym * ptr;
  tnode * tptr;

  globalblockcount++;
  if (isocompliant && (globalblockcount > 1))
    {
      printf("Error: Only one global block per orchestra.\n\n");
      showerrorplace(t_global->linenum, t_global->filename);
    }
  suspendvarchecks = 0;
  if (globalblockcount == 1)
    {
      retptr = make_tnode("<globalblock>",S_GLOBALBLOCK);
      groot = retptr->down = t_globalblock;
      retptr = make_stree(t_global,t_lc,retptr,t_rc,
			  "<globaldecl>",S_GLOBALDECL);
    }
  else
    {
      if (groot)
	{
	  tptr = groot;
	  while (tptr->next != NULL)
	    tptr = tptr->next;
	  tptr->next = t_globalblock;
	}
      else
	groot = t_globalblock;
      retptr = NULL;
    }
  ptr = locsymtable;
  while (ptr != NULL)
    {
      if ((ptr->kind == K_IMPORT) || 
	  (ptr->kind == K_EXPORT)||(ptr->kind == K_IMPORTEXPORT))
	{
	  printf("Error: Import/export tags in global declaration.\n");
	  showerrorplace(ptr->defnode->linenum, ptr->defnode->filename);
	}
      if (ptr->vartype == TMAPTYPE)
	{
	  printf("Error: Tablemap in global declaration.\n");
	  showerrorplace(ptr->defnode->down->linenum, 
			 ptr->defnode->down->filename);
	}
      if (ptr->rate == ARATETYPE)
	{
	  printf("Error: Global declaration may not be a-rate.\n");
	  showerrorplace(ptr->defnode->linenum, ptr->defnode->filename);
	}
      ptr = ptr->next;
    }

  tptr = locopcodecalls;
  while (tptr != NULL)
    {
      if (tptr->ttype == S_OPARRAYDECL)
	{
	  printf("Error: Global declarations may not be oparrays.\n");
	  showerrorplace(tptr->optr->down->linenum, tptr->optr->down->filename);
	}
      tptr = tptr->next;
    }

  if (globalsymtable)
    {
      ptr = globalsymtable;
      while (ptr->next != NULL)
	ptr = ptr->next;
      ptr->next = reversetable(locsymtable);
    }
  else
    globalsymtable = reversetable(locsymtable);

  if (globalopcodecalls)
    {
      tptr = globalopcodecalls;
      while (tptr->next != NULL)
	tptr = tptr->next;
      tptr->next = locopcodecalls;
    }
  else
    globalopcodecalls = locopcodecalls;

  locsymtable = NULL;
  locopcodecalls = NULL;
  return retptr;

}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*                                                              */
/*      Top-level functions for templates.                      */
/*                                                              */
/*______________________________________________________________*/


/************************************************************************/
/*templatedecl    : TEMPLATE LT identlist GT LP identlist RP            */
/*                  MAP LC identlist RC                                 */
/*                  WITH {make_templatepfields($6,$10);} LC mapblock RC */
/*                  LC vardecls block RC                                */
/* only non-terminals sent to function                                  */
/* needs to be updated to handle midi                                   */
/************************************************************************/

tnode * make_templatedecl(tnode * t_instrs,  tnode * t_miditags, 
			  tnode * t_pfields, tnode * t_maptokens,
                          tnode * t_mapargs, tnode * t_vardecls,
			  tnode * t_block)

{
  tnode * ptokens;          /* identlist for pfield string*/ 
  tnode * vtokens;
  tnode * retptr = NULL;
  tnode * cptr = NULL;      /* initialization not needed */
  tnode * tptr;
  tnode * lptr;
  tnode * mptr;
  sigsym * templatevars;
  sigsym * sptr;
  int symstate;
  int midinum;

  suspendvarchecks = 0;
  isaninstr = 1;

  tlocsymtable = reversetable(locsymtable);
  tlocopcodecalls = locopcodecalls;
  tlocdyncalls = locdyncalls;
  
  while (t_instrs != NULL)
    {
      if (t_instrs->ttype == S_IDENT)
	{
	  /* build instr structure */
	  if (retptr == NULL)
	    retptr = cptr = make_tnode("<instrdecl>",S_INSTRDECL);
	  else
	    {
	      cptr->next = make_tnode("<instrdecl>",S_INSTRDECL);
	      cptr = cptr->next;
	    }
	  cptr->down = make_tnode("INSTR",S_INSTR);
	  cptr->down->next = make_tnode(t_instrs->val, S_IDENT);
	  cptr->down->next->next =  make_tnode("(",S_LP);
	  cptr->down->next->next->next = ptokens =
	    make_tnode("<identlist>",S_IDENTLIST);
	  ptokens->next = make_tnode(")",S_RP);
	  ptokens->next->next = make_tnode("<miditag>",S_MIDITAG);

	  ptokens->next->next->next = make_tnode("{",S_LC);
	  ptokens->next->next->next->next = vtokens =
	    make_tnode("<vardecls>",S_VARDECLS);
	  vtokens->next = make_tnode("<block>",S_BLOCK);
	  vtokens->next->next =  make_tnode("}",S_RC);

	  /* check in name */

	  if ((getsym(&opcodenametable, t_instrs) != NULL) ||
	      wavegeneratorname(t_instrs) || coreopcodename(t_instrs))
	    {     
	      printf("Error: Template instr name %s not unique.\n\n",
		     t_instrs->val);
	      showerrorplace(t_instrs->linenum, t_instrs->filename);
	    }
	  symstate = addsym(&instrnametable, t_instrs);
	  symcheck(symstate, t_instrs);

	  instrnametable->kind = K_INSTRNAME;
	  instrnametable->obus = NULL;
	  instrnametable->defnode = cptr;
	  instrnametable->width = OUTCHANNELSWIDTH;
	  instrnametable->maxifstate = conditionalblocks;

	  if (t_miditags != NULL)
	    {
	      instrnametable->miditag = 1;
	      mptr = t_miditags->down;
	      t_miditags = t_miditags->next;
	      while (mptr != NULL)
		{
		  if (mptr->ttype == S_EXPR)
		    {		      
		      co_constcollapse(mptr);

		      if (((mptr->down->ttype != S_INTGR) && (mptr->down->ttype != S_NUMBER))
			  || (mptr->down->next != NULL))
			{
			  printf("Error: Template MIDI num must resolve to a constant.\n\n"
				 );
			  while (mptr->down != NULL)
			    mptr = mptr->down;
			  showerrorplace(mptr->linenum, mptr->filename);
			}
		      if (mptr->down->ttype == S_NUMBER)
			{
			  midinum = (int)(atof(mptr->down->val)+0.5);
			  vmcheck(mptr->down->val = (char *) calloc(20, 1));
			  sprintf(mptr->down->val, "%i", midinum);
			  mptr->down->ttype = S_INTGR;
			  mptr->down->res = ASINT;
			}

		      addvsymsort(&instrpresets, mptr->down->val, K_PRESET); 
		      sptr = getvsym(&instrpresets, mptr->down->val);

		      /* addvsymsort sets sptr->width to atoi(mptr->down->val) */

		      if (sptr->width > maxmidipreset)
			maxmidipreset = sptr->width;
		      sptr->defnode = make_tnode(t_instrs->val, S_INSTR);
		      sptr->defnode->sptr = instrnametable;
  
		    }
		  mptr = mptr->next;
		}
	    }

	  /* build substitution list */

	  if (t_mapargs == NULL)
	    lptr = NULL;
	  else
	    {
	      lptr = t_mapargs->down;
	      t_mapargs = t_mapargs->next;
	    }

	  templatevars = NULL;
	  tptr = t_maptokens;
	  while (tptr != NULL)
	    {
	      if (tptr->ttype == S_IDENT)
		{
		  reservednames(tptr);
		  symcheck(addsym(&templatevars,tptr),tptr);
		  templatevars->defnode = lptr; 
		  if (lptr == NULL)
		    {
		      printf("Error: Not enough template elements for %s.\n\n",
			     tptr->val);
		      showerrorplace(tptr->linenum, tptr->filename);
		    }
		  lptr = lptr->next;   /* skip over comma */
		  if (lptr != NULL)
		    lptr = lptr->next;
		  templatevars->defnode->next = NULL;
		}
	      tptr = tptr->next;
	    }

	  cptr->sptr = locsymtable = sclone(tlocsymtable);
	  locopcodecalls = tclone(tlocopcodecalls);
	  locdyncalls = tclone(tlocdyncalls);
	  vtokens->next->down = treeclone(t_block,&templatevars,DOSUB); 
	  vtokens->down = treeclone(t_vardecls,&templatevars,DOSUB);
	  cptr->optr = locopcodecalls;
	  cptr->dptr = locdyncalls;
	  varupdate(vtokens->next->down,&locsymtable);
	  varupdate(vtokens->down,&locsymtable);
	  tableexprcheck();
	}
      t_instrs = t_instrs->next;
    }

  locsymtable = NULL;
  locopcodecalls = NULL;
  locdyncalls = NULL;
  isaninstr = 0;
  conditionalblocks = 0;
  return retptr;
}

/***********************************************************************/
/*      patches opcode and oparray calls in template maplists          */
/***********************************************************************/

void templateopcodepatch(void)

{
  tnode * tptr = locopcodecalls;


  /* reset state machine -- for null mapblocks */

  lexstatemachine = TEMPLATE_REST;

  /* patch opcalls */

  while (tptr != NULL)
    {
      if (tptr->ttype == S_OPARRAYCALL)
	{
	  tptr->optr->optr = tptr->optr->down->optr = &maplistoparraycall;
	}
      else
	{
	  tptr->optr->optr = tptr->optr->down->optr = &maplistopcall;
	}
      tptr = tptr->next;
    }
  
  locopcodecalls = NULL;
}

/***********************************************************************/
/* adds parameter fields into symbol table before template parse begins*/
/***********************************************************************/

void make_templatepfields(tnode * t_preset, tnode * t_identlist)

{
  if (t_preset != NULL)
    {
      if (strcmp("preset",t_preset->val))
	{
	  printf("Error: Syntax error in template miditag.\n\n");
	  showerrorplace(t_preset->linenum, t_preset->filename);
	}
    }
  make_instrpfields(t_identlist);
  suspendvarchecks = 1;

  lexstatemachine = TEMPLATE_ACTIVE;

}

/***********************************************************************/
/* make_mapblock:    part of templatedecl definition                   */
/***********************************************************************/

tnode * make_mapblock(tnode * t_list, tnode * t_map)


{
  tnode * retptr;
  tnode * ptr;

  ptr = make_tnode("<mapblock>",S_BLOCK);
  ptr->down = t_map;
  if (t_list == NULL)
    {
      retptr = ptr;
    }
  else
    {
      retptr = t_list;
      while (t_list->next != NULL)
	t_list = t_list->next;
      t_list->next = ptr;
    }

  return retptr;
}


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*                                                              */
/* Top-level functions for items exclusive to the global block. */
/*                                                              */
/*                                                              */
/* globaldef    : rtparam                                       */ 
/*              | routedef                                      */
/*              | senddef                                       */
/*              | seqdef                                        */
/*                                                              */
/*______________________________________________________________*/

/***********************************************************************/
/*rtparam       : SRATE INTGR SEM        {$$ = make_rtparam($1,$2,$3);}*/ 
/*              | KRATE INTGR SEM        {$$ = make_rtparam($1,$2,$3);}*/ 
/*              | INCHANNELS INTGR SEM   {$$ = make_rtparam($1,$2,$3);}*/ 
/*              | OUTCHANNELS INTGR SEM  {$$ = make_rtparam($1,$2,$3);}*/
/*              | INTERP INTGR SEM       {$$ = make_rtparam($1,$2,$3);}*/ 
/***********************************************************************/

tnode * make_rtparam(tnode * t_param, tnode * t_intgr, tnode * t_sem)


{
  tnode * retptr;

  retptr = make_stree(t_param, t_intgr, t_sem, NULL, "<rtdef>", S_RTDEF);
  switch (t_param->ttype) {
  case S_SRATE:
    if (srate >= 0)
      {
	printf("Error: Srate parameter may only be set once.\n\n");
	showerrorplace(t_param->linenum, t_param->filename);
      }
    sscanf(t_intgr->val,"%i",&srate); 
    if ((srate < 4000) || (srate > 96000))
      {
	printf("Error: Srate parameter out of range (4000-96000).\n\n");
	showerrorplace(t_param->linenum, t_param->filename);
      }
    if (srate < krate)
      {
	printf("Error: Parameter srate < krate.\n\n");
	showerrorplace(t_param->linenum, t_param->filename);
      }
    break;
  case S_KRATE:
    if (krate >= 0)
      {
	printf("Error: Krate parameter may only be set once.\n\n");
	showerrorplace(t_param->linenum, t_param->filename);
      }
    sscanf(t_intgr->val,"%i",&krate); 
    if ((krate < 1) || (krate > 96000))
      {
	printf("Error: Krate parameter out of range (1-96000).\n\n");
	showerrorplace(t_param->linenum, t_param->filename);
      }
    if ((srate > 0) && (srate < krate))
      {
	printf("Error: Parameter krate > srate.\n\n");
	showerrorplace(t_param->linenum, t_param->filename);
      }
    break;
  case S_INCHANNELS:
    if (inchannels >= 0)
      {
	printf("Error: Inchannels parameter may only be set once.\n\n");
	showerrorplace(t_param->linenum, t_param->filename);
      }
    sscanf(t_intgr->val,"%i",&inchannels);
    if ((inchannels < 0))
      {
	printf("Error: Inchannels parameter must be >= 0.\n\n");
	showerrorplace(t_param->linenum, t_param->filename);
      }
    break;
  case S_OUTCHANNELS:
    if (outchannels >= 0)
      {
	printf("Error: Outchannels parameter may only be set once.\n\n");
	showerrorplace(t_param->linenum, t_param->filename);
      }
    sscanf(t_intgr->val,"%i",&outchannels);
    if ((outchannels < 0))
      {
	printf("Error: Outchannels parameter must be >= 0.\n\n");
	showerrorplace(t_param->linenum, t_param->filename);
      }
    break;
  case S_INTERP:
    if (interp >= 0)
      {
	printf("Error: Interp parameter may only be set once.\n\n");
	showerrorplace(t_param->linenum, t_param->filename);
      }
    sscanf(t_intgr->val,"%i",&interp);
    if ((interp != INTERP_LINEAR) && (interp != INTERP_SINC))
      {
	printf("Error: Interp parameter out of range (0-1).\n\n");
	showerrorplace(t_param->linenum, t_param->filename);
      }
    break;
  }


  return retptr;
}

/***********************************************************************/
/* routedef:    ROUTE LP IDENT COM identlist RP SEM                    */
/***********************************************************************/

tnode * make_routedef(tnode * t_route, tnode * t_lp, tnode * t_ident,
		      tnode * t_com, tnode * t_identlist, tnode * t_rp,
		      tnode * t_sem)


{
  tnode * retptr;

  retptr = make_stree(t_route, t_lp, t_ident, t_com, "<routedef>", S_ROUTEDEF);
  t_com->next= make_tnode("<identlist>",S_IDENTLIST);
  t_com->next->down = t_identlist;
  t_com->next->next = t_rp;
  t_rp->next = t_sem;

  if (t_identlist == NULL)
    {
      printf("Error: Route statements must reference an instrument.\n\n");
      showerrorplace(t_route->linenum, t_route->filename);
    }
  if (!strcmp(t_ident->val,"input_bus"))
    {
      printf("Error: Route statements references input_bus.\n\n");
      showerrorplace(t_route->linenum, t_route->filename);
    }

  return retptr;
}

/***********************************************************************/
/* senddef:  SEND LP IDENT SEM exprlist SEM identlist RP SEM           */
/***********************************************************************/

tnode * make_senddef(tnode * t_send, tnode * t_lp, tnode * t_ident,
		     tnode * t_sem1, tnode * t_exprlist, tnode * t_sem2,
		     tnode * t_namelist, tnode * t_rp, tnode * t_sem3)


{
  tnode * retptr, * ptr;
  char * name;

  if (t_namelist == NULL)
    {      
      printf("Error: Send statement must reference a bus.\n\n");
      showerrorplace(t_send->linenum, t_send->filename);
    }

  retptr = make_stree(t_send, t_lp, t_ident, t_sem1, "<senddef>", S_SENDDEF);
  t_sem1->next = make_tnode("<exprlist>",S_EXPRLIST);
  t_sem1->next->down = t_exprlist;
  t_sem1->next->next = t_sem2;
  t_sem2->next = make_tnode("<namelist>",S_NAMELIST);
  t_sem2->next->down = t_namelist;
  t_sem2->next->next = t_rp;
  t_rp->next = t_sem3;

  /* send statement defines buses - we log definitions below */

  ptr = t_namelist;
  while (ptr != NULL)
    if (ptr->ttype == S_NAME)
      {
	name = ptr->down->val;
	if ((addvsym(&busnametable, name, K_BUSNAME) != INSTALLED)
	    &&(!strcmp(name, "output_bus")))
	  {
	    printf("Error: Multiple definition of output_bus.\n\n");
	    showerrorplace(ptr->down->linenum, ptr->down->filename);
	  }
	if (!strcmp(name,"output_bus"))
	  {
	    if (outputbusinstance != NULL)
	      {
		printf("Error: Output_bus sent to multiple instances.\n\n");
		showerrorplace(ptr->down->linenum, ptr->down->filename);
	      }
	    outputbusinstance = make_tnode(t_ident->val, S_INSTANCE);
	    outputbusinstance->down = retptr;
	  }
	busnametable->defnode = retptr->down; /* send command creating bus */
	busnametable->width = 0;
	ptr->sptr = getsym(&busnametable,ptr->down);
	ptr = ptr->next;
      }
    else
      ptr = ptr->next;


  return retptr;
}

/***********************************************************************/
/* seqdef:    SEQUENCE LP identlist RP SEM                             */
/***********************************************************************/

tnode * make_seqdef(tnode * t_sequence, tnode * t_lp, tnode * t_identlist,
		    tnode * t_rp, tnode * t_sem)


{
  tnode * retptr;

  retptr = make_stree(t_sequence, t_lp, make_tnode("<identlist>",S_IDENTLIST),
        t_rp, "<seqdef>", S_SEQDEF);
  t_rp->next = t_sem;
  t_lp->next->down = t_identlist;
  if ((t_identlist == NULL)||(t_identlist->next == NULL)
                           ||(t_identlist->next->next == NULL))
    {
      printf("Error: Sequence statements must reference > 1 instruments.\n\n");
      showerrorplace(t_sequence->linenum, t_sequence->filename);
    }
  return retptr;
}


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*                                                              */
/* Top-level functions for statements in instrs or opcodes.     */
/*                                                              */
/*______________________________________________________________*/

/***********************************************************************/
/* statement (done): lvalue EQ expr SEM                                */
/*           (done)| expr SEM                                          */
/*           (done)| IF LP expr RP LC block RC                         */
/*           (done)| IF LP expr RP LC block RC ELSE LC block RC        */
/*           (done)| WHILE LP expr RP LC block RC                      */
/*           (done)| INSTR IDENT LP exprlist RP SEM                    */
/*           (done)| OUTPUT LP exprlist RP SEM                         */
/*           (done)| SPATIALIZE LP exprlist RP SEM                     */
/*           (done)| OUTBUS LP IDENT COM exprlist RP SEM               */
/*           (done)| EXTEND LP expr RP SEM                             */
/*           (done)| TURNOFF SEM                                       */
/*           (done)| RETURN LP exprlist RP SEM                         */
/*           (done)| PRINTF LP exprstrlist RP SEM                      */
/*    exprlist can be NULL -> t_three,t_four,t_five can be null        */
/***********************************************************************/

tnode * make_statement(tnode * t_one, tnode * t_two, tnode * t_three,
		       tnode * t_four, tnode * t_five, tnode * t_six,
                       tnode * t_seven, tnode * t_eight, tnode * t_nine,
		       tnode * t_ten, tnode * t_eleven)
{
  tnode * retptr;
  tnode * ptr;

  retptr = make_tnode("<statement>",S_STATEMENT);

  retptr->down = t_one;

  if (t_two->ttype == S_SEM)        /* expr; and turnoff; statements */
    {
      t_one->next = t_two;
      if (t_one->ttype == S_EXPR)     /* expr SEM */
	{
	  tablecheck(t_one);
	}
      else                            /* turnoff SEM */
	{
	}
      return retptr;
    }



  if (t_one->ttype == S_LVALUE)                /* lvalue EQ expr SEM */
    {
      tablecheck(t_three);
      t_one->next = t_two;
      t_two->next = t_three;
      t_three->next = t_four;
      t_one->res = ASFLOAT;
      return retptr;
    }


  if ((t_six == NULL)&&(t_seven == NULL))  
    {
      if (t_one->ttype == S_OUTPUT)       /* OUTPUT LP exprlist RP SEM     */
	{
	  tablelistcheck(t_three);
	  t_one->next = t_two;
	  t_two->next = make_tnode("<exprlist>",S_EXPRLIST);
	  t_two->next->down = t_three;
	  t_two->next->next = t_four;
	  t_four->next = t_five;
	  if (t_three == NULL)
	    {
	      printf("Error: Output statements must have width >= 1.\n\n");
	      showerrorplace(t_one->linenum, t_one->filename);
	    }
	  return retptr;
	  
	}
      
      if (t_one->ttype == S_RETURN)      /* RETURN LP exprlist RP SEM     */
	{
	  if (isaninstr == 1)
	    {
	      printf("Error: Instrs cannot contain return statements.\n\n");
	      showerrorplace(t_one->linenum, t_one->filename);
	    }
	  tablelistcheck(t_three);
	  if (t_three == NULL)
	    {
	      t_three = make_tnode("<expr>", S_EXPR);
	      t_three->down = make_tnode("0", S_INTGR);
	      t_three->res = t_three->down->res = ASINT;
	      t_three->vol = t_three->down->vol = CONSTANT;
	    }
	  t_one->next = t_two;
	  t_two->next = make_tnode("<exprlist>",S_EXPRLIST);
	  t_two->next->down = t_three;
	  t_two->next->next = t_four;
	  t_four->next = t_five;
	  return retptr;
	}

      if (t_one->ttype == S_PRINTF)      /* PRINTF LP exprstrlist RP SEM   */
	{
	  tablelistcheck(t_three);
	  t_one->next = t_two;
	  t_two->next = make_tnode("<exprstrlist>",S_EXPRSTRLIST);
	  t_two->next->down = t_three;
	  t_two->next->next = t_four;
	  t_four->next = t_five;
	  return retptr;
	}

      if (t_one->ttype == S_EXTEND)    /* EXTEND LP expr RP SEM */ 
	{                       
	  tablecheck(t_three);
	  t_one->next = t_two;
	  t_two->next = t_three;
	  t_three->next = t_four;
	  t_four->next = t_five;
	  return retptr;
	}


      if (t_one->ttype == S_SPATIALIZE)     /* SPATIALIZE LP exprlist RP SEM */  
	{
	  tablelistcheck(t_three);
	  t_one->next = t_two;
	  t_two->next = make_tnode("<exprlist>",S_EXPRLIST);
	  t_two->next->down = t_three;
	  t_two->next->next = t_four;
	  t_four->next = t_five;
	  if (locopcodecalls == NULL)
	    ptr = locopcodecalls = make_tnode(t_one->val, S_OPCALL);
	  else
	    {
	      ptr = locopcodecalls;
	      while (ptr->next != NULL)
		ptr = ptr->next;
	      ptr->next = make_tnode(t_one->val, S_OPCALL);
	      ptr = ptr->next;
	    }
	  t_one->optr = retptr->optr = ptr;
	  ptr->optr = retptr;
	  return retptr;
	}

      return NULL;
    }


  if (t_seven == NULL)
    {
      /* INSTR IDENT LP exprlist RP SEM */

      tablelistcheck(t_four);
      t_two->ttype = S_INSTRNAME;
      t_one->next = t_two;
      t_two->next = t_three;
      t_three->next = make_tnode("<exprlist>",S_EXPRLIST);
      t_three->next->down = t_four;
      t_three->next->next = t_five;
      t_five->next = t_six;

      if (locdyncalls == NULL)
	ptr = locdyncalls = make_tnode(t_two->val, S_INSTR);
      else
	{
	  ptr = locdyncalls;
	  while (ptr->next != NULL)
	    ptr = ptr->next;
	  ptr->next =make_tnode(t_two->val, S_INSTR);
	  ptr = ptr->next;
	}
      ptr->dptr = retptr;
      retptr->dptr = ptr;
      return retptr;
    }

  /* handle IF, WHILE, and OUTBUS here, do linking separately */
  
  if (t_one->ttype == S_IF) /*  IF LP expr RP LC block RC */
    {                       /*  IF LP expr RP LC block RC ELSE LC block RC */
      conditionalblocks++;
      tablecheck(t_three);
      t_one->next = t_two;
      t_two->next = t_three;
      t_three->next = t_four;
      t_four->next = t_five;
      t_five->next = make_tnode("<block>",S_BLOCK);
      t_five->next->down = t_six;
      t_five->next->next = t_seven;
      if (t_eight == NULL)
	return retptr;
      else               /* ELSE */
	{
	  conditionalblocks++;
	  t_seven->next = t_eight;
	  t_eight->next = t_nine;
	  t_nine->next = make_tnode("<block>",S_BLOCK);
	  t_nine->next->down = t_ten;
	  t_nine->next->next = t_eleven;
	  return retptr;
	}
    }

  if (t_one->ttype == S_WHILE)    /*  WHILE LP expr RP LC block RC  */
    {                           
      tablecheck(t_three);
      t_one->next = t_two;
      t_two->next = t_three;
      t_three->next = t_four;
      t_four->next = t_five;
      t_five->next = make_tnode("<block>",S_BLOCK);
      t_five->next->down = t_six;
      t_five->next->next = t_seven;
      return retptr;
    }

  if (t_one->ttype == S_OUTBUS)    /* OUTBUS LP IDENT COM exprlist RP SEM  */
    { 
      tablelistcheck(t_five);
      t_three->ttype = S_OUTBUSNAME;
      t_one->next = t_two;
      t_two->next = t_three;
      t_three->next = t_four;
      t_four->next = make_tnode("<exprlist>",S_EXPRLIST);
      t_four->next->down = t_five;
      t_four->next->next = t_six;
      t_six->next = t_seven;
      if (t_five == NULL)
	{
	  printf("Error: Outbus statements must have width >= 1.\n\n");
	  showerrorplace(t_one->linenum, t_one->filename);
	}

      /* log outbus statement on outbustable */

      ptr = make_tnode(t_three->val, S_BUS);
      ptr->down = retptr;
      if (outbustable == NULL)
	outbustable = ptr;
      else
	{
	  ptr->next = outbustable;
	  outbustable = ptr;
	}

      return retptr;

    }

  return NULL; /* should never happen */
}

/***********************************************************************/
/* lvalue          : IDENT                                             */
/*                 | IDENT LB expr RB                                  */
/***********************************************************************/

tnode * make_lval(tnode * t_ident, tnode * t_lb, tnode * t_expr,
		  tnode * t_rb)

{
  tnode * retptr;

  if (suspendvarchecks == 0)
    {
      t_ident->sptr = getsym(&locsymtable,t_ident);
      if ((t_ident->sptr == NULL) && strcmp(t_ident->val,"MIDIctrl") && 
	  strcmp(t_ident->val,"params"))
	{
	  printf("Error: Inappropriate lval name %s.\n", t_ident->val);
	  showerrorplace(t_ident->linenum, t_ident->filename);
	}

      /* a place to check for faster-rate assignments in poly-ops */
      /*
      if (t_ident->sptr)
	{
	  switch (t_ident->sptr->rate) 
	    {
	    case IRATETYPE:
	      break;
	    case KRATETYPE:
	      break;
	    case ARATETYPE:
	      break;
	    case XRATETYPE:
	      break;
	    }
	}
      */

      tablecheck(t_ident);
    }
  retptr = make_tnode("<lvalue>",S_LVALUE);
  retptr->down = t_ident;

  if (t_ident->sptr != NULL)
    {
      t_ident->res = t_ident->sptr->res;
      t_ident->vartype = t_ident->sptr->vartype;
    }
  else
    {
      t_ident->res = standardres(t_ident);
      t_ident->vartype = standardvartype(t_ident);
    }

  if (t_lb == NULL)
    {
      retptr->res = t_ident->res;
      retptr->vartype = t_ident->vartype;
      return retptr;
    }

  /* this section handles indexed arrays*/

  t_ident->next = t_lb;
  t_lb->next = t_expr;
  t_expr->next = t_rb;

  if (t_ident->vartype == SCALARTYPE)
    {
      printf("Error: Using array indexing on a scalar variable.\n\n");
      showerrorplace(t_ident->linenum, t_ident->filename);
    }

  retptr->vartype = SCALARTYPE;
  retptr->res = t_ident->res;
  tablecheck(t_expr);

  return retptr;

}


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*                                                              */
/*       Top-level functions for variable declarations.         */
/*                                                              */
/*______________________________________________________________*/

/***********************************************************************/
/* vardecl         : taglist stype namelist SEM                        */
/*                 | stype namelist SEM                                */
/***********************************************************************/

tnode * make_simplevar(tnode * t_taglist, tnode * t_type,
		       tnode * t_namelist, tnode * t_sem, 
		       char * name, int number)
{
  tnode * retptr;
  tnode * ptr;
  int newkind;
  int symstate;
  int target;

  retptr = make_tnode(name,number);
  retptr->rate = t_type->rate;

  if (number == S_OPVARDECL)
    {
      if ((opcodenametable->rate == XRATETYPE) && (!nonpolyparams) &&
	  ((t_type->ttype == S_KSIG) || (t_type->ttype == S_ASIG)))
	{
	  if (t_type->ttype == S_KSIG)
	    printf("Error: Poly-opcode ksig declarations restricted.\n\n");
	  else
	    printf("Error: Poly-opcode asig declarations restricted.\n\n");
	  showerrorplace(t_type->linenum, t_type->filename);
	}
      if ((opcodenametable->rate == IRATETYPE) && 
	  ((t_type->ttype == S_KSIG) || (t_type->ttype == S_ASIG)))
	{
	  if (t_type->ttype == S_KSIG)
	    printf("Error: Ksig declarations not allowed in iopcodes.\n\n");
	  else
	    printf("Error: Asig declarations not allowed in iopcodes.\n\n");
	  showerrorplace(t_type->linenum, t_type->filename);
	}
      if ((opcodenametable->rate == KRATETYPE) && (t_type->ttype == S_ASIG))
	{
	  printf("Error: Asig declarations not allowed in kopcodes.\n\n");
	  showerrorplace(t_type->linenum, t_type->filename);
	}
    }
  if ((t_type->ttype == S_XSIG) && ((number == S_VARDECL) || 
      (opcodenametable->rate != XRATETYPE)))
    {	
      printf("Error: Only polymorphic opcodes can use xsig.\n\n");
      showerrorplace(t_type->linenum, t_type->filename);
    }
  target = 0;
  if (t_taglist == NULL)
    {
      retptr->down = t_type;
      newkind = K_NORMAL;
    }
  else
    {
      if ((t_type->ttype == S_ASIG)||(t_type->ttype == S_OPARRAY))
	{
	  printf("Error: Import/export tags on %s.\n\n", t_type->val);
	  showerrorplace(t_type->linenum, t_type->filename);
	}
      retptr->down = t_taglist;
      t_taglist->next = t_type;
      if (t_taglist->down->next == NULL)
	{
	  newkind = K_EXPORT;
	  if (t_taglist->down->ttype == S_IMPORTS)
	    {
	      newkind = K_IMPORT;
	      target = isaninstr;
	    }
	}
      else
	newkind = K_IMPORTEXPORT;
      if ((t_type->ttype == S_TABLE)&&(newkind == K_EXPORT))
	{
	  printf("Error: Wavetable placeholder needs imports tag.\n\n");
	  showerrorplace(t_type->linenum, t_type->filename);
	}
    }

  if (t_type->ttype == S_OPARRAY)
    {
      if (t_namelist->next != NULL)
	{
	  printf("Error: Multiple opcode declarations per line.\n\n");
	  showerrorplace(t_type->linenum, t_type->filename);
	}
      if (t_namelist->down->next == NULL)
	{
	  printf("Error: Opcode declarations without width.\n\n");
	  showerrorplace(t_type->linenum, t_type->filename);
	}

      if (locopcodecalls == NULL)
	{
	  ptr = locopcodecalls = make_tnode(t_namelist->down->val,
					    S_OPARRAYDECL);
	  ptr->optr = retptr;
	  retptr->optr = ptr;
	}
      else
	{
	  ptr = locopcodecalls;
	  if ((ptr->ttype == S_OPARRAYDECL) && 
	      (!strcmp(ptr->val,t_namelist->down->val)))
	    {
	      printf("Error: Multiple oparrays for same opcode.\n\n");
	      showerrorplace(t_type->linenum, t_type->filename);
	    }
	  while (ptr->next != NULL)
	    {
	      if ((ptr->ttype == S_OPARRAYDECL) && 
		  (!strcmp(ptr->val,t_namelist->down->val)))
		{
		  printf("Error: Multiple oparrays for same opcode.\n\n");
		  showerrorplace(t_type->linenum, t_type->filename);
		}
	      ptr = ptr->next;
	    }
	  ptr->next = make_tnode(t_namelist->down->val, S_OPARRAYDECL);
	  ptr = ptr->next;
	  ptr->optr = retptr;
	  retptr->optr = ptr;
	}
 
      switch (t_namelist->down->next->next->ttype) {
      case S_INTGR:
	sscanf(t_namelist->down->next->next->val,"%d",&(t_namelist->opwidth));
	if (t_namelist->opwidth < 1)
	  {
	    printf("Error: Oparray width must be >= 1.\n\n");
	    showerrorplace(t_type->linenum, t_type->filename);
	  }
	retptr->opwidth = ptr->opwidth = t_namelist->down->opwidth = 
	  t_namelist->opwidth;
	break;
      case S_INCHANNELS:
	retptr->opwidth = ptr->opwidth = t_namelist->down->opwidth = 
	  t_namelist->opwidth = INCHANNELSWIDTH;
	break;
      case S_OUTCHANNELS:
	retptr->opwidth = ptr->opwidth = t_namelist->down->opwidth = 
	  t_namelist->opwidth = OUTCHANNELSWIDTH;
	break;
      }

      t_type->next = t_namelist;
      t_namelist->next = t_sem;
      return retptr;
    }

  t_type->next = make_tnode("<namelist>",S_NAMELIST);
  t_type->next->next = t_sem;
  t_type->next->down = t_namelist;

  ptr = t_namelist;
  while (ptr != NULL)
    {
      if (ptr->ttype == S_NAME)
	{
	  reservednames(ptr->down);
	  symstate = addsym(&locsymtable, ptr->down);
	  symcheck(symstate, ptr->down);

	  ptr->sptr = locsymtable;
	  ptr->down->sptr = locsymtable;

	  if (ptr->down->next != NULL)              /* vector case */
	    {
	      ptr->down->vartype = locsymtable->vartype =
		ptr->vartype = VECTORTYPE;
	      switch (ptr->down->next->next->ttype) {
	      case S_INTGR:
		sscanf(ptr->down->next->next->val,"%d",&(ptr->width));
		if (ptr->width < 1)
		  {
		    printf("Error: Array length must be >= 1.\n\n");
		    showerrorplace(t_type->linenum, t_type->filename);
		  }
		ptr->down->width = locsymtable->width = ptr->width;
		break;
	      case S_INCHANNELS:
		ptr->down->width = locsymtable->width =
		  ptr->width = INCHANNELSWIDTH;
		break;
	      case S_OUTCHANNELS:
		ptr->down->width = locsymtable->width =
		  ptr->width = OUTCHANNELSWIDTH;
		break;
	      }
	    }
	  ptr->rate = ptr->down->rate = locsymtable->rate = t_type->rate;
	  ptr->sptr->kind = newkind;
	  if (t_type->ttype == S_TABLE)
	    ptr->down->vartype = locsymtable->vartype =
	      ptr->vartype = TABLETYPE;
	  else
	    if (target && (ptr->vartype == SCALARTYPE))
	      addsym(&targetsymtable, ptr->down);
	}
      ptr = ptr->next;
    }

  return retptr;
}


/***********************************************************************/
/* tabledecl         : TABLE IDENT LP IDENT COM exprstrlist RP           */
/***********************************************************************/

tnode * make_tabledecl(tnode * t_table, tnode * t_tablename, tnode * t_lp,
		       tnode * t_generator, tnode * t_com, 
		       tnode * t_exprstrlist, tnode *t_rp)

{

  tnode * retptr, * tptr;
  int symstate;

  retptr = make_tnode("<table>", S_TABLE);
  retptr->down = t_table;
  t_table->next = t_tablename;
  t_tablename->next = t_lp;
  t_lp->next = t_generator;
  t_generator->next = t_com;
  t_com->next = make_tnode("<exprstrlist>",S_EXPRSTRLIST);
  t_com->next->down = t_exprstrlist;
  t_com->next->next = t_rp;
  t_rp->next = make_tnode(";",S_SEM);

  if (!wavegeneratorname(t_generator))
    {
      printf("Error: Invalid generator name %s.\n\n",t_generator->val);
      showerrorplace(t_generator->linenum, t_generator->filename);
    }

  if (strcmp(t_generator->val,"sample"))
    {
      tptr = t_exprstrlist;
      while (tptr)
	{
	  if (tptr->ttype == S_STRCONST)
	    {      
	      printf("Error: String constant %s in non-sample generator.\n\n",
		     tptr->val);
	      showerrorplace(tptr->linenum, tptr->filename);
	    }
	  tptr = tptr->next;
	}
    }

  reservednames(t_tablename);

  symstate = addsym(&locsymtable, t_tablename);
  symcheck(symstate, t_tablename);
  locsymtable->defnode = retptr;

  t_tablename->sptr = locsymtable;
  locsymtable->kind = K_NORMAL;
  locsymtable->vartype = TABLETYPE;
  locsymtable->rate = IRATETYPE;
  locsymtable->width = 1;

  return retptr;
}


/***********************************************************************/
/* vardecl         :  TABLEMAP IDENT LP identlist RP SEM               */
/***********************************************************************/

tnode * make_tablemap(tnode * t_tablemap, tnode * t_name, tnode * t_lp,
		      tnode * t_identlist, tnode * t_rp, tnode * t_sem)

{

  tnode * retptr;
  int symstate;


  if (t_identlist == NULL)
    {
      printf("Error: Tablemap must use at least one table.\n\n");
      showerrorplace(t_identlist->linenum, t_identlist->filename);
    }
  retptr = make_tnode("<tablemap>", S_TABLEMAP);
  retptr->down = t_tablemap;
  t_tablemap->next = t_name;
  t_name->next = t_lp;
  t_lp->next = make_tnode("<identlist>",S_IDENTLIST);
  t_lp->next->down = t_identlist;
  t_lp->next->next = t_rp;
  t_rp->next = t_sem;
  reservednames(t_name);

  symstate = addsym(&locsymtable, t_name);
  symcheck(symstate, t_name);
  locsymtable->defnode = retptr;

  t_name->sptr = locsymtable;
  locsymtable->kind = K_NORMAL;
  locsymtable->vartype = TMAPTYPE;
  locsymtable->rate = IRATETYPE;
  locsymtable->width = 1;

  return retptr;
}


/***********************************************************************/
/* paramdecl       : otype name                                        */
/*                 ;                                                   */
/***********************************************************************/

tnode * make_paramdecl(tnode * t_otype, tnode * t_name)

{
  tnode * retptr;
  int symstate;

  retptr = make_tnode("<paramdecl>",S_PARAMDECL);
  retptr->down = t_otype;
  t_otype->next = t_name;

  reservednames(t_name->down);

  symstate = addsym(&locsymtable, t_name->down);
  symcheck(symstate, t_name->down);
  locsymtable->kind = K_PFIELD;
  retptr->sptr = t_name->sptr = t_name->down->sptr = locsymtable;

  if (t_name->down->next != NULL)              /* vector case */
    {
      retptr->vartype = t_name->down->vartype = locsymtable->vartype =
	t_name->vartype = VECTORTYPE;
      switch (t_name->down->next->next->ttype) {
      case S_INTGR:
	sscanf(t_name->down->next->next->val,"%d",&(t_name->width));
	if (t_name->width < 1)
	  {
	    printf("Error: Array length must be >= 1.\n\n");
	    showerrorplace(t_name->linenum, t_name->filename);
	  }
	retptr->width = t_name->down->width = locsymtable->width = t_name->width;
	break;
      case S_INCHANNELS:
	retptr->width = t_name->down->width = locsymtable->width =
	  t_name->width = INCHANNELSWIDTH;
	break;
      case S_OUTCHANNELS:
	retptr->width = t_name->down->width = locsymtable->width =
	  t_name->width = OUTCHANNELSWIDTH;
	break;
      }
    }

  
  switch (t_otype->ttype) {
  case S_IVAR:
    locsymtable->rate = retptr->rate = t_name->rate = IRATETYPE;
    nonpolyparams = 1;
    break;
  case S_KSIG:
    locsymtable->rate = retptr->rate = t_name->rate = KRATETYPE;
    if (locsymtable->rate > opcodenametable->rate)
      {
	printf("Error: Formal argument faster than opcode rate.\n\n");
	showerrorplace(t_otype->linenum, t_otype->filename);
      }
    nonpolyparams = 1;
    break;
  case S_ASIG:
    locsymtable->rate = retptr->rate = t_name->rate = ARATETYPE;
    if (locsymtable->rate > opcodenametable->rate)
      {
	printf("Error: Formal argument faster than opcode rate.\n\n");
	showerrorplace(t_otype->linenum, t_otype->filename);
      }
    nonpolyparams = 1;
    break;
  case S_XSIG:
    if (opcodenametable->rate != XRATETYPE)
      {
	printf("Error: Only polymorphic opcodes can use xsig.\n\n");
	showerrorplace(t_otype->linenum, t_otype->filename);
      }
    retptr->sptr->rate = retptr->rate = t_name->rate = XRATETYPE;
    break;
  case S_TABLE:
    retptr->vartype = t_name->down->vartype = locsymtable->vartype =
	t_name->vartype = TABLETYPE;
    locsymtable->rate = retptr->rate = t_name->rate = IRATETYPE;
    break;
  default:
    printf("Error: Oparray not permitted as opcode formal parameter.\n\n");
    showerrorplace(t_otype->linenum, t_otype->filename);
  }

  return retptr;
}

/***********************************************************************/
/* name  : IDENT                   {$$ = make_name($1,NULL,NULL,NULL);} */
/*       | IDENT LB INTGR RB       {$$ = make_name($1,$2,$3,$4);}       */
/*       | IDENT LB INCHANNELS RB  {$$ = make_name($1,$2,$3,$4);}       */
/*       | IDENT LB OUTCHANNELS RB {$$ = make_name($1,$2,$3,$4);}       */
/***********************************************************************/


tnode * make_name(tnode * t_ident, tnode * t_lb,
		  tnode * t_const, tnode *t_rb)


{
  tnode * retptr;

  retptr = make_tnode("<name>",S_NAME);
  retptr->down = t_ident;

  if (t_lb != NULL)              /* vector case, or oparray */
    {
      t_ident->next = t_lb;
      t_lb->next = t_const;
      t_const->next = t_rb;
    }

  return retptr;
}


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*                                                              */
/*          Top-level functions for expressions.                */
/*                                                              */
/*______________________________________________________________*/

/***********************************************************************/
/*expr  (done)    : IDENT                                              */
/*   (done)       | const                                              */
/*   (done)       | IDENT LB expr RB                                   */
/* treeupdate!    | SASBF LP exprlist RP                               */
/*   (done)       | IDENT LP exprlist RP                               */
/*   (done)       | IDENT LB expr RB LP exprlist RP                    */
/*   (done)       | expr Q expr COL expr %prec Q                       */
/*   (done)       | expr LEQ expr                                      */
/*   (done)       | expr GEQ expr                                      */
/*   (done)       | expr NEQ expr                                      */
/*   (done)       | expr EQEQ expr                                     */
/*   (done)       | expr GT expr                                       */
/*   (done)       | expr LT expr                                       */
/*   (done)       | expr AND expr                                      */
/*   (done)       | expr OR expr                                       */
/*   (done)       | expr PLUS expr                                     */
/*   (done)       | expr MINUS expr                                    */
/*   (done)       | expr STAR expr                                     */
/*   (done)       | expr SLASH expr                                    */
/*   (done)       | NOT expr %prec UNOT                                */
/*   (done)       | MINUS expr %prec UMINUS                            */
/*   (done)       | LP expr RP                                         */
/* (generated by parsing)                                              */
/*                  FLOATCAST LP expr RP     (int->float)              */ 
/***********************************************************************/

extern tnode * make_asfloat(tnode *);
extern int exprres(tnode *, tnode *);

tnode * make_expr(tnode * t_one, tnode * t_two, tnode * t_three, 
		  tnode * t_four, tnode * t_five, tnode * t_six,
		  tnode * t_seven)
{
  tnode * retptr;
  tnode * tptr;

  if (t_one->ttype == S_SASBF)
    {
      printf("Error: Sasbf synthesis not supported.\n\n");
      showerrorplace(t_one->linenum, t_one->filename);
    }

  retptr = make_tnode("<expr>",S_EXPR);
  if ((t_one->ttype == S_IDENT) && ( (t_two == NULL) ||    /* scalar */ 
       ((t_four->ttype == S_RB) && (t_five == NULL)) ))   /* array */
    {
      if (standardname(t_one))
	{
	  hasstandardname(t_one);
	  t_one->res = standardres(t_one);
	  t_one->vartype = standardvartype(t_one);
	}
      else
	{
	  if (suspendvarchecks == 0)
	    {
	      t_one->sptr = getsym(&locsymtable,t_one);
	      if (t_one->sptr == NULL)
		{
		  printf("Error: Variable %s not defined.\n\n",
			 t_one->val);
		  showerrorplace(t_one->linenum, t_one->filename);
		}
	      t_one->res = t_one->sptr->res;
	      t_one->vartype = t_one->sptr->vartype;
	    }
	}
    }

  retptr->down = t_one;
  if (t_two == NULL)                /* : IDENT and | const */
    {
      retptr->res = t_one->res;
      retptr->vartype = t_one->vartype;
      retptr->vol = t_one->vol;
      return retptr;
    }
  t_one->next = t_two;

  if (t_three == NULL)                /* unary ops and empty opcode call*/
    {
      if (t_four != NULL)             /* empty opcode or SASBF call */
	{
	  t_two->next = make_tnode("<exprlist>",S_EXPRLIST);
	  t_two->next->next = t_four;
	  if (locopcodecalls == NULL)
	    tptr = locopcodecalls = make_tnode(t_one->val, S_OPCALL);
	  else
	    {
	      tptr = locopcodecalls;
	      while (tptr->next != NULL)
		tptr = tptr->next;
	      tptr->next = make_tnode(t_one->val, S_OPCALL);
	      tptr = tptr->next;
	    }
	  t_one->optr = retptr->optr = tptr;
	  tptr->optr = retptr;
	  if (coreopcodename(t_one))
	    retptr->res = coreopcodeasint(t_one);
	  return retptr; 
	}
      tablecheck(t_two);
      switch (t_one->ttype) {
      case S_MINUS:                  /* MINUS expr %prec UMINUS */     
	retptr->res = t_two->res;
	break;
      case S_NOT:                    /* NOT expr %prec UNOT */
	retptr->res = ASINT;
	break;
      }
      return retptr;
    }

  t_two->next = t_three;
  if (t_four == NULL)
    {
      switch (t_two->ttype) {
      case S_EXPR:              /* LP expr RP */
	tablecheck(t_two);
	retptr->res = t_two->res;
	break;
      case S_LEQ:  case S_GEQ: case S_NEQ:   /* float op float -> int */
      case S_EQEQ: case S_GT: case S_LT:
      case S_AND: case S_OR:   
	tablecheck(t_one);
	tablecheck(t_three);
	retptr->res = ASINT;
	break;
      case S_STAR:  /*  *  */
	tablecheck(t_one);
	tablecheck(t_three);
	retptr->res = exprres(t_one, t_three);
	if ((t_one->down != NULL) && (t_one->down->ttype == S_EXPR) &&
	    (t_one->down->next != NULL) &&
	    (t_one->down->next->ttype == S_STAR))
	  {
	    retptr->down = t_one->down;
	    tptr = t_one->down;
	    while (tptr->next != NULL)
	      tptr = tptr->next;
	    tptr->next = t_two;
	  }
	if ((t_three->down != NULL) && (t_three->down->ttype == S_EXPR) &&
	    (t_three->down->next != NULL) &&
	    (t_three->down->next->ttype == S_STAR))
	  {
	    t_two->next = t_three->down;
	  }
	break;
      case S_PLUS: case S_MINUS:  /* +,- */
	tablecheck(t_one);
	tablecheck(t_three);
	retptr->res = exprres(t_one, t_three);
	if ((t_one->down != NULL) && (t_one->down->ttype == S_EXPR) &&
	    (t_one->down->next != NULL) &&
	    ((t_one->down->next->ttype == S_PLUS) || 
	     (t_one->down->next->ttype == S_MINUS)))
	  {
	    retptr->down = t_one->down;
	    tptr = t_one->down;
	    while (tptr->next != NULL)
	      tptr = tptr->next;
	    tptr->next = t_two;
	  }
	if ((t_three->down != NULL) && (t_three->down->ttype == S_EXPR) &&
	    (t_three->down->next != NULL) &&
	    ((t_three->down->next->ttype == S_PLUS) ||
	     (t_three->down->next->ttype == S_MINUS)))
	  {
	    t_two->next = t_three->down;
	  }
	break;
      case S_SLASH:   /* / */
	tablecheck(t_one);
	tablecheck(t_three);
	retptr->res = ASFLOAT;
	t_two->next = t_three = make_asfloat(t_three);
	break;
      }
      return retptr;
    }


  if (t_five == NULL)
    {
      if (t_two->ttype == S_LB)  /* array indexing */
	{
	  tablecheck(t_three);
	  t_three->next = t_four;
	  if (suspendvarchecks == 0)
	    {
	      if (t_one->vartype == SCALARTYPE)
		{
		  printf("Error: Array index on a scalar variable.\n\n");
		  showerrorplace(t_one->linenum, t_one->filename);
		}
	      if (t_one->vartype == TABLETYPE)
		{
		  printf("Error: Array index on a table variable.\n\n");
		  showerrorplace(t_one->linenum, t_one->filename);
		}
	      
	      if (t_one->vartype == TMAPTYPE)
		retptr->vartype = TABLETYPE;
	      else
		retptr->vartype = SCALARTYPE;
	    }
	  retptr->res = t_one->res;
	  return retptr;
	}

      if (t_one->ttype == S_IDENT)   /*  IDENT LP exprlist RP */
	{
	  t_two->next = make_tnode("<exprlist>",S_EXPRLIST);
	  t_two->next->down = t_three;
	  t_two->next->next = t_four;
	  if (locopcodecalls == NULL)
	    tptr = locopcodecalls = make_tnode(t_one->val, S_OPCALL);
	  else
	    {
	      tptr = locopcodecalls;
	      while (tptr->next != NULL)
		tptr = tptr->next;
	      tptr->next = make_tnode(t_one->val, S_OPCALL);
	      tptr = tptr->next;
	    }
	  t_one->optr = retptr->optr = tptr;
	  tptr->optr = retptr;
	  if (coreopcodename(t_one))
	    retptr->res = coreopcodeasint(t_one);
	  return retptr;
	}

      /* SASBF */
      t_three->next = t_four; /* will need <exprlist> */
      return retptr;
    }


  t_three->next = t_four;
  t_four->next = t_five;      /* expr Q expr COL expr */
  if (t_six == NULL)
    {
      if (t_seven != NULL)    /* zero argument oparray call */
	{
	  tablecheck(t_three);
	  t_five->next = make_tnode("<exprlist>",S_EXPRLIST);
	  t_five->next->next = t_seven;
	  
	  if (locopcodecalls == NULL)
	    tptr = locopcodecalls = make_tnode(t_one->val, S_OPARRAYCALL);
	  else
	    {
	      tptr = locopcodecalls;
	      while (tptr->next != NULL)
		tptr = tptr->next;
	      tptr->next = make_tnode(t_one->val, S_OPARRAYCALL);
	      tptr = tptr->next;
	    }
	  t_one->optr = retptr->optr = tptr;
	  tptr->optr = retptr;
	  
	  if (suspendvarchecks == 0)
	    {
	      tptr = locopcodecalls;
	      while ((tptr != NULL) && (tptr->ttype == S_OPARRAYDECL) &&
		     (strcmp(tptr->val,t_one->val) != 0))
		tptr = tptr->next;
	      if ((tptr == NULL) || (tptr->opwidth == 0))
		{
		  printf("Error: Undeclared oparray call %s.\n\n",
			 t_one->val);
		  showerrorplace(t_one->linenum, t_one->filename);
		}
	      t_one->optr->ibus = tptr;
	    }
	  if (coreopcodename(t_one))
	    retptr->res = coreopcodeasint(t_one);
	  return retptr;
	}
      tablecheck(t_one);
      tablecheck(t_three);
      tablecheck(t_five);
      if ( ((t_three->res == ASINT)   && (t_five->res == ASINT)) ||
	   ((t_three->res == ASFLOAT) && (t_five->res == ASFLOAT)) )
	retptr->res = t_three->res;
      else
	{
	  retptr->res = ASFLOAT;
	  switch (t_three->res) {
	  case ASINT:
	    t_two->next = make_asfloat(t_three);
	    t_two->next->next = t_four;
	    break;
	  case ASFLOAT:
	    t_four->next = make_asfloat(t_five);
	    break;
	  }
	}
      return retptr;
    }

  t_five->next = make_tnode("<exprlist>",S_EXPRLIST);
  t_five->next->down = t_six;
  t_five->next->next = t_seven;

  /*  IDENT LB expr RB LP exprlist RP */

  tablecheck(t_three);
  if (locopcodecalls == NULL)
    tptr = locopcodecalls = make_tnode(t_one->val, S_OPARRAYCALL);
  else
    {
      tptr = locopcodecalls;
      while (tptr->next != NULL)
	tptr = tptr->next;
      tptr->next = make_tnode(t_one->val, S_OPARRAYCALL);
      tptr = tptr->next;
    }
  t_one->optr = retptr->optr = tptr;
  tptr->optr = retptr;
      
  if (suspendvarchecks == 0)
    {
      tptr = locopcodecalls;
      while ((tptr != NULL) && (tptr->ttype == S_OPARRAYDECL) &&
	     (strcmp(tptr->val,t_one->val) != 0))
	tptr = tptr->next;
      if ((tptr == NULL) || (tptr->opwidth == 0))
	{
	  printf("Error: Undeclared oparray call %s.\n\n",
		 t_one->val);
	  showerrorplace(t_one->linenum, t_one->filename);
	}
      t_one->optr->ibus = tptr;
    }

  if (coreopcodename(t_one))
    retptr->res = coreopcodeasint(t_one);

  return retptr;
  
}


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*                                                              */
/*          Top-level functions for generic reductions.         */
/*                                                              */
/*______________________________________________________________*/

/*****************************************************/
/* handles two-element left-recursion tnode building */
/*****************************************************/

tnode * leftrecurse(tnode * left, tnode * right)

{

  tnode * ptr;
  tnode * ret;
  
  if (left != NULL) 
    {
      ptr = left;
      while (ptr->next != NULL)
	ptr = ptr->next;
      ptr->next = right;
      ret = left;
    }
  else
    ret = right;

  return ret;

}

/********************************************************/
/* handles left-recursion tnode building with separation*/
/********************************************************/

tnode * leftsrecurse(tnode * left, tnode * middle, tnode * right)

{

  tnode * ptr;
  tnode * ret = NULL;

  if ((left != NULL)&&(middle != NULL)) /* this should always happen */
    {
      ptr = left;
      while (ptr->next != NULL)
	ptr = ptr->next;
      ptr->next = middle;
      middle->next = right;
      ret = left;
    }
  else
    {
      if (left == NULL)
	{
	  if (middle == NULL)
	    ret = right;
	  else
	    {
	      middle->next = right;
	      ret = middle;
	    }
	}
    }

  return ret;

}

/*********************************************************************/
/* makes a new subtree, takes upto four tnodes -- NULL unused tnodes */ 
/*********************************************************************/

tnode * make_stree(tnode * t_one, tnode * t_two, tnode * t_three,
		   tnode * t_four, char * name, int number)


{

  tnode * retptr;
  
  retptr = make_tnode(name,number);
  retptr->down = t_one;

  if (t_one != NULL)
    {
      t_one->next = t_two;
      if (t_two != NULL)
	{
	  t_two->next = t_three;
	  if (t_three != NULL)
	    t_three->next = t_four;
	}
    }
  return retptr; 

}


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*                                                              */
/*    Utility functions specific to a complex reduction above.  */
/*                                                              */
/*______________________________________________________________*/

/************************************************************/
/* makes expression an ASFLOAT expr: used in make_expr()    */
/************************************************************/

tnode * make_asfloat(tnode * exprptr)

{
  tnode * retptr;

  if (exprptr->res == ASFLOAT)
    return exprptr;

  if ( (exprptr->down->ttype == S_INTGR) && (exprptr->down->next == NULL) &&
       (exprptr->down->down == NULL))
    {
      strcat(exprptr->down->val,".0");
      exprptr->down->ttype = S_NUMBER;
      exprptr->res = exprptr->down->res = ASFLOAT;
      return exprptr;
    }
  
  retptr = make_stree(make_tnode("(float)",S_FLOATCAST),
		      make_tnode("(",S_LP), exprptr,
		      make_tnode(")",S_RP), "<expr>", S_EXPR);

  retptr->res = ASFLOAT;
  return retptr;
}

/************************************************************/
/* res check for two tnodes in an expr: used in make_expr() */
/************************************************************/

int exprres(tnode * left, tnode * right)

{
  if ((left->res == ASINT) && (right->res == ASINT))
    return ASINT;
  return ASFLOAT;
}

/************************************************************/
/*  checks identifiers for reserved names: in declarations  */
/************************************************************/

void reservednames(tnode * tptr)

{
  int badname = 0;

  if (standardname(tptr))
    {
      printf("Error: Standard ");
      badname = 1;
    }
  if (coreopcodename(tptr))
    {
      printf("Error: Core opcode ");
      badname = 1;
    }
  if (wavegeneratorname(tptr))
    {
      printf("Error: Core wavetable generator ");
      badname = 1;
    }
  if (badname)
    {
      printf("name %s used for identifier.\n\n", tptr->val);
      showerrorplace(tptr->linenum, tptr->filename);
    }
}

extern void exprtableok(tnode * tptr);

/************************************************************/
/* checks table expressions for local variables: top decls  */
/************************************************************/

void tableexprcheck(void)

{

  sigsym * sptr = locsymtable;
  tnode * tptr;

  while (sptr != NULL)
    {
      if ((sptr->vartype == TABLETYPE) && (sptr->kind == K_NORMAL))
	{
	  tptr = sptr->defnode->down->next->next->next->next->next->down;
	  while (tptr != NULL)
	    {
	      if (tptr->ttype == S_EXPR)
		exprtableok(tptr);
	      tptr = tptr->next;
	    }
	}
      sptr = sptr->next;
    }
}

/************************************************************/
/* helps check table expressions for local variables: above */
/************************************************************/

void exprtableok(tnode * tptr)

{
  while (tptr != NULL)
    {
      if (tptr->down != NULL)
	{
	  exprtableok(tptr->down);
	}
      else
	{
	  if ((tptr->ttype == S_IDENT) && (tptr->sptr != NULL) &&
	      (tptr->sptr->kind != K_PFIELD) &&
	      (! ( ((tptr->sptr->kind == K_IMPORT)||
	            (tptr->sptr->kind == K_IMPORTEXPORT)) &&
		    (tptr->sptr->rate == IRATETYPE) ))&&
	      (tptr->sptr->vartype != TABLETYPE) && 
	      (tptr->sptr->vartype != TMAPTYPE))
	    {
	      printf("Error: Table expr may not use local variable %s.\n\n",
		     tptr->val);
	      showerrorplace(tptr->linenum, tptr->filename);
	    }
	}
      tptr = tptr->next;
    }

}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*                                                              */
/*     Utility functions also used in other sfront files.       */
/*                                                              */
/*______________________________________________________________*/

/***********************************************************************/
/*          detects table/tablemap variables in bad places             */      
/***********************************************************************/

void tablecheck(tnode * tptr)

{
  if ((tptr != NULL) && 
      ((tptr->vartype == TABLETYPE) || (tptr->vartype == TMAPTYPE)))
    {
      if (tptr->down != NULL)
	tptr = tptr->down;
      printf("Error: Table(map) %s used inappropriately.\n\n", tptr->val);
      showerrorplace(tptr->linenum, tptr->filename);
    }
}

/***********************************************************************/
/*          detects table/tablemap variables in bad places             */      
/***********************************************************************/

void tablelistcheck(tnode * tptr)

{
  while (tptr != NULL)
    {
      if ((tptr->vartype == TABLETYPE) || (tptr->vartype == TMAPTYPE))
	{
	  if (tptr->down != NULL)
	    tptr = tptr->down;
	  printf("Error: Table(map) %s used inappropriately.\n\n", tptr->val);
	  showerrorplace(tptr->linenum, tptr->filename);
	}
      tptr = tptr->next;
    }
}
