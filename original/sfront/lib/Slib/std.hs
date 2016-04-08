
//////////////////////////////////////////////////////////////////////////
//
//   Slib, Sfront's SAOL library
//   This file: The std library (general-purpose library elements)
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


//////////////////////////////////////////////////////////////////////////
//                                                
//
// This library is a collection of constants, macros, and other items that
// are generally useful for SAOL programming. A brief description of each
// item is below, sorted by category.
//
// Temporal constants
// ------------------
//
// Name        Description                               Equation
//
// ACYCLE      Number of a-passes in an execution cycle  [int(s_rate/k_rate)]
// ARATE       Audio sample rate                         [s_rate Hz]
// ATIME       Audio sampling period                     [1/s_rate seconds]
// KRATE       Control sampling rate                     [k_rate Hz]
// KTIME       Control sampling period                   [1/k_rate seconds]
// SRATE       Audio sample rate                         [s_rate Hz]
// STIME       Audio sample period                       [1/s_rate seconds]
//
//
// Tempo constants
// ---------------
//
// Name              Description                   Value
//           
// INIT_TEMPO        Initial tempo for MP4-SA      60   beats/second
// INIT_INVTEMPO     1/INIT_TEMPO                  1/60 seconds/beat
//
//
// Math Constants
// --------------
//
// Name              Description                   Value
//
// M_PI and PI       Trigonometric constant        3.141 ...
// M_E               e (Euler's constant)          2.71 ...
//
//
//
// The NONE Constant 
// -----------------
//
// The C language uses the null pointer 0 as a special value, and the
// symbol constant NULL is used in place of 0 in code to denote its use. 
// Likewise, SAOL and SASL use the integral value -1 to indicate a 
// special value in many contexts, and so we define NONE to have the 
// value -1, and encourage its use in the following contexts instead
// of the literal -1:
//
// ***   For the size parameter in wavetable generators, to indicate
//       that the generator should compute the size of the table.
//
// ***   For the duration parameter in SAOL and SASL instr statements,
//       to indicate indefinite duration.
//
// ***   The loops parameter in the oscil and koscil core opcodes.
//
// ***   The nharm parameter in the buzz core opcode and core wavetable
//       wavetable generator.
//
// Note that the fft and ifft are exceptional, and use 0 (not -1) to
// code special semantics for the len, shift, and size parameters. So
// be careful not to use NONE in fft and ifft opcode calls.
//
//
// Pitch Representations
// ---------------------
//
// SAOL supports four different representations of pitch (cps, midi,
// pch, oct) with a set of core opcodes to convert between them. The
// following constants let you specify constant notes in each 
// representation in a familiar form:
//
//
// Name              Description                   Value
//
// CPS_MIDDLEC      Middle C in cps notation       261.6256     
// MIDI_MIDDLEC     Middle C in midi notation      60
// PCH_MIDDLEC      Middle C in pch notation       8
// OCT_MIDDLEC      Middle C in oct notation       8
//
// CPS_CONCERTA     Concert A in cps notation      440
// MIDI_CONCERTA    Concert A in midi notation     69
// PCH_CONCERTA     Concert A in pch notation      8.9
// OCT_CONCERTA     Concert A in oct notation      8.75
//
// 
// These macros take a parameter, the number of semitones away from
// Middle C, and compute the cps, midi, pch, or oct value. Semitones
// may be a positive or negative integral value. 
//
// Name                   Description
//
// CPS_SEMITONES(x)      CPS  value x semitones away Middle C
// MIDI_SEMITONES(x)     MIDI value x semitones away Middle C
// PCH_SEMITONES(x)      PCH  value x semitones away Middle C
// OCT_SEMITONES(x)      OCT  value x semitones away Middle C
//
//
// This constant is a useful argument to the semitone macros:
//
// Name              Description                       Value
//
// OCTAVESTEPS       Number of semitones per octave      12
//
//
//
// MIDI number scaling
// -------------------
//
// See the ssm.h library for mnemonic scaled, smoothed replacements
// for MIDIctrl[], MIDIbend, and MIDItouch standard names. The 
// constants in this section are for computing on the (7-bit) velocity 
// and note-number values, and for creating alternatives to the ssm.h
// library.
//
// Name           Description                                 Value
//
// MIDI_MAX       Largest value for 7-bit MIDI numbers.       127
// MIDI_SCALE     To scale 7-bit MIDI into [0, 1]             1/127
// MIDI_NULL      The zero value for bipolar 7-bit MIDI       64
// MIDI_SSCALE    Use with MIDI_NULL for [-1, 1] scaling      1/64
//
// MIDI_BIGMAX    Largest value for 14-bit MIDI numbers,      16383
//                used by MIDIbend, and coded by two 
//                MIDIctrl[] entries byr some controllers.
// MIDI_BIGSCALE  To scale 14-bit MIDI into [0, 1]            1/16383
// MIDI_BIGNULL   The zero value for bipolar 14-bit MIDI      8192
// MIDI_BIGSSCALE Use with MIDI_BIGNULL for [-1, 1] scaling   1/8192
// MIDI_MSBSHIFT  Multiply MSB's of MIDIctrl[] by this        128
//                value, and add to LSB to get 14-bit MIDI
// 
// MIDI_OFF       For binary MIDIctrl[] entries. Values       63
//                greater than MIDI_OFF are 1, else 0.
// 
//
//
// Symbolic constants for the fracdelay() core opcode
// ------------------------------------------------
//
//   aopcode fracdelay(ksig method, xsig p1, xsig p2) 
// 
//
// This core opcode, which is usually used via a sequence of oparray
// calls, has different semantics for different integral values of
// the method parameter. The constants in this section are the 
// supported methods.
//
//   Name        Value     Description                
//
//
//  FRAC_INIT      1       Initializes delay line structure. p1 is the
//                         length of the delay, in seconds. Returns 0.
//
//  FRAC_TAP       2       Returns data from the delay line. p1 is the 
//                         position to read, in seconds. If p1 does not
//                         correspond to an integral delay line position,
//                         return value is interpolated.
//
//  FRAC_SET       3       Sets the delay line position p1 to value p2.
//                         p1 is truncated to an integral delay line 
//                         position. Returns 0.
//
//  FRAC_SUM       4       Sums the value p2 into delay line position p1.
//                         p1 is truncated to an integral delay line
//                         position. Returns new value of delay line 
//                         position that is updated.
//
//  FRAC_SHIFT     5       Shifts delay line by 1. Shifts a zero into
//                         the delay line, returns value shifted off the
//                         end of the delay line.
//
//
//
//  Symbolic constants for the random core wavetable generator
//  ----------------------------------------------------------
//
//    table t(random, size, dist, p1 [,p2])
//
//  This core wavetable generator creates a table of length size
//  filled with random numbers. The distribution of the random numbers
//  depends on the integral value of the dist parameter. The constants
//  in this section are the supported distributions.
//
//
//   Name           Value     Description                
//
//  RANDOM_UNIFORM    1       Uniform distribution over [p1, p2]
//
//  RANDOM_LINEAR     2       Linearly ramped distribution from p1 to p2
//
//  RANDOM_EXPON      3       Poisson distribution: p(x) =  p1*exp(-p1*x)
//
//  RANDOM_GAUSSIAN   4       Gaussian distribution: mean p1 and variance p2
//
//  RANDOM_PROCESS    5       Table holds a binary sequence (0.0 and 1.0) that
//                            is generated by a Poisson process specified
//                            by the formula  p(x) =  p1*exp(-p1*x)
//
//
//  Symbolic constants for the window core wavetable generator
//  ----------------------------------------------------------
//
//    table t(window, size, type [,p])
//
//  This core wavetable generator creates a table of length size
//  holding a windowing function. The type of window function created
//  depends on the integral value of the type parameter. The constants
//  in this section are the supported windows.
//
//
//   Name             Value     Window type
//
//   WINDOW_HAMMING     1       Hamming  
//   WINDOW_HANNING     2       Hanning  (raise cosine)
//   WINDOW_BARTLETT    3       Bartlett (triangular)
//   WINDOW_GAUSSIAN    4       Gaussian
//   WINDOW_KAISER      5       Kaiser, with parameter p
//   WINDOW_BOXCAR      6       Boxcar
//
//


