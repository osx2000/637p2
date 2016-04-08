
/*
#    Sfront, a SAOL to C translator    
#    This file: portaudio audio driver for sfront
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
/*             wav file audio driver for sfront                 */ 
/****************************************************************/

#define ASYSN_PORTAUDIO_DEBUG        0    /* 1 for debug printouts */
        
#define ASYSN_PORTAUDIO_SLEEPMS    250    /* exit check interval */

#define ASYSN_PORTAUDIO_BUFFMIN      2    /* at least double buffer   */
#define ASYSN_PORTAUDIO_BUFFDEFAULT  4    /* known to work well       */
#define ASYSN_PORTAUDIO_BUFFMAX      6    /* avoid huge buffer counts */
#define ASYSN_PORTAUDIO_LATENCYMAX 0.5    /* 500ms maximum latency    */

PaStream * asysn_portaudio_fd;
int asysn_portaudio_silence;
volatile int asysn_portaudio_done;

/* state for passive input driver */

#if (defined(ASYS_ACTIVE_IO) && !defined(ASYS_HASINPUT))

ASYS_ITYPE * asysn_portaudio_ibuf;
int asysn_portaudio_iptr;
int asysn_portaudio_ilast;
int asysn_portaudio_ileft;

#endif

/* state for passive output driver */

#if (defined(ASYS_ACTIVE_IO) && !defined(ASYS_HASOUTPUT))

ASYS_OTYPE * asysn_portaudio_obuf;
int asysn_portaudio_optr;
int asysn_portaudio_olast;
int asysn_portaudio_oleft;

#endif


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*       callback functions for different scenareos              */
/*_______________________________________________________________*/

#if defined(ASYS_ACTIVE_O)

/****************************************************************/
/*            output-only callback function                     */
/****************************************************************/

int asysn_portaudio_callback(void * inputBuffer, void * outputBuffer,
			     unsigned long framesPerBuffer, 
			     PaTimestamp outTime, void *userData)
{
  int optr = 0;
  int oleft = (int) (framesPerBuffer * ASYS_OCHAN);
  float * obuf = (float *) outputBuffer;
  int osize;

  if ((asysn_portaudio_done == ASYS_DONE) && (asysn_portaudio_silence == 0))
    {
      do 
	{
	  osize = oleft;
	  asysn_portaudio_done = asys_orun(&(obuf[optr]), &osize);
	  optr += osize;
	  oleft -= osize;
	} 
      while ((asysn_portaudio_done == ASYS_DONE) && (oleft > 0));

      if (oleft > 0)
	memset(&(obuf[optr]), 0, sizeof(float)*oleft);
    }
  else
    {
      memset(outputBuffer, 0, sizeof(float)*oleft);
      if (asysn_portaudio_silence)
	asysn_portaudio_silence--;
    }

  return 0;
}

#endif


#if defined(ASYS_ACTIVE_IO)

#if (defined(ASYS_HASOUTPUT) && defined(ASYS_HASINPUT))

/****************************************************************/
/*       Portaudio input and output callback function           */
/****************************************************************/

int asysn_portaudio_callback(void * inputBuffer, void * outputBuffer,
			     unsigned long framesPerBuffer, 
			     PaTimestamp outTime, void *userData)
{
  int ileft = (int) (framesPerBuffer * ASYS_ICHAN);
  int oleft = (int) (framesPerBuffer * ASYS_OCHAN);
  float * ibuf = (float *) inputBuffer;
  float * obuf = (float *) outputBuffer;
  int isize, osize;

  isize = ileft;
  osize = oleft;

  if ((asysn_portaudio_done == ASYS_DONE) && (asysn_portaudio_silence == 0))
    {
      asysn_portaudio_done = asys_iorun(ibuf, &isize, obuf, &osize);

      if (osize < oleft)
	memset(&(obuf[osize]), 0, sizeof(float)*(oleft - osize));
    }
  else
    {
      memset(outputBuffer, 0, sizeof(float)*oleft);
      if (asysn_portaudio_silence)
	asysn_portaudio_silence--;
    }

  return 0;
}

#endif


#if (defined(ASYS_HASOUTPUT) && !defined(ASYS_HASINPUT))

/****************************************************************/
/*       Portaudio output, passive input callback function      */
/****************************************************************/

