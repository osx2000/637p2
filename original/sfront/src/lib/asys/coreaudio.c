
/*
#    Sfront, a SAOL to C translator    
#    This file: Apple CoreAudio driver for sfront
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



/*~~~~~~~~~~~~~~~~~*/
/* include headers */
/*_________________*/

#include <sys/types.h>
#include <sys/times.h>
#include <unistd.h>
#include <sys/mman.h>
#include <mach/mach.h>
#include <mach/host_priv.h>
#include <mach/vm_region.h>
#include <mach-o/getsect.h>

#if defined(CSYS_CDRIVER_COREMIDI)
#include <CoreFoundation/CFString.h>
#endif

#include <CoreAudio/AudioHardware.h>
#include <CoreAudio/CoreAudioTypes.h>
#include <CoreAudio/HostTime.h>

/*~~~~~~~~~~~~~~*/
/* error macros */
/*______________*/

#define  ASYSIO_ERROR_REPORT(x) do {\
      fprintf(stderr, "  Error: %s.\n", x);\
      } while (0)

#define  ASYSIO_ERROR_RETURN(x) do {\
      fprintf(stderr, "  Error: %s.\n", x);\
      return ASYS_ERROR; } while (0)

#define  ASYSIO_ERROR_EMPTYRETURN(x) do {\
      fprintf(stderr, "  Error: %s.\n", x);\
      return;} while (0)

/*~~~~~~~~~~~*/
/* constants */
/*___________*/

/* test definitions for listenerprocs */

#define ASYSN_COREAUDIO_LPROC_MUTETEST  0     /* test mode: mute forces shutdown */

/* for latency and buffering */

#define ASYSN_COREAUDIO_LOWLATENCY   0.002    /* 2 ms buffer for low latency   */
#define ASYSN_COREAUDIO_HIGHLATENCY  0.020    /* 20 ms buffer for high latency */
#define ASYSN_COREAUDIO_LEADERTIME   0.050    /* silience to prime CoreAudio   */
#define ASYSN_COREAUDIO_TRUEMAXFRAMES 2205    /* workaround ...                */

/* for virtual memory paging */

#define ASYSN_COREAUDIO_TOUCHMARGIN   256    /* for touch: 1MB for others (PPC)  */
#define ASYSN_COREAUDIO_LOCKMARGIN   1024    /* for touch: 4MB for others (PPC)  */

/* piping codes */

#define ASYSN_COREAUDIO_PIPE_OK         0    /* normal termination for SA        */
#define ASYSN_COREAUDIO_PIPE_LPROC      1    /* listener proc request            */

/* for printout -- paging */

#define ASYSN_COREAUDIO_NOPRINT   2   /* don't print paging info */
#define ASYSN_COREAUDIO_SHORTRAM  1   /* not enough RAM          */
#define ASYSN_COREAUDIO_PAGEIN    0   /* successful page-in      */
#define ASYSN_COREAUDIO_STATFAIL -1  /* Host_statistics failed  */

int asysn_coreaudio_pageinfo;       /* codes success of page-in  */
int asysn_coreaudio_pagelock;       /* codes page-locking status */
int asysn_coreaudio_pagetot;        /* number of pages inuse     */

/* for printout -- coreaudio input */

#define ASYSN_COREAUDIO_INPUT_NONE  0   /* no input source           */
#define ASYSN_COREAUDIO_INPUT_IDEF  1   /* default coreaudio input   */
#define ASYSN_COREAUDIO_INPUT_ODEF  2   /* coreaudio output as input */

/*~~~~~~~~~~~*/
/* variables */
/*___________*/

#if defined(ASYS_HASOUTPUT)

#define ASYSO_MAXSTREAM   16            /* max number of ostreams  */

AudioDeviceID asyso_aid;                   /* audio output device  */
Float32 * asyso_buffptr[ASYSO_MAXSTREAM];  /* stream pointers      */
UInt32 asyso_buffchan[ASYSO_MAXSTREAM];    /* per-stream channels  */
#endif

#if defined(ASYS_HASINPUT)

#define ASYSI_MAXSTREAM   16            /* max number of istreams  */

AudioDeviceID asysi_aid;                   /* audio input device   */
Float32 * asysi_buffptr[ASYSI_MAXSTREAM];  /* stream pointers      */
UInt32 asysi_buffchan[ASYSI_MAXSTREAM];    /* per-stream channels  */
#endif

/* state for passive input driver */

#if (defined(ASYS_ACTIVE_IO) && !defined(ASYS_HASINPUT))

ASYS_ITYPE * asysn_coreaudio_ibuf;
int asysn_coreaudio_ilast;
int asysn_coreaudio_iptr;
#endif

/* state for passive output driver */

#if (defined(ASYS_ACTIVE_IO) && !defined(ASYS_HASOUTPUT))

ASYS_OTYPE * asysn_coreaudio_obuf;
int asysn_coreaudio_optr;
int asysn_coreaudio_olast;
#endif

int asysn_coreaudio_instatus;        /* code coreaudio input info   */

/* for printout -- frame info */

UInt32 asysn_coreaudio_frame_samples;  /* frame length in samples     */
float asysn_coreaudio_frame_seconds;   /* frame length in seconds     */

/* for printout -- mismatch warnings */

int asysn_coreaudio_insize_mismatch;   /* too many coreaudio inputs   */
int asysn_coreaudio_outsize_mismatch;  /* not enough coreaudio outputs */

/* general-purpose variables */

int asysn_coreaudio_pipepair[2];    /* asys_main/IOProc comm chan  */
int volatile asysn_coreaudio_live;  /* routes silence or SA to out */
int asysn_coreaudio_silentframes;   /* leader-tape length          */

int volatile asysn_coreaudio_touch; /* for page warmups           */
UInt64 asysn_coreaudio_ksync_then;   /* counter for time spent     */

/*~~~~~~~~~~~~~~~~~*/
/* forward externs */
/*_________________*/

extern float ksync(void);
extern void  main_ipass(void);
extern int   main_kpass(void);
extern void  main_apass(void);
extern void  main_control(void);

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*   asys_mac() functions     */
/*----------------------------*/

#if defined(ASYS_ACTIVE_O)

/********************************************************************/
/*                    CoreAudio output, no input                    */
/********************************************************************/

int asys_mac_orun(float * obuff[], UInt32 obuffchan[], UInt32 onumbuff, UInt32 frames)
{
  int frameidx = 0;
  int busidx, chanidx, buffidx;

  if (EV(asys_exit_status) == ASYS_EXIT)
    return ASYS_EXIT;

  while (frameidx < frames)
    if (EV(acycleidx) < EV(ACYCLE))
      {
	memset(&(TB(0)), 0, sizeof(float)*ENDBUS);
	main_apass();

#if (ASYS_OCHAN == 1L)   /* mono case */
	for (buffidx = 0; buffidx < onumbuff; buffidx++)
	  for (chanidx = 0; chanidx < obuffchan[buffidx]; chanidx++)
	    *((obuff[buffidx])++) = TB(BUS_output_bus);
#else
	busidx = BUS_output_bus;
	for (buffidx = 0; buffidx < onumbuff; buffidx++)
	  for (chanidx = 0; chanidx < obuffchan[buffidx]; chanidx++)
	    {
	      if (busidx < ENDBUS_output_bus)
		*(obuff[buffidx]) = TB(busidx++);
	      (obuff[buffidx])++;
	    }
#endif

	EV(acycleidx)++;
	frameidx++;
      }
    else
      {
	EV(acycleidx) = 0;
	if (EV(pass) == APASS)
	  {      
	    EV(cpuload) = ksync();
	    EV(kcycleidx)++;
	  }
	if (EV(kcycleidx) > EV(endkcycle))
	  return (EV(asys_exit_status) = ASYS_EXIT);

	EV(pass) = IPASS;
	EV(scorebeats) = EV(scoremult)*(EV(kcycleidx) - EV(kbase)) + EV(scorebase);
	EV(absolutetime) = (EV(kcycleidx) - 1)*EV(KTIME);
	main_ipass();
	EV(pass) = KPASS;
	main_control();
	if (main_kpass())
	  return (EV(asys_exit_status) = ASYS_EXIT);
	EV(pass) = APASS;
      }

  return ASYS_DONE;
}

