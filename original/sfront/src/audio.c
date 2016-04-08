
/*
#    Sfront, a SAOL to C translator    
#    This file: Handles asys interfaces
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

#define PORTAUDIO_OSS_TESTING      0  /* 1 to add Portaudio OSS driver */
#define DEPRECIATED_DSOUND_TESTING 0  /* 1 to add broken DSound driver */

/****************************************************************/
/* types of audio devices -- add your driver to the end         */
/*                and update DRIVER_END.                        */
/****************************************************************/

#define DRIVER_NONE         0
#define DRIVER_STD          1
#define DRIVER_NULL         2
#define DRIVER_RAW          3
#define DRIVER_WAV          4
#define DRIVER_AIF          5
#define DRIVER_HPUX         6
#define DRIVER_DSOUND       7
#define DRIVER_LINUX        8
#define DRIVER_VCDAT        9
#define DRIVER_FREEBSD     10
#define DRIVER_IRIX        11
#define DRIVER_PA_UNIX_OSS 12
#define DRIVER_PA_WIN_WMME 13
#define DRIVER_PA_WIN_DS   14
#define DRIVER_COREAUDIO   15
#define DRIVER_AUDIOUNIT   16
#define DRIVER_END         17

/****************************************************************/
/*             prints help screen for audio options             */
/****************************************************************/

void printaudiohelp(void)

{

  printf("Where C output binary sends its audio:\n");
  printf("       [-aout fname.wav]       WAV file fname.wav (16-bit)\n");
  printf("       [-aout fname.wav-24]    WAV file fname.wav (24-bit)\n");
  printf("       [-aout fname.wav-16]    WAV file fname.wav (16-bit)\n");
  printf("       [-aout fname.wav-8]     WAV file fname.wav (8-bit)\n");
  printf("       [-aout fname.aif]       AIFF file fname.aif (16-bit)\n");
  printf("       [-aout fname.aif-24]    AIFF file fname.aif (24-bit)\n");
  printf("       [-aout fname.aif-16]    AIFF file fname.aif (16-bit)\n");
  printf("       [-aout fname.aif-8]     AIFF file fname.aif (8-bit)\n");
  printf("       [-aout fname.raw]       16-bit integers to fname.raw\n");
  printf("       [-aout fname.dat]       View .dat files\n");
  printf("       [-aout std]             stdout\n");
  printf("       [-aout null]            do nothing with the output\n");
  printf("       [-aout linux]           soundcard under Linux (OSS).\n");
  printf("       [-aout pa_win_wmme]     soundcard under Windows (WMME API).\n");
  printf("       [-aout pa_win_ds]       soundcard under Windows (Direct API).\n");
  printf("       [-aout coreaudio]       soundcard under Mac OS X (CoreAudio).\n");
  printf("       [-aout audiounit]       AudioUnit plug-in (Mac OS X).\n");
  printf("       [-aout audiounit_debug] AudioUnit debugging mode.\n");
  printf("       [-aout freebsd]         soundcard under FreeBSD.\n");
  printf("       [-aout irix]            soundcard under IRIX (6.2 or later).\n");
  printf("       [-aout hpux]            soundcard under HPUX.\n");
  if (PORTAUDIO_OSS_TESTING)
    printf("       [-aout pa_unix_oss]     soundcard under UNIX OSS systems.\n");
  if (DEPRECIATED_DSOUND_TESTING)
    printf("       [-aout dsound]          depreciated Directsound driver.\n");
  printf("Where C output binary gets its audio:\n");
  printf("       [-ain fname.wav]       WAV file fname.wav\n");
  printf("       [-ain fname.aif]       AIFF file fname.aif\n");
  printf("       [-ain fname.raw]       16-bit integers from fname.raw\n");
  printf("       [-ain fname.dat]       View .dat files\n");
  printf("       [-ain std]             stdin\n");
  printf("       [-ain null]            silent input audio\n");
  printf("       [-ain linux]           soundcard under Linux (OSS).\n");
  printf("       [-ain pa_win_wmme]     soundcard under Windows (WMME API).\n");
  printf("       [-ain pa_win_ds]       soundcard under Windows (Direct API).\n");
  printf("       [-ain coreaudio]       soundcard under Mac OS X (CoreAudio).\n");
  printf("       [-ain audiounit]       AudioUnit plug-in (Mac OS X).\n");
  printf("       [-ain audiounit_debug] AudioUnit debugging mode.\n");
  printf("       [-ain freebsd]         soundcard under FreeBSD.\n");
  printf("       [-ain irix]            soundcard under IRIX (6.2 or later).\n");
  printf("       [-ain hpux]            soundcard under HPUX.\n");
  if (PORTAUDIO_OSS_TESTING)
    printf("       [-ain pa_unix_oss]     soundcard under UNIX OSS systems.\n");
}


/****************************************************************/
/*             maps -aout name to symbol                        */
/****************************************************************/

int aoutfilecheck(char * fname)

