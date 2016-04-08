
/*
#    Sfront, a SAOL to C translator    
#    This file: wiretap logger for audiounit driver for sfront
#
# Copyright (c) 1999-2008, Regents of the University of California
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


/****************************************************************/
/****************************************************************/
/*          Wiretap debugger for audio driver for sfront        */
/****************************************************************/

/*~~~~~~~~~~~~~*/
/* debug level */
/*_____________*/

/*  Level 0:  No debugging messages            */
/*  Level 1:  Session setup and error messages */
/*  Level 2:  + All MIDI events                */
/*  Level 3:  + All Rendering calls            */

#if !defined(ASYS_AUDIOUNIT_DEBUG_LEVEL)
#define ASYS_AUDIOUNIT_DEBUG_LEVEL 2
#endif

/*~~~~~~~~~~~~~~~~~*/
/* include headers */
/*_________________*/

#if !defined(__COREAUDIO_USE_FLAT_INCLUDES__)
#include <CoreServices/CoreServices.h>
#include <AudioUnit/AudioUnit.h>
#include <CoreAudio/AudioHardware.h>
#else
#include <ConditionalMacros.h>
#include <CoreServices.h>
#include <AudioUnit.h>
#include <AudioHardware.h>
#endif

/*~~~~~~~~~~~~~~~~~~*/
/* endian detection */
/*__________________*/

SInt32 asysn_audiounit_sint32_endian_test = -2;

#define ASYS_AUDIOUNIT_SINT32_BIGENDIAN \
        ((((char *)(&asysn_audiounit_sint32_endian_test))[0]) == ((char)(-1)))

/****************************************************************/
/*                The Wiretap Debugging System                  */
/****************************************************************/

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/* Global Variables and Constants */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#define ASYS_AUDIOUNIT_LOGFILE_NAME "/tmp/wiretap.txt"

int asysn_audiounit_first_logfile_open = 1;
FILE * asysn_audiounit_logfile;

enum
{
  asysn_audiounit_PropertySizeUnknown = -1,
  asysn_audiounit_PropertySizeWildCard = -2,
};

extern int asysn_audiounit_opencount;   /* number of audiounit instances */

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/* Low-Level Logfile Management */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/********************************************/
/* clears logfile at the start of a session */
/********************************************/

void asysn_audiounit_logfile_initialize(void) 
{
  asysn_audiounit_logfile = fopen(ASYS_AUDIOUNIT_LOGFILE_NAME, "w");
  fclose(asysn_audiounit_logfile);
  asysn_audiounit_first_logfile_open = 0;
}


/*~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~*/
/* Name-Printing Routines */
/*~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~*/

/*************************/
/* prints out scope name */
/*************************/

void asysn_audiounit_print_scope_name(int scope)

{
  switch(scope) {
  case kAudioUnitScope_Global:  /* 0 */
    fprintf(asysn_audiounit_logfile, "kAudioUnitScope_Global", scope);
    break;
  case kAudioUnitScope_Input:	/* 1 */
    fprintf(asysn_audiounit_logfile, "kAudioUnitScope_Input", scope);
    break;
  case kAudioUnitScope_Output:  /* 2 */
    fprintf(asysn_audiounit_logfile, "kAudioUnitScope_Output", scope);
    break;
  case kAudioUnitScope_Group:   /* 3 */
    fprintf(asysn_audiounit_logfile, "kAudioUnitScope_Group", scope);
    break;
  case kAudioUnitScope_Part:    /* 4 */
    fprintf(asysn_audiounit_logfile, "kAudioUnitScope_Part", scope);
    break;
  default:
    fprintf(asysn_audiounit_logfile, "(%i)", scope);
    break;
  }
}

/****************************/
/* prints out selector name */
/****************************/

void asysn_audiounit_print_selector_name(int selector)

{
  switch((int)((SInt16)(selector))) {

  case kComponentOpenSelect:
    fprintf(asysn_audiounit_logfile, "kComponentOpenSelect");
    break;
  case kComponentCloseSelect:
    fprintf(asysn_audiounit_logfile, "kComponentCloseSelect");
    break;
  case kComponentCanDoSelect:
    fprintf(asysn_audiounit_logfile, "kComponentCanDoSelect");
    break;
  case kComponentVersionSelect:
    fprintf(asysn_audiounit_logfile, "kComponentVersionSelect");
    break;
  case kComponentRegisterSelect:
    fprintf(asysn_audiounit_logfile, "kComponentRegisterSelect");
    break;
  case kComponentTargetSelect:
    fprintf(asysn_audiounit_logfile, "kComponentTargetSelect");
    break;
  case kComponentUnregisterSelect:
    fprintf(asysn_audiounit_logfile, "kComponentUnregisterSelect");
    break;
  case kComponentGetMPWorkFunctionSelect:
    fprintf(asysn_audiounit_logfile, "kComponentGetMPWorkFunctionSelect");
    break;
  case kComponentExecuteWiredActionSelect:
    fprintf(asysn_audiounit_logfile, "kComponentExecuteWiredActionSelect");
    break;
  case kComponentGetPublicResourceSelect:
    fprintf(asysn_audiounit_logfile, "kComponentGetPublicResourceSelect");
    break;
  case kAudioUnitRange:
    fprintf(asysn_audiounit_logfile, "kAudioUnitRange");
    break;
  case kAudioUnitInitializeSelect:
    fprintf(asysn_audiounit_logfile, "kAudioUnitInitializeSelect");
    break;
  case kAudioUnitUninitializeSelect:
    fprintf(asysn_audiounit_logfile, "kAudioUnitUninitializeSelect");
    break;
  case kAudioUnitGetPropertyInfoSelect:
    fprintf(asysn_audiounit_logfile, "kAudioUnitGetPropertyInfoSelect");
    break;
  case kAudioUnitGetPropertySelect:
    fprintf(asysn_audiounit_logfile, "kAudioUnitGetPropertySelect");
    break;
  case kAudioUnitSetPropertySelect:
    fprintf(asysn_audiounit_logfile, "kAudioUnitSetPropertySelect");
    break;
  case kAudioUnitAddPropertyListenerSelect:
    fprintf(asysn_audiounit_logfile, "kAudioUnitAddPropertyListenerSelect");
    break;
  case kAudioUnitRemovePropertyListenerSelect:
    fprintf(asysn_audiounit_logfile, "kAudioUnitRemovePropertyListenerSelect");
    break;
  case kAudioUnitRemovePropertyListenerWithUserDataSelect:
    fprintf(asysn_audiounit_logfile, "kAudioUnitRemovePropertyListenerWithUserDataSelect");
    break;
  case kAudioUnitAddRenderNotifySelect:
    fprintf(asysn_audiounit_logfile, "kAudioUnitAddRenderNotifySelect");
    break;
  case kAudioUnitRemoveRenderNotifySelect:
    fprintf(asysn_audiounit_logfile, "kAudioUnitRemoveRenderNotifySelect");
    break;
  case kAudioUnitGetParameterSelect:
    fprintf(asysn_audiounit_logfile, "kAudioUnitGetParameterSelect");
    break;
  case kAudioUnitSetParameterSelect:
    fprintf(asysn_audiounit_logfile, "kAudioUnitSetParameterSelect");
    break;
  case kAudioUnitScheduleParametersSelect:
    fprintf(asysn_audiounit_logfile, "kAudioUnitScheduleParametersSelect");
    break;
  case kAudioUnitRenderSelect:
    fprintf(asysn_audiounit_logfile, "kAudioUnitRenderSelect");
    break;
  case kAudioUnitResetSelect:
    fprintf(asysn_audiounit_logfile, "kAudioUnitResetSelect");
    break;
  case kMusicDeviceMIDIEventSelect:
    fprintf(asysn_audiounit_logfile, "kMusicDeviceMIDIEventSelect");
    break;
  case kMusicDeviceSysExSelect:
    fprintf(asysn_audiounit_logfile, "kMusicDeviceSysExSelect");
    break;
  case kMusicDevicePrepareInstrumentSelect:
    fprintf(asysn_audiounit_logfile, "kMusicDevicePrepareInstrumentSelect");
    break;
  case kMusicDeviceReleaseInstrumentSelect:
    fprintf(asysn_audiounit_logfile, "kMusicDeviceReleaseInstrumentSelect");
    break;
  case kMusicDeviceStartNoteSelect:
    fprintf(asysn_audiounit_logfile, "kMusicDeviceStartNoteSelect");
    break;
  case kMusicDeviceStopNoteSelect:
    fprintf(asysn_audiounit_logfile, "kMusicDeviceStopNoteSelect");
    break;
  default:
    fprintf(asysn_audiounit_logfile, "(%i)", (int)((SInt16)(selector)));
    break;
  }
}

/*******************************/
/* prints out return code name */
/*******************************/

void asysn_audiounit_print_returncode_name(ComponentResult returncode)

{
  switch (returncode) {
  case noErr:
    fprintf(asysn_audiounit_logfile, "noErr (%i)", 
	    (int) returncode);
    break;
  case badComponentInstance:
    fprintf(asysn_audiounit_logfile, "badComponentInstance (%i)", 
	    (int) returncode);
    break;
  case badComponentSelector:
    fprintf(asysn_audiounit_logfile, "badComponentSelector (%i)", 
	    (int) returncode);
    break;
  case kAudioUnitErr_InvalidProperty:
    fprintf(asysn_audiounit_logfile, "kAudioUnitErr_InvalidProperty (%i)", 
	    (int) returncode);
    break;
  case kAudioUnitErr_InvalidParameter:
    fprintf(asysn_audiounit_logfile, "kAudioUnitErr_InvalidParameter (%i)", 
	    (int) returncode);
    break;
  case kAudioUnitErr_InvalidElement:
    fprintf(asysn_audiounit_logfile, "kAudioUnitErr_InvalidElement (%i)", 
	    (int) returncode);
    break;
  case kAudioUnitErr_NoConnection:
    fprintf(asysn_audiounit_logfile, "kAudioUnitErr_NoConnection (%i)", 
	    (int) returncode);
    break;
  case kAudioUnitErr_FailedInitialization:
    fprintf(asysn_audiounit_logfile, "kAudioUnitErr_FailedInitialization (%i)", 
	    (int) returncode);
    break;
  case kAudioUnitErr_TooManyFramesToProcess:
    fprintf(asysn_audiounit_logfile, "kAudioUnitErr_TooManyFramesToProcess (%i)", 
	    (int) returncode);
    break;
  case kAudioUnitErr_IllegalInstrument:
    fprintf(asysn_audiounit_logfile, "kAudioUnitErr_IllegalInstrument (%i)", 
	    (int) returncode);
    break;
  case kAudioUnitErr_InstrumentTypeNotFound:
    fprintf(asysn_audiounit_logfile, "kAudioUnitErr_InstrumentTypeNotFound (%i)", 
	    (int) returncode);
    break;
  case kAudioUnitErr_InvalidFile:
    fprintf(asysn_audiounit_logfile, "kAudioUnitErr_InvalidFile (%i)", 
	    (int) returncode);
    break;
  case kAudioUnitErr_UnknownFileType:
    fprintf(asysn_audiounit_logfile, "kAudioUnitErr_UnknownFileType (%i)", 
	    (int) returncode);
    break;
  case kAudioUnitErr_FileNotSpecified:
    fprintf(asysn_audiounit_logfile, "kAudioUnitErr_FileNotSpecified (%i)", 
	    (int) returncode);
    break;
  case kAudioUnitErr_FormatNotSupported:
    fprintf(asysn_audiounit_logfile, "kAudioUnitErr_FormatNotSupported (%i)", 
	    (int) returncode);
    break;
  case kAudioUnitErr_Uninitialized:
    fprintf(asysn_audiounit_logfile, "kAudioUnitErr_Uninitialized (%i)", 
	    (int) returncode);
    break;
  case kAudioUnitErr_InvalidScope:
    fprintf(asysn_audiounit_logfile, "kAudioUnitErr_InvalidScope (%i)", 
	    (int) returncode);
    break;
  case kAudioUnitErr_PropertyNotWritable:
    fprintf(asysn_audiounit_logfile, "kAudioUnitErr_PropertyNotWritable (%i)", 
	    (int) returncode);
    break;
  case kAudioUnitErr_CannotDoInCurrentContext:
    fprintf(asysn_audiounit_logfile, "kAudioUnitErr_CannotDoInCurrentContext (%i)", 
	    (int) returncode);
    break;
  case kAudioUnitErr_InvalidPropertyValue:
    fprintf(asysn_audiounit_logfile, "kAudioUnitErr_InvalidPropertyValue (%i)", 
	    (int) returncode);
    break;
  case kAudioUnitErr_PropertyNotInUse:
    fprintf(asysn_audiounit_logfile, "kAudioUnitErr_PropertyNotInUse (%i)", 
	    (int) returncode);
    break;
  case kAudioUnitErr_Initialized:
    fprintf(asysn_audiounit_logfile, "kAudioUnitErr_Initialized (%i)", 
	    (int) returncode);
    break;
  case kAudioUnitErr_InvalidOfflineRender:
    fprintf(asysn_audiounit_logfile, "kAudioUnitErr_InvalidOfflineRender (%i)", 
	    (int) returncode);
    break;
  case kAudioUnitErr_Unauthorized:
    fprintf(asysn_audiounit_logfile, "kAudioUnitErr_Unauthorized (%i)", 
	    (int) returncode);
    break;
  default:
    fprintf(asysn_audiounit_logfile, "(%i)", 
	    (int) returncode);
    break;
  }
}

/**************************************/
/* prints return code if interesting  */
/**************************************/

void asysn_audiounit_print_nonzero_returncode(ComponentResult returncode)
{
  if (returncode)
    {
      fprintf(asysn_audiounit_logfile, "\n\tReturning "); 
      asysn_audiounit_print_returncode_name(returncode);
    }
}

/****************************/
/* prints out property name */
/****************************/

void asysn_audiounit_print_property_name(int property)

