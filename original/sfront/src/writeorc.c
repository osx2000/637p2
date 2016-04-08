
/*
#    Sfront, a SAOL to C translator    
#    This file: Code generation: misc
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


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*         void toptree(int) and void postscript(void)          */
/*                                                              */
/* These two top-level functions are called by the main() of    */
/* sfront (located in sfmain.c). Toptree() prints out the       */
/* instr functions for a rate (ipass, kpass, or apass), and     */
/* postscript() prints out control functions and main().        */
/*                                                              */
/*______________________________________________________________*/


extern void ipassmake(tnode *);
extern void kpassmake(tnode *);
extern void apassmake(tnode *);

/****************************************************************/
/*   create _ipass, _kpass, and _apass high-level functions     */
/****************************************************************/

void toptree(int mode)

{
  sigsym * sptr;
  tnode * tptr;
  int i;

  /* print functions for non-effects instruments*/

  sptr = instrnametable;
  while (sptr != NULL)
    {
      if (reachableinstrexeff(sptr))
	{
	  currinputwidth = 1;
	  currinstrwidth = sptr->width;
	  curropcodeprefix = currinstancename = sptr->val;
	  currinstrument = sptr;
	  currinstance = NULL;

	  switch(mode) {
	  case PRINTIPASS:
	    ipassmake(sptr->defnode);
	    break;
	  case PRINTKPASS:
	    kpassmake(sptr->defnode);
	    break;
	  case PRINTAPASS:
	    if (sptr->cref->alines)
	      apassmake(sptr->defnode);
	    break;
	  }
	}
      sptr = sptr->next;
    }

  /* set starting point for ninstr[] effects positions  */
  /* this must change if startup implementation changes */
  
  i = (startupinstr != NULL);

  /* print functions for effects instruments*/

  tptr = instances;
  while (tptr != NULL)
    {
      redefstatic(i++);
      currinputwidth = tptr->inwidth;
      currinstrwidth = tptr->sptr->width;
      curropcodeprefix = currinstancename = tptr->val;
      currinstrument = tptr->sptr;
      currinstance = tptr;

      switch(mode) {
      case PRINTIPASS:
	ipassmake(tptr->sptr->defnode);
	break;
      case PRINTKPASS:
	kpassmake(tptr->sptr->defnode);
	break;
      case PRINTAPASS:
	if (tptr->sptr->cref->alines)
	  apassmake(tptr->sptr->defnode);
	break;
      }
      tptr = tptr->next;
    }

  /* restore #defines for control and global code to follow */

  redefnormal();                         
}


extern void printinputgroups(void);
extern void printsystemsinit(void);
extern void printeffectsinit(void);
extern void printshutdown(void);
extern void printmaincontrol(void);

/****************************************************************/
/*    control and global functions that complete the sa.c file  */
/****************************************************************/

void postscript(void)

{
  int active;

  printsaoltables(S_DATA); /* print saol wavetable data                */
  postcorefunctions();     /* global data for core opcodes: writepre.c */
  printinputgroups();      /* functions for complex input[] access     */
  printsystemsinit();      /* system_init(), global inits for sa.c     */
  printeffectsinit();      /* effects_init(), effects chain inits      */
  printshutdown();         /* function for clean exit of sa.c program  */
  printmainloops();        /* loops for instr execution: writemain.c   */
  printmaincontrol();      /* loops for table, tempo and control cmds  */

  /* print the actual main() for sa.c, from lib/csrc library */

  active = (ainflow == ACTIVE_FLOW) || (aoutflow == ACTIVE_FLOW);

  if (active)
    {
      if (!nomain)
	makecallback();
    }
  else
    {
      switch (makeaudiotypeout(aout)) {
      case SAMPLE_SHORT:
	makeruntime();
	break;
      case SAMPLE_FLOAT:
	makeruntimef();
	break;
      }
    }
}


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*                                                              */
/*        Second level functions called by toptree()            */
/*                                                              */
/*______________________________________________________________*/


/****************************************************************/
/*                  generates ipass functions                   */
/****************************************************************/

void ipassmake(tnode * tbranch)


