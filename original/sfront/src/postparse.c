
/*
#    Sfront, a SAOL to C translator    
#    This file: SAOL parsing and post-parse linking
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

extern void doparse(void);
extern void globalparamcheck(void);
extern void globalvariablecheck(void);
extern void businitcheck(void);
extern void globalstatementcheck(void);
extern void globalimportscheck(void);
extern void linkcalls(void);
extern void mainscoreread(void);
extern void mainrecursioncheck(void);
extern void seqloopdetect(void);
extern void instrorder(void);
extern void instanceorder(void);
extern void buswidth(void); 
extern void inbusinstall(void);
extern void outbusinstall(void);
extern void sendoutputflag(void);
extern void instanceclone(void);


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*                   void saolparse()                           */
/*                                                              */
/* This is the top-level function for parsing SAOL programs,    */
/* called in the main() of sfront located in sfmain.c. This     */
/* function parses the program, and does post-parse linking.    */
/*                                                              */
/*______________________________________________________________*/


/****************************************************************/
/*                    parses all SAOL files                     */
/****************************************************************/


void saolparse(void)
{

  doparse();              /* parse SAOL ASCII/mp4 file(s)       */

  /*******************/
  /* syntax checking */
  /*******************/

  globalparamcheck();     /* syntax-check global { } params     */
  globalvariablecheck();  /* syntax-check global { } variables  */
  businitcheck();         /* initialize audio bus variables     */
  globalstatementcheck(); /* syntax-check global { } statements */
  globalimportscheck();   /* matches local & global tables/vars */

  /************************************/
  /* data-structure creation/checking */
  /************************************/

  linkcalls();            /* link up opcode and dynamic instrs  */ 
  mainscoreread();        /* read static score from all sources */
  mainrecursioncheck();   /* check for recursive opcode calls   */
  seqloopdetect();        /* check for sequence statement loop  */

  instrorder();           /* put instruments in correct order   */
  instanceorder();        /* put instances in correct order   */
  buswidth();             /* compute the width of all buses     */

  inbusinstall();         /* install input buses                */
  outbusinstall();        /* install output buses               */
  sendoutputflag();       /* flags instrs that write outputbus  */
  instanceclone();        /* clones instances                   */

}


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*               functions called by saolparse()                */
/*                                                              */
/* these second-level functions handle the major phases of      */
/*       parsing and post-parse linking and cleanup.            */
/*                                                              */
/*______________________________________________________________*/


/*******************************************************************/
/*          call bison-created function to parse SAOL code         */
/*******************************************************************/

void doparse(void)

{

  /* open (first) file for parsing */

  if (saolfilelist)
    {
      /* ASCII SAOL file(s) */

      currsaolfile = saolfilelist;
      saollinenumber = 1;
      saolsourcefile = currsaolfile->val;
      if (currsaolfile->filename)
	saolfile = fopen(currsaolfile->filename,"r");
      else
	saolfile = fopen(currsaolfile->val,"r");    
    }
  else
    readprepare(BINORC);    /* SAOL in binary .mp4 file */
    
  /* do the parse -- yyparse() created by bison, from parser.y */

  if (1 == yyparse ())
    internalerror("postparse.c","saolparse()");
}


/****************************************************************/
/*           does range checks for global parameters            */
/****************************************************************/

void globalparamcheck(void)

{
  int i, driver_srate, driver_inchannels;

  if ((srate < 0) || (inchannels < 0))
    {
      driver_srate = -(srate < 0);
      driver_inchannels = -(inchannels < 0);
      if (getvsym(&busnametable,"input_bus"))
	makeainparams(ain, &driver_srate, &driver_inchannels);
      if (inchannels < 0)
	{
	  inchannels = (driver_inchannels > 0) ?  driver_inchannels : 0;
	  if ((!inchannels) && getvsym(&busnametable,"input_bus"))
	    printf("Warning: input_bus will be set to silence.\n");
	}
      if (srate < 0)
	srate = (driver_srate > 0) ? driver_srate : 32000;
    }

  if (outchannels <= 0)
    {
      if (outchannels == 0)
	printf("Warning: Setting global parameter outchannel to 1.\n");
      outchannels = 1;
    }

  if (interp_cmdline >= 0)
    interp = interp_cmdline;    /* command-line flag override */
  else
    if (interp < 0)
      interp = INTERP_LINEAR;   /* SA default is linear interpolation */

  if (krate < 0)
    krate = 100;

  if (krate > srate)
    {
      printf("Error: Global parameter krate > srate.\n");
      noerrorplace();
    }

  saol_krate = krate;

  if (srate % krate)
    while (srate % (++krate));

  if (saol_krate != krate)
    printf("Warning: krate changed from %i to %i to evenly divide srate.\n",
	   saol_krate, krate);

  /* computes next power of two above acycle, < 8192 */

  i = 1;
  twocycle = srate/krate;
  while ((i *= 2) < twocycle);
  if (i > twocycle)
    twocycle = i;
  if (twocycle > 8192)
    twocycle = 8192;

}

/****************************************************************/
/*      does range checks for global variables                  */
/****************************************************************/

void globalvariablecheck(void)

{
  sigsym * sptr;

  sptr = globalsymtable;
  while (sptr != NULL)
    {
      if (sptr->width == INCHANNELSWIDTH)
	{
	  if (inchannels == 0)
	    {
	      printf("Error: Declaring zero-sized arrays using inchannels.\n");
	      showerrorplace(sptr->defnode->linenum, sptr->defnode->filename);
	    }
	  sptr->width = inchannels;
	}
      if (sptr->width == OUTCHANNELSWIDTH)
	{
	  if (outchannels == 0)
	    {
	      printf("Error: Declaring zero-sized arrays using outchannels.\n"
		     );
	      showerrorplace(sptr->defnode->linenum, sptr->defnode->filename);
	    }
	  sptr->width = outchannels;
	}
      sptr = sptr->next;
    }
}

/****************************************************************/
/*      does range checks for bus-related variables             */
/****************************************************************/

void businitcheck(void)