int asysn_portaudio_callback(void * inputBuffer, void * outputBuffer,
			     unsigned long framesPerBuffer, 
			     PaTimestamp outTime, void *userData)
{
  int isize, osize;
  int optr = 0;
  int oleft = (int) (framesPerBuffer * ASYS_OCHAN);
  float * obuf = (float *) outputBuffer;

  if ((asysn_portaudio_done == ASYS_DONE) && (asysn_portaudio_silence == 0))
    {
      do 
	{
	  if (asysn_portaudio_ileft == 0)
	    {  
	      if ((asys_getbuf(&asysn_portaudio_ibuf, &asysn_portaudio_ilast) 
		   != ASYS_DONE) || (asysn_portaudio_ilast == 0))
		{
		  asysn_portaudio_done = ASYS_EXIT;
		  break;
		}
	      asysn_portaudio_iptr  = 0;
	      asysn_portaudio_ileft = asysn_portaudio_ilast;
	    }

	  isize = asysn_portaudio_ileft;
	  osize = oleft;
	  asysn_portaudio_done = asys_iorun(&(asysn_portaudio_ibuf
					      [asysn_portaudio_iptr]),
					    &isize, &(obuf[optr]), &osize);
	  asysn_portaudio_iptr += isize;
	  asysn_portaudio_ileft -= isize;
	  optr += osize;
	  oleft -= osize;
	}
      while ((asysn_portaudio_done == ASYS_DONE) && (oleft > 0));

      if (oleft > 0)
	memset(&(obuf[optr]), 0, sizeof(float)*oleft);
    }
  else
    {
      memset(outputBuffer, 0, sizeof(float)*oleft);
      if (asysn_portaudio_silence)
	asysn_portaudio_silence--;
    }

  return 0;
}


#endif


#if (!defined(ASYS_HASOUTPUT) && defined(ASYS_HASINPUT))

/****************************************************************/
/*       Portaudio input, passive output callback function      */
/****************************************************************/

int asysn_portaudio_callback(void * inputBuffer, void * outputBuffer,
			     unsigned long framesPerBuffer, 
			     PaTimestamp outTime, void *userData)
{
  int isize, osize;
  int iptr = 0;
  int ileft = (int) (framesPerBuffer * ASYS_ICHAN);
  float * ibuf = (float *) inputBuffer;

  while ((asysn_portaudio_done == ASYS_DONE) && ileft)
    {
      isize = ileft;
      osize = asysn_portaudio_oleft;

      asysn_portaudio_done = asys_iorun(&(ibuf[iptr]), &isize,
					&(asysn_portaudio_obuf
					  [asysn_portaudio_optr]), &osize);

      if (asysn_portaudio_done == ASYS_DONE)
	{
	  asysn_portaudio_optr  += osize;
	  asysn_portaudio_oleft -= osize;
	  iptr  += isize;
	  ileft -= isize;

	  if (asysn_portaudio_oleft == 0)
	    {
	      if (asys_putbuf(&asysn_portaudio_obuf, &asysn_portaudio_olast)
		  == ASYS_DONE)
		{
		  asysn_portaudio_oleft = asysn_portaudio_olast;
		  asysn_portaudio_optr  = 0;
		}
	      else
		asysn_portaudio_done = ASYS_EXIT;
	    }
	}
      else
	{
	  osize = asysn_portaudio_olast - asysn_portaudio_oleft;
	  asys_putbuf(&asysn_portaudio_obuf, &osize);
	}
    }
}

#endif

#endif


/****************************************************************/
/*               initialize Portaudio system                    */
/****************************************************************/

int asysn_portaudio_init(void)

{
  PaError err;

  if ((err = Pa_Initialize()) != paNoError)
    {
      fprintf(stderr, "Error: PortAudio initialization failure (%s).\n", 
	      Pa_GetErrorText(err));
      if (err == paHostError)
	fprintf(stderr, "     : Native error (%s).\n", strerror(errno));
      return ASYS_ERROR;
    }
  return ASYS_DONE;
}


/****************************************************************/
/*              check default output suitability                */
/****************************************************************/

int asysn_portaudio_outcheck(double * srate, int ochannels, PaDeviceID * pid)