{
  tnode * tptr = tbranch;
  sigsym * ptr1;
  sigsym * gptr;
  int first = 0;
  char name[STRSIZE];

  currblockrate = IRATETYPE;

  if (currinstance == NULL)
    fprintf(outfile,"void %s_ipass("
	    "ENGINE_PTR_DECLARE_COMMA struct ninstr_types * nstate)\n{\n",
	    currinstancename);
  else
    fprintf(outfile,"void %s_ipass(ENGINE_PTR_DECLARE)\n{\n", currinstancename);


  fprintf(outfile,"   int i;\n");
  ptr1 = tptr->sptr;
  while (ptr1 != NULL)
    {
      if ((ptr1->vartype == TABLETYPE) && 
	  ((ptr1->kind == K_NORMAL)|| (ptr1->kind == K_IMPORT)))
	{
	  if (!first)
	    {
	      fprintf(outfile,"   int j;\n");
	      first = 1;
	    }
	  if (ptr1->kind == K_NORMAL)
	    wavegeneratorvar(ptr1);
	}
      ptr1 = ptr1->next;
    }
  fprintf(outfile,"\n");
  fprintf(outfile,"memset(&(NVU(0)), 0, %s_ENDVAR*sizeof(NVU(0)));\n",
	  currinstancename);

  /* later reclaim pointer memory */

  fprintf(outfile,"memset(&(NT(0)), 0, %s_ENDTBL*sizeof(NT(0)));\n",
	  currinstancename);

  printdurassign();

  ptr1 = tptr->sptr;
  while (ptr1 != NULL)
    {
      if (ptr1->vartype != TABLETYPE)
	{
	  switch (ptr1->kind) {
	  case K_PFIELD:
	    fprintf(outfile,"   NV(%s_%s) = \n",
		    currinstancename,ptr1->val);
	    fprintf(outfile,"   NS(iline->p[%s_%s]);\n",
		    currinstancename,ptr1->val);
	    break;
	  case K_IMPORT: case K_IMPORTEXPORT:
	    gptr = getvsym(&globalsymtable,ptr1->val);
	    if (ptr1->rate == IRATETYPE)
	      {
		if (gptr == NULL)
		  {
		    printf("Error: Import has no matching ivar global.\n\n");
		    showerrorplace(ptr1->defnode->linenum, 
				   ptr1->defnode->filename);
		  }
		if ((gptr->vartype != ptr1->vartype)||
		    (gptr->width != truewidth(ptr1->width))||
		    (gptr->rate != ptr1->rate))
		  {
		    printf("Error: Import mismatch with global.\n\n");
		    showerrorplace(ptr1->defnode->linenum, 
				   ptr1->defnode->filename);
		  }
		if (ptr1->tref->mirror != GLOBALMIRROR)
		  fprintf(outfile,
		"   memcpy(&(NVU(%s_%s)), &(NGU(GBL_%s)), %i*sizeof(NGU(0)));\n",
			currinstancename,ptr1->val,ptr1->val,
			truewidth(ptr1->width));
	      }
	    break;
	  case K_EXPORT:
	    if ((ptr1->rate == IRATETYPE) &&
		(ptr1->tref->mirror == GLOBALMIRROR) &&
		((ptr1->tref->assigntot == 0) || 
		 (ptr1->tref->varstate)))
	      fprintf(outfile,
		"   memset(&(NGU(GBL_%s)), 0, %i*sizeof(NGU(0)));\n",
			ptr1->val, truewidth(ptr1->width));
	  default:
	    break;
	  }
	}
      ptr1 = ptr1->next;
    }

  ptr1 = tptr->sptr;
  while (ptr1 != NULL)
    {
      if (ptr1->vartype == TABLETYPE)
	{
	  sprintf(name,"TBL_%s",currinstancename);
	  createtable(ptr1,name, S_INSTR);
	}
      ptr1 = ptr1->next;
    }
  blocktree(tptr->down,PRINTIPASS);
  ptr1 = tptr->sptr;
  while (ptr1 != NULL)
    {
      if ((ptr1->rate == IRATETYPE)&&
	  ((ptr1->kind==K_EXPORT)||(ptr1->kind==K_IMPORTEXPORT)))
	{
	  gptr = getvsym(&globalsymtable,ptr1->val);
	  if (gptr == NULL)
	    {
	      printf("Error: Import has no matching ivar global.\n\n");
	      showerrorplace(ptr1->defnode->linenum, 
			     ptr1->defnode->filename);
	    }
	  if ((gptr->vartype != ptr1->vartype)||
	      (gptr->width != truewidth(ptr1->width))||
	      (gptr->rate != ptr1->rate))
	    {
	      printf("Error: Import mismatch with global.\n\n");
	      showerrorplace(ptr1->defnode->linenum, 
			     ptr1->defnode->filename);
	    }
	  if ((ptr1->vartype != TABLETYPE) &&
	      (ptr1->tref->mirror != GLOBALMIRROR))
	    fprintf(outfile,
	     "   memcpy(&(NGU(GBL_%s)), &(NVU(%s_%s)), %i*sizeof(NGU(0)));\n",
		    ptr1->val,currinstancename,ptr1->val,
		    truewidth(ptr1->width));
	}
      ptr1 = ptr1->next;
    }
  fprintf(outfile,"\n}\n\n");
}


extern void updatedynamictables(tnode *, char *);

/****************************************************************/
/*                  generates kpass functions                   */
/****************************************************************/

void kpassmake(tnode * tbranch)

{
  tnode * tptr = tbranch;
  sigsym * ptr1;
  sigsym * gptr;

  currblockrate = KRATETYPE;

  if (currinstance == NULL)
    fprintf(outfile,"void %s_kpass("
	    "ENGINE_PTR_DECLARE_COMMA struct ninstr_types * nstate)\n{\n\n",
	    currinstancename);
  else
    fprintf(outfile,"void %s_kpass(ENGINE_PTR_DECLARE)\n{\n\n", currinstancename);

  fprintf(outfile,"   int i;\n\n");

  if (currinstrument->cref->itime)
    fprintf(outfile,
	    "   NS(iline->itime) = ((float)(EV(kcycleidx) - NS(iline->kbirth)))*EV(KTIME);\n\n");

  printdurassign();

  ptr1 = tptr->sptr;
  while (ptr1 != NULL)
    {
      if ((ptr1->rate == KRATETYPE)&&
	  ((ptr1->kind==K_IMPORT)||(ptr1->kind==K_IMPORTEXPORT)))
	{
	  gptr = getvsym(&globalsymtable,ptr1->val);
	  if (gptr != NULL)
	    {
	      if ((gptr->vartype != ptr1->vartype)||
		  (gptr->width != truewidth(ptr1->width))||
		  (gptr->rate != ptr1->rate))
		{
		  printf("Error: Import mismatch with global.\n\n");
		  showerrorplace(ptr1->defnode->linenum, 
				 ptr1->defnode->filename);
		}
	      if (ptr1->tref->mirror != GLOBALMIRROR)
		fprintf(outfile,
	      "   memcpy(&(NVU(%s_%s)), &(NGU(GBL_%s)), %i*sizeof(NGU(0)));\n",
		      currinstancename,ptr1->val,ptr1->val,
		      truewidth(ptr1->width));
	    }
	}
      ptr1 = ptr1->next;
    }
  updatedynamictables(tptr, currinstancename);
  blocktree(tptr->down,PRINTKPASS);
  ptr1 = tptr->sptr;
  while (ptr1 != NULL)
    {
      if ((ptr1->rate == KRATETYPE)&&
	  ((ptr1->kind==K_EXPORT)||(ptr1->kind==K_IMPORTEXPORT)))
	{
	  gptr = getvsym(&globalsymtable,ptr1->val);
	  if (gptr == NULL)
	    {
	      printf("Error: Import has no matching ksig global.\n\n");
	      showerrorplace(ptr1->defnode->linenum, 
			     ptr1->defnode->filename);
	    }
	  if ((gptr->vartype != ptr1->vartype)||
	      (gptr->width != truewidth(ptr1->width))||
	      (gptr->rate != ptr1->rate))
	    {
	      printf("Error: Import width mismatch with global.\n\n");
	      showerrorplace(ptr1->defnode->linenum, 
			     ptr1->defnode->filename);
	    }
	  if (ptr1->tref->mirror != GLOBALMIRROR)
	    fprintf(outfile,
	     "   memcpy(&(NGU(GBL_%s)), &(NVU(%s_%s)), %i*sizeof(NGU(0)));\n",
		    ptr1->val,currinstancename,ptr1->val,
		    truewidth(ptr1->width));
	}
      ptr1 = ptr1->next;
    }
  fprintf(outfile,"\n}\n\n");
}


