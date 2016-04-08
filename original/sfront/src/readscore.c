
/*
#    Sfront, a SAOL to C translator    
#    This file: Reads, processes, and generates code for SASL data.
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
/*                  void readscore(int)                         */
/*                                                              */
/* This top-level function for reading score data is called in  */
/* postparse.c, for both ASCII and binary score data. Note that */
/* some of the second-level score-reading functions located     */
/* later in this file are also used by readmidi.c.              */
/*                                                              */
/*______________________________________________________________*/

extern tnode * scorelex(int);
extern int parseend(sasdata *, tnode *);
extern int parsetempo(sasdata *, tnode *, int);
extern int parsetable(sasdata *, tnode *, int, int);
extern int parsecontrol(sasdata *, tnode *, int);
extern int parselcontrol(sasdata *, tnode *, int);
extern int parselinstr(sasdata *, tnode *, int, int);
extern int parseinstr(sasdata *, tnode *, int, int);
extern void showbadline(tnode *);
extern void badline(tnode *);

/*********************************************************/
/*     read .mp4/ASCII SASL: called in postparse.c       */
/*********************************************************/

void readscore(int scotype)

{
  sasdata * sdata = NULL;
  tnode * nsl;
  tnode * tptr;
  tnode * tokenptr;
  int found = 1;
  int tcount;
  int hpe;

  switch (scotype) {
  case BCONFSCORE:
    found = readprepare(BINSCORE); /* falls through */
  case FCONFSCORE:
    sdata = confsasl;
    break;
  case BSSTRSCORE:
    found = readprepare(BINSSTR); /* falls through */
    bitlinecount = -1;
  case FSSTRSCORE:
    sdata = sstrsasl;
    break;
  default:
    internalerror("readscore.c", "readscore() switch");
  }

  if (!found)
    return;

  while ((tptr = scorelex(scotype))->ttype != S_EOF)
    if (tptr->ttype != S_NEWLINE)
      {
	tokenptr = nsl = tptr;
	tcount = 1;

	if ((tptr->ttype == S_BADCHAR) || (tptr->ttype == S_BADNUMBER))
	  {
	    printf("Error: Misformed %s on SASL line.\n",
		   (tptr->ttype == S_BADCHAR) ? "token or string" : "number");
	    showbadline(nsl);
	  }

	while (( (tptr = scorelex(scotype))->ttype != S_NEWLINE ) &&
	       (tptr->ttype != S_EOF))
	  {

	    if ((tptr->ttype == S_BADCHAR) || (tptr->ttype == S_BADNUMBER))
	      {
		printf("Error: Misformed %s on SASL line.\n",
		       (tptr->ttype == S_BADCHAR) ? 
		       "token or string" : "number");
		showbadline(nsl);
	      }

	    if (tptr->ttype == S_STAR)
	      {
		printf("Error: '*' must be first symbol on SASL line.\n");
		showbadline(nsl);
	      }
	    tokenptr->next = tptr; 
	    tokenptr = tptr; 
	    tcount++;
	  }
	if (scotype == BSSTRSCORE)
	  sdata = bitscohastime ? sstrsasl : abssasl;
	if (nsl->ttype == S_STAR)
	  {
	    nsl = nsl->next;
	    hpe = 1;
	    tcount--;
	  }
	else
	  hpe = 0;
	found = 0;
	if (tcount <= 1)
	  badline(nsl);
	if (tcount == 2)
	  {
	    if (sdata->endtimeval != NULL)
	      sdata->scorefsize--;
	    found = parseend(sdata, nsl);
	  }
	if (tcount == 3)
	  {
	    found = parsetempo(sdata, nsl, hpe);
	    if (!found)
	      found = parseinstr(sdata, nsl, tcount, hpe);
	  }
	if ((!found) &&(tcount > 3) && (nsl->next->ttype == S_TABLE) )
	  found = parsetable(sdata, nsl, tcount, hpe);
	if ((!found) &&(tcount == 4) && (nsl->next->ttype == S_CONTROL))
	  found = parsecontrol(sdata, nsl, hpe);
	if ((!found) && (tcount == 5) && (nsl->next->next->ttype == S_CONTROL))
	  found = parselcontrol(sdata, nsl, hpe);
	if ((!found) && (tcount > 4))
	  found = parselinstr(sdata, nsl, tcount, hpe);
	if ((!found) &&(tcount > 3))
	  found = parseinstr(sdata, nsl, tcount, hpe);
	if (!found)
	  badline(nsl);
	sdata->scorefsize++;
	if (tptr->ttype == S_EOF)
	  {
	    if (cppsaol == 0)
	      printf("Warning: -sco file does not end in newline.\n");
	    break;
	  }
      }

}


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*                                                              */
/* Top-level functions for intermediate score processing        */
/*                 Called in sfmain.c                           */
/*                                                              */
/*______________________________________________________________*/

extern tnode * maketempomap(void);
extern int midipointers(tnode **, tnode **);
extern void timestampconvert(tnode *, int);

/*********************************************************/
/*    renumber abssasl and sstrmidi to absolute time     */
/*********************************************************/

void renumberabs(void)

{
  tnode * tempo, * control, * mstart, * mend;
  tnode * tmapptr, * tptr, * endptr;
  float tval, ktime, fval,  midiscale;
  float scorebase, scoremult, scorebeats;
  int kcycleidx, kbase,  hasmidi;
  
  
  /* logic: mstrfile always needs tempomap, but   */
  /*        sstrfile needs it only if writing.    */
  /* will need to change for has_time=0 support   */
  
  if ((boutfile && sstrfile) || mstrfile)
    tempomap = maketempomap();
  
  if (!mstrfile)
    return;
  
  /* add instr, table, and endval for has_time=0 support */
     
  tempo = abssasl->temporoot;
  control = abssasl->controlroot;

  /* do renumbering to absolute time */

  hasmidi = midipointers(&mstart, &mend);
  tmapptr = tempomap;
  ktime = 1.0F/krate;
  kcycleidx = kbase = 1;
  tval = 120.0F;           /* midi file default tempo */
  scorebase = 0.0F;
  ktime = scoremult = 1.0F/krate;
  midiscale = 1.0F/sstrmidi->miditicks;
  while (tempo || control || hasmidi)
    {
      scorebeats = scoremult*(kcycleidx - kbase) + scorebase;

      hasmidi = 0;

      /* convert starttimes for midinotes */

      tptr = mstart;
      while (tptr)
	{
	  while (tptr->down && 
		 (tptr->down->rate*midiscale <= scorebeats))
	    {
	      timestampconvert(tptr->down, kcycleidx);
	      tptr->down = tptr->down->next;
	    }
	  if (tptr->down)
	    {
	      hasmidi = 1;
	    }
	  tptr = tptr->next;
	}

      /* convert endtimes for midinotes */

      tptr = mend;
      while (tptr)
	{
	  endptr = tptr->down;
	  while (endptr && endptr->opwidth)
	    {
	      if (endptr->width*midiscale <= scorebeats)
		{
		  if (!endptr->inwidth) 
		    {
		      endptr->inwidth = kcycleidx;
		      endptr->time = (kcycleidx-1.5F)/(float)krate;
		    }
		  if (endptr == tptr->down)
		    tptr->down = tptr->down->next;
		}
	      endptr = endptr->next;
	    }
	  if (tptr->down)
	    hasmidi = 1;
	  tptr = tptr->next;
	}

      while (tempo && (tempo->time <= scorebeats))
	{
	  timestampconvert(tempo, kcycleidx);
	  tempo = tempo->next;
	}
      while (control && (control->time <= scorebeats))
	{
	  timestampconvert(control, kcycleidx);
	  control = control->next;
	}

      while (tmapptr && (tmapptr->time <= scorebeats))
	{
	  kbase = kcycleidx;
	  scorebase = scorebeats;
	  if ((fval = (float)atof(tmapptr->val)) < 0.0F)
	    printf("Warning: Encode ignoring negative tempo command.\n\n");
	  else
	    tval = fval;
	  scoremult = 1.666667e-02F*ktime*tval;
	  tmapptr = tmapptr->next;
	}
      kcycleidx++;
    }

  abssasl->compendtime = ((float)kcycleidx/krate);

  if (has.o_settempo)
    printf("Warning: Settempo() call ignored by streaming encoding\n\n");

}


extern void mergerootlist(tnode **, tnode **, tnode **, tnode **,
			  tnode **, tnode **);
extern void mergelabeltable(sigsym *); 

/*********************************************************/
/*       merge confsasl and sstrsasl into allsasl        */
/*********************************************************/

void mergescores(void)