{
  switch(property) {
  case kAudioUnitProperty_ClassInfo: /* 0 */
    fprintf(asysn_audiounit_logfile, "kAudioUnitProperty_ClassInfo");
    break;
  case kAudioUnitProperty_MakeConnection: /* 1 */
    fprintf(asysn_audiounit_logfile, "kAudioUnitProperty_MakeConnection");
    break;
  case kAudioUnitProperty_SampleRate: /* 2 */
    fprintf(asysn_audiounit_logfile, "kAudioUnitProperty_SampleRate");
    break;
  case kAudioUnitProperty_ParameterList: /* 3 */
    fprintf(asysn_audiounit_logfile, "kAudioUnitProperty_ParameterList");
    break;
  case kAudioUnitProperty_ParameterInfo: /* 4 */
    fprintf(asysn_audiounit_logfile, "kAudioUnitProperty_ParameterInfo");
    break;
  case kAudioUnitProperty_FastDispatch: /* 5 */
    fprintf(asysn_audiounit_logfile, "kAudioUnitProperty_FastDispatch");
    break;
  case kAudioUnitProperty_CPULoad: /* 6 */
    fprintf(asysn_audiounit_logfile, "kAudioUnitProperty_CPULoad");
    break;
  case kAudioUnitProperty_StreamFormat: /* 8 */
    fprintf(asysn_audiounit_logfile, "kAudioUnitProperty_StreamFormat");
    break;
  case kAudioUnitProperty_SRCAlgorithm: /* 9 */
    fprintf(asysn_audiounit_logfile, "kAudioUnitProperty_SRCAlgorithm");
    break;
  case kAudioUnitProperty_ReverbRoomType: /* 10 */
    fprintf(asysn_audiounit_logfile, "kAudioUnitProperty_ReverbRoomType");
    break;
  case kAudioUnitProperty_ElementCount: /* 11, also kAudioUnitProperty_BusCount */
    fprintf(asysn_audiounit_logfile, "kAudioUnitProperty_ElementCount_or_BusCount");
    break;
  case kAudioUnitProperty_Latency: /* 12 */
    fprintf(asysn_audiounit_logfile, "kAudioUnitProperty_Latency");
    break;
  case kAudioUnitProperty_SupportedNumChannels: /* 13 */
    fprintf(asysn_audiounit_logfile, "kAudioUnitProperty_SupportedNumChannels");
    break;
  case kAudioUnitProperty_MaximumFramesPerSlice: /* 14 */
    fprintf(asysn_audiounit_logfile, "kAudioUnitProperty_MaximumFramesPerSlice");
    break;
  case kAudioUnitProperty_SetExternalBuffer: /* 15 */
    fprintf(asysn_audiounit_logfile, "kAudioUnitProperty_SetExternalBuffer");
    break;
  case kAudioUnitProperty_ParameterValueStrings: /* 16 */
    fprintf(asysn_audiounit_logfile, "kAudioUnitProperty_ParameterValueStrings");
    break;
  case kAudioUnitProperty_MIDIControlMapping: /* 17 */ 
    fprintf(asysn_audiounit_logfile, "kAudioUnitProperty_MIDIControlMapping");
    break;
  case kAudioUnitProperty_GetUIComponentList: /* 18 */
    fprintf(asysn_audiounit_logfile, "kAudioUnitProperty_GetUIComponentList");
    break;
  case kAudioUnitProperty_AudioChannelLayout: /* 19 */  
    fprintf(asysn_audiounit_logfile, "kAudioUnitProperty_AudioChannelLayout");
    break;
  case kAudioUnitProperty_TailTime: /* 20 */
    fprintf(asysn_audiounit_logfile, "kAudioUnitProperty_TailTime");
    break;
  case kAudioUnitProperty_BypassEffect: /* 21 */
    fprintf(asysn_audiounit_logfile, "kAudioUnitProperty_BypassEffect");
    break;
  case kAudioUnitProperty_LastRenderError: /* 22 */
    fprintf(asysn_audiounit_logfile, "kAudioUnitProperty_LastRenderError");
    break;
  case kAudioUnitProperty_SetRenderCallback: /* 23 */
    fprintf(asysn_audiounit_logfile, "kAudioUnitProperty_SetRenderCallback");
    break;
  case kAudioUnitProperty_FactoryPresets: /* 24 */
    fprintf(asysn_audiounit_logfile, "kAudioUnitProperty_FactoryPresets");
    break;
  case kAudioUnitProperty_ContextName: /* 25 */
    fprintf(asysn_audiounit_logfile, "kAudioUnitProperty_ContextName");
    break;
  case kAudioUnitProperty_RenderQuality: /* 26 */
    fprintf(asysn_audiounit_logfile, "kAudioUnitProperty_RenderQuality");
    break;
  case kAudioUnitProperty_HostCallbacks: /* 27 */
    fprintf(asysn_audiounit_logfile, "kAudioUnitProperty_HostCallbacks");
    break;
  case kAudioUnitProperty_CurrentPreset: /* 28 */
    fprintf(asysn_audiounit_logfile, "kAudioUnitProperty_CurrentPreset");
    break;
  case kAudioUnitProperty_InPlaceProcessing: /* 29 */
    fprintf(asysn_audiounit_logfile, "kAudioUnitProperty_InPlaceProcessing");
    break;
  case kAudioUnitProperty_ElementName: /* 30 */
    fprintf(asysn_audiounit_logfile, "kAudioUnitProperty_ElementName");
    break;
  case kAudioUnitProperty_CocoaUI: /* 31 */
    fprintf(asysn_audiounit_logfile, "kAudioUnitProperty_CocoaUI");
    break;
  case kAudioUnitProperty_SupportedChannelLayoutTags: /* 32 */
    fprintf(asysn_audiounit_logfile, "kAudioUnitProperty_SupportedChannelLayoutTags");
    break;
  case kAudioUnitProperty_ParameterStringFromValue: /* 33 */
    /* also kAudioUnitProperty_ParameterValueName */
    fprintf(asysn_audiounit_logfile, "kAudioUnitProperty_ParameterStringFromValue");
    break;
  case kAudioUnitProperty_ParameterIDName: /* 34 */
    fprintf(asysn_audiounit_logfile, "kAudioUnitProperty_ParameterIDName");
    break;
  case kAudioUnitProperty_ParameterClumpName: /* 35 */
    fprintf(asysn_audiounit_logfile, "kAudioUnitProperty_ParameterClumpName");
    break;
  case kAudioUnitProperty_PresentPreset: /* 36 */
    fprintf(asysn_audiounit_logfile, "kAudioUnitProperty_PresentPreset");
    break;
  case kAudioUnitProperty_OfflineRender: /* 37 */
    fprintf(asysn_audiounit_logfile, "kAudioUnitProperty_OfflineRender");
    break;
  case kAudioUnitProperty_ParameterValueFromString: /* 38 */
    fprintf(asysn_audiounit_logfile, "kAudioUnitProperty_ParameterValueFromString");
    break;
  case kAudioUnitProperty_IconLocation: /* 39 */
    fprintf(asysn_audiounit_logfile, "kAudioUnitProperty_IconLocation");
    break;
  case kAudioUnitProperty_PresentationLatency: /* 40 */
    fprintf(asysn_audiounit_logfile, "kAudioUnitProperty_PresentationLatency");
    break;
  case kAudioUnitProperty_AllParameterMIDIMappings: /* 41 */
    fprintf(asysn_audiounit_logfile, "kAudioUnitProperty_AllParameterMIDIMappings");
    break;
  case kAudioUnitProperty_AddParameterMIDIMapping: /* 42 */
    fprintf(asysn_audiounit_logfile, "kAudioUnitProperty_AddParameterMIDIMapping");
    break;
  case kAudioUnitProperty_RemoveParameterMIDIMapping: /* 43 */
    fprintf(asysn_audiounit_logfile, "kAudioUnitProperty_RemoveParameterMIDIMapping");
    break;
  case kAudioUnitProperty_HotMapParameterMIDIMapping: /* 44 */
    fprintf(asysn_audiounit_logfile, "kAudioUnitProperty_HotMapParameterMIDIMapping");
    break;
  case kAudioUnitProperty_DependentParameters: /* 45 */
    fprintf(asysn_audiounit_logfile, "kAudioUnitProperty_DependentParameters");
    break;
  case kAudioUnitProperty_AUHostIdentifier: /* 46 */
    fprintf(asysn_audiounit_logfile, "kAudioUnitProperty_AUHostIdentifier");
    break;
  case kAudioUnitProperty_MIDIOutputCallbackInfo: /* 47 */
    fprintf(asysn_audiounit_logfile, "kAudioUnitProperty_MIDIOutputCallbackInfo");
    break;
  case kAudioUnitProperty_ClassInfoFromDocument: /* 50 */
    fprintf(asysn_audiounit_logfile, "kAudioUnitProperty_ClassInfoFromDocument");
    break;
  case kMusicDeviceProperty_InstrumentCount: /* 1000 */
    fprintf(asysn_audiounit_logfile, "kAudioUnitProperty_InstrumentCount");
    break;
  case kMusicDeviceProperty_InstrumentName: /* 1001 */
    fprintf(asysn_audiounit_logfile, "kAudioUnitProperty_InstrumentName");
    break;
  case kMusicDeviceProperty_GroupOutputBus: /* 1002 */
    fprintf(asysn_audiounit_logfile, "kAudioUnitProperty_GroupOutputBus");
    break;
  case kMusicDeviceProperty_SoundBankFSSpec: /* 1003 */
    fprintf(asysn_audiounit_logfile, "kAudioUnitProperty_SoundBankFSSpec");
    break;
  case kMusicDeviceProperty_InstrumentNumber: /* 1004 */
    fprintf(asysn_audiounit_logfile, "kAudioUnitProperty_InstrumentNumber");
    break;
  case kMusicDeviceProperty_UsesInternalReverb: /* 1005 */ 
    /* also kAudioUnitProperty_UsesInternalReverb */
    fprintf(asysn_audiounit_logfile, "kAudioUnitProperty_UsesInternalReverb");
    break;
  case kMusicDeviceProperty_MIDIXMLNames: /* 1006 */
    fprintf(asysn_audiounit_logfile, "kAudioUnitProperty_MIDIXMLNames");
    break;
  case kMusicDeviceProperty_BankName: /* 1007 */
    fprintf(asysn_audiounit_logfile, "kAudioUnitProperty_BankName");
    break;
  case kMusicDeviceProperty_SoundBankData: /* 1008 */
    fprintf(asysn_audiounit_logfile, "kAudioUnitProperty_SoundBankData");
    break;
  case kMusicDeviceProperty_PartGroup: /* 1010 */
    fprintf(asysn_audiounit_logfile, "kAudioUnitProperty_PartGroup");
    break;
  case kMusicDeviceProperty_StreamFromDisk: /* 1011 */
    fprintf(asysn_audiounit_logfile, "kAudioUnitProperty_StreamFromDisk");
    break;
  case kMusicDeviceProperty_SoundBankFSRef: /* 1012 */
    fprintf(asysn_audiounit_logfile, "kAudioUnitProperty_SoundBankFSRef");
    break;
  case kAudioOutputUnitProperty_CurrentDevice: /* 2000 */
    fprintf(asysn_audiounit_logfile, "kAudioOutputUnitProperty_CurrentDevice");
    break;
  case kAudioOutputUnitProperty_IsRunning: /* 2001 */
    fprintf(asysn_audiounit_logfile, "kAudioOutputUnitProperty_IsRunning");
    break;
  case kAudioOutputUnitProperty_ChannelMap: /* 2002 */ 
    fprintf(asysn_audiounit_logfile, "kAudioOutputUnitProperty_ChannelMap");
    break;
  case kAudioOutputUnitProperty_EnableIO: /* 2003 */
    fprintf(asysn_audiounit_logfile, "kAudioOutputUnitProperty_EnableIO");
    break;
  case kAudioOutputUnitProperty_StartTime: /* 2004 */
    fprintf(asysn_audiounit_logfile, "kAudioOutputUnitProperty_StartTime");
    break;
  case kAudioOutputUnitProperty_SetInputCallback: /* 2005 */
    fprintf(asysn_audiounit_logfile, "kAudioOutputUnitProperty_SetInputCallback");
    break;
  case kAudioOutputUnitProperty_HasIO: /* 2006 */
    fprintf(asysn_audiounit_logfile, "kAudioOutputUnitProperty_HasIO");
    break;
  case kAudioOutputUnitProperty_StartTimestampsAtZero: /* 2007 */ 
    fprintf(asysn_audiounit_logfile, "kAudioOutputUnitProperty_StartTimestampsAtZero");
    break;

  case kAudioUnitProperty_SpatializationAlgorithm: /* 3000 */
    fprintf(asysn_audiounit_logfile, "kAudioUnitProperty_SpatializationAlgorithm");
    break;
  case kAudioUnitProperty_SpeakerConfiguration: /* 3001 */
    fprintf(asysn_audiounit_logfile, "kAudioUnitProperty_SpeakerConfiguration");
    break;
  case kAudioUnitProperty_DopplerShift: /* 3002 */
    fprintf(asysn_audiounit_logfile, "kAudioUnitProperty_DopplerShift");
    break;
  case kAudioUnitProperty_3DMixerRenderingFlags: /* 3003 */
    fprintf(asysn_audiounit_logfile, "kAudioUnitProperty_3DMixerRenderingFlags");
    break;
  case kAudioUnitProperty_3DMixerDistanceAtten: /* 3004 */
    fprintf(asysn_audiounit_logfile, "kAudioUnitProperty_3DMixerDistanceAtten");
    break;
  case kAudioUnitProperty_MatrixLevels: /* 3006 */
    fprintf(asysn_audiounit_logfile, "kAudioUnitProperty_MatrixLevels");
    break;
  case kAudioUnitProperty_MeteringMode: /* 3007 */
    fprintf(asysn_audiounit_logfile, "kAudioUnitProperty_MeteringMode");
    break;
#if (0)   /* depreciated */
  case kAudioUnitProperty_PannerMode: /* 3008 */
    fprintf(asysn_audiounit_logfile, "kAudioUnitProperty_PannerMode");
    break;
#endif
  case kAudioUnitProperty_MatrixDimensions: /* 3009 */
    fprintf(asysn_audiounit_logfile, "kAudioUnitProperty_MatrixDimensions");
    break;
  case kAudioUnitProperty_3DMixerDistanceParams: /* 3010 */
    fprintf(asysn_audiounit_logfile, "kAudioUnitProperty_3DMixerDistanceParams");
    break;
  case kAudioUnitProperty_MeterClipping: /* 3011 */ 
    fprintf(asysn_audiounit_logfile, "kAudioUnitProperty_MeterClipping");
    break;
  case kAudioOfflineUnitProperty_InputSize: /* 3020 */
    fprintf(asysn_audiounit_logfile, "kAudioOfflineUnitProperty_InputSize");
    break;
  case kAudioOfflineUnitProperty_OutputSize: /* 3021 */
    fprintf(asysn_audiounit_logfile, "kAudioOfflineUnitProperty_OutputSize");
    break;
  case kAudioUnitOfflineProperty_StartOffset: /* 3022 */
    fprintf(asysn_audiounit_logfile, "kAudioOfflineUnitProperty_StartOffset");
    break;
  case kAudioUnitOfflineProperty_PreflightRequirements: /* 3023 */
    fprintf(asysn_audiounit_logfile, "kAudioOfflineUnitProperty_PreflightRequirements");
    break;
  case kAudioUnitOfflineProperty_PreflightName: /* 3024 */
    fprintf(asysn_audiounit_logfile, "kAudioOfflineUnitProperty_PreflightName");
    break;

  case kAudioUnitProperty_ScheduleAudioSlice: /* 3300 */
    fprintf(asysn_audiounit_logfile, "kAudioUnitProperty_ScheduleAudioSlice");
    break;
  case kAudioUnitProperty_ScheduleStartTimeStamp: /* 3301 */ 
    fprintf(asysn_audiounit_logfile, "kAudioUnitProperty_ScheduleStartTimeStamp");
    break;
  case kAudioUnitProperty_CurrentPlayTime: /* 3302 */ 
    fprintf(asysn_audiounit_logfile, "kAudioUnitProperty_CurrentPlayTime");
    break;
  case kAudioUnitProperty_ScheduledFileIDs: /* 3310 */	
    fprintf(asysn_audiounit_logfile, "kAudioUnitProperty_ScheduledFileIDs");
    break;
  case kAudioUnitProperty_ScheduledFileRegion: /* 3311 */  
    fprintf(asysn_audiounit_logfile, "kAudioUnitProperty_ScheduledFileRegion");
    break;
  case kAudioUnitProperty_ScheduledFilePrime: /* 3312 */   
    fprintf(asysn_audiounit_logfile, "kAudioUnitProperty_ScheduledFilePrime");
    break;
  case kAudioUnitProperty_ScheduledFileBufferSizeFrames: /* 3313 */
    fprintf(asysn_audiounit_logfile, "kAudioUnitProperty_ScheduledFileBufferSizeFrames");
    break;
  case kAudioUnitProperty_ScheduledFileNumberBuffers: /* 3314 */	
    fprintf(asysn_audiounit_logfile, "kAudioUnitProperty_ScheduledFileNumberBuffers");
    break;
  case kAudioUnitProperty_DeferredRendererPullSize: /* 3320 */	
    fprintf(asysn_audiounit_logfile, "kAudioUnitProperty_DeferredRendererPullSize");
    break;
  case kAudioUnitProperty_DeferredRendererExtraLatency: /* 3321 */  
    fprintf(asysn_audiounit_logfile, "kAudioUnitProperty_DeferredRendererExtraLatency");
    break;
  case kAudioUnitProperty_DeferredRendererWaitFrames: /* 3322 */	
    fprintf(asysn_audiounit_logfile, "kAudioUnitProperty_DeferredRendererWaitFrames");
    break;
  case kAUNetReceiveProperty_Hostname: /* 3511 */
  case kAUNetReceiveProperty_Password: /* 3512 */
  case kAUNetSendProperty_PortNum: /* 3513 */
  case kAUNetSendProperty_TransmissionFormat: /* 3514 */
  case kAUNetSendProperty_TransmissionFormatIndex: /* 3515 */
  case kAUNetSendProperty_ServiceName: /* 3516 */
  case kAUNetSendProperty_Disconnect: /* 3517 */
  case kAUNetSendProperty_Password: /* 3518 */
    fprintf(asysn_audiounit_logfile, "AUNet Property %i", property);
    break;
  case kAudioUnitMigrateProperty_FromPlugin: /* 4000 */
    fprintf(asysn_audiounit_logfile, "kAudioUnitMigrateProperty_FromPlugin");
    break;
  case kAudioUnitMigrateProperty_OldAutomation: /* 4001 */
    fprintf(asysn_audiounit_logfile, "kAudioUnitMigrateProperty_OldAutomation");
    break;
  default:
    fprintf(asysn_audiounit_logfile, "(%i)", property);
    break;
  }
}

/*****************************/
/* prints out parameter name */
/*****************************/

void asysn_audiounit_print_parameter_name(int parameter)

{
  switch(parameter) {
  default:
    fprintf(asysn_audiounit_logfile, "(%i)", parameter);
    break;
  }
}

/******************************/
/* prints out parameter units */
/******************************/

void asysn_audiounit_print_parameter_units(int units)

{
  switch(units) {
  case kAudioUnitParameterUnit_Generic:
    fprintf(asysn_audiounit_logfile, "(generic)");
    break;
  case kAudioUnitParameterUnit_Indexed:
    fprintf(asysn_audiounit_logfile, "(indexed)");
    break;
  case kAudioUnitParameterUnit_Boolean:
    fprintf(asysn_audiounit_logfile, "(boolean)");
    break;
  case kAudioUnitParameterUnit_Percent:
    fprintf(asysn_audiounit_logfile, "%%");
    break;
  case kAudioUnitParameterUnit_Seconds:
    fprintf(asysn_audiounit_logfile, "s");
    break;
  case kAudioUnitParameterUnit_SampleFrames:
    fprintf(asysn_audiounit_logfile, "frames");
    break;
  case kAudioUnitParameterUnit_Phase:
    fprintf(asysn_audiounit_logfile, "degrees (phase)");
    break;
  case kAudioUnitParameterUnit_Rate:
    fprintf(asysn_audiounit_logfile, "1/s (rate)");
    break;
  case kAudioUnitParameterUnit_Hertz:
    fprintf(asysn_audiounit_logfile, "Hz");
    break;
  case kAudioUnitParameterUnit_Cents:
    fprintf(asysn_audiounit_logfile, "cents");
    break;
  case kAudioUnitParameterUnit_RelativeSemiTones:
    fprintf(asysn_audiounit_logfile, "semitones (relative)");
    break;
  case kAudioUnitParameterUnit_MIDINoteNumber:
    fprintf(asysn_audiounit_logfile, "note number");
    break;
  case kAudioUnitParameterUnit_MIDIController:
    fprintf(asysn_audiounit_logfile, "CC number");
    break;
  case kAudioUnitParameterUnit_Decibels:
    fprintf(asysn_audiounit_logfile, "dB");
    break;
  case kAudioUnitParameterUnit_LinearGain:
    fprintf(asysn_audiounit_logfile, "gain (linear)");
    break;
  case kAudioUnitParameterUnit_Degrees:
    fprintf(asysn_audiounit_logfile, "degrees");
    break;
  case kAudioUnitParameterUnit_EqualPowerCrossfade:
    fprintf(asysn_audiounit_logfile, "(crossfade - equal power)");
    break;
  case kAudioUnitParameterUnit_MixerFaderCurve1:
    fprintf(asysn_audiounit_logfile, "(mixer fader curve1)");
    break;
  case kAudioUnitParameterUnit_Pan:
    fprintf(asysn_audiounit_logfile, "pan");
    break;
  case kAudioUnitParameterUnit_Meters:
    fprintf(asysn_audiounit_logfile, "m");
    break;
  case kAudioUnitParameterUnit_AbsoluteCents:
    fprintf(asysn_audiounit_logfile, "cents (absolute)");
    break;
  case kAudioUnitParameterUnit_CustomUnit:
    fprintf(asysn_audiounit_logfile, "custom");
    break;
  default:
    fprintf(asysn_audiounit_logfile, "(%i)", units);
    break;
  };
}