/****************************************************************/
/*                  generates apass functions                   */
/****************************************************************/

void apassmake(tnode * tbranch)

{
  tnode * tptr = tbranch;
  tnode * bptr;
  int i;

  currblockrate = ARATETYPE;
  if (currinstance == NULL)
    fprintf(outfile, "void %s_apass(" 
	    "ENGINE_PTR_DECLARE_COMMA struct ninstr_types * nstate)\n{\n\n",
	    currinstancename);
  else
    {
      fprintf(outfile, "void %s_apass(ENGINE_PTR_DECLARE)\n{\n\n", currinstancename);

      if (shadowcheck())
	{
	  bptr = currinstance->ibus;
	  while (bptr)
	    {
	      for (i = 0; i < bptr->width; i++)
		{
		  fprintf(outfile, "   STB(BUS_%s + %i) = TB(BUS_%s + %i);\n",
			  bptr->val, i, bptr->val, i);
		  if (bptr->sptr == outputbus)
		    fprintf(outfile, "   TB(BUS_%s + %i) = 0.0F;\n",
			    bptr->val, i);
		}
	      bptr = bptr->next;
	    }
	  fprintf(outfile, "\n");
	}
    }
  blocktree(tptr->down,PRINTAPASS); 
  fprintf(outfile,"}\n\n");

}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*                                                              */
/*        Second level functions called by postscript()         */
/*                                                              */
/*______________________________________________________________*/



/****************************************************************/
/*     prints input[] functions for complex send()/route()'s    */
/****************************************************************/

void printinputgroups(void)

{
  tnode * instptr;
  tnode * pptr;
  int i,old;

  /* create functions for instances */

  instptr = instances;
  while (instptr != NULL)
    {

      if (instptr->usesinput)
	{
	  currinstance = instptr;   /* set inputbusmacro() context */
   
	  fprintf(outfile,"\n\nfloat finput%i(float findex)\n\n{\n",
		 instptr->arrayidx);
	  fprintf(outfile,"   int index = ROUND(findex);\n\n");
	  fprintf(outfile,"   switch(index) { \n");
	  pptr = instptr->ibus;
	  i = 0;
	  while ((pptr != NULL) && (i < instptr->inwidth))
	    {
	      old = i;
	      while ((i<(pptr->width + old))&&(i<instptr->inwidth))
		{
		  fprintf(outfile,"   case %i:\n",i);
		  i++;
		}
	      fprintf(outfile,"   return %s(BUS_%s - %i + index);\n",
		      inputbusmacro(), pptr->val,old);
	      pptr = pptr->next;
	    }
	  fprintf(outfile,"   default:\n");
	  fprintf(outfile,"    return 0.0F;\n");
	  fprintf(outfile,"   }\n}\n\n");
	}

      if (instptr->usesingroup)
	{
	  fprintf(outfile,
		  "\n\nfloat finGroup%i(float findex)\n\n{\n",
		 instptr->arrayidx);
	  fprintf(outfile,"   int index = ROUND(findex);\n\n");
	  fprintf(outfile,"   switch(index) { \n");
	  pptr = instptr->ibus;
	  i = 0;
	  while ((pptr != NULL) && (i < instptr->inwidth))
	    {
	      old = i;
	      while ((i<(pptr->width + old))&&(i<instptr->inwidth))
		{
		  fprintf(outfile,"   case %i:\n",i);
		  i++;
		}
	      fprintf(outfile,"   return %i.0F;\n",
		      old+1);
	      pptr = pptr->next;
	    }
	  fprintf(outfile,"   default:\n");
	  fprintf(outfile,"    return 0.0F;\n");
	  fprintf(outfile,"   }\n}\n\n");
	}
      instptr = instptr->next;
    }

}


extern void printmidiglobals(void);

/****************************************************************/
/*            prints systems_init function                      */
/****************************************************************/

void printsystemsinit(void)

