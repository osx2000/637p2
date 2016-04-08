
/*
#    Sfront, a SAOL to C translator    
#    This file: Reads MIDI files
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


/*********************************************************/
/*              data structures mappings                 */
/*                                                       */
/*  for each note:                                       */
/*                                                       */
/*     rate: start time                                  */
/*    width: end time                                    */
/*      res: note number                                 */
/*  vartype: velocity                                    */
/*  opwidth: starttime, in kcycleidx (>= 1)              */
/*  inwidth: endtime, in kcycleidx (>= 1)                */
/*     time: endtime, as float                           */
/*  special: 1 if an all sounds off ending               */
/*                                                       */
/*  for instr:                                           */
/*                                                       */
/*  arrayidx: # of notes                                 */
/*  special: instr id number                             */
/*  vartype: preset                                      */
/*  width:   minimal extended channel                    */  
/*  res:     extended channel number                     */
/*********************************************************/


/*********************************************************/
/*        gets next block of MIDI bytes                  */
/*********************************************************/

void getmidibytes(midata * mdata, int type, 
		  unsigned char * c, int numbytes)

{
  int i;
  

  /* from self-contained .mid files */

  if ((type == FCONFMIDI) || (type == FSSTRMIDI))
    {
      switch (type) {
      case FCONFMIDI:
	if ((int)fread(c, sizeof(char), numbytes, midifile) != numbytes)
	  {
	    printf("Error: Corrupt MIDI file.\n");
	    noerrorplace();
	  }
	break;
      case FSSTRMIDI:
	if ((int)fread(c, sizeof(char), numbytes, mstrfile) != numbytes)
	  {
	    printf("Error: Corrupt MIDI file (streaming).\n");
	    noerrorplace();
	  }
	break;
      }
      mdata->midifsize += numbytes;
      return;
    }

  /* from .mp4 file -- uses readbit() from mp4read.c */

  if (type == BCONFMIDI)
    {
      i = 0;
      while ((i < numbytes)&&(bitreadlen>0))
	{
	  c[i] = (unsigned char)readbit(8);
	  bitreadlen--;
	  i++;
	}
      if ((!bitreadlen) && (i < numbytes))
	{
	  printf("Error: Corrupt midi_file in MP4 file.\n");
	  noerrorplace();
	}
    }

  if (type == BSSTRMIDI)
    {
      i = 0;
      while (i < numbytes)
	c[i++] = (unsigned char)readbit(8);
    }
    
  return;

}

/***********************************************************/
/*  checks byte stream for MIDI cookie --                  */
/*    return 0 for found cookie                            */
/*    return 1 for different cookie                        */
/*    return 2 for EOF                                     */
/***********************************************************/

int miditypecheck(midata * mdata, int type, char * d)

{
  char c[4];
  int i;

  /* from self-contained .mid files */

  if ((type == FCONFMIDI) || (type == FSSTRMIDI))
    {
      switch (type) {
      case FCONFMIDI:
	if (fread(c, sizeof(char), 4, midifile) != 4)
	  return 2;
	break;
      case FSSTRMIDI:
	if (fread(c, sizeof(char), 4, mstrfile) != 4)
	  return 2;
	break;
      }
      mdata->midifsize += 4;
      if (strncmp(c,d,4))
	return 1;
      return 0;
    }

  /* from .mp4 file -- uses readbit() from mp4read.c */

  if (!bitreadlen)
    return 2;

  i = 0;
  while ((i < 4)&&(bitreadlen>0))
    {
      c[i] = (char)readbit(8);
      bitreadlen--;
      i++;
    }
  if (!bitreadlen)
    {
      printf("Error: Corrupt midi_file in MP4 file.\n");
      noerrorplace();
    }
  if (strncmp(c,d,4))
    return 1;
  return 0;


}
  

/*********************************************************/
/*     converts byte stream to an unsigned int          */
/*********************************************************/

unsigned int intval(midata * mdata, int type, int numbytes)

