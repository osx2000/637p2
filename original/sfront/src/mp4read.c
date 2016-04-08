
/*
#    Sfront, a SAOL to C translator    
#    This file: Reads binary mp4 files
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
/*                   gets next byte from bitfile                  */
/******************************************************************/

void readnextbyte(void)

{

  if (fread(&bitstoread,sizeof(char),1,bitfile)!=1)
    {
      printf("Error: MP4 input file reading problem.\n");
      noerrorplace();
    }
  bitreadpos = 7;

}

/******************************************************************/
/*           reads numbits LSBs of bitval from bitfile            */
/******************************************************************/


unsigned int readbit(unsigned int numbits)

{
  unsigned int ret = 0;

  while (numbits > 0)
    {
      if (bitreadpos < 0)
	readnextbyte();
      ret |= ((bitstoread & (1 << bitreadpos)) != 0) << (numbits-1);
      numbits--;
      bitreadpos--;
    }
  return ret;

}

/******************************************************************/
/*        checks for next accessunit, sets bitaccesstime          */
/******************************************************************/

int readaccesstime()

{
  int numbits = 32;
  union { unsigned int l; float f ; } u;


  u.l = 0;
  while (numbits > 0)
    {
      while (bitreadpos >= 0)
	{
	  u.l |= ((bitstoread & (1 << bitreadpos)) != 0) << (numbits-1);
	  bitreadpos--;
	  if (!(--numbits))
	    {
	      bitaccesstime = u.f;
	      if (u.f >= 0.0F)
		midictime = (int)(u.f*krate) + 2;
	      else
		midictime = (int)(u.f*krate - 1.0F) + 2;
	      return 1;
	    }
	}
      if (fread(&bitstoread,sizeof(char),1,bitfile)!=1)
	return 0;
      bitreadpos = 7;
    }
  return 0;  /* should never happen */
}


/******************************************************************/
/*           flushes numbits of bitval from bitfile                */
/******************************************************************/


void readflush(unsigned int numbits)

{

  while (numbits > 0)
    {
      if (bitreadpos < 0)
	readnextbyte();
      numbits--;
      bitreadpos--;
    }

}

/******************************************************************/
/*           move past midi_file block                            */
/******************************************************************/

void midifileflush(void)

{

  readflush(8*readbit(32));

}

/******************************************************************/
/*           move past symboltable block                          */
/******************************************************************/

void symboltableflush(void)

{
  unsigned int num;

  num = readbit(16);
  while (num > 0)
    {
      readflush(8*readbit(4));      
      num--;
    }
}

/******************************************************************/
/*           move past orcfile block                          */
/******************************************************************/

void orcfileflush(void)

