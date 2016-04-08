
/*
#    Sfront, a SAOL to C translator    
#    This file: coremidi control driver for sfront
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
/*                coremidi control driver for sfront            */ 
/****************************************************************/

#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/uio.h>
#include <fcntl.h>

#if !(defined(ASYS_OUTDRIVER_COREAUDIO)||defined(ASYS_INDRIVER_COREAUDIO))
#include <CoreFoundation/CFString.h>
#endif

#include <CoreMIDI/CoreMIDI.h>

/***************************/
/* graceful_exit constants */
/***************************/

#define CSYSI_GE_RUNNING  0
#define CSYSI_GE_DOEXIT   1
#define CSYSI_GE_EXITED   2

#define CSYSI_RETRY_MAX   256    /* retry counter for read() EINTR */

/*************************/
/* MIDI source constants */
/*************************/

#define CSYSI_MAXPORTS 4       /* must match ../../control.c  */

/**********************************/
/* buffer constants -- needs work */
/**********************************/

#define CSYSI_BUFFSIZE   1024  /* size of the holding buffer   */
#define CSYSI_SENDSIZE   256   /* maximum size of one packet[] */
#define CSYSI_SYSEXSIZE    128  /* buffer size for F0/F4/F5     */

/****************************/
/* System command constants */
/****************************/

#define CSYSI_BLOCK_SYSTEM_COMMANDS   0   /* remove 0xF from source */
#define CSYSI_PASS_SYSTEM_COMMANDS    1   /* retain 0xF             */

#define CSYSI_SYSTEM_GATE CSYSI_PASS_SYSTEM_COMMANDS  /* sets mode */

#define CSYSI_COMMAND_ONGOING         0   /* arbitrary-length command ongoing   */
#define CSYSI_COMMAND_COMPLETED       1   /* arbitrary-length command completed */


/**********************/
/* preamble constants */
/**********************/

/*

 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 
-------------------------------------------------------------------
|    0xF7       |T|R|R|R|  EXT  |  tstamp (T = 1)  ... | MIDI ... |
-------------------------------------------------------------------

  T is set to 1 if a non-NOW timestamp follows the header octet.
  R's are reserved.
  EXT codes which of 16 16-channel MIDI sources.

*/

#define CSYSI_PREAMBLE_SIZE    6  /* read_proc preamble w/ timestamp */
#define CSYSI_PREAMBLE_NOWSIZE 2  /* read_proc preamble w/o timestamp */

#define CSYSI_PREAMBLE_POS_MARKER  0  /* first octet is the marker: 0xFF     */
#define CSYSI_PREAMBLE_POS_CONTROL 1  /* second octet holds control bits     */
#define CSYSI_PREAMBLE_POS_TSTAMP  2  /* last 4 (optional) octets for tstamp */

#define CSYSI_PREAMBLE_FLAGS_MARKER  0xF7u  /* marker value: for easy parsing */
#define CSYSI_PREAMBLE_FLAGS_T       0x80u  /* timestamp follows this octet   */
#define CSYSI_PREAMBLE_FLAGS_R2      0x40u  /* reserved bits                  */
#define CSYSI_PREAMBLE_FLAGS_R1      0x20u  
#define CSYSI_PREAMBLE_FLAGS_R0      0x10u  
#define CSYSI_PREAMBLE_FLAGS_EXT     0x0Fu  /* the EXT nibble                 */

/*****************************/
/* simulated F4/F5 constants */
/*****************************/

/*
  SysEx format to simulate undefined commands:

  | F7 | 7D | code | octone (if bit set) | octtwo (if bit set) | F0 |

*/

#define CSYSI_UNDEFINED_ID          0x7Du  /* Manufacturers ID */

#define CSYSI_UNDEFINED_F4          0x40u   /* set is F4, clear is F5 */
#define CSYSI_UNDEFINED_OCTONE      0x20u   /* presence of 1st octet  */
#define CSYSI_UNDEFINED_OCTTWO      0x10u   /* presence of 2nd octet  */   

/************************/
/* input port variables */
/************************/

MIDIClientRef csysi_inclient;  /* the input client  */
MIDIPortRef   csysi_inport;    /* it's port         */

