
/*
#    Sfront, a SAOL to C translator    
#    This file: Launches notes for control driver
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


#include "tree.h"


/*********************************************************/
/*          core routine for control parsing             */
/*********************************************************/

void makecontrolsys(void)

{

  int lc = 0;

  if (cmidi || session)
    {
      z[lc++]="unsigned char  ndata, vdata;";
      z[lc++]="unsigned short ec;";
    }
  if (csasl)
    {
      z[lc++]="unsigned char priority;";
      z[lc++]="unsigned short id;";
      z[lc++]="unsigned short label;";
      z[lc++]="unsigned int pnum;";
      z[lc++]="float * p;";
    }
  z[lc++]="float oldtime;";
  z[lc++]="instr_line * oldest;";
  z[lc++]="int ne, newevents, netevents;";
  z[lc++]="unsigned char cmd;";
  z[lc++]="float fval;\n";

  if (cin)
    z[lc++]="newevents = csys_newdata(ENGINE_PTR);";

  if (session && cmidi)
    {
      z[lc++]="netevents = nsys_newdata();";
      z[lc++]="if (netevents || (newevents & CSYS_MIDIEVENTS))";
      z[lc++]="do";
      z[lc++]=" {";
      z[lc++]="  if (netevents)";
      z[lc++]="   {";
      z[lc++]="     netevents = nsys_midievent(&cmd,&ndata,&vdata,&ec);";
      z[lc++]="     ne = netevents ? CSYS_MIDIEVENTS : ";
      z[lc++]="          (newevents & CSYS_MIDIEVENTS); ";
      z[lc++]="   }";
      z[lc++]="  else";
      z[lc++]="   {";
      z[lc++]="     ne = csys_midievent(ENGINE_PTR_COMMA &cmd,&ndata,&vdata,&ec,&fval);";
      z[lc++]="     if (cmd >= CSYS_MIDI_TSTART)";
      z[lc++]="       nsys_midisend(cmd, ndata, vdata, ec);";
      z[lc++]="   }";
    }
  else
    {
      if (cmidi)
	{
	  z[lc++]="if (newevents & CSYS_MIDIEVENTS)";
	  z[lc++]="do";
	  z[lc++]=" {";
	  z[lc++]="  ne = csys_midievent(ENGINE_PTR_COMMA &cmd,&ndata,&vdata,&ec,&fval);";
	}
      if (session)
	{
	  z[lc++]="netevents = nsys_newdata();";
	  z[lc++]="if (netevents)";
	  z[lc++]="do";
	  z[lc++]=" {";
	  z[lc++]="  ne = nsys_midievent(&cmd,&ndata,&vdata,&ec);";
	}
    }

  if (cmidi || session)
    {
      z[lc++]="  if ((ec < CSYS_MAXEXTCHAN)||((cmd&0xF0)==CSYS_MIDI_SPECIAL))";
      z[lc++]="  switch(cmd&0xF0) {";
      z[lc++]="   case CSYS_MIDI_NOTEON:";
      z[lc++]="   if ((EV(cme_first)[ec])==NULL)";
      z[lc++]="     break;";
      z[lc++]="   if (vdata != 0)";
      z[lc++]="    {";
      z[lc++]="     if (*EV(cme_next)[ec] == NULL)";
      z[lc++]="       *EV(cme_next)[ec] = *EV(cme_first)[ec] = *EV(cme_last)[ec];";
      z[lc++]="     else";
      z[lc++]="       {";
      z[lc++]="         oldest = NULL;";
      z[lc++]="         oldtime = MAXENDTIME;";
      z[lc++]="         *EV(cme_next)[ec] = *EV(cme_first)[ec];";
      z[lc++]="         while ((*EV(cme_next)[ec]) < (*EV(cme_end)[ec]))";
      z[lc++]="   	 {";
      z[lc++]="   	   if (((*EV(cme_next)[ec])->noteon == NOTUSEDYET)||";
      z[lc++]="   	       ((*EV(cme_next)[ec])->noteon == ALLDONE))";
      z[lc++]="   	     break;";
      z[lc++]="            if ((*EV(cme_next)[ec])->starttime < oldtime)";
      z[lc++]="             {";
      z[lc++]="               oldest =  (*EV(cme_next)[ec]);";
      z[lc++]="               oldtime = (*EV(cme_next)[ec])->starttime;";
      z[lc++]="             }";
      z[lc++]="   	   (*EV(cme_next)[ec])++;";
      z[lc++]="   	 }";
      z[lc++]="        if ((*EV(cme_next)[ec]) == (*EV(cme_end)[ec]))";
      z[lc++]="         {";
      z[lc++]="           *EV(cme_next)[ec] = oldest;"; 
      z[lc++]="           oldest->nstate->iline = NULL;";
      z[lc++]="         }";
      z[lc++]="         if ((*EV(cme_next)[ec]) > (*EV(cme_last)[ec]))";
      z[lc++]="   	    *EV(cme_last)[ec] = *EV(cme_next)[ec];";
      z[lc++]="       }";
      z[lc++]="     (*EV(cme_next)[ec])->starttime = EV(scorebeats) - EV(scoremult);"; 
      z[lc++]="     (*EV(cme_next)[ec])->endtime = MAXENDTIME;";
      z[lc++]="     (*EV(cme_next)[ec])->abstime = 0.0F;"; 
      z[lc++]="     (*EV(cme_next)[ec])->time = 0.0F;"; 
      z[lc++]="     (*EV(cme_next)[ec])->sdur = -1.0F;";
      z[lc++]="     (*EV(cme_next)[ec])->released = 0;";
      z[lc++]="     (*EV(cme_next)[ec])->turnoff = 0;";
      z[lc++]="     (*EV(cme_next)[ec])->noteon = TOBEPLAYED;";
      z[lc++]="     (*EV(cme_next)[ec])->numchan = ec + CSYS_EXTCHANSTART;";
      z[lc++]="     (*EV(cme_next)[ec])->preset  = cme_preset[ec];"; 
      z[lc++]="     (*EV(cme_next)[ec])->notenum  = ndata;"; 
      z[lc++]="     (*EV(cme_next)[ec])->p[0] = ndata ;"; 
      z[lc++]="     (*EV(cme_next)[ec])->p[1] = vdata ;"; 
      z[lc++]="     break;";
      z[lc++]="    }";
      z[lc++]="   case CSYS_MIDI_NOTEOFF:"; 
      z[lc++]="    if ((EV(cme_first)[ec])==NULL)";
      z[lc++]="      break;";
      z[lc++]="    oldest = NULL;";
      z[lc++]="    oldtime = MAXENDTIME;";
      z[lc++]="    for (sysidx=*EV(cme_first)[ec];sysidx<=*EV(cme_last)[ec];sysidx++)";
      z[lc++]="      if ((sysidx->notenum == ndata) && ";
      z[lc++]="          (sysidx->numchan == ec + CSYS_EXTCHANSTART) &&";
      z[lc++]="          (sysidx->starttime < oldtime))";
      z[lc++]="       {";
      z[lc++]="         oldest = sysidx;";
      z[lc++]="         oldtime = sysidx->starttime;";
      z[lc++]="       }";
      z[lc++]="    if (oldest)"; 
      z[lc++]="       {";
      z[lc++]="          oldest->endtime = EV(scorebeats) - EV(scoremult);"; 
      z[lc++]="          oldest->notenum += 256;"; 
      z[lc++]="       }";
      z[lc++]="    break;";
      z[lc++]="   case CSYS_MIDI_PTOUCH:";
      z[lc++]="    NG(CSYS_FRAMELEN*(ec+CSYS_EXTCHANSTART)+CSYS_TOUCHPOS+ndata)=vdata;";
      z[lc++]="   break;";
      z[lc++]="   case CSYS_MIDI_CTOUCH:";
      z[lc++]="    NG(CSYS_FRAMELEN*(ec+CSYS_EXTCHANSTART)+CSYS_CHTOUCHPOS)=ndata;";
      z[lc++]="   break;";
      z[lc++]="   case CSYS_MIDI_WHEEL:";
      z[lc++]="    NG(CSYS_FRAMELEN*(ec + CSYS_EXTCHANSTART) + CSYS_BENDPOS) = ";
      z[lc++]="                                            128*vdata + ndata;";
      z[lc++]="   break;";
      z[lc++]="   case CSYS_MIDI_CC:";
      z[lc++]="    NG(CSYS_FRAMELEN*(ec+CSYS_EXTCHANSTART)+CSYS_CCPOS+ndata)=vdata;";
      z[lc++]="    switch(ndata) {";
      z[lc++]="     case CSYS_MIDI_CC_BANKSELECT_LSB:";
      z[lc++]="      EV(csys_banklsb) = vdata;";
      z[lc++]="      EV(csys_bank) = EV(csys_banklsb) + 128*EV(csys_bankmsb);";
      z[lc++]="      break;";
      z[lc++]="     case CSYS_MIDI_CC_BANKSELECT_MSB:";
      z[lc++]="      EV(csys_bankmsb) = vdata;";
      z[lc++]="      EV(csys_bank) = EV(csys_banklsb) + 128*EV(csys_bankmsb);";
      z[lc++]="      break;";
      z[lc++]="     case CSYS_MIDI_CC_ALLSOUNDOFF:";

      /* Uncomment following line to make an ALLSOUNDOFF on any channel */
      /*                 kill all instrs on all channels.               */
      /*
      z[lc++]="      for (ec = 0; ec < CSYS_MAXEXTCHAN; ec++)";
      */

      z[lc++]="       if ((EV(cme_first)[ec]))";
      z[lc++]="        for (sysidx=*EV(cme_first)[ec];sysidx<=*EV(cme_last)[ec];sysidx++)";
      z[lc++]="         {";
      z[lc++]="            sysidx->noteon = ALLDONE;";
      z[lc++]="            sysidx->nstate->iline = NULL;";
      z[lc++]="         }";
      z[lc++]="      break;";
      z[lc++]="     case CSYS_MIDI_CC_ALLNOTESOFF:";

      /* Uncomment following line to make an ALLNOTESOFF on any channel */
      /*                 kill all instrs on all channels.               */
      /*
      z[lc++]="      for (ec = 0; ec < CSYS_MAXEXTCHAN; ec++)";
      */

      z[lc++]="       if ((EV(cme_first)[ec]))";
      z[lc++]="        for (sysidx=*EV(cme_first)[ec];sysidx<=*EV(cme_last)[ec];sysidx++)";
      z[lc++]="         {";
      z[lc++]="            sysidx->endtime =  EV(scorebeats) - EV(scoremult);";
      z[lc++]="            sysidx->notenum += 256;";
      z[lc++]="         }";
      z[lc++]="      break;";
      z[lc++]="     case CSYS_MIDI_CC_RESETALLCONTROL:";
      z[lc++]="      for (i = 0; i < CSYS_MIDI_CC_ALLSOUNDOFF; i++)";
      z[lc++]="        NG(CSYS_FRAMELEN*(ec+CSYS_EXTCHANSTART)+CSYS_CCPOS+i)=0.0F;";
      z[lc++]="      NG(CSYS_FRAMELEN*(ec + CSYS_EXTCHANSTART)+";
      z[lc++]="         CSYS_MIDI_CC_CHANVOLUME_MSB) = 100.0F;";
      z[lc++]="      NG(CSYS_FRAMELEN*(ec + CSYS_EXTCHANSTART)+";
      z[lc++]="         CSYS_MIDI_CC_PAN_MSB) = 64.0F;";
      z[lc++]="      NG(CSYS_FRAMELEN*(ec + CSYS_EXTCHANSTART)+";
      z[lc++]="         CSYS_MIDI_CC_EXPRESSION_MSB) = 127.0F;";
      z[lc++]="      EV(csys_bank) = EV(csys_banklsb) = EV(csys_bankmsb) = 0;";
      z[lc++]="      break;";
      z[lc++]="     default:";
      z[lc++]="      break;";
      z[lc++]="     }";
      z[lc++]="   break;";
      z[lc++]="   case CSYS_MIDI_PROGRAM:";
      z[lc++]="    if ((CSYS_NULLPROGRAM == 0) && (ndata >= CSYS_MAXPRESETS))";
      z[lc++]="      break;";
      z[lc++]="    if ((EV(cme_first)[ec]))";
      z[lc++]="     for (sysidx=*EV(cme_first)[ec];sysidx<=*EV(cme_last)[ec];sysidx++)";
      z[lc++]="       if ((sysidx->numchan == ec + CSYS_EXTCHANSTART) &&";
      z[lc++]="          (sysidx->noteon <= PLAYING))";
      z[lc++]="        {";
      z[lc++]="           sysidx->endtime = EV(scorebeats) - EV(scoremult);"; 
      z[lc++]="           sysidx->notenum += 256;"; 
      z[lc++]="        }";
      z[lc++]="    if ((CSYS_NULLPROGRAM == 0) || (ndata < CSYS_MAXPRESETS))";
      z[lc++]="      {";
      z[lc++]="       EV(cme_first)[ec] = EV(cmp_first)[ndata];";
      z[lc++]="       EV(cme_last)[ec] = EV(cmp_last)[ndata];";
      z[lc++]="       EV(cme_end)[ec] = EV(cmp_end)[ndata];";
      z[lc++]="       EV(cme_next)[ec] = EV(cmp_next)[ndata];";
      z[lc++]="      }";
      z[lc++]="    else";
      z[lc++]="      EV(cme_first)[ec] = EV(cme_last)[ec] = EV(cme_end)[ec] = EV(cme_next)[ec] = NULL;";
      z[lc++]="   break;";
      z[lc++]="   case CSYS_MIDI_SPECIAL:";
      z[lc++]="    if (cmd == CSYS_MIDI_NEWTEMPO)";
      z[lc++]="     {";
      z[lc++]="      if (fval <= 0.0F)";
      z[lc++]="        break;";
      z[lc++]="      EV(kbase) = EV(kcycleidx);";
      z[lc++]="      EV(scorebase) = EV(scorebeats);";
      z[lc++]="      EV(tempo) = fval;";
      z[lc++]="      EV(scoremult) = 1.666667e-02F*EV(KTIME)*EV(tempo);";
      z[lc++]="      EV(endkcycle) = EV(kbase) + (int)(EV(KRATE)*(EV(endtime)-EV(scorebase))*";
      z[lc++]="                  (60.0F/EV(tempo)));";
      z[lc++]="      break;";
      z[lc++]="     }";
      z[lc++]="    if (cmd == CSYS_MIDI_ENDTIME)";
      z[lc++]="     {";
      z[lc++]="      if (fval <= EV(scorebeats))";
      z[lc++]="      {";
      z[lc++]="        EV(endtime) = EV(scorebeats);";
      z[lc++]="        EV(endkcycle) = EV(kcycleidx);";
      z[lc++]="      }";
      z[lc++]="      else";
      z[lc++]="      {";
      z[lc++]="        EV(endtime) = fval;";
      z[lc++]="        EV(endkcycle) = EV(kbase) + (int)(EV(KRATE)*(EV(endtime)-EV(scorebase))*";
      z[lc++]="                    (60.0F/EV(tempo)));";
      z[lc++]="      }";
      z[lc++]="      break;";
      z[lc++]="     }";
      z[lc++]="    if (cmd == CSYS_MIDI_POWERUP)";
      z[lc++]="     {";
      z[lc++]="      ec &= 0xFFF0;";
      z[lc++]="      memset(&(NGU(CSYS_FRAMELEN*(ec+CSYS_EXTCHANSTART))), 0,";
      z[lc++]="             sizeof(NGU(0))*CSYS_FRAMELEN*16);";
      z[lc++]="      for (i = 0; i < 16; i++)";
      z[lc++]="      {";
      z[lc++]="        NG(CSYS_FRAMELEN*(ec + CSYS_EXTCHANSTART)+";
      z[lc++]="           CSYS_MIDI_CC_CHANVOLUME_MSB) = 100.0F;";
      z[lc++]="        NG(CSYS_FRAMELEN*(ec + CSYS_EXTCHANSTART)+";
      z[lc++]="           CSYS_MIDI_CC_PAN_MSB) = 64.0F;";
      z[lc++]="        NG(CSYS_FRAMELEN*(ec + CSYS_EXTCHANSTART)+";
      z[lc++]="           CSYS_MIDI_CC_EXPRESSION_MSB) = 127.0F;";
      z[lc++]="        NG(CSYS_FRAMELEN*(ec + CSYS_EXTCHANSTART)+";
      z[lc++]="           CSYS_BENDPOS) = 8192.0F;";
      z[lc++]="        NG(CSYS_FRAMELEN*(ec + CSYS_EXTCHANSTART)+";
      z[lc++]="           CSYS_EXTPOS) = (float)ec;";
      z[lc++]="        if ((EV(cme_first)[ec]))";
      z[lc++]="         for (sysidx=*EV(cme_first)[ec];sysidx<=*EV(cme_last)[ec];sysidx++)";
      z[lc++]="          if (sysidx->numchan == (ec + CSYS_EXTCHANSTART))";
      z[lc++]="            {";
      z[lc++]="               sysidx->endtime =  EV(scorebeats) - EV(scoremult);";
      z[lc++]="               sysidx->notenum += 256;";
      z[lc++]="            }";
      z[lc++]="        EV(cme_first)[ec] = EV(cmp_first)[i % CSYS_MAXPRESETS];";
      z[lc++]="        EV(cme_last)[ec] = EV(cmp_last)[i % CSYS_MAXPRESETS];";
      z[lc++]="        EV(cme_end)[ec] = EV(cmp_end)[i % CSYS_MAXPRESETS];";
      z[lc++]="        EV(cme_next)[ec] = EV(cmp_next)[i % CSYS_MAXPRESETS];";
      z[lc++]="        ec++;";
      z[lc++]="      }";
      z[lc++]="      break;";
      z[lc++]="     }";
      z[lc++]="   break;";
      z[lc++]="   default:";
      z[lc++]="   break;";
      z[lc++]="  }";
      z[lc++]=" } while (ne != CSYS_DONE);";

      if (session)
	z[lc++]=" nsys_endcycle();";
    }

  if (csasl)
    {
      z[lc++]="if (newevents & CSYS_SASLEVENTS)";
      z[lc++]="do";
      z[lc++]="{";
      z[lc++]="  ne = csys_saslevent(ENGINE_PTR_COMMA &cmd,&priority,&id,&label,&fval,&pnum,&p);";
      z[lc++]="  switch(cmd) {";
      z[lc++]="   case CSYS_SASL_ENDTIME:";
      z[lc++]="    if (fval <= EV(scorebeats))";
      z[lc++]="    {";
      z[lc++]="      EV(endtime) = EV(scorebeats);";
      z[lc++]="      EV(endkcycle) = EV(kcycleidx);";
      z[lc++]="    }";
      z[lc++]="    else";
      z[lc++]="    {";
      z[lc++]="      EV(endtime) = fval;";
      z[lc++]="      EV(endkcycle) = EV(kbase) + (int)(EV(KRATE)*(EV(endtime)-EV(scorebase))*";
      z[lc++]="                  (60.0F/EV(tempo)));";
      z[lc++]="    }";
      z[lc++]="    break;";
      z[lc++]="   case CSYS_SASL_TEMPO:";
      z[lc++]="    if (fval <= 0.0F)";
      z[lc++]="      break;";
      z[lc++]="    EV(kbase) = EV(kcycleidx);";
      z[lc++]="    EV(scorebase) = EV(scorebeats);";
      z[lc++]="    EV(tempo) = fval;";
      z[lc++]="    EV(scoremult) = 1.666667e-02F*EV(KTIME)*EV(tempo);";
      z[lc++]="    EV(endkcycle) = EV(kbase) + (int)(EV(KRATE)*(EV(endtime)-EV(scorebase))*";
      z[lc++]="                (60.0F/EV(tempo)));";
      z[lc++]="    break;";
      z[lc++]="   case CSYS_SASL_CONTROL:";
      z[lc++]="    if (id > CSYS_SASL_NOINSTR)";
      z[lc++]="      break;";
      z[lc++]="    if (EV(maxsc) == CSYS_SASL_MAXCONTROL)";
      z[lc++]="      break;";
      z[lc++]="    EV(saslcontrol)[EV(maxsc)].id = id;";
      z[lc++]="    EV(saslcontrol)[EV(maxsc)].label = label;";
      z[lc++]="    EV(saslcontrol)[EV(maxsc)].fptr = pnum;";
      z[lc++]="    EV(saslcontrol)[EV(maxsc)++].fval = fval;";
      z[lc++]="    break;";
      z[lc++]="   case CSYS_SASL_TABLE:";
      z[lc++]="    if (id > GBL_ENDTBL)";
      z[lc++]="      break;";
      z[lc++]="    EV(sasltable)[EV(maxtb)].tgen = label;";
      z[lc++]="    EV(sasltable)[EV(maxtb)].pnum = pnum;";
      z[lc++]="    EV(sasltable)[EV(maxtb)].p = NULL;";
      z[lc++]="    if (pnum)";
      z[lc++]="     {"; /* calloc uses pnum+1 for tgen_sample */
      z[lc++]="       EV(sasltable)[EV(maxtb)].p = (float *)"; 
      z[lc++]="                              calloc(pnum+1, sizeof(float));";
      z[lc++]="       while ((pnum--) > 0)";
      z[lc++]="         EV(sasltable)[EV(maxtb)].p[pnum] = p[pnum];";
      z[lc++]="     }";
      z[lc++]="    EV(sasltable)[EV(maxtb)++].index = id;";
      z[lc++]="    break;";
      z[lc++]="   case CSYS_SASL_INSTR:";
      z[lc++]="    if (id >= CSYS_MAXSASLINSTR)";
      z[lc++]="      break;";
      z[lc++]="    if (*EV(cs_next)[id] == NULL)";
      z[lc++]="      *EV(cs_next)[id] = *EV(cs_first)[id] = *EV(cs_last)[id];";
      z[lc++]="    else";
      z[lc++]="      {";
      z[lc++]="        oldest = NULL;";
      z[lc++]="        oldtime = MAXENDTIME;";
      z[lc++]="        *EV(cs_next)[id] = *EV(cs_first)[id];";
      z[lc++]="        while ((*EV(cs_next)[id]) < (*EV(cs_end)[id]))";
      z[lc++]="         {";
      z[lc++]="           if (((*EV(cs_next)[id])->noteon == NOTUSEDYET)||";
      z[lc++]="               ((*EV(cs_next)[id])->noteon == ALLDONE))";
      z[lc++]="             break;";
      z[lc++]="           if ((*EV(cs_next)[id])->starttime < oldtime)";
      z[lc++]="            {";
      z[lc++]="              oldest = (*EV(cs_next)[id]);";
      z[lc++]="              oldtime = (*EV(cs_next)[id])->starttime;";
      z[lc++]="            }";
      z[lc++]="           (*EV(cs_next)[id])++;";
      z[lc++]="         }";
      z[lc++]="        if ((*EV(cs_next)[id]) == (*EV(cs_end)[id]))";
      z[lc++]="         {";
      z[lc++]="           *EV(cs_next)[id] = oldest;"; 
      z[lc++]="           oldest->nstate->iline = NULL;";
      z[lc++]="         }";
      z[lc++]="        if ((*EV(cs_next)[id]) > (*EV(cs_last)[id]))";
      z[lc++]="         *EV(cs_last)[id] = *EV(cs_next)[id];";
      z[lc++]="      }";
      z[lc++]="    (*EV(cs_next)[id])->starttime = EV(scorebeats) - EV(scoremult);"; 
      z[lc++]="    (*EV(cs_next)[id])->endtime = MAXENDTIME;";
      z[lc++]="    (*EV(cs_next)[id])->abstime = 0.0F;"; 
      z[lc++]="    (*EV(cs_next)[id])->time = 0.0F;"; 
      z[lc++]="    (*EV(cs_next)[id])->sdur = fval;";
      z[lc++]="    (*EV(cs_next)[id])->released = 0;";
      z[lc++]="    (*EV(cs_next)[id])->turnoff = 0;";
      z[lc++]="    (*EV(cs_next)[id])->noteon = TOBEPLAYED;";
      z[lc++]="    (*EV(cs_next)[id])->numchan = 0;";
      z[lc++]="    (*EV(cs_next)[id])->notenum = 0;";
      z[lc++]="    (*EV(cs_next)[id])->preset  = 0;"; 
      z[lc++]="    (*EV(cs_next)[id])->label  = label;";
      z[lc++]="    pnum = (pnum > MAXPFIELDS) ? MAXPFIELDS : pnum;";
      z[lc++]="    while ((pnum--) > 0)";
      z[lc++]="     (*EV(cs_next)[id])->p[pnum] = p[pnum];"; 
      z[lc++]="    break;";
      z[lc++]="   case CSYS_SASL_NOOP:";
      z[lc++]="    break;";
      z[lc++]="   default:";
      z[lc++]="    break;";
      z[lc++]="}";
      z[lc++]=" } while (ne != CSYS_DONE);";
    }
  printraw(lc);
  fprintf(outfile,"\n");

}