{  
  sigsym * sptr, * inbus;
  tnode * tptr, * gptr;

  /********************************************************/
  /* add output bus to busnametable if not there already  */
  /********************************************************/

  if (addvsym(&busnametable,"output_bus", K_BUSNAME)==INSTALLED)
    {
      busnametable->defnode = NULL;
    }
  outputbus = getvsym(&busnametable,"output_bus");
  outputbus->width = outchannels;

  /**************************/
  /* specify input_bus size */
  /**************************/

  inbus = getvsym(&busnametable,"input_bus");
  if (inbus != NULL)
    inbus->width = inchannels;

  /****************************/
  /* syntax check outbustable */
  /****************************/

  tptr = outbustable;
  while (tptr != NULL)
    {
      tptr->down->sptr = tptr->sptr = getvsym(&busnametable,tptr->val);
      if (tptr->sptr == NULL)
	{
	  printf("Error: Outbus statement references undefined bus.\n");
	  showerrorplace(tptr->down->down->linenum,
			 tptr->down->down->filename);
	}
      if (tptr->sptr == inbus)
	{
	  printf("Error: Outbus statement references input_bus.\n");
	  showerrorplace(tptr->down->down->linenum,
			 tptr->down->down->filename);
	}
      tptr = tptr->next;
    }

  /***********************************************/
  /* assume all instruments route to outchannel  */
  /***********************************************/

  sptr = instrnametable;
  while (sptr != NULL)
    {
      sptr->width = outchannels;
      sptr->outputbus = 1;
      sptr = sptr->next;
    }

  /***********************************/
  /* record explicitly defined buses */
  /***********************************/

  gptr = groot;
  while (gptr != NULL)
    {
      if (gptr->ttype == S_SENDDEF)
	{
	  tptr = gptr->down->next->next->next->next->next->next->down;

	  while (tptr)
	    {
	      if ((tptr->ttype == S_NAME) && (tptr->down->next))
		{
		  switch (tptr->down->next->next->ttype) {
		  case S_INTGR:
		    if (tptr->sptr->width && 
			(tptr->sptr->width != atoi(tptr->down->next->next->val)))
		      {
			printf("Error: Bus %s width declaration mismatch.\n",
			       tptr->sptr->val);
			showerrorplace(tptr->down->linenum, tptr->down->filename);
		      }
		    tptr->sptr->width = atoi(tptr->down->next->next->val);
		    break;
		  case S_INCHANNELS:
		    if (!inchannels)
		      {
			printf("Error: Zero-width bus %s (via inchannels).\n",
			       tptr->sptr->val);
			showerrorplace(tptr->down->linenum, tptr->down->filename);
		      }
		    if (tptr->sptr->width && (tptr->sptr->width != inchannels))
		      {
			printf("Error: Bus %s width declaration mismatch.\n",
			       tptr->sptr->val);
			showerrorplace(tptr->down->linenum, tptr->down->filename);
		      }
		    tptr->sptr->width = inchannels;
		    break;
		  case S_OUTCHANNELS:
		    if (!outchannels)
		      {
			printf("Error: Zero-width bus %s (via outchannels).\n",
			       tptr->sptr->val);
			showerrorplace(tptr->down->linenum, tptr->down->filename);
		      }
		    if (tptr->sptr->width && (tptr->sptr->width != outchannels))
		      {
			printf("Error: Bus %s width declaration mismatch.\n",
			       tptr->sptr->val);
			showerrorplace(tptr->down->linenum, tptr->down->filename);
		      }
		    tptr->sptr->width = outchannels;
		    break;
		  }
		}
	      tptr = tptr->next;
	    }
	}
      gptr = gptr->next;
    }

}


extern void varupdate(tnode *, sigsym **);

/****************************************************************/
/*      does syntax checks for statements in global block       */
/*                                                              */
/*      ROUTE LP IDENT COM identlist RP SEM                     */
/*               (bus)     (instrs)                             */
/*                                                              */
/*      SEND LP IDENT SEM exprlist SEM identlist RP SEM         */
/*              (instr)    (params)     (buses)                 */
/*                                                              */
/*      SEQUENCE LP identlist RP SEM                            */
/*      TABLE IDENT LP IDENT COM exprstrlist RP                 */
/*                                                              */
/****************************************************************/

void globalstatementcheck(void)

{  
  int i;
  tnode * tptr, * iptr;
  tnode * gptr;
  sigsym * sptr;

  gptr = groot;
  while (gptr != NULL)
    {
      switch (gptr->ttype) {
      case S_ROUTEDEF:  

	/* sptr now points to bus it sums onto */

	gptr->sptr = getsym(&busnametable,gptr->down->next->next);
	if (gptr->sptr == NULL)
	  {
	    printf("Error: Undefined bus %s used in route statement.\n",
		   gptr->down->next->next->val);
	    showerrorplace(gptr->down->linenum,gptr->down->filename);
	  }

	i = 0;
	iptr = NULL;
	tptr = gptr->down->next->next->next->next->down; /* instr list */

	while (tptr != NULL)
	  {
	    if (tptr->ttype == S_IDENT)
	      {
		/* sptr now points to instrument it routes from */

		tptr->sptr = getsym(&instrnametable,tptr);
		if (tptr->sptr == NULL)
		  {
		    printf("Error: Undefined instrument in route statement.\n");
		    showerrorplace(gptr->down->linenum,gptr->down->filename);
		  }

		/* state variables for instr width resolving */

		if ((i++) == 0)
		  iptr = tptr;
		else
		  if (iptr)
		    iptr = (iptr->sptr == tptr->sptr) ? iptr : NULL;
		
		tptr->sptr->width = OUTCHANNELSWIDTH;
		tptr->sptr->outputbus = 0;
	      }

	    tptr = tptr->next;
	  }

	/* if instr field suitable, and bus width known, resolve */

	if (iptr && gptr->sptr->width && !(gptr->sptr->width % i))
	  {
	    iptr->sptr->width = gptr->sptr->width/i;
	    iptr->sptr->outputbus = (gptr->sptr == outputbus);
	  }

	break;
      case S_SENDDEF:    

	/* sptr now points to instrument it creates */

	gptr->sptr =  getsym(&instrnametable,gptr->down->next->next);
	if (gptr->sptr == NULL)
	  {
	    printf("Error: Undefined instrument in send statement.\n");
	    showerrorplace(gptr->down->linenum,gptr->down->filename);
	  }
	gptr->sptr->effects++;
	tptr = gptr->down->next->next->next->next->down;
	while (tptr != NULL)
	  {
	    if (tptr->ttype == S_EXPR)
	      varupdate(tptr,&globalsymtable);
	    tptr = tptr->next;
	  }
	break;
      case S_SEQDEF:                 
	tptr = gptr->down->next->next->down;
	i = 0;
	while (tptr != NULL)
	  {
	    if (tptr->ttype == S_IDENT)
	      {
		i++;
		sptr = getsym(&instrnametable,tptr);
		if (sptr == NULL)
		  {
		    printf("Error: Undefined instr in sequence statement.\n");
		    showerrorplace(gptr->down->linenum,gptr->down->filename);
		  }
	      }
	    tptr = tptr->next;
	  }
	if (i<2)
	  {
	    printf("Error: Sequence statement has < 2 instruments.\n");
	    showerrorplace(gptr->down->linenum,gptr->down->filename);
	  }
	break;
      case S_TABLE:
	tptr = gptr->down->next->next->next->next->next->down;
	while (tptr != NULL)
	  {
	    if (tptr->ttype == S_EXPR)
	      varupdate(tptr,&globalsymtable);
	    tptr = tptr->next;
	  }
	break;
      default:
	break;
      }
      gptr = gptr->next;
    }
}


