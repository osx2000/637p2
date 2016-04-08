
/*
#    Sfront, a SAOL to C translator    
#    This file: Writes binary mp4 files
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
/*           writes numbits LSBs of bitval to boutfile            */
/******************************************************************/


void bitwrite(unsigned int bitval, unsigned int numbits)

{

  while (numbits > 0)
    {
      bitstowrite |= ((bitval & (1 << (numbits-1))) != 0) << bitwritepos;
      numbits--;
      bitwritepos--;
      if (bitwritepos < 0)
	{
	  if (fwrite(&bitstowrite,sizeof(char),1,boutfile)!=1)
	    {
	      internalerror("mp4write.c","bitwrite() -- file writing");
	    }
	  bitstowrite = 0;
	  bitwritepos = 7;
	}
    }
  
}

/******************************************************************/
/*                    flushes boutfile                            */
/******************************************************************/


void bitflush(void)

{
  if (bitwritepos != 7)
    {
      if (fwrite(&bitstowrite,sizeof(char),1,boutfile)!=1)
	internalerror("mp4write.c","in bitflush() -- write error");
      bitstowrite = 0;
      bitwritepos = 7;
    }
}



/******************************************************************/
/*           adds a sample file to be written                     */
/******************************************************************/


sigsym * addsamplefile(char * val)

{
  sigsym * sptr;
  char name[STRSIZE];

  if (addvsym(&bitsampleout, val, K_INTERNAL) == INSTALLED)
    {
      if (bitsampleout->next != NULL)
	bitsampleout->special =
	  bitsampleout->next->special + 1;
      sprintf(name,"_sym_sample%i",
	      bitsampleout->special);
      if ((addvsym(&bitsymtable, name, K_INTERNAL)
	   == INSTALLED)
	  && (bitsymtable->next != NULL))
	bitsymtable->special =
	  bitsymtable->next->special + 1;
      sptr = bitsymtable;
    }
  else
    {
      sptr = getvsym(&bitsampleout, val);
      if (sptr == NULL)
	internalerror("mp4write.c",
		      "in addsamplefile() -- STRCONST1");
      sprintf(name,"_sym_sample%i", sptr->special);
      sptr = getvsym(&bitsymtable, name);
      if (sptr == NULL)
	internalerror("mp4write.c",
		      "in addsamplefile() -- STRCONST2");
    }

  return sptr;
}


/******************************************************************/
/*              updates global block state machine                */
/******************************************************************/

int globalstateupdate(tnode ** t, tnode ** g, int * gs)

{
  int globalstate;
  tnode * tptr, * gptr;
  int ret;

  tptr = *t;
  gptr = *g;
  globalstate = *gs;
  ret = 1;

  if (yylval->ttype == S_RC)
    switch (globalstate) {
    case GLOBAL_FIRST:              /* end of first global block */
      globalstate = GLOBAL_REST; 
      gptr = tptr;
      break;
    case GLOBAL_ACTIVE:             /* end of later global block */
      ret = yylex();                /* move past RC */
      globalstate = GLOBAL_REST; 
      break;
    }
  if (yylval->ttype == S_GLOBAL)
    {
      if (globalstate)              /* not GLOBAL_DORMANT */
	{
	  yylex();                 /* get LC, never returns 0 */
	  yylex();                 /* get first token, never returns 0 */
	}
      globalstate++;               /* GLOBAL_DORMANT -> GLOBAL_FIRST  */
                                   /* GLOBAL_REST    -> GLOBAL_ACTIVE */
    }
 
  *t = tptr;
  *g = gptr;
  *gs = globalstate;
  return ret;

}

/******************************************************************/
/*              updates global block state machine                */
/******************************************************************/

void printstateupdate(int * printflag)

{
  if (yylval->ttype == S_PRINTF)
    *printflag = 1;
  if (yylval->ttype == S_SEM)
    *printflag = 0;
}


/******************************************************************/
/*                       writes saol tokens                       */
/******************************************************************/

void bitsaolwrite(void)

{
  unsigned int intval;
  int i;
  union { unsigned int l; float f ; } u;
  sigsym * sptr;
  tnode * saolroot, * tptr, * gptr;
  int ntokens = 2;         /* includes S_EOO */
  int printflag = 0;
  int globalstate = 0;

  /* redo lexical analysis, count tokens */

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
  globalstate = (yylval->ttype == S_GLOBAL) ? GLOBAL_FIRST : GLOBAL_DORMANT;

  while (yylex()) 
    {
      if (!printflag && (yylval->ttype != S_PRINTF))
	{
	  if (!globalstateupdate(&tptr, &gptr, &globalstate))
	    break;
	  if (globalstate == GLOBAL_ACTIVE)
	    {
	      yylval->next = gptr->next;
	      gptr->next = yylval;
	      gptr = yylval;
	    }
	  else
	    {
	      tptr->next = yylval;
	      tptr = yylval;
	    }
	  if ((yylval->ttype == S_IDENT) && (!identtoken(yylval)))
	    if ((addvsym(&bitsymtable, yylval->val, K_INTERNAL) == INSTALLED)
		&& (bitsymtable->next != NULL))
	      bitsymtable->special = bitsymtable->next->special + 1;
	  ntokens++;
	}
      else
	printstateupdate(&printflag);
    }
  bitwrite(ntokens,16);  

  /* write out tokens */

  while (saolroot != NULL)
    {
      if ((saolroot->ttype < 0x25)||
	  ((saolroot->ttype > 0x4F) &&( saolroot->ttype < 0x68)))
	{
	  bitwrite(saolroot->ttype, 8);
	}
      else
	{
	  switch(saolroot->ttype) {
	  case S_IDENT:
	    if ((i = identtoken(saolroot)))
	      {
		bitwrite(i, 8);
	      }
	    else
	      {
		bitwrite(S_IDENT, 8);
		sptr = getvsym(&bitsymtable,saolroot->val);
		if (sptr == NULL)
		  internalerror("mp4write.c","in bitsaolwrite() -- S_IDENT");
		bitwrite(sptr->special, 16);
	      }
	    break;
	  case S_STRCONST:       /* must be sample, convert to token */
	    sptr = addsamplefile(saolroot->val);
	    bitwrite(S_IDENT, 8);
	    bitwrite(sptr->special, 16);
	    break;
	  case S_INTGR:
	    if (saolroot->val[0] != '-')
	      intval = strtoul(saolroot->val, NULL, 10);
	    else
	      {
		bitwrite(S_MINUS, 8);
		intval = strtoul(&(saolroot->val[1]), NULL, 10);
	      }
	    if (intval < 256)
	      {
		bitwrite(S_BYTE, 8);
		bitwrite(intval, 8);
	      }
	    else
	      {	  
		bitwrite(S_INTGR, 8);
		bitwrite(intval, 32);
	      }
	    break;
	  case S_NUMBER:
	    bitwrite(saolroot->ttype, 8);
	    u.f = (float)atof(saolroot->val);
	    bitwrite(u.l, 32);
	    break;
	  default:
	    break;
	  }
	}
      saolroot = saolroot->next;
    }
  bitwrite(S_EOO, 8);
}