typedef struct csysi_portinfo {
  int ext;                 /* EXT field for preamble, also array index */
  int valid;               /* 0 if unused, 1 if online, -1 if offline  */
  MIDIEndpointRef src;     /* current src  (may be invalid if offline) */
  int id;                  /* unique ID of src (currently unused)      */

#if (CSYSI_SYSTEM_GATE == CSYSI_PASS_SYSTEM_COMMANDS)
  unsigned char sysbuff[CSYSI_SYSEXSIZE];       /* System command buffer */
  int sysex_size;           /* current size of sysbuff[]                 */
#endif

} csysi_portinfo;

csysi_portinfo csysi_ports[CSYSI_MAXPORTS];

/**********************************/
/* read_proc pipe data structures */
/**********************************/

int csysi_readproc_pipepair[2];
int csysi_graceful_exit;

/************************/
/* command buffer state */
/************************/

unsigned char csysi_buffer[CSYSI_BUFFSIZE];
int csysi_len, csysi_cnt;
unsigned short csysi_currext;

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

/*******************/
/* forward externs */
/*******************/

extern void csysi_read_proc(const MIDIPacketList * pktlist,
			    void * readProcRefCon, 
			    void * srcConnRefCon);

extern void csysi_notify_proc(const MIDINotification * message,
			      void * refcon);

extern int csysi_bugrecovery(MIDIPacket * p, unsigned char * mdata,
			     int * idx, int len);

extern int csysi_packet_arblength(MIDIPacket * p, unsigned char * mdata,
				  int * idx, int * len, unsigned char ext);

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*     high-level functions: called by sfront engine            */
/*______________________________________________________________*/

/****************************************************************/
/*             initialization routine for control               */
/****************************************************************/

int csys_setup(void)
     
