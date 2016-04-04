
/*
#    Sfront, a SAOL to C translator    
#    This file: hpux audio driver for sfront
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
/*             HPUX/9.0 audio driver for sfront                 */ 
/*                                                              */
/****************************************************************/
        
#include <fcntl.h>
#include <sys/socket.h>
#include <audio/Alib.h>


#if defined(ASYS_HASOUTPUT)

/* global variables for HPUX audio output */

Audio *asyso_a;                  /* audio ptr                     */
ATransID asyso_x;                /* audio stream                  */
int asyso_s;                    /* socket for audio stream       */
int asyso_pflag;                /* flag for hardware head start  */
int asyso_pidx;                 /* number of bytes of head start */
int asyso_size;                 /* number of shorts in a buffer  */
short * asyso_buf;               /* location for output buffer    */
 
#endif

#if defined(ASYS_HASINPUT)

Audio    *asysi_a;      /* audio ptr                     */
ATransID asysi_x;       /* audio stream                  */
int     asysi_s;       /* socket for audio stream       */
int     asysi_size;    /* number of shorts in a buffer  */
short    *asysi_buf;    /* location for output buffer    */
fd_set   asysi_selFD;   /* select file descriptor        */
fd_set   asysi_redFD;   /* ready file descriptor         */
int     asysi_stdFD;   /* stdin file descriptor         */
int     asysi_maxFD;   /* max-numbered file descriptor  */

#endif


#if defined(ASYS_HASOUTPUT)

/****************************************************************/
/*        sets up audio system for a given srate/channels       */
/****************************************************************/

int asyso_setup(int srate, int ochannels, int osize, char * oname,
		int toption)