#endif  /* ASYS_ACTIVE_O */


#if defined(ASYS_ACTIVE_IO) && defined(ASYS_HASINPUT) && defined(ASYS_HASOUTPUT)

/********************************************************************/
/*                    CoreAudio output and input                    */
/********************************************************************/

int asys_mac_iorun(float * obuff[], UInt32 obuffchan[], UInt32 onumbuff, 
		   float * ibuff[], UInt32 ibuffchan[], UInt32 inumbuff, 
		   UInt32 frames)
{
  int frameidx = 0;
  int mono_in = ((inumbuff == 1) && (ibuffchan[0] == 1));
  int busidx, chanidx, buffidx;

  if (EV(asys_exit_status) == ASYS_EXIT)
    return ASYS_EXIT;

  while (frameidx < frames)
    if (EV(acycleidx) < EV(ACYCLE))
      {
	memset(&(TB(0)), 0, sizeof(float)*ENDBUS);

	if (!mono_in)
	  {
	    busidx = BUS_input_bus;
	    for (buffidx = 0; buffidx < inumbuff; buffidx++)
	      for (chanidx = 0; chanidx < ibuffchan[buffidx]; chanidx++)
		{
		  if (busidx < ENDBUS_input_bus)
		    TB(busidx++) = *(ibuff[buffidx]);
		  (ibuff[buffidx])++;
		}
	  }
	else
	  {
	    for(busidx=BUS_input_bus;busidx<ENDBUS_input_bus;busidx++)
	      TB(busidx) = *(ibuff[0]);
	    (ibuff[0])++;
	  }

	main_apass();

#if (ASYS_OCHAN == 1L)   /* mono case */
	for (buffidx = 0; buffidx < onumbuff; buffidx++)
	  for (chanidx = 0; chanidx < obuffchan[buffidx]; chanidx++)
	    *((obuff[buffidx])++) = TB(BUS_output_bus);
#else
	busidx = BUS_output_bus;
	for (buffidx = 0; buffidx < onumbuff; buffidx++)
	  for (chanidx = 0; chanidx < obuffchan[buffidx]; chanidx++)
	    {
	      if (busidx < ENDBUS_output_bus)
		*(obuff[buffidx]) = TB(busidx++);
	      (obuff[buffidx])++;
	    }
#endif

	EV(acycleidx)++;
	frameidx++;
      }
    else
      {
	EV(acycleidx) = 0;
	if (EV(pass) == APASS)
	  {      
	    EV(cpuload) = ksync();
	    EV(kcycleidx)++;
	  }
	if (EV(kcycleidx) > EV(endkcycle))
	  return (EV(asys_exit_status) = ASYS_EXIT);

	EV(pass) = IPASS;
	EV(scorebeats) = EV(scoremult)*(EV(kcycleidx) - EV(kbase)) + EV(scorebase);
	EV(absolutetime) = (EV(kcycleidx) - 1)*EV(KTIME);
	main_ipass();
	EV(pass) = KPASS;
	main_control();
	if (main_kpass())
	  return (EV(asys_exit_status) = ASYS_EXIT);
	EV(pass) = APASS;
      }

  return ASYS_DONE;
}

#endif /* ASYS_ACTIVE_IO -- CoreAudio I + O */


#if defined(ASYS_ACTIVE_IO) && !defined(ASYS_HASINPUT)

/********************************************************************/
/*                  CoreAudio output, passive input                 */
/********************************************************************/

int asys_mac_orun(float * obuff[], UInt32 obuffchan[], UInt32 onumbuff,
		   UInt32 frames)
{
  int frameidx = 0;
  int busidx, chanidx, buffidx;

  if (EV(asys_exit_status) == ASYS_EXIT)
    return ASYS_EXIT;

  while (frameidx < frames)
    if (EV(acycleidx) < EV(ACYCLE))
      {
	memset(&(TB(0)), 0, sizeof(float)*ENDBUS);

	if (asysn_coreaudio_ilast == asysn_coreaudio_iptr)
	  {  
	    if ((asys_getbuf(&asysn_coreaudio_ibuf, &asysn_coreaudio_ilast) 
		 != ASYS_DONE) || (asysn_coreaudio_ilast == 0))
	      return (EV(asys_exit_status) = ASYS_EXIT);
	    asysn_coreaudio_iptr = 0;
	  }

#if (ASYS_ITYPENAME == ASYS_SHORT)
	for(busidx=BUS_input_bus;busidx<ENDBUS_input_bus;busidx++)
	  TB(busidx) = 3.051851e-5F*asysn_coreaudio_ibuf[asysn_coreaudio_iptr++];
#endif

#if (ASYS_ITYPENAME == ASYS_FLOAT)
	for(busidx=BUS_input_bus;busidx<ENDBUS_input_bus;busidx++)
	  TB(busidx) = asysn_coreaudio_ibuf[asysn_coreaudio_iptr++];
#endif

	main_apass();

#if (ASYS_OCHAN == 1L)   /* mono case */
	for (buffidx = 0; buffidx < onumbuff; buffidx++)
	  for (chanidx = 0; chanidx < obuffchan[buffidx]; chanidx++)
	    *((obuff[buffidx])++) = TB(BUS_output_bus);
#else
	busidx = BUS_output_bus;
	for (buffidx = 0; buffidx < onumbuff; buffidx++)
	  for (chanidx = 0; chanidx < obuffchan[buffidx]; chanidx++)
	    {
	      if (busidx < ENDBUS_output_bus)
		*(obuff[buffidx]) = TB(busidx++);
	      (obuff[buffidx])++;
	    }
#endif

	EV(acycleidx)++;
	frameidx++;
      }
    else
      {
	EV(acycleidx) = 0;
	if (EV(pass) == APASS)
	  {      
	    EV(cpuload) = ksync();
	    EV(kcycleidx)++;
	  }
	if (EV(kcycleidx) > EV(endkcycle))
	  return (EV(asys_exit_status) = ASYS_EXIT);

	EV(pass) = IPASS;
	EV(scorebeats) = EV(scoremult)*(EV(kcycleidx) - EV(kbase)) + EV(scorebase);
	EV(absolutetime) = (EV(kcycleidx) - 1)*EV(KTIME);
	main_ipass();
	EV(pass) = KPASS;
	main_control();
	if (main_kpass())
	  return (EV(asys_exit_status) = ASYS_EXIT);
	EV(pass) = APASS;
      }

  return ASYS_DONE;
}

#endif /* ASYS_ACTIVE_IO -- CoreAudio O only */


#if defined(ASYS_ACTIVE_IO) && !defined(ASYS_HASOUTPUT)

/********************************************************************/
/*                  CoreAudio input, passive output                 */
/********************************************************************/