{
  int connected, numsrc, flags, i, numvsrc;
  MIDIEndpointRef src;
  MIDIEntityRef ent;
  char name[256];
  CFStringRef cf_name;

  /********************************/
  /* create input client and port */
  /********************************/

  if (MIDIClientCreate(CFSTR("sfront_inclient"), NULL, 
		       (void *) NULL, &csysi_inclient))
    CSYSI_ERROR_RETURN("Could not create CoreMIDI client");

  if (MIDIInputPortCreate(csysi_inclient, CFSTR("sfront_inport"),
			  csysi_read_proc, (void *) NULL, 
			  &csysi_inport))
    CSYSI_ERROR_RETURN("Could not create CoreMIDI inport");

  /********************************************************/
  /* open pipes, since data may start flowing immediately */
  /********************************************************/

  if (socketpair(AF_UNIX, SOCK_DGRAM, 0, csysi_readproc_pipepair))
    CSYSI_ERROR_RETURN("Cannot open readproc pipe");

  if ((flags = fcntl(csysi_readproc_pipepair[0], F_GETFL, 0)) == -1)
    CSYSI_ERROR_RETURN("Unknown flags for read end of readproc pipe");

  if (fcntl(csysi_readproc_pipepair[0], F_SETFL, flags | O_NONBLOCK) == -1) 
    CSYSI_ERROR_RETURN("Cannot O_NONBLOCK read end of readproc pipe");

  /********************************************************/
  /* for now, connect only one source, exit if no sources */
  /********************************************************/
  
  for (i = 0; i < CSYSI_MAXPORTS; i++)
    csysi_ports[i].ext = i;

  numvsrc = connected = 0;
  numsrc = (int) MIDIGetNumberOfSources();
  
  for (i = 0; i < numsrc; i++)
    if (src = MIDIGetSource((ItemCount) i))
      {
	if (MIDIEndpointGetEntity(src, &ent) || (ent == NULL))
	  {
	    numvsrc++;
	    continue;
	  }

	if (MIDIPortConnectSource(csysi_inport, src,
				  &(csysi_ports[connected].ext)))
	  continue;

	csysi_ports[connected].valid = 1;
	csysi_ports[connected].src = src;
	connected++;

	if ((connected == CSYSI_MAXPORTS) && (i + 1 != numsrc))
	  {
	    fprintf(stderr, 
		    "\nWarning: -cin coremidi configured for %i ports max,\n", 
		    CSYSI_MAXPORTS);
	    fprintf(stderr, 
		    "         skipping the last %i MIDI port(s) in your setup.\n", 
		    numsrc - i - 1);
	    break;
	  }
      }

  fprintf(stderr, "\nCoreMIDI setup info:\n");

  if (!connected)
    {
      fprintf(stderr, "   No CoreMIDI sources to connect with, exiting ...\n");
      if (numvsrc)
	fprintf(stderr, "   (Sfront doesn't support virtual sources yet.)\n");
      return CSYS_ERROR;
    }

  /*******************************/
  /* print out source identities */
  /*******************************/

  for (i = 0; i < connected; i++)
    {
      fprintf(stderr, "   SAOL MIDI channels %i-%i connect to ",
	      16*csysi_ports[i].ext, 16*(csysi_ports[i].ext + 1) - 1);

      src = csysi_ports[i].src;
      
      if (MIDIObjectGetStringProperty(src, kMIDIPropertyName, &cf_name))
	{
	  fprintf(stderr, "(unnamed source).\n");
	  continue;
	}

      if (!CFStringGetCString(cf_name, name, 256, CFStringGetSystemEncoding()))
	{
	  fprintf(stderr, "(unnamed source).\n");
	  CFRelease(cf_name);
	  continue;
	}

      CFRelease(cf_name);
      fprintf(stderr, "%s.\n", name);
    }

  if (numvsrc)
    fprintf(stderr, 
	    "   (Virtual sources also found; sfront doesn't support them yet.)\n");

  fprintf(stderr, "\nMIDI Preset Numbers ");

  i = 0;
  while (i < csys_sfront_argc)
    {
      if (!(strcmp(csys_sfront_argv[i],"-bitc") && 
	    strcmp(csys_sfront_argv[i],"-bit") &&
	    strcmp(csys_sfront_argv[i],"-orc")))
	{
	  i++;
	  fprintf(stderr, "for %s ", csys_sfront_argv[i]);
	  break;
	}
      i++;
    }

  fprintf(stderr, "(use MIDI controller to select):\n");

  for (i = 0; i < CSYS_PRESETNUM; i++)
    {
      fprintf(stderr, "%3i. %s", 
	      csys_presets[i].preset,
	      csys_instr[csys_presets[i].index].name);
      if ((i&1))
	fprintf(stderr, "\n");
      else
	{
	  fprintf(stderr, "\t\t");
	  if (i == (CSYS_PRESETNUM-1))
	    fprintf(stderr, "\n");
	}
    }

  return CSYS_DONE;
}

/****************************************************************/
/*             polling routine for new data                     */
/****************************************************************/

int csys_newdata(void)
     
{
  int len, retry;

  if (((len = read(csysi_readproc_pipepair[0], csysi_buffer, 
		   CSYSI_SENDSIZE)) < 0) && (errno == EAGAIN))
    return CSYS_NONE;  /* fast path */

  if (csysi_graceful_exit == CSYSI_GE_EXITED)
    return CSYS_NONE;

  csysi_len = 0;
  retry = 0;

  do 
    {
      if (len > 0)
	{
	  if ((csysi_len += len) > (CSYSI_BUFFSIZE - CSYSI_SENDSIZE))
	    break;
	}
      else
	{
	  if (len == 0)
	    {
	      fprintf(stderr, "-cin coremidi: writev() error.\n");
	      csysi_graceful_exit = CSYSI_GE_DOEXIT;
	      break;
	    }
	  else
	    if ((errno != EINTR) || (++retry > CSYSI_RETRY_MAX)) 
	      {
		fprintf(stderr, "-cin coremidi: pipe read() error.\n");
		csysi_graceful_exit = CSYSI_GE_DOEXIT;
		break;
	      }
	}

      len = read(csysi_readproc_pipepair[0], &(csysi_buffer[csysi_len]),
		 CSYSI_SENDSIZE);
    } 
  while ((len >= 0) || (errno != EAGAIN));

  csysi_cnt = 0;
  return CSYS_MIDIEVENTS;
}

/****************************************************************/
/*                 processes a MIDI event                       */
/****************************************************************/

int csys_midievent(unsigned char * cmd,   unsigned char * ndata, 
	           unsigned char * vdata, unsigned short * extchan,
		   float * fval)

