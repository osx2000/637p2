
/*
#    Sfront, a SAOL to C translator    
#    This file: reference counting for static optimization support
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


extern void assignrefer(sigsym *, tnode *, int);
extern void outbusrefer(sigsym *, tnode *, int);
extern void simplerefer(sigsym *, tnode *, int);
extern void ifrefer(sigsym *, tnode *, int);
extern void whilerefer(sigsym *, tnode *, int);
extern void dynglobals(tnode *, int);
extern  int tokenrefer(sigsym *, tnode *, int);
extern  int unaryrefer(sigsym *, tnode *, int);
extern  int floatcastrefer(sigsym *, tnode *, int);
extern  int parenrefer(sigsym *, tnode *, int);
extern  int multirefer(sigsym *, tnode *, int);
extern  int binaryrefer(sigsym *, tnode *, int);
extern  int switchrefer(sigsym *, tnode *, int);
extern  int opcoderefer(sigsym *, tnode *, int);
extern tnode * rewriteor(tnode * t_one, tnode * t_three, tnode * tptr);
extern tnode * rewriteand(tnode * t_one, tnode * t_three, tnode * tptr);

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*               void staterefer() && void exprrefer()                 */
/*                                                                     */
/* There functions coodinate all static optimization reference         */
/* counting for SAOL statements and stand-alone SAOL exprs.            */
/* These are called from optmain.c. refermirror() is also called       */
/* in optmain.c, and is located at the end of this file.               */
/*_____________________________________________________________________*/


/***********************************************************************/
/*              does reference counts for a statement                  */
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

void staterefer(sigsym * sptr, tnode * tptr, int passtype)

{

  switch(tptr->down->ttype) {
  case S_LVALUE:
    assignrefer(sptr, tptr, passtype);
    break;
  case S_EXTEND:
    /* falls through */
    (sptr->cref->conlines)++;
  case S_OUTPUT:
  case S_SPATIALIZE:
    /* falls through */
    (sptr->cref->syslines)++;
  case S_EXPR:
  case S_RETURN:
  case S_PRINTF:
    simplerefer(sptr, tptr, passtype);
    break;
  case S_IF:
    ifrefer(sptr, tptr, passtype);
    break;
  case S_WHILE:
    whilerefer(sptr, tptr, passtype);
    break;
  case S_INSTR:
    (sptr->cref->syslines)++;
    dynglobals(tptr, passtype);
    simplerefer(sptr, tptr, passtype);
    break;
  case S_TURNOFF:
    (sptr->cref->conlines)++;
    (sptr->cref->syslines)++;
    break;
  case S_OUTBUS:
    (sptr->cref->syslines)++;
    outbusrefer(sptr, tptr, passtype);
    simplerefer(sptr, tptr, passtype);
    break;
  }

}  
    

/***********************************************************************/
/*                 reference counts an expr                            */
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

void exprrefer(sigsym * sptr, tnode * tptr, int passtype)


{
  if (tokenrefer(sptr, tptr, passtype))
    return;
  if (unaryrefer(sptr, tptr, passtype))
    return;
  if (floatcastrefer(sptr, tptr, passtype))
    return;
  if (parenrefer(sptr, tptr, passtype))
    return;
  if (multirefer(sptr, tptr, passtype))
    return;
  if (binaryrefer(sptr, tptr, passtype))
    return;
  if (switchrefer(sptr, tptr, passtype))
    return;
  if (opcoderefer(sptr, tptr, passtype))
    return;

}



/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*                                                                     */
/* The second-level functions called by staterefer(), many of          */
/* of which use exprrefer to handle expressions.                       */
/*                                                                     */
/*_____________________________________________________________________*/


/***********************************************************************/
/*                reference counts assignment statements               */
/*                                                                     */
/* lvalue          : IDENT                                             */
/*                 | IDENT LB expr RB                                  */
/*                                                                     */
/* assignment: lvalue EQ expr SEM                                      */
/***********************************************************************/

void assignrefer(sigsym * sptr, tnode * tptr, int passtype)

{
  
  if (tptr->down->down->next != NULL)  /* lval is an array */
    {
      exprrefer(sptr, tptr->down->down->next->next, passtype); 
    }
  
  exprrefer(sptr, tptr->down->next->next, passtype); 

  if (tptr->down->down->sptr == NULL)
    {
      if (passtype > sptr->cref->MIDIctrl)
	sptr->cref->MIDIctrl = passtype;
      if (passtype > sptr->cref->params)
	sptr->cref->params = passtype;
    }
  else
    {
      /* update variable reference counts */

      (tptr->down->down->sptr->tref->assigntot)++;
      if (whilerefdepth)
	(tptr->down->down->sptr->tref->assignwhile)++;
      if (ifrefdepth)
	(tptr->down->down->sptr->tref->assignif)++;

      if (passtype > tptr->down->down->sptr->tref->assignrate)
	{
	  tptr->down->down->sptr->tref->assignrate = passtype;
	}

      if (tptr->down->down->sptr->kind == K_PFIELD)
	(sptr->cref->callparam)++;	

    }
}

