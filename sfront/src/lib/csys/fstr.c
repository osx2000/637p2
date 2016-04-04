
/*
#    Sfront, a SAOL to C translator    
#    This file: Plays streaming data from .mp4 file.
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


/****************************************************************/
/****************************************************************/
/*           streaming file control driver for sfront           */ 
/****************************************************************/

#define SAMP_SR        5
#define SAMP_LOOPSTART 4
#define SAMP_LOOPEND   3
#define SAMP_BASEFREQ  2
#define SAMP_LLMEM     1
#define SAMP_DATABLOCK 5

FILE * csysi_bitfile;   /* streaming file */

/* bit-level state variables */

unsigned char csysi_bitstoread = 0;
int csysi_bitreadpos = - 1;

/* bit-level parsing constants */

#define CSYSI_IDENT        0xF0
#define CSYSI_NUMBER       0xF1
#define CSYSI_INTGR        0xF2
#define CSYSI_STRCONST     0xF3
#define CSYSI_BYTE         0xF4

#define CSYSI_BINORC   0
#define CSYSI_BINSCORE 1
#define CSYSI_BINMIDI  2
#define CSYSI_BINSAMP  3
#define CSYSI_BINSBF   4
#define CSYSI_BINSYM   5

#define CSYSI_EVSCORE  0
#define CSYSI_EVMIDI   1
#define CSYSI_EVSAMPLE 2

#define CSYSI_MIDIMASKCOM  0xF0
#define CSYSI_MIDISYSTEM   0xF0
#define CSYSI_MIDIMASKCHAN 0x0F
#define CSYSI_METATEMPO    0x51

#define CSYSI_MAXENDTIME   21600.0F

/* word-level state variables */

float csysi_bitaccesstime = 0.0F;  /* current SA_access_unit time */

int csysi_moreaccessunits = 1;     /* more access_units left to read */
int csysi_endofevent = 1;          /* no more events in access_unit  */

int csysi_numabstime = 0;          /* number of abs-time sasl events */
int csysi_numscotime = 0;          /* number of rel-time sasl events */
int csysi_nummidi = 0;             /* number of midi events */

int csysi_absready = 0;            /* abs-time sasl events triggered */
int csysi_scoready = 0;            /* rel-time sasl events triggered */
int csysi_midiready = 0;           /* midi events triggered */

int csysi_targetvar = -1;         /* pointer into csys_target[] */
int csysi_targetcount = 0;        /* pointer info {instr,var}index  */

unsigned char csysi_runstat = 0;  /* holds running status byte      */
int csysi_endflag = 0;            /* flag for issuing last end command */
float csysi_compendtime = 0;      /* supplied computed endtime         */

 
/* data structures for pending events */

#define CSYSI_MAXSASL 64

/* add at the head, consume from the tail */

int csysi_headsco = 0;  /* next available place for data */
int csysi_tailsco = -1; /* first occupied place for data */
 
int csysi_headabs = 0;  /* next available place for data */
int csysi_tailabs = -1; /* first occupied place for data */
 
typedef struct csysi_sevent {
  float atime;
  float stime;
  unsigned char cmd;
  unsigned char priority;
  unsigned short id;
  unsigned short label;
  float fval;
  unsigned int pnum;
  float * p;
} csysi_sevent;

csysi_sevent csysi_scoqueue[CSYSI_MAXSASL];
csysi_sevent csysi_absqueue[CSYSI_MAXSASL];

#define CSYSI_MAXMIDI 64

/* add at the head, consume from the tail */

int csysi_headmidi = 0;  /* next available place for data */
int csysi_tailmidi = -1; /* first occupied place for data */
 
typedef struct csysi_mevent {
  float atime;
  unsigned char cmd;
  unsigned char ndata;
  unsigned char vdata;
  unsigned short extchan;
  float fval;
} csysi_mevent;

csysi_mevent csysi_midiqueue[CSYSI_MAXMIDI];

/* data structure for samples */

typedef struct csysi_sampleunit {
  float atime;
  int token;
  int len;
  float * p;
  struct csysi_sampleunit * next;
} csysi_sampleunit;

csysi_sampleunit * csysi_samples = NULL;


/******************************************************************/
/*                   gets next byte from bitfile                  */
/******************************************************************/

void csysi_readnextbyte(void)

