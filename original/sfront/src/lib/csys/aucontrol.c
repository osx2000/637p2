
/*
#    Sfront, a SAOL to C translator    
#    This file: aucontrol control driver for sfront
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
/*               audiounit control driver for sfront            */ 
/****************************************************************/

/*~~~~~~~~~~~~~~~~~*/
/* include headers */
/*_________________*/

/* for socket system */

#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/uio.h>
#include <fcntl.h>

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*        MIDI event constants       */
/*  Must match audiounit.c versions  */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/* retry limit for socket writing */

#define CSYSI_AUCONTROL_RETRY_MAX  256

/* bitfield constants for MIDIevent flags variable */

#define CSYSI_AUCONTROL_MIDIFLAGS_WAITING 0x01u  /* queuing flag bit */

/* bitfield constants for SASLevent flags variable */

#define CSYSI_AUCONTROL_SASLFLAGS_WAITING 0x01u  /* queuing flag bit */

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*   typedef for MIDI events   */
/*  Fields & order must match  */
/*  asysn_audiounit_MIDIevent  */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

typedef struct csysi_aucontrol_MIDIevent {
  unsigned char cmd;
  unsigned char d0;
  unsigned char d1;
  unsigned char flags;
  int kcycleidx;
} csysi_aucontrol_MIDIevent;

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*   typedef for SASL events   */
/*  Fields & order must match  */
/*  asysn_audiounit_SASLevent  */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

typedef struct csysi_aucontrol_SASLevent {
  int index;
  Float32 value;
  unsigned char flags;
  int kcycleidx;
} csysi_aucontrol_SASLevent;

/*~~~~~~~~~~~~~~~~~*/
/* aucontrol state */
/*~~~~~~~~~~~~~~~~~*/

typedef struct csysi_aucontrol_state {
  int mpipe;
  csysi_aucontrol_MIDIevent nextMIDIevent;
  int spipe;
  csysi_aucontrol_SASLevent nextSASLevent;
} csysi_aucontrol_state;

/*~~~~~~~~~~~~~~~~~~~~~~~~~*/
/* helper function externs */
/*_________________________*/

extern int csysi_aucontrol_midievent_read(csysi_aucontrol_state * mystate);
extern int csysi_aucontrol_saslevent_read(csysi_aucontrol_state * mystate);

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*     high-level functions: called by sfront engine            */
/*______________________________________________________________*/

/****************************************************************/
/*             initialization routine for control               */
/****************************************************************/

int csys_setup(ENGINE_PTR_DECLARE)
     
{
  csysi_aucontrol_state * mystate;
  int msize;
  char message[256];

  msize = sizeof(csysi_aucontrol_state);
  if (!(mystate = calloc(1, msize)))
    return CSYS_ERROR;
  asysn_audiounit_memstatus(mystate, msize, MADV_WILLNEED);

  EV(csys_state) = (void *) mystate;

  if ((EV(csys_argc) == 5) && 
      !strcmp(EV(csys_argv)[1], "-asys_audiounit_mpipe") &&
      !strcmp(EV(csys_argv)[3], "-asys_audiounit_spipe"))
    {
      mystate->mpipe = atoi(EV(csys_argv[2]));
      mystate->spipe = atoi(EV(csys_argv[4]));
    }

  sprintf(message, "Opening aucontrol driver for %s\n\n", 
	  (EV(csys_argc) >= 1) ? EV(csys_argv)[0] : "(unknown)");

  ASYS_AUDIOUNIT_WIRETAP_PUTSTRING(message);

  return CSYS_DONE;
}

/****************************************************************/
/*             polling routine for new data                     */
/****************************************************************/

int csys_newdata(ENGINE_PTR_DECLARE)
     
