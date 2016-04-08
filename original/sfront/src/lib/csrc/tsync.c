
/*
#    Sfront, a SAOL to C translator    
#    This file: Time synchronization routines for real-time input.
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


/******************************************************************/
/*                                                                */
/* This file defines two functions ksyncinit(), called before     */
/* the first ipass begins, and ksync(), called at the start       */
/* of every ipass. These routines act to ensure that each         */
/* sequence of IPASS/KPASS/APASS takes at least EV(KUTIME). ksync */
/* returns an estimate of cpuload, range [0.0:1.0], which codes   */
/* what fraction of the kcycle was used for computing audio.      */
/*                                                                */
/*                                                                */
/* Rules for writing your own timesync sync.c file:               */
/*                                                                */
/* (1) all globals and functions must begin with sync_, all defs  */
/*     must begin with SYNC_.                                     */
/* (2) Define both ksync() and ksyncinit().                       */
/*                                                                */
/* Note that:                                                     */
/*  EV(KUTIME): an int, defines k-cycle length in microseconds    */
/*  EV(KMTIME): a float, defines k-cycle length in milliseconds   */
/*  EV(KTIME):  a float, defines k-cycle length in seconds        */
/******************************************************************/


#include <sys/time.h>

#define SYNC_ONESECOND 1000000L
#define SYNC_SELDELAY 1000L
#define SYNC_NORM (1.0F/EV(KUTIME))

/***********************************************************/
/*         synchronizes on k-rate boundaries               */
/***********************************************************/

float ksync()

{
  float ret = 1.0F;

  if ((EV(sync_last.tv_usec) += EV(KUTIME)) < SYNC_ONESECOND) /* non-rollover case */
    {
      gettimeofday(&EV(sync_this), NULL);
      if (((EV(sync_delay.tv_usec) = EV(sync_last.tv_usec) - EV(sync_this.tv_usec)) >= 0)
	  && (EV(sync_last.tv_sec) == EV(sync_this.tv_sec)))
	{
	  ret -= SYNC_NORM*EV(sync_delay.tv_usec);
	  ret = (ret > 0.0F) ? ret : 0.0F;

	  if ((EV(sync_delay.tv_usec) -= SYNC_SELDELAY) > 0)
	    select(0, NULL, NULL, NULL, &EV(sync_delay));

	  do 
	    gettimeofday(&EV(sync_this), NULL);
	  while (((EV(sync_this.tv_sec) == EV(sync_last.tv_sec)))
		 &&(EV(sync_last.tv_usec) > EV(sync_this.tv_usec)));
	}
      return ret;
    }

  EV(sync_last.tv_sec)++;
  EV(sync_last.tv_usec) %= SYNC_ONESECOND;

  gettimeofday(&EV(sync_this), NULL);
  if ((EV(sync_last.tv_sec) > EV(sync_this.tv_sec)) ||
      ((EV(sync_last.tv_sec) == EV(sync_this.tv_sec)) &&
       ((EV(sync_delay.tv_usec) = EV(sync_last.tv_usec) - EV(sync_this.tv_usec)) >= 0)))
    {
      if (EV(sync_last.tv_sec) > EV(sync_this.tv_sec))
	EV(sync_delay.tv_usec) = SYNC_ONESECOND  - EV(sync_this.tv_usec)
	  + EV(sync_last.tv_usec);

      ret -= SYNC_NORM*EV(sync_delay.tv_usec);
      ret = (ret > 0.0F) ? ret : 0.0F;

      if ((EV(sync_delay.tv_usec) -= SYNC_SELDELAY) > 0)
	select(0, NULL, NULL, NULL, &EV(sync_delay));

      do
	gettimeofday(&EV(sync_this), NULL);
      while ((EV(sync_last.tv_sec) > EV(sync_this.tv_sec)) || 
	     ((EV(sync_last.tv_sec) == EV(sync_this.tv_sec)) &&
	      (EV(sync_last.tv_usec) > EV(sync_this.tv_usec))));
    }
  return ret;

}

/***********************************************************/
/*         initializes k-rate boundaries sync              */
/***********************************************************/

void ksyncinit()

{
  EV(sync_delay.tv_sec) = 0;
  gettimeofday(&EV(sync_last), NULL);
}