{
  int retry = 0;

  while (fread(&csysi_bitstoread,sizeof(char),1,csysi_bitfile)!=1)
    {
      if (feof(csysi_bitfile))
	csys_terminate("premature end of .mp4 file");
      if (++retry > IOERROR_RETRY)
	csys_terminate("i/o problems during .mp4 file read");
      clearerr(csysi_bitfile);
    }
  csysi_bitreadpos = 7;

}

/******************************************************************/
/*           reads numbits LSBs of bitval from bitfile            */
/******************************************************************/


unsigned int csysi_readbit(unsigned int numbits)

{
  unsigned int ret = 0;

  while (numbits > 0)
    {
      if (csysi_bitreadpos < 0)
	csysi_readnextbyte();
      ret |= 
	((csysi_bitstoread & (1 << csysi_bitreadpos)) != 0) << (numbits-1);
      numbits--;
      csysi_bitreadpos--;
    }
  return ret;

}

/******************************************************************/
/*        checks for next accessunit, sets bitaccesstime          */
/******************************************************************/

int csysi_readaccesstime()

{
  int numbits = 32;
  union { unsigned int l; float f ; } u;
  int retry = 0;

  u.l = 0;
  while (numbits > 0)
    {
      while (csysi_bitreadpos >= 0)
	{
	  u.l |= 
	    ((csysi_bitstoread & (1 << csysi_bitreadpos)) != 0) << (numbits-1);
	  csysi_bitreadpos--;
	  if (!(--numbits))
	    {
	      csysi_bitaccesstime = u.f;
	      return 1;
	    }
	}
      while (fread(&csysi_bitstoread,sizeof(char),1,csysi_bitfile)!=1)
	{
	  if (feof(csysi_bitfile))
	    return 0;
	  if (++retry > IOERROR_RETRY)
	    csys_terminate("i/o problems during .mp4 file read");
	  clearerr(csysi_bitfile);
	}
      csysi_bitreadpos = 7;
    }
  return 0;  /* should never happen */
}


/******************************************************************/
/*           flushes numbits of bitval from bitfile                */
/******************************************************************/


void csysi_readflush(unsigned int numbits)

{

  while (numbits > 0)
    {
      if (csysi_bitreadpos < 0)
	csysi_readnextbyte();
      numbits--;
      csysi_bitreadpos--;
    }

}

/******************************************************************/
/*           move past midi_file block                            */
/******************************************************************/

void csysi_midifileflush(void)

{

  csysi_readflush(8*csysi_readbit(32));

}

/******************************************************************/
/*           move past symboltable block                          */
/******************************************************************/

void csysi_symboltableflush(void)

{
  unsigned int num;

  num = csysi_readbit(16);
  while (num > 0)
    {
      csysi_readflush(8*csysi_readbit(4));      
      num--;
    }
}

/******************************************************************/
/*           move past orcfile block                          */
/******************************************************************/

void csysi_orcfileflush(void)

{
  unsigned int num, token;

  num = csysi_readbit(16);
  while (num > 0)
    {
      token = csysi_readbit(8);
      switch(token) {
      case CSYSI_IDENT:
	csysi_readflush(16);
	break;
      case CSYSI_NUMBER:
	csysi_readflush(32);
	break;
      case CSYSI_INTGR:
	csysi_readflush(32);
	break;
      case CSYSI_STRCONST:
	csysi_readflush(8*csysi_readbit(8));
	break;
      case CSYSI_BYTE:
	csysi_readflush(8);
	break;
      default:
	break;
      }
      num--;
    }
}

/******************************************************************/
/*           move past one score_line                             */
/******************************************************************/

void csysi_scorelineflush(void)

{
  int hastime, haslabel, scoretype;
  int numpfields, destroy, refers, sym;

  hastime = csysi_readbit(1);
  if (hastime)
    csysi_readflush(33);
  csysi_readflush(1);
  scoretype = csysi_readbit(3);
  switch (scoretype) {
  case CSYS_SASL_INSTR:
    haslabel = csysi_readbit(1);
    if (haslabel)
      csysi_readflush(16);
    csysi_readflush(48);
    numpfields = csysi_readbit(8);
    csysi_readflush(32*numpfields);
    break;
  case CSYS_SASL_CONTROL: 
    haslabel = csysi_readbit(1);
    if (haslabel)
      csysi_readflush(16);
    csysi_readflush(48);
    break;
  case CSYS_SASL_TABLE:
    csysi_readflush(16);
    destroy = csysi_readbit(1);
    if (!destroy)
      {
	sym = csysi_readbit(8);
	refers = csysi_readbit(1);
	if (refers)
	  csysi_readflush(16);	
	numpfields = csysi_readbit(16);
	if (sym == CSYS_SASL_TGEN_CONCAT)
	  {
	    csysi_readflush(32);
	    if ((numpfields - 1) > 0)
	      csysi_readflush(16*(numpfields-1));
	  }
	else
	  csysi_readflush(32*numpfields);
      }
    break;
  case CSYS_SASL_ENDTIME:     
    break;
  case CSYS_SASL_TEMPO:   
    csysi_readflush(32);
    break;
  default:
    csys_terminate("Unknown score line type in .mp4 file");
  }

}

