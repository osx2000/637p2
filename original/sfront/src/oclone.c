
/*
#    Sfront, a SAOL to C translator    
#    This file: Links and creates instrnametable data structure
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
/*      void installopnames() and void installdyninstr()        */
/*                                                              */
/* These top-level functions are called in linkcalls() and      */
/* instancecode in postparse.c, and clone copies of opcode      */
/* calls and dynamic instrs into instruments, instances         */
/* and the global block.                                        */
/*                                                              */
/*______________________________________________________________*/


extern sigsym * opcodelink(tnode *);

/****************************************************************/
/*            opcode name install loop                          */
/****************************************************************/

void installopnames(tnode * tptr)

{
  tnode * oplist = tptr;
  tnode * lptr;

  if (curropcalldepth++ > MAXOPCODEDEPTH)
    {
      printf("Error: Recursive opcalls involving opcode %s.\n\n",
	     tptr->val);
      showerrorplace(tptr->optr->down->linenum, 
		     tptr->optr->down->filename);
    }

  while (tptr != NULL)
    {
      tptr->optr->sptr = tptr->sptr = opcodelink(tptr);
      if ((tptr->sptr == NULL) && (!coreopcodename(tptr)))
	{
	  printf("Error: opcode %s not defined.\n", tptr->val);
	  showerrorplace(tptr->optr->down->linenum,
			 tptr->optr->down->filename);
	}
      if (tptr->sptr == NULL)
	{
	  tptr->optr->sptr = coreopcodeadd(tptr, &(tptr->sptr));
	  coreopcodevarargs(tptr);
	  hascoreopcode(tptr,1);
	}
      if (tptr->sptr == NULL)
	internalerror("oclone.c","installopnames");

      /* updates tptr->ibus to point to cloned declaration */

      if ((tptr->ttype == S_OPARRAYCALL))
	{
	  lptr = oplist;
	  while ((lptr != NULL) && (lptr->ttype == S_OPARRAYDECL) &&
		 (strcmp(lptr->val,tptr->val) != 0))
	    lptr = lptr->next;
	  if ((lptr == NULL) || (lptr->opwidth == 0))
	    {
	      printf("Error: Undeclared oparray call %s.\n\n",
		     tptr->val);
	      showerrorplace(tptr->optr->down->linenum, 
			     tptr->optr->down->filename);
	    }
	  tptr->ibus = lptr;
	}

      tptr = tptr->next;
    }
  curropcalldepth--;
}


/****************************************************************/
/*            dynamic instrument install loop                   */
/****************************************************************/

void installdyninstr(tnode * tptr)

{

  while (tptr != NULL)
    {
      tptr->dptr->sptr = tptr->sptr = getvsym(&instrnametable,tptr->val);
      if (tptr->sptr == NULL)
	{
	  printf("Error: Instr %s not defined.\n", tptr->val);
	  showerrorplace(tptr->dptr->down->next->linenum,
			 tptr->dptr->down->next->filename);
	}
      tptr->sptr->dyn = 1;
      tptr = tptr->next;
    }

}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*       tclone(), eclone(), treeclone(), and sclone()          */
/*                                                              */
/* These top-level functions used to clone structures in many   */
/*      sfront files, including installopnames() above.         */
/*                                                              */
/*______________________________________________________________*/


/********************************************************/
/*               clones a tnode list                    */
/********************************************************/

tnode * tclone(tnode * tptr)


{
  tnode * retptr = NULL;
  tnode * cptr = NULL;   /* initialization not needed */

  while (tptr != NULL)
    {
      if (retptr == NULL)
	{
	  vmcheck(retptr = cptr = (tnode*) malloc(sizeof(tnode)));
	}
      else
	{
	  vmcheck(cptr->next = (tnode*) malloc(sizeof(tnode)));
	  cptr = cptr->next;
	}
      cptr->val = dupval(tptr->val);;
      cptr->sptr = NULL;  
      cptr->ttype = tptr->ttype;
      cptr->res = tptr->res;
      cptr->rate = tptr->rate;
      cptr->special = tptr->special;
      cptr->width = tptr->width;
      cptr->vartype = tptr->vartype;
      cptr->vol = tptr->vol;
      cptr->next = NULL;
      cptr->down = NULL;
      cptr->ibus = NULL;
      cptr->arrayidx = tptr->arrayidx;
      cptr->usesinput = tptr->usesinput;
      cptr->usesingroup = tptr->usesingroup;
      cptr->optr = tptr->optr;
      if (cptr->optr != NULL)
	{
	  cptr->optr->optr = cptr;
	  cptr->optr->down->optr = cptr;
	  cptr->optr = NULL;
	}
      cptr->dptr = tptr->dptr;
      if (cptr->dptr != NULL)
	{
	  cptr->dptr->dptr = cptr;
	  cptr->dptr = NULL;
	}
      cptr->opwidth = tptr->opwidth;
      cptr->staterate = tptr->staterate;
      cptr->extra = NULL;
      cptr->extrarate = tptr->extrarate;
      cptr->time = tptr->time;
      cptr->linenum = tptr->linenum;
      cptr->filename = tptr->filename;

      tptr = tptr->next;
    }
  return retptr;

}

