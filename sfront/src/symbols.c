
/*
#    Sfront, a SAOL to C translator    
#    This file: Library for sigsym and tnode structs
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

/***************************************/
/* simple operations on symbol tables  */
/***************************************/

/****************************************************************************/
/* adds an IDENT idnode to top of symboltable table, checks for duplication */
/****************************************************************************/

int addsym(sigsym ** table, tnode * idnode)

{

  sigsym* tmp;

  if (idnode->ttype != S_IDENT)
    return SYSERROR1; 

  tmp = *table;
  while (tmp != NULL)
    {
      if (!strcmp(tmp->val,idnode->val))
	return DUPLICATE;
      else
	tmp = tmp->next;
    }
  vmcheck(tmp = (sigsym*) malloc(sizeof(sigsym)));

  tmp->val = dupval(idnode->val);
  tmp->rate = idnode->rate;
  tmp->special = idnode->special;
  tmp->width = idnode->width;
  tmp->res = idnode->res;
  tmp->kind = K_NORMAL;
  tmp->vartype = SCALARTYPE;
  tmp->vol = VARIABLE;
  tmp->defnode = idnode;
  tmp->next = *table;
  tmp->numinst = 0;
  tmp->obus = NULL;
  tmp->maxifstate = 0;
  tmp->effects = 0;
  tmp->score = 0;
  tmp->ascore = 0;
  tmp->midi = 0;
  tmp->amidi = 0;
  tmp->miditag = 0;
  tmp->dyn = 0;
  tmp->startup = 0;
  tmp->calrate = UNKNOWN;
  tmp->cref = NULL;
  tmp->tref = NULL;

  (*table) = tmp;
  return INSTALLED;
}

/****************************************************************************/
/* adds a tnode-less symbol to the top of the symbol table                  */
/****************************************************************************/

int addvsym(sigsym ** table, char * name, int kind)

{

  sigsym* tmp;

  tmp = *table;
  while (tmp != NULL)
    {
      if (!strcmp(tmp->val, name))
	return DUPLICATE;
      else
	tmp = tmp->next;
    }

  vmcheck(tmp = (sigsym*) malloc(sizeof(sigsym)));

  tmp->val = dupval(name);
  tmp->rate = UNKNOWN;
  tmp->special = 0;
  tmp->width = 1;
  tmp->res = UNKNOWN;
  tmp->kind = kind;
  tmp->vartype = SCALARTYPE;
  tmp->vol = VARIABLE;
  tmp->defnode = NULL;
  tmp->next = *table;
  tmp->numinst = 0;
  tmp->obus = NULL;
  tmp->maxifstate = 0;
  tmp->effects = 0;
  tmp->score = 0;
  tmp->midi = 0;
  tmp->ascore = 0;
  tmp->amidi = 0;
  tmp->miditag = 0;
  tmp->dyn = 0;
  tmp->startup = 0;
  tmp->calrate = UNKNOWN;
  tmp->cref = NULL;
  tmp->tref = NULL;

  (*table) = tmp;
  return INSTALLED;
}

/****************************************************************************/
/* adds a tnode-less symbol to the end of the symbol table                  */
/****************************************************************************/

sigsym * addvsymend(sigsym ** table, char * name, int kind)

{

  sigsym* tmp;
  sigsym* last = NULL;

  tmp = *table;
  while (tmp != NULL)
    {
      if (!strcmp(tmp->val, name))
	return tmp;
      else
	{
	  last = tmp;
	  tmp = tmp->next;
	}
    }

  vmcheck(tmp = (sigsym*) malloc(sizeof(sigsym)));

  tmp->val = dupval(name);
  tmp->rate = UNKNOWN;
  tmp->special = 0;
  tmp->width = 1;
  tmp->res = UNKNOWN;
  tmp->kind = kind;
  tmp->vartype = SCALARTYPE;
  tmp->vol = VARIABLE;
  tmp->defnode = NULL;
  tmp->next = NULL;
  tmp->numinst = 0;
  tmp->obus = NULL;
  tmp->maxifstate = 0;
  tmp->effects = 0;
  tmp->score = 0;
  tmp->midi = 0;
  tmp->ascore = 0;
  tmp->amidi = 0;
  tmp->miditag = 0;
  tmp->dyn = 0;
  tmp->startup = 0;
  tmp->calrate = UNKNOWN;
  tmp->cref = NULL;
  tmp->tref = NULL;

  if ((*table) == NULL)
    (*table) = tmp;
  else
    last->next = tmp;

  return tmp;
}

