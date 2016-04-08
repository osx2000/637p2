
///////////////////////////////////////////////////////////////////////////////
//
//    SAOL Resonator-Based Physical Model Library
//    This file: Common to all resonators
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
//    Maintainer: John Lazzaro, lazzaro@cs.berkeley.edu
//
//////////////////////////////////////////////////////////////////////////

//
//    Original Author: John Wawrzynek
//    Maintainer: John Lazzaro, lazzaro@cs.berkeley.edu
//
///////////////////////////////////////////////////////////////////////////////

#include <reson/macros.hs>

///////////////////////////////////////////////////////////////////////////////
//
// Distributed global block
//

global {

  // vel table holds status for each note
  //
  //  -1  -- no instr active for this note
  //   0  -- instr active, no new strikes
  // > 0  -- new note strike, value of MIDIvel

  table RESON_SC(vel)(step, MIDI_MSBSHIFT, 0, -1, MIDI_MSBSHIFT);

  ksig RESON_SC(poly);               // number of active notes at once

  sequence(RESON_SC(kbd), RESON_SC(audio));
}

//
///////////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////////
//
//
// k-pass semantics for the striker model
//

kopcode RESON_SC(strikeupdate)(ksig ky[1], ksig nm, ksig silent, 
	                 ksig notenum, ivar vw, ivar vwn)

{
  imports exports table RESON_SC(vel);
  imports exports ksig RESON_SC(poly);
  ksig exit, count;

  count = silent ? (count + 1) : max(count - 1, 0);
  if (((count > 5) && (itime > 0.25)) || exit)
    {
      if (!exit)
	{
	  turnoff;
	  exit = 1;
	  tablewrite(RESON_SC(vel), int(notenum), -1);
	  RESON_SC(poly) = RESON_SC(poly) - 1;
	}
      ky[0] = 0;
    }
  else
    {
      if (tableread(RESON_SC(vel), int(notenum)) > 0)
	{
	  ky[0] = vw*tableread(RESON_SC(vel), int(notenum));
	  nm = ky[0]*vwn;
	  tablewrite(RESON_SC(vel), int(notenum), 0);
	}
      else
	{
	  ky[0] = 0;
	}
    }
}

//
//
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
//
// instrs used for audio output and note creation 
//
///////////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////////
//
// Instr for creating audio output. Each active note has its own
// instance of RESON_SC(audio).
//

instr RESON_SC(audio)(notenum) {

  ivar a[RESON_RESNUM], b[RESON_RESNUM], g[RESON_RESNUM];
  ivar aa, ab, sg, vw, vwn;
  ksig nm, ky[1], silent;
  asig out;

  asig y[RESON_RESNUM], y1[RESON_RESNUM], y2[RESON_RESNUM]; 
  asig sy[RESON_RESNUM];
  asig ay, ay1, ay2, dummy, x;


  ///////////////////////////
  // happens at i-rate
  ///////////////////////////

  RESON_SC(resinit)(a, b, g, notenum);
  RESON_SC(strikeinit)(aa, ab, sg, vw, vwn, notenum);

  ///////////////////////////
  // happens at k-rate
  ///////////////////////////

  silent = (rms(out) < RESON_INAUDIBLE);
  RESON_SC(strikeupdate)(ky, nm, silent, notenum, vw, vwn);

  ///////////////////////////
  // happens at a-rate
  ///////////////////////////

  dummy = 0;                          // until optimizer improves

  ay = aa*ay1 + ab*ay2 + ky[dummy];   // attack resonator
  x = (arand(nm) + sg)*ay;

  y = a*y1 + b*y2 + x;                // resonator bank

  ay2 = ay1;                          // update filter state
  ay1 = (abs(ay)>1e-30) ? ay : 0.0; 

  ky[dummy] = 0;
  y2 = y1; 
  y1 = y;

  sy = g*y;                           // gain adjust
  out = RESON_SUMOUT;                 // sum over sy[]

  output(out);

}


///////////////////////////////////////////////////////////////////////////////
//
// Instr for handling MIDI or SASL control input. Primary duty is to
// update the global RESON_SC(vel)[] array.
//

instr RESON_SC(kbd)(pitch, velocity) RESON_PRESET 

{ 
  imports exports table RESON_SC(vel);
  imports exports ksig RESON_SC(poly);
  ksig vval, kpitch;

  ///////////////////////////
  // happens at k-rate
  ///////////////////////////

  vval = velocity;
  kpitch = pitch;
  if (tableread(RESON_SC(vel), int(kpitch)) == -1)
    {
      if (RESON_SC(poly) < RESON_POLYPHONY)
	{
	  tablewrite(RESON_SC(vel), int(pitch), vval);
	  instr RESON_SC(audio)(0, -1, pitch);
	  RESON_SC(poly) = RESON_SC(poly) + 1;
	}
    }
  else
    {
      tablewrite(RESON_SC(vel), int(pitch), vval);
    }

  turnoff;

}


#undef RESON_SUMOUT