/////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////
//                                                
// library definitions begin here
//

#ifndef SLIB_STD
#define SLIB_STD

#define ACYCLE (int(s_rate/k_rate))
#define ARATE  (s_rate)
#define ATIME  (1/s_rate)
#define KRATE  (k_rate)
#define KTIME  (1/k_rate)
#define SRATE  (s_rate)
#define STIME  (1/s_rate)

#define INIT_TEMPO     60
#define INIT_INVTEMPO  (1/60)

#define PI             3.14159265358979323846
#define M_PI           3.14159265358979323846
#define M_E            2.7182818284590452354

#define NONE           -1

#define  CPS_MIDDLEC     261.6256     
#define  MIDI_MIDDLEC    60
#define  PCH_MIDDLEC     8
#define  OCT_MIDDLEC     8

#define  CPS_CONCERTA    440
#define  MIDI_CONCERTA    69
#define  PCH_CONCERTA    8.9
#define  OCT_CONCERTA   8.75

#define MIDI_SEMITONES(x) (x + 60)
#define CPS_SEMITONES(x)  (cpsmidi(x + 60))
#define OCT_SEMITONES(x)  ((x + 96)/12)
#define PCH_SEMITONES(x)  (pchmidi(x + 60))
#define OCTAVESTEPS       12

#define  MIDI_MAX       127
#define  MIDI_SCALE     (1/127)
#define  MIDI_NULL      64
#define  MIDI_SSCALE    (1/64)
#define  MIDI_BIGMAX    (16383)
#define  MIDI_BIGSCALE  (1/16383)
#define  MIDI_BIGNULL   8192
#define  MIDI_BIGSSCALE (1/8192)
#define  MIDI_MSBSHIFT  128
#define  MIDI_OFF       63

#define FRAC_INIT  1
#define FRAC_TAP   2
#define FRAC_SET   3
#define FRAC_SUM   4
#define FRAC_SHIFT 5

#define RANDOM_UNIFORM  1
#define RANDOM_LINEAR   2
#define RANDOM_EXPON    3
#define RANDOM_GAUSSIAN 4
#define RANDOM_PROCESS  5

#define WINDOW_HAMMING  1
#define WINDOW_HANNING  2
#define WINDOW_BARTLETT 3
#define WINDOW_GAUSSIAN 4
#define WINDOW_KAISER   5
#define WINDOW_BOXCAR   6

#endif // SLIB_STD 