extern void checktable(sigsym *);
extern void checktableimport(sigsym *);

/****************************************************************/
/*  syntax checks global tables against instr/op import/export  */
/****************************************************************/

void globalimportscheck(void)

{  
  sigsym * sptr;

  /* check tables */

  checktable(instrnametable);
  checktable(opcodenametable);
  checktableimport(instrnametable);
  checktableimport(opcodenametable);

  /* cull imports variables -- only used for cin right now */

  sptr = targetsymtable;
  while (sptr)
    {
      if (getsym(&globalsymtable,sptr->defnode))
	deletesym(&targetsymtable, sptr);
      sptr = sptr->next;
    }
}


/****************************************************************/
/*      do cloning for instrument opcode and dyn instr calls    */
/****************************************************************/

void linkcalls(void)

{  
  sigsym * sptr;
  tnode * tptr;

  sptr = instrnametable;
  while (sptr != NULL)
    {
      installopnames(sptr->defnode->optr);
      installdyninstr(sptr->defnode->dptr);
      sptr = sptr->next;
    }

  installopnames(globalopcodecalls);
  tptr = globalopcodecalls;
  while (tptr != NULL)
    {
      if ((tptr->sptr->rate == KRATETYPE)||(tptr->sptr->rate == ARATETYPE))
	{
	  printf("Error: Only i-rate opcodes permitted in global block.\n");
	  showerrorplace(tptr->optr->down->linenum,
			 tptr->optr->down->filename);
	}
      tptr->rate = IRATETYPE;
      tptr = tptr->next;
    }
}


/****************************************************************/
/*      read static score information from all sources          */
/****************************************************************/

void mainscoreread(void)

{

  if (bitfile != NULL)
    {
      readscore(BCONFSCORE);
      if (scooutfile)
	ascsaslwrite(confsasl);

      readmidi(confmidi, confsasl, BCONFMIDI);
      if (midoutfile)
	binmidiwrite(BCONFMIDI);

      if (bitreadaccessunits)
	readscore(BSSTRSCORE);  /* handles streaming SASL and MIDI */
      if (scooutfile)
	ascsaslwrite(sstrsasl);
    }
  else
    {
      if (saslfile != NULL)
	{
	  readscore(FCONFSCORE);
	  if (scooutfile)
	    ascsaslwrite(confsasl);
	}
      if (midifile != NULL)
	{
	  readmidi(confmidi, confsasl, FCONFMIDI);
	  if (midoutfile)
	    binmidiwrite(FCONFMIDI);
	}
      if (sstrfile != NULL)
	{
	  readscore(FSSTRSCORE);
	  if (scooutfile)
	    ascsaslwrite(sstrsasl);
	}
      if (mstrfile != NULL)
	{
	  readmidi(sstrmidi, abssasl, FSSTRMIDI);
	  if (midoutfile && !confmidi)
	    binmidiwrite(FSSTRMIDI);
	  if (sstrmidi->midinumchan > 16)
	    {
	      printf("Warning: Too many -mstr MIDI channels, using\n");
	      printf("         first 16 out of %i\n\n",sstrmidi->midinumchan);
	    }
	}
    }
}


extern void checkopcoderecursion(sigsym *);

/****************************************************************/
/*      check for recursive opcode calls, set special flag      */
/****************************************************************/

void mainrecursioncheck(void)

{
  sigsym * sptr;
  tnode * tptr;

  sptr = instrnametable;
  while (sptr != NULL)
    {
      checkopcoderecursion(sptr);
      sptr->special = specialupdate(sptr->defnode->down->
		  next->next->next->next->next->next->next->next->down);
      sptr = sptr->next;
    }
  
  tptr = globalopcodecalls;
  while (tptr != NULL)
    {
      checkopcoderecursion(tptr->sptr);
      tptr->sptr->special = tptr->special = 
	specialupdate(tptr->sptr->defnode->down->
		      next->next->next->next->next->next->next->next->down);
      tptr = tptr->next;
    }
}


extern void dotree(sigsym *);

/****************************************************************/
/*         does loop detection  on sequence statements          */
/****************************************************************/

void seqloopdetect(void)

{

  sigsym * seqinstrlist = NULL;
  sigsym * lastsym;
  sigsym * currsym;
  tnode * seqptr;
  tnode * tptr;
  tnode * lptr;

  /* first, build graph */

  seqptr = groot;
  while (seqptr != NULL) /* SEQUENCE LP identlist RP SEM */
    {
      if (seqptr->ttype == S_SEQDEF)
	{
	  tptr = seqptr->down->next->next->down;
	  lastsym = NULL; 
	  while (tptr != NULL)
	    {
	      if (tptr->ttype == S_IDENT)
		{
		  addvsym(&seqinstrlist,tptr->val,S_IDENT);
		  currsym = getsym(&seqinstrlist,tptr);
		  if ((currsym != NULL) && (lastsym != NULL))
		    {
		      if (lastsym->defnode == NULL)
			{
			  lastsym->defnode=make_tnode("link", S_INSTANCE);
			  lastsym->defnode->sptr = currsym;
			}
		      else
			{
			  lptr = lastsym->defnode;
			  while (lptr->next != NULL)
			    lptr = lptr->next;
			  lptr->next = make_tnode("link", S_INSTANCE);
			  lptr->next->sptr = currsym;
			}
		    }
		  lastsym = currsym;
		}
	      tptr = tptr->next;
	    }
	}
      seqptr = seqptr->next;
    }

  currsym = seqinstrlist;
  while (currsym != NULL)
    {
      lastsym = seqinstrlist;
      while (lastsym != NULL)
	{
	  lastsym->res = 0;
	  lastsym = lastsym->next;
	}
      dotree(currsym);
      if (currsym->res == 1)
	{
	  printf("Error: Sequence statements have an explicit loop.\n");
	  printf("       Look in global block to find the error.\n");
	  noerrorplace();
	}
      currsym = currsym->next;
    }
}


/****************************************************************/
/*   re-orders instrnametable to indicate execution ordering    */
/*                                                              */
/*      ROUTE LP IDENT COM identlist RP SEM                     */
/*               (bus)     (instrs)                             */
/*                                                              */
/*      SEND LP IDENT SEM exprlist SEM identlist RP SEM         */
/*              (instr)    (params)     (buses)                 */
/*                                                              */
/*      SEQUENCE LP identlist RP SEM                            */
/*                                                              */
/****************************************************************/