/******************************************************************/
/*           move past score_file block                           */
/******************************************************************/

void csysi_scorefileflush(void)

{
  unsigned int numlines;

  numlines = csysi_readbit(20);
  while (numlines > 0)
    {
      csysi_scorelineflush();
      numlines--;
    }
}


/******************************************************************/
/*           move past sample block                           */
/******************************************************************/

void csysi_sampleflush(void)

{

  unsigned int len;

  csysi_readflush(16);       /* symbol */
  len = csysi_readbit(24);   
  if (csysi_readbit(1))      /* srate */
    csysi_readflush(17);
  if (csysi_readbit(1))      /* loop */
    csysi_readflush(48);
  if (csysi_readbit(1))      /* base */
    csysi_readflush(32);
  csysi_readflush((16+csysi_readbit(1)*16)*len);

}

/******************************************************************/
/*          flushes StructuredAudioSpecificConfig                 */
/******************************************************************/


void csysi_flushconfig(void) 

{
  int type;

  while (csysi_readbit(1))
    {
      type = csysi_readbit(3);
      switch (type) {
      case CSYSI_BINORC:
	csysi_orcfileflush();
	break;
      case CSYSI_BINSCORE:
	csysi_scorefileflush();
	break;
      case CSYSI_BINMIDI: 
	csysi_midifileflush();
	break;
      case CSYSI_BINSAMP:
	csysi_sampleflush();
	break;
      case CSYSI_BINSBF:
	csys_terminate(".mp4 uses SASBF");
	break;
      case CSYSI_BINSYM:   
	csysi_symboltableflush();
	break;
      }
    }
}

/******************************************************************/
/*             puts score data in saslqueue[idx]                   */
/******************************************************************/

void csysi_scorelinefill(void)

