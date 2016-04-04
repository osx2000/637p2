
/*
#    Sfront, a SAOL to C translator    
#    This file: Collapses constants
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


extern void exprcollapse(int, tnode *);
extern void assignoptconst(int, tnode **);
extern void ifoptconst(int, tnode **);
extern void ifelseoptconst(int, tnode **);
extern void whileoptconst(int, tnode **);

extern int tokencollapse(int, tnode *);
extern int unarycollapse(int, tnode *);
extern int floatcastcollapse(int, tnode *);
extern int parencollapse(int, tnode *);
extern int multicollapse(int, tnode *);
extern int binarycollapse(int, tnode *);
extern int switchcollapse(int, tnode *);
extern int opcodecollapse(int, tnode *);


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*       void stateoptconst() and void exprcollapse()                  */
/*                                                                     */
/* These functions do const optimization on SAOL statements and        */
/* expressions, and are called from optmain.c. Following these         */
/* functions are the second-level functions for const optimization.    */
/*                                                                     */
/*_____________________________________________________________________*/

/***********************************************************************/
/*                const-optimizes a statement                          */
/*      this function exists so it can be called recursively           */
/*                                                                     */
/* statement (done): lvalue EQ expr SEM                                */
/*           (done)| expr SEM                                          */
/*           (done)| EXTEND LP expr RP SEM                             */
/*           (done)| IF LP expr RP LC block RC                         */
/*           (done)| IF LP expr RP LC block RC ELSE LC block RC        */
/*           (done)| WHILE LP expr RP LC block RC                      */
/*           (done)| INSTR IDENT LP exprlist RP SEM                    */
/*           (done)| OUTPUT LP exprlist RP SEM                         */
/*           (done)| SPATIALIZE LP exprlist RP SEM                     */
/*           (done)| OUTBUS LP IDENT COM exprlist RP SEM               */
/*           (done)| TURNOFF SEM                                       */
/*           (done)| RETURN LP exprlist RP SEM                         */
/*           (done)| PRINTF LP exprstrlist RP SEM                      */
/***********************************************************************/

void stateoptconst(int passrate, tnode ** lptr)

{
  tnode *tptr = *lptr;

  switch(tptr->down->ttype) {
  case S_LVALUE:
    assignoptconst(passrate, lptr);
    break;
  case S_EXPR:
    exprcollapse(passrate, tptr->down);
    break;
  case S_EXTEND:
    exprcollapse(passrate, tptr->down->next->next);
    break;
  case S_IF:
    if (tptr->down->next->next->next->next->next->next->next == NULL)
      ifoptconst(passrate, lptr);
    else
      ifelseoptconst(passrate, lptr);
    break;
  case S_WHILE:
    whileoptconst(passrate, lptr);
    break;
  case S_PRINTF:
    tptr = tptr->down->next->next->down;
    while (tptr != NULL)
      {
	if (tptr->ttype == S_EXPR)
	  exprcollapse(passrate, tptr);
	tptr = tptr->next;
      }
    break;
  case S_INSTR:
  case S_OUTPUT:
  case S_SPATIALIZE:
  case S_OUTBUS:
  case S_RETURN:
    tptr = tptr->down;
    while (tptr->ttype != S_EXPRLIST)
      tptr = tptr->next;
    tptr = tptr->down;
    while (tptr != NULL)
      {
	if (tptr->ttype == S_EXPR)
	  exprcollapse(passrate, tptr);
	tptr = tptr->next;
      }
    break;
  }

}  

/***********************************************************************/
/*               expression collapse main routine                      */      
/*                                                                     */
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

void exprcollapse(int passrate, tnode * tptr) 

{

  if (tokencollapse(passrate, tptr))
    return;
  if (unarycollapse(passrate, tptr))
    return;
  if (floatcastcollapse(passrate, tptr))
    return;
  if (parencollapse(passrate, tptr))
    return;
  if (multicollapse(passrate, tptr))
    return;
  if (binarycollapse(passrate, tptr))
    return;
  if (switchcollapse(passrate, tptr))
    return;
  if (opcodecollapse(passrate, tptr))
    return;

  internalerror("optconst.c", "exprcollapse parse error");

}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*                                                                     */
/* The second-level functions called by stateoptconst(), many of       */
/* of which use exprcollapse() to handle expressions.                  */
/*                                                                     */
/*_____________________________________________________________________*/


/***********************************************************************/
/*                const optimizes an assignment statement               */
/*                                                                     */
/* assignment: lvalue EQ expr SEM                                      */
/***********************************************************************/

void assignoptconst(int passrate, tnode ** lptr)

