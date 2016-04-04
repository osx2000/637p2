
/*
#    Sfront, a SAOL to C translator    
#    This file: merged linux/freebsd audio driver for sfront
#    Copyright (C) 2000  Bertrand Petit
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


#define ASYSIO_LINUX   0
#define ASYSIO_FREEBSD 1

#define ASYSIO_OSTYPE  ASYSIO_LINUX

/****************************************************************/
/****************************************************************/
/*                linux audio driver for sfront                 */ 
/****************************************************************/

/**************************************************/
/* define flags for fifo mode, and for a timer to */
/* catch SAOL infinite loops                      */
/**************************************************/

#if (defined(ASYS_HASOUTPUT) && (ASYSIO_OSTYPE == ASYSIO_LINUX) && \
     defined(CSYS_CDRIVER_LINMIDI) && (ASYS_TIMEOPTION == ASYS_TIMESYNC) && \
     !defined(ASYS_HASINPUT))
#define ASYSIO_USEFIFO 1
#endif

#if (defined(ASYS_HASOUTPUT) && (ASYSIO_OSTYPE == ASYSIO_LINUX) && \
    (ASYS_TIMEOPTION != ASYS_TIMESYNC))
#define ASYSIO_USEFIFO 1
#endif

#ifndef ASYSIO_USEFIFO
#define ASYSIO_USEFIFO 0
#endif

/**************************************************/
/* include headers, based on flags set above      */
/**************************************************/

#include <unistd.h>
#include <sys/ioctl.h>
#include <fcntl.h>

#if (ASYSIO_OSTYPE == ASYSIO_LINUX)
#include <sys/soundcard.h>
#include <endian.h>
#endif

#if (ASYSIO_OSTYPE == ASYSIO_FREEBSD)
#include <machine/soundcard.h>
#include <machine/endian.h>
#endif

#include <signal.h>  
#include <sys/time.h>  

#if ASYSIO_USEFIFO
#include <sched.h>  
#if (ASYS_TIMEOPTION == ASYS_TIMESYNC)
#include <time.h>
#endif
#endif

/******************************/
/* other constant definitions */
/******************************/

#ifndef ASYSIO_DSPDEV
#define ASYSIO_DSPDEV "/dev/dsp"
#endif

/* determines native audio format */

#if (BYTE_ORDER == BIG_ENDIAN)
# define ASYSIO_AFORMAT AFMT_S16_BE
#else
# if (BYTE_ORDER == LITTLE_ENDIAN)
#  define ASYSIO_AFORMAT AFMT_S16_LE
# else
#  error "BYTE_ORDER not defined?"
# endif
#endif

/* codes for IO types */

#define ASYSIO_I  0
#define ASYSIO_O  1
#define ASYSIO_IO 2

/* minimum fragment size */

#define ASYSIO_FRAGMIN    16
#define ASYSIO_LOGFRAGMIN 4

/* number of silence buffers */

#define ASYSO_LNUMBUFF 4

/* maximum number of I/O retries before termination */

#define ASYSIO_MAXRETRY 256

#if ASYSIO_USEFIFO                      /* SCHED_FIFO constants for ksync()  */

#if (ASYS_TIMEOPTION == ASYS_TIMESYNC)
#define ASYSIO_SYNC_TIMEOUT    5        /* idle time to leave SCHED_FIFO     */
#define ASYSIO_SYNC_ACTIVE     0        /* machine states for noteon timeout */
#define ASYSIO_SYNC_WAITING    1
#define ASYSIO_SYNC_SCHEDOTHER 2
#else
#define ASYSIO_MAXBLOCK ((int)EV(KRATE))*2  /* max wait tor let SCHED_OTHERs run */
#endif

#endif

/************************/
/* variable definitions */
/************************/

int  asysio_fd;                    /* device pointer */
int asysio_srate;                 /* sampling rate */
int asysio_channels;              /* number of channels */
int asysio_size;                  /* # samples in a buffer */
int asysio_bsize;                 /* actual # bytes in a buffer */            
int asysio_requested_bsize;       /* requested # bytes in a buffer */        
int asysio_input;                 /* 1 if ASYSIO */
int asysio_blocktime;             /* time (in bytes) blocked in kcycle */

struct count_info asysio_ptr;      /* for GET{I,O}*PTR  ioctl calls */
struct audio_buf_info asysio_info; /* for GET{I,O}SPACE ioctl calls */

#if defined(ASYS_HASOUTPUT)
short * asyso_buf = NULL;          /* output buffer */
int asysio_puts;                   /* total number of putbufs */
int asysio_reset;                  /* flags an overrun */
#endif

#if defined(ASYS_HASINPUT)
short * asysi_buf = NULL;          /* input buffer */
struct audio_buf_info asysi_info;  /* input dma status */
#endif

sigset_t asysio_iloop_mask;            /* for masking off iloop interrupt */
struct sigaction asysio_iloop_action;  /* for setting up iloop interrupt  */
struct itimerval asysio_iloop_timer;   /* for setting up iloop timer      */