/**************************************/
/* prints out a MIDI channel command  */
/**************************************/

void asysn_audiounit_print_midievent(unsigned char command, unsigned char d0, 
				     unsigned char c0)

{
  if ((command & 0xF0) == 0x80)  /* note off */
    {
      fprintf(asysn_audiounit_logfile, "\n\tNote Off: ch %i note %i vel %i", 
	      command & 0x0F, d0, c0);
      return;
    }
  if ((command & 0xF0) == 0x90)  /* note on */
    {
      fprintf(asysn_audiounit_logfile, "\n\tNote On: ch %i note %i vel %i", 
	      command & 0x0F, d0, c0);
      return;
    }
  if ((command & 0xF0) == 0xA0)  /* aftertouch */
    {
      fprintf(asysn_audiounit_logfile, "\n\tPoly Aftertouch: ch %i note %i vel %i", 
	      command & 0x0F, d0, c0);
      return;
    }
  if ((command & 0xF0) == 0xB0)  /* control change */
    {
      fprintf(asysn_audiounit_logfile, "\n\tControl change: ch %i num %i val %i", 
	      command & 0x0F, d0, c0);
      return;
    }
  if ((command & 0xF0) == 0xC0)  /* patch change */
    {
      fprintf(asysn_audiounit_logfile, "\n\tPatch Change: ch %i pp %i", 
	      command & 0x0F, d0);
      return;
    }
  if ((command & 0xF0) == 0xD0)  /* channel aftertouch */
    {
      fprintf(asysn_audiounit_logfile, "\n\tChannel Aftertouch: ch %i val %i", 
	      command & 0x0F, d0);
      return;
    }
  if ((command & 0xF0) == 0xE0)  /* pitch wheel */
    {
      fprintf(asysn_audiounit_logfile, "\n\tPitch wheel: ch %i top %i bot %i", 
	      command & 0x0F, c0, d0);
      return;
    }
  fprintf(asysn_audiounit_logfile, "\n\tSystem command: %hhX", command);
  return;
}


/*~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~*/
/* Size-Checking Routines */
/*~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~*/

/*************************************/
/* number of bytes of params[] array */
/*************************************/

int asysn_audiounit_expected_param_size(ComponentParameters *p)

{
  switch(p->what) {

  case kComponentOpenSelect:
    return 4;
    break;
  case kComponentCloseSelect:
    return 4;
    break;
  case kComponentCanDoSelect:
    return 2;
    break;
  case kComponentVersionSelect:
    return 0;
    break;
  case kComponentRegisterSelect:
    return 0;
    break;
  case kComponentTargetSelect:
    return 4;
    break;
  case kComponentUnregisterSelect:
    return 0;
    break;
  case kComponentGetMPWorkFunctionSelect:
  case kComponentExecuteWiredActionSelect:
  case kComponentGetPublicResourceSelect:
    return -1;  /* todo: find correct values for these selectors */
    break;
  case kAudioUnitRange:
    return -1;  /* todo: find correct values for this selector */
    break;
  case kAudioUnitInitializeSelect:
    return 0;
    break;
  case kAudioUnitUninitializeSelect:
    return 0;
    break;
  case kAudioUnitGetPropertyInfoSelect:
    return 20;
    break;
  case kAudioUnitGetPropertySelect:
    return 20;
    break;
  case kAudioUnitSetPropertySelect:
    return 20;
    break;
  case kAudioUnitAddPropertyListenerSelect:
    return 12;
    break;
  case kAudioUnitRemovePropertyListenerSelect:
    return 8;
    break;
  case kAudioUnitRemovePropertyListenerWithUserDataSelect:
    return 12;
    break;
  case kAudioUnitAddRenderNotifySelect:
    return 8;
    break;
  case kAudioUnitRemoveRenderNotifySelect:
    return 8;
    break;
  case kAudioUnitGetParameterSelect:
    return 16;
    break;
  case kAudioUnitSetParameterSelect:
    return 20;
    break;
  case kAudioUnitScheduleParametersSelect:
    return 8;
    break;
  case kAudioUnitRenderSelect:
    return 20;
    break;
  case kAudioUnitResetSelect:
    return 8;
    break;
  case kMusicDeviceMIDIEventSelect:
    return 16;
    break;
  case kMusicDeviceSysExSelect:
    return 8;
    break;
  case kMusicDevicePrepareInstrumentSelect:
    return 4;
    break;
  case kMusicDeviceReleaseInstrumentSelect:
    return 4;
    break;
  case kMusicDeviceStartNoteSelect:
    return 20;
    break;
  case kMusicDeviceStopNoteSelect:
    return 12;
    break;
  default:
    break;
  }

  return -1;  /* return -1 for unrecognized selectors */
}

/*********************************/
/* error-checks paramSize value  */
/*********************************/

void asysn_audiounit_print_paramsize_check(ComponentParameters *p)

{
  int expected = asysn_audiounit_expected_param_size(p);

  if (expected >=0)
    {
      if (p->paramSize != expected)
	fprintf(asysn_audiounit_logfile, 
		"\n\tWARNING: Expected paramSize %i, actual paramsize %i",
		expected, p->paramSize); 
    }
  else
    fprintf(asysn_audiounit_logfile, 
	    "\n\tWARNING: Unknown selector, cannot verify paramSize value "); 
}

/********************************************/
/* number of bytes of get/set property data */
/********************************************/

int asysn_audiounit_expected_property_size(AudioUnitPropertyID id)

{
  switch(id) {
  case kAudioUnitProperty_ClassInfo: /* 0 */
    return sizeof(void *);
  case kAudioUnitProperty_MakeConnection: /* 1 */
    return sizeof(AudioUnitConnection);
  case kAudioUnitProperty_SampleRate: /* 2 */
    return sizeof(Float64);
  case kAudioUnitProperty_ParameterList: /* 3 */ 
    return asysn_audiounit_PropertySizeWildCard;  
  case kAudioUnitProperty_ParameterInfo: /* 4 */ 
    return sizeof(AudioUnitParameterInfo);  
  case kAudioUnitProperty_FastDispatch: /* 5 */ 
    /* sizeof(void *), but we always want printing */
    return asysn_audiounit_PropertySizeWildCard;  
  case kAudioUnitProperty_CPULoad:      /* 6 */
    return sizeof(Float32);
  case kAudioUnitProperty_StreamFormat: /* 8 */ 
    return sizeof(AudioStreamBasicDescription);
  case kAudioUnitProperty_SRCAlgorithm: /* 9 */
    return sizeof(OSType);
  case kAudioUnitProperty_ReverbRoomType: /* 10 */
    return sizeof(UInt32);
  case kAudioUnitProperty_ElementCount: /* 11, also kAudioUnitProperty_BusCount */
    return sizeof(UInt32);
  case kAudioUnitProperty_Latency: /* 12 */
    return sizeof(Float64);
  case kAudioUnitProperty_SupportedNumChannels: /* 13 */
    return asysn_audiounit_PropertySizeWildCard;  
  case kAudioUnitProperty_MaximumFramesPerSlice: /* 14 */
    return sizeof(UInt32);
  case kAudioUnitProperty_SetExternalBuffer: /* 15 */
    return sizeof(AudioUnitExternalBuffer);
  case kAudioUnitProperty_ParameterValueStrings: /* 16 */
    return sizeof(CFArrayRef);
  case kAudioUnitProperty_MIDIControlMapping: /* 17 */ 
    return asysn_audiounit_PropertySizeWildCard;
  case kAudioUnitProperty_GetUIComponentList: /* 18 */
    return asysn_audiounit_PropertySizeWildCard;
  case kAudioUnitProperty_AudioChannelLayout: /* 19 */
    /* AudioChannelLayout, variable length */  
    return asysn_audiounit_PropertySizeWildCard;
  case kAudioUnitProperty_TailTime: /* 20 */
    return sizeof(Float64);
  case kAudioUnitProperty_BypassEffect: /* 21 */
    return sizeof(UInt32);
  case kAudioUnitProperty_LastRenderError: /* 22 */
    return sizeof(OSStatus);
  case kAudioUnitProperty_SetRenderCallback: /* 23 */
    return sizeof(AURenderCallbackStruct);
  case kAudioUnitProperty_FactoryPresets: /* 24 */ 
    return sizeof(CFArrayRef);  
  case kAudioUnitProperty_ContextName: /* 25 */
    return sizeof(CFStringRef);
  case kAudioUnitProperty_RenderQuality: /* 26 */
    return sizeof(UInt32);
  case kAudioUnitProperty_HostCallbacks: /* 27 */
    return sizeof(HostCallbackInfo);
  case kAudioUnitProperty_CurrentPreset: /* 28 */
    return sizeof(AUPreset);
  case kAudioUnitProperty_InPlaceProcessing: /* 29 */
    return sizeof(UInt32);
  case kAudioUnitProperty_ElementName: /* 30 */
    return sizeof(CFStringRef);
  case kAudioUnitProperty_CocoaUI: /* 31 */
    /* AudioUnitCocoaViewInfo, variable length */
    return asysn_audiounit_PropertySizeWildCard;
  case kAudioUnitProperty_SupportedChannelLayoutTags: /* 32 */
    /* AudioChannelLayoutTags[kVariableLengthArray] */
    return asysn_audiounit_PropertySizeWildCard;
  case kAudioUnitProperty_ParameterStringFromValue: /* 33 */
    /* also kAudioUnitProperty_ParameterValueName */
    return sizeof(AudioUnitParameterStringFromValue);
  case kAudioUnitProperty_ParameterIDName: /* 34 */
    return sizeof(AudioUnitParameterIDName);
  case kAudioUnitProperty_ParameterClumpName: /* 35 */
    return sizeof(AudioUnitParameterIDName);
  case kAudioUnitProperty_PresentPreset: /* 36 */
    return sizeof(AUPreset);
  case kAudioUnitProperty_OfflineRender: /* 37 */
    return sizeof(UInt32);
  case kAudioUnitProperty_ParameterValueFromString: /* 38 */
    return sizeof(AudioUnitParameterValueFromString);
  case kAudioUnitProperty_IconLocation: /* 39 */
    return sizeof(CFURLRef);
  case kAudioUnitProperty_PresentationLatency: /* 40 */
    return sizeof(Float64);
  case kAudioUnitProperty_AllParameterMIDIMappings: /* 41 */
    /* array of AUParameterMIDIMapping */
    return asysn_audiounit_PropertySizeWildCard;
  case kAudioUnitProperty_AddParameterMIDIMapping: /* 42 */
    /* array of AUParameterMIDIMapping */
    return asysn_audiounit_PropertySizeWildCard;
  case kAudioUnitProperty_RemoveParameterMIDIMapping: /* 43 */
    /* array of AUParameterMIDIMapping */
    return asysn_audiounit_PropertySizeWildCard;
  case kAudioUnitProperty_HotMapParameterMIDIMapping: /* 44 */
    /* one AUParameterMIDIMapping or NULL */
    return asysn_audiounit_PropertySizeWildCard;
  case kAudioUnitProperty_DependentParameters: /* 45 */
    /* array of AUDependentParameter */
    return asysn_audiounit_PropertySizeWildCard;
  case kAudioUnitProperty_ClassInfoFromDocument: /* 50 */
    return sizeof(void *);
  case kMusicDeviceProperty_InstrumentCount: /* 1000 */
    return sizeof(UInt32);
  case kMusicDeviceProperty_InstrumentName: /* 1001 */
    return sizeof(char *);
  case kMusicDeviceProperty_GroupOutputBus: /* 1002 */
    return sizeof(UInt32);
  case kMusicDeviceProperty_SoundBankFSSpec: /* 1003 */
    return sizeof(FSSpec);
  case kMusicDeviceProperty_InstrumentNumber: /* 1004 */
    return sizeof(MusicDeviceInstrumentID);
  case kMusicDeviceProperty_UsesInternalReverb: /* 1005 */ 
    /* also kAudioUnitProperty_UsesInternalReverb */
    return sizeof(UInt32);
  case kMusicDeviceProperty_MIDIXMLNames: /* 1006 */
    return sizeof(CFURLRef);
  case kMusicDeviceProperty_BankName: /* 1007 */
    return sizeof(CFStringRef);
  case kMusicDeviceProperty_SoundBankData: /* 1008 */
    return asysn_audiounit_PropertySizeWildCard;
  case kMusicDeviceProperty_PartGroup: /* 1010 */
    return sizeof(AudioUnitElement);
  case kMusicDeviceProperty_StreamFromDisk: /* 1011 */
    return sizeof(UInt32);
  case kMusicDeviceProperty_SoundBankFSRef: /* 1012 */
    return sizeof(FSRef);
  case kAudioOutputUnitProperty_CurrentDevice: /* 2000 */
    return sizeof(AudioDeviceID);
  case kAudioOutputUnitProperty_IsRunning: /* 2001 */
    return sizeof(UInt32);
  case kAudioOutputUnitProperty_ChannelMap: /* 2002 */
    /* variable-length array of SInt32 */ 
    return asysn_audiounit_PropertySizeWildCard;
  case kAudioOutputUnitProperty_EnableIO: /* 2003 */
    return sizeof(UInt32);
  case kAudioOutputUnitProperty_StartTime: /* 2004 */
    return sizeof(AudioOutputUnitStartAtTimeParams);
  case kAudioOutputUnitProperty_SetInputCallback: /* 2005 */
    return sizeof(AURenderCallbackStruct);
  case kAudioOutputUnitProperty_HasIO: /* 2006 */
    return sizeof(UInt32);
  case kAudioOutputUnitProperty_StartTimestampsAtZero: /* 2007 */ 
    return sizeof(UInt32);
  case kAudioUnitProperty_SpatializationAlgorithm: /* 3000 */
    return sizeof(UInt32);
  case kAudioUnitProperty_SpeakerConfiguration: /* 3001 */
    return sizeof(UInt32);
  case kAudioUnitProperty_DopplerShift: /* 3002 */
    return sizeof(UInt32);
  case kAudioUnitProperty_3DMixerRenderingFlags: /* 3003 */
    return asysn_audiounit_PropertySizeUnknown;
  case kAudioUnitProperty_3DMixerDistanceAtten: /* 3004 */
    return asysn_audiounit_PropertySizeUnknown;
  case kAudioUnitProperty_MatrixLevels: /* 3006 */
    return asysn_audiounit_PropertySizeUnknown;
  case kAudioUnitProperty_MeteringMode: /* 3007 */
    return asysn_audiounit_PropertySizeUnknown;
#if (0)  /* depreciated */
  case kAudioUnitProperty_PannerMode: /* 3008 */
    return sizeof(UInt32);
#endif
  case kAudioUnitProperty_MatrixDimensions: /* 3009 */
    return asysn_audiounit_PropertySizeUnknown;
  case kAudioUnitProperty_3DMixerDistanceParams: /* 3010 */
    return asysn_audiounit_PropertySizeUnknown;
  case kAudioUnitProperty_MeterClipping: /* 3011 */ 
    return asysn_audiounit_PropertySizeUnknown;
  case kAudioOfflineUnitProperty_InputSize: /* 3020 */
    return sizeof(UInt64);
  case kAudioOfflineUnitProperty_OutputSize: /* 3021 */
    return sizeof(UInt64);
  case kAudioUnitOfflineProperty_StartOffset: /* 3022 */
    return sizeof(UInt64);
  case kAudioUnitOfflineProperty_PreflightRequirements: /* 3023 */
    return sizeof(UInt32);
  case kAudioUnitOfflineProperty_PreflightName: /* 3024 */
    return sizeof(CFStringRef);
  case kAudioUnitProperty_ScheduleAudioSlice: /* 3300 */	
    return asysn_audiounit_PropertySizeUnknown;
  case kAudioUnitProperty_ScheduleStartTimeStamp: /* 3301 */ 
    return asysn_audiounit_PropertySizeUnknown;
  case kAudioUnitProperty_CurrentPlayTime: /* 3302 */ 
    return asysn_audiounit_PropertySizeUnknown;
  case kAudioUnitProperty_ScheduledFileIDs: /* 3310 */	
    return asysn_audiounit_PropertySizeUnknown;
  case kAudioUnitProperty_ScheduledFileRegion: /* 3311 */  
    return asysn_audiounit_PropertySizeUnknown;
  case kAudioUnitProperty_ScheduledFilePrime: /* 3312 */   
    return asysn_audiounit_PropertySizeUnknown;
  case kAudioUnitProperty_ScheduledFileBufferSizeFrames: /* 3313 */
    return asysn_audiounit_PropertySizeUnknown;
  case kAudioUnitProperty_ScheduledFileNumberBuffers: /* 3314 */	
    return asysn_audiounit_PropertySizeUnknown;
  case kAudioUnitProperty_DeferredRendererPullSize: /* 3320 */	
    return asysn_audiounit_PropertySizeUnknown;
  case kAudioUnitProperty_DeferredRendererExtraLatency: /* 3321 */  
    return asysn_audiounit_PropertySizeUnknown;
  case kAudioUnitProperty_DeferredRendererWaitFrames: /* 3322 */	
    return asysn_audiounit_PropertySizeUnknown;
  case kAUNetReceiveProperty_Hostname: /* 3511 */
    return asysn_audiounit_PropertySizeUnknown;
  case kAUNetReceiveProperty_Password: /* 3512 */
    return asysn_audiounit_PropertySizeUnknown;
  case kAUNetSendProperty_PortNum: /* 3513 */
    return asysn_audiounit_PropertySizeUnknown;
  case kAUNetSendProperty_TransmissionFormat: /* 3514 */
    return asysn_audiounit_PropertySizeUnknown;
  case kAUNetSendProperty_TransmissionFormatIndex: /* 3515 */
    return asysn_audiounit_PropertySizeUnknown;
  case kAUNetSendProperty_ServiceName: /* 3516 */
    return asysn_audiounit_PropertySizeUnknown;
  case kAUNetSendProperty_Disconnect: /* 3517 */
    return asysn_audiounit_PropertySizeUnknown;
  case kAUNetSendProperty_Password: /* 3518 */
    return asysn_audiounit_PropertySizeUnknown;
  case kAudioUnitMigrateProperty_FromPlugin: /* 4000 */
    return sizeof(CFArrayRef);
  case kAudioUnitMigrateProperty_OldAutomation: /* 4001 */
    return sizeof(AudioUnitParameterValueTranslation);
  default:
    return asysn_audiounit_PropertySizeUnknown;  
  }
} 