{
  sigsym * iptr;
  int first = 0;

  redefglobal();

  fprintf(outfile,"\n\nENGINE_PTR_TYPE system_init(int argc, char **argv,"
	  " float sample_rate)\n");

  fprintf(outfile,"{\n");
  fprintf(outfile,"   int i;\n");
  fprintf(outfile,"   ENGINE_PTR_CREATE_SEMICOLON\n\n");

  iptr = globalsymtable;
  while (iptr != NULL)
    {
      if ((iptr->vartype == TABLETYPE) && (iptr->kind == K_NORMAL))
	{
	  if (!first)
	    {
	      fprintf(outfile,"   int j;\n");
	      first = 1;
	    }
	  wavegeneratorvar(iptr);
	}
      iptr = iptr->next;
    }

  fprintf(outfile,"\n   ENGINE_PTR_NULLRETURN_SEMICOLON\n\n");

  fprintf(outfile,"   engine_init(ENGINE_PTR_COMMA sample_rate);\n");
  if (fixedseed)
    fprintf(outfile,"\n   srand(9753193);\n");
  else
    fprintf(outfile,"\n   srand(((unsigned int)time(0))|1);\n");
    
  fprintf(outfile,"   EV(asys_argc) = argc;\n");
  fprintf(outfile,"   EV(asys_argv) = argv;\n\n");

  if (cin)
    {
      fprintf(outfile,"   EV(csys_argc) = argc;\n");
      fprintf(outfile,"   EV(csys_argv) = argv;\n\n");     
    }

  fprintf(outfile,"\n");

  if (session)
    {
      fprintf(outfile,"   if (signal(SIGINT, SIG_IGN) != SIG_IGN)\n");
      fprintf(outfile,"     if (signal(SIGINT, sigint_handler) == SIG_ERR)\n");
      fprintf(outfile,"        epr(0,NULL,NULL,\"%s\");\n\n",
	      "Can't set up SIGINT signal handler.");
    }

  if (catchsignals)
    {
      if (!session)
	{
	  fprintf(outfile,
		  "\n\n   if (signal(SIGINT, signal_handler) == SIG_ERR)\n");
	  fprintf(outfile,"      epr(0,NULL,NULL,\"%s\");\n",
		  "Can't set up SIGINT signal handler.");
	}
      fprintf(outfile,"   if (signal(SIGILL, signal_handler) == SIG_ERR)\n");
      fprintf(outfile,"      epr(0,NULL,NULL,\"%s\");\n",
	      "Can't set up SIGILL signal handler.");
      fprintf(outfile,"   if (signal(SIGABRT, signal_handler) == SIG_ERR)\n");
      fprintf(outfile,"      epr(0,NULL,NULL,\"%s\");\n",
	      "Can't set up SIGABRT signal handler.");
      fprintf(outfile,"   if (signal(SIGFPE, signal_handler) == SIG_ERR)\n");
      fprintf(outfile,"      epr(0,NULL,NULL,\"%s\");\n",
	      "Can't set up SIGFPE signal handler.");
      fprintf(outfile,"   if (signal(SIGSEGV, signal_handler) == SIG_ERR)\n");
      fprintf(outfile,"      epr(0,NULL,NULL,\"%s\");\n",
	      "Can't set up SIGSEGV signal handler.");
      fprintf(outfile,"   if (signal(SIGTERM, signal_handler) == SIG_ERR)\n");
      fprintf(outfile,"      epr(0,NULL,NULL,\"%s\");\n\n",
	      "Can't set up SIGTERM signal handler.");
    }

  printmidiglobals();

  fprintf(outfile,"   memset(&(NVU(GBL_STARTVAR)), 0, \n");
  fprintf(outfile,"          (GBL_ENDVAR-GBL_STARTVAR)*sizeof(NVU(0)));\n");

  /* later reclaim tables */

  fprintf(outfile,"   memset(&(NT(0)), 0, GBL_ENDTBL*sizeof(NT(0)));\n");

  if (startupinstr != NULL)
    {
      fprintf(outfile,"   EV(u_startup)[0].starttime = 0.0F;\n");
      fprintf(outfile,"   EV(u_startup)[0].endtime = MAXENDTIME;\n");
      fprintf(outfile,"   EV(u_startup)[0].abstime = 0.0F;\n");
      fprintf(outfile,"   EV(u_startup)[0].released = 0;\n");
      fprintf(outfile,"   EV(u_startup)[0].turnoff = 0;\n");
      fprintf(outfile,"   EV(u_startup)[0].time = 0.0F;\n");
      fprintf(outfile,"   EV(u_startup)[0].itime = 0.0F;\n");
      fprintf(outfile,"   EV(u_startup)[0].sdur = -1.0F;\n");
      fprintf(outfile,"   EV(u_startup)[0].kbirth = EV(kbase);\n\n");

      if (totmidichan)
	fprintf(outfile,"   EV(u_startup)[0].numchan = EV(midimasterchannel);\n\n");

      fprintf(outfile,"   EV(u_startup)[0].noteon = PLAYING;\n");
      fprintf(outfile,"   EV(u_startup)[0].notestate = EV(nextstate);\n");
      fprintf(outfile,"   EV(u_startup)[0].nstate = &(EV(ninstr)[EV(nextstate)]);\n");
      fprintf(outfile,"   EV(ninstr)[EV(nextstate)].iline = &EV(u_startup)[0];\n");
      fprintf(outfile,"   EV(nextstate)++;\n");
      fprintf(outfile,"   startup_ipass(ENGINE_PTR_COMMA EV(u_startup)[0].nstate);\n");
    }
  iptr = globalsymtable;
  currinputwidth = inchannels;
  currinstrwidth = outchannels;
  curropcodeprefix = currinstancename = "GBL";
  currinstrument = NULL;  
  currinstance = NULL;

  while (iptr != NULL)
    {
      if (iptr->vartype == TABLETYPE)
	{
	  if (iptr->kind == K_NORMAL)
	    createtable(iptr,"TBL_GBL", S_GLOBAL);
	}
      iptr = iptr->next;
    }
  curropcodeprefix = currinstancename = NULL;

  redefnormal();

  fprintf(outfile,"      memset(&(TB(0)), 0, ENDBUS*sizeof(TB(0)));\n\n");

  fprintf(outfile,"\n  ENGINE_PTR_RETURN_SEMICOLON\n");
  fprintf(outfile,"}\n\n");

}

