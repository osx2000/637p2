
#    Sfront, a SAOL to C translator
#    This file: Creates CocoaFilterView for lpf
#
# License only covers the Makefile. All other files contain the
# license Apple uses to allow redistribution of its sample SDK code.
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


## This Makefile is called by ../Makefile.  The Make variables
## GCC_DEBUG, UNIVERSAL, COCOA_VIEW_BUNDLE_NAME, OPT, and CC are
## passed in by ../Makefile.  Attempts to change these variables
## in this Makefile will be ignored by make.

##
## The lines below do the actual work of the Makefile.
## You will normally not need to edit the lines below.
##

# conditionals to parse user configuration YES/NO strings

ifeq ($(GCC_DEBUG), YES)
	DEBUG = -g
else
	DEBUG =
endif

ifeq ($(SNOW_LEOPARD), YES)
	SNOW_LEOPARD_FLAGS = -isysroot /Developer/SDKs/MacOSX10.6.sdk
else
	SNOW_LEOPARD_FLAGS = 
endif

# gcc command-line definitions

OBJECTIVEC = -x objective-c 
CFLAGS = $(OPT) $(MODEL) $(SNOW_LEOPARD_FLAGS) $(DEBUG) $(OBJECTIVEC)
IOLINK = -framework AppKit -framework AudioUnit -framework AudioToolbox -bundle

OBJS = UCBLopassFilter_GraphView.o UCBLopassFilter_UIView.o\
	UCBLopassFilter_ViewFactory.o

# rules

$(COCOA_VIEW_BUNDLE_NAME).bundle/Contents/MacOS/$(COCOA_VIEW_BUNDLE_NAME) : $(OBJS)
	$(CC) $(OPT) $(MODEL) $(SNOW_LEOPARD_FLAGS) $(DEBUG) $(OBJS) $(IOLINK) -o $(COCOA_VIEW_BUNDLE_NAME).bundle/Contents/MacOS/$(COCOA_VIEW_BUNDLE_NAME)

clean: 
	-rm -rf *.o *~ 

install: $(COCOA_VIEW_BUNDLE_NAME).bundle/Contents/MacOS/$(COCOA_VIEW_BUNDLE_NAME)

remove: clean
	-rm $(COCOA_VIEW_BUNDLE_NAME).bundle/Contents/MacOS/$(COCOA_VIEW_BUNDLE_NAME)	