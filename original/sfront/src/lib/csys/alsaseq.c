
/*
#    Sfront, a SAOL to C translator    
#    This file: ALSA Sequencer Control driver for Sfront.
#
#    This driver was originaly based on the alsamidi driver, but has
#    been almost completely rewritten.
#
#    Copyright (C) 2001  Enrique Robledo Arnuncio
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
/* Linux ALSA /dev/snd/seq control driver for sfront            */
/* See the alsaseq README file included in the sfman for usage  */
/* tips and some configuration issues.                          */
/* Specific command line options:                               */
/*                                                              */
/*      -csys_alsaseq_subscribe client:port [client:port ...]   */
/*                                                              */
/****************************************************************/

#undef CSYSI_DEBUG

/* Configuration for multiple connections */
#define CSYSI_MAX_DEVS 32
#define CSYSI_MAP2_BITS 6
#define CSYSI_MASK1  0xFFC0

/* Driver info */
snd_seq_t *csysi_handle;
int        csysi_port;
unsigned short csysi_self_addr;

/* ALSA address <-> channel device number associative tables interface */
void csysi_init_maps();
int  csysi_new_ext_channel_device(unsigned short  alsa_addr);
int  csysi_read_ext_channel_device(unsigned short alsa_addr);
void csysi_remove_ext_channel_device(unsigned short alsa_addr);


/****************************************************************/
/*             initialization routine for control               */
/****************************************************************/

int csysi_parse_address(const char* str)
{
  const char *port = NULL;
  int c,p;
  const char* pt;
  
  if (str==NULL)
    return -1;
  for (pt=str; *pt!=0; pt++) {
    if (*pt == ':' || *pt == '.')
      if (port==NULL) port = pt+1;
      else return -1;
    else
      if (!isdigit(*pt))
	return -1;
  }
  if (port == NULL)
    return -1;
  c = atoi(str);
  p = atoi(port);
  if (c<0 || c>255 || p<0 || p>255)
    return -1;
  return c<<8|p;
}

int csys_setup(void)
{
  int res,arg,addr;
  
  /* Driver connection */
  if ((res = snd_seq_open(&csysi_handle, SND_SEQ_OPEN_IN)) < 0) {
    fprintf(stderr, "ALSASEQ: Open failed: %s\n", snd_strerror(res));
    return CSYS_ERROR;
  }
  if ((res = snd_seq_set_client_name(csysi_handle,
				     "Sfront SA Synthesizer")) < 0) {
    fprintf(stderr, "ALSASEQ: Could not set name: %s\n",
	    snd_strerror(res));
    snd_seq_close(csysi_handle);
    return CSYS_ERROR;
  }
  if ((res = snd_seq_create_simple_port(csysi_handle,
					"MIDI input",
					SND_SEQ_PORT_CAP_WRITE | 
					SND_SEQ_PORT_CAP_SUBS_WRITE,
					SND_SEQ_PORT_TYPE_SYNTH)) < 0) {
    fprintf(stderr, "ALSASEQ: Port creation failed: %s\n",
	    snd_strerror(res));
    snd_seq_close(csysi_handle);
    return CSYS_ERROR;
  }
  csysi_port = res;
  csysi_self_addr = snd_seq_client_id(csysi_handle) << 8 + res;
  
  /* Extended channel mapping tables initialization */
  csysi_init_maps();
  
  /* Command line processing */
  for (arg=1; arg<EV(csys_argc); arg++) {
    if (strcmp(EV(csys_argv[arg]),"-csys_alsaseq_subscribe"))
      continue;
    while ((arg+1)<EV(csys_argc) && EV(csys_argv[arg+1])[0] != '-') {
      arg++;
      if ((addr = csysi_parse_address(EV(csys_argv[arg]))) < 0) {
	fprintf(stderr,"ALSASEQ: Warning: Invalid ALSA address: %s\n\n",
		EV(csys_argv[arg]));
	continue;
      }
      if (csysi_new_ext_channel_device(addr) < 0) {
	fprintf(stderr,"ALSASEQ: Warning: Ignored subscription to %s:\n"
		"         Too many extended channel devices in use.\n\n",
		EV(csys_argv[arg]));
	continue;
      }
      if ((res = snd_seq_connect_from(csysi_handle,
				      csysi_port,
				      (addr&0xFF00)>>8,
				      addr&0x00FF)) < 0) {
	csysi_remove_ext_channel_device(addr);
	fprintf(stderr,
		"ALSASEQ: Warning: Could not connect"
		"to the given address: %s\n\n",
		EV(csys_argv[arg]));
      }
    }
  }
  return CSYS_DONE;
}

/****************************************************************/
/*             polling routine for new data                     */
/****************************************************************/

int csys_newdata(void)
     
{
  int len;
  
  len = snd_seq_event_input_pending(csysi_handle,1);
  if (len <= 0)
    return CSYS_NONE;
  return CSYS_MIDIEVENTS;
}


/****************************************************************/
/*                 processes a MIDI event                       */
/****************************************************************/