{  
  const PaDeviceInfo * pidinfo;
  PaError err;
  int i;

  if ((*pid = Pa_GetDefaultOutputDeviceID()) == paNoDevice)
    {
      fprintf(stderr, "Error: No PortAudio output device.\n");
      return ASYS_ERROR;
    }

  if ((pidinfo = Pa_GetDeviceInfo(*pid)) == NULL)
    {
      fprintf(stderr, "Error: Bad PortAudio default output ID.\n");
      return ASYS_ERROR;
    }

  if (pidinfo->maxOutputChannels < ochannels)
    {
      fprintf(stderr, "Error: SAOL outchannels %i > soundcard limit %i.\n",
	      ochannels, pidinfo->maxOutputChannels);
      return ASYS_ERROR;
    }

  if (pidinfo->numSampleRates == -1)
    {
      if ((*srate < pidinfo->sampleRates[0]) ||
	  (*srate > pidinfo->sampleRates[1]))
	{
	  fprintf(stderr, "Error: SAOL srate %lf > outside PortAudio range "
		  "[%lf, %lf].\n", *srate, pidinfo->sampleRates[0],
		  pidinfo->sampleRates[1]);
	  return ASYS_ERROR;
	}
    }
  else
    {
      err = paHostError;
      for (i = 0; i < pidinfo->numSampleRates; i++)
	if (*srate == pidinfo->sampleRates[i])
	  {
	    err = paNoError;
	    break;
	  }

      /* later add approximate check */
      
      if (err == paHostError)
	{
	  fprintf(stderr, "Error: SAOL srate %lf not among PortAudio rates ",
		  *srate);
	  for (i = 0; i < pidinfo->numSampleRates; i++)
	    fprintf(stderr, "%lf ", pidinfo->sampleRates[i]);
	  fprintf(stderr, "\n");
	  return ASYS_ERROR;
	}
    }

  return ASYS_DONE;
}


/****************************************************************/
/*              check default input suitability                 */
/****************************************************************/

int asysn_portaudio_incheck(double * srate, int ichannels, PaDeviceID * pid)

{  
  const PaDeviceInfo * pidinfo;
  PaError err;
  int i;

  if ((*pid = Pa_GetDefaultInputDeviceID()) == paNoDevice)
    {
      fprintf(stderr, "Error: No PortAudio input device.\n");
      return ASYS_ERROR;
    }

  if ((pidinfo = Pa_GetDeviceInfo(*pid)) == NULL)
    {
      fprintf(stderr, "Error: Bad PortAudio default input ID.\n");
      return ASYS_ERROR;
    }

  if (pidinfo->maxInputChannels < ichannels)
    {
      fprintf(stderr, "Error: SAOL inchannels %i > soundcard limit %i.\n",
	      ichannels, pidinfo->maxInputChannels);
      return ASYS_ERROR;
    }

  if (pidinfo->numSampleRates == -1)
    {
      if ((*srate < pidinfo->sampleRates[0]) ||
	  (*srate > pidinfo->sampleRates[1]))
	{
	  fprintf(stderr, "Error: SAOL srate %lf > outside PortAudio irange "
		  "[%lf, %lf].\n", *srate, pidinfo->sampleRates[0],
		  pidinfo->sampleRates[1]);
	  return ASYS_ERROR;
	}
    }
  else
    {
      err = paHostError;
      for (i = 0; i < pidinfo->numSampleRates; i++)
	if (*srate == pidinfo->sampleRates[i])
	  {
	    err = paNoError;
	    break;
	  }

      /* later add approximate check */
      
      if (err == paHostError)
	{
	  fprintf(stderr, "Error: SAOL srate %lf not among PortAudio irates ",
		  *srate);
	  for (i = 0; i < pidinfo->numSampleRates; i++)
	    fprintf(stderr, "%lf ", pidinfo->sampleRates[i]);
	  fprintf(stderr, "\n");
	  return ASYS_ERROR;
	}
    }

  return ASYS_DONE;
}


/****************************************************************/
/*                determine buffer parameters                   */
/****************************************************************/

int asysn_portaudio_buffparam(unsigned long * framesPerBuffer,
			      unsigned long * numberOfBuffers,
			      double samplerate)

{
  double totframes, maxframes;
  unsigned long num;
  int i, found;

  /* high-latency mac_carbon needs special treatment */

#ifdef ASYS_OUTDRIVER_MAC_CARBON

  if (ASYS_LATENCYTYPE == ASYS_HIGHLATENCY)
    {
      *numberOfBuffers = 10;
      *framesPerBuffer = (ASYS_LATENCY/ATIME)/(*numberOfBuffers);
      return ASYS_DONE;
    }

#endif

  /* used for windows and low-latency mac_carbon */

  *numberOfBuffers = ASYSN_PORTAUDIO_BUFFDEFAULT;
  totframes = ASYS_LATENCY/ATIME;
  maxframes = ASYSN_PORTAUDIO_LATENCYMAX/ATIME;
  found = 0;

  do {

    *framesPerBuffer = (unsigned long) (totframes/(*numberOfBuffers));
  
    for (i = 1; i < *framesPerBuffer; i *= 2)
      if ((*framesPerBuffer >= i) && (*framesPerBuffer <= i*2))
	{
	  *framesPerBuffer = (((*framesPerBuffer - i) 
			       < (2*i - *framesPerBuffer)) ? i : 2*i);
	  break;
	}

    num = Pa_GetMinNumBuffers(*framesPerBuffer, samplerate);

    if (num <= *numberOfBuffers)
      found = 1;
    else
      totframes *= 2;

  } while ((found == 0) && (totframes < maxframes));

  /* handle a driver with broken Pa_GetMinNumBuffers() */
  
  if (found == 0)
    {
      *numberOfBuffers = ASYSN_PORTAUDIO_BUFFDEFAULT;
      *framesPerBuffer = (ASYS_LATENCY/ATIME)/(*numberOfBuffers);
    }

  asysn_portaudio_silence = *numberOfBuffers;
  return ASYS_DONE;
}