{  
  int i;
  sigsym * sptr;

  vmcheck(allsasl = calloc(1, sizeof(sasdata)));

  /* after these calls, confsasl and sstrsasl root/tails are NULL */
  
  mergerootlist(&(confsasl->temporoot), &(confsasl->tempotail), 
		&(sstrsasl->temporoot), &(sstrsasl->tempotail),
		&(allsasl->temporoot), &(allsasl->tempotail));
  mergerootlist(&(confsasl->tableroot), &(confsasl->tabletail), 
		&(sstrsasl->tableroot), &(sstrsasl->tabletail),
		&(allsasl->tableroot), &(allsasl->tabletail));
  mergerootlist(&(confsasl->controlroot), &(confsasl->controltail), 
		&(sstrsasl->controlroot), &(sstrsasl->controltail),
		&(allsasl->controlroot), &(allsasl->controltail));
  mergerootlist(&(confsasl->instrroot), &(confsasl->instrtail), 
		&(sstrsasl->instrroot), &(sstrsasl->instrtail),
		&(allsasl->instrroot), &(allsasl->instrtail));

  /* confsasl/strsasl num's, scoresize, endtime still valid */

  allsasl->numtempo =  confsasl->numtempo + sstrsasl->numtempo;
  allsasl->numtable =  confsasl->numtable + sstrsasl->numtable;

  allsasl->scorefsize = confsasl->scorefsize + sstrsasl->scorefsize;

  allsasl->endtimeval = confsasl->endtimeval ? confsasl->endtimeval :
    sstrsasl->endtimeval;

  if (confsasl->endtimeval && sstrsasl->endtimeval)
    {
      if (atof(confsasl->endtimeval) > atof(sstrsasl->endtimeval))
	allsasl->endtimeval = confsasl->endtimeval;
      else
	allsasl->endtimeval = sstrsasl->endtimeval;
    }

  /* after this section, conf/sstr labeltables are null */

  allsasl->labeltable = confsasl->labeltable ? confsasl->labeltable :
    sstrsasl->labeltable;

  if (confsasl->labeltable && sstrsasl->labeltable)
    mergelabeltable(sstrsasl->labeltable);

  if (abssasl->labeltable)
    {
      i = 0;
      sptr = abssasl->labeltable;
      while (sptr)
	{
	  i++;
	  sptr = sptr->next;
	}
      abssasl->numlabels = i;
      mergelabeltable(abssasl->labeltable);
    }

  abssasl->labeltable = confsasl->labeltable = sstrsasl->labeltable =  NULL;

  i = 0;
  sptr = allsasl->labeltable;
  while (sptr)
    {
      sptr->special = i++;
      sptr = sptr->next;
    }
  allsasl->numlabels = i - abssasl->numlabels;
  

}


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*                                                              */
/* Top-level functions for printing out score data and score    */
/* functions. Called in writepre.c and sfmain.c.                */
/*                                                              */
/*______________________________________________________________*/

/*********************************************************/
/*     computes end-time of last SASL note for endtime   */
/*********************************************************/

void initendsasl(void)

{
  float tval;
  float newtval;

  if (allsasl->instrtail != NULL)
    {
      vmcheck(allsasl->endtimeval = (char *) calloc(1024, sizeof(char)));
      if (allsasl->instrtail->ttype == S_INSTR)
	{
	  if (atof(allsasl->instrtail->down->next->next->val)>0)
	    sprintf(allsasl->endtimeval,"%1.6e", 2.0F +
		    atof(allsasl->instrtail->val) + 
		    atof(allsasl->instrtail->down->next->next->val));
	  else
	    sprintf(allsasl->endtimeval,"%1.6e", 
		    atof(allsasl->instrtail->val)+10.0F);
	}
      else
	{
	  if (atof(allsasl->instrtail->down->next->
		   next->next->next->val) > 0 )
	    sprintf(allsasl->endtimeval,"%1.6e", 2.0F + 
		    atof(allsasl->instrtail->down->next->next->val) +
		    atof(allsasl->instrtail->
			 down->next->next->next->next->val));
	  else
	    sprintf(allsasl->endtimeval,"%1.6e", 10.0F +
		    atof(allsasl->instrtail->down->next->next->val));
	}
    }


  if (abssasl->instrtail != NULL)
    {
      /* determine worst-case tempo */

      tval = confmidi->miditicks ? 120.0F : 60.0F;
      vmcheck(abssasl->endtimeval = (char *) calloc(1024, sizeof(char)));
      if ((abssasl->tempotail != NULL)||(allsasl->tempotail != NULL))
	{
	  if (abssasl->tempotail)
	    {
	      newtval = (float)atof(abssasl->tempotail->down->next->next->val);
	      if ((newtval > 0.0F) && (newtval < tval))
		tval = newtval;
	    }
	  if (allsasl->tempotail)
	    {
	      newtval = (float)atof(allsasl->tempotail->down->next->next->val);
	      if ((newtval > 0.0F) && (newtval < tval))
		tval = newtval;
	    }
	}

      /* apply tempo to duration */

      if (abssasl->instrtail->ttype == S_INSTR)
	{
	  if (atof(abssasl->instrtail->down->next->next->val)>0)
	    sprintf(abssasl->endtimeval,"%1.6e", 2.0F +
		    atof(abssasl->instrtail->val) + (60.0F/tval)*
		    atof(abssasl->instrtail->down->next->next->val));
	  else
	    sprintf(abssasl->endtimeval,"%1.6e", 
		    atof(abssasl->instrtail->val)+10.0F);
	}
      else
	{
	  if (atof(abssasl->instrtail->down->next->
		   next->next->next->val) > 0 )
	    sprintf(abssasl->endtimeval,"%1.6e", 2.0F + 
		    atof(abssasl->instrtail->down->next->next->val) +
		    (60.0F/tval)*atof(abssasl->instrtail->
			 down->next->next->next->next->val));
	  else
	    sprintf(abssasl->endtimeval,"%1.6e", 10.0F +
		    atof(abssasl->instrtail->down->next->next->val));
	}
    }

}

/*********************************************************/
/*     declares initialized endtime variable             */
/*********************************************************/

void initendtime(void)

{

  sigsym * sptr;

  /* if absolute or relative endtimeval set, use it */
  /* if not, come up with an allsasl value */
 
  if ((allsasl->endtimeval == NULL) && (abssasl->endtimeval == NULL))
    {
      if (cin || session)
	fprintf(outfile,"#define CSYS_GIVENENDTIME 0\n\n");
 
      /* first, let MIDI streams set it */

      if (confmidi->miditicks > 0)
	{
	  vmcheck(allsasl->endtimeval = (char *) calloc(1024, sizeof(char)));
 	  sprintf(allsasl->endtimeval,"%1.6e",
		  ((float)confmidi->midimaxtime/confmidi->miditicks)+2.0F);
	}
      else
	{

	  /* or else let SASL do it */

	  if ((allsasl->instrtail != NULL)||(abssasl->instrtail != NULL))
	    initendsasl();
	  else
	    {
	      /* or else use absolute time or a constant */

	      if (abssasl->compendtime)
		{
		  vmcheck(abssasl->endtimeval = (char *) calloc(1024, 1));
		  sprintf(abssasl->endtimeval,"%f",abssasl->compendtime
			  + 2.0F);
		}
	      else
		{
		  vmcheck(allsasl->endtimeval = (char *) calloc(1024, 1));
		  sptr = getvsym(&busnametable,"input_bus");
		  if ((sptr == NULL) && (!cin) && (!session))
		    sprintf(allsasl->endtimeval,"-1.0");
		  else
		    sprintf(allsasl->endtimeval,"60.0");
		}
	    }
	}
    }
  else
    if (cin || session)
      fprintf(outfile,"#define CSYS_GIVENENDTIME 1\n\n");
    

  fprintf(outfile,"#define MAXENDTIME 1E+37\n\n");

  if (abssasl->endtimeval)
    fprintf(outfile,"int endkinit;\n");

  if (allsasl->endtimeval || abssasl->endtimeval)
    fprintf(outfile,"float endtime;\n");
  else
    internalerror("readscore.c", "initendtime tail");
}

/*************************************************************/
/* initialize endkinit and endtime. prints in engine_init() */
/*************************************************************/

void initendtimeassign(void)

{
  if (abssasl->endtimeval)
    fprintf(outfile,"  EV(endkinit) = %i;\n", 2 + (int)
	    (krate*atof(abssasl->endtimeval)));

  if (allsasl->endtimeval)
    fprintf(outfile,"  EV(endtime) = %sF;\n",allsasl->endtimeval);
  else
    {
      if (abssasl->endtimeval)
	fprintf(outfile,"  EV(endtime) = %fF;\n", 
		(1.0F/krate)*((2 + (int)(krate*atof(abssasl->endtimeval)))));
      else 
	internalerror("readscore.c", "initendtime tail");
    }
  fprintf(outfile,"\n");
}


/*********************************************************/
/*       declarations for the score instr variables      */
/*********************************************************/

void initscoreinstr(int type, sigsym * sptr)

{
  int score;
  char * prefix;
  char * val;

  if (type == RELTSTAMP)
    {
      if (!( allsasl->instrroot))
	return;
      prefix = "s";
      score = sptr->score;
    }
  else
    {
      if (!(abssasl->instrroot))
	return;
      prefix = "sa";
      score = sptr->ascore;
    }

  fprintf(outfile,"instr_line %s_%s[%i];\n",prefix,sptr->val, score);

  if ((score > 1) || csasl)
    {
      val = dupunderscore(sptr->val);
      fprintf(outfile,"instr_line * %s_%sfirst;\n", prefix, val);
      fprintf(outfile,"instr_line * %s_%slast;\n", prefix, val);
      fprintf(outfile,"instr_line * %s_%send;\n\n", prefix,val);
      free(val);
    }

}