/***********************************************************************/
/*                reference count simple statements                    */
/*                                                                     */
/*                   expr SEM                                          */
/*                   OUTPUT LP exprlist RP SEM                         */
/*                   SPATIALIZE LP exprlist RP SEM                     */
/*                   OUTBUS LP IDENT COM exprlist RP SEM               */
/*                   EXTEND LP expr RP SEM                             */
/*                   RETURN LP exprlist RP SEM                         */
/***********************************************************************/

void simplerefer(sigsym * sptr, tnode * tptr, int passtype)

{
  tnode * lptr;

  lptr = tptr->down;
  while ((lptr->ttype != S_EXPRLIST)&&
	 (lptr->ttype != S_EXPR) &&
	 (lptr->ttype != S_EXPRSTRLIST))
    lptr = lptr->next;
  if (lptr->ttype == S_EXPR)
    {
      exprrefer(sptr, lptr, passtype); 
    }
  else
    {
      lptr = lptr->down;
      while (lptr != NULL)
	{
	  if (lptr->ttype == S_EXPR)
	    {
	      exprrefer(sptr, lptr, passtype); 
	    }
	  lptr = lptr->next;
	}
    }
}


/***********************************************************************/
/*                reference counts if-else statements                  */
/*                                                                     */
/*           (done)| IF LP expr RP LC block RC                         */
/*           (done)| IF LP expr RP LC block RC ELSE LC block RC        */
/***********************************************************************/

void ifrefer(sigsym * sptr, tnode * tptr, int passtype)

{
  tnode * lptr;
  tnode * bptr; 


  lptr = tptr->down;

  if (lptr->inwidth)
    lptr->arrayidx = (sptr->cref->ifstate)++;

  while ((lptr->ttype != S_EXPR))
    lptr = lptr->next;

  exprrefer(sptr, lptr, passtype); 

  ifrefdepth++;
  ifrefglobaldepth++;

  while (lptr != NULL)
    {
      if ((lptr->ttype == S_ELSE) && (lptr->inwidth))
	lptr->arrayidx = (sptr->cref->ifstate)++;

      if (lptr->ttype == S_BLOCK)
	{
	  bptr = lptr->down;
	  while (bptr != NULL)
	    {
	      /* here, we reference count all statements  */
	      /* at the passtype. when we revisit this    */
	      /* code, we should take the statement rate  */
	      /* bptr->rate into account.                 */

	      staterefer(sptr, bptr, passtype);
	      bptr = bptr->next;
	    }
	}
      lptr = lptr->next;
    }

  ifrefdepth--;
  ifrefglobaldepth--;

  return;

}


/***********************************************************************/
/*                reference counts while statements                    */
/*              may be merged into ifrefer() later on                  */
/*                                                                     */
/*           (done)| WHILE LP expr RP LC block RC                      */
/***********************************************************************/

void whilerefer(sigsym * sptr, tnode * tptr, int passtype)

{
  tnode * lptr;
  tnode * bptr; 


  lptr = tptr->down;
  while ((lptr->ttype != S_EXPR))
    lptr = lptr->next;

  exprrefer(sptr, lptr, passtype); 

  whilerefdepth++;
  whilerefglobaldepth++;

  while (lptr != NULL)
    {
      if (lptr->ttype == S_BLOCK)
	{
	  bptr = lptr->down;
	  while (bptr != NULL)
	    {
	      /* overly cautious to check rate, but later we may */
	      /* run optrefer again after optimizatons ...       */

	      if (bptr->rate == passtype)
		{
		  staterefer(sptr, bptr, passtype);
		}
	      bptr = bptr->next;
	    }
	}
      lptr = lptr->next;
    }

  whilerefdepth--;
  whilerefglobaldepth--;

  return;

}


extern void dynglobalupdate(sigsym *);


/***********************************************************************/
/*               reference counts dynamic instr globals                */
/*                                                                     */
/*                   INSTR IDENT LP exprlist RP SEM                    */
/*                                                                     */
/***********************************************************************/


void dynglobals(tnode * tptr, int passtype)

{

  /* update global tables dyninstr flag if i-rate instr */

  if ((passtype == IRATETYPE) && tptr->sptr) 
    dynglobalupdate(tptr->sptr);

}

/***********************************************************************/
/*            updates bus mirroring for outbus() statement             */
/*                                                                     */
/*                OUTBUS LP IDENT COM exprlist RP SEM                  */
/***********************************************************************/

