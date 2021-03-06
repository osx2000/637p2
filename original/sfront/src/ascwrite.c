
/*
#    Sfront, a SAOL to C translator    
#    This file: Writes ascii SAOL and SASL files
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

/******************************************************************/
/*               writes saol tokens -- recursive                  */
/******************************************************************/

void ascsaoltreewrite(tnode * tbranch, int * indent)

{
  int i;
  sigsym * sptr;

  while (tbranch != NULL)
    {
      if (tbranch->down != NULL)
	ascsaoltreewrite(tbranch->down, indent);
      else
	{
	  if ((tbranch->ttype < 0xFF)||(tbranch->ttype == S_PRINTF))
	    {
	      switch(tbranch->ttype) {  /* space before */
	      case S_NUMBER:
	      case S_INTGR:
		fprintf(orcoutfile," ");
		break;
	      case S_STRCONST:
		fprintf(orcoutfile," \"");
		break;
	      case S_LC:
		*indent += 2;
		break;
	      case S_RC:
		*indent -= 2;
		break;
	      default:
		break;
	      }
	      if (bitsymin)
		{
		  sptr = getvsym(&bitsymin,tbranch->val);
		  if (sptr)
		    fprintf(orcoutfile,"%s",sptr->defnode->val);
		  else
		    fprintf(orcoutfile,"%s",tbranch->val);
		}
	      else
		fprintf(orcoutfile,"%s",tbranch->val);
	      switch(tbranch->ttype) {  /* space after */
	      case S_NUMBER:
	      case S_INTGR:
	      case S_INSTR:
	      case S_AOPCODE:
	      case S_KOPCODE:
	      case S_IOPCODE:
	      case S_OPCODE:
	      case S_ASIG:
	      case S_KSIG:
	      case S_IVAR:
	      case S_XSIG:
	      case S_OPARRAY:
	      case S_TABLE:
	      case S_TABLEMAP:
	      case S_IMPORTS:
	      case S_EXPORTS:
		fprintf(orcoutfile," ");
		break;
	      case S_STRCONST:
		fprintf(orcoutfile,"\" ");
	      default:
		break;
	      }

	      switch(tbranch->ttype) { /* <CR>'s after */
	      case S_SEM:
	      case S_LC:
		fprintf(orcoutfile,"\n");
		for (i = 0; i<*indent; i++)
		  fprintf(orcoutfile," ");
		break;
	      case S_RC:
		fprintf(orcoutfile,"\n\n");
		for (i = 0;i<*indent; i++)
		  fprintf(orcoutfile," ");
		break;
	      default:
		break;
	      }
	    }
	}
      tbranch = tbranch->next;
    }
}


/******************************************************************/
/*               writes saol tokens -- wrapper                  */
/******************************************************************/

void ascsaolwrite (void)

{
  int indent = 0;
  tnode * saolroot, * tptr;

  fprintf(orcoutfile,"\n//\n// automatically generated by sfront\n//\n\n");

  if (ascsaolptree)
    ascsaoltreewrite(troot,&indent);
  else
    {
      if (saolfilelist == NULL)
	readprepare(BINORC);
      else
	{
	  currsaolfile = saolfilelist;
	  saollinenumber = 1;
	  saolsourcefile = currsaolfile->val;
	  if (currsaolfile->filename)
	    saolfile = fopen(currsaolfile->filename,"r");
	  else
	    saolfile = fopen(currsaolfile->val,"r");
	}
      
      yylex();
      saolroot = tptr = yylval;
      while (yylex()) 
	{
	  tptr->next = yylval;
	  tptr = yylval;
	}
      ascsaoltreewrite(saolroot,&indent);
    }

}

/******************************************************************/
/*                       writes sasl line                         */
/******************************************************************/

void slinewrite(tnode * tptr)

{
  sigsym * sptr;

  while (tptr != NULL)
    {
      if (bitsymin)
	{
	  sptr = getvsym(&bitsymin, tptr->val);
	  if (sptr)
	    fprintf(scooutfile," %s ",sptr->defnode->val);
	  else
	    fprintf(scooutfile," %s ",tptr->val);
	}
      else
	fprintf(scooutfile, " %s ", tptr->val);
      tptr = tptr->next;
    }
  fprintf(scooutfile, "\n");
      
}


/******************************************************************/
/*                       writes sasl file                         */
/******************************************************************/

void ascsaslwrite(sasdata * sdata)

{
  tnode * tptr;
  
  for (tptr = sdata->temporoot; tptr != NULL; tptr = tptr->next)
    slinewrite(tptr->down);

  for (tptr = sdata->tableroot; tptr != NULL; tptr = tptr->next)
    slinewrite(tptr->down);

  for (tptr = sdata->controlroot; tptr != NULL; tptr = tptr->next)
    slinewrite(tptr->down);

  for (tptr = sdata->instrroot; tptr != NULL; tptr = tptr->next)
    slinewrite(tptr->down);

  if (sdata->endtimeval != NULL)
    fprintf(scooutfile, "%s end\n", sdata->endtimeval);

}