#if ASYSIO_USEFIFO 
int asysio_fifo;                       /* can get into sched_fifo mode */
struct sched_param asysio_fifoparam;   /* param block for fifo mode */
struct sched_param asysio_otherparam;  /* param block for other mode */

#if (ASYS_TIMEOPTION == ASYS_TIMESYNC)

/* state machine variables for noteon timeout */
int    asysio_sync_state;
time_t asysio_sync_waitstart;
extern int csysi_newnote;       /* from linmidi */

#else

/* state to detect long periods w/o blocking */
int asysio_sync_noblock;                /* how many acycles since last block */
struct timespec asysio_sync_sleeptime;  /* time to wait during forced block  */

#endif

#endif

#if defined(ASYS_KSYNC)                      /* ksync() state */
struct count_info asysio_sync_ptr;           
int asysio_sync_target, asysio_sync_incr;    
float asysio_sync_cpuscale;                  
#endif

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*                      shutdown routines                       */
/*______________________________________________________________*/


/****************************************************************/
/*                    shuts down soundcard                      */
/****************************************************************/

void asysio_shutdown(void)

{
  if (ioctl(asysio_fd, SNDCTL_DSP_SYNC, 0) == -1)
    {
      fprintf(stderr, "\nSoundcard Error: SNDCTL_DSP_SYNC Ioctl Problem\n");
      fprintf(stderr, "Errno Message: %s\n\n", strerror(errno));
    }
  close(asysio_fd);

  /* so that a slow exit doesn't trigger timer */

  asysio_iloop_timer.it_value.tv_sec = 0;
  asysio_iloop_timer.it_value.tv_usec = 0;
  asysio_iloop_timer.it_interval.tv_sec = 0;
  asysio_iloop_timer.it_interval.tv_usec = 0;
  
  if (setitimer(ITIMER_PROF, &asysio_iloop_timer, NULL) < 0)
    {
      fprintf(stderr, "\nSoundcard Driver Error:\n\n");
      fprintf(stderr, "  Couldn't set up ITIMER_PROF timer.\n");
      fprintf(stderr, "  Errno Message: %s\n\n", strerror(errno));
    }
}


#if (defined(ASYS_HASOUTPUT)&&(!defined(ASYS_HASINPUT)))

/****************************************************************/
/*                    shuts down audio output                   */
/****************************************************************/

void asys_oshutdown(void)

{
  asysio_shutdown();
  if (asyso_buf != NULL)
    free(asyso_buf);
}

#endif

#if (!defined(ASYS_HASOUTPUT)&&(defined(ASYS_HASINPUT)))

/****************************************************************/
/*                    shuts down audio input                    */
/****************************************************************/

void asys_ishutdown(void)

{
  asysio_shutdown();  
  if (asysi_buf != NULL)
    free(asysi_buf);
}

#endif


#if (defined(ASYS_HASOUTPUT)&&(defined(ASYS_HASINPUT)))

/****************************************************************/
/*              shuts down audio input and output               */
/****************************************************************/

void asys_ioshutdown(void)

{
  asysio_shutdown();
  if (asyso_buf != NULL)
    free(asyso_buf);
  if (asysi_buf != NULL)
    free(asysi_buf);
}

#endif


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*                 initialization routines                      */
/*______________________________________________________________*/


/****************************************************************/
/*          generic error-checking ioctl wrapper                */
/****************************************************************/

#define ASYSIO_IOCTL_CALL(x,y,z)  do { if (ioctl(x,y,z) == -1){\
      fprintf(stderr, "  Error: %s Ioctl Problem\n", #y ); \
      fprintf(stderr, "  Errno Message: %s\n\n", strerror(errno));\
      close(asysio_fd); return ASYS_ERROR;}} while (0)

#define  ASYSIO_ERROR_RETURN(x) do {\
      fprintf(stderr, "  Error: %s.\n", x);\
      fprintf(stderr, "  Errno Message: %s\n\n", strerror(errno));\
      close(asysio_fd);\
      return ASYS_ERROR; } while (0)

#define  ASYSIO_ERROR_RETURN_NOERRNO(x) do {\
      fprintf(stderr, "  Error: %s.\n", x);\
      close(asysio_fd);\
      return ASYS_ERROR; } while (0)


/****************************************************************/
/*               opens the soudcard device                      */
/****************************************************************/

int asysio_opendevice(int dir, int toption)

{

  switch(dir) {
  case ASYSIO_I:
    asysio_fd = open(ASYSIO_DSPDEV, O_RDONLY, 0);
    asysio_input = 1;
    break;
  case ASYSIO_O:
    asysio_fd = open(ASYSIO_DSPDEV, O_WRONLY, 0);
    asysio_input = 0;
    break;
  case ASYSIO_IO:
    asysio_fd = open(ASYSIO_DSPDEV, O_RDWR, 0);
    asysio_input = 1;
    break;
  default:
    fprintf(stderr, "  Error: Unexpected dir parameter value in \n");
    fprintf(stderr, "         asysio_setup.\n\n");
    return ASYS_ERROR;
  }

  if (asysio_fd == -1)
    {
      fprintf(stderr, "  Error: Can't open device %s (%s)\n\n", ASYSIO_DSPDEV,
	      strerror(errno));
      return ASYS_ERROR;
    }
  return ASYS_DONE;

}