void outbusrefer(sigsym * sptr, tnode * tptr, int passtype)

{
  tnode * iptr;
  sigsym * bptr;


  /* mirror if outbus writes a bus the instance is sent */

  if (currinstance && (passtype == ARATETYPE))
    {
      bptr = getvsym(&busnametable, tptr->down->next->next->val);

      iptr = currinstance->ibus;   /* cycle through input[] buses */
      while (iptr)
	{
	  sptr->cref->inmirror |= (bptr == iptr->sptr);
	  iptr = iptr->next;
	}

      /* update global flag for shadowbus usage */

      useshadowbus |= sptr->cref->inmirror;
    }
}


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*                                                                     */
/* The second-level functions called by exprrefer().                   */
/*                                                                     */
/*_____________________________________________________________________*/


/***********************************************************************/
/*              reference counts S_IDENT and constants                 */      
/***********************************************************************/

int tokenrefer(sigsym * sptr, tnode * tptr, int passtype)

{
  if ((tptr->down->ttype == S_NUMBER) ||
      (tptr->down->ttype == S_INTGR)  ||
      ((tptr->down->ttype == S_IDENT) &&
       (standardname(tptr->down) ||
	tptr->down->sptr != NULL)))
    {

      switch (tptr->down->ttype) {
      case S_IDENT:
	if (tptr->down->next == NULL)
	  {
	    if (standardname(tptr->down))
	      {
		/* standard name token actions */

		if (!(strcmp(tptr->down->val,"dur")))
		  {
		    if (passtype == IRATETYPE)
		      (sptr->cref->idur)++;
		    else
		      (sptr->cref->kadur)++;
		  }
		if (!(strcmp(tptr->down->val,"itime")))
		  (sptr->cref->itime)++;
	      }
	    else
	      {
		/* unindexed variable actions */

		/* check for state variables */

		if (((tptr->down->sptr->kind == K_NORMAL) ||
		     (tptr->down->sptr->kind == K_EXPORT))
		    && (tptr->down->sptr->tref->assigntot == 0))
		  {
		    (tptr->down->sptr->tref->varstate)++;
		    (sptr->cref->statevars)++;
		  }

		/* access reference counting */
		/* note this is conservative for user-defined      */
		/* opcodes, since a variable may be passed into    */
		/* an opcode w/o actually being accessed inside it */

		(tptr->down->sptr->tref->accesstot)++;
		if (passtype > tptr->down->sptr->tref->accessrate)
		  tptr->down->sptr->tref->accessrate = passtype;

	      }
	  }
	else
	  {
	    /* indexed variables go here */

	    /* process index */

	    exprrefer(sptr, tptr->down->next->next, passtype);

	    /* process variable itself */

	    if (standardname(tptr->down))
	      {
		/* standard name token actions */
	      }
	    else
	      {
		/* indexed variable actions */

		/* check for state variables */

		if (((tptr->down->sptr->kind == K_NORMAL) ||
		     (tptr->down->sptr->kind == K_EXPORT))
		    && (tptr->down->sptr->tref->assigntot == 0))
		  {
		    (tptr->down->sptr->tref->varstate)++;
		    (sptr->cref->statevars)++;
		  }

		/* access reference counting */

		(tptr->down->sptr->tref->accesstot)++;
		if (passtype > tptr->down->sptr->tref->accessrate)
		  tptr->down->sptr->tref->accessrate = passtype;

	      }
	  }
	break;
      case S_NUMBER:
      case S_INTGR:
	break;
      }
      return 1;
    }
  else
    return 0;
}


/***********************************************************************/
/*                reference counts unary <expr>                        */      
/***********************************************************************/

int unaryrefer(sigsym * sptr, tnode * tptr, int passtype)

{
  tnode * t_one;
  tnode * t_two;


  if (((t_one = tptr->down) == NULL) || ((t_two = tptr->down->next) == NULL) ||
      (tptr->down->next->next != NULL))
    return 0;

  if ((t_one->ttype != S_MINUS) && (t_one->ttype != S_NOT)) 
    return 0;
  if (t_two->ttype != S_EXPR)
    return 0;

  exprrefer(sptr, t_two, passtype);

  return 1;
}


/***********************************************************************/
/*                reference counts floatcast expr                      */      
/***********************************************************************/

int floatcastrefer(sigsym * sptr, tnode * tptr, int passtype)

{
  tnode * t_three;

  if ((tptr->down == NULL) || (tptr->down->ttype != S_FLOATCAST))
    return 0;

  t_three = tptr->down->next->next;

  if ((tptr->down->next->next->next == NULL) ||
      (tptr->down->next->next->next->next != NULL))
    return 0;

  exprrefer(sptr, t_three, passtype);

  return 1;
}


