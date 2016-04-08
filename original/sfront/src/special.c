
/*
#    Sfront, a SAOL to C translator    
#    This file: Handles specialops
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


/************************************************************/
/*         special check for two tnodes in an <expr>        */
/************************************************************/

int exprspecial(tnode * left, tnode * right)

{
  return (left->special || right->special);
}

/************************************************************/
/*         special check for an exprlist                    */
/************************************************************/

int exprlistspecial(tnode * elist)

{
  int sp = 0;

  elist = elist->down;
  while (elist != NULL)
    {
      sp = sp || elist->special;
      elist = elist->next;
    }
  return sp;

}

/************************************************************/
/* computes special flag for a list of statements           */
/************************************************************/

int blockspecial(tnode * block)

{

  int sp = 0;

  while (block != NULL) 
    {
      sp = sp || block->special;
      block = block->next;
    }
  return sp;

}

/****************************************************************/
/*           updates special flag in parse tree                 */
/****************************************************************/

int specialupdate(tnode * tbranch)

{

  tnode * tptr = tbranch;
  tnode * cptr;
  int sp = 0;

  while (tptr != NULL)
    {
      specialupdate(tptr->down);
      switch (tptr->ttype) {
      case S_LVALUE:
	if (tptr->down->next == NULL)         /* unindexed */
	  {
	  }
	else                                  /* indexed */
	  {
	    tptr->special = tptr->down->next->next->special;
	  }
	break;
      case S_STATEMENT:
	if (tptr->down->next->ttype == S_SEM)
	  {
	    if (tptr->down->ttype == S_EXPR)     /* expr SEM */
	      {
		tptr->special = tptr->down->special;
	      }
	    else                                 /* turnoff SEM */
	      {
		
	      }
	    break;
	  }
	if (tptr->down->ttype == S_LVALUE) /* lvalue EQ expr SEM */ 
	  {
	    tptr->special = exprspecial(tptr->down,tptr->down->next->next);
	    break;
	  }
	if (tptr->down->ttype == S_OUTPUT)
	  {
	    tptr->special = exprlistspecial(tptr->down->next->next);
	    break;
	  }
	if (tptr->down->ttype == S_OUTBUS)
	  {
	    tptr->special = exprlistspecial(tptr->down->next->next->next->next);
	    break;
	  }
	if (tptr->down->ttype == S_INSTR)
	  {
	    tptr->special = exprlistspecial(tptr->down->next->next->next);
	    break;
	  }
	if (tptr->down->ttype == S_RETURN)
	  {
	    tptr->special = exprlistspecial(tptr->down->next->next);
	    break;
	  }
	if (tptr->down->ttype == S_PRINTF)
	  {
	    tptr->special = exprlistspecial(tptr->down->next->next);
	    break;
	  }
	if (tptr->down->ttype == S_IF)
	  {
	    cptr = tptr->down->next->next->next->next->next;
	    tptr->special = tptr->down->next->next->special;
	    if (!(tptr->special) && blockspecial(cptr->down))
	      {
		printf("Error: If block has specialops, guard doesn't.\n");
		showerrorplace(tptr->down->linenum, tptr->down->filename);
	      }
	    if (cptr->next->next != NULL)
	      {
		cptr = cptr->next->next->next->next;
		if (!(tptr->special) && blockspecial(cptr->down))
		  {
		    printf("Error: Else block has specialops, ");
		    printf("guard doesn't.\n");
		    showerrorplace(tptr->down->linenum, tptr->down->filename);

		  }
	      }
	    break;
	  }
	if (tptr->down->ttype == S_WHILE)
	  {
	    cptr = tptr->down->next->next->next->next->next;
	    tptr->special = tptr->down->next->next->special;
	    if ((tptr->special) || (blockspecial(cptr->down)))
	      {
		printf("Error: While guard or block may not be specialop.\n");
		showerrorplace(tptr->down->linenum, tptr->down->filename);
	      }
	    break;
	  }
	break;
      case S_EXPR:
	if (tptr->down->next == NULL)          
	  {
	    if (tptr->down->ttype == S_IDENT)     /* ident */
	      {
	      }
	    else
	      {                                  /* constant */
	      }
	    break;
	  }
	if ((tptr->down->ttype == S_MINUS) ||
	    (tptr->down->ttype == S_NOT))      /* unary */
	  {
	    tptr->special = tptr->down->next->special;
	    break;
	  }
	if (tptr->down->ttype == S_LP) /* works for float->into to */
	  {
	    tptr->special = tptr->down->next->special;
	    break;
	  }
	if (tptr->down->ttype == S_FLOATCAST) 
	  {
	    tptr->special = tptr->down->next->next->special;
	    break;
	  }
	if ((tptr->down->next->ttype == S_LEQ)   ||
	    (tptr->down->next->ttype == S_GEQ)   ||
	    (tptr->down->next->ttype == S_NEQ)   ||
	    (tptr->down->next->ttype == S_EQEQ)  ||
	    (tptr->down->next->ttype == S_GT)    ||
	    (tptr->down->next->ttype == S_LT)    ||
	    (tptr->down->next->ttype == S_AND)   ||
	    (tptr->down->next->ttype == S_OR)    ||
	    (tptr->down->next->ttype == S_SLASH)  )
	  {
	    tptr->special = exprspecial(tptr->down, tptr->down->next->next);
	    break;
	  }
	if ((tptr->down->next->ttype == S_PLUS)  ||
	    (tptr->down->next->ttype == S_MINUS) ||
	    (tptr->down->next->ttype == S_STAR)  )
	  {
	    cptr = tptr->down;
	    tptr->special = 0;
	    while (cptr != NULL)
	      {
		if (cptr->ttype == S_EXPR)
		  tptr->special = tptr->special || cptr->special;
		cptr = cptr->next;
	      }
	    break;
	  }
	if (tptr->down->next->ttype == S_Q)
	  {
	    tptr->special = exprspecial(tptr->down,
					tptr->down->next->next);
	    tptr->special = exprspecial(tptr,
					tptr->down->next->next->next->next);
	    break;
	  }
	if ((tptr->down->next->ttype == S_LB) &&         /*array index*/
	    (tptr->down->next->next->next->next == NULL))
	  {
	    tptr->special = tptr->down->next->next->special;
	    break;
	  }
	if ((tptr->optr != NULL))   /* opcode and oparray */
	  {
	    if (coreopcodename(tptr->down))
	      tptr->special = tptr->down->special = tptr->optr->special =
		tptr->optr->sptr->special = coreopcodespecial(tptr->down);
	    else
	      {
		/* user-defined opcodes -- process the opcode lines,   */
		/* don't set the user-defined opcode special flag, so  */
		/* the semantics of specialops are hidden below.       */

		specialupdate(tptr->optr->sptr->defnode->down->
			      next->next->next->next->next->next->next->down);

	      }
	    if (tptr->optr->ttype == S_OPCALL)
	      cptr = tptr->down->next->next->down;
	    else
	      cptr = tptr->down->next->next->next->next->next->down;
	    while (cptr != NULL) 
	      {
		if (cptr->ttype == S_EXPR)
		  {
		    tptr->special = tptr->special || cptr->special;
		  }
		cptr = cptr->next;
	      }
	    break;
	  }
	break;
      }
      sp = sp || tptr->special;
      tptr = tptr->next;
    }
  return sp;
}



