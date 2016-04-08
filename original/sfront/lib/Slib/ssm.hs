
//////////////////////////////////////////////////////////////////////////
//
//   Slib, Sfront's SAOL library
//   This file: The SSM (Scaled Smoothed MIDInames) library
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
// This library is intended to replace direct MIDIctrl[], MIDIbend, 
// and MIDItouch calls in SAOL programs. It defines a set of SSM
// symbols with the following properties:
//
//  *  Can be used like standard names in expressions
//  *  Linear [0,1], [-1, 1], or binary 0/1 scaling as required
//  *  Non-binary names as smoothed at the k-rate, using the port()
//     opcode, at a default timeconstant of 4/krate. This rate
//     may be overriden by setting the SSM_SMOOTHRATE symbol to a time
//     value (in seconds).
//  *  Scaled, non-smoothed value for all symbols available as SMname.
//     For binary names, SMname defined identically to SSMname.
//  *  If SSM_HIGHRES defined, also uses LSB (default is MSB only)
//
// 
//       Name        Scaling     Function           uses SAOL stname
//
//  SSMattack        [0,1]       Sound Attack        MIDIctrl[73]
//  SSMbalance      [-1,+1]      Stereo Balance      MIDIctrl[8, 40]
//  SSMbend         [-1,+1]      Pitch Bend Wheel    MIDIbend
//  SSMbreath        [0,1]       Breath Controller   MIDIctrl[2, 34]
//  SSMbright        [0,1]       Sound Brightness    MIDIctrl[74]
//  SSMbutton1       0/1         G.P Button 1        MIDIctrl[80]
//  SSMbutton2       0/1         G.P Button 2        MIDIctrl[81]
//  SSMbutton3       0/1         G.P Button 3        MIDIctrl[82]
//  SSMbutton4       0/1         G.P Button 4        MIDIctrl[83]
//  SSMchorus        [0,1]       Chorus Level        MIDIctrl[93]
//  SSMdataslider    [0,1]       Data Entry Slider   MIDIctrl[6, 38]
//  SSMdetune        [0,1]       Detuning Amount     MIDIctrl[94]
//  SSMeffect        [0,1]       Effects Level       MIDIctrl[91]
//  SSMeffect1       [0,1]       Effect Control 1    MIDIctrl[12,44]
//  SSMeffect2       [0,1]       Effect Control 2    MIDIctrl[13,45]
//  SSMexpress       [0,1]       Expression          MIDIctrl[11,43]
//  SSMfoot          [0,1]       Foot Controller     MIDIctrl[2, 34]
//  SSMhold           0/1        Hold Pedal          MIDIctrl[64]
//  SSMhold2          0/1        Hold 2 Pedal        MIDIctrl[69]
//  SSMlegato         0/1        Legato Pedal        MIDIctrl[68]
//  SSMlocal          0/1        Local Kbd Off/On    MIDIctrl[122]
//  SSMmodwheel      [0,1]       Modulation Wheel    MIDIctrl[1, 33]   
//  SSMpan          [-1,+1]      Stereo Panning      MIDIctrl[10,42]
//  SSMphasor        [0,1]       Phasor Level        MIDIctrl[95]
//  SSMporta          0/1        Portamento On/Off   MIDIctrl[65]
//  SSMportactrl      0/1        Portamento Control  MIDIctrl[84]
//  SSMportatime     [0,1]       Portamento Time     MIDIctrl[5, 36]
//  SSMrelease       [0,1]       Sound Release       MIDIctrl[72]
//  SSMslider1       [0,1]       G. P. Slider 1      MIDIctrl[16, 48]
//  SSMslider2       [0,1]       G. P. Slider 2      MIDIctrl[17, 49]
//  SSMslider3       [0,1]       G. P. Slider 3      MIDIctrl[18, 50]
//  SSMslider4       [0,1]       G. P. Slider 4      MIDIctrl[19, 51]
//  SSMsoft           0/1        Soft Pedal          MIDIctrl[67]
//  SSMsound6        [0,1]       Sound Control 6     MIDIctrl[75]
//  SSMsound7        [0,1]       Sound Control 7     MIDIctrl[76]
//  SSMsound8        [0,1]       Sound Control 8     MIDIctrl[77]
//  SSMsound9        [0,1]       Sound Control 9     MIDIctrl[78]
//  SSMsound10       [0,1]       Sound Control 10    MIDIctrl[79]
//  SSMsust           0/1        Sustenuto Pedal     MIDIctrl[66]
//  SSMtimbre        [0,1]       Sound Timbre        MIDIctrl[71]
//  SSMtouch         [0,1]       Aftertouch          MIDItouch
//  SSMtremelo       [0,1]       Tremelo Level       MIDIctrl[92]
//  SSMundef3        [0,1]       Undefined           MIDIctrl[3, 35]
//  SSMundef14       [0,1]       Undefined           MIDIctrl[14, 46]
//  SSMundef15       [0,1]       Undefined           MIDIctrl[15, 47]
//  SSMundef20       [0,1]       Undefined           MIDIctrl[20, 52]
//  SSMundef21       [0,1]       Undefined           MIDIctrl[21, 53]
//  SSMundef22       [0,1]       Undefined           MIDIctrl[22, 54]
//  SSMundef23       [0,1]       Undefined           MIDIctrl[23, 55]
//  SSMundef24       [0,1]       Undefined           MIDIctrl[24, 56]
//  SSMundef25       [0,1]       Undefined           MIDIctrl[25, 57]
//  SSMundef26       [0,1]       Undefined           MIDIctrl[26, 58]
//  SSMundef27       [0,1]       Undefined           MIDIctrl[27, 59]
//  SSMundef28       [0,1]       Undefined           MIDIctrl[28, 60]
//  SSMundef29       [0,1]       Undefined           MIDIctrl[29, 61]
//  SSMundef30       [0,1]       Undefined           MIDIctrl[30, 62]
//  SSMundef31       [0,1]       Undefined           MIDIctrl[31, 63]
//  SSMundef85       [0,1]       Undefined           MIDIctrl[85]
//  SSMundef86       [0,1]       Undefined           MIDIctrl[86]
//  SSMundef87       [0,1]       Undefined           MIDIctrl[87]
//  SSMundef88       [0,1]       Undefined           MIDIctrl[88]
//  SSMundef89       [0,1]       Undefined           MIDIctrl[89]
//  SSMundef90       [0,1]       Undefined           MIDIctrl[90]
//  SSMundef102      [0,1]       Undefined           MIDIctrl[102]
//  SSMundef103      [0,1]       Undefined           MIDIctrl[103]
//  SSMundef104      [0,1]       Undefined           MIDIctrl[104]
//  SSMundef105      [0,1]       Undefined           MIDIctrl[105]
//  SSMundef106      [0,1]       Undefined           MIDIctrl[106]
//  SSMundef107      [0,1]       Undefined           MIDIctrl[107]
//  SSMundef108      [0,1]       Undefined           MIDIctrl[108]
//  SSMundef109      [0,1]       Undefined           MIDIctrl[109]
//  SSMundef110      [0,1]       Undefined           MIDIctrl[110]
//  SSMundef111      [0,1]       Undefined           MIDIctrl[111]
//  SSMundef112      [0,1]       Undefined           MIDIctrl[112]
//  SSMundef113      [0,1]       Undefined           MIDIctrl[113]
//  SSMundef114      [0,1]       Undefined           MIDIctrl[114]
//  SSMundef115      [0,1]       Undefined           MIDIctrl[115]
//  SSMundef116      [0,1]       Undefined           MIDIctrl[116]
//  SSMundef117      [0,1]       Undefined           MIDIctrl[117]
//  SSMundef118      [0,1]       Undefined           MIDIctrl[118]
//  SSMundef119      [0,1]       Undefined           MIDIctrl[119]
//  SSMvar           [0,1]       Sound Variation     MIDIctrl[70]
//  SSMvolume        [0,1]       Channel Volume      MIDIctrl[7, 39]
//
//
//  Note that decoder support is needed to handle Registered and
//  Non-Registered Parameters, since these are event-based. So, 
//  no definitions for Data Entry Button +/- and Parameter Number
//  controllers appear above. I did include the Data Entry Slider,
//  since its possible to use these values in a non-event-based way.
//
//
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
//                                                
// library definitions begin here
//