{
  tnode *tptr = *lptr;

  /* collapse expression */

  exprcollapse(passrate, tptr->down->next->next);

  /* do lval array index if needed */

  if (tptr->down->down->next != NULL)
    exprcollapse(passrate, tptr->down->down->next->next); 

  if (tptr->down->down->sptr) /* not MIDIctrl[], params[] */
    {

      /* this is the place where variables assigned to an ASINT */
      /* value get their ASINT res value. However, right now    */
      /* blocktree.c can't deal with this, so we keep defaults  */

      /*   the new code for later       */
      /*  tptr->down->down->sptr->res =
	  tptr->down->next->next->res;  */

      /* for now */

      tptr->down->down->sptr->res = ASFLOAT;

      /* end of ASINT code */

      tptr->down->down->sptr->vol = VARIABLE;

      /* substitute scalar variables set to constant values */
      /*     that are not inside a guard statement          */

      if ((!currconstoptif) && (!currconstoptwhile) &&
	  (tptr->down->next->next->vol == CONSTANT) &&
	  (tptr->down->down->sptr->vartype == SCALARTYPE))
	{

	  if ((tptr->down->next->next->down->ttype != S_NUMBER)&&
	      (tptr->down->next->next->down->ttype != S_INTGR))
	    internalerror("optconst.c", "assignoptconst S_EXPR error");

	  tptr->down->down->sptr->vol = CONSTANT;

	  tptr->down->down->sptr->consval = 
	    dupval(tptr->down->next->next->down->val);

	  /* once ASINT IDENTs supports, update tptr res too */

	  tptr->down->down->sptr->res =
	    tptr->down->next->next->res; 
 
	  /* delete assignments, if:
	   * [1] no guarded assignment
	   * [2] variable not slated to be exported
	   * [3] variable not slated to be returned in a call-by-ref
	   * [4] variable not slated to be passed to a call-by-ref
	   * [5] variable not written to at a faster rate
	   */

	  if ( /* [1] */
	      (tptr->down->down->sptr->tref->assignif == 0) &&
	      (tptr->down->down->sptr->tref->assignwhile == 0) &&
	       /* [2] */
	      (tptr->down->down->sptr->kind != K_EXPORT) &&
	      (tptr->down->down->sptr->kind != K_IMPORTEXPORT) &&
	       /* [3] */
	      (! ((tptr->down->down->sptr->kind == K_PFIELD) &&
		  currconstoptlevel)) &&
	       /* [4] */
	      (tptr->down->down->sptr->tref->assignbyref == 0) &&
	      /* [5] */
	      (tptr->down->down->sptr->tref->assignrate <=
	       tptr->down->down->sptr->rate))
	    *lptr = tptr->next;

	}
    }
  return;

}

/***********************************************************************/
/*                optimizes if statements                              */
/*                                                                     */
/*           (done)| IF LP expr RP LC block RC                         */
/***********************************************************************/

void ifoptconst(int passrate, tnode ** lptr)

{
  tnode * tptr = *lptr;
  tnode * gptr = tptr->down->next->next;
  tnode ** bptr;
  tnode * before;
  int flags;

  exprcollapse(passrate, gptr);

  /* for now, we take the most conservative approach to slower-rate */
  /* if statements -- we don't optimize away constant-guards at all */
  /* when we revisit, we should be more aggressive                  */

  flags = tptr->down->inwidth;

  if ((gptr->vol == CONSTANT) && (flags == 0))
    {
      /* we can't unallocate if variable, because */
      /* arrayidx's are already numbered. later,  */
      /* consider ways to handle this problem.    */

      if (((gptr->down->ttype == S_INTGR) &&
	   (atoi(gptr->down->val) == 0)) ||
	  ((gptr->down->ttype == S_NUMBER) &&
	   ((float)atof(gptr->down->val) == 0.0F)))
	{
	  /* false -- delete if statement */

	  *lptr = tptr->next;
	}
      else
	{
	  /* true -- delete guard */

	  gptr = gptr->next->next->next->down;

	  bptr = &gptr;
	  while ((*bptr) != NULL)
	    {
	      /* a good opportunity for slower-rate processing  */
	      /* when we revisit this code: check (*bptr)->rate */

	      bptr = &((*bptr)->next);
	    }
	  if (gptr != NULL)
	    {
	      *lptr = gptr;
	      *bptr = tptr->next;
	    }
	  else
	    *lptr = tptr->next;
	}
    }
  else
    {
      currconstoptif++;

      /* stateoptconst() rate parameters in the section below   */
      /* should be semantically correct, but I may be mistaken. */

      /* pass for slower-rate i-rate statements */

      if (flags & IRATESECTION)
	{ 
	  bptr = &(gptr->next->next->next->down);
	  while ((*bptr) != NULL)
	    {
	      before = *bptr;
	      if ((*bptr)->rate == IRATETYPE)
		stateoptconst(IRATETYPE, bptr);
	      if (before == (*bptr))
		bptr = &((*bptr)->next);
	    }
	}

      /* pass for slower-rate k-rate statements */

      if (flags & KRATESECTION)
	{
	  bptr = &(gptr->next->next->next->down);
	  while ((*bptr) != NULL)
	    {
	      before = *bptr;
	      if ((*bptr)->rate == KRATETYPE)
		stateoptconst(KRATETYPE, bptr);
	      if (before == (*bptr))
		bptr = &((*bptr)->next);
	    }
	}

      /* pass for at-rate statements */

      bptr = &(gptr->next->next->next->down);
      while ((*bptr) != NULL)
	{
	  before = *bptr;
	  if ((*bptr)->rate == passrate)
	    stateoptconst(passrate, bptr);
	  if (before == (*bptr))
	    bptr = &((*bptr)->next);
	}

      currconstoptif--;
    }
  return;
}

/***********************************************************************/
/*                optimizes if-else statements                         */
/*                                                                     */
/*           (done)| IF LP expr RP LC block RC ELSE LC block RC        */
/***********************************************************************/

void ifelseoptconst(int passrate, tnode ** lptr)

