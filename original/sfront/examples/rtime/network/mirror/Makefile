
#    Sfront, a SAOL to C translator    
#    This file: Makefile for an sfront example
#
# Copyright (c) 2000-2006, Regents of the University of California
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



##
## sets network session to special mirror session

NETWORK = -session mirror

##
## Audio output driver, as shown below configured for linux.
## To run one of the other UNIX platform tests, change AOUT to be
## the correct driver name, and see "IOLINK" comment below.
##
## AOUT = -aout irix     ## For SGI IRIX
## AOUT = -aout hpux     ## For HPUX
## AOUT = -aout freebsd  ## For FreeBSD
## 
## See the README file for Macintosh OS X compilation instructions.
##

AOUT = -aout linux 


##
## control driver, as shown below configured for ascii keyboard. 
## if you have a MIDI keyboard, the linmidi driver (OSS) is known to work,
## other MIDI jack control drivers are untested but should work too.

CIN = -cin ascii


##
## for AOUT and CIN selections that need libraries linked with sa.c
## Linux needs no linking options. For other platforms:
## 
## IOLINK = -laudio   ## For SGI IRIX 
## IOLINK = -lAlib    ## For HPUX
## IOLINK =           ## FreeBSD needs no linking options.
##

IOLINK = 


##
## end of audio/network/control configuration
##

FILENAME = mirror

SAOLFILE = $(FILENAME).saol
SASLFILE = $(FILENAME).sasl

CC = gcc
OPT = -O3
CFLAGS = $(OPT)
SFRONT = sfront

$(FILENAME): $(SAOLFILE) 
	$(SFRONT) $(NETWORK) $(AOUT) $(CIN) -orc $(SAOLFILE) -sco $(SASLFILE)
	$(CC) $(CFLAGS) sa.c -lm $(IOLINK) -o sa
	./sa 

clean: 
	rm -rf sa.c sa audio a*.wav $(MP4FILE) $(OUTFILE) *.info *~ safe