void instrorder(void)  

{
  sigsym * outputbusinstr = NULL;
  tnode * sendptr;
  tnode * routeptr;
  tnode * seqptr;
  tnode * blist;
  sigsym * lastinstr;
  sigsym * iptr;
  int changed, passcount;

  /* first, take instrument being sent output_bus off list */

  if (outputbusinstance)
    {
      outputbusinstr = getvsym(&instrnametable, outputbusinstance->val);
      if (!outputbusinstr)
	internalerror("postparse.c","instrorder() -- outputbusinstr");
      deletesym(&instrnametable,outputbusinstr);
    }

  /* as well as startup instrument */
  /* do "instruments spawned by startup" later */

  if ((startupinstr = getvsym(&instrnametable,"startup")))
    {
      startupinstr->startup = 1;
      deletesym(&instrnametable,startupinstr);
    }

  /* do send - route ordering */

  sendptr = groot;

  while (sendptr) 
    {
      if (sendptr->ttype == S_SENDDEF)
	{
	  blist = sendptr->down->next->next->next->next->next->next->down;
	  while (blist)
	    {
	      if ((blist->ttype == S_NAME) &&               /* a bus that       */
		  (strcmp(blist->down->val,"output_bus")))  /* isn't output_bus */
		{         
		  routeptr = groot;
		  while (routeptr)
		    {
		      if ((routeptr->ttype == S_ROUTEDEF) &&
			  (!strcmp(routeptr->down->next->next->val,
				   blist->down->val)))
			{
			  /* found a route statement routing a bus sent by */
			  /* the send statement */

			  /* find position of last instr in list */
			  lastinstr = findlast(&instrnametable,
			    routeptr->down->next->next->next->next->down);

			  /* put send instr after this if its before */

			  moveafter(&instrnametable,
				    getvsym(&instrnametable,
                                    sendptr->down->next->next->val),
				    lastinstr);
			}
		      routeptr = routeptr->next;
		    }
		}
	      blist = blist->next;
	    }
	}
      sendptr=sendptr->next;
    }

  /* add outputbusinstr and startupinstr back into the list */

  if (outputbusinstr)
    {
      if (instrnametable == NULL)
	instrnametable = outputbusinstr;
      else
	{
	  lastinstr = instrnametable;
	  while (lastinstr->next !=NULL)
	    lastinstr = lastinstr->next;
	  lastinstr->next = outputbusinstr;
	}
      outputbusinstr->next = NULL;
    }

  if (startupinstr)
    {
      startupinstr->next = instrnametable;
      instrnametable = startupinstr;
    }

  /* do sequence statement alterations */

  passcount = changed = 1;
  while ((changed) && (passcount < 10000))
    {
      changed = 0;
      passcount++;
      seqptr = groot;
      while (seqptr != NULL) 
	{
	  if (seqptr->ttype == S_SEQDEF)
	    {
	      blist = seqptr->down->next->next->down;
	      while (blist->next != NULL)
		{
		  changed |= movebefore(&instrnametable,
			     getsym(&instrnametable,blist),
			     getsym(&instrnametable,blist->next->next));
		  blist = blist->next->next;
		}
	    }
	  seqptr= seqptr->next;
	}
    }
  if (changed)
    internalerror("postparse.c","end of instrorder()");

  /* move startup back to start of list */

  if (startupinstr && (startupinstr != instrnametable))
    {
      iptr = instrnametable;
      while (iptr && iptr->next)
	{
	  if (iptr->next == startupinstr)
	    {
	      iptr->next = iptr->next->next;
	      break;
	    }
	  iptr = iptr->next;
	}
      startupinstr->next = instrnametable;
      instrnametable = startupinstr;
    }

  /* set vol field to ordinal count in instrnametable */

  iptr = instrnametable;
  lastinstr = NULL;

  while (iptr)
    {
      if (reachableinstr(iptr))
	{
	  iptr->vol = numinstrnames++;
	  lastinstr = iptr;
	  iptr = iptr->next;
	}
      else
	{
	  if (lastinstr)
	    {
	      lastinstr->next = iptr->next;
	      iptr->next = unusedinstrtable;
	      unusedinstrtable = iptr;
	      iptr = lastinstr->next;
	    }
	  else
	    {
	      iptr = instrnametable->next;
	      instrnametable->next = unusedinstrtable;
	      unusedinstrtable = instrnametable;
	      instrnametable = iptr;
	    }
	}
    }

}


/****************************************************************/
/*          creates instances, indicates execution ordering      */
/****************************************************************/

void instanceorder (void)

{
   sigsym * sptr;
   tnode * sendptr;
   tnode * instptr = NULL;  /* initialization not needed */
   tnode * tptr;
   char * name;

   sptr = instrnametable;
   while (sptr)
     {
       sendptr = groot;
       while (sendptr)
	 {
	   if ((sendptr->ttype == S_SENDDEF) && (sendptr->sptr == sptr))
	     {
	       if ((outputbusinstance) && 
		   (sendptr == outputbusinstance->down))
		 {
		   sptr->numinst++;
		   vmcheck(name = (char *) malloc(255));
		   sprintf(name,"%s%i", sptr->val, sptr->numinst);
		   outputbusinstance->val = name;
		   outputbusinstance->sptr = sptr;
		   if (instances == NULL)
		     {
		       instances = outputbusinstance;
		       instptr = instances;
		     }
		   else
		     {
		       instptr->next = outputbusinstance;
		       instptr = instptr->next;
		     }
		 }
	       else
		 {
		   sptr->numinst++;
		   vmcheck(name = (char *) malloc(255));
		   sprintf(name,"%s%i", sptr->val, sptr->numinst);
		   tptr = make_tnode(name, S_INSTANCE);
		   tptr->down = sendptr;
		   tptr->sptr = sptr;
		   if (instances == NULL)
		     {
		       instances = tptr;
		       instptr = instances;
		     }
		   else
		     {
		       instptr->next = tptr;
		       instptr = instptr->next;
		     }
		 }
	     }
	   sendptr = sendptr->next;
	 }
       sptr = sptr->next;
     }

  /* if instance being sent output bus, we know its output width */

  if (outputbusinstance)
    {
      outputbusinstance->sptr->width = outchannels;
      outputbusinstance->sptr->outputbus = 1;
    }
}


extern void checkopcodeargswidth(tnode *);
extern void resolveinstrument(sigsym *);
extern void instrbuswrite(sigsym *);
extern int  instrinputwidth(tnode *);

/****************************************************************/
/*        routine to compute width of all buses                 */
/****************************************************************/

void buswidth(void)