int asys_mac_irun(float * ibuff[], UInt32 ibuffchan[], UInt32 inumbuff, 
		  UInt32 frames)
{
  int frameidx = 0;
  int mono_in = ((inumbuff == 1) && (ibuffchan[0] == 1));
  int busidx, chanidx, buffidx;

  if (EV(asys_exit_status) == ASYS_EXIT)
    return ASYS_EXIT;

  while (frameidx < frames)
    if (EV(acycleidx) < EV(ACYCLE))
      {
	memset(&(TB(0)), 0, sizeof(float)*ENDBUS);

	if (!mono_in)
	  {
	    busidx = BUS_input_bus;
	    for (buffidx = 0; buffidx < inumbuff; buffidx++)
	      for (chanidx = 0; chanidx < ibuffchan[buffidx]; chanidx++)
		{
		  if (busidx < ENDBUS_input_bus)
		    TB(busidx++) = *(ibuff[buffidx]);
		  (ibuff[buffidx])++;
		}
	  }
	else
	  {
	    for(busidx=BUS_input_bus;busidx<ENDBUS_input_bus;busidx++)
	      TB(busidx) = *(ibuff[0]);
	    (ibuff[0])++;
	  }

	main_apass();

#if (ASYS_OTYPENAME == ASYS_FLOAT)
	for (busidx = BUS_output_bus; busidx < ENDBUS_output_bus; busidx++)
	  if ((TB(busidx) <= 1.0F) && (TB(busidx) >= -1.0F))
	    asysn_coreaudio_obuf[asysn_coreaudio_optr++] = TB(busidx);
	  else
	    {
	      if (TB(busidx) > 0.0F)
		asysn_coreaudio_obuf[asysn_coreaudio_optr++] = 1.0F;
	      else
		asysn_coreaudio_obuf[asysn_coreaudio_optr++] = -1.0F;
	    }
#endif

#if (ASYS_OTYPENAME == ASYS_SHORT)
	for (busidx = BUS_output_bus; busidx < ENDBUS_output_bus; busidx++)
	  if ((TB(busidx) <= 1.0F) && (TB(busidx) >= -1.0F))
	    asysn_coreaudio_obuf[asysn_coreaudio_optr++] = ((float)0x7FFF)*TB(busidx);
	  else
	    {
	      if (TB(busidx) > 0.0F)
		asysn_coreaudio_obuf[asysn_coreaudio_optr++] = 0x7FFF;
	      else
		asysn_coreaudio_obuf[asysn_coreaudio_optr++] = 0xFFFF;
	    }
#endif

	if (asysn_coreaudio_optr == asysn_coreaudio_olast)
	  {
	    if (asys_putbuf(&asysn_coreaudio_obuf,&asysn_coreaudio_olast)==ASYS_DONE)
	      asysn_coreaudio_optr = 0;
	    else
	      return (EV(asys_exit_status) = ASYS_EXIT);
	  }

	EV(acycleidx)++;
	frameidx++;
      }
    else
      {
	EV(acycleidx) = 0;
	if (EV(pass) == APASS)
	  {      
	    EV(cpuload) = ksync();
	    EV(kcycleidx)++;
	  }
	if (EV(kcycleidx) > EV(endkcycle))
	  {
	    if (asysn_coreaudio_optr)
	      asys_putbuf(&asysn_coreaudio_obuf, &asysn_coreaudio_optr);
	    return (EV(asys_exit_status) = ASYS_EXIT);
	  }
	EV(pass) = IPASS;
	EV(scorebeats) = EV(scoremult)*(EV(kcycleidx) - EV(kbase)) + EV(scorebase);
	EV(absolutetime) = (EV(kcycleidx) - 1)*EV(KTIME);
	main_ipass();
	EV(pass) = KPASS;
	main_control();
	if (main_kpass())
	  {
	    if (asysn_coreaudio_optr)
	      asys_putbuf(&asysn_coreaudio_obuf, &asysn_coreaudio_optr);
	    return (EV(asys_exit_status) = ASYS_EXIT);
	  }
	EV(pass) = APASS;
      }

  return ASYS_DONE;
}

#endif /* ASYS_ACTIVE_IO -- CoreAudio I only */


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*   callback function suite  */
/*----------------------------*/

#if defined(ASYS_ACTIVE_O) || (defined(ASYS_ACTIVE_IO) && !defined(ASYS_HASINPUT))

/********************************************************************/
/*          CoreAudio output, no input or passive input             */
/********************************************************************/

OSStatus asysn_coreaudio_only_output(AudioDeviceID inDevice, 
				     const AudioTimeStamp * inNow,
				     const AudioBufferList * inInputData,
				     const AudioTimeStamp * inInputTime,
				     AudioBufferList * outOutputData,
				     const AudioTimeStamp * outOutputTime, 
				     void * inClientData)
{
  UInt32 numbuff = ASYSO_MAXSTREAM;
  UInt32 i, frames, retcode;

  /* exit gracefully if audio output goes away */

  if ((outOutputTime->mFlags & kAudioTimeStampHostTimeValid) &&
      (outOutputTime->mHostTime == 0))
    return kAudioHardwareNoError;

  /* set up cpuload timing state */

  if (inNow->mFlags & kAudioTimeStampHostTimeValid)
    asysn_coreaudio_ksync_then = inNow->mHostTime - asysn_coreaudio_ksync_then;
  else
    asysn_coreaudio_ksync_then = (AudioGetCurrentHostTime() -
				  asysn_coreaudio_ksync_then);

  if (asysn_coreaudio_live == ASYS_DONE)
    {
      /* set up pointers for asys_mac_orun() call, then make call */

      if (outOutputData->mNumberBuffers <= ASYSO_MAXSTREAM)
	numbuff = outOutputData->mNumberBuffers;

      for (i = 0; i < numbuff; i++)
	{
	  asyso_buffptr[i] = (Float32 *) outOutputData->mBuffers[i].mData;
	  asyso_buffchan[i] = outOutputData->mBuffers[i].mNumberChannels;
	}

      if (asyso_buffchan[0] <= 2)
	frames = outOutputData->mBuffers[0].mDataByteSize >> (1 + asyso_buffchan[0]);
      else
	frames = outOutputData->mBuffers[0].mDataByteSize/(asyso_buffchan[0] << 2);

      asysn_coreaudio_live = asys_mac_orun(asyso_buffptr, asyso_buffchan,
					   numbuff, frames);

      /* inform main thread of normal exit or error */

      if (asysn_coreaudio_live != ASYS_DONE)
	{
	  retcode = ASYSN_COREAUDIO_PIPE_OK;
	  if (write(asysn_coreaudio_pipepair[1], &retcode, sizeof(UInt32)) < 0)
	    perror("Error writing asys_main pipe");
	}
    }
  else
    {
      /* silience at start of program */

      if (asysn_coreaudio_silentframes && !(--asysn_coreaudio_silentframes))
	asysn_coreaudio_live = ASYS_DONE;
    }

  /* set up cpuload timing state for next time */

  asysn_coreaudio_ksync_then = (AudioGetCurrentHostTime() - 
				asysn_coreaudio_ksync_then);
  return kAudioHardwareNoError;
}

#endif  /* CoreAudio output, no input or passive input */


#if defined(ASYS_ACTIVE_IO) && defined(ASYS_HASINPUT) && defined(ASYS_HASOUTPUT)

/********************************************************************/
/*      coreaudio input and output, from the same audio device      */
/********************************************************************/

