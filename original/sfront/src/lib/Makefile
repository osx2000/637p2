
##    Sfront, a SAOL to C translator    
##    This file: Makefile to make sfront + new libraries
##
## Copyright (c) 1999-2006, Regents of the University of California
## All rights reserved.
##
## Redistribution and use in source and binary forms, with or without
## modification, are permitted provided that the following conditions are
## met:
##
##  Redistributions of source code must retain the above copyright
##  notice, this list of conditions and the following disclaimer.
##
##  Redistributions in binary form must reproduce the above copyright
##  notice, this list of conditions and the following disclaimer in the
##  documentation and/or other materials provided with the distribution.
##
##  Neither the name of the University of California, Berkeley nor the
##  names of its contributors may be used to endorse or promote products
##  derived from this software without specific prior written permission.
##
## THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
## "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
## LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
## A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
## OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
## SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
## LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
## DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
## THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
## (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
## OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
##
##    Maintainer: John Lazzaro, lazzaro@cs.berkeley.edu


##
## The current version and release date.
##
##             --IDSTRING--

##
## Compiler optimization and debug options. Mac OS X users 
## should replace "gcc" with "cc".
##

CC = gcc
OPT  = -O2 
DBUG = -g
MODEL = -m32

##
## Machine/OS specific compiling options should be added
## to CFLAGS below.
##

CFLAGS = $(OPT) $(DBUG) $(MODEL)

##
## 

LIBMAKER = libmaker

OBJS = libmaker.o 

##
##  Networking support is not enabled in this sfront distribution.
##  See the sfront/src/NETWORK.HTML file for full instructions on how
##  to enable networking support.  In this file, comment out the
##  blank NSYS definition below, and uncommment the NSYS definition
##  one the two lines that follow (be sure to uncomment both lines).
##

NSYS = 

##  NSYS = net_sfront net_include net_globals net_siplib \
##         net_rtplib net_rtcplib net_jsend net_jrecv net_crypto


PSYS =  pa_hosthdr pa_porthdr pa_tracehdr pa_lib pa_unix_oss \
	pa_win_wmme pa_dshdr pa_dswrap pa_dsound

CSRC =  runtime runtimef callback robust preamble runspt \
        tsync psync tgen 

ASYS  = aif hpux raw wav dsound linux freebsd std vcdat null irix \
	portaudio coreaudio audiounit wiretap

CSYS = ascii gliss linmidi freebsdmidi win32 alsamidi alsaseq fstr \
       coremidi aucontrol

all: $(LIBMAKER)
	./libmaker -csrc $(CSRC)
	./libmaker -asys $(ASYS)
	./libmaker -csys $(CSYS)
	./libmaker -nsys $(NSYS)
	./libmaker -psys $(PSYS)
	(cd .. ; make; cd lib)

cfiles: $(LIBMAKER)
	./libmaker -csrc $(CSRC)
	./libmaker -asys $(ASYS)
	./libmaker -csys $(CSYS)
	./libmaker -nsys $(NSYS)
	./libmaker -psys $(PSYS)

$(LIBMAKER):  $(OBJS) libmaker.h
	$(CC) $(CFLAGS) $(OBJS) -o $(LIBMAKER)

do_csrc: $(LIBMAKER)
	./libmaker -csrc $(CSRC)
	(cd .. ; make; cd lib)

do_asys: $(LIBMAKER)
	./libmaker -asys $(ASYS)
	(cd .. ; make; cd lib)

do_csys: $(LIBMAKER)
	./libmaker -csys $(CSYS)
	(cd .. ; make; cd lib)

do_nsys: $(LIBMAKER)
	./libmaker -nsys $(NSYS)
	(cd .. ; make; cd lib)

do_psys: $(LIBMAKER)
	./libmaker -psys $(PSYS)
	(cd .. ; make; cd lib)

clean:
	rm -f *.o $(LIBMAKER)




