

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


FILENAME = ascii

SAOLFILE = $(FILENAME).saol
SASLFILE = $(FILENAME).sasl

##
## By default, this is set up to work for Linux. To run one
## of the other UNIX platform tests, change OUTFILE to be
## the correct driver name, and see "IOLINK" comment below.
##
## OUTFILE = irix     ## For SGI IRIX
## OUTFILE = hpux     ## For HPUX
## OUTFILE = freebsd  ## For FreeBSD
## 
## See the README file for Macintosh OS X compilation instructions.
##

OUTFILE = linux

CC = gcc
OPT = -O3
CFLAGS = $(OPT)
SFRONT = sfront 

OUTMODE = -aout $(OUTFILE)
INMODE = 
CNMODE = -cin ascii


## for INFILE/OUTFILE selections that need libraries
## IOLINK adds linking options for soundcard drivers. The
## default linux option uses no linking options.
##
## 
## IOLINK = -laudio   ## For SGI IRIX 
## IOLINK = -lAlib    ## For HPUX
## IOLINK =           ## FreeBSD needs no linking options.
##

IOLINK = 

##
## makes a raw 16-bit signed integer audio file
##
 
$(OUTFILE): $(SAOLFILE) 
	$(SFRONT) $(OUTMODE) $(CNMODE) -orc $(SAOLFILE) -sco $(SASLFILE)
	$(CC) $(CFLAGS) sa.c -lm $(IOLINK) -o sa
	./sa 

clean: 
	rm -rf sa.c sa audio a*.wav $(MP4FILE) $(OUTFILE) *.info *~ safe



