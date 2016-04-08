

/*
#    Sfront, a SAOL to C translator    
#    This file: ALSA MIDI (Card 0, Device 0) control driver for sfront
#
#    Can anyone think of a way of specifying the card/device number?
#
#    This driver is a small modification of the linmidi.c driver.
#    The modifications were made by Steven Pickles (pix)
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


#include <sys/asoundlib.h>

/****************************************************************/
/****************************************************************/
/*           linux /dev/mid00 control driver for sfront         */
/*     alsamidi doesn't support root mode in -timesync yet      */ 
/****************************************************************/

#define CSYSI_BUFFSIZE  1024
#define CSYSI_SYSEX_EOX 0xF7

/* set CSYSI_DELAY to 0 to wait for partially completed MIDI commands */
/* waiting for commands decreases variance of the latency, at the     */
/* expense of losing computation cycles                               */

#define CSYSI_DELAY 1

void *csysi_handle;
int csysi_card = 0;
int csysi_device = 0;
int csysi_error = 0;

unsigned char csysi_hold[CSYSI_BUFFSIZE];
unsigned char csysi_data[CSYSI_BUFFSIZE];
int csysi_len;
int csysi_cnt;
unsigned char csysi_cmd;
unsigned char csysi_num;
unsigned short csysi_extchan;
unsigned char csysi_ndata = 0xFF;

/****************************************************************/
/*             initialization routine for control               */
/****************************************************************/

int csys_setup(void)
{

  if ((csysi_error = snd_rawmidi_open(&csysi_handle, csysi_card, csysi_device, SND_RAWMIDI_OPEN_INPUT)) < 0) {
    fprintf(stderr, "ALSAMIDI: open failed: %s\n", snd_strerror(csysi_error));
    return CSYS_ERROR;
  }

  if ((csysi_error = snd_rawmidi_block_mode(csysi_handle, 0)) < 0) {
    fprintf(stderr, "ALSAMIDI: disabling block mode failed: %s\n", snd_strerror(csysi_error));
    return CSYS_ERROR;
  }

  return CSYS_DONE;
}

/****************************************************************/
/*             polling routine for new data                     */
/****************************************************************/

int csys_newdata(void)
     
{
  int i = 0;
  int len;

  len = snd_rawmidi_read(csysi_handle, csysi_hold, CSYSI_BUFFSIZE); 
  if (len < 0)
    return CSYS_NONE;

  csysi_len = csysi_cnt = 0;
  while (i < len)
    {
      if (csysi_hold[i] <= CSYSI_SYSEX_EOX)
	csysi_data[csysi_len++] = csysi_hold[i];
      i++;
    }

  if (!csysi_len)
    return CSYS_NONE;
  return CSYS_MIDIEVENTS;
  
}

/****************************************************************/
/*             gets one byte from MIDI stream                   */
/****************************************************************/

unsigned char csysi_getbyte(void)

{
  unsigned char d;

  while ( !(
	    (snd_rawmidi_read(csysi_handle, &d, 1) == 1) &&
	    (d <= CSYSI_SYSEX_EOX)) );
  return d;
}

/****************************************************************/
/*             flushes MIDI system messages                     */
/****************************************************************/

int csysi_sysflush(unsigned short type)

{
  unsigned char byte;

  if ((type == 6) || /* one-byte messages */
      (type == 1) || /* undefined messages */
      (type == 4) ||
      (type == 5))
    { 
      if (csysi_cnt == csysi_len)
	return CSYS_NONE;
      else
	return CSYS_MIDIEVENTS;
    }
  
  if (type == 3) /* song select -- 1 data byte */
    {
      if (csysi_cnt == csysi_len)
	csysi_getbyte();
      else
	csysi_cnt++;
      if (csysi_cnt == csysi_len)
	return CSYS_NONE;
      else
	return CSYS_MIDIEVENTS;
    }
  
  if (type == 2) /* song pointer -- 2 data bytes */
    {
      if (csysi_cnt < csysi_len)
	csysi_cnt++;
      else
	csysi_getbyte();
      if (csysi_cnt < csysi_len)
	csysi_cnt++;
      else
	csysi_getbyte();
      if (csysi_cnt == csysi_len)
	return CSYS_NONE;
      else
	return CSYS_MIDIEVENTS;
    }

  if (type == 0) 
    {
      if (csysi_cnt < csysi_len)
	byte = csysi_data[csysi_cnt++];
      else
	byte = csysi_getbyte();
      while (byte < CSYS_MIDI_NOTEOFF)
	if (csysi_cnt < csysi_len)
	  byte = csysi_data[csysi_cnt++];
	else
	  byte = csysi_getbyte();
      if (byte != CSYSI_SYSEX_EOX) /* non-compliant MIDI */
	{
	  if ((byte&0xF0) != 0xF0)
	    {
	      csysi_cmd = byte&0xF0;
	      csysi_extchan = byte&0x0F;
	    }
	  switch (byte&0xF0) {
	  case CSYS_MIDI_NOTEOFF:
	  case CSYS_MIDI_NOTEON:
	  case CSYS_MIDI_PTOUCH:
	  case CSYS_MIDI_WHEEL:
	  case CSYS_MIDI_CC:
	    csysi_num = 2;
	    break;
	  case CSYS_MIDI_PROGRAM:
	  case CSYS_MIDI_CTOUCH:
	    csysi_num = 1;
	    break;
	  case 0xF0: 
	    if ((byte&0x0F)==2) /* song pointer -- 2 data bytes */
	      {
		if (csysi_cnt < csysi_len)
		  csysi_cnt++;
		else
		  csysi_getbyte();
		if (csysi_cnt < csysi_len)
		  csysi_cnt++;
		else
		  csysi_getbyte();
	      }
	    if ((byte&0x0F)==3) /* song select -- 1 data byte */
	      {
		if (csysi_cnt < csysi_len)
		  csysi_cnt++;
		else
		  csysi_getbyte();
	      }
	    break;
	  }
	}
    }

  /* outside of if {} to catch errant F7 bytes */

  if (csysi_cnt == csysi_len)
    return CSYS_NONE;
  else
    return CSYS_MIDIEVENTS;

}