/**********************************************************************/
/* error-checks property size value: return status (0 == size match)  */
/**********************************************************************/

int asysn_audiounit_print_propertysize_check(AudioUnitPropertyID id, UInt32 datasize)

{
  int expected = asysn_audiounit_expected_property_size(id);

  if (expected == asysn_audiounit_PropertySizeUnknown) 
    return 1;          

  if ((expected != asysn_audiounit_PropertySizeWildCard) && (datasize != expected))
    {
      fprintf(asysn_audiounit_logfile, 
	      "\n\tWARNING: Expected property data size %i, actual size %i",
	      expected, (unsigned int) datasize);
      return 1;
    }
  return 0;
}


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/* Property-Specific Print Routines */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/************************************************/
/*    Helper for kAudioUnitProperty_ClassInfo   */
/*      Reads integer item from a ClassInfo     */
/************************************************/

int asysn_audiounit_wiretap_classinfo_readint(CFMutableDictionaryRef ClassInfo, 
				      char * ckey, SInt32 * value)

{
  CFStringRef key;
  CFNumberRef num;
  SInt32 newval;
  int errcode = -1;

  key = CFStringCreateWithCString(NULL, ckey, kCFStringEncodingASCII);
  if (key)
    {
      num = CFDictionaryGetValue(ClassInfo, key);
      if (num && CFNumberGetValue(num, kCFNumberSInt32Type, &newval))
	{
	  errcode = 0;
	  *value = newval;
	}
      CFRelease(key);
    }
  return errcode;
}

/************************************************/
/*    Helper for kAudioUnitProperty_ClassInfo   */
/*      Reads float item from a ClassInfo       */
/************************************************/

int asysn_audiounit_wiretap_classinfo_readfloat(CFMutableDictionaryRef ClassInfo, 
					char * ckey, Float32 * value)

{
  CFStringRef key;
  CFNumberRef num;
  Float32 newval;
  int errcode = -1;

  key = CFStringCreateWithCString(NULL, ckey, kCFStringEncodingASCII);
  if (key)
    {
      num = CFDictionaryGetValue(ClassInfo, key);
      if (num && CFNumberGetValue(num, kCFNumberFloat32Type, &newval))
	{
	  errcode = 0;
	  *value = newval;
	}
      CFRelease(key);
    }
  return errcode;
}

/************************************************/
/*    Helper for kAudioUnitProperty_ClassInfo   */
/*      Reads CFString item from a ClassInfo    */
/************************************************/

int asysn_audiounit_wiretap_classinfo_readcfstr(CFMutableDictionaryRef ClassInfo, 
					char * ckey, CFStringRef * cfstr)

{
  CFStringRef key;
  CFNumberRef num;
  CFStringRef newstr;
  int errcode = -1;

  key = CFStringCreateWithCString(NULL, ckey, kCFStringEncodingASCII);
  if (key)
    {
      if (newstr = CFDictionaryGetValue(ClassInfo, key))
	{
	  errcode = 0;
	  CFRetain(newstr);
	  if (*cfstr)
	    CFRelease(*cfstr);
	  *cfstr = newstr;
	}
      CFRelease(key);
    }
  return errcode;
}

/************************************************/
/*    Helper for kAudioUnitProperty_ClassInfo   */
/*  Big-endian int read from a CFMutableDataRef */
/************************************************/

SInt32 asysn_audiounit_wiretap_classinfo_pdata_intread(CFMutableDataRef pdata, UInt8 ** p)

{
  SInt32 value;
  
  value = ((*p)[0] << 24) | ((*p)[1] << 16) | ((*p)[2] << 8) | ((*p)[3]);
  (*p) += 4;
  return value;
}

/**************************************************/
/*     Helper for kAudioUnitProperty_ClassInfo    */
/*  Big-endian float read from a CFMutableDataRef */
/**************************************************/

Float32 asysn_audiounit_wiretap_classinfo_pdata_floatread(CFMutableDataRef pdata, UInt8 ** p)

{
  union { SInt32 i; Float32 f ; } u;

  u.i = ((*p)[0] << 24) | ((*p)[1] << 16) | ((*p)[2] << 8) | ((*p)[3]);
  (*p) += 4;
  return u.f;
}

/**************************************/
/*  kAudioUnitProperty_ClassInfo      */
/*                                    */
/* *data is a CFPropertyListRef       */
/*                                    */
/**************************************/

void asysn_audiounit_print_property_ClassInfo(CFMutableDictionaryRef * data)

{
  CFStringRef cfstr;
  char * nptr, * cstr = NULL;
  CFStringRef key;
  SInt32 value;
  Float32 fval;
  UInt32 scope, bus, count, idx;
  CFMutableDataRef pdata;
  UInt8 * p, * pmax;
  float * pval = NULL;
  int i, pval_size = 0;
  char c[4];

  if (*data == NULL)
    {
      fprintf(asysn_audiounit_logfile, "\n\tA null preset"); 
      return;
    }

  if (CFGetTypeID(*data) != CFDictionaryGetTypeID())
    {
      fprintf(asysn_audiounit_logfile, "\n\tA preset dictionary with a bad type"); 
      return;
    }

  if (asysn_audiounit_wiretap_classinfo_readint(*data, kAUPresetVersionKey, &value))
    {
      fprintf(asysn_audiounit_logfile, 
	      "\n\tA preset dictionary without a valid version field"); 
      return;
    }
  else
    if (value)
      {
	fprintf(asysn_audiounit_logfile, 
		"\n\tA preset dictionary with a bad version number"); 
	return;
      }

  fprintf(asysn_audiounit_logfile, "\n\tA preset for "); 

  if (!asysn_audiounit_wiretap_classinfo_readint(*data, kAUPresetTypeKey, (SInt32 *) c))
    {
      if (ASYS_AUDIOUNIT_SINT32_BIGENDIAN)
	fprintf(asysn_audiounit_logfile, "%c%c%c%c ", c[0], c[1], c[2], c[3]); 
      else
	fprintf(asysn_audiounit_logfile, "%c%c%c%c ", c[3], c[2], c[1], c[0]); 
    }

  if (!asysn_audiounit_wiretap_classinfo_readint(*data, kAUPresetSubtypeKey, (SInt32 *) c))
    {
      if (ASYS_AUDIOUNIT_SINT32_BIGENDIAN)
	fprintf(asysn_audiounit_logfile, "%c%c%c%c ", c[0], c[1], c[2], c[3]); 
      else
	fprintf(asysn_audiounit_logfile, "%c%c%c%c ", c[3], c[2], c[1], c[0]); 
    }

  if (!asysn_audiounit_wiretap_classinfo_readint(*data, kAUPresetManufacturerKey, (SInt32 *) c))
    {
      if (ASYS_AUDIOUNIT_SINT32_BIGENDIAN)
	fprintf(asysn_audiounit_logfile, "%c%c%c%c ", c[0], c[1], c[2], c[3]); 
      else
	fprintf(asysn_audiounit_logfile, "%c%c%c%c ", c[3], c[2], c[1], c[0]); 
    }

  key = CFStringCreateWithCString(NULL, kAUPresetDataKey, kCFStringEncodingASCII);

  if (key && (pdata = (CFMutableDataRef) CFDictionaryGetValue(*data, key)))
    {
      p = (UInt8 *) CFDataGetBytePtr(pdata);
      pmax = p + CFDataGetLength(pdata);

      do {
	if (p == pmax)
	  break;
	if ((pmax - p) >= 12)
	  {
	    scope = asysn_audiounit_wiretap_classinfo_pdata_intread(pdata, &p);
	    bus = asysn_audiounit_wiretap_classinfo_pdata_intread(pdata, &p);
	    count = asysn_audiounit_wiretap_classinfo_pdata_intread(pdata, &p);
	    fprintf(asysn_audiounit_logfile, "\n\t%u parameters for scope %u, "
		    "element %u:", (unsigned int) count, (unsigned int) scope, (unsigned int) bus);
	    if ((pmax - p) >= count*(sizeof(SInt32) + sizeof(Float32)))
	      {
		pval = calloc(count, sizeof(float));
		pval_size = count;
		while (count--)
		  {
		    idx = asysn_audiounit_wiretap_classinfo_pdata_intread(pdata, &p);
		    fval = asysn_audiounit_wiretap_classinfo_pdata_floatread(pdata, &p);
		    fprintf(asysn_audiounit_logfile, "\n\tP[%u] = %g", (unsigned int) idx, fval);
		    if ((idx >= 0) && (idx < pval_size))
		      pval[idx] = fval;
		  }
	      }
	    else
	      {
		fprintf(asysn_audiounit_logfile, "\n\tCorrupt data structure");
		break;
	      }
	  }
	else
	  {
	    fprintf(asysn_audiounit_logfile, "\n\tCorrupt data structure");
	    break;
	  }
      } while (1);
      
      CFRelease(key);
    }

  cfstr = NULL;

  if (!asysn_audiounit_wiretap_classinfo_readcfstr(*data, kAUPresetNameKey, &cfstr))
    {
      cstr = calloc(CFStringGetLength(cfstr) + 1, 1);
      CFStringGetCString(cfstr, cstr, CFStringGetLength(cfstr) + 1, 
			 kCFStringEncodingASCII);
      fprintf(asysn_audiounit_logfile, "\n\tPreset Name: %s", cstr);
      CFRelease(cfstr);    /* keep cstr to print out factory table */
    }

  if (!asysn_audiounit_wiretap_classinfo_readint(*data, kAUPresetRenderQualityKey, 
					 (SInt32 *) &value))
    {
      fprintf(asysn_audiounit_logfile, "\n\tRender Quality: "); 
      switch (value) {
      case 127:
	fprintf(asysn_audiounit_logfile, "Maximum (127)");
	break;
      case 96:
	fprintf(asysn_audiounit_logfile, "High (96)");
	break;
      case 64:
	fprintf(asysn_audiounit_logfile, "Medium (64)");
	break;
      case 32:
	fprintf(asysn_audiounit_logfile, "Low (32)");
	break;
      case 0:
	fprintf(asysn_audiounit_logfile, "Minimum (0)");
	break;
      default:
	fprintf(asysn_audiounit_logfile, "%i ", (int) value);
      }
    }

  if (!asysn_audiounit_wiretap_classinfo_readfloat(*data, kAUPresetCPULoadKey, &fval))
      fprintf(asysn_audiounit_logfile, "\n\tCPU Load: %g", fval);

  if (cstr)
    {
      if (pval)
	{
	  nptr = cstr;
	  do {
	    if (*nptr == ' ')
	      *nptr = '_';
	  } while (*(++nptr));
	  fprintf(asysn_audiounit_logfile, 
		  "\n\ttable aup_factory_%s(data, %i", cstr, pval_size);
	  for (i = 0; i < pval_size; i++)
	    fprintf(asysn_audiounit_logfile, ", %g", pval[i]);
	  fprintf(asysn_audiounit_logfile, ");\n\t");
	  free(pval);
	}
      free(cstr);
    }

  /* add kAUPresetElementNameKey here if named elements/buses are supported */

  return;
}

/**************************************/
/*  kAudioUnitProperty_ParameterList  */
/*                                    */
/* An array of                        */
/* UInt32 AudioUnitParameterID        */
/*                                    */
/**************************************/

void asysn_audiounit_print_property_ParameterList(AudioUnitParameterID * data,
					    UInt32 datasize)

{
  unsigned int i;

  if (datasize)
    for (i = 0; i < datasize/sizeof(UInt32); i++)
      fprintf(asysn_audiounit_logfile, "\n\tParameterID %u", 
	      (unsigned int) data[i]);
  else
    fprintf(asysn_audiounit_logfile, "\n\tEmpty parameter list");
}

/*****************************************/
/*  kAudioUnitProperty_ParameterInfo     */
/*                                       */
/* element specifies the parameter       */
/* *data holds AudioUnitParameterInfo    */
/*                                       */
/* char                    name[60];     */
/* CFStringRef             cfNameString; */
/* AudioUnitParameterUnit  unit;         */
/* Float32                 minValue;     */
/* Float32                 maxValue;     */
/* Float32                 defaultValue; */
/* UInt32                  flags;        */
/*                                       */
/*****************************************/

void asysn_audiounit_print_property_ParameterInfo(AudioUnitParameterInfo * data,
						  UInt32 element)

{
  CFIndex slen;
  char * cstr;

  fprintf(asysn_audiounit_logfile, "\n\tParameter info for element %u", 
	  (unsigned int) element);
  fprintf(asysn_audiounit_logfile, "\n\tName: %s", data->name);
  fprintf(asysn_audiounit_logfile, "\n\tUnits: ");
  asysn_audiounit_print_parameter_units(data->unit);

  if (data->unit == kAudioUnitParameterUnit_CustomUnit)
    {
      if (data->unitName)
	{
	  slen = CFStringGetLength(data->unitName) + 1;
	  cstr = calloc(slen, 1);
	  CFStringGetCString(data->unitName, cstr, slen, kCFStringEncodingASCII);
	  fprintf(asysn_audiounit_logfile, "\n\tCustom Unit: %s", cstr);
	  free(cstr);
	}
      else
	fprintf(asysn_audiounit_logfile, "\n\tCustom Unit: <null>");
    }

  fprintf(asysn_audiounit_logfile, "\n\tMinimum value: %g", data->minValue);
  fprintf(asysn_audiounit_logfile, "\n\tDefault value: %g", data->defaultValue);
  fprintf(asysn_audiounit_logfile, "\n\tMaximum value: %g", data->maxValue);

  if (data->flags)
    {
      if (data->flags & kAudioUnitParameterFlag_Global)
	fprintf(asysn_audiounit_logfile, "\n\tFlag: global");
      if (data->flags & kAudioUnitParameterFlag_Input)
	fprintf(asysn_audiounit_logfile, "\n\tFlag: input");
      if (data->flags & kAudioUnitParameterFlag_Output)
	fprintf(asysn_audiounit_logfile, "\n\tFlag: output");
      if (data->flags & kAudioUnitParameterFlag_Group) 
	fprintf(asysn_audiounit_logfile, "\n\tFlag: group");
      if (data->flags & kAudioUnitParameterFlag_CFNameRelease)
	fprintf(asysn_audiounit_logfile, "\n\tFlag: CF-name-release");
      if (data->flags & kAudioUnitParameterFlag_MeterReadOnly)
	fprintf(asysn_audiounit_logfile, "\n\tFlag: meter-read-only");
      if (data->flags & kAudioUnitParameterFlag_DisplaySquareRoot)	
	fprintf(asysn_audiounit_logfile, "\n\tFlag: display-square-root");
      if (data->flags & kAudioUnitParameterFlag_DisplaySquared)	
	fprintf(asysn_audiounit_logfile, "\n\tFlag: display-squared");
      if (data->flags & kAudioUnitParameterFlag_DisplayCubed)
	fprintf(asysn_audiounit_logfile, "\n\tFlag: display-cubed");
      if (data->flags & kAudioUnitParameterFlag_DisplayCubeRoot)	
	fprintf(asysn_audiounit_logfile, "\n\tFlag: display-cube-root");
      if (data->flags & kAudioUnitParameterFlag_DisplayExponential)
	fprintf(asysn_audiounit_logfile, "\n\tFlag: display-exponential");
      if (data->flags & kAudioUnitParameterFlag_HasClump)
	fprintf(asysn_audiounit_logfile, "\n\tFlag: has-clump");
      if (data->flags & kAudioUnitParameterFlag_HasName)	
	fprintf(asysn_audiounit_logfile, "\n\tFlag: has-name");
      if (data->flags & kAudioUnitParameterFlag_ValuesHaveStrings)
	fprintf(asysn_audiounit_logfile, "\n\tFlag: values-have-strings");
      if (data->flags & kAudioUnitParameterFlag_DisplayLogarithmic)
	fprintf(asysn_audiounit_logfile, "\n\tFlag: display-logarithmic");
      if (data->flags & kAudioUnitParameterFlag_IsHighResolution)
	fprintf(asysn_audiounit_logfile, "\n\tFlag: high-resolution");
      if (data->flags & kAudioUnitParameterFlag_NonRealTime)	
	fprintf(asysn_audiounit_logfile, "\n\tFlag: non-real-time");
      if (data->flags & kAudioUnitParameterFlag_CanRamp)	
	fprintf(asysn_audiounit_logfile, "\n\tFlag: can-ramp");
      if (data->flags & kAudioUnitParameterFlag_ExpertMode) 		
	fprintf(asysn_audiounit_logfile, "\n\tFlag: expert-mode");
      if (data->flags & kAudioUnitParameterFlag_HasCFNameString) 	
	fprintf(asysn_audiounit_logfile, "\n\tFlag: has-CF-name-string");
      if (data->flags & kAudioUnitParameterFlag_IsGlobalMeta)	
	fprintf(asysn_audiounit_logfile, "\n\tFlag: global-meta");
      if (data->flags & kAudioUnitParameterFlag_IsElementMeta)		
	fprintf(asysn_audiounit_logfile, "\n\tFlag: element-meta");
      if (data->flags & kAudioUnitParameterFlag_IsReadable)
	fprintf(asysn_audiounit_logfile, "\n\tFlag: readable");
      if (data->flags & kAudioUnitParameterFlag_IsWritable)
	fprintf(asysn_audiounit_logfile, "\n\tFlag: writable");
    }
}