{
  tnode * tptr = *lptr;
  tnode * gptr = tptr->down->next->next;
  tnode ** bptr;
  tnode * before;
  int iflags, eflags;

  exprcollapse(passrate, gptr);


  /* for now, we take the most conservative approach to slower-rate */
  /* if statements -- we don't optimize away constant-guards at all */
  /* when we revisit, we should be more aggressive                  */

  iflags = tptr->down->inwidth;
  eflags = tptr->down->next->next->next->next->next->next->next->inwidth;

  if ((gptr->vol == CONSTANT) && (iflags == 0) && (eflags == 0))
    {
      if (((gptr->down->ttype == S_INTGR) &&
	   (atoi(gptr->down->val) == 0)) ||
	  ((gptr->down->ttype == S_NUMBER) &&
	   ((float)atof(gptr->down->val) == 0.0F)))
	{

	  /* false -- insert if block */

	  gptr = gptr->next->next->next->next->next->next->next->down;

	}
      else
	{
	  /* true -- insert if block */

	  gptr = gptr->next->next->next->down;

	}

      /* insertion code */

      bptr = &gptr;
      while ((*bptr) != NULL)
	{
	  /* a good opportunity for slower-rate processing  */
	  /* when we revisit this code: check (*bptr)->rate */

	    bptr = &((*bptr)->next);
	}
      if (gptr != NULL)
	{
	  *lptr = gptr;
	  *bptr = tptr->next;
	}
      else
	*lptr = tptr->next;
    }
  else
    {
      currconstoptif++;

      /* stateoptconst() rate parameters in the section below   */
      /* should be semantically correct, but I may be mistaken. */

      /* pass for slower-rate i-rate statements */

      if (iflags & IRATESECTION)
	{ 
	  bptr = &(gptr->next->next->next->down);
	  while ((*bptr) != NULL)
	    {
	      before = *bptr;
	      if ((*bptr)->rate == IRATETYPE)
		stateoptconst(IRATETYPE, bptr);
	      if (before == (*bptr))
		bptr = &((*bptr)->next);
	    }
	}
      
      /* pass for slower-rate k-rate statements */

      if (iflags & KRATESECTION)
	{
	  bptr = &(gptr->next->next->next->down);
	  while ((*bptr) != NULL)
	    {
	      before = *bptr;
	      if ((*bptr)->rate == KRATETYPE)
		stateoptconst(KRATETYPE, bptr);
	      if (before == (*bptr))
		bptr = &((*bptr)->next);
	    }
	}

      /* pass for at-rate statements */

      bptr = &(gptr->next->next->next->down);
      while ((*bptr) != NULL)
	{
	  before = *bptr;
	  if ((*bptr)->rate == passrate)
	    stateoptconst(passrate, bptr);
	  if (before == (*bptr))
	    bptr = &((*bptr)->next);
	}

      /***********************************************************/
      /* else -- presently loses some optimization opportunities */
      /***********************************************************/

      /* pass for slower-rate i-rate statements */

      if (eflags & IRATESECTION)
	{       
	  bptr = &(gptr->next->next->next->next->next->next->next->down);
	  while ((*bptr) != NULL)
	    {
	      before = *bptr;
	      if ((*bptr)->rate == IRATETYPE)
		stateoptconst(IRATETYPE, bptr);
	      if (before == (*bptr))
		bptr = &((*bptr)->next);
	    }
	}

      /* pass for slower-rate k-rate statements */

      if (eflags & KRATESECTION)
	{ 
	  bptr = &(gptr->next->next->next->next->next->next->next->down);
	  while ((*bptr) != NULL)
	    {
	      before = *bptr;
	      if ((*bptr)->rate == KRATETYPE)
		stateoptconst(KRATETYPE, bptr);
	      if (before == (*bptr))
		bptr = &((*bptr)->next);
	    }
	}

      /* pass for at-rate statements */

      bptr = &(gptr->next->next->next->next->next->next->next->down);
      while ((*bptr) != NULL)
	{
	  before = *bptr;
	  if ((*bptr)->rate == passrate)
	    stateoptconst(passrate, bptr);
	  if (before == (*bptr))
	    bptr = &((*bptr)->next);
	}

      currconstoptif--;
    }
  return;
}

/***********************************************************************/
/*                optimizes while statements                           */
/*                                                                     */
/*           (done)| WHILE LP expr RP LC block RC                      */
/***********************************************************************/

void whileoptconst(int passrate, tnode ** lptr)

{
  tnode * tptr = *lptr;
  tnode * gptr = tptr->down->next->next;
  tnode ** bptr;
  tnode * before;
  int lastrate;

  /* starts function because guard evaluated multiple times */

  currconstoptwhile++;
  lastrate = currconstwhilerate;
  currconstwhilerate = passrate;

  exprcollapse(passrate, gptr);

  if ((gptr->vol == CONSTANT) && 
      ( ((gptr->down->ttype == S_INTGR) &&
	 (atoi(gptr->down->val) == 0)) ||
	((gptr->down->ttype == S_NUMBER) &&
	 ((float)atof(gptr->down->val) == 0.0F))))
    {
      /* false -- delete while statement */
      *lptr = tptr->next;
    }
  else
    {
      bptr = &(gptr->next->next->next->down);
      while ((*bptr) != NULL)
	{
	  before = *bptr;
	  if ((*bptr)->rate == passrate)
	    stateoptconst(passrate, bptr);
	  if (before == (*bptr))
	    bptr = &((*bptr)->next);
	}
    }

  currconstoptwhile--;
  currconstwhilerate = lastrate;

}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*                                                                     */
/* The second-level functions called by exprcollapse().                */
/*                                                                     */
/*_____________________________________________________________________*/


extern void tablemapcollapse(tnode *);

/***********************************************************************/
/*              collapses S_IDENT and constants                        */      
/***********************************************************************/

int tokencollapse(int passrate, tnode * tptr)

