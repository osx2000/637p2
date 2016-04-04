
/*
#    Sfront, a SAOL to C translator    
#    This file: Merged linux/freebsd MIDI Input control driver 
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



#include <fcntl.h>
#include <signal.h>  
#include <sys/time.h>  

/****************************************************************/
/****************************************************************/
/*               MIDI Input control driver for sfront           */ 
/****************************************************************/

#ifndef CSYSI_MIDIDEV
#define CSYSI_MIDIDEV "/dev/midi00"
#endif

#define CSYSI_BUFFSIZE    1024
#define CSYSI_SYSEX_EOX   0xF7

/* set CSYSI_DELAY to 0 to wait for partially completed MIDI commands */
/* waiting for commands decreases variance of the latency, at the     */
/* expense of losing computation cycles                               */

#define CSYSI_DELAY 1

/* variables for SIGALRM for MIDI overrun */

/* period for interrupt: 320us per MIDI byte @ 128 bytes, minus safety zone */

#define CSYSI_ALARMPERIOD  40000

/* maximum number of I/O retries before termination */

#define CSYSI_MAXRETRY 256

sigset_t         csysi_overrun_mask;    /* for masking off overrun interrupt */
struct sigaction csysi_overrun_action;  /* for setting up overrun interrupt  */
struct itimerval csysi_overrun_timer;   /* for setting up overrun timer      */

/* flag for new note on/off */

int csysi_newnote = 0;

/* MIDI parsing state variables */

int csysi_midi = 0;

unsigned char csysi_hold[CSYSI_BUFFSIZE];
int csysi_holdidx = 0;

unsigned char csysi_data[CSYSI_BUFFSIZE];
int csysi_len;
int csysi_cnt;
unsigned char csysi_cmd;
unsigned char csysi_num;
unsigned short csysi_extchan;
unsigned char csysi_ndata = 0xFF;

/****************************************************************/
/*          generic error-checking wrappers                     */
/****************************************************************/

#define  CSYSI_ERROR_RETURN(x) do {\
      fprintf(stderr, "  Error: %s.\n", x);\
      fprintf(stderr, "  Errno Message: %s\n\n", strerror(errno));\
      return CSYS_ERROR; } while (0)

#define  CSYSI_ERROR_TERMINATE(x) do {\
      fprintf(stderr, "  Runtime Errno Message: %s\n", strerror(errno));\
      epr(0,NULL,NULL, "Soundcard error -- " x );}  while (0)


/****************************************************************/
/*         signal handler to catch MIDI buffer overruns      */
/****************************************************************/

void csysi_overrun_handler(int signum) 

{   
  int retry = 0;
  int len;

  while ((len = read(csysi_midi, &(csysi_hold[csysi_holdidx]), 
	     CSYSI_BUFFSIZE-csysi_holdidx)) < 0)
    {
      if (++retry > CSYSI_MAXRETRY)
	CSYSI_ERROR_TERMINATE("Too many I/O retries -- csysi_overrun_handler");

      if (errno == EAGAIN)      /* no data ready */
	break;
      if (errno == EINTR)       /* interrupted, try again */
	continue;

      CSYSI_ERROR_TERMINATE("Couldn't read MIDI device");
    }

  if (len > 0)
    {
      if ((csysi_holdidx += len) >= CSYSI_BUFFSIZE)
	fprintf(stderr, "  Warning: MIDI overrun, data lost\n\n");
    }

  /* reset timer */

  if (setitimer(ITIMER_REAL, &csysi_overrun_timer, NULL) < 0)
    CSYSI_ERROR_TERMINATE("Couldn't reset ITIMER_REAL timer");
}


/****************************************************************/
/*             initialization routine for control               */
/****************************************************************/

int csys_setup(void)
     
