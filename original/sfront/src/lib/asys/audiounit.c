
/*
#    Sfront, a SAOL to C translator    
#    This file: audiounit audio driver for sfront
#
# Copyright (c) 1999-2008, Regents of the University of California
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
/*           Apple AudioUnit audio driver for sfront            */
/****************************************************************/

/*~~~~~~~~~~~~~*/
/* debug level */
/*_____________*/

/*  Level 0:  No debugging messages            */
/*  Level 1:  Session setup and error messages */
/*  Level 2:  + All MIDI events                */
/*  Level 3:  + All Rendering calls            */

#if !defined(ASYS_AUDIOUNIT_DEBUG_LEVEL)
#define ASYS_AUDIOUNIT_DEBUG_LEVEL 0
#endif

/*~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/* output clipping behavior */
/*__________________________*/

#define ASYS_AUDIOUNIT_OUTPUT_CLIPPING  0

/*~~~~~~~~~~~~~~~~~~~~~*/
/* aucontrol detection */
/*_____________________*/

#if defined(CSYS_CDRIVER_AUCONTROL) || defined(CSYS_CDRIVER_AUCONTROLM)
#define ASYS_AUDIOUNIT_HAS_AUCONTROL  1
#else
#define ASYS_AUDIOUNIT_HAS_AUCONTROL  0
#endif

#if defined(CSYS_CDRIVER_AUCONTROLM)
#define ASYS_AUDIOUNIT_MIDISUPPORT  1
#else
#define ASYS_AUDIOUNIT_MIDISUPPORT  0
#endif

/*~~~~~~~~~~~~~~~~~~~*/
/* reentrant defines */
/*___________________*/

#if (ENGINE_STYLE == ENGINE_REENTRANT)
#define MY_ENGINE_PTR        My->ENGINE_PTR
#define MY_ENGINE_PTR_COMMA  My->ENGINE_PTR,
#define MY_ENGINE_PTR_ASSIGNED_TO  My->ENGINE_PTR = 
#define MY_ENGINE_PTR_DESTROY_SEMICOLON  free(MY_ENGINE_PTR);
#define MY_ENGINE_PTR_IS_NULL (MY_ENGINE_PTR == NULL)
#define MY_ENGINE_PTR_IS_NOT_NULL (MY_ENGINE_PTR != NULL)
#define ENGINE_MEMSTATUS_SEMICOLON(x) \
        asysn_audiounit_memstatus(ENGINE_PTR, ENGINE_SIZE, x);
#else
#define MY_ENGINE_PTR 
#define MY_ENGINE_PTR_COMMA 
#define MY_ENGINE_PTR_ASSIGNED_TO  
#define MY_ENGINE_PTR_DESTROY_SEMICOLON 
#define MY_ENGINE_PTR_IS_NULL (0)
#define MY_ENGINE_PTR_IS_NOT_NULL (1)
#define ENGINE_MEMSTATUS_SEMICOLON(x) 
#endif

/*~~~~~~~~~~~~~~~~~*/
/* include headers */
/*_________________*/

#if !defined(__COREAUDIO_USE_FLAT_INCLUDES__)
#include <CoreServices/CoreServices.h>
#include <AudioUnit/AudioUnit.h>
#include <AudioToolbox/AudioUnitUtilities.h>
#else
#include <ConditionalMacros.h>
#include <CoreServices.h>
#include <AudioUnit.h>
#include <AudioUnitUtilities.h>
#endif

/* 64-bit integer headers */

#include <stdint.h>

/* for control driver socket system */

#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/uio.h>
#include <fcntl.h>

/* include files for memory paging */

#include <sys/mman.h>

/* include files for synchronization */

#include <libkern/OSAtomic.h>

/*~~~~~~~~~~~~~~~~~~*/
/* endian detection */
/*__________________*/

Float32 asysn_audiounit_float32_endian_test = -1;

#define ASYS_AUDIOUNIT_FLOAT32_BIGENDIAN \
        ((((char *)(&asysn_audiounit_float32_endian_test))[0]) != 0)
 
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*     audiounit defines     */
/*___________________________*/

/* after if if-tree runs, only one audio type should be 1 */

#if (!defined(ASYS_HASINPUT) && defined(ASYS_HASOUTPUT) && defined(ASYS_ACTIVE_O))
#define ASYS_AUDIOUNIT_MUSICDEVICE 1
#else
#define ASYS_AUDIOUNIT_MUSICDEVICE 0
#endif
                                     
#if (defined(ASYS_HASINPUT) && defined(ASYS_HASOUTPUT) && defined(ASYS_ACTIVE_IO))
#define ASYS_AUDIOUNIT_EFFECT 1
#else
#define ASYS_AUDIOUNIT_EFFECT 0
#endif


/*~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*  The Wiretap Macro Calls */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/********************/
/* level one macros */
/********************/

#if (ASYS_AUDIOUNIT_DEBUG_LEVEL >= 1)
 
#define ASYS_AUDIOUNIT_WIRETAP(a, b) do {\
      asysn_audiounit_wiretap(a, b);\
      } while (0)
#define ASYS_AUDIOUNIT_WIRETAP_PUTSTRING(a) do {\
      asysn_audiounit_wiretap_putstring(a);\
      } while (0)
#define ASYS_AUDIOUNIT_WIRETAP_PRINT_COMPONENT_RESULT(a) do {\
      asysn_audiounit_wiretap_print_component_result(a);\
      } while (0)

#else

#define ASYS_AUDIOUNIT_WIRETAP(a, b) do { } while (0)
#define ASYS_AUDIOUNIT_WIRETAP_PUTSTRING(a) do { } while (0)
#define ASYS_AUDIOUNIT_WIRETAP_PRINT_COMPONENT_RESULT(a) do { } while (0)

#endif

/********************/
/* level two macros */
/********************/

#if (ASYS_AUDIOUNIT_DEBUG_LEVEL >= 2)

#define ASYS_AUDIOUNIT_WIRETAP_MIDIEVENT(a, b, c, d, e, f) do {\
      asysn_audiounit_wiretap_midievent(a, b, c, d, e, f);\
      } while (0)
#define ASYS_AUDIOUNIT_WIRETAP_M(a, b) do {\
      asysn_audiounit_wiretap(a, b);\
      } while (0)
#define ASYS_AUDIOUNIT_WIRETAP_GETPARAMETER(a, b, c, d, e, f) do {\
      asysn_audiounit_wiretap_getparameter(a, b, c, d, e, f);\
      } while (0)
#define ASYS_AUDIOUNIT_WIRETAP_SETPARAMETER(a, b, c, d, e, f, g) do {\
      asysn_audiounit_wiretap_setparameter(a, b, c, d, e, f, g);\
      } while (0)
#else

#define ASYS_AUDIOUNIT_WIRETAP_MIDIEVENT(a, b, c, d, e, f) do { } while (0)
#define ASYS_AUDIOUNIT_WIRETAP_M(a, b) do { } while (0)
#define ASYS_AUDIOUNIT_WIRETAP_GETPARAMETER(a, b, c, d, e, f) do { } while (0)
#define ASYS_AUDIOUNIT_WIRETAP_SETPARAMETER(a, b, c, d, e, f, g) do { } while (0)

#endif

/**********************/
/* level three macros */
/**********************/

#if (ASYS_AUDIOUNIT_DEBUG_LEVEL >= 3)

#define ASYS_AUDIOUNIT_WIRETAP_RENDERER(a, b, c, d, e, f) do {\
      asysn_audiounit_wiretap_renderer(a, b, c, d, e, f);\
      } while (0)
#define ASYS_AUDIOUNIT_WIRETAP_R(a, b) do {\
      asysn_audiounit_wiretap(a, b);\
      } while (0)

#else

#define ASYS_AUDIOUNIT_WIRETAP_RENDERER(a, b, c, d, e, f) do { } while (0)
#define ASYS_AUDIOUNIT_WIRETAP_R(a, b) do { } while (0)

#endif


/****************************************************************/
/*        Part One: Sfront asys_ dummy function calls           */
/****************************************************************/
        
#if (defined(ASYS_HASOUTPUT) && !defined(ASYS_HASINPUT))

/****************************************************************/
/*        sets up audio output for a given srate/channels       */
/****************************************************************/

int asys_osetup(int srate, int ochannels, int osample, 
		char * oname, int toption) { return ASYS_DONE; }

#endif

#if (!defined(ASYS_HASOUTPUT) && defined(ASYS_HASINPUT))

/****************************************************************/
/*        sets up audio input for a given srate/channels       */
/****************************************************************/

int asys_isetup(int srate, int ichannels, int isample, 
                char * iname, int toption) { return ASYS_DONE; }

#endif

#if (defined(ASYS_HASOUTPUT) && defined(ASYS_HASINPUT))

/****************************************************************/
/*   sets up audio input and output for a given srate/channels  */
/****************************************************************/

int asys_iosetup(int srate, int ichannels, int ochannels,
                 int isample, int osample, char * iname, 
		 char * oname, int toption) { return ASYS_DONE; }
#endif

#if (defined(ASYS_HASOUTPUT)&&(!defined(ASYS_HASINPUT)))

/****************************************************************/
/*                    shuts down audio output                   */
/****************************************************************/

void asys_oshutdown(void)  { }

#endif

#if (!defined(ASYS_HASOUTPUT)&&(defined(ASYS_HASINPUT)))

/****************************************************************/
/*              shuts down audio input device                   */
/****************************************************************/

void asys_ishutdown(void) { }

#endif

#if (defined(ASYS_HASOUTPUT)&&(defined(ASYS_HASINPUT)))

/****************************************************************/
/*              shuts down audio input and output device        */
/****************************************************************/

void asys_ioshutdown(void) { }

#endif

/****************************************************************/
/*        syntactically necessary, but will never be called     */
/****************************************************************/

void asys_main(void) { }

/*~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*   ksync function suite  */
/*-------------------------*/

#if defined(ASYS_KSYNC)

/***********************************************************/
/*         initializes k-rate boundaries sync              */
/***********************************************************/

void ksyncinit() { }

/***********************************************************/
/*         synchronizes on k-rate boundaries               */
/***********************************************************/

float ksync() {  return 0.0F; }  /* encodes "uses no CPU" */

#endif   /* ASYS_KSYNC */


/*************************************************************/
/*         Start of Part Two: The AudioUnit Component        */
/*************************************************************/

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*  AudioUnit Property Constants  */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#define ASYS_AUDIOUNIT_NULL    0
#define ASYS_AUDIOUNIT_MONO    1
#define ASYS_AUDIOUNIT_STEREO  2

#define ASYS_AUDIOUNIT_OUTPUT_MAXCHANNELS ASYS_AUDIOUNIT_STEREO

#if defined(ASYS_HASINPUT)
#define ASYS_AUDIOUNIT_INPUT_MAXCHANNELS  ASYS_AUDIOUNIT_STEREO
#define ASYS_AUDIOUNIT_SUPPORTED_FORMATS  4
#else
#define ASYS_AUDIOUNIT_INPUT_MAXCHANNELS  0
#define ASYS_AUDIOUNIT_SUPPORTED_FORMATS  2
#endif

/* static input and output channels, from SAOL program */

#define ASYS_AUDIOUNIT_OUTPUT_CHANNELS (ENDBUS_output_bus - BUS_output_bus)

#if defined(ASYS_HASINPUT)
#define ASYS_AUDIOUNIT_INPUT_CHANNELS (ENDBUS_input_bus - BUS_input_bus)
#else
#define ASYS_AUDIOUNIT_INPUT_CHANNELS 0
#endif

/* element limits */

#if (ASYS_AUDIOUNIT_EFFECT)

#define ASYS_AUDIOUNIT_ELEMENT_INPUTPREF   1  /* our default */
#define ASYS_AUDIOUNIT_ELEMENT_INPUTMAX    8  /* maximum we accept */

#define ASYS_AUDIOUNIT_ELEMENT_OUTPUTPREF  1  /* our default */
#define ASYS_AUDIOUNIT_ELEMENT_OUTPUTMAX   1  /* maximum we accept */

#define ASYS_AUDIOUNIT_ELEMENT_ARRAYSIZE   8  /* for I-bound data structures */

#else

#define ASYS_AUDIOUNIT_ELEMENT_INPUTPREF   0  /* our default */
#define ASYS_AUDIOUNIT_ELEMENT_INPUTMAX    0  /* maximum we accept */

#define ASYS_AUDIOUNIT_ELEMENT_OUTPUTPREF  1  /* our default */
#define ASYS_AUDIOUNIT_ELEMENT_OUTPUTMAX   1  /* maximum we accept */

#define ASYS_AUDIOUNIT_ELEMENT_ARRAYSIZE   1  /* for I-bound data structures */

#endif

/* maximum number of frames per slice */

#define ASYS_AUDIOUNIT_FRAMES_PER_SLICE  4096

/* in-place processing not supported */

#define ASYS_AUDIOUNIT_INPLACE_PROCESSING 0 

/* number of instruments -- only used by MusicDevice */

#define ASYS_AUDIOUNIT_INSTRUMENT_COUNT  1 

/* constants for property listeners */

#define ASYS_AUDIOUNIT_PROPLISTEN_PARAMETERINFO  0
#define ASYS_AUDIOUNIT_PROPLISTEN_PARAMETERLIST  1
#define ASYS_AUDIOUNIT_PROPLISTEN_CPULOAD        2
#define ASYS_AUDIOUNIT_PROPLISTEN_LATENCY        3
#define ASYS_AUDIOUNIT_PROPLISTEN_TAILTIME       4
#define ASYS_AUDIOUNIT_PROPLISTEN_FACTORYPRESETS 5
#define ASYS_AUDIOUNIT_PROPLISTEN_RENDERQUALITY  6
#define ASYS_AUDIOUNIT_PROPLISTEN_CURRENTPRESET  7
#define ASYS_AUDIOUNIT_PROPLISTEN_PRESENTPRESET  8
#define ASYS_AUDIOUNIT_PROPLISTEN_STREAMFROMDISK 9
#define ASYS_AUDIOUNIT_PROPLISTEN_MAXIMUMFRAMESPERSLICE 10
#define ASYS_AUDIOUNIT_PROPLISTEN_ARRAYSIZE     11   /* ++ for each property added */

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*  reset/paging behaviors   */
/*___________________________*/

/* active reset does Uninitialize/Initialize for ResetSelect */

#define ASYS_AUDIOUNIT_PASSIVE_RESET 0
#define ASYS_AUDIOUNIT_ACTIVE_RESET  1
#define ASYS_AUDIOUNIT_RESET_TYPE ASYS_AUDIOUNIT_PASSIVE_RESET

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/* istate-machine constants  */
/*___________________________*/

#define ASYS_AUDIOUNIT_ISTATE_ABSENT           0 
#define ASYS_AUDIOUNIT_ISTATE_INSTANCE         1
#define ASYS_AUDIOUNIT_ISTATE_INSTANCE_PASSING 2
#define ASYS_AUDIOUNIT_ISTATE_FLUX             3
#define ASYS_AUDIOUNIT_ISTATE_FLUX_PASSING     4
#define ASYS_AUDIOUNIT_ISTATE_ENGINE           5
#define ASYS_AUDIOUNIT_ISTATE_ENGINE_PASSING   6
#define ASYS_AUDIOUNIT_ISTATE_ENGINE_RENDER    7

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/* istate-machine return values  */
/*_______________________________*/

#define ASYS_AUDIOUNIT_IRESPONSE_COMPLETE               0
#define ASYS_AUDIOUNIT_IRESPONSE_INITIALIZE             1
#define ASYS_AUDIOUNIT_IRESPONSE_UNINITIALIZE           2
#define ASYS_AUDIOUNIT_IRESPONSE_CLOSE                  3
#define ASYS_AUDIOUNIT_IRESPONSE_RENDER                 4
#define ASYS_AUDIOUNIT_IRESPONSE_WIRE                   5
#define ASYS_AUDIOUNIT_IRESPONSE_REINITIALIZE           6
#define ASYS_AUDIOUNIT_IRESPONSE_REUNINITIALIZE         7
#define ASYS_AUDIOUNIT_IRESPONSE_UNINITIALIZE_AND_CLOSE 8
#define ASYS_AUDIOUNIT_IRESPONSE_MY_IS_NULL             9
#define ASYS_AUDIOUNIT_IRESPONSE_ERROR                 10

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*   Control Driver Constants     */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/* retry limit for socket writing */

#define ASYS_AUDIOUNIT_RETRY_MAX  256

/* bitfield constants for MIDIevent flags variable */

#define ASYS_AUDIOUNIT_MIDIFLAGS_WAITING 0x01u  /* queuing flag bit */

/* bitfield constants for SASLevent flags variable */

#define ASYS_AUDIOUNIT_SASLFLAGS_WAITING 0x01u  /* queuing flag bit */

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*   SAOL Parameter Constants    */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#define ASYS_AUDIOUNIT_PARAMETER_DEFAULT_UNIT kAudioUnitParameterUnit_Generic
#define ASYS_AUDIOUNIT_PARAMETER_DEFAULT_MINIMUM 0.0F
#define ASYS_AUDIOUNIT_PARAMETER_DEFAULT_MAXIMUM 1.0F
#define ASYS_AUDIOUNIT_PARAMETER_DEFAULT_DEFAULT 0.5F

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*           TypeDefs             */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*  typedef for a MIDI event   */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

typedef struct asysn_audiounit_MIDIevent {
  unsigned char cmd;
  unsigned char d0;
  unsigned char d1;
  unsigned char flags;
  int kcycleidx;
} asysn_audiounit_MIDIevent;

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*  typedef for a SASL event   */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

typedef struct asysn_audiounit_SASLevent {
  int index;
  Float32 value;
  unsigned char flags;
  int kcycleidx;
} asysn_audiounit_SASLevent;

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*  typedef for SAOL parameter */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

typedef struct asysn_audiounit_saolparam {
  Float32 value;
  int index;
  int use;
} asysn_audiounit_saolparam;

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*  typedef for a listener proc  */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

typedef struct asysn_audiounit_proplisten {
  AudioUnitPropertyListenerProc lproc;
  void * lrefcon;
  struct asysn_audiounit_proplisten * next;
} asysn_audiounit_proplisten;

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*  typedef for a render notify proc  */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

typedef struct asysn_audiounit_rendernotify {
  AURenderCallback nproc;
  void * nrefcon;
  struct asysn_audiounit_rendernotify * next;
} asysn_audiounit_rendernotify;

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/* typedef for instance state */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

typedef struct asysn_audiounit_InstanceState {

  ComponentInstance component;  /* component instance ID */
  volatile int istate;          /* initialization state machine */
  ENGINE_PTR_DECLARE_SEMICOLON  /* engine pointer for the instance */

  /*~~~~~~~~~~~~~~~~~~~~~~~*/
  /* mirrored engine state */
  /*~~~~~~~~~~~~~~~~~~~~~~~*/

  UInt32 acycle;
  Float32 krate;
  volatile uint64_t acycleidx_kcycleidx;

  /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
  /* current value of properties */
  /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

  /* common to Effects and MusicDevices */

  AudioUnitConnection MakeConnection[ASYS_AUDIOUNIT_ELEMENT_ARRAYSIZE];
  Float32 CPULoad;
  OSSpinLock lock_CPULoad;
  AudioStreamBasicDescription OutputStreamFormat;
  OSType SRCAlgorithm;
  UInt32 InputElementCount;
  UInt32 OutputElementCount;
  Float64 Latency;
  AUChannelInfo SupportedNumChannels[ASYS_AUDIOUNIT_SUPPORTED_FORMATS];
  UInt32 MaximumFramesPerSlice;
  Float64 TailTime;
  UInt32 BypassEffect;
  UInt32 LastBypassEffect;
  OSStatus LastRenderError;
  AURenderCallbackStruct SetRenderCallback[ASYS_AUDIOUNIT_ELEMENT_ARRAYSIZE];
  asysn_audiounit_rendernotify * rendernotify;
  OSSpinLock lock_rendernotify;
  UInt32 RenderQuality;
  UInt32 InPlaceProcessing;
  AUPreset PresentPreset;
  OSSpinLock lock_PresentPreset;
  UInt32 OfflineRender;
  Float64 PresentationLatency;
  UInt32 StreamFromDisk;
  asysn_audiounit_proplisten * proplisteners[ASYS_AUDIOUNIT_PROPLISTEN_ARRAYSIZE];
  OSSpinLock lock_proplisteners;
  OSSpinLock lock_sampledelivery;  /* for MakeConnection and SetRenderCallback */
  
  /* exclusive to MusicDevices and MusicEffects (?) */

  UInt32 InstrumentCount;

  /* properties exclusive to Effects */

  AudioStreamBasicDescription InputStreamFormat[ASYS_AUDIOUNIT_ELEMENT_ARRAYSIZE];

  /*~~~~~~~~~~~~~~~~~~~~*/
  /* per-instance state */
  /*~~~~~~~~~~~~~~~~~~~~*/

  /* data output array: common to Effects and Music Devices */

  Float32 * mData_Output[ASYS_AUDIOUNIT_OUTPUT_MAXCHANNELS];

  /* rendering templates and data carriers: exclusive to Effects */

  AudioBufferList * AudioBufferTemplate;
  AudioBufferList * AudioBufferCarrier;

  /*~~~~~~~~~~~~~~~~~~~~~*/
  /* aup_ property state */
  /*~~~~~~~~~~~~~~~~~~~~~*/

  int num_saolparams;
  AudioUnitParameterID * parameterlist;
  AudioUnitParameterInfo * parameterinfo;
  int * pvs_size;
  char *** pvs_cstr;
  asysn_audiounit_saolparam * saolparam;

  int num_factory;
  Float32 ** factorypreset_values;
  AUPreset * factorypreset_info;
  CFMutableArrayRef factorypreset_array;

  /*~~~~~~~~~~~~~*/
  /* ksync state */
  /*~~~~~~~~~~~~~*/

  UInt64 ksync_timespent;  /* total time spent in one kpass */
  Float32 ksync_normalize;    /* unit translations: 1/ticks to Hz */
  Float32 ksync_last_cpuload;   /* used for missing data heuristic  */

  /*~~~~~~~~~~~~~~~~~~~~~~~~*/
  /* state passed to engine */
  /*~~~~~~~~~~~~~~~~~~~~~~~~*/

  char componentname[32];

  char mpipeflag[32];
  char mpipevalue[32];

  char spipeflag[32];
  char spipevalue[32];

  char * argv[5];
  int argc;

  /*~~~~~~~~~~~~~~~~~*/
  /* aucontrol state */
  /*~~~~~~~~~~~~~~~~~*/

  int mpipepair[2];
  int spipepair[2];

} asysn_audiounit_InstanceState;


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/* The page-management system  */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

int asysn_audiounit_opencount = 0;   /* number of audiounit instances */
OSSpinLock asysn_audiounit_lock_opencount;      /* protects opencount */

/********************************************/
/* set memory status of a region of memory  */
/********************************************/

void asysn_audiounit_memstatus(void * addr, size_t len, int advice)

{
  madvise((caddr_t) addr, len, advice);
  return;
}


/*~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~*/
/* The ksync system  */
/*~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~*/

/*********************************************/
/* startup initialization for ksync system   */
/*********************************************/

void asysn_audiounit_ksyncinit(asysn_audiounit_InstanceState * My)

{
  My->ksync_timespent = 0;

  OSSpinLockLock(&(My->lock_CPULoad));

  My->ksync_normalize = (Float32) (My->krate/AudioGetHostClockFrequency());
  if (My->CPULoad)
    My->ksync_normalize = My->ksync_normalize/My->CPULoad;

  OSSpinLockUnlock(&(My->lock_CPULoad));

  My->ksync_last_cpuload = 0.0F;   
}

/********************************************/
/* computes cpuload value at end of a kpass */
/********************************************/

Float32 asysn_audiounit_ksync(asysn_audiounit_InstanceState * My)

{
  UInt64 now = AudioGetCurrentHostTime();
  Float32 ret;

  ret = My->ksync_normalize*((Float32)(now - My->ksync_timespent));

  if (ret > 1.0F)
    ret = 1.0F;

  if (ret < 1.0F)
    My->ksync_last_cpuload = ret;
  else
    {
      ret = My->ksync_last_cpuload;
      My->ksync_last_cpuload = 1.0F;
    }

  My->ksync_timespent = now;

  return ret;
}


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*    Fast Dispatch Functions     */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/***********************************************/
/* Helper Routine for MIDIEvent Fast Dispatch  */
/***********************************************/

ComponentResult asysn_audiounit_sendMIDIevent(asysn_audiounit_MIDIevent * MIDIevent,
					      asysn_audiounit_InstanceState * My)

{
  uint64_t acycleidx_kcycleidx;
  int32_t acycleidx, kcycleidx;
  int write_failed = 0;
  int retry = 0;
  int acount;

  if (!ASYS_AUDIOUNIT_HAS_AUCONTROL)
    return noErr;

  /* on entry, MIDIevent->kcycle holds offset of event (in samples) from the */
  /* start of the next buffer. This start time in the sfront engine is coded */
  /* by the tuple (kcycleidx, acycleidx).                                    */

  acycleidx_kcycleidx = My->acycleidx_kcycleidx;
  kcycleidx = (int32_t)(acycleidx_kcycleidx & 0x00000000FFFFFFFF);
  acycleidx = (int32_t)(acycleidx_kcycleidx >> 32);

  acount = acycleidx + MIDIevent->kcycleidx;
  MIDIevent->kcycleidx = kcycleidx;

  /* initialization of acycleidx_kcycleidx skirts zero-time condition issue */

  while (acount >= My->acycle)
    {
      acount -= My->acycle;
      MIDIevent->kcycleidx++;
    }

  while (write(My->mpipepair[1], MIDIevent, sizeof(asysn_audiounit_MIDIevent)) < 0)
    if ((errno != EINTR) || (++retry >= ASYS_AUDIOUNIT_RETRY_MAX))
      {
	ASYS_AUDIOUNIT_WIRETAP_PUTSTRING("\tSocket write error in sendMIDIevent\n\n");
	write_failed = 1;
	break;
      }
 
  return noErr;
}

/***********************************/
/* MIDIEvent Fast Dispatch Handler */
/***********************************/

ComponentResult asysn_audiounit_MyMIDIEventProc(void * inComponentStorage,
						UInt32 inStatus,
						UInt32 inData1,
						UInt32 inData2,
						UInt32 inOffsetSampleFrame)
{
  asysn_audiounit_MIDIevent MIDIevent;
  int result;

  if (!inComponentStorage)           /* avoid race condition */
    return noErr;

  MIDIevent.cmd = (unsigned char) inStatus;
  MIDIevent.d0 = (unsigned char)inData1;
  MIDIevent.d1 = (unsigned char)inData2;
  MIDIevent.flags = ASYS_AUDIOUNIT_MIDIFLAGS_WAITING;
  MIDIevent.kcycleidx = (int) inOffsetSampleFrame;

  result = asysn_audiounit_sendMIDIevent
    (&MIDIevent, (asysn_audiounit_InstanceState *) inComponentStorage);

  ASYS_AUDIOUNIT_WIRETAP_MIDIEVENT(inComponentStorage, inStatus, inData1, 
				   inData2, inOffsetSampleFrame, "Fast Dispatch");

  return result;
}

/**************************************************/
/* Helper Routine for GetParameter Fast Dispatch  */
/**************************************************/

ComponentResult asysn_audiounit_getSASLevent(AudioUnitParameterID inID, 
					     Float32 * outValue,
					     asysn_audiounit_InstanceState * My)

{
  if (inID >= My->num_saolparams)
    return kAudioUnitErr_InvalidParameter;

  *outValue = My->saolparam[inID].value;
  return noErr;
}

/**************************************/
/* GetParameter Fast Dispatch Handler */
/**************************************/

ComponentResult asysn_audiounit_MyGetParameterProc(void * inComponentStorage,
						   AudioUnitParameterID inID,
						   AudioUnitScope inScope,
						   AudioUnitElement inElement,
						   Float32 * outValue)
{
  int result;

  if (!inComponentStorage)           /* avoid race condition */
    return noErr;

  if (inScope != kAudioUnitScope_Global)  /* inScope */
    return kAudioUnitErr_InvalidScope;

  result = asysn_audiounit_getSASLevent
    (inID, outValue, (asysn_audiounit_InstanceState *) inComponentStorage);

  ASYS_AUDIOUNIT_WIRETAP_GETPARAMETER(inComponentStorage, inID, inScope, 
				      inElement, outValue, "Fast Dispatch");

  return result;
}

