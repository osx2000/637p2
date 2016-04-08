
#    Sfront, a SAOL to C translator    
#    This file: Makefile for Sfront
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
## The current version and release date.
##
##             0.99 08/30/11

##
## BINDIR specifies the directory for the object sfront file.
## The "make install" option uses BINDIR, and the examples
## supplied with sfront assume this directory is in your path.
## It defaults to the sfront/bin directory.

BINDIR = ../bin/

##
## Compiler optimization and debug options.
## 

CC = gcc 
OPT  = -O2 
DBUG =  -g
ECHECK = -ansi -Wall
MODEL = -m32

##
## Windows/UWIN users should replace this line with
##
## SFRONT = sfront.exe
##

SFRONT = sfront

##
## Machine/OS specific compiling options should be added
## to CFLAGS below.
##

CFLAGS = $(MODEL) $(OPT) $(DBUG) $(ECHECK)

## A few files gcc compiles very slowly on some platforms,
## so we add this flag to fix it.  Delete for other compilers.

SLOWSCHED = -fno-schedule-insns

##
## Machine/OS specific linking options should be added to
## LINKLIB below

LINKLIB = -lm

## Changes below this line shouldn't be needed -- if they are,
## add an appropriate definition above this line and refer to
## it below.

OBJS = parser.tab.o lex.o symbols.o globals.o treeupdate.o wtparse.o wtconst.o\
	corevars.o corecode.o special.o postparse.o readscore.o sfmain.o\
	writeorc.o blocktree.o writepre.o writeop.o writemain.o readmidi.o\
	mp4write.o tokens.o ascwrite.o mp4read.o collapse.o coreinline.o\
	audio.o control.o cmainpass.o csrclib.o asyslib.o csyslib.o \
	psyslib.o nsyslib.o parsehelp.o optconst.o optrate.o optrefer.o \
	optmain.o oclone.o stparse.o

## package maintainers need this to be first

all: $(SFRONT)

install: $(SFRONT)
	cp $(SFRONT) $(BINDIR)

clean:
	rm -f *.o $(SFRONT)

cleanall:
	rm -f *.o sfront $(BINDIR)sfront sfront.exe parser.tab.c parser.tab.h

csrclib.o:  csrclib.c tree.h parser.tab.h
	$(CC) $(CFLAGS) $(SLOWSCHED) -c -o csrclib.o csrclib.c

asyslib.o:  asyslib.c tree.h parser.tab.h
	$(CC) $(CFLAGS) $(SLOWSCHED) -c -o asyslib.o asyslib.c

csyslib.o:  csyslib.c tree.h parser.tab.h
	$(CC) $(CFLAGS) $(SLOWSCHED) -c -o csyslib.o csyslib.c

psyslib.o:  psyslib.c tree.h parser.tab.h
	$(CC) $(CFLAGS) $(SLOWSCHED) -c -o psyslib.o psyslib.c

nsyslib.o:  nsyslib.c tree.h parser.tab.h
	$(CC) $(CFLAGS) $(SLOWSCHED) -c -o nsyslib.o nsyslib.c

$(SFRONT):  $(OBJS) tree.h parser.tab.h
	$(CC) $(CFLAGS) $(OBJS) $(LINKLIB) -o $(SFRONT)

parser.tab.c:	parsehelp.c parser.y tree.h
	-bison -d parser.y	


##
## This section makes a distribution, in the directory
## $(BTYPE)dist which is at the same directory level as 
## sfront directory this Makefile resides in. To get
## this to work, you'll need to change the VERSION 
## string at the top of the file to be --VERSION--.
## You'll also need to change SFPLACE, SBPLACE and some "
## "cp" commands to copy documentation into the directories.
##
## If you're splitting off a new distribution, I'd suggest
## the following scheme: the original version number, followed
## by your initials, followed by your own version numbers. For
## example, Jane Q. Smith would split off version 0.5 into
## 0.5jqs0.1


## Name to prefix to binary distribution created by "make newdist"
##

BTYPE = gen

## New version number, date, and ID.
## Do "ls -lrt's" to check book freshness.
##

NEWVERSION = 0.99
NEWDATE = `date '+%m/%d/%y'`
NEWIDSTRING = $(NEWVERSION) $(NEWDATE)

## New versions of the sfront manual and book
##

SFPLACE = /Users/lazzaro/work/code/sfront/html/sfman/
SBPLACE = /Users/lazzaro/work/code/sfront/html/book/
CPPLACE = /Users/lazzaro/work/code/sfront/html/copyright/

