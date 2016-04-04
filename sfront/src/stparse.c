
/*
#    Sfront, a SAOL to C translator    
#    This file: Parses standard names
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

/***************************************************/
/* returns 1 if S_IDENT is a standard name, else 0 */
/***************************************************/

int standardname(tnode * ident)

{
  if (ident->ttype != S_IDENT)
    return 0;
  if (!(strcmp(ident->val,"k_rate")))
    return 1;
  if (!(strcmp(ident->val,"s_rate")))
    return 1;
  if (!(strcmp(ident->val,"inchan")))
    return 1;
  if (!(strcmp(ident->val,"outchan")))
    return 1;
  if (!(strcmp(ident->val,"time")))
    return 1;
  if (!(strcmp(ident->val,"dur")))
    return 1;
  if (!(strcmp(ident->val,"itime")))
    return 1;
  if (!(strcmp(ident->val,"preset")))
    return 1;
  if (!(strcmp(ident->val,"channel")))
    return 1;
  if (!(strcmp(ident->val,"MIDIctrl")))
    return 1;
  if (!(strcmp(ident->val,"MIDItouch")))
    return 1;
  if (!(strcmp(ident->val,"MIDIbend")))
    return 1;
  if (!(strcmp(ident->val,"input")))
    return 1;
  if (!(strcmp(ident->val,"inGroup")))
    return 1;
  if (!(strcmp(ident->val,"released")))
    return 1;
  if (!(strcmp(ident->val,"cpuload")))
    return 1;
  if (!(strcmp(ident->val,"position")))
    return 1;
  if (!(strcmp(ident->val,"direction")))
    return 1;
  if (!(strcmp(ident->val,"listenerPosition")))
    return 1;
  if (!(strcmp(ident->val,"listenerDirection")))
    return 1;
  if (!(strcmp(ident->val,"minFront")))
    return 1;
  if (!(strcmp(ident->val,"maxFront")))
    return 1;
  if (!(strcmp(ident->val,"minBack")))
    return 1;
  if (!(strcmp(ident->val,"maxBack")))
    return 1;
  if (!(strcmp(ident->val,"params")))
    return 1;

  return 0;
}

/***************************************************/
/*          updates has array                      */
/***************************************************/

void hasstandardname(tnode * ident)

{
  if (ident->ttype != S_IDENT)
    return;
  if (!(strcmp(ident->val,"k_rate")))
    {
      has.s_k_rate++;
      return;
    }
  if (!(strcmp(ident->val,"s_rate")))
    {
      has.s_s_rate++;
      return;
    }
  if (!(strcmp(ident->val,"inchan")))
    {
      has.s_inchan++;
      return;
    }
  if (!(strcmp(ident->val,"outchan")))
    {
      has.s_outchan++;
      return;
    }
  if (!(strcmp(ident->val,"time")))
    {
      has.s_time++;
      return;
    }
  if (!(strcmp(ident->val,"dur")))
    {
      has.s_dur++;
      return;
    }
  if (!(strcmp(ident->val,"itime")))
    {
      has.s_itime++;
      return;
    }
  if (!(strcmp(ident->val,"preset")))
    {
      has.s_preset++;
      return;
    }
  if (!(strcmp(ident->val,"channel")))
    {
      has.s_channel++;
      return;
    }
  if (!(strcmp(ident->val,"MIDIctrl")))
    {
      has.s_MIDIctrl++;
      return;
    }
  if (!(strcmp(ident->val,"MIDItouch")))
    {
      has.s_MIDItouch++;
      return;
    }
  if (!(strcmp(ident->val,"MIDIbend")))
    {
      has.s_MIDIbend++;
      return;
    }
  if (!(strcmp(ident->val,"input")))
    {
      has.s_input++;
      return;
    }
  if (!(strcmp(ident->val,"inGroup")))
    {
      has.s_inGroup++;
      return;
    }
  if (!(strcmp(ident->val,"released")))
    {
      has.s_released++;
      return;
    }
  if (!(strcmp(ident->val,"cpuload")))
    {
      has.s_cpuload++;
      return;
    }
  if (!(strcmp(ident->val,"position")))
    {
      has.s_position++;
      return;
    }
  if (!(strcmp(ident->val,"direction")))
    {
      has.s_direction++;
      return;
    }
  if (!(strcmp(ident->val,"listenerPosition")))
    {
      has.s_listenerPosition++;
      return;
    }
  if (!(strcmp(ident->val,"listenerDirection")))
    {
      has.s_listenerDirection++;
      return;
    }
  if (!(strcmp(ident->val,"minFront")))
    {
      has.s_minFront++;
      return;
    }
  if (!(strcmp(ident->val,"maxFront")))
    {
      has.s_maxFront++;
      return;
    }
  if (!(strcmp(ident->val,"minBack")))
    {
      has.s_minBack++;
      return;
    }
  if (!(strcmp(ident->val,"maxBack")))
    {
      has.s_maxBack++;
      return;
    }
  if (!(strcmp(ident->val,"params")))
    {
      has.s_params++;
      return;
    }
}