/***********************************************************************/
/*              reference counts in (<expr>)                           */      
/***********************************************************************/

int parenrefer(sigsym * sptr, tnode * tptr, int passtype)

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

  exprrefer(sptr, t_two, passtype);

  return 1;
}

/***********************************************************************/
/*          reference counts in multi-operator *, +, -              */      
/***********************************************************************/

int multirefer(sigsym * sptr, tnode * tptr, int passtype)

{
  tnode * eptr;

  if ((tptr->down == NULL) || (tptr->down->ttype != S_EXPR) ||
      (tptr->down->next == NULL) ||
      ( (tptr->down->next->ttype != S_STAR) &&
	(tptr->down->next->ttype != S_PLUS) &&
	(tptr->down->next->ttype != S_MINUS)))
    return 0;

  eptr = tptr->down;

  while (eptr != NULL)
    {
      if (eptr->ttype == S_EXPR)
	{
	  exprrefer(sptr, eptr, passtype);
	}
      eptr = eptr->next;
    }

  return 1;
}


/***********************************************************************/
/*         reference counts binary <expr> (except for +,-,*)           */      
/***********************************************************************/

int binaryrefer(sigsym * sptr, tnode * tptr, int passtype)

{
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

  exprrefer(sptr, t_one, passtype);
  exprrefer(sptr, t_three, passtype);

  if ((tptr->width > 1) && ((t_two->ttype == S_AND) || (t_two->ttype == S_OR)))
    {
      if (t_two->ttype == S_AND)
	tptr->down = rewriteand(t_one, t_three, tptr);
      if (t_two->ttype == S_OR)
	tptr->down = rewriteor(t_one, t_three, tptr);
    }


  return 1;

}

/***********************************************************************/
/*            reference counts switch statement                        */      
/***********************************************************************/

int switchrefer(sigsym * sptr, tnode * tptr, int passtype)

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

  exprrefer(sptr, t_one, passtype);
  exprrefer(sptr, t_three, passtype);
  exprrefer(sptr, t_five, passtype);

  return 1;

}


extern void userdefrefer(sigsym *, tnode *, int);

/***********************************************************************/
/*           reference counts opcode/oparray calls                     */
/*                                                                     */
/*                 IDENT LP exprlist RP                                */
/*                 IDENT LB expr RB LP exprlist RP                     */
/***********************************************************************/

int opcoderefer(sigsym * sptr, tnode * tptr, int passtype)

{
  int core, oparraystack;
  tnode * t_one;
  tnode * t_two;
  tnode * t_three;
  tnode * t_four;
  tnode * iptr;

  if (((t_one = tptr->down) == NULL) || ((t_two = tptr->down->next) == NULL) ||
      ((t_three = tptr->down->next->next) == NULL) ||
      ((t_four = tptr->down->next->next->next) == NULL))
    return 0;

  if ((t_one->ttype != S_IDENT) || ((t_two->ttype != S_LP) &&
				    (t_two->ttype != S_LB)))
    return 0;


  tptr->down->optr->sptr->cref->callrate = passtype;
  tptr->down->optr->sptr->cref->callif = ifrefglobaldepth;
  tptr->down->optr->sptr->cref->callwhile = whilerefglobaldepth;

  core = coreopcodename(t_one);
  oparraystack = curroparraydepth;

  if (t_two->ttype == S_LB)
    {
      exprrefer(sptr, t_three, passtype);
      t_three = t_four->next->next;
      if ((++curroparraydepth) > maxoparraydepth)
	maxoparraydepth = curroparraydepth;
    }

  if ((t_three->ttype != S_EXPRLIST))
    internalerror("optrefer.c","opcoderefer -- no S_EXPRLIST");

  iptr = t_three->down;

  while (iptr != NULL)
    {
      if (iptr->ttype == S_EXPR)
	{
	  exprrefer(sptr, iptr, passtype);
	}
      iptr = iptr->next;
    }

  if (core)
    corerefer(sptr, t_one, t_three->down, passtype);
  else
    userdefrefer(sptr, tptr, passtype);

  curroparraydepth = oparraystack;
  return 1;

}



/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*                                                                     */
/* Utility functions, used by second level refercount calls.           */
/*                                                                     */
/*_____________________________________________________________________*/


/***********************************************************************/
/*  updates dyninstr flag on imported i-rate scalars in globals        */
/*  called by dynglobals(), which is called in staterefer().           */
/***********************************************************************/

void dynglobalupdate(sigsym * iptr)