/****************************************************************/
/*            print effects_init function                       */
/****************************************************************/

void printeffectsinit(void)

{
  sigsym * sptr;
  tnode * instptr;
  tnode * pptr;
  tnode * pvalptr;
  int i;

  /* print effects_init */

  redefglobal();
  fprintf(outfile,"\n\nvoid effects_init(ENGINE_PTR_DECLARE)\n{\n\n");

  sptr = instrnametable;
  while (sptr != NULL)
    {
      if (sptr->effects)
	{
	  i = -1;
	  instptr = instances;
	  while (instptr != NULL)
	    {
	      if (!strcmp(instptr->sptr->val, sptr->val))
		{

		  i++;
		  /* list of parameters in instr definition */
		  pptr = instptr->down->sptr->defnode->down->next->next->next->down;

		  /* list of values for these */
		  pvalptr = instptr->down->down->next->next->next->next->down;

		  while (pptr != NULL)
		    {
		      if (pvalptr == NULL)
			{
			  printf("Error: Send parameter list mismatches instr.\n\n");
			  showerrorplace(instptr->down->down->linenum, 
					 instptr->down->down->filename);
			}
		      fprintf(outfile,"EV(e_%s)[%i].p[%s_%s] = ",sptr->val,i,
			      instptr->val, pptr->val);
		      currinputwidth = inchannels;
		      currinstrwidth = outchannels;
		      widthupdate(pvalptr);
		      curropcoderate = ARATETYPE;
		      currtreerate = UNKNOWN;
		      rateupdate(pvalptr);
		      if (pvalptr->width != 1)
			{
			  printf("Error: Unindexed arrays in send parameter list.\n\n");
			  showerrorplace(instptr->down->down->linenum, 
					 instptr->down->down->filename);
			}
		      if (pvalptr->rate != IRATETYPE)
			{
			  printf("Error: Send parameters must be i-rate.\n\n");
			  showerrorplace(instptr->down->down->linenum, 
					 instptr->down->down->filename);
			}
		      if (pvalptr->vartype == TABLETYPE)
			{
			  printf("Error: Table used as send parameter.\n\n");
			  showerrorplace(instptr->down->down->linenum, 
					 instptr->down->down->filename);
			}
		      curropcodeprefix = currinstancename = "GBL";
		      currinstrument = NULL;  
		      currinstance = NULL;
		      currblockrate = IRATETYPE;
		      blocktree(pvalptr->down, PRINTTOKENS);
		      fprintf(outfile,";\n");
		      pptr = pptr->next;
		      pvalptr = pvalptr->next;
		      if (pptr != NULL)
			pptr = pptr->next;
		      if (pvalptr != NULL)
			pvalptr = pvalptr->next;
		    }

		  fprintf(outfile,"EV(e_%s)[%i].noteon = TOBEPLAYED;\n",sptr->val,i);

		  /* internal variables */

		  fprintf(outfile,"EV(e_%s)[%i].starttime = 0.0F;\n",sptr->val,i);
		  fprintf(outfile,"EV(e_%s)[%i].abstime = 0.0F;\n",sptr->val,i);
		  fprintf(outfile,"EV(e_%s)[%i].released = 0;\n",sptr->val,i);
		  fprintf(outfile,"EV(e_%s)[%i].turnoff = 0;\n",sptr->val,i);

		  /* standard names */

		  fprintf(outfile,"EV(e_%s)[%i].time = 0.0F;\n",sptr->val,i);
		  fprintf(outfile,"EV(e_%s)[%i].itime = 0.0F;\n",sptr->val,i);
		  fprintf(outfile,"EV(e_%s)[%i].sdur = -1.0F;\n",sptr->val,i);
		  fprintf(outfile,"EV(e_%s)[%i].kbirth = EV(kbase);\n",sptr->val,i);

		  if (totmidichan)
		    fprintf(outfile,"EV(e_%s)[%i].numchan = EV(midimasterchannel);\n",sptr->val,i);
		  
		}
	      instptr = instptr->next;
	    }
	}
      sptr = sptr->next;
    }
  
  fprintf(outfile,"\n\n}\n\n");
  redefnormal();

  
}

/****************************************************************/
/*               prints shut_down function                      */
/****************************************************************/

void printshutdown(void)

