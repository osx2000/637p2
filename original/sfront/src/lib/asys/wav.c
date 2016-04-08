
/*
#    Sfront, a SAOL to C translator    
#    This file: WAV audio driver for sfront
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
        
#include <stdio.h>
#include <string.h>

#if defined(ASYS_HASOUTPUT)

/* default name for output audio file */
#define ASYSO_DEFAULTNAME "output.wav"

/* global variables, must start with asyso_ */

FILE * asyso_fd;     /* output file pointer */
char * asyso_name;   /* name of file  */        
int asyso_srate;    /* sampling rate */
int asyso_channels; /* number of channels */
int asyso_size;     /* number of float samples _buf */
int asyso_nsamp;    /* total number of samples written */
int asyso_bps;      /* number of bytes per sample, 1-3 */
int asyso_doswap;   /* needs byteswap on write */
float * asyso_buf;   /* output floats from sfront */ 
unsigned char * asyso_cbuf;  /* output chars to file */
#endif

#if defined(ASYS_HASINPUT)

/* default name for input audio file */

#define ASYSI_DEFAULTNAME "input.wav"

/* only used for asysi_soundtypecheck */

#define ASYSI_MATCH  0
#define ASYSI_EOF 1
#define ASYSI_NOMATCH 2

/* global variables, must start with asysi_ */

FILE * asysi_fd;     /* input file pointer */
char * asysi_name;   /* name of file  */        
int asysi_srate;    /* sampling rate */
int asysi_channels; /* number of channels */
int asysi_bytes;     /* number of bytes in a buffer */
int asysi_nsamp;    /* total number of samples read */
int asysi_bps;      /* number of bytes per sample, 1-3 */
int asysi_doswap;   /* needs byteswap on read */
unsigned char * asysi_cbuf;  /* buffer of WAV file bytes */
float * asysi_buf;   /* float buffer for sfront */ 

#endif

#if defined(ASYS_HASOUTPUT)

/*********************************************************/
/*        writes next block of WAV/AIFF bytes            */
/*********************************************************/

int asyso_putbytes(unsigned char * c, int numbytes)

{
  if (rwrite(c, sizeof(char), numbytes, asyso_fd) != numbytes)
    return ASYS_ERROR;
  return ASYS_DONE;
}

/*********************************************************/
/*        writes unsigned int to a WAV files             */
/*********************************************************/

int asyso_putint(unsigned int val, int numbytes)

{
  unsigned char c[4];

  if (numbytes > 4)
    return ASYS_ERROR;
  switch (numbytes) {
  case 4:
    c[0] = (unsigned char) (val&0x000000FF);
    c[1] = (unsigned char)((val >> 8)&0x000000FF);
    c[2] = (unsigned char)((val >> 16)&0x000000FF);
    c[3] = (unsigned char)((val >> 24)&0x000000FF);
    return asyso_putbytes(c, 4);
  case 3:
    c[0] = (unsigned char) (val&0x000000FF);
    c[1] = (unsigned char)((val >> 8)&0x000000FF);
    c[2] = (unsigned char)((val >> 16)&0x000000FF);
    return asyso_putbytes(c, 3);
  case 2:
    c[0] = (unsigned char) (val&0x000000FF);
    c[1] = (unsigned char)((val >> 8)&0x000000FF);
    return asyso_putbytes(c, 2);
  case 1:
    c[0] = (unsigned char) (val&0x000000FF);
    return asyso_putbytes(c,1);
  default:
    return ASYS_ERROR;
  }

}

/****************************************************************/
/*        core routine for audio output setup                   */
/****************************************************************/

int asyso_setup(int srate, int ochannels, int osize, char * name)