{
  sigsym * sptr;
  sigsym * gptr;
  tnode * tptr;

  sptr = iptr->defnode->sptr;
  while (sptr != NULL)
    {
      if ((sptr->rate == IRATETYPE) && 
	  ((sptr->vartype == SCALARTYPE) ||
	   (sptr->vartype == VECTORTYPE)) && 
	  ((sptr->kind == K_EXPORT) ||
	   (sptr->kind == K_IMPORT) ||
	   (sptr->kind == K_IMPORTEXPORT)) &&
	  (gptr = getvsym(&globalsymtable, sptr->val)))
	{
	  gptr->tref->dynaccess++;
	}
      sptr = sptr->next;
    }

  tptr = iptr->defnode->optr;
  while (tptr != NULL)
    {
      if (tptr->ttype != S_OPARRAYDECL)
	dynglobalupdate(tptr->sptr);
      tptr = tptr->next;
    }

}


extern void tmaprefer(sigsym *, int, int);

/***********************************************************************/
/* reference counts user-define opcode call, called by opcoderefer()   */
/*                                                                     */
/*      optype IDENT LP paramlist RP LC opvardecls block RC            */
/*                                                                     */
/*                 IDENT LP exprlist RP                                */
/*                 IDENT LB expr RB LP exprlist RP                     */
/***********************************************************************/

void userdefrefer(sigsym * sptr, tnode * tptr, int passtype)

{
  sigsym * s2ptr;
  tnode * cbrptr = NULL;
  tnode * iptr, * i2ptr;
  int ifstack;
  int whilestack;
  int numassign, numalias;

  s2ptr = tptr->down->optr->sptr;
  iptr = tptr->down->optr->sptr->defnode->down->next->next->next
    ->next->next->next->next->down;


  /* do line-by-line reference count of opcode call */

  ifstack = ifrefdepth;
  whilestack = whilerefdepth;

  while (iptr != NULL)
    {

      /* referencing-counting each line using the   */
      /* passtype. when we revisit this issue, take */
      /* into account the line rate (iptr->rate).   */

      ifrefdepth = whilerefdepth = 0;
      staterefer(s2ptr, iptr, passtype);

      iptr = iptr->next;
    }

  ifrefdepth = ifstack;
  whilerefdepth = whilestack;


  /* update call-by-ref changes to sptr symbol table */

  s2ptr = tptr->down->optr->sptr->defnode->sptr;
  if (tptr->down->next->ttype == S_LP)
    iptr = tptr->down->next->next->down;
  else
    iptr = tptr->down->next->next->next->next->next->down;


  while ((s2ptr != NULL) && (s2ptr->kind == K_PFIELD))
    {
      while (iptr->ttype != S_EXPR)
	iptr = iptr->next;

      /* handle variables written to */

      if ((iptr->down->ttype == S_IDENT) && 
	  (iptr->down->sptr != NULL) &&
	  (s2ptr->tref->assigntot))
	{
	  (iptr->down->sptr->tref->assigntot)++;
	  if (s2ptr->tref->assigntval)
	    (iptr->down->sptr->tref->assigntval)++;
	  (iptr->down->sptr->tref->assignbyref)++;
	  if (whilerefdepth)
	    (iptr->down->sptr->tref->assignwhile)++;
	  if (ifrefdepth)
	    (iptr->down->sptr->tref->assignif)++;
	  if (passtype > iptr->down->sptr->tref->assignrate)
	    {
	      iptr->down->sptr->tref->assignrate = passtype; 
	    }
	  if (iptr->down->sptr->kind == K_PFIELD)
	    (sptr->cref->callparam)++;	

	  /* note assignbyref not set for tmap elements */

	  tmaprefer(iptr->down->sptr, (s2ptr->tref->assigntval) ?
		    TVALCHANGE : TPARAMCHANGE, passtype);
	}

      /* handle writing to MIDIctrl and params */

      if ((s2ptr->tref->assigntot) && (iptr->down->ttype == S_IDENT))
	{
	  if ( (!strcmp(iptr->down->val,"MIDIctrl")) && 
	       (passtype > sptr->cref->MIDIctrl))
	    sptr->cref->MIDIctrl = passtype;
	  if ( (!strcmp(iptr->down->val,"params")) && 
	       (passtype > sptr->cref->params))
	    sptr->cref->params = passtype;
	}

      /* set mirroring flag in s2ptr. Note that we don't mirror */
      /* indexed array arguments, since there's no real gain    */

      if (((iptr->down->ttype == S_IDENT) &&
	   (iptr->down->sptr != NULL) &&
	   (iptr->down->next == NULL) && 
	   ((s2ptr->vartype == SCALARTYPE) ||
	    (s2ptr->vartype == VECTORTYPE))))
	{

	  s2ptr->tref->mirror = OPCODEMIRROR;

	  /* collect all call-by-refs for aliasing check */

	  i2ptr = cbrptr;
	  cbrptr = make_tnode("", S_IDENT);
	  cbrptr->next = i2ptr;

	  cbrptr->sptr = s2ptr;         /* formal param */
	  cbrptr->down = iptr->down;    /* actual param */
	}

      s2ptr= s2ptr->next;
      iptr = iptr->next;
    }

  /* check for "same argument passed twice" aliasing */
  /* thanks to daveg [at] synaptics [dot] com        */

  while (cbrptr)
    {
      numassign = cbrptr->sptr->tref->assigntot;
      numalias = 0;

      /* see if at least one alias happens, and */
      /* if at least one assignment made.       */

      iptr = cbrptr->next;
      while (iptr)
	{
	  if (cbrptr->down->sptr == iptr->down->sptr)
	    {
	      numalias = 1;
	      if (iptr->sptr->tref->assigntot)
		{
		  numassign = 1;
		  break;
		}
	    }
	  iptr = iptr->next;
	}

      /* if alias exist, delete them, after clearing */
      /* mirror flag if at least on assignment made  */
      
      if (numalias)
	{
	  iptr = cbrptr;
	  while (iptr && iptr->next)
	    {
	      if (cbrptr->down->sptr == iptr->next->down->sptr)
		{
		  if (numassign)
		    {
		      iptr->next->sptr->tref->mirror = REQUIRED;
		    }
		  i2ptr = iptr->next;
		  iptr->next = iptr->next->next;
		  free(i2ptr);
		}
	      iptr = iptr->next;
	    }
	}

      i2ptr = cbrptr;
      cbrptr = cbrptr->next;
      free(i2ptr);
    }

  /* update relevent reference counts from opcode */

  s2ptr = tptr->down->optr->sptr;

  if (s2ptr->cref->MIDIctrl > sptr->cref->MIDIctrl)
    sptr->cref->MIDIctrl = s2ptr->cref->MIDIctrl;

  if (s2ptr->cref->params > sptr->cref->params)
    sptr->cref->params = s2ptr->cref->params;

  if (s2ptr->cref->settune > sptr->cref->settune)
    sptr->cref->settune = s2ptr->cref->settune;

  sptr->cref->kadur += s2ptr->cref->kadur;
  sptr->cref->idur += s2ptr->cref->idur;
  sptr->cref->itime += s2ptr->cref->itime;
  sptr->cref->statevars += s2ptr->cref->statevars;
  sptr->cref->statewave += s2ptr->cref->statewave;
  sptr->cref->syslines += s2ptr->cref->syslines;
  sptr->cref->conlines += s2ptr->cref->conlines;

}


