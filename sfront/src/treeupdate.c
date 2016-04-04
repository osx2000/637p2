
/*
#    Sfront, a SAOL to C translator    
#    This file: Width and rate type checking
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

/************************************/
/* returns string for rate          */
/************************************/

char * ratetostr(int rate)

{
  switch (rate) {
  case UNKNOWN: 
    return dupval("(unknown)");
  case IRATETYPE: 
    return dupval("i-rate");
  case KRATETYPE: 
    return dupval("k-rate");
  case ARATETYPE: 
    return dupval("a-rate");
  case XRATETYPE: 
    return dupval("poly-rate");
  }

  return dupval("(unknown)"); /* should never happen */
}

/************************************/
/* returns true value of a ->width  */
/************************************/

int truewidth(int width)

{
  if ((width == INCHANNELSWIDTH) || (width == CHANNELSWIDTH))
    return currinputwidth;
  if (width == OUTCHANNELSWIDTH)
    return currinstrwidth;
  return width;
}


/*******************************************************************/
/* width check for two tnodes in an <expr>, aborts if it fails, if */
/* it succeeds it returns the width of the finished expression     */
/*******************************************************************/

int exprwidth(tnode * left, tnode * right)

{
  tnode * tptr;

  switch(left->width) {
  case CHANNELSWIDTH:
    switch(right->width) {
    case CHANNELSWIDTH:
    case OUTCHANNELSWIDTH:
    case INCHANNELSWIDTH:
    case 1:
      return CHANNELSWIDTH;
    default:
      return right->width;
    }
  case OUTCHANNELSWIDTH:
    switch(right->width) {
    case CHANNELSWIDTH:
    case INCHANNELSWIDTH:
      return CHANNELSWIDTH;
    case OUTCHANNELSWIDTH:
    case 1:
      return OUTCHANNELSWIDTH;
    default:
      return right->width;
    }
  case INCHANNELSWIDTH:
    switch(right->width) {
    case CHANNELSWIDTH:
    case OUTCHANNELSWIDTH:
      return CHANNELSWIDTH;
    case INCHANNELSWIDTH:
    case 1:
      return INCHANNELSWIDTH;
    default:
      return right->width;
    }
  case 1:
    return right->width;
  }


  switch(right->width) {
  case CHANNELSWIDTH:
  case OUTCHANNELSWIDTH:
  case INCHANNELSWIDTH:
  case 1:
    break;
  default:
    if (left->width != right->width)
      {
	tptr = left;
	while (tptr->down != NULL)
	  tptr = tptr->down;
	printf("Error: Width mismatch in subexpression.\n");
	showerrorplace(tptr->linenum, tptr->filename);
      }
  }
  return left->width; 

}



/*******************************************************************/
/* width check for an <expr> * <expr> * <expr>, aborts if it fails */
/* it succeeds it returns the width of the finished expression     */
/*******************************************************************/

int multiwidth(tnode * tptr)

{
  int width;

  width = tptr->width;

  while (tptr->next != NULL)
    {
      tptr = tptr->next->next;

      switch(width) {
      case CHANNELSWIDTH:
	switch(tptr->width) {
	case CHANNELSWIDTH:
	case OUTCHANNELSWIDTH:
	case INCHANNELSWIDTH:
	case 1:
	  width = CHANNELSWIDTH;
	default:
	  width = tptr->width;
	}
      case OUTCHANNELSWIDTH:
	switch(tptr->width) {
	case CHANNELSWIDTH:
	case INCHANNELSWIDTH:
	  width = CHANNELSWIDTH;
	case OUTCHANNELSWIDTH:
	case 1:
	  width = OUTCHANNELSWIDTH;
	default:
	  width = tptr->width;
	}
      case INCHANNELSWIDTH:
	switch(tptr->width) {
	case CHANNELSWIDTH:
	case OUTCHANNELSWIDTH:
	  width = CHANNELSWIDTH;
	case INCHANNELSWIDTH:
	case 1:
	  width = INCHANNELSWIDTH;
	default:
	  width = tptr->width;
	}
      case 1:
	width = tptr->width;
      default:
	switch(tptr->width) {
	case CHANNELSWIDTH:
	case OUTCHANNELSWIDTH:
	case INCHANNELSWIDTH:
	case 1:
	  break;
	default:
	  if (width != tptr->width)
	    {
	      while (tptr->down != NULL)
		tptr = tptr->down;
	      printf("Error: Width mismatch in subexpression.\n");
	      showerrorplace(tptr->linenum, tptr->filename);
	    }
	}
      }
    }

  return width;

}



/****************************************************************/
/*              updates widths in parse tree                   */
/****************************************************************/

int widthupdate(tnode * tbranch)