/***************************************************/
/*       returns rate of standard name             */
/***************************************************/

int standardrate(tnode * ident)

{
  if (ident->ttype != S_IDENT)
    return UNKNOWN;
  if (!(strcmp(ident->val,"k_rate")))
    return IRATETYPE;
  if (!(strcmp(ident->val,"s_rate")))
    return IRATETYPE;
  if (!(strcmp(ident->val,"inchan")))
    return IRATETYPE;
  if (!(strcmp(ident->val,"outchan")))
    return IRATETYPE;
  if (!(strcmp(ident->val,"time")))
    return IRATETYPE;
  if (!(strcmp(ident->val,"dur")))
    return IRATETYPE;
  if (!(strcmp(ident->val,"itime")))
    return KRATETYPE;
  if (!(strcmp(ident->val,"preset")))
    return IRATETYPE;
  if (!(strcmp(ident->val,"channel")))
    return IRATETYPE;
  if (!(strcmp(ident->val,"MIDIctrl")))
    return KRATETYPE;
  if (!(strcmp(ident->val,"MIDItouch")))
    return KRATETYPE;
  if (!(strcmp(ident->val,"MIDIbend")))
    return KRATETYPE;
  if (!(strcmp(ident->val,"input")))
    return ARATETYPE;
  if (!(strcmp(ident->val,"inGroup")))
    return IRATETYPE;
  if (!(strcmp(ident->val,"released")))
    return KRATETYPE;
  if (!(strcmp(ident->val,"cpuload")))
    return KRATETYPE;
  if (!(strcmp(ident->val,"position")))
    return KRATETYPE;
  if (!(strcmp(ident->val,"direction")))
    return KRATETYPE;
  if (!(strcmp(ident->val,"listenerPosition")))
    return KRATETYPE;
  if (!(strcmp(ident->val,"listenerDirection")))
    return KRATETYPE;
  if (!(strcmp(ident->val,"minFront")))
    return KRATETYPE;
  if (!(strcmp(ident->val,"maxFront")))
    return KRATETYPE;
  if (!(strcmp(ident->val,"minBack")))
    return KRATETYPE;
  if (!(strcmp(ident->val,"maxBack")))
    return KRATETYPE;
  if (!(strcmp(ident->val,"params")))
    return KRATETYPE;

  return UNKNOWN;
}

/***************************************************/
/*       returns ASINT/ASFLOAT of standard name    */
/***************************************************/

int standardres(tnode * ident)

