
/*
#    Sfront, a SAOL to C translator    
#    This file: pushes k-rate/i-rate expressions out of a-rate statements
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

extern void assignoptrate(int, tnode *, tnode **, int *);
extern void simpleoptrate(int, tnode *, tnode **, int *);
extern void guardoptrate(int, tnode *, tnode **, int *);
extern void tmapoptrate(int, tnode *, tnode **, int *);
extern int saferateopt(int, tnode *);
extern void ratefactor(tnode *, tnode **,  int *,  int);
extern void multioptrate(int, tnode *);
extern void idxoptrate(int, tnode *, tnode **, int *);
extern void opcodemirrorupdate(tnode *, tnode *);


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*       void stateoptrate() and void exproptrate()                    */
/*                                                                     */
/* These functions do rate optimization on SAOL statements and         */
/* expressions, and are called from optmain.c. Following these         */
/* functions are the second-level functions for rate optimization.     */
/*                                                                     */
/*_____________________________________________________________________*/


/***********************************************************************/
/*                rate optimizes a statement                           */
/*                                                                     */
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
/***********************************************************************/

void stateoptrate(int staterate, tnode * tptr, tnode ** lines, int * num)

{


  switch(tptr->down->ttype) {
  case S_LVALUE:
    assignoptrate(staterate, tptr, lines, num);
    break;
  case S_EXPR:
  case S_INSTR:
  case S_OUTPUT:
  case S_SPATIALIZE:
  case S_OUTBUS:
  case S_EXTEND:
  case S_RETURN:
  case S_PRINTF:
    simpleoptrate(staterate, tptr, lines, num);
    break;
  case S_IF:
  case S_WHILE:
    guardoptrate(staterate, tptr, lines, num);
  }

}  

/***********************************************************************/
/*                     reduces an expr                                 */
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

void exproptrate(int staterate, tnode * tptr, tnode ** lines, int *num)

{
  tnode * fptr = NULL;

  if (tptr->rate < staterate)
    {
      switch (tptr->down->ttype) {
      case S_NUMBER:
      case S_INTGR:
	break;
      case S_IDENT:
	if (tptr->down->next == NULL)                /* simple variable */
	  break;
	if (tptr->down->vartype == TMAPTYPE)         /* tablemap */
	  {
	    tmapoptrate(staterate, tptr, lines, num);  /* optimize index */
	    break;
	  }
	/* indexed arrays with constant index */
	if ((tptr->down->next->next->ttype == S_EXPR) &&
	    (tptr->down->next->next->next->next == NULL) &&
	    (tptr->down->next->next->vol == CONSTANT))
	  break;
      case S_EXPR:
      case S_NOT:
      case S_LP:
      case S_FLOATCAST:
      case S_MINUS:
	if (saferateopt(staterate, tptr))
	  {
	    ratefactor(tptr, lines, num, ASFLOAT);
	  }
	break;
      default:
	internalerror("optrate.c","error 1 in exproptrate()");
      }
    }
  else
    {
      multioptrate(staterate, tptr);
      tptr = tptr->down;
      switch (tptr->ttype) {
      case S_NUMBER:
      case S_INTGR:
	break;
      case S_IDENT:
	if (tptr->next == NULL)                /* simple variable */
	  break;
	if (tptr->next->ttype == S_LB)
	  {
	    idxoptrate(staterate, tptr->next->next, lines,  num);   
         
	    /* break if indexed tmap or array */

	    if (!(tptr->next->next->next->next)) 
	      break;    

	    /* an oparray call -- save formal parameters */
	    
	    if (!coreopcodename(tptr))
	      fptr = tptr->optr->sptr->defnode->down->next->next->next->down;

	    tptr = tptr->next->next->next;   /* push past brackets */
	  }
	else
	  {
	    /* opcode call */

	    if (!coreopcodename(tptr))
	      fptr = tptr->optr->sptr->defnode->down->next->next->next->down;

	  }
	tptr = tptr->next->next->down;  /* opcode/oparray falls through */
      case S_EXPR:
      case S_NOT:
      case S_MINUS:
      case S_LP:
      case S_FLOATCAST:
	while (tptr != NULL)                    
	  {
	    if (tptr->ttype == S_EXPR)
	      {
		exproptrate(staterate, tptr, lines, num);
		opcodemirrorupdate(tptr, fptr);
	      }
	    tptr = tptr->next;
	    fptr = fptr ? (fptr->next) : NULL;
	  }
	break;
      default:
	internalerror("optrate.c","error 2 in exproptrate()");
      }
    }
  
}


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*                                                                     */
/* The second-level functions called by stateoptrate(), many of        */
/* of which use exproptrate to handle expressions.                     */
/*                                                                     */
/*_____________________________________________________________________*/


