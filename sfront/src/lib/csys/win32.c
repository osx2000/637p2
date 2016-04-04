/*
#    Sfront, a SAOL to C translator    
#    This file: Win32 Directsound MIDI IN soundcard jack driver for sfront
#
# Copyright (c) 1999-2006, Regents of the University of California
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
#    Author: Professor John Wawrzynek, UCB CS Division
#    Maintainance Email To: John Lazzaro, lazzaro@cs.berkeley.edu
#
*/


/*
 * WIN32 midi driver for sfront.
 *
 */

#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>

#include <stdio.h>


/*
 * Globals
 */
#define  CSYSI_MIDIBUFSIZ 256

// midi related

DWORD    csysi_midiBufA[CSYSI_MIDIBUFSIZ];
DWORD    csysi_midiBufB[CSYSI_MIDIBUFSIZ];

DWORD * csysi_midiWrite;
DWORD * csysi_bufWrPtr;

DWORD * csysi_bufRdPtr;
DWORD * csysi_midiRdEnd;

HMIDIIN csysi_hMidiIn;

// console stuff for graceful exit

HANDLE csysi_hStdin;
INPUT_RECORD csysi_irInBuf[1];
DWORD csysi_cNumRead, csysi_fdwSaveOldMode;
BOOL csysi_exitFlag;



/****************************************************************/
/*             Callback routine for incoming events             */
/****************************************************************/

void CALLBACK csysi_MidiInProc(HMIDIIN hMidiIn, UINT wMsg, DWORD dwInstance,
                         DWORD dwParam1, DWORD dwParam2)
{
  if (csysi_bufWrPtr==csysi_midiWrite+CSYSI_MIDIBUFSIZ)
    {
      fprintf(stderr, "Midi input buffer overflow, dropping event.\n");
      return;
    }
  *csysi_bufWrPtr++ = dwParam1;
}

/****************************************************************/
/*             initialization routine for control               */
/****************************************************************/

int csys_setup(void)
{
  csysi_midiWrite = csysi_bufWrPtr = &csysi_midiBufA;

  // open first available midi input device

  if (midiInOpen(&csysi_hMidiIn, (UINT) 0, (DWORD) 
                 &csysi_MidiInProc, 0L, (DWORD) CALLBACK_FUNCTION))
    {
      fprintf(stderr, "MidiInOpen error.\n");
      return CSYS_ERROR;
    }

  if (midiInStart(csysi_hMidiIn))
    {
      fprintf(stderr, "MidiInStart error.\n");
      return CSYS_ERROR;
    }

  // Console

  csysi_hStdin = GetStdHandle(STD_INPUT_HANDLE);
  if (csysi_hStdin == INVALID_HANDLE_VALUE)
    {
      fprintf(stderr, "GetStdHandle error.\n");
      exit(-1);
    }

  if (! GetConsoleMode(csysi_hStdin, &csysi_fdwSaveOldMode) )
    {
      fprintf(stderr, "GetConsoleMode error.\n");
      exit(-1);
    }

  if (! SetConsoleMode(csysi_hStdin, ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT)
)
    {
      fprintf(stderr, "GetConsoleMode error.\n");
      exit(-1);
    }

  csysi_cNumRead = 0;
  csysi_exitFlag = FALSE;

  return CSYS_DONE;
}

/****************************************************************/
/*             polling routine for new data                     */
/****************************************************************/

int csys_newdata(void)
     
{
  // check for console input
  PeekConsoleInput(csysi_hStdin, csysi_irInBuf, 1, &csysi_cNumRead);
  if (csysi_cNumRead>0) 
    {
      csysi_exitFlag = TRUE;
      fprintf(stderr, "exiting ... \n");
      return CSYS_MIDIEVENTS;
    }

  // now midi stuff
  csysi_bufRdPtr = csysi_midiWrite;
  csysi_midiRdEnd = csysi_bufWrPtr; 

  // An input event RIGHT HERE will be lost!

  // switch write buffer (avoid locking out incoming)
  if (csysi_midiWrite==csysi_midiBufA) 
    csysi_bufWrPtr = csysi_midiWrite = csysi_midiBufB;
  else 
      csysi_bufWrPtr = csysi_midiWrite = csysi_midiBufA;

  if (csysi_midiRdEnd==csysi_bufRdPtr)
    return CSYS_NONE;
  else
    return CSYS_MIDIEVENTS;
}

/****************************************************************/
/*                 processes a MIDI event                       */
/****************************************************************/

int csys_midievent(unsigned char * cmd,   unsigned char * ndata, 
                   unsigned char * vdata, unsigned short * extchan,
                   float * fval)
{
  DWORD msg;

  // keyboard exit
  if (csysi_exitFlag)
    {
      *cmd = CSYS_MIDI_ENDTIME;
      *fval = EV(scorebeats);
      return CSYS_NONE;
    }

  // unpack event
  msg = *csysi_bufRdPtr++;
  *cmd = 0xff & msg;
  *ndata = (0xff00 & msg) >> 8;
  *vdata = (0xff0000 & msg) >> 16;
  *extchan = 0;

  if (csysi_midiRdEnd==csysi_bufRdPtr)
    return CSYS_NONE;
  else
    return CSYS_MIDIEVENTS;
}

/****************************************************************/
/*                  closing routine for control                 */
/****************************************************************/

void csys_shutdown(void)
     
{
  // restore console
  if (! SetConsoleMode(csysi_hStdin, csysi_fdwSaveOldMode) )
    {
      fprintf(stderr, "GetConsoleMode error.\n");
      exit(-1);
    }

  //close midi
  if (midiInClose(csysi_hMidiIn))
    {
      fprintf(stderr, "MidiInClose error.\n");
    }
  return;
}