{
  sigsym * sptr;
  tnode * iptr;
  int first, old, i;

  checkopcodeargswidth(globalopcodecalls);
  iptr = instances;
  sptr = instrnametable;
  while (sptr != NULL)
    {
      if (sptr->effects == 0)
	{
	  currinputwidth = 1;
	  currinstrwidth = OUTCHANNELSWIDTH;
	  widthupdate(sptr->defnode->down->
		      next->next->next->next->next->next->next->next->down);
	  resolveinstrument(sptr);
	  checkopcodeargswidth(sptr->defnode->optr);
	  sptr->width = currinstrwidth;
	  instrbuswrite(sptr);
	}
      else
	{
	  first = 1;
	  while ((iptr != NULL) && (iptr->sptr == sptr))
	    {
	      currinputwidth = iptr->inwidth = instrinputwidth(iptr);
	      currinstrwidth = OUTCHANNELSWIDTH;
	      widthupdate(sptr->defnode->down->
			  next->next->next->next->next->next->next->next->down);
	      resolveinstrument(sptr);
	      checkopcodeargswidth(sptr->defnode->optr);
	      if (first)
		{
		  sptr->width = currinstrwidth;
		  instrbuswrite(sptr);
		  first = 0;
		}
	      else
		{
		  old = sptr->width;
		  sptr->width = currinstrwidth;
		  instrbuswrite(sptr);   /* do first. to catch the common errors */
		  if (old != sptr->width)
		    {
		      printf("Error: Instances of %s have different widths"
			     " (%i and %i).\n",
			   sptr->val, old, sptr->width);
		      showerrorplace(iptr->down->down->linenum,
				     iptr->down->down->filename);
		    }
		}
	      iptr = iptr->next;
	    }
	}
      sptr = sptr->next;
    }

  sptr = busnametable;
  i = 0;
  while (sptr != NULL)
    {
      if (sptr->width == 0)
	{
	  printf("Warning: Bus %s has undefined output width.\n"
		 "         Sfront uses width of 1 (implementation-dependent).\n\n",
		 sptr->val);

	  sptr->width = 1;
	}
      sptr->vol = i++;
      sptr = sptr->next;
    }

}


/****************************************************************/
/*            installs input buses in instances                 */
/****************************************************************/

void inbusinstall(void)

{
  tnode * tptr;
  tnode * bptr;
  tnode * ibusptr;
  tnode * ibusroot;
  int idx = 0;

  tptr = instances;
  while (tptr != NULL)
    {

      /* number all effects */

      tptr->arrayidx = idx;
      idx++;

      /* make input bus */

      bptr = tptr->down->down->next->next->next->next->next->next->down;
      ibusroot = ibusptr = make_tnode("",S_BUS);
      tptr->width = 0;
      while (bptr != NULL)
	{
	  if (bptr->ttype == S_NAME)
	    {
	      ibusptr->val = dupval(bptr->sptr->val);
	      ibusptr->sptr = bptr->sptr;
	      ibusptr->width = bptr->sptr->width;
	      if (ibusptr->width > 0)
		tptr->width += ibusptr->width;
	      if (bptr->next != NULL)
		{
		  ibusptr->next = make_tnode("",S_BUS);
		  ibusptr = ibusptr->next;
		}
	    }
	  bptr = bptr->next;
	}
      tptr->ibus = ibusroot;
      tptr = tptr->next;

    }
}


/****************************************************************/
/*            installs output buses in instruments              */
/****************************************************************/


void outbusinstall(void)

{
   tnode * gptr = groot;
   tnode * iptr;
   tnode * bptr;
   tnode * tptr;
   sigsym * sptr;
   int i;

   /* create all obuses from route statements */

   while (gptr != NULL)   /* ROUTE LP IDENT COM   identlist      RP SEM */
     {                    /*         busname    instrument list         */
       if (gptr->ttype == S_ROUTEDEF)
	 {
	   iptr = gptr->down->next->next->next->next->down; /* instr list */
	   i = 0;
	   while (iptr != NULL)
	     {
	       if (iptr->ttype == S_IDENT)
		 {
		   bptr = make_tnode(gptr->sptr->val,S_BUS);
		   bptr->sptr = gptr->sptr;  /* write output onto this bus */
		   bptr->res = i;            /* with this offset */
		   if ((i==0)&&(iptr->next == NULL)) /* write mono -- special case */
		     {
		       bptr->width = bptr->sptr->width;
		     }
		   else
		    bptr->width = iptr->sptr->width;   
		   i = i + iptr->sptr->width;
		   if (iptr->sptr->obus == NULL)
		     iptr->sptr->obus = bptr;
		   else
		     {
		       tptr = iptr->sptr->obus;
		       while (tptr->next != NULL)
			 tptr= tptr->next;
		       tptr->next = bptr;
		     }
		 }
	       iptr = iptr->next;
	     }
	 }
       gptr = gptr->next;
     }

   /* and those that default to output_bus */
   
   sptr = instrnametable;
   while (sptr != NULL)
     {
       if (sptr->obus == NULL)
	 {
	   sptr->obus = make_tnode("output_bus",S_BUS);
	   sptr->obus->sptr = outputbus;
	   sptr->obus->width = outputbus->width;
	 }
       sptr = sptr->next;
     }

}

/****************************************************************/
/*          sets outputbus flag for instruments                 */
/****************************************************************/

void sendoutputflag (void)

{
   sigsym * sptr;
   int found = 0;

   sptr = instrnametable;
   while (sptr != NULL)
     {
       if (found)
	 sptr->outputbus = 0;
       else
	 sptr->outputbus = 1;
       if ((outputbusinstance != NULL) && 
	   (outputbusinstance->sptr == sptr)) 
	 {
	   found = 1;
	 }
       sptr = sptr->next;
     }

}


extern sigsym * instancecode(sigsym *);

/****************************************************************/
/*          clones sptr fields of instances                     */
/****************************************************************/

void instanceclone (void)

{

  tnode * tptr;

  tptr = instances;
  while (tptr != NULL)
    {
      tptr->sptr = instancecode(tptr->sptr);
      checkopcodeargswidth(tptr->sptr->defnode->optr);
      tptr = tptr->next;
    }

}




/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*                                                              */
/*                   Utility functions                          */
/*                                                              */
/*______________________________________________________________*/


/****************************************************************/
/* syntax check for table/tablemaps: in globalimportscheck()    */
/****************************************************************/

void checktable(sigsym * sptr)