{

  if ((tptr->down->ttype == S_NUMBER) ||
      (tptr->down->ttype == S_INTGR)  ||
      ((tptr->down->ttype == S_IDENT) &&
       (standardname(tptr->down) ||
	tptr->down->sptr != NULL)))
    {
      switch (tptr->down->ttype) {
      case S_NUMBER:
	tptr->vol = tptr->down->vol = CONSTANT;
	tptr->res = tptr->down->res = ASFLOAT;
	tptr->rate = tptr->down->rate = IRATETYPE;
	break;
      case S_INTGR:
	tptr->vol = tptr->down->vol = CONSTANT;
	tptr->res = tptr->down->res = ASINT;
	tptr->rate = tptr->down->rate = IRATETYPE;
	break;
      case S_IDENT:
	if (tptr->down->next == NULL)
	  {
	    if (standardname(tptr->down))
	      {
		tptr->vol = standardcollapse(tptr->down);
		if (tptr->vol == CONSTANT)
		  {
		    tptr->res = tptr->down->res;
		    tptr->rate = IRATETYPE;
		  }
		else
		  tptr->res = tptr->down->res = standardres(tptr->down);
	      }
	    else
	      {

		/*
		 * replaces variables currently CONSTANT that 
		 * are SCALARTYPE and aren't written at a faster rate
		 * are in no danger of being assigned a new value in
		 * a while loop.
		 *
		 * later support VECTORTYPE constants
		 *
		 */

		if ((tptr->down->sptr->vartype == SCALARTYPE) &&
		    (tptr->down->sptr->vol == CONSTANT) && 
		    (tptr->down->sptr->tref->assignrate <=
		     tptr->down->sptr->rate) && 
		    ((!currconstoptwhile) || 
		     (tptr->down->sptr->rate != currconstwhilerate) ||
		     (tptr->down->sptr->tref->assignwhile == 0)))
		  {
		    if (tptr->down->sptr->res == ASINT)
		      tptr->down->ttype = S_INTGR;
		    else
		      tptr->down->ttype = S_NUMBER;
		    tptr->down->val = dupval(tptr->down->sptr->consval);
		    tptr->vol = tptr->down->vol = CONSTANT;
		    tptr->rate = tptr->down->rate = IRATETYPE;

		    /* inserts constants ASINT/ASFLOAT status into tree */

		    tptr->res = tptr->down->res = tptr->down->sptr->res;
		    tptr->down->sptr->tref->mirror = REQUIRED; 

		  }
		else
		  {
		    tptr->vol = tptr->down->vol = VARIABLE;

		    /* right now, blocktree cannot ASINT IDENTs */

		    tptr->res = tptr->down->res = ASFLOAT;
		  }

	      }
	  }
	else
	  {
	    /* currently can't reduce indexed arrays */

	    /* reduce index */

	    exprcollapse(passrate, tptr->down->next->next);

	    /* handle tablemaps */

	    if ((tptr->down->vartype == TMAPTYPE) &&
		(tptr->down->next->next->vol == CONSTANT))
	      {
		tablemapcollapse(tptr);
	      }
	    else
	      {
		/* handle array itself */

		tptr->vol = tptr->down->vol = VARIABLE;
		if (standardname(tptr->down))
		  tptr->res = tptr->down->res = standardres(tptr->down);
		else
		  tptr->res = tptr->down->res = tptr->down->sptr->res;
	      }

	  }
      }
      return 1;
    }
  else
    return 0;
}

/***********************************************************************/
/*          collapses constants in unary <expr>                        */      
/***********************************************************************/

int unarycollapse(int passrate, tnode * tptr)

{
  char name[128];
  tnode * t_one;
  tnode * t_two;


  if (((t_one = tptr->down) == NULL) || ((t_two = tptr->down->next) == NULL) ||
      (tptr->down->next->next != NULL))
    return 0;

  if ((t_one->ttype != S_MINUS) && (t_one->ttype != S_NOT)) 
    return 0;
  if (t_two->ttype != S_EXPR)
    return 0;

  exprcollapse(passrate, t_two);

  if (t_two->vol == VARIABLE)
    {
      tptr->vol = VARIABLE;
      tptr->res = t_two->res;
      return 1;
    }

  switch(t_one->ttype) {
  case S_MINUS:   
    if ((t_two->down->ttype == S_NUMBER) || largeinteger(t_two->down->val))
      {
	sprintf(name, "%e", - atof(t_two->down->val));
	t_two->ttype = S_NUMBER;
	t_two->res = ASFLOAT;
      }
    else
      {
	sprintf(name, "%i", - atoi(t_two->down->val));
	t_two->ttype = S_INTGR;
	t_two->res = ASINT;
      }
    break;
  case S_NOT:
    t_two->res = ASINT;
    t_two->ttype = S_INTGR;
    sprintf(name, "%i", !atof(t_two->down->val));
    break;
  default:
    internalerror("optconst.c", "unarycollapse illegal operator");
  }

  t_two->linenum = t_two->down->linenum;
  t_two->filename = t_two->down->filename;
  t_two->val = dupval(name);
  t_two->down = NULL;
  t_two->vol = CONSTANT;
  t_two->rate = IRATETYPE;
  tptr->down = t_two;
  tptr->res = t_two->res;
  tptr->vol = CONSTANT;
  tptr->rate = IRATETYPE;
  return 1;

}

/***********************************************************************/
/*                  collapses floatcast expr                           */      
/***********************************************************************/

int floatcastcollapse(int passrate, tnode * tptr)

