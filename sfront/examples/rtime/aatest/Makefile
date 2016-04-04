
#    Sfront, a SAOL to C translator    
#    This file: Makefile for an sfront example
#
# License also applies to SAOL and SASL files. MIDI file is public 
# domain, courtesy of the http://www.mutopiaproject.org project
#
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


FILENAME = aatest

SAOLFILE = $(FILENAME).saol
SASLFILE = $(FILENAME).sasl
MIDIFILE = $(FILENAME).mid
MP4FILE  = $(FILENAME).mp4

INFILE = 

##
## By default, this is set up to work for Linux. To run one
## of the other UNIX platform tests, change OUTFILE to be
## the correct driver name, and see "IOLINK" comment below.
##
## OUTFILE = irix     ## For SGI IRIX
## OUTFILE = hpux     ## For HPUX
## OUTFILE = freebsd  ## For FreeBSD
## 
## See the README file in this directory for Microsoft Windows
## and Macintosh OS X compilation instructions.
##

OUTFILE = linux

CC = gcc
OPT = -O3
CFLAGS = $(OPT)
SFRONT = sfront 
PLAYER = play
CMP = cmp

OUTMODE = -aout $(OUTFILE)
INMODE = 

## IOLINK adds linking options for soundcard drivers. The
## default linux option uses no linking options.
##
## 
## IOLINK = -laudio   ## For SGI IRIX 
## IOLINK = -lAlib    ## For HPUX
## IOLINK =           ## FreeBSD needs no linking options.
##

IOLINK = 

## for INFILE/OUTFILE std selection:  > foo < bar

REDIRECT = 

##
## makes a raw 16-bit signed integer audio file
##
 
$(OUTFILE): $(SAOLFILE) $(MIDIFILE)
	$(SFRONT) $(OUTMODE) $(INMODE) -orc $(SAOLFILE) -sco $(SASLFILE) -midi $(MIDIFILE)
	$(CC) $(CFLAGS) sa.c -lm $(IOLINK) -o sa
	./sa $(REDIRECT)

## tests bitstream creation code, by creating an MP4 file, decoding
## it, and comparing audio out with audio created by original ASCII
## files. doesn't work with stdin/stdout.

mp4test	: $(OUTFILE)
	rm -rf $(MP4FILE) sa.c
	mv $(OUTFILE) safe
	$(SFRONT)  -orc $(SAOLFILE) -midi $(MIDIFILE) -sco $(SASLFILE) -bitout $(MP4FILE)
	$(SFRONT) $(OUTMODE) $(INMODE) -bit $(MP4FILE)
	$(CC) $(CFLAGS) sa.c -lm $(IOLINK) -o sa
	./sa $(REDIRECT)
	$(CMP) $(OUTFILE) safe

## tests a new sfront against an old one. assume safe is the audio
## created by old sfront. 

compare	: $(OUTFILE)
	$(CMP) $(OUTFILE) safe

safe	: $(OUTFILE)
	cp $(OUTFILE) safe

## for performance testing
##

timing	: 
	/usr/bin/time -p ./sa 

##
##
##

play  : $(OUTFILE)
	$(PLAYER) $(OUTFILE)

clean: 
	rm -rf sa.c sa audio a*.wav $(MP4FILE) $(OUTFILE) *.info *~ safe