/**************************************************/
/* Helper Routine for SetParameter Fast Dispatch  */
/**************************************************/

ComponentResult asysn_audiounit_sendSASLevent(asysn_audiounit_SASLevent * SASLevent,
					      asysn_audiounit_InstanceState * My)

{
  uint64_t acycleidx_kcycleidx;
  int32_t acycleidx, kcycleidx;
  int write_failed = 0;
  int retry = 0;
  int acount;

  if (!ASYS_AUDIOUNIT_HAS_AUCONTROL)
    return noErr;

  /* on entry:                                                               */
  /*            SASLevent->index holds AudioUnit index (not SAOL index) and  */
  /*            has been range-checked.                                      */
  /*                                                                         */
  /*            SASLevent->value is range-checked.                           */
  /*                                                                         */
  /*            SASLevent->flags is not set.                                 */
  /*                                                                         */
  /*            SASLevent->kcycle holds offset of event (in samples) from    */
  /*            the start of the next buffer. This start time in the sfront  */
  /*            engine is coded by the tuple (kcycleidx, acycleidx).         */

  SASLevent->index = My->saolparam[SASLevent->index].index;
  SASLevent->flags = ASYS_AUDIOUNIT_SASLFLAGS_WAITING;

  acycleidx_kcycleidx = My->acycleidx_kcycleidx;
  kcycleidx = (int32_t)(acycleidx_kcycleidx & 0x00000000FFFFFFFF);
  acycleidx = (int32_t)(acycleidx_kcycleidx >> 32);

  acount = acycleidx + SASLevent->kcycleidx;
  SASLevent->kcycleidx = kcycleidx;

  /* initialization of acycleidx_kcycleidx skirts zero-time condition issue */

  while (acount >= My->acycle)
    {
      acount -= My->acycle;
      SASLevent->kcycleidx++;
    }

  while (write(My->spipepair[1], SASLevent, sizeof(asysn_audiounit_SASLevent)) < 0)
    if ((errno != EINTR) || (++retry >= ASYS_AUDIOUNIT_RETRY_MAX))
      {
	ASYS_AUDIOUNIT_WIRETAP_PUTSTRING("\tSocket write error in sendSASLevent\n\n");
	write_failed = 1;
	break;
      }
 
  return noErr;
}

/**************************************/
/* SetParameter Fast Dispatch Handler */
/**************************************/