{
  unsigned int num, token;

  num = readbit(16);
  while (num > 0)
    {
      token = readbit(8);
      switch(token) {
      case S_IDENT:
	readflush(16);
	break;
      case S_NUMBER:
	readflush(32);
	break;
      case S_INTGR:
	readflush(32);
	break;
      case S_STRCONST:
	readflush(8*readbit(8));
	break;
      case S_BYTE:
	readflush(8);
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

void scorelineflush(void)

{
  int hastime, haslabel, scoretype;
  int numpfields, destroy, refers, sym;

  hastime = readbit(1);
  if (hastime)
    readflush(33);
  readflush(1);
  scoretype = readbit(3);
  switch (scoretype) {
  case BININSTR:
    haslabel = readbit(1);
    if (haslabel)
      readflush(16);
    readflush(48);
    numpfields = readbit(8);
    readflush(32*numpfields);
    break;
  case BINCONTROL: 
    haslabel = readbit(1);
    if (haslabel)
      readflush(16);
    readflush(48);
    break;
  case BINTABLE:
    readflush(16);
    destroy = readbit(1);
    if (!destroy)
      {
	sym = readbit(8);
	refers = readbit(1);
	if (refers)
	  readflush(16);
	numpfields = readbit(16);
	if (sym == S_CONCAT)
	  {
	    readflush(32);
	    if ((numpfields - 1) > 0)
	      readflush(16*(numpfields-1));
	  }
	else
	  readflush(32*numpfields);
      }
    break;
  case BINEND:     
    break;
  case BINTEMPO:   
    readflush(32);
    break;
  default:
    printf("Error: Unknown type of score line %i in MP4 file.\n",
	   scoretype);
    noerrorplace();
  }

}

/******************************************************************/
/*           move past score_file block                           */
/******************************************************************/

void scorefileflush(void)

{
  unsigned int numlines;

  numlines = readbit(20);
  while (numlines > 0)
    {
      scorelineflush();
      numlines--;
    }
}


/******************************************************************/
/*           move past sample block                           */
/******************************************************************/

void sampleflush(void)

{

  unsigned int len;

  readflush(16);       /* symbol */
  len = readbit(24);   
  if (readbit(1))      /* srate */
    readflush(17);
  if (readbit(1))      /* loop */
    readflush(48);
  if (readbit(1))      /* base */
    readflush(32);
  readflush((16+readbit(1)*16)*len);

}

/******************************************************************/
/*                    finds next chunk                            */
/******************************************************************/


int readfindchunk (int goal) 

{
  int type;

  while (readbit(1))
    {
      type = readbit(3);
      if (type == goal)
	return 1;
      else
      switch (type) {
      case BINORC:
	orcfileflush();
	break;
      case BINSCORE:
	scorefileflush();
	break;
      case BINMIDI: 
	midifileflush();
	break;
      case BINSAMP:
	sampleflush();
	break;
      case BINSBF:
	printf("Error: SASBF used in MP4 File (not supported by sfront).\n");
	noerrorplace();
	break;
      case BINSYM:   
	symboltableflush();
	break;
      }
    }
  if (goal != BINSSTR)
    return 0;

  return readaccesstime();

}


/*********************************************************/
/*        writes next block of WAV bytes                 */
/*********************************************************/

void putsoundbytes(unsigned char * c, int numbytes)

{
  if ((int)fwrite(c, sizeof(char), numbytes, soundfile) != numbytes)
    {
      printf("Error: During MP4 read (sample file writing error).\n");
      noerrorplace();
    }
}

/*********************************************************/
/*     converts byte stream to an unsigned int          */
/*********************************************************/

void wavefileintwrite(unsigned int val, int numbytes)

{
  unsigned char c[4];

  if (numbytes > 4)
    {
      printf("Error: MP4 corruption (sample).\n");
      noerrorplace();
    }
  switch (numbytes) {
  case 4:
    c[0] = (unsigned char) (val&0x000000FF);
    c[1] = (unsigned char)((val >> 8)&0x000000FF);
    c[2] = (unsigned char)((val >> 16)&0x000000FF);
    c[3] = (unsigned char)((val >> 24)&0x000000FF);
    putsoundbytes(c, 4);
    return;
  case 3:
    c[0] = (unsigned char) (val&0x000000FF);
    c[1] = (unsigned char)((val >> 8)&0x000000FF);
    c[2] = (unsigned char)((val >> 16)&0x000000FF);
    putsoundbytes(c, 3);
    return;
  case 2:
    c[0] = (unsigned char) (val&0x000000FF);
    c[1] = (unsigned char)((val >> 8)&0x000000FF);
    putsoundbytes(c, 2);
    return;
  case 1:
    c[0] = (unsigned char) (val&0x000000FF);
    putsoundbytes(c,1);
    return;
  default:
    return;
  }

}


/******************************************************************/
/*                    read next sample                            */
/******************************************************************/

void readnextsample(void)

{
  union { unsigned int l; float f ; } u;
  unsigned int len;
  int srate = 0;         /* initialization not needed */
  char name[STRSIZE];
  int hasbase, hasloop, unity, fract;
  int start = 0;
  int end = 0;
  int floatsamples = 0;
  float base = 0.0F;


  sprintf(name,"a%i",readbit(16));
  addvsym(&bitsamplein, dupval(name), K_INTERNAL);
  len = readbit(24);   
  if (readbit(1))      /* srate */
    srate = readbit(17);
  if ((hasloop = readbit(1)))      /* loop */
    {
      start = readbit(24);
      end = readbit(24);
    }
  if ((hasbase = readbit(1)))      /* base */
    {
      u.l = readbit(32);
      base = u.f;
    }

  floatsamples = readbit(1);
  start *= (floatsamples + 2);
  end *= (floatsamples + 2);                   
  strcat(name,".wav");
  soundfile = fopen(name,"wb");
  if (soundfile == NULL)
    {
      printf("Error: During MP4 read (while writing sample %s).\n",name);
      noerrorplace();
    }
  putsoundbytes((unsigned char *) "RIFF",4);
  wavefileintwrite(2*len+36 + ((hasloop || hasbase) ? 
			       ((hasloop ? 68 : 44)) : 0), 4);
  putsoundbytes((unsigned char *) "WAVEfmt ",8);
  wavefileintwrite(16,4);
  wavefileintwrite(1,2);     /* PCM  */
  wavefileintwrite(1,2);     /* mono */
  wavefileintwrite(srate,4); /* srate */
  wavefileintwrite(srate*(2 + floatsamples), 4);  /* bytes/sec */
  wavefileintwrite(2,2);     /* block align 2 */
  wavefileintwrite(16+8*floatsamples,2);     /* 2/3 bytes per sample */
  putsoundbytes((unsigned char *) "data",4);
  wavefileintwrite(len*(2+floatsamples),4); /* number of bytes */
  if (floatsamples)
    {
      while (len > 0)
	{
	  u.l = readbit(32);
	  wavefileintwrite((int)(u.f*(pow(2,23) - 1)), 3); 
	  len--;
	}
    }
  else
    {
      while (len > 0)
	{
	  wavefileintwrite(readbit(16),2); 
	  len--;
	}
    }
  if (hasloop || hasbase)
    {
      putsoundbytes((unsigned char *) "smpl",4);
      if (hasloop)
	wavefileintwrite(60,4); /* Size */
      else 
	wavefileintwrite(36,4); /* Size */
      wavefileintwrite(0,4);    /* Manufacturer */
      wavefileintwrite(0,4);    /* Product */
      wavefileintwrite(0,4);    /* Sample Period */
      if (hasbase)
	{
	  base =  (float)(69.0 + 1.731234e+01*log(base*0.00227273));
	  unity = (int)base;
	  fract = (int)((base - (float)unity)*4.295e9);
	}
      else
	{
	  unity = 60;
	  fract = 0;
	}
      wavefileintwrite(unity,4);    /* UnityNote */
      wavefileintwrite(fract,4);    /* PitchFraction */
      wavefileintwrite(0,4);        /* SMPTEFormat */
      wavefileintwrite(0,4);        /* SMPTEOffset */
      wavefileintwrite(hasloop,4);  /* SampleLoops */
      wavefileintwrite(0,4);        /* SamplerData */
      if (hasloop)
	{
	  wavefileintwrite(0,4);        /* ID */
	  wavefileintwrite(0,4);        /* Type */
	  wavefileintwrite(start,4);    /* Start*/
	  wavefileintwrite(end,4);      /* End*/
	  wavefileintwrite(0,4);        /* Fraction */
	  wavefileintwrite(0,4);        /* Playcount*/
	}
    }
  fclose(soundfile);
  
}

/******************************************************************/
/*                    read in sample set                          */
/******************************************************************/

void readsampleset(void)

{

  rewind(bitfile);
  bitstoread = 0;
  bitreadpos = -1;

  /* add code here to read file preables */

  while (readfindchunk(BINSAMP))
    {
      readnextsample();
    }

}

/******************************************************************/
/*                    read in sample set                          */
/******************************************************************/

void readsymboltable(void)

{
  int len, slen, i,j;
  char name[32],num[64];

  rewind(bitfile);
  bitstoread = 0;
  bitreadpos = -1;   

  /* add code here to read preambles */

  i = 0;
  while (readfindchunk(BINSYM))
    {
      len = readbit(16);
      while (len > 0)
	{
	  slen = readbit(4);
	  j = 0;
	  while (slen > 0)
	    {
	      name[j] = (char)readbit(8);
	      j++;
	      slen--;
	    }
	  name[j] = '\0';
	  sprintf(num,"a%i",i);
	  addvsym(&bitsymin, dupval(num), K_INTERNAL);
	  bitsymin->defnode = make_tnode(dupval(name), S_IDENT);
	  i++;
	  len--;
	}
    }

}

/******************************************************************/
/*                    prepare to read file                        */
/******************************************************************/

int readprepare(int goal)


{
  int ret;

  rewind(bitfile);
  bitstoread = 0;
  bitreadpos = -1;   

  /* add code here to read file preambles */

  ret = readfindchunk(goal);

  if (!ret)
    return ret;
  switch (goal) {
  case BINORC:
  case BINSYM:
    bitreadlen = readbit(16);
    return 1;
  case BINSCORE:
    bitreadlen = readbit(20);
    return 1;
  case BINMIDI:
    bitreadlen = readbit(32);
    return 1;
  case BINSSTR:
    return 1;
  }
 return 0;

}

/******************************************************************/
/*                  return next orc token                         */
/******************************************************************/

int orclex(void)

{
  union { unsigned int l; float f ; } u;
  int ret, token, symbol, len, i;
  unsigned int intgr;
  char name[32];


  if (bitreadlen == 0)  /* if no EOO */
    {
      ret = readfindchunk(BINORC);
      if (!ret)
	return ret;
    }

  bitreadlen--;
  token = readbit(8);

  if (token == S_EOO)
    {
      do {
	ret = readfindchunk(BINORC);
	if (!ret)
	  return ret;
	bitreadlen--;
	token = readbit(8);
      } while (token == S_EOO);
    }

  switch (token) {
  case S_IDENT:
    symbol = readbit(16);
    sprintf(name,"a%i",symbol);
    if (getvsym(&bitsamplein,name))
      {
	sprintf(name,"a%i.wav",symbol);
	yylval = make_tnode(dupval(name),S_STRCONST);
	return STRCONST;
      }
    yylval = make_tnode(dupval(name),token);
    return IDENT;
  case S_NUMBER:
    u.l = readbit(32);
    sprintf(name,"%e",u.f);
    yylval = make_tnode(dupval(name), token);
    yylval->rate =  IRATETYPE;
    yylval->vol = CONSTANT;
    return NUMBER;
  case S_INTGR:
    intgr = readbit(32);
    sprintf(name,"%u",intgr);
    yylval = make_tnode(dupval(name), token);
    yylval->rate =  IRATETYPE;
    yylval->res = ASINT;
    yylval->vol = CONSTANT;
    return INTGR;
  case S_BYTE:
    intgr = readbit(8);
    sprintf(name,"%i",intgr);
    yylval = make_tnode(dupval(name), S_INTGR);
    yylval->rate =  IRATETYPE;
    yylval->res = ASINT;
    yylval->vol = CONSTANT;
    return INTGR;
  case S_STRCONST:
    len = readbit(8);
    name[len] = '\0';
    for (i = 0; i < len; i++)
      name[i] = (char)readbit(8);
    yylval = make_tnode(dupval(name), token);
    return STRCONST;
  }

  yylval = make_tnode(dupval(strfortoken(name,token)), tokenmap(token));
  switch (token) {
  case S_AOPCODE:
  case S_ASIG:
    yylval->rate = ARATETYPE;
    break;
  case S_KOPCODE:
  case S_KSIG:
    yylval->rate = KRATETYPE;
    break;
  case S_IOPCODE:
  case S_IVAR:
  case S_TABLE:
  case S_TABLEMAP:
    yylval->rate = IRATETYPE;
    break;
  case S_OPCODE:
  case S_XSIG:
    yylval->rate = XRATETYPE;
    break;
  case S_OUTCHANNELS:
  case S_INCHANNELS:
  case S_INTERP:
    yylval->res = ASINT;
    break;

  /* these case handle lex state machine */

  case S_TEMPLATE:
    if (lexstatemachine == TEMPLATE_REST)
      lexstatemachine = TEMPLATE_ACTIVE;
    break;
  case S_PRESET:
    if (lexstatemachine == TEMPLATE_ACTIVE)
      lexstatemachine = TEMPLATE_PRESET;
    break;
  case S_WITH:
    if (lexstatemachine == TEMPLATE_ACTIVE)
      lexstatemachine = TEMPLATE_WITH;
    break;
  case S_GT:
    if (lexstatemachine == TEMPLATE_WMAPLIST)
      return lexstate_wmap();
    if (lexstatemachine == TEMPLATE_PMAPLIST)
      return lexstate_pmap();
    break;
  case S_LT:
    if (lexstatemachine == TEMPLATE_WITH)
      {	    
	lexstatemachine = TEMPLATE_WMAPLIST;
	return LTT; 
      }	
    if (lexstatemachine == TEMPLATE_PRESET)
      {
	lexstatemachine = TEMPLATE_PMAPLIST;
	return LTT; 
      }
    break;
  }
  return parsetokenmap(token);
}

/******************************************************************/
/*                  parses top of score_line                      */
/******************************************************************/

void binscorelinetop(void)

{
  union { unsigned int l; float f ; } u;
  int haslabel;


  bitscohastime = readbit(1);
  if (bitscohastime)
    {
      readflush(1);   /* use_if_late */
      u.l = readbit(32);
      bitscotime = u.f;
    }
  else
    {      
      bitscotime = bitaccesstime; 
    }
  readflush(1);      /* priority */
  bitscoretype = readbit(3);
  switch (bitscoretype) {
  case BININSTR:
    haslabel = readbit(1);
    if (haslabel)
      {
	bitscolabel = readbit(16);
	bitlinecount = 5;
      }
    else
      {
	bitlinecount = 3;
	bitscolabel = -1;
      }
    break;
  case BINCONTROL:
    haslabel = readbit(1);
    if (haslabel)
      {
	bitscolabel = readbit(16);
	bitlinecount = 5;
      }
    else
      {
	bitlinecount = 4;
	bitscolabel = -1;
      }
    break;
  case BINTABLE:
    bitlinecount = 4;
    break;
  case BINEND:
    bitlinecount = 2;
    break;
  case BINTEMPO:
    bitlinecount = 3;
    break;
  default:
    printf("Error: While reading MP4 file (invalid score event %i).\n",
	   bitscoretype);
    noerrorplace();
  }
}

/******************************************************************/
/*             creates timestamp field for score lines            */
/******************************************************************/

tnode * binscoretimestamp(int type)

{

  char name[64];
  tnode * tptr;

  sprintf(name,"%e",bitscotime);
  tptr = make_tnode(dupval(name), S_NUMBER);
  if (!bitscohastime)
    {
      if (type == BSSTRSCORE)
	{
	  tptr->opwidth = midictime;      
	  if (bitaccesstime > abssasl->compendtime)
	    abssasl->compendtime = bitaccesstime;
	}
      else
	{   
	  printf("Error: While reading MP4 file. Score line in \n");
	  printf("       StructuredAudioSpecificConfig has its \n");
	  printf("       has_time bit cleared.\n");
	  noerrorplace();
	}
    }
  return tptr;

}

/******************************************************************/
/*                  parses bottom of score_line                   */
/******************************************************************/

tnode * binscorelinebottom(int type)

{
  union { unsigned int l; float f ; } u;
  char name[64];
 

  switch (bitscoretype) {
  case BINEND:
    switch (bitlinecount--) {
    case 2:
      return binscoretimestamp(type);
    case 1:
      return make_tnode(dupval("end"), S_END);
    }
    break;
  case BINTEMPO:
    switch (bitlinecount--) {
    case 3:
      return binscoretimestamp(type);
    case 2:
      return make_tnode(dupval("tempo"), S_TEMPO);
    case 1:
      u.l = readbit(32);
      sprintf(name,"%e",u.f);
      return make_tnode(dupval(name), S_NUMBER);
    }
    break;
  case BINCONTROL:
    if (bitscolabel < 0)
      {
	switch (bitlinecount--) {
	case 4:
	  return binscoretimestamp(type);
	case 3:
	  return make_tnode(dupval("control"), S_CONTROL);
	case 2:
	  sprintf(name,"a%i",readbit(16)); 
	  return make_tnode(dupval(name), S_IDENT);
	case 1:
	  u.l = readbit(32);
	  sprintf(name,"%e",u.f);
	  return make_tnode(dupval(name), S_NUMBER);
	}
      }
    else
      {
	switch (bitlinecount--) {
	case 5:
	  return binscoretimestamp(type);
	case 4:
	  sprintf(name,"a%i",bitscolabel); 
	  return make_tnode(dupval(name), S_IDENT);
	case 3:
	  return make_tnode(dupval("control"), S_CONTROL);
	case 2:
	  sprintf(name,"a%i",readbit(16)); 
	  return make_tnode(dupval(name), S_IDENT);
	case 1:
	  u.l = readbit(32);
	  sprintf(name,"%e",u.f);
	  return make_tnode(dupval(name), S_NUMBER);
	}
      }
    break;
  case BININSTR:
    if (bitlinecount == 1)
      {
	if (bitscopfields < 0)
	  {
	    u.l = readbit(32);      /* duration */
	    sprintf(name,"%e",u.f);
	    bitscopfields = readbit(8);
	    if (!bitscopfields)
	      {
		bitlinecount--;
		bitscopfields--;
	      }
	    return make_tnode(dupval(name), S_NUMBER);
	  }
	else
	  {
	    u.l = readbit(32);      /* pfield */
	    sprintf(name,"%e",u.f);
	    bitscopfields--;
	    if (!bitscopfields)
	      {
		bitlinecount--;
		bitscopfields--;
	      }
	    return make_tnode(dupval(name), S_NUMBER);
	  }
      }
    if (bitscolabel < 0)
      {
	switch (bitlinecount--) {
	case 3:
	  return binscoretimestamp(type);
	case 2:
	  sprintf(name,"a%i",readbit(16)); 
	  return make_tnode(dupval(name), S_IDENT);
	}
      }
    else
      {
	switch (bitlinecount--) {
	case 5:
	  sprintf(name,"a%i",bitscolabel); 
	  return make_tnode(dupval(name), S_IDENT);
	case 4:
	  return make_tnode(dupval(":"), S_COL);
	case 3:
	  return binscoretimestamp(type);
	case 2:
	  sprintf(name,"a%i",readbit(16)); 
	  return make_tnode(dupval(name), S_IDENT);
	}
      }
  case BINTABLE:
    if (bitlinecount == 1)
      {
	if (bitscopfields < 0)
	  {
	    if (readbit(1)) /* destroy */
	      {
		bitlinecount--;
		return make_tnode(dupval("destroy"), S_IDENT);
	      }
	    else
	      {
		bittabletoken = readbit(8);
		if (readbit(1))  /* refer_to_sample */
		  {
		    bitsampletoken = readbit(16);
		    bitsamplefirst = 1;
		  }
		else
		  bitsampletoken = -1;
		bitscopfieldsmax = bitscopfields = readbit(16);
		if (!bitscopfields)
		  {
		    bitlinecount--;
		    bitscopfields--;
		  }
		return make_tnode(dupval(strfortoken(name,
						     bittabletoken)), S_IDENT);
	      }
	  }
	else
	  {
	    if ((bittabletoken == S_CONCAT) &&
		(bitscopfieldsmax != bitscopfields))
	      {
		u.l = readbit(16);      /* table token */
		sprintf(name,"a%i", u.l);
		bitscopfields--;
		if (!bitscopfields)
		  {
		    bitlinecount--;
		    bitscopfields--;
		  }
		bitsampletoken = -1;   /* safety for illegal bitstreams */
		return make_tnode(dupval(name), S_IDENT);
	      }
	    else
	      {
		u.l = readbit(32);      /* pfield */
		if ((bitsampletoken == -1)||(bitsamplefirst))
		  sprintf(name,"%e",u.f);
		else
		  sprintf(name,"a%i.wav", bitsampletoken);
		bitscopfields--;
		if (!bitscopfields)
		  {
		    bitlinecount--;
		    bitscopfields--;
		  }
	      }
	    if (bitsampletoken == -1)
	      return make_tnode(dupval(name), S_NUMBER);
	    else
	      {
		if (bitsamplefirst)
		  {
		    bitsamplefirst = 0;
		    return make_tnode(dupval(name), S_NUMBER);
		  }
		else
		  {
		    bitsampletoken = -1;
		    return make_tnode(dupval(name), S_STRCONST);
		  }
	      }
	  }
      }
    switch (bitlinecount--) {
    case 4:
      return binscoretimestamp(type);
    case 3:
      return make_tnode(dupval("table"), S_TABLE);
    case 2:
      sprintf(name,"a%i",readbit(16)); 
      return make_tnode(dupval(name), S_IDENT);
    }
    break;
  }

  return NULL; /* should never execute */

}

/******************************************************************/
/*              return next conf score token                      */
/******************************************************************/

tnode * binconflex(void)

{
  int ret;

  if ((bitreadlen == 0) && (bitlinecount == -1))
    {
      ret = readfindchunk(BINSCORE);
      if (!ret)
	return make_tnode("\n", S_EOF);
      bitlinecount = 0;
      bitreadlen = readbit(20);
    }

  if (!bitlinecount)
    { 
      if (bitreadlen > 0)
	{
	  bitreadlen--;
	  binscorelinetop();
	}
      else
	bitlinecount = -1;
      return make_tnode("\n", S_NEWLINE);
    }

  return binscorelinebottom(BCONFSCORE); 
}


/******************************************************************/
/*             finds next streaming scoreline                     */
/******************************************************************/

int readnextscoreline(void)

{
  int type;

  while (1)
    {
      while (readbit(1))  /* more data here */
	{
	  type = readbit(2);
	  switch (type)
	    {
	    case EVSCORE:
	      return 1;
	    case EVMIDI:
	      midieventread();
	      break;
	    case EVSAMPLE:
	      readnextsample();
	      break;
	    default:
	      printf("Error: Unknown streaming event type in MP4 file.\n");
	      noerrorplace();
	    }
	}
      if (!readaccesstime())
	return 0;
    }

}


/******************************************************************/
/*              return next sstr score token                      */
/******************************************************************/

tnode * binsstrlex(void)

{
  int ret;

  if ((bitlinecount == -1))
    {
      ret = readnextscoreline();
      if (!ret)
	return make_tnode("\n", S_EOF);
      binscorelinetop();
    }

  if (!bitlinecount)
    { 
      bitlinecount = -1;
      return make_tnode("\n", S_NEWLINE);
    }

  return binscorelinebottom(BSSTRSCORE); 
}