/***********************************************************************/
/*                rate optimizes an assignment statement               */
/*                                                                     */
/* lvalue          : IDENT                                             */
/*                 | IDENT LB expr RB                                  */
/*                                                                     */
/* assignment: lvalue EQ expr SEM                                      */
/***********************************************************************/

void assignoptrate(int staterate, tnode * tptr, tnode ** lines, int * num)

{

  if ((staterate == ARATETYPE)||(staterate == KRATETYPE))
    {
      if (tptr->down->down->next != NULL)  /* lval is an array */
	{
	  exproptrate(staterate, tptr->down->down->next->next,
		      lines, num);
	}
      exproptrate(staterate, tptr->down->next->next,
		  lines, num);
    }
  return;

}


/***********************************************************************/
/*                optimizes simple statements                          */
/*                                                                     */
/*                   expr SEM                                          */
/*                   INSTR IDENT LP exprlist RP SEM                    */
/*                   OUTPUT LP exprlist RP SEM                         */
/*                   SPATIALIZE LP exprlist RP SEM                     */
/*                   OUTBUS LP IDENT COM exprlist RP SEM               */
/*                   EXTEND LP expr RP SEM                             */
/*                   RETURN LP exprlist RP SEM                         */
/*                   PRINTF LP exprstrlist RP SEM                      */
/***********************************************************************/

void simpleoptrate(int staterate, tnode * tptr, tnode ** lines, int * num)

{
  tnode * lptr;

  if ((staterate == ARATETYPE)||(staterate == KRATETYPE))
    {
      lptr = tptr->down;
      while ((lptr->ttype != S_EXPRLIST)&&
	     (lptr->ttype != S_EXPR) &&
	     (lptr->ttype != S_EXPRSTRLIST))
	lptr = lptr->next;
      if (lptr->ttype == S_EXPR)
	exproptrate(staterate, lptr, lines, num);
      else
	{
	  lptr = lptr->down;
	  while (lptr != NULL)
	    {
	      if (lptr->ttype == S_EXPR)
		exproptrate(staterate, lptr, lines, num);
	      lptr = lptr->next;
	    }
	}
    }
  return;


}


extern void guardreduce(int, tnode *, tnode **, int *);


/***********************************************************************/
/*                optimizes statements with guards                     */
/*                                                                     */
/*           (done)| IF LP expr RP LC block RC                         */
/*           (done)| IF LP expr RP LC block RC ELSE LC block RC        */
/*           (done)| WHILE LP expr RP LC block RC                      */
/***********************************************************************/

void guardoptrate(int staterate, tnode * tstate, tnode ** lines, int * num)

{
  tnode * guardptr;

  if ((staterate == ARATETYPE)||(staterate == KRATETYPE))
    {
      guardptr = tstate->down->next->next;

      /* first optimize guard */

      exproptrate(staterate, guardptr, lines, num);

      /* do rate-optimizations  */

      if (staterate == ARATETYPE)
	guardreduce(KRATETYPE, tstate, lines, num);
      guardreduce(staterate, tstate, lines, num);

    }
  return;

}


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*                                                                     */
/* The second-level functions called by exproptrate().                 */
/*                                                                     */
/*_____________________________________________________________________*/


extern void tmapfactor(tnode *, tnode **,  int *);


/***********************************************************************/
/*              optimizes tablemap indexes                             */
/***********************************************************************/

void tmapoptrate(int staterate, tnode * tptr, tnode ** lines, int *num)

{
  tnode * eptr = tptr->down->next->next;  

  if (eptr->rate < staterate) 
    {
      if (saferateopt(staterate, eptr))
	{
	  tmapfactor(tptr, lines, num);
	  return;
	}
    }

  /* if falls through, optimize sub-expressions */

  exproptrate(staterate, eptr, lines, num);
            
}


