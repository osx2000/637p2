
///////////////////////////////////////////////////////////////////////////////
//
//    SAOL Resonator-Based Physical Model Library
//    This file: Aluminum bars
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
//     #define ABAR_MIDIPRESET  30
//
//     If ABAR_MIDIPRESET is not set, it preset number of 30 is 
//     chosen. To specify that the instrument is not a MIDI 
//     instrument, define ABAR_NOMIDIPRESET
//
// [2] Include the library, as
//
//     #include <reson/abar.hs>
//
// [3] The instrument abar_audio generates the audio output for
//     the instrument: use this name in route statements. 
//
// [4] The instrument abar_kbd handles new note generation. For
//     MIDI control, just set MIDI NoteOn commands to the preset
//     selected by ABAR_MIDIPRESET. For SASL or dynamic control, 
//     call this instr:
//     
//     instr abar_kbd(pitch, velocity)
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
//     ABAR_POLYPHONY          The maximum number of notes at
//                             one time. Note that since the
//                             instrument models the bar itself,
//                             any number of hits on a single bar
//                             counts as one note. Typical numbers
//                             for real-time performance on a 450MHz
//                             PIII is 2-3.
//
//     ABAR_INAUDIBLE          A constant that codes the softest sound
//                             that is audible. Higher values release
//                             slots for new notes sooner, but clip off
//                             the end of the ringing. A good starting
//                             point is 1e-4.
//
//     There parameter are set in a manner identical to 
//     ABAR_MIDIPRESET as shown in [1].
//     
///////////////////////////////////////////////////////////////////////////////

#ifndef RESON_ABAR
#define RESON_ABAR

#include <Slib/ssm.hs>
#include <Slib/std.hs>


///////////////////////////////////////////////////////////////////////////////
//

//
// Pre-processor defines for user preferences
//
//

// parses user preferences

#if ABAR_NOMIDIPRESET
#define RESON_PRESET
#else
#ifdef ABAR_MIDIPRESET
#define RESON_PRESET preset ABAR_MIDIPRESET
#else
#define RESON_PRESET preset 0 
#endif
#endif

// Inaudible rms value

#ifdef ABAR_INAUDIBLE
#define RESON_INAUDIBLE ABAR_INAUDIBLE
#else
#define RESON_INAUDIBLE 1e-5
#endif

// Number of simultaneous notes

#ifdef ABAR_POLYPHONY
#define RESON_POLYPHONY ABAR_POLYPHONY 
#else
#define RESON_POLYPHONY 128 
#endif

//
// Pre-processor defines for internal constants
//

//
// Macros that generates unique global names

#define RESON_SC(x) abar_ ## x

// Number of resonators in the model

#define RESON_RESNUM 10

//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
// Distributed global block for scaling prototype bar
//

global {

  // hand-tuned version of qscale = 0.0301937*exp(0.058521*x) for q scaling

  table RESON_SC(qscale)(expseg, -1, 
			 0,  0.0301937, 
                        128, 0.0301937*exp(0.058521*128));


  // implements gscale = 0.4*(1/492.372)*exp(0.0540561*x) for g scaling
  
  table RESON_SC(gscale)(expseg, -1, 0, 0.4*(1/492.372), 128,
			 0.4*(1/492.372)*exp(0.0540561*128));
			 
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

  norm = tableread(RESON_SC(qscale), int(notenum));
  scale = cpsmidi(notenum)/(CPS_MIDDLEC*1.02973);

  freq[0] = 1077*scale;  q[0] = 2000*norm;
  freq[1] = 2160*scale;  q[1] =  500*norm;
  freq[2] = 2940*scale;  q[2] =  500*norm;
  freq[3] = 3220*scale;  q[3] =  500*norm;
  freq[4] = 3520*scale;  q[4] =  500*norm;
  freq[5] = 3940*scale;  q[5] = 2000*norm;
  freq[6] = 5400*scale;  q[6] =  500*norm;
  freq[7] = 5680*scale;  q[7] = 2000*norm;
  freq[8] = 6900*scale;  q[8] = 2000*norm;
  freq[9] = 7840*scale;  q[9] =  500*norm;

  norm = tableread(RESON_SC(gscale), int(notenum));

  g[0] = (freq[0] < s_rate/2) ? 1.0*norm : 0.0;
  g[1] = (freq[1] < s_rate/2) ? 0.7*norm : 0.0;
  g[2] = (freq[2] < s_rate/2) ? 0.7*norm : 0.0;
  g[3] = (freq[3] < s_rate/2) ? 0.6*norm : 0.0;
  g[4] = (freq[4] < s_rate/2) ? 0.4*norm : 0.0;
  g[5] = (freq[5] < s_rate/2) ? 0.4*norm : 0.0;
  g[6] = (freq[6] < s_rate/2) ? 0.3*norm : 0.0;
  g[7] = (freq[7] < s_rate/2) ? 1.0*norm : 0.0;
  g[8] = (freq[8] < s_rate/2) ? 1.0*norm : 0.0;
  g[9] = (freq[9] < s_rate/2) ? 1.0*norm : 0.0;

  // compute actual resonator coefficients

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

  afreq = 2000;  // attack resonator frequency 

  // Compute resonator bank coefficients

  ar = exp(-2*PI*(afreq/s_rate));
  aa = 2*ar;
  ab = -ar*ar;

  vw = MIDI_SCALE; // no keyboard normalization curve
  sg = 0.004;      // "signal gain" empirical constant (should not scale).
  vwn = 0.04;      //  velocity scaling for nm

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