{
  short swaptest = 0x0100;
  char * val;

  asyso_doswap = *((char *)&swaptest);
  if (name == NULL)
    val = ASYSO_DEFAULTNAME;
  else
    val = name;

  switch (ASYS_OUTFILE_WORDSIZE) {
  case ASYS_OUTFILE_WORDSIZE_8BIT: 
    asyso_bps = 1;
    break;
  case ASYS_OUTFILE_WORDSIZE_16BIT:
    asyso_bps = 2;
    break;
  case ASYS_OUTFILE_WORDSIZE_24BIT:
    asyso_bps = 3;
    break;
  }

  asyso_name = strcpy((char *) calloc((strlen(val)+1),sizeof(char)), val);
  asyso_fd = fopen(asyso_name,"wb");
  if (asyso_fd == NULL)
    return ASYS_ERROR;

  /* preamble for wav file */

  asyso_putbytes((unsigned char *) "RIFF",4);
  asyso_putint(0,4);       /* patched later */
  asyso_putbytes((unsigned char *) "WAVEfmt ",8);
  asyso_putint(16,4);
  asyso_putint(1,2);                          /* PCM  */
  asyso_putint(ochannels,2);                  /* number of channels */
  asyso_putint(srate,4);                      /* srate */
  asyso_putint(srate*ochannels*asyso_bps, 4); /* bytes/sec */
  asyso_putint(ochannels*asyso_bps, 2);       /* block align */
  asyso_putint(8*asyso_bps, 2);               /* bits per sample */
  asyso_putbytes((unsigned char *) "data",4);
  asyso_putint(0,4);                          /* patched later */

  asyso_srate = srate;
  asyso_channels = ochannels;
  asyso_size = osize;
  asyso_nsamp = 0;
  asyso_cbuf = (unsigned char *) malloc(osize*asyso_bps);

  if (asyso_cbuf == NULL)
    {
      fprintf(stderr, "Can't allocate WAV byte output buffer (%s).\n",
	      strerror(errno));
      return ASYS_ERROR;
    }

  asyso_buf = (float *)calloc(osize, sizeof(float));

  if (asyso_buf == NULL)
    {
      fprintf(stderr, "Can't allocate WAV float output buffer (%s).\n",
	      strerror(errno));
      return ASYS_ERROR;
    }

  return ASYS_DONE;
}

#endif

#if defined(ASYS_HASINPUT)

/*********************************************************/
/*            gets next block of WAV bytes               */
/*********************************************************/

int asysi_getbytes(unsigned char * c, int numbytes)

{
  if ((int)rread(c, sizeof(char), numbytes, asysi_fd) != numbytes)
    return ASYS_ERROR;
  return ASYS_DONE;
}

/*********************************************************/
/*        flushes next block of WAV bytes                */
/*********************************************************/

int asysi_flushbytes(int numbytes)

{
  unsigned char c;

  while (numbytes > 0)
    {
      if (rread(&c, sizeof(char), 1, asysi_fd) != 1)
	return ASYS_ERROR;
      numbytes--;
    }
  return ASYS_DONE;

}

/*********************************************************/
/*     converts byte stream to an unsigned int           */
/*********************************************************/

int asysi_getint(int numbytes, unsigned int * ret)

{
  unsigned char c[4];

  if (numbytes > 4)
    return ASYS_ERROR;
  if (ASYS_DONE != asysi_getbytes(&c[0],numbytes))
    return ASYS_ERROR;
  switch (numbytes) {
  case 4:
    *ret  =  (unsigned int)c[0];
    *ret |=  (unsigned int)c[1] << 8;
    *ret |=  (unsigned int)c[2] << 16;
    *ret |=  (unsigned int)c[3] << 24;
    return ASYS_DONE;
  case 3:
    *ret  =  (unsigned int)c[0];
    *ret |=  (unsigned int)c[1] << 8;
    *ret |=  (unsigned int)c[2] << 16;
    return ASYS_DONE;
  case 2:
    *ret  =  (unsigned int)c[0];
    *ret |=  (unsigned int)c[1] << 8;
    return ASYS_DONE;
  case 1:
    *ret = (unsigned int)c[0];
    return ASYS_DONE;
  default:
    return ASYS_ERROR;
  }

}
  
/***********************************************************/
/*  checks byte stream for AIFF/WAV cookie --              */
/***********************************************************/

int asysi_soundtypecheck(char * d)