#ifndef SLIB_SSM
#define SLIB_SSM

// where MIDI_ constants are found

#include <Slib/std.hs>

#ifndef SSM_SMOOTHRATE
#define SSM_SMOOTHRATE (4/k_rate)
#endif 

#define  SMattack (MIDI_SCALE*MIDIctrl[73])
#define SSMattack (port(SMattack, SSM_SMOOTHRATE))

#ifdef SSM_HIGHRES
#define  SMbalance (MIDI_BIGSSCALE*(MIDI_MSBSHIFT*MIDIctrl[8] + MIDIctrl[40] - MIDI_BIGNULL))
#else
#define  SMbalance (MIDI_SSCALE*(MIDIctrl[8] - MIDI_NULL))
#endif
#define SSMbalance (port(SMbalance, SSM_SMOOTHRATE))

#define  SMbend (MIDI_BIGSSCALE*(MIDIbend - MIDI_BIGNULL))
#define SSMbend (port(SMbend, SSM_SMOOTHRATE))

#ifdef SSM_HIGHRES
#define  SMbreath (MIDI_BIGSCALE*(MIDI_MSBSHIFT*MIDIctrl[2] + MIDIctrl[34]))
#else
#define  SMbreath (MIDI_SCALE*MIDIctrl[2])
#endif
#define SSMbreath (port(SMbreath, SSM_SMOOTHRATE))