{
  union { unsigned int l; float f ; } u;
  int sym, i, j, sampletoken, size, skip;
  csysi_sampleunit * sidx;
  csysi_sevent * cq;

  if (csysi_readbit(1))      /* has_time */
    {
      cq = &(csysi_scoqueue[csysi_headsco]);
      cq->atime = csysi_bitaccesstime;
      if (csysi_tailsco == -1)
	csysi_tailsco = csysi_headsco;
      csysi_headsco = (csysi_headsco+1)&(CSYSI_MAXSASL-1);

      csysi_readflush(1);   /* use_if_late */
      u.l = csysi_readbit(32);
      cq->stime = u.f;
      if (u.f < 0)
	cq->stime = -1.0F;
      csysi_numscotime++;

    }
  else
    {
      cq = &(csysi_absqueue[csysi_headabs]);
      cq->atime = csysi_bitaccesstime;
      if (csysi_tailabs == -1)
	csysi_tailabs = csysi_headabs;
      csysi_headabs = (csysi_headabs+1)&(CSYSI_MAXSASL-1);

      csysi_numabstime++;
    }
  cq->priority = csysi_readbit(1);      
  switch (cq->cmd = csysi_readbit(3)) {
  case CSYS_SASL_INSTR:
    if (csysi_readbit(1)) /* has label */
      cq->label = csysi_readbit(16);
    else
      cq->label = CSYS_NOLABEL;
    cq->cmd = CSYS_SASL_NOOP;
    sym = csysi_readbit(16);  /* token for instr */
    for (i=0; i < CSYS_INSTRNUM; i++)
      if (csys_instr[i].token == sym)
	{
	  cq->cmd = CSYS_SASL_INSTR;
	  cq->id = csys_instr[i].index;
	}
    u.l = csysi_readbit(32);         
    cq->fval = u.f;  /* duration */
 
    /* pfields */
    cq->p = malloc((cq->pnum = csysi_readbit(8))*sizeof(float));
    for (i = 0; i < cq->pnum; i++)
      {
	u.l = csysi_readbit(32);  
	cq->p[i] =  u.f;
      }
    break;
  case CSYS_SASL_CONTROL:
    if (csysi_readbit(1)) /* has label */
      {
	/* expands into to a set of calls later on */

	cq->label = csysi_readbit(16);
	cq->pnum = csysi_readbit(16);
      }
    else
      {
	sym = csysi_readbit(16);
	cq->cmd = CSYS_SASL_NOOP;
	cq->label = CSYS_NOLABEL;
	cq->id = CSYS_SASL_NOINSTR;
	for (i=0; i < CSYS_GLOBALNUM; i++)
	  if (csys_global[i].token == sym)
	    {
	      cq->cmd = CSYS_SASL_CONTROL;
	      cq->pnum = csys_global[i].index;
	      break;
	    }
      }
    u.l = csysi_readbit(32);
    cq->fval = u.f;
    break;
  case CSYS_SASL_TABLE:
    sidx = NULL;
    sym = csysi_readbit(16);
    if (csysi_readbit(1)) /* destroy */
      {
	cq->label = CSYS_SASL_TGEN_DESTROY;
	cq->pnum = 0;
	cq->p = NULL;
      }
    else
      {
	cq->label = csysi_readbit(8); /* generator */
	if (csysi_readbit(1)) /* refers to sample */
	  {
	    sampletoken = csysi_readbit(16);
	    sidx = csysi_samples;
	    while (sidx)
	      {
		if (sidx->token == sampletoken)
		  {
		    while (sidx->next && 
			   (sidx->next->token == sampletoken) &&
			   (sidx->next->atime <= csysi_bitaccesstime))
		      sidx = sidx->next;
		    break;
		  }
		sidx = sidx->next;
	      }
	  }
	cq->p = malloc((cq->pnum = csysi_readbit(16))*sizeof(float));
	for (i = 0; i < cq->pnum; i++)
	  {
	    if (i && (cq->label == CSYS_SASL_TGEN_CONCAT))
	      {
		u.l = csysi_readbit(16);  
		cq->p[i] = -1.0F;
		for (j=0; j < CSYS_GLOBALNUM; j++)
		  if ((csys_global[j].token == u.l) &&
		      (csys_global[j].type == CSYS_TABLE))
		    {
		      cq->p[i] = csys_global[j].index;
		      break;
		    }
		if (cq->p[i] == -1.0F)
		  csys_terminate("concat uses an unknown table");
	      }
	    else
	      {
		u.l = csysi_readbit(32);  
		cq->p[i] = u.f;
	      }
	  }	
	cq->cmd = CSYS_SASL_NOOP;
	for (i=0; i < CSYS_GLOBALNUM; i++)
	  if ((csys_global[i].token == sym) &&
	      (csys_global[i].type == CSYS_TABLE))
	    {
	      cq->cmd = CSYS_SASL_TABLE;
	      cq->id = csys_global[i].index;
	      break;
	    }
	if (cq->label == CSYS_SASL_TGEN_SAMPLE)
	  {
	    if (!sidx)
	      csys_terminate("sample block not found");

	    size = -1;
	    skip = 0;
	    if (cq->pnum)
	      size = ROUND(cq->p[0]);
	    if (cq->pnum > 2)
	      skip = ROUND(cq->p[2]);
	    /* free (cq->p); */
	    if ( ((size < 0) || (size + SAMP_DATABLOCK <= sidx->len))
		 && (skip <= 0))
	      {
		cq->p = sidx->p;
		if (size < 0)
		  cq->pnum = sidx->len;
		else
		  cq->pnum = size + SAMP_DATABLOCK;
	      }
	    else
	      {
		if (size < 0)
		  cq->pnum = size = sidx->len - skip;
		else
		  cq->pnum = size = size + SAMP_DATABLOCK;
		cq->p = calloc(size, sizeof(float));
		cq->p[size - SAMP_LLMEM] = 1;
		cq->p[size - SAMP_SR] = sidx->p[sidx->len - SAMP_SR];
		cq->p[size - SAMP_LOOPSTART] = 
		  ROUND(sidx->p[sidx->len - SAMP_LOOPSTART]) - skip;
		cq->p[size - SAMP_LOOPEND] =  
		  ROUND(sidx->p[sidx->len - SAMP_LOOPEND]) - skip;
		cq->p[size - SAMP_BASEFREQ] =  
		  sidx->p[sidx->len - SAMP_BASEFREQ];
		j = skip;
		i = 0;
		while ((i < (size - SAMP_DATABLOCK)) && (j < (sidx->len - SAMP_DATABLOCK)))
		  cq->p[i++] = sidx->p[j++];
	      }
	  }
      }
    break;
  case CSYS_SASL_ENDTIME:
    csysi_endflag = 1;    /* obey SASL endtime command */
    break;
  case CSYS_SASL_TEMPO:      
    u.l = csysi_readbit(32);
    cq->fval = u.f;
    break;
  default:
    csys_terminate("Unknown score line type in .mp4 file");
  }

}