OSStatus asysn_coreaudio_input_output(AudioDeviceID inDevice, 
				      const AudioTimeStamp * inNow,
				      const AudioBufferList * inInputData,
				      const AudioTimeStamp * inInputTime,
				      AudioBufferList * outOutputData,
				      const AudioTimeStamp * outOutputTime, 
				      void * inClientData)
{
  UInt32 inumbuff = ASYSI_MAXSTREAM;
  UInt32 onumbuff = ASYSO_MAXSTREAM;
  UInt32 i, frames, retcode;

  /* exit gracefully if audio input or output goes away */

  if (((inInputTime->mFlags & kAudioTimeStampHostTimeValid) &&
       (inInputTime->mHostTime == 0)) ||
      ((outOutputTime->mFlags & kAudioTimeStampHostTimeValid) &&
       (outOutputTime->mHostTime == 0)))
    return kAudioHardwareNoError;

  /* set up cpuload timing state */

  if (inNow->mFlags & kAudioTimeStampHostTimeValid)
    asysn_coreaudio_ksync_then = inNow->mHostTime - asysn_coreaudio_ksync_then;
  else
    asysn_coreaudio_ksync_then = (AudioGetCurrentHostTime() -
				  asysn_coreaudio_ksync_then);

  if (asysn_coreaudio_live == ASYS_DONE)
    {
      /* set up pointers for asys_mac_iorun() call, then make call */

      if (outOutputData->mNumberBuffers <= ASYSO_MAXSTREAM)
	onumbuff = outOutputData->mNumberBuffers;

      for (i = 0; i < onumbuff; i++)
	{
	  asyso_buffptr[i] = (Float32 *) outOutputData->mBuffers[i].mData;
	  asyso_buffchan[i] = outOutputData->mBuffers[i].mNumberChannels;
	}

      if (asyso_buffchan[0] <= 2)
	frames = outOutputData->mBuffers[0].mDataByteSize >> (1 + asyso_buffchan[0]);
      else
	frames = outOutputData->mBuffers[0].mDataByteSize/(asyso_buffchan[0] << 2);

      if (inInputData->mNumberBuffers <= ASYSI_MAXSTREAM)
	inumbuff = inInputData->mNumberBuffers;

      for (i = 0; i < inumbuff; i++)
	{
	  asysi_buffptr[i] = (Float32 *) inInputData->mBuffers[i].mData;
	  asysi_buffchan[i] = inInputData->mBuffers[i].mNumberChannels;
	}

      asysn_coreaudio_live = asys_mac_iorun(asyso_buffptr, asyso_buffchan,
					    onumbuff, asysi_buffptr, 
					    asysi_buffchan, inumbuff, frames);

      /* inform main thread of normal exit or error */

      if (asysn_coreaudio_live != ASYS_DONE)
	{
	  retcode = ASYSN_COREAUDIO_PIPE_OK;
	  if (write(asysn_coreaudio_pipepair[1], &retcode, sizeof(UInt32)) < 0)
	    perror("Error writing asys_main pipe");
	}
    }
  else
    {
      /* silience at start of program */

      if (asysn_coreaudio_silentframes && !(--asysn_coreaudio_silentframes))
	asysn_coreaudio_live = ASYS_DONE;
    }

  /* set up cpuload timing state for next time */

  asysn_coreaudio_ksync_then = (AudioGetCurrentHostTime() - 
				asysn_coreaudio_ksync_then);
  return kAudioHardwareNoError;
}

#endif  /* ASYS_ACTIVE_IO, coreaudio input and output */


#if defined(ASYS_ACTIVE_IO) && !defined(ASYS_HASOUTPUT)

/********************************************************************/
/*      coreaudio input and output, from the same audio device      */
/********************************************************************/

OSStatus asysn_coreaudio_only_input(AudioDeviceID inDevice, 
				      const AudioTimeStamp * inNow,
				      const AudioBufferList * inInputData,
				      const AudioTimeStamp * inInputTime,
				      AudioBufferList * outOutputData,
				      const AudioTimeStamp * outOutputTime, 
				      void * inClientData)
{
  UInt32 inumbuff = ASYSI_MAXSTREAM;
  UInt32 i, frames, retcode;

  /* exit gracefully if audio input goes away */

  if ((inInputTime->mFlags & kAudioTimeStampHostTimeValid) &&
      (inInputTime->mHostTime == 0))
    return kAudioHardwareNoError;

  /* set up cpuload timing state */

  if (inNow->mFlags & kAudioTimeStampHostTimeValid)
    asysn_coreaudio_ksync_then = inNow->mHostTime - asysn_coreaudio_ksync_then;
  else
    asysn_coreaudio_ksync_then = (AudioGetCurrentHostTime() -
				  asysn_coreaudio_ksync_then);

  if (asysn_coreaudio_live == ASYS_DONE)
    {
      /* set up pointers for asys_mac_irun() call, then make call */

      if (inInputData->mNumberBuffers <= ASYSI_MAXSTREAM)
	inumbuff = inInputData->mNumberBuffers;

      for (i = 0; i < inumbuff; i++)
	{
	  asysi_buffptr[i] = (Float32 *) inInputData->mBuffers[i].mData;
	  asysi_buffchan[i] = inInputData->mBuffers[i].mNumberChannels;
	}

      if (asysi_buffchan[0] <= 2)
	frames = inInputData->mBuffers[0].mDataByteSize >> (1 + asysi_buffchan[0]);
      else
	frames = inInputData->mBuffers[0].mDataByteSize/(asysi_buffchan[0] << 2);

      asysn_coreaudio_live = asys_mac_irun(asysi_buffptr, asysi_buffchan, 
					   inumbuff, frames);

      /* inform main thread of normal exit or error */

      if (asysn_coreaudio_live != ASYS_DONE)
	{
	  retcode = ASYSN_COREAUDIO_PIPE_OK;
	  if (write(asysn_coreaudio_pipepair[1], &retcode, sizeof(UInt32)) < 0)
	    perror("Error writing asys_main pipe");
	}
    }
  else
    {
      /* silience at start of program */

      if (asysn_coreaudio_silentframes && !(--asysn_coreaudio_silentframes))
	asysn_coreaudio_live = ASYS_DONE;
    }

  /* set up cpuload timing state for next time */

  asysn_coreaudio_ksync_then = (AudioGetCurrentHostTime() - 
				asysn_coreaudio_ksync_then);
  return kAudioHardwareNoError;
}

#endif  /* ASYS_ACTIVE_IO, coreaudio input only */


/*~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*   setup function suite  */
/*-------------------------*/

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/* todo: rework this function for x86/lion. */
/*       for now, we "if 0" the entire body */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
 
/********************************************************************/
/*                    page in working set                           */
/********************************************************************/

void asysn_coreaudio_page()
{
  UInt32 psize, pagetop, addrtop, touchtop, vm_total, err;
  unsigned int statsize = HOST_VM_INFO_COUNT;
  vm_statistics_data_t vm_stat;
  struct vm_region_basic_info rinfo;
  mach_msg_type_number_t rinfo_count;
  mach_port_t object_name;
  vm_address_t raddr;
  vm_size_t rsize;

#if 0

  /**********************************/
  /* get current system paging info */
  /**********************************/

  asysn_coreaudio_pageinfo = ASYSN_COREAUDIO_PAGEIN;

  if (host_statistics(mach_host_self(), HOST_VM_INFO, 
		      (host_info_t) &vm_stat, &statsize))
    {
      asysn_coreaudio_pageinfo = ASYSN_COREAUDIO_STATFAIL;
      return;
    }

  vm_total = (vm_stat.free_count + vm_stat.active_count
	      + vm_stat.inactive_count); 

  /***********************************/
  /* compute highest page, advise OS */
  /***********************************/

  psize = getpagesize();
  pagetop = (get_end() - 1)/psize;

  madvise((caddr_t) psize, (pagetop - 1)*psize, MADV_WILLNEED);

  if ((pagetop - 1 + ASYSN_COREAUDIO_TOUCHMARGIN) <= vm_total)
    touchtop = pagetop;
  else
    {
      touchtop = vm_total + 1;
      asysn_coreaudio_pageinfo = ASYSN_COREAUDIO_SHORTRAM;
    }
  
  /***********************************************************/
  /* touch every page except page 0 -- we crash in this loop */
  /***********************************************************/

  for (addrtop = touchtop*psize; addrtop >= psize; addrtop -= psize)
    asysn_coreaudio_touch ^= *((int *) addrtop);

  /******************************/
  /* attempt to do page locking */
  /******************************/

  do {
    err = 1;

    /* can only lock if enough room */
    
    if ((pagetop - 1 + ASYSN_COREAUDIO_LOCKMARGIN) > vm_total)
      break; err++;
    
    addrtop = psize;

    while (addrtop < get_end())
      {
	raddr = addrtop;
	rinfo_count = VM_REGION_BASIC_INFO_COUNT;
    
	if (vm_region(mach_task_self(), &raddr, &rsize, VM_REGION_BASIC_INFO, 
		      (vm_region_info_t) &rinfo, &rinfo_count, &object_name))
	  break;

	if (raddr >= get_end())
	  break;

	if ((raddr + rsize) > get_end())
	  rsize = get_end() - raddr;

	if (vm_wire(mach_host_self(), mach_task_self(), 
		    (vm_address_t) raddr, (vm_size_t) rsize,
		    rinfo.protection))
	  break;

	addrtop = raddr + rsize;
      }

  } while ((err = 0));

  asysn_coreaudio_pagelock = (err == 0);
  asysn_coreaudio_pagetot  = pagetop;

#else

  /* don't print a paging-info banner */

  asysn_coreaudio_pageinfo = ASYSN_COREAUDIO_NOPRINT;

#endif

}