{
  char c[4];

  if (rread(c, sizeof(char), 4, asysi_fd) != 4)
    return ASYSI_EOF;
  if (strncmp(c,d,4))
    return ASYSI_NOMATCH;
  return ASYSI_MATCH;
}
  
/****************************************************************/
/*        core routine for audio input setup                   */
/****************************************************************/

int asysi_setup(int srate, int ichannels, int isize, char * name)


{
  short swaptest = 0x0100;
  unsigned int i, cookie;
  int len;
  char * val;

  asysi_doswap = *((char *)&swaptest);

  if (name == NULL)
    val = ASYSI_DEFAULTNAME;
  else
    val = name;
  asysi_name = strcpy((char *) calloc((strlen(val)+1),sizeof(char)), val);
  asysi_fd = fopen(asysi_name,"rb");
  if (asysi_fd == NULL)
    return ASYS_ERROR;

  if (asysi_soundtypecheck("RIFF")!= ASYSI_MATCH)
    return ASYS_ERROR;
  if (asysi_flushbytes(4)!= ASYS_DONE)
    return ASYS_ERROR;
  if (asysi_soundtypecheck("WAVE")!= ASYSI_MATCH)
    return ASYS_ERROR;
  while ((cookie = asysi_soundtypecheck("fmt "))!=ASYSI_MATCH)
    {
      if (cookie == ASYSI_EOF)
	return ASYS_ERROR;
      if (asysi_getint(4, &i) != ASYS_DONE)
	return ASYS_ERROR;
      if (asysi_flushbytes(i + (i % 2))!= ASYS_DONE)
	return ASYS_ERROR;
    }
  if (asysi_getint(4, &i) != ASYS_DONE)
    return ASYS_ERROR;
  len = i;
  if ((len -= 16) < 0)
    return ASYS_ERROR;
  if (asysi_getint(2, &i) != ASYS_DONE)
    return ASYS_ERROR;
  if (i != 1)
    {
      fprintf(stderr,"Error: Can only handle PCM WAV files\n");
      return ASYS_ERROR;
    }
  if (asysi_getint(2, &i) != ASYS_DONE)
    return ASYS_ERROR;
  if (i != ichannels)
    {
      fprintf(stderr,"Error: Inchannels doesn't match WAV file\n");
      return ASYS_ERROR;
    }
  if (asysi_getint(4, &i) != ASYS_DONE)
    return ASYS_ERROR;
  if (srate != i)
    fprintf(stderr,"Warning: SAOL srate %i mismatches WAV file srate %i\n",
	    srate,i);
  asysi_flushbytes(6);
  if (asysi_getint(2, &i) != ASYS_DONE)
    return ASYS_ERROR;
  if ((i < 8) || (i > 24))
    {
      fprintf(stderr,"Error: Can't handle %i bit data\n",i);
      return ASYS_ERROR;
    }
  asysi_bps = i/8;
  asysi_flushbytes(len + (len % 2));

  while ((cookie = asysi_soundtypecheck("data"))!=ASYSI_MATCH)
    {
      if (cookie == ASYSI_EOF)
	return ASYS_ERROR;
      if (asysi_getint(4, &i) != ASYS_DONE)
	return ASYS_ERROR;
      if (asysi_flushbytes(i + (i % 2))!= ASYS_DONE)
	return ASYS_ERROR;
    }
  if (asysi_getint(4, &i) != ASYS_DONE)
    return ASYS_ERROR;

  asysi_nsamp = i/asysi_bps;
  asysi_srate = srate;
  asysi_channels = ichannels;
  asysi_bytes = isize*asysi_bps;
  asysi_cbuf = (unsigned char *) malloc(asysi_bytes);

  if (asysi_cbuf == NULL)
    {
      fprintf(stderr, "Can't allocate WAV input byte buffer (%s).\n",
	      strerror(errno));
      return ASYS_ERROR;
    }

  asysi_buf = (float *) malloc(sizeof(float)*isize);

  if (asysi_buf == NULL)
    {
      fprintf(stderr, "Can't allocate WAV input float buffer (%s).\n",
	      strerror(errno));
      return ASYS_ERROR;
    }

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

  fseek(asyso_fd, 4, SEEK_SET);
  asyso_putint(asyso_bps*(unsigned int)asyso_nsamp+36,4);
  fseek(asyso_fd, 32, SEEK_CUR);
  asyso_putint(asyso_bps*(unsigned int)asyso_nsamp,4);
  fclose(asyso_fd);

}