/************************************/
/*  kAudioUnitProperty_FastDispatch */
/*                                  */
/* *data points to the function     */
/* that handles selector "element"  */
/*                                  */
/************************************/

void asysn_audiounit_print_property_FastDispatch(int element, int * data)

{
  fprintf(asysn_audiounit_logfile, "\n\tDispatch route for selector ");
  asysn_audiounit_print_selector_name(element);
  if ((*data) == 0)
    fprintf(asysn_audiounit_logfile, "\n\tReturns a NULL function pointer ");
}

/************************************/
/*  kAudioUnitProperty_StreamFormat */
/*                                  */
/*  *data points to an              */
/*  AudioStreamBasicDescription:    */ 
/*                                  */
/*  Float64 mSampleRate;            */
/*  UInt32  mFormatID;              */
/*  UInt32  mFormatFlags;           */
/*  UInt32  mBytesPerPacket;        */
/*  UInt32  mFramesPerPacket;       */
/*  UInt32  mBytesPerFrame;         */
/*  UInt32  mChannelsPerFrame;      */
/*  UInt32  mBitsPerChannel;        */
/*  UInt32  mReserved;              */
/*                                  */
/* (line 117, CoreAudioTypes.h)     */
/*                                  */
/************************************/

void asysn_audiounit_print_property_StreamFormat(AudioStreamBasicDescription * data)

{
  if (data->mSampleRate == kAudioStreamAnyRate)
    fprintf(asysn_audiounit_logfile, "\n\tmSampleRate: kAudioStreamAnyRate"); 
  else
    fprintf(asysn_audiounit_logfile, "\n\tmSampleRate: %lg Hz", data->mSampleRate);

  if (data->mFormatID == kAudioFormatLinearPCM)
    fprintf(asysn_audiounit_logfile, "\n\tmFormatID: kAudioFormatLinearPCM"); 
  else
    fprintf(asysn_audiounit_logfile, "\n\tmFormatID: Compressed Format %i", 
	    (int) data->mFormatID);

  fprintf(asysn_audiounit_logfile, "\n\tmFormatFlags: ");

  if (data->mFormatFlags & kAudioFormatFlagIsFloat)
    fprintf(asysn_audiounit_logfile, "Float ");

  if (data->mFormatFlags & kAudioFormatFlagIsBigEndian)
    fprintf(asysn_audiounit_logfile, "BigEndian ");

  if (data->mFormatFlags & kAudioFormatFlagIsSignedInteger)
    fprintf(asysn_audiounit_logfile, "SignedInteger ");

  if (data->mFormatFlags & kAudioFormatFlagIsPacked)
    fprintf(asysn_audiounit_logfile, "Packed ");

  if (data->mFormatFlags & kAudioFormatFlagIsAlignedHigh)
    fprintf(asysn_audiounit_logfile, "AlignedHigh ");

  if (data->mFormatFlags & kAudioFormatFlagIsNonInterleaved)
    fprintf(asysn_audiounit_logfile, "NonInterleaved ");

  if (data->mFormatFlags & kAudioFormatFlagIsNonMixable)
    fprintf(asysn_audiounit_logfile, "NonMixable ");

  fprintf(asysn_audiounit_logfile, "\n\tmBytesPerPacket: %u", 
	  (unsigned int) data->mBytesPerPacket); 
  fprintf(asysn_audiounit_logfile, "\n\tmFramesPerPacket: %u", 
	  (unsigned int) data->mFramesPerPacket);
  fprintf(asysn_audiounit_logfile, "\n\tmBytesPerFrame: %u", 
	  (unsigned int) data->mBytesPerFrame);
  fprintf(asysn_audiounit_logfile, "\n\tmChannelsPerFrame: %u", 
	  (unsigned int) data->mChannelsPerFrame);
  fprintf(asysn_audiounit_logfile, "\n\tmBitsPerChannel: %u", 
	  (unsigned int) data->mBitsPerChannel);
  fprintf(asysn_audiounit_logfile, "\n\tmReserved: %u", 
	  (unsigned int) data->mReserved);
}


/********************************************/
/*  kAudioUnitProperty_SupportedNumChannels */
/*                                          */
/* An array of                              */
/* UInt32 AUChannelInfo                     */
/*                                          */
/********************************************/

void asysn_audiounit_print_property_SupportedNumChannels(AUChannelInfo * data,
					    UInt32 datasize)

{
  unsigned int i;

  if (datasize)
    for (i = 0; i < datasize/sizeof(AUChannelInfo); i++)
      fprintf(asysn_audiounit_logfile, "\n\tFormat %i: in = %i/out = %i", 
	      i, data[i].inChannels, data[i].outChannels);
  else
    fprintf(asysn_audiounit_logfile, "\n\tEmpty supported format list");
}

/*********************************************/
/*  kAudioUnitProperty_ParameterValueStrings */
/*                                           */
/* *data points to the CFArrayRef holding    */
/* value strings for parameter "element"     */
/*                                           */
/*********************************************/

void asysn_audiounit_print_property_ParameterValueStrings(CFArrayRef * data)
{
  CFArrayRef array;
  CFStringRef cfstr;
  CFIndex asize, slen, i;
  char * cstr;
  
  if ((array = *((CFArrayRef *) data)) && (asize = CFArrayGetCount(array)))
    {
      fprintf(asysn_audiounit_logfile, "\n\tParameter strings:");
      for (i = 0; i < asize; i++)
	if (cfstr = CFArrayGetValueAtIndex(array, i))
	  {
	    slen = CFStringGetLength(cfstr) + 1;
	    cstr = calloc(slen, 1);
	    CFStringGetCString(cfstr, cstr, slen, kCFStringEncodingASCII);
	    fprintf(asysn_audiounit_logfile, "\n\t%i: %s", (int) i, cstr);
	    CFRelease(cfstr);
	    free(cstr);
	  }
	else
	  fprintf(asysn_audiounit_logfile, "\n\t%i: <non-existent string>", 
		  (int) i);
    }
  else
    fprintf(asysn_audiounit_logfile, "\n\tNull CFArrayRef or empty CFArray");
}


/**************************************/
/*  kAudioUnitProperty_FactoryPresets */
/*                                    */
/* *data is a CFArrayRef              */
/* array elements are AUPresets       */
/*                                    */
/* struct AUPreset {                  */
/*  SInt32       presetNumber;        */
/*  CFStringRef  presetName;          */
/* };                                 */
/*                                    */
/**************************************/

void asysn_audiounit_print_property_FactoryPresets(CFArrayRef * data)

{
  CFArrayRef array;
  AUPreset * preset;
  CFIndex asize, slen, i;
  char * cstr;

  if ((array = *((CFArrayRef *) data)) && (asize = CFArrayGetCount(array)))
    {
      fprintf(asysn_audiounit_logfile, "\n\tFactory presets:");
      for (i = 0; i < asize; i++)
	if (preset = (AUPreset *) CFArrayGetValueAtIndex(array, i))
	  {
	    slen = CFStringGetLength(preset->presetName) + 1;
	    cstr = calloc(slen, 1);
	    CFStringGetCString(preset->presetName, cstr, slen, kCFStringEncodingASCII);
	    fprintf(asysn_audiounit_logfile, "\n\t%i: %s", 
		    (int) preset->presetNumber, cstr);
	    free(cstr);
	  }
	else
	  fprintf(asysn_audiounit_logfile, "\n\t%i: <non-existent preset>", 
		  (int) i);
    }
  else
    fprintf(asysn_audiounit_logfile, "\n\tNull CFArrayRef or empty CFArray");

}


/*******************************************/
/*  kAudioUnitProperty_CocoaUI             */
/*                                         */
/* An AudioUnitCocoaViewInfo               */
/*                                         */
/* typedef struct AudioUnitCocoaViewInfo { */
/* CFURLRef  mCocoaAUViewBundleLocation;   */
/* CFStringRef mCocoaAUViewClass[1];       */
/* };                                      */
/*                                         */
/*******************************************/