{
  if (csysi_graceful_exit == CSYSI_GE_DOEXIT)
    {
      csysi_graceful_exit = CSYSI_GE_EXITED;
      *cmd = CSYS_MIDI_ENDTIME;
      *fval = EV(scorebeats);
      return CSYS_NONE;
    }

  while (csysi_buffer[csysi_cnt] == CSYSI_PREAMBLE_FLAGS_MARKER)
    {
      /* later handle semantics of the timestamp bits */

      csysi_currext = 
	(csysi_buffer[++csysi_cnt] & CSYSI_PREAMBLE_FLAGS_EXT) << 4;

      /* skip over the rest of the preamble */

      if ((csysi_buffer[csysi_cnt] & CSYSI_PREAMBLE_FLAGS_T) == 0)
	csysi_cnt++;     
      else 
	csysi_cnt += 5;

      if (csysi_cnt == csysi_len)
	{
	  *cmd = CSYS_MIDI_NOOP;  
	  return CSYS_NONE;
	}
    }

  /* assumes:
   *   1 - integral number of commands
   *   2 - F0, F7, "specialized"
   *   3 - F4/F5 has 0, 1, or 2 data bytes
   *   4 - input stream is perfect MIDI + "specials"
   */

  if (csysi_buffer[csysi_cnt] < (unsigned char)(CSYS_MIDI_SYSTEM))
    {
      if ((CSYSI_SYSTEM_GATE == CSYSI_PASS_SYSTEM_COMMANDS) &&
	  (csysi_buffer[csysi_cnt] < (unsigned char) (CSYS_MIDI_NOTEOFF)))
	{
	  *cmd = csysi_buffer[csysi_cnt++];
	  *extchan = csysi_currext;
	  *ndata = csysi_buffer[csysi_cnt++];
	  
	  if (*cmd != CSYS_MIDI_GMRESET)
	    *vdata = csysi_buffer[csysi_cnt++];
	}
      else
	{
	  *cmd = 0xF0u & csysi_buffer[csysi_cnt];
	  *extchan = (0x0Fu & csysi_buffer[csysi_cnt++]) + csysi_currext;
	  *ndata = csysi_buffer[csysi_cnt++];
	  
	  if ((*cmd != CSYS_MIDI_PROGRAM) && (*cmd != CSYS_MIDI_CTOUCH))
	    *vdata = csysi_buffer[csysi_cnt++];
	}
    }
  else
    {
      *cmd = csysi_buffer[csysi_cnt++];
      *extchan = csysi_currext;

      if (*cmd < CSYS_MIDI_SYSTEM_SYSEX_END)
	{
	  if ((*cmd == CSYS_MIDI_SYSTEM_QFRAME) || 
	      (*cmd == CSYS_MIDI_SYSTEM_SONG_SELECT) ||
	      (*cmd == CSYS_MIDI_SYSTEM_SONG_PP))
	    *ndata = csysi_buffer[csysi_cnt++];

	  if (*cmd == CSYS_MIDI_SYSTEM_SONG_PP)
	    *vdata = csysi_buffer[csysi_cnt++];

	  if ((*cmd == CSYS_MIDI_SYSTEM_UNUSED1) || 
	      (*cmd == CSYS_MIDI_SYSTEM_UNUSED2))
	    {
	      if ((csysi_cnt < csysi_len) && 
		  !(csysi_buffer[csysi_cnt] & 0x80u))
		*ndata = csysi_buffer[csysi_cnt++];
	      else
		*ndata = CSYS_MIDI_SYSTEM_SYSEX_END;

	      if ((csysi_cnt < csysi_len) && 
		  !(csysi_buffer[csysi_cnt] & 0x80u))
		*vdata = csysi_buffer[csysi_cnt++];
	      else
		*vdata = CSYS_MIDI_SYSTEM_SYSEX_END;
	    }
	}
    }

  if (csysi_cnt == csysi_len)
    return CSYS_NONE;
  else
    return CSYS_MIDIEVENTS;
}

/****************************************************************/
/*                  closing routine for control                 */
/****************************************************************/

void csys_shutdown(void)
     