{
  int i, found;
  char * suffix;

  aoutname = dupval(fname);

  /* first, non-filename entries */

  if (!strcmp(fname,"std"))
    {
      aout = DRIVER_STD;
      aoutflow = PASSIVE_FLOW;
      return 0;
    }
  if (!strcmp(fname,"null"))
    {
      aout = DRIVER_NULL;
      aoutflow = PASSIVE_FLOW;
      return 0;
    }
  if (!strcmp(fname,"linux"))
    {
      aout = DRIVER_LINUX;
      aoutflow = PASSIVE_FLOW;
      return 0;
    }
  if (!strcmp(fname,"freebsd"))
    {
      aout = DRIVER_FREEBSD;
      aoutflow = PASSIVE_FLOW;
      return 0;
    }
  if (!strcmp(fname,"hpux"))
    {
      aout = DRIVER_HPUX;
      aoutflow = PASSIVE_FLOW;
      return 0;
    }
  if (!strcmp(fname,"irix"))
    {
      aout = DRIVER_IRIX;
      aoutflow = PASSIVE_FLOW;
      printf("IRIX driver users note:\n");
      printf("  Link using -laudio option.\n\n");
      return 0;
    }
  if (PORTAUDIO_OSS_TESTING && (!strcmp(fname,"pa_unix_oss")))
    {
      aout = DRIVER_PA_UNIX_OSS;
      aoutflow = ACTIVE_FLOW;

      if (ain != DRIVER_PA_UNIX_OSS)
	{
	  printf("Generic UNIX OSS driver note:\n");
	  printf("[1] Use the OS-specific drivers (linux, irix, etc) if\n");
	  printf("    possible; use pa_unix_oss for unsupported systems.\n");
	  printf("[2] Compile with: gcc -O3 sa.c -lm -lpthread -o sa\n\n");
	}
      return 0;
    }
  if (!strcmp(fname,"pa_win_wmme"))
    {
      aout = DRIVER_PA_WIN_WMME;
      aoutflow = ACTIVE_FLOW;

      if (ain != DRIVER_PA_WIN_WMME)
	{
	  printf("Windows MultiMedia Extension API note:\n");
	  printf("[1] MS VC++ compile: cl sa.c winmm.lib -o sa.exe\n");
	  printf("[2] gcc compile:  gcc -O3 sa.c -lm -lwinmm -o sa.exe\n");
	  printf("[3] Also try pa_win_ds, a DirectSound driver.\n\n");
	}
      return 0;
    }
  if (!strcmp(fname,"pa_win_ds"))
    {
      aout = DRIVER_PA_WIN_DS;
      aoutflow = ACTIVE_FLOW;

      if (ain != DRIVER_PA_WIN_DS)
	{
	  printf("Windows DirectSound notes:\n");
	  printf("[1] Developer Microsoft DirectSound must be installed:\n");
	  printf("    http://www.microsoft.com/directx/download.asp\n");
	  printf("[2] MS VC++ compile:\n");
	  printf("    "
		 "cl sa.c dsound.lib dxguid.lib winmm.lib -o sa.exe\n");
	  printf("[3] The sa.c file might not compile under gcc.\n");
	  printf("[4] Also try pa_win_wmme, WMME API driver.\n\n"); 
	}
      return 0;
    }
  if (!strcmp(fname,"coreaudio"))
    {
      aout = DRIVER_COREAUDIO;
      aoutflow = ACTIVE_FLOW;

      found = 0;

      for (i = 2; i < sfront_argc; i++)
	if (!strcmp("-cin", sfront_argv[i-1]))
	  {
	    found = !strcmp("coremidi", sfront_argv[i]);
	    break;
	  }

      if (ain != DRIVER_COREAUDIO)
	{
	  printf("Macintosh OS X CoreAudio notes:\n");

	  if (found)
	    {
	      printf("[1] cc -O3 sa.c -lm -framework coreaudio" 
		     " -framework coremidi \\\n");
	      printf("    -framework corefoundation -o sa\n");
	    }
	  else
	    printf("[1] cc -O3 sa.c -lm -framework CoreAudio -o sa\n");

	  printf("\n");
	}

      return 0;
    }
  if ((!strcmp(fname,"audiounit")) || (!strcmp(fname,"audiounit_debug"))) 
    {
      aout = DRIVER_AUDIOUNIT;
      aoutflow = ACTIVE_FLOW;

      adebug |= (strcmp(fname,"audiounit") != 0);
      reentrant = 1;
      nomain = 1;

      if (ain != DRIVER_AUDIOUNIT)
	{
	  printf("Macintosh OS X AudioUnit notes:\n");
	  printf("[1] cc -O3 sa.c -lm -framework AudioUnit -framework CoreAudio"
		 " -framework CoreServices -bundle -o sa\n");
	  printf("\n");
	}

      return 0;
    }
  if (DEPRECIATED_DSOUND_TESTING && (!strcmp(fname,"dsound")))
    {
      aout = DRIVER_DSOUND;
      aoutflow = PASSIVE_FLOW;

      printf("Dsound driver users note:\n");
      printf("[1] Try the newer pa_win_wmme or pa_win_ds drivers.\n");
      printf("[2] Dsound driver does not compile under gcc.\n");
      printf("[3] Link dxguid.lib and dsound.lib when compiling sa.c.\n");
      printf("[4] Interactive mode (i.e. -cin win32) not working well.\n\n");

      return 0;
    }

  /* then, filename entries */

  if (strstr(fname,".raw") != NULL)
    {
      aout = DRIVER_RAW;
      aoutflow = PASSIVE_FLOW;
      return 0;
    }
  if ((suffix = strstr(fname,".wav")) != NULL)
    {
      aout = DRIVER_WAV;
      aoutflow = PASSIVE_FLOW;
      reentrant = 0;

      if (strlen(suffix) == 4)  /* no wordsize specified */
	return 0;

      if ((strlen(suffix) == 5) || (strlen(suffix) >= 8) || (suffix[4] != '-')) 
	{
	  printf("Error: Audio output filename syntax -aout %s is incorrect.\n",
		 fname); 
	  printf("       Try filename.wav-16 or filename.wav-24 or filename.wav-8.\n\n");

	  noerrorplace();  /* never returns */
	}

      *(strrchr(aoutname, '-')) = '\0';   /* can never return null */

      if ((strlen(suffix) == 6) && (suffix[5] == '8'))
	{
	  outfile_wordsize = WORDSIZE_8BIT;
	  return 0;
	}

      if ((strlen(suffix) == 7) && (suffix[5] == '1') && (suffix[6] == '6'))
	{
	  outfile_wordsize = WORDSIZE_16BIT;
	  return 0;
	}

      if ((strlen(suffix) == 7) && (suffix[5] == '2') && (suffix[6] == '4'))
	{
	  outfile_wordsize = WORDSIZE_24BIT;
	  return 0;
	}

      printf("Error: Audio output filename syntax -aout %s is incorrect.\n",
	     fname);
      printf("       Try filename.wav-16 or filename.wav-24 or filename.wav-8.\n\n");

      noerrorplace();  /* never returns */
    }
  if ((suffix = strstr(fname,".aif")) != NULL)
    {
      aout = DRIVER_AIF;
      aoutflow = PASSIVE_FLOW;

      if (strstr(fname,".aiff"))   /* solves parsing "aiff" below */
	{
	  suffix = strstr(fname,".aiff");
	  suffix++;
	}

      if (strlen(suffix) == 4)  /* no wordsize specified */
	return 0;

      if ((strlen(suffix) == 5) || (strlen(suffix) >= 8) || (suffix[4] != '-')) 
	{
	  printf("Error: Audio output filename syntax -aout %s is incorrect.\n",
		 fname);
	  printf("       Try filename.aif-16 or filename.aif-24 or filename.aif-8.\n\n");

	  noerrorplace();  /* never returns */
	}

      *(strrchr(aoutname, '-')) = '\0';   /* can never return null */

      if ((strlen(suffix) == 6) && (suffix[5] == '8'))
	{
	  outfile_wordsize = WORDSIZE_8BIT;
	  return 0;
	}

      if ((strlen(suffix) == 7) && (suffix[5] == '1') && (suffix[6] == '6'))
	{
	  outfile_wordsize = WORDSIZE_16BIT;
	  return 0;
	}

      if ((strlen(suffix) == 7) && (suffix[5] == '2') && (suffix[6] == '4'))
	{
	  outfile_wordsize = WORDSIZE_24BIT;
	  return 0;
	}

      printf("Error: -aout filename specification syntax %s is incorrect.\n",
	     fname); 
      printf("       Try filename.aif-16 or filename.aif-24 or filename.aif-8.\n\n");

      noerrorplace();  /* never returns */

      return 0;
    }
  if (strstr(fname,".dat") != NULL)
    {
      aout = DRIVER_VCDAT;
      aoutflow = PASSIVE_FLOW;
      return 0;
    }

  return 1;
}

/****************************************************************/
/*             maps -ain commands to symbol                     */
/****************************************************************/

int ainfilecheck(char * fname)