{
  csysi_aucontrol_state * mystate = (csysi_aucontrol_state *) EV(csys_state);
  int has_midi, has_sasl;

#if defined(CSYS_CDRIVER_AUCONTROLM)
  has_midi = (mystate->nextMIDIevent.flags & CSYSI_AUCONTROL_MIDIFLAGS_WAITING) ||
    (csysi_aucontrol_midievent_read(mystate) == CSYS_MIDIEVENTS);
#else
  has_midi = 0;
#endif

  has_sasl = (mystate->nextSASLevent.flags & CSYSI_AUCONTROL_SASLFLAGS_WAITING) ||
    (csysi_aucontrol_saslevent_read(mystate) == CSYS_SASLEVENTS);

  if (!(has_midi || has_sasl))    /* quick exit in common case */
    return CSYS_NONE;

  has_midi = has_midi && (mystate->nextMIDIevent.kcycleidx <= EV(kcycleidx));
  has_sasl = has_sasl && (mystate->nextSASLevent.kcycleidx <= EV(kcycleidx));

  if (!(has_midi || has_sasl))
    return CSYS_NONE;

  if (has_midi && has_sasl)
    return CSYS_EVENTS;

#if defined(CSYS_CDRIVER_AUCONTROLM)
  if (has_midi)
    return CSYS_MIDIEVENTS;
#endif

  if (has_sasl)
    return CSYS_SASLEVENTS;
}


#if defined(CSYS_CDRIVER_AUCONTROLM)

/****************************************************************/
/*                 processes a MIDI event                       */
/****************************************************************/

int csys_midievent(ENGINE_PTR_DECLARE_COMMA unsigned char * cmd,  
		   unsigned char * ndata, unsigned char * vdata, 
		   unsigned short * extchan, float * fval)

{
  csysi_aucontrol_state * mystate = (csysi_aucontrol_state *) EV(csys_state);

  if ((*cmd = mystate->nextMIDIevent.cmd) < CSYS_MIDI_SYSTEM)
    {
      *extchan = (*cmd) & 0x0Fu;  
      *ndata = mystate->nextMIDIevent.d0;
      *vdata = mystate->nextMIDIevent.d1;
    }
  else
    *cmd = CSYS_MIDI_NOOP;  /* filter MIDI System commands */

  if (csysi_aucontrol_midievent_read(mystate) == CSYS_NONE)
    {
      mystate->nextMIDIevent.flags &= ~(CSYSI_AUCONTROL_MIDIFLAGS_WAITING);
      return CSYS_NONE;
    }

  if (mystate->nextMIDIevent.kcycleidx <= EV(kcycleidx))
    return CSYS_MIDIEVENTS;

  return CSYS_NONE;
}

#endif

/****************************************************************/
/*                 processes a SASL event                       */
/****************************************************************/

int csys_saslevent(ENGINE_PTR_DECLARE_COMMA unsigned char * cmd, 
		   unsigned char * priority, unsigned short * id, 
		   unsigned short * label, float * fval, 
		   unsigned int * pnum, float ** p)

{
  csysi_aucontrol_state * mystate = (csysi_aucontrol_state *) EV(csys_state);

  *cmd = CSYS_SASL_CONTROL;
  *priority = 1;
  *id = CSYS_SASL_NOINSTR;
  *label = CSYS_NOLABEL;
  *pnum = mystate->nextSASLevent.index;
  *fval = mystate->nextSASLevent.value;
  
  if (csysi_aucontrol_saslevent_read(mystate) == CSYS_NONE)
    {
      mystate->nextSASLevent.flags &= ~(CSYSI_AUCONTROL_SASLFLAGS_WAITING);
      return CSYS_NONE;
    }

  if (mystate->nextSASLevent.kcycleidx <= EV(kcycleidx))
    return CSYS_SASLEVENTS;

  return CSYS_NONE;
}


/****************************************************************/
/*                  closing routine for control                 */
/****************************************************************/

void csys_shutdown(ENGINE_PTR_DECLARE)
     
{
  csysi_aucontrol_state * mystate = (csysi_aucontrol_state *) EV(csys_state);
  int msize;
  char message[256];

  /* flush unprocessed events from MIDI and SASL pipes */

#if defined(CSYS_CDRIVER_AUCONTROLM)
  do { } while (csysi_aucontrol_midievent_read(mystate) != CSYS_NONE);
#endif
  do { } while (csysi_aucontrol_saslevent_read(mystate) != CSYS_NONE);

  /* log driver close, and free mystate */

  sprintf(message, "Closing aucontrol driver for %s\n\n",
	  (EV(csys_argc) >= 1) ? EV(csys_argv)[0] : "(unknown)");

  ASYS_AUDIOUNIT_WIRETAP_PUTSTRING(message);

  msize = sizeof(csysi_aucontrol_state);
  asysn_audiounit_memstatus(mystate, msize, MADV_FREE);
  free(mystate);

  return;
}


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*                     helper functions                         */
/*______________________________________________________________*/