{
  tnode * t_three;

  if ((tptr->down == NULL) || (tptr->down->ttype != S_FLOATCAST))
    return 0;

  t_three = tptr->down->next->next;

  if ((tptr->down->next->next->next == NULL) ||
      (tptr->down->next->next->next->next != NULL))
    return 0;

  exprcollapse(passrate, t_three);

  if (t_three->vol == VARIABLE)
    {
      tptr->vol = VARIABLE;
      tptr->res = t_three->res;
      return 1;
    }

  tptr->down = t_three->down;
  tptr->down->next = tptr->down->down = NULL;
  tptr->vol = CONSTANT;
  tptr->res = tptr->down->res = ASFLOAT;
  if (tptr->down->ttype == S_INTGR)
    {
      tptr->down->ttype = S_NUMBER;
      strcat(tptr->down->val,".0");
    }
  tptr->rate = IRATETYPE;
  return 1;

}
    
/***********************************************************************/
/*          collapses constants in  (<expr>)                           */      
/***********************************************************************/

int parencollapse(int passrate, tnode * tptr)

{
  tnode * t_one, * t_two, * t_three;

  if (((t_one = tptr->down) == NULL) || 
      ((t_two = tptr->down->next) == NULL) ||
      ((t_three = tptr->down->next->next) == NULL) || 
      (tptr->down->next->next->next != NULL) ) 
    return 0;

  if ((t_one->ttype != S_LP) || (t_two->ttype != S_EXPR) ||
      (t_three->ttype != S_RP))
    return 0;

  exprcollapse(passrate, t_two);

  tptr->res = t_two->res;
  tptr->vol = t_two->vol;

  if (tptr->vol == CONSTANT)
    {
      tptr->down = t_two->down;
      tptr->rate = IRATETYPE;
      t_two->next = NULL;
    }

  return 1;

}
    
/***********************************************************************/
/*          collapses constants in multi-operator *, +, -              */      
/***********************************************************************/

int multicollapse(int passrate, tnode * tptr)

{
  char name[128];
  tnode * eptr;
  tnode * first = NULL;
  int asanint = 1;
  int tot = 0;
  int res = ASINT;
  float fval = 0.0F;
  int lval = 0;

  if ((tptr->down == NULL) || (tptr->down->ttype != S_EXPR) ||
      (tptr->down->next == NULL) ||
      ( (tptr->down->next->ttype != S_STAR) &&
	(tptr->down->next->ttype != S_PLUS) &&
	(tptr->down->next->ttype != S_MINUS)))
    return 0;


  /* count number of constant operands */

  eptr = tptr->down;

  while (eptr != NULL)
    {
      if (eptr->ttype == S_EXPR)
	{
	  exprcollapse(passrate, eptr);
	  if (res == ASINT)
	    res = eptr->res;
	  if (eptr->vol == CONSTANT)
	    {
	      tot++;
	      if (tot == 1)
		first = eptr;
	      if ((eptr->down->ttype == S_NUMBER) || 
		  largeinteger(eptr->down->val))
		asanint = 0;
	    }
	}
      eptr = eptr->next;
    }

  tptr->res = res;
  if (tot < 2)
    {
      tptr->vol = VARIABLE;
      return 1;
    }

  /* set up first operand, keeping in mind - semantics */

  fval = (float)atof(first->down->val);
  if (asanint)
    lval = atoi(first->down->val);

  if (tptr->down != first)
    {
      eptr = tptr->down;
      while (eptr->next != first)
	eptr = eptr->next;
      if (eptr->ttype == S_MINUS)
	{
	  eptr->ttype = S_PLUS;
	  eptr->val[0] = '+';

	  fval = - fval;
	  if (asanint)
	    lval = - lval;
	}
    }

  /* compute with rest of operands */

  eptr = first;
  while (eptr->next != NULL)
    {
      if (eptr->next->next->vol == CONSTANT)
	{
	  switch (eptr->next->ttype) {
	  case S_STAR:
	    fval *= (float)atof(eptr->next->next->down->val);
	    if (asanint)
	      lval *= atoi(eptr->next->next->down->val);
	    break;
	  case S_PLUS:
	    fval += (float)atof(eptr->next->next->down->val);
	    if (asanint)
	      lval += atoi(eptr->next->next->down->val);
	    break;
	  case S_MINUS:
	    fval -= (float)atof(eptr->next->next->down->val);
	    if (asanint)
	      lval -= atoi(eptr->next->next->down->val);
	    break;
	  }
	  eptr->next = eptr->next->next->next;
	}
      else
	eptr = eptr->next->next;
    }

  if ((tptr->down == first) && (first->next == NULL))
    {
      eptr = first;
      eptr->linenum = eptr->down->linenum;
      eptr->filename = eptr->down->filename;
    }
  else
    eptr = first->down;

  if (asanint && ((float)lval == fval))
    {
      eptr->ttype = S_INTGR;
      eptr->res = ASINT;
      sprintf(name, "%i", lval);
    }
  else
    {
      eptr->ttype = S_NUMBER;
      eptr->res = ASFLOAT;
      sprintf(name, "%e", fval);
      tptr->res = ASFLOAT;
    }

  eptr->vol = CONSTANT;
  eptr->val = dupval(name);
  eptr->down = NULL;
  eptr->rate = IRATETYPE;

  if ((tptr->down == first) && (first->next == NULL))
    {
      tptr->res = eptr->res; 
      tptr->vol = CONSTANT;
      tptr->rate = IRATETYPE;
    }

  return 1;

}


extern void dividereduction(tnode *, tnode *);

/***********************************************************************/
/*      collapses constants in binary <expr> (except for +,-,*)        */      
/***********************************************************************/

int binarycollapse(int passrate, tnode * tptr)