/*********************************************************/
/*   engine_init() assignments for instr variables      */
/*********************************************************/

void initscoreinstrassign(int type, sigsym * sptr)

{
  int i, score;
  tnode * tptr;
  char * prefix;
  char * val;

  if (type == RELTSTAMP)
    {
      if (!(tptr = allsasl->instrroot))
	return;
      prefix = "s";
      score = sptr->score;
    }
  else
    {
      if (!(tptr = abssasl->instrroot))
	return;
      prefix = "sa";
      score = sptr->ascore;
    }

  fprintf(outfile,"  memcpy(EV(%s_%s), %s_%s_init, sizeof EV(%s_%s));\n",
	  prefix, sptr->val, prefix, sptr->val, prefix, sptr->val);

  i = -1;
  while (tptr != NULL)
    {
      if (tptr->sptr == sptr)
	{
	  i++;
	}
      tptr= tptr->next;
    }

  if ((score > 1) || csasl)
    {
      val = dupunderscore(sptr->val);
      fprintf(outfile,"  EV(%s_%sfirst) = &EV(%s_%s)[0];\n",
	      prefix,val,prefix,sptr->val);
      fprintf(outfile,"  EV(%s_%slast) = &EV(%s_%s)[0];\n",
	      prefix,val,prefix,sptr->val);
      fprintf(outfile,"  EV(%s_%send) = &EV(%s_%s)[%i];\n\n",
	      prefix,val,prefix,sptr->val,i);
      free(val);
    }
}


/*********************************************************/
/* declare and init true constant vars for score instrs  */
/*********************************************************/

void initscoreinstrconstant(int type, sigsym * sptr)

{

  int i, j, score;
  tnode * tptr;
  tnode * pvalptr;
  char * prefix;
  sigsym * label;

  if (type == RELTSTAMP)
    {
      if (!(tptr = allsasl->instrroot))
	return;
      prefix = "s";
      score = sptr->score;
    }
  else
    {
      if (!(tptr = abssasl->instrroot))
	return;
      prefix = "sa";
      score = sptr->ascore;
    }

  fprintf(outfile,"instr_line %s_%s_init[%i] = {\n",prefix,sptr->val,
	  score);

  i = -1;
  while (tptr != NULL)
    {
      if (tptr->sptr == sptr)
	{
	  i++;
	  if (i != 0)
	    fprintf(outfile,",\n");
	  fprintf(outfile,"{");
	  tptr->arrayidx = i;

	  /* float starttime, float endtime, float startabs, float endabs */

	  if (type == RELTSTAMP)
	    fprintf(outfile," %sF, MAXENDTIME, MAXENDTIME, MAXENDTIME, ",
		    tptr->val);
	  else
	    fprintf(outfile," MAXENDTIME, MAXENDTIME, %sF, MAXENDTIME, ",
		    tptr->val);

	  /* float abstime, float time, float itime */

	  fprintf(outfile," 0.0F, 0.0F, 0.0F, ");

	  /* float sdur */

	  if (tptr->down->next->ttype == S_COL)
	    {
	      fprintf(outfile, "%sF, ",tptr->down->next->next->next->next->val);
	      pvalptr = tptr->down->next->next->next->next->next;
	      label = getsym(&(allsasl->labeltable), tptr->down);
	    }
	  else
	    {
	      fprintf(outfile, " %sF, ",tptr->down->next->next->val);
	      pvalptr = tptr->down->next->next->next;
	      label = NULL;
	    }

	  /* int kbirth, int released, int turnoff, int noteon,  */
	  /* int notestate, int launch, int numchan, int preset, */
	  /* int notenum */

	  fprintf(outfile,"0, 0, 0, 1, 0, 0, 0, 0, 0,");

	  /* int label */

	  if (label)
	    fprintf(outfile, " %i,", label->special + 1);
	  else
	    fprintf(outfile, " 0,");

	  /* float p[] */

	  fprintf(outfile," {");
	  j = numpfields;
	  while (j > 0)
	    {
	      if (pvalptr != NULL)
		{
		  fprintf(outfile," %sF ",pvalptr->val);
		  pvalptr = pvalptr->next;	
		}
	      else
		fprintf(outfile," 0.0F ");
	      if (!(--j))
		fprintf(outfile,"},");
	      else
		fprintf(outfile,",");
	    }


	  /* struct ninstr_types * nstate */

	  fprintf(outfile," NULL "); /* last 3:*pass*/
	  fprintf(outfile,"}");
	}
      tptr= tptr->next;
    }
  fprintf(outfile,"};\n\n");

}


/*********************************************************/
/*       declarations for the score control variables    */
/*********************************************************/

void initscorecontrol(int type)

{
  char * prefix;
  sasdata * sdata;
  tnode * tptr;
  tnode * lptr;
  sigsym * psptr;
  sigsym * label;

  if (type == RELTSTAMP)
    {
      sdata = allsasl;
      prefix = "s";
    }
  else
    {
      sdata = abssasl;
      prefix = "sa";
    }

  sdata->numcontrol = 0;
  tptr = sdata->controlroot;
  while (tptr != NULL)
    {
      if (tptr->ttype == S_LCONTROL)
	{

	  label = getsym(&(allsasl->labeltable), tptr->down->next);
	  if (label == NULL)
	    {
	      printf("Error: Unknown score label %s.\n",tptr->down->next->val);
	      showbadline(tptr->down);
	    }
	  lptr = label->defnode;
	  while (lptr != NULL)
	    {
	      psptr = getsym(&(lptr->down->sptr->defnode->sptr),
			     tptr->down->next->next->next);
	      if ((psptr != NULL) && (psptr->kind == K_IMPORT))
		sdata->numcontrol++;
	      lptr = lptr->next;
	    }
	}
      else
	{
	  psptr = getsym(&globalsymtable,tptr->down->next->next);
	  if ((psptr != NULL) ||
	      (!strcmp("MIDIctrl",tptr->down->next->next->val)) ||
	      (!strcmp("MIDIbend",tptr->down->next->next->val)) ||
	      (!strcmp("MIDItouch",tptr->down->next->next->val)) )
	    sdata->numcontrol++;
	}
      tptr = tptr->next;
    }

  if (sdata->numcontrol == 0)
    return;

  fprintf(outfile, "scontrol_lines %scontrol[%i];\n", prefix, sdata->numcontrol);
  fprintf(outfile, "scontrol_lines * %scontrolidx;\n", prefix);
  fprintf(outfile, "scontrol_lines * end%scontrol;\n\n", prefix);
}

/*********************************************************/
/*   content_init() assignments for control variables    */
/*********************************************************/

void initscorecontrolassign(int type)

{
  char * prefix;
  sasdata * sdata;
  sigsym * sptr; 
  tnode * tptr;
  tnode * lptr;
  sigsym * psptr;
  sigsym * label;
  int i;

  if (type == RELTSTAMP)
    {
      sdata = allsasl;
      prefix = "s";
    }
  else
    {
      sdata = abssasl;
      prefix = "sa";
    }

  if (sdata->numcontrol == 0)
    return;

  fprintf(outfile,"  memcpy(EV(%scontrol), %scontrol_init, sizeof EV(%scontrol));\n",
	  prefix, prefix, prefix);

  i = 0;
  tptr = sdata->controlroot;
  while (tptr != NULL)
    {
      if (tptr->ttype == S_LCONTROL)
	{

	  label = getsym(&(allsasl->labeltable), tptr->down->next);
	  if (label == NULL)
	    {
	      printf("Error: Unknown score label %s.\n",tptr->down->next->val);
	      showbadline(tptr->down);
	    }
	  lptr = label->defnode;
	  while (lptr != NULL)
	    {
	      sptr = lptr->down->sptr; 
	      psptr = getsym(&(lptr->down->sptr->defnode->sptr),
			     tptr->down->next->next->next);
	      if ((psptr != NULL) && (psptr->kind == K_IMPORT))
		{
		  fprintf(outfile, "  EV(%scontrol)[%i].iline = &EV(%s_%s)[%i];\n", 
			  prefix, i, prefix, sptr->val, 
			  lptr->down->arrayidx);
		  i++;
		}
	      lptr = lptr->next;
	    }
	}
      else
	{
	  psptr = getsym(&globalsymtable,tptr->down->next->next);
	  if ((psptr != NULL) ||
	      (!strcmp("MIDIctrl",tptr->down->next->next->val)) ||
	      (!strcmp("MIDIbend",tptr->down->next->next->val)) ||
	      (!strcmp("MIDItouch",tptr->down->next->next->val)) )
	    i++;
	}
      tptr = tptr->next;
    }

  fprintf(outfile, "  EV(%scontrolidx) = &EV(%scontrol)[0];\n", prefix, prefix);
  fprintf(outfile, "  EV(end%scontrol) = &EV(%scontrol)[%i];\n\n",
	  prefix, prefix, sdata->numcontrol-1);
}

/*********************************************************/
/* declare and init true constant vars for score control */
/*********************************************************/