/******************************************************************/
/*                     write symbol table                         */
/******************************************************************/

void bitsymbolwrite(void)

{
  unsigned int totsym;
  sigsym * sptr;
  int i,j, slen;

  totsym = bitsymtable->special+1;
  sptr = reversetable(sclone(bitsymtable));

  do {

    i = totsym % 65535;

    bitwrite(1,1);       /* more data */
    bitwrite(BINSYM,3);  /* symbol table file */

    bitwrite(i,16);     /* length of symbol table */
    while ( i > 0 )
      {

	slen = strlen(sptr->val);
	slen = (slen < 15) ? slen : 15;
	bitwrite(slen,4); /* string length */

	for (j=0; j < slen; j++) /* char by char */
	  bitwrite((unsigned char) (sptr->val[j]), 8);
	totsym--;
	i--;
	sptr = sptr->next;
      }

  } while (totsym > 0);

}


/******************************************************************/
/*                  writes preamable of SASL statement            */
/*       change absolute to 1 for absolute-time SASL encode       */
/*             but write bad config files presently ...           */
/******************************************************************/


void saslpreamble(float reltime, int priority)

{
  union { unsigned int l; float f ; } u;
  int absolute = 0;

  if (absolute)
    {
      bitwrite(0,1);              /* does not have time */
    }
  else
    {
      bitwrite(1,1);              /* has time */
      bitwrite(1,1);              /* use if late */
      u.f = reltime;              /* float timestamp */
      bitwrite(u.l, 32);
    }
  bitwrite(priority,1);              /* priority */

}

/******************************************************************/
/*                   write SASL end statement                     */
/******************************************************************/

void saslendwrite(char * endstr)

{  
  union { unsigned int l; float f ; } u;

  if (endstr == NULL)
    return;

  bitwrite(1,1);              /* has time */
  bitwrite(1,1);              /* use if late */
  u.f = (float)atof(endstr);  /* float timestamp */
  bitwrite(u.l, 32);           
  bitwrite(1,1);              /* priority */
  bitwrite(BINEND,3);         /* end opcode */

}



/******************************************************************/
/*                   write SASL tempo statement                   */
/******************************************************************/

void sasltempowrite(tnode * tempo)

{  
  union { unsigned int l; float f ; } u;

  saslpreamble(tempo->time, tempo->special);
  bitwrite(BINTEMPO,3);   /* tempo opcode */
  
  u.f = (float)atof(tempo->down->next->next->val); /* float tempo */
  bitwrite(u.l, 32);    

}


/******************************************************************/
/*                   write SASL table statement                   */
/******************************************************************/

void sasltablewrite(tnode * table, int sampletoken)

{  
  union { unsigned int l; float f ; } u;
  sigsym * sptr;
  int j, token;
  tnode * tptr;

  saslpreamble(table->time, table->special);
  bitwrite(BINTABLE,3);   /* table opcode */

  sptr = getvsym(&bitsymtable, /* write table symbol */
		 table->down->next->next->val);
  if (sptr == NULL)
    internalerror("mp4write.c","sasltablewrite() -- table");
  bitwrite(sptr->special,16);
  if (!strcmp("destroy",
	      table->down->next->next->next->val))
    {
      bitwrite(1,1);          /* destroy table */
    }
  else
    {
      bitwrite(0,1);          /* don't destroy table */
      token = identtoken(table->down->next->next->next);
      token = (token == S_BUZZOPCODE) ? S_BUZZWAVE : token;
      bitwrite(token,8);
      if (token == S_SAMPLE)
	{
	  bitwrite(1,1);          /* refer to sample */
	  bitwrite(sampletoken, 16);
	}
      else
	{
	  bitwrite(0,1);          /* no sample included */
	}
      bitwrite(table->width, 16); /* number of pfields */
      j = table->width;
      tptr = table->down->next->next->next->next;
      while (tptr != NULL)
	{
	  switch (tptr->ttype) {
	  case S_NUMBER:
	  case S_INTGR:
	    u.f = (float)atof(tptr->val); /* normal pfield */
	    bitwrite(u.l, 32);
	    break;
	  case S_STRCONST:
	    bitwrite(0, 32);             /* dummy value for str */
	    break;
	  case S_IDENT:
	    sptr = getvsym(&bitsymtable, tptr->val);
	    if (!(sptr = getvsym(&bitsymtable, tptr->val)))
	      internalerror("mp4write.c","sasltablewrite() -- concatnames");
	    bitwrite(sptr->special,16); /* table symbol name */
	    break;
	  }
	  j--;
	  tptr = tptr->next;
	}
      if (j != 0)
	internalerror("mp4write.c","sasltablewrite() -- pfields");
    }


}


/******************************************************************/
/*                   write SASL instr statement                   */
/******************************************************************/

void saslinstrwrite(tnode * instr)

{  
  union { unsigned int l; float f ; } u;
  sigsym * sptr;
  int j;
  tnode * tptr;

  saslpreamble(instr->time, instr->special);
  bitwrite(BININSTR,3);       /* instr opcode */

  if (instr->down->ttype == S_IDENT)  /* has label */
    {
      bitwrite(1,1);          /* has label */
      sptr = getvsym(&bitsymtable, instr->down->val);
      if (sptr == NULL)
	internalerror("mp4write.c","bitscorewrite() -- controlvar");
      bitwrite(sptr->special,16);
      sptr = getvsym(&bitsymtable, 
		     instr->down->next->next->next->val);
      if (sptr == NULL)
	internalerror("mp4write.c","bitscorewrite() -- instr1");
      bitwrite(sptr->special,16); /* instr name symbol */
      tptr = instr->down->next->next->next->next;  /* dur */
    }
  else
    {
      bitwrite(0,1);          /* hasn't label */

      sptr = getvsym(&bitsymtable, instr->down->next->val);
      if (sptr == NULL)
	internalerror("mp4write.c","bitscorewrite() -- instr2");
      bitwrite(sptr->special,16); /* instr name symbol */
      tptr = instr->down->next->next; /* points to dur */
    }
	    
  u.f = (float)atof(tptr->val); /* float duration */
  bitwrite(u.l, 32);    

  instr->width = (instr->width > 255) ? 255 : instr->width;
  bitwrite(instr->width, 8); /* number of pfields */
  tptr = tptr->next;
  j = instr->width;
  while ((j > 0) && (tptr != NULL))
    {
      u.f = (float)atof(tptr->val); /* pfield value */
      bitwrite(u.l, 32);    
      j--;
      tptr = tptr->next;
    }

}