void asysn_audiounit_print_property_CocoaViewInfo(AudioUnitCocoaViewInfo * data,
						  UInt32 datasize)
{
  unsigned char url_cstring[4096];
  CFStringRef cfstr;
  char * cstr;
  int i;

  if ((data->mCocoaAUViewBundleLocation) &&
      (CFURLGetFileSystemRepresentation(data->mCocoaAUViewBundleLocation, 
					true, &(url_cstring[0]), 4096)))
    fprintf(asysn_audiounit_logfile, "URL: %s", url_cstring);
  else
    if (data->mCocoaAUViewBundleLocation)
      fprintf(asysn_audiounit_logfile, "URL is > 4095 chars.");
    else
      fprintf(asysn_audiounit_logfile, "URL is null.");

  for (i = 0; i < (datasize - sizeof(CFURLRef))/sizeof(CFStringRef); i++)
    if ((cfstr = data->mCocoaAUViewClass[i]))
      {
	cstr = calloc(CFStringGetLength(cfstr) + 1, 1);
	CFStringGetCString(cfstr, cstr, CFStringGetLength(cfstr) + 1, 
			   kCFStringEncodingASCII);
	fprintf(asysn_audiounit_logfile, "\n\tClass (%i): %s", i, cstr);
	free(cstr);
      }
    else
      fprintf(asysn_audiounit_logfile, "\n\tClass (%i) is a null string.", i);
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/* The Main Property Print Routine */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/****************************/
/* prints out property data */
/****************************/

void asysn_audiounit_print_property_data(AudioUnitPropertyID id, AudioUnitScope scope,
					 AudioUnitElement element, UInt32 datasize,
					 void * data)
{
  char name[256];      
  int namesize = 255;   /* paranoia: 256 probably would work OK ... */

  fprintf(asysn_audiounit_logfile, "\n\t");
  asysn_audiounit_print_scope_name(scope);

  switch(id) {                           /* Print element/bus info */
  default:
    if (element)                         /* skip default to reduce clutter */ 
      fprintf(asysn_audiounit_logfile, "\n\tElement %u", (unsigned int) element);
    break;
  case kAudioUnitProperty_FastDispatch:   /* skip element print for these properties */
  case kAudioUnitProperty_ParameterInfo: 
  case kAudioUnitProperty_MakeConnection: 
  case kMusicDeviceProperty_InstrumentName:
    break;
  }

  if (asysn_audiounit_print_propertysize_check(id, datasize))  
    return;               /* can we print *data ? if not return early */

  fprintf(asysn_audiounit_logfile, "\n\t");

  switch(id) {  /* print property data structures */
  case kAudioUnitProperty_ClassInfo: /* 0 */
    asysn_audiounit_print_property_ClassInfo((CFMutableDictionaryRef *) data);
    break;
  case kAudioUnitProperty_MakeConnection: /* 1 */
    fprintf(asysn_audiounit_logfile, 
	    "\n\tConnection from output %u of AU %u to input bus %u%s", 
	    (unsigned int) ((AudioUnitConnection *) data)->sourceOutputNumber,
	    (unsigned int) ((AudioUnitConnection *) data)->sourceAudioUnit,
	    (unsigned int) ((AudioUnitConnection *) data)->destInputNumber,
	    (((AudioUnitConnection *) data)->destInputNumber != element) ? 
	    " (mismatch with element)" : "");
    break;
  case kAudioUnitProperty_SampleRate: /* 2 */
    fprintf(asysn_audiounit_logfile, "\n\tA Sample rate is %lf Hz", 
	    *((Float64 *) data));
    break;
  case kAudioUnitProperty_ParameterList: /* 3 */
    asysn_audiounit_print_property_ParameterList((AudioUnitParameterID *) data,
					   datasize);
    break;
  case kAudioUnitProperty_ParameterInfo: /* 4 */ 
    asysn_audiounit_print_property_ParameterInfo((AudioUnitParameterInfo *) data,
					   element);
    break;
  case kAudioUnitProperty_FastDispatch: /* 5 */
    asysn_audiounit_print_property_FastDispatch(element, (int *) data);
    break;
  case kAudioUnitProperty_CPULoad: /* 6 */ 
    fprintf(asysn_audiounit_logfile, "\n\tCPULoad value is %g", *((Float32 *) data));
    break;
  case kAudioUnitProperty_StreamFormat: /* 8 */ 
    asysn_audiounit_print_property_StreamFormat((AudioStreamBasicDescription *) data);
    break;
  case kAudioUnitProperty_SRCAlgorithm: /* 9 */
    fprintf(asysn_audiounit_logfile, "\n\tSRC algorithm is '%c%c%c%c'", 
	    ((char *) data)[0], ((char *) data)[1], 
	    ((char *) data)[2], ((char *) data)[3]);
    break;
  case kAudioUnitProperty_ReverbRoomType: /* 10 */
    switch (*((UInt32 *)data)) {
    case kReverbRoomType_SmallRoom:
      fprintf(asysn_audiounit_logfile, "\n\tSmallRoom");
      break;
    case kReverbRoomType_MediumRoom:
      fprintf(asysn_audiounit_logfile, "\n\tMediumRoom");
      break;
    case kReverbRoomType_LargeRoom:
      fprintf(asysn_audiounit_logfile, "\n\tLargeRoom");
      break;
    case kReverbRoomType_MediumHall:
      fprintf(asysn_audiounit_logfile, "\n\tMediumHall");
      break;
    case kReverbRoomType_LargeHall:
      fprintf(asysn_audiounit_logfile, "\n\tLargeHall");
      break;
    case kReverbRoomType_Plate:
      fprintf(asysn_audiounit_logfile, "\n\tPlate");
      break;
    }
    break;
  case kAudioUnitProperty_ElementCount: /* 11, also kAudioUnitProperty_BusCount */
    fprintf(asysn_audiounit_logfile, "\n\tBusCount/ElementCount value is %u", 
	    (unsigned int) *((UInt32 *) data));
    break;
  case kAudioUnitProperty_Latency: /* 12 */
    fprintf(asysn_audiounit_logfile, "\n\tLatency value is %lfs", *((Float64 *) data));
    break;
  case kAudioUnitProperty_SupportedNumChannels: /* 13 */
    asysn_audiounit_print_property_SupportedNumChannels((AUChannelInfo *) data,
					   datasize);
    break;
  case kAudioUnitProperty_MaximumFramesPerSlice: /* 14 */
    fprintf(asysn_audiounit_logfile, "\n\tMaximumFramesPerSlice value is %u", 
	    (unsigned int) *((UInt32 *) data));
    break;
  case kAudioUnitProperty_SetExternalBuffer: /* 15 */
    fprintf(asysn_audiounit_logfile, "\n\tBuffer size: %u bytes", 
	    (unsigned int) ((AudioUnitExternalBuffer *) data)->size);
    break;
  case kAudioUnitProperty_ParameterValueStrings: /* 16 */
    asysn_audiounit_print_property_ParameterValueStrings((CFArrayRef *) data);
    break;
  case kAudioUnitProperty_MIDIControlMapping: /* 17 */ 
    fprintf(asysn_audiounit_logfile, "\n\tArray of %u MIDI control mappings",
	    (unsigned int) (datasize/sizeof(AudioUnitMIDIControlMapping)));
    break;
  case kAudioUnitProperty_GetUIComponentList: /* 18 */
    fprintf(asysn_audiounit_logfile, 
	    "\n\tArray of %u user-interface component instances",
	    (unsigned int) (datasize/sizeof(ComponentInstance)));
    break;
  case kAudioUnitProperty_AudioChannelLayout: /* 19 */  
    fprintf(asysn_audiounit_logfile, "\n\tLayout data for multi-channel audio");
    break;
  case kAudioUnitProperty_TailTime: /* 20 */
    fprintf(asysn_audiounit_logfile, "\n\tTail time is %lfs", *((Float64 *) data));
    break;
  case kAudioUnitProperty_BypassEffect: /* 21 */
    fprintf(asysn_audiounit_logfile, "\n\tAudio is %s", 
	    *((UInt32 *) data) ? "bypassed" : "not-bypassed");
    break;
  case kAudioUnitProperty_LastRenderError: /* 22 */
    fprintf(asysn_audiounit_logfile, "\n\tLast error code was %i",  
	    (int) *((OSStatus *) data));
    break;
  case kAudioUnitProperty_SetRenderCallback: /* 23 */
    fprintf(asysn_audiounit_logfile, "\n\tFunction address %u, data object address %u", 
	    (unsigned int)(((AURenderCallbackStruct *) data)->inputProc),
	    (unsigned int)(((AURenderCallbackStruct *) data)->inputProcRefCon));
    break;
  case kAudioUnitProperty_FactoryPresets: /* 24 */
    asysn_audiounit_print_property_FactoryPresets((CFArrayRef *) data);
    break;
  case kAudioUnitProperty_ContextName: /* 25 */
    fprintf(asysn_audiounit_logfile, "\n\t%sCFStringRef for a context name",
	    *((CFStringRef *) data) == NULL ? "Null-ptr " : "");
    break;
  case kAudioUnitProperty_RenderQuality: /* 26 */
    fprintf(asysn_audiounit_logfile, "\n\tRenderQuality value is %u", 
	    (unsigned int) *((UInt32 *) data));
    break;
  case kAudioUnitProperty_HostCallbacks: /* 27 */
    if (((HostCallbackInfo *) data)->beatAndTempoProc)
      fprintf(asysn_audiounit_logfile, "\n\tOffering GetBeatAndTempo callback");
    if (((HostCallbackInfo *) data)->musicalTimeLocationProc)
      fprintf(asysn_audiounit_logfile, "\n\tOffering GetMusicalTimeLocation callback");
    if (((HostCallbackInfo *) data)->transportStateProc)
      fprintf(asysn_audiounit_logfile, "\n\tOffering GetTransportStateProc callback");
    break;
  case kAudioUnitProperty_CurrentPreset: /* 28 */
    fprintf(asysn_audiounit_logfile, "\n\tCurrent preset index is %i", 
	    (int) *((SInt32 *) data));
    break;
  case kAudioUnitProperty_InPlaceProcessing: /* 29 */
    fprintf(asysn_audiounit_logfile, "\n\tInPlaceProcessing value is %s", 
	    *((UInt32 *) data) ? "on" : "off");
    break;
  case kAudioUnitProperty_ElementName: /* 30 */
    fprintf(asysn_audiounit_logfile, "\n\t%sCFStringRef for an element name",
	    *((CFStringRef *) data) == NULL ? "Null-ptr " : "");
    break;
  case kAudioUnitProperty_CocoaUI: /* 31 */
    asysn_audiounit_print_property_CocoaViewInfo((AudioUnitCocoaViewInfo *) data,
						 datasize);
    break;
  case kAudioUnitProperty_SupportedChannelLayoutTags: /* 32 */
    fprintf(asysn_audiounit_logfile, 
	    "\n\tInformation about multi-channel audio layouts");
    break;
  case kAudioUnitProperty_ParameterStringFromValue: /* 33 */
    /* also kAudioUnitProperty_ParameterValueName */
    fprintf(asysn_audiounit_logfile, "\n\tString for value %g for Parameter ID %u", 
	    *(((AudioUnitParameterValueName *) data)->inValue),
	    (unsigned int) ((AudioUnitParameterValueName *) data)->inParamID);
    break;
  case kAudioUnitProperty_ParameterIDName: /* 34 */
    fprintf(asysn_audiounit_logfile, "\n\tExtended naming info for a parameter");
    break;
  case kAudioUnitProperty_ParameterClumpName: /* 35 */
    fprintf(asysn_audiounit_logfile, "\n\tExtended naming info for a parameter clump");
    break;
  case kAudioUnitProperty_PresentPreset: /* 36 */
    if (CFStringGetCString(((AUPreset *) data)->presetName, name, 
			   namesize, kCFStringEncodingASCII))
      fprintf(asysn_audiounit_logfile, "\n\tCurrent preset is %s (%u)", 
	      name, (unsigned int) ((AUPreset *) data)->presetNumber);
    else
      fprintf(asysn_audiounit_logfile, "\n\tCurrent preset is %u", 
	      (unsigned int) ((AUPreset *) data)->presetNumber);
    break;
  case kAudioUnitProperty_OfflineRender: /* 37 */
    fprintf(asysn_audiounit_logfile, "\n\tOffline rendering is %s", 
	    *((UInt32 *) data) ? "on" : "off");
    break;
  case kAudioUnitProperty_ParameterValueFromString: /* 38 */
    fprintf(asysn_audiounit_logfile, "\n\tString for value %g for Parameter ID %u", 
	    ((AudioUnitParameterValueFromString *) data)->outValue,
	    (unsigned int)((AudioUnitParameterValueFromString *) data)->inParamID);
    break;
  case kAudioUnitProperty_IconLocation: /* 39 */
    fprintf(asysn_audiounit_logfile, "\n\t%sCFURLRef for an icon location",
	    *((CFURLRef *) data) == NULL ? "Null-ptr " : "");
    break;
  case kAudioUnitProperty_PresentationLatency: /* 40 */
    fprintf(asysn_audiounit_logfile, "\n\tPresentation latency is %lg", 
	    *((Float64 *) data));
    break;
  case kAudioUnitProperty_AllParameterMIDIMappings: /* 41 */
    fprintf(asysn_audiounit_logfile, "\n\tMIDI mapping data structures");
    break;
  case kAudioUnitProperty_AddParameterMIDIMapping: /* 42 */
    fprintf(asysn_audiounit_logfile, "\n\tMIDI mapping data structures");
    break;
  case kAudioUnitProperty_RemoveParameterMIDIMapping: /* 43 */
    fprintf(asysn_audiounit_logfile, "\n\tMIDI mapping data structures");
    break;
  case kAudioUnitProperty_HotMapParameterMIDIMapping: /* 44 */
    fprintf(asysn_audiounit_logfile, "\n\tMIDI mapping data structures");
    break;
  case kAudioUnitProperty_DependentParameters: /* 45 */
    fprintf(asysn_audiounit_logfile, "\n\tMIDI mapping data structures");
    break;
  case kAudioUnitProperty_ClassInfoFromDocument: /* 50 */
    asysn_audiounit_print_property_ClassInfo((CFMutableDictionaryRef *) data);
    break;
  case kMusicDeviceProperty_InstrumentCount: /* 1000 */
    fprintf(asysn_audiounit_logfile, "\n\tNumber of instruments: %u", 
	    (unsigned int) *((UInt32 *) data));
    break;
  case kMusicDeviceProperty_InstrumentName: /* 1001 */
    fprintf(asysn_audiounit_logfile, "\n\tInstrument name for %u: %s", (unsigned int) element, 
	    *((char **) data));
    break;
  case kMusicDeviceProperty_GroupOutputBus: /* 1002 */
    fprintf(asysn_audiounit_logfile, "\n\tOutput bus for group %u: %u", (unsigned int) element, 
	    (unsigned int) *((UInt32 *) data));
    break;
  case kMusicDeviceProperty_SoundBankFSSpec: /* 1003 */
    fprintf(asysn_audiounit_logfile, "\n\tPointer to a SoundBank file");
    break;
  case kMusicDeviceProperty_InstrumentNumber: /* 1004 */
    fprintf(asysn_audiounit_logfile, "\n\tInstrument index %u has ID %u", (unsigned int) element, 
	    (unsigned int) *((MusicDeviceInstrumentID *) data));
    break;
  case kMusicDeviceProperty_UsesInternalReverb: /* 1005 */ 
    /* also kAudioUnitProperty_UsesInternalReverb */
    fprintf(asysn_audiounit_logfile, "\n\tInternal reverb is %s", 
	    *((UInt32 *) data) ? "in use" : "not in use");
    break;
  case kMusicDeviceProperty_MIDIXMLNames: /* 1006 */
    fprintf(asysn_audiounit_logfile, "\n\t%sCFURLRef for a MIDI XML file",
	    *((CFURLRef *) data) == NULL ? "Null-ptr " : "");
    break;
  case kMusicDeviceProperty_BankName: /* 1007 */
    fprintf(asysn_audiounit_logfile, "\n\t%sCFStringRef for an bank name",
	    *((CFStringRef *) data) == NULL ? "Null-ptr " : "");
    break;
  case kMusicDeviceProperty_SoundBankData: /* 1008 */
    fprintf(asysn_audiounit_logfile, "\n\tPointer to an in-memory SoundBank");
    break;
  case kMusicDeviceProperty_PartGroup: /* 1010 */
    fprintf(asysn_audiounit_logfile, "\n\tGroup IO for for part %u: %u", (unsigned int) element, 
	    (unsigned int) *((UInt32 *) data));
    break;
  case kMusicDeviceProperty_StreamFromDisk: /* 1011 */
    fprintf(asysn_audiounit_logfile, "\n\tStreamFromDisk value: %u", 
	    (unsigned int) *((UInt32 *) data));
    break;
  case kMusicDeviceProperty_SoundBankFSRef: /* 1012 */
    fprintf(asysn_audiounit_logfile, "\n\tReference to a SoundBank file");
    break;
  case kAudioOutputUnitProperty_CurrentDevice: /* 2000 */
    fprintf(asysn_audiounit_logfile, "\n\tID of current device: %u", 
	    (unsigned int) *((UInt32 *) data));
    break;
  case kAudioOutputUnitProperty_IsRunning: /* 2001 */
    fprintf(asysn_audiounit_logfile, "\n\tDevice is%s running",
	    *((UInt32 *) data) ? "" : " not");
    break;
  case kAudioOutputUnitProperty_ChannelMap: /* 2002 */ 
    fprintf(asysn_audiounit_logfile, "\n\tI/O channel map for an OutputUnit");
    break;
  case kAudioOutputUnitProperty_EnableIO: /* 2003 */
    fprintf(asysn_audiounit_logfile, "\n\t%s for unit is %s", 
	    element ? "Input" : "Output",  
	    *((UInt32 *) data) ? "enabled" : "disabled");
    break;
  case kAudioOutputUnitProperty_StartTime: /* 2004 */
    if (((AudioOutputUnitStartAtTimeParams *) data)->mTimestamp.mFlags & 
	kAudioTimeStampSampleTimeValid)
      fprintf(asysn_audiounit_logfile, "\n\tStart at sample time: %lg", 
	      ((AudioOutputUnitStartAtTimeParams *) data)->mTimestamp.mSampleTime);
    if (((AudioOutputUnitStartAtTimeParams *) data)->mTimestamp.mFlags & 
	kAudioTimeStampSampleTimeValid)
      fprintf(asysn_audiounit_logfile, "\n\tStart at host time: %llu", 
	      (unsigned long long) 
	      ((AudioOutputUnitStartAtTimeParams *) data)->mTimestamp.mHostTime);
    break;
  case kAudioOutputUnitProperty_SetInputCallback: /* 2005 */
    fprintf(asysn_audiounit_logfile, "\n\tCallback for new input from an Output Unit");
    break;
  case kAudioOutputUnitProperty_HasIO: /* 2006 */
    fprintf(asysn_audiounit_logfile, "\n\t%s for unit is %s", 
	    element ? "Input" : "Output",  
	    *((UInt32 *) data) ? "available" : "not available");
    break;
  case kAudioOutputUnitProperty_StartTimestampsAtZero: /* 2007 */ 
    fprintf(asysn_audiounit_logfile, "\n\tTimestamps start %s",
	    *((UInt32 *) data) ? "at zero" : "reflects HAL timestamps");
    break;
  case kAudioUnitProperty_SpatializationAlgorithm: /* 3000 */
    switch (*((UInt32 *)data)) {
    case kSpatializationAlgorithm_EqualPowerPanning:
      fprintf(asysn_audiounit_logfile, "\n\tEqual Power Panning");
      break;
    case kSpatializationAlgorithm_SphericalHead:
      fprintf(asysn_audiounit_logfile, "\n\tSpherical Head");
      break;
    case kSpatializationAlgorithm_HRTF:
      fprintf(asysn_audiounit_logfile, "\n\tHRTF");
      break;
    case kSpatializationAlgorithm_SoundField:
      fprintf(asysn_audiounit_logfile, "\n\tSound Field");
      break;
    case kSpatializationAlgorithm_VectorBasedPanning:
      fprintf(asysn_audiounit_logfile, "\n\tVector-Based Panning");
      break;
    case kSpatializationAlgorithm_StereoPassThrough:
      fprintf(asysn_audiounit_logfile, "\n\tStereo Pass Through");
      break;
    }
    break;
  case kAudioUnitProperty_SpeakerConfiguration: /* 3001 */
    switch (*((UInt32 *)data)) {
    case kSpeakerConfiguration_HeadPhones: 
      fprintf(asysn_audiounit_logfile, "\n\tHeadPhones");
      break;
    case kSpeakerConfiguration_Stereo: 
      fprintf(asysn_audiounit_logfile, "\n\tStereo");
      break;
    case kSpeakerConfiguration_Quad:
      fprintf(asysn_audiounit_logfile, "\n\tQuad");
      break;
    case  kSpeakerConfiguration_5_0:
      fprintf(asysn_audiounit_logfile, "\n\t5.0/5.1 speaker setup");
      break;
    }
    break;
  case kAudioUnitProperty_DopplerShift: /* 3002 */
    fprintf(asysn_audiounit_logfile, "\n\tDoppler shift is %s",
	    *((UInt32 *) data) ? "on" : "off");
    break;
  case kAudioUnitProperty_3DMixerRenderingFlags: /* 3003 */
    break;
  case kAudioUnitProperty_3DMixerDistanceAtten: /* 3004 */
    break;
  case kAudioUnitProperty_MatrixLevels: /* 3006 */
    break;
  case kAudioUnitProperty_MeteringMode: /* 3007 */
    break;
#if (0)      /* depreciated */
  case kAudioUnitProperty_PannerMode: /* 3008 */
    switch (*((UInt32 *)data)) {
    case kPannerMode_Normal:
      fprintf(asysn_audiounit_logfile, "\n\tPassThru");
      break;    
    case kPannerMode_FaderMode:
      fprintf(asysn_audiounit_logfile, "\n\tFader");
      break;    
    case 2:   /* kPannerMode_Panner */
      fprintf(asysn_audiounit_logfile, "\n\tPanner");
      break;    
    }
    break;
#endif
  case kAudioUnitProperty_MatrixDimensions: /* 3009 */
    break;
  case kAudioUnitProperty_3DMixerDistanceParams: /* 3010 */
    break;
  case kAudioUnitProperty_MeterClipping: /* 3011 */ 
    break;
  case kAudioOfflineUnitProperty_InputSize: /* 3020 */
    fprintf(asysn_audiounit_logfile, "\n\tInput size is %llu", *((UInt64 *) data));
    break;
  case kAudioOfflineUnitProperty_OutputSize: /* 3021 */
    fprintf(asysn_audiounit_logfile, "\n\tOutput size is %llu", *((UInt64 *) data));
    break;
  case kAudioUnitOfflineProperty_StartOffset: /* 3022 */
    fprintf(asysn_audiounit_logfile, "\n\tStart offset is is %llu", 
	    *((UInt64 *) data));
    break;
  case kAudioUnitOfflineProperty_PreflightRequirements: /* 3023 */
    switch (*((UInt32 *)data)) {
    case kOfflinePreflight_NotRequired:
      fprintf(asysn_audiounit_logfile, "\n\tNot Required");
      break;    
    case kOfflinePreflight_Optional:
      fprintf(asysn_audiounit_logfile, "\n\tOptional");
      break;    
    case kOfflinePreflight_Required:
      fprintf(asysn_audiounit_logfile, "\n\tRequired");
      break;    
    }
    break;
  case kAudioUnitOfflineProperty_PreflightName: /* 3024 */
    fprintf(asysn_audiounit_logfile, "\n\t%sCFStringRef for an pre-flight name",
	    *((CFStringRef *) data) == NULL ? "Null-ptr " : "");
    break;
  case kAudioUnitProperty_ScheduleAudioSlice: /* 3300 */	
    break;
  case kAudioUnitProperty_ScheduleStartTimeStamp: /* 3301 */ 
    break;
  case kAudioUnitProperty_CurrentPlayTime: /* 3302 */ 
    break;
  case kAudioUnitProperty_ScheduledFileIDs: /* 3310 */	
    break;
  case kAudioUnitProperty_ScheduledFileRegion: /* 3311 */  
    break;
  case kAudioUnitProperty_ScheduledFilePrime: /* 3312 */   
    break;
  case kAudioUnitProperty_ScheduledFileBufferSizeFrames: /* 3313 */
    break;
  case kAudioUnitProperty_ScheduledFileNumberBuffers: /* 3314 */	
    break;
  case kAudioUnitProperty_DeferredRendererPullSize: /* 3320 */	
    break;
  case kAudioUnitProperty_DeferredRendererExtraLatency: /* 3321 */  
    break;
  case kAudioUnitProperty_DeferredRendererWaitFrames: /* 3322 */	
    break;
  case kAUNetReceiveProperty_Hostname: /* 3511 */
    break;
  case kAUNetReceiveProperty_Password: /* 3512 */
    break;
  case kAUNetSendProperty_PortNum: /* 3513 */
    break;
  case kAUNetSendProperty_TransmissionFormat: /* 3514 */
    break;
  case kAUNetSendProperty_TransmissionFormatIndex: /* 3515 */
    break;
  case kAUNetSendProperty_ServiceName: /* 3516 */
    break;
  case kAUNetSendProperty_Disconnect: /* 3517 */
    break;
  case kAUNetSendProperty_Password: /* 3518 */
    break;
  case kAudioUnitMigrateProperty_FromPlugin: /* 4000 */
    fprintf(asysn_audiounit_logfile, "\n\t%sCFArrayRef of plug-in conversion data",
	    *((CFArrayRef *) data) == NULL ? "Null-ptr " : "");
    break;
  case kAudioUnitMigrateProperty_OldAutomation: /* 4001 */
    fprintf(asysn_audiounit_logfile, 
	    "\n\tAutomation data for plug-in format conversion");
    break;
  default:
    break;
  }
}


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/* Selector-Specific Print Routines */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/*************************************/
/* kComponentCanDoSelect             */
/*                                   */
/*  params[0]: selector              */
/*                                   */
/*************************************/

void asysn_audiounit_print_kComponentCanDoSelect(ComponentParameters * p, 
						ComponentResult returncode)
{
  asysn_audiounit_print_selector_name(p->what);

  switch ((int)((SInt16)(p->params[0]))) {
    case kComponentOpenSelect:
    case kComponentCloseSelect:
    case kAudioUnitInitializeSelect:
    case kAudioUnitUninitializeSelect:
    case kAudioUnitGetPropertyInfoSelect:
    case kAudioUnitGetPropertySelect:
    case kAudioUnitSetPropertySelect:
    case kAudioUnitRenderSelect:
    case kAudioUnitResetSelect:
    case kAudioUnitAddPropertyListenerSelect:
    case kAudioUnitRemovePropertyListenerSelect:
    case kAudioUnitRemovePropertyListenerWithUserDataSelect:
    case kAudioUnitAddRenderNotifySelect:
    case kAudioUnitRemoveRenderNotifySelect:
    case kAudioUnitGetParameterSelect:
    case kAudioUnitSetParameterSelect:
    case kMusicDeviceMIDIEventSelect:
      fprintf(asysn_audiounit_logfile, "\n\tFor known selector ");
      asysn_audiounit_print_selector_name((int)((SInt16)(p->params[0])));
      break;
    default:
      fprintf(asysn_audiounit_logfile, "\n\tFor unknown selector (%i)", 
	      (int)((SInt16)(p->params[0])));
      break;
    }

  asysn_audiounit_print_nonzero_returncode(returncode);
  asysn_audiounit_print_paramsize_check(p);
  fprintf(asysn_audiounit_logfile, ".\n\n");
}

/*************************************/
/* kComponentOpenSelect              */
/*                                   */
/*  params[0]: ComponentInstance ci  */
/*                                   */
/*************************************/

void asysn_audiounit_print_kComponentOpenSelect(ComponentParameters * p, 
						ComponentResult returncode)
{
  asysn_audiounit_print_selector_name(p->what);
  fprintf(asysn_audiounit_logfile, "\n\tFor # %u.  %i instance(s) now open", 
	  (unsigned) (p->params[0]), asysn_audiounit_opencount);
  asysn_audiounit_print_nonzero_returncode(returncode);
  asysn_audiounit_print_paramsize_check(p);
  fprintf(asysn_audiounit_logfile, ".\n\n");
}

/*************************************/
/* kComponentCloseSelect             */
/*                                   */
/*  params[0]: ComponentInstance ci  */
/*                                   */
/*************************************/

void asysn_audiounit_print_kComponentCloseSelect(ComponentParameters * p, 
						 ComponentResult returncode)
{
  asysn_audiounit_print_selector_name(p->what);
  fprintf(asysn_audiounit_logfile, "\n\tFor instance # %u.  %i instance(s) remain open",
	   (unsigned) (p->params[0]), asysn_audiounit_opencount);
  asysn_audiounit_print_nonzero_returncode(returncode);
  asysn_audiounit_print_paramsize_check(p);
  fprintf(asysn_audiounit_logfile, ".\n\n");
}

/*************************************/
/* kAudioUnitInitializeSelect        */
/*                                   */
/*************************************/

void asysn_audiounit_print_kAudioUnitInitializeSelect(ComponentParameters * p, 
						      ComponentResult returncode)
{
  asysn_audiounit_print_selector_name(p->what);
  asysn_audiounit_print_nonzero_returncode(returncode);
  asysn_audiounit_print_paramsize_check(p);
  fprintf(asysn_audiounit_logfile, ".\n\n");
}

/*************************************/
/* kAudioUnitUninitializeSelect      */
/*                                   */
/*************************************/

void asysn_audiounit_print_kAudioUnitUninitializeSelect(ComponentParameters * p, 
							ComponentResult returncode)
{
  asysn_audiounit_print_selector_name(p->what);
  asysn_audiounit_print_nonzero_returncode(returncode);
  asysn_audiounit_print_paramsize_check(p);
  fprintf(asysn_audiounit_logfile, ".\n\n");
}

/********************************************/
/* kAudioUnitGetPropertyInfoSelect          */
/*                                          */
/*  params[0]: Boolean * outWritable;       */
/*  params[1]: UInt32 * outDataSize;        */
/*  params[2]: AudioUnitElement inElement;  */
/*  params[3]: AudioUnitScope inScope;      */
/*  params[4]: AudioUnitPropertyID inID;    */
/*                                          */
/********************************************/

void asysn_audiounit_print_kAudioUnitGetPropertyInfoSelect(ComponentParameters * p, 
							   ComponentResult returncode)
{
  asysn_audiounit_print_selector_name(p->what);
  fprintf(asysn_audiounit_logfile, "\n\tFor "); 
  asysn_audiounit_print_property_name(p->params[4]);
  if ((p->params[0]) && (p->params[1]))
    {
      fprintf(asysn_audiounit_logfile, "\n\t%u byte %s property on bus %i, ", 
	      (unsigned int) *((UInt32 *)(p->params[1])), *((Boolean *)(p->params[0])) ? 
	      "writeable" : "read-only", (int) p->params[2]);
      asysn_audiounit_print_scope_name(p->params[3]);
    }
  else
    fprintf(asysn_audiounit_logfile, "\n\tNULL pointer(s) returned");
  asysn_audiounit_print_nonzero_returncode(returncode);
  asysn_audiounit_print_paramsize_check(p);
  fprintf(asysn_audiounit_logfile, ".\n\n");
}


/********************************************/
/* kAudioUnitGetPropertySelect              */
/*                                          */
/*  params[0]: UInt32 * ioDataSize;         */
/*  params[1]: void * outData;              */
/*  params[2]: AudioUnitElement inElement;  */
/*  params[3]: AudioUnitScope inScope;      */
/*  params[4]: AudioUnitPropertyID inID;    */
/*                                          */
/********************************************/

void asysn_audiounit_print_kAudioUnitGetPropertySelect(ComponentParameters * p, 
						       ComponentResult returncode)
{
  asysn_audiounit_print_selector_name(p->what);
  fprintf(asysn_audiounit_logfile, "\n\tFor "); 
  asysn_audiounit_print_property_name(p->params[4]);
  asysn_audiounit_print_property_data(p->params[4], /* AudioUnitPropertyID id */ 
		      p->params[3],                   /* AudioUnitScope scope */
		      p->params[2],               /* AudioUnitElement element */
		      *((UInt32 *)((void *)(p->params[0]))), /* UInt32 datasize */
		      (void *)(p->params[1]));               /* void * data */
  asysn_audiounit_print_nonzero_returncode(returncode);
  asysn_audiounit_print_paramsize_check(p);
  fprintf(asysn_audiounit_logfile, ".\n\n");
}


/********************************************/
/* kAudioUnitSetPropertySelect              */
/*                                          */
/*  params[0]: UInt32 inDataSize;           */
/*  params[1]: const void * inData;         */
/*  params[2]: AudioUnitElement inElement;  */
/*  params[3]: AudioUnitScope inScope;      */
/*  params[4]: AudioUnitPropertyID inID;    */
/*                                          */
/********************************************/

void asysn_audiounit_print_kAudioUnitSetPropertySelect(ComponentParameters * p, 
						       ComponentResult returncode)
{
  asysn_audiounit_print_selector_name(p->what);
  fprintf(asysn_audiounit_logfile, "\n\tFor "); 
  asysn_audiounit_print_property_name(p->params[4]);
  asysn_audiounit_print_property_data(p->params[4], /* AudioUnitPropertyID id */ 
				      p->params[3], /* AudioUnitScope scope */
				      p->params[2], /* AudioUnitElement element */ 
				      p->params[0], /* UInt32 datasize */
				      (void *)(p->params[1]));  /* void * data */
  asysn_audiounit_print_nonzero_returncode(returncode);
  asysn_audiounit_print_paramsize_check(p);
  fprintf(asysn_audiounit_logfile, ".\n\n");
}


/*****************************************************/
/* kAudioUnitAddPropertyListenerSelect               */
/*                                                   */
/*  params[0]: void * inProcRefCon;                  */
/*  params[1]: AudioUnitPropertyListenerProc inProc; */
/*  params[2]: AudioUnitPropertyID inID;             */
/*                                                   */
/* The caller passes in a function inProc to call    */
/* the AU changes the property, and an object        */ 
/* inProcRefCon to pass to the function.             */
/*****************************************************/

void asysn_audiounit_print_kAudioUnitAddPropertyListenerSelect
(ComponentParameters * p, ComponentResult returncode)

{
  asysn_audiounit_print_selector_name(p->what);
  fprintf(asysn_audiounit_logfile, "\n\tFor "); 
  asysn_audiounit_print_property_name(p->params[2]);
  fprintf(asysn_audiounit_logfile, ", faddr %u", (unsigned int) (p->params[1])); 
  fprintf(asysn_audiounit_logfile, ", caddr %u", (unsigned int) (p->params[0])); 
  asysn_audiounit_print_nonzero_returncode(returncode);
  asysn_audiounit_print_paramsize_check(p);
  fprintf(asysn_audiounit_logfile, ".\n\n");
}

/*****************************************************/
/* kAudioUnitRemovePropertyListenerSelect            */
/*                                                   */
/*  params[0]: AudioUnitPropertyListenerProc inProc; */
/*  params[1]: AudioUnitPropertyID inID;             */
/*                                                   */
/* Instructs the AudioUnit to stop calling the       */
/* inProc for the specified property inID.           */
/*****************************************************/

void asysn_audiounit_print_kAudioUnitRemovePropertyListenerSelect
(ComponentParameters * p, ComponentResult returncode)

{
  asysn_audiounit_print_selector_name(p->what);
  fprintf(asysn_audiounit_logfile, "\n\tFor "); 
  asysn_audiounit_print_property_name(p->params[1]);
  fprintf(asysn_audiounit_logfile, ", function address %u", 
	  (unsigned int) (p->params[0])); 
  asysn_audiounit_print_nonzero_returncode(returncode);
  asysn_audiounit_print_paramsize_check(p);
  fprintf(asysn_audiounit_logfile, ".\n\n");
}

/******************************************************/
/* kAudioUnitRemovePropertyListenerWithUserDataSelect */
/*                                                    */
/*  params[0]: void * inProcRefCon;                   */
/*  params[1]: AudioUnitPropertyListenerProc inProc;  */
/*  params[2]: AudioUnitPropertyID inID;              */
/*                                                    */
/* Instructs the AudioUnit to stop calling the        */
/* inProc/ProcRefCon for the specified property inID. */
/******************************************************/

void asysn_audiounit_print_kAudioUnitRemovePropertyListenerWithUserDataSelect
(ComponentParameters * p, ComponentResult returncode)

{
  asysn_audiounit_print_selector_name(p->what);
  fprintf(asysn_audiounit_logfile, "\n\tFor "); 
  asysn_audiounit_print_property_name(p->params[2]);
  fprintf(asysn_audiounit_logfile, ", faddr %u", (unsigned int) (p->params[1])); 
  fprintf(asysn_audiounit_logfile, ", caddr %u", (unsigned int) (p->params[0])); 
  asysn_audiounit_print_nonzero_returncode(returncode);
  asysn_audiounit_print_paramsize_check(p);
  fprintf(asysn_audiounit_logfile, ".\n\n");
}

/*************************************/
/* kAudioUnitAddRenderNotifySelect   */
/*                                   */
/*  params[0]: void * inProcRefCon;  */
/*  params[1]: ProcPtr inProc;       */
/*                                   */
/*************************************/

void asysn_audiounit_print_kAudioUnitAddRenderNotifySelect
(ComponentParameters * p, ComponentResult returncode)
{
  asysn_audiounit_print_selector_name(p->what);
  asysn_audiounit_print_nonzero_returncode(returncode);
  asysn_audiounit_print_paramsize_check(p);
  fprintf(asysn_audiounit_logfile, ".\n\n");
}

/****************************************/
/* kAudioUnitRemoveRenderNotifySelect   */
/*                                      */
/*  params[0]: void * inProcRefCon;     */
/*  params[1]: ProcPtr inProc;          */
/*                                      */
/****************************************/

void asysn_audiounit_print_kAudioUnitRemoveRenderNotifySelect
(ComponentParameters * p, ComponentResult returncode)
{
  asysn_audiounit_print_selector_name(p->what);
  asysn_audiounit_print_nonzero_returncode(returncode);
  asysn_audiounit_print_paramsize_check(p);
  fprintf(asysn_audiounit_logfile, ".\n\n");
}

/*******************************************/
/* kAudioUnitGetParameterSelect            */
/*                                         */
/* params[0]: Float32 * outValue           */
/* params[1]: AudioUnitElement inElement   */
/* params[2]: AudioUnitScope inScope       */
/* params[3]: AudioUnitParameterID inID    */
/*                                         */
/*******************************************/

void asysn_audiounit_print_kAudioUnitGetParameterSelect(ComponentParameters * p, 
							ComponentResult returncode)
{
  asysn_audiounit_print_selector_name(p->what);
  fprintf(asysn_audiounit_logfile, "\n\tParameter "); 
  asysn_audiounit_print_parameter_name(p->params[3]);
  fprintf(asysn_audiounit_logfile, " value is %g", *((Float32 *)(p->params[0])));
  fprintf(asysn_audiounit_logfile, "\n\t"); 
  asysn_audiounit_print_scope_name(p->params[2]);
  fprintf(asysn_audiounit_logfile, ", element %i", (int) p->params[1]); 
  asysn_audiounit_print_nonzero_returncode(returncode);
  asysn_audiounit_print_paramsize_check(p);
  fprintf(asysn_audiounit_logfile, ".\n\n");
}

/*********************************************/
/* kAudioUnitSetParameterSelect              */
/*                                           */
/* params[0]: UInt32 inBufferOffsetInFrames  */
/* params[1]: Float32 inValue                */
/* params[2]: AudioUnitElement inElement     */
/* params[3]: AudioUnitScope inScope;        */
/* params[4]: AudioUnitParameterID inID;     */
/*                                           */
/*********************************************/

void asysn_audiounit_print_kAudioUnitSetParameterSelect(ComponentParameters * p, 
							ComponentResult returncode)
{
  asysn_audiounit_print_selector_name(p->what);
  fprintf(asysn_audiounit_logfile, "\n\tParameter "); 
  asysn_audiounit_print_parameter_name(p->params[4]);

  /* todo: verify endian-ness correctness */

  fprintf(asysn_audiounit_logfile, " value is %g", 
	  *((Float32 *)((unsigned char *)(&(p->params[1])))));

  if (p->params[0])
    fprintf(asysn_audiounit_logfile, " (frame offset %u)", 
	    (unsigned int)(p->params[0]));
  fprintf(asysn_audiounit_logfile, "\n\t"); 
  asysn_audiounit_print_scope_name(p->params[3]);
  fprintf(asysn_audiounit_logfile, ", element %i", (int) p->params[2]); 
  asysn_audiounit_print_nonzero_returncode(returncode);
  asysn_audiounit_print_paramsize_check(p);
  fprintf(asysn_audiounit_logfile, ".\n\n");
}


/***************************************************************/
/* kAudioUnitScheduleParametersSelect                          */
/*                                                             */
/* params[0]: UInt32 inNumParamEvents;                         */
/* params[1]: const AudioUnitParameterEvent* inParameterEvent; */
/*                                                             */
/***************************************************************/

void asysn_audiounit_print_kAudioUnitScheduleParametersSelect(ComponentParameters * p, 
							      ComponentResult returncode)
{
  asysn_audiounit_print_selector_name(p->what);
  fprintf(asysn_audiounit_logfile, "\n\tScheduling %u updates ", 
	  (unsigned int)(p->params[0])); 
  asysn_audiounit_print_nonzero_returncode(returncode);
  asysn_audiounit_print_paramsize_check(p);
  fprintf(asysn_audiounit_logfile, ".\n\n");
}


/**********************************************************/
/* kAudioUnitRenderSelect                                 */
/*                                                        */
/* params[0]: AudioBufferList * ioData;                   */
/* params[1]: UInt32 inNumberFrames;                      */
/* params[2]: UInt32 inOutputBusNumber;                   */
/* params[3]: const AudioTimeStamp * inTimeStamp;         */
/* params[4]: AudioUnitRenderActionFlags * ioActionFlags; */
/*                                                        */
/**********************************************************/

void asysn_audiounit_print_kAudioUnitRenderSelect(ComponentParameters * p, 
						  ComponentResult returncode)
{

}


/*******************************************/
/* kAudioUnitResetSelect                   */
/*                                         */
/*  params[0]: AudioUnitElement inElement; */
/*  params[1]: AudioUnitScope inScope;     */
/*                                         */
/*******************************************/

void asysn_audiounit_print_kAudioUnitResetSelect(ComponentParameters * p, 
						 ComponentResult returncode)
{
  asysn_audiounit_print_selector_name(p->what);
  fprintf(asysn_audiounit_logfile, "\n\tIn "); 
  asysn_audiounit_print_scope_name(p->params[1]);
  fprintf(asysn_audiounit_logfile, ", element %i", (int) p->params[0]); 
  asysn_audiounit_print_nonzero_returncode(returncode);
  asysn_audiounit_print_paramsize_check(p);
  fprintf(asysn_audiounit_logfile, ".\n\n");
}


/******************************************/
/* kMusicDeviceMIDIEventSelect            */
/*                                        */
/*  params[3]: UInt32 inStatus            */
/*  params[2]: UInt32 inData1             */
/*  params[1]: UInt32 inData2             */
/*  params[0]: UInt32 inOffsetSampleFrame */
/*                                        */
/******************************************/

void asysn_audiounit_print_kMusicDeviceMIDIEventSelect(ComponentParameters * p, 
						       ComponentResult returncode)
{
  asysn_audiounit_print_selector_name(p->what);
  asysn_audiounit_print_midievent((unsigned char)((UInt32)(p->params[3])),
		  (unsigned char)((UInt32)(p->params[2])),
		  (unsigned char)((UInt32)(p->params[1])));
  if (p->params[0])
    fprintf(asysn_audiounit_logfile, "\n\tOffset %u samples from buffer start",
	    (unsigned int)(p->params[0])); 
  asysn_audiounit_print_nonzero_returncode(returncode);
  asysn_audiounit_print_paramsize_check(p);
  fprintf(asysn_audiounit_logfile, ".\n\n");
}


/**********************************/
/* kMusicDeviceSysExSelect        */
/*                                */
/*  params[0]: UInt8 * inData     */
/*  params[1]: UInt32 inLength    */
/*    (order may be wrong)        */
/**********************************/

void asysn_audiounit_print_kMusicDeviceSysExSelect(ComponentParameters * p, 
						   ComponentResult returncode)
{
  asysn_audiounit_print_selector_name(p->what);
  fprintf(asysn_audiounit_logfile, "\n\t%u-byte MIDI SysEx command", 
	  (unsigned int)(p->params[1])); 
  asysn_audiounit_print_nonzero_returncode(returncode);
  asysn_audiounit_print_paramsize_check(p);
  fprintf(asysn_audiounit_logfile, ".\n\n");
}



/****************************************************/
/* kMusicDevicePrepareInstrumentSelect              */
/*                                                  */
/* params[0]: MusicDeviceInstrumentID inInstrument  */
/*                                                  */
/****************************************************/

void asysn_audiounit_print_kMusicDevicePrepareInstrumentSelect
(ComponentParameters * p, ComponentResult returncode)
{
  asysn_audiounit_print_selector_name(p->what);
  fprintf(asysn_audiounit_logfile, "\n\tInstrument ID: %u", 
	  (unsigned int)(p->params[0])); 
  asysn_audiounit_print_nonzero_returncode(returncode);
  asysn_audiounit_print_paramsize_check(p);
  fprintf(asysn_audiounit_logfile, ".\n\n");
}


/****************************************************/
/* kMusicDeviceReleaseInstrumentSelect              */
/*                                                  */
/* params[0]: MusicDeviceInstrumentID inInstrument  */
/*                                                  */
/****************************************************/

void asysn_audiounit_print_kMusicDeviceReleaseInstrumentSelect
(ComponentParameters * p, ComponentResult returncode)
{
  asysn_audiounit_print_selector_name(p->what);
  fprintf(asysn_audiounit_logfile, "\n\tInstrument ID: %u", 
	  (unsigned int)(p->params[0])); 
  asysn_audiounit_print_nonzero_returncode(returncode);
  asysn_audiounit_print_paramsize_check(p);
  fprintf(asysn_audiounit_logfile, ".\n\n");
}


/****************************************************/
/* kMusicDeviceStartNoteSelect                      */
/*                                                  */
/*  params[0]: MusicDeviceInstrumentID inInstrument */
/*  params[1]: MusicDeviceGroupID inGroupID         */
/*  params[2]: NoteInstanceID * outNoteInstanceID   */
/*  params[3]: UInt32 inOffsetSampleFrame           */
/*  params[4]: MusicDeviceNoteParams * inParams     */
/*          (order may be reversed)                 */
/****************************************************/

void asysn_audiounit_print_kMusicDeviceStartNoteSelect(ComponentParameters * p, 
						       ComponentResult returncode)
{
  asysn_audiounit_print_selector_name(p->what);
  fprintf(asysn_audiounit_logfile, "\n\tInstrument ID: %u", 
	  (unsigned int)(p->params[0])); 
  fprintf(asysn_audiounit_logfile, "\n\tGroup ID: %u", 
	  (unsigned int)(p->params[1])); 
  if (p->params[3])
    fprintf(asysn_audiounit_logfile, "\n\tOffset %u samples from buffer start",
	    (unsigned int)(p->params[3])); 
  asysn_audiounit_print_nonzero_returncode(returncode);
  asysn_audiounit_print_paramsize_check(p);
  fprintf(asysn_audiounit_logfile, ".\n\n");
}


/****************************************************/
/* kMusicDeviceStopNoteSelect                       */
/*                                                  */
/*  params[0]: MusicDeviceGroupID inGroupID         */
/*  params[1]: NoteInstanceID * outNoteInstanceID   */
/*  params[2]: UInt32 inOffsetSampleFrame           */
/*         (order may be reversed)                  */
/****************************************************/

void asysn_audiounit_print_kMusicDeviceStopNoteSelect(ComponentParameters * p, 
						      ComponentResult returncode)
{
  asysn_audiounit_print_selector_name(p->what);
  fprintf(asysn_audiounit_logfile, "\n\tInstrument ID: %u", 
	  (unsigned int)(p->params[0])); 
  if (p->params[3])
    fprintf(asysn_audiounit_logfile, "\n\tOffset %u samples from buffer start",
	    (unsigned int)(p->params[2])); 
  asysn_audiounit_print_nonzero_returncode(returncode);
  asysn_audiounit_print_paramsize_check(p);
  fprintf(asysn_audiounit_logfile, ".\n\n");
}

/*~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~*/
/* The wiretap API calls */
/*~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~*/

#if (ASYS_AUDIOUNIT_DEBUG_LEVEL >= 1)

/**************************************************/
/* entry function for snooping on component calls */
/**************************************************/

void asysn_audiounit_wiretap(ComponentParameters * p, ComponentResult returncode)

{
  if (asysn_audiounit_first_logfile_open) asysn_audiounit_logfile_initialize();
  asysn_audiounit_logfile = fopen(ASYS_AUDIOUNIT_LOGFILE_NAME, "a");

  switch (p->what) { /* jump table to routines for specific selectors */
  case kComponentCanDoSelect:
    asysn_audiounit_print_kComponentCanDoSelect(p, returncode);
    break;
  case kComponentOpenSelect:
    asysn_audiounit_print_kComponentOpenSelect(p, returncode);
    break;
  case kComponentCloseSelect:
    asysn_audiounit_print_kComponentCloseSelect(p, returncode);
    break;
  case kAudioUnitInitializeSelect:
    asysn_audiounit_print_kAudioUnitInitializeSelect(p, returncode);
    break;
  case kAudioUnitUninitializeSelect:
    asysn_audiounit_print_kAudioUnitUninitializeSelect(p, returncode);
    break;
  case kAudioUnitGetPropertyInfoSelect:
    asysn_audiounit_print_kAudioUnitGetPropertyInfoSelect(p, returncode);
    break;
  case kAudioUnitGetPropertySelect:
    asysn_audiounit_print_kAudioUnitGetPropertySelect(p, returncode);
    break;
  case kAudioUnitSetPropertySelect:
    asysn_audiounit_print_kAudioUnitSetPropertySelect(p, returncode);
    break;
  case kAudioUnitAddPropertyListenerSelect:
    asysn_audiounit_print_kAudioUnitAddPropertyListenerSelect(p, returncode);
    break;
  case kAudioUnitRemovePropertyListenerSelect:
    asysn_audiounit_print_kAudioUnitRemovePropertyListenerSelect(p, returncode);
    break;
  case kAudioUnitRemovePropertyListenerWithUserDataSelect:
    asysn_audiounit_print_kAudioUnitRemovePropertyListenerWithUserDataSelect(p, returncode);
    break;
  case kAudioUnitAddRenderNotifySelect:
    asysn_audiounit_print_kAudioUnitAddRenderNotifySelect(p, returncode);
    break;
  case kAudioUnitRemoveRenderNotifySelect:
    asysn_audiounit_print_kAudioUnitRemoveRenderNotifySelect(p, returncode);
    break;
  case kAudioUnitGetParameterSelect:
    asysn_audiounit_print_kAudioUnitGetParameterSelect(p, returncode);
    break;
  case kAudioUnitSetParameterSelect:
    asysn_audiounit_print_kAudioUnitSetParameterSelect(p, returncode);
    break;
  case kAudioUnitScheduleParametersSelect:
    asysn_audiounit_print_kAudioUnitScheduleParametersSelect(p, returncode);
    break;
  case kAudioUnitRenderSelect:
    asysn_audiounit_print_kAudioUnitRenderSelect(p, returncode);
    break;
  case kAudioUnitResetSelect:
    asysn_audiounit_print_kAudioUnitResetSelect(p, returncode);
    break;
  case kMusicDeviceMIDIEventSelect:
    asysn_audiounit_print_kMusicDeviceMIDIEventSelect(p, returncode);
    break;
  case kMusicDeviceSysExSelect:
    asysn_audiounit_print_kMusicDeviceSysExSelect(p, returncode);
    break;
  case kMusicDevicePrepareInstrumentSelect:
    asysn_audiounit_print_kMusicDevicePrepareInstrumentSelect(p, returncode);
    break;
  case kMusicDeviceReleaseInstrumentSelect:
    asysn_audiounit_print_kMusicDeviceReleaseInstrumentSelect(p, returncode);
    break;
  case kMusicDeviceStartNoteSelect:
    asysn_audiounit_print_kMusicDeviceStartNoteSelect(p, returncode);
    break;
  case kMusicDeviceStopNoteSelect:
    asysn_audiounit_print_kMusicDeviceStopNoteSelect(p, returncode);
    break;
  default:  /* the generic selector print routine */
    asysn_audiounit_print_selector_name(p->what);
    asysn_audiounit_print_nonzero_returncode(returncode);
    asysn_audiounit_print_paramsize_check(p);
    fprintf(asysn_audiounit_logfile, ".\n\n");
  }
  fclose(asysn_audiounit_logfile);
}

/*************************************************/
/* entry function for snooping on renderer calls */
/*************************************************/

void asysn_audiounit_wiretap_renderer(void * inComponentStorage,
				      AudioUnitRenderActionFlags * ioActionFlags,
				      const AudioTimeStamp * inTimeStamp,
				      UInt32 inOutputBusNumber,
				      UInt32 inNumberFrames,
				      AudioBufferList * ioData)
{
  int i;

  if (asysn_audiounit_first_logfile_open) asysn_audiounit_logfile_initialize();
  asysn_audiounit_logfile = fopen(ASYS_AUDIOUNIT_LOGFILE_NAME, "a");

  fprintf(asysn_audiounit_logfile, "Renderer report"); 

  if (inTimeStamp->mFlags & kAudioTimeStampSampleTimeValid)
      fprintf(asysn_audiounit_logfile, "\n\tSample time: %lg", 
	      inTimeStamp->mSampleTime); 
  if (inTimeStamp->mFlags & kAudioTimeStampHostTimeValid)
      fprintf(asysn_audiounit_logfile, "\n\tHost time: %llu", 
	      inTimeStamp->mHostTime); 

  fprintf(asysn_audiounit_logfile, "\n\t%u frames on output bus %u", 
	  (unsigned int) inNumberFrames, (unsigned int) inOutputBusNumber); 

  if (*ioActionFlags)
    {
      fprintf(asysn_audiounit_logfile, "\n\tFlags:"); 
      if ((*ioActionFlags) &  kAudioUnitRenderAction_PreRender)
	fprintf(asysn_audiounit_logfile, " PreRender"); 
      if ((*ioActionFlags) &  kAudioUnitRenderAction_PostRender)
	fprintf(asysn_audiounit_logfile, " PostRender"); 
      if ((*ioActionFlags) &  kAudioUnitRenderAction_OutputIsSilence)
	fprintf(asysn_audiounit_logfile, " OutputIsSilence"); 
      if ((*ioActionFlags) & kAudioOfflineUnitRenderAction_Preflight)
	fprintf(asysn_audiounit_logfile, " Preflight");
      if ((*ioActionFlags) & kAudioOfflineUnitRenderAction_Render)
	fprintf(asysn_audiounit_logfile, " Render");
      if ((*ioActionFlags) &kAudioOfflineUnitRenderAction_Complete)
	fprintf(asysn_audiounit_logfile, " Complete");
    }

  fprintf(asysn_audiounit_logfile, "\n\tmNumberBuffers: %u", 
	  (unsigned int) ioData->mNumberBuffers);

  for (i = 0; i < ioData->mNumberBuffers; i++)
  fprintf(asysn_audiounit_logfile, 
	  "\n\t\tBuffer %u has %u channel(s) of %u-ptr %u-byte waveforms", 
	  i, (unsigned int) ioData->mBuffers[i].mNumberChannels, 
	  (unsigned int)(ioData->mBuffers[i].mData),
	  (unsigned int)ioData->mBuffers[i].mDataByteSize);

  fprintf(asysn_audiounit_logfile, "\n"); 
  fclose(asysn_audiounit_logfile);
}


/**************************************************/
/* entry function for snooping on midievent calls */
/**************************************************/

void asysn_audiounit_wiretap_midievent(void * inComponentStorage,
				       UInt32 inStatus,
				       UInt32 inData1,
				       UInt32 inData2,
				       UInt32 inOffsetSampleFrame, 
				       char * type)

{
  if (asysn_audiounit_first_logfile_open) asysn_audiounit_logfile_initialize();
  asysn_audiounit_logfile = fopen(ASYS_AUDIOUNIT_LOGFILE_NAME, "a");

  fprintf(asysn_audiounit_logfile, "%s MIDI Event", type);
  asysn_audiounit_print_midievent((unsigned char)(inStatus), (unsigned char)(inData1),
		  (unsigned char)(inData2));
  if (inOffsetSampleFrame)
    fprintf(asysn_audiounit_logfile, "\n\tOffset %u samples from buffer start",
	    (unsigned int) inOffsetSampleFrame);
 
  fprintf(asysn_audiounit_logfile, "\n\n"); 
  fclose(asysn_audiounit_logfile);
}

/*****************************************************/
/* entry function for snooping on getparameter calls */
/*****************************************************/

void asysn_audiounit_wiretap_getparameter(void * inComponentStorage,
					  AudioUnitParameterID inID,
					  AudioUnitScope inScope,
					  AudioUnitElement inElement,
					  Float32 * outValue,
					  char * type)
     
{
  if (asysn_audiounit_first_logfile_open) asysn_audiounit_logfile_initialize();
  asysn_audiounit_logfile = fopen(ASYS_AUDIOUNIT_LOGFILE_NAME, "a");

  fprintf(asysn_audiounit_logfile, "%s GetParameter Event", type);

  fprintf(asysn_audiounit_logfile, "\n\tParameter "); 
  asysn_audiounit_print_parameter_name(inID);

  fprintf(asysn_audiounit_logfile, " value is %g", *outValue);

  fprintf(asysn_audiounit_logfile, "\n\t");
 
  asysn_audiounit_print_scope_name(inScope);
  fprintf(asysn_audiounit_logfile, ", element %u", 
	  (unsigned int) inElement); 
 
  fprintf(asysn_audiounit_logfile, "\n\n"); 
  fclose(asysn_audiounit_logfile);
}

/*****************************************************/
/* entry function for snooping on setparameter calls */
/*****************************************************/

void asysn_audiounit_wiretap_setparameter(void * inComponentStorage,
					  AudioUnitParameterID inID,
					  AudioUnitScope inScope,
					  AudioUnitElement inElement,
					  Float32 inValue,
					  UInt32 inBufferOffsetInFrames,
					  char * type)
     
{
  if (asysn_audiounit_first_logfile_open) asysn_audiounit_logfile_initialize();
  asysn_audiounit_logfile = fopen(ASYS_AUDIOUNIT_LOGFILE_NAME, "a");

  fprintf(asysn_audiounit_logfile, "%s SetParameter Event", type);

  fprintf(asysn_audiounit_logfile, "\n\tParameter "); 
  asysn_audiounit_print_parameter_name(inID);

  fprintf(asysn_audiounit_logfile, " value is %g", inValue);

  if (inBufferOffsetInFrames)
    fprintf(asysn_audiounit_logfile, " (frame offset %u)", 
	    (unsigned int) inBufferOffsetInFrames);
  fprintf(asysn_audiounit_logfile, "\n\t");
 
  asysn_audiounit_print_scope_name(inScope);
  fprintf(asysn_audiounit_logfile, ", element %u", 
	  (unsigned int) inElement); 
 
  fprintf(asysn_audiounit_logfile, "\n\n"); 
  fclose(asysn_audiounit_logfile);
}

/******************************************/
/* prints arbitrary string to the logfile */
/******************************************/

void asysn_audiounit_wiretap_putstring(char * s)

{
  if (asysn_audiounit_first_logfile_open) asysn_audiounit_logfile_initialize();
  asysn_audiounit_logfile = fopen(ASYS_AUDIOUNIT_LOGFILE_NAME, "a");
  fprintf(asysn_audiounit_logfile, "%s", s); 
  fclose(asysn_audiounit_logfile);
}

/*************************************/
/* prints out a ComponentResult name */
/*************************************/

void asysn_audiounit_wiretap_print_component_result(ComponentResult returncode)

{
  if (asysn_audiounit_first_logfile_open) asysn_audiounit_logfile_initialize();
  asysn_audiounit_logfile = fopen(ASYS_AUDIOUNIT_LOGFILE_NAME, "a");
  asysn_audiounit_print_returncode_name(returncode);
  fclose(asysn_audiounit_logfile);
}

#endif  /* (ASYS_AUDIOUNIT_DEBUG_LEVEL >= 1) */


/****************************************************************/
/*     End of wiretap logger for audiounit driver for sfront    */
/****************************************************************/
/****************************************************************/