#if defined(CSYS_CDRIVER_AUCONTROLM)

/****************************************************************/
/*              read a MIDI event from the socket               */
/****************************************************************/

int csysi_aucontrol_midievent_read(csysi_aucontrol_state * mystate)

{
  int retry = 0;
  int len;

  if (mystate->mpipe == 0)
      return CSYS_NONE;   /* no mpipe, so no MIDIevents */

  do {

    if (((len = read(mystate->mpipe, &(mystate->nextMIDIevent),
		     sizeof(csysi_aucontrol_MIDIevent))) < 0) && (errno == EAGAIN))
      return CSYS_NONE;        /* no MIDIevents in mpipe */

    if (len == sizeof(csysi_aucontrol_MIDIevent))
      return CSYS_MIDIEVENTS;  /* a MIDIevent was read  */

    if ((len >= 0) || (errno != EINTR) || (++retry > CSYSI_AUCONTROL_RETRY_MAX))
      {
	if (len == 0)
	  ASYS_AUDIOUNIT_WIRETAP_PUTSTRING
	    ("\tError in csys_newdata: read() returned a zero-length MIDIevent\n");

	if (len > 0)
	  ASYS_AUDIOUNIT_WIRETAP_PUTSTRING
	    ("\tError in csys_newdata: read() returned an incomplete MIDIevent\n");
	
	if ((len < 0) && (errno != EINTR))
	  ASYS_AUDIOUNIT_WIRETAP_PUTSTRING
	    ("\tError in csys_newdata: errno other than EINTR or EAGAIN (MIDI)\n");

	if ((len < 0) && (retry > ASYS_AUDIOUNIT_RETRY_MAX))
	  ASYS_AUDIOUNIT_WIRETAP_PUTSTRING
	    ("\tError in csys_newdata: Maximum retry for EINTR exceeded (MIDI)\n");

	return CSYS_NONE;     /* to do: clear flag, consider error reporting */
      }
    
  } while(1);   /* loop to try again when (errno == EINTR) */

  return CSYS_NONE;
}

#endif 

/****************************************************************/
/*              read a SASL event from the socket               */
/****************************************************************/

int csysi_aucontrol_saslevent_read(csysi_aucontrol_state * mystate)

{
  int retry = 0;
  int len;

  if (mystate->spipe == 0)
      return CSYS_NONE;   /* no spipe, so no SASLevents */

  do {

    if (((len = read(mystate->spipe, &(mystate->nextSASLevent),
		     sizeof(csysi_aucontrol_SASLevent))) < 0) && (errno == EAGAIN))
      return CSYS_NONE;        /* no SASLevents in spipe */

    if (len == sizeof(csysi_aucontrol_SASLevent))
      return CSYS_SASLEVENTS;  /* a SASLevent was read  */

    if ((len >= 0) || (errno != EINTR) || (++retry > CSYSI_AUCONTROL_RETRY_MAX))
      {
	if (len == 0)
	  ASYS_AUDIOUNIT_WIRETAP_PUTSTRING
	    ("\tError in csys_newdata: read() returned a zero-length SASLevent\n");

	if (len > 0)
	  ASYS_AUDIOUNIT_WIRETAP_PUTSTRING
	    ("\tError in csys_newdata: read() returned an incomplete SASLevent\n");
	
	if ((len < 0) && (errno != EINTR))
	  ASYS_AUDIOUNIT_WIRETAP_PUTSTRING
	    ("\tError in csys_newdata: errno other than EINTR or EAGAIN (SASL)\n");

	if ((len < 0) && (retry > ASYS_AUDIOUNIT_RETRY_MAX))
	  ASYS_AUDIOUNIT_WIRETAP_PUTSTRING
	    ("\tError in csys_newdata: Maximum retry for EINTR exceeded (SASL)\n");

	return CSYS_NONE;     /* to do: clear flag, consider error reporting */
      }
    
  } while(1);   /* loop to try again when (errno == EINTR) */

  return CSYS_NONE;
}