{
  if (ident->ttype != S_IDENT)
    return ASFLOAT;
  if (!(strcmp(ident->val,"k_rate")))
    return ASINT;
  if (!(strcmp(ident->val,"s_rate")))
    return ASINT;
  if (!(strcmp(ident->val,"inchan")))
    return ASINT;
  if (!(strcmp(ident->val,"outchan")))
    return ASINT;
  if (!(strcmp(ident->val,"time")))
    return ASFLOAT;
  if (!(strcmp(ident->val,"dur")))
    return ASFLOAT;
  if (!(strcmp(ident->val,"itime")))
    return ASFLOAT;
  if (!(strcmp(ident->val,"preset")))
    return ASINT;
  if (!(strcmp(ident->val,"channel")))
    return ASINT;
  if (!(strcmp(ident->val,"MIDIctrl")))
    return ASFLOAT;
  if (!(strcmp(ident->val,"MIDItouch")))
    return ASFLOAT;
  if (!(strcmp(ident->val,"MIDIbend")))
    return ASFLOAT;
  if (!(strcmp(ident->val,"input")))
    return ASFLOAT;
  if (!(strcmp(ident->val,"inGroup")))
    return ASFLOAT;
  if (!(strcmp(ident->val,"released")))
    return ASINT;
  if (!(strcmp(ident->val,"cpuload")))
    return ASFLOAT;
  if (!(strcmp(ident->val,"position")))
    return ASFLOAT;
  if (!(strcmp(ident->val,"direction")))
    return ASFLOAT;
  if (!(strcmp(ident->val,"listenerPosition")))
    return ASFLOAT;
  if (!(strcmp(ident->val,"listenerDirection")))
    return ASFLOAT;
  if (!(strcmp(ident->val,"minFront")))
    return ASFLOAT;
  if (!(strcmp(ident->val,"maxFront")))
    return ASFLOAT;
  if (!(strcmp(ident->val,"minBack")))
    return ASFLOAT;
  if (!(strcmp(ident->val,"maxBack")))
    return ASFLOAT;
  if (!(strcmp(ident->val,"params")))
    return ASFLOAT;

  return ASFLOAT;
}

/***************************************************/
/*   returns scalar/vector type of standardname    */
/***************************************************/

int standardvartype(tnode * ident)

{
  if (ident->ttype != S_IDENT)
    return SCALARTYPE;
  if (!(strcmp(ident->val,"k_rate")))
    return SCALARTYPE;
  if (!(strcmp(ident->val,"s_rate")))
    return SCALARTYPE;
  if (!(strcmp(ident->val,"inchan")))
    return SCALARTYPE;
  if (!(strcmp(ident->val,"outchan")))
    return SCALARTYPE;
  if (!(strcmp(ident->val,"time")))
    return SCALARTYPE;
  if (!(strcmp(ident->val,"dur")))
    return SCALARTYPE;
  if (!(strcmp(ident->val,"itime")))
    return SCALARTYPE;
  if (!(strcmp(ident->val,"preset")))
    return SCALARTYPE;
  if (!(strcmp(ident->val,"channel")))
    return SCALARTYPE;
  if (!(strcmp(ident->val,"MIDIctrl")))
    return VECTORTYPE;
  if (!(strcmp(ident->val,"MIDItouch")))
    return SCALARTYPE;
  if (!(strcmp(ident->val,"MIDIbend")))
    return SCALARTYPE;
  if (!(strcmp(ident->val,"input")))
    return VECTORTYPE;
  if (!(strcmp(ident->val,"inGroup")))
    return VECTORTYPE;
  if (!(strcmp(ident->val,"released")))
    return SCALARTYPE;
  if (!(strcmp(ident->val,"cpuload")))
    return SCALARTYPE;
  if (!(strcmp(ident->val,"position")))
    return VECTORTYPE;
  if (!(strcmp(ident->val,"direction")))
    return VECTORTYPE;
  if (!(strcmp(ident->val,"listenerPosition")))
    return VECTORTYPE;
  if (!(strcmp(ident->val,"listenerDirection")))
    return VECTORTYPE;
  if (!(strcmp(ident->val,"minFront")))
    return SCALARTYPE;
  if (!(strcmp(ident->val,"maxFront")))
    return SCALARTYPE;
  if (!(strcmp(ident->val,"minBack")))
    return SCALARTYPE;
  if (!(strcmp(ident->val,"maxBack")))
    return SCALARTYPE;
  if (!(strcmp(ident->val,"params")))
    return VECTORTYPE;

  return SCALARTYPE;
}