/****************************************************************************/
/* adds a tnode-less symbol to the symbol table so table is sorted by width */
/****************************************************************************/

int addvsymsort(sigsym ** table, char * name, int kind)

{

  sigsym * tmp;
  sigsym * place = NULL;
  int val;

  val = atoi(name);
  tmp = *table;
  while (tmp != NULL)
    {
      if (val == tmp->width)
	return DUPLICATE;
      else
	{
	  if (tmp->width < val)
	    place = tmp;
	  tmp = tmp->next;
	}
    }

  vmcheck(tmp = (sigsym*) malloc(sizeof(sigsym)));

  tmp->val = dupval(name);
  tmp->rate = UNKNOWN;
  tmp->special = 0;
  tmp->width = val;
  tmp->res = UNKNOWN;
  tmp->kind = kind;
  tmp->vartype = SCALARTYPE;
  tmp->vol = VARIABLE;
  tmp->defnode = NULL;
  tmp->numinst = 0;
  tmp->obus = NULL;
  tmp->maxifstate = 0;
  tmp->effects = 0;
  tmp->score = 0;
  tmp->midi = 0;
  tmp->ascore = 0;
  tmp->amidi = 0;
  tmp->miditag = 0;
  tmp->dyn = 0;
  tmp->startup = 0;
  tmp->calrate = UNKNOWN;
  tmp->cref = NULL;
  tmp->tref = NULL;

  if (((*table) == NULL) || (place == NULL))
    {
      tmp->next = *table;
      (*table) = tmp;
    }
  else
    {
      tmp->next = place->next;
      place->next = tmp;
    }

  return INSTALLED;
}

/****************************************************************************/
/* returns a symbol to the table, given an S_IDENT tnode                    */
/****************************************************************************/

sigsym * getsym(sigsym ** table, tnode * idnode)

{

  sigsym* tmp;
  int foundit = 0;

  if (idnode->ttype != S_IDENT)
    return NULL; 

  tmp = *table;
  while ((tmp != NULL) && (!foundit))
    {
      if (!strcmp(tmp->val,idnode->val))
	foundit = 1;
      else
	tmp = tmp->next;
    }
  if (foundit)
    return tmp;
  return NULL;

}

/****************************************************************************/
/* returns a symbol to the table, given a name                              */
/****************************************************************************/

sigsym * getvsym(sigsym ** table, char *  name)

{

  sigsym* tmp = *table;
  int foundit = 0;

  while ((tmp != NULL) && (!foundit))
    {
      if (!strcmp(tmp->val, name))
	foundit = 1;
      else
	tmp = tmp->next;
    }
  if (foundit)
    return tmp;
  return NULL;

}


/****************************************************************************/
/* aborts program on error                                                  */
/****************************************************************************/

void symcheck(int symstate, tnode * tsym)


{
  if (symstate == DUPLICATE)
    {
      if (tsym != NULL)
	{
	  printf("Error: Duplicate symbol %s\n", tsym->val);
	  showerrorplace(tsym->linenum, tsym->filename);
	}
      else
	internalerror("sigsym.c","symcheck() -- Error 1");
    }
  if (symstate == SYSERROR1)
    internalerror("sigsym.c","symcheck() -- Error 2");

  if (symstate == NOTPRESENT)
    internalerror("sigsym.c","symcheck() -- Error 3");

}

/****************************************************************************/
/* deletes node from symboltable  */
/****************************************************************************/