/********************************************************************/
/*             setup device-independent properties                  */
/********************************************************************/

int asysn_coreaudio_globalsetup()

{
  UInt32 state = 1;

  /* set up pipe to main thread */

  if (pipe(asysn_coreaudio_pipepair))
    ASYSIO_ERROR_RETURN("Cannot open end-of-audio pipe");

  /* let HAL know the main thread sleeps */

  if (AudioHardwareSetProperty(kAudioHardwarePropertySleepingIsAllowed,
			       sizeof(UInt32), &state))
    ASYSIO_ERROR_RETURN("Cannot notify HAL of sleep status");

  return ASYS_DONE;
}

/********************************************************************/
/*               check for requested device type                    */
/********************************************************************/

int asysn_coreaudio_devicesetup(AudioDeviceID * aid, int input)

{
  UInt32 kopen, size, state;
  char * message;

  /* see if device exists */

  size = sizeof(AudioDeviceID);
  kopen = (input ? kAudioHardwarePropertyDefaultInputDevice :
	   kAudioHardwarePropertyDefaultOutputDevice);

  if (AudioHardwareGetProperty(kopen, &size, aid))
    {
      message = (input ? "Default input device not found" :
		 "Default output device not found");
      ASYSIO_ERROR_RETURN(message);
    }

  /* see if device is ready to use */

  size = sizeof(UInt32);

  if (AudioDeviceGetProperty(*aid, 0, input, kAudioDevicePropertyDeviceIsAlive, 
			     &size, &state))
    {
      message = (input ? 
		 "Cannot sense if input device is ready to use" :
		 "Cannot sense if output device is ready to use");
      ASYSIO_ERROR_RETURN(message);
    }

  if (!state)
    {
      message = (input ? 
		 "Input device is not ready to use" :
		 "Output device is not ready to use");
      ASYSIO_ERROR_RETURN(message);
    }

  if (input)
    asysn_coreaudio_instatus = ASYSN_COREAUDIO_INPUT_IDEF;

  return ASYS_DONE;
}

/********************************************************************/
/*         returns a device name + manufacturer as a string         */
/********************************************************************/

char * asysn_coreaudio_getdevicename(AudioDeviceID aid, UInt32 input)

{
  Boolean wflag;
  UInt32 devsize, mansize;
  char * devname, * manname, * retstr;

  if (AudioDeviceGetPropertyInfo(aid, 0, input, kAudioDevicePropertyDeviceName, 
				 &devsize, &wflag))
    return NULL;
  
  devname = malloc(devsize);
  
  if (AudioDeviceGetProperty(aid, 0, input, kAudioDevicePropertyDeviceName, 
			     &devsize, devname))
    return NULL;

  if (AudioDeviceGetPropertyInfo(aid, 0, input, kAudioDevicePropertyDeviceManufacturer, 
				 &mansize, &wflag))
    return NULL;
  
  manname = malloc(mansize);
  
  if (AudioDeviceGetProperty(aid, 0, input, kAudioDevicePropertyDeviceManufacturer, 
			     &mansize, manname))
    return NULL;

  retstr = calloc(mansize + devsize + 16, 1);

  sprintf(retstr, "%s (%s)", devname, manname);

  free(devname);
  free(manname);
  return retstr;
}

#if defined(ASYS_COREAUDIO_DEBUG)

/********************************************************************/
/*                  print all system devices                        */
/********************************************************************/

int asysn_coreaudio_printdevices(void)

{
  UInt32 size, numdev, i, input;
  AudioDeviceID * aid;
  char * message;
  Boolean wflag;

  /* get size of device list */

  if (AudioHardwareGetPropertyInfo(kAudioHardwarePropertyDevices, &size, &wflag))
    ASYSIO_ERROR_RETURN("Cannot determine size of device array");

  aid = malloc(size);
  numdev = size/sizeof(AudioDeviceID);

  if (AudioHardwareGetProperty(kAudioHardwarePropertyDevices, &size, aid))
    ASYSIO_ERROR_RETURN("Cannot obtain the device array");

  for (i = 0; i < numdev; i++)
    for (input = 0; input <= 1; input++)
      {
	if (AudioDeviceGetPropertyInfo(aid[i], 0, input, kAudioDevicePropertyDeviceName, 
				       &size, &wflag))
	  ASYSIO_ERROR_RETURN("Cannot determine name size");
	
	message = malloc(size);
	
	if (AudioDeviceGetProperty(aid[i], 0, input, kAudioDevicePropertyDeviceName, 
				   &size, message))
	  ASYSIO_ERROR_RETURN("Cannot determine name");
	
	if (AudioDeviceGetPropertyInfo(aid[i], 0, input, kAudioDevicePropertyStreams, 
				       &size, &wflag))
	  ASYSIO_ERROR_RETURN("Cannot determine stream size");

	printf("%s %u: %s [%u stream(s)]\n", input ? "Input" : "Output", i, 
	       message, size/sizeof(AudioStreamID));
	
	free(message);
      }

  return ASYS_DONE;
}

#endif 

#if (defined(ASYS_HASOUTPUT) && defined(ASYS_HASINPUT))

/********************************************************************/
/*     initializes asyso_aid and asysi_aid for coreaudio i + o      */
/********************************************************************/

int asysn_coreaudio_iodevicesetup(void)

{
  UInt32 size, input, state;
  char * message;

  size = sizeof(AudioDeviceID);

  if (AudioHardwareGetProperty(kAudioHardwarePropertyDefaultOutputDevice,
			       &size, &asyso_aid))
    ASYSIO_ERROR_RETURN("Default audio device does not exist");

  size = sizeof(AudioDeviceID);

  asysn_coreaudio_instatus = ASYSN_COREAUDIO_INPUT_ODEF;

  if (!AudioHardwareGetProperty(kAudioHardwarePropertyDefaultInputDevice,
				&size, &asysi_aid) && (asysi_aid == asyso_aid))
    asysn_coreaudio_instatus = ASYSN_COREAUDIO_INPUT_IDEF;


  if (asysi_aid != asyso_aid)
    {
      message = "Driver requires the default device to do input AND output.\n"
	"         Create an aggregate device w/ Audio MIDI Setup; make it the default";
      ASYSIO_ERROR_RETURN(message);
    }

  /* see if device is ready to use */

  for (input = 0; input <= 1; input++)
    {
      size = sizeof(UInt32);

      if (AudioDeviceGetProperty(asyso_aid, 0, input, 
				 kAudioDevicePropertyDeviceIsAlive, &size, &state))
	{
	  message = (input ? 
		     "Cannot sense if input device is ready to use" :
		     "Cannot sense if output device is ready to use");
	  ASYSIO_ERROR_RETURN(message);
	}

      if (!state)
	{
	  message = (input ? 
		     "Input device is not ready to use" :
		     "Output device is not ready to use");
	  ASYSIO_ERROR_RETURN(message);
	}
    }

  return ASYS_DONE;
}

#endif

/********************************************************************/
/*         returns the stream configuration for a device            */
/********************************************************************/

int asysn_coreaudio_streaminfo(AudioDeviceID aid, Boolean input, 
			       UInt32 * numstreams,  /* number of streams     */
			       double * srate,       /* srate of stream 1     */
			       UInt32 * numchan      /* total num channels    */
			       )