{

  tnode * tptr = tbranch;
  tnode * bptr;
  int retval = NOTDEFINED;


  while (tptr != NULL)
    {
      if (tptr->down != NULL)
	{
	  retval = widthupdate(tptr->down);
	  switch (tptr->ttype) {
	  case S_LVALUE:
	    if (tptr->down->next == NULL)         /* unindexed */
	      {
		if (standardname(tptr->down))
		  tptr->width = standardwidth(tptr->down);
		else
		  switch (tptr->down->sptr->width) {
		  case CHANNELSWIDTH:             /* should never happen */
		    tptr->width = CHANNELSWIDTH;
		    break;
		  case INCHANNELSWIDTH: 
		    tptr->width = currinputwidth;
		    break;
		  case OUTCHANNELSWIDTH: 
		    tptr->width = currinstrwidth;
		    break;
		  default:
		    tptr->width = tptr->down->width;
		  }
	      }
	    else                                  /* indexed */
	      {
		tptr->width = 1;
		if (tptr->down->next->next->width > 1)
		  {
		    printf("Error: Lvalue array index must be width 1.\n");
		    showerrorplace(tptr->down->linenum, 
				   tptr->down->filename);
		  }
	      }
	    break;
	  case S_STATEMENT:
	    if (tptr->down->ttype == S_LVALUE)
	      {
		if ((tptr->down->width == 1) &&    /* special case */ 
		    (tptr->down->next->next->width > 1))
		  {
		    printf("Error: Width mismatch in assignment statement.\n");
		    showerrorplace(tptr->down->linenum, 
				   tptr->down->filename);
		  }
		else
		  tptr->width = exprwidth(tptr->down,
					  tptr->down->next->next);
		break;
	      }
	    if (tptr->down->ttype == S_OUTPUT)
	      {
		bptr = tptr->down->next->next->down;
		tptr->width = 0;
		while (bptr != NULL)
		  {
		    if (bptr->ttype == S_EXPR)
		      {
			switch(tptr->width) {
			case CHANNELSWIDTH:
			  break;
			case OUTCHANNELSWIDTH:
			  if ((bptr->width == INCHANNELSWIDTH)||
			      (bptr->width == CHANNELSWIDTH))
			    tptr->width = CHANNELSWIDTH;
			  break;
			case INCHANNELSWIDTH:
			  if ((bptr->width == OUTCHANNELSWIDTH)||
			      (bptr->width == CHANNELSWIDTH))
			    tptr->width = CHANNELSWIDTH;
			  break;
			default:
			  if ((bptr->width == OUTCHANNELSWIDTH)||
			      (bptr->width == OUTCHANNELSWIDTH)||
			      (bptr->width == CHANNELSWIDTH))
			    tptr->width = bptr->width;
			  else
			    tptr->width += bptr->width;
			  break;
			}
		      }
		    bptr = bptr->next;
		  }
		switch(currinstrwidth) {
		case CHANNELSWIDTH:
		  if (tptr->width > 0)
		    currinstrwidth = tptr->width;
		  break;
		case OUTCHANNELSWIDTH:
		  if (tptr->width > 0)
		    currinstrwidth = tptr->width;
		  if ((tptr->width == INCHANNELSWIDTH)||
		      (tptr->width == CHANNELSWIDTH))
		    currinstrwidth = CHANNELSWIDTH;
		  break;
		case INCHANNELSWIDTH:
		  if (tptr->width > 0)
		    currinstrwidth = tptr->width;
		  if ((tptr->width == OUTCHANNELSWIDTH)||
		      (tptr->width == CHANNELSWIDTH))
		    currinstrwidth = CHANNELSWIDTH;
		  break;
		default:
		  if ((tptr->width > 1)&&
		      (currinstrwidth > 1) &&
		      (currinstrwidth!=tptr->width))
		    {
		      printf("Error: Output statements width mismatch.\n");
		      showerrorplace(tptr->down->linenum, 
				     tptr->down->filename);
		    }
		  if (tptr->width > 1)
		    currinstrwidth = tptr->width;
		  break;
		}
		break;
	      }
	    if (tptr->down->ttype == S_OUTBUS)
	      {
		if (tptr->sptr->width >= 1)
		  {
		    bptr = tptr->down->next->next->next->next->down;
		    tptr->width = 0;
		    while (bptr != NULL)
		      {
			if (bptr->ttype == S_EXPR)
			  {
			    switch(tptr->width) {
			    case CHANNELSWIDTH:
			      break;
			    case OUTCHANNELSWIDTH:
			      if ((bptr->width == INCHANNELSWIDTH)||
				  (bptr->width == CHANNELSWIDTH))
				tptr->width = CHANNELSWIDTH;
			      break;
			    case INCHANNELSWIDTH:
			      if ((bptr->width == OUTCHANNELSWIDTH)||
				  (bptr->width == CHANNELSWIDTH))
				tptr->width = CHANNELSWIDTH;
			      break;
			    default:
			      if ((bptr->width == OUTCHANNELSWIDTH)||
				  (bptr->width == OUTCHANNELSWIDTH)||
				  (bptr->width == CHANNELSWIDTH))
				tptr->width = bptr->width;
			      else
				tptr->width += bptr->width;
			      break;
			    }
			  }
			bptr = bptr->next;
		      }
		    if ((tptr->width > 1) &&
			(tptr->width != tptr->sptr->width))
		      {
			printf("Error: Outbus statements width mismatch.\n");
			showerrorplace(tptr->down->linenum, 
				       tptr->down->filename);
		      }
		  }
		break;
	      }
	    if (tptr->down->ttype == S_INSTR)
	      {
		bptr = tptr->down->next->next->next->down;
		while (bptr != NULL)
		  {
		    if ((bptr->ttype == S_EXPR)&&(bptr->width > 1))
		      {
			printf("Error: Instr exprs must have width 1.\n");
			showerrorplace(tptr->down->linenum, 
				       tptr->down->filename);
		      }
		    bptr = bptr->next;
		  }
		break;
	      }
	    if (tptr->down->ttype == S_IF)
	      {
		if (tptr->down->next->next->width > 1)
		  {
		    printf("Error: If guard must have width 1.\n");
		    showerrorplace(tptr->down->linenum, 
				   tptr->down->filename);
		  }
		break;
	      }
	    if (tptr->down->ttype == S_WHILE)
	      {
		if (tptr->down->next->next->width > 1)
		  {
		    printf("Error: While guard must have width 1.\n");
		    showerrorplace(tptr->down->linenum, 
				   tptr->down->filename);
		  }
		break;
	      }
	    if (tptr->down->ttype == S_EXTEND)
	      {
		if (tptr->down->next->next->width > 1)
		  {
		    printf("Error: Extend expression must have width 1.\n");
		    showerrorplace(tptr->down->linenum, 
				   tptr->down->filename);
		  }
		break;
	      }
	    if (tptr->down->ttype == S_PRINTF)
	      {
		bptr = tptr->down->next->next->down;
		if (bptr->ttype != S_STRCONST)
		  {
		    printf("Error: Printf must start with format string.\n\n");
		    showerrorplace(tptr->down->linenum, tptr->down->filename);
		  }
		bptr = bptr->next;
		while (bptr != NULL)
		  {
		    if (bptr->ttype == S_EXPR)
		      {		
			if (bptr->width > 1)
			  {
			    printf("Error: A printf argument not width 1.\n");
			    showerrorplace(tptr->down->linenum, 
					   tptr->down->filename);
			  }
		      }
		    bptr = bptr->next;
		  }
		break;
	      }
	    if (tptr->down->ttype == S_SPATIALIZE)
	      {
		bptr = tptr->down->next->next->down;
		if (bptr == NULL)
		  {
		    printf("Error: Spatialize lacks audio signal.\n");
		    showerrorplace(tptr->down->linenum, 
				   tptr->down->filename);
		  }
		if (bptr->width > 1)
		  {
		    printf("Error: Spatialize audio signal must be width 1.\n");
		    showerrorplace(tptr->down->linenum, 
				   tptr->down->filename);
		  }
		bptr = bptr->next;
		if (bptr != NULL)
		  bptr = bptr->next;
		if (bptr == NULL)
		  {
		    printf("Error: Spatialize lacks azimuth.\n");
		    showerrorplace(tptr->down->linenum, 
				   tptr->down->filename);
		  }
		if (bptr->width > 1)
		  {
		    printf("Error: Spatialize azimuth must be width 1.\n");
		    showerrorplace(tptr->down->linenum, 
				   tptr->down->filename);
		  }
		bptr = bptr->next;
		if (bptr != NULL)
		  bptr = bptr->next;
		if (bptr == NULL)
		  {
		    printf("Error: Spatialize lacks elevation.\n");
		    showerrorplace(tptr->down->linenum, 
				   tptr->down->filename);
		  }
		if (bptr->width > 1)
		  {
		    printf("Error: Spatialize elevation must be width 1.\n");
		    showerrorplace(tptr->down->linenum, 
				   tptr->down->filename);
		  }
		bptr = bptr->next;
		if (bptr != NULL)
		  bptr = bptr->next;
		if (bptr == NULL)
		  {
		    printf("Error: Spatialize lacks distance.\n");
		    showerrorplace(tptr->down->linenum, 
				   tptr->down->filename);
		  }
		if (bptr->width > 1)
		  {
		    printf("Error: Spatialize distance must be width 1.\n");
		    showerrorplace(tptr->down->linenum, 
				   tptr->down->filename);
		  }
		if (bptr->next != NULL)
		  {
		    printf("Error: Spatialize has too many expressions.\n");
		    showerrorplace(tptr->down->linenum, 
				   tptr->down->filename);
		  }
		tptr->width = 1;
		break;
	      }
	    if (tptr->down->ttype == S_RETURN) /* RETURN LP exprlist RP SEM */ 
	      {
		bptr = tptr->down->next->next->down;
		tptr->width = 0;
		while (bptr != NULL)
		  {
		    if (bptr->ttype == S_EXPR)
		      {
			switch(tptr->width) {
			case CHANNELSWIDTH:
			  break;
			case OUTCHANNELSWIDTH:
			  if ((bptr->width == INCHANNELSWIDTH)||
			      (bptr->width == CHANNELSWIDTH))
			    tptr->width = CHANNELSWIDTH;
			  break;
			case INCHANNELSWIDTH:
			  if ((bptr->width == OUTCHANNELSWIDTH)||
			      (bptr->width == CHANNELSWIDTH))
			    tptr->width = CHANNELSWIDTH;
			  break;
			default:
			  if ((bptr->width == OUTCHANNELSWIDTH)||
			      (bptr->width == OUTCHANNELSWIDTH)||
			      (bptr->width == CHANNELSWIDTH))
			    tptr->width = bptr->width;
			  else
			    tptr->width += bptr->width;
			  break;
			}
		      }
		      bptr = bptr->next;
		  }
		if (retval == NOTDEFINED)
		  retval = tptr->width;
		else
		  {
		    if ((tptr->width >= 0) && (retval >= 0))
		      {
			if (tptr->width != retval)
			  {
			    printf("Error: Return statements width mismatch.\n");
			    showerrorplace(tptr->down->linenum, 
					   tptr->down->filename);    
			  }
		      }
		  }
		break;
	      }
	    break;
	  case S_EXPR:
	    if (tptr->down->next == NULL)          /* ident or constant */
	      {
		tptr->width = tptr->down->width;
		break;
	      }
	    if (tptr->optr != NULL)               /* opcode call */
	      {
		tptr->optr->width = tptr->width =
		  widthupdate(tptr->optr->sptr->defnode->down->
			 next->next->next->next->next->next->next->down);
		if (tptr->width == NOTDEFINED)
		  tptr->optr->width = tptr->width = 1;
                break;
              }
	    if ((tptr->down->ttype == S_MINUS) ||
                (tptr->down->ttype == S_NOT))      /* unary */
	      {
		tptr->width = tptr->down->next->width;
		break;
	      }
	    if (tptr->down->ttype == S_LP) /* works for float->into to */
	      {
		tptr->width = tptr->down->next->width;
		break;
	      }
	    if (tptr->down->ttype == S_FLOATCAST) 
	      {
		tptr->width = tptr->down->next->next->width;
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
		tptr->width = exprwidth(tptr->down,
					tptr->down->next->next);
		break;
	      }
	    if ((tptr->down->next->ttype == S_PLUS)  ||
                (tptr->down->next->ttype == S_MINUS) ||
		(tptr->down->next->ttype == S_STAR) )
	      {
		tptr->width = multiwidth(tptr->down);
		break;
	      }
	    if (tptr->down->next->ttype == S_Q)
	      {
		tptr->width = exprwidth(tptr->down,
					tptr->down->next->next);
		tptr->width = exprwidth(tptr,
					tptr->down->next->next->next->next);
		break;
	      }
	    if ((tptr->down->next->ttype == S_LB) &&         /*array index*/
		(tptr->down->next->next->next->next == NULL))
		{
		  if (tptr->down->next->next->width > 1)
		    {
		      printf("Error: Array index must have width 1.\n");
		      showerrorplace(tptr->down->linenum, 
				     tptr->down->filename);
		    }
		  tptr->width = 1;
		  break;
		}
	    break;
	  }
	}
      else
	if (tptr->ttype == S_IDENT)
	  {
	    if (standardname(tptr))
	      tptr->width = standardwidth(tptr);
	    else
	      if (tptr->sptr != NULL)
		{
		  switch (tptr->sptr->width) {
		  case CHANNELSWIDTH:             /* should never happen */
		    tptr->width = CHANNELSWIDTH;
		    break;
		  case INCHANNELSWIDTH: 
		    tptr->width = currinputwidth;
		    break;
		  case OUTCHANNELSWIDTH: 
		    tptr->width = currinstrwidth;
		    break;
		  default:
		    tptr->width = tptr->sptr->width;
		  }
		}
	  }
      tptr = tptr->next;
    }
  return retval;
}