int deletesym(sigsym ** table,sigsym * symnode)

{

  sigsym * tptr;

  tptr = *table;

  if ((tptr == NULL)||(symnode == NULL))
    return NOTPRESENT; 

  if (tptr == symnode)
    {
      *table = symnode->next;
      return DELETED;
    }

  while ((tptr->next != symnode) && (tptr->next != NULL))
    tptr = tptr->next;

  if (tptr->next == NULL)
    return NOTPRESENT; 

  tptr->next = symnode->next;
  return DELETED;

}

/****************************************************************************/
/*               reverse order of sigsym list                               */
/****************************************************************************/

sigsym * reversetable(sigsym * sptr)


{

  sigsym* retptr = NULL;
  sigsym* cptr;

  while (sptr != NULL)
    {
      if (retptr == NULL)
	{
	  retptr = sptr;
	  sptr = sptr->next;
	  retptr->next = NULL;
	}
      else
	{
	  cptr = sptr;
	  sptr = sptr->next;
	  cptr->next = retptr;
	  retptr = cptr;
	}
    }
  return retptr;

}

/****************************************************************************/
/* these routines special-purpose, for instr reordering                     */
/****************************************************************************/

/****************************************************************************/
/* find symbol-table position of last-occuring instr on ilist               */
/****************************************************************************/

sigsym * findlast(sigsym ** table, tnode * ilist)


{

  tnode * tptr = ilist;
  sigsym * lastptr = NULL;
  sigsym * iptr;
  sigsym * target;
  int after;

  while (tptr != NULL)
    {
      if (tptr->ttype == S_IDENT)  /* an instrument */
	{
	  target = getsym(table,tptr);
	  iptr = *table;
	  after = 0;
	  while (iptr != target)
	    {
	      if (iptr == lastptr)
		after = 1;
	      iptr = iptr->next;
	    }
	  if ((after)||(lastptr == NULL))
	    {
	      lastptr = target;
	    }
	}
      tptr = tptr->next;
    }
  return lastptr;

}

/****************************************************************************/
/* reorder two elements in the list if necessary                            */
/****************************************************************************/

void moveafter(sigsym ** table, sigsym * putthis, 
	       sigsym * afterthis)

{

  sigsym * iptr = *table;

  if (putthis == afterthis)
    return;

  if (iptr == putthis)
    {
      *table = putthis->next;
      putthis->next = afterthis->next;
      afterthis->next = putthis;
      return;
    }

  while (iptr != afterthis)
    {
      if (iptr->next == putthis)
	{
	  iptr->next = iptr->next->next;
	  putthis->next = afterthis->next;
	  afterthis->next = putthis;
	  return;
	}
      iptr = iptr->next;
    }
  return;


}


/****************************************************************************/
/* reorder two elements in the list if necessary                            */
/****************************************************************************/

int movebefore(sigsym ** table, sigsym * putthis, sigsym * beforethis)

{

  sigsym * iptr = *table;
  sigsym * placeforit = NULL;

  while ((iptr != beforethis)&&(iptr != NULL))
    {
      if (iptr == putthis)
	return 0;
      placeforit = iptr;
      iptr = iptr->next;
    }
  if (iptr == NULL)
    return 0;
  while ((iptr->next != putthis)&&(iptr != NULL))
    iptr = iptr->next;

  iptr->next = putthis->next;

  if (placeforit != NULL)
    {
      placeforit->next = putthis;
      putthis->next = beforethis;
    }
  else
    {
      putthis->next = beforethis;
      *table = putthis;
    }
  return 1;
}


/*****************************************************/
/* strdup not ANSI -- rewritten with different name  */
/*****************************************************/

char * dupval(char * val)

{
  char * cpy;

  if (val)
    {
      vmcheck(cpy = (char *) calloc((strlen(val)+1),sizeof(char)));
      return strcpy(cpy, val);
    }
  else
    return NULL;

}

/*****************************************************/
/* returns string with underscores postpended        */
/*****************************************************/