extern int namecheck(tnode * , char * );
extern int nospeedtraps(tnode *, int);


/***********************************************************************/
/*           checks to see if optimization is safe                     */
/***********************************************************************/

int saferateopt(int staterate, tnode * tptr)

{
  /* first check for dur in the expression, and adjust */
  /* expression rate accordingly                       */

  if ( ((currinstrument->cref->kadur)||(currinstrument->cref->idur))
       && namecheck(tptr,"dur"))
    {
      /* not safe to push dur from krate to irate, since */
      /* dur's value can change (SASL tempo line, extend)*/

      if (staterate == KRATETYPE)
	return 0;

      /* make sure expression isn't moved to i-rate */
      /* can be altered (by tptr->rate = KRATETYPE) */
      /* once opcode calls integrated into instrs   */

      if (tptr->rate == IRATETYPE)
	tptr->rate = KRATETYPE; 

    }

  /* search expression tree for speedtraps and other problems */

  if (!nospeedtraps(tptr, tptr->rate))
    return 0;

  return 1;

}

extern void updatecallrate(tnode * tptr, int rate);

/***********************************************************************/
/*    factors out subexpression, creates temporary variable            */
/***********************************************************************/

void ratefactor(tnode * tptr, tnode ** lines,  int *num,  int newres)

{
  tnode * nptr;
  char name[128];

  /*
  printf("optimizing: ");
  nptr = tptr->down;
  while (nptr != NULL)
    {
      if (nptr->ttype == S_EXPR)
	printf(" %s ", nptr->down->val);
      else
	printf(" %s ", nptr->val);
      nptr = nptr->next;
    }
  printf("\n");
  */

  /* make new assignment statement */

  (*lines)->next = nptr = make_tnode("<statement>",S_STATEMENT);
  (*lines) = nptr;

  nptr->rate = tptr->rate;


  nptr->down = make_tnode("<lvalue>",S_LVALUE);
  nptr = nptr->down;
  nptr->rate = tptr->rate;
  nptr->vartype = tptr->vartype;
  nptr->res = newres;
  nptr->width = tptr->width;
  nptr->next = make_tnode("=",S_EQ);
  nptr = nptr->next;
  nptr->next = eclone(tptr);
  nptr = nptr->next;
  updatecallrate(nptr, tptr->rate);
  nptr->next = make_tnode(";", S_SEM);

  /* make new lval */

  nptr = (*lines)->down;  /* <lvalue> */
  sprintf(name, "_tvr%i", (*num)++);
  nptr->down = make_tnode(dupval(name),S_IDENT);
  nptr = nptr->down;
  nptr->rate = tptr->rate;
  nptr->width = tptr->width;
  nptr->res = newres;
  if (addsym(&locsymtable, nptr) != INSTALLED)
    internalerror("optrate.c","error 1 in ratefactor()");
  nptr->sptr = locsymtable;
  vmcheck(nptr->sptr->tref = (trefer *) calloc(1, sizeof(trefer)));
  nptr->vartype = nptr->sptr->vartype = tptr->vartype;
  nptr->vol = nptr->sptr->vol =  tptr->vol;

  /* change tptr */

  tptr->down = make_tnode(dupval(name),S_IDENT);
  tptr->down->res = newres;
  tptr->down->sptr = locsymtable;
  if (tptr->optr)
    tptr->optr = NULL;

  tptr->res = newres;

}


/***********************************************************************/
/*       checks for EXPR * EXPR * EXPR, and makes lower-rate           */
/*                 sub-expressions if possible.                        */
/***********************************************************************/

void multioptrate(int staterate, tnode * tptr)