/************************************************************/
/*         rate check for two tnodes in an <expr>           */
/************************************************************/

int exprrate(tnode * left, tnode * right)

{
  return ((left->rate > right->rate) ? left->rate : right->rate);
}

/************************************************************/
/*         rate check for <expr> * <expr> * <expr> ...      */
/************************************************************/

int multirate(tnode * tptr)

{
  int rate = tptr->rate;

  while (tptr->next != NULL)
    {
      tptr = tptr->next->next;
      if (rate < tptr->rate)
	rate = tptr->rate;
    }
  return rate;

}

/************************************************************/
/*         computes true rate of an IDENT                   */
/************************************************************/

int truerate(tnode * tptr)

{
  if (tptr->sptr->rate == XRATETYPE)
    {
      if (tptr->sptr->kind == K_PFIELD)
	return tptr->sptr->calrate;
      else
	return curropcoderate;
    }
  return tptr->sptr->rate;
}

/************************************************************/
/*   labels opcodes with rate of its controlling statement  */
/************************************************************/

void opcodelabel(tnode * tptr, int rate)

{
  while (tptr != NULL)
    {
      if (tptr->down != NULL)
	opcodelabel(tptr->down, rate);
      if ((tptr->ttype == S_EXPR) && (tptr->optr != NULL))
	tptr->staterate = tptr->optr->staterate = rate;
      tptr = tptr->next;
    }
}