void initscorecontrolconstant(int type)

{
  char * prefix;
  sasdata * sdata;
  int i;
  tnode * tptr;
  tnode * lptr;
  sigsym * psptr;
  sigsym * label;

  if (type == RELTSTAMP)
    {
      sdata = allsasl;
      prefix = "s";
    }
  else
    {
      sdata = abssasl;
      prefix = "sa";
    }

  sdata->numcontrol = 0;
  tptr = sdata->controlroot;
  while (tptr != NULL)
    {
      if (tptr->ttype == S_LCONTROL)
	{

	  label = getsym(&(allsasl->labeltable), tptr->down->next);
	  if (label == NULL)
	    {
	      printf("Error: Unknown score label %s.\n",tptr->down->next->val);
	      showbadline(tptr->down);
	    }
	  lptr = label->defnode;
	  while (lptr != NULL)
	    {
	      psptr = getsym(&(lptr->down->sptr->defnode->sptr),
			     tptr->down->next->next->next);
	      if ((psptr != NULL) && (psptr->kind == K_IMPORT))
		sdata->numcontrol++;
	      lptr = lptr->next;
	    }
	}
      else
	{
	  psptr = getsym(&globalsymtable,tptr->down->next->next);
	  if ((psptr != NULL) ||
	      (!strcmp("MIDIctrl",tptr->down->next->next->val)) ||
	      (!strcmp("MIDIbend",tptr->down->next->next->val)) ||
	      (!strcmp("MIDItouch",tptr->down->next->next->val)) )
	    sdata->numcontrol++;
	}
      tptr = tptr->next;
    }

  if (sdata->numcontrol == 0)
    return;

  fprintf(outfile, "scontrol_lines %scontrol_init[%i] = {\n", 
	  prefix, sdata->numcontrol);
  i = -1;
  tptr = sdata->controlroot;
  while (tptr != NULL)
    {
      if (tptr->ttype == S_LCONTROL)
	{

	  label = getsym(&(allsasl->labeltable),tptr->down->next);
	  lptr = label->defnode;
	  while (lptr != NULL)
	    {
	      psptr = getsym(&(lptr->down->sptr->defnode->sptr),
			     tptr->down->next->next->next);
	      if ((psptr != NULL) && (psptr->kind == K_IMPORT))
		{
		  i++;
		  if (i != 0)
		    fprintf(outfile,",\n");
		  fprintf(outfile,"{");
		  fprintf(outfile," %sF,", tptr->val);           /* t */
		  fprintf(outfile," %i,", label->special + 1);   /* label */
		  fprintf(outfile," %i, ",lptr->down->arrayidx); /* siptr */
		  fprintf(outfile," NULL, ");                    /* iline */
		  fprintf(outfile,"%s_%s, ",                    /* imptr */
			  lptr->down->sptr->val,psptr->val);
		  fprintf(outfile,"%sF ",                       /* imval */
			  tptr->down->next->next->next->next->val);
		  fprintf(outfile,"}");
		}
	      lptr = lptr->next;
	    }
	}
      else
	{
	  psptr = getsym(&globalsymtable,tptr->down->next->next);
	  if ((psptr != NULL) ||
	      (!strcmp("MIDIctrl",tptr->down->next->next->val)) ||
	      (!strcmp("MIDIbend",tptr->down->next->next->val)) ||
	      (!strcmp("MIDItouch",tptr->down->next->next->val)) )
	    {
	      i++;
	      if (i != 0)
		fprintf(outfile,",\n");
	      fprintf(outfile,"{");
	      fprintf(outfile," %sF, 0, -1, NULL,",tptr->val);       
                                               /* t, label,siptr, iline */
	      if (psptr != NULL)
		fprintf(outfile," GBL_%s, ", psptr->val);        /* imptr */
	      else
		{
		  if (!strcmp("MIDIctrl",tptr->down->next->next->val))
		    fprintf(outfile," %i, ", MIDIFRAMELEN*
			    tptr->down->next->next->down->width + 
			    MIDICTRLPOS + tptr->down->next->next->arrayidx);  
		  if (!strcmp("MIDIbend",tptr->down->next->next->val))
		    fprintf(outfile," %i, ", MIDIFRAMELEN*
			    tptr->down->next->next->down->width + 
			    MIDIBENDPOS);  
		  if (!strcmp("MIDItouch",tptr->down->next->next->val))
		    {
		      if (tptr->down->next->next->arrayidx >= 0)
			fprintf(outfile," %i, ", MIDIFRAMELEN*
				tptr->down->next->next->down->width + 
			MIDITOUCHPOS + tptr->down->next->next->arrayidx);
		      else
			fprintf(outfile," %i, ", MIDIFRAMELEN*
				tptr->down->next->next->down->width + 
				MIDICHTOUCHPOS);
		    }
		}
	      fprintf(outfile," %sF ",                            /* imval */
		      tptr->down->next->next->next->val);
	      fprintf(outfile,"}");
	    }
	}
      tptr = tptr->next;
    }

  fprintf(outfile,"\n};\n\n");
}


/*********************************************************/
/*       declarations for the score tempo variables      */
/*********************************************************/

void initscoretempo(int type)

{
  char * prefix;

  if (type == RELTSTAMP)
    {
      if (!(allsasl->temporoot))
	return;
      prefix = "s";
    }
  else
    {
      if (!(abssasl->temporoot))
	return;
      prefix = "sa";
    }
  
  fprintf(outfile,"int end%stempo;\n", prefix);
  fprintf(outfile,"int %stempoidx;\n\n", prefix);
}

/*********************************************************/
/*   content_init() assignments for the tempo variables  */
/*********************************************************/

void initscoretempoassign(int type)

{
  char * prefix;
  tnode * tptr;
  int i;
  
  if (type == RELTSTAMP)
    {
      if (!(tptr = allsasl->temporoot))
	return;
      prefix = "s";
    }
  else
    {
      if (!(tptr = abssasl->temporoot))
	return;
      prefix = "sa";
    }

  i = 0;
  while (tptr != NULL)
    {
      i++;
      tptr = tptr->next;
    }
  
  fprintf(outfile,"  EV(end%stempo) = %i;\n",prefix,i-1);
}


/*********************************************************/
/* declare and init "true constant" vars for score tempo */
/*********************************************************/

void initscoretempoconstant(int type)

{
  char * prefix;
  int i,  numtempo;
  tnode * tptr;
  

  if (type == RELTSTAMP)
    {
      if (!(tptr = allsasl->temporoot))
	return;
      numtempo = allsasl->numtempo;
      prefix = "s";
    }
  else
    {
      if (!(tptr = abssasl->temporoot))
	return;
      numtempo = abssasl->numtempo;
      prefix = "sa";
    }

  fprintf(outfile, "stempo_lines %stempo[%i] = {\n", prefix, numtempo + 1);

  i = 0;
  while (tptr != NULL)
    {
      if (i++ != 0)
	fprintf(outfile,",\n");
      fprintf(outfile,"{");
      fprintf(outfile,"%sF, %sF",tptr->val,tptr->down->next->next->val);
      fprintf(outfile,"}");
      tptr = tptr->next;
    }
  fprintf(outfile,"};\n\n");
}

/*********************************************************/
/*       declarations for the score table variables      */
/*********************************************************/

void initscoretablevars(int type)

{
  char * prefix;

  if (type == RELTSTAMP)
    {
      if (!(allsasl->tableroot))
	return;
      prefix = "s";
    }
  else
    {
      if (!(abssasl->tableroot))
	return;
      prefix = "sa";
    }

  fprintf(outfile,"int end%stable;\n\n", prefix);
  fprintf(outfile,"int %stableidx;\n\n", prefix);
}  

/*********************************************************/
/*   content_init() assignments for the table variables  */
/*********************************************************/

void initscoretableassign(int type)

{
  int i;
  tnode * tptr;
  char * prefix;

  if (type == RELTSTAMP)
    {
      if (!(tptr = allsasl->tableroot))
	return;
      prefix = "s";
    }
  else
    {
      if (!(tptr = abssasl->tableroot))
	return;
      prefix = "sa";
    }

  i = 0;
  while (tptr != NULL)
    {
      i++;
      tptr = tptr->next;
    }

  fprintf(outfile,"  EV(end%stable) = %i;\n\n",prefix,i-1);
}  


/*********************************************************/
/* declare and init true constant vars for score tables  */
/*********************************************************/

void initscoretableconstant(int type)

{
  int i, numtable;
  tnode * tptr;
  tnode * defnode;
  char * prefix;

  if (type == RELTSTAMP)
    {
      if (!(tptr = allsasl->tableroot))
	return;
      numtable = allsasl->numtable;
      prefix = "s";
    }
  else
    {
      if (!(tptr = abssasl->tableroot))
	return;
      numtable = abssasl->numtable;
      prefix = "sa";
    }

  fprintf(outfile,"stable_lines %stable[%i] = {\n", 
	  prefix, numtable+1);

  i = 0;
  while (tptr != NULL)
    {
      if (i != 0)
	fprintf(outfile,",");

      fprintf(outfile,"\n{");

      defnode = tptr->sptr->defnode;
      
      fprintf(outfile," %sF, TBL_GBL_%s, %i, ", 
	      tptr->val, defnode->down->next->val, 
	      defnode->usesinput ? defnode->arrayidx : -1);

      if (defnode->usesinput)
	fprintf(outfile,"NULL, &(score_%stdata%i[0])", prefix, i);
      else
	fprintf(outfile," score_%stable%i, NULL", prefix, i);

      fprintf(outfile,"}");
      i++;
      tptr = tptr->next;
    }
  fprintf(outfile,"};\n\n");
}  