{
  ainname = dupval(fname);

  /* first, non-filename entries */

  if (!strcmp(fname,"std"))
    {
      ain = DRIVER_STD;
      ainflow = PASSIVE_FLOW;
      ainlatency = LOW_LATENCY_DRIVER; 
      return 0;
    }
  if (!strcmp(fname,"null"))
    {
      ain = DRIVER_NULL;
      ainflow = PASSIVE_FLOW;
      ainlatency = HIGH_LATENCY_DRIVER; 
      return 0;
    }
  if (!strcmp(fname,"linux"))
    {
      ain = DRIVER_LINUX;
      ainflow = PASSIVE_FLOW;
      ainlatency = LOW_LATENCY_DRIVER; 
      return 0;
    }
  if (!strcmp(fname,"freebsd"))
    {
      ain = DRIVER_FREEBSD;
      ainflow = PASSIVE_FLOW;
      ainlatency = LOW_LATENCY_DRIVER; 
      return 0;
    }
  if (!strcmp(fname,"irix"))
    {
      ain = DRIVER_IRIX;
      ainflow = PASSIVE_FLOW;
      ainlatency = LOW_LATENCY_DRIVER; 
      return 0;
    }
  if (!strcmp(fname,"hpux"))
    {
      ain = DRIVER_HPUX;
      ainflow = PASSIVE_FLOW;
      ainlatency = LOW_LATENCY_DRIVER; 
      return 0;
    }
  if (PORTAUDIO_OSS_TESTING && (!strcmp(fname,"pa_unix_oss")))
    {
      ain = DRIVER_PA_UNIX_OSS;
      ainflow = ACTIVE_FLOW;
      ainlatency = LOW_LATENCY_DRIVER; 

      if (aout != DRIVER_PA_UNIX_OSS)
	{
	  printf("Generic UNIX OSS driver note:\n");
	  printf("[1] Use the OS-specific drivers (linux, irix, etc) if\n");
	  printf("    possible; use pa_unix_oss for unsupported systems.\n");
	  printf("[2] Compile with: gcc -O3 sa.c -lm -lpthread -o sa\n\n");
	}
      return 0;
    }
  if (!strcmp(fname,"pa_win_wmme"))
    {
      ain = DRIVER_PA_WIN_WMME;
      ainflow = ACTIVE_FLOW;
      ainlatency = LOW_LATENCY_DRIVER; 

      if (aout != DRIVER_PA_WIN_WMME)
	{
	  printf("Windows MultiMedia Extension API note:\n");
	  printf("[1] MS VC++ compile: cl sa.c winmm.lib -o sa.exe\n");
	  printf("[2] gcc compile:  gcc -O3 sa.c -lm -lwinmm -o sa.exe\n");
	  printf("[3] Also try pa_win_ds, the DirectSound driver.\n\n");
	}
      return 0;
    }
  if (!strcmp(fname,"pa_win_ds"))
    {
      ain = DRIVER_PA_WIN_DS;
      ainflow = ACTIVE_FLOW;
      ainlatency = LOW_LATENCY_DRIVER; 

      if (aout != DRIVER_PA_WIN_DS)
	{
	  printf("Windows DirectSound notes:\n");
	  printf("[1] Developer Microsoft DirectSound must be installed:\n");
	  printf("    http://www.microsoft.com/directx/download.asp\n");
	  printf("[2] MS VC++ compile:\n");
	  printf("    "
		 "cl sa.c dsound.lib dxguid.lib winmm.lib -o sa.exe\n");
	  printf("[3] The sa.c file might not compile under gcc.\n");
	  printf("[4] Also try pa_win_wmme, the WMME API driver.\n\n"); 
	}
      return 0;
    }
  if (!strcmp(fname,"coreaudio"))
    {
      ain = DRIVER_COREAUDIO;
      ainflow = ACTIVE_FLOW;
      ainlatency = LOW_LATENCY_DRIVER; 

      if (aout != DRIVER_COREAUDIO)
	{
	  printf("Macintosh OS X CoreAudio notes:\n");
          printf("[1] cc -O3 sa.c -lm -framework CoreAudio -o sa\n\n");
	}
      return 0;
    }
  if ((!strcmp(fname,"audiounit")) || (!strcmp(fname,"audiounit_debug"))) 
    {
      ain = DRIVER_AUDIOUNIT;
      ainflow = ACTIVE_FLOW;
      ainlatency = LOW_LATENCY_DRIVER;

      adebug |= (strcmp(fname,"audiounit") != 0);

      if (aout != DRIVER_AUDIOUNIT)
	{
	  printf("Macintosh OS X AudioUnit notes:\n");
	  printf("[1] cc -O3 sa.c -lm -framework AudioUnit -framework CoreAudio"
		 " -framework CoreServices -bundle -o sa\n");
	  printf("\n");
	}

      return 0;
    }

  /* then, filename entries */

  if (strstr(fname,".raw") != NULL)
    {
      ain = DRIVER_RAW;
      ainflow = PASSIVE_FLOW;
      ainlatency = HIGH_LATENCY_DRIVER; 
      return 0;
    }
  if (strstr(fname,".wav") != NULL)
    {
      ain = DRIVER_WAV;
      ainflow = PASSIVE_FLOW;
      ainlatency = HIGH_LATENCY_DRIVER; 
      return 0;
    }
  if (strstr(fname,".aif") != NULL)
    {
      ain = DRIVER_AIF;
      ainflow = PASSIVE_FLOW;
      ainlatency = HIGH_LATENCY_DRIVER; 
      return 0;
    }
  if (strstr(fname,".dat") != NULL)
    {
      ain = DRIVER_VCDAT;
      ainflow = PASSIVE_FLOW;
      ainlatency = HIGH_LATENCY_DRIVER; 
      return 0;
    }

  return 1;
}

/****************************************************************/
/*             returns sample type for output                  */
/****************************************************************/

int makeaudiotypeout(int anum)

{
  int ret = SAMPLE_SHORT;

  switch (anum) {
  case DRIVER_PA_UNIX_OSS:
  case DRIVER_PA_WIN_WMME:
  case DRIVER_PA_WIN_DS:
  case DRIVER_COREAUDIO:
  case DRIVER_AUDIOUNIT:
  case DRIVER_VCDAT:
  case DRIVER_WAV:
  case DRIVER_AIF:
    ret = SAMPLE_FLOAT;
    break;
  default:
    break;
  }
  return ret;
}

/****************************************************************/
/*             returns sample type for input                    */
/****************************************************************/

int makeaudiotypein(int anum)

{
  int ret = SAMPLE_SHORT;

  switch (anum) {
  case DRIVER_PA_UNIX_OSS:
  case DRIVER_PA_WIN_WMME:
  case DRIVER_PA_WIN_DS:
  case DRIVER_COREAUDIO:
  case DRIVER_AUDIOUNIT:
  case DRIVER_VCDAT:
  case DRIVER_WAV:
  case DRIVER_AIF:
    ret = SAMPLE_FLOAT;
    break;
  default:
    break;
  }
  return ret;
}

/****************************************************************/
/*  audio output driver request function for ksync duties       */
/****************************************************************/

int makeaoutsync(int anum)

{
  int ret;

  switch (anum) {
  case DRIVER_LINUX:     /* included in audio driver file */  
  case DRIVER_FREEBSD:    
  case DRIVER_DSOUND:
  case DRIVER_PA_UNIX_OSS:
  case DRIVER_PA_WIN_WMME:
  case DRIVER_PA_WIN_DS:
  case DRIVER_COREAUDIO:
  case DRIVER_AUDIOUNIT:
    ret = 1;
    break;
  default:               /* use standard UNIX version */
    ret = 0;
    break;
  }
  return ret;
}

