
/*
#    Sfront, a SAOL to C translator    
#    This file: Win32 DirectSound audio driver for sfront
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
#    Author: Professor John Wawrzynek, UCB CS Division
#    Maintainance Email To: John Lazzaro, lazzaro@cs.berkeley.edu
*/
 
//////////////////////////
// dsound output
// by Vincent Siliakus & Thomas Jongepier
// november 1999
//////////////////////////
#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>
#include <dsound.h>
#include <objbase.h>
#include <initguid.h> 

#include <stdio.h>

#define ASYS_SUCCESS ASYS_DONE

#if defined(ASYS_HASOUTPUT)


/* Global variables for audio output.
 */
 

char asyso_playing = 0;
int asyso_wrtCursor = 0;
int asyso_bufsize;                                /* buffer size in
bytes        */
int asyso_blksize;                                /* size preferred by
sfront    */

double asyso_speriod;
LPDIRECTSOUND 		lpDirectSound;
LPDIRECTSOUNDBUFFER lpDsb;	
LPDIRECTSOUNDBUFFER lpPrDsb;
LPDIRECTSOUNDNOTIFY lpDsNotify;
HANDLE				*hBuffPos;
DSBPOSITIONNOTIFY   *PositionNotify;
PCMWAVEFORMAT		pcmwfp;
PCMWAVEFORMAT		pcmwf;
DSBUFFERDESC		dsprbdescp;
DSBUFFERDESC		dsbdesc;
DSCAPS				dscaps;
DSBCAPS				pr,sec;
HRESULT       		hr;	
unsigned int		ds_numofsubbuff;
unsigned int		ds_subbuffercounter=0;
unsigned int		ds_writeposition=0;
unsigned int		ds_counter=0;
unsigned int		ds_resetwritecrspoint=0;
unsigned int		SECBUFFBYTESIZE;
unsigned int		SUBBUFFBYTESIZE;
unsigned int		SUBBUFFSAMPLESIZE;
unsigned int		WRITEPOSITIONOFFSET;
unsigned int		TIMES;
unsigned int		EXCEPTIONTIMES;
 
  
void DevCaps(DSCAPS *);
void PrBuffCaps(DSBCAPS *);
void SecBuffCaps(DSBCAPS *);
void WaitForNotification();
void Close();
void Play();
void Stop();
int InitOutput(unsigned int);
 
void DevCaps(DSCAPS *DSCaps)
{	
	DSCaps->dwSize = sizeof(DSCAPS);
	IDirectSound_GetCaps(lpDirectSound,DSCaps);
}
 
void PrBuffCaps(DSBCAPS *PrBCaps)
{	
	PrBCaps->dwSize = sizeof(DSBCAPS); 
	IDirectSoundBuffer_GetCaps(lpPrDsb,PrBCaps); 
}
 
void SecBuffCaps(DSBCAPS *SecBCaps)
{	
	SecBCaps->dwSize = sizeof(DSBCAPS);
	IDirectSoundBuffer_GetCaps(lpDsb,SecBCaps);
}
 
void WaitForNotification()
{
	DWORD EventNumber = (WaitForMultipleObjects
		(ds_numofsubbuff, hBuffPos, FALSE, INFINITE));
	//ResetEvent(hBuffPos[EventNumber]); //for manual ResetEvent. 
}
 
void Close()
{ 
	for(ds_counter=0;ds_counter<ds_numofsubbuff;ds_counter++)
		CloseHandle(hBuffPos[ds_counter]);
	IDirectSoundBuffer_Release(lpDsb);
	IDirectSound_Release(lpDirectSound);
}
 
void Play() 
{ 
	IDirectSoundBuffer_Play(lpDsb,0,0,DSBPLAY_LOOPING ); 
} 
  
