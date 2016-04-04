
/*
#    Sfront, a SAOL to C translator    
#    This file: null audio driver for sfront (thanks to Michael McGonagle)
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
/*           nullin/nullout file audio driver for sfront        */
/*           for benchmarking and printf()-driven apps          */ 
/****************************************************************/
        
#if defined(ASYS_HASOUTPUT)

/* global variables, must start with asys_ */

int asyso_size;    /* number of samples in a buffer */
short * asyso_buf;  /* output buffer */

#endif

#if defined(ASYS_HASINPUT)

/* global variables, must start with asys_ */

int asysi_size;     /* number of samples in a buffer */
short * asysi_buf;   /* input buffer */

#endif

#if defined(ASYS_HASOUTPUT)

/****************************************************************/
/*        core routine for audio output setup                   */
/****************************************************************/

int asyso_setup(int srate, int ochannels, int osize, char * oname)

{
  asyso_size = osize;
  asyso_buf = (short *)calloc(osize, sizeof(short));
  return ASYS_DONE;
}

#endif

#if defined(ASYS_HASINPUT)

/****************************************************************/
/*        core routine for audio input setup                   */
/****************************************************************/

int asysi_setup(int srate, int ichannels, int isize, char * iname)

{
  asysi_size = isize;
  asysi_buf = (short *)calloc(isize, sizeof(short));
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

#if (defined(ASYS_HASOUTPUT)&&(!defined(ASYS_HASINPUT)))

/****************************************************************/
/*                    shuts down audio output                   */
/****************************************************************/

void asys_oshutdown(void)

{
}

#endif

#if (!defined(ASYS_HASOUTPUT)&&(defined(ASYS_HASINPUT)))

/****************************************************************/
/*              shuts down audio input device                   */
/****************************************************************/

void asys_ishutdown(void)

{
}

#endif

#if (defined(ASYS_HASOUTPUT)&&(defined(ASYS_HASINPUT)))

/****************************************************************/
/*              shuts down audio input and output device        */
/****************************************************************/

void asys_ioshutdown(void)

{
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
  *osize = asyso_size;
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
    *asys_ibuf = asysi_buf;
  *isize = asysi_size;
  return ASYS_DONE;
}

#endif