/****************************************************************/
/*   audio input driver request function for ksync duties       */
/****************************************************************/

int makeainsync(int anum)

{
  int ret;

  switch (anum) {
  case DRIVER_PA_UNIX_OSS:    /* only here as an example, may not work */
  case DRIVER_PA_WIN_WMME:
  case DRIVER_PA_WIN_DS:
  case DRIVER_COREAUDIO:
  case DRIVER_AUDIOUNIT:
    ret = 1;
    break;
  default:               
    ret = 0;
    break;
  }
  return ret;
}

/****************************************************************/
/*        preferred time option for audio output driver         */
/****************************************************************/

int makeaouttimedefault(int anum)

{
  int ret = RENDER;  /* assign PLAYBACK or TIMESYNC below */

  switch (anum) {
  case DRIVER_LINUX:
  case DRIVER_FREEBSD:
  case DRIVER_IRIX:
  case DRIVER_HPUX:
  case DRIVER_DSOUND:
  case DRIVER_PA_UNIX_OSS:
  case DRIVER_PA_WIN_WMME:
  case DRIVER_PA_WIN_DS:
  case DRIVER_COREAUDIO:
  case DRIVER_AUDIOUNIT:
    ret = PLAYBACK;
    break;
  }

  return ret;
}


/****************************************************************/
/*        preferred time option for audio input driver          */
/****************************************************************/

int makeaintimedefault(int anum)

{
  int ret = RENDER;  /* assign PLAYBACK or TIMESYNC below */

  switch (anum) {
  case DRIVER_LINUX:
  case DRIVER_FREEBSD:
  case DRIVER_IRIX:
  case DRIVER_HPUX:
  case DRIVER_DSOUND:
  case DRIVER_PA_UNIX_OSS:
  case DRIVER_PA_WIN_WMME:
  case DRIVER_PA_WIN_DS:
  case DRIVER_COREAUDIO:
  case DRIVER_AUDIOUNIT:
    ret = PLAYBACK;
    break;
  }

  return ret;
}


/****************************************************************/
/*   returns 1 if audio output driver needs rread()/rwrite()    */
/****************************************************************/

int makeaoutrobust(int anum)

{
  int ret = 0;  /* assign 1 to binary file drivers below */

  switch (anum) {
  case DRIVER_STD:
  case DRIVER_RAW:
  case DRIVER_WAV:
  case DRIVER_AIF:
    ret = 1;
    break;
  }

  return ret;
}

/****************************************************************/
/*   returns 1 if audio input driver needs rread()/rwrite()    */
/****************************************************************/

int makeainrobust(int anum)

{
  int ret = 0;  /* assign 1 to binary file drivers below */

  switch (anum) {
  case DRIVER_STD:
  case DRIVER_RAW:
  case DRIVER_WAV:
  case DRIVER_AIF:
    ret = 1;
    break;
  }

  return ret;
}


/****************************************************************/
/*   returns number of channels and sample rate for input       */
/*   important points:                                          */
/*        [1] only called when SAOL program doesn't specify     */
/*            global parameters srate and/or inchannels.        */
/*        [2] If sample rate is unknown, the *srate value       */
/*            will be -1, if known it will be 0. If the         */
/*            If the number of input channels is unknown,       */ 
/*            *inchannels will be -1, if known it will be 0.    */
/*        [3] if you know an unknown value (for example, by     */
/*            opening a file and reading its preamble)          */
/*            assign into its variable. assignment to known     */
/*            quantities is ignored.                            */
/*        [4] if you don't assign unknown quantities, then      */
/*            *srate = 32000 and *inchannels = 0 are defaults.  */
/*        [5] if you're doing heavy lifting to figure these     */
/*            values out, add to the extern list that precedes  */
/*            the function, and add a function AT THE END OF    */ 
/*            THE FILE. use the naming convention               */
/*            ainparams_drivername().                           */
/*        [6] this routine is called after global variable      */
/*            char * ainname is set by ainfilecheck             */
/****************************************************************/

extern void ainparams_wav(int * srate, int * inchannels);
extern void ainparams_aif(int * srate, int * inchannels);
extern void ainparams_info(int * srate, int * inchannels);

void makeainparams(int anum, int * srate, int * inchannels)

{

  switch (anum) {
  case DRIVER_WAV:
    ainparams_wav(srate, inchannels);
    break;
  case DRIVER_AIF:
    ainparams_aif(srate, inchannels);
    break;
  case DRIVER_RAW:
  case DRIVER_STD:
    ainparams_info(srate, inchannels);
    break;
  case DRIVER_VCDAT:
    *inchannels = 1;
    if (*srate < 0)
      warningmessage(0,"Assuming sample rate of -ain file is 44100Hz");
    *srate = 44100;
    break;
  case DRIVER_NULL:   
  case DRIVER_HPUX:   
  case DRIVER_IRIX:   
  case DRIVER_DSOUND: 
  case DRIVER_LINUX:  
  case DRIVER_FREEBSD:
  case DRIVER_PA_UNIX_OSS:
  case DRIVER_PA_WIN_WMME:
  case DRIVER_PA_WIN_DS:
  case DRIVER_COREAUDIO:
  case DRIVER_AUDIOUNIT:
    /* modern soundcards optimized to this configuration */
    *inchannels = 2;
    *srate = 44100;
    break;
  default:
    break;
  }

}

/****************************************************************/
/*         returns 1 if aout driver sends audio on stdout       */
/****************************************************************/

int stdoutused(int anum)

{
  return (anum == DRIVER_STD);
}

/****************************************************************/
/*         returns 1 if aout driver gets audio from stdin       */
/****************************************************************/

int stdinused(int anum)

{
  return (anum == DRIVER_STD);
}

/****************************************************************/
/* returns 1 if run-time printouts should go to wiretap logfile */
/****************************************************************/

int wiretap_logging(int anum)

{
  return (anum == DRIVER_AUDIOUNIT);
}

/****************************************************************/
/* returns 1 if the audio driver uses the reflection variables */
/****************************************************************/

int adriver_reflection(int anum)

{
  return (anum == DRIVER_AUDIOUNIT);
}

/****************************************************************/
/*                 sets defaults for audio system               */
/****************************************************************/

void audiodefaults(void)

{
  if (!ain)
    {
      ain = DRIVER_WAV;
      ainname = dupval("input.wav");
      ainlatency = HIGH_LATENCY_DRIVER; 
      ainflow = PASSIVE_FLOW;
    }
  if (!aout)
    {
      aout = DRIVER_WAV;
      aoutname = dupval("output.wav");
      aoutflow = PASSIVE_FLOW;
    }
  if (timeoptions == UNKNOWN)
    {
      timeoptions = makeaouttimedefault(aout);
      if (makeaintimedefault(ain) > timeoptions)
	timeoptions = makeaintimedefault(ain);
    }
}

/****************************************************************/
/*         portaudio driver files common to all platforms       */
/****************************************************************/

void makeportaudiocommon(void)