/******************************************************************/
/*                   write SASL control statement                 */
/******************************************************************/

void saslcontrolwrite(tnode * control)

{  
  union { unsigned int l; float f ; } u;
  sigsym * sptr;
  tnode * tptr;

  saslpreamble(control->time, control->special);
  bitwrite(BINCONTROL,3);        /* control opcode */

  if (control->down->next->ttype == S_IDENT)  /* has label */
    {
      bitwrite(1,1);             
      sptr = getvsym(&bitsymtable, control->down->next->val);
      if (sptr == NULL)
	internalerror("mp4write.c","bitscorewrite() -- label");
      bitwrite(sptr->special,16);
      sptr = getvsym(&bitsymtable, 
		     control->down->next->next->next->val);
      if (sptr == NULL)
	internalerror("mp4write.c","bitscorewrite() -- controlvar");
      bitwrite(sptr->special,16); /* control varname symbol */
      tptr = control->down->next->next->next->next;  /* value */
    }
  else
    {
      bitwrite(0,1);          /* hasn't label */

      sptr = getvsym(&bitsymtable, control->down->next->next->val);
      if (sptr == NULL)
	internalerror("mp4write.c","bitscorewrite() -- controlvar");
      bitwrite(sptr->special,16); /* control name symbol */
      tptr = control->down->next->next->next; /* value */
    }

  u.f = (float)atof(tptr->val); /* control value */
  bitwrite(u.l, 32);        

}


/******************************************************************/
/*                     write score file                           */
/******************************************************************/

void bitscorewrite(void)

{
  unsigned int totscore;
  int i;
  tnode * tempo, * table, * control, * instr;
  sigsym * sptr;
  char * endstr;

  totscore = confsasl->scorefsize;
  tempo = confsasl->temporoot;
  table = confsasl->tableroot;
  control = confsasl->controlroot;
  instr = confsasl->instrroot;
  endstr = confsasl->endtimeval;

  do {

    i = totscore % 1048575;

    bitwrite(1,1);         /* more data */
    bitwrite(BINSCORE,3);  /* a score file */
    bitwrite(i,20);        /* length of file */

    while ( i > 0 )
      {
	if (endstr != NULL)
	  {
	    saslendwrite(endstr);
	    endstr = NULL;
	    i--;
	    totscore--;
	    continue;
	  }

	if (tempo != NULL)
	  {
	    if (strcmp("MIDItempo",tempo->down->next->val))
	      {
		sasltempowrite(tempo);
		i--;
		totscore--;
	      }
	    tempo = tempo->next;
	    continue;
	  }

	if (table != NULL)
	  {
	    if (identtoken(table->down->next->next->next) == S_SAMPLE)
	      {
		sptr = addsamplefile(
			     table->down->next->next->next->next->next->val);
		sasltablewrite(table, sptr->special);
	      }
	    else
	      sasltablewrite(table, 0);
	    table = table->next;
	    i--;
	    totscore--;
	    continue;
	  }

	if (control != NULL)
	  {
	    if (strcmp("MIDIcontrol",control->down->next->val))
	      {
		saslcontrolwrite(control);
		i--;
		totscore--;
	      }
	    control = control->next;
	    continue;
	  }

	if (instr != NULL)
	  {
	    saslinstrwrite(instr);
	    instr = instr->next;
	    i--;
	    totscore--;
	    continue;
	  }

      }

  } while (totscore > 0);


}

/******************************************************************/
/*                     write midifile                         */
/******************************************************************/

void bitmidiwrite(void)

{
  unsigned char c;
  unsigned int i = confmidi->midifsize;

  bitwrite(1,1);        /* more data */
  bitwrite(BINMIDI,3);  /* a midi file */

  bitwrite(confmidi->midifsize,32); /* length of file */
  rewind(midifile);

  while (i > 0)
    {
      if (fread(&c, sizeof(char), 1, midifile) != 1)
	internalerror("mp4write.c","bitmidiwrite() -- filecount1");
      bitwrite(c,8);
      i--;
    }
  if ((fread(&c, sizeof(char), 1, midifile) == 1))
    internalerror("mp4write.c","bitmidiwrite() -- filecount2");
  
}

/*********************************************************/
/*              reports sample error                  */
/*********************************************************/

void bitsounderror(sigsym * sptr)

{
  printf("Error: During MP4 write (corrupt sample: %s).\n",
	 sptr->val);
  noerrorplace();
}


/*********************************************************/
/*    gets next block of WAV/AIFF bytes -- unsigned      */
/*********************************************************/

void getsoundbytes(unsigned char * c, int numbytes)

{
  if ((int)fread(c, sizeof(char), numbytes, soundfile) != numbytes)
    {
      printf("Error: During MP4 write (corrupt sample).\n");
      noerrorplace();
    }

}

/*********************************************************/
/*    gets next block of WAV/AIFF bytes -- signed        */
/*********************************************************/

void getsoundsbytes(char * c, int numbytes)

{
  if ((int)fread(c, sizeof(char), numbytes, soundfile) != numbytes)
    {
      printf("Error: During MP4 write (corrupt sample).\n");
      noerrorplace();
    }

}

/*********************************************************/
/*           gets next WAV/AIFF cookie -- 1 if EOF       */
/*********************************************************/

int getnextcookie(unsigned char * c)

{
  if ((int)fread(c, sizeof(char), 4, soundfile) == 4)
    return 0;
  else
    return 1;

}


/*********************************************************/
/*        flushes next block of WAV/AIFF bytes           */
/*********************************************************/

void flushsoundbytes(int numbytes)

{
  unsigned char c;

  while (numbytes > 0)
    {
      if (fread(&c, sizeof(char), 1, soundfile) != 1)
	{
	  printf("Error: During MP4 write (corrupt sample).\n");
	  noerrorplace();
	}
      numbytes--;
    }

}

/*********************************************************/
/*     converts byte stream to an unsigned int          */
/*********************************************************/

unsigned int wavefileintval(int numbytes)

{
  unsigned int ret = 0;
  unsigned char c[4];

  if (numbytes > 4)
    {
      printf("Error: During MP4 write (corrupt sample).\n");
      noerrorplace();
    }
  getsoundbytes(&c[0],numbytes);
  switch (numbytes) {
  case 4:
    ret  =  (unsigned int)c[0];
    ret |=  (unsigned int)c[1] << 8;
    ret |=  (unsigned int)c[2] << 16;
    ret |=  (unsigned int)c[3] << 24;
    break;
  case 3:
    ret  =  (unsigned int)c[0];
    ret |=  (unsigned int)c[1] << 8;
    ret |=  (unsigned int)c[2] << 16;
    break;
  case 2:
    ret  =  (unsigned int)c[0];
    ret |=  (unsigned int)c[1] << 8;
    break;
  case 1:
    ret = ((unsigned int)c[0]);
    break;
  default:
    break;
  }

  return ret;
}

  
/*********************************************************/
/*     converts byte stream to an unsigned int          */
/*********************************************************/