void Stop()
{
	IDirectSoundBuffer_Stop(lpDsb);
}
 
 
int InitOutput(unsigned int sys_osize, 
			   unsigned int samplerate, 
			   unsigned int numchannels)
{ 
	HWND hwnd = GetForegroundWindow();
	if(hwnd==NULL)hwnd = GetDesktopWindow();
	SECBUFFBYTESIZE	 =	sys_osize*16;//use 16 or 32	
	SUBBUFFBYTESIZE	 =	sys_osize;
	SUBBUFFSAMPLESIZE=	sys_osize/2;
  	WRITEPOSITIONOFFSET = SUBBUFFBYTESIZE;
	TIMES=SECBUFFBYTESIZE/SUBBUFFBYTESIZE;
	EXCEPTIONTIMES = (WRITEPOSITIONOFFSET/SUBBUFFBYTESIZE);
	ds_resetwritecrspoint =  
		SECBUFFBYTESIZE-(EXCEPTIONTIMES*SUBBUFFBYTESIZE);
	ds_numofsubbuff=SECBUFFBYTESIZE/SUBBUFFBYTESIZE;
	////////////////////////////////
	//dsound init
	////////////////////////////////
	//Create DirectSound Object
	if FAILED(DirectSoundCreate(NULL, &lpDirectSound, NULL))
		return ASYS_ERROR;
	//
	//Set Cooperative level of DirectSound Object
	if FAILED(IDirectSound_SetCooperativeLevel(lpDirectSound,hwnd, 
			DSSCL_PRIORITY))return ASYS_ERROR;
	//    
	////////////////////////////////  
   // Set up wave format structure.
   memset(&pcmwfp, 0, sizeof(PCMWAVEFORMAT));
    pcmwfp.wf.wFormatTag = WAVE_FORMAT_PCM;
    pcmwfp.wf.nChannels = numchannels;
    pcmwfp.wf.nSamplesPerSec = samplerate;
    pcmwfp.wf.nBlockAlign = 2*numchannels;
    pcmwfp.wf.nAvgBytesPerSec =
		pcmwfp.wf.nSamplesPerSec * pcmwfp.wf.nBlockAlign;
    pcmwfp.wBitsPerSample = 16;
	//
	////////////////////////////////
    // Set up DSBUFFERDESC structure.
    memset(&dsprbdescp, 0, sizeof(DSBUFFERDESC)); // Zero it out.
    dsprbdescp.dwSize = sizeof(DSBUFFERDESC);
    dsprbdescp.dwFlags = DSBCAPS_PRIMARYBUFFER;
    dsprbdescp.dwBufferBytes = 0;
    dsprbdescp.lpwfxFormat = NULL;
	//
	////////////////////////////////
	// Create Primary buffer.  
    if FAILED(IDirectSound_CreateSoundBuffer(lpDirectSound,&dsprbdescp, 
	&lpPrDsb, NULL))return ASYS_ERROR;
	// Set primary buffer to desired format.
    if FAILED(IDirectSoundBuffer_SetFormat(lpPrDsb,
		(LPWAVEFORMATEX)&pcmwfp))return ASYS_ERROR;
	//start playing (no sound yet)
    IDirectSoundBuffer_Play(lpPrDsb,0,0,DSBPLAY_LOOPING);
	//
    // Set up wave format structure.
    memset(&pcmwf, 0, sizeof(PCMWAVEFORMAT));
    pcmwf.wf.wFormatTag = WAVE_FORMAT_PCM;
    pcmwf.wf.nChannels = numchannels;
    pcmwf.wf.nSamplesPerSec = samplerate;
    pcmwf.wf.nBlockAlign = 2*numchannels;
    pcmwf.wf.nAvgBytesPerSec =
		pcmwf.wf.nSamplesPerSec * pcmwf.wf.nBlockAlign;
    pcmwf.wBitsPerSample = 16;	
	//
    // Set up DSBUFFERDESC structure.
    memset(&dsbdesc, 0, sizeof(DSBUFFERDESC)); // Zero it out.
    dsbdesc.dwSize = sizeof(DSBUFFERDESC);
    // Need default controls (pan, volume, frequency).
    dsbdesc.dwFlags = 
					 DSBCAPS_GLOBALFOCUS 
					|DSBCAPS_GETCURRENTPOSITION2
					|DSBCAPS_CTRLPOSITIONNOTIFY ;
    dsbdesc.dwBufferBytes = SECBUFFBYTESIZE;
    dsbdesc.lpwfxFormat = (LPWAVEFORMATEX)&pcmwf;
	//
	////////////////////////////////
	if FAILED(IDirectSound_CreateSoundBuffer(lpDirectSound,&dsbdesc, 
		&lpDsb, NULL))return ASYS_ERROR;
    //
	PositionNotify=(DSBPOSITIONNOTIFY*)malloc(ds_numofsubbuff*
		(sizeof(DSBPOSITIONNOTIFY)));
	hBuffPos=(HANDLE*)malloc(ds_numofsubbuff*sizeof(HANDLE));
	//
	for(ds_counter=0;ds_counter<ds_numofsubbuff;ds_counter++){
		hBuffPos[ds_counter]=CreateEvent(NULL,FALSE,FALSE,NULL);
		PositionNotify[ds_counter].dwOffset = SUBBUFFBYTESIZE*ds_counter;
		PositionNotify[ds_counter].hEventNotify=hBuffPos[ds_counter]; 
	}
	//
	if FAILED(IDirectSoundBuffer_QueryInterface(lpDsb,
		&IID_IDirectSoundNotify,(LPVOID *)&lpDsNotify))return ASYS_ERROR;  
	//
	IDirectSoundNotify_SetNotificationPositions(lpDsNotify,ds_numofsubbuff,
		PositionNotify); 
	//\\\\\\\\\\\\\\\
	//
	Play();
	return ASYS_SUCCESS;
}
 
 
#endif
 