{
  sigsym * iptr;

  redefglobal();

  iptr = getvsym(&busnametable,"input_bus");

  fprintf(outfile,"\n\nvoid shut_down(ENGINE_PTR_DECLARE)\n   {\n\n");
  fprintf(outfile,"  if (EV(graceful_exit))\n");
  fprintf(outfile,"    {\n");
  fprintf(outfile,"      fprintf(stderr, \"\\nShutting down system ... please wait.\\n\");\n");
  fprintf(outfile,"      fprintf(stderr, \"If no termination in 10 seconds, use Ctrl-C or Ctrl-\\\\ to force exit.\\n\");\n");
  fprintf(outfile,"      fflush(stderr);\n");
  fprintf(outfile,"    }\n");

  if ((ainflow == PASSIVE_FLOW) && (aoutflow == PASSIVE_FLOW))
    fprintf(outfile,"   asys_putbuf(&EV(asys_obuf), &EV(obusidx));\n");

  if ((ain == aout) && (iptr != NULL) && (inchannels > 0))
    fprintf(outfile,"   asys_ioshutdown();\n");
  else
    {
      if ((iptr != NULL) && (inchannels > 0))
	fprintf(outfile,"   asys_ishutdown();\n");
      fprintf(outfile,"   asys_oshutdown();\n");
    }

  if (cin)
    fprintf(outfile,"\n   csys_shutdown(ENGINE_PTR);\n");

  if (session)
    fprintf(outfile,"   nsys_shutdown();\n");

  fprintf(outfile,"\n   ENGINE_PTR_DESTROY_SEMICOLON\n");
  fprintf(outfile,"   }\n\n");

  if (session)
    {
      fprintf(outfile,"\n");
      fprintf(outfile,"void sigint_handler(int signum)\n {\n");
      fprintf(outfile,"\n");
      fprintf(outfile,"  if (EV(graceful_exit))\n");
      fprintf(outfile,"   exit(130);\n");
      fprintf(outfile,"  EV(graceful_exit) = 1;\n");
      fprintf(outfile," }\n\n");
    }

  if (catchsignals)
    {      
      fprintf(outfile,"\n\nvoid signal_handler(int signum)\n {\n\n");
      fprintf(outfile,"   fprintf(stderr, \"\\n\\nRuntime Error\\n\");\n");
      fprintf(outfile,"   fprintf(stderr, \"Abnormal Termination -- \");\n");

      fprintf(outfile,"   switch(signum) {\n");

      if (!session)
	{      
	  fprintf(outfile,"    case SIGINT:\n");
	  fprintf(outfile,"     fprintf(stderr, \"Interrupt\\n\");\n");
	  fprintf(outfile,"     break;\n");
	}

      fprintf(outfile,"    case SIGILL:\n");
      fprintf(outfile,"     fprintf(stderr, \"Illegal instruction\\n\");\n");
      fprintf(outfile,"     break;\n");

      fprintf(outfile,"    case SIGABRT:\n");
      fprintf(outfile,"     fprintf(stderr, \"Aborted\\n\");\n");
      fprintf(outfile,"     break;\n");

      fprintf(outfile,"    case SIGFPE:\n");
      fprintf(outfile,"     fprintf(stderr, \"Floating point exception\\n\");\n");
      fprintf(outfile,"     break;\n");

      fprintf(outfile,"    case SIGSEGV:\n");
      fprintf(outfile,"     fprintf(stderr, \"Segmentation fault\\n\");\n");
      fprintf(outfile,"     break;\n");

      fprintf(outfile,"    case SIGTERM:\n");
      fprintf(outfile,"     fprintf(stderr, \"Terminated\\n\");\n");
      fprintf(outfile,"     break;\n");

      fprintf(outfile,"   }\n");
      fprintf(outfile,"  fprintf(stderr, \"Closing Files ...\");\n");
      fprintf(outfile,"  shut_down();\n");
      fprintf(outfile,"  fprintf(stderr, \" Completed. Exiting.\\n\\n\");\n");
      fprintf(outfile,"  exit(0);\n");
      fprintf(outfile," }\n\n");
    }
}

extern void printtempoloop(char *);
extern void printtableloop(char *);
extern void printcontrolloop(char *);

/****************************************************************/
/*    printmaincontrol: prints control/tempo/table loops        */
/****************************************************************/

void printmaincontrol(void)

{

  fprintf(outfile,"void main_control(ENGINE_PTR_DECLARE)\n");
  fprintf(outfile,"\n");
  fprintf(outfile,"{\n");

  if (csasl|| allsasl->numtable || abssasl->numtable)
    fprintf(outfile,"   int i;\n");
  if (allsasl->numtable || abssasl->numtable)
    fprintf(outfile,"   int len;\n");
  if (csasl && ((allsasl->numcontrol) || (abssasl->numcontrol)))
    fprintf(outfile,"instr_line * sysidx;\n");  /* later, a typedef */

  fprintf(outfile,"\n");

  if (allsasl->numcontrol)
    printcontrolloop("s");
  if (abssasl->numcontrol)
    printcontrolloop("sa");

  if (allsasl->numtable)
    printtableloop("s");
  if (abssasl->numtable)
    printtableloop("sa");
  if (csasl)
    makesasltablesys();

  if (allsasl->numtempo)
    printtempoloop("s");
  if (abssasl->numtempo)
    printtempoloop("sa");


  fprintf(outfile,"}\n");

}


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*                                                              */
/*     Utility functions called by second-level functions       */
/*                                                              */
/*______________________________________________________________*/


/****************************************************************/
/* timestamp check for table sasl commands: used in kpassmake() */
/****************************************************************/

void updatedynamictables(tnode * iptr, char * prefix)

{

  sigsym * sptr;
  sigsym * gptr;
  tnode * optr;
  int opnum;
  char newprefix[STRSIZE];
  char name[STRSIZE];

  sptr = iptr->sptr;
  while (sptr != NULL)
    {
      if ((sptr->vartype == TABLETYPE)&&(sptr->kind==K_IMPORT))
        {    
	  if ((gptr = getvsym(&globalsymtable, sptr->val)) == NULL)
	    internalerror("writeorc","updatedynamictable");
	  if ((sptr->tref->assigntot || gptr->tref->assigntot) && 
	      ((strlen(sptr->val) <= 5) || strncmp(sptr->val, "_sym_", 5)))
	    {
	      fprintf(outfile, "   if (EV(gtables)[TBL_GBL_%s].stamp > ",
		      sptr->val);
	      fprintf(outfile, "NT(TBL_%s_%s).stamp)\n",
		      prefix, sptr->val);
	      fprintf(outfile,"   {\n");
	      sprintf(name,"TBL_%s",prefix);
	      createtable(sptr,name, S_FUTURE);
	      fprintf(outfile,"   }\n");
	    }
	}
      sptr = sptr->next;
    }  
  optr = iptr->optr;
  while (optr != NULL)
    {
      if (optr->ttype == S_OPCALL)
	{
	  sprintf(newprefix,"%s_%s%i",prefix,optr->val,optr->arrayidx);
	  updatedynamictables(optr->sptr->defnode,newprefix);
	}
      else
	{
	  for (opnum = 0; opnum < truewidth(optr->opwidth);opnum++)
	    {
	      sprintf(newprefix,"%s_%soparray%i",prefix,optr->val,(opnum+1));
	      updatedynamictables(optr->sptr->defnode,newprefix);
	    }
	}
      optr = optr->next;
    }
}

