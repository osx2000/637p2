

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


FILENAME = linbuzz

SAOLFILE = $(FILENAME).saol
SASLFILE = $(FILENAME).sasl
MP4FILE  = $(FILENAME).mp4

INFILE = 
OUTFILE = all


CC = gcc
OPT = -O3
CFLAGS = $(OPT)

# a relative path just so it works
# out of the box in this directory.
# Use an absolute path, or set the
# SFRONT_INCLUDE_PATH environment
# variable to the absolute path,
# so that the -Is flag is not needed.

LIBPATH = ../../../lib
LIBMODE = -gcc -cpp -Is $(LIBPATH)

SFRONT = sfront 


## try 0.004 if clicks happen

OUTMODE = -latency 0.002 -aout linux
INMODE = 
CNMODE = -cin linmidi

## for INFILE/OUTFILE selections that need libraries

IOLINK = 

## for INFILE/OUTFILE std selection:  > foo < bar

REDIRECT = 

##
## makes a raw 16-bit signed integer audio file
##
 
$(OUTFILE): $(SAOLFILE) 
	$(SFRONT) $(LIBMODE) $(OUTMODE) $(CNMODE) -orc $(SAOLFILE) -sco $(SASLFILE)
	$(CC) $(CFLAGS) sa.c -lm $(IOLINK) -o sa
	./sa $(REDIRECT)

## tests bitstream creation code, by creating an MP4 file, decoding
## it, and comparing audio out with audio created by original ASCII
## files. doesn't work with stdin/stdout.

mp4test	: $(OUTFILE)
	rm -rf $(MP4FILE) sa.c
	mv $(OUTFILE) safe
	$(SFRONT) $(LIBMODE) -symtab -orc $(SAOLFILE) -bitout $(MP4FILE)
	$(SFRONT) $(LIBMODE) $(OUTMODE) $(INMODE) -bit $(MP4FILE)
	$(CC) $(CFLAGS) sa.c -lm $(IOLINK) -o sa
	./sa $(REDIRECT)
	cmp $(OUTFILE) safe

## tests a new sfront against an old one. assume safe is the audio
## created by old sfront. 

compare	: $(OUTFILE)
	cmp $(OUTFILE) safe

safe	: $(OUTFILE)
	cp $(OUTFILE) safe

## for performance testing
##

timing	: 
	./sa $(REDIRECT)

##
##
##

## only works on HPUX, for WAV files

play  : $(OUTFILE)
	splayer $(OUTFILE)

clean: 
	rm -rf sa.c sa audio a*.wav $(MP4FILE) $(OUTFILE) *.info *~ safe