#define  SMbright  (MIDI_SCALE*MIDIctrl[74])
#define SSMbright (port(SMbright, SSM_SMOOTHRATE))
#define  SMbutton1 ((MIDIctrl[80] > MIDI_OFF) ? 1 : 0)
#define SSMbutton1 SMbutton1
#define  SMbutton2 ((MIDIctrl[81] > MIDI_OFF) ? 1 : 0)
#define SSMbutton2 SMbutton2
#define  SMbutton3 ((MIDIctrl[82] > MIDI_OFF) ? 1 : 0)
#define SSMbutton3 SMbutton3
#define  SMbutton4 ((MIDIctrl[83] > MIDI_OFF) ? 1 : 0)
#define SSMbutton4 SMbutton4
#define  SMchorus  (MIDI_SCALE*MIDIctrl[93])
#define SSMchorus (port(SMchorus, SSM_SMOOTHRATE))

#ifdef SSM_HIGHRES
#define SMdataslider (MIDI_BIGSCALE*(MIDI_MSBSHIFT*MIDIctrl[6] + MIDIctrl[38]))
#else
#define SMdataslider (MIDI_SCALE*MIDIctrl[6])
#endif
#define SSMdataslider (port(SMdataslider, SSM_SMOOTHRATE))

#define  SMdetune (MIDI_SCALE*MIDIctrl[94])
#define SSMdetune (port(SMdetune, SSM_SMOOTHRATE))
#define  SMeffect (MIDI_SCALE*MIDIctrl[91])
#define SSMeffect (port(SMeffect, SSM_SMOOTHRATE))

#ifdef SSM_HIGHRES
#define  SMeffect1 (MIDI_BIGSCALE*(MIDI_MSBSHIFT*MIDIctrl[12] + MIDIctrl[44]))
#else
#define  SMeffect1 (MIDI_SCALE*MIDIctrl[12])
#endif
#define SSMeffect1 (port(SMeffect1, SSM_SMOOTHRATE))

#ifdef SSM_HIGHRES
#define  SMeffect2 (MIDI_BIGSCALE*(MIDI_MSBSHIFT*MIDIctrl[13] + MIDIctrl[45]))
#else
#define  SMeffect2 (MIDI_SCALE*MIDIctrl[13])
#endif
#define SSMeffect2 (port(SMeffect2, SSM_SMOOTHRATE))