unsigned int aiffileintval(int numbytes)

{
  unsigned int ret = 0;
  unsigned char c[4];

  if (numbytes > 4)
    {
      printf("Error: During MP4 write (corrupt sample).\n");
      noerrorplace();
    }
  getsoundbytes(&c[0],numbytes);
  switch (numbytes) {
  case 4:
    ret  =  (unsigned int)c[3];
    ret |=  (unsigned int)c[2] << 8;
    ret |=  (unsigned int)c[1] << 16;
    ret |=  (unsigned int)c[0] << 24;
    break;
  case 3:
    ret  =  (unsigned int)c[2];
    ret |=  (unsigned int)c[1] << 8;
    ret |=  (unsigned int)c[0] << 16;
    break;
  case 2:
    ret  =  (unsigned int)c[1];
    ret |=  (unsigned int)c[0] << 8;
    break;
  case 1:
    ret = ((unsigned int)c[0]);
    break;
  default:
    break;
  }
  return ret;
}
  
/***********************************************************/
/*  checks byte stream for AIFF/WAV cookie --              */
/*    return 0 for found cookie                            */
/*    return 1 for different cookie                        */
/*    return 2 for EOF                                     */
/***********************************************************/

int soundtypecheck(char * d)

{
  int ret = 0;
  char c[4];

  if (fread(c, sizeof(char), 4, soundfile) != 4)
    ret = 2;
  else
    {
      if (strncmp(c,d,4))
	ret = 1;
    }
  return ret;
}
  
/******************************************************************/
/*                     write one samplefile                       */
/******************************************************************/

void nextsamplewrite(sigsym * sptr)