{
  char name[128];
  tnode * t_one, * t_two, * t_three;


  if (((t_one = tptr->down) == NULL) || ((t_two = tptr->down->next) == NULL) ||
      ((t_three = tptr->down->next->next) == NULL) ||
      (tptr->down->next->next->next != NULL))
    return 0;

  /* binary ops below */

  if ((t_one->ttype != S_EXPR) || (t_three->ttype != S_EXPR))
    return 0;

  if ((t_two->ttype != S_LEQ) && (t_two->ttype != S_GEQ) && 
      (t_two->ttype != S_NEQ) && (t_two->ttype != S_EQEQ) && 
      (t_two->ttype != S_GT) &&  (t_two->ttype != S_LT) &&  
      (t_two->ttype != S_AND) && (t_two->ttype != S_OR) &&  
      (t_two->ttype != S_SLASH)) 
    return 0;

  exprcollapse(passrate, t_one);
  exprcollapse(passrate, t_three);

  if ((t_one->vol == VARIABLE) && (t_two->ttype == S_SLASH) &&
      (t_three->vol == CONSTANT))
    {
      dividereduction(t_two, t_three);
      tptr->vol = VARIABLE;
      tptr->res = ASFLOAT;
      return 1;
    }

  if ((t_one->vol == VARIABLE) || (t_three->vol == VARIABLE))
    {
      tptr->vol = VARIABLE;
      if ((t_one->res == ASINT) && (t_three->res == ASINT))
	tptr->res = ASINT;
      else
	tptr->res = ASFLOAT;
      return 1;
    }

  switch (t_two->ttype)
    {
    case S_LEQ:
      sprintf(name,"%i", atof(t_one->down->val) <=
	      atof(t_three->down->val) );
      t_one->res = ASINT;
      t_one->ttype = S_INTGR;
      break;
    case S_GEQ:
      sprintf(name,"%i", atof(t_one->down->val) >=
	      atof(t_three->down->val) );
      t_one->res = ASINT;
      t_one->ttype = S_INTGR;
      break;
    case S_NEQ:   
      sprintf(name,"%i", atof(t_one->down->val) !=
	      atof(t_three->down->val) );
      t_one->res = ASINT;
      t_one->ttype = S_INTGR;
      break;
    case S_EQEQ:
      sprintf(name,"%i", atof(t_one->down->val) ==
	      atof(t_three->down->val) );
      t_one->res = ASINT;
      t_one->ttype = S_INTGR;
      break;
    case S_GT:
      sprintf(name,"%i", atof(t_one->down->val) >
	      atof(t_three->down->val) );
      t_one->res = ASINT;
      t_one->ttype = S_INTGR;
      break;
    case S_LT:
      sprintf(name,"%i", atof(t_one->down->val) <
	      atof(t_three->down->val) );
      t_one->res = ASINT;
      t_one->ttype = S_INTGR;
      break;
    case S_AND:
      sprintf(name,"%i", atof(t_one->down->val) &&
	      atof(t_three->down->val) );
      t_one->res = ASINT;
      t_one->ttype = S_INTGR;
      break;
    case S_OR:   
      sprintf(name,"%i", atof(t_one->down->val) ||
	      atof(t_three->down->val) );
      t_one->res = ASINT;
      t_one->ttype = S_INTGR;
      break;
    case S_SLASH:
      if (atof(t_three->down->val) == 0.0F)
	{
	   printf("Error: Divide by zero in constant expression.\n\n");
	   showerrorplace(t_three->down->linenum, t_three->down->filename);
	}
      sprintf(name,"%e", atof(t_one->down->val) /
	      atof(t_three->down->val) );
      t_one->res = ASFLOAT;
      t_one->ttype = S_NUMBER;
      break;
    default:
      internalerror("parsehelp.c", "binarycollapse illegal operator");
    }
  t_one->linenum = t_one->down->linenum;
  t_one->filename = t_one->down->filename;
  t_one->val = dupval(name);
  t_one->next = t_one->down = NULL;
  t_one->rate = IRATETYPE;
  tptr->res = t_one->res;
  tptr->vol = CONSTANT;
  tptr->rate = IRATETYPE;
  return 1;

}

/***********************************************************************/
/*          collapses constant switch statement                        */      
/***********************************************************************/

int switchcollapse(int passrate, tnode * tptr)