/****************************************************************/
/*         signal handler for trapping SAOL infinite loops      */
/****************************************************************/

void asysio_iloop_handler(int signum) 
{   
  fprintf(stderr, "  Error: Either\n\n");
  fprintf(stderr, "    [1] The SAOL program has an infinite loop in it, or\n");
  fprintf(stderr, "    [2] Content is too complex for real-time work.\n\n");
  fprintf(stderr, "  Exiting program ...\n\n");
  exit(0);
}


/****************************************************************/
/*         initializes iloop (heartbeat) interrupt              */
/****************************************************************/

int asysio_initiloop(void)

{

  /*********************************************************/
  /* set up signal handler for infinite-loop (iloop) timer */
  /*********************************************************/
  
  if (sigemptyset(&asysio_iloop_action.sa_mask) < 0)
    ASYSIO_ERROR_RETURN("Couldn't run sigemptyset(iloop) OS call");

  /* infinite-loop timer wins over midi overrun timer */

  if (sigaddset(&asysio_iloop_action.sa_mask, SIGALRM) < 0)
    ASYSIO_ERROR_RETURN("Couldn't run sigaddset(iloop) OS call");

  asysio_iloop_action.sa_flags = SA_RESTART;
  asysio_iloop_action.sa_handler = asysio_iloop_handler;
  
  if (sigaction(SIGPROF, &asysio_iloop_action, NULL) < 0)
    ASYSIO_ERROR_RETURN("Couldn't set up SIGPROF signal handler");


  /************************/
  /* set up timer and arm */
  /************************/

  asysio_iloop_timer.it_value.tv_sec = 3;
  asysio_iloop_timer.it_value.tv_usec = 0;
  asysio_iloop_timer.it_interval.tv_sec = 0;
  asysio_iloop_timer.it_interval.tv_usec = 0;

  if (setitimer(ITIMER_PROF, &asysio_iloop_timer, NULL) < 0)
    ASYSIO_ERROR_RETURN("Couldn't set up ITIMER_PROF timer");

  return ASYS_DONE;
}

/****************************************************************/
/*                 initializes sched_fifo                       */
/****************************************************************/

int asysio_initscheduler(void)

{

#if ASYSIO_USEFIFO

  /*******************************/
  /* set up sched_fifo variables */
  /*******************************/

  memset(&asysio_otherparam, 0, sizeof(struct sched_param));
  memset(&asysio_fifoparam, 0, sizeof(struct sched_param));
 
  if ((asysio_fifoparam.sched_priority =
       sched_get_priority_max(SCHED_FIFO)) < 0)
    ASYSIO_ERROR_RETURN("Couldn't get scheduling priority");

  asysio_fifoparam.sched_priority--;

  /********************************/
  /* try to enter sched-fifo mode */
  /********************************/

  asysio_fifo = !sched_setscheduler(0, SCHED_FIFO, &asysio_fifoparam);

#endif

  return ASYS_DONE;
}

/****************************************************************/
/*                 prints startup screen                        */
/****************************************************************/

int asysio_screenwriter(void)

{
  int i, found;
  int haslinmidi = 0;
  float actual_latency;


  fprintf(stderr, "%s ",  (ASYS_LATENCYTYPE == ASYS_HIGHLATENCY)? 
	  "Streaming" : "Interactive");

  fprintf(stderr, "%s Audio ", (asysio_channels == 2) ? "Stereo" : "Mono");

#if defined(ASYS_HASOUTPUT)
  fprintf(stderr, "%s", asysio_input ? "Input/Output" : "Output");
#else
  fprintf(stderr, "Input");
#endif

  found = i = 0;
  while (i < csys_sfront_argc)
    {
      if (!(strcmp(csys_sfront_argv[i],"-bitc") && 
	    strcmp(csys_sfront_argv[i],"-bit") &&
	    strcmp(csys_sfront_argv[i],"-orc")))
	{
	  i++;
	  fprintf(stderr, " for %s", csys_sfront_argv[i]);
	  found = 1;
	  break;
	}
      i++;
    }
  if (!found)
    fprintf(stderr, " for UNKNOWN");


  i = 0;
  while (i < csys_sfront_argc)
    {
      if (!strcmp(csys_sfront_argv[i],"-cin"))
	{
	  i++;
	  fprintf(stderr, " (-cin %s)", csys_sfront_argv[i]);
	  break;
	}
      i++;
    }
  fprintf(stderr, "\n\n");


#if defined(CSYS_CDRIVER_LINMIDI)

  haslinmidi = 1;

#endif

#if (defined(CSYS_CDRIVER_LINMIDI) || defined(CSYS_CDRIVER_ALSAMIDI)\
      || defined(CSYS_CDRIVER_ALSASEQ) || defined(CSYS_CDRIVER_ASCII))

  /* list midi presets available */

  fprintf(stderr, 
	  "MIDI Preset Numbers (use MIDI controller to select):\n\n");

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
  fprintf(stderr, "\n");