{
  UInt32 size, i;
  Boolean wflag, fpcm;
  char * message;
  AudioBufferList * abl;
  AudioStreamBasicDescription desc;
  
 if (AudioDeviceGetPropertyInfo(aid, 0, input, 
				 kAudioDevicePropertyStreamConfiguration,
				 &size, &wflag))
    {
      message = (input ? 
		 "Cannot determine size of input stream configuration" :
		 "Cannot determine size of output stream configuration");
      ASYSIO_ERROR_RETURN(message);
    }

  if ((abl = malloc(size)) == NULL)
    ASYSIO_ERROR_RETURN("Process out of memory");

  if (AudioDeviceGetProperty(aid, 0, input, 
			     kAudioDevicePropertyStreamConfiguration,
			     &size, abl))
    {
      message = (input ? 
		 "Cannot determine input stream configuration" :
		 "Cannot determine output stream configuration");
      ASYSIO_ERROR_RETURN(message);
    }

  if ((*numstreams = abl->mNumberBuffers) == 0)
    {
      message = (input ? 
		 "Audio device has no input streams" :
		 "Audio device has no output streams");
      ASYSIO_ERROR_RETURN(message);
    }

  *numchan = 0;
  fpcm = 1;

  for (i = 0; i < *numstreams; i++)
    {
      (*numchan) += abl->mBuffers[i].mNumberChannels;
      size = sizeof(AudioStreamBasicDescription);

      if (AudioDeviceGetProperty(aid, *numchan, input, 
				 kAudioDevicePropertyStreamFormat,
				 &size, &desc))
	{
	  message = (input ? 
		     "Cannot determine input stream format" :
		     "Cannot determine output stream format");
	  ASYSIO_ERROR_RETURN(message);
	}

      if (!i)
	*srate = desc.mSampleRate;

      fpcm = (fpcm && (desc.mFormatID == kAudioFormatLinearPCM) &&
	      (desc.mFormatFlags & kLinearPCMFormatFlagIsFloat));
    }

  free(abl);

  if (!fpcm)
    {
      message = (input ? 
		 "Audio input device is not linear PCM" :
		 "Audio output device is not linear PCM");
      ASYSIO_ERROR_RETURN(message);
    }

  return ASYS_DONE;
}


/********************************************************************/
/*            tries to reconfigure a one-stream device            */
/********************************************************************/

int asysn_coreaudio_reconfigure(AudioDeviceID aid, Boolean input, 
				int srate, double * core_srate)

{
  AudioStreamBasicDescription desc;
  UInt32 size;
  Boolean wflag;
  
  /* leave if the stream format is not writable */

 if (AudioDeviceGetPropertyInfo(aid, 1, input, 
				kAudioDevicePropertyStreamFormat,
				&size, &wflag) || (wflag == 0))
   return ASYS_DONE;

  /* get closest match */

  memset(&desc, 0, (size = sizeof(desc)));
  desc.mSampleRate = (Float64) srate;

  if (AudioDeviceGetProperty(aid, 1, input, 
			     kAudioDevicePropertyStreamFormatMatch, 
			     &size, &desc))
    return ASYS_DONE;
      
  /* don't change if it won't make any difference */

  if (*core_srate == desc.mSampleRate)
    return ASYS_DONE;

  /* make the change and exit */

  if (AudioDeviceSetProperty(aid, 0, 1, input, 
			     kAudioDevicePropertyStreamFormat, 
			     sizeof(desc), &desc))
    ASYSIO_ERROR_RETURN("Error reconfiguring the stream");

  *core_srate = desc.mSampleRate;
 	
  return ASYS_DONE;
}


/********************************************************************/
/*                   tries to set buffer latency                    */
/********************************************************************/

int asysn_coreaudio_latencyset(AudioDeviceID aid, Boolean input, 
			       double core_srate)
{
  Float32 seconds = ASYS_LATENCY;
  AudioValueRange framerange;
  UInt32 size, framesize;

  /* use user input or constants to set nominal latency */

  if (!ASYS_USERLATENCY)
    {
      if (ASYS_LATENCYTYPE == ASYS_HIGHLATENCY)
	seconds = ASYSN_COREAUDIO_HIGHLATENCY;
      else
	seconds = ASYSN_COREAUDIO_LOWLATENCY;
    }
  framesize = (UInt32)(seconds*core_srate + 0.5);

  /* clip to CoreAudio frame limits */

  size = sizeof(AudioValueRange);

  if (AudioDeviceGetProperty(aid, 0, input, 
			     kAudioDevicePropertyBufferFrameSizeRange,
			     &size, &framerange))
    ASYSIO_ERROR_RETURN("Cannot determine current framerange");

  framesize = (framesize > framerange.mMinimum) ? framesize : framerange.mMinimum;
  framesize = (framesize < framerange.mMaximum) ? framesize : framerange.mMaximum;

  framesize = ((framesize < ASYSN_COREAUDIO_TRUEMAXFRAMES) ? 
	       framesize : ASYSN_COREAUDIO_TRUEMAXFRAMES);

  /* set the final latency value */

  if (AudioDeviceSetProperty(aid, 0, 1, input, 
			     kAudioDevicePropertyBufferFrameSize, 
			     sizeof(UInt32), &framesize))
    ASYSIO_ERROR_RETURN("Error reconfiguring the framesize");

  size = sizeof(UInt32);

  if (AudioDeviceGetProperty(aid, 0, input, 
			     kAudioDevicePropertyBufferFrameSize,
			     &size, &framesize))
    ASYSIO_ERROR_RETURN("Cannot determine current framerange");

  /* set leader-tape length */

  asysn_coreaudio_silentframes = ((int)(ASYSN_COREAUDIO_LEADERTIME*
					core_srate/framesize)); 
  asysn_coreaudio_live = ASYS_EXIT;

  /* for printout routine */

  asysn_coreaudio_frame_seconds = (float)(framesize/core_srate);
  asysn_coreaudio_frame_samples = framesize;

  return ASYS_DONE;
}


/********************************************************************/
/*               print coreaudio banner to stdout                   */
/********************************************************************/

void asysn_coreaudio_printbanner(void)
{
  char * name;

  printf("\n");

#if defined(ASYS_ACTIVE_O) || (defined(ASYS_ACTIVE_IO) && defined(ASYS_HASOUTPUT))
  if ((name = asysn_coreaudio_getdevicename(asyso_aid, 0)))
    {
      printf("CoreAudio Output: %s.\n", name);
      if (asysn_coreaudio_outsize_mismatch)
	printf("         WARNING: Discarding extra SAOL outchannels.\n");
      free(name);
    }
#endif

#if defined(ASYS_ACTIVE_IO) && defined(ASYS_HASINPUT)
  if ((name = asysn_coreaudio_getdevicename(asysi_aid, 1)))
    {
      printf("CoreAudio  Input: %s.\n", name);
      if (asysn_coreaudio_insize_mismatch)
	printf("         WARNING: Discarding extra CoreAudio inputs.\n");
      if (asysn_coreaudio_instatus == ASYSN_COREAUDIO_INPUT_ODEF)
	printf("         WARNING: Using output device as input "
	       "(Multi-device I/O unsupported).");
      free(name);
    }
#endif

  printf("\n");
  printf("Buffer latency: %fs [%u frames]. Adjust with sfront -latency option.\n",
	  asysn_coreaudio_frame_seconds, 
	 (unsigned int) asysn_coreaudio_frame_samples);

  if (asysn_coreaudio_pagelock)
    printf("%i pages locked into RAM.\n", asysn_coreaudio_pagetot);
  else
    {
      switch (asysn_coreaudio_pageinfo) {
      case ASYSN_COREAUDIO_STATFAIL:
	printf("Failed to page in working set: expect artifacts\n");
	break;
      case ASYSN_COREAUDIO_SHORTRAM:
	printf("Not enough RAM to page in full working set: expect artifacts\n");
	break;
      case ASYSN_COREAUDIO_PAGEIN:
	printf("Working set (%i pages) paged in RAM; could not lock pages.\n",
	       asysn_coreaudio_pagetot);
	break;
      }
    }

  printf("\n");
}

/********************************************************************/
/*             generic coreaudio device listenerproc                */
/********************************************************************/