/****************************************************************/
/*    prints MIDI global constants: used in system_init()       */
/****************************************************************/

void printmidiglobals(void)

{
  int i, j;
  tnode * tptr;

  if (totmidichan)
    {

      /****************************/
      /* set default MIDI volumes */
      /****************************/

      fprintf(outfile, "    /* MIDI volume */\n\n");
      fprintf(outfile, "   ");
      i = 0;
      for (j = MIDIVOLUMEPOS; j < totmidichan*MIDIFRAMELEN; 
	   j = j + MIDIFRAMELEN)
	{
	  fprintf(outfile, "EV(global)[%i].f = ", j);
	  if ((i++) == 3)
	    {
	      fprintf(outfile, "\n   ");
	      i = 0;
	    }
	}
      fprintf(outfile, " 100.0F; \n\n");

      /************************/
      /* set default MIDI Pan */
      /************************/

      fprintf(outfile, "    /* MIDI Pan */\n\n");
      fprintf(outfile, "   ");
      i = 0;
      for (j = MIDIPANPOS; j < totmidichan*MIDIFRAMELEN; 
	   j = j + MIDIFRAMELEN)
	{
	  fprintf(outfile, "EV(global)[%i].f = ", j);
	  if ((i++) == 3)
	    {
	      fprintf(outfile, "\n   ");
	      i = 0;
	    }
	}
      fprintf(outfile, " 64.0F; \n\n");

      /*************************/
      /* set default MIDI Expr */
      /*************************/

      fprintf(outfile, "    /* MIDI Expression */\n\n");
      fprintf(outfile, "   ");
      i = 0;
      for (j = MIDIEXPRPOS; j < totmidichan*MIDIFRAMELEN; 
	   j = j + MIDIFRAMELEN)
	{
	  fprintf(outfile, "EV(global)[%i].f = ", j);
	  if ((i++) == 3)
	    {
	      fprintf(outfile, "\n   ");
	      i = 0;
	    }
	}
      fprintf(outfile, " 127.0F; \n\n");

      /*************************/
      /* set default MIDI Bend */
      /*************************/

      fprintf(outfile, "    /* MIDI Bend */\n\n");
      fprintf(outfile, "   ");
      i = 0;
      for (j = MIDIBENDPOS; j < totmidichan*MIDIFRAMELEN; 
	   j = j + MIDIFRAMELEN)
	{
	  fprintf(outfile, "EV(global)[%i].f = ", j);
	  if ((i++) == 3)
	    {
	      fprintf(outfile, "\n   ");
	      i = 0;
	    }
	}
      fprintf(outfile, " 8192.0F; \n\n");

      /********************************/
      /* set Extended Channel Numbers */
      /********************************/

      fprintf(outfile, "    /* MIDI Ext Channel Number */\n\n   ");
      i = 0;
      for (j = 0; j < totmidichan; j++)
	{
	  fprintf(outfile, "EV(global)[%i].f = ", 
		  MIDIEXTPOS + j*MIDIFRAMELEN);
	  if (j < confmidi->midinumchan)
	    {
	      tptr = confmidi->imidiroot;
	      while (tptr != NULL)
		{
		  if (tptr->width == j)
		    {
		      fprintf(outfile, "%i.0F; ", tptr->res);
		      tptr = NULL;
		    }
		  else
		    tptr = tptr->next;
		}
	    }
	  else
	    {
	      if (j < sstrmidi->midinumchan)
		{
		  tptr = sstrmidi->imidiroot;
		  while (tptr != NULL)
		    {
		      if (tptr->width == j)
			{
			  fprintf(outfile, "%i.0F; ", tptr->res);
			  tptr = NULL;
			}
		      else
			tptr = tptr->next;
		    }
		}
	      else
		{
		  /* wrong, fix later */
		  fprintf(outfile, "%i.0F; ",
			  (int)(confmidi->miditracks*MCHAN + j
				- confmidi->midinumchan));
		}
	    }
	  if (++i == 3)
	    {
	      i = 0;
	      fprintf(outfile, "\n   ");
	    }
	}
      fprintf(outfile, "\n\n");
    }

}

/****************************************************************/
/*    prints tempo-changing loop: used in printmaincontrol()    */
/****************************************************************/

void printtempoloop(char * prefix)