#if (defined(ASYS_HASOUTPUT) && !defined(ASYS_HASINPUT))

/****************************************************************/
/*  portaudio output only: a ASYS_ACTIVE_IO, all ASYS_ACTIVE_O  */
/****************************************************************/

int asys_osetup(int srate, int ochannels, int osample, 
		char * oname, int toption)

{
  double samplerate = (double) srate;
  unsigned long framesPerBuffer;
  unsigned long numberOfBuffers;
  PaDeviceID opid;
  PaError err;

  if (asysn_portaudio_init() == ASYS_ERROR)
    return ASYS_ERROR;

  if (asysn_portaudio_outcheck(&samplerate, ochannels, &opid) == ASYS_ERROR)
    return ASYS_ERROR;

  if (asysn_portaudio_buffparam(&framesPerBuffer, 
				&numberOfBuffers, samplerate) == ASYS_ERROR)
    return ASYS_ERROR;

  err = Pa_OpenStream(&asysn_portaudio_fd, 
		      paNoDevice,   0, paFloat32, NULL, 
		      opid, ochannels, paFloat32, NULL, 
		      samplerate, framesPerBuffer, numberOfBuffers, 
		      paClipOff | paDitherOff, asysn_portaudio_callback, NULL);
		      
  if (err != paNoError)
    {
      fprintf(stderr, "Error: Cannot open PortAudio stream (%s).\n", 
	      Pa_GetErrorText(err));
      return ASYS_ERROR;
    }

  if (ASYSN_PORTAUDIO_DEBUG)
    {
      printf("Opening output-only Portaudio stream, %i channels\n"
	     "sample rate %lf, %lu buffers of frame size %lu\n",
	     ochannels, samplerate, numberOfBuffers, framesPerBuffer);
    }

  return ASYS_DONE;
}

#endif

#if (!defined(ASYS_HASOUTPUT) && defined(ASYS_HASINPUT))

/****************************************************************/
/* portaudio input only: a ASYS_ACTIVE_IO case                  */
/****************************************************************/

int asys_isetup(int srate, int ichannels, int isample, 
		char * iname, int toption)

{
  double samplerate = (double) srate;
  unsigned long framesPerBuffer;
  unsigned long numberOfBuffers;
  PaDeviceID ipid;
  PaError err;

  if (asysn_portaudio_init() == ASYS_ERROR)
    return ASYS_ERROR;

  if (asysn_portaudio_incheck(&samplerate, ichannels, &ipid) == ASYS_ERROR)
    return ASYS_ERROR;

  if (asysn_portaudio_buffparam(&framesPerBuffer, 
				&numberOfBuffers, samplerate) == ASYS_ERROR)
    return ASYS_ERROR;

  err = Pa_OpenStream(&asysn_portaudio_fd, 
		      ipid, ichannels, paFloat32, NULL,
		      paNoDevice,   0, paFloat32, NULL, 
		      samplerate, framesPerBuffer, numberOfBuffers, 
		      paClipOff | paDitherOff, asysn_portaudio_callback, NULL);
		      
  if (err != paNoError)
    {
      fprintf(stderr, "Error: Cannot open PortAudio stream (%s).\n", 
	      Pa_GetErrorText(err));
      return ASYS_ERROR;
    }

  return ASYS_DONE;
}

#endif


#if (defined(ASYS_HASOUTPUT) && defined(ASYS_HASINPUT))

/****************************************************************/
/*   portaudio input and output: a ASYS_ACTIVE_IO case          */
/****************************************************************/

int asys_iosetup(int srate, int ichannels, int ochannels,
		 int isample, int osample, 
		 char * iname, char * oname, int toption)