{
  sigsym * ptr;
  tnode * tptr;

  while (sptr != NULL)
    {
      ptr = sptr->defnode->sptr;
      while (ptr != NULL)
	{
	  if ( (ptr->vartype == TABLETYPE) && (ptr->kind == K_IMPORTEXPORT)
	       && (getvsym(&globalsymtable,ptr->val)==NULL))
	    {
	      printf("Error: Imports exports on a future table.\n");
	      showerrorplace(ptr->defnode->linenum,
			     ptr->defnode->filename);
	    }
	  if (ptr->vartype == TMAPTYPE)
	    {
	      tptr = ptr->defnode->down->next->next->next->down;
	      while (tptr != NULL)
		{
		  if (tptr->ttype == S_IDENT)
		    {
		      tptr->sptr = getsym(&(sptr->defnode->sptr),tptr);
		      if ((tptr->sptr == NULL)
			  ||(tptr->sptr->vartype != TABLETYPE))
			{
			  printf("Error: Bad table '%s' for tablemap.\n",
				 tptr->val);
			  showerrorplace(tptr->linenum,
					 tptr->filename);
			}
		    }
		  tptr = tptr->next;
		}
	    }
	  ptr = ptr->next;
	}
      sptr = sptr->next;
    }
}


extern void futureglobalinit(sigsym *);

/****************************************************************/
/* imports check for future tables: in globalimportscheck()     */
/****************************************************************/

void checktableimport(sigsym * sptr)

{
  sigsym * ptr;

  while (sptr != NULL)
    {
      ptr = sptr->defnode->sptr;
      while (ptr != NULL)
	{
	  if ( (ptr->vartype == TABLETYPE) && (ptr->kind == K_IMPORT) )
	    {
	      if (addvsym(&globalsymtable, ptr->val, K_NORMAL) == INSTALLED)
		{
		  globalsymtable->vartype = TABLETYPE;
		  globalsymtable->rate = IRATETYPE;
		  globalsymtable->width = 1;
		  futureglobalinit(globalsymtable);
		}
	    }
	  ptr = ptr->next;
	}
      sptr = sptr->next;
    }
}

/****************************************************************/
/*      Helper routine for checktableimport() above             */
/****************************************************************/

void futureglobalinit(sigsym * sptr)

{
  tnode * tptr;
  tnode * gptr;

  gptr = make_tnode("<table>", S_TABLE);
  tptr = gptr->down = make_tnode("TABLE", S_TABLE);
  tptr->next = make_tnode(sptr->val, S_IDENT);
  tptr = tptr->next;
  tptr->next = make_tnode("(", S_LP);
  tptr = tptr->next;
  tptr->next = make_tnode("empty", S_IDENT);
  tptr = tptr->next;
  tptr->next = make_tnode(",", S_COM);
  tptr = tptr->next;
  tptr->next = make_tnode("<exprstrlist>", S_EXPRSTRLIST);
  tptr->next->next = make_tnode(")", S_RP);
  tptr->next->down = make_tnode("<expr>",S_EXPR);
  tptr->next->down->down = make_tnode("1", S_INTGR);
  tptr->next->down->rate  = tptr->next->down->down->rate  = IRATETYPE;
  tptr->next->down->width = tptr->next->down->down->width = 1;
  tptr->next->down->vartype = tptr->next->down->down->vartype = SCALARTYPE;
  tptr->next->down->res = tptr->next->down->down->res;
  sptr->defnode = gptr;

  gptr->vol = CONSTANT;
  gptr->arrayidx = 1;
  gptr->usesinput = 1;
  vmcheck(sptr->consval = calloc(2, sizeof(float)));
}

/****************************************************************/
/* links up variable references:  in globalstatementcheck()     */
/*                                and oclone.c functions        */
/****************************************************************/

void varupdate(tnode * tptr, sigsym ** varsymtable)

{
  while (tptr != NULL)
    {
      if (tptr->down != NULL)
	{
	  varupdate(tptr->down,varsymtable);

	  switch (tptr->ttype) {
	  case S_LVALUE:
	    if (tptr->down->sptr == NULL)
	      {
		if (strcmp(tptr->down->val,"MIDIctrl") && 
		    strcmp(tptr->down->val,"params"))
		  {
		    printf("Error: Lval must be variable or pfield.\n");
		    showerrorplace(tptr->down->linenum,
				   tptr->down->filename);
		  }
		break;
	      }
	    tablecheck(tptr->down);
	    if (tptr->down->next == NULL)
	      {
		tptr->vartype = tptr->down->vartype;
	      }
	    else
	      {
		tablecheck(tptr->down->next->next);
		if (tptr->down->vartype == SCALARTYPE)
		  {
		    printf("Error: Array index on a scalar variable.\n");
		    showerrorplace(tptr->down->linenum,
				   tptr->down->filename);
		  }
		tptr->vartype = SCALARTYPE;
	      }
	    break;
	  case S_STATEMENT:
	    if (tptr->down->next->ttype == S_SEM)
	      {
		if (tptr->down->ttype == S_EXPR)     /* expr SEM */
		  {
		    tablecheck(tptr->down);
		  }
		else                                 /* turnoff SEM */
		  {
		  }
		break;
	      }
	    if (tptr->down->ttype == S_OUTBUS)
	      {
		tptr->sptr = getvsym(&busnametable,
				     tptr->down->next->next->val);
		tablelistcheck(tptr->down->next->next);
		break;
	      }
	    if ((tptr->down->ttype == S_OUTPUT)||
		(tptr->down->ttype == S_RETURN)||
		(tptr->down->ttype == S_PRINTF)||
		(tptr->down->ttype == S_SPATIALIZE))
	      {
		tablelistcheck(tptr->down->next->next);
		break;
	      }
	    if (tptr->down->ttype == S_INSTR)
	      {
		tablelistcheck(tptr->down->next->next->next);
		break;
	      }
	    if ((tptr->down->ttype == S_IF)     ||
		(tptr->down->ttype == S_WHILE)  ||
		(tptr->down->ttype == S_EXTEND) ||
		(tptr->down->ttype == S_LVALUE)) /* lvalue EQ expr SEM */ 
	      {
		tablecheck(tptr->down->next->next);
		break;
	      }
	    break;
	  case S_EXPR:
	    if (tptr->down->next == NULL)          
	      {
		tptr->vartype = tptr->down->vartype;
		break;
	      }
	    if (tptr->down->ttype == S_MINUS)       /* unary */
	      {
		tablecheck(tptr->down->next);
		break;
	      }
	    if (tptr->down->ttype == S_NOT)      /* unary */
	      {
		tablecheck(tptr->down->next);
		break;
	      }
	    if (tptr->down->ttype == S_LP) /* works for float->into to */
	      {
		tablecheck(tptr->down->next);
		break;
	      }
	    if (tptr->down->ttype == S_FLOATCAST) 
	      {
		tablecheck(tptr->down->next->next);
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
		(tptr->down->next->ttype == S_PLUS)  ||
                (tptr->down->next->ttype == S_MINUS) ||
		(tptr->down->next->ttype == S_STAR)  ||
                (tptr->down->next->ttype == S_SLASH)  )
	      {
		tablecheck(tptr->down);
		tablecheck(tptr->down->next->next);
		break;
	      }
	    if (tptr->down->next->ttype == S_Q)
	      {
		tablecheck(tptr->down);
		tablecheck(tptr->down->next->next);
		tablecheck(tptr->down->next->next->next->next);
		break;
	      }
	    if (tptr->down->next->ttype == S_LB) 
	      {
		if (tptr->down->next->next->next->next == NULL) /* array index */
		  {
		    tablecheck(tptr->down->next->next);
		    if (tptr->down->vartype == SCALARTYPE)
		      {
			printf("Error: Array index on a scalar variable.\n");
			showerrorplace(tptr->down->linenum,
				       tptr->down->filename);
		      }
		    if (tptr->down->vartype == TABLETYPE)
		      {
			printf("Error: array index on a table variable.\n");
			showerrorplace(tptr->down->linenum,
				       tptr->down->filename);
		      }
		    if (tptr->down->vartype == TMAPTYPE)
		      tptr->vartype = TABLETYPE;
		    else
		      tptr->vartype = SCALARTYPE;
		  }
		else       /* oparray */
		  {
		    tablecheck(tptr->down->next->next);
		  }
		break;
	      }
	    break;
	  }
	}
      else
	{
	  /* note this does double duty -- for variable declarations */
	  /* and for tokens in code blocks. thus caveat below        */

	  if (tptr->ttype == S_OPARRAY)  /* is checked elsewhere */
	    tptr = tptr->next;

	  if ((tptr->ttype == S_IDENT) && (!standardname(tptr)) &&
	      (!wavegeneratorname(tptr)) && (!coreopcodename(tptr)) && 
	      (tptr->optr == NULL))
	    {
	      tptr->sptr = getsym(varsymtable, tptr);
	      if (tptr->sptr == NULL)
		{
		  printf("Error: Variable %s not defined.\n",tptr->val);
		  showerrorplace(tptr->linenum,tptr->filename);
		}
	      tptr->res = tptr->sptr->res;
	      tptr->vartype = tptr->sptr->vartype;
 	    }
	}
      tptr = tptr->next;
    }

}