#endif

#if defined(CSYS_CDRIVER_ASCII)

  fprintf(stderr, 
  "To play tunes on ASCII keyboard: a-z for notes, 0-9 for MIDI presets,\n");
  fprintf(stderr, 
  "cntrl-C exits. If autorepeat interferes, exit and run 'xset -r' (in X).\n\n");
  
#endif

  /* diagnose best flags to use, and if they are used */

#ifdef ASYS_HASOUTPUT

  if ((ASYS_LATENCYTYPE == ASYS_HIGHLATENCY) || asysio_input ||
      (!haslinmidi))
    {
      if (geteuid() || (ASYS_TIMEOPTION == ASYS_TIMESYNC))
	{
	  fprintf(stderr, "For best results, make these changes:\n"); 
	  fprintf(stderr, "\n");
	  if (ASYS_TIMEOPTION == ASYS_TIMESYNC)
	    fprintf(stderr, "   * Remove sfront -timesync flag\n");
	  if (geteuid())
	    fprintf(stderr, "   * Run sa.c executable as root.\n");
	  fprintf(stderr, "\n");
	}
    }
  else
    {
      fprintf(stderr, "This application runs best as root (%s), with:\n",
	      !geteuid() ? "which you are": "which you aren't"); 
      fprintf(stderr, "\n");
      fprintf(stderr, "  [1] Sfront -playback flag. Good audio quality, keeps\n");
      fprintf(stderr, "      the mouse/kbd alive");
      fprintf(stderr, "%s.\n", (ASYS_TIMEOPTION == ASYS_PLAYBACK) ?
	      " (currently chosen)":"");
      fprintf(stderr, "  [2] Sfront -timesync flag. Better quality, console\n");
      fprintf(stderr, "      freezes during MIDI input");
      fprintf(stderr, "%s.\n", (ASYS_TIMEOPTION == ASYS_TIMESYNC) ?
	      " (currently chosen)":"");
      fprintf(stderr, "\n");
    }

#endif

  /* latency information */

#if (defined(ASYS_HASOUTPUT))

  ASYSIO_IOCTL_CALL(asysio_fd, SNDCTL_DSP_GETOSPACE, &asysio_info);

  fprintf(stderr, "If audio artifacts still occur, try");
  
  actual_latency = ATIME*ASYSO_LNUMBUFF*(asysio_size >> (asysio_channels - 1));
  
  if (asysio_info.fragstotal < ASYSO_LNUMBUFF)
    {
      fprintf(stderr, " sfront -latency %f flag, and see\n", 
	      0.5F*actual_latency);
    }
  else
    {
      fprintf(stderr, " sfront -latency %f flag, and see\n", 
	      2.0F*actual_latency);

    }
  if (ASYS_LATENCYTYPE == ASYS_LOWLATENCY)
    fprintf(stderr, "http://www.cs.berkeley.edu/"
	    "~lazzaro/sa/sfman/user/use/index.html#interact\n") ;
  else
    fprintf(stderr, "http://www.cs.berkeley.edu/"
	    "~lazzaro/sa/sfman/user/use/index.html#stream\n") ;

  fprintf(stderr, "\n");

  if ((asysio_bsize != ASYSIO_FRAGMIN) &&
      (asysio_bsize == asysio_requested_bsize) && 
      (ASYS_LATENCYTYPE == ASYS_LOWLATENCY))
  {
    fprintf(stderr, "If interactive response is slow, try ");
    fprintf(stderr, "sfront -latency %f flag.\n", 0.5F*actual_latency);
    fprintf(stderr, "\n");
  }

#endif

  fprintf(stderr, 
  "USE AT YOUR OWN RISK. Running as root may damage your file system,\n");
  fprintf(stderr, 
  "and network use may result in a malicious attack on your machine.\n\n");

#if (ASYSIO_USEFIFO && (ASYS_TIMEOPTION == ASYS_TIMESYNC))

  if (!geteuid())
    {
      fprintf(stderr, 
      "NOTE: Mouse and keyboard are frozen for %i seconds after a MIDI\n",
	      ASYSIO_SYNC_TIMEOUT);
      fprintf(stderr, 
	      "NoteOn or NoteOff is received. Do not be alarmed.\n");
    }

#endif

  if (NSYS_NET)
    fprintf(stderr, "Network status: Contacting SIP server\n");

  return ASYS_DONE;

}


/****************************************************************/
/*        setup operations common to input and output           */
/****************************************************************/

int asysio_setup(int srate, int channels, int dir, int toption)