/*********************************************************/
/*       initializes score table externs             */
/*********************************************************/

void initscoretableexterns(int type)

{
  int i, numtable;
  tnode * tptr;
  char * prefix;

  if (type == RELTSTAMP)
    {
      if (!(tptr = allsasl->tableroot))
	return;
      numtable = allsasl->numtable;
      prefix = "s";
    }
  else
    {
      if (!(tptr = abssasl->tableroot))
	return;
      numtable = abssasl->numtable;
      prefix = "sa";
    }

  i = 0;
  while (i < numtable)
    {
      fprintf(outfile, "extern void score_%stable%i(ENGINE_PTR_TYPE);\n", prefix,i);
      fprintf(outfile, "extern %s score_%stdata%i[];\n", 
	      hexstrings ? "char" : "float", prefix, i);
      fprintf(outfile, "\n");
      i++;
    }
  fprintf(outfile, "\n");
}  


/*********************************************************/
/* prints functions to initialize SASL tables: sfmain.c  */
/*********************************************************/

void printtablefunctions(void)

{
  int i;
  tnode * tptr;
  char name[STRSIZE];

  currinputwidth = 1;
  currinstrwidth = 1;
  currinstancename = "GBL";
  curropcodeprefix  = "GBL";
  currinstrument = NULL;  
  currinstance = NULL;
  curropcodestack = NULL;
  redefglobal();

  tptr = allsasl->tableroot;
  i = -1;
  while (tptr != NULL)
    {
      i++;
      if (tptr->sptr->defnode->usesinput)
	{
	  sprintf(name, "score_stdata%i",i); 
	  printtablestring(tptr->sptr, name);
	}
      else
	{
	  fprintf(outfile,"\n\nvoid score_stable%i(ENGINE_PTR_DECLARE)\n{\n\n",i);
	  if (wavegeneratorname(tptr->down->next->next->next))
	    {
	      wavegeneratorvar(tptr->sptr);
	      fprintf(outfile,"   int i,j;\n\n");
	      createtable(tptr->sptr, "TBL_GBL", S_SASLFILE);
	    }
	  fprintf(outfile,"}\n\n");
	}
      tptr = tptr->next;
    }


  tptr = abssasl->tableroot;
  i = -1;
  while (tptr != NULL)
    {
      i++;
      if (tptr->sptr->defnode->usesinput)
	{
	  sprintf(name, "score_satdata%i",i); 
	  printtablestring(tptr->sptr, name);
	}
      else
	{
	  fprintf(outfile,"\n\nvoid score_satable%i(ENGINE_PTR_DECLARE)\n{\n\n",i);
	  if (wavegeneratorname(tptr->down->next->next->next))
	    {
	      wavegeneratorvar(tptr->sptr);
	      fprintf(outfile,"   int i,j;\n\n");
	      createtable(tptr->sptr, "TBL_GBL", S_SASLFILE);
	    }
	  fprintf(outfile,"}\n\n");
	}
      tptr = tptr->next;
    }

  redefnormal();
}


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*                                                              */
/* Second-level functions for reading score data. Note that     */
/* parsetempo() and parsecontrol() also called by readmidi.c    */
/*                                                              */
/*______________________________________________________________*/


/*********************************************************/
/*          lexical analyzer for score files             */
/*********************************************************/

tnode * scorelex (int scotype)

{
  FILE * sfile = NULL;
  int c,i,foundit, hashmark;
  char buf[LEXBUFSIZE] = {'\0'};
  tnode * scorelval;

  switch (scotype) {
  case BCONFSCORE:
    return binconflex();
    break;
  case FCONFSCORE:
    sfile = saslfile;
    break;
  case BSSTRSCORE:
    return binsstrlex();
    break;
  case FSSTRSCORE:
    sfile = sstrfile;
    break;
  default:
    internalerror("readscore.c", "scorelex() switch");
  }

  scorelval = make_tnode("", S_BADCHAR);

  /* delete whitespace, hash-defines, and comments */
 
  foundit = 0;
  while (!foundit)
    {
      while ((isspace(c = getc(sfile)))&&(c != '\n'));
      if ((c != '#') && (c != '/')) 
	foundit = 1;
      else
	{
	  hashmark = (c == '#');
	  c = getc(sfile);
	  if ((hashmark == 0) && (c != '/'))
	    {
	      ungetc(c,sfile);
	      buf[0]='/'; buf[1]='\0';
	      scorelval->val =  dupval(buf);
	      scorelval->ttype = S_BADCHAR;
	      return scorelval;
	    }
	  else
	    {
	      while ((c != '\n') && (c != EOF))
		c = getc(sfile);
	      if (c == EOF)
		foundit = 1;
	    }
	}
    }

  /* string constant */

  if (c == '"')
    {
      i=0; c = getc(sfile);
      while (c != EOF)
	if (c != '"')
	  {	      
	    if (i < LEXBUFSIZE)
	      buf[i++]=(char)c; 
	    c = getc(sfile);
	  }
	else
	  {
	    if ((i!=0)&&buf[i-1]=='\\')
	      {
		buf[i-1]= '"';
		c = getc(sfile);
	      }
	    else
	      {
		if (i < LEXBUFSIZE)
		  {
		    buf[i]='\0';
		    scorelval->ttype = S_STRCONST;
		  }
		else
		  {
		    /* string overflows buffer -- keep S_BADCHAR */

		    buf[LEXBUFSIZE - 1]= '\0';
		  }

		scorelval->val =  dupval(buf);
		return scorelval; 
	      }
	  }

      /* unterminated string constant fills file  -- keep S_BADCHAR */

      if (i < LEXBUFSIZE)
	buf[i]= '\0';
      else
	buf[LEXBUFSIZE - 1]= '\0';

      scorelval->val =  dupval(buf);
      return scorelval; 
    }

  /* parse integers and numbers */

  i = 0;
  if (c == '-')
    {
      buf[i]= '-'; i++;                           /* can't overflow */
      c = getc(sfile);
      if (!(isdigit(c)||(c == '.')))
	{
	  ungetc(c,sfile);
	  buf[i]='\0';                            /* can't overflow */
	  scorelval->val =  dupval(buf);
	  scorelval->ttype = S_BADNUMBER;
	  return scorelval;
	}
    }
  if (isdigit(c)||(c == '.'))    
    {
      if (c == '.')
	{
	  buf[i]= '0'; i++;                       /* can't overflow */
	  c = getc(sfile);
	  if (!isdigit(c))
	    {
	      ungetc(c,sfile);
	      buf[i]='\0';
	      scorelval->val =  dupval(buf);
	      scorelval->ttype = S_BADNUMBER;
	      return scorelval;
	    }
	  else
	    {
	      ungetc(c,sfile);
	      c = '.';
	    }
	}
      else
	{
	  while (isdigit(c))
	    {	     
	      if (i < LEXBUFSIZE)
		buf[i++]=(char)c; 
	      c = getc(sfile);
	    }
	  if ((c != 'e') && (c != '.'))      /* an integer, convert to number */
	    {
	      ungetc(c,sfile);
	      if (i + 2 < LEXBUFSIZE)
		{
		  buf[i]='.'; i++;
		  buf[i]='0'; i++;
		  buf[i]='\0';
		  scorelval->ttype = S_NUMBER;
		}
	      else
		{
		  buf[(i < LEXBUFSIZE) ? i : LEXBUFSIZE - 1] = '\0';
		  scorelval->ttype = S_BADNUMBER;
		}
	      scorelval->val =  dupval(buf);
	      return scorelval;
	    }
	}
      if (c == '.')
	{	      
	  if (i < LEXBUFSIZE)
	    buf[i++]=(char)c;
	  c = getc(sfile);
	  if (!isdigit(c))
	    {	  
	      if (i < LEXBUFSIZE)
		buf[i++]= '0';
	    }
	  else
	    {
	      while (isdigit(c))
		{
		  if (i < LEXBUFSIZE)
		    buf[i++]=(char)c;
		  c = getc(sfile);
		}
	    }
	}
      if (c != 'e')
	{
	  ungetc(c,sfile);
	  if (i < LEXBUFSIZE)
	    {
	      buf[i]='\0';
	      scorelval->ttype = S_NUMBER;
	    }
	  else
	    {
	      buf[LEXBUFSIZE - 1]= '\0';
	      scorelval->ttype = S_BADNUMBER;
	    }
	  scorelval->val =  dupval(buf);
	  return scorelval;
	}
      else
	{		  
	  if (i < LEXBUFSIZE)
	    buf[i++]=(char)c;
	  c = getc(sfile);
	  if ((c=='+')||(c=='-'))
	    {
	      if (i < LEXBUFSIZE)
		buf[i++]=(char)c;
	      c = getc(sfile);
	    }
	  if (isdigit(c))
	    {
	      while (isdigit(c))
		{
		  if (i < LEXBUFSIZE)
		    buf[i++]=(char)c;
		  c = getc(sfile);
		}
	      ungetc(c,sfile);

	      if (i < LEXBUFSIZE)
		{
		  buf[i]='\0';
		  scorelval->ttype = S_NUMBER;
		}
	      else
		{
		  buf[LEXBUFSIZE - 1]= '\0';
		  scorelval->ttype = S_BADNUMBER;
		}

	      scorelval->val =  dupval(buf);
	      return scorelval;
	    }
	  else
	    {
	      ungetc(c,sfile);
	      buf[i < LEXBUFSIZE ? i : LEXBUFSIZE - 1]='\0';
	      scorelval->val =  dupval(buf);
	      scorelval->ttype = S_BADNUMBER;
	      return scorelval;
	    }
	}
    }

  if ((isalpha(c))||(c == '_'))    /* keywords and identifiers */
    {

      i = 0;
      while (((isalnum(c)) || (c == '_')))
	{	  
	  if (i < LEXBUFSIZE - 1)   /* only first 16 chars matter in spec */
	    buf[i++] = (char)c;
	  c = getc(sfile);
	}
      buf[i] = '\0';
      ungetc(c,sfile);
      scorelval->val = dupval(buf);

      if (!strcmp(buf,"control"))
	{ 
	  scorelval->ttype = S_CONTROL;
	  return scorelval; 
	}

      if (!strcmp(buf,"tempo"))
	{ 
	  scorelval->ttype = S_TEMPO;
	  return scorelval; 
	}

      if (!strcmp(buf,"table"))
	{ 
	  scorelval->ttype = S_TABLE;
	  return scorelval; 
	}

      if (!strcmp(buf,"end"))
	{ 
	  scorelval->ttype = S_END;
	  return scorelval; 
	}

      scorelval->ttype = S_IDENT;
      return scorelval;
    }


  buf[0]=(char)c;
  buf[1]='\0';
  scorelval->val = dupval(buf);
  scorelval->ttype = c;

  switch (c) {

  case '\n':
    scorelval->ttype = S_NEWLINE;
    return scorelval;
  case EOF:
    scorelval->ttype = S_EOF;
    return scorelval;
  case ':':
    scorelval->ttype = S_COL;
    return scorelval;
  case '*':
    scorelval->ttype = S_STAR;
    return scorelval;
  }

  scorelval->ttype = S_BADCHAR;
  return scorelval;

}