#if (defined(ASYS_HASOUTPUT) && !defined(ASYS_HASINPUT))
 
/* Opens waveform output for a given srate/channels.
 */
 
int asys_osetup(int srate, int ochannels, int osample, 
                char * oname, int toption)
{

  int osize = ASYS_OCHAN*ACYCLE;

  InitOutput(osize,srate,ochannels);
  return ASYS_SUCCESS;
}
 
#endif
 
#if defined(ASYS_HASOUTPUT)
  
/*
 * Shuts down audio system.
 */
 
void asys_oshutdown(void)
 
{
  Stop();
  Close();
}
 
#endif
 
#if defined(ASYS_HASOUTPUT)
 
int asys_putbuf(short **buffer, int * osize)
 
{ 	
	DWORD dwBytes;
	*osize=SUBBUFFSAMPLESIZE; 
	ds_writeposition=ds_subbuffercounter*SUBBUFFBYTESIZE;	
	WaitForNotification();	
	if(ds_writeposition>ds_resetwritecrspoint)
	{
		ds_subbuffercounter=0;
		ds_writeposition=0;
	}
	hr = IDirectSoundBuffer_Unlock(lpDsb,*buffer, 
		SUBBUFFBYTESIZE, NULL,NULL);  
    hr = IDirectSoundBuffer_Lock(lpDsb,ds_writeposition, 
		SUBBUFFBYTESIZE, (LPVOID*)buffer,&dwBytes, NULL, 0, NULL);    
	ds_subbuffercounter++;
	return ASYS_DONE;
}
   
/****************************************************************/
/*        creates buffer, and generates starting silence        */
/****************************************************************/

int asys_preamble(ASYS_OTYPE * asys_obuf[], int * osize)

{
  int i;

  /* emulates old interface, since I don't understand asys_putbuf */
  /* well enough to make direct modifications  */

  /* set up first call, which allocates buffer */

  *asys_obuf = NULL;
  *osize = 0;

  if (asys_putbuf(asys_obuf, osize) == ASYS_ERROR)
    return ASYS_ERROR;

  /* clear allocated buffer */

  for (i = 0; i < (*osize); i++)
    (*asys_obuf)[i] = 0;

  /* send the 4 silence putbuf's of old API */

  for(i = 0; i < 4; i++)
    if (asys_putbuf(asys_obuf, osize) == ASYS_ERROR)
      return ASYS_ERROR;

  return ASYS_DONE;
}


#endif
 

/* Currently not supported audio input.  All routines are dummies.
 */

#if (!defined(ASYS_HASOUTPUT) && defined(ASYS_HASINPUT))

/* Initializes audio input.
 */
int asys_isetup(int srate, int ichannels, int isample, 
                char * iname, int toption)

{
  return ASYS_ERROR;
}

#endif

#if (defined(ASYS_HASOUTPUT) && defined(ASYS_HASINPUT))

/* Initialized bidirectional audio flows. */
int asys_iosetup(int srate, int ichannels, int ochannels,
                 int isample, int osample, 
                 char * iname, char * oname, int toption)

{
  return ASYS_ERROR;
}

#endif

#if defined(ASYS_HASINPUT)

/* Shuts down audio input device.
 */

void asys_ishutdown(void)

{
}

#endif

#if defined(ASYS_HASINPUT)

/* Gets one frame of audio from input.
 */

int asys_getbuf(ASYS_ITYPE * asys_ibuf[], int * isize)
{
  return ASYS_ERROR;
}

#endif


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*  ksync() and ksyncinit() for -playback and -timesync modes  */
/*_____________________________________________________________*/

#if defined(ASYS_KSYNC)