{
  int i, j, maxfrag;

  /******************/
  /* open soundcard */
  /******************/

  if (asysio_opendevice(dir, toption) == ASYS_ERROR)
    return ASYS_ERROR;
  
  /**************************************/
  /* set up bidirectional I/O if needed */
  /**************************************/

  if (dir == ASYSIO_IO)
    {

#if (ASYSIO_OSTYPE != ASYSIO_FREEBSD)

      ASYSIO_IOCTL_CALL(asysio_fd, SNDCTL_DSP_SETDUPLEX, 0);

#endif 

      ASYSIO_IOCTL_CALL(asysio_fd, SNDCTL_DSP_GETCAPS, &j);

      if (!(j & DSP_CAP_DUPLEX))
	ASYSIO_ERROR_RETURN_NOERRNO("Sound card can't do bidirectional audio");
    }

  /************************/
  /* range check channels */
  /************************/

  if (channels > 2)
    ASYSIO_ERROR_RETURN_NOERRNO("Sound card can't handle > 2 channels");

  /*********************/
  /* set fragment size */
  /*********************/

  j = ASYSIO_LOGFRAGMIN;
  i = ASYSIO_FRAGMIN >> channels;   /* only works for channels = 1, 2 */

  /* find closest power-of-two fragment size to latency request */

  while (2*ATIME*i*ASYSO_LNUMBUFF < ASYS_LATENCY)
    {
      i <<= 1;
      j++;
    }
  if ((ATIME*2*i*ASYSO_LNUMBUFF - ASYS_LATENCY) < 
      (ASYS_LATENCY - ATIME*i*ASYSO_LNUMBUFF))
    {
      i <<= 1;
      j++;
    }

  asysio_requested_bsize = 2*i*channels;

  maxfrag = (ASYS_TIMEOPTION != ASYS_TIMESYNC) ? ASYSO_LNUMBUFF :
             ASYSO_LNUMBUFF + ((ACYCLE/i) + 1);

  j |= (maxfrag << 16);
  ASYSIO_IOCTL_CALL(asysio_fd, SNDCTL_DSP_SETFRAGMENT, &j);

  /********************/
  /* set audio format */
  /********************/

  j = ASYSIO_AFORMAT;
  ASYSIO_IOCTL_CALL(asysio_fd, SNDCTL_DSP_SETFMT, &j);

  if (j != ASYSIO_AFORMAT)
    ASYSIO_ERROR_RETURN_NOERRNO("Soundcard can't handle native shorts");

  /****************************************************/
  /* set number of channels -- later add channels > 2 */
  /****************************************************/

  asysio_channels = channels--;
  ASYSIO_IOCTL_CALL(asysio_fd, SNDCTL_DSP_STEREO, &channels);

  if (channels != (asysio_channels-1))
    ASYSIO_ERROR_RETURN_NOERRNO("Soundcard can't handle number of channels");

  /*********************/
  /* set sampling rate */
  /*********************/

  asysio_srate = srate;
  ASYSIO_IOCTL_CALL(asysio_fd, SNDCTL_DSP_SPEED, &srate);

  if (abs(asysio_srate - srate) > 1000)
    ASYSIO_ERROR_RETURN_NOERRNO("Soundcard can't handle sampling rate");

  /******************************/
  /* compute actual buffer size */
  /******************************/

  ASYSIO_IOCTL_CALL(asysio_fd, SNDCTL_DSP_GETBLKSIZE, &asysio_bsize);
  asysio_size = asysio_bsize >> 1;

  /*************************/
  /* print out info screen */
  /*************************/

  if (asysio_screenwriter() == ASYS_ERROR)
    return ASYS_ERROR;

  /*********************************/
  /* set SCHED_FIFO if appropriate */
  /*********************************/

  if (asysio_initscheduler() == ASYS_ERROR)
    return ASYS_ERROR;

  /**********************************/
  /* set up iloop (heartbeat) timer */
  /**********************************/

  if (asysio_initiloop() == ASYS_ERROR)
    return ASYS_ERROR;

  return ASYS_DONE;
}



#if (defined(ASYS_HASOUTPUT) && !defined(ASYS_HASINPUT))

/****************************************************************/
/*        sets up audio output for a given srate/channels       */
/****************************************************************/

int asys_osetup(int srate, int ochannels, int osample, 
                char * oname, int toption)

{
  if (asysio_setup(srate, ochannels, ASYSIO_O, toption) == ASYS_ERROR)
    return ASYS_ERROR;

  if (!(asyso_buf = (short *)calloc(asysio_size, sizeof(short))))
    ASYSIO_ERROR_RETURN("Can't allocate output buffer");

  return ASYS_DONE;
}

#endif


#if (!defined(ASYS_HASOUTPUT) && defined(ASYS_HASINPUT))

/****************************************************************/
/*        sets up audio input for a given srate/channels       */
/****************************************************************/

int asys_isetup(int srate, int ichannels, int isample, 
                char * iname, int toption)

{
  if (asysio_setup(srate, ichannels, ASYSIO_I, toption) == ASYS_ERROR)
    return ASYS_ERROR;
  if (!(asysi_buf = (short *)malloc(asysio_bsize)))
    ASYSIO_ERROR_RETURN("Can't allocate input buffer");

  return ASYS_DONE;
}

#endif


#if (defined(ASYS_HASOUTPUT) && defined(ASYS_HASINPUT))

/****************************************************************/
/*   sets up audio input and output for a given srate/channels  */
/****************************************************************/

int asys_iosetup(int srate, int ichannels, int ochannels,
                 int isample, int osample, 
                 char * iname, char * oname, int toption)