/******************************************************************/
/*             reads the sample class                             */
/******************************************************************/

void csysi_sampleread(void)

{
  int sym, i, len;
  csysi_sampleunit * sidx, * newsamp;
  union { unsigned int l; float f ; } u;
  union { unsigned short us; signed short ss ; } s;

  sidx = (csysi_sampleunit *) malloc(sizeof(csysi_sampleunit));
  sidx->token = sym = csysi_readbit(16);
  sidx->len = len = csysi_readbit(24) + SAMP_DATABLOCK;
  sidx->p = (float *) malloc(len*sizeof(float));

  sidx->p[len - SAMP_LLMEM] = 0;
  if (csysi_readbit(1))  /* sampling rate */
    sidx->p[len - SAMP_SR] = csysi_readbit(17);
  else
    sidx->p[len - SAMP_SR] = EV(ARATE);

  if (csysi_readbit(1))  /* loop points */
    {
      sidx->p[len - SAMP_LOOPSTART] = csysi_readbit(24);
      sidx->p[len - SAMP_LOOPEND] = csysi_readbit(24);
    }
  else
    {
      sidx->p[len - SAMP_LOOPSTART] = -1;
      sidx->p[len - SAMP_LOOPEND] = -1;
    }

  if (csysi_readbit(1))  /* base frequency */
    {
      u.l = csysi_readbit(32);
      sidx->p[len - SAMP_BASEFREQ] = u.f;
    }
  else
    sidx->p[len - SAMP_BASEFREQ] = -1;

  i = 0;
  if (csysi_readbit(1))  /* float sample */
    {
      while (i < (len - SAMP_DATABLOCK))
	{
	  u.l = csysi_readbit(32);
	  sidx->p[i++] = u.f;
	}
    }
  else
    {      
      while (i < (len - SAMP_DATABLOCK))
	{
	  s.us = csysi_readbit(16);
	  sidx->p[i++] = s.ss*(1.0F/32767.0F);
	}
    }

  newsamp = sidx;

  if (!csysi_samples)
    {
      csysi_samples = newsamp;
      return;
    }

  if ((sidx = csysi_samples)->token > sym)
    {
      newsamp->next = csysi_samples;
      csysi_samples = newsamp;
      return;
    }

  while (sidx && (sidx->token <= sym))
    {

      if ((sidx->next == NULL) || 
	  (sidx->next->token > sym))
	break;

      if (sidx->token == sym)
	{
	  while (sidx->next && (sidx->next->token == sym))
	    sidx = sidx->next;
	  break;
	}
      sidx = sidx->next;
    }

  newsamp->next = sidx->next;
  sidx->next = newsamp;
  return;

}

/******************************************************************/
/*             checks if buffer is empty                          */
/******************************************************************/

int csysi_emptycheck(int len, csysi_mevent * mq)

{
  if (len)
    return 0;

  mq->cmd = CSYS_MIDI_NOOP;
  return 1;
}


/******************************************************************/
/*             puts midi data in csysi_midiqueue[idx]             */
/******************************************************************/

void csysi_midilinefill(void)