{
  double osamplerate = (double) srate;
  unsigned long framesPerBuffer;
  unsigned long numberOfBuffers;
  PaDeviceID opid, ipid;
  double isamplerate;
  PaError err;

  if (asysn_portaudio_init() == ASYS_ERROR)
    return ASYS_ERROR;

  if (asysn_portaudio_outcheck(&osamplerate, ochannels, &opid) == ASYS_ERROR)
    return ASYS_ERROR;

  isamplerate = osamplerate;

  if (asysn_portaudio_incheck(&isamplerate, ichannels, &ipid) == ASYS_ERROR)
    return ASYS_ERROR;

  if (asysn_portaudio_buffparam(&framesPerBuffer, 
				&numberOfBuffers, isamplerate) == ASYS_ERROR)
    return ASYS_ERROR;

  err = Pa_OpenStream(&asysn_portaudio_fd, 
		      ipid, ichannels, paFloat32, NULL, 
		      opid, ochannels, paFloat32, NULL, 
		      isamplerate, framesPerBuffer, numberOfBuffers,
		      paClipOff | paDitherOff, asysn_portaudio_callback, NULL);
		      
  if (err != paNoError)
    {
      fprintf(stderr, "Error: Cannot open PortAudio stream (%s).\n", 
	      Pa_GetErrorText(err));
      return ASYS_ERROR;
    }

  return ASYS_DONE;
}

#endif

/****************************************************************/
/*       common shutdown routine for all I/O combinations       */
/****************************************************************/

void asysn_portaudio_shutdown(void)

{
  int status = Pa_StreamActive(asysn_portaudio_fd);

  if (status == 1)
    {
      Pa_StopStream(asysn_portaudio_fd);
      status = Pa_StreamActive(asysn_portaudio_fd);
    }

  if (status == 0)
    Pa_CloseStream(asysn_portaudio_fd);

  Pa_Terminate();
}

#if (defined(ASYS_HASOUTPUT)&&(!defined(ASYS_HASINPUT)))

/****************************************************************/
/*                    shuts down audio output                   */
/****************************************************************/

void asys_oshutdown(void)

{
  asysn_portaudio_shutdown();
}

#endif

#if (!defined(ASYS_HASOUTPUT)&&(defined(ASYS_HASINPUT)))

/****************************************************************/
/*              shuts down audio input device                   */
/****************************************************************/

void asys_ishutdown(void)

{
  asysn_portaudio_shutdown();
}

#endif

#if (defined(ASYS_HASOUTPUT)&&(defined(ASYS_HASINPUT)))

/****************************************************************/
/*              shuts down audio input and output device        */
/****************************************************************/

void asys_ioshutdown(void)

{
  asysn_portaudio_shutdown();
}

#endif


/****************************************************************/
/*          active audio main -- works for all I/O types        */
/****************************************************************/

void asys_main(void)

{
  asysn_portaudio_done = ASYS_DONE;

  /* initialize passive audio input driver, if needed */
  
#if (defined(ASYS_ACTIVE_IO) && !defined(ASYS_HASINPUT))

  asysn_portaudio_ibuf = NULL;
  asysn_portaudio_iptr  = 0;
  asysn_portaudio_ilast = 0;

  if ((asys_getbuf(&asysn_portaudio_ibuf, &asysn_portaudio_ilast) 
       != ASYS_DONE) || (asysn_portaudio_ilast == 0))
    return;
  asysn_portaudio_ileft = asysn_portaudio_ilast;

#endif

  /* initialize passive audio output driver, if needed */

#if (defined(ASYS_ACTIVE_IO) && !defined(ASYS_HASOUTPUT))

  asysn_portaudio_obuf = NULL;
  asysn_portaudio_optr  = 0;
  asysn_portaudio_olast = 0;

  if (asys_preamble(&asysn_portaudio_obuf,&asysn_portaudio_olast) != ASYS_DONE)
    return;

  asysn_portaudio_oleft = asysn_portaudio_olast;

#endif

  if (Pa_StartStream(asysn_portaudio_fd) != paNoError)
    {
      asysn_portaudio_done = ASYS_ERROR;
      return;
    }
  
  while (asysn_portaudio_done == ASYS_DONE)
    Pa_Sleep(ASYSN_PORTAUDIO_SLEEPMS);
  
  if (asysn_portaudio_done == ASYS_ERROR)
    Pa_AbortStream(asysn_portaudio_fd);
  else
    Pa_StopStream(asysn_portaudio_fd);
}


#if defined(ASYS_KSYNC)

/***********************************************************/
/*         initializes k-rate boundaries sync              */
/***********************************************************/

void ksyncinit()

{
}

/***********************************************************/
/*         synchronizes on k-rate boundaries               */
/***********************************************************/

float ksync()

{
  return (float) Pa_GetCPULoad(asysn_portaudio_fd);
}

#endif