newdist: 
	rm -rf ../../$(BTYPE)dist
	mkdir ../../$(BTYPE)dist
	mkdir ../../$(BTYPE)dist/sfront
	mkdir ../../$(BTYPE)dist/bin
	mkdir ../../$(BTYPE)dist/sfront/bin
	mkdir ../../$(BTYPE)dist/sfront/lib
	mkdir ../../$(BTYPE)dist/sfront/lib/Slib
	mkdir ../../$(BTYPE)dist/sfront/lib/reson
	mkdir ../../$(BTYPE)dist/sfront/src
	mkdir ../../$(BTYPE)dist/sfront/src/win
	mkdir ../../$(BTYPE)dist/sfront/src/lib
	mkdir ../../$(BTYPE)dist/sfront/src/lib/csrc
	mkdir ../../$(BTYPE)dist/sfront/src/lib/asys
	mkdir ../../$(BTYPE)dist/sfront/src/lib/csys
	mkdir ../../$(BTYPE)dist/sfront/src/lib/nsys
	mkdir ../../$(BTYPE)dist/sfront/src/lib/psys
	mkdir ../../$(BTYPE)dist/sfront/examples
	mkdir ../../$(BTYPE)dist/sfront/examples/rtime
	mkdir ../../$(BTYPE)dist/sfront/examples/rtime/au
	mkdir ../../$(BTYPE)dist/sfront/examples/rtime/network
	mkdir ../../$(BTYPE)dist/sfront/copyright
	mkdir ../../$(BTYPE)dist/sfront/sfman
	mkdir ../../$(BTYPE)dist/sfront/sfman/devel
	mkdir ../../$(BTYPE)dist/sfront/sfman/devel/adriver
	mkdir ../../$(BTYPE)dist/sfront/sfman/devel/intro
	mkdir ../../$(BTYPE)dist/sfront/sfman/devel/cdriver
	mkdir ../../$(BTYPE)dist/sfront/sfman/devel/cdriver/intro
	mkdir ../../$(BTYPE)dist/sfront/sfman/devel/cdriver/code
	mkdir ../../$(BTYPE)dist/sfront/sfman/devel/cdriver/data
	mkdir ../../$(BTYPE)dist/sfront/sfman/user
	mkdir ../../$(BTYPE)dist/sfront/sfman/user/cmdline
	mkdir ../../$(BTYPE)dist/sfront/sfman/user/cmdline/alsaseq
	mkdir ../../$(BTYPE)dist/sfront/sfman/user/install
	mkdir ../../$(BTYPE)dist/sfront/sfman/user/network
	mkdir ../../$(BTYPE)dist/sfront/sfman/user/ref
	mkdir ../../$(BTYPE)dist/sfront/sfman/user/rtime
	mkdir ../../$(BTYPE)dist/sfront/sfman/user/use
	mkdir ../../$(BTYPE)dist/sfront/book
	mkdir ../../$(BTYPE)dist/sfront/book/ack
	mkdir ../../$(BTYPE)dist/sfront/book/append
	mkdir ../../$(BTYPE)dist/sfront/book/append/cor1
	mkdir ../../$(BTYPE)dist/sfront/book/append/cor2/
	mkdir ../../$(BTYPE)dist/sfront/book/append/cor2/doc/
	mkdir ../../$(BTYPE)dist/sfront/book/append/corea
	mkdir ../../$(BTYPE)dist/sfront/book/append/coref
	mkdir ../../$(BTYPE)dist/sfront/book/append/lang
	mkdir ../../$(BTYPE)dist/sfront/book/append/notcov
	mkdir ../../$(BTYPE)dist/sfront/book/append/rules
	mkdir ../../$(BTYPE)dist/sfront/book/append/stnames
	mkdir ../../$(BTYPE)dist/sfront/book/append/wave
	mkdir ../../$(BTYPE)dist/sfront/book/bib
	mkdir ../../$(BTYPE)dist/sfront/book/control
	mkdir ../../$(BTYPE)dist/sfront/book/control/midi
	mkdir ../../$(BTYPE)dist/sfront/book/control/saolc
	mkdir ../../$(BTYPE)dist/sfront/book/control/sasl
	mkdir ../../$(BTYPE)dist/sfront/book/opcodes
	mkdir ../../$(BTYPE)dist/sfront/book/opcodes/filter
	mkdir ../../$(BTYPE)dist/sfront/book/opcodes/sgen
	mkdir ../../$(BTYPE)dist/sfront/book/opcodes/sproc
	mkdir ../../$(BTYPE)dist/sfront/book/opcodes/user
	mkdir ../../$(BTYPE)dist/sfront/book/opcodes/user/example
	mkdir ../../$(BTYPE)dist/sfront/book/opcodes/user/example/osine
	mkdir ../../$(BTYPE)dist/sfront/book/saol
	mkdir ../../$(BTYPE)dist/sfront/book/saol/bus
	mkdir ../../$(BTYPE)dist/sfront/book/saol/exstat
	mkdir ../../$(BTYPE)dist/sfront/book/saol/simple
	mkdir ../../$(BTYPE)dist/sfront/book/saol/vars
	mkdir ../../$(BTYPE)dist/sfront/book/saol/wave/
	mkdir ../../$(BTYPE)dist/sfront/book/saol/wave/examples
	mkdir ../../$(BTYPE)dist/sfront/book/saol/wave/examples/tsine
	mkdir ../../$(BTYPE)dist/sfront/book/simple
	mkdir ../../$(BTYPE)dist/sfront/book/simple/sine
	mkdir ../../$(BTYPE)dist/sfront/book/special
	mkdir ../../$(BTYPE)dist/sfront/book/special/debug
	mkdir ../../$(BTYPE)dist/sfront/book/special/slib
	mkdir ../../$(BTYPE)dist/sfront/book/special/au
	mkdir ../../$(BTYPE)dist/sfront/book/template
	mkdir ../../$(BTYPE)dist/sfront/book/tut
	mkdir ../../$(BTYPE)dist/sfront/book/tut/examples
	mkdir ../../$(BTYPE)dist/sfront/book/tut/examples/sine
	mkdir ../../$(BTYPE)dist/sfront/book/tut/examples/vcsine
	mkdir ../../$(BTYPE)dist/sfront/book/tut/examples/vsine
	cp ../lib/Slib/gmidi.hs  ../../$(BTYPE)dist/sfront/lib/Slib/
	cp ../lib/Slib/ssm.hs  ../../$(BTYPE)dist/sfront/lib/Slib/
	cp ../lib/Slib/std.hs  ../../$(BTYPE)dist/sfront/lib/Slib/
	cp ../lib/Slib/trig.hs  ../../$(BTYPE)dist/sfront/lib/Slib/
	cp ../lib/reson/README  ../../$(BTYPE)dist/sfront/lib/reson/
	cp ../lib/reson/abar.hs  ../../$(BTYPE)dist/sfront/lib/reson/
	cp ../lib/reson/macros.hs  ../../$(BTYPE)dist/sfront/lib/reson/
	cp ../lib/reson/marimba.hs  ../../$(BTYPE)dist/sfront/lib/reson/
	cp ../lib/reson/sreson.hs  ../../$(BTYPE)dist/sfront/lib/reson/
	cp ../lib/reson/string.hs  ../../$(BTYPE)dist/sfront/lib/reson/
	cp ../README.HTML ../../$(BTYPE)dist/sfront/README.HTML
	cp ../README.HTML ../../$(BTYPE)dist/sfront/index.html
	cp lex.c ../../$(BTYPE)dist/sfront/src/
	cp symbols.c ../../$(BTYPE)dist/sfront/src/
	cp globals.c ../../$(BTYPE)dist/sfront/src/
	cp treeupdate.c ../../$(BTYPE)dist/sfront/src/
	cp wtparse.c ../../$(BTYPE)dist/sfront/src/
	cp wtconst.c ../../$(BTYPE)dist/sfront/src/
	cp stparse.c ../../$(BTYPE)dist/sfront/src/
	cp corevars.c ../../$(BTYPE)dist/sfront/src/
	cp corecode.c ../../$(BTYPE)dist/sfront/src/
	cp special.c ../../$(BTYPE)dist/sfront/src/
	cp postparse.c ../../$(BTYPE)dist/sfront/src/
	cp readscore.c ../../$(BTYPE)dist/sfront/src/
	cp sfmain.c ../../$(BTYPE)dist/sfront/src/
	cp writeorc.c ../../$(BTYPE)dist/sfront/src/
	cp blocktree.c ../../$(BTYPE)dist/sfront/src/
	cp oclone.c ../../$(BTYPE)dist/sfront/src/
	cp writepre.c ../../$(BTYPE)dist/sfront/src/
	cp writeop.c ../../$(BTYPE)dist/sfront/src/
	cp writemain.c ../../$(BTYPE)dist/sfront/src/
	cp readmidi.c ../../$(BTYPE)dist/sfront/src/
	cp mp4write.c ../../$(BTYPE)dist/sfront/src/
	cp tokens.c ../../$(BTYPE)dist/sfront/src/
	cp ascwrite.c ../../$(BTYPE)dist/sfront/src/
	cp mp4read.c ../../$(BTYPE)dist/sfront/src/
	cp collapse.c ../../$(BTYPE)dist/sfront/src/
	cp optconst.c ../../$(BTYPE)dist/sfront/src/
	cp optrate.c ../../$(BTYPE)dist/sfront/src/
	cp optrefer.c ../../$(BTYPE)dist/sfront/src/
	cp optmain.c ../../$(BTYPE)dist/sfront/src/
	cp coreinline.c ../../$(BTYPE)dist/sfront/src/
	cp audio.c ../../$(BTYPE)dist/sfront/src/
	cp control.c ../../$(BTYPE)dist/sfront/src/
	cp cmainpass.c ../../$(BTYPE)dist/sfront/src/
	(cd lib; make cfiles; make clean)
	cp csrclib.c ../../$(BTYPE)dist/sfront/src/
	cp csyslib.c ../../$(BTYPE)dist/sfront/src/
	cp asyslib.c ../../$(BTYPE)dist/sfront/src/
	cp psyslib.c ../../$(BTYPE)dist/sfront/src/
	cp nsyslib.c ../../$(BTYPE)dist/sfront/src/
	cp csrclib.h ../../$(BTYPE)dist/sfront/src/
	cp csyslib.h ../../$(BTYPE)dist/sfront/src/
	cp asyslib.h ../../$(BTYPE)dist/sfront/src/
	cp psyslib.h ../../$(BTYPE)dist/sfront/src/
	cp nsyslib.h ../../$(BTYPE)dist/sfront/src/
	touch parser.tab.c
	cp parser.y ../../$(BTYPE)dist/sfront/src/
	cp parsehelp.c ../../$(BTYPE)dist/sfront/src/
	sed -e "s;0.99 08/30/11;$(NEWIDSTRING);" Makefile > tmpmakefile
	mv tmpmakefile ../../$(BTYPE)dist/sfront/src/Makefile
	sed -e "s;0.99 08/30/11;$(NEWIDSTRING);" tree.h > tmptree.h
	cp tmptree.h ../../$(BTYPE)dist/sfront/src/tree.h
	cp parser.tab.c ../../$(BTYPE)dist/sfront/src/
	cp parser.tab.h ../../$(BTYPE)dist/sfront/src/
	cp README.HTML ../../$(BTYPE)dist/sfront/src/
	cp README.HTML ../../$(BTYPE)dist/sfront/bin/
	cp NETWORK.HTML ../../$(BTYPE)dist/sfront/src/
	cp win/sfront.dsp ../../$(BTYPE)dist/sfront/src/win/
	cp win/sfront.dsw ../../$(BTYPE)dist/sfront/src/win/
	cp lib/Makefile ../../$(BTYPE)dist/sfront/src/lib/
	cp lib/libmaker.c ../../$(BTYPE)dist/sfront/src/lib/
	cp lib/libmaker.h ../../$(BTYPE)dist/sfront/src/lib/
	cp lib/README.HTML ../../$(BTYPE)dist/sfront/src/lib/
	cp lib/csrc/callback.c ../../$(BTYPE)dist/sfront/src/lib/csrc/
	cp lib/csrc/preamble.c ../../$(BTYPE)dist/sfront/src/lib/csrc/
	cp lib/csrc/psync.c ../../$(BTYPE)dist/sfront/src/lib/csrc/
	cp lib/csrc/robust.c ../../$(BTYPE)dist/sfront/src/lib/csrc/
	cp lib/csrc/runspt.c ../../$(BTYPE)dist/sfront/src/lib/csrc/
	cp lib/csrc/runtime.c ../../$(BTYPE)dist/sfront/src/lib/csrc/
	cp lib/csrc/runtimef.c ../../$(BTYPE)dist/sfront/src/lib/csrc/
	cp lib/csrc/tgen.c ../../$(BTYPE)dist/sfront/src/lib/csrc/
	cp lib/csrc/tsync.c ../../$(BTYPE)dist/sfront/src/lib/csrc/
	cp lib/nsys/net_crypto.c ../../$(BTYPE)dist/sfront/src/lib/nsys/
	cp lib/nsys/net_globals.c ../../$(BTYPE)dist/sfront/src/lib/nsys/
	cp lib/nsys/net_include.c ../../$(BTYPE)dist/sfront/src/lib/nsys/
	cp lib/nsys/net_jrecv.c ../../$(BTYPE)dist/sfront/src/lib/nsys/
	cp lib/nsys/net_jsend.c ../../$(BTYPE)dist/sfront/src/lib/nsys/
	cp lib/nsys/net_rtcplib.c ../../$(BTYPE)dist/sfront/src/lib/nsys/
	cp lib/nsys/net_rtplib.c ../../$(BTYPE)dist/sfront/src/lib/nsys/
	cp lib/nsys/net_sfront.c ../../$(BTYPE)dist/sfront/src/lib/nsys/
	cp lib/nsys/net_siplib.c ../../$(BTYPE)dist/sfront/src/lib/nsys/
	cp lib/psys/pa_porthdr.c ../../$(BTYPE)dist/sfront/src/lib/psys/
	cp lib/psys/pa_hosthdr.c ../../$(BTYPE)dist/sfront/src/lib/psys/
	cp lib/psys/pa_tracehdr.c ../../$(BTYPE)dist/sfront/src/lib/psys/
	cp lib/psys/pa_lib.c ../../$(BTYPE)dist/sfront/src/lib/psys/
	cp lib/psys/pa_unix_oss.c ../../$(BTYPE)dist/sfront/src/lib/psys/
	cp lib/psys/pa_win_wmme.c ../../$(BTYPE)dist/sfront/src/lib/psys/
	cp lib/psys/pa_dshdr.c ../../$(BTYPE)dist/sfront/src/lib/psys/
	cp lib/psys/pa_dswrap.c ../../$(BTYPE)dist/sfront/src/lib/psys/
	cp lib/psys/pa_dsound.c  ../../$(BTYPE)dist/sfront/src/lib/psys/ 
	cp lib/asys/std.c    ../../$(BTYPE)dist/sfront/src/lib/asys/
	cp lib/asys/null.c    ../../$(BTYPE)dist/sfront/src/lib/asys/
	cp lib/asys/raw.c    ../../$(BTYPE)dist/sfront/src/lib/asys/
	cp lib/asys/wav.c    ../../$(BTYPE)dist/sfront/src/lib/asys/
	cp lib/asys/aif.c    ../../$(BTYPE)dist/sfront/src/lib/asys/
	cp lib/asys/vcdat.c    ../../$(BTYPE)dist/sfront/src/lib/asys/
	cp lib/asys/hpux.c   ../../$(BTYPE)dist/sfront/src/lib/asys/
	cp lib/asys/irix.c   ../../$(BTYPE)dist/sfront/src/lib/asys/
	cp lib/asys/linux.c   ../../$(BTYPE)dist/sfront/src/lib/asys/
	cp lib/asys/freebsd.c   ../../$(BTYPE)dist/sfront/src/lib/asys/
	cp lib/asys/dsound.c   ../../$(BTYPE)dist/sfront/src/lib/asys/
	cp lib/asys/portaudio.c   ../../$(BTYPE)dist/sfront/src/lib/asys/
	cp lib/asys/coreaudio.c ../../$(BTYPE)dist/sfront/src/lib/asys/
	cp lib/asys/audiounit.c ../../$(BTYPE)dist/sfront/src/lib/asys/
	cp lib/asys/wiretap.c ../../$(BTYPE)dist/sfront/src/lib/asys/
	cp lib/csys/gliss.c    ../../$(BTYPE)dist/sfront/src/lib/csys/
	cp lib/csys/ascii.c    ../../$(BTYPE)dist/sfront/src/lib/csys/
	cp lib/csys/coremidi.c    ../../$(BTYPE)dist/sfront/src/lib/csys/
	cp lib/csys/win32.c    ../../$(BTYPE)dist/sfront/src/lib/csys/
	cp lib/csys/linmidi.c    ../../$(BTYPE)dist/sfront/src/lib/csys/
	cp lib/csys/freebsdmidi.c  ../../$(BTYPE)dist/sfront/src/lib/csys/
	cp lib/csys/alsamidi.c    ../../$(BTYPE)dist/sfront/src/lib/csys/
	cp lib/csys/alsaseq.c    ../../$(BTYPE)dist/sfront/src/lib/csys/
	cp lib/csys/fstr.c    ../../$(BTYPE)dist/sfront/src/lib/csys/
	cp lib/csys/aucontrol.c    ../../$(BTYPE)dist/sfront/src/lib/csys/
	(cd ../examples; make clean;)
	(cd ../examples/RTIME; make clean;)
	cp -r ../examples/bach ../../$(BTYPE)dist/sfront/examples/
	cp -r ../examples/beat ../../$(BTYPE)dist/sfront/examples/
	cp -r ../examples/claps ../../$(BTYPE)dist/sfront/examples/
	cp -r ../examples/elpelele ../../$(BTYPE)dist/sfront/examples/
	cp -r ../examples/torvalds ../../$(BTYPE)dist/sfront/examples/
	cp -r ../examples/in ../../$(BTYPE)dist/sfront/examples/
	cp -r ../examples/min ../../$(BTYPE)dist/sfront/examples/
	cp -r ../examples/gliss ../../$(BTYPE)dist/sfront/examples/
	cp -r ../examples/pc ../../$(BTYPE)dist/sfront/examples/
	cp -r ../examples/perc ../../$(BTYPE)dist/sfront/examples/
	cp -r ../examples/scr1 ../../$(BTYPE)dist/sfront/examples/
	cp -r ../examples/speedt ../../$(BTYPE)dist/sfront/examples/
	cp -r ../examples/vowels ../../$(BTYPE)dist/sfront/examples/
	cp -r ../examples/Make.dist ../../$(BTYPE)dist/sfront/examples/Makefile
	cp -r ../examples/README.HTML ../../$(BTYPE)dist/sfront/examples/
	cp -r ../examples/RTIME/aatest ../../$(BTYPE)dist/sfront/examples/rtime/
	cp -r ../examples/RTIME/ascii ../../$(BTYPE)dist/sfront/examples/rtime/
	cp -r ../examples/RTIME/linmidi ../../$(BTYPE)dist/sfront/examples/rtime/
	cp -r ../examples/RTIME/linbuzz ../../$(BTYPE)dist/sfront/examples/rtime/
	cp -r ../examples/RTIME/linvoc ../../$(BTYPE)dist/sfront/examples/rtime/
	cp -r ../examples/RTIME/linain ../../$(BTYPE)dist/sfront/examples/rtime/
	cp -r ../examples/RTIME/network/README ../../$(BTYPE)dist/sfront/examples/rtime/network/
	cp -r ../examples/RTIME/network/mirror ../../$(BTYPE)dist/sfront/examples/rtime/network/
	cp -r ../examples/RTIME/network/nmp_audio ../../$(BTYPE)dist/sfront/examples/rtime/network/
	cp -r ../examples/RTIME/network/nmp_null ../../$(BTYPE)dist/sfront/examples/rtime/network/
	cp -r ../examples/RTIME/network/nmp_stream ../../$(BTYPE)dist/sfront/examples/rtime/network/
	cp -r ../examples/RTIME/au/hiss ../../$(BTYPE)dist/sfront/examples/rtime/au/
	cp -r ../examples/RTIME/au/sin ../../$(BTYPE)dist/sfront/examples/rtime/au/
	cp -r ../examples/RTIME/au/lpf ../../$(BTYPE)dist/sfront/examples/rtime/au/
	cp  $(CPPLACE)index.html ../../$(BTYPE)dist/sfront/copyright/
	cp  $(CPPLACE)gfdl.html ../../$(BTYPE)dist/sfront/copyright/
	cp  $(CPPLACE)bsd.html  ../../$(BTYPE)dist/sfront/copyright/
	cp  $(SFPLACE)index.html \
	../../$(BTYPE)dist/sfront/sfman/
	cp  $(SFPLACE)user/cmdline/index.html \
            ../../$(BTYPE)dist/sfront/sfman/user/cmdline/
	cp  $(SFPLACE)user/cmdline/alsaseq/index.html \
            ../../$(BTYPE)dist/sfront/sfman/user/cmdline/alsaseq/
	cp  $(SFPLACE)user/install/index.html \
            ../../$(BTYPE)dist/sfront/sfman/user/install/
	cp  $(SFPLACE)user/network/index.html \
            ../../$(BTYPE)dist/sfront/sfman/user/network/
	cp  $(SFPLACE)user/ref/index.html \
            ../../$(BTYPE)dist/sfront/sfman/user/ref/
	cp  $(SFPLACE)user/rtime/index.html \
            ../../$(BTYPE)dist/sfront/sfman/user/rtime/
	cp  $(SFPLACE)user/use/index.html \
            ../../$(BTYPE)dist/sfront/sfman/user/use/
	cp  $(SFPLACE)devel/intro/index.html \
            ../../$(BTYPE)dist/sfront/sfman/devel/intro/
	cp  $(SFPLACE)devel/adriver/index.html \
            ../../$(BTYPE)dist/sfront/sfman/devel/adriver/
	cp  $(SFPLACE)devel/cdriver/intro/index.html \
            ../../$(BTYPE)dist/sfront/sfman/devel/cdriver/intro/
	cp  $(SFPLACE)devel/cdriver/code/index.html \
            ../../$(BTYPE)dist/sfront/sfman/devel/cdriver/code/
	cp  $(SFPLACE)devel/cdriver/data/index.html \
            ../../$(BTYPE)dist/sfront/sfman/devel/cdriver/data/
	cp  $(SBPLACE)index.html \
	../../$(BTYPE)dist/sfront/book/
	cp  $(SBPLACE)ack/index.html \
	../../$(BTYPE)dist/sfront/book/ack/
	cp  $(SBPLACE)append/index.html \
	../../$(BTYPE)dist/sfront/book/append/
	cp  $(SBPLACE)append/cor1/index.html \
	../../$(BTYPE)dist/sfront/book/append/cor1/
	cp  $(SBPLACE)append/cor2/index.html \
	../../$(BTYPE)dist/sfront/book/append/cor2/
	cp  $(SBPLACE)append/cor2/doc/index.html \
	../../$(BTYPE)dist/sfront/book/append/cor2/doc/
	cp  $(SBPLACE)append/corea/index.html \
	../../$(BTYPE)dist/sfront/book/append/corea/
	cp  $(SBPLACE)append/coref/index.html \
	../../$(BTYPE)dist/sfront/book/append/coref/
	cp  $(SBPLACE)append/lang/index.html \
	../../$(BTYPE)dist/sfront/book/append/lang/
	cp  $(SBPLACE)append/notcov/index.html \
	../../$(BTYPE)dist/sfront/book/append/notcov
	cp  $(SBPLACE)append/rules/index.html \
	../../$(BTYPE)dist/sfront/book/append/rules/
	cp  $(SBPLACE)append/stnames/index.html \
	../../$(BTYPE)dist/sfront/book/append/stnames/
	cp  $(SBPLACE)append/wave/index.html \
	../../$(BTYPE)dist/sfront/book/append/wave/
	cp  $(SBPLACE)bib/index.html \
	../../$(BTYPE)dist/sfront/book/bib/
	cp  $(SBPLACE)control/index.html \
	../../$(BTYPE)dist/sfront/book/control/
	cp  $(SBPLACE)control/midi/index.html \
	../../$(BTYPE)dist/sfront/book/control/midi/
	cp  $(SBPLACE)control/saolc/index.html \
	../../$(BTYPE)dist/sfront/book/control/saolc/
	cp  $(SBPLACE)control/sasl/index.html \
	../../$(BTYPE)dist/sfront/book/control/sasl/
	cp  $(SBPLACE)opcodes/index.html \
	../../$(BTYPE)dist/sfront/book/opcodes/
	cp  $(SBPLACE)opcodes/filter/index.html \
	../../$(BTYPE)dist/sfront/book/opcodes/filter/
	cp  $(SBPLACE)opcodes/sgen/index.html \
	../../$(BTYPE)dist/sfront/book/opcodes/sgen/
	cp  $(SBPLACE)opcodes/sproc/index.html \
	../../$(BTYPE)dist/sfront/book/opcodes/sproc/
	cp  $(SBPLACE)opcodes/user/index.html \
	../../$(BTYPE)dist/sfront/book/opcodes/user/
	cp  $(SBPLACE)opcodes/user/example/osine/{osine.saol,osine.sasl} \
	../../$(BTYPE)dist/sfront/book/opcodes/user/example/osine/
	cp  $(SBPLACE)saol/index.html \
	../../$(BTYPE)dist/sfront/book/saol/
	cp  $(SBPLACE)saol/bus/index.html \
	../../$(BTYPE)dist/sfront/book/saol/bus/
	cp  $(SBPLACE)saol/exstat/index.html \
	../../$(BTYPE)dist/sfront/book/saol/exstat/
	cp  $(SBPLACE)saol/simple/index.html \
	../../$(BTYPE)dist/sfront/book/saol/simple/
	cp  $(SBPLACE)saol/vars/index.html \
	../../$(BTYPE)dist/sfront/book/saol/vars/
	cp  $(SBPLACE)saol/wave/index.html \
	../../$(BTYPE)dist/sfront/book/saol/wave/
	cp  $(SBPLACE)saol/wave/examples/tsine/tsine.saol \
	../../$(BTYPE)dist/sfront/book/saol/wave/examples/tsine/
	cp  $(SBPLACE)simple/index.html \
	../../$(BTYPE)dist/sfront/book/simple/
	cp  $(SBPLACE)simple/sine/{sine.saol,sine.sasl,sa.c,sine.mp4} \
	../../$(BTYPE)dist/sfront/book/simple/sine/
	cp  $(SBPLACE)special/index.html \
	../../$(BTYPE)dist/sfront/book/special/
	cp  $(SBPLACE)special/debug/index.html \
	../../$(BTYPE)dist/sfront/book/special/debug/
	cp  $(SBPLACE)special/slib/index.html \
	../../$(BTYPE)dist/sfront/book/special/slib/
	cp  $(SBPLACE)special/au/index.html \
	../../$(BTYPE)dist/sfront/book/special/au/
	cp  $(SBPLACE)special/au/hpf.png \
	../../$(BTYPE)dist/sfront/book/special/au/
	cp  $(SBPLACE)special/au/hiss.png \
	../../$(BTYPE)dist/sfront/book/special/au/
	cp  $(SBPLACE)special/au/factory.png \
	../../$(BTYPE)dist/sfront/book/special/au/
	cp  $(SBPLACE)special/au/lpf.png \
	../../$(BTYPE)dist/sfront/book/special/au/
	cp  $(SBPLACE)template/index.html \
	../../$(BTYPE)dist/sfront/book/template/
	cp  $(SBPLACE)tut/index.html \
	../../$(BTYPE)dist/sfront/book/tut/
	cp  $(SBPLACE)tut/examples/sine/{sa.c,sine.saol,sine.sasl} \
	../../$(BTYPE)dist/sfront/book/tut/examples/sine/
	cp  $(SBPLACE)tut/examples/vcsine/{vcsine.saol,vcsine.sasl} \
	../../$(BTYPE)dist/sfront/book/tut/examples/vcsine/
	cp  $(SBPLACE)tut/examples/vsine/{vsine.saol,vsine.sasl} \
	../../$(BTYPE)dist/sfront/book/tut/examples/vsine/
	(cd ../../$(BTYPE)dist/ ; tar -cvf sfront$(NEWVERSION).tar sfront; gzip sfront$(NEWVERSION).tar)
	(cd ../../$(BTYPE)dist/ ; zip -r sfront$(NEWVERSION).zip sfront)
	(cd ../../$(BTYPE)dist/sfront ; rm -rf examples/bach)
	(cd ../../$(BTYPE)dist/sfront ; rm -rf examples/elpelele)
	(cd ../../$(BTYPE)dist/sfront ; rm -rf examples/gliss)
	(cd ../../$(BTYPE)dist/sfront ; rm -rf examples/perc)
	(cd ../../$(BTYPE)dist/sfront ; rm -rf examples/scr1)
	(cd ../../$(BTYPE)dist/sfront ; rm -rf examples/speedt)
	(cd ../../$(BTYPE)dist/sfront ; rm -rf examples/torvalds)
	cp  ../examples/Make.lite ../../$(BTYPE)dist/sfront/examples/Makefile
	(cd ../../$(BTYPE)dist/ ; tar -cvf sfront$(NEWVERSION)lite.tar sfront; gzip sfront$(NEWVERSION)lite.tar)
	(cd ../../$(BTYPE)dist/ ; zip -r sfront$(NEWVERSION)lite.zip sfront)
	(cd ../../$(BTYPE)dist/ ; rm -rf bin sfront)
	mv ../../$(BTYPE)dist/* ../update/
	rm -rf ../../$(BTYPE)dist
	rm -f tmptree.h


##      Example syntax for conditionals
##	if [ "$(BTYPE)" != "gen" ]; then (cd ../../$(BTYPE)dist; cp bin/$(SFRONT) sfront/bin/); fi