/*********************************************************/
/*          parse function for SASL end statement        */
/*********************************************************/

int parseend(sasdata * sdata, tnode * nsl)

{

  if ( ((nsl->ttype == S_NUMBER) ||
	(nsl->ttype == S_INTGR))     &&
       (nsl->next->ttype == S_END) )
    {
      if (sdata->endtimeval == NULL)
	sdata->endtimeval =  dupval(nsl->val);
      else
	{
	  if (atof(nsl->val) <  atof(sdata->endtimeval))
	    sdata->endtimeval = dupval(nsl->val);
	}
    }
  else
    badline(nsl);
  return 1;
}


extern void mergenodes(tnode **, tnode *);

/*********************************************************/
/*          parse function for SASL tempo statement      */
/*********************************************************/

int parsetempo(sasdata * sdata, tnode * nsl, int hpe)

{

  tnode * newtempo;
  int ret = 0;

  if ( ((nsl->ttype == S_NUMBER) ||
	(nsl->ttype == S_INTGR))     &&
       (nsl->next->ttype == S_TEMPO) &&
       ((nsl->next->next->ttype == S_NUMBER) ||
	(nsl->next->next->ttype == S_INTGR)) )
    {
      sdata->numtempo++;
      newtempo = make_tnode(nsl->val, S_TEMPO);
      newtempo->down = nsl;
      newtempo->special = hpe;
      newtempo->time = (float) atof(nsl->val);
      if (sdata->temporoot == NULL)
	{
	  sdata->temporoot = sdata->tempotail = newtempo;
	} 
      else
	{
	  if (newtempo->time >= sdata->tempotail->time)
	    {
	      sdata->tempotail->next = newtempo;
	      sdata->tempotail = newtempo;
	    }
	  else
	    {
	      mergenodes(&(sdata->temporoot),newtempo);
	    }
	}
      ret = 1;
    }
  return ret;
}


extern void tablepfieldcheck(tnode *);
extern void scoreaddsymtable(tnode *);

/*********************************************************/
/*          parse function for SASL table statement      */
/*********************************************************/

int parsetable(sasdata * sdata, tnode * nsl, int tcount, int hpe)

{
  tnode * newtable;
  tnode * tptr;
  tnode * pptr;
  tnode * wgen;
  tnode * sizeptr;
  int flag = 1;

  if (((nsl->ttype == S_NUMBER) || (nsl->ttype == S_INTGR)) &&
      (nsl->next->ttype == S_TABLE) &&
      (nsl->next->next->ttype == S_IDENT) &&
      (nsl->next->next->next->ttype == S_IDENT) && 
      ((flag = (getvsym(&globalsymtable, nsl->next->next->val) != NULL))))
    {
      sdata->numtable++;
      newtable = make_tnode(nsl->val, S_TABLE);
      addvsym(&newtable->sptr,nsl->next->next->val, K_NORMAL);
      newtable->sptr->defnode = make_tnode("<table>", S_TABLE);
      newtable->sptr->defnode->vol = CONSTANT;
      tptr = newtable->sptr->defnode->down = make_tnode("TABLE", S_TABLE);
      tptr->next = make_tnode(nsl->next->next->val, S_IDENT);
      tptr = tptr->next;
      tptr->next = make_tnode("(", S_LP);
      tptr = tptr->next;
      tptr->next = make_tnode(nsl->next->next->next->val, S_IDENT);
      wgen = tptr = tptr->next;

      if (!wavegeneratorname(wgen))
	{
	  printf("Error: Invalid generator name %s.\n\n",tptr->val);
	  showbadline(nsl);
	}
      tablepfieldcheck(nsl);

      tptr->next = make_tnode(",", S_COM);
      tptr = tptr->next;
      tptr->next = make_tnode("<exprstrlist>", S_EXPRSTRLIST);
      tptr->next->next = make_tnode(")", S_RP);
      pptr = nsl->next->next->next->next;
      sizeptr = tptr->next->down = make_tnode("<expr>", S_EXPR);
      tptr = tptr->next->down;

      while (pptr != NULL)
	{
	  tptr->down = make_tnode(dupval(pptr->val), pptr->ttype);
	  tptr->rate = tptr->down->rate = IRATETYPE;
	  tptr->vol = tptr->down->vol = CONSTANT;
	  tptr->width = tptr->down->width = 1;
	  if (pptr->ttype == S_IDENT)
	    {
	      tptr->vartype = tptr->down->vartype = TABLETYPE;
	      tptr->down->sptr = pptr->sptr;
	    }
	  else
	    tptr->vartype = tptr->down->vartype = SCALARTYPE;
	  tptr->res = tptr->down->res;
	  if (pptr->next)
	    {
	      tptr->next = make_tnode(",", S_COM);
	      tptr = tptr->next;
	      if (pptr->next->ttype == S_STRCONST)
		{
		  tptr->next = make_tnode(dupval(pptr->next->val), S_STRCONST);
		  pptr = pptr->next;
		  tptr = tptr->next;
		  if (pptr->next)
		    {
		      tptr->next = make_tnode(",", S_COM);
		      tptr = tptr->next;
		      tptr->next = make_tnode("<expr>", S_EXPR);
		    }
		}
	      else
		tptr->next = make_tnode("<expr>", S_EXPR);
	      tptr = tptr->next;
	    }
	  pptr = pptr->next;
	}

      newtable->sptr->consval = (char *) 
	wavereduceconstants(newtable->sptr->defnode, nsl);

      if (newtable->sptr->defnode->usesinput == 0)
	haswavegenerator(wgen);

      newtable->down = nsl;
      newtable->special = hpe;
      newtable->width = tcount - 4;
      newtable->time = (float) atof(nsl->val);
      if (sdata->tableroot == NULL)
	{
	  sdata->tableroot = sdata->tabletail = newtable;
	} 
      else
	{
	  if (newtable->time >= sdata->tabletail->time)
	    {
	      sdata->tabletail->next = newtable;
	      sdata->tabletail = newtable;
	    }
	  else
	    {
	      mergenodes(&(sdata->tableroot),newtable);
	    }
	}
      if (boutfile)
	{
	  scoreaddsymtable(nsl->next->next);
	  scoreaddsymtable(nsl->next->next->next);
	}
    }
  else
    {
      if (flag)
	badline(nsl);
    }
  return 1;
}

/*********************************************************/
/*          parse function for SASL control statement    */
/*********************************************************/

int parsecontrol(sasdata * sdata, tnode * nsl, int hpe)