{
  tnode * t_one; 
  tnode * t_two; 
  tnode * t_three;
  tnode * t_four;
  tnode * t_five;

  if (((t_one = tptr->down) == NULL) || ((t_two = tptr->down->next) == NULL) ||
      ((t_three = tptr->down->next->next) == NULL) || 
      ((t_four = tptr->down->next->next->next) == NULL) || 
      ((t_five = tptr->down->next->next->next->next) == NULL) || 
      (tptr->down->next->next->next->next->next != NULL))  
    return 0;

  if ((t_one->ttype != S_EXPR) || (t_two->ttype != S_Q) ||
      (t_three->ttype != S_EXPR) || (t_four->ttype != S_COL) ||
      (t_five->ttype != S_EXPR))
    return 0;


  exprcollapse(passrate, t_one);
  exprcollapse(passrate, t_three);
  exprcollapse(passrate, t_five);

  if (t_one->vol == VARIABLE)
    {
      tptr->vol = VARIABLE;
      if ((t_three->res == ASINT) && (t_five->res == ASINT))
	tptr->res = ASINT;
      else
	tptr->res = ASFLOAT;
      return 1;
    }

  if ((t_three->vol == VARIABLE) || (t_five->vol == VARIABLE))
    {
      tptr->vol = VARIABLE;
      if (atof(t_one->down->val))
	{
	  tptr->down = make_tnode("(", S_LP);
	  tptr->down->next = t_three;
	  t_three->next = make_tnode(")", S_RP);
	  tptr->res = t_three->res;
	  tptr->vartype = t_three->vartype;
	  return 1;
	}
      else
	{
	  tptr->down = make_tnode("(", S_LP);
	  tptr->down->next = t_five;
	  t_five->next = make_tnode(")", S_RP);
	  tptr->res = t_five->res;
	  tptr->vartype = t_five->vartype;
	  return 1;
	}
    }

  tptr->vol = CONSTANT;
  tptr->rate = IRATETYPE;

  if (atof(t_one->down->val))
    {
      t_one->val = t_three->down->val;
      t_one->ttype =  t_three->down->ttype;
      t_one->res = t_three->down->res;
      t_one->linenum = t_three->down->linenum;
      t_one->filename = t_three->down->filename;
    }
  else
    {
      t_one->val = t_five->down->val;
      t_one->ttype =  t_five->down->ttype;
      t_one->res = t_five->down->res;
      t_one->linenum = t_five->down->linenum;
      t_one->filename = t_five->down->filename;
    }
  t_one->down = t_one->next = NULL;
  t_one->rate = IRATETYPE;
  tptr->res = t_one->res;
  return 1;
}


extern void userdefcollapse(int, tnode *, int);

/***********************************************************************/
/*          collapses constant-filled opcode calls                     */
/*                                                                     */
/*                 IDENT LP exprlist RP                                */
/*                 IDENT LB expr RB LP exprlist RP                     */
/***********************************************************************/

int opcodecollapse(int passrate, tnode * tptr)

{
  tnode * t_one;
  tnode * t_two;
  tnode * t_three;
  tnode * t_four;
  tnode * iptr;
  tnode * fptr = NULL;
  int allconst;
  int core;

  if (((t_one = tptr->down) == NULL) || ((t_two = tptr->down->next) == NULL) ||
      ((t_three = tptr->down->next->next) == NULL) ||
      ((t_four = tptr->down->next->next->next) == NULL))
    return 0;

  if ((t_one->ttype != S_IDENT) || ((t_two->ttype != S_LP) &&
				    (t_two->ttype != S_LB)))
    return 0;

  allconst = 1;
  core = coreopcodename(t_one);

  if (t_two->ttype == S_LB)
    {
      exprcollapse(passrate, t_three);

      if (t_three->vol == VARIABLE)
	allconst = 0;

      t_three = t_four->next->next;
    }

  if ((t_three->ttype != S_EXPRLIST))
    return 0;

  iptr = t_three->down;

  if (!core)
    fptr = t_one->optr->sptr->defnode->down->next->next->next->down;

  while (iptr != NULL)
    {
      if (iptr->ttype == S_EXPR)
	{

	  /* first reduce expression */


	  if (core)
	    exprcollapse(passrate, iptr);
	  else
	    {

	      /* special tasks for user-defined opcodes */

	      /* call-by reference check */

	      if ((iptr->down->ttype == S_IDENT)
		  && (iptr->down->sptr != NULL) && 
		  (fptr->sptr->tref->assigntot))
		{
		  iptr->down->sptr->vol = VARIABLE;
		  iptr->down->sptr->res = ASFLOAT;
		}
	      else
		exprcollapse(passrate, iptr);

	      /* opcode symbol-table constant initialization */
	      /*    later support VECTORTYPE constants       */

	      if ((fptr->sptr->vartype == SCALARTYPE)
		  && (iptr->vol == CONSTANT))
		{
		  fptr->sptr->vol = iptr->vol;
		  fptr->sptr->res = iptr->res;
		  fptr->sptr->consval = 
		    dupval(iptr->down->val);
		}

	    }

	  if (iptr->vol == VARIABLE)
	    allconst = 0;

	}
      iptr = iptr->next;

      if (!core)
	fptr = fptr->next;
    }

  tptr->vol = VARIABLE;

  if (core)
    {
      /* const-optimizing core opcodes */

      if (!allconst)              /* only reduce if all arguments constant */
	return 1;
      if (!coreopcodeprecompute(t_one))
	return 1;

      tptr->vol = t_one->vol = CONSTANT;

      hascoreopcode(t_one, -1);
      coreopcodecollapse(t_one, t_three->down);

      /* we used to delete opcode entry in t_one->optr here. now,    */
      /* we just invalidate it by setting  t_one->optr->optr to NULL */

      t_one->optr->optr = NULL;
      tptr->optr = t_one->optr = NULL;
      t_one->next = NULL;
      tptr->res = t_one->res;
      tptr->rate = t_one->rate = IRATETYPE;
    }
  else
    {
      /* const-optimizing user-defined opcodes */

      tptr->res = ASFLOAT;   
      userdefcollapse(passrate, tptr, allconst);
    }

  return 1;
}


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*                                                                     */
/* Utility functions, used by second level optconst calls.             */
/*                                                                     */
/*_____________________________________________________________________*/


/***********************************************************************/
/* collapses tablemaps with constant indexes: used in tolencollapse()  */      
/***********************************************************************/

void tablemapcollapse(tnode * tptr)