#ifdef SSM_HIGHRES
#define  SMexpress (MIDI_BIGSCALE*(MIDI_MSBSHIFT*MIDIctrl[11] + MIDIctrl[43]))
#else
#define  SMexpress (MIDI_SCALE*MIDIctrl[11])
#endif
#define SSMexpress (port(SMexpress, SSM_SMOOTHRATE))

#ifdef SSM_HIGHRES
#define  SMfoot (MIDI_BIGSCALE*(MIDI_MSBSHIFT*MIDIctrl[4] + MIDIctrl[36]))
#else
#define  SMfoot (MIDI_SCALE*MIDIctrl[4])
#endif
#define SSMfoot (port(SMfoot, SSM_SMOOTHRATE))

#define  SMhold   ((MIDIctrl[64] > MIDI_OFF)  ? 1 : 0)
#define SSMhold   SMhold
#define  SMhold2  ((MIDIctrl[69] > MIDI_OFF)  ? 1 : 0)
#define SSMhold2  SMhold2
#define  SMlegato ((MIDIctrl[68] > MIDI_OFF)  ? 1 : 0)
#define SSMlegato SMlegato
#define  SMlocal  ((MIDIctrl[122] > MIDI_OFF) ? 1 : 0)
#define SSMlocal  SMlocal

#ifdef SSM_HIGHRES
#define  SMmodwheel (MIDI_BIGSCALE*(MIDI_MSBSHIFT*MIDIctrl[1] + MIDIctrl[33]))
#else
#define  SMmodwheel (MIDI_SCALE*MIDIctrl[1])
#endif
#define SSMmodwheel (port(SMmodwheel, SSM_SMOOTHRATE))

#ifdef SSM_HIGHRES
#define  SMpan (MIDI_BIGSSCALE*(MIDI_MSBSHIFT*MIDIctrl[10] + MIDIctrl[42] - MIDI_BIGNULL))
#else
#define  SMpan (MIDI_SSCALE*(MIDIctrl[10] - MIDI_NULL))
#endif
#define SSMpan (port(SMpan, SSM_SMOOTHRATE))

#define  SMphasor (MIDI_SCALE*MIDIctrl[95])
#define SSMphasor (port(SMphasor, SSM_SMOOTHRATE))

#define  SMporta  ((MIDIctrl[65] > MIDI_OFF) ? 1 : 0)
#define SSMporta  SMporta

#define  SMportactrl (MIDI_SCALE*MIDIctrl[84])
#define SSMportactrl (port(SMportactrl, SSM_SMOOTHRATE))

#ifdef SSM_HIGHRES
#define  SMportatime (MIDI_BIGSCALE*(MIDI_MSBSHIFT*MIDIctrl[5] + MIDIctrl[36]))
#else
#define  SMportatime (MIDI_SCALE*MIDIctrl[5])
#endif
#define SSMportatime (port(SMportatime, SSM_SMOOTHRATE))

#define  SMrelease (MIDI_SCALE*MIDIctrl[72])
#define SSMrelease (port(SMrelease, SSM_SMOOTHRATE))

#ifdef SSM_HIGHRES
#define  SMslider1 (MIDI_BIGSCALE*(MIDI_MSBSHIFT*MIDIctrl[16] + MIDIctrl[48]))
#else
#define  SMslider1 (MIDI_SCALE*MIDIctrl[16])
#endif
#define SSMslider1 (port(SMslider1, SSM_SMOOTHRATE))

#ifdef SSM_HIGHRES
#define  SMslider2 (MIDI_BIGSCALE*(MIDI_MSBSHIFT*MIDIctrl[17] + MIDIctrl[49]))
#else
#define  SMslider2 (MIDI_SCALE*MIDIctrl[17])
#endif
#define SSMslider2 (port(SMslider2, SSM_SMOOTHRATE))

#ifdef SSM_HIGHRES
#define  SMslider3 (MIDI_BIGSCALE*(MIDI_MSBSHIFT*MIDIctrl[18] + MIDIctrl[50]))
#else
#define  SMslider3 (MIDI_SCALE*MIDIctrl[18])
#endif
#define SSMslider3 (port(SMslider3, SSM_SMOOTHRATE))