{
  unsigned int len = 0;
  unsigned int i, j, m, e;
  unsigned int srate = 0;  
  int channels = 1;         /* number of channels for the file              */
  int numbytes = 2;         /* number of bytes in a data word               */    
  int framebytes = 2;       /* number of bytes in one N-channel frame       */
  int chanpoint = 0;        /* 1st byte of channel in frame; -1 sum to mono */
  sigsym * s2ptr;
  unsigned char cookie[4];
  char name[STRSIZE];
  char * suffix;
  char at_n = -1;
  int ntotal = 0;
  int extlen, padbyte, markers;
  int * sbuff = NULL;
  int unity = 0;
  int fract = 0;
  int start = 0;
  int end = 0;
  int hasbase = 0;
  int hasloop = 0;
  char c;
  struct aiffmark * mptr = NULL;
  struct aiffmark * mhead = NULL;
  union { unsigned int l; float f ; } fwr;


  sprintf(name,"_sym_sample%i",sptr->special);
  s2ptr = getvsym(&bitsymtable, name);
  if (s2ptr == NULL)
    internalerror("mp4write.c","bitsamplewrite()");

  bitwrite(s2ptr->special, 16);  /* token */

  suffix = strrchr(sptr->val, '.');

  if ((suffix == NULL) ||
      !(strstr(suffix, ".wav") || strstr(suffix, ".WAV") || 
	strstr(suffix, ".aif") || strstr(suffix, ".AIF") || 
	strstr(suffix, ".aiff") || strstr(suffix, ".AIFF")))
    {
      printf("Error: Sample name %s lacks .wav or .aif(f) extension.\n",
	     sptr->val);
      noerrorplace();
    }

  extlen = (strstr(suffix, ".aiff") || strstr(suffix, ".AIFF")) ? 5 : 4;

  if (strlen(suffix) <= extlen)          
    soundfile = fopen(sptr->val,"rb");
  else
    {
      if ((sscanf(&(suffix[extlen]), "@%hhd%n", &at_n, &ntotal) < 1) || 
	  (strlen(suffix) != extlen + ntotal) || (at_n < 0))
	{
	  printf("Error: Syntax error in sample filename %s (or in @N channel).\n",
		 sptr->val);
	  noerrorplace();
	}
      suffix[extlen]= '\0';
      soundfile = fopen(sptr->val,"rb");
      suffix[extlen]= '@';
    }

  if (soundfile == NULL)
    bitsounderror(sptr);

  if (strstr(suffix,".wav")||strstr(suffix,".WAV"))
    {
      if (soundtypecheck("RIFF"))
	bitsounderror(sptr);
      flushsoundbytes(4);
      if (soundtypecheck("WAVE"))
	bitsounderror(sptr);
      while (soundtypecheck("fmt "))
	flushsoundbytes(wavefileintval(4));
      i = wavefileintval(4) - 16; /* length of format chunk */
      if (i < 0)
	bitsounderror(sptr);
      if (wavefileintval(2)!= 1) /* PCM wave files = 1 */
	bitsounderror(sptr);
      if ((channels = wavefileintval(2)) <= 0)
	bitsounderror(sptr);
      srate = wavefileintval(4);
      flushsoundbytes(6);
      numbytes = (wavefileintval(2)+7)/8;     /* bytes/sample */
      if ((numbytes <= 0) || (numbytes > 3))  /* 1/2/3 bytes only */
	{
	  printf("Error: Sample %s isn't 8/16/24 bit.\n",
		 sptr->val);
	  noerrorplace();
	}
      framebytes = channels*numbytes;
      if (channels == 1)
	chanpoint = 0;
      else
	{
	  if (at_n >= 0)
	    {
	      if (at_n > channels - 1)
		{
		  printf("Error: Sample generator @%i filename syntax requests\n"
			 "       the %i'th channel of an %i channel samplefile %s.\n", 
			 at_n, at_n+1, channels, sptr->val);
		  bitsounderror(sptr);
		}
	      chanpoint = at_n*numbytes;
	    }
	  else
	    chanpoint = -1;
	}
      flushsoundbytes(i + (i % 2));

      unity = fract = start = end = hasbase = hasloop = 0;
      while (!getnextcookie(cookie))
	{
	  if (!strncmp((char *) cookie,"data",4))
	    {
	      padbyte = ((i = wavefileintval(4)) % 2);
	      len = i = i/framebytes;
	      vmcheck(sbuff = (int *) calloc(len, sizeof(int)));
	      while (i > 0)
		{
		  sbuff[len - i] = 0;
		  for (j = 0; j < framebytes; j += numbytes)
		    if ((j == chanpoint) || (chanpoint < 0))
		      {
			switch (numbytes) {
			case 1:
			  sbuff[len - i] += (((int)wavefileintval(1))-128)<<8;
			  break;
			case 2:
			  sbuff[len - i] += ((int)wavefileintval(1));
			  sbuff[len - i] += ((int)((char)wavefileintval(1))) << 8;
			  break;
			case 3:
			  sbuff[len - i] += ((int)wavefileintval(1));
			  sbuff[len - i] += (((int)wavefileintval(1)) << 8);
			  sbuff[len - i] += ((int)((char)wavefileintval(1))) << 16;
			  break;
			}
		      }
		    else
		      flushsoundbytes(numbytes);
		  if (chanpoint < 0)
		    sbuff[len - i] /= channels; 
		  i--;
		}
	      if (padbyte)
		flushsoundbytes(1);
	    }
	  else
	    {
	      if (!strncmp((char *) cookie,"smpl",4))
		{
		  hasbase = 1;
		  i = wavefileintval(4);
		  flushsoundbytes(12);   /* Manu, Prod, Period */ 
		  unity = wavefileintval(4);
		  fract = wavefileintval(4);
		  flushsoundbytes(16);   /* SMPTEs, Loops, Data */ 
		  i -= 36;
		  if (i > 0)
		    {
		      hasloop = 1;
		      flushsoundbytes(8);   /* ID, Type */ 
		      start = wavefileintval(4)/framebytes;
		      end = wavefileintval(4)/framebytes;
		      flushsoundbytes(8);   /* Frac, count */
		      i -= 24;
		      flushsoundbytes(i + (i % 2));   /* other loops */ 
		    }
		}
	      else
		{
		  i = wavefileintval(4);
		  flushsoundbytes(i + (i % 2));
		}
	    }
	}
      bitwrite(len,24);               /* length */
      bitwrite(1, 1);                 /* has sample rate */
      bitwrite(srate,17);
      bitwrite(hasloop, 1);           /* loop points */
      if (hasloop)
	{
	  bitwrite(start, 24);        /* start of loop */
	  bitwrite(end, 24);          /* end of loop */
	}
      bitwrite(hasbase, 1);           /* base freq */
      if (hasbase)
	{
	  fwr.f = (float)(440.0*pow(2.0, 
		    ((double)unity + ((double)fract/4.295e9) - 69.0)/12.0));
	  bitwrite(fwr.l, 32);
	}
      if (numbytes == 3)
	bitwrite(1, 1);              /* write 24-bit data as floats */
      else
	bitwrite(0, 1);              /* otherwise, as 16-bit integers */
      i = len;
      if (sbuff == NULL)
	bitsounderror(sptr);
      if (numbytes == 3)
	while (i > 0)
	  {
	    fwr.f = ((float)pow(2, -23))*((float)(sbuff[len - (i--)]));
	    bitwrite(fwr.l, 32);
	  }
      else
	while (i > 0)
	  bitwrite(sbuff[len - (i--)], 16);
      free(sbuff);
    }
  else
    {
      if (soundtypecheck("FORM"))
	bitsounderror(sptr);
      flushsoundbytes(4);
      if (soundtypecheck("AIFF"))
	bitsounderror(sptr);
      
      /* right now, need COMM to come before SSND and INST */
      
      while (!getnextcookie(cookie))
	if (!strncmp((char *) cookie,"COMM",4))
	  {
	    i = aiffileintval(4) - 18;   /* length - parsed fields */
	    if ((channels = aiffileintval(2)) <= 0)
	      bitsounderror(sptr);
	    flushsoundbytes(4);       /* frames */
	    numbytes = aiffileintval(2);
	    if ((numbytes < 8)||(numbytes > 24))
	      {
		printf("Error: Sample %s isn't 8/16/24 bit.\n",
		       sptr->val);
		noerrorplace();
	      }
	    numbytes /= 8;
	    framebytes = channels*numbytes;
	    e = ((short)aiffileintval(2))&0x7FFF;
	    m = aiffileintval(4);
	    srate = (unsigned int) (0.5 + (m*exp(log(2)*(e - 16414.0F))));
	    if (channels == 1)
	      chanpoint = 0;
	    else
	      {
		if (at_n >= 0)
		  {
		    if (at_n > channels - 1)
		      {
			printf("Error: Sample generator @%i filename syntax requests\n"
			       "       the %i'th channel of an %i channel samplefile %s.\n", 
			       at_n, at_n+1, channels, sptr->val);
			bitsounderror(sptr);
		      }
		    chanpoint = at_n*numbytes;
		  }
		else
		  chanpoint = -1;
	      }
	    flushsoundbytes(4);
	    if (i)
	      flushsoundbytes(i + (i % 2));
	    break;
	  }
	else
	  {
	    i = aiffileintval(4);
	    flushsoundbytes(i + (i % 2));
	  }
      
      /* extract data from SSND, INST, MARK */
      
      while (!getnextcookie(cookie))
	{
	  if (!strncmp((char *) cookie,"SSND",4))
	    {
	      padbyte = ((i = (aiffileintval(4) - 8)) % 2);
	      len = i = i/framebytes;
	      flushsoundbytes(8);
	      vmcheck(sbuff = (int *) calloc(len, sizeof(int)));
	      while (i > 0)
		{
		  sbuff[len - i] = 0;
		  for (j = 0; j < framebytes; j += numbytes)
		    if ((j == chanpoint) || (chanpoint < 0))
		      {
			switch (numbytes) {
			case 1:
			  sbuff[len - i] += ((int)((char)wavefileintval(1))) << 8;
			  break;
			case 2:
			  sbuff[len - i] += ((int)((char)wavefileintval(1))) << 8;
			  sbuff[len - i] += ((int)wavefileintval(1));
			  break;
			case 3:
			  sbuff[len - i] += ((int)((char)wavefileintval(1))) << 16;
			  sbuff[len - i] += (((int)wavefileintval(1)) << 8);
			  sbuff[len - i] += ((int)wavefileintval(1));
			  break;
			}
		      }
		    else
		      flushsoundbytes(numbytes);
		  if (chanpoint < 0)
		    sbuff[len - i] /= channels;
		  i--;
		}
	      if (padbyte)
		flushsoundbytes(1);
	    }
	  else
	    {
	      if (!strncmp((char *) cookie,"INST",4))
		{
		  hasbase = 1;
		  i = aiffileintval(4) - 20;  /* length - parsed fields */
		  unity = aiffileintval(1); /* baseNote */
		  getsoundsbytes(&c,1);     /* sbytes .. because signed */
		  fract = c;                /* detune */
		  flushsoundbytes(6);       /* {lo,hi}{note,vel},gain */
		  if (aiffileintval(2))
		    hasloop = 1;
		  
		  /* start and end pointers into MARK chunk */
		  
		  start = aiffileintval(2); /* ID Marker for start */
		  end = aiffileintval(2);   /* ID Marker for end   */
		  flushsoundbytes(6);       /* release loop        */

		  if (i)
		    flushsoundbytes(i + (i % 2));
		}
	      else
		{
		  if (!strncmp((char *) cookie,"MARK",4))
		    {
		      i = aiffileintval(4);   /* size */
		      markers = aiffileintval(2); /* number of markers */
		      i -= 2;
		      
		      /* inserts marker positions into mhead list */

		      while (markers--)
			{
			  vmcheck(mptr = (aiffmark *)malloc(sizeof(aiffmark)));
			  mptr->id = (short) aiffileintval(2);
			  mptr->position = aiffileintval(4);
			  mptr->next = mhead;
			  mhead = mptr;
			  j = aiffileintval(1); /* pstring */
			  if (!(j&0x0001)) /* pad byte if even */
			    j++;
			  flushsoundbytes(j);
			  i -= 7 + j;
			}
		      if (i)
			flushsoundbytes(i + (i % 2));
		    }
		  else
		    {
		      i = aiffileintval(4);
		      flushsoundbytes(i + (i % 2));
		    }
		}
	    }
	}
      
      bitwrite(len,24);               /* length */
      bitwrite(1, 1);                 /* has sample rate */
      bitwrite(srate,17);
      if (hasloop)
	{
	  j = 0;
	  mptr = mhead;
	  while (mptr != NULL)
	    {
	      if (mptr->id == start)
		{
		  j = 1;
		  start = mptr->position;
		  if ((unsigned int)start == len)
		    start--;
		  break;
		}
	      mptr = mptr->next;
	    }
	  if (!j)
	    bitsounderror(sptr);      /* can't find start */
	  j = 0;
	  mptr = mhead;
	  while (mptr != NULL)
	    {
	      if (mptr->id == end)
		{
		  j = 1;
		  end = mptr->position;
		  if ((unsigned int)end == len)
		    end--;
		  break;
		}
	      mptr = mptr->next;
	    }
	  if (!j)
	    bitsounderror(sptr);      /* can't find end */
	  if (start > end)            /* in AIFF spec   */
	    hasloop = 0;
	}
      bitwrite(hasloop, 1);           /* loop points */
      if (hasloop)
	{
	  bitwrite(start, 24);        /* start of loop */
	  bitwrite(end, 24);          /* end of loop */
	}
      bitwrite(hasbase, 1);           /* base freq */
      if (hasbase)
	{
	  fwr.f = (float)(440.0*pow(2.0, 
		    ((double)unity + ((double)fract/100.0) - 69.0)/12.0));
	  bitwrite(fwr.l, 32);
	}


      if (numbytes == 3)
	bitwrite(1, 1);              /* write 24-bit data as floats */
      else
	bitwrite(0, 1);              /* otherwise, as 16-bit integers */
      i = len;
      if (sbuff == NULL)
	bitsounderror(sptr);
      if (numbytes == 3)
	while (i > 0)
	  {
	    fwr.f = ((float)pow(2, -23))*((float)(sbuff[len - (i--)]));
	    bitwrite(fwr.l, 32);
	  }
      else
	while (i > 0)
	  bitwrite(sbuff[len - (i--)], 16);
      free(sbuff);

      mptr = mhead;
      while (mptr)
	{
	  mhead = mptr->next;
	  free(mptr);
	  mptr = mhead;
	}
    }
  
  fclose(soundfile);

}
  