char * dupunderscore(char * val)

{
  char * cpy;
  char * pp = "___";

  if (val)
    {
      vmcheck(cpy = (char *) calloc((strlen(val)+strlen(pp)+1),sizeof(char)));
      strcpy(cpy, val);
      return strcat(cpy, pp);
    }
  else
    return NULL;

}

/********************************************************/
/*               makes a new tnode                      */
/********************************************************/

tnode * make_tnode(char * name, int number)

{

  tnode * retptr;
  
  vmcheck(retptr = (tnode *) malloc(sizeof(tnode)));

  retptr->val = name;

  retptr->ttype = number;
  retptr->rate = UNKNOWN;
  retptr->special = 0;
  retptr->width = 1;
  retptr->res = ASFLOAT;
  retptr->vartype = SCALARTYPE;
  retptr->vol = VARIABLE;

  retptr->sptr = NULL;  
  retptr->optr = NULL;
  retptr->dptr = NULL;
  retptr->opwidth = 0;
  retptr->staterate = UNKNOWN;
  retptr->extra = NULL;
  retptr->extrarate = UNKNOWN;

  retptr->ibus = NULL;
  retptr->arrayidx = 0;
  retptr->usesinput = 0;
  retptr->usesingroup = 0;
  retptr->time = 0;
  retptr->inwidth = 0;

  retptr->next = NULL;
  retptr->down = NULL;
  retptr->linenum = 0;
  retptr->filename = NULL;

  return retptr;
}

/********************************************************/
/*       converts string constant to signed int         */
/********************************************************/

int make_int(tnode * tptr)

{
  float f;

  if (tptr->ttype == S_NUMBER)
    {
      f = ((float)atof(tptr->val));
      if (tptr->val[0] != '-')
	return (f < ((float)(INT_MAX))) ? ((int)(f + 0.5F)) : INT_MAX;
      else
	return (f > ((float)(INT_MIN))) ? ((int)(f - 0.5F)) : INT_MIN;
    }
  return (int) atoi(tptr->val);
}

/********************************************************/
/*   checks if integer is too large for signed 32 bits  */
/********************************************************/

int largeinteger(char * s)

{
  return ((s[0] != '-') && (strlen(s) == 10) && (strcmp(s, "2147483647") > 0));
}

/*******************************************************************/
/*   determines if an instr is reachable, including effects        */
/*******************************************************************/

int reachableinstr(sigsym * iptr)

{
  int ret;

  ret = ((iptr->score) || (iptr->ascore) || (iptr->dyn) || (iptr->effects) ||
	 (iptr->startup) || (iptr->midi) || (iptr->amidi) || 
	 ((cmidi || session) && iptr->miditag) || csasl);
  return ret;
}

/*******************************************************************/
/*   determines if an instr is reachable, excepting effects        */
/*******************************************************************/

int reachableinstrexeff(sigsym * iptr)

{
  int ret;

  ret = ((iptr->score) || (iptr->ascore) || (iptr->dyn) ||
	 (iptr->startup) || (iptr->midi) || (iptr->amidi) || 
	 ((cmidi || session) && iptr->miditag) || csasl);
  return ret;
}

/*********************************************************/
/*   determines if an instr is reachable, except startup */
/*********************************************************/

int reachableinstrexstart(sigsym * iptr)

{
  int ret;

  ret = ((iptr->score) || (iptr->ascore) || (iptr->dyn) ||
	 (iptr->midi) || (iptr->amidi) || (iptr->effects) ||
	 ((cmidi || session) && iptr->miditag) || csasl);
  return ret;
}

/*******************************************************************/
/*   determines if an instr is reachable, except effects & startup */
/*******************************************************************/

int reachableinstrexeffexstart(sigsym * iptr)

{
  int ret;

  ret = ((iptr->score) || (iptr->ascore) || (iptr->dyn) ||
	 (iptr->midi) || (iptr->amidi) || 
	 ((cmidi || session) && iptr->miditag) || csasl);
  return ret;
}