{
  makepa_porthdr();
  makepa_hosthdr();
  makepa_tracehdr();
  makepa_lib();
  makeportaudio();
}


extern void audiounit_filemaker(void);
extern void audiounit_typemaker(void);

/****************************************************************/
/*    calls function that includes driver C code in sa.c file   */
/****************************************************************/

void makeaudiodriver(int anum)

{
  switch (anum) {
  case DRIVER_STD:
    makestd();
    break;
  case DRIVER_NULL:
    makenull();
    break;
  case DRIVER_RAW:
    makeraw();
    break;
  case DRIVER_LINUX:
    makelinux();
    break;
  case DRIVER_FREEBSD:
    makefreebsd();
    break;
  case DRIVER_HPUX:
    makehpux();
    break;
  case DRIVER_IRIX:
    makeirix();
    break;
  case DRIVER_DSOUND:
    makedsound();
    break;
  case DRIVER_WAV:
    makewav();
    break;
  case DRIVER_AIF:
    makeaif();
    break;
  case DRIVER_VCDAT:
    makevcdat();
    break;
  case DRIVER_PA_UNIX_OSS:
    makeportaudiocommon();
    makepa_unix_oss();
    break;
  case DRIVER_PA_WIN_WMME:
    makeportaudiocommon();
    makepa_win_wmme();
    break;
  case DRIVER_COREAUDIO:
    makecoreaudio();
    break;
  case DRIVER_AUDIOUNIT:
    audiounit_typemaker();
    if (adebug)
      makewiretap();
    makeaudiounit();
    audiounit_filemaker();
    break;
  case DRIVER_PA_WIN_DS:
    makeportaudiocommon();
    makepa_dshdr();
    makepa_dswrap();
    makepa_dsound();
    break;
  default:
    internalerror("audio.c","makeaudiodriver()");
  }
}

/****************************************************************/
/*           The Beginning of the End of the File               */
/*       The start of list of ainparams_mydriver functions      */
/*            New functions pre-pended to the end.              */
/****************************************************************/

/****************************************************************/
/*           Code block for the ainparams_info() function        */
/****************************************************************/


/*********************************************************/
/*          the function called by makeainparams()       */
/*********************************************************/

void ainparams_info(int * srate, int * inchannels)
     
{
  FILE * fd = NULL;     /* input file pointer */
  char * iname;
  int newint,i;

  if (ainname && (ain == DRIVER_RAW))
    {    
      vmcheck(iname = calloc(6+strlen(ainname), sizeof(char)));
      strcat(strcat(iname,ainname),".info");
      fd = fopen(iname, "r");
      free(iname);
    }
  else
    {
      if (ain == DRIVER_RAW)
	fd = fopen("input.raw.info","rb");
      else
	if (ain == DRIVER_STD)
	  fd = fopen("stdin.info","rb");
    }
  if (!fd)
    return;


  if (1 != (i = fscanf(fd, "%i", &newint)))
    {
      fclose(fd);
      return;
    }

  if (newint > 0)
    *srate = newint;

  if (1 != fscanf(fd, "%i", &newint))
    {
      fclose(fd);
      return;
    }

  if (newint > 0)
    *inchannels = newint;

  fclose(fd);
  return;

}


/****************************************************************/
/*           Code block for the ainparams_wav() function        */
/****************************************************************/


#define AINPARAMS_WAV_MATCH   0
#define AINPARAMS_WAV_EOF     1
#define AINPARAMS_WAV_NOMATCH 2

#define AINPARAMS_WAV_DONE    0
#define AINPARAMS_WAV_ERROR   1

/*********************************************************/
/*        flushes next block of WAV bytes                */
/*********************************************************/

int ainparams_wav_flushbytes(FILE * fd, int numbytes)

{
  unsigned char c;

  while (numbytes > 0)
    {
      if (fread(&c, sizeof(char), 1, fd) != 1)
	return AINPARAMS_WAV_ERROR;
      numbytes--;
    }
  return AINPARAMS_WAV_DONE;

}

  
/***********************************************************/
/*  checks byte stream for AIFF/WAV cookie --              */
/***********************************************************/

int ainparams_wav_soundtypecheck(FILE * fd, char * d)

{
  char c[4];

  if (fread(c, sizeof(char), 4, fd) != 4)
    return AINPARAMS_WAV_EOF;
  if (strncmp(c,d,4))
    return AINPARAMS_WAV_NOMATCH;
  return AINPARAMS_WAV_MATCH;
}


/*********************************************************/
/*            gets next block of WAV bytes               */
/*********************************************************/

int ainparams_wav_getbytes(FILE * fd, unsigned char * c, int numbytes)

{
  if ((int)fread(c, sizeof(char), numbytes, fd) != numbytes)
    return AINPARAMS_WAV_ERROR;
  return AINPARAMS_WAV_DONE;
}


/*********************************************************/
/*     converts byte stream to an unsigned int          */
/*********************************************************/

int ainparams_wav_getint(FILE * fd, int numbytes, unsigned int * ret)

{
  unsigned char c[4];

  if (numbytes > 4)
    return AINPARAMS_WAV_ERROR;
  if (AINPARAMS_WAV_DONE != ainparams_wav_getbytes(fd, &c[0],numbytes))
    return AINPARAMS_WAV_ERROR;
  switch (numbytes) {
  case 4:
    *ret  =  (unsigned int)c[0];
    *ret |=  (unsigned int)c[1] << 8;
    *ret |=  (unsigned int)c[2] << 16;
    *ret |=  (unsigned int)c[3] << 24;
    return AINPARAMS_WAV_DONE;
  case 3:
    *ret  =  (unsigned int)c[0];
    *ret |=  (unsigned int)c[1] << 8;
    *ret |=  (unsigned int)c[2] << 16;
    return AINPARAMS_WAV_DONE;
  case 2:
    *ret  =  (unsigned int)c[0];
    *ret |=  (unsigned int)c[1] << 8;
    return AINPARAMS_WAV_DONE;
  case 1:
    *ret = (unsigned int)c[0];
    return AINPARAMS_WAV_DONE;
  default:
    return AINPARAMS_WAV_ERROR;
  }

}

/*********************************************************/
/*          reads the header of a wave file              */
/*********************************************************/

int ainparams_wav_reader(FILE * fd, int * srate, int * inchannels)

