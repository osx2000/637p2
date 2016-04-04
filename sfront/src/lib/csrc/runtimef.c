
/*
#    Sfront, a SAOL to C translator    
#    This file: Main loop for runtime
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


int main(int argc, char *argv[])

{
  ENGINE_PTR_DECLARE_SEMICOLON
  int busidx;

  ENGINE_PTR_ASSIGNED_TO  system_init(argc, argv, SAOL_SRATE);
  if (ENGINE_PTR_IS_NULL) return -1;
  effects_init(ENGINE_PTR);
  main_initpass(ENGINE_PTR);
  for (EV(kcycleidx)=EV(kbase); EV(kcycleidx)<=EV(endkcycle); EV(kcycleidx)++)
    {
      EV(pass) = IPASS;
      EV(scorebeats) = EV(scoremult)*(EV(kcycleidx) - EV(kbase)) + EV(scorebase);
      EV(absolutetime) = (EV(kcycleidx) - 1)*EV(KTIME);
      main_ipass(ENGINE_PTR);
      EV(pass) = KPASS;
      main_control(ENGINE_PTR);
      if (main_kpass(ENGINE_PTR))
	break;
      EV(pass) = APASS;
      for (EV(acycleidx)=0; EV(acycleidx)<EV(ACYCLE); EV(acycleidx)++)
	{
	  memset(&(TB(0)), 0, sizeof(TB(0))*ENDBUS);
	  main_apass(ENGINE_PTR);
	  for (busidx=BUS_output_bus; busidx<ENDBUS_output_bus;busidx++)
	    {
	      TB(busidx) = (TB(busidx) >  1.0F) ?  1.0F : TB(busidx);
	      EV(asys_obuf)[EV(obusidx)++] = (TB(busidx) < -1.0F) ? -1.0F:TB(busidx);
	    }
	  if (EV(obusidx) >= EV(asys_osize))
	    {
	      EV(obusidx) = 0;
	      if (asys_putbuf(&EV(asys_obuf), &EV(asys_osize)))
		{
		  fprintf(stderr,"  Sfront: Audio output device problem\n\n");
		  EV(kcycleidx) = EV(endkcycle);
		  break;
		}
	    }
	}
      EV(acycleidx) = 0; 
      EV(cpuload) = ksync();
    }
  shut_down(ENGINE_PTR);
  return 0;
}