{
  int cnt = 0;
  int sign = 0;
  int maxrate = IRATETYPE;
  tnode * cptr;
  tnode * eptr = NULL;
  tnode * lptr;
  tnode * nptr;

  /* leave quickly if not a multi expression */

  if ((tptr->down->ttype != S_EXPR) ||
      (tptr->down->next  == NULL) || 
      ((tptr->down->next->ttype != S_PLUS) &&
       (tptr->down->next->ttype != S_MINUS) &&
       (tptr->down->next->ttype != S_STAR)))
    return;

  lptr = cptr = tptr->down;

  while (cptr != NULL)
    {
      if ((cptr->ttype == S_EXPR) && (cptr->rate < staterate)
	  && (cptr->time = (float)saferateopt(staterate, cptr)))
	{
	  if (!(cnt++))
	    eptr = lptr;
	  if (cptr->rate > maxrate)
	    maxrate = cptr->rate;
	}
      lptr = cptr;
      cptr = cptr->next;
    }

  if (cnt >= 2)  /* make subexpression */
    {
      if (eptr->ttype == S_MINUS)
	sign = 1;

      nptr = make_tnode("<expr>",S_EXPR);
      nptr->rate = maxrate;
      if (eptr == tptr->down)
	{
	  nptr->down = tptr->down;
	  nptr->next = tptr->down->next;
	  nptr->down->next = NULL;
	  tptr->down = nptr;
	}
      else
	{
	  nptr->down = eptr->next;
	  nptr->next = eptr->next->next;
	  nptr->down->next = NULL;
	  eptr->next = nptr;
	}

      eptr->time = 0;
      eptr = nptr;
      nptr = nptr->down;

      while (eptr->next != NULL)
	{
	  cptr = eptr->next->next;
	  if ((cptr->rate < staterate) && cptr->time)
	    {
	      if (sign)
		{
		  switch(eptr->next->ttype) {
		  case S_PLUS:
		    eptr->next->ttype = S_MINUS;
		    eptr->next->val[0] = '-';
		    break;
		  case S_MINUS:
		    eptr->next->ttype = S_PLUS;
		    eptr->next->val[0] = '+';
		    break;
		  default:
		    internalerror("optrate.c","error 1 in multioptrate()");
		  }
		}
	      nptr->next = eptr->next;
	      eptr->next = eptr->next->next->next;
	      nptr->next->next->next = NULL;
	      nptr = nptr->next->next;
	      cptr->time = 0;
	    }
	  else
	    eptr = cptr;
	}

    }
  else          /* clear time flags */
    {
      tptr = tptr->down;
      while (tptr != NULL)
	{
	  tptr->time = 0;
	  tptr = tptr->next;
	}
    }
}
  

/***********************************************************************/
/*              optimizes indices intended to be integer               */
/***********************************************************************/

void idxoptrate(int staterate, tnode * tptr, tnode ** lines, int *num)

{
  /* see if entire expression can be rate optimized */

  if (tptr->rate < staterate) 
    {
      switch(tptr->down->ttype) {
      case S_IDENT:

	/* if its an unindexed variable that is */
	/* already an NVI(), just leave now     */

	if ((tptr->down->next == NULL) &&
	    (tptr->res == ASINT))
	  return;

	/* all others fall through */

      case S_EXPR:
      case S_NOT:
      case S_LP:
      case S_FLOATCAST:
      case S_MINUS:
	if (saferateopt(staterate, tptr))
	  {
	    ratefactor(tptr, lines, num, ASINT);
	    return;
	  }
	break;
      default:
	break;
      }
    }

  /* if falls through, do the normal thing */

  exproptrate(staterate, tptr, lines, num);            
}



/***********************************************************************/
/*    updates mirror flag of user-defined opcode formal parameters     */
/*    with newly created actual values (checks for _tvr naming)        */
/***********************************************************************/

void opcodemirrorupdate(tnode * aptr, tnode * fptr)

{
  /* cull for only op(array) signal variables w/o mirror flag set */
  /* which don't change value of the variable                     */

  if ((!fptr) || (fptr->sptr->vartype == TABLETYPE) ||
      (fptr->sptr->tref->mirror == OPCODEMIRROR) || 
      (fptr->sptr->tref->assigntot))
    return;

  /* updates mirror status of new temporary variables */

  if ((aptr->down != NULL) && (aptr->down->ttype == S_IDENT)
      && (aptr->down->sptr != NULL) && (aptr->down->next == NULL) &&
      (strlen(aptr->down->val) > 4) && (!strncmp(aptr->down->val, "_tvr", 4)))
    {
      fptr->sptr->tref->mirror = OPCODEMIRROR;
    }
}



/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*                                                                     */
/* Utility functions, used by second level optrate calls and other     */
/* files (treeupdate.c)                                                */
/*                                                                     */
/*_____________________________________________________________________*/


/***********************************************************************/
/* used in treeupdate.c: checks if an opcode can use looser rate rules */
/***********************************************************************/