{
  unsigned int i, cookie;
  int len;

  if ((ainparams_wav_soundtypecheck(fd, "RIFF")!= AINPARAMS_WAV_MATCH) ||
      (ainparams_wav_flushbytes(fd, 4)!= AINPARAMS_WAV_DONE) ||
      (ainparams_wav_soundtypecheck(fd, "WAVE")!= AINPARAMS_WAV_MATCH))
    return AINPARAMS_WAV_ERROR;
  
  while ((cookie = ainparams_wav_soundtypecheck(fd, "fmt "))
	 != AINPARAMS_WAV_MATCH)
    {
      if ((cookie == AINPARAMS_WAV_EOF) || 
	  (ainparams_wav_getint(fd, 4, &i) != AINPARAMS_WAV_DONE))
	return AINPARAMS_WAV_ERROR;
      if (ainparams_wav_flushbytes(fd, i)!= AINPARAMS_WAV_DONE)
	return AINPARAMS_WAV_ERROR;
    }

  if (ainparams_wav_getint(fd, 4, &i) != AINPARAMS_WAV_DONE)
    return AINPARAMS_WAV_ERROR;

  len = i;
  if (((len -= 16) < 0) ||
      (ainparams_wav_getint(fd, 2, &i) != AINPARAMS_WAV_DONE))
    return AINPARAMS_WAV_ERROR;

  if (i != 1)   /* PCM magic number */
    return AINPARAMS_WAV_ERROR;

  if (ainparams_wav_getint(fd, 2, &i) != AINPARAMS_WAV_DONE)
    return AINPARAMS_WAV_ERROR;

  *inchannels = i;

  if (ainparams_wav_getint(fd, 4, &i) != AINPARAMS_WAV_DONE)
    return AINPARAMS_WAV_ERROR;
  
  *srate = i;

  return AINPARAMS_WAV_DONE;
}


/*********************************************************/
/*          the function called by makeainparams()       */
/*********************************************************/

void ainparams_wav(int * srate, int * inchannels)
     
{
  FILE * fd;     /* input file pointer */
  
  if (ainname)
    fd = fopen(ainname,"rb");
  else
    fd = fopen("output.wav","rb");
  
  if (!fd)
    {
      fprintf(stderr, "Warning: -ain WAV file %s cannot be opened.\n",
	      ainname ? ainname : "(default name: output.wav)");
      return;
    }

  if (ainparams_wav_reader(fd, srate, inchannels) == AINPARAMS_WAV_ERROR)
    fprintf(stderr, "Warning: -ain WAV file %s cannot be parsed.\n",
	    ainname ? ainname : "(default name: output.wav)");

  fclose(fd);
}

#undef AINPARAMS_WAV_MATCH  
#undef AINPARAMS_WAV_EOF 
#undef AINPARAMS_WAV_NOMATCH 
#undef AINPARAMS_WAV_DONE    
#undef AINPARAMS_WAV_ERROR   


/****************************************************************/
/*           Code block for the ainparams_aif() function        */
/****************************************************************/


#define AINPARAMS_AIF_MATCH   0
#define AINPARAMS_AIF_EOF     1
#define AINPARAMS_AIF_NOMATCH 2

#define AINPARAMS_AIF_DONE    0
#define AINPARAMS_AIF_ERROR   1
    

/***********************************************************/
/*  checks byte stream for AIFF cookie --                 */
/***********************************************************/

int ainparams_aif_soundtypecheck(FILE * fd, char * d)

{
  char c[4];

  if (fread(c, sizeof(char), 4, fd) != 4)
    return AINPARAMS_AIF_EOF;
  if (strncmp(c,d,4))
    return AINPARAMS_AIF_NOMATCH;
  return AINPARAMS_AIF_MATCH;
}
  
/*********************************************************/
/*        flushes next block of AIFF bytes                */
/*********************************************************/

int ainparams_aif_flushbytes(FILE * fd, int numbytes)

{
  unsigned char c;

  while (numbytes > 0)
    {
      if (fread(&c, sizeof(char), 1, fd) != 1)
	return AINPARAMS_AIF_ERROR;
      numbytes--;
    }
  return AINPARAMS_AIF_DONE;

}

/*********************************************************/
/*            gets next block of AIFF bytes               */
/*********************************************************/

int ainparams_aif_getbytes(FILE * fd, unsigned char * c, int numbytes)

{
  if ((int)fread(c, sizeof(char), numbytes, fd) != numbytes)
    return AINPARAMS_AIF_ERROR;
  return AINPARAMS_AIF_DONE;
}

/*********************************************************/
/*     converts byte stream to an unsigned int          */
/*********************************************************/

int ainparams_aif_getint(FILE * fd, int numbytes, unsigned int * ret)

{
  unsigned char c[4];

  if (numbytes > 4)
    return AINPARAMS_AIF_ERROR;
  if (AINPARAMS_AIF_DONE != ainparams_aif_getbytes(fd, &c[0],numbytes))
    return AINPARAMS_AIF_ERROR;
  switch (numbytes) {
  case 4:
    *ret  =  (unsigned int)c[3];
    *ret |=  (unsigned int)c[2] << 8;
    *ret |=  (unsigned int)c[1] << 16;
    *ret |=  (unsigned int)c[0] << 24;
    return AINPARAMS_AIF_DONE;
  case 3:
    *ret  =  (unsigned int)c[2];
    *ret |=  (unsigned int)c[1] << 8;
    *ret |=  (unsigned int)c[0] << 16;
    return AINPARAMS_AIF_DONE;
  case 2:
    *ret  =  (unsigned int)c[1];
    *ret |=  (unsigned int)c[0] << 8;
    return AINPARAMS_AIF_DONE;
  case 1:
    *ret = (unsigned int)c[0];
    return AINPARAMS_AIF_DONE;
  default:
    return AINPARAMS_AIF_ERROR;
  }

}
    
/*********************************************************/
/*     converts byte stream to an int                   */
/*********************************************************/

int ainparams_aif_getsint(FILE * fd, int numbytes, int * ret)

{
  unsigned char c[4];

  if (numbytes > 4)
    return AINPARAMS_AIF_ERROR;
  if (AINPARAMS_AIF_DONE != ainparams_aif_getbytes(fd, &c[0],numbytes))
    return AINPARAMS_AIF_ERROR;
  switch (numbytes) {
  case 4:
    *ret  =  (int)c[3];
    *ret |=  (int)c[2] << 8;
    *ret |=  (int)c[1] << 16;
    *ret |=  (int)c[0] << 24;
    return AINPARAMS_AIF_DONE;
  case 3:
    *ret  =  (int)c[2];
    *ret |=  (int)c[1] << 8;
    *ret |=  (int)c[0] << 16;
    return AINPARAMS_AIF_DONE;
  case 2:
    *ret  =  (int)c[1];
    *ret |=  (int)c[0] << 8;
    return AINPARAMS_AIF_DONE;
  case 1:
    *ret = (int)c[0];
    return AINPARAMS_AIF_DONE;
  default:
    return AINPARAMS_AIF_ERROR;
  }

}
    
/*********************************************************/
/*          reads the header of an AIF file              */
/*********************************************************/

int ainparams_aif_reader(FILE * fd, int * srate, int * inchannels)