{
  int len, system, bpm;
  unsigned char nextbyte;
  csysi_mevent * mq;

  mq = &(csysi_midiqueue[csysi_headmidi]);
  mq->atime = csysi_bitaccesstime;

  if (csysi_tailmidi == -1)
    csysi_tailmidi = csysi_headmidi;
  csysi_headmidi = (csysi_headmidi+1)&(CSYSI_MAXMIDI-1);

  len = csysi_readbit(24);

  csysi_nummidi++;
  if (csysi_emptycheck(len, mq))
    return;
  nextbyte = (unsigned char)csysi_readbit(8); 
  len--;

  if (!(nextbyte & 0x80))
    {
      mq->cmd = csysi_runstat;
      mq->ndata = nextbyte;
    }
  else
    {
      mq->cmd = nextbyte;
      if ((nextbyte & CSYSI_MIDIMASKCOM) != CSYSI_MIDISYSTEM)
	{      
	  csysi_runstat = nextbyte;
	  if (csysi_emptycheck(len, mq))
	    return;
	  mq->ndata = (unsigned char)csysi_readbit(8);
	  len--;
	}
    }

  mq->extchan = CSYSI_MIDIMASKCHAN & mq->cmd;

  switch ((mq->cmd)&CSYSI_MIDIMASKCOM) {
  case CSYS_MIDI_NOTEOFF:  /* two byte commands */
  case CSYS_MIDI_NOTEON:
  case CSYS_MIDI_PTOUCH:
  case CSYS_MIDI_WHEEL:
  case CSYS_MIDI_CC:
    if (csysi_emptycheck(len, mq))
      return;
    mq->vdata = (unsigned char)csysi_readbit(8);
    len--;
    break;
  case CSYS_MIDI_CTOUCH:   /* one byte commands */
  case CSYS_MIDI_PROGRAM:
    break;
  case CSYSI_MIDISYSTEM:   /* tempo command */
    if (mq->cmd != 0xFF)
      break;
    if (csysi_emptycheck(len, mq))
      return;
    if (((unsigned char)csysi_readbit(8)) != CSYSI_METATEMPO )
      {
	len--;
	break;
      }
    len--;

    if (len >= 4)
      {
	csysi_readflush(8); 
	bpm = csysi_readbit(24); 
	len -= 4;
	if (bpm)
	  {
	    mq->fval = 60e6F/bpm;
	    mq->cmd = CSYS_MIDI_NEWTEMPO;
	  }
	else
	  mq->cmd = CSYS_MIDI_NOOP;
      }
    else
      mq->cmd = CSYS_MIDI_NOOP;

    break;
  }
  
  csysi_readflush(len*8);  /* one command per event, spec says */

}

/******************************************************************/
/*             finds next streaming score event                   */
/******************************************************************/

int csysi_readnewevent(void)

{
  int type, ret;

  if (ret = csysi_readbit(1))  /* more data here */
    {
      type = csysi_readbit(2);
      switch (type)
	{
	case CSYSI_EVSCORE:
	  csysi_scorelinefill();
	  break;
	case CSYSI_EVMIDI:
	  csysi_midilinefill();
	  break;
	case CSYSI_EVSAMPLE:
	  csysi_sampleread();
	  break;
	default:
	  csys_terminate("Unknown event type in .mp4 file");
	}
    }
  return ret;

}

/******************************************************************/
/*        adds an endtime command to a queue                      */
/******************************************************************/

int csysi_setendtime(float fval)

{
  int ret = 0;
  
  if ((csysi_headmidi&(CSYSI_MAXMIDI-1)) != csysi_tailmidi)
    {
      csysi_midiqueue[csysi_headmidi].atime = csysi_bitaccesstime;
      csysi_midiqueue[csysi_headmidi].cmd = CSYS_MIDI_ENDTIME;
      csysi_midiqueue[csysi_headmidi].fval = fval;
      if (csysi_tailmidi == -1)
	csysi_tailmidi = csysi_headmidi;
      csysi_headmidi = (csysi_headmidi+1)&(CSYSI_MAXMIDI-1);
      csysi_nummidi++;
      ret = 1;
    }
  return ret;

}


/****************************************************************/
/*           fill sasl and midi queues with more data           */
/****************************************************************/

void csysi_fillqueues(void)

{
  /* while data is left in file, and room left in queues, */
  /* fill queues with more data                           */

  while (csysi_moreaccessunits &&
      ((csysi_headsco&(CSYSI_MAXSASL-1)) != csysi_tailsco) && 
      ((csysi_headabs&(CSYSI_MAXSASL-1)) != csysi_tailabs) && 
      ((csysi_headmidi&(CSYSI_MAXMIDI-1)) != csysi_tailmidi))
    {
      if (csysi_endofevent)
	{
	  csysi_moreaccessunits = csysi_readaccesstime();
	}
      if (csysi_moreaccessunits)
	{
	  csysi_endofevent = !csysi_readnewevent();
	}
    }

  if (!(CSYS_GIVENENDTIME) && !csysi_moreaccessunits &&
      !(csysi_endflag))
    {
      csysi_endflag = csysi_setendtime(csysi_compendtime);
    }

  return;

}