{
  close(csysi_readproc_pipepair[0]);
  close(csysi_readproc_pipepair[1]);
  return;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*       callback functions: called by coremidi directly        */
/*______________________________________________________________*/

extern int csysi_packet_shorten(MIDIPacket * p, 
				unsigned char * mdata,
				int * idx, unsigned char ext); 

/****************************************************************/
/*                  callback for MIDI input                     */
/****************************************************************/

void csysi_read_proc(const MIDIPacketList * pktlist,
		     void * readProcRefCon, void * srcConnRefCon)

{
  unsigned char preamble[CSYSI_PREAMBLE_NOWSIZE];
  unsigned char mdata[CSYSI_SENDSIZE];
  struct iovec vector[2];
  MIDIPacket * p;
  int num, idx, retry;
  unsigned char ext;

  /* initialize preamble */

  preamble[CSYSI_PREAMBLE_POS_MARKER] = CSYSI_PREAMBLE_FLAGS_MARKER;
  preamble[CSYSI_PREAMBLE_POS_CONTROL] = ext = 
    CSYSI_PREAMBLE_FLAGS_EXT & ((unsigned char) *((int *) srcConnRefCon));

  /* initialize write vector -- update for timestamp support */

  vector[0].iov_base = (char *) preamble;
  vector[0].iov_len = (size_t) CSYSI_PREAMBLE_NOWSIZE;
  vector[1].iov_base = (char *) mdata;

  /* loop through packets, send DGRAMs */

  p = (MIDIPacket *) &(pktlist->packet[0]);
  num = pktlist->numPackets;

  while (num--)
    {
      retry = idx = 0;

      do {
	if ((vector[1].iov_len = csysi_packet_shorten(p, mdata, &idx, ext)))
	  while (writev(csysi_readproc_pipepair[1], vector, 2) < 0)
	    if ((errno != EINTR) || (++retry >= CSYSI_RETRY_MAX))
	      {
		close(csysi_readproc_pipepair[0]);
		close(csysi_readproc_pipepair[1]);
		return;
	      }
      } while (idx < p->length);

      p = MIDIPacketNext(p);
    }
}

/****************************************************************/
/*                  callback for MIDI input                     */
/****************************************************************/

void csysi_notify_proc(const MIDINotification * message,
		       void * refcon)

{
  /* not currently registered with MIDIClientCreate() */
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*   mid-level functions: called by top-level driver functions  */
/*______________________________________________________________*/

/****************************************************************/
/*               copy SA-safe commands into mdata               */
/****************************************************************/

int csysi_packet_shorten(MIDIPacket * p, unsigned char * mdata,
			 int * idx, unsigned char ext)
{
  int len = 0;

  do {

    if (CSYSI_SYSTEM_GATE == CSYSI_PASS_SYSTEM_COMMANDS)
      if (csysi_ports[ext].sysex_size && (p->data[*idx] & 0xF0u))
	if (csysi_packet_arblength(p, mdata, idx, &len, ext) || (*idx == p->length))
	  return len;

    switch (p->data[*idx] & 0xF0u) {
    case CSYS_MIDI_NOTEOFF:
    case CSYS_MIDI_NOTEON:
    case CSYS_MIDI_PTOUCH:
    case CSYS_MIDI_WHEEL:
    case CSYS_MIDI_CC:
      if ((p->length >= (*idx + 3)) && (p->data[*idx + 1] < 128)
	  && (p->data[*idx + 2] < 128))
	{
	  memcpy(&(mdata[len]), &(p->data[*idx]), 3);
	  if (p->length == (*idx += 3))
	    return len + 3;
	  if ((len += 3) > (CSYSI_SENDSIZE - 3))
	    return len;
	}
      else
	return csysi_bugrecovery(p, mdata, idx, len);
      continue;
    case CSYS_MIDI_PROGRAM:
    case CSYS_MIDI_CTOUCH:
      if ((p->length >= (*idx + 2)) && (p->data[*idx + 1] < 128))
	{
	  memcpy(&(mdata[len]), &(p->data[*idx]), 2);
	  if (p->length == (*idx += 2))
	    return len + 2;
	  if ((len += 2) > (CSYSI_SENDSIZE - 3))
	    return len;     
	}
      else
	return csysi_bugrecovery(p, mdata, idx, len);
      continue;
    case CSYS_MIDI_SYSTEM:
      switch (p->data[*idx]) {
      case CSYS_MIDI_SYSTEM_TICK:                                /* 1 octet */ 
      case CSYS_MIDI_SYSTEM_TUNE_REQUEST:                   
      case CSYS_MIDI_SYSTEM_CLOCK:
      case CSYS_MIDI_SYSTEM_START:
      case CSYS_MIDI_SYSTEM_CONTINUE:
      case CSYS_MIDI_SYSTEM_STOP:
      case CSYS_MIDI_SYSTEM_UNUSED3:
      case CSYS_MIDI_SYSTEM_SENSE:
      case CSYS_MIDI_SYSTEM_RESET:
	if (CSYSI_SYSTEM_GATE == CSYSI_BLOCK_SYSTEM_COMMANDS)
	  {
	    if (p->length == (*idx += 1))
	      return len;
	  }
	else
	  {
	    memcpy(&(mdata[len]), &(p->data[*idx]), 1);
	    if (p->length == (*idx += 1))
	      return len + 1;
	    if ((len += 1) > (CSYSI_SENDSIZE - 3))
	      return len;
	  }
	continue;
      case CSYS_MIDI_SYSTEM_SYSEX_END:                      
	if (p->length == (*idx += 1))
	  return len;
	continue;
      case CSYS_MIDI_SYSTEM_QFRAME:                              /* 2 octets */ 
      case CSYS_MIDI_SYSTEM_SONG_SELECT:
	if (CSYSI_SYSTEM_GATE == CSYSI_BLOCK_SYSTEM_COMMANDS)
	  {
	    if (p->length == (*idx += 2))
	      return len;	 
	  }
	else
	  {
	    if ((p->length >= (*idx + 2)) && (p->data[*idx + 1] < 128))
	      {
		memcpy(&(mdata[len]), &(p->data[*idx]), 2);
		if (p->length == (*idx += 2))
		  return len + 2;
		if ((len += 2) > (CSYSI_SENDSIZE - 3))
		  return len;     
	      }
	    else
	      return csysi_bugrecovery(p, mdata, idx, len);
	  }
      continue;
      case CSYS_MIDI_SYSTEM_SONG_PP:                             /* 3 octets */ 
	if (CSYSI_SYSTEM_GATE == CSYSI_BLOCK_SYSTEM_COMMANDS)
	  {
	    if (p->length == (*idx += 3))
	      return len;
	  }
	else
	  {
	    if ((p->length >= (*idx + 3)) && (p->data[*idx + 1] < 128)
		&& (p->data[*idx + 2] < 128))
	      {
		memcpy(&(mdata[len]), &(p->data[*idx]), 3);
		if (p->length == (*idx += 3))
		  return len + 3;
		if ((len += 3) > (CSYSI_SENDSIZE - 3))
		  return len;
	      }
	    else
	      return csysi_bugrecovery(p, mdata, idx, len);
	  }
	continue;
      case CSYS_MIDI_SYSTEM_UNUSED1:           /* undefined Common commands */
      case CSYS_MIDI_SYSTEM_UNUSED2:
	if (CSYSI_SYSTEM_GATE == CSYSI_PASS_SYSTEM_COMMANDS)
	  {
	    /* case 1: zero data octets */
	    
	    if ((((*idx) + 1) == p->length) || (p->data[(*idx) + 1] & 0x80u))
	      {
		memcpy(&(mdata[len]), &(p->data[*idx]), 1);
		if (p->length == (*idx += 1))
		  return len + 1;
		if ((len += 1) > (CSYSI_SENDSIZE - 3))
		  return len;
		continue;
	      }

	    /* case 2: one data octet */
	    
	    if ((((*idx) + 2) == p->length) || (p->data[(*idx) + 2] & 0x80u))
	      {
		memcpy(&(mdata[len]), &(p->data[*idx]), 2);
		if (p->length == (*idx += 2))
		  return len + 2;
		if ((len += 2) > (CSYSI_SENDSIZE - 3))
		  return len;
		continue;
	      }

	    /* case 3: two data octets */
	    
	    if ((((*idx) + 3) == p->length) || (p->data[(*idx) + 3] & 0x80u))
	      {
		memcpy(&(mdata[len]), &(p->data[*idx]), 3);
		if (p->length == (*idx += 3))
		  return len + 3;
		if ((len += 3) > (CSYSI_SENDSIZE - 3))
		  return len;
		continue;
	      }
	  }
	*idx += 1;
	while ((*idx < p->length) && !(p->data[(*idx)] & 0x80u))
	  (*idx)++;
	if (*idx == p->length)
	  return len;
	continue;
      case CSYS_MIDI_SYSTEM_SYSEX_START:                 /* SysEx commands */
	if (CSYSI_SYSTEM_GATE == CSYSI_BLOCK_SYSTEM_COMMANDS)
	  {
	    if (p->data[(*idx)] == CSYS_MIDI_SYSTEM_SYSEX_START)
	      *idx += 1;
	    while ((*idx < p->length) && !(p->data[(*idx)] & 0x80u))
	      (*idx)++;
	    if (p->data[(*idx)] == CSYS_MIDI_SYSTEM_SYSEX_END)
	      (*idx)++;
	  }
	else
	  if (csysi_packet_arblength(p, mdata, idx, &len, ext))
	    return len;

	if (*idx == p->length)
	  return len;
	continue;
      }
      *idx = p->length;                                /* should never run     */
      return len;
    default:                                           /* a sysex continuation */
      if (CSYSI_SYSTEM_GATE == CSYSI_BLOCK_SYSTEM_COMMANDS)
	{
	  while ((*idx < p->length) && !(p->data[(*idx)] & 0x80u))
	    (*idx)++;
	  if (p->data[(*idx)] == CSYS_MIDI_SYSTEM_SYSEX_END)
	    (*idx)++;
	  if (*idx == p->length)
	    return len;
	}
      else
	if (csysi_packet_arblength(p, mdata, idx, &len, ext))
	  return len;

      if (*idx == p->length)
	return len;
      continue;
    }
  }
  while (1);
  
  *idx = p->length;  /* should never run, loop has no exit path */
  return len;
} 

/****************************************************************/
/*             recover from CoreMIDI legacy bugs                */
/****************************************************************/

int csysi_bugrecovery(MIDIPacket * p, unsigned char * mdata,
		   int * idx, int len)
{
  mdata[len] = CSYS_MIDI_SYSTEM_RESET;
  *idx = p->length;
  return len + 1;   
}

/****************************************************************/
/*          parse arbitrary-length command bodies               */
/****************************************************************/

int csysi_packet_arblength(MIDIPacket * p, unsigned char * mdata,
			   int * idx, int * len, unsigned char ext)
{

  if (p->data[(*idx)] == CSYS_MIDI_SYSTEM_SYSEX_START)
    {
      csysi_ports[ext].sysex_size = 0;
      *idx += 1;
    }

  while (*idx < p->length)
    if (p->data[(*idx)] & 0x80u)
      {
	if (p->data[(*idx)] == CSYS_MIDI_SYSTEM_SYSEX_END)
	  (*idx)++;
	
	switch (csysi_ports[ext].sysex_size) {
	case 2:
	  if (csysi_ports[ext].sysbuff[0] == 0x7Du)
	    {
	      if (csysi_ports[ext].sysbuff[1] & CSYSI_UNDEFINED_F4)
		mdata[(*len)++] = CSYS_MIDI_SYSTEM_UNUSED1;
	      else
		mdata[(*len)++] = CSYS_MIDI_SYSTEM_UNUSED2;
	      if ((p->length == (*idx)) || ((*len) > (CSYSI_SENDSIZE - 3)))
		return CSYSI_COMMAND_COMPLETED;
	    }
	  break;
	case 3:
	  if (csysi_ports[ext].sysbuff[0] == 0x7Du)
	    {
	      if (csysi_ports[ext].sysbuff[1] & CSYSI_UNDEFINED_F4)
		mdata[(*len)++] = CSYS_MIDI_SYSTEM_UNUSED1;
	      else
		mdata[(*len)++] = CSYS_MIDI_SYSTEM_UNUSED2;
	      mdata[(*len)++] = csysi_ports[ext].sysbuff[2];
	      if ((p->length == (*idx)) || ((*len) > (CSYSI_SENDSIZE - 3)))
		return CSYSI_COMMAND_COMPLETED;
	    }
	  break;
	case 4:
	  if (csysi_ports[ext].sysbuff[0] == 0x7Du)
	    {
	      if (csysi_ports[ext].sysbuff[1] & CSYSI_UNDEFINED_F4)
		mdata[(*len)++] = CSYS_MIDI_SYSTEM_UNUSED1;
	      else
		mdata[(*len)++] = CSYS_MIDI_SYSTEM_UNUSED2;
	      mdata[(*len)++] = csysi_ports[ext].sysbuff[2];
	      mdata[(*len)++] = csysi_ports[ext].sysbuff[3];
	      if ((p->length == (*idx)) || ((*len) > (CSYSI_SENDSIZE - 3)))
		return CSYSI_COMMAND_COMPLETED;
	    }
	  if ((csysi_ports[ext].sysbuff[0] == 0x7Eu) &&
	      (csysi_ports[ext].sysbuff[1] == 0x7Fu) &&
	      (csysi_ports[ext].sysbuff[2] == 0x09u) &&
	      ((csysi_ports[ext].sysbuff[3] == 0x01u) ||
	       (csysi_ports[ext].sysbuff[3] == 0x02u)))
	    {
	      mdata[(*len)++] = CSYS_MIDI_GMRESET;
	      mdata[(*len)++] = csysi_ports[ext].sysbuff[3];
	      if ((p->length == (*idx)) || ((*len) > (CSYSI_SENDSIZE - 3)))
		return CSYSI_COMMAND_COMPLETED;
	    }
	  break;
	case 6:   
	  if ((csysi_ports[ext].sysbuff[0] == 0x7Fu) &&
	      (csysi_ports[ext].sysbuff[1] == 0x7Fu) &&
	      (csysi_ports[ext].sysbuff[2] == 0x04u) &&
	      (csysi_ports[ext].sysbuff[3] == 0x01u) &&
	      (csysi_ports[ext].sysbuff[4] < 0x80u)  &&
	      (csysi_ports[ext].sysbuff[5] < 0x80u))
	    {
	      mdata[(*len)++] = CSYS_MIDI_MVOLUME;
	      mdata[(*len)++] = csysi_ports[ext].sysbuff[4];
	      mdata[(*len)++] = csysi_ports[ext].sysbuff[5];
	      if ((p->length == (*idx)) || ((*len) > (CSYSI_SENDSIZE - 3)))
		return CSYSI_COMMAND_COMPLETED;
	    }
	  break;
	case 8:   
	  if ((csysi_ports[ext].sysbuff[0] == 0x43u) &&
	      (csysi_ports[ext].sysbuff[1] == 0x73u) &&
	      (csysi_ports[ext].sysbuff[2] == 0x7Fu) &&
	      (csysi_ports[ext].sysbuff[3] == 0x32u) &&
	      (csysi_ports[ext].sysbuff[4] == 0x11u) &&
	      (csysi_ports[ext].sysbuff[5] < 0x80u) &&
	      (csysi_ports[ext].sysbuff[6] < 0x80u) &&
	      (csysi_ports[ext].sysbuff[7] < 0x80u))
	    {
	      mdata[(*len)++] = CSYS_MIDI_MANUEX;
	      mdata[(*len)++] = csysi_ports[ext].sysbuff[6];
	      mdata[(*len)++] = csysi_ports[ext].sysbuff[7];
	      if ((p->length == (*idx)) || ((*len) > (CSYSI_SENDSIZE - 3)))
		return CSYSI_COMMAND_COMPLETED;
	    }
	  break; 
	}
	csysi_ports[ext].sysex_size = 0;
	break;                               /* while */
      }
    else
      {
	if (csysi_ports[ext].sysex_size < CSYSI_SYSEXSIZE)
	  csysi_ports[ext].sysbuff[csysi_ports[ext].sysex_size++] =
	    p->data[*idx];
	(*idx)++;
      }

  return CSYSI_COMMAND_ONGOING;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*                 low-level functions                          */
/*______________________________________________________________*/