/***************************************************/
/*       returns width of standard name            */
/***************************************************/

int standardwidth(tnode * ident)

{
  if (ident->ttype != S_IDENT)
    return UNKNOWN;
  if (!(strcmp(ident->val,"k_rate")))
    return 1;
  if (!(strcmp(ident->val,"s_rate")))
    return 1;
  if (!(strcmp(ident->val,"inchan")))
    return 1;
  if (!(strcmp(ident->val,"outchan")))
    return 1;
  if (!(strcmp(ident->val,"time")))
    return 1;
  if (!(strcmp(ident->val,"dur")))
    return 1;
  if (!(strcmp(ident->val,"itime")))
    return 1;
  if (!(strcmp(ident->val,"preset")))
    return 1;
  if (!(strcmp(ident->val,"channel")))
    return 1;
  if (!(strcmp(ident->val,"MIDIctrl")))
    return 128;
  if (!(strcmp(ident->val,"MIDItouch")))
    return 1;
  if (!(strcmp(ident->val,"MIDIbend")))
    return 1;
  if (!(strcmp(ident->val,"input")))
    return currinputwidth;    
  if (!(strcmp(ident->val,"inGroup")))
    return currinputwidth;
  if (!(strcmp(ident->val,"released")))
    return 1;
  if (!(strcmp(ident->val,"cpuload")))
    return 1;
  if (!(strcmp(ident->val,"position")))
    return 3;
  if (!(strcmp(ident->val,"direction")))
    return 3;
  if (!(strcmp(ident->val,"listenerPosition")))
    return 3;
  if (!(strcmp(ident->val,"listenerDirection")))
    return 3;
  if (!(strcmp(ident->val,"minFront")))
    return 1;
  if (!(strcmp(ident->val,"maxFront")))
    return 1;
  if (!(strcmp(ident->val,"minBack")))
    return 1;
  if (!(strcmp(ident->val,"maxBack")))
    return 1;
  if (!(strcmp(ident->val,"params")))
    return 128;

  return 0;
}

/***************************************************/
/*       returns 1 if dur is a constant            */
/***************************************************/

int constdur(void)

{

  /* type of instruments where dur = -1 always */

  if ((currinstrument == NULL) ||      /* in global block */
      (currinstance != NULL) ||        /* code for effects instance, or */
      ((currinstrument->score == 0) && /* not in score and */
      (currinstrument->ascore == 0) && /* not in score and */
       (currinstrument->dyn == 0) &&   /* not dynamic and */
       (csasl == 0)))                  /* not SASL control */
    return 1;

  /* later add detection for other constant durs */

  return 0;
  
}

/***************************************************/
/*      collapses constant standard names          */
/***************************************************/

int standardcollapse(tnode * ident)

{
  char name[128];

  if ((!(strcmp(ident->val,"k_rate"))) && (!reentrant))
    {
      ident->vol   = CONSTANT;
      ident->ttype = S_INTGR;
      ident->res = ASINT;
      ident->rate = IRATETYPE;
      sprintf(name,"%i",krate);
      ident->val = dupval(name);
      return CONSTANT;
    }
  if ((!(strcmp(ident->val,"s_rate"))) && (!reentrant))
    {
      ident->vol   = CONSTANT;
      ident->ttype = S_INTGR;
      ident->res = ASINT;
      ident->rate = IRATETYPE;
      sprintf(name,"%i",srate);
      ident->val = dupval(name);
      return CONSTANT;
    }
  if (!(strcmp(ident->val,"inchan")))
    {
      ident->vol   = CONSTANT;
      ident->ttype = S_INTGR;
      ident->res = ASINT;
      ident->rate = IRATETYPE;
      sprintf(name,"%i", currinputwidth);
      ident->val = dupval(name);
      return CONSTANT;
    }
  if (!(strcmp(ident->val,"outchan")))
    {
      ident->vol   = CONSTANT;
      ident->ttype = S_INTGR;
      ident->res = ASINT;
      ident->rate = IRATETYPE;
      sprintf(name,"%i", currinstrwidth);
      ident->val = dupval(name);
      return CONSTANT;
    }
  if (!(strcmp(ident->val,"dur")))
    {
      if (constdur())
	{
	  /* right now -1 is the only detected constant dur */
	  ident->vol   = CONSTANT;
	  ident->ttype = S_INTGR;
	  ident->res = ASINT;
	  ident->rate = IRATETYPE;
	  ident->val = dupval("-1");
	  return CONSTANT;
	}
    }
  return VARIABLE;
}