/****************************************************************/
/*       Opcode recursion check: in mainrecursioncheck()        */
/****************************************************************/

void checkopcoderecursion(sigsym * sroot)

{

  tnode * tptr;

  /* check all of your opcode calls */

  tptr = sroot->defnode->optr;
  while (tptr != NULL)
    {
      if (tptr->ttype != S_OPARRAYDECL)
	{
	  tptr->sptr->numinst++;
	  if (tptr->sptr->numinst > 1)
	    {
	      printf("Error: Recursive opcode calls detected.\n");
	      showerrorplace(tptr->down->linenum,
			     tptr->down->filename);
	    }
	  checkopcoderecursion(tptr->sptr);
	  tptr->sptr->numinst--;
	}
      tptr = tptr->next;
    }

}

/*******************************************************************/
/* helper routine for sequence syntax check: in  seqloopdetect()   */
/*******************************************************************/

void dotree(sigsym * currsym)

{

  tnode * tptr;

  tptr = currsym->defnode;
  while (tptr != NULL)
    {
      if (tptr->sptr->res == 0)
	{
	  tptr->sptr->res = 1;
	  dotree(tptr->sptr);
	}
      tptr = tptr->next;
    }
  
}

/****************************************************************/
/*  Checks opcode arguments: in buswidth() and instanceclone()  */
/****************************************************************/

void checkopcodeargswidth(tnode * tcall)

{
  tnode * tptr;
  tnode * dptr;

  while ((tcall != NULL) && (tcall->ttype == S_OPARRAYDECL))  
    {
      if (truewidth(tcall->opwidth) == 0)
	{
	  printf("Error: Oparray width must be >= 1.\n");
	  showerrorplace(tcall->optr->down->linenum,
			 tcall->optr->down->filename);
	}
      tcall = tcall->next;
    }
  while (tcall != NULL)
    {
      checkopcodeargswidth(tcall->sptr->defnode->optr);

      /* tptr holds actual arguments */
      if (tcall->ttype == S_OPCALL)
	tptr = tcall->optr->down->next->next->down;
      else
	{
	  tptr = tcall->optr->down->next->next->next->next->next->down;
	}

      /* dptr holds formal arguments */
      dptr = tcall->sptr->defnode->down->next->next->next->down;

      if ((dptr != NULL) && (tptr == NULL))
	{
	  printf("Error: Opcode call argument mismatch (%s).\n",
		 tcall->optr->down->val);
	  showerrorplace(tcall->optr->down->linenum,
			 tcall->optr->down->filename);
	}
      while (dptr != NULL)
	{
	  if (dptr->ttype == S_PARAMDECL)
	    {
	      if (truewidth(tptr->width) != truewidth(dptr->width))
		{
		  printf("Error: Opcode call width mismatch (%s).\n",
			 tcall->optr->down->val);
		  showerrorplace(tcall->optr->down->linenum,
				 tcall->optr->down->filename);
		}
	      if (((tptr->vartype != TABLETYPE) && (dptr->vartype == TABLETYPE))||
		  ((tptr->vartype == TABLETYPE) && (dptr->vartype != TABLETYPE))||
		   (tptr->vartype == TMAPTYPE))
		{
		  printf("Error: Opcode table parameter mismatch.\n");
		  showerrorplace(tcall->optr->down->linenum,
				 tcall->optr->down->filename);
		}
	    }
	  dptr = dptr->next;
	  if (tptr == NULL)
	    {
	      printf("Error: Opcode call argument mismatch (%s).\n",
		     tcall->optr->down->val);
	      showerrorplace(tcall->optr->down->linenum,
			     tcall->optr->down->filename);
	    }
	  tptr = tptr->next;
	}
      if ((tptr != NULL) && (!coreopcodeargs(tcall,tptr)))
	{
	  printf("Error: Opcode call argument mismatch (%s).\n",
		 tcall->optr->down->val);
	  showerrorplace(tcall->optr->down->linenum,
			 tcall->optr->down->filename);
	}
      if (tcall->ttype == S_OPARRAYCALL)
	{
	  tptr = tcall->optr->down->next->next;
	  if (truewidth(tptr->width) != 1)
	    {
	      printf("Error: Oparray index must be single-valued.\n");
	      showerrorplace(tcall->optr->down->linenum,
			     tcall->optr->down->filename);
	    }
	}
      tcall = tcall->next;
    }

}

/****************************************************************/
/*        resolves instruments with undetermined widths         */
/****************************************************************/

void resolveinstrument(sigsym * sptr)