/***********************************************************************/
/* reference counts tables in a tablemap, used above in userdefrefer() */      
/***********************************************************************/

void tmaprefer(sigsym * sptr, int tabvals, int rate)

{
  tnode * tptr;

  if (sptr->vartype != TMAPTYPE)
    return;

  tptr = sptr->defnode->down->next->next->next->down;
  while (tptr != NULL)
    {
      if (tptr->ttype == S_IDENT)
	{
	  (tptr->sptr->tref->assigntot)++;
	  if (whilerefdepth)
	    (tptr->sptr->tref->assignwhile)++;
	  if (ifrefdepth)
	    (tptr->sptr->tref->assignif)++;
	  if (tabvals == TVALCHANGE)
	    (tptr->sptr->tref->assigntval)++;
	  if (rate > tptr->sptr->tref->assignrate)
	    tptr->sptr->tref->assignrate = rate;
	}
      tptr = tptr->next;
    }
}


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*                     void refermirror()                              */
/*                                                                     */
/* This function is also called in optmain.c, as the last stage of     */
/* a multi-pass algorithm to find imports/exports mirror variables.    */
/*                                                                     */
/*_____________________________________________________________________*/

/***********************************************************************/
/* sets mirror flag for imports/exports signals, called in optmain.c  */      
/***********************************************************************/

void refermirror(sigsym * sptr)