{
  int index;
  tnode * iptr;

  iptr = tptr->down->next->next->down;
  if ((iptr->ttype != S_INTGR) && (iptr->ttype != S_NUMBER))
    internalerror("optconst.c", "tmapcollapse -- 1");

  if ((index = make_int(iptr)) < 0)
    {
      printf("Error: Tablemap constant index is negative.\n\n");
      showerrorplace(tptr->down->next->linenum, tptr->down->next->filename);
    }
  
  iptr = tptr->down->sptr->defnode->down->next->next->next->down;
  while ((iptr->next != NULL) && (index > 0))
    {
      index--;
      iptr = iptr->next->next;
    }

  if (index)
    {
      printf("Error: Tablemap constant index is out of range.\n\n");
      showerrorplace(tptr->down->next->linenum, tptr->down->next->filename);
    }

  tptr->down->next = NULL;
  tptr->down->vartype = TABLETYPE;
  if (iptr->sptr == NULL)
    internalerror("optconst.c", "tmapcollapse -- 2");

  tptr->down->val = dupval(iptr->sptr->val);
  tptr->down->sptr = iptr->sptr;
  tptr->rate = tptr->down->rate = IRATETYPE;

}

/***********************************************************************/
/* strength reduction for constant divide: used in binarycollapse()    */      
/***********************************************************************/

void dividereduction(tnode * t_two, tnode * t_three)

{
  char name[128];

  if (atof(t_three->down->val) == 0.0F)
    {
      printf("Error: Divide by zero in constant expression.\n\n");
      showerrorplace(t_three->down->linenum, t_three->down->filename);
    }
  sprintf(name,"%e", 1/atof(t_three->down->val));
  t_three->down->val = dupval(name);
  t_three->down->ttype = S_NUMBER;
  t_three->res = t_three->down->res = ASFLOAT;
  t_three->rate = t_three->down->rate = IRATETYPE;
  t_two->val = "*";
  t_two->ttype = S_STAR;

}

/***********************************************************************/
/* collapses user-define opcode calls: used in opcodecollapse()        */
/***********************************************************************/

void userdefcollapse(int passrate, tnode * t_opcode, int allconst)

{
  sigsym * currinstrstack;
  int currconstoptifstack;
  int currconstoptwhilestack;
  int ratetype;
  tnode ** tptr;
  tnode * before, * eptr;

  currinstrstack = currconstoptlevel;
  currconstoptifstack = currconstoptif;
  currconstoptwhilestack = currconstoptwhile;

  currconstoptlevel = t_opcode->optr->sptr;

  /* can't delete opcode of any table expressions are non-constant */

  before = t_opcode->optr->sptr->defnode->down
    ->next->next->next->next->next->next->down;

  while (before != NULL)
    {
      if (before->ttype == S_TABLE)
	{
	  eptr = before->down->next->next->next->next->next->down;
	  while (eptr)
	    {
	      if (eptr->ttype == S_EXPR)
		{
		  exprcollapse(IRATETYPE, eptr);
		  if (eptr->vol != CONSTANT)
		    allconst = 0;
		}
	      eptr = eptr->next;
	    }
	}

      /* exports has semantics in absence of code */

      if ((before->ttype == S_OPVARDECL) &&
	  (before->down->ttype == S_TAGLIST) &&
	  (before->down->down->ttype == S_EXPORTS) && 
	  (before->down->down->next == NULL))
	{
	  allconst = 0;
	}

      before = before->next;
    }

  /* stateoptconst() rate parameters in the section below   */
  /* should be semantically correct, but I may be mistaken. */

  for (ratetype = IRATETYPE; ratetype <= passrate; ratetype++)
    {
      tptr = &(currconstoptlevel->defnode->down
	       ->next->next->next->next->next->next->next->down);
      while ((*tptr) != NULL)
	{
	  before = *tptr;

	  currconstoptif = 0;
	  currconstoptwhile = 0;
	  if ((*tptr)->rate == ratetype)
	    stateoptconst(ratetype, tptr);
      
	  if (before == (*tptr))
	    tptr = &((*tptr)->next);
	}
    }

  /* placement so that if statement below could delete elements */
  /* from the symbol table, although we don't do this anymore   */

  currconstoptlevel = currinstrstack;
  currconstoptif = currconstoptifstack;
  currconstoptwhile = currconstoptwhilestack;

  /* see if entire opcode can be eliminated */

  if (allconst)
    {
      before = t_opcode->optr->sptr->defnode->down
	       ->next->next->next->next->next->next->next->down;


      /* for now, only width 1 return values supported */
      /* improve once VECTORTYPE is supported */

      /* RETURN LP exprlist RP SEM */

      if ((before->down->ttype == S_RETURN) &&
	  (before->next == NULL)  &&
	  (before->down->next->next->down->next == NULL) &&  
	  (before->down->next->next->down->width == 1) &&
	  (before->down->next->next->down->vol == CONSTANT) )
	{
	  /* a return statement with a constant value */

	  t_opcode->down->ttype = before->down->next->next->down->down->ttype;
	  t_opcode->res = t_opcode->down->res = 
	    before->down->next->next->down->down->res;
	  t_opcode->rate = t_opcode->down->rate = IRATETYPE;
	  t_opcode->vol = t_opcode->down->vol = CONSTANT;
	  t_opcode->width = t_opcode->down->width = 1;
	  t_opcode->vartype = t_opcode->down->vartype = SCALARTYPE;
	  t_opcode->down->val = 
	    dupval(before->down->next->next->down->down->val);

	  /* we used to delete opcode entry in t_opcode->down->optr. */
	  /* we just invalidate it by setting its optr to NULL       */

	  t_opcode->down->optr->optr = NULL;
	  t_opcode->down->optr = t_opcode->optr = NULL;

	  t_opcode->down->next = NULL;
	}
    }

}