{

  if (ichannels != ochannels)
    ASYSIO_ERROR_RETURN_NOERRNO
      ("Soundcard needs SAOL inchannels == outchannels");

  if (asysio_setup(srate, ichannels, ASYSIO_IO, toption) == ASYS_ERROR)
    return ASYS_ERROR;

  if (!(asysi_buf = (short *)malloc(asysio_bsize)))
    ASYSIO_ERROR_RETURN("Can't allocate input buffer");

  if (!(asyso_buf = (short *)calloc(asysio_size, sizeof(short))))
    ASYSIO_ERROR_RETURN("Can't allocate output buffer");

  return ASYS_DONE;
}

#endif


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*            input, output, and recovery routines              */
/*______________________________________________________________*/


#if defined(ASYS_HASINPUT)

/****************************************************************/
/*               gets one frame of audio from input             */
/****************************************************************/

int asys_getbuf(ASYS_ITYPE * asys_ibuf[], int * isize)

{
  int diffcompute, starttime;
  int size, recv, bptr, retry;

  *isize = asysio_size;

  if (*asys_ibuf == NULL)
    *asys_ibuf = asysi_buf;

  ASYSIO_IOCTL_CALL(asysio_fd, SNDCTL_DSP_GETISPACE, &asysio_info);

#if defined(ASYS_HASOUTPUT)

  if (diffcompute = (asysio_info.bytes < asysio_bsize))
    {  
      ASYSIO_IOCTL_CALL(asysio_fd, SNDCTL_DSP_GETOPTR, &asysio_ptr);
      starttime = asysio_ptr.bytes;
    }

#endif

  retry = bptr = 0;
  size = asysio_bsize;

  while ((recv = read(asysio_fd, &((*asys_ibuf)[bptr]), size)) != size)
    {      
      if (++retry > ASYSIO_MAXRETRY)
	ASYSIO_ERROR_RETURN("Too many I/O retries -- asys_getbuf");

      if (recv < 0)  /* errors */
	{
	  if ((errno == EAGAIN) || (errno == EINTR))
	    continue;   
	  else
	    ASYSIO_ERROR_RETURN("Read error on output audio device");
	}
      else
	{
	  bptr += recv; /* partial read */
	  size -= recv;
	}
    }

#if defined(ASYS_HASOUTPUT)

  if (diffcompute)
    {  
      ASYSIO_IOCTL_CALL(asysio_fd, SNDCTL_DSP_GETOPTR, &asysio_ptr);
      asysio_blocktime += (asysio_ptr.bytes - starttime);
    }

#endif

  return ASYS_DONE;
}

#endif


#if defined(ASYS_HASOUTPUT)

/****************************************************************/
/*               sends one frame of audio to output             */
/****************************************************************/

int asys_putbuf(ASYS_OTYPE * asys_obuf[], int * osize)


{
  int size, sent, bptr, retry;
  int diffcompute, starttime;

  size = (*osize)*2;


  if (asysio_reset)
    return ASYS_DONE;

  ASYSIO_IOCTL_CALL(asysio_fd, SNDCTL_DSP_GETOSPACE, &asysio_info);

  asysio_reset = (++asysio_puts > ASYSO_LNUMBUFF) && 
    (asysio_info.fragments == asysio_info.fragstotal);
  if (asysio_reset)
    return ASYS_DONE;

#if (ASYS_TIMEOPTION != ASYS_TIMESYNC)

  if (diffcompute = (asysio_info.bytes < size))
    {  
      ASYSIO_IOCTL_CALL(asysio_fd, SNDCTL_DSP_GETOPTR, &asysio_ptr);
      starttime = asysio_ptr.bytes;
    }

#endif

  retry = bptr = 0;
  while ((sent = write(asysio_fd, &((*asys_obuf)[bptr]), size)) != size)
    {
      if (++retry > ASYSIO_MAXRETRY)
	ASYSIO_ERROR_RETURN("Too many I/O retries -- asys_putbuf");

      if (sent < 0)  /* errors */
	{
	  if ((errno == EAGAIN) || (errno == EINTR))
	    continue;   
	  else
	    ASYSIO_ERROR_RETURN("Write error on output audio device");
	}
      else
	{
	  bptr += sent;  /* partial write */
	  size -= sent;
	}
    }

#if (ASYS_TIMEOPTION != ASYS_TIMESYNC)

  if (diffcompute)
    {  
      ASYSIO_IOCTL_CALL(asysio_fd, SNDCTL_DSP_GETOPTR, &asysio_ptr);
      asysio_blocktime += (asysio_ptr.bytes - starttime);
    }

#endif

  *osize = asysio_size;
  return ASYS_DONE;
}


/****************************************************************/
/*        creates buffer, and generates starting silence        */
/****************************************************************/

int asys_preamble(ASYS_OTYPE * asys_obuf[], int * osize)

{
  int i;

  *asys_obuf = asyso_buf;
  *osize = asysio_size;

  for(i = 0; i < ASYSO_LNUMBUFF; i++)
    if (asys_putbuf(asys_obuf, osize) == ASYS_ERROR)
      return ASYS_ERROR;

  return ASYS_DONE;
}