{
  tnode * newcontrol;

  if ( ((nsl->ttype == S_NUMBER) ||
	(nsl->ttype == S_INTGR))     &&
       (nsl->next->next->ttype == S_IDENT) &&
       ((nsl->next->next->next->ttype == S_NUMBER) ||
	(nsl->next->next->next->ttype == S_INTGR)) )
    {
      newcontrol = make_tnode(nsl->val, S_CONTROL);
      newcontrol->down = nsl;
      newcontrol->special = hpe;
      newcontrol->time = (float) atof(nsl->val);
      if (sdata->controlroot == NULL)
	{
	  sdata->controlroot = sdata->controltail = newcontrol;
	} 
      else
	{
	  if (newcontrol->time >= sdata->controltail->time)
	    {
	      sdata->controltail->next = newcontrol;
	      sdata->controltail = newcontrol;
	    }
	  else
	    {
	      mergenodes(&(sdata->controlroot),newcontrol);
	    }
	}
      if (boutfile)
	scoreaddsymtable(nsl->next->next);
    }
  else
    badline(nsl);
  return 1;

}

/*********************************************************/
/*          parse function for SASL control statement    */
/*********************************************************/

int parselcontrol(sasdata * sdata, tnode * nsl, int hpe)

{
  tnode * newcontrol;
  
  if ( ((nsl->ttype == S_NUMBER) ||
	(nsl->ttype == S_INTGR))     &&
       (nsl->next->ttype == S_IDENT) &&
       (nsl->next->next->next->ttype == S_IDENT) &&
       ((nsl->next->next->next->next->ttype == S_NUMBER) ||
	(nsl->next->next->next->next->ttype == S_INTGR)) )
    {
      newcontrol = make_tnode(nsl->val, S_LCONTROL);
      newcontrol->down = nsl;
      newcontrol->special = hpe;
      newcontrol->time = (float) atof(nsl->val);
      if (sdata->controlroot == NULL)
	{
	  sdata->controlroot = sdata->controltail = newcontrol;
	} 
      else
	{
	  if (newcontrol->time >= sdata->controltail->time)
	    {
	      sdata->controltail->next = newcontrol;
	      sdata->controltail = newcontrol;
	    }
	  else
	    {
	      mergenodes(&(sdata->controlroot),newcontrol);
	    }
	}
      if (boutfile)
	{
	  scoreaddsymtable(nsl->next);
	  scoreaddsymtable(nsl->next->next->next);
	}
    }
  else
    badline(nsl);
  return 1;
}


extern void instrpfieldcheck(tnode *, tnode *);

/*********************************************************/
/*          parse function for SASL instr statement      */
/*********************************************************/

int parselinstr(sasdata * sdata, tnode * nsl, int tcount, int hpe)

{

  tnode * newinstr;
  sigsym * label;
  tnode * lptr;

  if ( (nsl->ttype == S_IDENT) &&
       (nsl->next->ttype == S_COL) &&
       ((nsl->next->next->ttype == S_NUMBER) ||
	(nsl->next->next->ttype == S_INTGR))  &&
       (nsl->next->next->next->ttype == S_IDENT) &&
       ((nsl->next->next->next->next->ttype == S_NUMBER) ||
	(nsl->next->next->next->next->ttype == S_INTGR)))
    {

      newinstr = make_tnode(nsl->next->next->val, S_LINSTR);
      newinstr->width = tcount - 5; /* num pfields */
      newinstr->special = hpe;
      newinstr->down = nsl;
      newinstr->time = (float) atof(newinstr->val);
      addvsym(&(sdata->labeltable), nsl->val, S_LINSTR);
      label = getsym(&(sdata->labeltable), nsl);
      lptr = make_tnode(nsl->next->next->next->val, S_INSTR);
      lptr->down = newinstr;
      if (label->defnode == NULL)
	label->defnode = lptr;
      else
	{
	  lptr->next = label->defnode;
	  label->defnode = lptr;
	}
      newinstr->sptr = getsym(&instrnametable,nsl->next->next->next);
      if (newinstr->sptr == NULL)
	{
	  printf("Error: Instr %s, used in -sco, not in -orc.\n",
		 nsl->next->next->next->val);
	  showbadline(nsl);
	}
      instrpfieldcheck(nsl, nsl->next->next->next->next->next);
      if (nsl->next->next->opwidth)
	newinstr->sptr->ascore++;
      else
	newinstr->sptr->score++;
      if (sdata->instrroot == NULL)
	{
	  sdata->instrroot = sdata->instrtail = newinstr;
	} 
      else
	{
	  if (newinstr->time >= sdata->instrtail->time)
	    {
	      sdata->instrtail->next = newinstr;
	      sdata->instrtail = newinstr;
	    }
	  else
	    {
	      mergenodes(&(sdata->instrroot), newinstr);
	    }
	}
      if (boutfile)
	{
	  scoreaddsymtable(nsl);
	  scoreaddsymtable(nsl->next->next->next);
	}
      return 1;
    }
  return 0;
}

/*********************************************************/
/*          parse function for SASL instr statement      */
/*********************************************************/

int parseinstr(sasdata * sdata, tnode * nsl, int tcount, int hpe)

{

  tnode * newinstr;

  if ( ((nsl->ttype == S_NUMBER) ||
	(nsl->ttype == S_INTGR))  &&
       (nsl->next->ttype == S_IDENT) &&
       ((nsl->next->next->ttype == S_NUMBER) ||
	(nsl->next->next->ttype == S_INTGR)))
    {
      newinstr = make_tnode(nsl->val, S_INSTR);
      newinstr->width = tcount - 3; /* num pfields */
      newinstr->special = hpe;
      newinstr->down = nsl;
      newinstr->time = (float) atof(nsl->val);
      newinstr->sptr = getsym(&instrnametable,nsl->next);
      if (newinstr->sptr == NULL)
	{
	  printf("Error: Instr %s, used in -sco, not in -orc.\n",
		 nsl->next->val);
	  showbadline(nsl);
	}
      instrpfieldcheck(nsl, nsl->next->next->next);
      if (nsl->opwidth)
	newinstr->sptr->ascore++;
      else
	newinstr->sptr->score++;
      if (sdata->instrroot == NULL)
	{
	  sdata->instrroot = sdata->instrtail = newinstr;
	} 
      else
	{
	  if (newinstr->time >= sdata->instrtail->time)
	    {
	      sdata->instrtail->next = newinstr;
	      sdata->instrtail = newinstr;
	    }
	  else
	    {
	      mergenodes(&(sdata->instrroot), newinstr);
	    }
	}
      if (boutfile)
	scoreaddsymtable(nsl->next);
      return 1;
    }
  return 0;
}

/*********************************************************/
/*          shows bad SASL line, closes sfront           */
/*********************************************************/

void showbadline(tnode * line)


{

  printf("Offending line from -sco file:\n\n");
  while (line != NULL)
    {
      printf(" %s ",line->val);
      line = line->next;
    }
  printf("\n");
  if (bitfile)
    {
      printf("Error originates in score_file SA block of -bit file.\n");
      printf("Use -scoout and -orcout to generate ASCII files and\n");
      printf("run sfront on these files to pinpoint error.\n");
    }
  else
    {
      if (cppsaol)
	{
	  printf("If this line not in your main -sco file, look at\n");
	  printf("files you may have included via pre-processing.\n");
	}
    }
  noerrorplace();

}

/*********************************************************/
/*          generic error function for SASL              */
/*********************************************************/

void badline(tnode * line)