{  
  sigsym * gptr;


  /* only need to calculate mirror once */

  if (sptr->tref->mirror)
    return;

  /* only imports and/or exports with a paired global */

  if (((sptr->kind != K_IMPORT) && (sptr->kind != K_IMPORTEXPORT) &&
       (sptr->kind != K_EXPORT)) || 
      ((sptr->vartype != VECTORTYPE) && (sptr->vartype != SCALARTYPE)) ||
      (!(gptr = getvsym(&globalsymtable, sptr->val))))
    return;

  /* k-rate imports and/or exports */

  if (sptr->rate == KRATETYPE)
    {
      if (sptr->kind == K_IMPORT)
	{
	  /* conditions for mirroring a k imports signal variable */

	  if ((sptr->tref->assigntot == 0) &&
	      (sptr->tref->totexport == 0) && 
	      ((sptr->tref->accessrate <= KRATETYPE) ||
	       (currinstrument && (gptr->tref->finalinstr < 
				   currinstrument->vol))))
	    sptr->tref->mirror = GLOBALMIRROR;
	}
      else
	{
	  /* conditions for mirroring a k exports signal variable */

	  if (sptr->kind == K_IMPORTEXPORT)
	    {
	      /* if sptr->tref->accesstot, we added 1 to totimport  */
	      /* if sptr->tref->assigntot, we added 1 to totexport   */

	      if ( (!(sptr->tref->assigntot || sptr->tref->accesstot)) || 
		  ((sptr->tref->totimport + !(sptr->tref->accesstot) <= 1) &&
		   (sptr->tref->totexport + !(sptr->tref->assigntot) <= 1) &&
		   (sptr->tref->assignrate <= KRATETYPE) &&
		  ((sptr->tref->accessrate <= KRATETYPE) ||
		   (currinstrument && (gptr->tref->finalinstr <= 
				       currinstrument->vol)))))
		sptr->tref->mirror = GLOBALMIRROR;
	    }
	  else
	    {
	      /* the exports case -- we always added 1 to totexport */
	      /* and never added 1 to totimport                     */

	      if ((sptr->tref->totimport == 0) &&
		  (sptr->tref->totexport <= 1) && 
		  (sptr->tref->accessrate <= KRATETYPE) &&
        	  (sptr->tref->assignrate <= KRATETYPE) &&
		  (sptr->tref->varstate == 0) && 
		  (sptr->tref->assigntot > 0))
		sptr->tref->mirror = GLOBALMIRROR;
	    }

	}
      return;
    }

  /* i-rate imports and/or exports */

  if (sptr->kind == K_IMPORT)
    {
      if ((sptr->tref->assigntot == 0) &&
	  (sptr->tref->totexport == 0) && 
	  (sptr->tref->accessrate <= IRATETYPE) &&
	  (sptr->tref->dynaccess == 0))
	sptr->tref->mirror = GLOBALMIRROR;
    }
  else
    {

      if (sptr->kind == K_IMPORTEXPORT)
	{
	  /* if sptr->tref->accesstot, we added 1 to totimport  */
	  /* if sptr->tref->assigntot, we added 1 to totexport   */
	  
	  if ( (!(sptr->tref->assigntot || sptr->tref->accesstot)) || 
	       ((sptr->tref->totimport + !(sptr->tref->accesstot) <= 1) &&
		(sptr->tref->totexport + !(sptr->tref->assigntot) <= 1) && 
		(sptr->tref->accessrate <= IRATETYPE) &&
		(sptr->tref->assignrate <= IRATETYPE) &&
		(sptr->tref->dynaccess == 0)))

	       sptr->tref->mirror = GLOBALMIRROR;
	}
      else
	{
	  /* the exports case -- we always added 1 to totexport */
	  /* and never added 1 to totimport                     */
	  
	  if ((sptr->tref->totimport == 0) &&
	      (sptr->tref->totexport <= 1) && 
	      (sptr->tref->accessrate <= IRATETYPE) &&
	      (sptr->tref->assignrate <= IRATETYPE) &&
	      (sptr->tref->assigntot > 0) && 
	      (sptr->tref->dynaccess == 0))
	    sptr->tref->mirror = GLOBALMIRROR;

	}

    }

  return;
  
}

extern void transfer_tnode(tnode * t_to, tnode * t_from);

/***********************************************************************/
/*                rewrites && for the vector case                      */      
/***********************************************************************/

tnode * rewriteand(tnode * t_one, tnode * t_three, tnode * tptr)

{
  tnode * t_p1, * t_n1, * t_p3, * t_n3;
  tnode * t_sum, * t_parens, * t_negate, * t_top;

  /***************************/
  /* generate t_one negation */
  /***************************/

  t_p1 = make_stree(make_tnode("(", S_LP), t_one, make_tnode(")", S_RP),
		    NULL, "<expr>", S_EXPR);

  transfer_tnode(t_p1, t_one);

  t_n1 = make_stree(make_tnode("!", S_NOT), t_p1, NULL, NULL, 
		    "<expr>", S_EXPR);

  transfer_tnode(t_n1, t_one);

  t_n1->res = ASINT;

  /*****************************/
  /* generate t_three negation */
  /*****************************/

  t_p3 = make_stree(make_tnode("(",S_LP), t_three, make_tnode(")",S_RP),
		    NULL, "<expr>", S_EXPR);

  transfer_tnode(t_p3, t_three);

  t_n3 = make_stree(make_tnode("!",S_NOT), t_p3, NULL, NULL, "<expr>", S_EXPR);

  transfer_tnode(t_n3, t_three);

  t_n3->res = ASINT;

  /********************/
  /* create summation */
  /********************/

  t_sum = make_stree(t_n1, make_tnode("+",S_PLUS), t_n3, NULL, "<expr>",
		     S_EXPR);

  transfer_tnode(t_sum, tptr);

  /***************/
  /* do negation */
  /***************/

  t_parens = make_stree(make_tnode("(", S_LP), t_sum, make_tnode(")", S_RP),
			 NULL, "<expr>", S_EXPR);

  transfer_tnode(t_parens, tptr);

  t_negate = make_stree(make_tnode("!",S_NOT), t_parens, 
			NULL, NULL, "<expr>", S_EXPR);

  transfer_tnode(t_negate, tptr);

  /******************/
  /* wrap in parens */
  /******************/

  t_top = make_tnode("(",S_LP);
  t_top->next = t_negate;
  t_top->next->next = make_tnode(")",S_RP);

  return t_top;

}


