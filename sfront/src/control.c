
/*
#    Sfront, a SAOL to C translator    
#    This file: Handles csys interfaces
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

/****************************************************************/
/* types of control devices -- add your driver to the end       */
/*                and update CDRIVER_END.                       */
/****************************************************************/

#define CDRIVER_NONE         0   /* must always be zero */
#define CDRIVER_GLISS        1
#define CDRIVER_ASCII        2
#define CDRIVER_WIN32        3
#define CDRIVER_LINMIDI      4
#define CDRIVER_ALSAMIDI     5
#define CDRIVER_ALSASEQ      6
#define CDRIVER_FREEBSDMIDI  7
#define CDRIVER_FSTR         8
#define CDRIVER_COREMIDI     9
#define CDRIVER_AUCONTROL    10
#define CDRIVER_AUCONTROLM   11
#define CDRIVER_END          12

/****************************************************************/
/*             prints help screen for control options           */
/****************************************************************/

void printcontrolhelp(void)

{

  printf("Where C output binary receives control information:\n");
  printf("       [-cin linmidi]       Linux MIDI OSS (sndcard MIDI IN)\n");
  printf("       [-cin alsamidi]      Linux MIDI ALSA (sndcard MIDI IN)\n");
  printf("       [-cin alsaseq]       Linux ALSA Sequencer\n");
  printf("       [-cin coremidi]      Mac OS X CoreMIDI (external only)\n");
  printf("       [-cin aucontrol]     Mac OS X AudioUnit parameter driver\n");
  printf("       [-cin aucontrolm]    Mac OS X AudioUnit parameter/MIDI driver\n");
  printf("       [-cin freebsdmidi]   FreeBSD MIDI (sndcard MIDI IN)\n");
  printf("       [-cin win32]         Win32 MIDI driver (sndcard MIDI IN)\n");
  printf("       [-cin gliss]         glissando driver (for testing)\n");
  printf("       [-cin ascii]         ASCII keyboard mono MIDI driver \n");
  printf("       [-cin fstr]          MP4 file streamer (use with -bitc option)\n");

}

/****************************************************************/
/*             type-checks -aout filename/device                */
/****************************************************************/

int cinfilecheck(char * fname)

{
  cinname = dupval(fname);

  /* first, non-filename entries */

  if (!strcmp(fname,"alsamidi"))
    {
      cin = CDRIVER_ALSAMIDI;
      csasl = 0;            /* driver sends sasl events */
      cmidi = 1;            /* driver sense midi events */
      cmaxchan = 16;        /* can use all 16 MIDI channels */
      clatency =            /* requires low latencies */
	LOW_LATENCY_DRIVER; 
      return 0;
    }

  if (!strcmp(fname,"alsaseq"))
    {
      cin = CDRIVER_ALSASEQ;
      csasl = 0;             /* driver sends sasl events */
      cmidi = 1;             /* driver sense midi events */
      cmaxchan = 32*16;      /* can use all 16 MIDI channels */
      clatency =             /* requires low latencies */
	LOW_LATENCY_DRIVER; 
      return 0;
    }

  if (!strcmp(fname,"ascii"))
    {
      cin = CDRIVER_ASCII;
      csasl = 0;            /* driver sends sasl events */
      cmidi = 1;            /* driver sense midi events */
      cmaxchan = 16;        /* set to maximum extended channel + 1 */
      clatency =            /* requires low latencies */
	LOW_LATENCY_DRIVER; 
      return 0;
    }

  if (!strcmp(fname,"coremidi"))
    {
      cin = CDRIVER_COREMIDI;
      csasl = 0;            /* driver sends sasl events */
      cmidi = 1;            /* driver sense midi events */
      cmaxchan = 4*16;      /* set to maximum extended channel + 1 */
      clatency =            /* requires low latencies */
	LOW_LATENCY_DRIVER; 
      return 0;
    }

  if (!strcmp(fname,"aucontrol"))
    {
      cin = CDRIVER_AUCONTROL;
      csasl = 1;            /* driver sends sasl events */
      cmidi = 0;            /* driver ignores midi events */
      clatency =            /* requires low latencies */
	LOW_LATENCY_DRIVER; 
      creentrant = 1;
      return 0;
    }

  if (!strcmp(fname,"aucontrolm"))
    {
      cin = CDRIVER_AUCONTROLM;
      csasl = 1;            /* driver sends sasl events */
      cmidi = 1;            /* driver sense midi events */
      cmaxchan = 16;        /* set to maximum extended channel + 1 */
      clatency =            /* requires low latencies */
	LOW_LATENCY_DRIVER; 
      creentrant = 1;
      return 0;
    }

  if (!strcmp(fname,"freebsdmidi"))
    {
      cin = CDRIVER_FREEBSDMIDI;
      csasl = 0;            /* driver sends sasl events */
      cmidi = 1;            /* driver sense midi events */
      cmaxchan = 16;        /* can use all 16 MIDI channels */
      clatency =            /* requires low latencies */
	LOW_LATENCY_DRIVER; 
      return 0;
    }

  if (!strcmp(fname,"fstr"))
    {
      cin = CDRIVER_FSTR;
      csasl = 1;            /* driver sends sasl events */
      cmidi = 1;            /* driver sense midi events */
      cmaxchan = 16;        /* set to maximum extended channel + 1 */
      clatency =            /* doesn't require low latencies */
	HIGH_LATENCY_DRIVER; 
      return 0;
    }

  if (!strcmp(fname,"gliss"))
    {
      cin = CDRIVER_GLISS;
      csasl = 1;            /* driver sends sasl events */
      cmidi = 1;            /* driver sense midi events */
      cmaxchan = 3;         /* set to maximum extended channel + 1 */
      clatency =            /* doesn't require low latencies */
	HIGH_LATENCY_DRIVER; 
      return 0;
    }

  if (!strcmp(fname,"linmidi"))
    {
      cin = CDRIVER_LINMIDI;
      csasl = 0;            /* driver sends sasl events */
      cmidi = 1;            /* driver sense midi events */
      cmaxchan = 16;        /* 16 local MIDI channels   */
      clatency =            /* requires low latencies */
	LOW_LATENCY_DRIVER; 
      return 0;
    }

  if (!strcmp(fname,"win32"))
    {
      cin = CDRIVER_WIN32;
      csasl = 0;            /* driver sends sasl events */
      cmidi = 1;            /* driver sense midi events */
      cmaxchan = 16;        /* can use all 16 MIDI channels */
      clatency =            /* requires low latencies */
	LOW_LATENCY_DRIVER; 
      return 0;
    }


  /* then, filename entries */

  /*
  if (strstr(fname,".raw") != NULL)
    {
      cin = DRIVER_RAW;
      return 0;
    }
   */

  return 1;
}