/************************************************************/
/*         statement rate check against opcode rate         */
/************************************************************/

void opcoderatecheck(tnode * statement)

{
  tnode * tptr;

  if (statement->rate > curropcoderate)
    {
      printf("Error: Statement runs at %s, faster than %s opcode.\n",
	     ratetostr(statement->rate),ratetostr(curropcoderate));
      tptr = statement;
      while (tptr->down != NULL)
	tptr = tptr->down;
      showerrorplace(tptr->linenum, tptr->filename);
    }
  opcodelabel(statement->down, statement->rate);
}


/************************************************************/
/*           computes slowest opcode in a block             */
/************************************************************/

int slowestopcode(tnode * tbranch)

{
  int newval;
  int retval = ARATETYPE;
  tnode * tptr = tbranch;

  while (tptr != NULL) 
    {
      if (tptr->down != NULL)
	{
	  newval = slowestopcode(tptr->down);
	  retval = (newval < retval) ? newval : retval;
	}
      if ((tptr->ttype == S_EXPR) && (tptr->optr != NULL))
	{
	  if (!looseopcoderules(tptr))
	    retval = (tptr->rate < retval) ? tptr->rate : retval;
	}
      tptr = tptr->next;
    }
  return retval;

}

/************************************************************/
/* computes maximum rate of a list of statements             */
/************************************************************/