/********************************************************/
/*               clones an expr tnode                   */
/********************************************************/

tnode * eclone(tnode * tptr)


{
  tnode * cptr = NULL;   

  vmcheck(cptr = (tnode*) malloc(sizeof(tnode)));
  cptr->val = dupval(tptr->val);;
  cptr->sptr = tptr->sptr;  
  cptr->ttype = tptr->ttype;
  cptr->res = tptr->res;
  cptr->rate = tptr->rate;
  cptr->special = tptr->special;
  cptr->width = tptr->width;
  cptr->vartype = tptr->vartype;
  cptr->vol = tptr->vol;
  cptr->next = tptr->next;
  cptr->down = tptr->down;
  cptr->ibus = tptr->ibus;
  cptr->arrayidx = tptr->arrayidx;
  cptr->usesinput = tptr->usesinput;
  cptr->usesingroup = tptr->usesingroup;
  cptr->optr = tptr->optr;
  if (cptr->optr != NULL)
    cptr->optr->optr = cptr;
  cptr->dptr = tptr->dptr;
  if (cptr->dptr != NULL)
    {
      cptr->dptr->dptr = cptr;
    }
  cptr->opwidth = tptr->opwidth;
  cptr->staterate = tptr->staterate;
  cptr->extra = cptr->extra;
  cptr->extrarate = tptr->extrarate;
  cptr->time = tptr->time;
  cptr->linenum = tptr->linenum;
  cptr->filename = tptr->filename;

  return cptr;

}


/****************************************************************************/
/*               clones a list of sigsyms                                   */
/****************************************************************************/

sigsym * sclone(sigsym * sptr)

{

  sigsym* retptr = NULL;
  sigsym* cptr = NULL;  /* initialization not needed */

  while (sptr != NULL)
    {
      if (retptr == NULL)
	{
	  vmcheck(retptr = cptr = (sigsym*) malloc(sizeof(sigsym)));
	}
      else
	{
	  vmcheck(cptr->next = (sigsym*) malloc(sizeof(sigsym)));
	  cptr = cptr->next;
	}

      cptr->val = dupval(sptr->val);
      cptr->rate = sptr->rate;
      cptr->special = sptr->special;
      cptr->width = sptr->width;
      cptr->res = sptr->res;
      cptr->vol = sptr->vol;
      cptr->kind = sptr->kind;
      cptr->vartype = sptr->vartype;
      cptr->defnode = sptr->defnode;
      if (sptr->defnode != NULL)
	{
	  cptr->defnode->sptr = cptr;
	}
      cptr->next = NULL;
      cptr->numinst = sptr->numinst;
      cptr->obus = NULL;
      cptr->maxifstate = sptr->maxifstate;
      cptr->effects = sptr->effects;
      cptr->score = sptr->score;
      cptr->ascore = sptr->ascore;
      cptr->midi = sptr->midi;
      cptr->amidi = sptr->amidi;
      cptr->miditag = sptr->miditag;
      cptr->dyn = sptr->dyn;
      cptr->startup = sptr->startup;
      cptr->calrate = sptr->calrate;
      cptr->cref = NULL;
      cptr->tref = NULL;
      sptr = sptr->next;
    }
  return retptr;

}


extern tnode * dptrmatch(tnode *, tnode *);
extern tnode * optrmatch(tnode *, tnode *);

/********************************************************/
/*               clones a tnode list                    */
/********************************************************/

tnode * treeclone(tnode * tptr, sigsym ** matchlist, int dosub)