{
  unsigned int i, m;
  int e;
  unsigned char c[4];


  if (ainparams_aif_soundtypecheck(fd,"FORM")!= AINPARAMS_AIF_MATCH)
    return AINPARAMS_AIF_ERROR;
  if (ainparams_aif_flushbytes(fd, 4)!= AINPARAMS_AIF_DONE)
    return AINPARAMS_AIF_ERROR;
  if (ainparams_aif_soundtypecheck(fd, "AIFF")!= AINPARAMS_AIF_MATCH)
    return AINPARAMS_AIF_ERROR;
  if (ainparams_aif_getbytes(fd, &c[0],4)!= AINPARAMS_AIF_DONE)
    return AINPARAMS_AIF_ERROR;

  while (strncmp((char *) c,"COMM",4))
    {
      if (ainparams_aif_getint(fd,4, &i) != AINPARAMS_AIF_DONE)
	return AINPARAMS_AIF_ERROR;
      if (ainparams_aif_flushbytes(fd, i + (i % 2))!= AINPARAMS_AIF_DONE)
	return AINPARAMS_AIF_ERROR;
      if (ainparams_aif_getbytes(fd, &c[0], 4)!= AINPARAMS_AIF_DONE)
	return AINPARAMS_AIF_ERROR;
    }

  if (ainparams_aif_flushbytes(fd, 4)!= AINPARAMS_AIF_DONE) /* length */
    return AINPARAMS_AIF_ERROR;
  if (ainparams_aif_getint(fd, 2, &i) != AINPARAMS_AIF_DONE)
    return AINPARAMS_AIF_ERROR;
  if (i > 0)
    *inchannels = i;

  if (ainparams_aif_flushbytes(fd, 4)!= AINPARAMS_AIF_DONE) /* frames */
    return AINPARAMS_AIF_ERROR;
  if (ainparams_aif_getint(fd, 2, &i) != AINPARAMS_AIF_DONE)
    return AINPARAMS_AIF_ERROR;
  if ((i < 8) || (i > 24))
    {
      fprintf(stderr,"Error: Can't handle %i bit data\n",i);
      return AINPARAMS_AIF_ERROR;
    }
  if (ainparams_aif_getsint(fd, 2, &e) != AINPARAMS_AIF_DONE)
    return AINPARAMS_AIF_ERROR;
  if (ainparams_aif_getint(fd, 4, &m) != AINPARAMS_AIF_DONE)
    return AINPARAMS_AIF_ERROR;

  i = (unsigned int)(0.5+(m*exp(log(2)*(e - 16414.0F))));
  if (i > 0)
    *srate = i;

  return AINPARAMS_AIF_DONE;
}

/*********************************************************/
/*          the function called by makeainparams()       */
/*********************************************************/

void ainparams_aif(int * srate, int * inchannels)
     
{
  FILE * fd;     /* input file pointer */
  
  if (ainname)
    fd = fopen(ainname,"rb");
  else
    fd = fopen("output.aif","rb");
  
  if (!fd)
    {
      fprintf(stderr, "Warning: -ain AIF file %s cannot be opened.\n",
	      ainname ? ainname : "(default name: output.aif)");
      return;
    }

  if (ainparams_aif_reader(fd, srate, inchannels) == AINPARAMS_AIF_ERROR)
    fprintf(stderr, "Warning: -ain AIF file %s cannot be parsed.\n",
	    ainname ? ainname : "(default name: output.aif)");

  fclose(fd);
}

#undef AINPARAMS_AIF_MATCH  
#undef AINPARAMS_AIF_EOF 
#undef AINPARAMS_AIF_NOMATCH 
#undef AINPARAMS_AIF_DONE    
#undef AINPARAMS_AIF_ERROR   



/****************************************************************/
/*         end of list of ainparams_mydriver functions          */
/****************************************************************/


/****************************************************************/
/*                 The Final Section of the File                */
/*          Specialized driver-specific functionality           */
/*       Functions for new drivers pre-pended to the end.       */
/****************************************************************/

/****************************************************************/
/* Audiounit driver:  sa.c defines for component type, subtype  */
/****************************************************************/

void audiounit_typemaker(void) 

{
  int lc = 0;

  z[lc++]="";
  z[lc++]="/* component definitions generated by sfront (audio.c) */";
  z[lc++]="";

  if (!strcmp(au_component_type, "aufx"))
    z[lc++]="#define ASYS_AUDIOUNIT_COMP_TYPE kAudioUnitType_Effect";

  if (!strcmp(au_component_type, "aumf"))
    z[lc++]="#define ASYS_AUDIOUNIT_COMP_TYPE kAudioUnitType_MusicEffect";

  if (!strcmp(au_component_type, "aumu"))
    z[lc++]="#define ASYS_AUDIOUNIT_COMP_TYPE kAudioUnitType_MusicDevice";

  /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
  /* if this sprintf is changed, also change audiounit_filemaker() printf */
  /*----------------------------------------------------------------------*/

  mz(lc);
  sprintf(z[lc++], "#define ASYS_AUDIOUNIT_COMP_SUBTYPE '%c%c%c%c'",
	  au_component_subtype[0], au_component_subtype[1], 
	  au_component_subtype[2], au_component_subtype[3]);

  /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
  /* if this sprintf is changed, also change audiounit_filemaker() printf */
  /*----------------------------------------------------------------------*/

  mz(lc);
  sprintf(z[lc++], "#define ASYS_AUDIOUNIT_COMP_MANU '%c%c%c%c'",
	  au_component_manu[0], au_component_manu[1], 
	  au_component_manu[2], au_component_manu[3]);

  z[lc++]="";
  z[lc++]="/* end of sfront component definitions */";
  z[lc++]="";

  printlib(lc);
}

/****************************************************************/
/* Audiounit driver:  Component resource & plist file creation  */
/****************************************************************/

void audiounit_filemaker(void) 