void blockrate(tnode * block, int * minrate, int * maxrate, int * flags)

{
  int klines = 0;
  int ilines = 0;

  *maxrate = IRATETYPE;
  *minrate = ARATETYPE;
  while (block != NULL) 
    {
      if (block->rate > *maxrate)
	*maxrate = block->rate;
      if (block->rate < *minrate)
	*minrate = block->rate;
      klines |= (block->rate == KRATETYPE);
      ilines |= (block->rate == IRATETYPE);
      block = block->next;
    }

  if (*minrate < *maxrate)
    *flags = ilines*IRATESECTION + 
      (((*maxrate) == ARATETYPE)*klines*KRATESECTION);
}

/************************************************************/
/*   deletes statements in a block slower than rate         */
/************************************************************/

void deleteslower(tnode * cptr, int maxrate)

{
  tnode * tptr;
  tnode * last;

  last = tptr = cptr->down;
  while (tptr != NULL)
    {
      if (tptr->rate < maxrate)
	{
	  if (last == tptr)
	    cptr->down = tptr->next;
	  else
	    last->next = tptr->next;
	}
      else
	last = tptr;
      tptr = tptr->next;
    }

}

/************************************************************/
/*           updates opcode symbol table                    */
/************************************************************/

void symrateupdate(tnode * tptr)

{
  sigsym * sptr;

  if (tptr->sptr->rate == XRATETYPE)
    {
      tptr->sptr->rate = tptr->rate;
      sptr = tptr->sptr->defnode->sptr;
      while (sptr != NULL)
	{
	  if (sptr->rate == XRATETYPE)
	    {
	      if (sptr->kind == K_PFIELD)
		sptr->rate = sptr->calrate;
	      else
		sptr->rate = tptr->rate;
	    }
	  sptr = sptr->next;
	}
    }

}

/************************************************************/
/*           update rates of an opcode                      */
/************************************************************/

void opraterecurse(tnode * tptr)


{
  while (tptr != NULL)
    {
      if (tptr->ttype != S_OPARRAYDECL)
	{
	  curropcoderate = tptr->rate;
	  currtreerate = UNKNOWN;

	  rateupdate(tptr->sptr->defnode->down->
		     next->next->next->next->next->next->next->down);
	  symrateupdate(tptr);
	  opraterecurse(tptr->sptr->defnode->optr);
	}
      tptr = tptr->next;
    }
}

/************************************************************/
/*       update wavetable rates for symrateupdate use       */
/*    error checks occur in waveinitcheck() [wtparse.c]     */
/************************************************************/

void tablerateupdate(tnode * tptr)


{
  tnode * pvalptr;

  while (tptr)
    {
      if (tptr->ttype == S_TABLE)
	{
	  pvalptr = tptr->down->next->next->next->next->next->down;
	  if (pvalptr)
	    {
	      curropcoderate = ARATETYPE;
	      currtreerate = UNKNOWN;
	      rateupdate(pvalptr);
	    }
	}
      tptr = tptr->next;
    }
}

/************************************************************/
/*           update rates of an instrument                  */
/************************************************************/

void inrateupdate(sigsym * sptr)

{
  tablerateupdate(sptr->defnode->down->
		  next->next->next->next->next->next->next->down);

  curropcoderate = ARATETYPE;
  currtreerate = UNKNOWN;

  rateupdate(sptr->defnode->down->
	     next->next->next->next->next->next->next->next->down);

  opraterecurse(sptr->defnode->optr);
}


/****************************************************************/
/*                 Checks opcode arguments                     */
/****************************************************************/

void checkopcodeargsrate(tnode * tptr)


{
  tnode * aptr;
  tnode * dptr;
  tnode * cptr;

  /* aptr holds actual arguments          */
  /* rates already computed prior to call */

  if (tptr->optr->ttype == S_OPCALL) 
    aptr = tptr->down->next->next->down;
  else
    aptr = tptr->down->next->next->next->next->next->down;

  /* dptr holds formal arguments */

  dptr = tptr->optr->sptr->defnode->down->next->next->next->down;

  while (dptr != NULL)
    {
      if (dptr->ttype == S_PARAMDECL)
	{
	  if (dptr->rate == XRATETYPE)
	    {
	      dptr->sptr->calrate = (currtreerate > aptr->rate) ?
		currtreerate : aptr->rate;
	    }
	  else
	    {
	      if ((aptr->vartype != TABLETYPE) && (aptr->rate > dptr->rate))
		{
		  printf("Error: Argument faster than formal argument.\n");
		  showerrorplace(tptr->down->linenum, 
				 tptr->down->filename);
		}
	    }
	}
      dptr = dptr->next;
      aptr = aptr->next;
    }

  if (tptr->optr->extra != NULL)      /* varargs */
    {
      dptr = tptr->optr->extra;
      while (dptr != NULL)
	{
	  if (dptr->ttype == S_PARAMDECL)
	    {
	      if ((dptr->rate != XRATETYPE) && (aptr->vartype != TABLETYPE)
		  && (aptr->rate > dptr->rate))
		{
		  printf("Error: Argument faster than formal argument.\n");
		  showerrorplace(tptr->down->linenum, 
				 tptr->down->filename);
		}
	    }
	  dptr = dptr->next;
	  aptr = aptr->next;
	}
    }

  if (tptr->optr->ttype == S_OPARRAYCALL)
    {
      cptr = tptr->down->next->next->next;
      tptr->down->next->next->next = NULL;
      rateupdate(tptr->down->next->next);
      tptr->down->next->next->next = cptr;
      if (tptr->down->next->next->rate > tptr->optr->rate)
	{
	  printf("Error: Oparray index faster than opcode.\n");
	  showerrorplace(tptr->down->linenum, 
			 tptr->down->filename);
	}
    }

}