{
  tnode * retptr = NULL;
  tnode * cptr = NULL; /* initialization not needed */
  tnode * lptr;
  sigsym * match;

  while (tptr != NULL)
    {
      if (retptr == NULL)
	{
	  vmcheck(retptr = cptr = (tnode*) malloc(sizeof(tnode)));
	}
      else
	{
	  vmcheck(cptr->next = (tnode*) malloc(sizeof(tnode)));
	  cptr = cptr->next;
	}
      cptr->down = treeclone(tptr->down, matchlist, dosub);

      cptr->vartype = tptr->vartype;
      cptr->sptr = NULL;

      /* special case of locsymtable definition */

      if ((tptr->sptr != NULL) && (tptr->sptr->defnode == tptr))
	{
	  tptr->sptr->defnode = cptr;
	  cptr->sptr = tptr->sptr;

	  /* patch up original symbol table pointer */
	  
	  tptr->sptr = getvsym(&tlocsymtable,tptr->sptr->val);

	}

      cptr->special = tptr->special;
      cptr->width = tptr->width;
      cptr->ibus = NULL;
      cptr->arrayidx = tptr->arrayidx;
      cptr->usesinput = tptr->usesinput;
      cptr->usesingroup = tptr->usesingroup;
      cptr->dptr = tptr->dptr;
      if (cptr->dptr != NULL)
	{
	  cptr->dptr->dptr = cptr;

	  /* patch original table pointer */

	  tptr->dptr = dptrmatch(tlocdyncalls, tptr);

	}
      cptr->optr = tptr->optr;

      if ((cptr->optr == &maplistopcall) ||     /* maplist calls */ 
	  (cptr->optr == &maplistoparraycall))
	{
	  if (tptr->down == NULL)
	    {
	      if (locopcodecalls == NULL)
		{
		  if (cptr->optr == &maplistopcall)
		    locopcodecalls = make_tnode(tptr->val, S_OPCALL);
		  else
		    locopcodecalls = make_tnode(tptr->val,
						S_OPARRAYCALL);
		  cptr->optr = locopcodecalls;
		  locopcodecalls->optr = cptr;
		}
	      else
		{
		  lptr = locopcodecalls;
		  while (lptr->next != NULL)
		    lptr = lptr->next;
		  if (cptr->optr == &maplistopcall)
		    lptr->next = make_tnode(tptr->val, S_OPCALL);
		  else
		    lptr->next = make_tnode(tptr->val, S_OPARRAYCALL);
		  lptr = lptr->next;	      
		  cptr->optr = lptr;
		  lptr->optr = cptr;
		}
	    }
	  else
	    {
	      lptr = locopcodecalls;
	      while (lptr != NULL)
		{
		  if (lptr->optr == cptr->down)
		    {
		      lptr->optr = cptr;
		      cptr->optr = lptr;
		      lptr = NULL;
		    }
		  else
		    lptr = lptr->next;
		}
	    }
	}
      else
	if (cptr->optr != NULL)
	  {
	    
	    /* patch original table pointer*/
	    
	    tptr->optr = optrmatch(tlocopcodecalls, tptr); 
	    
	    if (cptr->down != NULL) /* if its the retptr of the opcode call */
	      {
		tptr->down->optr = tptr->optr;  /* patch original table */
		cptr->optr->optr = cptr;  /* update locsymtable pointer */
		if (cptr->down->next->ttype == S_LB)  /* oparray */
		  {
		    lptr = locopcodecalls;
		    while ((lptr != NULL) && (lptr->ttype == S_OPARRAYDECL) &&
			   (strcmp(lptr->val, cptr->down->val) != 0))
		      lptr = lptr->next;
		    if ((lptr == NULL) || (lptr->opwidth == 0))
		      {
			printf("Error: Undeclared oparray call %s\n",
			       cptr->down->val);
			showerrorplace(cptr->down->linenum,
				       cptr->down->filename);
		      }
		    cptr->optr->ibus = lptr;
		  }
	      }
	  }

      cptr->opwidth = tptr->opwidth;
      cptr->staterate = tptr->staterate;
      cptr->extra = NULL;
      cptr->extrarate = tptr->extrarate;
      cptr->time = tptr->time;
      cptr->linenum = tptr->linenum;
      cptr->filename = tptr->filename;
      cptr->next = NULL;

      match = getsym(matchlist,tptr);
      if ( (match == NULL) || (!dosub) )
	{
	  cptr->val = dupval(tptr->val);
	  cptr->ttype = tptr->ttype;
	  cptr->res = tptr->res;
	  cptr->rate = tptr->rate;
	  cptr->vol = tptr->vol;
	}
      else
	{
	  switch (match->defnode->down->ttype) {
	  case S_IDENT:
	    if (match->defnode->down->next == NULL)   /* simple identifier */
	      {
		cptr->val = dupval(match->defnode->down->val);
		cptr->ttype = match->defnode->down->ttype;
		cptr->res = match->defnode->down->res;
		cptr->rate = match->defnode->down->rate;
		cptr->vol = match->defnode->down->vol;
	      }
	    else
	      {
		if ((tptr->next != NULL)||(tptr->down != NULL))
		  {
		    printf("Error: Inappropriate template substitution.\n");
		    showerrorplace(tptr->linenum, tptr->filename);
		  }
		if (match->defnode->down->next->ttype == S_LB)
		  {
		    if (match->defnode->down->next->next->next->next == NULL)
		      {
			/* indexed array */

			cptr->val = dupval(match->defnode->down->val);
			cptr->ttype = match->defnode->down->ttype;
			cptr->res = match->defnode->down->res;
			cptr->rate = match->defnode->down->rate;
			cptr->vol = match->defnode->down->vol;
			cptr->next = treeclone(
			       match->defnode->down->next, matchlist, NOSUB);
		      }
		    else
		      {
			/* oparray call */

			cptr->val = "(";
			cptr->ttype = S_LP;
			cptr->next = treeclone(match->defnode,
					       matchlist, NOSUB);
			cptr = cptr->next;
			cptr->next = make_tnode(")",S_RP);
			cptr = cptr->next;
		      }
		  }
		else
		  {
		    /* opcode call */

		    cptr->val = "(";
		    cptr->ttype = S_LP;
		    cptr->next = treeclone(match->defnode, matchlist, NOSUB);
		    cptr = cptr->next;
		    cptr->next = make_tnode(")",S_RP);
		    cptr = cptr->next;
		  }
	      }
	    break;
	  case S_NUMBER:
	  case S_INTGR:
	    cptr->val = dupval(match->defnode->down->val);
	    cptr->ttype = match->defnode->down->ttype;
	    cptr->res = match->defnode->down->res;
	    cptr->rate = match->defnode->down->rate;
	    cptr->vol = match->defnode->down->vol;
	    break;
	  default:
	    if ((tptr->next != NULL)||(tptr->down != NULL))
	      {
		printf("Error: Inappropriate template");
		printf(" substitution.\n");
		showerrorplace(tptr->linenum, tptr->filename);
	      }

	    /* expression */

	    cptr->val = "(";
	    cptr->ttype = S_LP;
	    cptr->next = treeclone(match->defnode, matchlist, NOSUB);
	    cptr = cptr->next;
	    cptr->next = make_tnode(")",S_RP);
	    cptr = cptr->next;
	    break;
	  }
	}

      tptr = tptr->next;
    }
  return retptr;

}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*                                                              */
/*                   Utility functions                          */
/*                                                              */
/*______________________________________________________________*/