int csys_midievent(unsigned char * cmd,
		   unsigned char * ndata, 
		   unsigned char * vdata, 
		   unsigned short * extchan,
		   float * fval)
     
{
  snd_seq_event_t *event;
  int len,res,device;
  unsigned short channel;
  unsigned short alsa_address;
  
  res = snd_seq_event_input(csysi_handle,&event);
  if (res<0) {
    *cmd = CSYS_MIDI_NOOP;
    return CSYS_DONE;
  }
  
  /* First, we check where does the event come from */
  alsa_address = event->source.client << 8 + event->source.port;
  
  if (alsa_address == csysi_self_addr) {
    switch (event->type) {
    case SND_SEQ_EVENT_PORT_USED:
      alsa_address = event->data.addr.client << 8 + 
	event->data.addr.port;
      device = csysi_new_ext_channel_device(alsa_address);
#ifdef CSYSI_DEBUG
      if (device < 0)
	fprintf(stderr,"ALSASEQ: Could not map %d:%d:"
		" no extended devices left\n",
		event->data.addr.client,
		event->data.addr.port);
      else
	fprintf(stderr,"ALSASEQ: %d:%d mapped to extended device %d\n",
		event->data.addr.client,
		event->data.addr.port,
		device);
#endif				
      break;
    }
    *cmd = CSYS_MIDI_NOOP;
    if ( snd_seq_event_input_pending(csysi_handle,1) <= 0)
      return CSYS_NONE;
    return CSYS_EVENTS;
  }
  
  device = csysi_read_ext_channel_device(alsa_address);
  if (device<0) {	/* Unregistered source. Should not happen. */
    *cmd = CSYS_MIDI_NOOP;
    if ( snd_seq_event_input_pending(csysi_handle,1) <= 0)
      return CSYS_NONE;
    return CSYS_EVENTS;
  }
  
  switch (event->type) {
  case SND_SEQ_EVENT_NOTEON:
    *cmd = CSYS_MIDI_NOTEON;
    *ndata = event->data.note.note;
    *vdata = event->data.note.velocity;
    channel = event->data.note.channel;
    break;
  case SND_SEQ_EVENT_NOTEOFF:
    *cmd = CSYS_MIDI_NOTEOFF;
    *ndata = event->data.note.note;
    channel = event->data.note.channel;
    break;
  case SND_SEQ_EVENT_KEYPRESS: /* Not tested */
    *cmd = CSYS_MIDI_PTOUCH;
    *ndata = event->data.note.note;
    *vdata = event->data.note.velocity;
    channel = event->data.note.channel;
    break;
  case SND_SEQ_EVENT_CONTROLLER:
    *cmd = CSYS_MIDI_CC;
    *ndata = event->data.control.param;
    *vdata = event->data.control.value;
    channel = event->data.control.channel;
    break;
  case SND_SEQ_EVENT_PGMCHANGE:
    *cmd = CSYS_MIDI_PROGRAM;
    *ndata = event->data.control.value;
    channel = event->data.control.channel;
    break;
  case SND_SEQ_EVENT_CHANPRESS:
    *cmd = CSYS_MIDI_CTOUCH;
    *ndata = event->data.control.value;
    channel = event->data.control.channel;
    break;
  case SND_SEQ_EVENT_PITCHBEND:
    *cmd = CSYS_MIDI_WHEEL;
    *ndata =  event->data.control.value &  0x007F;
    *vdata = (event->data.control.value & (0x007F << 7) ) >> 7;
    channel = event->data.control.channel;
    break;
  case SND_SEQ_EVENT_TEMPO: /* ??? Not tested. */
    *cmd = CSYS_MIDI_NEWTEMPO;
    *fval = (float)event->data.queue.param.value;
    break;
  case SND_SEQ_EVENT_PORT_UNUSED:
    csysi_remove_ext_channel_device(alsa_address);
#ifdef CSYSI_DEBUG
    fprintf(stderr,"ALSASEQ: %d:%d disconected from device %d\n",
	    event->data.addr.client,
	    event->data.addr.port,
	    device);
#endif
    break;
    /* No MIDI_ENDTIME event in alsa? */
  default:
    *cmd = CSYS_MIDI_NOOP;
  }
  
  *extchan = device * 16 + channel;
  
  len = snd_seq_event_input_pending(csysi_handle,1);
  if (len <= 0)
    return CSYS_NONE;
  return CSYS_EVENTS;
}

/****************************************************************/
/*                  closing routine for control                 */
/****************************************************************/

void csys_shutdown(void)
{
  snd_seq_close(csysi_handle);
}


/****************************************************************/
/*        ALSA address to extended channel device mapping       */
/****************************************************************/


/* New alsa subscriptions to our port are asigned an "extended device
 * number" (dev). Extended channels will be calculated as:
 *     extchan = dev*16 + MIDI_channel
 * The structures and functions below do the mapping between alsa
 * addresses and extended device number, using an asociative table
 * mechanism. A single level of indirection would require a 64Kb
 * table, so we use two levels of indirection.
 */