int looseopcoderules(tnode * tptr)

{

  /* if not doing rate optimization, no need for loose rules */

  if (rateoptimize == 0)
    return 0;

  /* exceptions only made for polymorphic opcodes */

  if (tptr->sptr->rate != XRATETYPE)
    return 0;

  /* exceptions only made for core opcodes */

  if (!coreopcodename(tptr->down))
    return 0;

  /* even if these opcodes set to too slow of a rate, the    */
  /* exceptions starting in line 840 of blocktree.c (the     */
  /* oparray/opcode call comment) ensure it will run anyways */
  /* so no need to be more paranoid                          */

  return 1;

}


/***********************************************************************/
/* used in guardoptrate(), optimizes guarded blocks at a given rate    */
/*                                                                     */
/*           (done)| IF LP expr RP LC block RC                         */
/*           (done)| IF LP expr RP LC block RC ELSE LC block RC        */
/*           (done)| WHILE LP expr RP LC block RC                      */
/***********************************************************************/

void guardreduce(int staterate, tnode * tstate, tnode ** lines, int * num)

{
  tnode * tptr;
  tnode * guardptr;          
  tnode * ifptr = NULL;     /* also used for while */
  tnode * elseptr = NULL;   
  tnode * xtra;             /* new if or while statements */
  tnode * top;              /* dummy if or while statement */
  tnode * elsextra = NULL;  /* holds else statements */
  tnode * elsetop = NULL;   /* dummy else statements */
  int unguarded, ginc;      /* to evaluate subexprs w/o guard */

  /* if IF block has slower-rate statements, we cannot      */
  /* rate optimize even a-rate statements, due to execution */
  /* order issues. Later revisit and be more aggressive.    */

  if ((tstate->down->ttype == S_IF) && tstate->down->inwidth)
    return;

  /* list of new if/while statements */

  top = xtra = make_tnode("<statement>",S_STATEMENT); 

  /* set up statement element pointers */

  guardptr = tstate->down->next->next;

  ifptr = tstate->down->next->next->next->next->next;
  if (ifptr->next->next)
    {

      /* if ELSE block has slower-rate statements, we cannot    */
      /* rate optimize even a-rate statements, due to execution */
      /* order issues. Later revisit and be more aggressive.    */

      if (ifptr->next->next->inwidth)
	return;

      elseptr = ifptr->next->next->next->next;
      elsetop = elsextra =
	make_tnode("<statement>",S_STATEMENT);
    }

  unguarded =  (guardptr->rate >= staterate) ||
    (!nospeedtraps(guardptr, guardptr->rate));
  ginc =  currrateunguarded || unguarded;

  currrateunguarded += ginc;

  /* do rate-optimization  */

  /* if or while block */

  tptr = ifptr->down;
  while (tptr != NULL)
    {
      /* revisit later to do slower-rate IF optimizations */

      if (tstate->rate == tptr->rate)
	stateoptrate(staterate, tptr, &xtra, num);
      tptr = tptr->next;
    }

  /* else block */
  
  if (elseptr)
    {
      tptr = elseptr->down;
      while (tptr != NULL)
	{
	  /* revisit later to do slower-rate ELSE optimizations */

	  if (tstate->rate == tptr->rate)
	    stateoptrate(staterate, tptr, &elsextra, num);
	  tptr = tptr->next;
	}
    }

  currrateunguarded -= ginc;

  if (unguarded)
    {
      /* branch taken for all while statements */
      /*    adds to xtra lines, w/o guard      */

      (*lines)->next = top->next;
      while ((*lines)->next != NULL)
	*lines = (*lines)->next;
      if (elsetop)
	{
	  (*lines)->next = elsetop->next;
	  while ((*lines)->next != NULL)
	    *lines = (*lines)->next;
	}
    }
  else
    {
      /* replicate if statement if at least one block has statements*/

      if (top->next || (elsetop && elsetop->next))
	{
	  (*lines)->next = tptr = make_tnode("<statement>",S_STATEMENT);
	  if (staterate == ARATETYPE)
	    tptr->rate = KRATETYPE;
	  else
	    tptr->rate = IRATETYPE;
	  tptr->down = make_tnode("if", S_IF);  
	  tptr = tptr->down;
	  tptr->next = make_tnode("(", S_LP);
	  tptr = tptr->next;
	  tptr->next = eclone(guardptr);
	  tptr = tptr->next;
	  tptr->next = make_tnode(")", S_RP);
	  tptr = tptr->next;
	  tptr->next = make_tnode("{", S_LC);
	  tptr = tptr->next;
	  tptr->next = make_tnode("<block>", S_BLOCK);
	  tptr->next->down = top->next;
	  tptr = tptr->next;
	  tptr->next = make_tnode("}", S_RC);
	  if (elsetop && elsetop->next)
	    {
	      tptr = tptr->next;	  
	      tptr->next = make_tnode("else", S_ELSE);  
	      tptr = tptr->next;
	      tptr->next = make_tnode("{", S_LC);
	      tptr = tptr->next;
	      tptr->next = make_tnode("<block>", S_BLOCK);
	      tptr->next->down = elsetop->next;
	      tptr = tptr->next;
	      tptr->next = make_tnode("}", S_RC);
	    }
	  *lines = (*lines)->next;
	}
    }
  
}