/***************************************************/
/*       handles input/ingroup for non-effects     */
/***************************************************/

void nobus(tnode ** ident)

{
  tnode * tptr = * ident;

  fprintf(outfile," 0 ");

  if ((tptr->next != NULL) &&     /* an array index */
      (tptr->next->ttype == S_LB))
    {
      *ident = tptr->next->next->next;  /* skip to end  */
    }

  return;

}

/***************************************************/
/*   handles input/ingroup in unindexed context    */
/***************************************************/

void arraybus(tnode ** ident, int group)

{
  tnode * bptr = currinstance->ibus;
  int i = 0;
  int gcount = 1;

  if (currinstance->inwidth == 1)   /* array of size 1 */
    {
      if (bptr == NULL)
	fprintf(outfile," 0 ");
      else
	{
	  if (group)
	    fprintf(outfile," 1 ");
	  else
	    fprintf(outfile," %s(BUS_%s) ", inputbusmacro(), bptr->val);
	}
      return;
    }

  while ((bptr != NULL)&& (i < currinstance->inwidth))
    {
      if (currarrayindex < i + bptr->width)
	{
	  if (group)
	    fprintf(outfile," %i ",gcount);
	  else
	    fprintf(outfile," %s(BUS_%s + %i) ", inputbusmacro(), 
		    bptr->val, currarrayindex - i);
	  return;
	}
      i += bptr->width;
      bptr = bptr->next;
      gcount++;
    }
  fprintf(outfile," 0 ");      /* out of range */
  return;

}

/***************************************************/
/*   handles input/ingroup with constant index     */
/***************************************************/

void constbus(tnode ** ident, int group)

{
  tnode * tptr = * ident;
  tnode * bptr = currinstance->ibus;
  int i = 0;
  int gcount = 1;
  int idxval;

  if ((idxval = make_int(tptr->next->next->down)) < 0)
    {	
      printf("Error: %s[] constant index < 0.\n\n", tptr->val);
      showerrorplace(tptr->linenum, tptr->filename);
    }

  while ((bptr != NULL)&& (i < currinstance->inwidth))
    {
      if (idxval < i + bptr->width)
	{
	  if (group)
	    fprintf(outfile," %i ",gcount);
	  else
	    fprintf(outfile," %s(BUS_%s + %i)", inputbusmacro(),
		    bptr->val, idxval - i);
	  *ident = tptr->next->next->next;
	  return;
	}
      i += bptr->width;
      bptr = bptr->next;
      gcount++;
    }

  fprintf(outfile," 0 ");      /* out of range */

  *ident = tptr->next->next->next;

  return;

}

/***************************************************/
/*       handles input/ingroup for one bus         */
/***************************************************/

void onebus(tnode ** ident, int group)

{
  tnode * tptr = * ident;
  tnode * bptr = currinstance->ibus;
  int currintstack;

  if (group)
    {
      fprintf(outfile," 1 ");
      return;
    }

  currintstack = currintprint;
  fprintf(outfile," %s(BUS_%s + ", inputbusmacro(), bptr->val);

  if (tptr->next->next->res == ASINT)
    {
      currintprint = ASINT;
      blocktree(tptr->next->next->down, PRINTTOKENS);
      fprintf(outfile,")");
    }
  else
    {
      fprintf(outfile,"((int)(0.5F + (");
      currintprint = ASFLOAT;
      blocktree(tptr->next->next->down, PRINTTOKENS);
      fprintf(outfile," )))) ");
    }

  *ident = tptr->next->next->next;  /* skip to end  */
  currintprint = currintstack;
  return;

}

