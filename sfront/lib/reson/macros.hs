
///////////////////////////////////////////////////////////////////////////////
//
//    SAOL Resonator-Based Physical Model Library
//    This file: Macros for resonator library
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
//
//    Original Author: John Wawrzynek
//    Maintainer: John Lazzaro, lazzaro@cs.berkeley.edu
//
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
//
// Vector sum over resonator output -- much faster than looping

#if (RESON_RESNUM == 0)
#define RESON_SUMOUT 0.0
#endif

#if (RESON_RESNUM == 1)
#define RESON_SUMOUT (sy[0])
#endif

#if (RESON_RESNUM == 2)
#define RESON_SUMOUT (sy[0]+sy[1])
#endif

#if (RESON_RESNUM == 3)
#define RESON_SUMOUT (sy[0]+sy[1]+sy[2])
#endif

#if (RESON_RESNUM == 4)
#define RESON_SUMOUT (sy[0]+sy[1]+sy[2]+sy[3])
#endif

#if (RESON_RESNUM == 5)
#define RESON_SUMOUT (sy[0]+sy[1]+sy[2]+sy[3]+sy[4])
#endif

#if (RESON_RESNUM == 6)
#define RESON_SUMOUT (sy[0]+sy[1]+sy[2]+sy[3]+sy[4]+sy[5])
#endif

#if (RESON_RESNUM == 7)
#define RESON_SUMOUT (sy[0]+sy[1]+sy[2]+sy[3]+sy[4]+sy[5]+sy[6])
#endif

#if (RESON_RESNUM == 8)
#define RESON_SUMOUT (sy[0]+sy[1]+sy[2]+sy[3]+sy[4]+sy[5]+sy[6]+sy[7])
#endif

#if (RESON_RESNUM == 9)
#define RESON_SUMOUT (sy[0]+sy[1]+sy[2]+sy[3]+sy[4]+sy[5]+sy[6]+sy[7]+sy[8])
#endif

#if (RESON_RESNUM == 10)
#define RESON_SUMOUT (sy[0]+sy[1]+sy[2]+sy[3]+sy[4]+sy[5]+sy[6]+sy[7]+sy[8]+sy[9])
#endif

#if (RESON_RESNUM == 11)
#define RESON_SUMOUT (sy[0]+sy[1]+sy[2]+sy[3]+sy[4]+sy[5]+sy[6]+sy[7]+sy[8]+sy[9] + sy[10])
#endif

#if (RESON_RESNUM == 12)
#define RESON_SUMOUT (sy[0]+sy[1]+sy[2]+sy[3]+sy[4]+sy[5]+sy[6]+sy[7]+sy[8]+sy[9] + sy[10] + sy[11])
#endif

#if (RESON_RESNUM > 12)
#define RESON_SUMOUT "extend reson/macro.hs"
#endif