/****************************************************************/
/*        Checks dynamic instrument arguments, sets rate        */
/****************************************************************/

void dinstrargsrate(tnode * tptr)          /* INSTR IDENT LP exprlist RP SEM */

{
  tnode * aptr;
  tnode * dptr;
  
  /* aptr holds actual arguments          */

  aptr = tptr->down->next->next->next->down;
  rateupdate(aptr);

  /* dptr holds formal arguments */

  dptr = tptr->dptr->sptr->defnode->down->next->next->next->down;

  if (aptr == NULL)
    {
      printf("Error: Instr statement must include time delay.\n");
      showerrorplace(tptr->down->linenum, 
		     tptr->down->filename);
    }

  tptr->rate = aptr->rate;
  aptr = aptr->next;
  if (aptr != NULL)
    aptr = aptr->next;

  if (aptr == NULL)
    {
      printf("Error: Instr statement must include duration.\n");
      showerrorplace(tptr->down->linenum, 
		     tptr->down->filename);
    }

  tptr->rate = (tptr->rate > aptr->rate) ? tptr->rate : aptr->rate;
  aptr = aptr->next;
  if (aptr != NULL)
    aptr = aptr->next;

  while (dptr != NULL)
    {
      if (dptr->ttype == S_IDENT)
	{
	  if (aptr == NULL)
	    {
	      printf("Error: Instr statement has insufficient parameters.\n");
	      showerrorplace(tptr->down->linenum, 
			     tptr->down->filename);
	    }
	  tptr->rate = (tptr->rate > aptr->rate) ? tptr->rate : aptr->rate;
	}
      dptr = dptr->next;
      if (aptr != NULL)
	aptr = aptr->next;
    }
  if (aptr != NULL)
    {
      printf("Error: Instr statement has too many parameters.\n");
      showerrorplace(tptr->down->linenum, 
		     tptr->down->filename);
    }

  if (currtreerate > tptr->rate)  /* guard, outer opcodes, */
    tptr->rate = currtreerate;

  if (tptr->rate == ARATETYPE)
    {
      printf("Error: Dynamic instrument statement cannot be a-rate.\n");
      showerrorplace(tptr->down->linenum, 
		     tptr->down->filename);
    }
}


/****************************************************************/
/*              updates rate in parse tree                   */
/****************************************************************/

void rateupdate(tnode * tbranch)