#if (ASYS_TIMEOPTION != ASYS_TIMESYNC)

#include <windows.h>
#include <windowsx.h>
#include <winbase.h>
#include <mmsystem.h>

#define SYNC_MAXCOUNT 4294967296L
#define SYNC_KMTIME EV(KUTIME)/1000

DWORD sync_last, sync_this, sync_delay;
TIMECAPS sync_tc;
BOOL sync_waitFlag;

/***********************************************************/
/*         support routines                                */
/***********************************************************/

void CALLBACK sync_callBack(UINT uID, UINT uMsg, 
                            DWORD dwUser, DWORD dw1, DWORD dw2)
{
  sync_waitFlag = TRUE;
}

BOOL sync_sleep(UINT ms)
{
  sync_waitFlag = FALSE;
  if FAILED(timeSetEvent(ms, (UINT) 1, &sync_callBack, 0L, TIME_ONESHOT))
    {
      fprintf(stderr, "timeSetEvent error.\n");
      exit(-1);
    }
  // spin wait
  while (!sync_waitFlag) {}
  return TRUE;
}

/***********************************************************/
/*         synchronizes on k-rate boundaries               */
/***********************************************************/

float ksync()

{
  float ret;

  sync_this = timeGetTime();
  
  if (sync_this<sync_last)   // timer wrap around
    sync_last = (DWORD) (sync_last - SYNC_MAXCOUNT) + SYNC_KMTIME;
  else 
    sync_last += SYNC_KMTIME;

  sync_delay = sync_last - sync_this;
  if (sync_delay>0)
    {
      /* sync_sleep(sync_delay); */   /* can this be safely commented out? */
      ret = (SYNC_KMTIME - sync_delay)/SYNC_KMTIME;
    }
  else 
    ret = 1.0F;

  return ret;
}

/***********************************************************/
/*         initializes k-rate boundaries sync              */
/***********************************************************/

void ksyncinit()

{
  // Set 1ms time resolution
  if FAILED(timeGetDevCaps(&sync_tc, sizeof(TIMECAPS)))
    {
      fprintf(stderr, "TimeGetDevCaps error.\n");
      exit(-1);
    }
  sync_last = timeGetTime();
}

#endif


#if (ASYS_TIMEOPTION == ASYS_TIMESYNC)

#include <windows.h>
#include <windowsx.h>
#include <winbase.h>
#include <mmsystem.h>

#define SYNC_MAXCOUNT 4294967296L
#define SYNC_KMTIME EV(KUTIME)/1000

DWORD sync_last, sync_this, sync_delay;
TIMECAPS sync_tc;
BOOL sync_waitFlag;

/***********************************************************/
/*         support routines                                */
/***********************************************************/

void CALLBACK sync_callBack(UINT uID, UINT uMsg, 
                            DWORD dwUser, DWORD dw1, DWORD dw2)
{
  sync_waitFlag = TRUE;
}

BOOL sync_sleep(UINT ms)
{
  sync_waitFlag = FALSE;
  if FAILED(timeSetEvent(ms, (UINT) 1, &sync_callBack, 0L, TIME_ONESHOT))
    {
      fprintf(stderr, "timeSetEvent error.\n");
      exit(-1);
    }
  // spin wait
  while (!sync_waitFlag) {}
  return TRUE;
}

/***********************************************************/
/*         synchronizes on k-rate boundaries               */
/***********************************************************/

float ksync()

{
  float ret;

  sync_this = timeGetTime();
  
  if (sync_this<sync_last)   // timer wrap around
    sync_last = (DWORD) (sync_last - SYNC_MAXCOUNT) + SYNC_KMTIME;
  else 
    sync_last += SYNC_KMTIME;

  sync_delay = sync_last - sync_this;
  if (sync_delay>0)
    {
      sync_sleep(sync_delay);
      ret = (SYNC_KMTIME - sync_delay)/SYNC_KMTIME;
    }
  else 
    ret = 1.0F;

  return ret;
}

/***********************************************************/
/*         initializes k-rate boundaries sync              */
/***********************************************************/

void ksyncinit()

{
  // Set 1ms time resolution
  if FAILED(timeGetDevCaps(&sync_tc, sizeof(TIMECAPS)))
    {
      fprintf(stderr, "TimeGetDevCaps error.\n");
      exit(-1);
    }
  sync_last = timeGetTime();
}

#endif

#endif  /* ASYS_KSYNC */