{
  /* use any fixed bus is routes from ... */

  if ((currinstrwidth <= UNKNOWN) && (sptr->width > UNKNOWN))
    currinstrwidth = sptr->width;  

  /* ... or else use default, and issue warning */

  if (currinstrwidth <= UNKNOWN)
    {	  
      printf("Warning: Instrument %s has undefined output width.\n"
	     "         Sfront uses width of 1 (implementation-dependent).\n\n",
	     sptr->val);
      currinstrwidth = 1;
    }
}


/****************************************************************/
/*    propagates instrument output to buses: in buswidth()      */
/****************************************************************/

void instrbuswrite(sigsym * iptr)

{
  tnode * gptr = groot;
  int found;
  int ready;
  int width;
  tnode * tptr;

  while (gptr != NULL)
    {
      if (gptr->ttype == S_ROUTEDEF)
	{
	  width = found = 0;
	  ready = 1;
	  tptr = gptr->down->next->next->next->next->down; /* instr list */
	  while ((tptr != NULL) && (ready))
	    {
	      if (tptr->ttype == S_IDENT)
		{
		  if (tptr->sptr->width < 1)
		    ready = 0;
		  else
		    {
		      if (tptr->sptr == iptr)
			found = 1;
		      width += tptr->sptr->width;
		    }
		}
	      tptr = tptr->next;
	    }
	  if (ready && found)
	    {
	      if (gptr->sptr->width == UNKNOWN)
		gptr->sptr->width = width;
	      else
		if ((width != gptr->sptr->width) && (width > 1))
		  {
		    printf("Error: width mismatch (%i and %i) for bus %s.\n",
			   width, gptr->sptr->width,
			   gptr->down->next->next->val);
		    showerrorplace(gptr->down->linenum,
				   gptr->down->filename);
		  }
	    }
	}
      gptr = gptr->next;
    }

  /* check width match for direct routes to output bus */

  if ((iptr->outputbus) && (iptr->width > 1) && (iptr->width != outchannels))
    {
      printf("Error: Width mismatch (%i vs %i) for instr %s on the output_bus.\n",
	     iptr->width, outchannels, iptr->val);
      showerrorplace(iptr->defnode->down->linenum, 
		     iptr->defnode->down->filename);
    }
}


/****************************************************************/
/*  given sequence rules, computes input width: in buswidth()   */
/****************************************************************/

int instrinputwidth(tnode  * iptr)

{
  tnode * bptr;
  int ret = 0;

  bptr = iptr->down->down->next->next->next->next->next->next->down;

  while (bptr != NULL)
    {
      if (bptr->ttype == S_NAME)
	ret += bptr->sptr->width;
      bptr = bptr->next;
    }

  if (ret == 0)   /* if no bus data, 5.7.3.3.5.2 says use 1 */
    ret = 1;

  return ret;
}

/****************************************************************/
/*         clone instrument code: in instanceclone()            */
/****************************************************************/

sigsym * instancecode(sigsym * sptr)

{
  tnode * t_miditag;
  tnode * t_vardecls;
  tnode * t_block;
  tnode * cptr;
  tnode * ptokens;          /* identlist for pfield string*/ 
  tnode * vtokens;
  sigsym * templatevars = NULL;
  sigsym * retptr = NULL;

  /* sptr code segments of interest */

  t_miditag = sptr->defnode->down->
    next->next->next->next->next;
  t_vardecls =  t_miditag->next->next;
  t_block = t_vardecls->next->down;
  t_vardecls = t_vardecls->down;
  t_miditag = t_miditag->down;

  /* table pointers for backpatch */

  tlocsymtable = sptr->defnode->sptr;
  tlocopcodecalls = sptr->defnode->optr;
  tlocdyncalls = sptr->defnode->dptr;

  /* make return symbol name, with assignment paranoia */

  addvsym(&retptr, sptr->val, K_INSTRNAME);
  retptr->width = sptr->width;
  retptr->effects = sptr->effects;
  retptr->score = sptr->score;  
  retptr->ascore = sptr->ascore;  
  retptr->dyn = sptr->dyn;    
  retptr->midi = sptr->midi;   
  retptr->amidi = sptr->amidi;   
  retptr->miditag = sptr->miditag;
  retptr->startup = sptr->startup;
  retptr->outputbus = sptr->outputbus;
  retptr->rate = sptr->rate;     
  retptr->special = sptr->special;  
  retptr->res = sptr->res;      
  retptr->vartype = sptr->vartype;  
  retptr->vol = sptr->vol;      
  retptr->numinst = sptr->numinst;  
  retptr->calrate = sptr->calrate;  
  retptr->obus = sptr->obus;  
  
  /* create datastructure for instr */

  retptr->defnode = cptr = make_tnode("<instrdecl>",S_INSTRDECL);
  cptr->down = make_tnode("INSTR",S_INSTR);
  cptr->down->next = make_tnode(sptr->val, S_IDENT);
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
  
  /* set miditag pointer */
  
  ptokens->next->next->down = t_miditag;
  
  cptr->sptr = locsymtable = sclone(tlocsymtable);
  locopcodecalls = tclone(tlocopcodecalls); 
  locdyncalls = tclone(tlocdyncalls);
  vtokens->next->down = treeclone(t_block,&templatevars,DOSUB); 
  vtokens->down = treeclone(t_vardecls,&templatevars,DOSUB);
  cptr->optr = locopcodecalls;
  cptr->dptr = locdyncalls;
  varupdate(vtokens->next->down,&locsymtable);
  varupdate(vtokens->down,&locsymtable);
  
  installopnames(retptr->defnode->optr);
  installdyninstr(retptr->defnode->dptr);

  return retptr;

}


/****************************************************************/
/* --         Checks leaf node array declarations               */
/*                     Not presently used                       */
/****************************************************************/

void checkopcodedecls(sigsym * sroot)


{

  tnode * tptr;
  sigsym * sptr;

  /* check your own arrays */

  sptr = sroot->defnode->sptr;
  while (sptr != NULL)
    {
      if ( (sptr->width == INCHANNELSWIDTH))
	{
	  printf("Error: Array[inchannels] has zero width.\n");
	  showerrorplace(sptr->defnode->linenum,
			 sptr->defnode->filename);
	}
      sptr = sptr->next;
    }

  /* check all of your opcode calls */

  tptr = sroot->defnode->optr;
  while (tptr != NULL)
    {
      if (tptr->ttype != S_OPARRAYDECL)
	{
	  checkopcodedecls(tptr->sptr);
	  if ( (tptr->opwidth == INCHANNELSWIDTH))
	    {
	      printf("Error: Array[inchannels] has zero width.\n");
	      showerrorplace(tptr->down->linenum,
			     tptr->down->filename);
	    }
	}
      tptr = tptr->next;
    }

}


    