#define CSYSI_MASK2 ~CSYSI_MASK1
#define CSYSI_SIZE1 ( 1 << (16-CSYSI_MAP2_BITS) )
#define CSYSI_SIZE2 ( 1 << CSYSI_MAP2_BITS )


unsigned char csysi_is_used_dev[CSYSI_MAX_DEVS];
unsigned char csysi_available_dev;
unsigned short csysi_used_devs;
unsigned char csysi_map1[CSYSI_SIZE1];
unsigned char csysi_map2[CSYSI_MAX_DEVS][CSYSI_SIZE2];
unsigned char csysi_map2_counts[CSYSI_MAX_DEVS];
unsigned char csysi_available_page;


/* Initialize Address mapping structures */
void csysi_init_maps()
{
  int i,j;
  
  for (i=0; i<CSYSI_SIZE1; i++)
    csysi_map1[i]=0xFF;
  for (i=0; i<CSYSI_MAX_DEVS; i++) {
    csysi_is_used_dev[i]=0;
    csysi_map2_counts[i]=0;
    for (j=0; j<CSYSI_SIZE2; j++)
      csysi_map2[i][j]=0xFF;
  }
  csysi_used_devs = 0;
  csysi_available_dev = 0;
  csysi_available_page = 0;
}

/* Get extended channel device number for an alsa address.  
 * Returns the extended channel number, or -1 if the address was not
 * mapped. */
int csysi_read_ext_channel_device(unsigned short addr)
{
  unsigned short idx1,idx2_1,idx2_2;

  idx1 = ( addr &  CSYSI_MASK1 ) >> CSYSI_MAP2_BITS;
  idx2_1 = csysi_map1[idx1];
  idx2_2 = ( addr & CSYSI_MASK2 );
  if ( idx2_1 > CSYSI_MAX_DEVS-1 || 
       csysi_map2[idx2_1][idx2_2] > CSYSI_MAX_DEVS-1 )
    return -1;
  return csysi_map2[idx2_1][idx2_2];
}

/* Add an ALSA address to the asociative tables.  Returns <0 if there
 * was no space left for the new address. Otherwhise, the address is
 * mapped to the first available device number, which is returned.
 * NOTE: In the worst case, the loops used to update the "available
 * page" and the "available device" pointers can have a length of
 * almost CSYSI_MAX_DEVS. This would happen in a system with most of
 * the available pages/devices used, where a page/device near the
 * begginig is freed, and then a new page/device is requested.
 * Hopefuly this can be asumed to be infrecuent enough, so we can
 * avoid using a more complicated mechanism to track free
 * pages/devices */
int csysi_new_ext_channel_device(unsigned short addr)
{
  unsigned short idx1,idx2_1,idx2_2,dev;

  if (csysi_used_devs > CSYSI_MAX_DEVS-1)
    return -1;
  
  idx1 = ( addr & CSYSI_MASK1 ) >> CSYSI_MAP2_BITS;
  idx2_1 = csysi_map1[idx1];
  idx2_2 = ( addr & CSYSI_MASK2 );
  if ( idx2_1 < CSYSI_MAX_DEVS &&
       csysi_map2[idx2_1][idx2_2] < CSYSI_MAX_DEVS )
    return csysi_map2[idx2_1][idx2_2];
  
  if (idx2_1 > CSYSI_MAX_DEVS-1) {
    idx2_1 = csysi_map1[idx1] = csysi_available_page;
    while (csysi_map2_counts[++csysi_available_page])
      if (csysi_available_page==CSYSI_MAX_DEVS)
	break;
  }
  
  dev = csysi_map2[idx2_1][idx2_2] = csysi_available_dev;
  csysi_used_devs++;
  csysi_map2_counts[idx2_1]++;
  csysi_is_used_dev[csysi_available_dev] = 1;
  while (csysi_is_used_dev[++csysi_available_dev])
    if (csysi_available_dev==CSYSI_MAX_DEVS)
      break;
  
  return dev;
}

/* Removes an ALSA address to the asociative tables. */
void csysi_remove_ext_channel_device(unsigned short addr)
{
  unsigned short idx1,idx2_1,idx2_2,dev;
  
  idx1 = ( addr & CSYSI_MASK1 ) >> CSYSI_MAP2_BITS;
  idx2_1 = csysi_map1[idx1];
  idx2_2 = ( addr & CSYSI_MASK2 );
  if ( idx2_1 > CSYSI_MAX_DEVS-1 || 
       csysi_map2[idx2_1][idx2_2] > CSYSI_MAX_DEVS-1 ||
       !csysi_map2_counts[idx2_1] ||
       !csysi_used_devs )
    return;
  
  dev = csysi_map2[idx2_1][idx2_2];
  csysi_map2[idx2_1][idx2_2] = 0xFF;
  csysi_is_used_dev[dev]=0;
  csysi_used_devs--;
  if (dev < csysi_available_dev)
    csysi_available_dev = dev;
  
  if (--csysi_map2_counts[idx2_1]==0) {
    csysi_map1[idx1] = 0xFF;
    if (idx2_1 < csysi_available_page)
      csysi_available_page = idx2_1;
  }
}
