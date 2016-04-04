
/*
#    Sfront, a SAOL to C translator    
#    This file: raw audio driver for sfront
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
/*             raw file audio driver for sfront                 */ 
/*       reads/writes 16-bit signed shorts in native format     */
/****************************************************************/
        
#include <stdio.h>
#include <string.h>

#if defined(ASYS_HASOUTPUT)

/* default name for output audio file */
#define ASYSO_DEFAULTNAME "output"

/* global variables, must start with asyso_ */

FILE * asyso_fd;     /* output file pointer */
char * asyso_name;   /* name of file  */        
int asyso_srate;    /* sampling rate */
int asyso_channels; /* number of channels */
int asyso_size;    /* number of samples in a buffer */
int asyso_nsamp;    /* total number of samples written */
short * asyso_buf;   /* location for output buffer */ 
#endif

#if defined(ASYS_HASINPUT)

/* default name for output audio file */
#define ASYSI_DEFAULTNAME "input"

/* global variables, must start with asysi_ */

FILE * asysi_fd;     /* input file pointer */
char * asysi_name;   /* name of file  */        
int asysi_srate;    /* sampling rate */
int asysi_channels; /* number of channels */
int asysi_size;    /* number of samples in a buffer */
int asysi_nsamp;    /* total number of samples written */
short * asysi_buf;   /* location for input buffer */ 

#endif

#if defined(ASYS_HASOUTPUT)

/****************************************************************/
/*        core routine for audio output setup                   */
/****************************************************************/

int asyso_setup(int srate, int ochannels, int osize, char * name)


{
  char * val;

  if (name == NULL)
    val = ASYSO_DEFAULTNAME;
  else
    val = name;
  asyso_name = strcpy((char *) calloc((strlen(val)+1),sizeof(char)), val);
  asyso_fd = fopen(asyso_name,"wb");
  if (asyso_fd == NULL)
    return ASYS_ERROR;
  asyso_srate = srate;
  asyso_channels = ochannels;
  asyso_size = osize;
  asyso_nsamp = 0;
  asyso_buf = (short *)calloc(osize, sizeof(short));
  return ASYS_DONE;
}

#endif

#if defined(ASYS_HASINPUT)

/****************************************************************/
/*        core routine for audio input setup                   */
/****************************************************************/

int asysi_setup(int srate, int ichannels, int isize, char * name)


{
  char * val;

  if (name == NULL)
    val = ASYSI_DEFAULTNAME;
  else
    val = name;
  asysi_name = strcpy((char *) calloc((strlen(val)+1),sizeof(char)), val);
  asysi_fd = fopen(asysi_name,"rb");
  if (asysi_fd == NULL)
    return ASYS_ERROR;
  asysi_srate = srate;
  asysi_channels = ichannels;
  asysi_size = isize;
  asysi_nsamp = 0;
  asysi_buf = (short *)malloc(sizeof(short)*isize);
  return ASYS_DONE;
}

#endif

#if (defined(ASYS_HASOUTPUT) && !defined(ASYS_HASINPUT))

/****************************************************************/
/*        sets up audio output for a given srate/channels       */
/****************************************************************/

int asys_osetup(int srate, int ochannels, int osample, 
                char * oname, int toption)

{
  return asyso_setup(srate, ochannels, ASYS_OCHAN*EV(ACYCLE), oname);
}

#endif


#if (!defined(ASYS_HASOUTPUT) && defined(ASYS_HASINPUT))

/****************************************************************/
/*        sets up audio input for a given srate/channels       */
/****************************************************************/

int asys_isetup(int srate, int ichannels, int isample, 
                char * iname, int toption)

{
  return asysi_setup(srate, ichannels, ASYS_ICHAN*EV(ACYCLE), iname);
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

  if (asysi_setup(srate, ichannels, ASYS_ICHAN*EV(ACYCLE), iname) != ASYS_DONE)
    return ASYS_ERROR;
  return asyso_setup(srate, ochannels, ASYS_OCHAN*EV(ACYCLE), oname);

}

#endif

#if defined(ASYS_HASOUTPUT)

/****************************************************************/
/*             shuts down audio output system                   */
/****************************************************************/

void asyso_shutdown(void)

{
  char name[1024];

  fclose(asyso_fd);
  strcpy(name,asyso_name);
  strcat(name,".info");
  asyso_fd = fopen(name,"w");
  fprintf(asyso_fd,"%i\n",asyso_srate);
  fprintf(asyso_fd,"%i\n",asyso_channels);
  fprintf(asyso_fd,"%i\n",asyso_nsamp);
  fclose(asyso_fd);
}

#endif

#if defined(ASYS_HASINPUT)

/****************************************************************/
/*               shuts down audio input system                  */
/****************************************************************/

void asysi_shutdown(void)

{
  char name[1024];

  fclose(asysi_fd);
  strcpy(name, asysi_name);
  strcat(name,".info");
  asysi_fd = fopen(name,"w");
  fprintf(asysi_fd,"%i\n", asysi_srate);
  fprintf(asysi_fd,"%i\n", asysi_channels);
  fprintf(asysi_fd,"%i\n", asysi_nsamp);
  fclose(asysi_fd);
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
  if (rwrite(*asys_obuf, sizeof(short), *osize, asyso_fd) != *osize)
    return ASYS_ERROR;
  asyso_nsamp += *osize;
  *osize = asyso_size;
  return ASYS_DONE;
}

#endif

#if defined(ASYS_HASINPUT)

/****************************************************************/
/*               sends one frame of audio to output             */
/****************************************************************/

int asys_getbuf(ASYS_ITYPE * asys_ibuf[], int * isize)

{
  if (*asys_ibuf == NULL)
    *asys_ibuf = asysi_buf;
  *isize = (int)rread(*asys_ibuf, sizeof(short), asysi_size, asysi_fd);
  asysi_nsamp += *isize;
  return ASYS_DONE;
}

#endif