{

  csysi_midi = open(CSYSI_MIDIDEV, O_RDONLY|O_NONBLOCK);

  if (csysi_midi == -1)
    CSYSI_ERROR_RETURN("Can't open MIDI input device");

  /* set up mask for overrun timer */
  
  if (sigemptyset(&csysi_overrun_mask) < 0)
    CSYSI_ERROR_RETURN("Couldn't run sigemptyset(overrun) OS call");

  if (sigaddset(&csysi_overrun_mask, SIGALRM) < 0)
    CSYSI_ERROR_RETURN("Couldn't run sigaddset(overrun) OS call");

  /* set up signal handler for overrun timer */
  
  if (sigemptyset(&csysi_overrun_action.sa_mask) < 0)
    CSYSI_ERROR_RETURN("Couldn't run sigemptyset(oaction) OS call");

  csysi_overrun_action.sa_flags = SA_RESTART;
  csysi_overrun_action.sa_handler = csysi_overrun_handler;
  
  if (sigaction(SIGALRM, &csysi_overrun_action, NULL) < 0)
    CSYSI_ERROR_RETURN("Couldn't set up SIGALRM signal handler");

  /* set up timer and arm */

  csysi_overrun_timer.it_value.tv_sec = 0;
  csysi_overrun_timer.it_value.tv_usec = CSYSI_ALARMPERIOD;
  csysi_overrun_timer.it_interval.tv_sec = 0;
  csysi_overrun_timer.it_interval.tv_usec = 0;

  if (setitimer(ITIMER_REAL, &csysi_overrun_timer, NULL) < 0)
    CSYSI_ERROR_RETURN("Couldn't set up ITIMER_REAL timer");

  return CSYS_DONE;
}

/****************************************************************/
/*       unmasks overrun timer at end of MIDI parsing           */
/****************************************************************/

int csysi_midiparseover(void)

{
  if (sigprocmask(SIG_UNBLOCK, &csysi_overrun_mask, NULL) < 0)
    CSYSI_ERROR_TERMINATE("Couldn't unmask MIDI overrun timer");

  return CSYS_NONE;
}

/****************************************************************/
/*             polling routine for new data                     */
/****************************************************************/

int csys_newdata(void)
     
{
  int i = 0;
  int retry = 0;
  int len;

  /* block overrun time and reset it */

  if (sigprocmask(SIG_BLOCK, &csysi_overrun_mask, NULL) < 0)
    CSYSI_ERROR_TERMINATE("Couldn't mask MIDI overrun timer");

  if (setitimer(ITIMER_REAL, &csysi_overrun_timer, NULL) < 0)
    CSYSI_ERROR_TERMINATE("Couldn't reset ITIMER_REAL timer");

  if (!csysi_holdidx)
    {
      while ((len = read(csysi_midi, csysi_hold, CSYSI_BUFFSIZE)) < 0)
	{      
	  if (++retry > CSYSI_MAXRETRY)
	    CSYSI_ERROR_TERMINATE("Too many I/O retries -- csys_newdata(if)");

	  if (errno == EAGAIN)
	    return csysi_midiparseover();   /* no data ready, so leave */
	  if (errno == EINTR)
	    continue;                       /* interrupted, try again */

	  /* all other errors fatal */

	  CSYSI_ERROR_TERMINATE("Couldn't read MIDI device");
	}
    }
  else
    {
      while ((len = read(csysi_midi, &(csysi_hold[csysi_holdidx]), 
	       CSYSI_BUFFSIZE-csysi_holdidx)) < 0)
	{
	  if (++retry > CSYSI_MAXRETRY)
	    CSYSI_ERROR_TERMINATE("Too many I/O retries -- csys_newdata(el)");

	  if (errno == EAGAIN)
	    break;                      /* no data ready, process buffer */
	  if (errno == EINTR)
	    continue;                   /* interrupted, try again */

	  /* all other errors fatal */

	  CSYSI_ERROR_TERMINATE("Couldn't read MIDI device");
	}

      len = (len < 0) ? csysi_holdidx : len + csysi_holdidx;
      csysi_holdidx = 0;
    }

  csysi_newnote = csysi_len = csysi_cnt = 0;
  while (i < len)
    {
      if (csysi_hold[i] <= CSYSI_SYSEX_EOX)
	csysi_data[csysi_len++] = csysi_hold[i];
      i++;
    }

  if (!csysi_len) 
    return csysi_midiparseover();

  /* leave interrupts locked until all data transferred */

  return CSYS_MIDIEVENTS;
  
}