#endif

#if defined(ASYS_HASINPUT)

/****************************************************************/
/*               shuts down audio input system                  */
/****************************************************************/

void asysi_shutdown(void)

{

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
  *asys_obuf = asyso_buf;
  *osize = asyso_size;
  return ASYS_DONE;
}

/****************************************************************/
/*               sends one frame of audio to output             */
/****************************************************************/

int asys_putbuf(ASYS_OTYPE * asys_obuf[], int * osize)

{
  float * buf = *asys_obuf;
  float fval;
  int val;
  int i = 0;
  int j = 0;

  switch (asyso_bps) {
  case 3:
    while (i < *osize)
      {
	fval = ((float)(pow(2, 23) - 1))*buf[i++];
	val = (int)((fval >= 0.0F) ? (fval + 0.5F) : (fval - 0.5F));
	asyso_cbuf[j++] = (unsigned char) (val & 0x000000FF);
	asyso_cbuf[j++] = (unsigned char)((val >> 8) & 0x000000FF);
	asyso_cbuf[j++] = (unsigned char)((val >> 16) & 0x000000FF);
      }
    break;
  case 2:
    while (i < *osize)
      {
	fval = ((float)(pow(2, 15) - 1))*buf[i++];
	val = (int)((fval >= 0.0F) ? (fval + 0.5F) : (fval - 0.5F));
	asyso_cbuf[j++] = (unsigned char) (val & 0x000000FF);
	asyso_cbuf[j++] = (unsigned char)((val >> 8) & 0x000000FF);
      }
    break;
  case 1:
    while (i < *osize)
      {
	fval = ((float)(pow(2, 7) - 1))*buf[i++];
	asyso_cbuf[j++] = (unsigned char)
	  (128 + ((char)((fval >= 0.0F) ? (fval + 0.5F) : (fval - 0.5F))));
      }
    break;
  }
  
  if (rwrite(asyso_cbuf, sizeof(char), j, asyso_fd) != j)
    return ASYS_ERROR;

  asyso_nsamp += *osize;
  *osize = asyso_size;
  return ASYS_DONE;
}

#endif


#if defined(ASYS_HASINPUT)

/****************************************************************/
/*               get one frame of audio from input              */
/****************************************************************/

int asys_getbuf(ASYS_ITYPE * asys_ibuf[], int * isize)

{
  int i = 0;
  int j = 0;

  if (*asys_ibuf == NULL)
    *asys_ibuf = asysi_buf;
  
  if (asysi_nsamp <= 0)
    {
      *isize = 0;
      return ASYS_DONE;
    }

  *isize = (int)rread(asysi_cbuf, sizeof(unsigned char), asysi_bytes, asysi_fd);

  switch (asysi_bps) {
  case 1:                              /* 8-bit  */
    while (i < *isize)
      {
	asysi_buf[i] = ((float)pow(2, -7))*(((short) asysi_cbuf[i]) - 128);
	i++;
      }
    break;
  case 2:                              /* 9-16 bit */
    *isize = (*isize) / 2;
    while (i < *isize)
      {
	asysi_buf[i] = ((float)pow(2, -15))*((int)(asysi_cbuf[j]) + 
				    (((int)((char)(asysi_cbuf[j+1]))) << 8)); 
	i++;
	j += 2;
      }
    break;
  case 3:                            /* 17-24 bit */
    *isize = (*isize) / 3;
    while (i < *isize)
      {
	asysi_buf[i] = ((float)pow(2, -23))*((int)(asysi_cbuf[j]) + 
				    (((int)(asysi_cbuf[j+1])) << 8) + 
				    (((int)((char) asysi_cbuf[j+2])) << 16));
	i++; 
	j += 3;
      }
    break;
  } 

  asysi_nsamp -= *isize;
  return ASYS_DONE;
}

#endif