/****************************************************************/
/*               recovers from an overrun                       */
/****************************************************************/

int asysio_recover(void)

{
  int size, recv, bptr, retry;
  int i;

  asysio_reset = 0;

  memset(asyso_buf, 0, asysio_bsize);

  /*************************/
  /* flush input if needed */
  /*************************/

#if defined(ASYS_HASINPUT)

  ASYSIO_IOCTL_CALL(asysio_fd, SNDCTL_DSP_GETISPACE, &asysi_info);
  
  while (asysi_info.fragments > 0)
    {
      retry = bptr = 0;
      size = asysio_bsize;

      while ((recv = read(asysio_fd, &(asysi_buf[bptr]), size)) != size)
	{      
	  if (++retry > ASYSIO_MAXRETRY)
	    ASYSIO_ERROR_RETURN("Too many I/O retries -- asysio_recover");

	  if (recv < 0)  /* errors */
	    {
	      if ((errno == EAGAIN) || (errno == EINTR))
		continue;   
	      else
		ASYSIO_ERROR_RETURN("Read error on output audio device");
	    }
	  else
	    {
	      bptr += recv; /* partial read */
	      size -= recv;
	    }
	}

      ASYSIO_IOCTL_CALL(asysio_fd, SNDCTL_DSP_GETISPACE, &asysi_info);
    }

  ibusidx = 0;
  if (asys_getbuf(&asys_ibuf, &EV(asys_isize))==ASYS_ERROR)
    return ASYS_ERROR;

#endif

  /**************************************/
  /* fill latency interval with silence */ 
  /**************************************/

  asysio_puts = 0;
  for(i = 0; i < ASYSO_LNUMBUFF; i++)
    if (asys_putbuf(&asyso_buf, &asysio_size) == ASYS_ERROR)
      return ASYS_ERROR;

  return ASYS_DONE;

}


#endif


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*        ksync() system for time synchronization          */ 
/*_________________________________________________________*/

#if defined(ASYS_KSYNC)

/***********************************************************/
/*         initializes k-rate boundaries sync              */
/***********************************************************/

void ksyncinit()

{
  asysio_sync_target = asysio_sync_incr = ACYCLE*asysio_channels*2;  
  asysio_sync_cpuscale = 1.0F/asysio_sync_incr;

  /* for -timesync, set up SCHED_FIFO watchdog state machine */

#if (ASYSIO_USEFIFO && (ASYS_TIMEOPTION == ASYS_TIMESYNC))

  if (asysio_fifo)

    {
      asysio_sync_state = ASYSIO_SYNC_SCHEDOTHER;
      if (sched_setscheduler(0, SCHED_OTHER, &asysio_otherparam))
	epr(0,NULL,NULL,"internal error -- sched_other unavailable");
    }

#endif

  /* elsewise, set up SCHED_FIFO monitor to force blocking */

#if (ASYSIO_USEFIFO && (ASYS_TIMEOPTION != ASYS_TIMESYNC))

  asysio_sync_noblock = 0;
  asysio_sync_sleeptime.tv_sec = 0;
  asysio_sync_sleeptime.tv_nsec = 2000001;  /* 2ms + epsilon forces block */
 
#endif

}


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*     different ksync()s for -timesync and -playback       */ 
/*__________________________________________________________*/


#if (ASYS_TIMEOPTION != ASYS_TIMESYNC)

/***********************************************************/
/*         synchronizes on k-rate boundaries               */
/***********************************************************/

float ksync()

{
  float ret;
  int comptime;

  if (asysio_reset)
    {
      if (asysio_recover()==ASYS_ERROR)
	epr(0,NULL,NULL, "Soundcard error -- failed recovery.");
      asysio_sync_target = asysio_sync_incr;
      ret = 1.0F;
    }
  else
    {

      ASYSIO_IOCTL_CALL(asysio_fd, SNDCTL_DSP_GETOPTR, &asysio_sync_ptr);

      if (asysio_sync_target == asysio_sync_incr)
	ret = 0.0F;
      else
	{
	  comptime = asysio_sync_ptr.bytes - asysio_blocktime;
	  if (comptime > asysio_sync_target)
	    ret = 1.0F;
	  else
	    ret = (asysio_sync_cpuscale*
		   (asysio_sync_incr - (asysio_sync_target - comptime)));
	}
      
      if ((asysio_sync_target = asysio_sync_incr + asysio_sync_ptr.bytes) < 0)
	epr(0,NULL,NULL,"Soundcard error -- rollover.");
    }

  /* reset infinite-loop timer */

  if (setitimer(ITIMER_PROF, &asysio_iloop_timer, NULL) < 0)
    {
      fprintf(stderr, "  Runtime Errno Message: %s\n", strerror(errno));
      epr(0,NULL,NULL, "Soundcard error -- Couldn't reset ITIMER_PROF");
    }

#if ASYSIO_USEFIFO

  if (asysio_fifo)
    {
      /* let other processes run if pending too long */

      if (asysio_blocktime)
	asysio_sync_noblock = 0;
      else
	asysio_sync_noblock++;

      if (asysio_sync_noblock > ASYSIO_MAXBLOCK)
	{
	  nanosleep(&asysio_sync_sleeptime, NULL); 
	  asysio_sync_noblock = 0;
	}
    }

#endif

  asysio_blocktime = 0;
  return ret;
}