/*********************************************************/
/*          SASL control command for control driver      */
/*********************************************************/


void makesaslcontrolsys(void)

{

  int lc = 0;

  z[lc++]="for (i = 0; i < EV(maxsc); i++)";
  z[lc++]=" {";
  z[lc++]="   if (EV(saslcontrol)[i].id == CSYS_SASL_NOINSTR)";
  z[lc++]="     {";
  z[lc++]="       if (EV(saslcontrol)[i].fptr < GBL_ENDVAR)";
  z[lc++]="        NG(EV(saslcontrol)[i].fptr) = EV(saslcontrol)[i].fval;";
  z[lc++]="      }";
  z[lc++]="    else";
  z[lc++]="     {";
  z[lc++]="       for (sysidx= *EV(cs_first)[EV(saslcontrol)[i].id];";
  z[lc++]="            sysidx<= *EV(cs_last)[EV(saslcontrol)[i].id];sysidx++)";
  z[lc++]="         if ((sysidx->label == EV(saslcontrol)[i].label)&&";
  z[lc++]="             (sysidx->noteon == PLAYING))";
  z[lc++]="           sysidx->nstate->v[EV(saslcontrol)[i].fptr].f";
  z[lc++]="                                = EV(saslcontrol)[i].fval;";

  /* extra loops needed for other sources of labelled instrs */

  if (allsasl->numlabels && allsasl->instrroot)
    {
      z[lc++]="";
      z[lc++]="    if ((EV(saslcontrol)[i].label <= CSYS_LABELNUM) &&";
      z[lc++]="         EV(css_first)[EV(saslcontrol)[i].id])";
      z[lc++]="      for (sysidx= *EV(css_first)[EV(saslcontrol)[i].id];";
      z[lc++]="           sysidx<= *EV(css_last)[EV(saslcontrol)[i].id];sysidx++)";
      z[lc++]="        if ((sysidx->label == EV(saslcontrol)[i].label)&&";
      z[lc++]="            (sysidx->noteon == PLAYING))";
      z[lc++]="          sysidx->nstate->v[EV(saslcontrol)[i].fptr].f";
      z[lc++]="                             = EV(saslcontrol)[i].fval;";
    }


  if (abssasl->numlabels && abssasl->instrroot)
    {
      z[lc++]="";
      z[lc++]="    if ((EV(saslcontrol)[i].label <= CSYS_LABELNUM) &&";
      z[lc++]="         EV(csa_first)[EV(saslcontrol)[i].id])";
      z[lc++]="      for (sysidx= *EV(csa_first)[EV(saslcontrol)[i].id];";
      z[lc++]="           sysidx<= *EV(csa_last)[EV(saslcontrol)[i].id];sysidx++)";
      z[lc++]="        if ((sysidx->label == EV(saslcontrol)[i].label)&&";
      z[lc++]="            (sysidx->noteon == PLAYING))";
      z[lc++]="          sysidx->nstate->v[EV(saslcontrol)[i].fptr].f";
      z[lc++]="                             = EV(saslcontrol)[i].fval;";

    }


  z[lc++]="     }";
  z[lc++]=" }";
  z[lc++]="EV(maxsc) = 0;";

  printraw(lc);
  fprintf(outfile,"\n");

}