/****************************************************************/
/*           finds, opens, and seeks streaming file             */
/****************************************************************/

void csysi_openfile(void)

{
  int i;
  char * name;

  /* first search ./sa file command line for .mp4 file */

  for (i=1;i<EV(csys_argc); i++)
    if (!strcmp(EV(csys_argv[i]),"-csys_fstr_file"))
      {
	i++;
	if (i == EV(csys_argc))
	  csys_terminate(".mp4 file not specified");
	if (!(csysi_bitfile = fopen(EV(csys_argv[i]),"rb")))
	  {
	    name = (char *) calloc(strlen(EV(csys_argv[i]))+5, sizeof(char));
	    sprintf(name,"%s.mp4",EV(csys_argv[i]));
	    if (!(csysi_bitfile = fopen(name,"rb")))
	      csys_terminate(".mp4 file not found");
	  }
	break;
      }

  /* then look though sfront command line for -bitc */

  if (!csysi_bitfile)
    {
      for (i=1;i<csys_sfront_argc; i++)
	if (!strcmp(csys_sfront_argv[i],"-bitc"))
	  {
	    i++;
	    if (i == csys_sfront_argc)
	      csys_terminate("-bitc file.mp4 unspecified");
	    if (!(csysi_bitfile = fopen(csys_sfront_argv[i],"rb")))
	      {
		name = (char *) calloc(strlen(csys_sfront_argv[i])+5,
				       sizeof(char));
		sprintf(name,"%s.mp4",csys_sfront_argv[i]);
		if (!(csysi_bitfile = fopen(name,"rb")))
		  csys_terminate("-bitc file.mp4 not found");
	      }
	    break;
	  }
    }

  if (!csysi_bitfile)
    csys_terminate(
	"Syntax: ./sa -csys_fstr_file file.mp4, or sfront -bitc file.mp4");

  /* if an explicit SASL endtime not give, invalidate computed endtime */

  if (!(CSYS_GIVENENDTIME))
    {
      csysi_compendtime = EV(endtime);
      if (!csysi_setendtime(CSYSI_MAXENDTIME))
	csys_terminate("problem setting init endtime");
    }

  /* skip to access_units, fill up queues */

  csysi_flushconfig();
  csysi_fillqueues();

}

/****************************************************************/
/*             initialization routine for control               */
/****************************************************************/

int csys_setup(void)
     
{
  csysi_openfile();
  return CSYS_DONE;
}


/****************************************************************/
/*             polling routine for new data                     */
/****************************************************************/

int csys_newdata(void)
     
{
  int i, found;

  csysi_absready = 0; 
  csysi_scoready = 0; 
  csysi_midiready = 0; 

  csysi_fillqueues();

  /* see if an event is ready */

  i = csysi_tailsco;
  found = ((i = csysi_tailsco) < 0);
  while (csysi_numscotime && !found)
    {
      if (csysi_scoqueue[i].stime <= EV(scorebeats))
	csysi_scoready++;
      else
	found = 1;
      i = (i+1)&(CSYSI_MAXSASL-1);
      if (i == csysi_headsco)
	found = 1;
    }
  
  i = csysi_tailabs;
  found = ((i = csysi_tailabs) < 0);
  while (csysi_numabstime && !found)
    {
      if (csysi_absqueue[i].atime < EV(absolutetime))
	csysi_absready++;
      else
	found = 1;
      i = (i+1)&(CSYSI_MAXSASL-1);
      if (i == csysi_headabs)
	found = 1;
    }
  
  i = csysi_tailmidi;
  found = ((i = csysi_tailmidi) < 0);
  while (csysi_nummidi && !found)
    {
      if (csysi_midiqueue[i].atime < EV(absolutetime))
	csysi_midiready++;
      else
	found = 1;
      i = (i+1)&(CSYSI_MAXMIDI-1);
      if (i == csysi_headmidi)
	found = 1;
    }

  if (csysi_midiready)
    {
      if ((csysi_absready || csysi_scoready))
	return CSYS_EVENTS;
      else
	return CSYS_MIDIEVENTS;
    }
  else
    {
      if ((csysi_absready || csysi_scoready))
	return CSYS_SASLEVENTS;
      else
	return CSYS_NONE;
    }

}

/****************************************************************/
/*                 processes a SASL event                       */
/****************************************************************/

int csys_saslevent(unsigned char * cmd, unsigned char * priority,
		   unsigned short * id, unsigned short * label,
		   float * fval, unsigned int * pnum, float ** p)