/***************************************************/
/*       handles MIDI names for non-midi programs  */
/***************************************************/

int hasmidisys(void)

{
  if (totmidichan)
    return totmidichan;

  if (currintprint == ASFLOAT)
    fprintf(outfile, " 0.0F ");
  else
    fprintf(outfile, " 0 ");

  return 0;

}

/***************************************************/
/*       handles audioBIFS names for !cin case     */
/***************************************************/

int hascdriver(void)

{
  if (cin || session)
    return 1;

  if (currintprint == ASFLOAT)
    fprintf(outfile, " 0.0F ");
  else
    fprintf(outfile, " 0 ");

  return 0;

}

/***************************************************/
/*       handles array audioBIFS names             */
/***************************************************/

void printaudiobiffarray(tnode * tptr, tnode ** ident)

{
  int currintstack;

  /* params will always be created if SAOL programs uses it */

  if (strcmp(tptr->val, "params") && (!hascdriver()))
    {
      if (tptr->next == NULL)
	return;
      *ident = tptr->next->next->next;  /* skip to end  */
      return;
    }

  if (tptr->next == NULL)  /* unindexed array */
    {
      fprintf(outfile," EV(%s[%i]) ", tptr->val, currarrayindex);
    }
  else                    /* indexed array */
    {
      currintstack = currintprint;
      fprintf(outfile," EV(%s[", tptr->val);
      if (tptr->next->next->res == ASINT)
	{
	  currintprint = ASINT;
	  blocktree(tptr->next->next->down, PRINTTOKENS);
	  fprintf(outfile," ]) ");
	}
      else
	{
	  currintprint = ASFLOAT;
	  fprintf(outfile, " (int)(0.5F + (");
	  blocktree(tptr->next->next->down, PRINTTOKENS);
	      fprintf(outfile," ))]) ");
	}
      *ident = tptr->next->next->next;  /* skip to end  */
      currintprint = currintstack;
    }
}


/***************************************************/
/*       prints C code for each standard name      */
/***************************************************/

void printstandardname(tnode ** ident)