/******************************************************************/
/*                     write samplefiles                          */
/******************************************************************/

void bitsamplewrite(void)

{
  sigsym * sptr;

  sptr = bitsampleout;
  while (sptr != NULL)
    {
      bitwrite(1,1);                 /* more data */
      bitwrite(BINSAMP,3);           /* a sample file */
      nextsamplewrite(sptr);
      sptr = sptr->next;
    }
}

/******************************************************************/
/*               lowest level MIDI streaming writeout             */
/******************************************************************/

void bitmidievent(int d0, int d1, int d2)

{
  int size = 0;

  switch (d0 & MIDIMASKCOM) {
  case MIDINOTEOFF:
  case MIDINOTEON:
  case MIDIKEYTOUCH:
  case MIDICONTROL:
  case MIDIWHEEL:
    size = 3;
    break;
  case MIDIPATCH:
  case MIDICHTOUCH:
    size = 2;
    break;
  case MIDISYSTEM:
    size = 6;
    break;
  default:
    internalerror("mp4write.c","bitmidievent() -- invalid command");
  }

  bitwrite(size, 24);

  /* will change once 1 byte commands supported */

  bitwrite(d0, 8);
  if (size <= 3)
    {
      bitwrite(d1, 8);
      if (size == 3)
	bitwrite(d2, 8);
    }
  else
    {
      bitwrite(0x51, 8);  /* tempo opcode */
      bitwrite(3, 8);     /* 3 bytes of data */
      bitwrite(d1, 24);
    }
}

/******************************************************************/
/*           parses and writes MIDI streaming commands            */
/******************************************************************/

void bitmidistream(int lotype, int kcycleidx)

{
  int d1, d2;
  tnode * tptr;

  d1 = d2 = 0;

  switch (lotype & MIDIMASKCOM) {

    /* these commands all reside in abssasl->control */

  case MIDIKEYTOUCH:
    /* d1 is note, d2 is aftertouch value*/
    d1 = abssasl->controlroot->down->next->next->arrayidx; 
    d2 = (int)atof(abssasl->controlroot->down->next->next->next->val);
    abssasl->controlroot = abssasl->controlroot->next;
    break;
  case MIDICHTOUCH:
    /* d1 is aftertouch value*/
    d1 = (int)atof(abssasl->controlroot->down->next->next->next->val);
    abssasl->controlroot = abssasl->controlroot->next;
    break;
  case MIDICONTROL:
    /* d1 is controller number, d2 is controller value*/
    d1 = abssasl->controlroot->down->next->next->arrayidx; 
    d2 = (int)atof(abssasl->controlroot->down->next->next->next->val);
    abssasl->controlroot = abssasl->controlroot->next;
    break;
  case MIDIWHEEL:
    /* d1 is wheel lsb,, d2 is wheel msb*/
    d1 = (int)atof(abssasl->controlroot->down->next->next->next->val);
    d2 = (d1 >> 7) & 0x0000007F;
    d1 &= 0x0000007F;
    abssasl->controlroot = abssasl->controlroot->next;
    break;

    /* only META command is the tempo command */

  case MIDISYSTEM:
    d1 = (int)(60e6/atof(abssasl->temporoot->down->next->next->val));
    abssasl->temporoot = abssasl->temporoot->next;
    break;

    /* noteons and noteoffs */

  case MIDINOTEON:
    d1 = midicurrinstr[lotype & MIDIMASKCHAN]->res;
    d2 = midicurrinstr[lotype & MIDIMASKCHAN]->vartype;
    midicurrinstr[lotype & MIDIMASKCHAN] = 
      midicurrinstr[lotype & MIDIMASKCHAN]->next;
    break;
  case MIDINOTEOFF:
    tptr = midicurrnote[lotype & MIDIMASKCHAN];
    while (tptr && ((tptr->opwidth > kcycleidx) || 
		    (tptr->inwidth > kcycleidx) || tptr->usesinput))
      tptr = tptr->next;
    if (tptr)
      {
	d1 = tptr->res;
	d2 = tptr->vartype;
	tptr->usesinput = 1;
	while (midicurrnote[lotype & MIDIMASKCHAN] && 
	       (midicurrnote[lotype & MIDIMASKCHAN])->usesinput)
	  midicurrnote[lotype & MIDIMASKCHAN] = 
	    midicurrnote[lotype & MIDIMASKCHAN]->next;
      }
    else
      internalerror("mp4write.c","bitmidistream -- midinoteoff");
    break;
  }

  bitmidievent(lotype, d1, d2);
}


