
//////////////////////////////////////////////////////////////////////////
//
//   Slib, Sfront's SAOL library
//   This file: The trig library (trigonometric and related opcodes)
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
// This library is a collection of polymorphic core opcode definitions,
// for trignometric and other related functions that are not in the 
// MP4-SA core opcode set.
//
//
// Opcode header                Description
//
// opcode tan(xsig x);          Tangent of x.
// opcode sinh(xsig x);         Hyperbolic sine of x.
// opcode cosh(xsig x);         Hyperbolic cosine of x.
// opcode tanh(xsig x);         Hyperbolic tangent of x.
// opcode asinh(xsig x);        Inverse hyperbolic sine of x.
// opcode acosh(xsig x);        Inverse hyperbolic cosine of x.
// opcode atanh(xsig x);        Inverse hyperbolic tangent of x.
// 
/////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////
//                                                
// library definitions begin here
//

#ifndef SLIB_TRIG
#define SLIB_TRIG

opcode tan(xsig x) 

{
  return ((sin(x)/cos(x)));
}

opcode sinh(xsig x)

{
  xsig tmp;

  tmp = exp(x);
  return(0.5*(tmp - (1/tmp)));
}

opcode cosh(xsig x)

{
  xsig tmp;

  tmp = exp(x);
  return(0.5*(tmp + (1/tmp)));
}

opcode tanh(xsig x)

{
  xsig tmp;

  tmp = exp(x);
  tmp = tmp*tmp;
  return((tmp - 1)/(tmp + 1));
}

opcode asinh(xsig x)

{
  return(log(x + sqrt(x*x + 1)));
}

opcode acosh(xsig x)

{
  return(log(x + sqrt(x*x - 1)));
}

opcode atanh(xsig x)

{
  return(0.5*log((1+x)/(1-x)));
}


#endif // SLIB_TRIG 