ComponentResult asysn_audiounit_MySetParameterProc(void * inComponentStorage,
						   AudioUnitParameterID inID,
						   AudioUnitScope inScope,
						   AudioUnitElement inElement,
						   Float32 inValue,
						   UInt32 inBufferOffsetInFrames)
{
  asysn_audiounit_InstanceState * My;
  asysn_audiounit_SASLevent SASLevent;
  int result;

  My = ((asysn_audiounit_InstanceState *) inComponentStorage);

  if (!My)           /* avoid race condition */
    return noErr;

  if ((inID < 0) || (inID >= My->num_saolparams))  /* range-check ID */
    return kAudioUnitErr_InvalidParameter;

  if (inScope != kAudioUnitScope_Global)  /* inScope */
    return kAudioUnitErr_InvalidScope;

  if (inValue > My->parameterinfo[inID].maxValue)
    inValue = My->parameterinfo[inID].maxValue;

  if (inValue < My->parameterinfo[inID].minValue)
    inValue = My->parameterinfo[inID].minValue;

  SASLevent.index = inID;
  SASLevent.value = My->saolparam[inID].value = inValue;
  SASLevent.kcycleidx = (int)(inBufferOffsetInFrames);

  result = asysn_audiounit_sendSASLevent(&SASLevent, My);

  ASYS_AUDIOUNIT_WIRETAP_SETPARAMETER(My, inID, inScope, 
				      inElement, inValue, inBufferOffsetInFrames, 
				      "Fast Dispatch");

  return result;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/* AUEventListenerNotify support  */
/*                                */
/* To let global variable writes  */
/* by SAOL programs be seen in UI */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/************************************************/
/* checks all variables, does update if needed  */
/************************************************/

void asysn_audiounit_eventlistenernotify(asysn_audiounit_InstanceState * My)

{
  ENGINE_PTR_DECLARE_SEMICOLON
  AudioUnitEvent event;
  Float32 value;
  int i;

  ENGINE_PTR_ASSIGNED_TO  MY_ENGINE_PTR;  /* for NG() reference */

  for (i = 0; i < My->num_saolparams; i++)
    if (My->saolparam[i].use & CSYS_WRITTEN)
      if ((value = NG(My->saolparam[i].index)) != My->saolparam[i].value)
	{
	  if (value > My->parameterinfo[i].maxValue)
	    NG(My->saolparam[i].index) = value = My->parameterinfo[i].maxValue;
	  if (value < My->parameterinfo[i].minValue)
	    NG(My->saolparam[i].index) = value = My->parameterinfo[i].minValue;
	  
	  if (value != My->saolparam[i].value)
	    {
	      My->saolparam[i].value = value;
	      event.mEventType = kAudioUnitEvent_ParameterValueChange; 
	      event.mArgument.mParameter.mAudioUnit = My->component; 
	      event.mArgument.mParameter.mParameterID = i;
	      event.mArgument.mParameter.mScope = kAudioUnitScope_Global; 
	      event.mArgument.mParameter.mElement = 0; 
	      OSMemoryBarrier();
	      AUEventListenerNotify(NULL, NULL, &event);
	    }
	}
}

/*------------------------------------------*/
/*  Specialized Renderers for Each AU Type  */
/*------------------------------------------*/

#if ASYS_AUDIOUNIT_MUSICDEVICE 

/************************************************************/
/* Helper Routine For Fast-Dispatch Renderer -- MusicDevice */
/************************************************************/

void asysn_audiounit_activeo_renderer(asysn_audiounit_InstanceState * My,
				      AudioBufferList * ioData)

{
  ENGINE_PTR_DECLARE_SEMICOLON
  UInt32 frames = ioData->mBuffers[0].mDataByteSize/sizeof(Float32);
  Float32 eincr, escale;
  UInt32 do_effect, do_bypass;
  Float32 * out_left, * out_right;
  Float32 left;
#if (ASYS_AUDIOUNIT_OUTPUT_CHANNELS > ASYS_AUDIOUNIT_MONO)
  Float32 right;
#endif

  ENGINE_PTR_ASSIGNED_TO  MY_ENGINE_PTR;


  if (My->BypassEffect == 0)
    {
      if (My->LastBypassEffect == 0)
	{
	  do_effect = 1;
	  do_bypass = 0;
	  eincr = escale = 0.0F;
	}
      else
	{
	  do_effect = 1;
	  do_bypass = 1;
	  eincr = escale = 1.0F/(frames + 1);
	  My->LastBypassEffect = 0;
	}
    }
  else
    {
      if (My->LastBypassEffect == 1)
	{
	  do_effect = 0;
	  do_bypass = 1;
	  eincr = escale = 0.0F;
	}
      else
	{
	  do_effect = 1;
	  do_bypass = 1;
	  eincr = - 1.0F/(frames + 1);
	  escale = 1.0F + eincr;
	  My->LastBypassEffect = 1;
	}
    }

  out_left = (Float32 *) (ioData->mBuffers[0].mData);

  if (ioData->mNumberBuffers > ASYS_AUDIOUNIT_MONO)
    out_right = (Float32 *) (ioData->mBuffers[1].mData);
  else
    out_right = NULL;

  while (frames)
    if (EV(acycleidx) < EV(ACYCLE))
      {
	memset(&(TB(0)), 0, ENDBUS*sizeof(ASYS_OTYPE));  /* set bus to silence */
	main_apass(ENGINE_PTR);                         /* compute samples    */

	if (do_effect)
	  {
	    left = TB(BUS_output_bus);

#if (ASYS_AUDIOUNIT_OUTPUT_CLIPPING)
	    if ((left > 1.0F) || (left < -1.0F))
	      left = (left > 0) ? 1.0F : -1.0F;
#endif	    
	    *out_left = left;
	    
	    if (out_right)
	      {
#if (ASYS_AUDIOUNIT_OUTPUT_CHANNELS == ASYS_AUDIOUNIT_MONO)
		*out_right = left;
#else
		right = TB(BUS_output_bus + 1);

#if (ASYS_AUDIOUNIT_OUTPUT_CLIPPING)
		if ((right > 1.0F) || (right < -1.0F))
		  right = (right > 0) ? 1.0F : -1.0F;
#endif
		*out_right = right;
#endif
	      }
	    else
	      {
#if (ASYS_AUDIOUNIT_OUTPUT_CHANNELS > ASYS_AUDIOUNIT_MONO)
		right = TB(BUS_output_bus + 1);

#if (ASYS_AUDIOUNIT_OUTPUT_CLIPPING)
		if ((right > 1.0F) || (right < -1.0F))
		  right = (right > 0) ? 1.0F : -1.0F;
#endif
		*out_left += right;
		*out_left *= 0.5F;
#endif
	      }

	    if (do_bypass)
	      {
		*out_left *= escale;
		if (out_right)
		  *out_right *= escale;
		escale += eincr;
	      }
	  }

	if (do_bypass && !do_effect)
	  {
	    *out_left = 0.0F;

	    if (out_right)
	      *out_right = 0.0F;
	  }

	out_left++;

	if (out_right)
	  out_right++;

	EV(acycleidx)++; frames--;     /* update all positions */
      }
    else
      {
	EV(acycleidx) = 0;
	if (EV(pass) == APASS)
	  {
	    EV(cpuload) = asysn_audiounit_ksync(My);
  	    EV(kcycleidx)++;         /* we run forever; don't test against endkcycle */
	  }
	EV(pass) = IPASS;
	EV(scorebeats) = EV(scoremult)*(EV(kcycleidx) - EV(kbase)) + EV(scorebase);
	EV(absolutetime) = (EV(kcycleidx) - 1)*EV(KTIME);
	main_ipass(ENGINE_PTR);
	EV(pass) = KPASS;
	main_control(ENGINE_PTR);
	main_kpass(ENGINE_PTR);      /* we run forever; don't check return value */
	if (ASYS_AUDIOUNIT_PARAMETERS_WRITTEN)
	  asysn_audiounit_eventlistenernotify(My);
	EV(pass) = APASS;
      }

  My->acycleidx_kcycleidx = (((uint64_t) (EV(kcycleidx))) & 0x00000000FFFFFFFF) | 
    (((uint64_t) (EV(acycleidx))) << 32);
}

/*****************************************/
/* Fast-Dispatch Renderer -- MusicDevice */
/*****************************************/

ComponentResult asysn_audiounit_MyRenderer(void * inComponentStorage,
					   AudioUnitRenderActionFlags * ioActionFlags,
					   const AudioTimeStamp * inTimeStamp,
					   UInt32 inOutputBusNumber,
					   UInt32 inNumberFrames,
					   AudioBufferList * ioData)
{
  UInt32 error_code;
  char message[128];
  asysn_audiounit_rendernotify * notify;
  AudioUnitRenderActionFlags action_flags;
  asysn_audiounit_InstanceState * My = ((asysn_audiounit_InstanceState *) 
					inComponentStorage);
  int i, iresponse;

  if (inNumberFrames > ASYS_AUDIOUNIT_FRAMES_PER_SLICE)
    return kAudioUnitErr_TooManyFramesToProcess;

  if (!My)
    return kAudioUnitErr_Uninitialized;   /* avoid race condition */

  switch(iresponse = asysn_audiounit_realtime_enterstate(My)) {
  case ASYS_AUDIOUNIT_IRESPONSE_WIRE:
  case ASYS_AUDIOUNIT_IRESPONSE_RENDER:
    break;
  case ASYS_AUDIOUNIT_IRESPONSE_MY_IS_NULL:
  case ASYS_AUDIOUNIT_IRESPONSE_ERROR:
  default:
    return kAudioUnitErr_Uninitialized;   /* avoid race condition, bugs */
  }

  /* supply return buffers (only!) if needed */

  for (i = 0; i < ioData->mNumberBuffers; i++)
    if (ioData->mBuffers[i].mData == NULL)
      ioData->mBuffers[i].mData = (void *) My->mData_Output[i];

  /* do PreRender notification */

  OSSpinLockLock(&(My->lock_rendernotify));

  notify = My->rendernotify;
  while (notify)
    {
      if (notify->nproc)
	{
	  action_flags = kAudioUnitRenderAction_PreRender;

	  if (error_code = ((* notify->nproc)
			    (notify->nrefcon, &action_flags, 
			     inTimeStamp, (AudioUnitElement) 0, inNumberFrames,
			     ioData)))
	    {
	      sprintf(message, "\nasysn_audiounit_MyRenderer"
		      "\n\tError: PreRender Rendernotify callback" 
		      " returned OSType %u\n", (unsigned int) error_code);
	      ASYS_AUDIOUNIT_WIRETAP_PUTSTRING(message);
	    }
	}
      notify = notify->next;
    }

  OSSpinLockUnlock(&(My->lock_rendernotify));

  /* the actual rendering done by the audio unit */

  if (iresponse == ASYS_AUDIOUNIT_IRESPONSE_RENDER)
    {
      My->ksync_timespent = AudioGetCurrentHostTime() - My->ksync_timespent;
      asysn_audiounit_activeo_renderer(My, ioData);
      My->ksync_timespent = AudioGetCurrentHostTime() - My->ksync_timespent;
    }
  else   /* for _WIRE, zero all buffers */
    for (i = 0; i < ioData->mNumberBuffers; i++)  
      memset(ioData->mBuffers[i].mData, 0, sizeof(Float32)*inNumberFrames);

  /* do PostRender notification */

  OSSpinLockLock(&(My->lock_rendernotify));

  notify = My->rendernotify;
  while (notify)
    {
      if (notify->nproc)
	{
	  action_flags = kAudioUnitRenderAction_PostRender;

	  if (error_code = ((* notify->nproc)
			    (notify->nrefcon, &action_flags, 
			     inTimeStamp, (AudioUnitElement) 0, inNumberFrames,
			     ioData)))
	    {
	      sprintf(message, "\nasysn_audiounit_MyRenderer"
		      "\n\tError: PostRender Rendernotify callback" 
		      " returned OSType %u\n", (unsigned int) error_code);
	      ASYS_AUDIOUNIT_WIRETAP_PUTSTRING(message);
	    }
	}
      notify = notify->next;
    }

  OSSpinLockUnlock(&(My->lock_rendernotify));

  asysn_audiounit_realtime_exitstate(My);

  return noErr;
}

#endif /* ASYS_AUDIOUNIT_MUSICDEVICE */


#if ASYS_AUDIOUNIT_EFFECT

/********************************************************/
/* Helper Routine For Fast-Dispatch Renderer -- Effects */
/********************************************************/

void asysn_audiounit_activeio_renderer(asysn_audiounit_InstanceState * My,
				       AudioBufferList * ioData)

{
  ENGINE_PTR_DECLARE_SEMICOLON
  UInt32 frames = ioData->mBuffers[0].mDataByteSize/sizeof(Float32);
  Float32 eincr, bincr, escale, bscale;
  UInt32 do_effect, do_bypass;
  Float32 * out_left, * out_right, * in_left, * in_right;
  Float32 left;
#if (ASYS_AUDIOUNIT_OUTPUT_CHANNELS > ASYS_AUDIOUNIT_MONO)
  Float32 right;
#endif

  ENGINE_PTR_ASSIGNED_TO  MY_ENGINE_PTR;

  if (My->BypassEffect == 0)
    {
      if (My->LastBypassEffect == 0)
	{
	  do_effect = 1;
	  do_bypass = 0;
	  eincr = bincr = bscale = escale = 0.0F;
	}
      else
	{
	  do_effect = 1;
	  do_bypass = 1;
	  eincr = escale = 1.0F/(frames + 1);
	  bincr = - eincr;
	  bscale = 1.0F - eincr;
	  My->LastBypassEffect = 0;
	}
    }
  else
    {
      if (My->LastBypassEffect == 1)
	{
	  do_effect = 0;
	  do_bypass = 1;
	  eincr = bincr = bscale = escale = 0.0F;
	}
      else
	{
	  do_effect = 1;
	  do_bypass = 1;
	  bincr = bscale = 1.0F/(frames + 1);
	  eincr = - bincr;
	  escale = 1.0F - bincr;
	  My->LastBypassEffect = 1;
	}
    }

  out_left = (Float32 *) (ioData->mBuffers[0].mData);

  if (ioData->mNumberBuffers > ASYS_AUDIOUNIT_MONO)
    out_right = (Float32 *) (ioData->mBuffers[1].mData);
  else
    out_right = NULL;

  in_left = (Float32 *) (My->AudioBufferCarrier->mBuffers[0].mData);

  if (My->AudioBufferCarrier->mNumberBuffers > ASYS_AUDIOUNIT_MONO)
    in_right = (Float32 *) (My->AudioBufferCarrier->mBuffers[1].mData);
  else
    in_right = NULL;

  while (frames)
    if (EV(acycleidx) < EV(ACYCLE))
      {
	memset(&(TB(0)), 0, ENDBUS*sizeof(ASYS_OTYPE));  /* set bus to silence */

	TB(BUS_input_bus) = *in_left;
 
	if (in_right)
	  {
#if (ASYS_AUDIOUNIT_INPUT_CHANNELS > ASYS_AUDIOUNIT_MONO)
	    TB(BUS_input_bus + 1) = *in_right;
#else
	    TB(BUS_input_bus) += *in_right;
	    TB(BUS_input_bus) *= 0.5F;
#endif
	  }
	else
	  {
#if (ASYS_AUDIOUNIT_INPUT_CHANNELS > ASYS_AUDIOUNIT_MONO)
	    TB(BUS_input_bus + 1) = *in_left;
#endif
	  }

	main_apass(ENGINE_PTR);                   /* compute samples    */

	if (do_effect)
	  {
	    left = TB(BUS_output_bus);

#if (ASYS_AUDIOUNIT_OUTPUT_CLIPPING)
	    if ((left > 1.0F) || (left < -1.0F))
	      left = (left > 0) ? 1.0F : -1.0F;
#endif
	    *out_left = left;

	    if (out_right)
	      {
#if (ASYS_AUDIOUNIT_OUTPUT_CHANNELS == ASYS_AUDIOUNIT_MONO)
		*out_right = left;
#else
		right = TB(BUS_output_bus + 1);

#if (ASYS_AUDIOUNIT_OUTPUT_CLIPPING)
		if ((right > 1.0F) || (right < -1.0F))
		  right = (right > 0) ? 1.0F : -1.0F;
#endif
		*out_right = right;
#endif
	      }
	    else
	      {
#if (ASYS_AUDIOUNIT_OUTPUT_CHANNELS > ASYS_AUDIOUNIT_MONO)
		right = TB(BUS_output_bus + 1);

#if (ASYS_AUDIOUNIT_OUTPUT_CLIPPING)
		if ((right > 1.0F) || (right < -1.0F))
		  right = (right > 0) ? 1.0F : -1.0F;
#endif
		*out_left += right;
		*out_left *= 0.5F;
#endif
	      }
	  }

	if (do_bypass)
	  {
	    if (!do_effect)
	      *out_left = *in_left;
	    else
	      {
		if (out_right || !in_right)
		  *out_left = escale*(*out_left) + bscale*(*in_left);
		else
		  *out_left = escale*(*out_left) + bscale*0.5F*(*in_left);
	      }

	    if (out_right)
	      {
		if (!do_effect)
		  {
		    if (in_right)
		      *out_right = *in_right;
		    else 
		      *out_right = *in_left;
		  }
		else
		  {
		    if (in_right)
		      *out_right = escale*(*out_right) + bscale*(*in_right);
		    else 
		      *out_right = escale*(*out_right) + bscale*(*in_left);
		  }
	      }
	    else
	      {
		if (in_right)
		  {
		    if (!do_effect)
		      *out_left = 0.5F*(*out_left + *in_right);
		    else
		      *out_left = *out_left + bscale*0.5F*(*in_right);
		  }
	      }

	    if (do_effect)
	      {
		bscale += bincr;
		escale += eincr;
	      }
	  }

	in_left++;
	out_left++;

	if (in_right)
	  in_right++;

	if (out_right)
	  out_right++;

	EV(acycleidx)++; frames--;  /* updates */
      }
    else
      {
	EV(acycleidx) = 0;
	if (EV(pass) == APASS)
	  {      
	    EV(cpuload) = asysn_audiounit_ksync(My);
	    EV(kcycleidx)++;         /* we run forever; don't test against endkcycle */
	  }
	EV(pass) = IPASS;
	EV(scorebeats) = EV(scoremult)*(EV(kcycleidx) - EV(kbase)) + EV(scorebase);
	EV(absolutetime) = (EV(kcycleidx) - 1)*EV(KTIME);
	main_ipass(ENGINE_PTR);
	EV(pass) = KPASS;
	main_control(ENGINE_PTR);
	main_kpass(ENGINE_PTR);      /* we run forever; don't check return value */
	if (ASYS_AUDIOUNIT_PARAMETERS_WRITTEN)
	  asysn_audiounit_eventlistenernotify(My);
	EV(pass) = APASS;
      }

  My->acycleidx_kcycleidx = (((uint64_t) (EV(kcycleidx))) & 0x00000000FFFFFFFF) | 
    (((uint64_t) (EV(acycleidx))) << 32);
}

/**********************************************************/
/* Helper Routine For Fast-Dispatch Renderer -- Pass-thru */
/**********************************************************/

void asysn_audiounit_activeio_passthru(asysn_audiounit_InstanceState * My,
				       AudioBufferList * ioData)

{
  UInt32 frames = ioData->mBuffers[0].mDataByteSize/sizeof(Float32);
  Float32 * out_left, * out_right, * in_left, * in_right;

  out_left = (Float32 *) (ioData->mBuffers[0].mData);

  if (ioData->mNumberBuffers > ASYS_AUDIOUNIT_MONO)
    out_right = (Float32 *) (ioData->mBuffers[1].mData);
  else
    out_right = NULL;

  in_left = (Float32 *) (My->AudioBufferCarrier->mBuffers[0].mData);

  if (My->AudioBufferCarrier->mNumberBuffers > ASYS_AUDIOUNIT_MONO)
    in_right = (Float32 *) (My->AudioBufferCarrier->mBuffers[1].mData);
  else
    in_right = NULL;

  while (frames)
    {
      *out_left = *in_left;

      if (out_right)
	{
	  if (in_right)
	    *out_right = *in_right;
	  else 
	    *out_right = *in_left;
	}
      else
	if (in_right)
	  *out_left = 0.5F*(*out_left + *in_right);
      
      in_left++;
      out_left++;
      
      if (in_right)
	in_right++;
      
      if (out_right)
	out_right++;
      
      frames--;
    }
}


/*************************************/
/* Fast-Dispatch Renderer -- Effects */
/*************************************/

ComponentResult asysn_audiounit_MyRenderer(void * inComponentStorage,
					   AudioUnitRenderActionFlags * ioActionFlags,
					   const AudioTimeStamp * inTimeStamp,
					   UInt32 inOutputBusNumber,
					   UInt32 inNumberFrames,
					   AudioBufferList * ioData)
{
  UInt32 error_code;
  char message[128];  
  asysn_audiounit_rendernotify * notify;
  AudioUnitRenderActionFlags action_flags;
  AudioTimeStamp AudioTimeStampCarrier;
  asysn_audiounit_InstanceState * My = ((asysn_audiounit_InstanceState *) 
					inComponentStorage);
  ComponentResult return_value;
  int i, iresponse;

  if (inNumberFrames > ASYS_AUDIOUNIT_FRAMES_PER_SLICE)
    return kAudioUnitErr_TooManyFramesToProcess;

  if (!My)
    return kAudioUnitErr_Uninitialized;    /* avoid race condition */

  switch(iresponse = asysn_audiounit_realtime_enterstate(My)) {
  case ASYS_AUDIOUNIT_IRESPONSE_WIRE:
  case ASYS_AUDIOUNIT_IRESPONSE_RENDER:
    break;
  case ASYS_AUDIOUNIT_IRESPONSE_MY_IS_NULL:
  case ASYS_AUDIOUNIT_IRESPONSE_ERROR:
  default:
    return kAudioUnitErr_Uninitialized;   /* avoid race condition, bugs */
  }

  return_value = noErr;
  action_flags = 0;

  /* prepare AudioTimeStampCarrier */

  memset(&AudioTimeStampCarrier, 0, sizeof(AudioTimeStamp));
  AudioTimeStampCarrier.mSampleTime = inTimeStamp->mSampleTime;
  AudioTimeStampCarrier.mFlags = kAudioTimeStampSampleTimeValid;

  /* prepare AudioBufferCarrier */

  memcpy(My->AudioBufferCarrier, My->AudioBufferTemplate, 
	 sizeof(UInt32) + My->AudioBufferTemplate->mNumberBuffers*sizeof(AudioBuffer));
 
  for (i = 0; i < My->AudioBufferCarrier->mNumberBuffers; i++)
    My->AudioBufferCarrier->mBuffers[i].mDataByteSize = 
      ioData->mBuffers[0].mDataByteSize;

  /* retrieve audio samples to process, log errors */ 
  /*   assumptions: 
     - a callback returning noErr returns the amount of data we asked for 
  */

  OSSpinLockLock(&(My->lock_sampledelivery));

  if (My->SetRenderCallback[0].inputProc)
    {
      if (error_code = ((* My->SetRenderCallback[0].inputProc)
			(My->SetRenderCallback[0].inputProcRefCon, &action_flags, 
			 &AudioTimeStampCarrier, (AudioUnitElement) 0, inNumberFrames,
			 My->AudioBufferCarrier)))
	{
	  sprintf(message, "\nMyRenderer"
		  "\n\tError: Render callback returned OSType %u\n", 
		  (unsigned int) error_code);
	  ASYS_AUDIOUNIT_WIRETAP_PUTSTRING(message);
	  return_value = kAudioUnitErr_NoConnection;
	}
    }
  else
    {
      if ((My->MakeConnection[0].sourceAudioUnit == 0) || 
	  (error_code = AudioUnitRender(My->MakeConnection[0].sourceAudioUnit, 
					&action_flags, &AudioTimeStampCarrier, 
					My->MakeConnection[0].sourceOutputNumber,
					inNumberFrames, My->AudioBufferCarrier)))
	{
	  if (My->MakeConnection[0].sourceAudioUnit == 0)
	    {
	      sprintf(message, "\nMyRenderer"
		      "\n\tError: No connection to use with AudioUnitRender\n");
	      ASYS_AUDIOUNIT_WIRETAP_PUTSTRING(message);
	      return_value = kAudioUnitErr_NoConnection;
	    }
	  else
	    {
	      sprintf(message, "\nMyRenderer\n\tError: AudioUnitRender returned:\n\t"); 
	      ASYS_AUDIOUNIT_WIRETAP_PUTSTRING(message);
	      ASYS_AUDIOUNIT_WIRETAP_PRINT_COMPONENT_RESULT(error_code);
	      sprintf(message, "\n"); 
	      ASYS_AUDIOUNIT_WIRETAP_PUTSTRING(message);
	      return_value = error_code;
	    }
	}
    }

  OSSpinLockUnlock(&(My->lock_sampledelivery));

  /* supply return buffers (only!) if needed */

  for (i = 0; i < ioData->mNumberBuffers; i++)  /* straight-wire, no gain */
    if (ioData->mBuffers[i].mData == NULL)
      ioData->mBuffers[i].mData = (void *) My->mData_Output[i];

  /* do PreRender notification */

  OSSpinLockLock(&(My->lock_rendernotify));

  notify = My->rendernotify;
  while (notify)
    {
      if (notify->nproc)
	{
	  action_flags = kAudioUnitRenderAction_PreRender;

	  if (error_code = ((* notify->nproc)
			    (notify->nrefcon, &action_flags, 
			     inTimeStamp, (AudioUnitElement) 0, inNumberFrames,
			     ioData)))
	    {
	      sprintf(message, "\nasysn_audiounit_MyRenderer"
		      "\n\tError: PreRender Rendernotify callback" 
		      " returned OSType %u\n", (unsigned int) error_code);
	      ASYS_AUDIOUNIT_WIRETAP_PUTSTRING(message);
	    }
	}
      notify = notify->next;
    }

  OSSpinLockUnlock(&(My->lock_rendernotify));

  /* the actual rendering done by the audio unit */

  if (return_value == noErr) 
    {
      if (iresponse == ASYS_AUDIOUNIT_IRESPONSE_RENDER)
	{
	  My->ksync_timespent = AudioGetCurrentHostTime() - My->ksync_timespent;
	  asysn_audiounit_activeio_renderer(My, ioData);
	  My->ksync_timespent = AudioGetCurrentHostTime() - My->ksync_timespent;
	}
      else
	asysn_audiounit_activeio_passthru(My, ioData);
    }
  else
    for (i = 0; i < ioData->mNumberBuffers; i++)  /* in case of error, silence */
      memset(ioData->mBuffers[i].mData, 0, ioData->mBuffers[i].mDataByteSize);

  /* do PostRender notification */

  OSSpinLockLock(&(My->lock_rendernotify));

  notify = My->rendernotify;
  while (notify)
    {
      if (notify->nproc)
	{
	  action_flags = kAudioUnitRenderAction_PostRender;

	  if (error_code = ((* notify->nproc)
			    (notify->nrefcon, &action_flags, 
			     inTimeStamp, (AudioUnitElement) 0, inNumberFrames,
			     ioData)))
	    {
	      sprintf(message, "\nasysn_audiounit_MyRenderer"
		      "\n\tError: PostRender Rendernotify callback" 
		      " returned OSType %u\n", (unsigned int) error_code);
	      ASYS_AUDIOUNIT_WIRETAP_PUTSTRING(message);
	    }
	}
      notify = notify->next;
    }

  OSSpinLockUnlock(&(My->lock_rendernotify));

  asysn_audiounit_realtime_exitstate(My);

  return return_value;
}

#endif /* ASYS_AUDIOUNIT_EFFECT */

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/* Helper Functions for all Selector Calls    */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/****************************************************************/
/* Helper Function for Factory Presets: returns a preset name   */
/****************************************************************/

CFStringRef asysn_audiounit_factorypresetname(CFIndex idx) 

{
  CFStringRef ret = NULL;
  char * nptr, * val_name;
  int i, j;

  j = 0;

  for (i = 0; i < CSYS_GLOBALNUM; i++)
    if ((csys_global[i].type == CSYS_TABLE) && csys_global[i].name &&
	!strncmp("aup_factory_", csys_global[i].name, strlen("aup_factory_")) &&
	strlen(csys_global[i].name) > strlen("aup_factory_") && ((j++) == idx))
      {
	nptr = val_name = malloc
	  (strlen(&(csys_global[i].name[strlen("aup_factory_")])) + 1);

	strcpy(val_name, &(csys_global[i].name[strlen("aup_factory_")]));

	do {
	  if (*nptr == '_')
	    *nptr = ' ';
	} while (*(++nptr));

	ret = CFStringCreateWithCString(NULL, val_name, kCFStringEncodingASCII);
	break;
      }
      
  return ret;
}

/****************************************************************/
/* Helper Function for Factory Presets: returns a preset number */
/****************************************************************/

int asysn_audiounit_factorypresetnumber(CFStringRef cfstr) 

{
  char * nptr, * val_name, * match;
  int i, j;

  if (!cfstr)
    return -1;

  match = calloc(CFStringGetLength(cfstr) + 1, 1);
  CFStringGetCString(cfstr, match, CFStringGetLength(cfstr) + 1, 
		     kCFStringEncodingASCII);

  j = 0;

  for (i = 0; i < CSYS_GLOBALNUM; i++)
    if ((csys_global[i].type == CSYS_TABLE) && csys_global[i].name &&
	!strncmp("aup_factory_", csys_global[i].name, strlen("aup_factory_")) &&
	strlen(csys_global[i].name) > strlen("aup_factory_"))
      {
	nptr = val_name = malloc
	  (strlen(&(csys_global[i].name[strlen("aup_factory_")])) + 1);

	strcpy(val_name, &(csys_global[i].name[strlen("aup_factory_")]));

	do {
	  if (*nptr == '_')
	    *nptr = ' ';
	} while (*(++nptr));

	if (!strcmp(match, val_name))
	  return j;

	j++;
      }
      
  return -1;
}


/***********************************************************************/
/* Helper Function for GetProperty: Create ParameterValueStrings array */
/***********************************************************************/

CFMutableArrayRef asysn_audiounit_parametervaluestrings_create
(asysn_audiounit_InstanceState * My, AudioUnitParameterID i) 

{
  CFMutableArrayRef array;
  int j;

  if ((i < 0) || (i >= My->num_saolparams) || (My->pvs_size[i] == 0))
    return NULL;

  array = CFArrayCreateMutable(NULL, My->pvs_size[i], &kCFTypeArrayCallBacks);

  for (j = 0; j < My->pvs_size[i]; j++)
    CFArrayInsertValueAtIndex(array, j, CFStringCreateWithCString
			      (NULL, My->pvs_cstr[i][j], kCFStringEncodingASCII));

  return array;
}

/*****************************************************************/
/* Pre-engine-execution passes for initialize_parametersystem()  */
/*****************************************************************/

ComponentResult asysn_audiounit_earlypass_parametersystem
                        (asysn_audiounit_InstanceState * My) 

{
  int i, j, msize;
  char * nptr;

  /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
  /* Pass 1 on csys_global[]: count parameters and factory presets  */
  /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

  for (i = 0; i < CSYS_GLOBALNUM; i++)
    if (csys_global[i].name &&
	(!strncmp(csys_global[i].name, "aup_", 4)) &&
	(strlen(csys_global[i].name) > 4))
      switch (csys_global[i].type) {
      case CSYS_IRATE:
      case CSYS_KRATE:
	if (csys_global[i].width != 1)
	  break;

	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	/* change sfront/src/writepre.c line 3206 */
	/* when the code below changes            */
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

	/* skip over reserved names */

 	if ((nptr = strstr(&(csys_global[i].name[4]), "_idx")) && 
	    (strlen(nptr) > 6) && (nptr[4] >= '0') && (nptr[4] <= '9'))
	  break;

	if (strstr(&(csys_global[i].name[4]), "_unit_"))
	  break;

	if ((strstr(&(csys_global[i].name[4]), "_slider_squarelaw")) ||
	    (strstr(&(csys_global[i].name[4]), "_slider_cubic")) ||
	    (strstr(&(csys_global[i].name[4]), "_slider_squareroot")) ||
	    (strstr(&(csys_global[i].name[4]), "_slider_cuberoot")) ||
	    (strstr(&(csys_global[i].name[4]), "_slider_log")) ||
	    (strstr(&(csys_global[i].name[4]), "_slider_exp")) ||
	    (strstr(&(csys_global[i].name[4]), "_slider_linear")) ||
	    (strstr(&(csys_global[i].name[4]), "_slider")) ||
	    (strstr(&(csys_global[i].name[4]), "_checkbox")) ||
	    (strstr(&(csys_global[i].name[4]), "_menu")) ||
	    (strstr(&(csys_global[i].name[4]), "_display_number")) ||
	    (strstr(&(csys_global[i].name[4]), "_display_checkbox")) ||
	    (strstr(&(csys_global[i].name[4]), "_display_menu")))
	  break;

	My->num_saolparams++;
	break;
      case CSYS_TABLE:
	if (!strncmp("aup_factory_", csys_global[i].name, strlen("aup_factory_"))
	    && (strlen(csys_global[i].name) > strlen("aup_factory_")))
	  My->num_factory++;
	break;
      default:
	break;
      }

  /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
  /* Create data structures for parameters, factory presets, etc  */
  /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

  if (My->num_saolparams == 0)
    return noErr;

  if (!(My->parameterlist = (AudioUnitParameterID *) 
	malloc(My->num_saolparams*sizeof(AudioUnitParameterID))))
    return kAudioUnitErr_FailedInitialization;

  for (i = 0; i < My->num_saolparams; i++)
    My->parameterlist[i] = i;

  if (!(My->parameterinfo = (AudioUnitParameterInfo *)
	calloc(My->num_saolparams, sizeof(AudioUnitParameterInfo))))
    return kAudioUnitErr_FailedInitialization;

  if (!(My->pvs_size = (int *) calloc(My->num_saolparams, sizeof(int))))
    return kAudioUnitErr_FailedInitialization;

  if (!(My->pvs_cstr = (char ***) calloc(My->num_saolparams, sizeof(char **))))
    return kAudioUnitErr_FailedInitialization;

  msize = My->num_saolparams*sizeof(asysn_audiounit_saolparam);
  if (!(My->saolparam = (asysn_audiounit_saolparam *) malloc(msize)))
    return kAudioUnitErr_FailedInitialization;
  asysn_audiounit_memstatus(My->saolparam, msize, MADV_WILLNEED);

  if (My->num_factory)
    {
      if (!(My->factorypreset_info = (AUPreset *) 
	    calloc(My->num_factory, sizeof(AUPreset))))
	return kAudioUnitErr_FailedInitialization;

      msize = My->num_factory*sizeof(Float32 *);
      if (!(My->factorypreset_values = (Float32 **) calloc(1, msize)))
	return kAudioUnitErr_FailedInitialization;
      asysn_audiounit_memstatus(My->factorypreset_values, msize, MADV_WILLNEED);

      for (i = 0; i < My->num_factory; i++)
	{
	  msize = My->num_saolparams*sizeof(Float32);
	  if (!(My->factorypreset_values[i] = (Float32 *) malloc(msize)))
	    return kAudioUnitErr_FailedInitialization;
	  asysn_audiounit_memstatus(My->factorypreset_values[i], msize, MADV_WILLNEED);
	}
    }

  /* todo: add parameter value strings data structures */

  /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
  /* Pass 2 on csys_global[]: Initialize parameter names  */
  /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

  j = 0;

  for (i = 0; i < CSYS_GLOBALNUM; i++)
    if (csys_global[i].name &&
	(!strncmp(csys_global[i].name, "aup_", 4)) &&
	(strlen(csys_global[i].name) > 4))
      switch (csys_global[i].type) {
      case CSYS_IRATE:
      case CSYS_KRATE:
	if (csys_global[i].width != 1)
	  break;

	/* skip over reserved names */

	if ((nptr = strstr(&(csys_global[i].name[4]), "_idx")) && 
	    (strlen(nptr) > 6) && (nptr[4] >= '0') && (nptr[4] <= '9'))
	  break;

	if (strstr(&(csys_global[i].name[4]), "_unit_"))
	  break;

	if ((strstr(&(csys_global[i].name[4]), "_slider_squarelaw")) ||
	    (strstr(&(csys_global[i].name[4]), "_slider_cubic")) ||
	    (strstr(&(csys_global[i].name[4]), "_slider_squareroot")) ||
	    (strstr(&(csys_global[i].name[4]), "_slider_cuberoot")) ||
	    (strstr(&(csys_global[i].name[4]), "_slider_log")) ||
	    (strstr(&(csys_global[i].name[4]), "_slider_exp")) ||
	    (strstr(&(csys_global[i].name[4]), "_slider_linear")) ||
	    (strstr(&(csys_global[i].name[4]), "_slider")) ||
	    (strstr(&(csys_global[i].name[4]), "_checkbox")) ||
	    (strstr(&(csys_global[i].name[4]), "_menu")) ||
	    (strstr(&(csys_global[i].name[4]), "_display_number")) ||
	    (strstr(&(csys_global[i].name[4]), "_display_checkbox")) ||
	    (strstr(&(csys_global[i].name[4]), "_display_menu")))
	  break;

	/* Do all initialization that may be done early */

	strncpy(My->parameterinfo[j].name, &(csys_global[i].name[4]), 60);
	My->parameterinfo[j].name[59] = '\0';

	My->parameterinfo[j].unit = ASYS_AUDIOUNIT_PARAMETER_DEFAULT_UNIT;
	My->parameterinfo[j].minValue = ASYS_AUDIOUNIT_PARAMETER_DEFAULT_MINIMUM;
	My->parameterinfo[j].maxValue = ASYS_AUDIOUNIT_PARAMETER_DEFAULT_MAXIMUM;
	My->parameterinfo[j].defaultValue = ASYS_AUDIOUNIT_PARAMETER_DEFAULT_DEFAULT;

	My->parameterinfo[j].flags = kAudioUnitParameterFlag_HasCFNameString |
	  kAudioUnitParameterFlag_IsReadable | kAudioUnitParameterFlag_IsWritable |
	  kAudioUnitParameterFlag_CFNameRelease;

	My->saolparam[j].index = csys_global[i].index;
	My->saolparam[j].use = csys_global[i].use;

	j++;
	break;
      default:
	break;
      }

  return noErr;
}


/******************************************************************/
/* Parameter-value strings pass for initialize_parametersystem()  */
/******************************************************************/

ComponentResult asysn_audiounit_parametervaluestrings_parametersystem
                (asysn_audiounit_InstanceState * My) 

{
  int i, j, k, str_idx, len;
  char * nptr, * match;

  /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
  /* Create parameter-value strings for Indexed parameters */
  /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

  for (j = 0; j < My->num_saolparams; j++)
    if (My->parameterinfo[j].unit == kAudioUnitParameterUnit_Indexed)
      {
	My->pvs_size[j] = (My->parameterinfo[j].maxValue 
			   - My->parameterinfo[j].minValue + 1);
	
	My->pvs_cstr[j] = (char **) calloc(My->pvs_size[j], sizeof(char *));

	if (!(My->pvs_cstr[j]))
	  return kAudioUnitErr_FailedInitialization;

	/* create array and default values */

	for (k = 0; k < My->pvs_size[j]; k++)
	  {
	    My->pvs_cstr[j][k] = (char *)calloc(strlen("4294967295") + 1, sizeof(char));

	    if (!(My->pvs_cstr[j][k]))
	      return kAudioUnitErr_FailedInitialization;

	    sprintf(My->pvs_cstr[j][k], "%u", k);
	  }

	/* overwrite default values with custom strings */

	len = strlen("aup_") + strlen(My->parameterinfo[j].name) + strlen("_idx");

	if (!(match = malloc(len + 1)))
	  return kAudioUnitErr_FailedInitialization;

	sprintf((nptr = match), "aup_%s_idx", My->parameterinfo[j].name); 

	do {
	  if (*nptr == ' ')
	    *nptr = '_';
	} while (*(++nptr));

	for (i = 0; i < CSYS_GLOBALNUM; i++)
	  if (csys_global[i].name && (csys_global[j].width == 1) &&
	      ((csys_global[i].type == CSYS_IRATE) || 
	       (csys_global[i].type == CSYS_KRATE)) &&
	      (!strncmp(csys_global[i].name, match, len)) && 
	      (strlen(csys_global[i].name) > len) &&
	      (sscanf(&(csys_global[i].name[len]), "%u_", &k) == 1) && 
	      (k >= 0) && (k <= My->pvs_size[j]))
	    {
	      str_idx = len;
	      while (csys_global[i].name[str_idx++] != '_');

	      if (csys_global[i].name[str_idx] != '\0')
		{
		  free(My->pvs_cstr[j][k]);

		  nptr = My->pvs_cstr[j][k] 
		    = malloc(strlen(&(csys_global[i].name[str_idx])) + 1);

		  if (!nptr)
		    return kAudioUnitErr_FailedInitialization;

		  strcpy(My->pvs_cstr[j][k], &(csys_global[i].name[str_idx]));

		  do {
		    if (*nptr == '_')
		      *nptr = ' ';
		  } while (*(++nptr));
		}
	    }

	free(match);
      }

  return noErr;
}

/*********************************************************/
/* Factory preset pass for initialize_parametersystem()  */
/*********************************************************/

ComponentResult asysn_audiounit_factorypreset_parametersystem
                (ENGINE_PTR_DECLARE_COMMA 
                 asysn_audiounit_InstanceState * My) 

{
  int i, j, k, idx;
  char * nptr, * match;

  /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
  /* initialize factory preset data structures */
  /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

  if (My->num_factory)
    {
      j = 0;
      My->factorypreset_array = CFArrayCreateMutable(NULL, My->num_factory, NULL);

      if (!(My->factorypreset_array))
	return kAudioUnitErr_FailedInitialization;

      for (i = 0; i < CSYS_GLOBALNUM; i++)
	if ((csys_global[i].type == CSYS_TABLE) && csys_global[i].name &&
	    !strncmp("aup_factory_", csys_global[i].name, strlen("aup_factory_")) &&
	    strlen(csys_global[i].name) > strlen("aup_factory_"))
	  {
	    nptr = match = malloc
	      (strlen(&(csys_global[i].name[strlen("aup_factory_")])) + 1);

	    if (!nptr)
	      return kAudioUnitErr_FailedInitialization;

	    strcpy(match, &(csys_global[i].name[strlen("aup_factory_")]));
	    do {
	      if (*nptr == '_')
		*nptr = ' ';
	    } while (*(++nptr));

	    My->factorypreset_info[j].presetNumber = j;
	    My->factorypreset_info[j].presetName = CFStringCreateWithCString
	      (NULL, match, kCFStringEncodingASCII);

	    if (!(My->factorypreset_info[j].presetName))
	      return kAudioUnitErr_FailedInitialization;

	    CFArrayAppendValue(My->factorypreset_array, &(My->factorypreset_info[j]));

	    free(match);

	    for (k = 0; k < My->num_saolparams; k++)
	      My->factorypreset_values[j][k] = My->parameterinfo[k].defaultValue;

	    if (EV(gtables)[idx = csys_global[i].index].t)
	      for (k = 0; (k < EV(gtables)[idx].len) && (k < My->num_saolparams); k++)
		{
		  My->factorypreset_values[j][k] = EV(gtables)[idx].t[k];
		  My->factorypreset_values[j][k]  = 
		    (My->factorypreset_values[j][k] < My->parameterinfo[k].minValue)
		    ? My->parameterinfo[k].minValue : My->factorypreset_values[j][k];
		  My->factorypreset_values[j][k]  = 
		    (My->factorypreset_values[j][k] > My->parameterinfo[k].maxValue)
		    ? My->parameterinfo[k].maxValue : My->factorypreset_values[j][k];
		}

	    j++;
	  }
    }

  return noErr;
}

/************************************************************/
/* Helper for Open Selector: Initializes parameter system.  */
/************************************************************/

ComponentResult asysn_audiounit_initialize_parametersystem
                   (asysn_audiounit_InstanceState * My) 

{
  int i, j, k, ret, match_len, unit_num;
  char * nptr, * kptr, * match;
  ENGINE_PTR_DECLARE_SEMICOLON

  if ((ASYS_AUDIOUNIT_HAS_AUCONTROL == 0) || (CSYS_GLOBALNUM == 0))
    return noErr;

  /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
  /* Parameter initialization passes that precede engine start */
  /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

  if ((ret = asysn_audiounit_earlypass_parametersystem(My)) != noErr)
    return ret;

  if (My->num_saolparams == 0)
    return noErr;

  /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
  /* Execute engine to define parameters and factory presets  */
  /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

  ENGINE_PTR_ASSIGNED_TO  system_init(My->argc, My->argv, 
				      My->OutputStreamFormat.mSampleRate);

  if (ENGINE_PTR_IS_NULL)
    return kAudioUnitErr_FailedInitialization;

  effects_init(ENGINE_PTR);
  main_initpass(ENGINE_PTR);
  asysn_audiounit_ksyncinit(My);

  /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
  /* Use engine results to complete parameter data structures */
  /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

  for (j = 0; j < My->num_saolparams; j++)
    {
      match = malloc(strlen("aup_") + strlen(My->parameterinfo[j].name) + 
		     strlen("_pinfo") + 1);

      if (!match)
	{
	  shut_down(ENGINE_PTR);
	  return kAudioUnitErr_FailedInitialization;
	}

      sprintf(match, "aup_%s_pinfo", My->parameterinfo[j].name); 

      for (i = 0; i < CSYS_GLOBALNUM; i++)
	if ((csys_global[i].type == CSYS_TABLE) && csys_global[i].name &&
	    !strcmp(match, csys_global[i].name))
	  {
	    k = csys_global[i].index;

	    if (EV(gtables)[k].t && (EV(gtables)[k].len >= 3) && 
		(EV(gtables)[k].t[0] <= EV(gtables)[k].t[1]) && 
		(EV(gtables)[k].t[1] <= EV(gtables)[k].t[2]))
	      {
		My->parameterinfo[j].minValue = EV(gtables)[k].t[0];
		My->parameterinfo[j].defaultValue = EV(gtables)[k].t[1];
		My->parameterinfo[j].maxValue = EV(gtables)[k].t[2];

		if (My->parameterinfo[j].minValue > My->parameterinfo[j].maxValue)
		  {
		    My->parameterinfo[j].minValue = EV(gtables)[k].t[2];
		    My->parameterinfo[j].maxValue = EV(gtables)[k].t[0];
		  }

		if (My->parameterinfo[j].defaultValue > My->parameterinfo[j].maxValue)
		  My->parameterinfo[j].defaultValue = EV(gtables)[k].t[2];

		if (My->parameterinfo[j].defaultValue < My->parameterinfo[j].minValue)
		  My->parameterinfo[j].defaultValue = EV(gtables)[k].t[0];
	      }
	    break;
	  }

      free(match);

      match = malloc((match_len = strlen("aup_") + strlen(My->parameterinfo[j].name) + 
		      strlen("_unit_")) + 1);

      if (!match)
	{
	  shut_down(ENGINE_PTR);
	  return kAudioUnitErr_FailedInitialization;
	}

      sprintf(match, "aup_%s_unit_", My->parameterinfo[j].name); 

      for (i = 0; i < CSYS_GLOBALNUM; i++)
	if (((csys_global[i].type == CSYS_IRATE) || 
	     (csys_global[i].type == CSYS_KRATE)) && 
	    (csys_global[i].width == 1) &&
	    csys_global[i].name &&
	    !strncmp(match, csys_global[i].name, match_len))
	  {
	    if (strlen(csys_global[i].name) <= match_len)
	      continue;

	    nptr = &(csys_global[i].name[match_len]);

	    /* for now, only support custom units */

	    kptr = malloc(strlen(nptr) + 1);

	    if (!kptr)
	      {
		shut_down(ENGINE_PTR);
		return kAudioUnitErr_FailedInitialization;
	      }

	    strcpy(kptr, &(csys_global[i].name[match_len]));

	    nptr = kptr;
	    do {
	      if (*nptr == '_')
		*nptr = ' ';
	    } while (*(++nptr));

	    My->parameterinfo[j].unitName = CFStringCreateWithCString
	      (NULL, kptr, kCFStringEncodingASCII);

	    if (!(My->parameterinfo[j].unitName))
	      {
		shut_down(ENGINE_PTR);
		return kAudioUnitErr_FailedInitialization;
	      }

	    My->parameterinfo[j].unit = kAudioUnitParameterUnit_CustomUnit;

	    free(kptr);
	    break;
	  }

      free(match);

      match = malloc((match_len = strlen("aup_") + strlen(My->parameterinfo[j].name) + 
		      strlen("_")) + 1);

      if (!match)
	{
	  shut_down(ENGINE_PTR);
	  return kAudioUnitErr_FailedInitialization;
	}

      sprintf(match, "aup_%s_", My->parameterinfo[j].name); 

      for (i = 0; i < CSYS_GLOBALNUM; i++)

	if (((csys_global[i].type == CSYS_IRATE) || 
	     (csys_global[i].type == CSYS_KRATE)) && 
	    (csys_global[i].width == 1) &&
	    csys_global[i].name &&
	    !strncmp(match, csys_global[i].name, match_len))
	  {
	    if (strlen(csys_global[i].name) <= match_len)
	      continue;

	    nptr = &(csys_global[i].name[match_len]);

	    if (!strcmp(nptr, "slider_squarelaw"))
	      My->parameterinfo[j].flags |= 
		kAudioUnitParameterFlag_DisplaySquared;
	    else
	      if (!strcmp(nptr, "slider_cubic"))
		My->parameterinfo[j].flags |= 
		  kAudioUnitParameterFlag_DisplayCubed;
	      else
		if (!strcmp(nptr, "slider_squareroot"))
		  My->parameterinfo[j].flags |= 
		    kAudioUnitParameterFlag_DisplaySquareRoot;
		else
		  if (!strcmp(nptr, "slider_cuberoot"))
		    My->parameterinfo[j].flags |= 
		      kAudioUnitParameterFlag_DisplayCubeRoot;
		  else
		    if (!strcmp(nptr, "slider_log"))
		      My->parameterinfo[j].flags |= 
			kAudioUnitParameterFlag_DisplayLogarithmic;
		    else
		      if (!strcmp(nptr, "slider_exp"))
			My->parameterinfo[j].flags |= 
			  kAudioUnitParameterFlag_DisplayExponential;
		      else
			if (!strcmp(nptr, "menu") || !strcmp(nptr, "display_menu"))
			  {
			    if (!strcmp(nptr, "display_menu"))
			      My->parameterinfo[j].flags &= 
				~ kAudioUnitParameterFlag_IsWritable;

			    My->parameterinfo[j].unit = 
			      kAudioUnitParameterUnit_Indexed;
			    if (My->parameterinfo[j].unitName)
			      {
				CFRelease(My->parameterinfo[j].unitName);
				My->parameterinfo[j].unitName = NULL;
			      }
			  }
			else
			  if (!strcmp(nptr, "checkbox") || 
			      !strcmp(nptr, "display_checkbox"))
			    {
			      if (!strcmp(nptr, "display_checkbox"))
				My->parameterinfo[j].flags &= 
				  ~ kAudioUnitParameterFlag_IsWritable;

			      My->parameterinfo[j].unit = 
				kAudioUnitParameterUnit_Boolean;
			      if (My->parameterinfo[j].unitName)
				{
				  CFRelease(My->parameterinfo[j].unitName);
				  My->parameterinfo[j].unitName = NULL;
				}
			    }
			  else
			    if (!strcmp(nptr, "display_number"))
			      {
				My->parameterinfo[j].flags &= 
				  ~ kAudioUnitParameterFlag_IsWritable;
			      }
			    else
			      if ((strcmp(nptr, "slider_linear") != 0) &&
				  (strcmp(nptr, "slider") != 0))
				continue;
	    break;
	  }

      free(match);
      
      My->saolparam[j].value = My->parameterinfo[j].defaultValue;

      nptr = &(My->parameterinfo[j].name[0]);
      do {
	if (*nptr == '_')
	  *nptr = ' ';
      } while (*(++nptr));
      
      My->parameterinfo[j].cfNameString = CFStringCreateWithCString
	(NULL, My->parameterinfo[j].name, kCFStringEncodingASCII);

      if (!(My->parameterinfo[j].cfNameString))
	{
	  shut_down(ENGINE_PTR);
	  return kAudioUnitErr_FailedInitialization;
	}
    }

  /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
  /* Create parameter-value strings for Indexed parameters */
  /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

  if ((ret = asysn_audiounit_parametervaluestrings_parametersystem(My)) != noErr)
    {
      shut_down(ENGINE_PTR);
      return ret;
    }

  /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
  /* initialize factory preset data structures */
  /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

  if ((ret = asysn_audiounit_factorypreset_parametersystem
       (ENGINE_PTR_COMMA My)) != noErr)
    {
      shut_down(ENGINE_PTR);
      return ret;
    }

  /*~~~~~~~~~~~~~~~~~~~*/
  /* Shut down engine  */
  /*~~~~~~~~~~~~~~~~~~~*/

  shut_down(ENGINE_PTR);
  return noErr;
}


/************************************************************/
/* Helper for Open Selector: Initializes property variables */
/************************************************************/

ComponentResult asysn_audiounit_initialize_properties
(asysn_audiounit_InstanceState * My) 

{
  int i;

  /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
  /* the first set of parameters define the signal flow  */
  /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

  /* the preferred audiounit bus widths */

  My->InputElementCount = ASYS_AUDIOUNIT_ELEMENT_INPUTPREF;
  My->OutputElementCount = ASYS_AUDIOUNIT_ELEMENT_OUTPUTPREF;

  /* defines fixed channel widths. todo: turn into an array of formats */

  if (ASYS_AUDIOUNIT_EFFECT)
    {
      My->SupportedNumChannels[0].inChannels = ASYS_AUDIOUNIT_MONO;
      My->SupportedNumChannels[0].outChannels = ASYS_AUDIOUNIT_MONO;
      My->SupportedNumChannels[1].inChannels = ASYS_AUDIOUNIT_MONO;
      My->SupportedNumChannels[1].outChannels = ASYS_AUDIOUNIT_STEREO;
      My->SupportedNumChannels[2].inChannels = ASYS_AUDIOUNIT_STEREO;
      My->SupportedNumChannels[2].outChannels = ASYS_AUDIOUNIT_MONO;
      My->SupportedNumChannels[3].inChannels = ASYS_AUDIOUNIT_STEREO;
      My->SupportedNumChannels[3].outChannels = ASYS_AUDIOUNIT_STEREO;
    }
  else
    {
      My->SupportedNumChannels[0].inChannels = ASYS_AUDIOUNIT_NULL;
      My->SupportedNumChannels[0].outChannels = ASYS_AUDIOUNIT_MONO;
      My->SupportedNumChannels[1].inChannels = ASYS_AUDIOUNIT_NULL;
      My->SupportedNumChannels[1].outChannels = ASYS_AUDIOUNIT_STEREO;
    }


  /* maximum audio sample chunk -- augment with other variables to resize buffers */
  
  My->MaximumFramesPerSlice = ASYS_AUDIOUNIT_FRAMES_PER_SLICE;

  /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
  /* our desired input and output stream format */
  /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

  for (i=0; i < ASYS_AUDIOUNIT_ELEMENT_ARRAYSIZE; i++)
    {
      /* mChannelsPerFrame code mono, stereo, etc */

      My->InputStreamFormat[i].mChannelsPerFrame = ASYS_AUDIOUNIT_INPUT_CHANNELS;

      /* could also be kAudioStreamAnyRate, once we support setproperty over-ride */

      My->InputStreamFormat[i].mSampleRate = SAOL_SRATE;
  
      /* a set of non-interleaved mono channels -- do not touch these */

      My->InputStreamFormat[i].mFormatID = kAudioFormatLinearPCM;
      
      My->InputStreamFormat[i].mFormatFlags = (kAudioFormatFlagIsFloat | 
					    kAudioFormatFlagIsPacked | 
					    kAudioFormatFlagIsNonInterleaved);
      
      if (ASYS_AUDIOUNIT_FLOAT32_BIGENDIAN)
	My->InputStreamFormat[i].mFormatFlags |= kAudioFormatFlagIsBigEndian;

      My->InputStreamFormat[i].mBytesPerPacket = 4;
      My->InputStreamFormat[i].mBytesPerFrame = 4;
      My->InputStreamFormat[i].mFramesPerPacket = 1;
      My->InputStreamFormat[i].mBitsPerChannel = 32;
      
      My->InputStreamFormat[i].mReserved = 0;
    }

  /* mChannelsPerFrame code mono, stereo, etc */

  My->OutputStreamFormat.mChannelsPerFrame = ASYS_AUDIOUNIT_OUTPUT_CHANNELS;

  /* could also be kAudioStreamAnyRate, once we support setproperty over-ride */

  My->OutputStreamFormat.mSampleRate = SAOL_SRATE;  

  My->acycle = ((int)(SAOL_SRATE))/((int)(SAOL_KRATE));
  My->krate = SAOL_KRATE;
  My->acycleidx_kcycleidx = 1;     /* acycleidx = 0, kcycleidx = 1 */

  /* a set of non-interleaved mono channels -- do not touch these */

  My->OutputStreamFormat.mFormatID = kAudioFormatLinearPCM;
  
  My->OutputStreamFormat.mFormatFlags = (kAudioFormatFlagIsFloat | 
					 kAudioFormatFlagIsPacked | 
					 kAudioFormatFlagIsNonInterleaved);

  if (ASYS_AUDIOUNIT_FLOAT32_BIGENDIAN)
    My->OutputStreamFormat.mFormatFlags |= kAudioFormatFlagIsBigEndian;

  My->OutputStreamFormat.mBytesPerPacket = 4;
  My->OutputStreamFormat.mBytesPerFrame = 4;
  My->OutputStreamFormat.mFramesPerPacket = 1;
  My->OutputStreamFormat.mBitsPerChannel = 32;

  My->OutputStreamFormat.mReserved = 0;

  /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
  /* end of streamformat property */
  /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

  /* tracks if the audio unit is currently bypassed */

  My->BypassEffect = 0;
  My->LastBypassEffect = 0;

  /* set to 1 if we renderer returns the buffers use for SetRenderCallback */

  My->InPlaceProcessing = ASYS_AUDIOUNIT_INPLACE_PROCESSING;

  /* define the null preset */

  My->PresentPreset.presetName = CFStringCreateWithCString(NULL, "Untitled", 
							   kCFStringEncodingASCII);
  if (!(My->PresentPreset.presetName))
    return kAudioUnitErr_FailedInitialization;

  My->PresentPreset.presetNumber = -1;

  My->lock_PresentPreset = 0;

  /* number of instruments offered by the music device */

  if (ASYS_AUDIOUNIT_MUSICDEVICE)
    My->InstrumentCount = ASYS_AUDIOUNIT_INSTRUMENT_COUNT;
  else
    My->InstrumentCount = 0;

  /* 1 if the audio unit intends to stream samples off of the disk */

  My->StreamFromDisk = 0;

  /* the function pointer and parameters SetRenderCallback provides */

  for (i=0; i < ASYS_AUDIOUNIT_ELEMENT_ARRAYSIZE; i++)
    {
      My->SetRenderCallback[i].inputProc = NULL;
      My->SetRenderCallback[i].inputProcRefCon = NULL;
    }

  /* how the host tells us who is driving us */

  for (i=0; i < ASYS_AUDIOUNIT_ELEMENT_ARRAYSIZE; i++)
    {
      My->MakeConnection[i].sourceAudioUnit = 0;
      My->MakeConnection[i].sourceOutputNumber = 0;
      My->MakeConnection[i].destInputNumber = 0;
    }

  /* lock for both SetRenderCallback and MakeConnection */

  My->lock_sampledelivery = 0;

 /* the function pointer and parameters RenderNotify provides, and its lock */
 
  My->rendernotify = NULL;
  My->lock_rendernotify = 0;

  /* tracks last rendering error for the rendering proc - clear after returning it */

  My->LastRenderError = noErr;

  /* for now, assume AU algorithm has no latency */

  My->Latency = 0.0;

  /* tail time (in a reverb sense) for the audio unit */

  My->TailTime = 0.0;

  /* to be used for selecting sfront's sample-rate conversion algorithm */

  My->SRCAlgorithm = kAudioUnitSRCAlgorithm_Polyphase;

  /* request for render quality, using 0-127 scaled enum in AudioUnitProperties.h */

  My->RenderQuality = kRenderQuality_Max;

  /* the % of total CPU load the host allots to us, 0 codes no limit */
  
  My->CPULoad = 0.0;
  My->lock_CPULoad = 0;
  My->ksync_normalize = (Float32) (My->krate/AudioGetHostClockFrequency());

  /* a heads-up for off-line operation, sent by host */

  My->OfflineRender = 0;

  /* a heads-up for the presentation latency, sent by host */

  My->PresentationLatency = 0.0;

  /* listener callback lists for each supported property */

  for (i = 0; i < ASYS_AUDIOUNIT_PROPLISTEN_ARRAYSIZE; i++)
    My->proplisteners[i] = NULL;

  My->lock_proplisteners = 0;

  /*~~~~~~~~~~~~~~~~~~~~~*/
  /* aup_ property state */
  /*~~~~~~~~~~~~~~~~~~~~~*/

  My->num_saolparams = 0;
  My->parameterlist = NULL;
  My->parameterinfo = NULL;
  My->pvs_size = NULL;
  My->pvs_cstr = NULL;
  My->saolparam = NULL;
  My->num_factory = 0;
  My->factorypreset_values = NULL;
  My->factorypreset_info = NULL;
  My->factorypreset_array = NULL;

  return noErr;
}

/************************************************************/
/* Helper for Open Selector: Memstatus for global variables */
/************************************************************/

void asysn_audiounit_globalvars_memstatus(int advice)

{
  int msize, i, j;

  /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
  /* global arrays used by main_ipass */
  /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#if (defined(CSYS_MAXEXTCHAN) && (CSYS_MAXEXTCHAN > 0))
  msize = CSYS_MAXEXTCHAN*sizeof(int);
  asysn_audiounit_memstatus(cme_preset, msize, advice);
#endif

#if (defined(CSYS_MAXSASLINSTR) && (CSYS_MAXSASLINSTR > 0))
  msize = CSYS_MAXSASLINSTR*sizeof(int);
  asysn_audiounit_memstatus(csys_instrtablesize, msize, advice);
  asysn_audiounit_memstatus(csys_instrvarsize, msize, advice);
#endif

  /*~~~~~~~~~~~~~~~~~~~~~*/
  /* global table arrays */
  /*~~~~~~~~~~~~~~~~~~~~~*/

#if (defined(CSYS_TABLE_CATALOG_SIZE) && (CSYS_TABLE_CATALOG_SIZE > 0))
  for (i = 0; i < CSYS_TABLE_CATALOG_SIZE; i++)
    {
      msize = csys_table_catalog[i].num*sizeof(float);
      asysn_audiounit_memstatus(csys_table_catalog[i].t, msize, advice);
    }
#endif 

  /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
  /* reflection interface variables */
  /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#if (defined(CSYS_GLOBALNUM) && (CSYS_GLOBALNUM > 0))
  msize = CSYS_GLOBALNUM*sizeof(csys_varstruct);
  asysn_audiounit_memstatus(csys_global, msize, advice);
#endif

#if (defined(CSYS_TARGETNUM) && (CSYS_TARGETNUM > 0))
  msize = CSYS_TARGETNUM*sizeof(csys_targetstruct);
  asysn_audiounit_memstatus(csys_target, msize, advice);
#endif

#if (defined(CSYS_LABELNUM) && (CSYS_LABELNUM > 0))
  msize = CSYS_LABELNUM*sizeof(csys_labelstruct);
  asysn_audiounit_memstatus(csys_labels, msize, advice);
#endif

#if (defined(CSYS_PRESETNUM) && (CSYS_PRESETNUM > 0))
  msize = CSYS_PRESETNUM*sizeof(csys_presetstruct);
  asysn_audiounit_memstatus(csys_presets, msize, advice);
#endif

#if (defined(CSYS_SAMPLENUM) && (CSYS_SAMPLENUM > 0))
  msize = CSYS_SAMPLENUM*sizeof(csys_samplestruct);
  asysn_audiounit_memstatus(csys_samples, msize, advice);
#endif

#if (defined(CSYS_BUSNUM) && (CSYS_BUSNUM > 0))
  msize = CSYS_BUSNUM*sizeof(csys_busstruct);
  asysn_audiounit_memstatus(csys_bus, msize, advice);
#endif

#if (defined(CSYS_ROUTENUM) && (CSYS_ROUTENUM > 0))
  msize = CSYS_ROUTENUM*sizeof(csys_routestruct);
  asysn_audiounit_memstatus(csys_route, msize, advice);
#endif

#if (defined(CSYS_SENDNUM) && (CSYS_SENDNUM > 0))
  msize = CSYS_SENDNUM*sizeof(csys_sendstruct);
  asysn_audiounit_memstatus(csys_send, msize, advice);
  for (i = 0; i < CSYS_SENDNUM; i++)
    if (csys_send[i].bus)
      {
	msize = csys_send[i].nbus*sizeof(int);
	asysn_audiounit_memstatus(csys_send[i].bus, msize, advice);
      }
#endif

#if (defined(CSYS_INSTRNUM) && (CSYS_INSTRNUM > 0))
  msize = CSYS_INSTRNUM*sizeof(csys_instrstruct);
  asysn_audiounit_memstatus(csys_instr, msize, advice);
  for (i = 0; i < CSYS_INSTRNUM; i++)
    {
      if (csys_instr[i].name)
	{
	  msize = (strlen(csys_instr[i].name) + 1)*sizeof(char);
	  asysn_audiounit_memstatus(csys_instr[i].name, msize, advice);
	}
      if (csys_instr[i].vars)
	{
	  msize = csys_instr[i].numvars*sizeof(csys_varstruct);
	  asysn_audiounit_memstatus(csys_instr[i].vars, msize, advice);
	  for (j = 0; j < csys_instr[i].numvars; j++)
	    if (csys_instr[i].vars[j].name)
	      {
		msize = (strlen(csys_instr[i].vars[j].name) + 1)*sizeof(char);
		asysn_audiounit_memstatus(csys_instr[i].vars[j].name, msize, advice);
	      }
	}
    }
#endif

} 

/************************************************************/
/* Helper for Open Selector: Recover from a failed open.    */
/************************************************************/

ComponentResult asysn_audiounit_instance_cleanup
(ComponentParameters * p, asysn_audiounit_InstanceState * My)

{
  int i, k, msize;

  if (My)
    {
      if (My->PresentPreset.presetName)
	CFRelease(My->PresentPreset.presetName);

      if (My->AudioBufferCarrier)
	{
	  msize = sizeof(UInt32) + sizeof(AudioBuffer)*ASYS_AUDIOUNIT_INPUT_MAXCHANNELS;
	  asysn_audiounit_memstatus(My->AudioBufferCarrier, msize, MADV_FREE);
	  free(My->AudioBufferCarrier);
	}

      if (My->AudioBufferTemplate)
	{
	  msize = sizeof(Float32)*ASYS_AUDIOUNIT_FRAMES_PER_SLICE;
	  for(i = 0; i < ASYS_AUDIOUNIT_INPUT_MAXCHANNELS; i++)
	    if (My->AudioBufferTemplate->mBuffers[i].mData)
	      {
		asysn_audiounit_memstatus(My->AudioBufferTemplate->mBuffers[i].mData, 
					  msize, MADV_FREE);
		free(My->AudioBufferTemplate->mBuffers[i].mData);
	      }
	  msize = sizeof(UInt32) + sizeof(AudioBuffer)*ASYS_AUDIOUNIT_INPUT_MAXCHANNELS;
	  asysn_audiounit_memstatus(My->AudioBufferTemplate, msize, MADV_FREE);
	  free(My->AudioBufferTemplate);
	}

      for(i = 0; i < ASYS_AUDIOUNIT_OUTPUT_MAXCHANNELS; i++)
	if (My->mData_Output[i])
	  {
	    msize = sizeof(Float32)*ASYS_AUDIOUNIT_FRAMES_PER_SLICE;
	    asysn_audiounit_memstatus(My->mData_Output[i], msize, MADV_FREE);
	    free(My->mData_Output[i]);
	  }

      if (My->mpipepair[0])
	close(My->mpipepair[0]);
      if (My->mpipepair[1])
	close(My->mpipepair[1]);
      if (My->spipepair[0])
	close(My->spipepair[0]);
      if (My->spipepair[1])
	close(My->spipepair[1]);

      if (My->parameterlist)
	free(My->parameterlist);

      if (My->parameterinfo)
	{
	  for (i = 0; i < My->num_saolparams; i++)
	    {
	      if (My->parameterinfo[i].cfNameString)
		CFRelease(My->parameterinfo[i].cfNameString);
	      if (My->parameterinfo[i].unitName)
		CFRelease(My->parameterinfo[i].unitName);
	    }
	  free(My->parameterinfo);
	}

      if (My->pvs_size)
	{
	  if (My->pvs_cstr)
	    {
	      for (i = 0; i < My->num_saolparams; i++)
		if (My->pvs_cstr[i])
		  {
		    for (k = 0; k < My->pvs_size[i]; k++)
		      if (My->pvs_cstr[i][k])
			free(My->pvs_cstr[i][k]);
		    free(My->pvs_cstr[i]);
		  }
	      free(My->pvs_cstr);
	    }
	  free(My->pvs_size);
	}

      if (My->saolparam)
	{
	  msize = My->num_saolparams*sizeof(asysn_audiounit_saolparam);
	  asysn_audiounit_memstatus(My->saolparam, msize, MADV_FREE);
	  free(My->saolparam);
	}

      if (My->factorypreset_info)
	{
	  for (i = 0; i < My->num_factory; i++)
	    if (My->factorypreset_info[i].presetName)
	      CFRelease(My->factorypreset_info[i].presetName);
	  free(My->factorypreset_info);
	}

      if (My->factorypreset_values)
	{
	  msize = My->num_saolparams*sizeof(Float32);
	  for (i = 0; i < My->num_factory; i++)
	    if (My->factorypreset_values[i])
	      {
		asysn_audiounit_memstatus(My->factorypreset_values[i], msize, MADV_FREE);
		free(My->factorypreset_values[i]);
	      }

	  msize = My->num_factory*sizeof(Float32 *);
	  asysn_audiounit_memstatus(My->factorypreset_values, msize, MADV_FREE);
	  free(My->factorypreset_values);
	}

      if (My->factorypreset_array)
	CFRelease(My->factorypreset_array);

      msize = sizeof(asysn_audiounit_InstanceState);
      asysn_audiounit_memstatus(My, msize, MADV_FREE);
      free(My);
    }

  SetComponentInstanceStorage((ComponentInstance) p->params[0], (Handle) NULL); 
  return kAudioUnitErr_FailedInitialization;
}

/**********************************************************************/
/* Helper for Initialize Selector: Send all parameters to SAOL engine */
/**********************************************************************/

ComponentResult asysn_audiounit_engine_parameter_update
(asysn_audiounit_InstanceState * My) 

{
  asysn_audiounit_SASLevent SASLevent;
  int result = noErr;
  int i;
  
  for (i = 0; i < My->num_saolparams; i++)
    {
      SASLevent.index = i;
      SASLevent.value = My->saolparam[i].value;
      SASLevent.kcycleidx = 0;
      result |= asysn_audiounit_sendSASLevent(&SASLevent, My);    
    }

  return result;
}

/***************************************************************/
/* Helper for Uninitialize Selector: empty MIDI and SASL pipes */
/***************************************************************/

void asysn_audiounit_engine_emptypipes
(asysn_audiounit_InstanceState * My) 
{
  asysn_audiounit_MIDIevent mdummy;
  asysn_audiounit_SASLevent sdummy;
  int retry = 0;

  if (ASYS_AUDIOUNIT_MIDISUPPORT && My->mpipepair[0])
    do { 
      if (retry++ > ASYS_AUDIOUNIT_RETRY_MAX)
	break;
    } while ((read(My->mpipepair[0], &mdummy, 
		   sizeof(asysn_audiounit_MIDIevent)) > 0) || (errno == EINTR));

  retry == 0;

  if (My->spipepair[0])
    do {
      if (retry++ > ASYS_AUDIOUNIT_RETRY_MAX)
	break;
    } while ((read(My->spipepair[0], &sdummy, 
		   sizeof(asysn_audiounit_SASLevent)) > 0) || (errno == EINTR));
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/* Helper Functions for the GetProperty Selector Helper for ClassInfo  */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/***********************************************/
/* Big-endian int append to a CFMutableDataRef */
/***********************************************/

void asysn_audiounit_classinfo_pdata_intwrite(CFMutableDataRef pdata, SInt32 value)

{
  unsigned char p[4];
  
  p[0] = 0x000000FF & (value >> 24);
  p[1] = 0x000000FF & (value >> 16);
  p[2] = 0x000000FF & (value >> 8);
  p[3] = 0x000000FF & (value);

  CFDataAppendBytes(pdata, (UInt8 *) p, 4);
}

/***************************************************/
/*  Big-endian float append to a CFMutableDataRef  */
/*   Assumes int and float share an enddian-ness   */   
/***************************************************/

void asysn_audiounit_classinfo_pdata_floatwrite(CFMutableDataRef pdata, Float32 fval)

{
  union { int i; float f ; } u;
  unsigned char p[4];

  u.f = fval;
  
  p[0] = 0x000000FF & (u.i >> 24);
  p[1] = 0x000000FF & (u.i >> 16);
  p[2] = 0x000000FF & (u.i >> 8);
  p[3] = 0x000000FF & (u.i);

  CFDataAppendBytes(pdata, (UInt8 *) p, 4);
}

/************************************/
/* Adds integer item to a ClassInfo */
/************************************/

int asysn_audiounit_classinfo_addint(CFMutableDictionaryRef ClassInfo, 
				     char * ckey, SInt32 value)

{
  CFStringRef key;
  CFNumberRef num;
  int errcode = -1;

  key = CFStringCreateWithCString(NULL, ckey, kCFStringEncodingASCII);
  if (key)
    {
      num = CFNumberCreate(NULL, kCFNumberSInt32Type, &value);
      if (num)
	{
	  CFDictionarySetValue(ClassInfo, key, num);
	  CFRelease(num);
	  errcode = 0;
	}
      CFRelease(key);
    }

  return errcode;
}

/************************************/
/* Adds float item to a ClassInfo */
/************************************/

int asysn_audiounit_classinfo_addfloat(CFMutableDictionaryRef ClassInfo, 
				       char * ckey, Float32 fval)

{
  CFStringRef key;
  CFNumberRef num;
  int errcode = -1;

  key = CFStringCreateWithCString(NULL, ckey, kCFStringEncodingASCII);
  if (key)
    {
      num = CFNumberCreate(NULL, kCFNumberFloatType, &fval);
      if (num)
	{
	  CFDictionarySetValue(ClassInfo, key, num);
	  CFRelease(num);
	  errcode = 0;
	}
      CFRelease(key);
    }

  return errcode;
}

/*************************************/
/* Adds CFString item to a ClassInfo */
/*************************************/

int asysn_audiounit_classinfo_addcfstr(CFMutableDictionaryRef ClassInfo, 
				       char * ckey, CFStringRef cfstr)

{
  CFStringRef key;
  int errcode = -1;

  key = CFStringCreateWithCString(NULL, ckey, kCFStringEncodingASCII);
  if (key)
    {
      CFDictionarySetValue(ClassInfo, key, cfstr);
      CFRelease(key);
      errcode = 0;
    }

  return errcode;
}

/************************************************************************/
/* Helper for GetProperty Selector: Create ClassInfo for use by AU host */
/************************************************************************/

CFMutableDictionaryRef asysn_audiounit_classinfo_create
                                           (asysn_audiounit_InstanceState * My) 

{
  CFMutableDictionaryRef ClassInfo;
  CFMutableDataRef pdata;
  CFStringRef key;
  int i;

  ClassInfo = CFDictionaryCreateMutable
    (NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);

  /* component definitions adapted from volume.r */

  asysn_audiounit_classinfo_addint(ClassInfo, kAUPresetVersionKey, 
				   0);  /* version 0 */
  asysn_audiounit_classinfo_addint(ClassInfo, kAUPresetTypeKey, 
				   ASYS_AUDIOUNIT_COMP_TYPE);
  asysn_audiounit_classinfo_addint(ClassInfo, kAUPresetSubtypeKey, 
				   ASYS_AUDIOUNIT_COMP_SUBTYPE);
  asysn_audiounit_classinfo_addint(ClassInfo, kAUPresetManufacturerKey, 
				   ASYS_AUDIOUNIT_COMP_MANU);

  /* parameter data */

  if ((pdata = CFDataCreateMutable(NULL, 0)) && My->num_saolparams)
    {
      asysn_audiounit_classinfo_pdata_intwrite(pdata, 
  			              kAudioUnitScope_Global);     /* global scope */
      asysn_audiounit_classinfo_pdata_intwrite(pdata, 
				      0);                       /* element (bus) 0 */
      asysn_audiounit_classinfo_pdata_intwrite(pdata, 
				      My->num_saolparams); /* number of parameters */

      for (i = 0; i < My->num_saolparams; i++)
	{
	  /* parameter index */

	  asysn_audiounit_classinfo_pdata_intwrite(pdata, i);

	  /* value */

	  asysn_audiounit_classinfo_pdata_floatwrite
	    (pdata, 
	     (My->parameterinfo[i].flags & kAudioUnitParameterFlag_IsWritable) ?
	     My->saolparam[i].value : My->parameterinfo[i].defaultValue);
	}
    }

  if (pdata)
    {
      key = CFStringCreateWithCString(NULL, kAUPresetDataKey, kCFStringEncodingASCII);
      if (key)
	{
	  CFDictionarySetValue(ClassInfo, key, pdata);
	  CFRelease(key);
	}
      CFRelease(pdata);
    }

  asysn_audiounit_classinfo_addcfstr(ClassInfo, kAUPresetNameKey, 
				     My->PresentPreset.presetName);
  asysn_audiounit_classinfo_addint(ClassInfo, kAUPresetRenderQualityKey, 
				   (SInt32) (My->RenderQuality));
  asysn_audiounit_classinfo_addfloat(ClassInfo, kAUPresetCPULoadKey, 
				     My->CPULoad);

  /* add kAUPresetElementNameKey here if named elements/buses are supported */

  return ClassInfo;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/* Helper Functions for the SetProperty Selector Helper for ClassInfo  */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/**************************************/
/* Read integer item from a ClassInfo */
/**************************************/

int asysn_audiounit_classinfo_readint(CFMutableDictionaryRef ClassInfo, 
				      char * ckey, SInt32 * value)

{
  CFStringRef key;
  CFNumberRef num;
  SInt32 newval;
  int errcode = -1;

  key = CFStringCreateWithCString(NULL, ckey, kCFStringEncodingASCII);
  if (key)
    {
      num = CFDictionaryGetValue(ClassInfo, key);
      if (num && CFNumberGetValue(num, kCFNumberSInt32Type, &newval))
	{
	  errcode = 0;
	  *value = newval;
	}
      CFRelease(key);
    }
  return errcode;
}

/**************************************/
/*  Read float item from a ClassInfo  */
/**************************************/

int asysn_audiounit_classinfo_readfloat(CFMutableDictionaryRef ClassInfo, 
					char * ckey, Float32 * value)

{
  CFStringRef key;
  CFNumberRef num;
  Float32 newval;
  int errcode = -1;

  key = CFStringCreateWithCString(NULL, ckey, kCFStringEncodingASCII);
  if (key)
    {
      num = CFDictionaryGetValue(ClassInfo, key);
      if (num && CFNumberGetValue(num, kCFNumberFloat32Type, &newval))
	{
	  errcode = 0;
	  *value = newval;
	}
      CFRelease(key);
    }
  return errcode;
}

/***************************************/
/* Read CFString item from a ClassInfo */
/***************************************/

int asysn_audiounit_classinfo_readcfstr(CFMutableDictionaryRef ClassInfo, 
					char * ckey, CFStringRef * cfstr)

{
  CFStringRef key;
  CFNumberRef num;
  CFStringRef newstr;
  int errcode = -1;

  key = CFStringCreateWithCString(NULL, ckey, kCFStringEncodingASCII);
  if (key)
    {
      if (newstr = CFDictionaryGetValue(ClassInfo, key))
	{
	  errcode = 0;
	  CFRetain(newstr);
	  if (*cfstr)
	    CFRelease(*cfstr);
	  *cfstr = newstr;
	}
      CFRelease(key);
    }
  return errcode;
}

/***********************************************/
/* Big-endian int read from a CFMutableDataRef */
/***********************************************/

SInt32 asysn_audiounit_classinfo_pdata_intread(CFMutableDataRef pdata, UInt8 ** p)

{
  SInt32 value;
  
  value = ((*p)[0] << 24) | ((*p)[1] << 16) | ((*p)[2] << 8) | ((*p)[3]);
  (*p) += 4;
  return value;
}

/*************************************************/
/* Big-endian float read from a CFMutableDataRef */
/*************************************************/

Float32 asysn_audiounit_classinfo_pdata_floatread(CFMutableDataRef pdata, UInt8 ** p)

{
  union { SInt32 i; Float32 f ; } u;

  u.i = ((*p)[0] << 24) | ((*p)[1] << 16) | ((*p)[2] << 8) | ((*p)[3]);
  (*p) += 4;
  return u.f;
}

/****************************************/
/* Read parameter data from a ClassInfo */
/****************************************/

int asysn_audiounit_classinfo_readpdata(asysn_audiounit_InstanceState * My, 
					CFMutableDictionaryRef ClassInfo)

{
  int errcode = -1;
  CFStringRef key;
  CFMutableDataRef pdata;
  UInt8 * p, * pmax;
  UInt32 scope, bus, count, idx;
  Float32 fval;

  key = CFStringCreateWithCString(NULL, kAUPresetDataKey, kCFStringEncodingASCII);

  if (key && (pdata = (CFMutableDataRef) CFDictionaryGetValue(ClassInfo, key)))
    {
      p = (UInt8 *) CFDataGetBytePtr(pdata);
      pmax = p + CFDataGetLength(pdata);

      do {
	if (p == pmax)
	  {
	    errcode = 0; 
	    break;
	  }
	if ((pmax - p) >= 12)
	  {
	    scope = asysn_audiounit_classinfo_pdata_intread(pdata, &p);
	    bus = asysn_audiounit_classinfo_pdata_intread(pdata, &p);
	    count = asysn_audiounit_classinfo_pdata_intread(pdata, &p);
	    if ((pmax - p) >= count*(sizeof(SInt32) + sizeof(Float32)))
	      if ((scope == kAudioUnitScope_Global) && !bus)
		while (count--)
		  {
		    idx = asysn_audiounit_classinfo_pdata_intread(pdata, &p);
		    fval = asysn_audiounit_classinfo_pdata_floatread(pdata, &p);
		    if (idx < My->num_saolparams)
		      {
			if (fval > My->parameterinfo[idx].maxValue)
			  fval = My->parameterinfo[idx].maxValue;
			
			if (fval < My->parameterinfo[idx].minValue)
			  fval = My->parameterinfo[idx].minValue;

			My->saolparam[idx].value = fval;
		      }
		  }
	      else
		p += count*(sizeof(SInt32) + sizeof(Float32));
	    else
	      break;
	  }
	else
	  break;
      } while (1);
      
      CFRelease(key);
    }

  return errcode;
}

/************************************************************************/
/* Helper for SetProperty Selector: Reads ClassInfo returned by AU host */
/************************************************************************/

void asysn_audiounit_classinfo_read(asysn_audiounit_InstanceState * My, 
				    CFMutableDictionaryRef ClassInfo) 

{
  asysn_audiounit_classinfo_readpdata(My, ClassInfo);

  asysn_audiounit_engine_parameter_update(My); 

  asysn_audiounit_classinfo_readint(ClassInfo, kAUPresetRenderQualityKey, 
				    (SInt32 *) &(My->RenderQuality));


  OSSpinLockLock(&(My->lock_CPULoad));

  asysn_audiounit_classinfo_readfloat(ClassInfo, kAUPresetCPULoadKey, 
				      &(My->CPULoad));
  My->ksync_normalize = (Float32) (My->krate/AudioGetHostClockFrequency());
  if (My->CPULoad)
    My->ksync_normalize = My->ksync_normalize/My->CPULoad;

  OSSpinLockUnlock(&(My->lock_CPULoad));


  OSSpinLockLock(&(My->lock_PresentPreset));

  asysn_audiounit_classinfo_readcfstr(ClassInfo, kAUPresetNameKey, 
				      &(My->PresentPreset.presetName));

  My->PresentPreset.presetNumber = 
    asysn_audiounit_factorypresetnumber(My->PresentPreset.presetName); 

  OSSpinLockUnlock(&(My->lock_PresentPreset));


  /* add kAUPresetElementNameKey here if named elements/buses are supported */
}

/********************************************************************/
/* Helper Function for SetProperty: PresentProperty Factory Presets */
/********************************************************************/

void asysn_audiounit_presentproperty_setfactory
(asysn_audiounit_InstanceState * My) 

{
  int j, k;

  j = My->PresentPreset.presetNumber;

  if ((j < 0) || (j > My->num_factory))
    return;

  for (k = 0; k < My->num_saolparams; k++)
    My->saolparam[k].value = My->factorypreset_values[j][k];

  asysn_audiounit_engine_parameter_update(My); 
}

/*********************************************************************/
/* Helper for Property Selectors: returns 1 for supported properties */
/*********************************************************************/

int asysn_audiounit_supported_property(AudioUnitPropertyID id, AudioUnitScope scope) 

{
  switch (id) {
  case kAudioUnitProperty_ClassInfo: /* 0 */
    return 1;
  case kAudioUnitProperty_MakeConnection: /* 1 */
    return 1;
  case kAudioUnitProperty_SampleRate: /* 2 */
    switch (scope) {
    case kAudioUnitScope_Input:
      return (ASYS_AUDIOUNIT_EFFECT) ? 1 : 0;
    case kAudioUnitScope_Global:
    case kAudioUnitScope_Output:
      return 1;
    default:
      return 0;
    }
  case kAudioUnitProperty_ParameterList: /* 3 */
    return  (scope == kAudioUnitScope_Global);
  case kAudioUnitProperty_ParameterInfo: /* 4 */ 
    return 1;
  case kAudioUnitProperty_FastDispatch:  /* 5 */
    return 1;
  case kAudioUnitProperty_CPULoad: /* 6 */ 
    return (scope == kAudioUnitScope_Global) ? 
      ASYS_AUDIOUNIT_SUPPORT_PROPERTY_CPULOAD : 0; 
  case kAudioUnitProperty_StreamFormat: /* 8 */ 
    switch (scope) {
    case kAudioUnitScope_Input:
      return (ASYS_AUDIOUNIT_EFFECT) ? 1 : 0;
    case kAudioUnitScope_Global:
    case kAudioUnitScope_Output:
      return 1;
    default:
      return 0;
    }
  case kAudioUnitProperty_SRCAlgorithm: /* 9 */
    return 1;
  case kAudioUnitProperty_ElementCount: /* 11, also kAudioUnitProperty_BusCount */
    return 1;
  case kAudioUnitProperty_Latency: /* 12 */
    return (scope == kAudioUnitScope_Global) ? 1 : 0; 
  case kAudioUnitProperty_SupportedNumChannels: /* 13 */
    return (scope == kAudioUnitScope_Global) ? 1 : 0; 
  case kAudioUnitProperty_MaximumFramesPerSlice: /* 14 */
    return (scope == kAudioUnitScope_Global) ? 1 : 0; 
  case kAudioUnitProperty_ParameterValueStrings: /* 16 */
    return (scope == kAudioUnitScope_Global) ? 1 : 0; 
  case kAudioUnitProperty_TailTime: /* 20 */
    return (scope == kAudioUnitScope_Global) ? 1 : 0; 
  case kAudioUnitProperty_BypassEffect: /* 21 */
    return (scope == kAudioUnitScope_Global) ? 1 : 0; 
  case kAudioUnitProperty_LastRenderError: /* 22 */
    return (scope == kAudioUnitScope_Global) ? 1 : 0; 
  case kAudioUnitProperty_SetRenderCallback:  /* 23 */
    return 1;
  case kAudioUnitProperty_FactoryPresets: /* 24 */
    return 1;
  case kAudioUnitProperty_RenderQuality: /* 26 */
    return (scope == kAudioUnitScope_Global) ? 
      ASYS_AUDIOUNIT_SUPPORT_PROPERTY_RENDERQUALITY : 0; 
  case kAudioUnitProperty_InPlaceProcessing: /* 29 */
    return 1;
  case kAudioUnitProperty_CocoaUI: /* 31 */
#if defined(ASYS_AUDIOUNIT_VIEW_BUNDLECF)
    return 1;
#else
    return 0;
#endif
  case kAudioUnitProperty_OfflineRender: /* 37 */
    return 1;
  case kAudioUnitProperty_PresentPreset: /* 36 */
    return 1;
  case kAudioUnitProperty_PresentationLatency: /* 40 */
    return 1;
  case kMusicDeviceProperty_InstrumentCount: /* 1000 */
    switch (scope) {
    case kAudioUnitScope_Global:
    case kAudioUnitScope_Input:
      return ASYS_AUDIOUNIT_MUSICDEVICE ? 1 : 0;
    default:
      return 0;
    }
  case kMusicDeviceProperty_StreamFromDisk: /* 1011 */
    return ASYS_AUDIOUNIT_SUPPORT_PROPERTY_STREAMFROMDISK;
  default:
    return 0;
  }
}

/*******************************************************************/
/* Helper for property Selectors: returns property size (in bytes) */
/*******************************************************************/

unsigned int asysn_audiounit_property_size(AudioUnitPropertyID id, 
					   asysn_audiounit_InstanceState * My)

{
  switch (id) {
  case kAudioUnitProperty_ClassInfo: /* 0 */
    return sizeof(CFPropertyListRef);   
  case kAudioUnitProperty_MakeConnection: /* 1 */
    return sizeof(AudioUnitConnection);   
  case kAudioUnitProperty_SampleRate: /* 2 */
    return sizeof(Float64);   
  case kAudioUnitProperty_ParameterList: /* 3 */
    return My->num_saolparams*sizeof(AudioUnitParameterID);   
  case kAudioUnitProperty_ParameterInfo: /* 4 */ 
    return sizeof(AudioUnitParameterInfo);   
  case kAudioUnitProperty_FastDispatch:  /* 5 */
    return sizeof(void *);   
  case kAudioUnitProperty_CPULoad: /* 6 */ 
    return sizeof(Float32);   
  case kAudioUnitProperty_StreamFormat: /* 8 */ 
    return sizeof(AudioStreamBasicDescription);   
  case kAudioUnitProperty_SRCAlgorithm: /* 9 */
    return sizeof(OSType);   
  case kAudioUnitProperty_ElementCount: /* 11, also kAudioUnitProperty_BusCount */
    return sizeof(UInt32);   
  case kAudioUnitProperty_Latency: /* 12 */
    return sizeof(Float64);   
  case kAudioUnitProperty_SupportedNumChannels: /* 13 */
    return ASYS_AUDIOUNIT_SUPPORTED_FORMATS*sizeof(AUChannelInfo);   
  case kAudioUnitProperty_MaximumFramesPerSlice: /* 14 */
    return sizeof(UInt32);   
  case kAudioUnitProperty_ParameterValueStrings: /* 16 */
    return sizeof(CFArrayRef);
  case kAudioUnitProperty_TailTime: /* 20 */
    return sizeof(Float64);   
  case kAudioUnitProperty_BypassEffect: /* 21 */
    return sizeof(UInt32);   
  case kAudioUnitProperty_LastRenderError: /* 22 */
    return sizeof(OSStatus);   
  case kAudioUnitProperty_SetRenderCallback:  /* 23 */
    return sizeof(AURenderCallbackStruct);   
  case kAudioUnitProperty_FactoryPresets: /* 24 */
    return sizeof(CFArrayRef);
  case kAudioUnitProperty_RenderQuality: /* 26 */
    return sizeof(UInt32);   
  case kAudioUnitProperty_InPlaceProcessing: /* 29 */
    return sizeof(UInt32);   
  case kAudioUnitProperty_CocoaUI: /* 31 */
#if defined(ASYS_AUDIOUNIT_VIEW_BUNDLECF)
    return sizeof(AudioUnitCocoaViewInfo);
#else
    break;
#endif
  case kAudioUnitProperty_PresentPreset: /* 36 */
    return sizeof(AUPreset);
  case kAudioUnitProperty_OfflineRender: /* 37 */
    return sizeof(UInt32);   
  case kAudioUnitProperty_PresentationLatency: /* 40 */
    return sizeof(Float64);   
  case kMusicDeviceProperty_InstrumentCount: /* 1000 */
    return sizeof(UInt32);
  case kMusicDeviceProperty_StreamFromDisk: /* 1011 */
    return sizeof(UInt32);   
  default:  /* no entries here -- all cases above should specify datasize */
    break;
  }

  ASYS_AUDIOUNIT_WIRETAP_PUTSTRING("WARNING: asysn_audiounit_property_size()" 
				   "called on an unsupported property\n");
  return 0;
}

/***************************************************************/
/* Helper for Property Listener: returns proplisteners[] index */
/***************************************************************/

int asysn_audiounit_proplisteners_index(AudioUnitPropertyID id)

{
  int property = -1;

  switch (id) {
  case kAudioUnitProperty_ParameterList: /* 3 */ 
    property = ASYS_AUDIOUNIT_PROPLISTEN_PARAMETERLIST;
    break;
  case kAudioUnitProperty_ParameterInfo: /* 4 */ 
    property = ASYS_AUDIOUNIT_PROPLISTEN_PARAMETERINFO;
    break;
  case kAudioUnitProperty_CPULoad: /* 6 */
    property = ASYS_AUDIOUNIT_PROPLISTEN_CPULOAD;
    break;
  case kAudioUnitProperty_Latency: /* 12 */
    property = ASYS_AUDIOUNIT_PROPLISTEN_LATENCY;
    break;
  case kAudioUnitProperty_MaximumFramesPerSlice: /* 14 */
    property = ASYS_AUDIOUNIT_PROPLISTEN_MAXIMUMFRAMESPERSLICE;
    break;
  case kAudioUnitProperty_TailTime: /* 20 */
    property = ASYS_AUDIOUNIT_PROPLISTEN_TAILTIME;
    break;
  case kAudioUnitProperty_FactoryPresets: /* 24 */
    property = ASYS_AUDIOUNIT_PROPLISTEN_FACTORYPRESETS;
    break;
  case kAudioUnitProperty_RenderQuality: /* 26 */
    property = ASYS_AUDIOUNIT_PROPLISTEN_RENDERQUALITY;
    break;
  case kAudioUnitProperty_CurrentPreset: /* 28 */
    property = ASYS_AUDIOUNIT_PROPLISTEN_CURRENTPRESET;
    break;
  case kAudioUnitProperty_PresentPreset: /* 36 */
    property = ASYS_AUDIOUNIT_PROPLISTEN_PRESENTPRESET;
    break;
  case kMusicDeviceProperty_StreamFromDisk: /* 1011 */
    property = ASYS_AUDIOUNIT_PROPLISTEN_STREAMFROMDISK;
    break;
  }

  return property;
}

/***************************************************************/
/* Helper for properties: informs listeners of a status change */
/***************************************************************/

void asysn_audiounit_proplisteners_update(
                             asysn_audiounit_InstanceState * My,
			     AudioUnitPropertyID id, 
			     AudioUnitScope scope,
			     AudioUnitElement element)

{
  int idx = asysn_audiounit_proplisteners_index(id);
  asysn_audiounit_proplisten * lp;

  if (idx < 0)
    return;

  OSSpinLockLock(&(My->lock_proplisteners));

  for (lp = My->proplisteners[idx]; lp != NULL; lp = lp->next)
    (*(lp->lproc))(lp->lrefcon, My->component, id, scope, element);

  /* some apps are buggy and need a double-send ... */

  for (lp = My->proplisteners[idx]; lp != NULL; lp = lp->next)
    (*(lp->lproc))(lp->lrefcon, My->component, id, scope, element);

  OSSpinLockUnlock(&(My->lock_proplisteners));

  return;
}


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*  The istate state transition functions    */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/***************************************************************/
/* State transition diagram on entrance to InitializeSelect:   */
/*                                                             */
/*           ABSENT: Return _MY_IS_NULL.                       */
/*         INSTANCE: Set FLUX, return _INITIALIZE.             */
/* INSTANCE_PASSING: Set FLUX_PASSING, return _INITIALIZE.     */
/*             FLUX: Spin to INSTANCE* or ENGINE*.             */
/*     FLUX_PASSING: Spin to INSTANCE* or ENGINE*.             */
/*           ENGINE: Return _REINITIALIZE.                     */
/*   ENGINE_PASSING: Return _REINITIALIZE.                     */
/*    ENGINE_RENDER: Return _REINITIALIZE.                     */
/***************************************************************/

int asysn_audiounit_initializeselect_enterstate(asysn_audiounit_InstanceState * My)

{
  int ret;

  do 
    {
      switch(My->istate) {
      case ASYS_AUDIOUNIT_ISTATE_ABSENT:
	ret = ASYS_AUDIOUNIT_IRESPONSE_MY_IS_NULL;
	break; 
      case ASYS_AUDIOUNIT_ISTATE_INSTANCE:
	if (!OSAtomicCompareAndSwap32Barrier
	    (ASYS_AUDIOUNIT_ISTATE_INSTANCE, 
	     ASYS_AUDIOUNIT_ISTATE_FLUX, 
	     (int32_t *) &(My->istate)))
	  continue;
	ret = ASYS_AUDIOUNIT_IRESPONSE_INITIALIZE;
	break;
      case ASYS_AUDIOUNIT_ISTATE_INSTANCE_PASSING:
	if (!OSAtomicCompareAndSwap32Barrier
	    (ASYS_AUDIOUNIT_ISTATE_INSTANCE_PASSING, 
	     ASYS_AUDIOUNIT_ISTATE_FLUX_PASSING, 
	     (int32_t *) &(My->istate)))
	  continue;
	ret = ASYS_AUDIOUNIT_IRESPONSE_INITIALIZE;
	break;
      case ASYS_AUDIOUNIT_ISTATE_FLUX:
      case ASYS_AUDIOUNIT_ISTATE_FLUX_PASSING:
	OSMemoryBarrier();
	continue;          /* spin to _INSTANCE or _ENGINE */
	break; 
      case ASYS_AUDIOUNIT_ISTATE_ENGINE:
      case ASYS_AUDIOUNIT_ISTATE_ENGINE_PASSING:
      case ASYS_AUDIOUNIT_ISTATE_ENGINE_RENDER:
	ret = ASYS_AUDIOUNIT_IRESPONSE_REINITIALIZE;
	break;
      default:
	ret = ASYS_AUDIOUNIT_IRESPONSE_ERROR;
	ASYS_AUDIOUNIT_WIRETAP_PUTSTRING
	  ("\tBad transition - initializeselect_entrance (default)\n\n");
	break; 
      }
    } while (0);

  return ret;
}

/***************************************************************/
/* State transition diagram on exit from InitializeSelect:     */
/*                                                             */
/*                                                             */
/*           ABSENT: Return _ERROR.                            */
/*         INSTANCE: Return _ERROR.                            */
/* INSTANCE_PASSING: Return _ERROR.                            */
/*             FLUX: Set ENGINE, return _COMPLETE.             */
/*     FLUX_PASSING: Set ENGINE_PASSING, return _COMPLETE.     */
/*           ENGINE: Return _ERROR.                            */
/*   ENGINE_PASSING: Return _ERROR.                            */
/*    ENGINE_RENDER: Return _ERROR.                            */
/*                                                             */
/***************************************************************/

int asysn_audiounit_initializeselect_exitstate(asysn_audiounit_InstanceState * My)

{
  int ret;

  do 
    {
      switch(My->istate) {
      case ASYS_AUDIOUNIT_ISTATE_ABSENT:
	ASYS_AUDIOUNIT_WIRETAP_PUTSTRING
	  ("\tBad transition - initializeselect_exit (absent)\n\n");
	ret = ASYS_AUDIOUNIT_IRESPONSE_ERROR;
	break;
      case ASYS_AUDIOUNIT_ISTATE_INSTANCE:
      case ASYS_AUDIOUNIT_ISTATE_INSTANCE_PASSING:
	ASYS_AUDIOUNIT_WIRETAP_PUTSTRING
	  ("\tBad transition - initializeselect_exit (instance*)\n\n");
	ret = ASYS_AUDIOUNIT_IRESPONSE_ERROR;
	break;
      case ASYS_AUDIOUNIT_ISTATE_FLUX:
	if (!OSAtomicCompareAndSwap32Barrier
	    (ASYS_AUDIOUNIT_ISTATE_FLUX, 
	     ASYS_AUDIOUNIT_ISTATE_ENGINE, 
	     (int32_t *) &(My->istate)))
	  continue;
	ret = ASYS_AUDIOUNIT_IRESPONSE_COMPLETE;
	break;
      case ASYS_AUDIOUNIT_ISTATE_FLUX_PASSING:
	if (!OSAtomicCompareAndSwap32Barrier
	    (ASYS_AUDIOUNIT_ISTATE_FLUX_PASSING, 
	     ASYS_AUDIOUNIT_ISTATE_ENGINE_PASSING, 
	     (int32_t *) &(My->istate)))
	  continue;
	ret = ASYS_AUDIOUNIT_IRESPONSE_COMPLETE;
	break;
      case ASYS_AUDIOUNIT_ISTATE_ENGINE:
      case ASYS_AUDIOUNIT_ISTATE_ENGINE_PASSING:
      case ASYS_AUDIOUNIT_ISTATE_ENGINE_RENDER:
	ASYS_AUDIOUNIT_WIRETAP_PUTSTRING
	  ("\tBad transition - initializeselect_exit (engine*)\n\n");
	ret = ASYS_AUDIOUNIT_IRESPONSE_ERROR;
	break;
      default:
	ASYS_AUDIOUNIT_WIRETAP_PUTSTRING
	  ("\tBad transition - initializeselect_exit (default)\n\n");
	ret = ASYS_AUDIOUNIT_IRESPONSE_ERROR;
	break;
      }
    } while (0);
  
  return ret;
}


/********************************************************************/
/* State transition diagram on entrance to UninitializeSelect:      */
/*                                                                  */
/*           ABSENT: Return _MY_IS_NULL.                            */
/*         INSTANCE: Return _REUNINITIALIZE.                        */ 
/* INSTANCE_PASSING: Return _REUNINITIALIZE.                        */ 
/*             FLUX: Spin to INSTANCE* or ENGINE*.                  */
/*     FLUX_PASSING: Spin to INSTANCE* or ENGINE*.                  */
/*           ENGINE: Set FLUX, return _UNINITIALIZE                 */
/*   ENGINE_PASSING: Spin to ENGINE.                                */
/*    ENGINE_RENDER: Spin to ENGINE.                                */
/********************************************************************/

int asysn_audiounit_uninitializeselect_enterstate(asysn_audiounit_InstanceState * My)

{
  int ret;

  do 
    {
      switch(My->istate) {
      case ASYS_AUDIOUNIT_ISTATE_ABSENT:
	ret = ASYS_AUDIOUNIT_IRESPONSE_MY_IS_NULL;
	break; 
      case ASYS_AUDIOUNIT_ISTATE_INSTANCE:
      case ASYS_AUDIOUNIT_ISTATE_INSTANCE_PASSING:
	ret = ASYS_AUDIOUNIT_IRESPONSE_REUNINITIALIZE;
	break;
      case ASYS_AUDIOUNIT_ISTATE_FLUX:
      case ASYS_AUDIOUNIT_ISTATE_FLUX_PASSING:
	OSMemoryBarrier();
	continue;          /* spin to _INSTANCE or _ENGINE */
	break; 
      case ASYS_AUDIOUNIT_ISTATE_ENGINE:
	if (!OSAtomicCompareAndSwap32Barrier
	    (ASYS_AUDIOUNIT_ISTATE_ENGINE, 
	     ASYS_AUDIOUNIT_ISTATE_FLUX, 
	     (int32_t *) &(My->istate)))
	  continue;
	ret = ASYS_AUDIOUNIT_IRESPONSE_UNINITIALIZE;
	break;
      case ASYS_AUDIOUNIT_ISTATE_ENGINE_PASSING:
      case ASYS_AUDIOUNIT_ISTATE_ENGINE_RENDER:
	OSMemoryBarrier();
	continue;           /* spin to _ENGINE */
      default:
	ret = ASYS_AUDIOUNIT_IRESPONSE_ERROR;
	ASYS_AUDIOUNIT_WIRETAP_PUTSTRING
	  ("\tBad transition - uninitializeselect_entrance\n\n");
	break; 
      }
    } while (0);

  return ret;
}


/***************************************************************/
/* State transition diagram on exit from UninitializeSelect:   */
/*                                                             */
/*           ABSENT: Return _ERROR.                            */
/*         INSTANCE: Return _ERROR.                            */
/* INSTANCE_PASSING: Return _ERROR.                            */
/*             FLUX: Set INSTANCE, return _COMPLETE.           */
/*     FLUX_PASSING: Set INSTANCE_PASSING, return _COMPLETE.   */
/*           ENGINE: Return _ERROR.                            */
/*   ENGINE_PASSING: Return _ERROR.                            */
/*    ENGINE_RENDER: Return _ERROR.                            */
/*                                                             */
/***************************************************************/

int asysn_audiounit_uninitializeselect_exitstate(asysn_audiounit_InstanceState * My)

{
  int ret;

  do 
    {
      switch(My->istate) {
      case ASYS_AUDIOUNIT_ISTATE_ABSENT:
	ASYS_AUDIOUNIT_WIRETAP_PUTSTRING
	  ("\tBad transition - uninitializeselect_exit (absent)\n\n");
	ret = ASYS_AUDIOUNIT_IRESPONSE_ERROR;
	break;
      case ASYS_AUDIOUNIT_ISTATE_INSTANCE:
      case ASYS_AUDIOUNIT_ISTATE_INSTANCE_PASSING:
	ASYS_AUDIOUNIT_WIRETAP_PUTSTRING
	  ("\tBad transition - uninitializeselect_exit (instance*)\n\n");
	ret = ASYS_AUDIOUNIT_IRESPONSE_ERROR;
	break;
      case ASYS_AUDIOUNIT_ISTATE_FLUX:
	if (!OSAtomicCompareAndSwap32Barrier
	    (ASYS_AUDIOUNIT_ISTATE_FLUX, 
	     ASYS_AUDIOUNIT_ISTATE_INSTANCE, 
	     (int32_t *) &(My->istate)))
	  continue;
	ret = ASYS_AUDIOUNIT_IRESPONSE_COMPLETE;
	break;
      case ASYS_AUDIOUNIT_ISTATE_FLUX_PASSING:
	if (!OSAtomicCompareAndSwap32Barrier
	    (ASYS_AUDIOUNIT_ISTATE_FLUX_PASSING, 
	     ASYS_AUDIOUNIT_ISTATE_INSTANCE_PASSING, 
	     (int32_t *) &(My->istate)))
	  continue;
	ret = ASYS_AUDIOUNIT_IRESPONSE_COMPLETE;
	break;
      case ASYS_AUDIOUNIT_ISTATE_ENGINE:
      case ASYS_AUDIOUNIT_ISTATE_ENGINE_PASSING:
      case ASYS_AUDIOUNIT_ISTATE_ENGINE_RENDER:
	ASYS_AUDIOUNIT_WIRETAP_PUTSTRING
	  ("\tBad transition - uninitializeselect_exit (engine*)\n\n");
	ret = ASYS_AUDIOUNIT_IRESPONSE_ERROR;
	break;
      default:
	ASYS_AUDIOUNIT_WIRETAP_PUTSTRING
	  ("\tBad transition - uninitializeselect_exit (default)\n\n");
	ret = ASYS_AUDIOUNIT_IRESPONSE_ERROR;
	break;
      }
    } while (0);
  
  return ret;
}

/*****************************************************************/
/* State transition diagram on entrance to CloseSelect:          */
/*                                                               */
/*           ABSENT: Return _MY_IS_NULL.                         */
/*         INSTANCE: Set ABSENT, return _CLOSE.                  */
/* INSTANCE_PASSING: Spin to INSTANCE                            */
/*             FLUX: Spin to INSTANCE or ENGINE                  */
/*     FLUX_PASSING: Spin to INSTANCE or ENGINE                  */
/*           ENGINE: Set ABSENT, return UNINITIALIZE_AND_CLOSE   */
/*   ENGINE_PASSING: Spin to ENGINE                              */
/*    ENGINE_RENDER: Spin to ENGINE                              */
/*****************************************************************/

int asysn_audiounit_closeselect_enterstate(asysn_audiounit_InstanceState * My)

{
  int ret;

  do 
    {
      switch(My->istate) {
      case ASYS_AUDIOUNIT_ISTATE_ABSENT:
	ret = ASYS_AUDIOUNIT_IRESPONSE_MY_IS_NULL;
	break; 
      case ASYS_AUDIOUNIT_ISTATE_INSTANCE:
	if (!OSAtomicCompareAndSwap32Barrier
	    (ASYS_AUDIOUNIT_ISTATE_INSTANCE, 
	     ASYS_AUDIOUNIT_ISTATE_ABSENT, 
	     (int32_t *) &(My->istate)))
	  continue;
	ret = ASYS_AUDIOUNIT_IRESPONSE_CLOSE;
	break;
      case ASYS_AUDIOUNIT_ISTATE_INSTANCE_PASSING:
	OSMemoryBarrier();
	continue;          /* spin to _INSTANCE */
	break;
      case ASYS_AUDIOUNIT_ISTATE_FLUX:
      case ASYS_AUDIOUNIT_ISTATE_FLUX_PASSING:
	OSMemoryBarrier();
	continue;          /* spin to _INSTANCE or _ENGINE */
	break;
      case ASYS_AUDIOUNIT_ISTATE_ENGINE:
	if (!OSAtomicCompareAndSwap32Barrier
	    (ASYS_AUDIOUNIT_ISTATE_ENGINE, 
	     ASYS_AUDIOUNIT_ISTATE_ABSENT, 
	     (int32_t *) &(My->istate)))
	  continue;
	ret = ASYS_AUDIOUNIT_IRESPONSE_UNINITIALIZE_AND_CLOSE;
	break;
      case ASYS_AUDIOUNIT_ISTATE_ENGINE_PASSING:
      case ASYS_AUDIOUNIT_ISTATE_ENGINE_RENDER:
	OSMemoryBarrier();
	continue;          /* spin to _ENGINE */
	break;
      default:
	ret = ASYS_AUDIOUNIT_IRESPONSE_ERROR;
	ASYS_AUDIOUNIT_WIRETAP_PUTSTRING
	  ("\tBad transition - closeselect_enter\n\n");
	break; 
      }
    } while (0);
  
  return ret;
}

/******************************************************************/
/* State transition diagram on entrance to real-time thread:      */
/*                                                                */
/*           ABSENT: Return _MY_IS_NULL (abnormal)                */
/*         INSTANCE: Set INSTANCE_PASSING, return _WIRE           */
/* INSTANCE_PASSING: Set INSTANCE, return _WIRE (abnormal)        */
/*             FLUX: Set FLUX_PASSING, return _WIRE               */
/*     FLUX_PASSING: Set FLUX, return _WIRE (abnormal)            */
/*           ENGINE: Set ENGINE_RENDER, return _RENDER            */
/*   ENGINE_PASSING: Set ENGINE_RENDER, return _RENDER (abnormal) */
/*    ENGINE_RENDER: Return _RENDER (abnormal)                    */
/******************************************************************/

int asysn_audiounit_realtime_enterstate(asysn_audiounit_InstanceState * My)

{
  int ret;

  do 
    {
      switch(My->istate) {
      case ASYS_AUDIOUNIT_ISTATE_ABSENT:
	ret = ASYS_AUDIOUNIT_IRESPONSE_MY_IS_NULL;
	ASYS_AUDIOUNIT_WIRETAP_PUTSTRING
	  ("\tBad transition - realtime_entrance (absent)\n\n");
	break; 
      case ASYS_AUDIOUNIT_ISTATE_INSTANCE:
	if (!OSAtomicCompareAndSwap32Barrier
	    (ASYS_AUDIOUNIT_ISTATE_INSTANCE, 
	     ASYS_AUDIOUNIT_ISTATE_INSTANCE_PASSING, 
	     (int32_t *) &(My->istate)))
	  continue;
	ret = ASYS_AUDIOUNIT_IRESPONSE_WIRE;
	break;
      case ASYS_AUDIOUNIT_ISTATE_INSTANCE_PASSING:
	if (!OSAtomicCompareAndSwap32Barrier
	    (ASYS_AUDIOUNIT_ISTATE_INSTANCE_PASSING, 
	     ASYS_AUDIOUNIT_ISTATE_INSTANCE, 
	     (int32_t *) &(My->istate)))
	  continue;
	ret = ASYS_AUDIOUNIT_IRESPONSE_WIRE;
	ASYS_AUDIOUNIT_WIRETAP_PUTSTRING
	  ("\tBad transition - realtime_entrance (instance_passing)\n\n");
	break;
      case ASYS_AUDIOUNIT_ISTATE_FLUX:
	if (!OSAtomicCompareAndSwap32Barrier
	    (ASYS_AUDIOUNIT_ISTATE_FLUX, 
	     ASYS_AUDIOUNIT_ISTATE_FLUX_PASSING, 
	     (int32_t *) &(My->istate)))
	  continue;
	ret = ASYS_AUDIOUNIT_IRESPONSE_WIRE;
	break;
      case ASYS_AUDIOUNIT_ISTATE_FLUX_PASSING:
	if (!OSAtomicCompareAndSwap32Barrier
	    (ASYS_AUDIOUNIT_ISTATE_FLUX_PASSING, 
	     ASYS_AUDIOUNIT_ISTATE_FLUX, 
	     (int32_t *) &(My->istate)))
	  continue;
	ret = ASYS_AUDIOUNIT_IRESPONSE_WIRE;
	ASYS_AUDIOUNIT_WIRETAP_PUTSTRING
	  ("\tBad transition - realtime_entrance (flux_passing)\n\n");
	break; 
      case ASYS_AUDIOUNIT_ISTATE_ENGINE:
	if (!OSAtomicCompareAndSwap32Barrier
	    (ASYS_AUDIOUNIT_ISTATE_ENGINE, 
	     ASYS_AUDIOUNIT_ISTATE_ENGINE_PASSING, 
	     (int32_t *) &(My->istate)))
	  continue;
	ret = ASYS_AUDIOUNIT_IRESPONSE_RENDER;
	break;
      case ASYS_AUDIOUNIT_ISTATE_ENGINE_PASSING:
	if (!OSAtomicCompareAndSwap32Barrier
	    (ASYS_AUDIOUNIT_ISTATE_ENGINE_PASSING, 
	     ASYS_AUDIOUNIT_ISTATE_ENGINE, 
	     (int32_t *) &(My->istate)))
	  continue;
	ret = ASYS_AUDIOUNIT_IRESPONSE_RENDER;
	ASYS_AUDIOUNIT_WIRETAP_PUTSTRING
	  ("\tBad transition - realtime_entrance (engine_passing)\n\n");
	break; 
      case ASYS_AUDIOUNIT_ISTATE_ENGINE_RENDER:
	if (!OSAtomicCompareAndSwap32Barrier
	    (ASYS_AUDIOUNIT_ISTATE_ENGINE_RENDER, 
	     ASYS_AUDIOUNIT_ISTATE_ENGINE, 
	     (int32_t *) &(My->istate)))
	  continue;
	ret = ASYS_AUDIOUNIT_IRESPONSE_RENDER;
	ASYS_AUDIOUNIT_WIRETAP_PUTSTRING
	  ("\tBad transition - realtime_entrance (engine_render)\n\n");
	break;
      default:
	ret = ASYS_AUDIOUNIT_IRESPONSE_ERROR;
	ASYS_AUDIOUNIT_WIRETAP_PUTSTRING
	  ("\tBad transition - realtime_entrance (default)\n\n");
	break; 
      }
    } while (0);

  return ret;
}

/************************************************************/
/* State transition diagram on exit from real-time thread:  */
/*                                                          */
/*           ABSENT: Return _MY_IS_NULL (abnormal)          */
/*         INSTANCE: Return _COMPLETE (abnormal)            */
/* INSTANCE_PASSING: Set INSTANCE, return _COMPLETE         */
/*             FLUX: Return _COMPLETE (abnormal)            */
/*     FLUX_PASSING: Set FLUX, return _COMPLETE             */
/*           ENGINE: Return _COMPLETE (abnormal)            */
/*   ENGINE_PASSING: Set ENGINE, return _COMPLETE           */
/*    ENGINE_RENDER: Set ENGINE, return _COMPLETE           */
/************************************************************/

int asysn_audiounit_realtime_exitstate(asysn_audiounit_InstanceState * My)

{
  int ret;

  do 
    {
      switch(My->istate) {
      case ASYS_AUDIOUNIT_ISTATE_ABSENT:
	ret = ASYS_AUDIOUNIT_IRESPONSE_MY_IS_NULL;
	ASYS_AUDIOUNIT_WIRETAP_PUTSTRING
	  ("\tBad transition - realtime_exit (absent)\n\n");
	break; 
      case ASYS_AUDIOUNIT_ISTATE_INSTANCE:
	ret = ASYS_AUDIOUNIT_IRESPONSE_COMPLETE;
	ASYS_AUDIOUNIT_WIRETAP_PUTSTRING
	  ("\tBad transition - realtime_exit (instance)\n\n");
	break;
      case ASYS_AUDIOUNIT_ISTATE_INSTANCE_PASSING:
	if (!OSAtomicCompareAndSwap32Barrier
	    (ASYS_AUDIOUNIT_ISTATE_INSTANCE_PASSING, 
	     ASYS_AUDIOUNIT_ISTATE_INSTANCE, 
	     (int32_t *) &(My->istate)))
	  continue;
	ret = ASYS_AUDIOUNIT_IRESPONSE_COMPLETE;
	break;
      case ASYS_AUDIOUNIT_ISTATE_FLUX:
	ret = ASYS_AUDIOUNIT_IRESPONSE_COMPLETE;
	ASYS_AUDIOUNIT_WIRETAP_PUTSTRING
	  ("\tBad transition - realtime_exit (flux)\n\n");
	break;
      case ASYS_AUDIOUNIT_ISTATE_FLUX_PASSING:
	if (!OSAtomicCompareAndSwap32Barrier
	    (ASYS_AUDIOUNIT_ISTATE_FLUX_PASSING, 
	     ASYS_AUDIOUNIT_ISTATE_FLUX, 
	     (int32_t *) &(My->istate)))
	  continue;
	ret = ASYS_AUDIOUNIT_IRESPONSE_COMPLETE;
	break; 
      case ASYS_AUDIOUNIT_ISTATE_ENGINE:
	ret = ASYS_AUDIOUNIT_IRESPONSE_COMPLETE;
	ASYS_AUDIOUNIT_WIRETAP_PUTSTRING
	  ("\tBad transition - realtime_exit (engine)\n\n");
	break;
      case ASYS_AUDIOUNIT_ISTATE_ENGINE_PASSING:
	if (!OSAtomicCompareAndSwap32Barrier
	    (ASYS_AUDIOUNIT_ISTATE_ENGINE_PASSING, 
	     ASYS_AUDIOUNIT_ISTATE_ENGINE, 
	     (int32_t *) &(My->istate)))
	  continue;
	ret = ASYS_AUDIOUNIT_IRESPONSE_COMPLETE;
	break; 
      case ASYS_AUDIOUNIT_ISTATE_ENGINE_RENDER:
	if (!OSAtomicCompareAndSwap32Barrier
	    (ASYS_AUDIOUNIT_ISTATE_ENGINE_RENDER, 
	     ASYS_AUDIOUNIT_ISTATE_ENGINE, 
	     (int32_t *) &(My->istate)))
	  continue;
	ret = ASYS_AUDIOUNIT_IRESPONSE_COMPLETE;
	break;
      default:
	ret = ASYS_AUDIOUNIT_IRESPONSE_ERROR;
	ASYS_AUDIOUNIT_WIRETAP_PUTSTRING
	  ("\tBad transition - realtime_exit (default)\n\n");
	break; 
      }
    } while (0);

  return ret;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~*/
/*  The Selector Calls    */
/*~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~*/

/**************************************/
/*          Selector: Open            */
/*                                    */
/*  params[0]: ComponentInstance ci   */
/*                                    */
/**************************************/

ComponentResult asysn_audiounit_kComponentOpenSelect(ComponentParameters * p, 
						     asysn_audiounit_InstanceState * My)
{ 
  int socket_flags;
  int i, msize;

  msize = sizeof(asysn_audiounit_InstanceState);
  if (!(My = (asysn_audiounit_InstanceState *) calloc(1, msize)))
    return asysn_audiounit_instance_cleanup(p, My);
  asysn_audiounit_memstatus(My, msize, MADV_WILLNEED);

  My->component = (ComponentInstance) (p->params[0]);
  sprintf(My->componentname, "c%u", (unsigned int) My->component);
  My->argv[0] = My->componentname;

  if (asysn_audiounit_initialize_properties(My) != noErr)
    return asysn_audiounit_instance_cleanup(p, My);

  if (ASYS_AUDIOUNIT_EFFECT)
    {
      msize = (sizeof(UInt32) + sizeof(AudioBuffer)*ASYS_AUDIOUNIT_INPUT_MAXCHANNELS);

      if (!(My->AudioBufferCarrier = calloc(msize, 1)))
	return asysn_audiounit_instance_cleanup(p, My);

      if (!(My->AudioBufferTemplate = calloc(msize, 1)))
	return asysn_audiounit_instance_cleanup(p, My);

      asysn_audiounit_memstatus(My->AudioBufferCarrier, msize, MADV_WILLNEED);
      asysn_audiounit_memstatus(My->AudioBufferTemplate, msize, MADV_WILLNEED);
      
      My->AudioBufferTemplate->mNumberBuffers = ASYS_AUDIOUNIT_INPUT_CHANNELS;

      msize = sizeof(Float32)*ASYS_AUDIOUNIT_FRAMES_PER_SLICE;

      for(i = 0; i < ASYS_AUDIOUNIT_INPUT_MAXCHANNELS; i++)
	{
	  My->AudioBufferTemplate->mBuffers[i].mNumberChannels = 1;

	  if (!(My->AudioBufferTemplate->mBuffers[i].mData = malloc(msize)))
	    return asysn_audiounit_instance_cleanup(p, My);

	  asysn_audiounit_memstatus(My->AudioBufferTemplate->mBuffers[i].mData, 
				    msize, MADV_WILLNEED);
	}
    }

  msize = sizeof(Float32)*ASYS_AUDIOUNIT_FRAMES_PER_SLICE;

  for(i = 0; i < ASYS_AUDIOUNIT_OUTPUT_MAXCHANNELS; i++)
    {
      if (!(My->mData_Output[i] = (Float32 *) malloc(msize)))
	return asysn_audiounit_instance_cleanup(p, My);
      asysn_audiounit_memstatus(My->mData_Output[i], msize, MADV_WILLNEED);
    }

  if (ASYS_AUDIOUNIT_HAS_AUCONTROL)
    {
      /*~~~~~~~~~~~~~~~~~~~~*/
      /* set up MIDI socket */
      /*--------------------*/

      if (socketpair(AF_UNIX, SOCK_DGRAM, 0, My->mpipepair))
	{
	  ASYS_AUDIOUNIT_WIRETAP_PUTSTRING
	    ("\n\tIn OpenSelect: Error during MIDI socketpair() call.\n");
	  return asysn_audiounit_instance_cleanup(p, My);
	} 

      if ((socket_flags = fcntl(My->mpipepair[0], F_GETFL, 0)) == -1)
	{
	  ASYS_AUDIOUNIT_WIRETAP_PUTSTRING
	    ("\n\tIn OpenSelect: Error during mpipepair[0] F_GETFL fcntl() call.\n");
	  return asysn_audiounit_instance_cleanup(p, My);
	}

      if (fcntl(My->mpipepair[0], F_SETFL, socket_flags | O_NONBLOCK) == -1)
	{
	  ASYS_AUDIOUNIT_WIRETAP_PUTSTRING
	    ("\n\tIn OpenSelect: Error during mpipepair[0] F_SETFL fcntl() call.\n");
	  return asysn_audiounit_instance_cleanup(p, My);
	}

      /*~~~~~~~~~~~~~~~~~~~~*/
      /* set up SASL socket */
      /*--------------------*/

      if (socketpair(AF_UNIX, SOCK_DGRAM, 0, My->spipepair))
	{
	  ASYS_AUDIOUNIT_WIRETAP_PUTSTRING
	    ("\n\tIn OpenSelect: Error during SASL socketpair() call.\n");
	  return asysn_audiounit_instance_cleanup(p, My);
	} 

      if ((socket_flags = fcntl(My->spipepair[0], F_GETFL, 0)) == -1)
	{
	  ASYS_AUDIOUNIT_WIRETAP_PUTSTRING
	    ("\n\tIn OpenSelect: Error during spipepair[0] F_GETFL fcntl() call.\n");
	  return asysn_audiounit_instance_cleanup(p, My);
	}

      if (fcntl(My->spipepair[0], F_SETFL, socket_flags | O_NONBLOCK) == -1)
	{
	  ASYS_AUDIOUNIT_WIRETAP_PUTSTRING
	    ("\n\tIn OpenSelect: Error during spipepair[0] F_SETFL fcntl() call.\n");
	  return asysn_audiounit_instance_cleanup(p, My);
	}

      /*~~~~~~~~~~~~~~~~~~~~~~*/
      /* set up argc and argv */
      /*----------------------*/

      My->argc = 5;

      My->argv[1] = My->mpipeflag;
      My->argv[2] = My->mpipevalue;

      sprintf(My->mpipeflag, "-asys_audiounit_mpipe");
      sprintf(My->mpipevalue, "%i", My->mpipepair[0]);

      My->argv[3] = My->spipeflag;
      My->argv[4] = My->spipevalue;

      sprintf(My->spipeflag, "-asys_audiounit_spipe");
      sprintf(My->spipevalue, "%i", My->spipepair[0]);
      }
  else
    {
      My->argc = 1;

      My->argv[1] = My->argv[2] = NULL;
      My->mpipepair[0] = My->mpipepair[1] = 0;

      My->argv[3] = My->argv[4] = NULL;
      My->spipepair[0] = My->spipepair[1] = 0;
    }

  if (asysn_audiounit_initialize_parametersystem(My) != noErr)
    return asysn_audiounit_instance_cleanup(p, My);

  OSSpinLockLock(&asysn_audiounit_lock_opencount);

  if (!(asysn_audiounit_opencount++))
    asysn_audiounit_globalvars_memstatus(MADV_WILLNEED);

  OSSpinLockUnlock(&asysn_audiounit_lock_opencount);

  My->istate = ASYS_AUDIOUNIT_ISTATE_INSTANCE;

  SetComponentInstanceStorage((ComponentInstance) p->params[0], (Handle) My); 

  return noErr; 
}

/**************************************/
/*           Selector: Close          */
/*                                    */
/*  params[0]: ComponentInstance ci   */
/*                                    */
/**************************************/

ComponentResult asysn_audiounit_kComponentCloseSelect(ComponentParameters * p, 
					    asysn_audiounit_InstanceState * My)
{ 
  ENGINE_PTR_DECLARE_SEMICOLON
  asysn_audiounit_proplisten * remove;
  asysn_audiounit_rendernotify * notify;
  int i, k, msize, ret;

  ret = asysn_audiounit_closeselect_enterstate(My);

  SetComponentInstanceStorage((ComponentInstance) p->params[0], (Handle) NULL); 

  switch (ret) {
  case ASYS_AUDIOUNIT_IRESPONSE_UNINITIALIZE_AND_CLOSE:
    ENGINE_PTR_ASSIGNED_TO  MY_ENGINE_PTR;
    ENGINE_MEMSTATUS_SEMICOLON(MADV_FREE)
    shut_down(ENGINE_PTR);
    MY_ENGINE_PTR_ASSIGNED_TO  NULL;
    break;
  case ASYS_AUDIOUNIT_IRESPONSE_CLOSE:
    break;
  case ASYS_AUDIOUNIT_IRESPONSE_MY_IS_NULL:
  case ASYS_AUDIOUNIT_IRESPONSE_ERROR:
  default:
    return noErr;
  }

  if (ASYS_AUDIOUNIT_HAS_AUCONTROL)
    {
      close(My->mpipepair[0]);
      close(My->mpipepair[1]);
      close(My->spipepair[0]);
      close(My->spipepair[1]);

      if (My->num_saolparams)
	{
	  free(My->parameterlist);
	  for (i = 0; i < My->num_saolparams; i++)
	    {
	      if (My->parameterinfo[i].cfNameString)
		CFRelease(My->parameterinfo[i].cfNameString);
	      if (My->parameterinfo[i].unitName)
		CFRelease(My->parameterinfo[i].unitName);
	    }
	  free(My->parameterinfo);

	  for (i = 0; i < My->num_saolparams; i++)
	    if (My->pvs_size[i])
	      {
		for (k = 0; k < My->pvs_size[i]; k++)
		  free(My->pvs_cstr[i][k]);
		free(My->pvs_cstr[i]);
	      }
	  free(My->pvs_size);
	  free(My->pvs_cstr);

	  msize = My->num_saolparams*sizeof(asysn_audiounit_saolparam);
	  asysn_audiounit_memstatus(My->saolparam, msize, MADV_FREE);
	  free(My->saolparam);
	}

      if (My->num_factory)
	{
	  for (i = 0; i < My->num_factory; i++)
	    {
	      if (My->factorypreset_values[i])
		{
		  msize = My->num_saolparams*sizeof(Float32);
		  asysn_audiounit_memstatus(My->factorypreset_values[i], 
					    msize, MADV_FREE);
		  free(My->factorypreset_values[i]);
		}

	      if (My->factorypreset_info[i].presetName)
		CFRelease(My->factorypreset_info[i].presetName);
	    }

	  msize = My->num_factory*sizeof(Float32 *);
	  asysn_audiounit_memstatus(My->factorypreset_values, msize, MADV_FREE);
	  free(My->factorypreset_values);

	  free(My->factorypreset_info);
	  CFRelease(My->factorypreset_array);
	}
    }


  OSSpinLockLock(&(My->lock_PresentPreset));

  CFRelease(My->PresentPreset.presetName);

  OSSpinLockUnlock(&(My->lock_PresentPreset));


  OSSpinLockLock(&(My->lock_proplisteners));

  for (i = 0; i < ASYS_AUDIOUNIT_PROPLISTEN_ARRAYSIZE; i++)
    while (My->proplisteners[i] != NULL)
      {
	remove = My->proplisteners[i];
	My->proplisteners[i] = My->proplisteners[i]->next;
	msize = sizeof(asysn_audiounit_proplisten);
	asysn_audiounit_memstatus(remove, msize, MADV_FREE);
	free(remove);
      }

  OSSpinLockUnlock(&(My->lock_proplisteners));


  OSSpinLockLock(&(My->lock_rendernotify));

  while (My->rendernotify)
    {
      notify = My->rendernotify;
      My->rendernotify = notify->next;
      msize = sizeof(asysn_audiounit_rendernotify);
      asysn_audiounit_memstatus(notify, msize, MADV_FREE);
      free(notify);
    }

  OSSpinLockUnlock(&(My->lock_rendernotify));


  if (ASYS_AUDIOUNIT_EFFECT)
    {
      msize = (sizeof(UInt32) + sizeof(AudioBuffer)*ASYS_AUDIOUNIT_INPUT_MAXCHANNELS);
      asysn_audiounit_memstatus(My->AudioBufferCarrier, msize, MADV_FREE);
      free(My->AudioBufferCarrier);

      msize = sizeof(Float32)*ASYS_AUDIOUNIT_FRAMES_PER_SLICE;
      for(i = 0; i < ASYS_AUDIOUNIT_INPUT_MAXCHANNELS; i++)
	{
	  asysn_audiounit_memstatus(My->AudioBufferTemplate->mBuffers[i].mData, 
				    msize, MADV_FREE);
	  free(My->AudioBufferTemplate->mBuffers[i].mData);
	}

      msize = (sizeof(UInt32) + sizeof(AudioBuffer)*ASYS_AUDIOUNIT_INPUT_MAXCHANNELS);
      asysn_audiounit_memstatus(My->AudioBufferTemplate, msize, MADV_FREE);
      free(My->AudioBufferTemplate);
    }

  for(i = 0; i < ASYS_AUDIOUNIT_OUTPUT_MAXCHANNELS; i++)
    {
      msize = sizeof(Float32)*ASYS_AUDIOUNIT_FRAMES_PER_SLICE;
      asysn_audiounit_memstatus(My->mData_Output[i], msize, MADV_FREE);
      free(My->mData_Output[i]);
    }

  msize = sizeof(asysn_audiounit_InstanceState);
  asysn_audiounit_memstatus(My, msize, MADV_FREE);
  free(My);

  OSSpinLockLock(&asysn_audiounit_lock_opencount);

  if (!(--asysn_audiounit_opencount))
    asysn_audiounit_globalvars_memstatus(MADV_FREE);

  if (asysn_audiounit_opencount < 0)   /* should never be true */
    asysn_audiounit_opencount = 0;

  OSSpinLockUnlock(&asysn_audiounit_lock_opencount);

  return noErr; 
}

/************************/
/* Selector: Initialize */
/*                      */
/************************/

ComponentResult asysn_audiounit_kAudioUnitInitializeSelect(ComponentParameters * p, 
  					        asysn_audiounit_InstanceState * My)
{ 
  ENGINE_PTR_DECLARE_SEMICOLON

  switch (asysn_audiounit_initializeselect_enterstate(My)) {
    case ASYS_AUDIOUNIT_IRESPONSE_INITIALIZE:
      break;
    case ASYS_AUDIOUNIT_IRESPONSE_MY_IS_NULL:
    case ASYS_AUDIOUNIT_IRESPONSE_REINITIALIZE:
    default:
      return noErr;
    }

  ENGINE_PTR_ASSIGNED_TO  system_init(My->argc, My->argv, 
				      My->OutputStreamFormat.mSampleRate);
  
  MY_ENGINE_PTR_ASSIGNED_TO  ENGINE_PTR;
  
  if (ENGINE_PTR_IS_NULL)
    return kAudioUnitErr_FailedInitialization;
  
  ENGINE_MEMSTATUS_SEMICOLON(MADV_WILLNEED)
    
  effects_init(ENGINE_PTR);
  main_initpass(ENGINE_PTR);
  asysn_audiounit_ksyncinit(My);
  
  EV(kcycleidx) = EV(kbase);
  EV(acycleidx) = EV(ACYCLE) + 1;   /* +1 denotes zero-time condition */
  EV(pass) = IPASS;
  
  My->acycle = EV(ACYCLE);

  OSSpinLockLock(&(My->lock_CPULoad));

  My->krate = EV(KRATE);

  My->ksync_normalize = (Float32) (My->krate/AudioGetHostClockFrequency());
  if (My->CPULoad)
    My->ksync_normalize = My->ksync_normalize/My->CPULoad;

  OSSpinLockUnlock(&(My->lock_CPULoad));

  My->acycleidx_kcycleidx = 1;     /* acycleidx = 0, kcycleidx = 1 */

  if (ASYS_AUDIOUNIT_HAS_AUCONTROL)
    asysn_audiounit_engine_parameter_update(My);
  
  asysn_audiounit_initializeselect_exitstate(My);

  return noErr;
}

/**************************/
/* Selector: Uninitialize */
/*                        */
/**************************/

ComponentResult asysn_audiounit_kAudioUnitUninitializeSelect(ComponentParameters * p, 
						   asysn_audiounit_InstanceState * My)
{ 
  int j;
  ENGINE_PTR_DECLARE_SEMICOLON

  switch (asysn_audiounit_uninitializeselect_enterstate(My)) {
    case ASYS_AUDIOUNIT_IRESPONSE_UNINITIALIZE:
      break;
    case ASYS_AUDIOUNIT_IRESPONSE_MY_IS_NULL:
    case ASYS_AUDIOUNIT_IRESPONSE_REUNINITIALIZE:
    default:
      return noErr;
    }

  ENGINE_PTR_ASSIGNED_TO  MY_ENGINE_PTR;
  ENGINE_MEMSTATUS_SEMICOLON(MADV_FREE)
  shut_down(ENGINE_PTR);
  MY_ENGINE_PTR_ASSIGNED_TO  NULL;

  My->acycle = ((int)(SAOL_SRATE))/((int)(SAOL_KRATE));

  OSSpinLockLock(&(My->lock_CPULoad));

  My->krate = SAOL_KRATE;

  My->ksync_normalize = (Float32) (My->krate/AudioGetHostClockFrequency());
  if (My->CPULoad)
    My->ksync_normalize = My->ksync_normalize/My->CPULoad;

  OSSpinLockUnlock(&(My->lock_CPULoad));

  My->acycleidx_kcycleidx = 1;   /* acycleidx = 0, kcycleidx = 1 */

  asysn_audiounit_uninitializeselect_exitstate(My);
  asysn_audiounit_engine_emptypipes(My);

  return noErr; 
}

/********************************************/
/*        Selector: GetPropertyInfo         */
/*                                          */
/*  params[0]: Boolean * outWritable;       */
/*  params[1]: UInt32 * outDataSize;        */
/*  params[2]: AudioUnitElement inElement;  */
/*  params[3]: AudioUnitScope inScope;      */
/*  params[4]: AudioUnitPropertyID inID;    */
/*                                          */
/********************************************/

ComponentResult asysn_audiounit_kAudioUnitGetPropertyInfoSelect
(ComponentParameters * p, asysn_audiounit_InstanceState * My)

{ 
  /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
  /* return early for invalid properties */
  /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

  if (!asysn_audiounit_supported_property((AudioUnitPropertyID)(p->params[4]),
			  (AudioUnitScope)(p->params[3])))
    return kAudioUnitErr_InvalidProperty; 

  /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
  /* provide datasize if requested by a non-null pointer */
  /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

  if (p->params[1])
    *((UInt32 *)(p->params[1])) = asysn_audiounit_property_size(p->params[4], My);

  /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
  /* provide writable status if requested by a non-null pointer */
  /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

  if (p->params[0])
    {
      switch (p->params[4] /* property ID */) {
      case kAudioUnitProperty_ParameterList: /* 3 */
      case kAudioUnitProperty_ParameterInfo: /* 4 */ 
      case kAudioUnitProperty_FastDispatch:  /* 5 */
	*((Boolean *)(p->params[0])) = 0;   
	break;
      case kAudioUnitProperty_ElementCount: /* 11, also kAudioUnitProperty_BusCount */
	/* for hosts that require AUs to support side-chaining */
	*((Boolean *)(p->params[0])) = (p->params[3] == kAudioUnitScope_Input);   
	break;
      case kAudioUnitProperty_SupportedNumChannels: /* 13 */
      case kAudioUnitProperty_ParameterValueStrings: /* 16 */
      case kAudioUnitProperty_FactoryPresets: /* 24 */
      case kAudioUnitProperty_CocoaUI: /* 31 */
	*((Boolean *)(p->params[0])) = 0;   
	break;
      default:
	*((Boolean *)(p->params[0])) = 1;   
	break;
      }
    }

  return noErr;
}

/********************************************/
/*         Selector: GetProperty            */
/*                                          */
/*  params[0]: UInt32 * ioDataSize;         */
/*  params[1]: void * outData;              */
/*  params[2]: AudioUnitElement inElement;  */
/*  params[3]: AudioUnitScope inScope;      */
/*  params[4]: AudioUnitPropertyID inID;    */
/*                                          */
/********************************************/

ComponentResult asysn_audiounit_kAudioUnitGetPropertySelect(ComponentParameters * p, 
					       asysn_audiounit_InstanceState * My)
{ 
  ComponentResult result = noErr;
  CFBundleRef au_bundle = NULL;
  CFURLRef view_bundle_url = NULL;
  UInt32 size;

  /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
  /* return early for invalid properties */
  /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

  if (!asysn_audiounit_supported_property((AudioUnitPropertyID)(p->params[4]),
			  (AudioUnitScope)(p->params[3])))
    return kAudioUnitErr_InvalidProperty; 

  /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
  /* check size of property storage */
  /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

  if (*((UInt32 *)(p->params[0])) != 
      (size = asysn_audiounit_property_size(p->params[4], My)))
    if (*((UInt32 *)(p->params[0])) > size)
      *((UInt32 *)(p->params[0])) = size;
    else
      return kAudioUnitErr_InvalidPropertyValue;

  /*~~~~~~~~~~~~~~~~~~~~~~~*/
  /* assign property value */
  /*~~~~~~~~~~~~~~~~~~~~~~~*/

  switch (p->params[4] /* property ID */) {
  case kAudioUnitProperty_ClassInfo: /* 0 */   
    *((CFPropertyListRef *)((void *)(p->params[1]))) = 
      asysn_audiounit_classinfo_create(My);
    break;
  case kAudioUnitProperty_MakeConnection: /* 1 */
    if ((p->params[2] >= ASYS_AUDIOUNIT_ELEMENT_ARRAYSIZE) ||
	(p->params[2] < 0))
      {
	result = kAudioUnitErr_InvalidProperty; 
	break;
      }
    OSSpinLockLock(&(My->lock_sampledelivery));
    ((AudioUnitConnection *)((void *)(p->params[1])))->sourceAudioUnit
      = My->MakeConnection[p->params[2]].sourceAudioUnit;
    ((AudioUnitConnection *)((void *)(p->params[1])))->sourceOutputNumber
      = My->MakeConnection[p->params[2]].sourceOutputNumber;
    ((AudioUnitConnection *)((void *)(p->params[1])))->destInputNumber
      = My->MakeConnection[p->params[2]].destInputNumber;
    OSSpinLockUnlock(&(My->lock_sampledelivery));
    break;
  case kAudioUnitProperty_SampleRate: /* 2 */
    switch (p->params[3] /* inScope */ ) {
    case kAudioUnitScope_Input:
    if (!(ASYS_AUDIOUNIT_EFFECT) || 
	(p->params[2] >= ASYS_AUDIOUNIT_ELEMENT_ARRAYSIZE) ||
	(p->params[2] < 0))
      {
	result = kAudioUnitErr_InvalidProperty; 
	break;
      }
    *((Float64 *)((void *)(p->params[1]))) = 
      My->InputStreamFormat[p->params[2]].mSampleRate;
    break;
    case kAudioUnitScope_Global:
    case kAudioUnitScope_Output:
      *((Float64 *)((void *)(p->params[1]))) = My->OutputStreamFormat.mSampleRate;
      break;
    default:
      result = kAudioUnitErr_InvalidProperty; 
      break;
    }
    break;
  case kAudioUnitProperty_ParameterList: /* 3 */
    memcpy(((AudioUnitParameterID *)((void *)(p->params[1]))), My->parameterlist,
	   sizeof(AudioUnitParameterID)*My->num_saolparams);
    break;
  case kAudioUnitProperty_ParameterInfo: /* 4 */
    if (p->params[2] < My->num_saolparams)  /* p->param[2] requests ParameterID. */
      {
	if (My->parameterinfo[p->params[2]].cfNameString)
	  CFRetain(My->parameterinfo[p->params[2]].cfNameString);
	if (My->parameterinfo[p->params[2]].unitName)
	  CFRetain(My->parameterinfo[p->params[2]].unitName);
	memcpy(((AudioUnitParameterInfo *)((void *)(p->params[1]))), 
	       &(My->parameterinfo[p->params[2]]), sizeof(AudioUnitParameterInfo));
      }
    else
      result = kAudioUnitErr_InvalidProperty; 
    break;
  case kAudioUnitProperty_FastDispatch:  /* 5 */
    switch (p->params[2] /* inElement */ )  {
    case kAudioUnitRenderSelect:
      *((AudioUnitRenderProc *)(p->params[1])) = asysn_audiounit_MyRenderer;
      break;
    case kMusicDeviceMIDIEventSelect:
      if (ASYS_AUDIOUNIT_MIDISUPPORT)
	*((MusicDeviceMIDIEventProc *)(p->params[1])) = asysn_audiounit_MyMIDIEventProc;
      else
	result = kAudioUnitErr_InvalidProperty; 
      break;
    case kAudioUnitGetParameterSelect:
      *((AudioUnitGetParameterProc *)(p->params[1])) = 
	asysn_audiounit_MyGetParameterProc;
      break;
    case kAudioUnitSetParameterSelect:
      *((AudioUnitSetParameterProc *)(p->params[1])) = 
	asysn_audiounit_MySetParameterProc;
      break;
    default:
      result = kAudioUnitErr_InvalidProperty; 
      break;
    }
    break;
  case kAudioUnitProperty_CPULoad: /* 6 */ 
    *((Float32 *)((void *)(p->params[1]))) = My->CPULoad;
    break;
  case kAudioUnitProperty_StreamFormat: /* 8 */ 
    switch (p->params[3] /* inScope */ ) {
    case kAudioUnitScope_Input:
    if (!(ASYS_AUDIOUNIT_EFFECT) || 
	(p->params[2] >= ASYS_AUDIOUNIT_ELEMENT_ARRAYSIZE) ||
	(p->params[2] < 0))
      {
	result = kAudioUnitErr_InvalidProperty; 
	break;
      }
    ((AudioStreamBasicDescription *)((void *)(p->params[1])))->mChannelsPerFrame
      = My->InputStreamFormat[p->params[2]].mChannelsPerFrame;
    ((AudioStreamBasicDescription *)((void *)(p->params[1])))->mSampleRate
      = My->InputStreamFormat[p->params[2]].mSampleRate;
    ((AudioStreamBasicDescription *)((void *)(p->params[1])))->mFormatID
      = My->InputStreamFormat[p->params[2]].mFormatID;
    ((AudioStreamBasicDescription *)((void *)(p->params[1])))->mFormatFlags
      = My->InputStreamFormat[p->params[2]].mFormatFlags;
    ((AudioStreamBasicDescription *)((void *)(p->params[1])))->mBytesPerPacket
      = My->InputStreamFormat[p->params[2]].mBytesPerPacket;
    ((AudioStreamBasicDescription *)((void *)(p->params[1])))->mBytesPerFrame
      = My->InputStreamFormat[p->params[2]].mBytesPerFrame;
    ((AudioStreamBasicDescription *)((void *)(p->params[1])))->mFramesPerPacket
      = My->InputStreamFormat[p->params[2]].mFramesPerPacket;
    ((AudioStreamBasicDescription *)((void *)(p->params[1])))->mBitsPerChannel
      = My->InputStreamFormat[p->params[2]].mBitsPerChannel;
    ((AudioStreamBasicDescription *)((void *)(p->params[1])))->mReserved
      = My->InputStreamFormat[p->params[2]].mReserved;
      break;
    case kAudioUnitScope_Global:
    case kAudioUnitScope_Output:
      ((AudioStreamBasicDescription *)((void *)(p->params[1])))->mChannelsPerFrame
	= My->OutputStreamFormat.mChannelsPerFrame;
      ((AudioStreamBasicDescription *)((void *)(p->params[1])))->mSampleRate
	= My->OutputStreamFormat.mSampleRate;
      ((AudioStreamBasicDescription *)((void *)(p->params[1])))->mFormatID
	= My->OutputStreamFormat.mFormatID;
      ((AudioStreamBasicDescription *)((void *)(p->params[1])))->mFormatFlags
	= My->OutputStreamFormat.mFormatFlags;
      ((AudioStreamBasicDescription *)((void *)(p->params[1])))->mBytesPerPacket
	= My->OutputStreamFormat.mBytesPerPacket;
      ((AudioStreamBasicDescription *)((void *)(p->params[1])))->mBytesPerFrame
	= My->OutputStreamFormat.mBytesPerFrame;
      ((AudioStreamBasicDescription *)((void *)(p->params[1])))->mFramesPerPacket
	= My->OutputStreamFormat.mFramesPerPacket;
      ((AudioStreamBasicDescription *)((void *)(p->params[1])))->mBitsPerChannel
	= My->OutputStreamFormat.mBitsPerChannel;
      ((AudioStreamBasicDescription *)((void *)(p->params[1])))->mReserved
	= My->OutputStreamFormat.mReserved;
      break;
    default:
      result = kAudioUnitErr_InvalidProperty; 
      break;
    }
    break;
  case kAudioUnitProperty_SRCAlgorithm: /* 9 */
    *((OSType *)((void *)(p->params[1]))) = My->SRCAlgorithm;
    break;
  case kAudioUnitProperty_ElementCount: /* 11, also kAudioUnitProperty_BusCount */
    switch (p->params[3] /* inScope */ ) {
    case kAudioUnitScope_Input:
      *((UInt32 *)((void *)(p->params[1]))) = My->InputElementCount;
      break;
    case kAudioUnitScope_Global:
    case kAudioUnitScope_Output:
      *((UInt32 *)((void *)(p->params[1]))) = My->OutputElementCount;
      break;
    case kAudioUnitScope_Part:
    case kAudioUnitScope_Group:
      *((UInt32 *)((void *)(p->params[1]))) = 0;
      break;
    default:
      result = kAudioUnitErr_InvalidProperty; 
      break;
    }
    break;
  case kAudioUnitProperty_Latency: /* 12 */
    *((Float64 *)((void *)(p->params[1]))) = My->Latency;
    break;
  case kAudioUnitProperty_SupportedNumChannels: /* 13 */
    memcpy(((AudioUnitParameterID *)((void *)(p->params[1]))), My->SupportedNumChannels,
	   ASYS_AUDIOUNIT_SUPPORTED_FORMATS*sizeof(AUChannelInfo));
    break;
  case kAudioUnitProperty_MaximumFramesPerSlice: /* 14 */
    *((UInt32 *)((void *)(p->params[1]))) = My->MaximumFramesPerSlice;
    break;
  case kAudioUnitProperty_ParameterValueStrings: /* 16 */
    if ((*((CFMutableArrayRef *)((void *)(p->params[1]))) = 
	 asysn_audiounit_parametervaluestrings_create(My, p->params[2])) == NULL)
      result = kAudioUnitErr_InvalidProperty; 
    break;
  case kAudioUnitProperty_TailTime: /* 20 */
    *((Float64 *)((void *)(p->params[1]))) = My->TailTime;
    break;
  case kAudioUnitProperty_BypassEffect: /* 21 */
    *((UInt32 *)((void *)(p->params[1]))) = My->BypassEffect;
    break;
  case kAudioUnitProperty_LastRenderError: /* 22 */
    *((OSStatus *)((void *)(p->params[1]))) = My->LastRenderError;
    My->LastRenderError = noErr;
    break;
  case kAudioUnitProperty_SetRenderCallback:  /* 23 */
    if ((p->params[2] >= ASYS_AUDIOUNIT_ELEMENT_ARRAYSIZE) ||
	(p->params[2] < 0))
      {
	result = kAudioUnitErr_InvalidProperty; 
	break;
      }
    OSSpinLockLock(&(My->lock_sampledelivery));
    ((AURenderCallbackStruct *)((void *)(p->params[1])))->inputProc
      = My->SetRenderCallback[p->params[2]].inputProc;
    ((AURenderCallbackStruct *)((void *)(p->params[1])))->inputProcRefCon
      = My->SetRenderCallback[p->params[2]].inputProcRefCon;
    OSSpinLockUnlock(&(My->lock_sampledelivery));
    break;
  case kAudioUnitProperty_FactoryPresets: /* 24 */
    if (My->num_factory)
      {
	CFRetain(My->factorypreset_array);
	*((CFPropertyListRef *)((void *)(p->params[1]))) = My->factorypreset_array;
      }
    else
      result = kAudioUnitErr_InvalidProperty; 
    break;
  case kAudioUnitProperty_RenderQuality: /* 26 */
    *((UInt32 *)((void *)(p->params[1]))) = My->RenderQuality;
    break;
  case kAudioUnitProperty_InPlaceProcessing: /* 29 */
    *((UInt32 *)((void *)(p->params[1]))) = My->InPlaceProcessing;
    break;
  case kAudioUnitProperty_CocoaUI: /* 31 */
#if defined(ASYS_AUDIOUNIT_VIEW_BUNDLECF)

    if ((au_bundle = CFBundleGetBundleWithIdentifier(ASYS_AUDIOUNIT_AU_BUNDLECF)))
      CFRetain(au_bundle);
    else
      {
	result = kAudioUnitErr_InvalidProperty; 
	break;
      }

    view_bundle_url = CFBundleCopyResourceURL(au_bundle,
					      ASYS_AUDIOUNIT_VIEW_BUNDLECF,
					      CFSTR("bundle"), NULL);
    CFRelease(au_bundle);

    if (!view_bundle_url)
      {
	result = kAudioUnitErr_InvalidProperty; 
	break;
      }

    ((AudioUnitCocoaViewInfo *)(p->params[1]))->mCocoaAUViewBundleLocation =
      view_bundle_url;

    ((AudioUnitCocoaViewInfo *)(p->params[1]))->mCocoaAUViewClass[0] =
      CFStringCreateCopy(NULL, ASYS_AUDIOUNIT_VIEW_BASECLASSCF);
#else
    result = kAudioUnitErr_InvalidProperty; 
#endif
    break;
  case kAudioUnitProperty_PresentPreset: /* 36 */
    OSSpinLockLock(&(My->lock_PresentPreset));
    ((AUPreset *)(p->params[1]))->presetNumber = My->PresentPreset.presetNumber;
    ((AUPreset *)(p->params[1]))->presetName = My->PresentPreset.presetName;
    CFRetain(My->PresentPreset.presetName);
    OSSpinLockUnlock(&(My->lock_PresentPreset));
    break;
  case kAudioUnitProperty_OfflineRender: /* 37 */
    *((UInt32 *)((void *)(p->params[1]))) = My->OfflineRender;
    break;
  case kAudioUnitProperty_PresentationLatency: /* 40 */
    *((Float64 *)((void *)(p->params[1]))) = My->PresentationLatency;
    break;
  case kMusicDeviceProperty_InstrumentCount: /* 1000 */
    if (ASYS_AUDIOUNIT_MUSICDEVICE)
      *((UInt32 *)((void *)(p->params[1]))) = My->InstrumentCount;
    else
      result = kAudioUnitErr_InvalidProperty; 
    break;
  case kMusicDeviceProperty_StreamFromDisk: /* 1011 */
    *((UInt32 *)((void *)(p->params[1]))) = My->StreamFromDisk;
    break;
  default:  /* should never happen */
    result = kAudioUnitErr_InvalidProperty; 
    break;
  }

  return result;
}

/********************************************/
/*         Selector: SetProperty            */
/*                                          */
/*  params[0]: UInt32 inDataSize;           */
/*  params[1]: const void * inData;         */
/*  params[2]: AudioUnitElement inElement;  */
/*  params[3]: AudioUnitScope inScope;      */
/*  params[4]: AudioUnitPropertyID inID;    */
/*                                          */
/********************************************/

ComponentResult asysn_audiounit_kAudioUnitSetPropertySelect(ComponentParameters * p, 
					       asysn_audiounit_InstanceState * My)
{ 
  ComponentResult result = noErr; 
  CFStringRef key;

  /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
  /* return early for invalid properties */
  /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

  if (!asysn_audiounit_supported_property((AudioUnitPropertyID)(p->params[4]),
			  (AudioUnitScope)(p->params[3])))
    return kAudioUnitErr_InvalidProperty; 

  switch (p->params[4] /* property ID */) {
  case kAudioUnitProperty_ClassInfo: /* 0 */  
    asysn_audiounit_classinfo_read(
                        My, *((CFMutableDictionaryRef *)((void *)(p->params[1])))); 
    asysn_audiounit_proplisteners_update(My, kAudioUnitProperty_PresentPreset,
					 kAudioUnitScope_Global, (AudioUnitElement) 0);
    asysn_audiounit_proplisteners_update(My, kAudioUnitProperty_RenderQuality,
					 kAudioUnitScope_Global, (AudioUnitElement) 0);
    asysn_audiounit_proplisteners_update(My, kAudioUnitProperty_CPULoad,
					 kAudioUnitScope_Global, (AudioUnitElement) 0);
    break;
  case kAudioUnitProperty_MakeConnection: /* 1 */
    if ((p->params[2] >= ASYS_AUDIOUNIT_ELEMENT_ARRAYSIZE) ||
	(p->params[2] < 0))
      {
	result = kAudioUnitErr_InvalidProperty; 
	break;
      }
    OSSpinLockLock(&(My->lock_sampledelivery));
    My->MakeConnection[p->params[2]].sourceAudioUnit = 
      ((AudioUnitConnection *)((void *)(p->params[1])))->sourceAudioUnit;
    My->MakeConnection[p->params[2]].sourceOutputNumber = 
      ((AudioUnitConnection *)((void *)(p->params[1])))->sourceOutputNumber;
    My->MakeConnection[p->params[2]].destInputNumber = 
      ((AudioUnitConnection *)((void *)(p->params[1])))->destInputNumber;
    OSSpinLockUnlock(&(My->lock_sampledelivery));
    break;
  case kAudioUnitProperty_SampleRate: /* 2 */
    switch (p->params[3] /* inScope */ ) {
    case kAudioUnitScope_Input:
      if (!(ASYS_AUDIOUNIT_EFFECT) || 
	  (p->params[2] >= ASYS_AUDIOUNIT_ELEMENT_ARRAYSIZE) ||
	  (p->params[2] < 0))
	{
	  result = kAudioUnitErr_InvalidProperty; 
	  break;
	}
	My->InputStreamFormat[p->params[2]].mSampleRate = 
	  *((Float64 *)((void *)(p->params[1]))); 
      break;
    case kAudioUnitScope_Output:
      My->OutputStreamFormat.mSampleRate = *((Float64 *)((void *)(p->params[1])));
      break;
    case kAudioUnitScope_Global:
      if ((ASYS_AUDIOUNIT_EFFECT) && 
	  (p->params[2] < ASYS_AUDIOUNIT_ELEMENT_ARRAYSIZE) ||
	  (p->params[2] >= 0))
	My->InputStreamFormat[p->params[2]].mSampleRate =
	  *((Float64 *)((void *)(p->params[1]))); 
      My->OutputStreamFormat.mSampleRate = *((Float64 *)((void *)(p->params[1])));
      break;
    default:
      result = kAudioUnitErr_InvalidProperty; 
      break;
    }
    break;
  case kAudioUnitProperty_ParameterList: /* 3 */
    result = kAudioUnitErr_InvalidProperty; 
    break;    /* hosts not permitted to change Parameter List */
  case kAudioUnitProperty_ParameterInfo: /* 4 */ 
    result = kAudioUnitErr_InvalidProperty; 
    break;    /* hosts not permitted to change Parameter Info */
  case kAudioUnitProperty_FastDispatch:  /* 5 */
    result = kAudioUnitErr_InvalidProperty; 
    break;    /* hosts not permitted to set the dispatch routines */
  case kAudioUnitProperty_CPULoad: /* 6 */

    OSSpinLockLock(&(My->lock_CPULoad));

    My->CPULoad = *((Float32 *)((void *)(p->params[1])));
    My->ksync_normalize = (Float32) (My->krate/AudioGetHostClockFrequency());
    if (My->CPULoad)
      My->ksync_normalize = My->ksync_normalize/My->CPULoad;

    OSSpinLockUnlock(&(My->lock_CPULoad));

    asysn_audiounit_proplisteners_update(My, kAudioUnitProperty_CPULoad,
					 kAudioUnitScope_Global, (AudioUnitElement) 0);
    break;
  case kAudioUnitProperty_StreamFormat: /* 8 */ 

    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
    /* reject non-native endian-ness */
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
    
    if (ASYS_AUDIOUNIT_FLOAT32_BIGENDIAN != 
	(((((AudioStreamBasicDescription *)((void *)(p->params[1])))->mFormatFlags)
	  & kAudioFormatFlagIsBigEndian) != 0))
      {
	result = kAudioUnitErr_InvalidPropertyValue; 
	break;
      }

    switch (p->params[3] /* inScope */ ) {
    case kAudioUnitScope_Input:

      if (!(ASYS_AUDIOUNIT_EFFECT) || 
	  (p->params[2] >= ASYS_AUDIOUNIT_ELEMENT_ARRAYSIZE) ||
	  (p->params[2] < 0))
	{
	  result = kAudioUnitErr_InvalidProperty; 
	  break;
	}

      /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
      /* if the AU uses an input, accept if it doesn't exceed channel limit */
      /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

      if (((AudioStreamBasicDescription *)
	   ((void *)(p->params[1])))->mChannelsPerFrame &&
	  (((AudioStreamBasicDescription *)
	    ((void *)(p->params[1])))->mChannelsPerFrame
	   <= ASYS_AUDIOUNIT_INPUT_MAXCHANNELS))
	{
	  if (p->params[2] == 0)
	    My->AudioBufferTemplate->mNumberBuffers = 
	      ((AudioStreamBasicDescription *)
	       ((void *)(p->params[1])))->mChannelsPerFrame;
	  
	  My->InputStreamFormat[p->params[2]].mChannelsPerFrame = 
	    ((AudioStreamBasicDescription *)
	     ((void *)(p->params[1])))->mChannelsPerFrame;
	  My->InputStreamFormat[p->params[2]].mSampleRate = 
	    ((AudioStreamBasicDescription *)
	     ((void *)(p->params[1])))->mSampleRate;
	  My->InputStreamFormat[p->params[2]].mFormatID = 
	    ((AudioStreamBasicDescription *)
	     ((void *)(p->params[1])))->mFormatID;
	  My->InputStreamFormat[p->params[2]].mFormatFlags = 
	    ((AudioStreamBasicDescription *)
	     ((void *)(p->params[1])))->mFormatFlags;
	  My->InputStreamFormat[p->params[2]].mBytesPerPacket = 
	    ((AudioStreamBasicDescription *)
	     ((void *)(p->params[1])))->mBytesPerPacket;
	  My->InputStreamFormat[p->params[2]].mBytesPerFrame = 
	    ((AudioStreamBasicDescription *)
	     ((void *)(p->params[1])))->mBytesPerFrame;
	  My->InputStreamFormat[p->params[2]].mFramesPerPacket = 
	    ((AudioStreamBasicDescription *)
	     ((void *)(p->params[1])))->mFramesPerPacket;
	  My->InputStreamFormat[p->params[2]].mBitsPerChannel = 
	    ((AudioStreamBasicDescription *)
	     ((void *)(p->params[1])))->mBitsPerChannel;
	  My->InputStreamFormat[p->params[2]].mReserved = 
	    ((AudioStreamBasicDescription *)
	     ((void *)(p->params[1])))->mReserved;
	}
      else
	result = kAudioUnitErr_InvalidProperty; 
      break;
    case kAudioUnitScope_Output:

      /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
      /* if number of output channels meet our limits, accept */
      /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

      if (((AudioStreamBasicDescription *)((void *)(p->params[1])))->mChannelsPerFrame  
	  && 
	  (((AudioStreamBasicDescription *)((void *)(p->params[1])))->mChannelsPerFrame 
	   <= ASYS_AUDIOUNIT_OUTPUT_MAXCHANNELS))
	{
	  My->OutputStreamFormat.mChannelsPerFrame = 
	    ((AudioStreamBasicDescription *)
	     ((void *)(p->params[1])))->mChannelsPerFrame;
	  My->OutputStreamFormat.mSampleRate = 
	    ((AudioStreamBasicDescription *)
	     ((void *)(p->params[1])))->mSampleRate;
	  My->OutputStreamFormat.mFormatID = 
	    ((AudioStreamBasicDescription *)
	     ((void *)(p->params[1])))->mFormatID;
	  My->OutputStreamFormat.mFormatFlags = 
	    ((AudioStreamBasicDescription *)
	     ((void *)(p->params[1])))->mFormatFlags;
	  My->OutputStreamFormat.mBytesPerPacket = 
	    ((AudioStreamBasicDescription *)
	     ((void *)(p->params[1])))->mBytesPerPacket;
	  My->OutputStreamFormat.mBytesPerFrame = 
	    ((AudioStreamBasicDescription *)
	     ((void *)(p->params[1])))->mBytesPerFrame;
	  My->OutputStreamFormat.mFramesPerPacket = 
	    ((AudioStreamBasicDescription *)
	     ((void *)(p->params[1])))->mFramesPerPacket;
	  My->OutputStreamFormat.mBitsPerChannel = 
	    ((AudioStreamBasicDescription *)
	     ((void *)(p->params[1])))->mBitsPerChannel;
	  My->OutputStreamFormat.mReserved = 
	    ((AudioStreamBasicDescription *)
	     ((void *)(p->params[1])))->mReserved;
	}
      else
	result = kAudioUnitErr_InvalidPropertyValue; 
      break;
    case kAudioUnitScope_Global:

      /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
      /* if number of output channels meet our limits, accept */
      /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

      if (((AudioStreamBasicDescription *)((void *)(p->params[1])))->mChannelsPerFrame  
	  && 
	  (((AudioStreamBasicDescription *)((void *)(p->params[1])))->mChannelsPerFrame 
	   <= ASYS_AUDIOUNIT_OUTPUT_MAXCHANNELS))
	{
	  My->OutputStreamFormat.mChannelsPerFrame = 
	    ((AudioStreamBasicDescription *)
	     ((void *)(p->params[1])))->mChannelsPerFrame;
	  My->OutputStreamFormat.mSampleRate = 
	    ((AudioStreamBasicDescription *)
	     ((void *)(p->params[1])))->mSampleRate;
	  My->OutputStreamFormat.mFormatID = 
	    ((AudioStreamBasicDescription *)
	     ((void *)(p->params[1])))->mFormatID;
	  My->OutputStreamFormat.mFormatFlags = 
	    ((AudioStreamBasicDescription *)
	     ((void *)(p->params[1])))->mFormatFlags;
	  My->OutputStreamFormat.mBytesPerPacket = 
	    ((AudioStreamBasicDescription *)
	     ((void *)(p->params[1])))->mBytesPerPacket;
	  My->OutputStreamFormat.mBytesPerFrame = 
	    ((AudioStreamBasicDescription *)
	     ((void *)(p->params[1])))->mBytesPerFrame;
	  My->OutputStreamFormat.mFramesPerPacket = 
	    ((AudioStreamBasicDescription *)
	     ((void *)(p->params[1])))->mFramesPerPacket;
	  My->OutputStreamFormat.mBitsPerChannel = 
	    ((AudioStreamBasicDescription *)
	     ((void *)(p->params[1])))->mBitsPerChannel;
	  My->OutputStreamFormat.mReserved = 
	    ((AudioStreamBasicDescription *)
	     ((void *)(p->params[1])))->mReserved;
	}
      else
	{
	  result = kAudioUnitErr_InvalidPropertyValue; 
	  break;
	}

      if ((ASYS_AUDIOUNIT_EFFECT) && 
	  (p->params[2] < ASYS_AUDIOUNIT_ELEMENT_ARRAYSIZE) ||
	  (p->params[2] >= 0))
	{

	  /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	  /* if the AU uses an input, accept if it doesn't exceed channel limit */
	  /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	  
	  if (((AudioStreamBasicDescription *)
	       ((void *)(p->params[1])))->mChannelsPerFrame &&
	      (((AudioStreamBasicDescription *)
		((void *)(p->params[1])))->mChannelsPerFrame
	       <= ASYS_AUDIOUNIT_INPUT_MAXCHANNELS)) 
	    {
	      if (p->params[2] == 0)
		My->AudioBufferTemplate->mNumberBuffers = 
		  ((AudioStreamBasicDescription *)
		   ((void *)(p->params[1])))->mChannelsPerFrame;
	      
	      My->InputStreamFormat[p->params[2]].mChannelsPerFrame = 
		((AudioStreamBasicDescription *)
		 ((void *)(p->params[1])))->mChannelsPerFrame;
	      My->InputStreamFormat[p->params[2]].mSampleRate = 
		((AudioStreamBasicDescription *)
		 ((void *)(p->params[1])))->mSampleRate;
	      My->InputStreamFormat[p->params[2]].mFormatID = 
		((AudioStreamBasicDescription *)
		 ((void *)(p->params[1])))->mFormatID;
	      My->InputStreamFormat[p->params[2]].mFormatFlags = 
		((AudioStreamBasicDescription *)
		 ((void *)(p->params[1])))->mFormatFlags;
	      My->InputStreamFormat[p->params[2]].mBytesPerPacket = 
		((AudioStreamBasicDescription *)
		 ((void *)(p->params[1])))->mBytesPerPacket;
	      My->InputStreamFormat[p->params[2]].mBytesPerFrame = 
		((AudioStreamBasicDescription *)
		 ((void *)(p->params[1])))->mBytesPerFrame;
	      My->InputStreamFormat[p->params[2]].mFramesPerPacket = 
		((AudioStreamBasicDescription *)
		 ((void *)(p->params[1])))->mFramesPerPacket;
	      My->InputStreamFormat[p->params[2]].mBitsPerChannel = 
		((AudioStreamBasicDescription *)
		 ((void *)(p->params[1])))->mBitsPerChannel;
	      My->InputStreamFormat[p->params[2]].mReserved = 
		((AudioStreamBasicDescription *)
		 ((void *)(p->params[1])))->mReserved;
	    }
	  else
	    result = kAudioUnitErr_InvalidProperty; 
	}
      break;
    default:
      result = kAudioUnitErr_InvalidProperty; 
      break;
    }
    break;
  case kAudioUnitProperty_SRCAlgorithm: /* 9 */
    My->SRCAlgorithm = *((OSType *)((void *)(p->params[1])));
    break;
  case kAudioUnitProperty_ElementCount: /* 11, also kAudioUnitProperty_BusCount */
    switch (p->params[3] /* inScope */ ) {
    case kAudioUnitScope_Input:
      if (*((UInt32 *)((void *)(p->params[1]))) <= ASYS_AUDIOUNIT_ELEMENT_INPUTMAX)
	My->InputElementCount = *((UInt32 *)((void *)(p->params[1])));
      else
	result = kAudioUnitErr_InvalidPropertyValue; 
      break;
    case kAudioUnitScope_Output:
      if (*((UInt32 *)((void *)(p->params[1]))) <= ASYS_AUDIOUNIT_ELEMENT_OUTPUTMAX)
	My->OutputElementCount = *((UInt32 *)((void *)(p->params[1])));
      else
	result = kAudioUnitErr_InvalidPropertyValue; 
      break;
    case kAudioUnitScope_Global:
      if (*((UInt32 *)((void *)(p->params[1]))) <= ASYS_AUDIOUNIT_ELEMENT_OUTPUTMAX)
	My->OutputElementCount = *((UInt32 *)((void *)(p->params[1])));
      else
	result = kAudioUnitErr_InvalidPropertyValue; 
      if (ASYS_AUDIOUNIT_EFFECT)
	{
	  if (*((UInt32 *)((void *)(p->params[1]))) <= ASYS_AUDIOUNIT_ELEMENT_INPUTMAX)
	    My->InputElementCount = *((UInt32 *)((void *)(p->params[1])));
	  else
	    result = kAudioUnitErr_InvalidPropertyValue;
	}
      break;
    case kAudioUnitScope_Part:
    case kAudioUnitScope_Group:
      break;
    default:
      result = kAudioUnitErr_InvalidProperty; 
      break;
    }
    break;
  case kAudioUnitProperty_Latency: /* 12 */
    My->Latency = *((Float64 *)((void *)(p->params[1])));
    asysn_audiounit_proplisteners_update(My, kAudioUnitProperty_Latency,
					 kAudioUnitScope_Global, (AudioUnitElement) 0);
    break;
  case kAudioUnitProperty_SupportedNumChannels: /* 13 */
    result = kAudioUnitErr_InvalidPropertyValue;
    break;
  case kAudioUnitProperty_MaximumFramesPerSlice: /* 14 */
    if (*((UInt32 *)((void *)(p->params[1]))) > ASYS_AUDIOUNIT_FRAMES_PER_SLICE)
      result = kAudioUnitErr_InvalidPropertyValue;
    else
      {
	My->MaximumFramesPerSlice = *((UInt32 *)((void *)(p->params[1])));
	asysn_audiounit_proplisteners_update(My, kAudioUnitProperty_MaximumFramesPerSlice,
					     kAudioUnitScope_Global, (AudioUnitElement) 0);
      }
    break;
  case kAudioUnitProperty_ParameterValueStrings: /* 16 */
    result = kAudioUnitErr_InvalidProperty; 
    break;    /* hosts not permitted to set ParameterValueStrings */
  case kAudioUnitProperty_TailTime: /* 20 */
    My->TailTime = *((Float64 *)((void *)(p->params[1])));
    asysn_audiounit_proplisteners_update(My, kAudioUnitProperty_TailTime,
					 kAudioUnitScope_Global, (AudioUnitElement) 0);
    break;
  case kAudioUnitProperty_BypassEffect: /* 21 */
    My->BypassEffect = *((UInt32 *)((void *)(p->params[1])));
    break;
  case kAudioUnitProperty_LastRenderError: /* 22 */
    My->LastRenderError = *((OSStatus *)((void *)(p->params[1])));
    break;
  case kAudioUnitProperty_SetRenderCallback:  /* 23 */
    if ((p->params[2] >= ASYS_AUDIOUNIT_ELEMENT_ARRAYSIZE) ||
	(p->params[2] < 0))
      {
	result = kAudioUnitErr_InvalidProperty; 
	break;
      }
    OSSpinLockLock(&(My->lock_sampledelivery));
    memcpy(&(My->SetRenderCallback[p->params[2]]), 
	   (AURenderCallbackStruct *)(p->params[1]),
	   sizeof(AURenderCallbackStruct));
    OSSpinLockUnlock(&(My->lock_sampledelivery));
    break;
  case kAudioUnitProperty_FactoryPresets: /* 24 */
    result = kAudioUnitErr_InvalidProperty; 
    break;    /* hosts not permitted to change Factory Presets */
  case kAudioUnitProperty_RenderQuality: /* 26 */
    My->RenderQuality = *((UInt32 *)((void *)(p->params[1])));
    asysn_audiounit_proplisteners_update(My, kAudioUnitProperty_RenderQuality,
					 kAudioUnitScope_Global, (AudioUnitElement) 0);
    break;
  case kAudioUnitProperty_InPlaceProcessing: /* 29 */
    if (*((UInt32 *)((void *)(p->params[1]))) != ASYS_AUDIOUNIT_INPLACE_PROCESSING)
      result = kAudioUnitErr_InvalidPropertyValue;
    break;
  case kAudioUnitProperty_PresentPreset: /* 36 */
    OSSpinLockLock(&(My->lock_PresentPreset));
    CFRelease(My->PresentPreset.presetName);
    My->PresentPreset.presetNumber = ((AUPreset *)(p->params[1]))->presetNumber;
    My->PresentPreset.presetName = ((AUPreset *)(p->params[1]))->presetName;
    CFRetain(My->PresentPreset.presetName);
    OSSpinLockUnlock(&(My->lock_PresentPreset));
    asysn_audiounit_presentproperty_setfactory(My);
    asysn_audiounit_proplisteners_update(My, kAudioUnitProperty_PresentPreset,
					 kAudioUnitScope_Global, (AudioUnitElement) 0);
    break;
  case kAudioUnitProperty_OfflineRender: /* 37 */
    My->OfflineRender = *((UInt32 *)((void *)(p->params[1]))); 
    break;
  case kAudioUnitProperty_PresentationLatency: /* 40 */
    My->PresentationLatency = *((Float64 *)((void *)(p->params[1]))); 
    break;
  case kMusicDeviceProperty_InstrumentCount: /* 1000 */
    if (ASYS_AUDIOUNIT_EFFECT)
      result = kAudioUnitErr_InvalidPropertyValue;
    else
      if (*((UInt32 *)((void *)(p->params[1]))) != ASYS_AUDIOUNIT_INSTRUMENT_COUNT)
	result = kAudioUnitErr_InvalidPropertyValue;
    break;
  case kMusicDeviceProperty_StreamFromDisk: /* 1011 */
    My->StreamFromDisk = *((UInt32 *)((void *)(p->params[1]))); 
    asysn_audiounit_proplisteners_update(My, kMusicDeviceProperty_StreamFromDisk,
					 kAudioUnitScope_Global, (AudioUnitElement) 0);
    break;
  default:   /* should never happen */
    result = kAudioUnitErr_InvalidProperty; 
    break;
  }

  return result;
}

/**********************************************************/
/*                 Selector: Render                       */
/*                                                        */
/* params[0]: AudioBufferList * ioData;                   */
/* params[1]: UInt32 inNumberFrames;                      */
/* params[2]: UInt32 inOutputBusNumber;                   */
/* params[3]: const AudioTimeStamp * inTimeStamp;         */
/* params[4]: AudioUnitRenderActionFlags * ioActionFlags; */
/*                                                        */
/**********************************************************/

ComponentResult asysn_audiounit_kAudioUnitRenderSelect(ComponentParameters * p, 
					  asysn_audiounit_InstanceState * My)
{ 
  return (asysn_audiounit_MyRenderer((void *) My, 
				     (AudioUnitRenderActionFlags *)(p->params[4]),
				     (const AudioTimeStamp *)(p->params[3]),
				     (UInt32)(p->params[2]), (UInt32)(p->params[1]),
				     (AudioBufferList *)(p->params[0])));
}

/*******************/
/* Selector: Reset */
/*                 */
/*******************/

ComponentResult asysn_audiounit_kAudioUnitResetSelect(ComponentParameters * p, 
					 asysn_audiounit_InstanceState * My)
{ 

#if (ASYS_AUDIOUNIT_RESET_TYPE == ASYS_AUDIOUNIT_ACTIVE_RESET)
  asysn_audiounit_kAudioUnitUninitializeSelect(p, My);
  asysn_audiounit_kAudioUnitInitializeSelect(p, My);
#endif

  return noErr; 
}

/*****************************************************/
/* Selector: Add Property Listener                   */
/*                                                   */
/*  params[0]: void * inProcRefCon;                  */
/*  params[1]: AudioUnitPropertyListenerProc inProc; */
/*  params[2]: AudioUnitPropertyID inID;             */
/*                                                   */
/* The caller passes in a function inProc to call    */
/* the AU changes the property, and an object        */ 
/* inProcRefCon to pass to the function.             */
/*****************************************************/

ComponentResult asysn_audiounit_kAudioUnitAddPropertyListenerSelect(
                                           ComponentParameters * p, 
					   asysn_audiounit_InstanceState * My)
{
  asysn_audiounit_proplisten * lp;
  AudioUnitPropertyListenerProc lproc;
  void * lrefcon;
  int idx, found, msize;

  if ((idx = asysn_audiounit_proplisteners_index(p->params[2])) < 0)
    return noErr; 
  
  lrefcon = (void *)(p->params[0]);
  lproc = (AudioUnitPropertyListenerProc)(p->params[1]);

  if (!lproc)
    return noErr; 

  found = 0;

  OSSpinLockLock(&(My->lock_proplisteners));

  for (lp = My->proplisteners[idx]; lp != NULL; lp = lp->next)
    if ((lp->lrefcon == lrefcon) && (lp->lproc == lproc))
      {
	found = 1;
	break;
      }

  if (!found)
    {
      msize = sizeof(asysn_audiounit_proplisten);
      if (lp = malloc(msize))
	{
	  asysn_audiounit_memstatus(lp, msize, MADV_WILLNEED);
	  lp->lproc = lproc;
	  lp->lrefcon = lrefcon;
	  lp->next = My->proplisteners[idx];
	  My->proplisteners[idx] = lp;
	}
    }

  OSSpinLockUnlock(&(My->lock_proplisteners));

  /* end of critical section */
  
  return noErr; 
}

/*****************************************************/
/* Selector: Remove Property Listener                */
/*                                                   */
/*  params[0]: AudioUnitPropertyListenerProc inProc; */
/*  params[1]: AudioUnitPropertyID inID;             */
/*                                                   */
/* Instructs the AudioUnit to stop calling the       */
/* inProc for the specified property inID.           */
/*****************************************************/

ComponentResult asysn_audiounit_kAudioUnitRemovePropertyListenerSelect(
                                           ComponentParameters * p, 
					   asysn_audiounit_InstanceState * My)
{
  asysn_audiounit_proplisten * lp, * remove, ** trailer;
  AudioUnitPropertyListenerProc lproc;
  int idx, msize;

  if ((idx = asysn_audiounit_proplisteners_index(p->params[1])) < 0)
    return noErr; 

  lproc = (AudioUnitPropertyListenerProc)(p->params[0]);

  OSSpinLockLock(&(My->lock_proplisteners));

  lp = My->proplisteners[idx];
  trailer = &(My->proplisteners[idx]);

  while (lp != NULL)
    if (lp->lproc == lproc)
      {
	remove = lp;
	lp = (*trailer) = lp->next;
	msize = sizeof(asysn_audiounit_proplisten);
	asysn_audiounit_memstatus(remove, msize, MADV_FREE);
 	free(remove);
      }
    else
      {
	trailer = &(lp->next);
	lp = lp->next;
      }

  OSSpinLockUnlock(&(My->lock_proplisteners));

  return noErr; 
}

/*****************************************************/
/* Selector: Remove Property Listener With User Data */
/*                                                   */
/*  params[0]: void * inProcRefCon;                  */
/*  params[1]: AudioUnitPropertyListenerProc inProc; */
/*  params[2]: AudioUnitPropertyID inID;             */
/*                                                   */
/* Instructs the AudioUnit to stop calling the       */
/* inProc for the specified property inID.           */
/*****************************************************/

ComponentResult asysn_audiounit_kAudioUnitRemovePropertyListenerWithUserDataSelect(
                                           ComponentParameters * p, 
					   asysn_audiounit_InstanceState * My)
{
  asysn_audiounit_proplisten * lp, * remove, ** trailer;
  AudioUnitPropertyListenerProc lproc;
  void * lrefcon;
  int idx, msize;

  if ((idx = asysn_audiounit_proplisteners_index(p->params[2])) < 0)
    return noErr; 

  lrefcon = (void *)(p->params[0]);
  lproc = (AudioUnitPropertyListenerProc)(p->params[1]);

  OSSpinLockLock(&(My->lock_proplisteners));

  lp = My->proplisteners[idx];
  trailer = &(My->proplisteners[idx]);

  while (lp != NULL)
    if ((lp->lproc == lproc) && (lp->lrefcon == lrefcon))
      {
	remove = lp;
	lp = (*trailer) = lp->next;
	msize = sizeof(asysn_audiounit_proplisten);
	asysn_audiounit_memstatus(remove, msize, MADV_FREE);
 	free(remove);
      }
    else
      {
	trailer = &(lp->next);
	lp = lp->next;
      }

  OSSpinLockUnlock(&(My->lock_proplisteners));

  return noErr; 
}

/*************************************/
/*  Selector: Add Render Notify      */
/*                                   */
/*  params[0]: void * inProcRefCon;  */
/*  params[1]: ProcPtr inProc;       */
/*                                   */
/*************************************/

ComponentResult asysn_audiounit_kAudioUnitAddRenderNotifySelect
(ComponentParameters * p, asysn_audiounit_InstanceState * My)
{ 
  asysn_audiounit_rendernotify * notify;
  int msize;

  msize = sizeof(asysn_audiounit_rendernotify);
  if (!(notify = malloc(msize)))
    return noErr;
  asysn_audiounit_memstatus(notify, msize, MADV_WILLNEED);

  notify->nproc = (AURenderCallback)(p->params[1]);
  notify->nrefcon = (void *)(p->params[0]);

  OSSpinLockLock(&(My->lock_rendernotify));

  notify->next = My->rendernotify;
  My->rendernotify = notify;

  OSSpinLockUnlock(&(My->lock_rendernotify));

  return noErr;
}

/*************************************/
/*  Selector: Remove Render Notify   */
/*                                   */
/*  params[0]: void * inProcRefCon;  */
/*  params[1]: ProcPtr inProc;       */
/*                                   */
/*************************************/

ComponentResult asysn_audiounit_kAudioUnitRemoveRenderNotifySelect
(ComponentParameters * p, asysn_audiounit_InstanceState * My)
{ 
  asysn_audiounit_rendernotify * notify, * remove, ** trailer;
  int msize;

  OSSpinLockLock(&(My->lock_rendernotify));

  notify = My->rendernotify;
  trailer = &(My->rendernotify);

  while (notify != NULL)
    if ((notify->nproc == (AURenderCallback)(p->params[1])) &&
	(notify->nrefcon == (void *)(p->params[0])))
      {
	remove = notify;
	notify = (*trailer) = notify->next;

	msize = sizeof(asysn_audiounit_rendernotify);
	asysn_audiounit_memstatus(remove, msize, MADV_FREE);
	free(remove);
      }
    else
      {
	trailer = &(notify->next);
	notify = notify->next;
      }

  OSSpinLockUnlock(&(My->lock_rendernotify));

  return noErr;
}


/******************************************/
/*     Selector: GetParameterSelect       */
/*                                        */
/* params[0]: Float32 * outValue          */
/* params[1]: AudioUnitElement inElement  */
/* params[2]: AudioUnitScope inScope      */
/* params[3]: AudioUnitParameterID inID   */
/*                                        */
/******************************************/

ComponentResult asysn_audiounit_kAudioUnitGetParameterSelect
(ComponentParameters * p, asysn_audiounit_InstanceState * My)
{ 
  if (((AudioUnitScope)(p->params[2])) != kAudioUnitScope_Global)  /* inScope */
    return kAudioUnitErr_InvalidScope;

  return asysn_audiounit_getSASLevent(((AudioUnitParameterID)(p->params[3])) /* inID */,
				      ((Float32 *)(p->params[0])) /* outValue */, My);
}


/*********************************************/
/*     Selector: SetParameterSelect          */
/*                                           */
/* params[0]: UInt32 inBufferOffsetInFrames  */
/* params[1]: Float32 inValue                */
/* params[2]: AudioUnitElement inElement     */
/* params[3]: AudioUnitScope inScope;        */
/* params[4]: AudioUnitParameterID inID;     */
/*                                           */
/*********************************************/

ComponentResult asysn_audiounit_kAudioUnitSetParameterSelect
(ComponentParameters * p, asysn_audiounit_InstanceState * My)
{
  asysn_audiounit_SASLevent SASLevent;
  int result;
  Float32 inValue;
  int inID;

  if (((AudioUnitScope)(p->params[3])) != kAudioUnitScope_Global)  /* inScope */
    return kAudioUnitErr_InvalidScope;

  inID = (AudioUnitParameterID)(p->params[4]);

  if ((inID < 0) || (inID >= My->num_saolparams))  /* range-check ID */
    return kAudioUnitErr_InvalidParameter;

  inValue = *((Float32 *)((unsigned char *)(&(p->params[1]))));

  if (inValue > My->parameterinfo[inID].maxValue)
    inValue = My->parameterinfo[inID].maxValue;

  if (inValue < My->parameterinfo[inID].minValue)
    inValue = My->parameterinfo[inID].minValue;

  SASLevent.index = inID;
  SASLevent.value = My->saolparam[inID].value = inValue;
  SASLevent.kcycleidx = (int)((UInt32)(p->params[0]));

  return asysn_audiounit_sendSASLevent(&SASLevent, My);
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/* The AudioUnit Entry Function */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/************************************************************/
/*   The function Component Manager calls to access the AU  */
/************************************************************/

extern ComponentResult asysn_audiounit_entry(ComponentParameters * p, Handle * obj) 
{ 
  asysn_audiounit_MIDIevent MIDIevent;
  ComponentResult result;

  if (obj == NULL) 
    switch (p->what) {
    case kComponentOpenSelect:
    case kComponentVersionSelect:
    case kComponentCanDoSelect:
      break;
    default:
    return noErr;  /* avoid race condition */
    }

  switch (p->what) { 
  case kComponentOpenSelect:
    result = asysn_audiounit_kComponentOpenSelect
      (p, (asysn_audiounit_InstanceState *) obj);
    break;
  case kComponentCloseSelect:
    result = asysn_audiounit_kComponentCloseSelect
      (p, (asysn_audiounit_InstanceState *) obj);
    break;
  case kAudioUnitInitializeSelect:
    result = asysn_audiounit_kAudioUnitInitializeSelect
      (p, (asysn_audiounit_InstanceState *) obj);
    break;
  case kAudioUnitUninitializeSelect:
    result = asysn_audiounit_kAudioUnitUninitializeSelect
      (p, (asysn_audiounit_InstanceState *) obj);
    break;
  case kAudioUnitGetPropertyInfoSelect:
    result = asysn_audiounit_kAudioUnitGetPropertyInfoSelect
      (p, (asysn_audiounit_InstanceState *) obj);
    break;
  case kAudioUnitGetPropertySelect:
    result = asysn_audiounit_kAudioUnitGetPropertySelect
      (p, (asysn_audiounit_InstanceState *) obj);
    break;
  case kAudioUnitSetPropertySelect:
    result = asysn_audiounit_kAudioUnitSetPropertySelect
      (p, (asysn_audiounit_InstanceState *) obj);
    break;
  case kAudioUnitRenderSelect:
    result = asysn_audiounit_kAudioUnitRenderSelect
      (p, (asysn_audiounit_InstanceState *) obj);
    break;
  case kAudioUnitResetSelect:
    result = asysn_audiounit_kAudioUnitResetSelect
      (p, (asysn_audiounit_InstanceState *) obj);
    break;
  case kAudioUnitAddPropertyListenerSelect:
    result = asysn_audiounit_kAudioUnitAddPropertyListenerSelect
      (p, (asysn_audiounit_InstanceState *) obj);
    break;
  case kAudioUnitRemovePropertyListenerSelect:
    result = asysn_audiounit_kAudioUnitRemovePropertyListenerSelect
      (p, (asysn_audiounit_InstanceState *) obj);
    break;
  case kAudioUnitRemovePropertyListenerWithUserDataSelect:
    result = asysn_audiounit_kAudioUnitRemovePropertyListenerWithUserDataSelect
      (p, (asysn_audiounit_InstanceState *) obj);
    break;
  case kAudioUnitAddRenderNotifySelect:
    result = asysn_audiounit_kAudioUnitAddRenderNotifySelect
      (p, (asysn_audiounit_InstanceState *) obj);
    break;
  case kAudioUnitRemoveRenderNotifySelect:
    result = asysn_audiounit_kAudioUnitRemoveRenderNotifySelect
      (p, (asysn_audiounit_InstanceState *) obj);
    break;
  case kAudioUnitGetParameterSelect:
    result = asysn_audiounit_kAudioUnitGetParameterSelect
      (p, (asysn_audiounit_InstanceState *) obj);
    break;
  case kAudioUnitSetParameterSelect:
    result = asysn_audiounit_kAudioUnitSetParameterSelect
      (p, (asysn_audiounit_InstanceState *) obj);
    break;
  case kAudioUnitScheduleParametersSelect:
    result = noErr;
    break;
  case kMusicDeviceMIDIEventSelect:
    if (ASYS_AUDIOUNIT_MIDISUPPORT)
      {
	MIDIevent.cmd = (unsigned char)((UInt32)(p->params[3]));
	MIDIevent.d0 = (unsigned char)((UInt32)(p->params[2]));
	MIDIevent.d1 = (unsigned char)((UInt32)(p->params[1]));
	MIDIevent.flags = ASYS_AUDIOUNIT_MIDIFLAGS_WAITING;
	MIDIevent.kcycleidx = (int)((UInt32)(p->params[0]));
	result = asysn_audiounit_sendMIDIevent
	  (&MIDIevent, (asysn_audiounit_InstanceState *) obj);
      }
    else
      result = badComponentSelector;
    break;
  case kMusicDeviceSysExSelect:
  case kMusicDevicePrepareInstrumentSelect:
  case kMusicDeviceReleaseInstrumentSelect:
  case kMusicDeviceStartNoteSelect:
  case kMusicDeviceStopNoteSelect:
  default:
    result = badComponentSelector;
    break;
  case kComponentVersionSelect:
    result = 0x00010000;    /* major version 1, minor version 0 */
    break;
  case kComponentCanDoSelect:
    switch ((int)((SInt16)(p->params[0]))) {
    case kComponentOpenSelect:
    case kComponentCloseSelect:
    case kAudioUnitInitializeSelect:
    case kAudioUnitUninitializeSelect:
    case kAudioUnitGetPropertyInfoSelect:
    case kAudioUnitGetPropertySelect:
    case kAudioUnitSetPropertySelect:
    case kAudioUnitRenderSelect:
    case kAudioUnitResetSelect:
    case kAudioUnitAddPropertyListenerSelect:
    case kAudioUnitRemovePropertyListenerSelect:
    case kAudioUnitRemovePropertyListenerWithUserDataSelect:
    case kAudioUnitAddRenderNotifySelect:
    case kAudioUnitRemoveRenderNotifySelect:
    case kAudioUnitGetParameterSelect:
    case kAudioUnitSetParameterSelect:
      result = 1;
      break;
    case kMusicDeviceMIDIEventSelect:
      result = ASYS_AUDIOUNIT_MIDISUPPORT;
    default:
      result = 0;
      break;
    }
    break;
  }

  switch (p->what) { 
  case kAudioUnitRenderSelect:
    ASYS_AUDIOUNIT_WIRETAP_R(p, result);
    break;
  case kMusicDeviceMIDIEventSelect:
    ASYS_AUDIOUNIT_WIRETAP_M(p, result);
    break;
  default:
    ASYS_AUDIOUNIT_WIRETAP(p, result);
    break;
  }

  return result;
}

/****************************************************************/
/*         End of Part Three (The AudioUnit Component)          */
/****************************************************************/

/****************************************************************/
/*        End of Apple AudioUnit audio driver for sfront        */
/****************************************************************/
/****************************************************************/