{
   printf("Error: SASL Syntax error: \n\n");
   showbadline(line);
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*                                                              */
/*   Second-level functions for intermediate score processing   */
/*                                                              */
/*______________________________________________________________*/

extern void addtempomap(tnode **, tnode *);

/*********************************************************/
/*    makes a global tempo map for renumbering           */
/*********************************************************/

tnode * maketempomap(void) 

{
  tnode * ret, * tptr;
  tnode * ctempo, * stempo, * atempo;
  int state;

  /* tempos from SASL and MIDI in the configuration block */

  ctempo = confsasl->temporoot;   

  /* tempos from has_time = 1 in SASL */

  stempo = sstrsasl->temporoot;

  /* tempos generated from mstrfile read, in relative form */

  atempo = abssasl->temporoot;

  /* default tempo */

  if (mstrfile)
    tptr = ret = make_tnode("120.0", S_TEMPO);
  else
    tptr = ret = make_tnode("60.0", S_TEMPO);

  ret->time = 0.0;
  
  while (ctempo || stempo || atempo)
    {
      state = (ctempo != NULL) + 2*(stempo != NULL) + 4*(atempo != NULL);
      switch(state) {
      case 1:  /* ctempo only */
	addtempomap(&ctempo, tptr);
	break;
      case 2:  /* stempo only */
	addtempomap(&stempo, tptr);
	break;
      case 3:  /* stempo and ctempo */
	if (stempo->time < ctempo->time)
	  addtempomap(&stempo, tptr);
	else
	  addtempomap(&ctempo, tptr);
	break;
      case 4:  /* atempo only */
	addtempomap(&atempo, tptr);
	break;
      case 5:  /* atempo and ctempo */
	if (atempo->time < ctempo->time)
	  addtempomap(&atempo, tptr);
	else
	  addtempomap(&ctempo, tptr);
	break;
      case 6:  /* atempo and stempo */
	if (stempo->time < atempo->time)
	  addtempomap(&stempo, tptr);
	else
	  addtempomap(&atempo, tptr);
	break;
      case 7:  /* atempo and stempo and ctempo */
	if ((stempo->time <= atempo->time) &&
	    (stempo->time <= ctempo->time))
	  addtempomap(&stempo, tptr);
	else
	  {
	    if (atempo->time < ctempo->time)
	      addtempomap(&atempo, tptr);
	    else
	      addtempomap(&ctempo, tptr);
	  }
	break;
      }
      tptr = tptr->next;
    }

  /* delete dummy 60.0/120.0 */

  ret = ret->next;
  return ret;
      
}

/*********************************************************/
/*    set up mstart and mend pointers for renumbering    */
/*                                                       */
/* fields in each midi note:                             */
/*                                                       */
/* rate: starttime, in miditicks                         */
/* width: endtime, in miditicks                          */
/* opwidth: starttime, in kcycleidx (>= 1)               */
/* inwidth: endtime, in kcycleidx (>= 1)                 */
/* res: notenumber                                       */
/* vartype: velocity                                     */
/* usesinput: flag for noteoffwrite                      */
/*********************************************************/

int midipointers(tnode ** mstart, tnode ** mend)

{
  int hasmidi;
  tnode * tptr, *startptr, *endptr;

  /* set up lists of current MIDI channel list pointers */

  hasmidi = 0;
  tptr = sstrmidi->imidiroot;
  (*mstart) = (*mend) = startptr = endptr = NULL;
  while (tptr)
    {
      if (!(*mstart))
	{
	  (*mstart) = startptr = make_tnode("tag", S_MIDITAG);
	  (*mend) = endptr = make_tnode("tag", S_MIDITAG);
	}
      else
	{
	  startptr->next = make_tnode("tag", S_MIDITAG);
	  startptr = startptr->next;
	  endptr->next = make_tnode("tag", S_MIDITAG);
	  endptr = endptr->next;
	}
      startptr->down = endptr->down = tptr->down; /* list of notes */
      if (tptr->down)
	hasmidi = 1;
      tptr = tptr->next;
    }
  return hasmidi;

}

/*********************************************************/
/*    converts timestamps from relative to absolute      */
/*********************************************************/

void timestampconvert(tnode * tptr, int kcycleidx)

{
  float ktime;

  ktime = 1.0F/krate;
  vmcheck(tptr->val = (char *) calloc(64, sizeof(char)));
  sprintf(tptr->val, "%f", (kcycleidx-1.5F)*ktime);
  tptr->opwidth = kcycleidx;

}

/*********************************************************/
/*              merges a sasl rootlist                   */
/*********************************************************/

void mergerootlist(tnode ** oneroot, tnode ** onetail,
		   tnode ** tworoot, tnode ** twotail,
		   tnode ** outroot, tnode ** outtail)

{
  tnode * tptr = NULL;

  /* first handle simple cases */

  if (*tworoot == NULL)
    {
      *outroot = *oneroot;
      *outtail = *onetail;
      *onetail = NULL;
      return;
    }
  if (*oneroot == NULL)
    {
      *outroot = *tworoot;
      *outtail = *twotail;
      *twotail = NULL;
      return;
    }

  /* set up the merge */

  if ((*oneroot)->time < (*tworoot)->time)
    {
      *outroot = tptr = *oneroot;
      *oneroot = (*oneroot)->next;
    }
  else
    {
      *outroot = tptr = *tworoot;
      *tworoot = (*tworoot)->next;
    }

  /* do the merge */

  while ((*oneroot != NULL) || (*tworoot != NULL))
    {
      if (*oneroot == NULL)
	{
	  tptr->next = *tworoot;
	  *outtail = *twotail;
	  *tworoot = NULL;
	  break;
	}
      if (*tworoot == NULL)
	{
	  tptr->next = *oneroot;
	  *outtail = *onetail;
	  *oneroot = NULL;
	  break;
	}
      if ((*oneroot)->time < (*tworoot)->time)
	{
	  tptr->next = *oneroot;
	  *oneroot = (*oneroot)->next;
	}
      else
	{
	  tptr->next = *tworoot;
	  *tworoot = (*tworoot)->next;
	}
      tptr = tptr->next;
    }

  *onetail = *twotail = NULL;
  return;
}

/*********************************************************/
/*       merges a labeltable list into allsasl           */
/*********************************************************/

void mergelabeltable(sigsym * sptr) 

{
  sigsym * label;
  tnode * lptr;

  while (sptr)
    {
      if ((label = getvsym(&(allsasl->labeltable),sptr->val)))
	{
	  lptr = sptr->defnode;
	  while (lptr->next != NULL)
	    lptr = lptr->next;
	  lptr->next = label->defnode;
	  label->defnode = sptr->defnode;
	}
      else
	{
	  addvsym(&(allsasl->labeltable), sptr->val, S_LINSTR);
	  label = getvsym(&(allsasl->labeltable),sptr->val);
	  label->defnode = sptr->defnode;
	}
      sptr = sptr->next;
    }
}


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*                                                              */
/*       Utility functions for reading score data.              */
/*                                                              */
/*______________________________________________________________*/

/*********************************************************/
/*          adds score IDENT to symbol table             */
/*********************************************************/

void scoreaddsymtable(tnode * tptr)

{
  if (!identtoken(tptr))
    if ((addvsym(&bitsymtable, tptr->val, K_INTERNAL) == INSTALLED)
	&& (bitsymtable->next != NULL))
      bitsymtable->special = bitsymtable->next->special + 1;
}

/*********************************************************/
/*          merge sort for SASL note-ons                 */
/*********************************************************/

void mergenodes(tnode ** rootnode, tnode * newnode)

{

  tnode * tptr;

  if ((*rootnode)->time >= newnode->time)
    {
      newnode->next = *rootnode;
      *rootnode = newnode;
      return;
    }
  else
    {
      tptr = *rootnode;
      while (tptr->next != NULL)
	{
	  if (tptr->next->time >= newnode->time)
	    {
	      newnode->next = tptr->next;
	      tptr->next = newnode;
	      return;
	    }
	  tptr = tptr->next;
	}
      internalerror("readscore.c", "mergenodes()");
    }

}


/*********************************************************/
/*          checks instr pfields for bad elements        */
/*********************************************************/

void instrpfieldcheck(tnode * nsl, tnode * ptest)

{

  while (ptest != NULL)
    {
      if ((ptest->ttype != S_NUMBER) && (ptest->ttype != S_INTGR))
	{
	  printf("Error: Element %s not allowed in SASL instr pfields\n",
		 ptest->val);
	  showbadline(nsl);
	}
      ptest = ptest->next;
    }

}

/*********************************************************/
/*          checks table pfields for bad elements        */
/*********************************************************/

void tablepfieldcheck(tnode * nsl)

{

  tnode * ptest;
  int sample, concat;

  sample = !(strcmp(nsl->next->next->next->val,"sample"));
  concat = !(strcmp(nsl->next->next->next->val,"concat"));

  ptest = nsl->next->next->next->next;

  /* check type of size parameter */

  if ((ptest->ttype != S_NUMBER) && (ptest->ttype != S_INTGR))
    {
      printf("Error: Bad size parameter %s in SASL table pfields\n",
	     ptest->val);
      showbadline(nsl);
    }
  ptest = ptest->next;

  /* special cases for concat and sample parameters */

  if (sample)
    {
      if (!ptest || (ptest->ttype != S_STRCONST))
	{
	  if (ptest)
	    printf("Error: Bad filename parameter %s in SASL table pfields\n",
		   ptest->val);
	  else
	    printf("Error: No filename parameter in SASL table pfields\n");
	  showbadline(nsl);
	}
      ptest = ptest->next;
    }

  if (concat)
    {
      while (ptest != NULL)
	{
	  if ((ptest->ttype != S_IDENT) || 
	      (!(ptest->sptr = getvsym(&globalsymtable,ptest->val))) ||
	      (ptest->sptr->vartype != TABLETYPE))
	    {
	      printf("Error: Bad table parameter %s in SASL table pfields\n",
		     ptest->val);
	      showbadline(nsl);
	    }
	  ptest = ptest->next;
	}
    }

  /* checks all numeric parameters */

  while (ptest != NULL)
    {
      if ((ptest->ttype != S_NUMBER) && (ptest->ttype != S_INTGR))
	{
	  printf("Error: Element %s not allowed in SASL table pfields\n",
		 ptest->val);
	  showbadline(nsl);
	}
      ptest = ptest->next;
    }

}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*                                                              */
/*       Utility functions for intermediate score processing.   */
/*                                                              */
/*______________________________________________________________*/

/*********************************************************/
/*      creates a new element for the tempo map          */
/*********************************************************/

void addtempomap(tnode ** mptr, tnode * tptr)

{
  tptr->next = make_tnode((*mptr)->down->next->next->val, S_TEMPO);
  tptr->next->time = (*mptr)->time;
  *mptr = (*mptr)->next;
}