/****************************************************************/
/*                 processes a MIDI event                       */
/****************************************************************/

int csys_midievent(unsigned char * cmd,   unsigned char * ndata, 
	           unsigned char * vdata, unsigned short * extchan,
		   float * fval)

{
  unsigned char overflow[2], oval;
  int len, tot, idx;

  if (csysi_data[csysi_cnt] > 127)    /* a command byte */
    {
      *cmd = 0xF0 & csysi_data[csysi_cnt];
      *extchan = 0x0F & csysi_data[csysi_cnt];
      if (*cmd != 0xF0)
	{
	  csysi_cmd = *cmd;
	  csysi_extchan = *extchan;
	}
      csysi_cnt++;
      switch (*cmd) {
      case CSYS_MIDI_NOTEOFF:
      case CSYS_MIDI_NOTEON:
      case CSYS_MIDI_PTOUCH:
      case CSYS_MIDI_WHEEL:
      case CSYS_MIDI_CC:
	csysi_num = 2;
	if (CSYSI_DELAY && ((csysi_cnt + 1) == csysi_len)) /* delay cmd */
	  {
	    csysi_ndata = csysi_data[csysi_cnt];
	    *cmd = CSYS_MIDI_NOOP;
	    return CSYS_NONE;
	  }
	break;
      case CSYS_MIDI_PROGRAM:
      case CSYS_MIDI_CTOUCH:
	csysi_num = 1;
	break;
      case 0xF0: 
	*cmd = CSYS_MIDI_NOOP;
	return csysi_sysflush(*extchan);
	break;
      }
      if (CSYSI_DELAY && (csysi_cnt == csysi_len)) /* delay cmd */
	{
	  *cmd = CSYS_MIDI_NOOP;
	  return CSYS_NONE;
	}
    }
  else  /* running status or a delayed MIDI command */
    {
      *cmd = csysi_cmd;
      *extchan = csysi_extchan;
      if (CSYSI_DELAY && (csysi_ndata != 0xFF)) /* finish delayed cmd */
	{
	  *ndata = csysi_ndata;
	  csysi_ndata = 0xFF;
	  *vdata = csysi_data[csysi_cnt++];
	  if (csysi_cnt == csysi_len)
	    return CSYS_NONE;
	  else
	    return CSYS_MIDIEVENTS;
	}
      if (CSYSI_DELAY && (csysi_num == 2) && /* (further) delay cmd */
	  (csysi_cnt + 1 == csysi_len))
	{
	  csysi_ndata = csysi_data[csysi_cnt];
	  *cmd = CSYS_MIDI_NOOP;
	  return CSYS_NONE;
	}
    }

  /* do complete commands and finish some types of delayed commands */

  if (csysi_cnt + csysi_num <= csysi_len)
    {
      *ndata = csysi_data[csysi_cnt++];
      if (csysi_num == 2)
	*vdata = csysi_data[csysi_cnt++];
      if (csysi_cnt == csysi_len)
	return CSYS_NONE;
      else
	return CSYS_MIDIEVENTS;
    }

  /* should never execute if CSYSI_DELAY is 1 */

  tot = csysi_cnt + csysi_num - csysi_len;
  idx = 0;
  while (tot > 0)
    {
      if ( 
	  //(read(csysi_midi, &oval, 1) == 1) &&
	  (snd_rawmidi_read(csysi_handle, &oval, 1) == 1) &&
	   (oval <= CSYSI_SYSEX_EOX) )
	{
	  tot--;
	  overflow[idx++] = oval;
	}
    }

  if (csysi_num == 1) 
    {
      *ndata = overflow[0];
      return CSYS_NONE;
    }
  if (csysi_cnt + 1 == csysi_len)
    {
      *ndata = csysi_data[csysi_cnt++];
      *vdata = overflow[0];
    }
  else
    {
      *ndata = overflow[0];
      *vdata = overflow[1];
    }
  return CSYS_NONE;

  
}

/****************************************************************/
/*                  closing routine for control                 */
/****************************************************************/

void csys_shutdown(void)
     
{
  snd_rawmidi_close(csysi_handle);
}




