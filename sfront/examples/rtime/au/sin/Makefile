
#    Sfront, a SAOL to C translator    
#    This file: Creates a MusicDevice AudioUnit
#
# License below also covers SAOL and SASL programs in this directory.
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


## To use this Makefile:
##
## "make":  Creates a AU component from your SAOL file in this directory
## "make install": Installs your plug-in and launches an AU host for testing
## "make clean": Removes all files created by "make" from this directory
## "make remove": Un-installs your plugin, and does a "make clean"
##

## 
## Use the variables below to configure how this Makefile works.
## See Part V/4 of the MP4SA book for more details. Note that
## YES and NO are case-sensitive -- yes and no won't work properly.
##

## To compile the AU binary using gcc's debug mode, set to YES

GCC_DEBUG = NO

## To compile SAOL file with the wiretap debug facility, set to YES
## Wiretap mode creates clicks in audio output; do not use for production code

WIRETAP_DEBUG = NO

## Change to YES to compile against the Snow Leopard SDK (and above)
## The default NO creates a component that only runs on your native OS/CPU

SNOW_LEOPARD = NO

## The project name: must match SAOL file name (without .saol suffix)
## Project names must not use white-space characters (space, tab, etc)
## This Makefile only works for MusicDevice projects.

PROJECT_NAME = sin

## The 4-character codes for AU component subtype and manufacturer

COMPONENT_SUBTYPE_CODE = snsn
COMPONENT_MANUFACTURER_CODE = ucBe

## Name and Manufacturer strings shown in the AU Host user interface

USER_INTERFACE_NAME = "Sinewave Synth"
USER_INTERFACE_MANUFACTURER = "John Lazzaro, UCB EECS"

## Manufacturer URL (expressed using Java-Classpath-string syntax)

MANUFACTURER_URL = edu.berkeley.eecs

## Mac OS X AU library directory in which to install your plug-in

COMPONENT_LIBRARY = ~/Library/Audio/Plug-Ins/Components

## The Terminal command launched after installing your plug-in

AUHOST_LAUNCH =	open $(PROJECT_NAME).band

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

ifeq ($(WIRETAP_DEBUG), YES)
	OUTMODE = -aout audiounit_debug
else
	OUTMODE = -aout audiounit
endif

ifeq ($(SNOW_LEOPARD), YES)
	SNOW_LEOPARD_FLAGS = -isysroot /Developer/SDKs/MacOSX10.6.sdk
	R_SNOW_LEOPARD_FLAGS = -isysroot /Developer/SDKs/MacOSX10.6.sdk
else
	SNOW_LEOPARD_FLAGS = 
	R_SNOW_LEOPARD_FLAGS = 
endif

ifdef COMPONENT_SUBTYPE_CODE
	COMP_SUBTYPE = -au_component_subtype $(COMPONENT_SUBTYPE_CODE)
else
	COMP_SUBTYPE =
endif

ifdef COMPONENT_MANUFACTURER_CODE
	COMP_MANU = -au_component_manu $(COMPONENT_MANUFACTURER_CODE)
else
	COMP_MANU =
endif

ifdef USER_INTERFACE_NAME
	UI_NAME =  -au_ui_name $(USER_INTERFACE_NAME)
else
	UI_NAME = 
endif

ifdef USER_INTERFACE_MANUFACTURER
	UI_MANU = -au_ui_manu $(USER_INTERFACE_MANUFACTURER)
else
	UI_MANU =
endif

ifdef MANUFACTURER_URL
	MANU_URL =  -au_manu_url $(MANUFACTURER_URL)
else
	MANU_URL = 
endif

# sfront command-line definitions

SFRONT = sfront
BINARYFILE = ./$(PROJECT_NAME).component/Contents/MacOS/$(PROJECT_NAME)
DRIVERS = -cin aucontrolm $(OUTMODE)
FS_NAME =  -au_filesystem_name $(PROJECT_NAME)
AU_ID =  $(UI_NAME) $(COMP_SUBTYPE) $(COMP_MANU) $(UI_MANU) $(MANU_URL)

# gcc command-line definitions

CC = gcc 
OPT = -O3
MODEL = -m32
CFLAGS = $(MODEL) $(OPT) $(DEBUG) $(SNOW_LEOPARD_FLAGS)
IOLINK = -framework AudioUnit -framework AudioToolbox -framework CoreAudio -framework CoreServices -bundle

# rezedit definitions

RSRC = ./$(PROJECT_NAME).component/Contents/Resources/$(PROJECT_NAME).rsrc
RINCLUDES = -I /System/Library/Frameworks/CoreServices.framework/Frameworks/CarbonCore.framework/Versions/A/Headers
RFLAGS = -useDF -d SystemSevenOrLater=1 -script Roman -d "ppc_$ppc" -d i386_YES -arch i386  

# rules

$(BINARYFILE): $(PROJECT_NAME).saol 
	-rm -rf $(PROJECT_NAME).component
	mkdir $(PROJECT_NAME).component{,/Contents,/Contents/{Resources,MacOS}}
	$(SFRONT) $(DRIVERS) $(AU_ID) -orc $(PROJECT_NAME).saol 
	$(CC) $(CFLAGS) sa.c -lm $(IOLINK) -o $(BINARYFILE)
	Rez $(RFLAGS) $(R_SNOW_LEOPARD_FLAGS) $(RINCLUDES) -o $(RSRC) ./$(PROJECT_NAME).r
	-rm -rf $(PROJECT_NAME).r
	mv -f Info.Plist $(PROJECT_NAME).component/Contents/
	printf "BNDL????" > $(PROJECT_NAME).component/Contents/PkgInfo

install: $(BINARYFILE)
	-rm -rf $(COMPONENT_LIBRARY)/$(PROJECT_NAME).component
	cp -r $(PROJECT_NAME).component $(COMPONENT_LIBRARY)/
	$(AUHOST_LAUNCH)

clean: 
	-rm -rf sa.c *~ safe Info.Plist $(PROJECT_NAME).r $(PROJECT_NAME).component

remove: clean
	-rm -rf $(COMPONENT_LIBRARY)/$(PROJECT_NAME).component