{
  AudioAttributes aa;
  SSPlayParams sp;    
  SStream as; 
  AGainEntry g[2];

  asyso_size = osize;
  asyso_buf = (short *)calloc(osize, sizeof(short));

  asyso_a = AOpenAudio("", NULL);
  aa = *ABestAudioAttributes(asyso_a);

  aa.attr.sampled_attr.data_format = ADFLin16;
  aa.attr.sampled_attr.bits_per_sample = 16;
  aa.attr.sampled_attr.sampling_rate = srate;
  aa.attr.sampled_attr.channels = ochannels;
  aa.attr.sampled_attr.interleave = 1;
  aa.attr.sampled_attr.duration.type = ATTFullLength;


  if (ochannels > 2)
    return ASYS_ERROR;

  if (ochannels == 1)
    {
      g[0].gain = AUnityGain;
      g[0].u.o.out_ch = AOCTMono;
      g[0].u.o.out_dst = AODTMonoJack;
    }
  else
    {
      g[0].gain = g[1].gain = AUnityGain;
      g[0].u.o.out_ch = AOCTLeft;
      g[1].u.o.out_ch = AOCTRight;
      g[0].u.o.out_dst = AODTLeftLineOut;
      g[1].u.o.out_dst = AODTRightLineOut;
    }

  sp.gain_matrix.type = AGMTOutput;
  sp.gain_matrix.num_entries = ochannels;
  sp.gain_matrix.gain_entries = g;
  sp.play_volume = AUnityGain;   

  if (toption == ASYS_RENDER)
    sp.priority = APriorityHigh; 
  else
    sp.priority = APriorityUrgent; 

  sp.event_mask = 0;             

  asyso_x = APlaySStream(asyso_a, ~0, &aa, &sp , &as, NULL);
  if ((asyso_s = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    return ASYS_ERROR;
  if (connect(asyso_s, (struct sockaddr *)&as.tcp_sockaddr,
		   sizeof(struct sockaddr_in)) < 0)
    return ASYS_ERROR;

  if (toption == ASYS_RENDER)
    {
      asyso_pidx = 3 * 2 * ochannels * srate;
      asyso_pflag = 1;
      APauseAudio(asyso_a, asyso_x, NULL, NULL);
    }
  else
    asyso_pflag = asyso_pidx = 0;
  return ASYS_DONE;
}

#endif


#if defined(ASYS_HASINPUT)

/****************************************************************/
/*              initializes audio input                         */
/****************************************************************/

int asysi_setup(int srate, int ichannels, int isize, char * iname)

{
  AudioAttributes rr;
  SStream sr;
  SSRecordParams  asr;
  AGainEntry g[2];

  asysi_size = isize;
  asysi_buf = (short *)malloc(sizeof(short)*isize);

  asysi_a = AOpenAudio("", NULL);
  rr = *aBestAudioAttributes(asysi_a);   

  rr.attr.sampled_attr.data_format = ADFLin16;
  rr.attr.sampled_attr.bits_per_sample = 16;
  rr.attr.sampled_attr.sampling_rate = srate;
  rr.attr.sampled_attr.channels = ichannels;
  rr.attr.sampled_attr.interleave = 1;

  if (ichannels > 2)
    return ASYS_ERROR;

  if (ichannels == 1)
    {
      g[0].u.i.in_ch = AICTMono;
      g[0].gain = AUnityGain;
      g[0].u.i.in_src = AISTMonoAuxiliary;
      g[0].u.i.in_src = AISTMonoMicrophone;
    }
  else
    {
      g[0].u.i.in_ch = AICTLeft;
      g[0].gain = AUnityGain;
      g[0].u.i.in_src = AISTLeftAuxiliary;
      g[0].u.i.in_src = AISTLeftMicrophone;
      g[1].u.i.in_ch = AICTRight;
      g[1].gain = AUnityGain;
      g[1].u.i.in_src = AISTRightAuxiliary;
      g[1].u.i.in_src = AISTRightMicrophone;
    }

  asr.gain_matrix.type = AGMTInput;
  asr.gain_matrix.num_entries = ichannels;
  asr.gain_matrix.gain_entries = g;
  asr.record_gain = AUnityGain;  
  asr.event_mask = 0;

  if ((asysi_s = socket( AF_INET, SOCK_STREAM, 0 )) < 0)
    return ASYS_ERROR;
  asysi_x = ARecordSStream(asysi_a, ~0, &rr, &asr,
			&sr, NULL );
  if (connect(asysi_s,(struct sockaddr *)&sr.tcp_sockaddr,
		   sizeof(struct sockaddr_in)) < 0)
    return ASYS_ERROR;

  asysi_maxFD = asysi_stdFD = fileno(stdin);
  if (asysi_s > asysi_maxFD)
    asysi_maxFD = asysi_s;
  FD_ZERO(&asysi_redFD);
  FD_ZERO(&asysi_selFD);
  FD_SET(asysi_stdFD, &asysi_selFD);
  FD_SET(asysi_s, &asysi_selFD);
  
  fprintf(stderr, "Hit RETURN to stop recording\n" );
  return ASYS_DONE;
}

#endif


#if (!defined(ASYS_HASOUTPUT) && defined(ASYS_HASINPUT))

/****************************************************************/
/*              initializes audio input                         */
/****************************************************************/

int asys_isetup(int srate, int ichannels, int isample, 
                char * iname, int toption)

{
  return asysi_setup(srate, ichannels, ASYS_ICHAN*ACYCLE, iname);
}

#endif

#if (defined(ASYS_HASOUTPUT) && !defined(ASYS_HASINPUT))

/****************************************************************/
/*        sets up audio system for a given srate/channels       */
/****************************************************************/

int asys_osetup(int srate, int ochannels, int osample, 
                char * oname, int toption)

{
  return asyso_setup(srate, ochannels, ASYS_OCHAN*ACYCLE, oname, toption);
}

#endif 

#if (defined(ASYS_HASOUTPUT) && defined(ASYS_HASINPUT))

/****************************************************************/
/*        initializes bidirectional audio flows                 */
/****************************************************************/

int asys_iosetup(int srate, int ichannels, int ochannels,
                 int isample, int osample, 
                 char * iname, char * oname, int toption)


{
  if (asyso_setup(srate, ochannels, osize, ASYS_OCHAN*ACYCLE, 
		  toption) != ASYS_DONE)
    return ASYS_ERROR;
  return asysi_setup(srate, ichannels, ASYS_ICHAN*ACYCLE, iname);
}

#endif


#if defined(ASYS_HASINPUT)

/****************************************************************/
/*              core routine -- shuts down audio input device   */
/****************************************************************/

void asysi_shutdown(void)

{
  close(asysi_s);
  ACloseAudio(asysi_a, NULL);
}

#endif


#if defined(ASYS_HASOUTPUT)

/****************************************************************/
/*            core routine -- shuts down audio output           */
/****************************************************************/

void asyso_shutdown(void)

{
  if (asyso_pflag) 
    AResumeAudio(asyso_a, asyso_x, NULL, NULL);
  close(asyso_s);
  ASetCloseDownMode(asyso_a, AKeepTransactions, NULL);
  ACloseAudio(asyso_a, NULL);
}

#endif


#if (defined(ASYS_HASOUTPUT)&&(!defined(ASYS_HASINPUT)))

/****************************************************************/
/*                    shuts down audio output                   */
/****************************************************************/

void asys_oshutdown(void)

{
  asyso_shutdown();
}

#endif

#if (!defined(ASYS_HASOUTPUT)&&(defined(ASYS_HASINPUT)))

/****************************************************************/
/*              shuts down audio input device                   */
/****************************************************************/

void asys_ishutdown(void)

{
  asysi_shutdown();
}

#endif

#if (defined(ASYS_HASOUTPUT)&&(defined(ASYS_HASINPUT)))

/****************************************************************/
/*              shuts down audio input and output device        */
/****************************************************************/

void asys_ioshutdown(void)

{
  asysi_shutdown();
  asyso_shutdown();
}

#endif



#if defined(ASYS_HASOUTPUT)
  
/****************************************************************/
/*        creates buffer, and generates starting silence        */
/****************************************************************/

int asys_preamble(ASYS_OTYPE * asys_obuf[], int * osize)

{
  int i;

  *asys_obuf = asyso_buf;
  *osize = asyso_size;
  return ASYS_DONE;
}


/****************************************************************/
/*               sends one frame of audio to output             */
/****************************************************************/

int asys_putbuf(ASYS_OTYPE * asys_obuf[], int * osize)

{
  int bsize, bwrite;
  char * buf;

  bsize = 2*(*osize);
  buf = (char *)(asyso_buf);
  while (bsize) 
    {
      if ((bwrite = write(asyso_s, buf, bsize )) < 0)
	return ASYS_ERROR;
      buf += bwrite;
      bsize -= bwrite;
      if ((asyso_pflag)&&(((asyso_pidx -= bwrite) <= 0)||(bwrite == 0)))
	{
	  AResumeAudio(asyso_a, asyso_x, NULL, NULL);
	  asyso_pflag = 0;
	}
    }
  return ASYS_DONE;
}

#endif

#if defined(ASYS_HASINPUT)

/****************************************************************/
/*               gets one frame of audio from input             */
/****************************************************************/

int asys_getbuf(ASYS_ITYPE * asys_ibuf[], int * isize)

{

  if (*asys_ibuf == NULL)
    {
      *asys_ibuf = asysi_buf;
      *isize = asysi_size;
    }

  if ((*isize) == 0)
    return ASYS_DONE;

  while (1)
    {
      asysi_redFD = asysi_selFD;

      if (select(asysi_maxFD + 1, (int *)&asysi_redFD, (int *)NULL,
		      (int *)NULL, NULL) < 0)
	return ASYS_ERROR;
    
      if (FD_ISSET(asysi_s, &asysi_redFD))
	{
	  *isize = read(asysi_s, (char *) asysi_buf, 2*asysi_size);
	  *isize = (*isize)/2;
	  if (*isize < 0)
	    return ASYS_ERROR;
	  else
	    return ASYS_DONE;
	}
    
      if (FD_ISSET(asysi_stdFD, &asysi_redFD)) 
	{
	  FD_CLR(asysi_stdFD, &asysi_selFD);
	  AStopAudio(asysi_a, asysi_x, ASMThisTrans,
		     (ATransStatus *)NULL, NULL);
	  *isize = 0;
	  return ASYS_DONE;
	}
    }
}

#endif