#endif


#if (ASYS_TIMEOPTION == ASYS_TIMESYNC)

/***********************************************************/
/*         synchronizes on k-rate boundaries               */
/***********************************************************/

float ksync()

{
  float ret;
  int comptime;

  if (!asysio_reset)
    {
      ASYSIO_IOCTL_CALL(asysio_fd, SNDCTL_DSP_GETOPTR, &asysio_sync_ptr);
      if (asysio_sync_ptr.bytes > asysio_sync_target)
	{
	  comptime = asysio_sync_ptr.bytes - asysio_blocktime;
	  if (comptime < asysio_sync_target)
	    ret = (asysio_sync_cpuscale*
		   (asysio_sync_incr - (asysio_sync_target - comptime)));
	  else
	    ret = 1.0F;
	  ret = (asysio_sync_target != asysio_sync_incr) ? ret : 0.0F;
	}
      else
	{	  
	  comptime = asysio_sync_ptr.bytes - asysio_blocktime;
	  ret = (asysio_sync_cpuscale*
		 (asysio_sync_incr - (asysio_sync_target - comptime)));
	  asysio_reset = asysio_input && 
	    ((asysio_sync_target-asysio_sync_ptr.bytes) == asysio_sync_incr);
	  while ((asysio_sync_ptr.bytes < asysio_sync_target) && !asysio_reset)
	    {  
	      ASYSIO_IOCTL_CALL(asysio_fd, SNDCTL_DSP_GETOSPACE, &asysio_info);
	      asysio_reset = (asysio_info.fragments == asysio_info.fragstotal);
	      ASYSIO_IOCTL_CALL(asysio_fd, SNDCTL_DSP_GETOPTR,&asysio_sync_ptr);
	    }
	}
    }
  if (asysio_reset)
    {
      if (asysio_recover()==ASYS_ERROR)
	epr(0,NULL,NULL,"Sound driver error -- failed recovery.");
      ASYSIO_IOCTL_CALL(asysio_fd, SNDCTL_DSP_GETOPTR, &asysio_sync_ptr);
      asysio_sync_target = asysio_sync_ptr.bytes;
      ret = 1.0F;
    }
  if ((asysio_sync_target += asysio_sync_incr) < 0)
    epr(0,NULL,NULL,"Sound driver error -- rollover.");

  /* reset infinite-loop timer */

  if (setitimer(ITIMER_PROF, &asysio_iloop_timer, NULL) < 0)
    {
      fprintf(stderr, "  Runtime Errno Message: %s\n", strerror(errno));
      epr(0,NULL,NULL, "Soundcard error -- Couldn't reset ITIMER_PROF");
    }

#if ASYSIO_USEFIFO

  if (asysio_fifo)
    {
      switch (asysio_sync_state) {
      case ASYSIO_SYNC_ACTIVE:
	if (!csysi_newnote)
	  {
	    asysio_sync_state = ASYSIO_SYNC_WAITING;
	    asysio_sync_waitstart = time(NULL);
	  }
	break;
      case ASYSIO_SYNC_WAITING:
	if (csysi_newnote)
	  asysio_sync_state = ASYSIO_SYNC_ACTIVE;
	else
	  if ((time(NULL) - asysio_sync_waitstart) >= ASYSIO_SYNC_TIMEOUT)
	    {
	      asysio_sync_state = ASYSIO_SYNC_SCHEDOTHER;
	      if (sched_setscheduler(0, SCHED_OTHER, &asysio_otherparam))
		epr(0,NULL,NULL,"internal error -- sched_other unavailable");
	    }
	break;
      case ASYSIO_SYNC_SCHEDOTHER:
	if (csysi_newnote)
	  {
	    asysio_sync_state = ASYSIO_SYNC_ACTIVE;
	    if (sched_setscheduler(0, SCHED_FIFO, &asysio_fifoparam))
	      fprintf(stderr, "  Note: Process no longer root, " 
		      "improved audio quality no longer possible.\n");
	  }
	break;
      }
    }

#endif

  asysio_blocktime = 0;
  return ret;
}

#endif

#endif /* ASYS_KSYNC */

#undef ASYSIO_IOCTL_CALL
#undef ASYSIO_ERROR_RETURN
#undef ASYSIO_ERROR_RETURN_NOERRNO
#undef ASYSIO_LINUX
#undef ASYSIO_FREEBSD
#undef ASYSIO_OSTYPE
#undef ASYSIO_DSPDEV
#undef ASYSIO_AFORMAT
#undef ASYSIO_I  
#undef ASYSIO_O  
#undef ASYSIO_IO 
#undef ASYSIO_FRAGMIN
#undef ASYSIO_LOGFRAGMIN 
#undef ASYSO_LNUMBUFF
#undef ASYSIO_MAXRETRY

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*                    end of soundcard driver                   */
/*______________________________________________________________*/