/***********************************************************************/
/*          used in tmapoptrate(), creates                             */
/*                                                                     */
/*                  S_TMAPIDX IDENT1 IDENT2 <expr> ;                   */
/*                                                                     */
/*  IDENT1 is the name of the tablemap                                 */
/*  IDENT2 is the name of the new K_INTERNAL pointer                   */
/*  <expr> is the tablemap index expression                            */
/***********************************************************************/

void tmapfactor(tnode * tptr, tnode ** lines,  int *num)

{
  tnode * eptr = tptr->down->next->next;  /* tmap expression */ 
  tnode * nptr;
  char name[128];

  /* make tablemap statement */

  nptr = make_tnode("<statement>",S_STATEMENT);
  nptr->rate = tptr->rate;
  (*lines)->next = nptr;
  (*lines) = nptr;

  nptr->down = make_tnode("tmapidx", S_TMAPIDX);
  nptr = nptr->down;

  /* first IDENT points to tablemap in symbol table */

  nptr->next = make_tnode(tptr->down->val ,S_IDENT);
  nptr = nptr->next;
  nptr->sptr = tptr->down->sptr;
  nptr->vartype = tptr->down->vartype;

  /* secoond IDENT creates K_INTERNAL table */

  sprintf(name, "_tvr%i", (*num)++);
  nptr->next = make_tnode(dupval(name),S_IDENT);
  nptr = nptr->next;
  nptr->rate = IRATETYPE;
  nptr->width = 1;
  if (addsym(&locsymtable, nptr) != INSTALLED)
    internalerror("optrate.c","error 1 in tmapfactor()");  
  nptr->sptr = locsymtable;
  nptr->vartype = nptr->sptr->vartype = TABLETYPE;
  nptr->sptr->kind = K_INTERNAL;
  nptr->vol = nptr->sptr->vol = VARIABLE;
  vmcheck(nptr->sptr->tref =  (trefer *) calloc(1, sizeof(trefer)));

  /* expression is tablemap indexing expression */

  nptr->next = eclone(eptr);
  nptr = nptr->next;
  nptr->next = make_tnode(";", S_SEM);

  /* change tptr */

  tptr->down = make_tnode(dupval(name),S_IDENT);
  tptr->down->rate = eptr->rate;
  tptr->down->vartype = TABLETYPE;
  tptr->down->sptr = locsymtable;
  tptr->optr = NULL;

}


/************************************************************************/
/* in safeoptrate(): checks expr for user-defined opcodes & speed traps */
/************************************************************************/

int nospeedtraps(tnode * tptr, int subrate)