{
  unsigned int ret = 0;
  unsigned char c[4];

  if (numbytes > 4)
    internalerror("readmidi.c","intval()");
  getmidibytes(mdata, type, &c[0],numbytes);
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
  
/*********************************************************/
/*     extracts tick-per-quarter-note from stream        */
/*********************************************************/

unsigned int tickval(midata * mdata, int type)

{
  unsigned int ret = 0;
  unsigned char c[2];

  getmidibytes(mdata, type, &c[0],2);
  if ( c[0] & 0x80)
    {
      printf("Error: MIDI/SMPTE Time Code Not Supported.\n");
      noerrorplace();
    }
  else
    {
      ret  =  (unsigned int)c[1];
      ret |=  (unsigned int)c[0] << 8;
    }
  return ret;

}

/*********************************************************/
/*     flushes unneeded bytes from MIDI stream           */
/*********************************************************/

void flushmidi(midata * mdata, int type, unsigned int len)

{
  unsigned char c;

  while (len > 0)
    {
      getmidibytes(mdata, type, &c,1);
      len--;
    }
  return;

}

/*********************************************************/
/*     prints chars from MIDI stream                     */
/*********************************************************/

void textoutmidi(midata * mdata, int type, 
		 unsigned int len, char * prepend)

{
  unsigned char c;

  if (!midiverbose)
    {
      flushmidi(mdata, type, len);
      return;
    }

  printf("%s ",prepend);
  while (len > 0)
    {
      getmidibytes(mdata, type, &c,1);
      putchar(c);
      len--;
    }
  putchar('\n');
  return;

}

/*********************************************************/
/*        reads variable-length number MIDI number       */
/*********************************************************/

unsigned int getvlength(midata * mdata, int type, 
			 unsigned int * len)

{
  unsigned int ret = 0;
  unsigned char c;

  do {
    getmidibytes(mdata, type, &c, 1);
    (*len)--;
    ret = ret << 7;
    ret |= ((unsigned int)(c&0x7F));
    } 
  while (c&0x80);
  return ret;
    
}

/*********************************************************/
/*        reads next MIDI command                        */
/*********************************************************/

void nextmidicommand(midata * mdata,
		     int type, 
		     unsigned int * len, 
		     unsigned char * command,
		     unsigned char * d0)

{

  unsigned char nextbyte; 

  midictime += getvlength(mdata, type, len);
  getmidibytes(mdata, type, &nextbyte, 1);
  (*len)--;
  if (!(nextbyte & 0x80))
    *d0 = nextbyte;  /* running status, use last command */
  else
    {
      *command = nextbyte;
      if (!(( (*command) & MIDIMASKCOM) == MIDISYSTEM)) /* no sys messages */
	{
	  getmidibytes(mdata, type, d0, 1);
	  (*len)--;
	}
    }
  if (midictime > mdata->midimaxtime)
    mdata->midimaxtime = midictime;

}

/*********************************************************/
/*        creates timestamped * tptr for new command     */
/*********************************************************/

tnode * miditimestampptr(midata * mdata)

{  
  char tname[STRSIZE];
  tnode * tptr;

  /* sets timestamp 
   *
   * if a -mstr or -midi MIDIfile, then set to the score
   * time; -mstr files will have this value overwritten in
   * renumberabs() call.
   *
   * if midi_event in an MP4 file, set to absolute time,
   * and update 
   */
   
  if (mdata->miditicks)
    sprintf(tname,"%e", (float)midictime/mdata->miditicks);
  else
    {
      sprintf(tname,"%e", bitaccesstime);
      if (bitaccesstime > abssasl->compendtime)
	abssasl->compendtime = bitaccesstime;
    }

  tptr = make_tnode(dupval(tname), S_NUMBER);

  if (!(mdata->miditicks))
    tptr->opwidth = (int) midictime;

  return(tptr);

}

/*********************************************************/
/*        reads next MIDI command                        */
/*********************************************************/

void midisyscommands(midata * mdata,
		     int type,
		     sasdata * sdata,
		     unsigned int * len,
		     unsigned char command)


{
  unsigned int flen,i;
  char tname[STRSIZE];
  tnode * tptr;
  
  switch(command) {
  case MIDISYSEX0:
  case MIDISYSEX7:
    flen = getvlength(mdata, type, len);
    flushmidi(mdata, type, flen);
    *len -= flen;
    return;
  case MIDISPP:
    flushmidi(mdata, type, 2);
    *len -= 2;
    return;
  case MIDISSP:
    flushmidi(mdata, type, 1);
    *len -= 1;
    return;
  case MIDITUNE:
  case MIDITIME:
  case MIDISTART:
  case MIDICONT:
  case MIDISTOP:
  case MIDISENSE:
    return;
  case MIDIMETA:
    break;
  default:
    return;
  }
  
  getmidibytes(mdata, type, &command, 1);
  *len -= 1;
  flen = getvlength(mdata, type, len);
  *len -= flen;
  switch (command) {
  case METASEQNUM:
    if (midiverbose)
      printf("Sequence Number %i\n", intval(mdata, type, 2));
    else
      flushmidi(mdata, type, flen);
    break;
  case METATEXT:
    textoutmidi(mdata, type, flen, "Text: ");
    break;
  case METACOPYR:
    textoutmidi(mdata, type, flen,"Copyright: ");
    break;   
  case METASEQNAME:
    textoutmidi(mdata, type, flen,"Sequence Name: ");
    break; 
  case METAINSTR:
    textoutmidi(mdata, type, flen,"Instr Name: ");
    break;   
  case METALYRIC:
    textoutmidi(mdata, type, flen,"Lyric: ");
    break;   
  case METAMARKER:
    textoutmidi(mdata, type, flen,"Marker: ");
    break;  
  case METACUEPT:
    textoutmidi(mdata, type, flen,"Cue Point: ");
    break;   
  case METACHANNEL:
    if (midiverbose)
      {
	i = intval(mdata, type, 1);
	printf("On Ext Channel %i (%i):\n",16*mdata->miditracks+i,i);
      }
    else
      flushmidi(mdata, type, flen);
    break; 
  case METAPORT:
    if (midiverbose)
      printf("On Port %i:\n",intval(mdata, type, 1));
    else
      flushmidi(mdata, type, flen);
    break;    
  case METATEMPO:
    tptr = miditimestampptr(mdata);
    tptr->next = make_tnode("MIDItempo", S_TEMPO);
    sprintf(tname,"%e", 60e6/intval(mdata, type, 3));
    if (midiverbose)
      printf("Tempo in beats/sec: %s\n", tname);
    tptr->next->next = make_tnode(dupval(tname), S_NUMBER);
    parsetempo(sdata, tptr, 1); /* 1 == priority event */
    break;
  case METASMPTE:
    if (midiverbose)
      printf("SMPTE Offset %i %i %i %i %i\n",
	     intval(mdata, type, 1), intval(mdata, type, 1), intval(mdata, type, 1),
	     intval(mdata, type, 1), intval(mdata, type, 1));
    else
      flushmidi(mdata, type, flen);
    break;   
  case METATIMESIG:
    if (midiverbose)
      printf("Time Signature: %i/%i, (MIDI %i %i), \n",
	     intval(mdata, type, 1), intval(mdata, type, 1),
	     intval(mdata, type, 1), intval(mdata, type, 1));
    else
      flushmidi(mdata, type, flen);
    break; 
  default:
    flushmidi(mdata, type, flen);
  }

}

/*********************************************************/
/*             prints out MIDI File Summary              */
/*********************************************************/

void midisummary(midata * mdata)

{
  tnode * tptr;
  tnode * nptr;
  int notes[MNOTE];
  int i;

  printf("Summary of MIDI File\n");
  printf("Beat Length %f, Number of Minimal Extended Channels %i\n",
	 (float)mdata->midimaxtime/mdata->miditicks, mdata->midinumchan);
  tptr = mdata->imidiroot;
  while (tptr != NULL)
    {
      printf("Instr %i, On Preset %i, On Ext Chan %i(%i) Has %i Notes\n",
	     tptr->special, tptr->vartype, tptr->res, 
	     tptr->width, tptr->arrayidx);
      for (i = 0; i < MNOTE; i++)
	notes[i] = 0;
      nptr = tptr->down;
      while (nptr != NULL)
	{
	  notes[nptr->res]++;
	  nptr = nptr->next;
	}
      printf("notes played:");
      for (i = 0; i < MNOTE; i++)
	if (notes[i] > 0)
	  printf(" %i[x%i] ",i, notes[i]);
      printf("\n");
      tptr = tptr->next;
    }
}

/*********************************************************/
/*             process MThd header for MIDI Tracks       */
/*********************************************************/

void midiheader(midata * mdata, int type)

{

  int len, format;
  
  if (miditypecheck(mdata, type, "MThd")>0)
    {
      printf("Error: Corrupted MIDI File.\n");
      noerrorplace();
    }
  len = intval(mdata, type, 4);
  format = intval(mdata, type, 2);
  if (midiverbose)
    printf("MIDI File Format %i\n", format);
  if (format > 1)
    {
      printf("Error: Only MIDI File Format 0 and 1 Supported.\n");
      noerrorplace();
    }
  flushmidi(mdata, type, 2); /* numtracks */
  mdata->miditicks = tickval(mdata, type);
  len -= 6;
  flushmidi(mdata, type, len);

}


/*********************************************************/
/*             change binding of MIDI channel            */
/*********************************************************/

void newpatch(midata * mdata, unsigned int chan, unsigned int patch)

{
  char pname[32];
  sigsym * sptr;

  sprintf(pname,"%i", midibank[chan]+patch);
  sptr = getvsym(&instrpresets, pname);
  if (midiverbose)
    {
      printf("Patch change on channel %i(%i), to preset %i (%i,%i) -- ",
	     mdata->miditracks*16 + chan, chan, midibank[chan]+patch,
	     midibank[chan],patch);
      if (sptr == NULL)
	printf("NOT found in SAOL file\n");
      else
	printf("`%s' in SAOL file\n",sptr->defnode->sptr->val);
    }
  if (sptr != NULL) /* if instr exists for preset */
    {
      midicurrinstr[chan] = make_tnode(pname, S_INSTR);
      midicurrinstr[chan]->sptr = sptr->defnode->sptr;
      midicurrinstr[chan]->vartype = midibank[chan]+patch;
      if (midiext[chan] < 0)
	{
	  midicurrinstr[chan]->width = midiext[chan] = mdata->midinumchan;
	  (mdata->midinumchan)++;
	}
      else
	midicurrinstr[chan]->width = midiext[chan];
      midicurrinstr[chan]->res = mdata->miditracks*16 + chan;
      if (mdata->imidiroot == NULL)
	{
	  mdata->imidiroot = mdata->imiditail = midicurrinstr[chan];
	}
      else
	{
	  mdata->imiditail->next = midicurrinstr[chan];
	  mdata->imiditail->next->special = mdata->imiditail->special + 1;
	  mdata->imiditail = midicurrinstr[chan];
	}
      midicurrnote[chan] = NULL;
      midifirst[chan] = 0;
    }
  else
    midicurrinstr[chan] = NULL;

}

/*********************************************************/
/*             add note to data structures               */
/*********************************************************/

void addnote(midata * mdata, unsigned int chan,
	     unsigned int note, unsigned int vel, int type)

{
  char pname[32];
  sigsym * sptr;
  tnode * tptr;

  if (midicurrinstr[chan] == NULL)
    {
      if (!midifirst[chan])
	{
	  /* try connecting to patch 0 */
	  sprintf(pname,"%i", midibank[chan]);
	  sptr = getvsym(&instrpresets, pname);
	  if (midiverbose)
	    {
	      printf("First note on channel %i(%i), try preset %i (%i,0) -- ",
		     mdata->miditracks*16 + chan, chan, midibank[chan],
		     midibank[chan]);
	      if (sptr == NULL)
		printf("NOT found in SAOL file\n");
	      else
		printf("`%s' in SAOL file\n",sptr->defnode->sptr->val);
	    }
	  if (sptr == NULL)
	    {
	      midifirst[chan] = 1;
	      return;
	    }
	  midicurrinstr[chan] = make_tnode(pname, S_INSTR);
	  midicurrinstr[chan]->sptr = sptr->defnode->sptr;
	  midicurrinstr[chan]->vartype = (int)midibank[chan];
	  midicurrinstr[chan]->width = midiext[chan] = mdata->midinumchan;
	  (mdata->midinumchan)++;
	  midicurrinstr[chan]->res = (mdata->miditracks)*16 + chan;
	  if (mdata->imidiroot == NULL)
	    mdata->imidiroot = mdata->imiditail = midicurrinstr[chan];
	  else
	    {
	      mdata->imiditail->next = midicurrinstr[chan];
	      mdata->imiditail->next->special = mdata->imiditail->special + 1;
	      mdata->imiditail = midicurrinstr[chan];
	    }
	}
      else
	return;
    }

  midicurrinstr[chan]->arrayidx++;
  if (!midifirst[chan])
    {
      if ((type == FCONFMIDI)||(type == BCONFMIDI))
	midicurrinstr[chan]->sptr->midi++;
      else
	midicurrinstr[chan]->sptr->amidi++;
      midifirst[chan] = 1;
    }

  if (midicurrnote[chan]==NULL)
    {
      midicurrnote[chan] = miditimestampptr(mdata);
      midicurrinstr[chan]->down = midicurrnote[chan];
    }
  else
    {
      midicurrnote[chan]->next = miditimestampptr(mdata);
      midicurrnote[chan] = midicurrnote[chan]->next;
    }

  /* miditimestampptr sets opwidth for midi_events */

  if (mdata->miditicks)
    midicurrnote[chan]->rate = (int)midictime;

  midicurrnote[chan]->res = note;
  midicurrnote[chan]->vartype = vel;
  if (midilastnote[chan][note] == NULL)
    midilastnote[chan][note] = midicurrnote[chan];
  else
    {
      tptr = midilastnote[chan][note];
      while (tptr->down != NULL)
	tptr = tptr->down;
      tptr->down = midicurrnote[chan];
    }

}

/*********************************************************/
/*                    end note                           */
/*********************************************************/

void endnote(midata * mdata, unsigned int chan, unsigned int note)

{

  if (midilastnote[chan][note] == NULL)
    return;

  if (mdata->miditicks)
    {
      midilastnote[chan][note]->width = (int)midictime;

      /* for -mstr, overwritten in renumberabs() call */

      midilastnote[chan][note]->time = (float)midictime/mdata->miditicks;
    }
  else
    {
      midilastnote[chan][note]->inwidth = (int)midictime; 
      midilastnote[chan][note]->time = bitaccesstime;      
      if (bitaccesstime > abssasl->compendtime)
	abssasl->compendtime = bitaccesstime;
    }

  midilastnote[chan][note] = midilastnote[chan][note]->down;

}

/*********************************************************/
/*             clean up at end of track                  */
/*********************************************************/

void midicleanup(midata * mdata)

{
  volatile int i, j, k;   /* avoid gcc 4.2.1 optimizer bug */

  for (i = 0; i < MCHAN; i++)
    for (j = 0; j < MNOTE; j++)
      while (midilastnote[i][j] != NULL)
	endnote(mdata, i,j);

  if (midiverbose)
    for (i = 0; i < MCHAN; i++)
      {
	if (midihastouchc[i])
	  printf("Ext chan %i(%i) has %i chan-touch events\n",
		 mdata->miditracks*MCHAN+i,i,midihastouchc[i]);
	if (midihastouchk[i])
	  printf("Ext chan %i(%i) has %i key-touch events\n",
		 mdata->miditracks*MCHAN+i,i,midihastouchk[i]);
	if (midihaswheel[i])
	  printf("Ext chan %i(%i) has %i wheel events\n",
		 mdata->miditracks*MCHAN+i,i,midihaswheel[i]);
	k = 0;
	for (j = 0; j < MNOTE; j++)
	  k |= midihasctrlflag[i][j];
	if (k > 0)
	  {
	    printf("Ext chan %i(%i) uses controllers:", 
		   mdata->miditracks*MCHAN + i, i);
	    for (j = 0; j < MNOTE; j++)
	      if (midihasctrlflag[i][j] > 0)
		printf(" %i",j);
	    printf("\n");
	  }
      }

}

/*********************************************************/
/*                MIDIbend <- val                        */
/*********************************************************/

void pitchwheel(midata * mdata,
		sasdata * sdata,
		unsigned int chan, 
		unsigned int lsb, 
		unsigned int msb)

{
  char tname[STRSIZE];
  int val;
  tnode * tptr;

  if (midicurrinstr[chan] == NULL)
    return;

  val = (msb << 7) | lsb;

  tptr = miditimestampptr(mdata);
  tptr->next = make_tnode("MIDIcontrol", S_CONTROL);
  tptr->next->next = make_tnode("MIDIbend", S_IDENT);
  tptr->next->next->down = midicurrinstr[chan];
  sprintf(tname,"%i.0", val);
  tptr->next->next->next = make_tnode(dupval(tname), S_NUMBER);
  parsecontrol(sdata, tptr, 1);

}

/*********************************************************/
/*                MIDItouch <- val                        */
/*********************************************************/

void keytouch(midata * mdata,
	      sasdata * sdata,
	      unsigned int chan, 
	      unsigned int note, 
	      unsigned int val)

{
  char tname[STRSIZE];
  tnode * tptr;

  if (midicurrinstr[chan] == NULL)
    return;

  tptr = miditimestampptr(mdata);
  tptr->next = make_tnode("MIDIcontrol", S_CONTROL);
  tptr->next->next = make_tnode("MIDItouch", S_IDENT);
  tptr->next->next->arrayidx = note;
  tptr->next->next->down = midicurrinstr[chan];
  sprintf(tname,"%i.0", val);
  tptr->next->next->next = make_tnode(dupval(tname), S_NUMBER);
  parsecontrol(sdata, tptr, 1);

}

/*********************************************************/
/*                MIDICtrl[num] <- val                   */
/*********************************************************/

void controller(midata * mdata,
		sasdata * sdata,
		unsigned int chan, 
		unsigned int num, 
		unsigned int val)

{
  char tname[STRSIZE];
  tnode * tptr;
  int i;

  if (midiverbose)
    midihasctrlflag[chan][num]++;
  if (num == 0)                                 /* bank select */
    midibank[chan] = val;
  if ((num == 120)||(num == 123))               /* all notes/sounds off */
    for (i = 0; i < 128; i++)
      while (midilastnote[chan][i] != NULL)
	{
	  midilastnote[chan][i]->special = (num == 120);
	  midiallsoundsoff |= (num == 120);
	  endnote(mdata, chan, i);
	}

  if (midicurrinstr[chan] == NULL)
    return;

  tptr = miditimestampptr(mdata);
  tptr->next = make_tnode("MIDIcontrol", S_CONTROL);
  tptr->next->next = make_tnode("MIDIctrl", S_IDENT);
  tptr->next->next->arrayidx = num;
  tptr->next->next->down = midicurrinstr[chan];
  sprintf(tname,"%i.0", val);
  tptr->next->next->next = make_tnode(dupval(tname), S_NUMBER);
  parsecontrol(sdata, tptr, 1);

}

/*********************************************************/
/*        zeros out transient MIDI data structures       */
/*********************************************************/

void cleanmidivars(void)

{
  int i, j;

  midictime = 0;
  for (i=0;i<MCHAN;i++)
    {
      midicurrnote[i] = NULL;
      midicurrinstr[i] = NULL;
      midibank[i] = 0;
      midifirst[i] = 0;
      if (midiverbose)
	{
	  midihastouchc[i] = 0;
	  midihastouchk[i] = 0;
	  midihaswheel[i] = 0;
	  for(j=0;j<MNOTE;j++)
	    midihasctrlflag[i][j]=0;
	}
      midiext[i] = -1;
      for (j=0;j<MNOTE;j++)
	midilastnote[i][j] = NULL;
    }
}

/*********************************************************/
/*             process -midi: called in postparse.c       */
/*********************************************************/

void readmidi(midata * mdata, sasdata * sdata, int type)

{

  unsigned int len;       /* bytes remaining in chunk */
  unsigned char command;   /* current MIDI command */  
  unsigned char d0, d1;    /* data bytes of MIDI command */
  int cookie;

  /* later change to use type */

  if (bitfile && (!readprepare(BINMIDI)))
    {
      cleanmidivars();   /* prepare for streaming midi reading */
      return;
    }

  midiheader(mdata, type);
  while ( (cookie = miditypecheck(mdata, type, "MTrk")) != 2)
    {
      if (midiverbose)
	printf("Track Chunk %i\n",mdata->miditracks);
      len = intval(mdata, type, 4);
      if (cookie == 1)
	{
	  flushmidi(mdata, type, len);
	  continue;
	}
      cleanmidivars();
      while (len > 0)
	{
	  nextmidicommand(mdata,type,&len,&command,&d0);
	  switch ((command & MIDIMASKCOM)) {
	  case MIDINOTEOFF:
	    flushmidi(mdata, type, 1);
	    len--;
	    endnote(mdata, command & MIDIMASKCHAN, d0);
	    break;
	  case MIDINOTEON:
	    getmidibytes(mdata, type, &d1, 1); 
	    len--;
	    if (d1 == 0)
	      endnote(mdata, command & MIDIMASKCHAN, d0);
	    else
	      addnote(mdata,command & MIDIMASKCHAN, d0, d1, type);
	    break;
	  case MIDICHTOUCH:
	    if (midiverbose)
	      midihastouchc[command & MIDIMASKCHAN]++;
	    keytouch(mdata, sdata, command & MIDIMASKCHAN, -1, d0);
	    break;
	  case MIDIKEYTOUCH:
	    if (midiverbose)
	      midihastouchk[command & MIDIMASKCHAN]++;
	    getmidibytes(mdata, type, &d1, 1); 
	    len--;
	    keytouch(mdata, sdata, command & MIDIMASKCHAN, d0, d1);
	    break;
	  case MIDIWHEEL:
	    if (midiverbose)
	      midihaswheel[command & MIDIMASKCHAN]++;
	    getmidibytes(mdata, type, &d1, 1); 
	    len--;
	    pitchwheel(mdata, sdata, command & MIDIMASKCHAN, d0, d1);
	    break;
	  case MIDICONTROL:
	    getmidibytes(mdata, type, &d1, 1); 
	    len--;
	    controller(mdata, sdata, command & MIDIMASKCHAN, d0, d1);
	    break;
	  case MIDIPATCH:
	    newpatch(mdata, command & MIDIMASKCHAN, d0);
	    break;
	  case MIDISYSTEM:
	    midisyscommands(mdata, type, sdata, &len,command);
	    break;
	  default:
	    printf("Error: MIDI file corruption.\n");
	    noerrorplace();
	  }
	}
      midicleanup(mdata);
      (mdata->miditracks)++;
    }
  if (midiverbose)
    midisummary(mdata);

  cleanmidivars();   /* prepare for streaming midi reading */
  
}

/*********************************************************/
/* declares MIDI instr variables (called in writepre.c)  */
/*********************************************************/

void initmidiinstr(int type, sigsym * sptr, int * totlines)

{
  char * prefix;
  char * val;
  tnode * tptr;
  int i;

  if (type == RELTSTAMP)
    {
      tptr = confmidi->imidiroot;
      prefix = "m";
      i = sptr->midi;
    }
  else
    {
      tptr = sstrmidi->imidiroot;
      prefix = "ma";
      i = sptr->amidi;
    }

  while ((tptr != NULL) && ( i > 0))
    {
      if (tptr->sptr != sptr)
	{
	  tptr = tptr->next;
	  continue;
	}

      fprintf(outfile,"instr_line %s%i_%s[%i];\n", prefix,
	      tptr->special, sptr->val, tptr->arrayidx);
      (*totlines) +=  tptr->arrayidx;
      
      val = dupunderscore(sptr->val);
      fprintf(outfile,
	      "instr_line * %s%i_%sfirst;\n", prefix,tptr->special,val);
      fprintf(outfile,"instr_line * %s%i_%slast;\n",prefix, tptr->special,val);
      fprintf(outfile,"instr_line * %s%i_%send;\n\n",prefix,tptr->special,val);
      free(val);
      i--;
      tptr = tptr->next;
    }
}


/*********************************************************/
/*   content_init() assignments for MIDI instr variables */
/*********************************************************/

void initmidiinstrassign(int type, sigsym * sptr)

{
  char * prefix;
  char * val;
  tnode * tptr;
  int i;

  if (type == RELTSTAMP)
    {
      tptr = confmidi->imidiroot;
      prefix = "m";
      i = sptr->midi;
    }
  else
    {
      tptr = sstrmidi->imidiroot;
      prefix = "ma";
      i = sptr->amidi;
    }

  while ((tptr != NULL) && ( i > 0) )
    {
      if (tptr->sptr != sptr)
	{
	  tptr = tptr->next;
	  continue;
	}

      fprintf(outfile,"  memcpy(EV(%s%i_%s), %s%i_%s_init, sizeof EV(%s%i_%s));\n", 
	      prefix, tptr->special, sptr->val, prefix, tptr->special, sptr->val,
	      prefix, tptr->special, sptr->val);

      val = dupunderscore(sptr->val);
      fprintf(outfile, "  EV(%s%i_%sfirst) = &EV(%s%i_%s[0]);\n", prefix,
	      tptr->special,val,prefix,tptr->special,sptr->val);
      fprintf(outfile,"  EV(%s%i_%slast) = &EV(%s%i_%s[0]);\n",
	      prefix, tptr->special,val, prefix, tptr->special,sptr->val);
      fprintf(outfile, "  EV(%s%i_%send) = &EV(%s%i_%s[%i]);\n\n", prefix,
	      tptr->special,val,prefix,tptr->special,sptr->val,
	      tptr->arrayidx - 1);
      free(val);
      i--;
      tptr = tptr->next;
    }
}

/*********************************************************/
/* declare and init true constant vars for MIDI instrs  */
/*********************************************************/

void initmidiinstrconstant(int type, sigsym * sptr)

{
  char * prefix;
  tnode * tptr;
  tnode * nptr;
  int i, j, incr;

  if (type == RELTSTAMP)
    {
      tptr = confmidi->imidiroot;
      prefix = "m";
      i = sptr->midi;
      incr = sstrmidi->midinumchan;
    }
  else
    {
      tptr = sstrmidi->imidiroot;
      prefix = "ma";
      i = sptr->amidi;
      incr = 0;
    }

  while ((tptr != NULL) && ( i > 0) )
    {
      if (tptr->sptr != sptr)
	{
	  tptr = tptr->next;
	  continue;
	}

      fprintf(outfile,"instr_line %s%i_%s_init[%i] = {\n", prefix,
	      tptr->special, sptr->val, tptr->arrayidx);
      nptr = tptr->down;
      while (nptr != NULL)
	{
	  if (nptr != tptr->down)
	    fprintf(outfile,",\n");
	  fprintf(outfile,"{");
	  /* float starttime, float endtime, float startabs, float endabs */

	  if (type == RELTSTAMP)
	    {
	      fprintf(outfile, " %sF, %1.6eF, MAXENDTIME, MAXENDTIME, ",
		      nptr->val, nptr->time);
	    }
	  else
	    {
	      fprintf(outfile, " MAXENDTIME, MAXENDTIME, %sF, %1.6eF, ",
		      nptr->val, nptr->time);
	    }

	  /* float abstime, float time, float itime, float sdur */
	      
	  fprintf(outfile, " 0.0F, 0.0F, 0.0F, -1.0F,"); 

	  /* int kbirth, int released, int turnoff, int noteon  */
	  /* int notestate, int launch, int numchan, int preset */
	  /* int notenum, int label                             */

	  fprintf(outfile, " 0, 0, 0, 1, 0, 0, %i, %i, %i, %i,", 
		  tptr->width + incr, /* numchan: minimal extended channel */
		  tptr->vartype,      /* preset */
		  nptr->res,          /* notenum: note number */
		  nptr->special       /* label: 1 if all-sounds note off */ 
		  );         

	  /* float p[] */

	  if ((j = numpfields))
	    fprintf(outfile," {");

	  if (j > 0)
	    {
	      fprintf(outfile," %i.0F ",nptr->res);  /* note number */
	      if (!(--j))
		fprintf(outfile,"},");
	      else
		fprintf(outfile,",");
	    }
	  if (j > 0)
	    {
	      fprintf(outfile," %i.0F ",nptr->vartype); /* velocity */
	      if (!(--j))
		fprintf(outfile,"},");
	      else
		fprintf(outfile,",");
	    }
	  while (j > 0)
	    {
	      fprintf(outfile," 0.0F ");
	      if (!(--j))
		fprintf(outfile,"},");
	      else
		fprintf(outfile,",");
	    }
	  
	  /* struct ninstr_types * nstate */

	  fprintf(outfile," NULL ");
	  fprintf(outfile,"}");
	  nptr = nptr->next;
	}
      fprintf(outfile,"};\n\n");
      i--;
      tptr = tptr->next;
    }
  
}

/*********************************************************/
/*             regenerates MIDI file                     */
/*********************************************************/

void binmidiwrite(int type)

{
  unsigned char c;

  if (type == FCONFMIDI)
    {
      rewind(midifile);
      while (fread(&c, sizeof(char), 1, midifile) == 1)
	fwrite(&c, sizeof(char), 1, midoutfile);
      return;
    }

  if (type == FSSTRMIDI)
    {
      rewind(mstrfile);
      while (fread(&c, sizeof(char), 1, mstrfile) == 1)
	fwrite(&c, sizeof(char), 1, midoutfile);
      return;
    }

  if (bitfile && (!readprepare(BINMIDI)))
    return;

  while (bitreadlen > 0)
    {
      c = (unsigned char)readbit(8);
      fwrite(&c, sizeof(char), 1, midoutfile);
      bitreadlen--;
    }

}


/*********************************************************/
/*        reads start of the next midi_event             */
/*********************************************************/

void nextmidievent(unsigned int * len, 
		   unsigned char * command,
		   unsigned char * d0)

{

  unsigned char nextbyte; 

  nextbyte = (unsigned char)readbit(8); 
  (*len)--;
  if (!(nextbyte & 0x80))
    {
      *command = midirunstat;
      *d0 = nextbyte;  
    }
  else
    {
      midirunstat = (*command) = nextbyte;
      if (!(( (*command) & MIDIMASKCOM) == MIDISYSTEM)) /* no sys messages */
	{
	  *d0 = (unsigned char)readbit(8); 
	  (*len)--;
	}
    }

}




/*********************************************************/
/*   main function for reading streaming midi_event      */
/*********************************************************/

void midieventread(void)

{
  unsigned int len;       /* bytes remaining in midi_event */
  unsigned char command;   /* current MIDI command */  
  unsigned char d0, d1;    /* data bytes of MIDI command */

  d0 = d1 = command = 0;
  len = (unsigned int) readbit(24);
  while (len > 0)
    {
      nextmidievent(&len, &command, &d0);
      switch ((command & MIDIMASKCOM)) {
      case MIDINOTEOFF:
	if (len--)
	  readflush(8); /* note-off velocity */
	endnote(sstrmidi, command & MIDIMASKCHAN, d0);
	break;
      case MIDINOTEON:
	if (len--)
	  d1 = (unsigned char)readbit(8); 
	if (d1 == 0)
	  endnote(sstrmidi, command & MIDIMASKCHAN, d0);
	else
	  addnote(sstrmidi, command & MIDIMASKCHAN, d0, d1, BSSTRMIDI);
	break;
      case MIDICHTOUCH:
	if (midiverbose)
	  midihastouchc[command & MIDIMASKCHAN]++;
	keytouch(sstrmidi, abssasl, command & MIDIMASKCHAN, -1, d0); 
	break;
      case MIDIKEYTOUCH:
	if (midiverbose)
	  midihastouchk[command & MIDIMASKCHAN]++;
	if (len--)
	  d1 = (unsigned char)readbit(8); 
	keytouch(sstrmidi, abssasl, command & MIDIMASKCHAN, d0, d1); 
	break;
      case MIDIWHEEL:
	if (midiverbose)
	  midihaswheel[command & MIDIMASKCHAN]++;
	if (len--)
	  d1 = (unsigned char)readbit(8); 
	pitchwheel(sstrmidi, abssasl, command & MIDIMASKCHAN, d0, d1); 
	break;
      case MIDICONTROL:
	if (len--)
	  d1 = (unsigned char)readbit(8); 
	controller(sstrmidi, abssasl, command & MIDIMASKCHAN, d0, d1); 
	break;
      case MIDIPATCH:
	newpatch(sstrmidi, command & MIDIMASKCHAN, d0); 
	break;
      case MIDISYSTEM:
	midisyscommands(sstrmidi, BSSTRMIDI, abssasl, &len, command);
	break;
      default:
	printf("Error: MIDI file corruption.\n");
	noerrorplace();
      }
    }

  return;


}