#ifdef SSM_HIGHRES
#define  SMslider4 (MIDI_BIGSCALE*(MIDI_MSBSHIFT*MIDIctrl[19] + MIDIctrl[51]))
#else
#define  SMslider4 (MIDI_SCALE*MIDIctrl[19])
#endif
#define SSMslider4 (port(SMslider4, SSM_SMOOTHRATE))



#define  SMsoft    ((MIDIctrl[67] > MIDI_OFF) ? 1 : 0)
#define SSMsoft    SMsoft
#define  SMsound6  (MIDI_SCALE*MIDIctrl[75])
#define SSMsound6  (port(SMsound6, SSM_SMOOTHRATE))
#define  SMsound7  (MIDI_SCALE*MIDIctrl[76])
#define SSMsound7  (port(SMsound7, SSM_SMOOTHRATE))
#define  SMsound8  (MIDI_SCALE*MIDIctrl[77])
#define SSMsound8  (port(SMsound8, SSM_SMOOTHRATE))
#define  SMsound9  (MIDI_SCALE*MIDIctrl[78])
#define SSMsound9  (port(SMsound9, SSM_SMOOTHRATE))
#define  SMsound10 (MIDI_SCALE*MIDIctrl[79])
#define SSMsound10 (port(SMsound10, SSM_SMOOTHRATE))
#define  SMsust    ((MIDIctrl[66] > MIDI_OFF) ? 1 : 0)
#define SSMsust    SMsust
#define  SMtimbre  (MIDI_SCALE*MIDIctrl[71])
#define SSMtimbre  (port(SMtimbre, SSM_SMOOTHRATE))
#define  SMtremelo (MIDI_SCALE*MIDIctrl[92])
#define SSMtremelo (port(SMtremelo, SSM_SMOOTHRATE))
#define  SMtouch   (MIDI_SCALE*MIDItouch)
#define SSMtouch   (port(SMtouch, SSM_SMOOTHRATE))


#ifdef SSM_HIGHRES
#define  SMundef3 (MIDI_BIGSCALE*(MIDI_MSBSHIFT*MIDIctrl[3] + MIDIctrl[35]))
#else
#define  SMundef3 (MIDI_SCALE*MIDIctrl[3])
#endif
#define SSMundef3 (port(SMundef3, SSM_SMOOTHRATE))


#ifdef SSM_HIGHRES
#define  SMundef14 (MIDI_BIGSCALE*(MIDI_MSBSHIFT*MIDIctrl[14] + MIDIctrl[46]))
#else
#define  SMundef14 (MIDI_SCALE*MIDIctrl[14])
#endif
#define SSMundef14 (port(SMundef14, SSM_SMOOTHRATE))

#ifdef SSM_HIGHRES
#define  SMundef15 (MIDI_BIGSCALE*(MIDI_MSBSHIFT*MIDIctrl[15] + MIDIctrl[47]))
#else
#define  SMundef15 (MIDI_SCALE*MIDIctrl[15])
#endif
#define SSMundef15 (port(SMundef15, SSM_SMOOTHRATE))

#ifdef SSM_HIGHRES
#define  SMundef20 (MIDI_BIGSCALE*(MIDI_MSBSHIFT*MIDIctrl[20] + MIDIctrl[52]))
#else
#define  SMundef20 (MIDI_SCALE*MIDIctrl[20])
#endif
#define SSMundef20 (port(SMundef20, SSM_SMOOTHRATE))

#ifdef SSM_HIGHRES
#define  SMundef21 (MIDI_BIGSCALE*(MIDI_MSBSHIFT*MIDIctrl[21] + MIDIctrl[53]))
#else
#define  SMundef21 (MIDI_SCALE*MIDIctrl[21])
#endif
#define SSMundef21 (port(SMundef21, SSM_SMOOTHRATE))