{
  char * rezname;
  FILE * f;

  /************************/
  /* make Info.Plist file */
  /************************/

  f = fopen("Info.Plist", "w");

  fprintf(f, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
  fprintf(f, "<!DOCTYPE plist PUBLIC \"-//Apple Computer//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">\n");
  fprintf(f, "<plist version=\"1.0\">\n");
  fprintf(f, "<dict>\n");
  fprintf(f, "	<key>CFBundleDevelopmentRegion</key>\n");
  fprintf(f, "	<string>English</string>\n");
  fprintf(f, "	<key>CFBundleExecutable</key>\n");
  fprintf(f, "	<string>%s</string>\n", au_filesystem_name);
  fprintf(f, "	<key>CFBundleIdentifier</key>\n");
  fprintf(f, "	<string>%s.audiounit.%s</string>\n", au_manu_url, au_filesystem_name);
  fprintf(f, "	<key>CFBundleInfoDictionaryVersion</key>\n");
  fprintf(f, "	<string>6.0</string>\n");
  fprintf(f, "	<key>CFBundlePackageType</key>\n");
  fprintf(f, "	<string>BNDL</string>\n");
  fprintf(f, "	<key>CFBundleShortVersionString</key>\n");
  fprintf(f, "	<string>1.1</string>\n");
  fprintf(f, "	<key>CFBundleSignature</key>\n");
  fprintf(f, "	<string>%c%c%c%c</string>\n", '?', '?', '?', '?');
  fprintf(f, "	<key>CFBundleVersion</key>\n");
  fprintf(f, "	<string>1.1</string>\n");
  fprintf(f, "	<key>CSResourcesFileMapped</key>\n");
  fprintf(f, "	<true/>\n");
  fprintf(f, "</dict>\n");
  fprintf(f, "</plist>\n");
  fclose(f);

  /************************/
  /* make resource file  */
  /************************/

  rezname = malloc(strlen(au_filesystem_name) + 3);
  sprintf(rezname, "%s.r", au_filesystem_name);
  f = fopen(rezname, "w");
  free(rezname);

  /* very preliminary: go through line by line and improve this */

  fprintf(f, "// Resource file for %s (created by sfront)\n\n", au_filesystem_name);
  fprintf(f, "#include <AudioUnit/AudioUnit.r>\n\n");

  if (ain == DRIVER_AUDIOUNIT)
    {
      if (cinname && !strcmp(cinname,"aucontrolm"))
	fprintf(f, "#define RES_ID  1002\n");    /* guessing this is OK ... */
      else
	fprintf(f, "#define RES_ID  1000\n");
    }
  else
    fprintf(f, "#define RES_ID  13472\n");

  if (!strcmp(au_component_type, "aufx"))
    fprintf(f,"#define COMP_TYPE kAudioUnitType_Effect\n");

  if (!strcmp(au_component_type, "aumf"))
    fprintf(f, "#define COMP_TYPE kAudioUnitType_MusicEffect\n");

  if (!strcmp(au_component_type, "aumu"))
    fprintf(f, "#define COMP_TYPE kAudioUnitType_MusicDevice\n");
  
  /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
  /* if this printf is changed, also change audiounit_typemaker() sprintf */
  /*----------------------------------------------------------------------*/

  fprintf(f, "#define COMP_SUBTYPE '%c%c%c%c'\n", au_component_subtype[0],
	  au_component_subtype[1], au_component_subtype[2], au_component_subtype[3]);

  /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
  /* if this printf is changed, also change audiounit_typemaker() sprintf */
  /*----------------------------------------------------------------------*/

  fprintf(f, "#define COMP_MANUF '%c%c%c%c'\n", au_component_manu[0], 
	  au_component_manu[1], au_component_manu[2], au_component_manu[3]);
 
  fprintf(f, "#define VERSION  0x00010000\n"); 
  fprintf(f, "#define NAME \"%s: %s\"\n", au_ui_manu, au_ui_name);
  fprintf(f, "#define DESCRIPTION \"%s AU\"\n", au_filesystem_name);
  fprintf(f, "#define ENTRY_POINT \"asysn_audiounit_entry\"\n\n");

  fprintf(f, "#define UseExtendedThingResource  1\n"); 
  fprintf(f, "#include <CoreServices/CoreServices.r>\n"); 
  fprintf(f, "#ifndef cmpThreadSafeOnMac\n"); 
  fprintf(f, "#define cmpThreadSafeOnMac  0x10000000\n"); 
  fprintf(f, "#endif\n"); 
  fprintf(f, "#undef  TARGET_REZ_MAC_PPC\n"); 
  fprintf(f, "#ifdef ppc_YES\n"); 
  fprintf(f, "	#define TARGET_REZ_MAC_PPC  1\n"); 
  fprintf(f, "#else\n"); 
  fprintf(f, "	#define TARGET_REZ_MAC_PPC  0\n"); 
  fprintf(f, "#endif\n"); 
  fprintf(f, "#undef  TARGET_REZ_MAC_X86\n"); 
  fprintf(f, "#ifdef i386_YES\n"); 
  fprintf(f, "	#define TARGET_REZ_MAC_X86  1\n"); 
  fprintf(f, "#else\n"); 
  fprintf(f, "	#define TARGET_REZ_MAC_X86  0\n"); 
  fprintf(f, "#endif\n"); 
  fprintf(f, "#if TARGET_OS_MAC\n"); 
  fprintf(f, "	#if TARGET_REZ_MAC_PPC && TARGET_REZ_MAC_X86\n"); 
  fprintf(f, "		#define TARGET_REZ_FAT_COMPONENTS  1\n"); 
  fprintf(f, "		#define Target_PlatformType  platformPowerPCNativeEntryPoint\n"); 
  fprintf(f, "		#define Target_SecondPlatformType  platformIA32NativeEntryPoint\n"); 
  fprintf(f, "	#elif TARGET_REZ_MAC_X86\n"); 
  fprintf(f, "		#define Target_PlatformType  platformIA32NativeEntryPoint\n"); 
  fprintf(f, "	#else\n"); 
  fprintf(f, "		#define Target_PlatformType  platformPowerPCNativeEntryPoint\n"); 
  fprintf(f, "	#endif\n"); 
  fprintf(f, "	#define Target_CodeResType  'dlle'\n"); 
  fprintf(f, "	#define TARGET_REZ_USE_DLLE  1\n"); 
  fprintf(f, "#else\n"); 
  fprintf(f, "	#error get a real platform type\n"); 
  fprintf(f, "#endif // not TARGET_OS_MAC\n"); 
  fprintf(f, "#ifndef TARGET_REZ_FAT_COMPONENTS\n"); 
  fprintf(f, "  #define TARGET_REZ_FAT_COMPONENTS  0\n"); 
  fprintf(f, "#endif\n"); 
  fprintf(f, "resource 'STR ' (RES_ID, purgeable) {\n"); 
  fprintf(f, "	NAME\n"); 
  fprintf(f, "};\n"); 
  fprintf(f, "resource 'STR ' (RES_ID + 1, purgeable) {\n"); 
  fprintf(f, "	DESCRIPTION\n"); 
  fprintf(f, "};\n"); 
  fprintf(f, "resource 'dlle' (RES_ID) {\n"); 
  fprintf(f, "	ENTRY_POINT\n"); 
  fprintf(f, "};\n"); 
  fprintf(f, "resource 'thng' (RES_ID, NAME) {\n"); 
  fprintf(f, "	COMP_TYPE,\n"); 
  fprintf(f, "	COMP_SUBTYPE,\n"); 
  fprintf(f, "	COMP_MANUF,\n"); 
  fprintf(f, "	0, 0, 0, 0,\n"); 
  fprintf(f, "	'STR ',	RES_ID,\n"); 
  fprintf(f, "	'STR ',	RES_ID + 1,\n"); 
  fprintf(f, "	0, 0,  /* icon */\n"); 
  fprintf(f, "	VERSION,\n"); 
  fprintf(f, "	componentHasMultiplePlatforms | componentDoAutoVersion,\n"); 
  fprintf(f, "	0,\n"); 
  fprintf(f, "	{\n"); 
  fprintf(f, "		cmpThreadSafeOnMac,\n");  
  fprintf(f, "		Target_CodeResType, RES_ID,\n"); 
  fprintf(f, "		Target_PlatformType,\n"); 
  fprintf(f, "#if TARGET_REZ_FAT_COMPONENTS\n"); 
  fprintf(f, "		cmpThreadSafeOnMac, \n"); 
  fprintf(f, "		Target_CodeResType, RES_ID,\n"); 
  fprintf(f, "		Target_SecondPlatformType,\n"); 
  fprintf(f, "#endif\n"); 
  fprintf(f, "	}\n"); 
  fprintf(f, "};\n"); 
  fprintf(f, "#undef RES_ID\n"); 
  fprintf(f, "#undef COMP_TYPE\n"); 
  fprintf(f, "#undef COMP_SUBTYPE\n"); 
  fprintf(f, "#undef COMP_MANUF\n"); 
  fprintf(f, "#undef VERSION\n"); 
  fprintf(f, "#undef NAME\n"); 
  fprintf(f, "#undef DESCRIPTION\n"); 
  fprintf(f, "#undef ENTRY_POINT\n"); 
  
  fclose(f);
}