{
  fprintf(outfile,"   while ((EV(%stempoidx) <= EV(end%stempo))&&\n",
	  prefix, prefix);

  if (strcmp("sa", prefix))
    fprintf(outfile,"        (stempo[EV(stempoidx)].t <= EV(scorebeats)))\n");
  else
    fprintf(outfile,"        (satempo[EV(satempoidx)].t <= EV(absolutetime)))\n");

  fprintf(outfile,"   {\n");  
  fprintf(outfile,"    EV(kbase) = EV(kcycleidx);\n");
  fprintf(outfile,"    EV(scorebase) = EV(scorebeats);\n");
  fprintf(outfile,"    EV(tempo) = %stempo[EV(%stempoidx)].newtempo;\n",
	  prefix, prefix);
  fprintf(outfile,"    EV(scoremult) = 1.666667e-02F*EV(KTIME)*EV(tempo);\n");
  if (allsasl->endtimeval)
    {
      fprintf(outfile,"    EV(endkcycle) = EV(kbase) + (int)\n");
      fprintf(outfile,
	      "      (EV(KRATE)*(EV(endtime) - EV(scorebase))*(60.0F/EV(tempo)));\n");
      if (abssasl->endtimeval)
	{
	  fprintf(outfile, 
		  "     EV(endkcycle) = (EV(endkinit) > EV(endkcycle)) ?\n");
	  fprintf(outfile, 
		  "                  EV(endkinit) : EV(endkcycle);\n\n");
	}
    }
  fprintf(outfile,"    EV(%stempoidx)++;\n", prefix);
  fprintf(outfile,"    }\n\n");

}


/****************************************************************/
/*    prints table-trigger loop: used in printmaincontrol()     */
/****************************************************************/

void printtableloop(char * prefix)

{
  int lc = 0;

  /* add table free(), but be sure not to free() static memory.*/

  z[lc++]="while ((EV(%stableidx) <= EV(end%stable)) &&"; 

  if (strcmp("sa", prefix))
    z[lc++]="     (%stable[EV(%stableidx)].t <= EV(scorebeats)))";
  else
    z[lc++]="     (%stable[EV(%stableidx)].t <= EV(absolutetime)))";  

  z[lc++]="   {";  
  z[lc++]="     if (NT((i = %stable[EV(%stableidx)].gindex)).llmem)";
  z[lc++]="      free(NT(i).t);";
  z[lc++]="     memset(&(NT(i)), 0, sizeof(NT(0)));";
  z[lc++]="     if ((len = %stable[EV(%stableidx)].size) < 0)"; 
  z[lc++]="       (*%stable[EV(%stableidx)].tmake) (ENGINE_PTR);";
  z[lc++]="     else"; 
  z[lc++]="      {"; 
  z[lc++]="        NT(i).stamp = EV(scorebeats);"; 
  if (interp == INTERP_SINC)
    {
      z[lc++]="        NT(i).sffl = 1.0F;";
      z[lc++]="        NT(i).sfui = 0x00010000;";
      z[lc++]="        NT(i).dsincr = SINC_PILEN;";
    }
  z[lc++]="        NT(i).lenf= (float)(NT(i).len = len);";
  z[lc++]="        NT(i).oconst= NT(i).lenf*EV(ATIME);";
  z[lc++]="        NT(i).tend = len - 1;";
  z[lc++]="        NT(i).t = (float *)(%stable[EV(%stableidx)].data);";
  z[lc++]="      }"; 
  z[lc++]="     EV(%stableidx)++;";
  z[lc++]="   }";

  printcontrolblock(lc, prefix);
}


/****************************************************************/
/*   prints control-trigger loop: used in printmaincontrol()    */
/****************************************************************/

void printcontrolloop(char * prefix)

{
  fprintf(outfile,"  while (EV(%scontrolidx) && (EV(%scontrolidx) <= EV(end%scontrol)) &&\n",
	  prefix, prefix, prefix);

  if (strcmp("sa", prefix))
    fprintf(outfile,"	 (EV(scontrolidx)->t <= EV(scorebeats)))\n");
  else
    fprintf(outfile,"	 (EV(sacontrolidx)->t <= EV(absolutetime)))\n");

  fprintf(outfile,"  {\n");
  fprintf(outfile,"    if (EV(%scontrolidx)->siptr < 0)\n", prefix);
  fprintf(outfile,"	EV(global)[EV(%scontrolidx)->imptr].f = EV(%scontrolidx)->imval;\n"
	  ,prefix, prefix);
  fprintf(outfile,"    else\n");
  fprintf(outfile,"	{\n");
  fprintf(outfile,"	  if ( (EV(%scontrolidx)->iline != NULL) &&\n", prefix);
  fprintf(outfile,"	       (EV(%scontrolidx)->iline->noteon == PLAYING) )\n",
	  prefix);
  fprintf(outfile,"          EV(%scontrolidx)->iline->nstate->v[EV(%scontrolidx)->imptr].f\n", prefix, prefix);
  fprintf(outfile,"	      = EV(%scontrolidx)->imval;\n", prefix);

  if (csasl)
    makesaslcrosscontrol( prefix);

  fprintf(outfile,"	}\n");


  fprintf(outfile,"    EV(%scontrolidx)++;\n", prefix);
  fprintf(outfile,"  }\n\n");

}

/****************************************************************/
/*    returns 1 if current context requires bus shadowing       */
/****************************************************************/

int shadowcheck(void)

{
  return (useshadowbus && (currinstance != NULL) &&
	  currinstance->sptr->cref->inmirror);
}

/****************************************************************/
/*        returns TB or STB as needed for context               */
/****************************************************************/

char * inputbusmacro(void)

{
  return ((useshadowbus && (currinstance != NULL) && 
	   currinstance->sptr->cref->inmirror) ? "STB" : "TB");
}

/****************************************************************/
/*                  diagnostic function -- not used             */
/****************************************************************/

void ptokens(tnode * tbranch)

{
  tnode * tptr = tbranch;

  while (tptr != NULL)
    {
      if ((tptr->down == NULL))
	{
	  if (tptr->val[0]!='<')
	    fprintf(outfile," %s ",tptr->val);
	  if ((tptr->val[0]==';')||(tptr->val[0]=='{'))
	    fprintf(outfile,"\n");
	}
      else
	{
	  ptokens(tptr->down);
	}
      
      tptr = tptr->next;
    }
}