OSStatus asysn_coreaudio_devlisten(AudioDeviceID inDevice, 
				   UInt32 inChannel,
				   Boolean isInput,
				   AudioDevicePropertyID inPropertyID,
				   void * inClientData)
{
  UInt32 retcode = ASYSN_COREAUDIO_PIPE_LPROC; 
  UInt32 size = sizeof(UInt32);
  UInt32 state;

  /* for now, only monitoring properties that return UInt32's   */
  /* this will need to be more general for arbitrary properties */

  if (AudioDeviceGetProperty(inDevice, inChannel, isInput, inPropertyID, 
			     &size, &state))
    return kAudioHardwareNoError;

  if ((inPropertyID == kAudioDevicePropertyDeviceIsAlive) && state)
    return kAudioHardwareNoError;

  if (write(asysn_coreaudio_pipepair[1], &retcode, sizeof(UInt32)) < 0)
    perror("Error writing asys_main pipe");

  return kAudioHardwareNoError;
}

/********************************************************************/
/*              set up listener procs for a device                  */
/********************************************************************/

int asysn_coreaudio_setup_dlisten(AudioDeviceID aid, int input)
     
{
  /* for testing purpose: mute key emulates device disconnect */

#if (ASYSN_COREAUDIO_LPROC_MUTETEST)

  if (AudioDeviceAddPropertyListener(aid, 0, input, 
				     kAudioDevicePropertyMute,
				     asysn_coreaudio_devlisten,
				     (void *) NULL))
    ASYSIO_ERROR_RETURN("Cannot add new PropertyListener to device");

#endif

  /* monitor device disconnect property */

  if (AudioDeviceAddPropertyListener(aid, 0, input, 
				     kAudioDevicePropertyDeviceIsAlive,
				     asysn_coreaudio_devlisten,
				     (void *) NULL))
    ASYSIO_ERROR_RETURN("Cannot add new PropertyListener to device");

  return ASYS_DONE;
}


/********************************************************************/
/*              shutdown listener procs for a device                */
/********************************************************************/

int asysn_coreaudio_shutdown_dlisten(AudioDeviceID aid, int input)

{
  /* remove all properties added in _setup_ above */

#if (ASYSN_COREAUDIO_LPROC_MUTETEST)

  if (AudioDeviceRemovePropertyListener(aid, 0, input, 
					kAudioDevicePropertyMute,
					asysn_coreaudio_devlisten))
    ASYSIO_ERROR_RETURN("Cannot remove PropertyListener from device");
  
#endif
  
  if (AudioDeviceRemovePropertyListener(aid, 0, input, 
					kAudioDevicePropertyDeviceIsAlive,
					asysn_coreaudio_devlisten))
    ASYSIO_ERROR_RETURN("Cannot remove PropertyListener from device");

  return ASYS_DONE;
}

#if (defined(ASYS_HASOUTPUT) && !defined(ASYS_HASINPUT))

/********************************************************************/
/* coreaudio audio output only: ASYS_ACTIVE_O + one ASYS_ACTIVE_IO  */
/********************************************************************/

int asys_osetup(int srate, int ochannels, int osample, 
		char * oname, int toption)

{
  Float64 core_srate;
  UInt32 core_numchan, numstreams;

  asysn_coreaudio_page();

  if (asysn_coreaudio_globalsetup())
    return ASYS_ERROR;

  if (asysn_coreaudio_devicesetup(&asyso_aid, 0))
    return ASYS_ERROR;

  if (asysn_coreaudio_streaminfo(asyso_aid, 0, &numstreams, &core_srate,
				 &core_numchan)) 
    return ASYS_ERROR;

  if (((int) core_srate) != srate)
    {
      if (asysn_coreaudio_reconfigure(asyso_aid, 0, srate, &core_srate))
	return ASYS_ERROR;

      if (((int) core_srate) != srate)
	{
	  printf("Error: CoreAudio cannot handle SAOL srate %i\n", (int) srate);
	  printf("       Change SAOL program srate to %i\n", (int) core_srate);
	  return ASYS_ERROR;
	}

      if (asysn_coreaudio_streaminfo(asyso_aid, 0, &numstreams, &core_srate,
				     &core_numchan)) 
	return ASYS_ERROR;
    }

  if (asysn_coreaudio_latencyset(asyso_aid, 0, core_srate))
      return ASYS_ERROR;

  if (AudioDeviceAddIOProc(asyso_aid, asysn_coreaudio_only_output,
			   (void *) NULL))
    ASYSIO_ERROR_RETURN("Cannot add new IOProc to device");

  asysn_coreaudio_outsize_mismatch = ((ochannels < core_numchan) &&
				      (ochannels > 1));

  if (asysn_coreaudio_setup_dlisten(asyso_aid, 0))
    return ASYS_ERROR;

  asysn_coreaudio_printbanner();

  return ASYS_DONE;
}

#endif /* CoreAudio output, no input or passive input */


#if (!defined(ASYS_HASOUTPUT) && defined(ASYS_HASINPUT))

/****************************************************************/
/* coreaudio audio input only: an ASYS_ACTIVE_IO case           */
/****************************************************************/

int asys_isetup(int srate, int ichannels, int isample, 
		char * iname, int toption)

{
  UInt32 in_numchan, in_numstreams;
  Float64 in_srate;

  asysn_coreaudio_page();

  if (asysn_coreaudio_globalsetup())
    return ASYS_ERROR;

  if (asysn_coreaudio_devicesetup(&asysi_aid, 0))
    return ASYS_ERROR;

  if (asysn_coreaudio_streaminfo(asysi_aid, 1, &in_numstreams, &in_srate,
				 &in_numchan)) 
    return ASYS_ERROR;

  if (((int) in_srate) != srate)
    {
      if (asysn_coreaudio_reconfigure(asysi_aid, 1, srate, &in_srate))
	return ASYS_ERROR;

      if (((int) in_srate) != srate)
	{
	  printf("Error: CoreAudio cannot handle SAOL srate %i\n",srate);
	  printf("       Change SAOL program srate to %i\n", (int) in_srate);
	  return ASYS_ERROR;
	}

      if (asysn_coreaudio_streaminfo(asysi_aid, 1, &in_numstreams, &in_srate,
				     &in_numchan)) 
	return ASYS_ERROR;
    }

  if (asysn_coreaudio_latencyset(asysi_aid, 1, in_srate))
      return ASYS_ERROR;

  if (AudioDeviceAddIOProc(asysi_aid, asysn_coreaudio_only_input,
			   (void *) NULL))
    ASYSIO_ERROR_RETURN("Cannot add new IOProc to device");

  asysn_coreaudio_insize_mismatch = ((ichannels < in_numchan) && 
				     (in_numchan > 1));

  if (asysn_coreaudio_setup_dlisten(asysi_aid, 1))
    return ASYS_ERROR;

  asysn_coreaudio_printbanner();

  return ASYS_DONE;
}

#endif


#if (defined(ASYS_HASOUTPUT) && defined(ASYS_HASINPUT))

/****************************************************************/
/*   coreaudio input and output: an ASYS_ACTIVE_IO case         */
/****************************************************************/

int asys_iosetup(int srate, int ichannels, int ochannels,
		 int isample, int osample, 
		 char * iname, char * oname, int toption)
{
  UInt32 in_numchan, out_numchan, in_numstreams, out_numstreams;
  Float64 in_srate, out_srate;

  asysn_coreaudio_page();

  if (asysn_coreaudio_globalsetup())
    return ASYS_ERROR;

  if (asysn_coreaudio_iodevicesetup())
    return ASYS_ERROR;

  if (asysn_coreaudio_streaminfo(asyso_aid, 0, &out_numstreams, &out_srate,
				 &out_numchan)) 
    return ASYS_ERROR;

  if (asysn_coreaudio_streaminfo(asysi_aid, 1, &in_numstreams, &in_srate,
				 &in_numchan)) 
    return ASYS_ERROR;

  if ((((int) in_srate) != srate) || (((int) out_srate) != srate))
    {
      if (asysn_coreaudio_reconfigure(asyso_aid, 0, srate, &out_srate))
	return ASYS_ERROR;

      if (asysn_coreaudio_reconfigure(asysi_aid, 1, srate, &in_srate))
	return ASYS_ERROR;

      if ((((int) in_srate) != srate) || (((int) out_srate) != srate))
	ASYSIO_ERROR_RETURN("Audio output device srate mismatch");

      if (asysn_coreaudio_streaminfo(asyso_aid, 0, &out_numstreams, &out_srate,
				     &out_numchan)) 
	return ASYS_ERROR;

      if (asysn_coreaudio_streaminfo(asysi_aid, 1, &in_numstreams, &in_srate,
				     &in_numchan)) 
	return ASYS_ERROR;
    }

  if (asysn_coreaudio_latencyset(asyso_aid, 0, out_srate))
      return ASYS_ERROR;

  if (AudioDeviceAddIOProc(asyso_aid, asysn_coreaudio_input_output,
			   (void *) NULL))
    ASYSIO_ERROR_RETURN("Cannot add new IOProc to device");

  asysn_coreaudio_outsize_mismatch = ((ochannels < out_numchan) &&
				      (ochannels > 1));

  asysn_coreaudio_insize_mismatch = ((ichannels < in_numchan) && 
				     (in_numchan > 1));

  if (asysn_coreaudio_setup_dlisten(asyso_aid, 0))
    return ASYS_ERROR;

  if (asysn_coreaudio_setup_dlisten(asysi_aid, 1))
    return ASYS_ERROR;

  asysn_coreaudio_printbanner();

  return ASYS_DONE;
}

