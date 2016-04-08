
/*
#    Sfront, a SAOL to C translator    
#    This file: Main loop for runtime: active driver, shorts
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



#if defined(ASYS_ACTIVE_O)

int asys_orun(ASYS_OTYPE obuf[], int * osize)

{
  int obusidx = 0;
  int busidx;

  if (EV(asys_exit_status) == ASYS_EXIT)
    return ASYS_EXIT;

  while (obusidx < *osize)
    if (EV(acycleidx) < EV(ACYCLE))
      {
	memset(&(TB(0)), 0, sizeof(TB(0))*ENDBUS);
	main_apass(ENGINE_PTR);
	for (busidx=BUS_output_bus; busidx<ENDBUS_output_bus;busidx++)
	  {
	    TB(busidx) = (TB(busidx) >  1.0F) ?  1.0F : TB(busidx);

#if (ASYS_OTYPENAME == ASYS_SHORT)
	    TB(busidx) = (TB(busidx) < -1.0F) ? -1.0F : TB(busidx);
	    obuf[obusidx++] = (short) (32767.0F * TB(busidx));
#endif

#if (ASYS_OTYPENAME == ASYS_FLOAT)
	    obuf[obusidx++] = (TB(busidx) < -1.0F) ? -1.0F : TB(busidx);
#endif

	  }
	EV(acycleidx)++;
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
	    *osize = obusidx;
	    return (EV(asys_exit_status) = ASYS_EXIT);
	  }
	EV(pass) = IPASS;
	EV(scorebeats) = EV(scoremult)*(EV(kcycleidx) - EV(kbase)) + EV(scorebase);
	EV(absolutetime) = (EV(kcycleidx) - 1)*EV(KTIME);
	main_ipass(ENGINE_PTR);
	EV(pass) = KPASS;
	main_control(ENGINE_PTR);
	if (main_kpass(ENGINE_PTR))
	  {
	    *osize = obusidx;
	    return (EV(asys_exit_status) = ASYS_EXIT);
	  }
	EV(pass) = APASS;
      }

  return ASYS_DONE;
}

#endif  /* ASYS_ACTIVE_O */


#if defined(ASYS_ACTIVE_IO)

int asys_iorun(ASYS_ITYPE ibuf[], int * isize,ASYS_OTYPE obuf[], int * osize)

{  
  int ibusidx = 0;
  int obusidx = 0;
  int busidx;

  if (EV(asys_exit_status) == ASYS_EXIT)
    return ASYS_EXIT;

  while ((obusidx < *osize) && (ibusidx < *isize))
    if (EV(acycleidx) < EV(ACYCLE))
      {
	memset(&(TB(0)), 0, sizeof(TB(0))*ENDBUS);

#if (ASYS_ITYPENAME == ASYS_SHORT)
	for(busidx=BUS_input_bus;busidx<ENDBUS_input_bus;busidx++)
	  TB(busidx) = 3.051851e-5F*ibuf[ibusidx++];
#endif

#if (ASYS_ITYPENAME == ASYS_FLOAT)
	for(busidx=BUS_input_bus;busidx<ENDBUS_input_bus;busidx++)
	  TB(busidx) = ibuf[ibusidx++];
#endif

	main_apass(ENGINE_PTR);
	for (busidx=BUS_output_bus; busidx<ENDBUS_output_bus;busidx++)
	  {
	    TB(busidx) = (TB(busidx) >  1.0F) ?  1.0F : TB(busidx);

#if (ASYS_OTYPENAME == ASYS_SHORT)
	    TB(busidx) = (TB(busidx) < -1.0F) ? -1.0F : TB(busidx);
	    obuf[obusidx++] = (short) (32767.0F * TB(busidx));
#endif

#if (ASYS_OTYPENAME == ASYS_FLOAT)
	    obuf[obusidx++] = (TB(busidx) < -1.0F) ? -1.0F : TB(busidx);
#endif
	  }
	EV(acycleidx)++;
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
	    *osize = obusidx;
	    *isize = ibusidx;
	    return (EV(asys_exit_status) = ASYS_EXIT);
	  }
	EV(pass) = IPASS;
	EV(scorebeats) = EV(scoremult)*(EV(kcycleidx) - EV(kbase)) + EV(scorebase);
	EV(absolutetime) = (EV(kcycleidx) - 1)*EV(KTIME);
	main_ipass(ENGINE_PTR);
	EV(pass) = KPASS;
	main_control(ENGINE_PTR);
	if (main_kpass(ENGINE_PTR))
	  {
	    *osize = obusidx;
	    *isize = ibusidx;
	    return (EV(asys_exit_status) = ASYS_EXIT);
	  }
	EV(pass) = APASS;
      }

  if (obusidx < *osize)
    *osize = obusidx;
  if (ibusidx < *isize)
    *isize = ibusidx;
  return ASYS_DONE;
}

#endif /* ASYS_ACTIVE_IO */


int main(int argc, char *argv[])

{
  system_init(argc, argv, SAOL_SRATE);
  effects_init(ENGINE_PTR);
  main_initpass(ENGINE_PTR);

  EV(kcycleidx) = EV(kbase);
  EV(acycleidx) = EV(ACYCLE);
  EV(pass) = IPASS;

  asys_main();
  shut_down(ENGINE_PTR);
  return 0;
}