/******************************************************************/
/*          checks if more data left to stream out                */
/******************************************************************/

int morestream(float currtime, int kcycleidx, int * hitype, int * lotype)

{
  float minval = currtime;
  int i;
  tnode * tptr;

  *hitype = BINRES2;

  if (sstrsasl->temporoot && (sstrsasl->temporoot->time < minval))
    {
      minval = sstrsasl->temporoot->time;
      *hitype = BINSCORE;
      *lotype = BINTEMPO;
    }

  if (sstrsasl->tableroot && (sstrsasl->tableroot->time < minval))
    {
      minval = sstrsasl->tableroot->time;
      *hitype = BINSCORE;
      *lotype = BINTABLE;
    }

  if (sstrsasl->controlroot && (sstrsasl->controlroot->time < minval))
    {
      minval = sstrsasl->controlroot->time;
      *hitype = BINSCORE;
      *lotype = BINCONTROL;
    }

  if (sstrsasl->instrroot && (sstrsasl->instrroot->time < minval))
    {
      minval = sstrsasl->instrroot->time;
      *hitype = BINSCORE;
      *lotype = BININSTR;
    }

  if (((*hitype) == BINRES2) && abssasl->controlroot &&
      (abssasl->controlroot->opwidth <= kcycleidx))
    {
      if (!strcmp("MIDIcontrol", abssasl->controlroot->down->next->val))
	{
	  *hitype = BINMIDI;
	  *lotype = MIDIMASKCHAN;
	  if (!strcmp("MIDIbend", abssasl->controlroot->down->next->next->val))
	    *lotype = MIDIWHEEL;
	  if (!strcmp("MIDIctrl", abssasl->controlroot->down->next->next->val))
	    *lotype = MIDICONTROL;
	  if (!strcmp("MIDItouch",abssasl->controlroot->down->next->next->val))
	    {
	      if (abssasl->controlroot->down->next->next->arrayidx == -1)
		*lotype = MIDICHTOUCH;
	      else
		*lotype = MIDIKEYTOUCH;
	    }
	  if (*lotype == MIDIMASKCHAN)
	    internalerror("mp4write.c","morestream() -- MIDI control 1");

	  /* insert MIDI channel number */

	  *lotype |= abssasl->controlroot->down->next->next->down->width;
	}
      else
	 internalerror("mp4write.c","morestream() -- MIDI control 2");
    }

  if (((*hitype) == BINRES2) && abssasl->temporoot &&
      (abssasl->temporoot->opwidth <= kcycleidx))
    {
      *hitype = BINMIDI;
      *lotype = MIDIMETA;
    }

  if (*hitype == BINRES2)
    {
      for (i=0;i < MCHAN;i++)
	{
	  if (midicurrinstr[i] && (midicurrinstr[i]->opwidth <= kcycleidx))
	    {
	      /* if a noteon ready, code it and break from for loop */

	      *hitype = BINMIDI;
	      *lotype = MIDINOTEON | i;
	      break;
	    }
	  if ((tptr = midicurrnote[i]))
	    {
	      while (tptr && (tptr->opwidth <= kcycleidx))
		{
		  if ((tptr->inwidth <= kcycleidx) && !(tptr->usesinput))
		    {
		      /* if a noteoff ready, code it and break from while */

		      *hitype = BINMIDI;
		      *lotype = MIDINOTEOFF | i;
		      break;
		    }
		  tptr = tptr->next;
		}
	      if (*hitype == BINMIDI) /* if a NOTEOFF found */
		break;
	    }
	}
    }

  if (*hitype == BINRES2)
    return 0;

  return 1;
}

/******************************************************************/
/*                 start new hytpye for access unit               */
/******************************************************************/

void startaccessdata(int hitype)

{
  bitwrite(1, 1);   /* more data */

  switch (hitype) {
  case BINSCORE:
    bitwrite(0, 2);   /* score_line */
    break;
  case BINMIDI:
    bitwrite(1, 2);   /* midi_event */
    break;
  case BINSAMP:
    bitwrite(2, 2);   /* sample */
    break;
  default:
    internalerror("mp4write.c","startaccessdata() -- invalid hitype");
  }
}

/******************************************************************/
/*                 write beginning of access unit                 */
/******************************************************************/

void startaccessunit(float timestamp)

{

  union { unsigned int l; float f ; } u;

  u.f = timestamp;
  bitwrite(u.l, 32);

}

/******************************************************************/
/*                 write end of access unit                       */
/******************************************************************/

void closeaccessunit(int first)

{
  if (first)
    {
      bitwrite(0, 1);   /* no more data */
    }
}

/******************************************************************/
/*                     write samplefiles                          */
/******************************************************************/

void bitaccesssamplewrite(void)

{
  startaccessdata(BINSAMP);
  nextsamplewrite(bitsampleout);
}

/******************************************************************/
/*              write MIDI streaming patch/bank commands          */
/******************************************************************/

void bitstreamingmidiinit(void)

{
  tnode * tptr;
  int first = 0;
  int chan;

  if ( !(tptr = sstrmidi->imidiroot))
    return;

  if (sstrmidi->midinumchan > 15)
    {
      printf("Error: MIDI streaming write only handles 16 MIDI channels.\n");
      printf("       The -mstr file has %i channels and can't be streamed.\n",
	     sstrmidi->midinumchan);
      printf("       To do a non-streaming encode, use the -midi option.\n");
      noerrorplace();
    }

  chan = 0;
  while (tptr != NULL)
    {
      if (!first)
	startaccessunit(0.0F);
      startaccessdata(BINMIDI);

      /* do a bank command if needed */

      if (tptr->vartype > 127)
	{
	  bitmidievent(MIDICONTROL|chan, 0, tptr->vartype/128);
	  startaccessdata(BINMIDI);
	}

      /* always do preset command */
      
      bitmidievent(MIDIPATCH|chan, tptr->vartype % 128, 0);

      /*  latest noteon       latest noteoff                */
     
      midicurrinstr[chan] = midicurrnote[chan] = tptr->down;
      chan++;
      tptr = tptr->next;
      first = 1;
    }

  for (; chan < MCHAN; chan++)
    midicurrinstr[chan] = midicurrnote[chan] = NULL;

  closeaccessunit(1);

}