#endif

/*~~~~~~~~~~~~~~~~~~~~~~~*/
/*   asys_main function  */
/*-----------------------*/

/********************************************************************/
/*        asys_main: starts IOProc, waits for error or exit         */
/********************************************************************/
 
void asys_main(void)

{
  UInt32 retcode;
  int ret, pval, retry;
  fd_set pset;

#if defined(ASYS_ACTIVE_IO) && !defined(ASYS_HASINPUT)

  /* set up passive input */

  asysn_coreaudio_ibuf = NULL;
  asysn_coreaudio_iptr  = 0;
  asysn_coreaudio_ilast = 0;

  if ((asys_getbuf(&asysn_coreaudio_ibuf, &asysn_coreaudio_ilast) 
       != ASYS_DONE) || (asysn_coreaudio_ilast == 0))
    return;

#endif

#if defined(ASYS_ACTIVE_IO) && !defined(ASYS_HASOUTPUT)

  /* set up passive output */

  asysn_coreaudio_obuf = NULL;
  asysn_coreaudio_optr  = 0;
  asysn_coreaudio_olast = 0;

  if (asys_preamble(&asysn_coreaudio_obuf,&asysn_coreaudio_olast) != ASYS_DONE)
    return;

#endif

  /* start the correct device */
 
#if defined(ASYS_ACTIVE_O) || (defined(ASYS_ACTIVE_IO) && !defined(ASYS_HASINPUT))

  if (AudioDeviceStart(asyso_aid, asysn_coreaudio_only_output))
    ASYSIO_ERROR_EMPTYRETURN("Cannot start audio output");

#endif /* ASYS_ACTIVE_O */

#if defined(ASYS_ACTIVE_IO) && defined(ASYS_HASINPUT) && defined(ASYS_HASOUTPUT)

  if (AudioDeviceStart(asyso_aid, asysn_coreaudio_input_output))
    ASYSIO_ERROR_EMPTYRETURN("Cannot start audio input/output");

#endif /* ASYS_HASINPUT && ASYS_HASOUTPUT */

#if defined(ASYS_ACTIVE_IO) && !defined(ASYS_HASOUTPUT)

  if (AudioDeviceStart(asysi_aid, asysn_coreaudio_only_input))
    ASYSIO_ERROR_EMPTYRETURN("Cannot start audio input");

#endif /* ASYS_ACTIVE_IO && !ASYS_HASOUTPUT */
 
  /* wait for interesting things to happen ... */

  pval = asysn_coreaudio_pipepair[0];

  do {
    FD_ZERO(&pset);
    FD_SET(pval, &pset);
    ret = select(pval + 1, &pset, NULL, NULL, NULL);

    /* ignore signal-interrupted select()'s */
    
    if ((ret < 0) && ((errno == EINTR) || (errno == EAGAIN)))
      continue;
    
    /* check for normal end */
    
    if ((ret == 1) && (FD_ISSET(pval, &pset)))
      {
	if (read(pval, &retcode, sizeof(UInt32)) == sizeof(UInt32))
	  if (retcode == ASYSN_COREAUDIO_PIPE_LPROC)
	    printf("Error: CoreAudio HAL requested shutdown.\n\n");

	break;
      }

    printf("Error: Abnormal select() in asys_main.\n\n");
    break;

  } while (1);
}


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*   shutdown function suite   */
/*-----------------------------*/

#if (defined(ASYS_HASOUTPUT) && !defined(ASYS_HASINPUT))

/********************************************************************/
/*         shutdown function for audio output only scenareo         */
/********************************************************************/

void asys_oshutdown(void)
{
  if (AudioDeviceStop(asyso_aid, asysn_coreaudio_only_output))
    ASYSIO_ERROR_EMPTYRETURN("Cannot stop audio output");

  if (asysn_coreaudio_shutdown_dlisten(asyso_aid, 0))
    return;

  close(asysn_coreaudio_pipepair[0]);
  close(asysn_coreaudio_pipepair[1]);
}

#endif


#if (!defined(ASYS_HASOUTPUT)&&(defined(ASYS_HASINPUT)))

/****************************************************************/
/*              shuts down audio input device                   */
/****************************************************************/

void asys_ishutdown(void)

{
  if (AudioDeviceStop(asysi_aid, asysn_coreaudio_only_input))
    ASYSIO_ERROR_EMPTYRETURN("Cannot stop audio input");

  if (asysn_coreaudio_shutdown_dlisten(asysi_aid, 1))
    return;

  close(asysn_coreaudio_pipepair[0]);
  close(asysn_coreaudio_pipepair[1]);
}

#endif


#if (defined(ASYS_HASOUTPUT)&&(defined(ASYS_HASINPUT)))

/****************************************************************/
/*              shuts down audio input and output device        */
/****************************************************************/

void asys_ioshutdown(void)

{
  if (AudioDeviceStop(asyso_aid, asysn_coreaudio_input_output))
    ASYSIO_ERROR_EMPTYRETURN("Cannot stop audio output");

  if (asysn_coreaudio_shutdown_dlisten(asyso_aid, 0))
    return;

  if (asysn_coreaudio_shutdown_dlisten(asysi_aid, 1))
    return;

  close(asysn_coreaudio_pipepair[0]);
  close(asysn_coreaudio_pipepair[1]);
}

#endif


/*~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*   ksync function suite  */
/*-------------------------*/

#if defined(ASYS_KSYNC)

float asysn_coreaudio_ksync_normal;   /* normalization value */
float asysn_coreaudio_ksync_lastret;  /* swap-out heuristic  */

/***********************************************************/
/*         initializes k-rate boundaries sync              */
/***********************************************************/

void ksyncinit()

{
  asysn_coreaudio_ksync_then = 0;
  asysn_coreaudio_ksync_normal = (float) (EV(KRATE)/AudioGetHostClockFrequency());
}

/***********************************************************/
/*         synchronizes on k-rate boundaries               */
/***********************************************************/

float ksync()

{
  UInt64 now = AudioGetCurrentHostTime();
  float ret;

  ret = (((float)(now - asysn_coreaudio_ksync_then))*
	 asysn_coreaudio_ksync_normal);

  if (ret > 1.0F)
    ret = 1.0F;

  if (ret < 1.0F)
    asysn_coreaudio_ksync_lastret = ret;
  else
    {
      if (asysn_coreaudio_ksync_lastret < 1.0F)
	ret = asysn_coreaudio_ksync_lastret;
      else
	ret = 1.0F;
      asysn_coreaudio_ksync_lastret = 1.0F;
    }

  asysn_coreaudio_ksync_then = now;
  return ret;
}

#endif  /* ASYS_KSYNC */

#undef  ASYSIO_ERROR_REPORT
#undef  ASYSIO_ERROR_RETURN
#undef  ASYSIO_ERROR_EMPTYRETURN


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*                    end of coreaudio driver                   */
/*______________________________________________________________*/