#ifdef SSM_HIGHRES
#define  SMundef22 (MIDI_BIGSCALE*(MIDI_MSBSHIFT*MIDIctrl[22] + MIDIctrl[54]))
#else
#define  SMundef22 (MIDI_SCALE*MIDIctrl[22])
#endif
#define SSMundef22 (port(SMundef22, SSM_SMOOTHRATE))

#ifdef SSM_HIGHRES
#define  SMundef23 (MIDI_BIGSCALE*(MIDI_MSBSHIFT*MIDIctrl[23] + MIDIctrl[55]))
#else
#define  SMundef23 (MIDI_SCALE*MIDIctrl[23])
#endif
#define SSMundef23 (port(SMundef23, SSM_SMOOTHRATE))

#ifdef SSM_HIGHRES
#define  SMundef24 (MIDI_BIGSCALE*(MIDI_MSBSHIFT*MIDIctrl[24] + MIDIctrl[56]))
#else
#define  SMundef24 (MIDI_SCALE*MIDIctrl[24])
#endif
#define SSMundef24 (port(SMundef24, SSM_SMOOTHRATE))

#ifdef SSM_HIGHRES
#define  SMundef25 (MIDI_BIGSCALE*(MIDI_MSBSHIFT*MIDIctrl[25] + MIDIctrl[57]))
#else
#define  SMundef25 (MIDI_SCALE*MIDIctrl[25])
#endif
#define SSMundef25 (port(SMundef25, SSM_SMOOTHRATE))

#ifdef SSM_HIGHRES
#define  SMundef26 (MIDI_BIGSCALE*(MIDI_MSBSHIFT*MIDIctrl[26] + MIDIctrl[58]))
#else
#define  SMundef26 (MIDI_SCALE*MIDIctrl[26])
#endif
#define SSMundef26 (port(SMundef26, SSM_SMOOTHRATE))

#ifdef SSM_HIGHRES
#define  SMundef27 (MIDI_BIGSCALE*(MIDI_MSBSHIFT*MIDIctrl[27] + MIDIctrl[59]))
#else
#define  SMundef27 (MIDI_SCALE*MIDIctrl[27])
#endif
#define SSMundef27 (port(SMundef27, SSM_SMOOTHRATE))

#ifdef SSM_HIGHRES
#define  SMundef28 (MIDI_BIGSCALE*(MIDI_MSBSHIFT*MIDIctrl[28] + MIDIctrl[60]))
#else
#define  SMundef28 (MIDI_SCALE*MIDIctrl[28])
#endif
#define SSMundef28 (port(SMundef28, SSM_SMOOTHRATE))

#ifdef SSM_HIGHRES
#define  SMundef29 (MIDI_BIGSCALE*(MIDI_MSBSHIFT*MIDIctrl[29] + MIDIctrl[61]))
#else
#define  SMundef29 (MIDI_SCALE*MIDIctrl[29])
#endif
#define SSMundef29 (port(SMundef29, SSM_SMOOTHRATE))

#ifdef SSM_HIGHRES
#define  SMundef30 (MIDI_BIGSCALE*(MIDI_MSBSHIFT*MIDIctrl[30] + MIDIctrl[62]))
#else
#define  SMundef30 (MIDI_SCALE*MIDIctrl[30])
#endif
#define SSMundef30 (port(SMundef30, SSM_SMOOTHRATE))

#ifdef SSM_HIGHRES
#define  SMundef31 (MIDI_BIGSCALE*(MIDI_MSBSHIFT*MIDIctrl[31] + MIDIctrl[63]))
#else
#define  SMundef31 (MIDI_SCALE*MIDIctrl[31])
#endif
#define SSMundef31 (port(SMundef31, SSM_SMOOTHRATE))

#define  SMundef85 (MIDI_SCALE*MIDIctrl[85])
#define SSMundef85 (port(SMundef85, SSM_SMOOTHRATE))

#define  SMundef86 (MIDI_SCALE*MIDIctrl[86])
#define SSMundef86 (port(SMundef86, SSM_SMOOTHRATE))