/****************************************************************/
/*             returns name of C file for driver                */
/****************************************************************/

void makecontroldriver(int anum)

{

  switch (anum) {
  case CDRIVER_ALSAMIDI:
    makealsamidi();
    break;
  case CDRIVER_ALSASEQ:
    makealsaseq();
    break;
  case CDRIVER_ASCII:
    makeascii();
    break;
  case CDRIVER_COREMIDI:
    makecoremidi();
    break;
  case CDRIVER_AUCONTROL:
  case CDRIVER_AUCONTROLM:
    makeaucontrol();
    break;
  case CDRIVER_FREEBSDMIDI:
    makefreebsdmidi();
    break;
  case CDRIVER_FSTR:
    makefstr();
    break;
  case CDRIVER_GLISS:
    makegliss();
    break;
  case CDRIVER_LINMIDI:
    makelinmidi();
    break;
  case CDRIVER_WIN32:
    makewin32();
    break;
  default:
    internalerror("control.c","makecontroldriver()");
  }

}

/****************************************************************/
/*             generates C code for network driver              */
/****************************************************************/

void makenetworkdriver(void)

{
  fprintf(outfile,"#define NSYS_NETSTART %i\n", netstart);
  fprintf(outfile,"#define NSYS_MSETS %i\n\n", netmsets);
  fprintf(outfile,"#define NSYS_SM_FEC_NONE %i\n", FEC_NONE);
  fprintf(outfile,"#define NSYS_SM_FEC_NOGUARD %i\n", FEC_NOGUARD);
  fprintf(outfile,"#define NSYS_SM_FEC_MINIMAL %i\n", FEC_MINIMAL);
  fprintf(outfile,"#define NSYS_SM_FEC_STANDARD %i\n", FEC_STANDARD);
  fprintf(outfile,"#define NSYS_SM_FEC_EXTRA %i\n", FEC_EXTRA);
  fprintf(outfile,"#define NSYS_SIP_IP \"%s\"\n", sip_ip);
  fprintf(outfile,"#define NSYS_SIP_RTP_PORT %hu\n", sip_port);
  fprintf(outfile,"#define NSYS_SIP_RTCP_PORT %hu\n", sip_port + 1);
  fprintf(outfile,"#define NSYS_MSESSION_INTERVAL %i\n\n", msession_interval);
  fprintf(outfile, "int nsys_feclevel = %i;\n", feclevel);
  fprintf(outfile, "int nsys_lateplay = %i;\n", lateplay);
  fprintf(outfile, "float nsys_latetime = %g;\n", latetime);
  fprintf(outfile,"char * nsys_sessionname = \"%s\";\n", session);

  if (sessionkey)
    fprintf(outfile, "char * nsys_sessionkey = \"%s\";\n\n", sessionkey);
  else
    fprintf(outfile, "char * nsys_sessionkey = NULL;\n\n");

#if (NET_STATUS == HAS_NETWORKING)
    
  makenet_include();
  makenet_sfront();
  makenet_globals();
  makenet_siplib();
  makenet_rtplib();
  makenet_rtcplib();
  makenet_jsend();
  makenet_jrecv();
  makenet_crypto();

#endif

}