{
  int i, done, j, scofound;
  csysi_sevent * cq;


  done = 1;
  if (csysi_scoready)
    {
      cq = &(csysi_scoqueue[csysi_tailsco]);
      scofound = 1;
    }
  else
    {
      if (csysi_absready)
	{
	  cq = &(csysi_absqueue[csysi_tailabs]);
	  scofound = 0;
	}
      else
	csys_terminate("saslevent() queue error");
    }

  *priority = cq->priority;
  *id = cq->id;
  *label = cq->label;
  *fval = cq->fval;
  *pnum = cq->pnum;
  *p = cq->p;

  switch (*cmd = cq->cmd) {
  case CSYS_SASL_ENDTIME :  
    *fval = EV(scorebeats);
    break;
  case CSYS_SASL_NOOP :
  case CSYS_SASL_TEMPO : 
  case CSYS_SASL_TABLE :
  case CSYS_SASL_INSTR :
    break;
  case CSYS_SASL_CONTROL :
    if ((*id) == CSYS_SASL_NOINSTR)
      break;
    if (csysi_targetvar == -1)
      {
	csysi_targetvar=0;
	while (csysi_targetvar < CSYS_TARGETNUM)
	  {
	    if (csys_target[csysi_targetvar].token == (*pnum))
	      break;
	    csysi_targetvar++;
	  }
	if (csysi_targetvar == CSYS_TARGETNUM) 
	  {
	    /* specified variable not imported -- abort command */
	    *cmd = CSYS_SASL_NOOP;
	    csysi_targetvar = -1;
	    break;
	  }
	csysi_targetcount = 0;
      }
    *id = csys_target[csysi_targetvar].instrindex[csysi_targetcount];
    *pnum = csys_target[csysi_targetvar].varindex[csysi_targetcount];
    if ((++csysi_targetcount) < csys_target[csysi_targetvar].numinstr)
      done = 0;
    else
      csysi_targetvar = -1;
    break;
  }

  if (done)
    {
      if (scofound)
	{
	  csysi_scoready--;
	  csysi_numscotime--;
	  csysi_tailsco = (csysi_tailsco+1)&(CSYSI_MAXSASL-1);
	  if (csysi_tailsco == csysi_headsco)  /* empty */
	    {
	      csysi_headsco = 0;
	      csysi_tailsco = -1;
	    }
	}
      else
	{
	  csysi_absready--;
	  csysi_numabstime--;
	  csysi_tailabs = (csysi_tailabs+1)&(CSYSI_MAXSASL-1);
	  if (csysi_tailabs == csysi_headabs)  /* empty */
	    {
	      csysi_headabs = 0;
	      csysi_tailabs = -1;
	    }
	}
    }

  if (csysi_scoready || csysi_absready)
    return CSYS_SASLEVENTS;
  else
    return CSYS_NONE;

}

	
/****************************************************************/
/*                  closing routine for control                 */
/****************************************************************/

void csys_shutdown(void)
     
{
}


/****************************************************************/
/*                 processes a MIDI event                       */
/****************************************************************/


int csys_midievent(unsigned char * cmd,   unsigned char * ndata, 
	           unsigned char * vdata, unsigned short * extchan,
		   float * fval)

{

  int i = csysi_tailmidi;

  *cmd = csysi_midiqueue[i].cmd;
  *ndata = csysi_midiqueue[i].ndata;
  *vdata = csysi_midiqueue[i].vdata;
  *extchan = csysi_midiqueue[i].extchan;
  *fval = csysi_midiqueue[i].fval;

  if (*cmd == CSYS_MIDI_ENDTIME)
    *fval = (*fval > EV(scorebeats) + 5.0F) ? *fval : EV(scorebeats) + 5.0F;

  csysi_midiready--;
  csysi_nummidi--;

  csysi_tailmidi = (csysi_tailmidi+1)&(CSYSI_MAXMIDI-1);
  if (csysi_tailmidi == csysi_headmidi)  /* empty */
    {
      csysi_headmidi = 0;
      csysi_tailmidi = -1;
    }

  if (csysi_midiready)
    return CSYS_MIDIEVENTS;
  else
    return CSYS_NONE;

}

#undef SAMP_SR 
#undef SAMP_LOOPSTART 
#undef SAMP_LOOPEND 
#undef SAMP_BASEFREQ 
#undef SAMP_DATABLOCK 