/****************************************************************/
/*            creates cloned opcode symbol table                */
/****************************************************************/


sigsym * opcodelink(tnode * tptr)

{
  sigsym * opdef;
  sigsym * newsym = NULL;
  sigsym * templatevars = NULL;
  tnode * t_code;

  if ((opdef = getvsym(&opcodenametable,tptr->val)) == NULL)
    return NULL;

  /* create clone of opcode table entry */

  addvsym(&newsym, tptr->val, opdef->kind);

  newsym->rate  = opdef->rate;
  newsym->kind  = opdef->kind;
  newsym->width = opdef->width;
  newsym->maxifstate = opdef->maxifstate;

  /* create clone of opcode definition */

  newsym->defnode = make_tnode("<opcodedecl>",S_OPCODEDECL);

  newsym->defnode->sptr = locsymtable = 
    sclone(tlocsymtable = opdef->defnode->sptr);
  locopcodecalls = tclone(tlocopcodecalls = opdef->defnode->optr);
  locdyncalls = tclone(tlocdyncalls = opdef->defnode->dptr);
  newsym->defnode->down = treeclone(opdef->defnode->down,&templatevars,DOSUB);
  newsym->defnode->optr = locopcodecalls;
  newsym->defnode->dptr = locdyncalls;

  /* hook up variable declarations */

  t_code = newsym->defnode->down
    ->next->next->next->next->next->next->down;
  varupdate(t_code,&locsymtable);

  /* hook up main code block */

  t_code = newsym->defnode->down
    ->next->next->next->next->next->next->next->down;
  varupdate(t_code,&locsymtable);

  /* hook up pfield entries -- varupdate won't do this correctly */

  t_code = newsym->defnode->down->next->next->next->down;
  while (t_code != NULL)
    {
      if (t_code->ttype == S_PARAMDECL)
	{
	  t_code->sptr = t_code->down->next->sptr =
	    t_code->down->next->down->sptr = 
	    getsym(&locsymtable, t_code->down->next->down);
	  t_code->sptr->defnode = t_code->down->next->down;
	}
      t_code = t_code->next;
    }

  installopnames(newsym->defnode->optr);
  installdyninstr(newsym->defnode->dptr);

  return newsym;
}

/********************************************************/
/*    return table entry whose dptr matches target      */
/********************************************************/

tnode * dptrmatch(tnode * tptr, tnode * target)

{

  while (tptr != NULL)
    {
      if (tptr->dptr == target)
	return tptr;
      tptr = tptr->next;
    }
  return tptr;

}

/********************************************************/
/*    return table entry whose optr matches target      */
/********************************************************/

tnode * optrmatch(tnode * tptr, tnode * target)

{
  while (tptr != NULL)
    {
      if (tptr->optr == target)
	{
	  return tptr;
	}
      tptr = tptr->next;
    }
  return tptr;

}





