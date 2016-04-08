
///////////////////////////////////////////////////////////////////////////////
//
//    SAOL Resonator-Based Physical Model Library
//    This file: Marimba bars
//
// Copyright (c) 1999-2006, Regents of the University of California
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//  Redistributions of source code must retain the above copyright
//  notice, this list of conditions and the following disclaimer.
//
//  Redistributions in binary form must reproduce the above copyright
//  notice, this list of conditions and the following disclaimer in the
//  documentation and/or other materials provided with the distribution.
//
//  Neither the name of the University of California, Berkeley nor the
//  names of its contributors may be used to endorse or promote products
//  derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//    Original Author: John Wawrzynek
//    Maintainer: John Lazzaro, lazzaro@cs.berkeley.edu
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
// To use library in your SAOL program:
//
// [1] If you want to choose a MIDI preset number (say, 30):
//     
//     #define MARIMBA_MIDIPRESET  30
//
//     If MARIMBA_MIDIPRESET is not set, it preset number of 30 is 
//     chosen. To specify that the instrument is not a MIDI 
//     instrument, define MARIMBA_NOMIDIPRESET
//
// [2] Include the library, as
//
//     #include <reson/marimba.hs>
//
// [3] The instrument marimba_audio generates the audio output for
//     the instrument: use this name in route statements. 
//
// [4] The instrument marimba_kbd handles new note generation. For
//     MIDI control, just set MIDI NoteOn commands to the preset
//     selected by MARIMBA_MIDIPRESET. For SASL or dynamic control, 
//     call this instr:
//     
//     instr marimba_kbd(pitch, velocity)
//
//     Where pitch is a MIDI note numbers, and velocity is a 
//     MIDI note-on velocity value (range 0-127). This instr
//     should be called with duration of -1. 
//
// [5] The instr doesn't set a srate or krate, so you can do
//     it in your own global block. These parameters are 
//     recommended for sfront real-time control:
//
//     srate 44100;
//     krate 1000;
//
//
// [6] Two optional parameters set the real-time polyphonic
//     limits of the instrument. If these aren't defined by 
//     the user, the defaults are set to be good for rendering
//     arbitrary polyphony in an accurate way. 
//
//     MARIMBA_POLYPHONY       The maximum number of notes at
//                             one time. Note that since the
//                             instrument models the bar itself,
//                             any number of hits on a single bar
//                             counts as one note. Typical numbers
//                             for real-time performance on a 450MHz
//                             PIII is 2-3.
//
//     MARIMBA_INAUDIBLE       A constant that codes the softest sound
//                             that is audible. Higher values release
//                             slots for new notes sooner, but clip off
//                             the end of the ringing. A good starting
//                             point is 1e-4.
//
//     There parameter are set in a manner identical to 
//     MARIMBA_MIDIPRESET as shown in [1].
//     
///////////////////////////////////////////////////////////////////////////////

#ifndef RESON_MARIMBA
#define RESON_MARIMBA

#include <Slib/ssm.hs>
#include <Slib/std.hs>

///////////////////////////////////////////////////////////////////////////////
//

//
// Pre-processor defines for user preferences
//
//

// parses user preferences

#if MARIMBA_NOMIDIPRESET
#define RESON_PRESET
#else
#ifdef MARIMBA_MIDIPRESET
#define RESON_PRESET preset MARIMBA_MIDIPRESET
#else
#define RESON_PRESET preset 0 
#endif
#endif

// Inaudible rms value

#ifdef MARIMBA_INAUDIBLE
#define RESON_INAUDIBLE MARIMBA_INAUDIBLE
#else
#define RESON_INAUDIBLE 1e-5
#endif

// Number of simultaneous notes

#ifdef MARIMBA_POLYPHONY
#define RESON_POLYPHONY MARIMBA_POLYPHONY 
#else
#define RESON_POLYPHONY 128 
#endif

//
// Pre-processor defines for internal constants
//

//
// Macros that generates unique global names

#define RESON_SC(x) marimba_ ## x

// Number of resonators in the model

#define RESON_RESNUM 3


//
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
//
// Distributed global block for scaling prototype bar
//

global {

  // hand-tuned version of qscale = 0.0551223*exp(0.0579087*x) for q scaling

  table RESON_SC(qscale)(expseg, -1, 0, 0.5*0.0551223, 
	                         128, 0.5*0.0551223*exp(0.0579087*128));

  // implements gscale = 0.004*(1/223.066)*exp(0.0813501*x) for g scaling
  
  table RESON_SC(gscale)(expseg, -1, 0,  0.004*(1/223.066), 128,
			 0.004*(1/223.066)*exp(0.0813501*128));
			 
}

///////////////////////////////////////////////////////////////////////////////
//
// The resinit iopcode initializes the resonance model for the thing being
// struck or plucked.
//


iopcode RESON_SC(resinit) (ivar a[RESON_RESNUM], ivar b[RESON_RESNUM], 
		ivar g[RESON_RESNUM], ivar notenum)

{
  ivar r[RESON_RESNUM], freq[RESON_RESNUM], q[RESON_RESNUM];
  ivar j, scale, norm;
  imports exports table RESON_SC(qscale);
  imports exports table RESON_SC(gscale);

  // set f/q/g for prototype bar

  norm = tableread(RESON_SC(qscale), int(notenum + 12));	
  scale = cpsmidi(notenum + 12)/CPS_MIDDLEC;

  freq[0] =  261.63*scale; q[0] = 240*norm;  
  freq[1] = 1041.29*scale; q[1] = 200*norm;  
  freq[2] = 2616.30*scale; q[2] = 150*norm;  

  norm = tableread(RESON_SC(gscale), int(notenum+12));

  g[0] = (freq[0] < s_rate/2) ? norm : 0.0;
  g[1] = (freq[1] < s_rate/2) ? norm : 0.0;
  g[2] = (freq[2] < s_rate/2) ? norm : 0.0;

  // Compute actual resonator coefficients
  //
  // (Doesn't need changing for new models).

  j = 0;
  while ( j < RESON_RESNUM)
    {
      r[j] = exp(-freq[j]/(s_rate*q[j]));
      a[j] = 2*r[j]*cos(2*PI*(freq[j]/s_rate));
      b[j] = - r[j]*r[j];
      j = j + 1;
    }

}

//
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
//
// The strikeinit iopcode initializes the striker model.


iopcode RESON_SC(strikeinit)(ivar aa, ivar ab, ivar sg, ivar vw, 
			     ivar vwn, ivar notenum)

{
  ivar ar, afreq;

  afreq = 261.63;  // attack resonator frequency 

  // Compute resonator bank coefficients

  ar = exp(-2*PI*(afreq/s_rate));
  aa = 2*ar;
  ab = -ar*ar;

  vw = MIDI_SCALE; // keyboard normalization curve
  sg = 0.0025;     // "signal gain" empirical constant (should not scale).
  vwn = 0.01;      //  velocity scaling for nm
}

//
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
//
// all code common to the structured resonator family 

#include <reson/sreson.hs>


#undef RESON_RESNUM 
#undef RESON_INAUDIBLE 
#undef RESON_POLYPHONY 
#undef RESON_PRESET
#undef RESON_SC

//     
///////////////////////////////////////////////////////////////////////////////

#endif