/***********************************************************************/
/*                rewrites || for the vector case                      */      
/***********************************************************************/

tnode * rewriteor(tnode * t_one, tnode * t_three, tnode * tptr)

{
  tnode * t_p1, * t_n1, * t_p3, * t_n3;
  tnode * t_sum, * t_parens, * t_negate, * t_top;

  /**********************************/
  /* generate t_one double negation */
  /**********************************/

  t_p1 = make_stree(make_tnode("(", S_LP), t_one, make_tnode(")", S_RP),
		    NULL, "<expr>", S_EXPR);

  transfer_tnode(t_p1, t_one);

  t_n1 = make_stree(make_tnode("!", S_NOT), t_p1, NULL, NULL, 
		    "<expr>", S_EXPR);

  transfer_tnode(t_n1, t_one);

  t_n1->res = ASINT;

  t_p1 = make_stree(make_tnode("(", S_LP), t_n1, make_tnode(")", S_RP),
		    NULL, "<expr>", S_EXPR);

  transfer_tnode(t_p1, t_one);

  t_n1 = make_stree(make_tnode("!", S_NOT), t_p1, NULL, NULL, 
		    "<expr>", S_EXPR);

  transfer_tnode(t_n1, t_one);

  t_n1->res = ASINT;

  /*****************************/
  /* generate t_three negation */
  /*****************************/

  t_p3 = make_stree(make_tnode("(",S_LP), t_three, make_tnode(")",S_RP),
		    NULL, "<expr>", S_EXPR);

  transfer_tnode(t_p3, t_three);

  t_n3 = make_stree(make_tnode("!",S_NOT), t_p3, NULL, NULL, "<expr>", S_EXPR);

  transfer_tnode(t_n3, t_three);

  t_n3->res = ASINT;

  t_p3 = make_stree(make_tnode("(",S_LP), t_n3, make_tnode(")",S_RP),
		    NULL, "<expr>", S_EXPR);

  transfer_tnode(t_p3, t_three);

  t_n3 = make_stree(make_tnode("!",S_NOT), t_p3, NULL, NULL, "<expr>", S_EXPR);

  transfer_tnode(t_n3, t_three);

  t_n3->res = ASINT;

  /********************/
  /* create summation */
  /********************/

  t_sum = make_stree(t_n1, make_tnode("+",S_PLUS), t_n3, NULL, "<expr>",
		     S_EXPR);

  transfer_tnode(t_sum, tptr);

  /**********************/
  /* do double negation */
  /**********************/

  t_parens = make_stree(make_tnode("(", S_LP), t_sum, make_tnode(")", S_RP),
			 NULL, "<expr>", S_EXPR);

  transfer_tnode(t_parens, tptr);

  t_negate = make_stree(make_tnode("!",S_NOT), t_parens, 
			NULL, NULL, "<expr>", S_EXPR);

  transfer_tnode(t_negate, tptr);

  t_parens = make_stree(make_tnode("(", S_LP), t_negate, make_tnode(")", S_RP),
			 NULL, "<expr>", S_EXPR);

  transfer_tnode(t_parens, tptr);

  t_negate = make_stree(make_tnode("!",S_NOT), t_parens, 
			NULL, NULL, "<expr>", S_EXPR);

  transfer_tnode(t_negate, tptr);

  /******************/
  /* wrap in parens */
  /******************/

  t_top = make_tnode("(",S_LP);
  t_top->next = t_negate;
  t_top->next->next = make_tnode(")",S_RP);

  return t_top;

}

/**********************************************************/
/*  copies selected tnode state -- used by rewritelogic() */
/*********************************************************/

void transfer_tnode(tnode * t_to, tnode * t_from)

{

  t_to->rate = t_from->rate;
  t_to->special = t_from->special;
  t_to->width = t_from->width;
  t_to->res = t_from->res;
  t_to->vartype = t_from->vartype;
  t_to->vol = t_from->vol;

}