/*********************************************************/
/*         cross-control command for control driver      */
/*********************************************************/

void makesaslcrosscontrol(char * prefix)

{

  fprintf(outfile,"         for (i = 0; i < CSYS_MAXSASLINSTR; i++)\n");
  fprintf(outfile,"           for (sysidx= *EV(cs_first)[i];sysidx<= *EV(cs_last)[i];sysidx++)\n");
  fprintf(outfile,"             if ((sysidx->label == EV(%scontrolidx)->label)&&\n", prefix);
  fprintf(outfile,"                 (sysidx->noteon == PLAYING))\n");
  fprintf(outfile,"               sysidx->nstate->v[EV(%scontrolidx)->imptr].f\n",prefix);
  fprintf(outfile,"                                     = EV(%scontrolidx)->imval;\n",prefix);

}

/*********************************************************/
/*          SASL table command for control driver        */
/*********************************************************/


void makesasltablesys(void)

{

  int lc = 0;

  z[lc++]="for (i = 0; i < EV(maxtb); i++)";
  z[lc++]=" {";
  z[lc++]="   switch (EV(sasltable)[i].tgen) {";                 
  z[lc++]="    case CSYS_SASL_TGEN_SAMPLE:";
  z[lc++]="     tgen_sample(ENGINE_PTR_COMMA &(EV(gtables)[EV(sasltable)[i].index]),";
  z[lc++]="                 EV(sasltable)[i].pnum, EV(sasltable)[i].p);";
  z[lc++]="     break;";
  z[lc++]="    case CSYS_SASL_TGEN_DATA:";
  z[lc++]="     tgen_data(ENGINE_PTR_COMMA &(EV(gtables)[EV(sasltable)[i].index]),";
  z[lc++]="                 EV(sasltable)[i].pnum, EV(sasltable)[i].p);";
  z[lc++]="     break;";
  z[lc++]="    case CSYS_SASL_TGEN_RANDOM:";
  z[lc++]="     tgen_random(ENGINE_PTR_COMMA &(EV(gtables)[EV(sasltable)[i].index]),";
  z[lc++]="                 EV(sasltable)[i].pnum, EV(sasltable)[i].p);";
  z[lc++]="     break;";
  z[lc++]="    case CSYS_SASL_TGEN_STEP:";
  z[lc++]="     tgen_step(ENGINE_PTR_COMMA &(EV(gtables)[EV(sasltable)[i].index]),";
  z[lc++]="                 EV(sasltable)[i].pnum, EV(sasltable)[i].p);";
  z[lc++]="     break;";
  z[lc++]="    case CSYS_SASL_TGEN_LINESEG:";
  z[lc++]="     tgen_lineseg(ENGINE_PTR_COMMA &(EV(gtables)[EV(sasltable)[i].index]),";
  z[lc++]="                 EV(sasltable)[i].pnum, EV(sasltable)[i].p);";
  z[lc++]="     break;";
  z[lc++]="    case CSYS_SASL_TGEN_EXPSEG:";
  z[lc++]="     tgen_expseg(ENGINE_PTR_COMMA &(EV(gtables)[EV(sasltable)[i].index]),";
  z[lc++]="                 EV(sasltable)[i].pnum, EV(sasltable)[i].p);";
  z[lc++]="     break;";
  z[lc++]="    case CSYS_SASL_TGEN_CUBICSEG:";
  z[lc++]="     tgen_cubicseg(ENGINE_PTR_COMMA &(EV(gtables)[EV(sasltable)[i].index]),";
  z[lc++]="                 EV(sasltable)[i].pnum, EV(sasltable)[i].p);";
  z[lc++]="     break;";
  z[lc++]="    case CSYS_SASL_TGEN_POLYNOMIAL:";
  z[lc++]="     tgen_polynomial(ENGINE_PTR_COMMA &(EV(gtables)[EV(sasltable)[i].index]),";
  z[lc++]="                 EV(sasltable)[i].pnum, EV(sasltable)[i].p);";
  z[lc++]="     break;";
  z[lc++]="    case CSYS_SASL_TGEN_SPLINE:"; 
  z[lc++]="     tgen_spline(ENGINE_PTR_COMMA &(EV(gtables)[EV(sasltable)[i].index]),";
  z[lc++]="                 EV(sasltable)[i].pnum, EV(sasltable)[i].p);";
  z[lc++]="     break;";
  z[lc++]="    case CSYS_SASL_TGEN_WINDOW  :";   
  z[lc++]="     tgen_window(ENGINE_PTR_COMMA &(EV(gtables)[EV(sasltable)[i].index]),";
  z[lc++]="                 EV(sasltable)[i].pnum, EV(sasltable)[i].p);";
  z[lc++]="     break;";
  z[lc++]="    case CSYS_SASL_TGEN_HARM :";  
  z[lc++]="     tgen_harm(ENGINE_PTR_COMMA &(EV(gtables)[EV(sasltable)[i].index]),";
  z[lc++]="                 EV(sasltable)[i].pnum, EV(sasltable)[i].p);";
  z[lc++]="     break;";
  z[lc++]="    case CSYS_SASL_TGEN_HARM_PHASE:"; 
  z[lc++]="     tgen_harm_phase(ENGINE_PTR_COMMA &(EV(gtables)[EV(sasltable)[i].index]),";
  z[lc++]="                 EV(sasltable)[i].pnum, EV(sasltable)[i].p);";
  z[lc++]="     break;";
  z[lc++]="    case CSYS_SASL_TGEN_PERIODIC:";
  z[lc++]="     tgen_periodic(ENGINE_PTR_COMMA &(EV(gtables)[EV(sasltable)[i].index]),";
  z[lc++]="                 EV(sasltable)[i].pnum, EV(sasltable)[i].p);";
  z[lc++]="     break;";
  z[lc++]="    case CSYS_SASL_TGEN_BUZZ :"; 
  z[lc++]="     tgen_buzz(ENGINE_PTR_COMMA &(EV(gtables)[EV(sasltable)[i].index]),";
  z[lc++]="                 EV(sasltable)[i].pnum, EV(sasltable)[i].p);";
  z[lc++]="     break;";
  z[lc++]="    case CSYS_SASL_TGEN_CONCAT:";
  z[lc++]="     tgen_concat_global(ENGINE_PTR_COMMA &(EV(gtables)[EV(sasltable)[i].index]),";
  z[lc++]="                 EV(sasltable)[i].pnum, EV(sasltable)[i].p);";
  z[lc++]="     break;";
  z[lc++]="    case CSYS_SASL_TGEN_EMPTY :";
  z[lc++]="     tgen_empty(ENGINE_PTR_COMMA &(EV(gtables)[EV(sasltable)[i].index]),";
  z[lc++]="                 EV(sasltable)[i].pnum, EV(sasltable)[i].p);";
  z[lc++]="     break;";
  z[lc++]="    case CSYS_SASL_TGEN_DESTROY:";
  z[lc++]="     tgen_destroy(ENGINE_PTR_COMMA &(EV(gtables)[EV(sasltable)[i].index]),";
  z[lc++]="                 EV(sasltable)[i].pnum, EV(sasltable)[i].p);";
  z[lc++]="     break;";
  z[lc++]="     }";
  z[lc++]=" }";
  z[lc++]="EV(maxtb) = 0;";

  printraw(lc);
  fprintf(outfile,"\n");

}