{
  tnode * eptr = tptr;

  tptr = tptr->down;

  if (tptr->ttype == S_IDENT)
    {
      /* if the expression uses MIDIctrl, and MIDIctrl has */
      /* been written at a rate faster than subexpression  */
      /* rate, then don't do the  rate-optimization. ditto */
      /* params.                                           */

      if ((currinstrument->cref->MIDIctrl > subrate) && 
	  (!strcmp(tptr->val,"MIDIctrl")))
	return 0;

      if ((currinstrument->cref->params > subrate) && 
	  (!strcmp(tptr->val,"params")))
	return 0;

      /* if a variables has been written at a rate faster than   */
      /* the subexpression rate, then don't do rate optimization. */
      /* this code catches table writes as well as array writes   */

      if ((tptr->sptr) && (tptr->sptr->tref->assignrate > subrate))
	return 0;

      /* check restrictions on opcalls and oparrays */

      if ((tptr->next) && ((tptr->next->ttype == S_LP) || 
	      (tptr->next->next->next->next != NULL)))
	{

	  /* user-defined opcodes can't be rate-optimized since     */
	  /* semantics will change. if this is changed later to     */
	  /* aggresively optimize stateless poly-opts, the other    */
	  /* culls in nospeedtraps as well as the culls in saferate */
	  /* must be reconsidered for safety.                       */

	  if (coreopcodename(tptr))
	    {

	      /* don't optimize pitch ops if tuning has been  */
	      /* assigned at a faster rate than subexpression */

	      if ((currinstrument->cref->settune > subrate) &&
		  ((!strcmp(tptr->val,"cpsmidi")) ||
		   (!strcmp(tptr->val,"cpsoct")) ||
		   (!strcmp(tptr->val,"cpspch")) ||
		   (!strcmp(tptr->val,"midicps")) ||
		   (!strcmp(tptr->val,"octcps")) ||
		   (!strcmp(tptr->val,"pchcps")) ||
		   (!strcmp(tptr->val,"gettune")) ||
		   (!strcmp(tptr->val,"settune"))))
		  return 0;

	      /* don't optimize specialops, to maintain relative */
	      /* ordering at the k-pass and a-pass. Probably too */
	      /* conservative for opcalls, needed for oparrays.  */

	      if (coreopcodespecial(tptr))
		return 0;

	      /* if rate-optimized sub-expressions of if and */
	      /* while blocks are executed w/o guards, don't */
	      /* optimize k- and i-rate core opcodes that    */
	      /* change internal state, global state, tables */

	      if (currrateunguarded && coreopcodespeedtrap(tptr))
		return 0;

	      /* optimizing tablereads to i-rate in the startup  */
	      /* instrument is dangerous, because global tables are not */
	      /* created until after the startup's i-cycle runs. */
	      /* Relabeling the tableread to KRATETYPE is dangerous */
	      /* if done here: treeupdate.c is the right place, but */
	      /* would require too much new coding to get right. */

	      if (startupinstr && (currinstrument == startupinstr) &&
		  (!strcmp(tptr->val,"tableread")) && (eptr->rate == IRATETYPE))
		return 0;
	    }
	  else
	    {
	      /* currently, we never rate optimize user-defined  */
	      /* opcode calls, and never rate optimize the lines */
	      /* of user-defined opcodes. revisit to improve.    */
	      
	      return 0;
	    }
	}
    }

  while (tptr != NULL)
    {
      if ((tptr->ttype == S_EXPR) &&
	  (!nospeedtraps(tptr, subrate)))
	return 0;
      if (tptr->ttype == S_EXPRLIST)
	tptr = tptr->down;
      else
	tptr = tptr->next;
    }
  return 1;

}


/***********************************************************************/
/* in saferateopt(): checks if expr contains a particular identifier   */
/***********************************************************************/

int namecheck(tnode * tptr, char * name)

{

  tptr = tptr->down;

  if ((tptr->ttype == S_IDENT)&&(!strcmp(tptr->val,name)))
    return 1;

  while (tptr != NULL)
    {
      if ((tptr->ttype == S_EXPR) && (namecheck(tptr, name)))
	return 1;
      if (tptr->ttype == S_EXPRLIST)
	tptr = tptr->down;
      else
	tptr = tptr->next;
    }
  return 0;

}


/***********************************************************************/
/* in ratefactor(): updates cref->callrate for all opcalls in expr     */
/***********************************************************************/

void updatecallrate(tnode * tptr, int rate)

{

  tptr = tptr->down;

  if ((tptr->ttype == S_IDENT) && (tptr->optr))
    tptr->optr->sptr->cref->callrate = rate;

  while (tptr != NULL)
    {
      if (tptr->ttype == S_EXPR)
	updatecallrate(tptr, rate);
      if (tptr->ttype == S_EXPRLIST)
	tptr = tptr->down;
      else
	tptr = tptr->next;
    }

}