/****************************************************************/
/*             gets one byte from MIDI stream                   */
/****************************************************************/

unsigned char csysi_getbyte(void)

{
  unsigned char d;
  int retry = 0;

  /* used when we need to risk waiting for the next byte */

  while (1)
    {
      if (read(csysi_midi, &d, 1) != 1)
	{
	  if (errno == EAGAIN) /* no data ready  */
	    {
	      retry = 0;
	      continue;
	    }
	  if (errno == EINTR) /* interrupted */
	    {	  
	      if (++retry > CSYSI_MAXRETRY)
		CSYSI_ERROR_TERMINATE("Too many I/O retries -- csysi_getbyte");
	      continue;
	    }
	  CSYSI_ERROR_TERMINATE("Couldn't read MIDI device");
	}
      else
	{
	  retry = 0;
	  if (d <= CSYSI_SYSEX_EOX)
	    break;
	  else
	    continue;
	}
    }

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
	return csysi_midiparseover();
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
	return csysi_midiparseover();
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
	return csysi_midiparseover();
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
    return csysi_midiparseover();
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
	    return csysi_midiparseover();
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
	  return csysi_midiparseover();
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
	  csysi_newnote |= (((*cmd) == CSYS_MIDI_NOTEON) |
			    ((*cmd) == CSYS_MIDI_NOTEOFF));
	  *vdata = csysi_data[csysi_cnt++];
	  if (csysi_cnt == csysi_len)
	    return csysi_midiparseover();
	  else
	    return CSYS_MIDIEVENTS;
	}
      if (CSYSI_DELAY && (csysi_num == 2) && /* (further) delay cmd */
	  (csysi_cnt + 1 == csysi_len))
	{
	  csysi_ndata = csysi_data[csysi_cnt];
	  *cmd = CSYS_MIDI_NOOP;
	  return csysi_midiparseover();
	}
    }

  /* do complete commands and finish some types of delayed commands */

  if (csysi_cnt + csysi_num <= csysi_len)
    {
      csysi_newnote |= (((*cmd) == CSYS_MIDI_NOTEON) |
			((*cmd) == CSYS_MIDI_NOTEOFF));
      *ndata = csysi_data[csysi_cnt++];
      if (csysi_num == 2)
	*vdata = csysi_data[csysi_cnt++];
      if (csysi_cnt == csysi_len)
	return csysi_midiparseover();
      else
	return CSYS_MIDIEVENTS;
    }

  /* should never execute if CSYSI_DELAY is 1 */

  csysi_newnote |= (((*cmd) == CSYS_MIDI_NOTEON) |
		    ((*cmd) == CSYS_MIDI_NOTEOFF));

  tot = csysi_cnt + csysi_num - csysi_len;
  idx = 0;
  while (tot > 0)
    {
      overflow[idx++] = csysi_getbyte();
      tot--;
    }
  if (csysi_num == 1) 
    {
      *ndata = overflow[0];
      return csysi_midiparseover();
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
  return csysi_midiparseover();
  
}


/****************************************************************/
/*                  closing routine for control                 */
/****************************************************************/

void csys_shutdown(void)
     
{
  /* disarm timer */

  if (sigprocmask(SIG_BLOCK, &csysi_overrun_mask, NULL) < 0)
    CSYSI_ERROR_TERMINATE("Couldn't mask MIDI overrun time");

  csysi_overrun_timer.it_value.tv_sec = 0;
  csysi_overrun_timer.it_value.tv_usec = 0;
  csysi_overrun_timer.it_interval.tv_sec = 0;
  csysi_overrun_timer.it_interval.tv_usec = 0;

  if (setitimer(ITIMER_REAL, &csysi_overrun_timer, NULL) < 0)
    CSYSI_ERROR_TERMINATE("Couldn't disarm ITIMER_REAL timer");

  close(csysi_midi);
}


#undef CSYSI_MIDIDEV
#undef CSYSI_BUFFSIZE
#undef CSYSI_SYSEX_EOX
#undef CSYSI_DELAY
#undef CSYSI_ALARMPERIOD
#undef CSYSI_ERROR_RETURN
#undef CSYSI_ERROR_TERMINATE