{

  tnode * tptr = tbranch;
  tnode * bptr;
  tnode * cptr;
  tnode * c2ptr;
  int minrate, maxrate, flags;
  int min2rate, max2rate, flags2;
  int treeratestack;

  treeratestack = currtreerate;
  while (tptr != NULL)
    {
      if (tptr->down != NULL)
	{

	  /* update currtreerate for this level of the tree */

	  if ( (tptr->ttype == S_STATEMENT) && 
               ((tptr->down->ttype == S_WHILE) || (tptr->down->ttype == S_IF)))
	    {
	      cptr = tptr->down->next->next->next;
	      tptr->down->next->next->next = NULL;
	      rateupdate(tptr->down->next->next);
	      if (tptr->down->next->next->rate > currtreerate)
		currtreerate = tptr->down->next->next->rate;
	      tptr->down->next->next->next = cptr;
	      rateupdate(cptr->next->next->down);

	      /* else block */

	      if (cptr->next->next->next->next != NULL)
		rateupdate(cptr->next->next->next->next->next->next->down);
	    }
	  else /* everything else but if, if-else and while */
	    {
	      if ( (tptr->ttype == S_STATEMENT) && 
		   (tptr->down->ttype == S_INSTR))
		{
		  dinstrargsrate(tptr); /* sets tptr->rate */
		}
	      if ( (tptr->ttype == S_STATEMENT) && 
		   (tptr->down->ttype == S_EXTEND))
		{
		  cptr = tptr->down->next->next;
		  bptr = cptr->next;
		  cptr->next = NULL;
		  rateupdate(cptr);
		  cptr->next = bptr;

		  /*
		  if (cptr->rate == ARATETYPE)
		    {
		      printf("Error: Extend <expr> may not be arate.\n");
		      showerrorplace(tptr->down->linenum, 
				     tptr->down->filename);
		    }
		  */

		  tptr->rate = cptr->rate;
		  if (currtreerate > tptr->rate)  /* guard, outer opcodes, */
		    tptr->rate = currtreerate;
		  if (tptr->rate == ARATETYPE)
		    tptr->rate = KRATETYPE;
		}
	      if ( (tptr->ttype == S_EXPR) && (tptr->optr != NULL) )
		{
		  if (tptr->sptr->rate == XRATETYPE)
		    {
		      /* fastest of:  formal parameters, */ 

		      tptr->rate =    
			tptr->sptr->defnode->down->next->next->next->rate;

		      /* including varargs */

		      if ((tptr->optr->extra != NULL) &&
			  (tptr->optr->extrarate > tptr->rate))
			tptr->rate = tptr->optr->extrarate;

		      /* actual parameters, */

		      if (tptr->optr->ttype == S_OPCALL)
			cptr = tptr->down->next->next->down;
		      else
			cptr = tptr->down->next->next->next->next->next->down;

		      while (cptr != NULL)       
			{
			  if ((cptr->ttype == S_EXPR) && 
			      (cptr->vartype != TABLETYPE) &&
			      (cptr->vartype != TMAPTYPE) )
			    {
			      bptr = cptr->next;
			      cptr->next = NULL;
			      rateupdate(cptr);
			      if (cptr->rate > tptr->rate)
				tptr->rate = cptr->rate;
			      cptr->next = bptr;
			    }
			  cptr = cptr->next;
			}

		      /* guard, and outer opcodes */

		      if ( ((!looseopcoderules(tptr)) ||
			    delicatepolyops(tptr->down)) &&
			   (currtreerate > tptr->rate))
			  tptr->rate = currtreerate;

		      if (tptr->rate == UNKNOWN)
			tptr->rate = KRATETYPE;

		      tptr->optr->rate = tptr->rate;
		    }
		  else
		    {
		      if (tptr->optr->ttype == S_OPCALL)
			cptr = tptr->down->next->next->down;
		      else
			cptr = tptr->down->next->next->next->next->next->down;
		      while (cptr != NULL) 
			{
			  if (cptr->ttype == S_EXPR)
			    {
			      bptr = cptr->next;
			      cptr->next = NULL;
			      rateupdate(cptr);
			      cptr->next = bptr;
			    }
			  cptr = cptr->next;
			}
		      tptr->rate = tptr->optr->rate = tptr->sptr->rate;
		    }
		  checkopcodeargsrate(tptr);
		  if (tptr->rate > currtreerate)
		    currtreerate = tptr->rate;
		}
	      rateupdate(tptr->down);
	    }

	  switch (tptr->ttype) {
	  case S_LVALUE:
	    if (standardname(tptr->down))
	      tptr->down->rate = standardrate(tptr->down);
	    else
	      tptr->down->rate = truerate(tptr->down);
	    if (tptr->down->next == NULL)         /* unindexed */
	      {
		tptr->rate = tptr->down->rate; 
	      }
	    else                                  /* indexed */
	      {
		tptr->rate = (tptr->down->rate > tptr->down->next->next->rate) ?
		  tptr->down->rate : tptr->down->next->next->rate;
	      }
	    break;
	  case S_STATEMENT:
	    if (tptr->down->next->ttype == S_SEM)
	      {
		if (tptr->down->ttype == S_EXPR)     /* expr SEM */
		  {
		    tptr->rate = tptr->down->rate;
		  }
		else                                 /* turnoff SEM */
		  {
		    tptr->rate = KRATETYPE;
		  }
		opcoderatecheck(tptr);
		break;
	      }
	    if (tptr->down->ttype == S_LVALUE) /* lvalue EQ expr SEM */ 
	      {
		if (tptr->down->rate < tptr->down->next->next->rate)
		  {
		    printf("Error: LHS `%s' is %s, slower than RHS %s.\n",
			   tptr->down->down->val,
			   ratetostr(tptr->down->rate), 
			   ratetostr(tptr->down->next->next->rate));
		    showerrorplace(tptr->down->down->linenum, 
				   tptr->down->down->filename);
		  }
		tptr->rate = tptr->down->rate;	
		opcoderatecheck(tptr);
		break;
	      }
	    if (tptr->down->ttype == S_OUTPUT)
	      {
		tptr->rate = ARATETYPE;
		opcoderatecheck(tptr);
		break;
	      }
	    if (tptr->down->ttype == S_OUTBUS)
	      {
		tptr->rate = ARATETYPE;
		opcoderatecheck(tptr);
		break;
	      }
	    if (tptr->down->ttype == S_INSTR)
	      {
		/* taken care of above */
		break;
	      }
	    if (tptr->down->ttype == S_EXTEND)
	      {
		/* taken care of above */
		break;
	      }
	    if (tptr->down->ttype == S_RETURN)
	      {
		tptr->rate = curropcoderate;		
		bptr = tptr->down->next->next->down;
		while (bptr)
		  {
		    if ((bptr->ttype == S_EXPR) &&
			(bptr->rate > curropcoderate))
		      {		  
			printf("Error: %s return expression faster" 
			       " than %s opcode.\n", 
			       ratetostr(bptr->rate), 
			       ratetostr(curropcoderate));
			showerrorplace(tptr->down->linenum, 
				       tptr->down->filename);
		      }
		    bptr = bptr->next;
		  }
		opcoderatecheck(tptr);
		break;
	      }
	    if (tptr->down->ttype == S_PRINTF)
	      {
		tptr->rate = IRATETYPE;
		bptr = tptr->down->next->next->down;
		while (bptr)
		  {
		    if ((bptr->ttype == S_EXPR) &&
			(bptr->rate > tptr->rate))
		      tptr->rate = bptr->rate;
		    bptr = bptr->next;
		  }
		opcoderatecheck(tptr);
		break;
	      }
	    if (tptr->down->ttype == S_SPATIALIZE)
	      {
		tptr->rate = tptr->optr->rate = ARATETYPE;
		opcoderatecheck(tptr);
		break;
	      }
	    if (tptr->down->ttype == S_IF)
	      {
		cptr = tptr->down->next->next->next->next->next;
		minrate = maxrate = tptr->down->next->next->rate;
		flags = 0;
		if (cptr->down != NULL)
		  {
		    blockrate(cptr->down, &minrate, &maxrate, &flags);
		    if (minrate < tptr->down->next->next->rate)
		      {
			printf("Error: If block has statements slower than guard.\n");
			showerrorplace(tptr->down->linenum, 
				       tptr->down->filename);
		      }
		    if (slowestopcode(cptr->down) < 
			tptr->down->next->next->rate)
		      {
			printf("Error: If block has opcodes slower than guard.\n");
			showerrorplace(tptr->down->linenum, 
				       tptr->down->filename);
		      }
		  }
		tptr->rate = maxrate;
		if (cptr->next->next != NULL)  /* an if-else statement */
		  {
		    c2ptr = cptr->next->next->next->next;
		    flags2 = 0;
		    if (c2ptr->down != NULL)
		      {
			blockrate(c2ptr->down, &min2rate, &max2rate, &flags2);
			if (min2rate < tptr->down->next->next->rate)
			  {
			    printf("Error: Else block has statements slower than guard.\n");
			    showerrorplace(tptr->down->linenum, 
				       tptr->down->filename);
			  }
			if (slowestopcode(c2ptr->down) <
			    tptr->down->next->next->rate)
			  {
			    printf("Error: Else block has opcodes slower than guard.\n");
			    showerrorplace(tptr->down->linenum, 
					   tptr->down->filename);
			  }
			tptr->rate = (max2rate > tptr->rate) ? max2rate :  tptr->rate;
			if (min2rate != tptr->rate)
			  {
			    /* legal slower-rate ELSE statement */

			    tptr->down->next->next->next
			      ->next->next->next->next->inwidth = flags2;
			  }
		      }
		  }
		if (minrate != tptr->rate)
		  {
		    /* legal slower-rate IF statement */
		    
		    tptr->down->inwidth = flags;
		  }
		opcoderatecheck(tptr);
		break;
	      }
	    if (tptr->down->ttype == S_WHILE)
	      {
		minrate = maxrate = tptr->rate = tptr->down->next->next->rate;
		cptr = tptr->down->next->next->next->next->next;
		if (cptr->down != NULL)
		  blockrate(cptr->down, &minrate, &maxrate, &flags);
		if ( !( (minrate == maxrate) && (minrate == tptr->rate) ) )
		  {
		    printf("Error: While block has statements ");
		    printf("different from guard rate.\n");
		    showerrorplace(tptr->down->linenum, 
				   tptr->down->filename);
		  }
		if (slowestopcode(cptr->down) < tptr->rate)
		  {
		    printf("Error: While block opcodes slower than guard.\n");
		    showerrorplace(tptr->down->linenum, 
				   tptr->down->filename);
		  }
		opcoderatecheck(tptr);
		break;
	      }
	    break;
	  case S_EXPR:
	    if (tptr->down->next == NULL)          
	      {
		if (tptr->down->ttype == S_IDENT)     /* ident */
		  {
		    if (standardname(tptr->down))
		      tptr->rate = tptr->down->rate = standardrate(tptr->down);
		    else
		      tptr->rate = tptr->down->rate = truerate(tptr->down);
		  }
		else
		  tptr->rate = tptr->down->rate;   /* constant */
		break;
	      }
	    if ((tptr->down->ttype == S_MINUS) ||
                (tptr->down->ttype == S_NOT))      /* unary */
	      {
		tptr->rate = tptr->down->next->rate;
		break;
	      }
	    if (tptr->down->ttype == S_LP) /* works for float->into to */
	      {
		tptr->rate = tptr->down->next->rate;
		break;
	      }
	    if (tptr->down->ttype == S_FLOATCAST) 
	      {
		tptr->rate = tptr->down->next->next->rate;
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
		tptr->rate = exprrate(tptr->down, tptr->down->next->next);
		break;
	      }
	    if ((tptr->down->next->ttype == S_PLUS)  ||
                (tptr->down->next->ttype == S_MINUS) ||
		(tptr->down->next->ttype == S_STAR) )
	      {
		tptr->rate = multirate(tptr->down);
		break;
	      }
	    if (tptr->down->next->ttype == S_Q)
	      {
		tptr->rate = exprrate(tptr->down,
					tptr->down->next->next);
		tptr->rate = exprrate(tptr,
					tptr->down->next->next->next->next);
		break;
	      }
	    if ((tptr->down->next->ttype == S_LB) &&         /*array index*/
		(tptr->down->next->next->next->next == NULL))
		{
		  if (standardname(tptr->down))
		    tptr->down->rate = standardrate(tptr->down);
		  else
		    tptr->down->rate = truerate(tptr->down);
		  tptr->rate = (tptr->down->rate > tptr->down->next->next->rate) ?
		                tptr->down->rate : tptr->down->next->next->rate ;
		  break;
		}
	    /* opcode and oparray calls taken care of above */
	    break;
	  }
	}
      currtreerate = treeratestack;
      tptr = tptr->next;
    }

}