{

  tnode * bptr;
  tnode * tptr = * ident;
  int group;
  int currintstack;

  if (tptr->ttype != S_IDENT)
    return;
  if (!(strcmp(tptr->val,"k_rate")))
    {
      if (reentrant)
	{
	  if (currintprint == ASFLOAT)
	    fprintf(outfile," EV(KRATE) ");
	  else
	    fprintf(outfile," ((int)(EV(KRATE))) ");
	}
      else
	{
	  if (currintprint == ASFLOAT)
	    fprintf(outfile," %i.0F ", krate);
	  else
	    fprintf(outfile," %i ", krate);
	}
      return;
    }
  if (!(strcmp(tptr->val,"s_rate")))
    {
      if (reentrant)
	{
	  if (currintprint == ASFLOAT)
	    fprintf(outfile," EV(ARATE) ");
	  else
	    fprintf(outfile," ((int)(EV(ARATE))) ");
	}
      else
	{
	  if (currintprint == ASFLOAT)
	    fprintf(outfile," %i.0F ", srate);
	  else
	    fprintf(outfile," %i ", srate);
	}
      return;
    }
  if (!(strcmp(tptr->val,"inchan")))
    {
      if (currintprint == ASFLOAT)
	fprintf(outfile," %i.0F ", currinputwidth);
      else
	fprintf(outfile," %i ", currinputwidth);
      return;
    }
  if (!(strcmp(tptr->val,"outchan")))
    {
      if (currintprint == ASFLOAT)
	fprintf(outfile," %i.0F ", currinstrwidth);
      else
	fprintf(outfile," %i ", currinstrwidth);
      return;
    }
  if (!(strcmp(tptr->val,"time")))
    {
      fprintf(outfile," NS(iline->time) ");
      return;
    }
  if (!(strcmp(tptr->val,"dur")))
    {
      if (currinstance != NULL)
	fprintf(outfile, " NS(v[%s__dur].f) ", currinstance->val);
      else
	{
	  if (currinstrument != NULL)
	    fprintf(outfile, "  NS(v[%s__dur].f) ", currinstrument->val);
	  else
	    fprintf(outfile," 0 ");
	}
      return;
    }
  if (!(strcmp(tptr->val,"itime")))
    {
      fprintf(outfile, " NS(iline->itime) ");
      return;
    }
  if (!(strcmp(tptr->val,"preset")))
    {
      if (hasmidisys())
	{      
	  if (currintprint == ASFLOAT)
	    fprintf(outfile," ((float)NS(iline->preset)) ");
	  else
	    fprintf(outfile," NS(iline->preset) ");
	}
      return;
    }
  if (!(strcmp(tptr->val,"channel")))
    {
      if (hasmidisys())
	fprintf(outfile," NG(%i*NS(iline->numchan)+%i)",
		MIDIFRAMELEN, MIDIEXTPOS);
      return;
    }
  if (!(strcmp(tptr->val,"MIDIctrl")))
    {
      
      /* MIDIctrl may be LHS, and so we can't use hasmidisys() */
      /* to create 0.0F in the case of no MIDI inputs ...      */

      if (tptr->next == NULL)  /* unindexed array */
	{
	  if (totmidichan)
	    fprintf(outfile," NG(%i*NS(iline->numchan) + %i + %i) ",
		    MIDIFRAMELEN,MIDICTRLPOS,currarrayindex);
	  else
	    fprintf(outfile," EV(fakeMIDIctrl[%i]) ", currarrayindex);
	}
      else                    /* indexed array */
	{
	  currintstack = currintprint;

	  if (totmidichan)
	    fprintf(outfile," NG(%i*NS(iline->numchan) + %i + ",
		    MIDIFRAMELEN,MIDICTRLPOS);
	  else
	    fprintf(outfile," EV(fakeMIDIctrl[( ");

	  if (tptr->next->next->res == ASINT)
	    {
	      currintprint = ASINT;
	      blocktree(tptr->next->next->down, PRINTTOKENS);
	      fprintf(outfile," ) ");
	    }
	  else
	    {
	      currintprint = ASFLOAT;
	      fprintf(outfile, " ((int)(0.5F + (");
	      blocktree(tptr->next->next->down, PRINTTOKENS);
	      fprintf(outfile," )))) ");
	    }

	  if (!totmidichan)
	    fprintf(outfile, " ]) ");

	  *ident = tptr->next->next->next;  /* skip to end  */
	  currintprint = currintstack;
	}
      return;
    }
  if (!(strcmp(tptr->val,"MIDItouch")))
    {
      if (!hasmidisys())
	return;

      fprintf(outfile,
      " ((NG(%i*NS(iline->numchan)+%i+(255&(NS(iline->notenum))))>0.0F) ? \n",
	      MIDIFRAMELEN, MIDITOUCHPOS);
      fprintf(outfile,
      " NG(%i*NS(iline->numchan)+%i+(255&(NS(iline->notenum)))) : \n",
	      MIDIFRAMELEN, MIDITOUCHPOS);
      fprintf(outfile,
      " NG(%i*NS(iline->numchan)+%i))\n",
	      MIDIFRAMELEN, MIDICHTOUCHPOS);
      return;
    }
  if (!(strcmp(tptr->val,"MIDIbend")))
    {
      if (hasmidisys())
	fprintf(outfile," NG(%i*NS(iline->numchan)+%i)",
		MIDIFRAMELEN, MIDIBENDPOS);
      return;
    }
  if ((!(strcmp(tptr->val,"input")))||(!(strcmp(tptr->val,"inGroup"))))
    {
      if (currinstance == NULL)  /* non-effects instruments */
	{
	  nobus(ident);
	  return;
	}

      group = !strcmp(tptr->val,"inGroup");

      if (tptr->next == NULL)     /* unindexed array -- easy special case */
	{
	  arraybus(ident,group);
	  return;
	}

      bptr = currinstance->ibus;

      if (tptr->next->next->vol == CONSTANT) /* index is a constant */
	{
	  constbus(ident, group);
	  return;
	}

      if (bptr->next == NULL)     /* just one bus -- easy special case */
	{
	  onebus(ident, group);
	  return;
	}

      /* later, do krate index special case */

      /* slow worst case */
      /* later optimize for ASINT indexes and in other ways too */

      currintstack = currintprint;
      currintprint = tptr->next->next->res;
      if (group)
	{
	  currinstance->usesingroup = 1;
	  printf ("finGroup%i((float)( \n",currinstance->arrayidx);
	}
      else
	{
	  currinstance->usesinput = 1;
	  fprintf(outfile,"finput%i((float)( ",currinstance->arrayidx);
	}
      blocktree(tptr->next->next->down, PRINTTOKENS);
      fprintf(outfile,"))");
      *ident = tptr->next->next->next;  /* skip to end  */
      currintprint = currintstack;
      return;
    }
  if (!(strcmp(tptr->val,"released")))
    {
      if (currintprint == ASFLOAT)
	fprintf(outfile," ((float)NS(iline->released)) ");
      else
	fprintf(outfile, " NS(iline->released) ");
      return;
    }
  if (!(strcmp(tptr->val,"cpuload")))
    {
      fprintf(outfile," EV(cpuload) ");
      return;
    }
  if (!(strcmp(tptr->val,"position")))
    { 
      printaudiobiffarray(tptr, ident);
      return;
    }
  if (!(strcmp(tptr->val,"direction")))
    { 
      printaudiobiffarray(tptr, ident);
      return;
    }
  if (!(strcmp(tptr->val,"listenerPosition")))
    { 
      printaudiobiffarray(tptr, ident);
      return;
    }
  if (!(strcmp(tptr->val,"listenerDirection")))
    { 
      printaudiobiffarray(tptr, ident);
      return;
    }
  if (!(strcmp(tptr->val,"minFront")))
    {  
      if (hascdriver())
	fprintf(outfile," %s ", tptr->val);
      return;
    }
  if (!(strcmp(tptr->val,"maxFront")))
    {
      if (hascdriver())
	fprintf(outfile," %s ", tptr->val);
      return;
    }
  if (!(strcmp(tptr->val,"minBack")))
    {
      if (hascdriver())
	fprintf(outfile," %s ", tptr->val);
      return;
    }
  if (!(strcmp(tptr->val,"maxBack")))
    {
      if (hascdriver())
	fprintf(outfile," %s ", tptr->val);
      return;
    }
  if (!(strcmp(tptr->val,"params")))
    { 
      printaudiobiffarray(tptr, ident);
      return;
    }

}


/***************************************************/
/*       prints C code to assign _dur              */
/***************************************************/

void printdurassign(void)

{
  if ( (!constdur()) &&  
       (((currblockrate == IRATETYPE) && (currinstrument->cref->idur)) ||
	((currblockrate == KRATETYPE) && (currinstrument->cref->kadur))))
    {

      /* this can be simplified for special cases  */

      if (currinstance)
	fprintf(outfile, "   NS(v[%s__dur].f) = \n", currinstance->val);
      else
	{
	  if (currinstrument)
	    fprintf(outfile, "   NS(v[%s__dur].f) = \n", currinstrument->val);
	}

      if (currinstrument)
	{
	  fprintf(outfile,"   ((NS(iline->sdur) < 0.0F) ? -1.0F :\n");
	  fprintf(outfile,"   (NS(iline->abstime) + \n");
	  fprintf(outfile,"   (EV(kcycleidx)-1)*EV(KTIME) - NS(iline->time) + \n");
	  fprintf(outfile,"   POS((60/EV(tempo))*(NS(iline->endtime) - \n");
	  fprintf(outfile,"   EV(scorebeats)))));\n\n");
	}
    }
}
