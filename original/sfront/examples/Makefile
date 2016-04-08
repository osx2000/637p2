
#    Sfront, a SAOL to C translator    
#    This file: Makefile for sfront examples
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


SHELL = /bin/sh

# 
# Runs all the examples. 
# For quicker testing, delete pc, elpelele, and scr1.
# do (cd $$i ; echo "Making in $$i..."; make clean; done

SIMPLE = bach min vowels claps perc beat in speedt gliss torvalds scr1 elpelele pc

PACKAGES = $(SIMPLE) 

## Makes audio files.

audio:
	for i in $(PACKAGES) ;\
	do (cd $$i ; echo "Making in $$i ..."; make -e ); done

## Makes mp4 testing.

mp4test:
	for i in $(PACKAGES) ;\
	do (cd $$i ; echo "Making in $$i ..."; make -e mp4test); done

## compares audio to safe in each directory

compare:
	for i in $(PACKAGES) ;\
	do (cd $$i ; echo "Making in $$i ..."; make -e compare); done

## for timing ./sa > audio

timing:
	for i in $(PACKAGES) ;\
	do (cd $$i ; echo "Making in $$i ..."; make -e timing); done

## copies audio to safe in each directory

safe:
	for i in $(PACKAGES) ;\
	do (cd $$i ; echo "Making in $$i ..."; make -e safe); done

## Plays files through D/A (system-dependent -- set up for HPUX)

play:
	for i in $(PACKAGES) ;\
	do (cd $$i ; echo "Making in $$i ..."; make -e play); done

## Deletes all files created during testing.

clean:
	for i in $(PACKAGES) ;\
	do (cd $$i ; echo "Making in $$i ..."; make -e clean); done


 
 
 