/******************************************************************/
/*                 write all access units                         */
/******************************************************************/

void bitstreamingwrite(void)

{
  sigsym * oldbitsample, * sptr;
  int first, hitype, lotype, hasend;
  tnode * tempo, * table, * control, * instr;
  tnode * abstempo, * abscontrol;
  tnode * tmapptr;
  char * endval;
  float tval, ktime, fval;
  float scorebase, scoremult, scorebeats;
  int kcycleidx, kbase;
  float endstream = 1E+37F;
  int endkcycle = 0x7FFFFFFF;


  /* save current values, restore before exit */

  /* commands included in -sstr */

  tempo = sstrsasl->temporoot;
  table = sstrsasl->tableroot;
  control = sstrsasl->controlroot;
  instr = sstrsasl->instrroot;
  endval = sstrsasl->endtimeval;

  /* MIDIfile parsing creates controlroot and temporoot elements */

  abstempo = abssasl->temporoot;
  abscontrol = abssasl->controlroot;

  /* initialize relative-to-absolute time conversion */

  tmapptr = tempomap;
  kcycleidx = kbase = 1;
  tval = 60.0F;
  ktime = scoremult = 1.0F/krate;
  scorebase = 0.0F;

  if ((hasend = (sstrsasl->endtimeval || confsasl->endtimeval)))
    {
      if (sstrsasl->endtimeval)
	{
	  endstream = (float)atof(sstrsasl->endtimeval);
	  endkcycle = (int)((endstream-scorebase)/scoremult) + kbase;
	}
      else
	{
	  endstream = (float)atof(confsasl->endtimeval);
	  endkcycle = (int)((endstream-scorebase)/scoremult) + kbase;
	}
    }

  /* sets up midicurrnote and midicurrinstr for noteon/noteoff */

  bitstreamingmidiinit();

  while (morestream(endstream, endkcycle, &hitype, &lotype))
    {
      first = 0;
      scorebeats = scoremult*(kcycleidx - kbase) + scorebase;
      while (morestream(scorebeats, kcycleidx, &hitype, &lotype))
	{
	  if (!first)  
	    {
	      startaccessunit((kcycleidx > 1) ?
			      ktime*(kcycleidx-1.5F) : 0.0F);
	      first = 1;
	    }
	  if (hitype == BINSCORE)
	    {
	      switch(lotype) {
	      case BININSTR:
		startaccessdata(hitype);
		saslinstrwrite(sstrsasl->instrroot);
		sstrsasl->instrroot = sstrsasl->instrroot->next;
		break;
	      case BINCONTROL:
		startaccessdata(hitype);
		saslcontrolwrite(sstrsasl->controlroot);
		sstrsasl->controlroot = sstrsasl->controlroot->next;
		break;
	      case BINTEMPO:
		startaccessdata(hitype);
		sasltempowrite(sstrsasl->temporoot);
		sstrsasl->temporoot = sstrsasl->temporoot->next;
		break;
	      case BINTABLE:
		if (identtoken(sstrsasl->tableroot->down->next->next->next)
		    == S_SAMPLE)
		  {
		    oldbitsample = bitsampleout;
		    sptr = addsamplefile(sstrsasl->tableroot->down
					 ->next->next->next->next->next->val);
		    if (bitsampleout != oldbitsample)
		      bitaccesssamplewrite();
		    startaccessdata(hitype);
		    sasltablewrite(sstrsasl->tableroot, sptr->special);
		  }
		else
		  {
		    startaccessdata(hitype);
		    sasltablewrite(sstrsasl->tableroot, 0);
		  }
		sstrsasl->tableroot = sstrsasl->tableroot->next;
		break;
	      }
	    }
	  if (hitype == BINMIDI)
	    {
	      startaccessdata(hitype);
	      bitmidistream(lotype, kcycleidx);
	    }
	}
      if (first && (kcycleidx == endkcycle) && sstrsasl->endtimeval)
	{
	  startaccessdata(BINSCORE);
	  saslendwrite(sstrsasl->endtimeval);
	  sstrsasl->endtimeval = NULL;
	}
      closeaccessunit(first);

      while (tmapptr && (tmapptr->time <= scorebeats))
	{
	  kbase = kcycleidx;
	  scorebase = scorebeats;
	  if ((fval = (float)atof(tmapptr->val)) < 0.0F)
	    printf("Warning: Encode ignoring negative tempo command.\n\n");
	  else
	    tval = fval;
	  scoremult = 1.666667e-02F*ktime*tval;
	  if (hasend)
	    endkcycle = (int)((endstream-scorebase)/scoremult) + kbase;
	  tmapptr = tmapptr->next;
	}
      kcycleidx++;
    }
  if (sstrsasl->endtimeval)
    {
      startaccessunit(ktime*endkcycle);
      startaccessdata(BINSCORE);
      saslendwrite(sstrsasl->endtimeval);
      closeaccessunit(1);
    }

  /* restoration or sstrsasl and abssasl */

  sstrsasl->endtimeval = endval;
  sstrsasl->temporoot = tempo;
  sstrsasl->tableroot = table;
  sstrsasl->controlroot = control;
  sstrsasl->instrroot = instr;

  abssasl->temporoot = abstempo;
  abssasl->controlroot = abscontrol;

}


/******************************************************************/
/*                        wrapper routine                         */
/******************************************************************/

void mp4write(void)

{

  /* this is where any header bits should go */

  /* bitwrite(0,10);*/ /* magic number */

  /* start of spec-compliant data stream */

  bitwrite(1,1);       /* more data */
  bitwrite(BINORC,3);  /* score file */

  bitsaolwrite();

  if ((bitsymtable != NULL)&& (!bitwritenosymbols) )
    bitsymbolwrite();

  if (midifile != NULL)
    bitmidiwrite();

  if (saslfile != NULL)
    bitscorewrite();

  if (bitsampleout != NULL)
    {
      bitsamplewrite();
    }

  bitwrite(0,1);       /* no more data */

  if (sstrfile || mstrfile)
    bitstreamingwrite();


  bitflush();
  fclose(boutfile);
  boutfile = NULL;
}