#define  SMundef87 (MIDI_SCALE*MIDIctrl[87])
#define SSMundef87 (port(SMundef87, SSM_SMOOTHRATE))

#define  SMundef88 (MIDI_SCALE*MIDIctrl[88])
#define SSMundef88 (port(SMundef88, SSM_SMOOTHRATE))

#define  SMundef89 (MIDI_SCALE*MIDIctrl[89])
#define SSMundef89 (port(SMundef89, SSM_SMOOTHRATE))

#define  SMundef90 (MIDI_SCALE*MIDIctrl[90])
#define SSMundef90 (port(SMundef90, SSM_SMOOTHRATE))

#define  SMundef102 (MIDI_SCALE*MIDIctrl[102])
#define SSMundef102 (port(SMundef102, SSM_SMOOTHRATE))

#define  SMundef103 (MIDI_SCALE*MIDIctrl[103])
#define SSMundef103 (port(SMundef103, SSM_SMOOTHRATE))

#define  SMundef104 (MIDI_SCALE*MIDIctrl[104])
#define SSMundef104 (port(SMundef104, SSM_SMOOTHRATE))

#define  SMundef105 (MIDI_SCALE*MIDIctrl[105])
#define SSMundef105 (port(SMundef105, SSM_SMOOTHRATE))

#define  SMundef106 (MIDI_SCALE*MIDIctrl[106])
#define SSMundef106 (port(SMundef106, SSM_SMOOTHRATE))

#define  SMundef107 (MIDI_SCALE*MIDIctrl[107])
#define SSMundef107 (port(SMundef107, SSM_SMOOTHRATE))

#define  SMundef108 (MIDI_SCALE*MIDIctrl[108])
#define SSMundef108 (port(SMundef108, SSM_SMOOTHRATE))

#define  SMundef109 (MIDI_SCALE*MIDIctrl[109])
#define SSMundef109 (port(SMundef109, SSM_SMOOTHRATE))

#define  SMundef110 (MIDI_SCALE*MIDIctrl[110])
#define SSMundef110 (port(SMundef110, SSM_SMOOTHRATE))

#define  SMundef111 (MIDI_SCALE*MIDIctrl[111])
#define SSMundef111 (port(SMundef111, SSM_SMOOTHRATE))

#define  SMundef112 (MIDI_SCALE*MIDIctrl[112])
#define SSMundef112 (port(SMundef112, SSM_SMOOTHRATE))

#define  SMundef113 (MIDI_SCALE*MIDIctrl[113])
#define SSMundef113 (port(SMundef113, SSM_SMOOTHRATE))

#define  SMundef114 (MIDI_SCALE*MIDIctrl[114])
#define SSMundef114 (port(SMundef114, SSM_SMOOTHRATE))

#define  SMundef115 (MIDI_SCALE*MIDIctrl[115])
#define SSMundef115 (port(SMundef115, SSM_SMOOTHRATE))

#define  SMundef116 (MIDI_SCALE*MIDIctrl[116])
#define SSMundef116 (port(SMundef116, SSM_SMOOTHRATE))

#define  SMundef117 (MIDI_SCALE*MIDIctrl[117])
#define SSMundef117 (port(SMundef117, SSM_SMOOTHRATE))

#define  SMundef118 (MIDI_SCALE*MIDIctrl[118])
#define SSMundef118 (port(SMundef118, SSM_SMOOTHRATE))

#define  SMundef119 (MIDI_SCALE*MIDIctrl[119])
#define SSMundef119 (port(SMundef119, SSM_SMOOTHRATE))

#define  SMvar     (MIDI_SCALE*MIDIctrl[70])
#define SSMvar     (port(SMvar, SSM_SMOOTHRATE))

#ifdef SSM_HIGHRES
#define  SMvolume (MIDI_BIGSCALE*(MIDI_MSBSHIFT*MIDIctrl[7] + MIDIctrl[39]))
#else
#define  SMvolume (MIDI_SCALE*MIDIctrl[7])
#endif
#define SSMvolume (port(SMvolume, SSM_SMOOTHRATE))

#endif // SLIB_SSM 
