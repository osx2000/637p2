

/*
#    Sfront, a SAOL to C translator    
#    This file: fast note gliss control driver for sfront
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
#    Maintainer: John Lazzaro, lazzaro@cs.berkeley.edu
*/



#define CSYSI_BOTTOMNOTE 20
#define CSYSI_TOPNOTE 100
#define CSYSI_VELOCITY 64
#define CSYSI_LENGTH  60
#define CSYSI_NUMPFIELDS 2
/****************************************************************/
/****************************************************************/
/*             glissando control driver for sfront              */ 
/****************************************************************/

int csysi_flag = CSYSI_LENGTH;
int csysi_on = 0;
int csysi_note = CSYSI_BOTTOMNOTE;
int csysi_noteinc = 1;
int csysi_instr = 1;
float csysi_pfields[CSYSI_NUMPFIELDS];

/****************************************************************/
/*             initialization routine for control               */
/****************************************************************/

int csys_setup(void)
     
{
  return CSYS_DONE;
}

/****************************************************************/
/*             polling routine for new data                     */
/****************************************************************/

int csys_newdata(void)
     
{
  if (csysi_flag)
    {
      csysi_flag--;
      return CSYS_NONE;
    }

  csysi_flag = CSYSI_LENGTH;
  if (csysi_noteinc == 1)
    {
      csysi_on = !csysi_on;
      return CSYS_MIDIEVENTS;
    }
  else
    return CSYS_SASLEVENTS;

}

/****************************************************************/
/*                 processes a MIDI event                       */
/****************************************************************/


int csys_midievent(unsigned char * cmd,   unsigned char * ndata, 
	           unsigned char * vdata, unsigned short * extchan,
		   float * fval)

{

  if (csysi_on)
    {
      *cmd = CSYS_MIDI_NOTEON;
      *ndata = csysi_note;
    }
  else
    {
      *cmd = CSYS_MIDI_NOTEOFF;
      *ndata = csysi_note;
      csysi_note += csysi_noteinc;
      if ((csysi_note == CSYSI_TOPNOTE)||(csysi_note == CSYSI_BOTTOMNOTE))
	{
	  csysi_noteinc *= -1;
	}
    }

  *vdata =  CSYSI_VELOCITY;
  *extchan = 0;
  return CSYS_NONE;

}


/****************************************************************/
/*                 processes a SASL event                       */
/****************************************************************/

int csys_saslevent(unsigned char * cmd, unsigned char * priority,
		   unsigned short * id, unsigned short * label,
		   float * fval, unsigned int * pnum, float ** p)

{

  if (csysi_instr)
    {
      csysi_instr = 0;
      *cmd = CSYS_SASL_INSTR;
      *priority = 0;
      *id = CSYS_SASL_INSTR_spiano;
      *label = 5;
      *fval = 0.10;
      *pnum = CSYSI_NUMPFIELDS;
      csysi_pfields[0] = csysi_note;
      csysi_pfields[1] = CSYSI_VELOCITY;
      *p = &csysi_pfields[0];
      
      csysi_note += csysi_noteinc;
      if ((csysi_note == CSYSI_TOPNOTE)||(csysi_note == CSYSI_BOTTOMNOTE))
	{
	  csysi_noteinc *= -1;
	}
      return CSYS_SASLEVENTS;
    }
  else
    {
      csysi_instr = 1;
      *cmd = CSYS_SASL_CONTROL;
      *priority = 0;
      *id = CSYS_SASL_NOINSTR;
      *label = 5;
      *fval = 0;
      *pnum = CSYS_SASL_GBL_pedal;
      return CSYS_NONE;
    }
}

	
/****************************************************************/
/*                  closing routine for control                 */
/****************************************************************/

void csys_shutdown(void)
     
{
}

