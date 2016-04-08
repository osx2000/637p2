
/*
#    Sfront, a SAOL to C translator    
#    This file: Code generation: declarations/initializations
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
#include "parser.tab.h"


void makepreamble(void);
void makeprevars(void);   
void makesysinfo(void);    
void corefunctions(void);  
void makebusnames(void);   
void makeglobalvars(void); 
void makeglobaltabs(void); 
void instrvars(int *); 
void instrtabs(int *); 
void effvars(int *);   
void efftabs(int *);    
void makefexterns(void);
void printinstrvars(void); 
void make_ninstr(int *, int *); 
void libfunctions(void);
void eprfunction(void);
void trueconstants(void); 
void makeengineinit(void);
void printreflection(void);
void audiolibrary(void);   
void controllibrary(void); 

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*                       void preamble()                        */
/*                                                              */
/* This function prints out the start of the sa.c file --       */
/* everything up to the code generation for opcodes. Some       */
/* of the functions it calls are defined as externs above.      */
/*                                                              */
/*______________________________________________________________*/


void preamble(void)

{
  int mvar = 1;     /* sets size of instr variable array       */
  int mtab = 1;     /* sets size of instr table array          */

  makepreamble();   /* generates lib/csrc/preamble.c constants */
  makeprevars();    /* externs, ... that must precede engine  */
 
  makesysinfo();    /* all constants and data structures for   */
                    /* sound engine that are known early on    */

  corefunctions();  /* defs and vars for library functions     */

  makebusnames();   /* prints out audio bus structures         */

  makeglobalvars(); /* prints out global variable structures   */
                    /* including many of the MIDI variables    */
  makeglobaltabs(); /* prints out the global tables            */

  instrvars(&mvar); /* symbol variables for non-effects instrs */
  instrtabs(&mtab); /* symbol tables for effects instrs        */
  effvars(&mvar);   /* symbol variables for non-effects instrs */
  efftabs(&mtab);   /* symbol tables for effects instrs        */
  printinstrvars(); /* print all instr variables and defines   */
  make_ninstr(&mvar,&mtab); /* create the instr variable stack */

  makefexterns();   /* all extern function headers             */
  trueconstants();  /* program constant arrays for SASL, etc   */

  makeengineinit();  /* function to initialize engine vars    */
  printreflection(); /* C reflection interface of SAOL program */
  libfunctions();    /* functions and constants: opcodes/tables */
  audiolibrary();    /* include -aout and -ain libraries        */
  controllibrary();  /* include -cin libraries                  */
  eprfunction();     /* error-logging function */
}


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*                                                              */
/* The second-level functions called by preamble. Some helper   */
/* functions are defined about the function itself, others      */
/* defined in a utility function section that ends the file.    */
/*                                                              */
/*______________________________________________________________*/


/*****************************************************************/
/*       this function is defined in the utility section.        */
/*       it creates typedefs/defines for control drivers         */
/*****************************************************************/

extern void enginedefines(void);
extern void printcdriverstructs(void);

/****************************************************************/
/*   items that must precede the engine (C global variables)   */
/****************************************************************/

void makeprevars(void) 

{
  int i;

  /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
  /* macros and defines for reentrant operation               */
  /*__________________________________________________________*/

  enginedefines();

  /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
  /* for control driver (since typedefs must precede engine) */
  /*__________________________________________________________*/

  if (cin || session || adriver_reflection(ain))
    printcdriverstructs();

  /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
  /* sfront argc/argv: true global constants, not in engine  */
  /*__________________________________________________________*/
  
  fprintf(outfile,"\nint csys_sfront_argc = %i;\n", sfront_argc);
  fprintf(outfile,"\nchar * csys_sfront_argv[%i] = {\n", sfront_argc);

  i = 0;
  while (i < sfront_argc)
    {
      fprintf(outfile,"\"%s\"", sfront_argv[i++]);
      if (i < sfront_argc)
	fprintf(outfile,",\n");
      else
	fprintf(outfile,"};\n\n\n");
    }

}

/****************************************************************/
/*  constants and datastructures of sound engine (known early)  */
/****************************************************************/

void makesysinfo(void) 

{
  sigsym * sptr;
  float minlatency, maxlatency, defaultlatency;

  /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
  /* simulation engine state variables  */
  /*____________________________________*/

  if (reentrant)
    fprintf(outfile,"\ntypedef struct engine {\n\n");
  else
    fprintf(outfile,"\n/* global engine begins here */\n\n");

  fprintf(outfile,"/* audio and control rates */\n\n");
  fprintf(outfile,"float globaltune;\n");
  fprintf(outfile,"float invglobaltune;\n");
  fprintf(outfile,"float tempo;\n");
  fprintf(outfile,"float scoremult;\n");
  fprintf(outfile,"float scorebeats;          /* current score beat */\n");
  fprintf(outfile,"float absolutetime;        /* current absolute time */\n");
  fprintf(outfile,"int kbase;                 /* kcycle of last tempo change */\n");
  fprintf(outfile,"float scorebase;           /* scorebeat of last tempo change */\n");
  fprintf(outfile, "\n/* counters & bounds acycles and kcycles */\n\n");
  fprintf(outfile,"int endkcycle;\n");
  fprintf(outfile,"int kcycleidx;\n");
  fprintf(outfile,"int acycleidx;\n");
  fprintf(outfile,"int pass;\n");
  fprintf(outfile,"int beginflag;\n");
  fprintf(outfile,"sig_atomic_t graceful_exit;\n\n");

  fprintf(outfile,"int nextstate;     /* counter for active instrument state */\n");
  fprintf(outfile,"int oldstate;      /* detects loops in nextstate updates  */\n");
  fprintf(outfile,"float cpuload;     /* current cpu-time value              */\n\n");

  /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
  /* application ID, network status  */
  /*_________________________________*/

  fprintf(outfile,"#define APPNAME \"sfront\"\n");
  fprintf(outfile,"#define APPVERSION \"%s\"\n", IDSTRING);
  fprintf(outfile,"#define NSYS_NET %i\n", session ? 1 : 0);

  /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
  /* all the temporal constants    */
  /*_______________________________*/

  fprintf(outfile,"#define INTERP_LINEAR 0\n");
  fprintf(outfile,"#define INTERP_SINC 1\n");

  if (interp == INTERP_LINEAR)
    fprintf(outfile,"#define INTERP_TYPE INTERP_LINEAR\n");
  if (interp == INTERP_SINC)
    fprintf(outfile,"#define INTERP_TYPE INTERP_SINC\n");

  fprintf(outfile,"\n#define SAOL_SRATE %i.0F\n", srate);
  fprintf(outfile,"#define SAOL_KRATE %i.0F\n\n", saol_krate);

  if (reentrant)
    {
      fprintf(outfile,"float ARATE;       /* audio sample rate                   */\n");
      fprintf(outfile,"float ATIME;       /* audio sampling period               */\n");
      fprintf(outfile,"unsigned int ACYCLE;  /* number of samples in an a-cycle  */\n\n");
      fprintf(outfile,"float KRATE;       /* control sample rate                 */\n");
      fprintf(outfile,"float KTIME;       /* control sampling period             */\n");
      fprintf(outfile,"float KMTIME;      /* control sampling period (ms)        */\n");
      fprintf(outfile,"int KUTIME;        /* control sampling period (us)        */\n\n");
    }
  else
    {
      fprintf(outfile,"#define ARATE %i.0F\n", srate);
      fprintf(outfile,"#define ATIME %eF\n", 1.0F/srate);
      fprintf(outfile,"#define ACYCLE %i\n\n", srate/krate);
      fprintf(outfile,"#define KRATE %i.0F\n", krate);
      fprintf(outfile,"#define KTIME %eF\n", 1.0F/krate);
      fprintf(outfile,"#define KMTIME %eF\n", 1000.0F/krate);
      fprintf(outfile,"#define KUTIME %i\n", (int)((1000000.0F/krate)+0.5F));
    }

  /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
  /* symbols for audio I/O         */
  /*_______________________________*/

  fprintf(outfile,"#define ASYS_RENDER   0\n");
  fprintf(outfile,"#define ASYS_PLAYBACK 1\n");
  fprintf(outfile,"#define ASYS_TIMESYNC 2\n\n");
  fprintf(outfile,"#define ASYS_SHORT   0\n");
  fprintf(outfile,"#define ASYS_FLOAT   1\n\n");

  /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
  /* Identifying CF Strings for AudioUnits */
  /*---------------------------------------*/

  if (aout && (!strcmp(aoutname,"audiounit") || !strcmp(aoutname,"audiounit_debug")))
    {
      fprintf(outfile,"#define ASYS_AUDIOUNIT_AU_BUNDLECF CFSTR(\"%s.audiounit.%s\")\n",
	      au_manu_url, au_filesystem_name);

      if (au_view_bundlename && au_view_baseclass)
	{
	  fprintf(outfile,"#define ASYS_AUDIOUNIT_VIEW_BUNDLECF CFSTR(\"%s\")\n",
		  au_view_bundlename);
	  fprintf(outfile,"#define ASYS_AUDIOUNIT_VIEW_BASECLASSCF CFSTR(\"%s\")\n",
		  au_view_baseclass);
	}

      /* specify support for AU properties shown in generic UI*/
      fprintf(outfile,"\n");

      fprintf(outfile,"#define ASYS_AUDIOUNIT_SUPPORT_PROPERTY_RENDERQUALITY  (0)\n");

      fprintf(outfile,"#define ASYS_AUDIOUNIT_SUPPORT_PROPERTY_STREAMFROMDISK (0)\n");

      fprintf(outfile,"#define ASYS_AUDIOUNIT_SUPPORT_PROPERTY_CPULOAD (%i)\n",
	      has.s_cpuload ? 1 : 0);

      fprintf(outfile, "\n");
    }

  /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
  /* the audio output variables    */
  /*_______________________________*/

  fprintf(outfile,"/* audio i/o */\n\n");
  fprintf(outfile,"#define ASYS_OCHAN %i\n",outputbus->width);

  switch (makeaudiotypeout(aout)) {
  case SAMPLE_SHORT:
    fprintf(outfile,"#define ASYS_OTYPENAME  ASYS_SHORT\n");
    fprintf(outfile,"#define ASYS_OTYPE  short\n");
    break;
  case SAMPLE_FLOAT:
    fprintf(outfile,"#define ASYS_OTYPENAME  ASYS_FLOAT\n");
    fprintf(outfile,"#define ASYS_OTYPE  float\n");
    break;
  }

  if ((ainflow == PASSIVE_FLOW) && (aoutflow == PASSIVE_FLOW))
    {
      fprintf(outfile,"\nint asys_osize;\n");
      fprintf(outfile,"int obusidx;\n\n");
      fprintf(outfile,"ASYS_OTYPE * asys_obuf;\n\n");
    }
  else
    {
      fprintf(outfile,"\nint asys_exit_status;\n\n");
    }

  /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
  /* the audio input variables     */
  /*_______________________________*/

  sptr = getvsym(&busnametable,"input_bus");
  if ((sptr != NULL) && (inchannels > 0))
    {
      fprintf(outfile,"#define ASYS_ICHAN %i\n",inchannels);

      switch (makeaudiotypein(ain)) {
      case SAMPLE_SHORT:
	fprintf(outfile,"#define ASYS_ITYPENAME  ASYS_SHORT\n");
	fprintf(outfile,"#define ASYS_ITYPE  short\n");
	break;
      case SAMPLE_FLOAT:
	fprintf(outfile,"#define ASYS_ITYPENAME  ASYS_FLOAT\n");
	fprintf(outfile,"#define ASYS_ITYPE  float\n");
	break;
      }

      if ((ainflow == PASSIVE_FLOW) && (aoutflow == PASSIVE_FLOW))
	{
	  fprintf(outfile,"int asys_isize;\n");
	  fprintf(outfile,"int ibusidx;\n\n");
	  fprintf(outfile,"ASYS_ITYPE * asys_ibuf;\n\n");
	}
    }

  /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
  /*   active audio driver setup   */
  /*_______________________________*/

  if ((ainflow == ACTIVE_FLOW) || (aoutflow == ACTIVE_FLOW))
    {
      if ((sptr != NULL) && (inchannels > 0))
	fprintf(outfile, "#define ASYS_ACTIVE_IO\n\n");
      else
	fprintf(outfile, "#define ASYS_ACTIVE_O\n\n"); 
    }

  /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
  /*   latency and timeoption      */
  /*_______________________________*/

  /* print out latency as number of samples */
  /* will always be a power of 2 */

  fprintf(outfile,"#define ASYS_USERLATENCY  %i\n", (latency != -1));
  fprintf(outfile,"#define ASYS_LOWLATENCY   0\n");
  fprintf(outfile,"#define ASYS_HIGHLATENCY  1\n");

  if ((timeoptions == TIMESYNC) || (clatency == LOW_LATENCY_DRIVER) ||
      (ainlatency == LOW_LATENCY_DRIVER) || session)
    {
      fprintf(outfile,"#define ASYS_LATENCYTYPE  ASYS_LOWLATENCY\n");
      minlatency = LOW_LATENCY_MIN;
      defaultlatency = LOW_LATENCY_DEFAULT;
      maxlatency = LOW_LATENCY_MAX;
    }
  else
    {
      fprintf(outfile,"#define ASYS_LATENCYTYPE  ASYS_HIGHLATENCY\n");
      minlatency = HIGH_LATENCY_MIN;
      defaultlatency = HIGH_LATENCY_DEFAULT;
      maxlatency = HIGH_LATENCY_MAX;
    }

  if (latency == -1)
    latency = defaultlatency;
  else
    {
      if (latency < minlatency)
	{
	  latency = minlatency;
	  printf("Warning: Requested latency too small, using (%fs).\n\n",
		 latency);
	}
      if (latency > maxlatency)
	{	    
	  printf("Warning: Using requested latency (%fs), but\n",
		 latency);
	  printf("its probably too large and will cause artifacts.\n\n");
	}
    }

  fprintf(outfile,"#define ASYS_LATENCY %fF\n", latency);  

  switch(timeoptions) {
  case RENDER:
    fprintf(outfile,"#define ASYS_TIMEOPTION ASYS_RENDER\n\n");
    break;
  case PLAYBACK:
    fprintf(outfile,"#define ASYS_TIMEOPTION ASYS_PLAYBACK\n\n");
    break;
  case TIMESYNC:
    fprintf(outfile,"#define ASYS_TIMEOPTION ASYS_TIMESYNC\n\n");
    break;
  }

  /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
  /*  user-requested output word size      */
  /*_______________________________________*/

  fprintf(outfile,"/* AIF or WAV output file wordsize */\n\n");
  fprintf(outfile,"#define ASYS_OUTFILE_WORDSIZE_8BIT  %i\n", WORDSIZE_8BIT);
  fprintf(outfile,"#define ASYS_OUTFILE_WORDSIZE_16BIT  %i\n", WORDSIZE_16BIT);
  fprintf(outfile,"#define ASYS_OUTFILE_WORDSIZE_24BIT  %i\n", WORDSIZE_24BIT);

  fprintf(outfile,"#define ASYS_OUTFILE_WORDSIZE  %i\n\n", outfile_wordsize);

  /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
  /*  ./sa command-line argc/argv for drivers  */
  /*___________________________________________*/

  fprintf(outfile,"int asys_argc;\n");
  fprintf(outfile,"char ** asys_argv;\n\n");
  
  if (cin)
    {
      fprintf(outfile,"int csys_argc;\n");
      fprintf(outfile,"char ** csys_argv;\n\n");
      fprintf(outfile,"void * csys_state;\n\n");
    }

  /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
  /*  global variables for tsync/psync     */
  /*_______________________________________*/

  if ((makeaoutsync(aout) == 0) && (makeainsync(ain) == 0))
    {
      if (timeoptions == TIMESYNC)  /* from tsync */
	fprintf(outfile,"struct timeval sync_last, sync_this, sync_delay;\n");
      if (timeoptions == PLAYBACK)  /* from psync */
	fprintf(outfile,"struct timeval sync_last, sync_this;\n");
    }

  /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
  /*  params and audiobifs state variables */
  /*_______________________________________*/

  if ((cin == 0) && (session == 0))
    {
      if (has.s_params)
	fprintf(outfile,"float params[128];\n\n");
    }
  else
    {
      fprintf(outfile,"float position[3];\n");
      fprintf(outfile,"float direction[3];\n");
      fprintf(outfile,"float listenerPosition[3];\n");
      fprintf(outfile,"float listenerDirection[3];\n");
      fprintf(outfile,"float minFront;\n");
      fprintf(outfile,"float maxFront;\n");
      fprintf(outfile,"float minBack;\n");
      fprintf(outfile,"float maxBack;\n");
      fprintf(outfile,"float params[128];\n\n");
    }

  /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
  /* maximum depth of any oparray chain         */
  /*____________________________________________*/

  if (maxoparraydepth)
    fprintf(outfile,"#define MAXOPARRAY %i\n", maxoparraydepth);
    
}

/****************************************************************/
/*    hand-coded functions for core opcode/gens . Should be a   */
/*    library eventually.                                       */
/****************************************************************/

void corefunctions(void)

{
  int i, j, tsize;

  if (has.o_grain)
    {
      fprintf(outfile,"\n/* for grain core opcode */\n\n");
      fprintf(outfile,"#define GRLOSCIL 0\n");
      fprintf(outfile,"#define GRDOSCIL 1\n");
      fprintf(outfile,"#define GROSCIL  2\n");
      fprintf(outfile,"#define GRTIME   0\n");
      fprintf(outfile,"#define GRPHASE  1\n");
      fprintf(outfile,"#define GRFREQ   2\n");
      fprintf(outfile,"#define GRAMP    3\n");
      fprintf(outfile,"#define GRDUR    4\n");
      fprintf(outfile,"#define GRINVDUR 5\n");
      fprintf(outfile,"#define GRSTATE  6\n");
      fprintf(outfile,"#define GRNUM   256\n\n");
    }

  if (has.o_frac)
    {
      fprintf(outfile,"double fracdummy;\n\n");
    }

  if (has.o_flange)
    {
      fprintf(outfile,"#define FLNOFF 0.002F\n\n");
    }

  if (has.o_chorus)
    {
      fprintf(outfile,"#define CHROFF 0.020F\n\n");
    }

  if (has.w_sample)
    {      
      fprintf(outfile,"FILE * sfile;\n\n");
      /* add rfseek later ... */
    }

  if (has.o_buzz)
    {
      tsize = 2048;
      fprintf(outfile,"#define TRIGSIZE %i\n",tsize);
      fprintf(outfile,"#define TRIGQUAD %i\n",tsize/4);
      fprintf(outfile,"#define TRIGSIZEF %i.0F\n",tsize);
      fprintf(outfile,"#define BUZZDIVISOR 0.02F\n");
      fprintf(outfile,"#define BUZZMINVAL 0.995F\n");
      fprintf(outfile,"#define BUZZMAXVAL 1.005F\n\n");
      fprintf(outfile,"#define TSIN(x) sintab[(int)(x)]\n");
      fprintf(outfile,"#define TCOS(x) sintab[(int)(x)+TRIGQUAD]\n\n");
    }

  if (interp == INTERP_SINC) 
    {
      fprintf(outfile,"\n/* Note: Re-run sfront to change the sinc filter. */\n\n");
      fprintf(outfile,"#define SINC_UPMAX %fF\n", sinc_upmax);
      fprintf(outfile,"#define SINC_PILEN %uU\n", sinc_pilen);
      fprintf(outfile,"#define SINC_ZCROSS %uU\n", sinc_zcross);
      fprintf(outfile,"#define SINC_SIZE %uU\n", 1 + sinc_zcross*sinc_pilen);

      j = 1;
      i = 0;
      while (i < 32)
	{
	  if (j == sinc_pilen)
	    {
	      fprintf(outfile,"#define SINC_LOG_PILEN %iU\n", i);
	      break;
	    }
	  if ((++i) == 32)  /* Should never happen */
	    {
	      printf("Error: SINC_PILEN not a power of 2.\n");
	      noerrorplace();
	    }
	  j = j << 1;
	}
      fprintf(outfile,"\n");
    }

}

/****************************************************************/
/*  this isn't called in preamble, but in writeorc, since the   */
/*  tables are huge. for fft/ifft, we only print those we need. */
/****************************************************************/

void postcorefunctions(void)

{
  int i, tsize, ds, dc, j, tab;

  if (has.o_fft || has.o_ifft)
    {
      tsize = 4;
      tab = 2;
      while (tsize <= 8192)   /* must match constant in libexterns */
	{
	  if (ffttables[tab])
	    {
	      fprintf(outfile,"float fft%itab[%i] = { \n",tsize,3*(tsize>>2));
	      for (i=0;i<3*(tsize>>2);i++)
		{
		  fprintf(outfile,"%10eF", cos((6.283185/tsize)*i));
		  if (i != (3*(tsize>>2)-1))
		    fprintf(outfile,", ");
		  if ((i % 8) == 0)
		    fprintf(outfile,"\n");
		}
	      fprintf(outfile,"};\n\n");
	      
	      fprintf(outfile,"int fft%imap[%i] = { \n", tsize, tsize);
	      for (i=0;i< tsize;i++)
		{
		  j=0;
		  ds = tsize >> 1;
		  dc = 1;
		  while (ds)
		    {
		      j |= (ds&i) ? dc : 0;
		      ds = ds >> 1;
		      dc = dc << 1;
		    }	      
		  fprintf(outfile,"%i", j);
		  if (i != tsize - 1)
		    fprintf(outfile,", ");
		  if ((i % 8) == 0)
		    fprintf(outfile,"\n");
		}
	      fprintf(outfile,"};\n\n");
	    }

	  tab++;
	  tsize *= 2;
	}
    }

  if (has.o_buzz)
    {
      tsize = 2048;   /* must match constant in corefunctions */
      fprintf(outfile,"float sintab[TRIGSIZE+TRIGQUAD] = { \n");
      for (i=0;i<tsize;i++)
	{
	  fprintf(outfile,"%10eF", sin((6.283185/tsize)*i));
	  fprintf(outfile,", ");
	  if ((i % 8) == 0)
	    fprintf(outfile,"\n");
	}
      for (i=0;i<(tsize/4);i++)
	{
	  fprintf(outfile,"%10eF", sin((6.283185/tsize)*i));
	  if (i != ((tsize/4)-1))
	    fprintf(outfile,", ");
	  if ((i % 8) == 0)
	    fprintf(outfile,"\n");
	}
      fprintf(outfile,"};\n\n");
    }

  if (interp == INTERP_SINC) 
    {
      if (has.o_doscil || has.o_oscil ||has.o_koscil || has.o_loscil ||
	  has.o_pluck || has.o_fracdelay || has.o_flange || has.o_chorus ||
	  has.o_tableread)
	{
	  fprintf(outfile,"float sinc[SINC_SIZE] = {1.0F");
	  for (i = 1; i <= sinc_zcross*sinc_pilen; i++)
	    {
	      if (i % sinc_pilen)
		fprintf(outfile, ", %.9gF", (sin((PI*i)/sinc_pilen)/((PI*i)/sinc_pilen))*
			(0.54+0.46*cos((PI*i)/(sinc_pilen*sinc_zcross))));
	      else
		fprintf(outfile,", 0.0F");
	      
	      if ((i % 8) == 0)
		fprintf(outfile,"\n");
	    }
	  fprintf(outfile,"};\n");
	}
    }

}

/****************************************************************/
/*            prints out the audio bus structures               */
/****************************************************************/

void makebusnames(void) 

{
  sigsym * sptr;
  int i;

  sptr = busnametable;
  i = 0;
  while (sptr != NULL)
    {
      fprintf(outfile,"#define BUS_%s %i\n",sptr->val,i);
      i = i + sptr->width;
      if ((sptr == outputbus) || !strcmp(sptr->val, "input_bus"))
	fprintf(outfile,"#define ENDBUS_%s %i\n",sptr->val,i);
      sptr = sptr->next;
    }
  fprintf(outfile,"#define ENDBUS %i\n\n",i);
  fprintf(outfile,"float bus[ENDBUS];\n");
  
  if (useshadowbus)
    fprintf(outfile,"float sbus[ENDBUS];\n");   /* shadow bus */

  fprintf(outfile,"\n");
}

/*****************************************************************/
/* this function is defined in the utility section; its prints   */
/*    the symbol defines of opcode variables and decendents.     */
/*****************************************************************/

extern void opcodedefines(tnode * optr, char * prefix, 
			  int * i, int * j);


/****************************************************************/
/*      prints out the global variables + some MIDI variables   */
/****************************************************************/

void makeglobalvars(void) 

{  
  int i, j;
  sigsym * sptr;
  sigsym * bptr;

  /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
  /*  we first print out consts    */
  /*  for every global signal      */
  /*  variable, leaving space      */
  /*  at the top for MIDI vars.    */
  /*_______________________________*/


  /* calculate MIDI space, and define midimasterchannel space */

  totmidichan = confmidi->midinumchan + sstrmidi->midinumchan;
  if (cmidi || session)
    {
      fprintf(outfile,"#define CSYS_EXTCHANSTART %i\n\n",totmidichan);
      fprintf(outfile,"int midimasterchannel;\n\n");
      totmidichan += cmaxchan;
    }
  else
    {
      if (totmidichan)
	fprintf(outfile,"int midimasterchannel;\n\n");
      else
	fprintf(outfile,"float fakeMIDIctrl[256];\n\n");
    }

  i = totmidichan*MIDIFRAMELEN;

  if (i>0)
    fprintf(outfile,"/* first %i locations for MIDI */\n",i);
  fprintf(outfile,"#define GBL_STARTVAR %i\n",i);

  /* actually print out global signal variable consts */

  currinputwidth = inchannels;
  currinstrwidth = outchannels;

  sptr = globalsymtable;
  while (sptr != NULL)
    {
      fprintf(outfile,"#define GBL_%s %i\n",sptr->val,i);

      /* tables done during TBL_GBL */

      if (csasl && (sptr->vartype!=TABLETYPE))
	{
	  if (bitfile && bitsymin)
	    {
	      if ((bptr = getvsym(&bitsymin,sptr->val)))
		fprintf(outfile,"#define CSYS_SASL_GBL_%s %i\n",
			bptr->defnode->val, i);
	    }
	  else
	    fprintf(outfile,"#define CSYS_SASL_GBL_%s %i\n",sptr->val,i);
	}

      i = i + truewidth(sptr->width);
      sptr = sptr->next;
    }

  /* print out signal variable consts for opcodes called in tables */

  j = 1;
  currinstrument = NULL;
  opcodedefines(globalopcodecalls,"GBL",&i,&j);
  fprintf(outfile,"#define GBL_ENDVAR %i\n",i);


  /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
  /* We then print out the         */
  /* initialized constant          */
  /* space for globals. Note       */
  /* only MIDI has non-zero        */
  /* values now, but startup       */
  /* could be executed in sfront   */
  /* to provide additional inits.  */
  /*_______________________________*/

  fprintf(outfile,"\n/* SAOL global block variables */\n\n");

  fprintf(outfile, "dstack global[GBL_ENDVAR+1];\n\n");

  if (maxoparraydepth)
    {  
      fprintf(outfile,"dstack * globalvstate[MAXOPARRAY];\n");
      fprintf(outfile,"tableinfo * globaltstate[MAXOPARRAY];\n");
    }

}

/*****************************************************************/
/* this function is defined in the utilities section; it prints  */
/* the symbol defines for tables of an opcode and its decendents.*/
/*****************************************************************/

void opcodetabledefines(tnode * optr, char * prefix, 
			int * i, int * j);


/****************************************************************/
/*              prints out the global tables                    */
/****************************************************************/

void makeglobaltabs(void) 

{  
  int i, j;
  sigsym * sptr;
  sigsym * bptr;

  sptr = globalsymtable;
  i = 0;
  while (sptr != NULL)
    {
      if (sptr->vartype == TABLETYPE)
	{
	  fprintf(outfile,"#define TBL_GBL_%s %i\n",sptr->val,i);
	  if (csasl)
	    {
	      if (bitfile && bitsymin)
		{
		  bptr = getvsym(&bitsymin,sptr->val);

		  /* bptr is null for synthetic global tables */
		  
		  if (bptr)
		    fprintf(outfile,"#define CSYS_SASL_GBL_%s %i\n",
			    bptr->defnode->val, i);
		}
	      else
		if ((sptr->val[0]!='_'))
		  fprintf(outfile,"#define CSYS_SASL_GBL_%s %i\n",
			  sptr->val,i);
	    }
	  i++;  
	}
      sptr = sptr->next;
    }
  j = 1;
  opcodetabledefines(globalopcodecalls,"GBL",&i,&j);
  fprintf(outfile,"#define GBL_ENDTBL %i\n\n",i);
  fprintf(outfile,"tableinfo gtables[GBL_ENDTBL+1];\n\n");


}


/*****************************************************************/
/*       this function is defined in the utility section.        */
/*       it prints out extra symbols for the csys driver         */
/*****************************************************************/

extern void instrdriversymbols(sigsym *, sigsym *, int i);

/****************************************************************/
/*   prints out variable symbols for non-effects instrs         */
/****************************************************************/

void instrvars(int * maxvars) 

{  
  int i, j, k;
  sigsym * sptr;
  sigsym * iptr;

  j = 1;
  iptr = instrnametable;
  while (iptr != NULL)
    {
      if (reachableinstrexeff(iptr))
	{
	  i = 0;
	  currinputwidth = 1;
	  currinstrwidth = iptr->width;
	  currinstrument = iptr;
	  sptr = iptr->defnode->sptr;
	  while (sptr != NULL)
	    {
	      if ((sptr->vartype != TMAPTYPE) &&
		  (sptr->tref->mirror == REQUIRED))
 		{
		  fprintf(outfile,"#define %s_%s %i\n",iptr->val,sptr->val,i);
		  if (csasl)
		    instrdriversymbols(sptr, iptr, i);
		  i += truewidth(sptr->width);
		}
	      sptr = sptr->next;
	    }
	  for (k = 0; k < iptr->cref->ifstate; k++)
	    fprintf(outfile,"#define %s__sym_if%i %i\n", iptr->val, k, i++);
	  opcodedefines(iptr->defnode->optr,iptr->val,&i,&j);
	  j = 1;
	  fprintf(outfile,"#define %s_ENDVAR %i\n\n",iptr->val,i);
	  if (i > (*maxvars))
	    *maxvars = i;
	}
      iptr = iptr->next;
    }
  currinstrument = NULL;

}


/****************************************************************/
/*   prints out table symbols for non-effects instrs         */
/****************************************************************/

void instrtabs(int * maxtables) 

{    
  int i, j;
  sigsym * sptr;
  sigsym * iptr;

  j = 1;
  iptr = instrnametable;
  while (iptr != NULL)
    {
      if (reachableinstrexeff(iptr))
	{
	  sptr = iptr->defnode->sptr;
	  i = 0;
	  while (sptr != NULL)
	    {
	      if (sptr->vartype == TABLETYPE)
		{
		  fprintf(outfile,"#define TBL_%s_%s %i\n",
			  iptr->val,sptr->val,i);
		  i++;
		}
	      sptr = sptr->next;
	    }
	  opcodetabledefines(iptr->defnode->optr,iptr->val,&i,&j);
	  j = 1;
	  fprintf(outfile,"#define %s_ENDTBL %i\n\n",iptr->val,i);
	  if (i > (*maxtables))
	    *maxtables = i;
	}
      iptr = iptr->next;
    }

}

/****************************************************************/
/*   prints out variable symbols for effects instrs         */
/****************************************************************/

void effvars(int * maxvars) 

{

  sigsym * sptr;
  tnode * tptr;
  int i, j, k;

  j = 1;
  tptr = instances;
  while (tptr != NULL)
    {
      i = 0;
      currinputwidth = tptr->inwidth;
      currinstrwidth = tptr->sptr->width;
      currinstrument = tptr->sptr;
      sptr = tptr->sptr->defnode->sptr;
      while (sptr != NULL)
	{	    	      
	  if ((sptr->vartype != TMAPTYPE) &&
	      (sptr->tref->mirror == REQUIRED)) 
	    {
	      fprintf(outfile,"#define %s_%s %i\n",tptr->val,sptr->val,i);
	      i += truewidth(sptr->width);
	    }
	  sptr = sptr->next;
	}
      for (k = 0; k < tptr->sptr->cref->ifstate; k++)
	fprintf(outfile,"#define %s__sym_if%i %i\n", tptr->val, k, i++);
      opcodedefines(tptr->sptr->defnode->optr,tptr->val,&i,&j);
      j = 1;
      fprintf(outfile,"#define %s_ENDVAR %i\n\n",tptr->val,i);
      if (i > (*maxvars))
	*maxvars = i;
      tptr = tptr->next;
    }
  currinstrument = NULL;

}


/****************************************************************/
/*   prints out table symbols for effects instrs         */
/****************************************************************/

void efftabs(int * maxtables) 

{    
  tnode * tptr;
  sigsym * sptr;
  int i, j;

  j = 1;
  tptr = instances;
  while (tptr != NULL)
    {
      sptr = tptr->sptr->defnode->sptr;
      i = 0;
      while (sptr != NULL)
	{
	  if (sptr->vartype == TABLETYPE)
	    {
	      fprintf(outfile,"#define TBL_%s_%s %i\n",tptr->val,sptr->val,i);
	      i++;
	    }
	  sptr = sptr->next;
	}
      opcodetabledefines(tptr->sptr->defnode->optr,tptr->val,&i,&j);
      j = 1;
      fprintf(outfile,"#define %s_ENDTBL %i\n\n",tptr->val,i);
      if (i > (*maxtables))
	*maxtables = i;
      tptr = tptr->next;
    }

}

/*****************************************************************/
/* these functions are defined in the utility section at the     */
/* bottom of this file. they print out function headers so that  */
/*              correct compilation can occur.                   */
/*****************************************************************/

extern void printopexterns(tnode * tptr);
extern void printpassexterns(void);  
extern void printfinputexterns(void);
extern void printwavetableexterns(void);
extern void coreopcodeexterns(void);

/****************************************************************/
/*        prints out all extern function headers                */
/****************************************************************/

void makefexterns(void) 

{  
  sigsym * sptr;
  tnode * tptr;

  if (reentrant)
    fprintf(outfile,"\n} engine;\n\n");
  else
    fprintf(outfile,"\n/* global engine ends here */\n\n");

  /*~~~~~~~~~~~~~~~~~~~~~~*/
  /* score table externs  */
  /*______________________*/

  initscoretableexterns(RELTSTAMP);
  initscoretableexterns(ABSTSTAMP);

  /*-----------------*/
  /* signal handlers */
  /*~~~~~~~~~~~~~~~~~*/

  if (catchsignals)
    {
      fprintf(outfile,"extern void signal_handler(int);\n");
    }
  fprintf(outfile,"extern void sigint_handler(int);\n\n");

  /*------------------------------*/
  /* active audio driver externs  */
  /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

  if (((ainflow == ACTIVE_FLOW) || (aoutflow == ACTIVE_FLOW)) && !nomain) 
    {
      sptr = getvsym(&busnametable,"input_bus");
      
      if ((sptr != NULL) && (inchannels > 0))
	{
	  fprintf(outfile, "extern int asys_iorun(ASYS_ITYPE ibuf[],"
		  " int * isize,\n");
	  fprintf(outfile, "	                  ASYS_OTYPE obuf[],"
		  " int * osize);\n\n");
	}
      else
	{
	  fprintf(outfile, "extern int asys_orun(ASYS_OTYPE obuf[],"
		  " int * osize);\n\n");
	}
    }

  fprintf(outfile,"extern ENGINE_PTR_TYPE system_init(int argc, char **argv,"
	  " float sample_rate);\n");
  fprintf(outfile,"extern void effects_init(ENGINE_PTR_DECLARE);\n");
  fprintf(outfile,"extern void main_initpass(ENGINE_PTR_DECLARE);\n");
  fprintf(outfile,"extern void shut_down(ENGINE_PTR_DECLARE);\n\n");

  fprintf(outfile,"extern void main_apass(ENGINE_PTR_DECLARE);\n");
  fprintf(outfile,"extern int  main_kpass(ENGINE_PTR_DECLARE);\n");
  fprintf(outfile,"extern void main_ipass(ENGINE_PTR_DECLARE);\n");
  fprintf(outfile,"extern void main_control(ENGINE_PTR_DECLARE);\n\n");

  /*-----------------------*/
  /* headers for an instrs */
  /*-----------------------*/

  sptr = instrnametable;
  while (sptr != NULL)
    { 
      if (reachableinstrexeff(sptr))
	{
	  currinstance = NULL;
	  currinstancename = sptr->val;
	  printopexterns(sptr->defnode->optr);
	}
      sptr = sptr->next;
    }

  /*--------------------*/
  /* and effects instrs */
  /*--------------------*/

  tptr = instances;
  while (tptr != NULL)
    {
      currinstance = tptr;
      currinstancename = tptr->val;
      printopexterns(tptr->sptr->defnode->optr);
      tptr = tptr->next;
    }

  /*--------------------------------------*/
  /* and opcodes used in the global block */
  /*--------------------------------------*/
			
  currinstancename = "GBL";
  currinstance = NULL;
  printopexterns(globalopcodecalls);

  /*---------------------------*/
  /* and miscellaneous externs */
  /*---------------------------*/

  printpassexterns();          /* the _{i,k,a}pass functions  */
  printfinputexterns();        /* audio input[] functions     */
  printsaoltables(S_EXPORTS);  /* constant wavetable arrays   */
  printtablecatalog();         /* constant table array ptrs   */
  coreopcodeexterns();         /* core-opcode and interp      */
}

/*****************************************************************/
/*    these functions are defined in the utility section.        */
/*         they print out note data and structures               */
/* for the instrument types, and the reflection data structures  */
/*****************************************************************/

void printinstrlists(int *, int *, int *);
void printcdrivervars(int *);
void printdynamiclists(void);


/****************************************************************/
/*                 print instr declarations                     */
/****************************************************************/

void printinstrvars(void)

{

  int totlines = 1;
  int usesdyn = 0;
  int instrnum = 0;

  initendtime();                     /*    initializes endtime variable   */

  printinstrlists(&totlines, &usesdyn, &instrnum);   /* makes instr lists */

  if (cin || session)
    printcdrivervars(&instrnum); /* makes control driver MIDI/SASL lists */

  initscorecontrol(RELTSTAMP);         /* control, tempo, and table lists */
  initscorecontrol(ABSTSTAMP);
  initscoretempo(RELTSTAMP);
  initscoretempo(ABSTSTAMP);
  initscoretablevars(RELTSTAMP);
  initscoretablevars(ABSTSTAMP);

  /* heuristic for setting amount of note/line space */

  if (usesdyn || cin || session)
    {
      if (MAXDNOTES > 3*totlines)
	{
	  fprintf(outfile,"#define MAXLINES %i\n",MAXDNOTES);
	  fprintf(outfile,"#define MAXSTATE %i\n",MAXDNOTES + 4*totlines);
	}
      else
	{
	  fprintf(outfile,"#define MAXLINES %i\n",4*totlines);
	  fprintf(outfile,"#define MAXSTATE %i\n",4*totlines);
	}
    }
  else
    fprintf(outfile,"#define MAXSTATE %i\n",totlines);

  fprintf(outfile,"\n");

  if (usesdyn)
    printdynamiclists();  /* makes dynamic instrument lists */
}


void printcdriverconstant(void);
void printinstrlistconstant(void);

/****************************************************************/
/* declare and initialize "true constant" lists for instrs, etc */
/****************************************************************/

void trueconstants(void)

{
  printcdriverconstant();

  printinstrlistconstant();

  initscorecontrolconstant(RELTSTAMP); 
  initscorecontrolconstant(ABSTSTAMP);
  initscoretempoconstant(RELTSTAMP);
  initscoretempoconstant(ABSTSTAMP);
  initscoretableconstant(RELTSTAMP); 
  initscoretableconstant(ABSTSTAMP);
}

/****************************************************************/
/*            create the instr variable stack                   */
/****************************************************************/

void make_ninstr(int * maxvars, int * maxtables)

{

  fprintf(outfile,"\n");
  fprintf(outfile,"#define MAXVARSTATE %i\n",*maxvars);
  fprintf(outfile,"#define MAXTABLESTATE %i\n",*maxtables);
  fprintf(outfile,"\n");

  fprintf(outfile,"/* ninstr: used for score, effects, */\n");
  fprintf(outfile,"/* and dynamic instruments          */\n");
  fprintf(outfile,"\n");
  fprintf(outfile,"struct ninstr_types {\n");
  fprintf(outfile,"\n");
  fprintf(outfile,"instr_line * iline; /* pointer to score line */\n");
  fprintf(outfile,"dstack v[MAXVARSTATE];     /* parameters & variables*/\n");
  fprintf(outfile,"tableinfo t[MAXTABLESTATE]; /* tables */\n");

  if (maxoparraydepth)
    {  
      fprintf(outfile,"dstack * vstate[MAXOPARRAY];\n");
      fprintf(outfile,"tableinfo * tstate[MAXOPARRAY];\n");
    }

  fprintf(outfile,"\n");
  fprintf(outfile,"} ninstr[MAXSTATE];\n");
  fprintf(outfile,"\n");

}

/****************************************************************/
/*  library functions used by wavetables, opcodes, etc          */
/****************************************************************/

void libfunctions(void)

{
  /*--------------------------------------------------*/
  /* forward define for epr() error recovery function */
  /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

  fprintf(outfile,"\n/* handles termination in case of error */\n");
  fprintf(outfile,"\n");
  fprintf(outfile,"extern void epr(int linenum, char * filename, char * token,"
	  " char * message);\n");

  /*-------------------------------------*/
  /* function to support control drivers */
  /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

  if ((cin != 0) || (session != 0))
    {
      fprintf(outfile,"\n");  
      fprintf(outfile,"void csys_terminate(char * message)\n");  
      fprintf(outfile,"{\n");
      fprintf(outfile,"   epr(0,NULL,\"%s control driver\", message);\n",cinname);
      fprintf(outfile,"}\n\n");
    }

  /*------------------------------*/
  /* function for robust file I/O */
  /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

  if (makeainrobust(ain) || makeaoutrobust(aout) || has.w_sample)
    makerobust();

  /*-----------------------------------------------------*/
  /* functions for specific opcodes and table generators */
  /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

  if (has.w_window)
    {
      fprintf(outfile,"\n/* for window wavetable generator */\n\n");
      fprintf(outfile,"double modbessel(double x)\n\n");
      fprintf(outfile,"{\n");
      fprintf(outfile,"  double sum,a;\n\n");
      fprintf(outfile,"  sum = a = 1.0;\n");
      fprintf(outfile,"  a *= x*x; sum += a*2.5e-1;\n");
      fprintf(outfile,"  a *= x*x; sum += a*1.5625e-2;\n");
      fprintf(outfile,"  a *= x*x; sum += a*4.340278e-4;\n");
      fprintf(outfile,"  a *= x*x; sum += a*6.781684e-6;\n");
      fprintf(outfile,"  a *= x*x; sum += a*6.781684e-8; \n");
      fprintf(outfile,"  a *= x*x; sum += a*4.709503e-10;\n");
      fprintf(outfile,"  a *= x*x; sum += a*2.402808e-12; \n");
      fprintf(outfile,"  a *= x*x; sum += a*9.385967e-15;\n");
      fprintf(outfile,"  a *= x*x; sum += a*2.896903e-17;\n");
      fprintf(outfile,"  a *= x*x; sum += a*7.242258e-20;\n\n");
      fprintf(outfile,"  return sum;\n");
      fprintf(outfile,"}\n\n");
    }

  if (has.o_speedt)
    makerunspt();

  if (csasl)
    maketgen();
}

/****************************************************************/
/*  error-reporting function definition                         */
/****************************************************************/

void eprfunction(void)

{
  char errname[128];

  /*-----------------------------------*/
  /* the epr() error recovery function */
  /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

  if (wiretap_logging(aout))
    strcpy(errname, "asysn_audiounit_logfile");
  else
    strcpy(errname, "stderr");

  fprintf(outfile,"\n/* handles termination in case of error */\n");
  fprintf(outfile,"\n");
  fprintf(outfile,"void epr(int linenum, char * filename, char * token,"
	  " char * message)\n");
  fprintf(outfile,"\n");
  fprintf(outfile,"{\n");
  fprintf(outfile,"\n");

  if (wiretap_logging(aout))
    {
      fprintf(outfile, "\n#if (ASYS_AUDIOUNIT_DEBUG_LEVEL >= 1)\n");
      fprintf(outfile, 
      "  asysn_audiounit_logfile = fopen(ASYS_AUDIOUNIT_LOGFILE_NAME, \"a\");\n");
    }

  fprintf(outfile,"  fprintf(%s, \"\\nRuntime Error.\\n\");\n", errname);
  fprintf(outfile,"  if (filename != NULL)\n");
  fprintf(outfile,"    fprintf(%s, \"Location: File %%s near line %%i.\\n\","
	  "filename, linenum);\n", errname);
  fprintf(outfile,"  if (token != NULL)\n");
  fprintf(outfile,"    fprintf(%s, \"While executing: %%s.\\n\",token);\n", errname);
  fprintf(outfile,"  if (message != NULL)\n");
  fprintf(outfile,"    fprintf(%s, \"Potential problem: %%s.\\n\",message);\n", 
	  errname);
  fprintf(outfile,"  fprintf(%s, \"Exiting program.\\n\\n\");\n", errname);
  fprintf(outfile,"  exit(-1);\n");

  if (wiretap_logging(aout))
    {
      fprintf(outfile, "  fclose(asysn_audiounit_logfile);\n");
      fprintf(outfile, "#endif /* (ASYS_AUDIOUNIT_DEBUG_LEVEL >= 1) */\n");
    }

  fprintf(outfile,"\n");
  fprintf(outfile,"}\n\n");
}

void printcdriverassign(void);
void printinstrlistassign(void);

/****************************************************************/
/*  creates function that initializes global engine variables  */
/****************************************************************/

void makeengineinit(void)

{
  fprintf(outfile,"\nvoid engine_init(ENGINE_PTR_DECLARE_COMMA float sample_rate)\n{\n");

  if (reentrant)
    {
      fprintf(outfile,"  int int_krate = (int)(SAOL_KRATE);\n\n");
      fprintf(outfile,"  int int_srate = (int)(sample_rate);\n\n");
    }

  fprintf(outfile,"  EV(globaltune) = 440.0F;\n");
  fprintf(outfile,"  EV(invglobaltune) = 2.272727e-03F;\n");
  fprintf(outfile,"  EV(kbase) = 1;\n");
  fprintf(outfile,"  EV(kcycleidx) = 1;\n");
  fprintf(outfile,"  EV(pass) = IPASS;\n");
  fprintf(outfile,"  EV(tempo) = %i.0F;\n", confmidi->miditicks ? 120 : 60);

  if (reentrant)
    {
      fprintf(outfile,"  EV(ARATE) = sample_rate;\n");
      fprintf(outfile,"  EV(ATIME) = 1.0F/sample_rate;\n\n");
      fprintf(outfile,"  if (int_srate %% int_krate)\n");
      fprintf(outfile,"    while (int_srate %% (++int_krate));\n\n");
      fprintf(outfile,"  EV(ACYCLE) = int_srate/int_krate;\n");
      fprintf(outfile,"  EV(KRATE) = (float) int_krate;\n");
      fprintf(outfile,"  EV(KTIME) = 1.0F/EV(KRATE);\n");
      fprintf(outfile,"  EV(KMTIME) = 1000.0F*EV(KTIME);\n");
      fprintf(outfile,"  EV(KUTIME) = (int)(1000000.0F*EV(KTIME) + 0.5F);\n");
    }

  fprintf(outfile,"\n");
  fprintf(outfile,"  EV(scoremult) = EV(KTIME);\n");

  if ((ainflow == ACTIVE_FLOW) || (aoutflow == ACTIVE_FLOW))
    fprintf(outfile,"  EV(asys_exit_status) = ASYS_DONE;\n\n");

  if (cmidi || session)
    fprintf(outfile,"  EV(midimasterchannel) = CSYS_EXTCHANSTART;\n\n");

  /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
  /* score variable initializations */
  /*________________________________*/

  initendtimeassign();                /* initializes endtime variable   */

  printcdriverassign();

  printinstrlistassign();             /* includes dynamic instruments   */

  initscorecontrolassign(RELTSTAMP);  /* control, tempo, and table lists */
  initscorecontrolassign(ABSTSTAMP);

  initscoretempoassign(RELTSTAMP);
  initscoretempoassign(ABSTSTAMP);

  initscoretableassign(RELTSTAMP);
  initscoretableassign(ABSTSTAMP);

  fprintf(outfile,"}\n\n");
}

/*****************************************************************/
/*       these function is defined in the utility section.       */
/*       they print out the audio input and output libraries     */
/*****************************************************************/

extern void makeoutputaudiodriver(sigsym * inputbus);
extern void makeinputaudiodriver(sigsym * inputbus);

/****************************************************************/
/*             loads in appropriate audio library               */
/****************************************************************/

void audiolibrary(void)

{
  sigsym * inputbus;
  int i, j;

  /********************************/
  /* print CDRIVER, ADRIVER names */
  /********************************/

  if (cin)
    {
      fprintf(outfile,"#define CSYS_CDRIVER_");
      for (i=0; i< (int)strlen(cinname); i++)
	fprintf(outfile, "%c", toupper(cinname[i]));
      fprintf(outfile,"\n");
    }

  fprintf(outfile,"#define ASYS_OUTDRIVER_");
  j = 0;
  for (i=0; i< (int)strlen(aoutname); i++)
    if (aoutname[i] == '.')
      {
	j = i + 1;
	break;
      }
  for (i=j; i< (int)strlen(aoutname); i++)
    fprintf(outfile, "%c", toupper(aoutname[i]));
  fprintf(outfile,"\n");

  inputbus = getvsym(&busnametable,"input_bus");
  if ((inputbus != NULL) && (inchannels > 0))
    {
      fprintf(outfile,"#define ASYS_INDRIVER_");
      j = 0;
      for (i=0; i< (int)strlen(ainname); i++)
	if (ainname[i] == '.')
	  {
	    j = i + 1;
	    break;
	  }
      for (i=j; i< (int)strlen(ainname); i++)
	fprintf(outfile, "%c", toupper(ainname[i]));
      fprintf(outfile,"\n");
    }

  /****************************/
  /* include actual libraries */
  /****************************/

  if (((ainflow == PASSIVE_FLOW) && (aoutflow == PASSIVE_FLOW)) || 
      (ain == aout) || (ainflow == ACTIVE_FLOW))
    {
      makeoutputaudiodriver(inputbus);
      makeinputaudiodriver(inputbus);
    }
  else
    {
      /* aout is active flow, define passive input driver first */ 

      makeinputaudiodriver(inputbus);
      makeoutputaudiodriver(inputbus);
    }

  fprintf(outfile,"\n");

  /*********************************************/
  /* include default ksync/ksyncinit if needed */
  /*********************************************/

  if ((makeaoutsync(aout) == 0) && (makeainsync(ain) == 0))
    {
      if (timeoptions == RENDER)
	{
	  fprintf(outfile,"\nfloat ksync() { return 0.0F; }\n\n");
	  fprintf(outfile,"\nvoid ksyncinit() { }\n\n");
	}
      if (timeoptions == TIMESYNC)
	maketsync();
      if (timeoptions == PLAYBACK)
	makepsync();
    }

  fprintf(outfile,"\n");

}

/*****************************************************************/
/*      this function is defined in the utilities section        */
/*    it prints out the control driver reflection interface      */
/*****************************************************************/


/****************************************************************/
/*             loads in appropriate control library             */
/****************************************************************/

void controllibrary(void)

{

  if ((cin == 0) && (session == 0))
    return;

  fprintf(outfile,"#define CSYS_DONE 0\n");
  fprintf(outfile,"#define CSYS_ERROR 1\n\n");
  fprintf(outfile,"#define CSYS_NONE 0\n");

  if (cmidi || session)
    fprintf(outfile,"#define CSYS_MIDIEVENTS 1\n");

  if (csasl)
    fprintf(outfile,"#define CSYS_SASLEVENTS 2\n");

  fprintf(outfile,"#define CSYS_EVENTS 3\n\n");

  if (session)
    {
      fprintf(outfile,"#define NSYS_DONE 0\n");
      fprintf(outfile,"#define NSYS_NONE 0\n");
      fprintf(outfile,"#define NSYS_MIDIEVENTS 1\n");
    }

  if (cmidi || session)
    {
      fprintf(outfile,"#define CSYS_MIDI_NUMCHAN  16\n\n");

      fprintf(outfile,"#define CSYS_MIDI_SPECIAL  0x70u\n");
      fprintf(outfile,"#define CSYS_MIDI_NOOP     0x70u\n");
      fprintf(outfile,"#define CSYS_MIDI_NEWTEMPO 0x71u\n");
      fprintf(outfile,"#define CSYS_MIDI_ENDTIME  0x72u\n");
      fprintf(outfile,"#define CSYS_MIDI_POWERUP  0x73u\n");
      fprintf(outfile,"#define CSYS_MIDI_TSTART   0x7Du\n");
      fprintf(outfile,"#define CSYS_MIDI_MANUEX   0x7Du\n");
      fprintf(outfile,"#define CSYS_MIDI_MVOLUME  0x7Eu\n");
      fprintf(outfile,"#define CSYS_MIDI_GMRESET  0x7Fu\n");
      fprintf(outfile,"#define CSYS_MIDI_NOTEOFF  0x80u\n");
      fprintf(outfile,"#define CSYS_MIDI_NOTEON   0x90u\n");
      fprintf(outfile,"#define CSYS_MIDI_PTOUCH   0xA0u\n");
      fprintf(outfile,"#define CSYS_MIDI_CC       0xB0u\n");
      fprintf(outfile,"#define CSYS_MIDI_PROGRAM  0xC0u\n");
      fprintf(outfile,"#define CSYS_MIDI_CTOUCH   0xD0u\n");
      fprintf(outfile,"#define CSYS_MIDI_WHEEL    0xE0u\n");
      fprintf(outfile,"#define CSYS_MIDI_SYSTEM   0xF0u\n\n");

      fprintf(outfile,"#define CSYS_MIDI_SYSTEM_SYSEX_START  0xF0u\n");
      fprintf(outfile,"#define CSYS_MIDI_SYSTEM_QFRAME       0xF1u\n");
      fprintf(outfile,"#define CSYS_MIDI_SYSTEM_SONG_PP      0xF2u\n"); 
      fprintf(outfile,"#define CSYS_MIDI_SYSTEM_SONG_SELECT  0xF3u\n");
      fprintf(outfile,"#define CSYS_MIDI_SYSTEM_UNUSED1      0xF4u\n");
      fprintf(outfile,"#define CSYS_MIDI_SYSTEM_UNUSED2      0xF5u\n");
      fprintf(outfile,"#define CSYS_MIDI_SYSTEM_TUNE_REQUEST 0xF6u\n");
      fprintf(outfile,"#define CSYS_MIDI_SYSTEM_SYSEX_END    0xF7u\n");
      fprintf(outfile,"#define CSYS_MIDI_SYSTEM_CLOCK        0xF8u\n");
      fprintf(outfile,"#define CSYS_MIDI_SYSTEM_TICK         0xF9u\n");
      fprintf(outfile,"#define CSYS_MIDI_SYSTEM_START        0xFAu\n");
      fprintf(outfile,"#define CSYS_MIDI_SYSTEM_CONTINUE     0xFBu\n");
      fprintf(outfile,"#define CSYS_MIDI_SYSTEM_STOP         0xFCu\n");
      fprintf(outfile,"#define CSYS_MIDI_SYSTEM_UNUSED3      0xFDu\n");
      fprintf(outfile,"#define CSYS_MIDI_SYSTEM_SENSE        0xFEu\n");
      fprintf(outfile,"#define CSYS_MIDI_SYSTEM_RESET        0xFFu\n\n");
      
      fprintf(outfile,"#define CSYS_MIDI_CC_BANKSELECT_MSB  0x00u\n"); 
      fprintf(outfile,"#define CSYS_MIDI_CC_MODWHEEL_MSB    0x01u\n");    
      fprintf(outfile,"#define CSYS_MIDI_CC_BREATHCNTRL_MSB 0x02u\n");    
      fprintf(outfile,"#define CSYS_MIDI_CC_FOOTCNTRL_MSB   0x04u\n"); 
      fprintf(outfile,"#define CSYS_MIDI_CC_PORTAMENTO_MSB  0x05u\n");    
      fprintf(outfile,"#define CSYS_MIDI_CC_DATAENTRY_MSB   0x06u\n"); 
      fprintf(outfile,"#define CSYS_MIDI_CC_CHANVOLUME_MSB  0x07u\n");    
      fprintf(outfile,"#define CSYS_MIDI_CC_BALANCE_MSB     0x08u\n");    
      fprintf(outfile,"#define CSYS_MIDI_CC_PAN_MSB         0x0Au\n");    
      fprintf(outfile,"#define CSYS_MIDI_CC_EXPRESSION_MSB  0x0Bu\n");    
      fprintf(outfile,"#define CSYS_MIDI_CC_EFFECT1_MSB     0x0Cu\n");    
      fprintf(outfile,"#define CSYS_MIDI_CC_EFFECT2_MSB     0x0Du\n");    
      fprintf(outfile,"#define CSYS_MIDI_CC_GEN1_MSB        0x10u\n");    
      fprintf(outfile,"#define CSYS_MIDI_CC_GEN2_MSB        0x11u\n");    
      fprintf(outfile,"#define CSYS_MIDI_CC_GEN3_MSB        0x12u\n");   
      fprintf(outfile,"#define CSYS_MIDI_CC_GEN4_MSB        0x13u\n"); 
      fprintf(outfile,"#define CSYS_MIDI_CC_BANKSELECT_LSB  0x20u\n");    
      fprintf(outfile,"#define CSYS_MIDI_CC_MODWHEEL_LSB    0x21u\n");       
      fprintf(outfile,"#define CSYS_MIDI_CC_BREATHCNTRL_LSB 0x22u\n");       
      fprintf(outfile,"#define CSYS_MIDI_CC_FOOTCNTRL_LSB   0x24u\n");    
      fprintf(outfile,"#define CSYS_MIDI_CC_PORTAMENTO_LSB  0x25u\n");       
      fprintf(outfile,"#define CSYS_MIDI_CC_DATAENTRY_LSB   0x26u\n");    
      fprintf(outfile,"#define CSYS_MIDI_CC_CHANVOLUME_LSB  0x27u\n");       
      fprintf(outfile,"#define CSYS_MIDI_CC_BALANCE_LSB     0x28u\n");    
      fprintf(outfile,"#define CSYS_MIDI_CC_PAN_LSB         0x2Au\n");   
      fprintf(outfile,"#define CSYS_MIDI_CC_EXPRESSION_LSB  0x2Bu\n");   
      fprintf(outfile,"#define CSYS_MIDI_CC_EFFECT1_LSB     0x2Cu\n");   
      fprintf(outfile,"#define CSYS_MIDI_CC_EFFECT2_LSB     0x2Du\n");   
      fprintf(outfile,"#define CSYS_MIDI_CC_GEN1_LSB        0x30u\n");   
      fprintf(outfile,"#define CSYS_MIDI_CC_GEN2_LSB        0x31u\n");   
      fprintf(outfile,"#define CSYS_MIDI_CC_GEN3_LSB        0x32u\n");  
      fprintf(outfile,"#define CSYS_MIDI_CC_GEN4_LSB        0x33u\n");
      fprintf(outfile,"#define CSYS_MIDI_CC_SUSTAIN         0x40u\n");        
      fprintf(outfile,"#define CSYS_MIDI_CC_PORTAMENTO      0x41u\n");   
      fprintf(outfile,"#define CSYS_MIDI_CC_SUSTENUTO       0x42u\n"); 
      fprintf(outfile,"#define CSYS_MIDI_CC_SOFTPEDAL       0x43u\n"); 
      fprintf(outfile,"#define CSYS_MIDI_CC_LEGATO          0x44u\n");
      fprintf(outfile,"#define CSYS_MIDI_CC_HOLD2           0x45u\n");
      fprintf(outfile,"#define CSYS_MIDI_CC_SOUNDCONTROL1   0x46u\n");
      fprintf(outfile,"#define CSYS_MIDI_CC_SOUNDCONTROL2   0x47u\n");
      fprintf(outfile,"#define CSYS_MIDI_CC_SOUNDCONTROL3   0x48u\n");
      fprintf(outfile,"#define CSYS_MIDI_CC_SOUNDCONTROL4   0x49u\n");
      fprintf(outfile,"#define CSYS_MIDI_CC_SOUNDCONTROL5   0x4Au\n");
      fprintf(outfile,"#define CSYS_MIDI_CC_SOUNDCONTROL6   0x4Bu\n");
      fprintf(outfile,"#define CSYS_MIDI_CC_SOUNDCONTROL7   0x4Cu\n");
      fprintf(outfile,"#define CSYS_MIDI_CC_SOUNDCONTROL8   0x4Du\n");
      fprintf(outfile,"#define CSYS_MIDI_CC_SOUNDCONTROL9   0x4Eu\n");
      fprintf(outfile,"#define CSYS_MIDI_CC_SOUNDCONTROL10  0x4Fu\n");
      fprintf(outfile,"#define CSYS_MIDI_CC_GEN5            0x50u\n");   
      fprintf(outfile,"#define CSYS_MIDI_CC_GEN6            0x51u\n");   
      fprintf(outfile,"#define CSYS_MIDI_CC_GEN7            0x52u\n");  
      fprintf(outfile,"#define CSYS_MIDI_CC_GEN8            0x53u\n");
      fprintf(outfile,"#define CSYS_MIDI_CC_PORTAMENTOSRC   0x54u\n");
      fprintf(outfile,"#define CSYS_MIDI_CC_EFFECT1DEPTH    0x5Bu\n");
      fprintf(outfile,"#define CSYS_MIDI_CC_EFFECT2DEPTH    0x5Cu\n");
      fprintf(outfile,"#define CSYS_MIDI_CC_EFFECT3DEPTH    0x5Du\n");
      fprintf(outfile,"#define CSYS_MIDI_CC_EFFECT4DEPTH    0x5Eu\n");
      fprintf(outfile,"#define CSYS_MIDI_CC_EFFECT5DEPTH    0x5Fu\n");
      fprintf(outfile,"#define CSYS_MIDI_CC_DATAENTRYPLUS   0x60u\n");
      fprintf(outfile,"#define CSYS_MIDI_CC_DATAENTRYMINUS  0x61u\n");
      fprintf(outfile,"#define CSYS_MIDI_CC_NRPN_LSB        0x62u\n");
      fprintf(outfile,"#define CSYS_MIDI_CC_NRPN_MSB        0x63u\n");
      fprintf(outfile,"#define CSYS_MIDI_CC_RPN_LSB         0x64u\n");
      fprintf(outfile,"#define CSYS_MIDI_CC_RPN_MSB         0x65u\n");
      fprintf(outfile,"#define CSYS_MIDI_CC_ALLSOUNDOFF     0x78u\n");
      fprintf(outfile,"#define CSYS_MIDI_CC_RESETALLCONTROL 0x79u\n");
      fprintf(outfile,"#define CSYS_MIDI_CC_LOCALCONTROL    0x7Au\n");
      fprintf(outfile,"#define CSYS_MIDI_CC_ALLNOTESOFF     0x7Bu\n");
      fprintf(outfile,"#define CSYS_MIDI_CC_OMNI_OFF        0x7Cu\n");
      fprintf(outfile,"#define CSYS_MIDI_CC_OMNI_ON         0x7Du\n");
      fprintf(outfile,"#define CSYS_MIDI_CC_MONOMODE        0x7Eu\n");
      fprintf(outfile,"#define CSYS_MIDI_CC_POLYMODE        0x7Fu\n");
      fprintf(outfile,"\n");
    }

  if (csasl)
    {
      fprintf(outfile,"#define CSYS_SASL_INSTR    0x00\n");
      fprintf(outfile,"#define CSYS_SASL_CONTROL  0x01\n");
      fprintf(outfile,"#define CSYS_SASL_TABLE    0x02\n");
      fprintf(outfile,"#define CSYS_SASL_ENDTIME  0x04\n");
      fprintf(outfile,"#define CSYS_SASL_TEMPO    0x05\n");
      fprintf(outfile,"#define CSYS_SASL_NOOP     0x06\n");
      fprintf(outfile,"\n");
      fprintf(outfile,"#define CSYS_SASL_TGEN_SAMPLE       0x6F\n");
      fprintf(outfile,"#define CSYS_SASL_TGEN_DATA         0x70\n");
      fprintf(outfile,"#define CSYS_SASL_TGEN_RANDOM       0x71\n");
      fprintf(outfile,"#define CSYS_SASL_TGEN_STEP         0x72\n");
      fprintf(outfile,"#define CSYS_SASL_TGEN_LINESEG      0x73\n");
      fprintf(outfile,"#define CSYS_SASL_TGEN_EXPSEG       0x74\n");
      fprintf(outfile,"#define CSYS_SASL_TGEN_CUBICSEG     0x75\n");
      fprintf(outfile,"#define CSYS_SASL_TGEN_POLYNOMIAL   0x76\n");
      fprintf(outfile,"#define CSYS_SASL_TGEN_SPLINE       0x77\n");
      fprintf(outfile,"#define CSYS_SASL_TGEN_WINDOW       0x78\n");
      fprintf(outfile,"#define CSYS_SASL_TGEN_HARM         0x79\n");
      fprintf(outfile,"#define CSYS_SASL_TGEN_HARM_PHASE   0x7A\n");
      fprintf(outfile,"#define CSYS_SASL_TGEN_PERIODIC     0x7B\n");
      fprintf(outfile,"#define CSYS_SASL_TGEN_BUZZ         0x7C\n");
      fprintf(outfile,"#define CSYS_SASL_TGEN_CONCAT       0x7D\n");
      fprintf(outfile,"#define CSYS_SASL_TGEN_EMPTY        0x7E\n");
      fprintf(outfile,"#define CSYS_SASL_TGEN_DESTROY      0x7F\n");
      fprintf(outfile,"\n");

    }

  if (cin)
    makecontroldriver(cin);

  if (session)
    makenetworkdriver();

  fprintf(outfile,"\n");
}


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*                                                              */
/* Utility functions, used by second level preamble() calls     */
/*                                                              */
/*______________________________________________________________*/


/****************************************************************/
/*          print correct defines/macros for engine type        */
/****************************************************************/

void enginedefines(void)

{
  fprintf(outfile, "\n/* defines for a %srentrant sa.c file */\n\n",
	  reentrant ? "" : "non-");

  fprintf(outfile, "#define ENGINE_NONREENTRANT  0\n");  
  fprintf(outfile, "#define ENGINE_REENTRANT     1\n\n");

  if (reentrant)
    {
      fprintf(outfile, "#define ENGINE_STYLE  ENGINE_REENTRANT\n");  
      fprintf(outfile, "#define ENGINE_PTR_TYPE  engine *\n");
      fprintf(outfile, "#define ENGINE_PTR_TYPE_COMMA  engine *,\n");
      fprintf(outfile, "#define ENGINE_PTR_DECLARE  engine * eng\n");
      fprintf(outfile, "#define ENGINE_PTR_DECLARE_COMMA  engine * eng,\n");
      fprintf(outfile, "#define ENGINE_PTR_DECLARE_SEMICOLON  engine * eng;\n");
      fprintf(outfile, "#define ENGINE_PTR  eng\n");
      fprintf(outfile, "#define ENGINE_PTR_COMMA  eng,\n");
      fprintf(outfile, "#define ENGINE_PTR_ASSIGNED_TO  eng = \n");
      fprintf(outfile, "#define ENGINE_PTR_CREATE_SEMICOLON  "
	      "engine * eng = (engine *) calloc(1, sizeof(engine));\n");
      fprintf(outfile, "#define ENGINE_SIZE  (sizeof(engine))\n");
      fprintf(outfile, "#define ENGINE_PTR_NULLRETURN_SEMICOLON  " 
	      "if (!eng) return NULL;\n");
      fprintf(outfile, "#define ENGINE_PTR_IS_NULL  (eng == NULL)\n");
      fprintf(outfile, "#define ENGINE_PTR_IS_NOT_NULL  (eng != NULL)\n");
      fprintf(outfile, "#define ENGINE_PTR_DESTROY_SEMICOLON  free(eng);\n");
      fprintf(outfile, "#define ENGINE_PTR_RETURN_SEMICOLON  return eng;\n");
      fprintf(outfile, "#define EV(x)   eng->x\n");
    }
  else
    {
      fprintf(outfile, "#define ENGINE_STYLE  ENGINE_NONREENTRANT\n");  
      fprintf(outfile, "#define ENGINE_PTR_TYPE  void\n");
      fprintf(outfile, "#define ENGINE_PTR_TYPE_COMMA  \n");
      fprintf(outfile, "#define ENGINE_PTR_DECLARE  void\n");
      fprintf(outfile, "#define ENGINE_PTR_DECLARE_COMMA  \n");
      fprintf(outfile, "#define ENGINE_PTR_DECLARE_SEMICOLON  \n");
      fprintf(outfile, "#define ENGINE_PTR \n");
      fprintf(outfile, "#define ENGINE_PTR_COMMA \n");
      fprintf(outfile, "#define ENGINE_PTR_ASSIGNED_TO  \n");
      fprintf(outfile, "#define ENGINE_PTR_CREATE_SEMICOLON  \n");
      fprintf(outfile, "#define ENGINE_SIZE \n");
      fprintf(outfile, "#define ENGINE_PTR_NULLRETURN_SEMICOLON \n");
      fprintf(outfile, "#define ENGINE_PTR_IS_NULL  (0)\n");
      fprintf(outfile, "#define ENGINE_PTR_IS_NOT_NULL  (1)\n");
      fprintf(outfile, "#define ENGINE_PTR_DESTROY_SEMICOLON \n");
      fprintf(outfile, "#define ENGINE_PTR_RETURN_SEMICOLON  return;\n");
      fprintf(outfile, "#define EV(x)   x\n");
    }

  fprintf(outfile, "\n");
  fprintf(outfile, "#define NV(x)   nstate->v[x].f\n");
  fprintf(outfile, "#define NVI(x)  nstate->v[x].i\n");
  fprintf(outfile, "#define NVUI(x) nstate->v[x].ui\n");
  fprintf(outfile, "#define NVPS(x) nstate->v[x].ps\n");
  fprintf(outfile, "#define NVU(x)  nstate->v[x]\n");
  fprintf(outfile, "#define NT(x)   nstate->t[x]\n");
  fprintf(outfile, "#define NS(x)   nstate->x\n");
  fprintf(outfile, "#define NSP     ENGINE_PTR_COMMA nstate\n");
  fprintf(outfile, "#define NP(x)   nstate->v[x].f\n");
  fprintf(outfile, "#define NPI(x)  nstate->v[x].i\n");
  fprintf(outfile, "#define NPUI(x) nstate->v[x].ui\n\n");
  fprintf(outfile, "#define NPPS(x) nstate->v[x].ps\n\n");

  fprintf(outfile, "#define NG(x)   EV(global)[x].f\n");
  fprintf(outfile, "#define NGI(x)  EV(global)[x].i\n");
  fprintf(outfile, "#define NGUI(x) EV(global)[x].ui\n");
  fprintf(outfile, "#define NGU(x)  EV(global)[x]\n");
  fprintf(outfile, "#define TB(x)   EV(bus[x])\n");
  fprintf(outfile, "#define STB(x)  EV(sbus[x])\n\n");

  fprintf(outfile,"#define MAXPFIELDS %i\n\n", numpfields);

  fprintf(outfile,"typedef struct instr_line { \n\n");
  fprintf(outfile,"float starttime;  /* score start time of note */\n");
  fprintf(outfile,"float endtime;    /* score end time of note */\n");
  fprintf(outfile,"float startabs;   /* absolute start time of note */\n");
  fprintf(outfile,"float endabs;     /* absolute end time of note */\n");
  fprintf(outfile,"float abstime;    /* absolute time extension */\n");
  fprintf(outfile,"float time;       /* time of note start (absolute) */\n");
  fprintf(outfile,"float itime;      /* elapsed time (absolute) */\n");
  fprintf(outfile,"float sdur;       /* duration of note in score time*/\n\n");
  fprintf(outfile,"int kbirth;       /* kcycleidx for note launch */\n");
  fprintf(outfile,"int released;     /* flag for turnoff*/\n");
  fprintf(outfile,"int turnoff;      /* flag for turnoff */\n");
  fprintf(outfile,"int noteon;       /* NOTYETUSED,TOBEPLAYED,PAUSED,PLAYING,ALLDONE */\n");
  fprintf(outfile,"int notestate;    /* index into state array */\n");
  fprintf(outfile,"int launch;       /* only for dynamic instruments */\n");
  fprintf(outfile,"int numchan;      /* only for MIDI notes */\n");
  fprintf(outfile,"int preset;       /* only for MIDI notes */\n");
  fprintf(outfile,"int notenum;      /* only for MIDI notes */\n");
  fprintf(outfile,"int label;        /* SASL label index + 1, or 0 (no label) */\n");
  fprintf(outfile,"                  /* for static MIDI, all-sounds noteoff */\n\n");
  fprintf(outfile,"float p[MAXPFIELDS];  /* parameters */\n\n");
  fprintf(outfile,"struct ninstr_types * nstate; /* pointer into state array */\n\n");

  fprintf(outfile,"} instr_line;\n\n");

  fprintf(outfile,"typedef struct scontrol_lines {\n");
  fprintf(outfile,"\n");
  fprintf(outfile,"float t;                  /* trigger time */\n");
  fprintf(outfile,"int label;                /* index into label array */\n");
  fprintf(outfile,"int siptr;                /* score instr line to control */\n");
  fprintf(outfile,"instr_line * iline;       /* pointer to score line */\n");
  fprintf(outfile,"int imptr;                /* position of variable in v[] */\n");
  fprintf(outfile,"float imval;              /* value to import into v[] */\n");
  fprintf(outfile,"\n");
  fprintf(outfile,"} scontrol_lines;\n\n");

  fprintf(outfile,"typedef struct csys_table_ptr {\n");
  fprintf(outfile,"\n");
  fprintf(outfile,"int num;                  /* number of floats in array */\n");
  fprintf(outfile,"float * t;                /* pointer to array */\n");
  fprintf(outfile,"\n");
  fprintf(outfile,"} csys_table_ptr;\n\n\n");

}

/****************************************************************/
/*   print datastructures for MIDI/SASL control preambles       */
/****************************************************************/

void printcdriverstructs(void)

{
  fprintf(outfile,"#define MAXCNOTES %i\n\n",MAXDNOTES);
  fprintf(outfile,"#define CSYS_INSTRNUM %i\n\n", 
	  numinstrnames ? numinstrnames : 1);

  fprintf(outfile,"#define CSYS_IRATE 0\n");
  fprintf(outfile,"#define CSYS_KRATE 1\n");
  fprintf(outfile,"#define CSYS_ARATE 2\n");
  fprintf(outfile,"#define CSYS_TABLE 3\n\n");

  fprintf(outfile,"#define CSYS_NORMAL 0\n");
  fprintf(outfile,"#define CSYS_IMPORT 1\n");
  fprintf(outfile,"#define CSYS_EXPORT 2\n");
  fprintf(outfile,"#define CSYS_IMPORTEXPORT 3\n");
  fprintf(outfile,"#define CSYS_PFIELD 4\n");
  fprintf(outfile,"#define CSYS_INTERNAL 5\n\n");

  fprintf(outfile,"#define CSYS_UNUSED 0\n");
  fprintf(outfile,"#define CSYS_READ 1\n");
  fprintf(outfile,"#define CSYS_WRITTEN 2\n");
  fprintf(outfile,"#define CSYS_WRITTEN_AND_READ 3\n\n");

  fprintf(outfile,"#define CSYS_STATUS_EFFECTS    1 \n");
  fprintf(outfile,"#define CSYS_STATUS_SCORE      2 \n");
  fprintf(outfile,"#define CSYS_STATUS_MIDI       4 \n");
  fprintf(outfile,"#define CSYS_STATUS_DYNAMIC    8 \n");
  fprintf(outfile,"#define CSYS_STATUS_STARTUP   16 \n\n");

  fprintf(outfile,"typedef struct csys_varstruct {\n");
  fprintf(outfile,"  int index;\n");
  fprintf(outfile,"  char * name;\n");
  fprintf(outfile,"  int token;\n");
  fprintf(outfile,"  int type;\n");
  fprintf(outfile,"  int tag;\n");
  fprintf(outfile,"  int width;\n");
  fprintf(outfile,"  int use;\n");
  fprintf(outfile,"} csys_varstruct;\n\n");

  fprintf(outfile,"typedef struct csys_instrstruct {\n");
  fprintf(outfile,"  int index;\n");
  fprintf(outfile,"  char * name;\n");
  fprintf(outfile,"  int token;\n");
  fprintf(outfile,"  int numvars;\n");
  fprintf(outfile,"  csys_varstruct * vars;\n");
  fprintf(outfile,"  int outwidth;\n");
  fprintf(outfile,"  int status;\n");
  fprintf(outfile,"} csys_instrstruct;\n\n");

  fprintf(outfile,"typedef struct csys_labelstruct {\n");
  fprintf(outfile,"  int index;\n");
  fprintf(outfile,"  char * name;\n");
  fprintf(outfile,"  int token;\n");
  fprintf(outfile,"  int iflag[CSYS_INSTRNUM];\n");
  fprintf(outfile,"} csys_labelstruct;\n\n");

  fprintf(outfile,"typedef struct csys_presetstruct {\n");
  fprintf(outfile,"  int index;\n");
  fprintf(outfile,"  int preset;\n");
  fprintf(outfile,"} csys_presetstruct;\n\n");

  fprintf(outfile,"typedef struct csys_samplestruct {\n");
  fprintf(outfile,"  int index;\n");
  fprintf(outfile,"  int token;\n");
  fprintf(outfile,"  char * name;\n");
  fprintf(outfile,"  char * fname;\n");
  fprintf(outfile,"} csys_samplestruct;\n\n");

  fprintf(outfile,"typedef struct csys_routestruct {\n");
  fprintf(outfile,"  int bus;\n");     
  fprintf(outfile,"  int ninstr;\n");  
  fprintf(outfile,"  int * instr;\n"); 
  fprintf(outfile,"} csys_routestruct;\n\n");

  fprintf(outfile,"typedef struct csys_sendstruct {\n");
  fprintf(outfile,"  int instr;\n");  
  fprintf(outfile,"  int nbus;\n");   
  fprintf(outfile,"  int * bus;\n");  
  fprintf(outfile,"} csys_sendstruct;\n\n");

  fprintf(outfile,"typedef struct csys_busstruct {\n");  
  fprintf(outfile,"  int index;\n");     
  fprintf(outfile,"  char * name;\n");   
  fprintf(outfile,"  int width;\n");     
  fprintf(outfile,"  int oflag;\n");    
  fprintf(outfile,"} csys_busstruct;\n\n");  

  fprintf(outfile,"typedef struct csys_targetstruct {\n");    
  fprintf(outfile,"char * name;\n");    
  fprintf(outfile,"int token;\n");    
  fprintf(outfile,"int numinstr;\n");    
  fprintf(outfile,"int * instrindex;\n");    
  fprintf(outfile,"int * varindex;\n");    
  fprintf(outfile,"} csys_targetstruct;\n\n");  

}


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/* This set of functions prints out the variables and           */
/* table symbols used by opcodes and its decendents. Used       */
/* for global, instr, and effects opcode symbol tables.         */
/*______________________________________________________________*/

/****************************************************************/
/*       set ibus values for oparraycalls -- recursive          */
/****************************************************************/

void opcodearrayidx(tnode * optr, int * j)

{
  int k = 1;

  while (optr != NULL)
    {
      switch (optr->ttype) {
      case S_OPARRAYDECL:
	opcodearrayidx(optr->sptr->defnode->optr,&k);
	k = 1;
	break;
      case S_OPCALL:
	opcodearrayidx(optr->sptr->defnode->optr,&k);
	k = 1;
	optr->arrayidx = *j;
	*j = *j + 1;
	break;
      case S_OPARRAYCALL:
	opcodearrayidx(optr->sptr->defnode->optr,&k);
	k = 1;
	optr->arrayidx = *j;
	*j = *j + 1;
      default:
	break;
      }
      optr = optr->next;
    }

}

extern void oparrayextrastate(tnode * optr, char * prefix,
			      int * i, int opidx, tnode * olist);

/****************************************************************/
/*       prints one set of variables for oparray defines        */
/****************************************************************/

void oparrayslice(tnode * optr, char * prefix,
		  int * i, int opidx, tnode * olist)

{  
  int k = 1;
  int m;
  sigsym * sptr;
  char newprefix[STRSIZE];

  /* does not save memory space for inlined core opcodes */
  /* and for mirrored variables, for consistent alignment  */
  /* note that optr->optr may be NULL (const-optimized)    */

  sptr = optr->sptr->defnode->sptr;
  while (sptr != NULL)
    {
      if (sptr->vartype != TMAPTYPE)
	{
	  fprintf(outfile,"#define %s_%soparray%i_%s %i\n",
		  prefix, optr->val, opidx, sptr->val,*i);
	  if (truewidth(sptr->width) == 0)  
	    {  
	      printf("Error: Zero-width arrays ");
	      printf("not permitted.\n");
	      showerrorplace(sptr->defnode->down->linenum,
			     sptr->defnode->down->filename);
	    }
	  *i = *i + truewidth(sptr->width);
	}	
      sptr = sptr->next;
    }

  if (coreopcodehasextras(optr))
    oparrayextrastate(optr, prefix, i, opidx, olist);

  if (!coreopcodename(optr))
    fprintf(outfile, "#define %s_%soparray%i__sym_opstate %i\n", 
	    prefix, optr->val, opidx, (*i)++);

  for (m = 0; m < optr->sptr->maxifstate; m++)
    fprintf(outfile, "#define %s_%soparray%i__sym_if%i %i\n", 
	    prefix, optr->val, opidx, m, (*i)++);

  sprintf(newprefix,"%s_%soparray%i",prefix,optr->val,opidx);
  opcodedefines(optr->sptr->defnode->optr,newprefix,i,&k);

}

extern int opstate_needed(tnode * optr);

/****************************************************************/
/*   print of #ifdef for opcodes -- recursive                   */
/****************************************************************/

void opcodedefines(tnode * optr, char * prefix,
		   int * i, int * j)

{
  tnode * olist = optr;
  sigsym * sptr;
  char newprefix[STRSIZE];
  int k = 1;
  int opnum, opsize, m;

  while (optr != NULL)
    {
      switch (optr->ttype) {
      case S_OPARRAYDECL:
	curroparraydepth++;
	opsize = 0;		    
	fprintf(outfile,"#define %s_%sopbase %i\n",prefix,optr->val, *i);
	oparrayslice(optr, prefix, &opsize, 0, olist);
	fprintf(outfile,"#define %s_%sopsize %i\n",prefix,optr->val,opsize);
	for (opnum = 0; opnum < truewidth(optr->opwidth);opnum++)
	  oparrayslice(optr, prefix, i, opnum + 1, olist);
	curroparraydepth--;
	break;
      case S_OPCALL:

	/* does not save memory space for inlined core opcodes */
	/* and for mirrored variables, for consistent alignment, */
	/* once we hit the first oparray in the stack.           */
	/* note that optr->optr may be NULL (const-optimized)    */

	if (curroparraydepth || 
	    (optr->optr && (!coreopcodecaninline(optr->optr->down))))
	  {
	    sptr = optr->sptr->defnode->sptr;
	    while (sptr != NULL)
	      {
		if ((sptr->vartype != TMAPTYPE) && 
		    (curroparraydepth || (sptr->tref->mirror == REQUIRED)))
		  {
		    fprintf(outfile,"#define %s_%s%i_%s %i\n",prefix,
			    optr->val,*j,sptr->val,*i);
		    if (truewidth(sptr->width) == 0)  
		      {   
			/* will be caught before this */
			printf("Error: Zero-width arrays not permitted.\n");
			showerrorplace(sptr->defnode->down->linenum,
				       sptr->defnode->down->filename);
		      }
		    *i = *i + truewidth(sptr->width);
		  }
		sptr = sptr->next;
	      }
	    
	    if (opstate_needed(optr))
	      fprintf(outfile,"#define %s_%s%i__sym_opstate %i\n", prefix,
		      optr->val,*j, (*i)++);

	    for (m = 0; m < (curroparraydepth ? optr->sptr->maxifstate 
			     : optr->sptr->cref->ifstate); m++)
	      fprintf(outfile,"#define %s_%s%i__sym_if%i %i\n", prefix,
		      optr->val,*j, m, (*i)++);
	  }

	if (curroparraydepth || (optr->optr && 
				 (!polyopcallexcept(optr->optr->down))
				 && (optr->sptr->cref->callrate > optr->rate)))
	  fprintf(outfile,"#define %s_%s%i__sym_ocstate %i\n",
		  prefix, optr->val, *j, (*i)++);

	fprintf(outfile,"#define %s_%s%i_return %i\n",prefix,optr->val,*j,*i);
	*i = *i + truewidth(optr->width);
	sprintf(newprefix,"%s_%s%i",prefix,optr->val,*j);
	opcodedefines(optr->sptr->defnode->optr,newprefix,i,&k);
	k = 1;
	optr->arrayidx = *j;
	*j = *j + 1;
	break;
      case S_OPARRAYCALL:

	/*  no calls to opcodedefine() --> no curroparraydepth{++,--} */

	if (curroparraydepth || (optr->optr && 
				 (!polyopcallexcept(optr->optr->down))
				 && (optr->sptr->cref->callrate > optr->rate)))
	  fprintf(outfile,"#define %s_%s%i__sym_ocstate %i\n",
		  prefix, optr->val, *j, (*i)++);

	fprintf(outfile,"#define %s_%s%i_return %i\n",prefix,optr->val,*j,*i);
	*i = *i + truewidth(optr->width);
	opcodearrayidx(optr->sptr->defnode->optr,&k);
	k = 1;
	optr->arrayidx = *j;
	*j = *j + 1;
      default:
	break;
      }
      optr = optr->next;
    }

}

extern void oparraytableextrastate(tnode * optr, char * prefix,
				   int * i, int opidx, tnode * olist);

/****************************************************************/
/*       prints one set of tables for oparray defines           */
/****************************************************************/

void oparraytableslice(tnode * optr, char * prefix,
		  int * i, int opidx, tnode * olist)

{    
  int k = 1;
  sigsym * sptr;
  char newprefix[STRSIZE];

  sptr = optr->sptr->defnode->sptr;
  while (sptr != NULL)
    {
      if ((sptr->vartype == TABLETYPE)&&(sptr->kind != K_PFIELD))
	{
	  fprintf(outfile,"#define TBL_%s_%soparray%i_%s %i\n",
		  prefix,optr->val,(opidx),sptr->val,*i);
	  *i = *i + 1;
	}
      sptr = sptr->next;
    }

  if (!strcmp(optr->val, "reverb"))
    oparraytableextrastate(optr, prefix, i, opidx, olist);

  sprintf(newprefix,"%s_%soparray%i",prefix,optr->val,opidx);
  opcodetabledefines(optr->sptr->defnode->optr,newprefix,i,&k);

}

/****************************************************************/
/*   print of #ifdef for opcode tables -- recursive             */
/****************************************************************/

void opcodetabledefines(tnode * optr,char * prefix,
			int * i, int * j)

{
  tnode * olist = optr;
  sigsym * sptr;
  char newprefix[STRSIZE];
  int k = 1;
  int opnum, opsize;

  while (optr != NULL)
    {
      switch (optr->ttype) {
      case S_OPARRAYDECL:

	opsize = 0;		    
	fprintf(outfile,"#define TBL_%s_%sopbase %i\n",prefix,optr->val, *i);
	oparraytableslice(optr, prefix, &opsize, 0, olist);
	fprintf(outfile,"#define TBL_%s_%sopsize %i\n", prefix, optr->val,
		opsize);

	for (opnum = 0; opnum < truewidth(optr->opwidth);opnum++)
	  oparraytableslice(optr, prefix, i, opnum + 1, olist);
	break;
      case S_OPCALL:
	sptr = optr->sptr->defnode->sptr;
	while (sptr != NULL)
	  {
	    if ((sptr->vartype == TABLETYPE)&&(sptr->kind != K_PFIELD))
	      {
		fprintf(outfile,"#define TBL_%s_%s%i_%s %i\n",
			prefix,optr->val, *j,sptr->val,*i);
		*i = *i + 1;
	      }
	    sptr = sptr->next;
	  }
	sprintf(newprefix,"%s_%s%i",prefix,optr->val,*j);
	opcodetabledefines(optr->sptr->defnode->optr,newprefix,i,&k);
	k = 1;
	*j = *j + 1;
	break;
      case S_OPARRAYCALL:
	*j = *j + 1;
      default:
	break;
      }
      optr = optr->next;
    }

}


/****************************************************************/
/*  oparray space for state slots for extra formal parameters   */
/****************************************************************/

void oparrayextrastate(tnode * optr, char * prefix,
		       int * i, int opidx, tnode * olist)

{
  tnode * tptr;
  tnode * maxextra = NULL;
  int maxnum = 0;
  int j;

  /* find oparray call with the most extra parameters */

  while (olist)
    {
      if ((olist->ttype == S_OPARRAYCALL) && !strcmp(olist->val, optr->val))
	{
	  j = 0;
	  tptr = olist->extra;
	  while (tptr)
	    {
	      if (tptr->ttype == S_PARAMDECL)
		j++;
	      tptr = tptr->next;
	    }
	  if (j > maxnum)
	    {
	      maxnum = j;
	      maxextra = olist->extra;
	    }
	}
      olist = olist->next;
    }

  /* if no such oparray call exists, exit */

  if (!maxextra)
    return;

  /* declare memory slots for extra parameters */

  while (maxextra)
    {
      if (maxextra->ttype == S_PARAMDECL)
	{
	  fprintf(outfile,"#define %s_%soparray%i_%s %i\n",
		  prefix, optr->val, opidx, maxextra->down->next->down->val, *i);
	  *i = *i + 1;
	}
      maxextra = maxextra->next;
    }

  /* add extra state variables for a few opcodes */

  if (!strcmp(optr->val, "fir"))
    {
      for (j = 1; j <= maxnum; j++)
	{
	  fprintf(outfile,"#define %s_%soparray%i_z%i %i\n",
		  prefix, optr->val, opidx, j, *i);
	  *i = *i + 1;
	}
      return;
    }

  if (!strcmp(optr->val, "iir"))
    {
      for (j = maxnum/2; j > 0; j--)
	{
	  fprintf(outfile,"#define %s_%soparray%i_d%i %i\n",
		  prefix, optr->val, opidx, j, *i);
	  *i = *i + 1;
	}
      return;
    }

  if (!strcmp(optr->val, "reverb"))
    {
      fprintf(outfile,"#define %s_%soparray%i_d2_0 %i\n",
	      prefix, optr->val, opidx, *i);
      *i = *i + 1;

      fprintf(outfile,"#define %s_%soparray%i_d1_0 %i\n",
	      prefix, optr->val, opidx, *i);
      *i = *i + 1;

      fprintf(outfile,"#define %s_%soparray%i_b0_0 %i\n",
	      prefix, optr->val, opidx, *i);
      *i = *i + 1;

      fprintf(outfile,"#define %s_%soparray%i_b1_0 %i\n",
	      prefix, optr->val, opidx, *i);
      *i = *i + 1;

      fprintf(outfile,"#define %s_%soparray%i_b2_0 %i\n",
	      prefix, optr->val, opidx, *i);
      *i = *i + 1;

      fprintf(outfile,"#define %s_%soparray%i_a1_0 %i\n",
	      prefix, optr->val, opidx, *i);
      *i = *i + 1;

      fprintf(outfile,"#define %s_%soparray%i_a2_0 %i\n",
	      prefix, optr->val, opidx, *i);
      *i = *i + 1;

      maxnum--;   /* subtract for f0 */

      j = maxnum/2;  /* number of additional (f,r) pairs */

      while (j > 0)
	{
	  fprintf(outfile,"#define %s_%soparray%i_dline%i_0 %i\n",
		  prefix, optr->val, opidx, j, *i);
	  *i = *i + 1;

	  fprintf(outfile,"#define %s_%soparray%i_dline%i_1 %i\n",
		  prefix, optr->val, opidx, j, *i);
	  *i = *i + 1;

	  fprintf(outfile,"#define %s_%soparray%i_dline%i_2 %i\n",
		  prefix, optr->val, opidx, j, *i);
	  *i = *i + 1;

	  fprintf(outfile,"#define %s_%soparray%i_dline%i_3 %i\n",
		  prefix, optr->val, opidx, j, *i);
	  *i = *i + 1;

	  fprintf(outfile,"#define %s_%soparray%i_g%i_0 %i\n",
		  prefix, optr->val, opidx, j, *i);
	  *i = *i + 1;

	  fprintf(outfile,"#define %s_%soparray%i_g%i_1 %i\n",
		  prefix, optr->val, opidx, j, *i);
	  *i = *i + 1;

	  fprintf(outfile,"#define %s_%soparray%i_g%i_2 %i\n",
		  prefix, optr->val, opidx, j, *i);
	  *i = *i + 1;

	  fprintf(outfile,"#define %s_%soparray%i_g%i_3 %i\n",
		  prefix, optr->val, opidx, j, *i);
	  *i = *i + 1;

	  fprintf(outfile,"#define %s_%soparray%i_d2_%i %i\n",
		  prefix, optr->val, opidx, j, *i);
	  *i = *i + 1;
	  
	  fprintf(outfile,"#define %s_%soparray%i_d1_%i %i\n",
		  prefix, optr->val, opidx, j, *i);
	  *i = *i + 1;
	  
	  fprintf(outfile,"#define %s_%soparray%i_b0_%i %i\n",
		  prefix, optr->val, opidx, j, *i);
	  *i = *i + 1;
	  
	  fprintf(outfile,"#define %s_%soparray%i_b1_%i %i\n",
		  prefix, optr->val, opidx, j, *i);
	  *i = *i + 1;
	  
	  fprintf(outfile,"#define %s_%soparray%i_b2_%i %i\n",
		  prefix, optr->val, opidx, j, *i);
	  *i = *i + 1;
	  
	  fprintf(outfile,"#define %s_%soparray%i_a1_%i %i\n",
		  prefix, optr->val, opidx, j, *i);
	  *i = *i + 1;
	  
	  fprintf(outfile,"#define %s_%soparray%i_a2_%i %i\n",
		  prefix, optr->val, opidx, j, *i);
	  *i = *i + 1;
	  
	  j--;
	}
    }
}

/****************************************************************/
/*  oparray space for table slots for extra formal parameters   */
/****************************************************************/

void oparraytableextrastate(tnode * optr, char * prefix,
			    int * i, int opidx, tnode * olist)

{
  tnode * tptr;
  tnode * maxextra = NULL;
  int maxnum = 0;
  int j;

  /* only called for "reverb" oparray definitions */

  /* find oparray call with the most extra parameters */

  while (olist)
    {
      if ((olist->ttype == S_OPARRAYCALL) && !strcmp(olist->val, optr->val))
	{
	  j = 0;
	  tptr = olist->extra;
	  while (tptr)
	    {
	      if (tptr->ttype == S_PARAMDECL)
		j++;
	      tptr = tptr->next;
	    }
	  if (j > maxnum)
	    {
	      maxnum = j;
	      maxextra = olist->extra;
	    }
	}
      olist = olist->next;
    }

  /* if no such oparray call exists, exit */

  if (!maxextra)
    return;

  maxnum--;   /* subtract for f0 */

  j = maxnum/2;  /* number of additional (f,r) pairs */

  while (j > 0)
    {
      fprintf(outfile,"#define TBL_%s_%soparray%i_dline%i_0 %i\n",
	      prefix, optr->val, opidx, j, *i);
      *i = *i + 1;
      
      fprintf(outfile,"#define TBL_%s_%soparray%i_dline%i_1 %i\n",
	      prefix, optr->val, opidx, j, *i);
      *i = *i + 1;
      
      fprintf(outfile,"#define TBL_%s_%soparray%i_dline%i_2 %i\n",
	      prefix, optr->val, opidx, j, *i);
      *i = *i + 1;
      
      fprintf(outfile,"#define TBL_%s_%soparray%i_dline%i_3 %i\n",
	      prefix, optr->val, opidx, j, *i);
      *i = *i + 1;
      
      j--;
    }
}

/****************************************************************/
/*     determines if the opstate state variable is needed       */
/****************************************************************/

int opstate_needed(tnode * optr)

{
  if (!coreopcodename(optr))
    {
      if (curroparraydepth)
	return 1;

      switch (optr->rate) {
      case KRATETYPE:
	return (optr->sptr->cref->ilines > 0);
	break;
      case ARATETYPE:
	return (optr->sptr->cref->klines || optr->sptr->cref->ilines);
	break;
      }
    }

  return 0;
}


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/* These functions are utilities for printing out instr symbols */
/*______________________________________________________________*/

/****************************************************************/
/*            prints control driver instr symbols               */
/****************************************************************/

void instrdriversymbols(sigsym * sptr, sigsym * iptr, int i)

{
  sigsym * bptr;
  sigsym * b2ptr;

 if ((sptr->kind == K_IMPORT) ||(sptr->kind == K_IMPORTEXPORT))
   {
     if (bitfile && bitsymin)
       {
	 bptr = getvsym(&bitsymin,sptr->val);
	 b2ptr = getvsym(&bitsymin,iptr->val);
	 
	 /* null for synthetic global tables */
	 
	 if (bptr && b2ptr)
	   fprintf(outfile,
		   "#define CSYS_SASL_IMPORT_%s_%s %i\n",
		   b2ptr->defnode->val, bptr->defnode->val,
		   i);
       }
     else
       if ((sptr->vartype!=TABLETYPE)||(sptr->val[0]!='_'))
	 fprintf(outfile,
		 "#define CSYS_SASL_IMPORT_%s_%s %i\n",
		 iptr->val,sptr->val,i);
   }
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/* This function family prints the external function headers    */
/*           needed later in the sa.c file.                     */
/*______________________________________________________________*/


/****************************************************************/
/*                 print opcode declarations                    */
/****************************************************************/


void printopexterns(tnode * tptr)

{

  char * stack;
  char newfix[STRSIZE];
  char newname[STRSIZE];
  int opnum;

  while (tptr != NULL)
    {
      if (tptr->optr && ((tptr->ttype == S_OPARRAYCALL) ||   
	  ((tptr->ttype == S_OPCALL) &&
	   (!coreopcodecaninline(tptr->optr->down)))))
	{
	  sprintf(newfix,"%s_%s%i",currinstancename, tptr->val, 
		  tptr->arrayidx);
	  sprintf(newname,"%s__sym_%s%i",currinstancename, tptr->val, 
		  tptr->arrayidx);
	  if (currinstance == NULL)
	    {
	      fprintf(outfile,
		      "float %s(ENGINE_PTR_TYPE_COMMA "
		      "struct ninstr_types *);\n", newname);
	      if (tptr->special)
		fprintf(outfile,
			"void %s_spec(ENGINE_PTR_TYPE_COMMA "
			"struct ninstr_types *);\n", newname);
	    }
	  else
	    {
	      fprintf(outfile,"float %s(ENGINE_PTR_TYPE);\n", newname);
	      if (tptr->special)
		fprintf(outfile,"void %s_spec(ENGINE_PTR_TYPE);\n", newname);
	    }

	  /* do constant oparray index syntax check */

	  if ((tptr->ttype == S_OPARRAYCALL) && 
	      (tptr->optr->down->next->next->vol == CONSTANT))
	    {
	      opnum = make_int(tptr->optr->down->next->next->down);
		  
	      if ((opnum < 0) || (opnum >= truewidth(tptr->ibus->opwidth)))
		{
		  printf("Error: Oparray call %s constant index %i out of range.\n\n",
			 tptr->val, opnum);
		  showerrorplace(tptr->optr->down->linenum, 
				 tptr->optr->down->filename);
		}
	    }

	  /* identical naming scheme for oparrays and opcalls */

	  stack = currinstancename;
	  currinstancename = newfix;
	  printopexterns(tptr->sptr->defnode->optr);
	  currinstancename = stack;

	}
      tptr = tptr->next;
    }

}


/****************************************************************/
/*            prints pass-function externs                     */
/****************************************************************/

void printpassexterns(void)

{

  sigsym * sptr;
  tnode * tptr;

  sptr = instrnametable;
  while (sptr != NULL)
    {
      if (reachableinstrexeffexstart(sptr))
	{
	  fprintf(outfile,"void %s_ipass("
		  "ENGINE_PTR_TYPE_COMMA struct ninstr_types *);\n",sptr->val);
	  fprintf(outfile,"void %s_kpass("
		  "ENGINE_PTR_TYPE_COMMA struct ninstr_types *);\n",sptr->val);
	  if (sptr->cref->alines)
	    fprintf(outfile,"void %s_apass("
		    "ENGINE_PTR_TYPE_COMMA struct ninstr_types *);\n", sptr->val);
	}
      sptr = sptr->next;
    }
  fprintf(outfile,"\n");

  tptr = instances;
  while (tptr != NULL)
    {
      fprintf(outfile,"void %s_ipass(ENGINE_PTR_TYPE);\n",tptr->val);
      fprintf(outfile,"void %s_kpass(ENGINE_PTR_TYPE);\n",tptr->val);
      if (tptr->sptr->cref->alines)
	fprintf(outfile,"void %s_apass(ENGINE_PTR_TYPE);\n",tptr->val);
      tptr = tptr->next;
    }
  fprintf(outfile,"\n");
}


/****************************************************************/
/*        print finput/finGroup  declarations                   */
/****************************************************************/

void printfinputexterns(void)

{

  tnode * tptr = instances;
  
  while (tptr != NULL)
    {
      fprintf(outfile,"float finput%i(float);\n",
	      tptr->arrayidx);
      fprintf(outfile,"float finGroup%i(float);\n",
	      tptr->arrayidx);
      tptr = tptr->next;
    }
  fprintf(outfile,"\n");
}


/****************************************************************/
/*   prints core-opcode and sample-rate interpolation externs   */
/*        large tables are defined in postcorefunctions()       */  
/****************************************************************/

void coreopcodeexterns(void)

{
  int tsize;

  if (has.o_fft || has.o_ifft)
    {
      tsize = 4;
      while (tsize <= 8192)
	{
	  fprintf(outfile,"extern float fft%itab[]; \n",tsize);
	  fprintf(outfile,"extern int fft%imap[]; \n", tsize);
	  tsize *= 2;
	}
      fprintf(outfile,"\n");
    }
  
  if (has.o_buzz)
    fprintf(outfile,"extern float sintab[];\n\n");
  
  if ((interp == INTERP_SINC) && (has.o_doscil || has.o_oscil || has.o_koscil || 
				  has.o_loscil || has.o_pluck || has.o_fracdelay || 
				  has.o_flange || has.o_chorus || has.o_tableread))
    fprintf(outfile,"extern float sinc[];\n\n");
}


extern void printsaolopcodetables(tnode * optr, char * prefix, int type);

/****************************************************************/
/*     prints arrays externs for SAOL constant wavetables       */
/*           also called in writeorc.c to print table data      */
/****************************************************************/

void printsaoltables(int type)

{
  tnode * tptr;
  sigsym * sptr;
  sigsym * iptr;
  char name[STRSIZE];

  sptr = globalsymtable;

  while (sptr != NULL)
    {
      if ((sptr->vartype == TABLETYPE) && (sptr->kind == K_NORMAL) &&
	  (sptr->defnode->vol == CONSTANT) && (sptr->defnode->usesinput))
	{
	  if (type == S_DATA)
	    {	  
	      sprintf(name, "table_global_%s",sptr->val);
	      printtablestring(sptr, name);
	    }
	  else
	    fprintf(outfile,"extern %s table_global_%s[];\n", 
		    hexstrings ? "char" : "float", sptr->val);
	}
      sptr = sptr->next;
    }

  printsaolopcodetables(globalopcodecalls, "global", type); 

  iptr = instrnametable;
  while (iptr != NULL)
    {
      if (reachableinstrexeff(iptr))
	{
	  sptr = iptr->defnode->sptr;
	  while (sptr != NULL)
	    {
	      if ((sptr->vartype == TABLETYPE) &&
		  (sptr->kind == K_NORMAL) &&
		  (sptr->defnode->vol == CONSTANT) &&
		  (sptr->defnode->usesinput))
		{
		  if (type == S_DATA)
		    {	  
		      sprintf(name, "table_%s_%s", iptr->val, sptr->val);
		      printtablestring(sptr, name);
		    }
		  else
		    fprintf(outfile,"extern %s table_%s_%s[];\n",
			    hexstrings ? "char" : "float", 
			    iptr->val, sptr->val);
		}
	      sptr = sptr->next;
	    }
	  printsaolopcodetables(iptr->defnode->optr, iptr->val, type);
	}
      iptr = iptr->next;
    }

  tptr = instances;
  while (tptr != NULL)
    {
      sptr = tptr->sptr->defnode->sptr;
      while (sptr != NULL)
	{
	  if (sptr->vartype == TABLETYPE)
	    {
	      if ((sptr->vartype == TABLETYPE) &&
		  (sptr->kind == K_NORMAL) &&
		  (sptr->defnode->vol == CONSTANT) &&
		  (sptr->defnode->usesinput))
		{
		  if (type == S_DATA)
		    {	  
		      sprintf(name, "table_%s_%s", tptr->val, sptr->val);
		      printtablestring(sptr, name);
		    }
		  else
		    fprintf(outfile,"extern %s table_%s_%s[];\n",
			    hexstrings ? "char" : "float", 
			    tptr->val, sptr->val);
		}
	    }
	  sptr = sptr->next;
	}

      printsaolopcodetables(tptr->sptr->defnode->optr, tptr->val, type);
      tptr = tptr->next;
    }

  fprintf(outfile,"\n");
}

/****************************************************************/
/*   print array externs and data for opcall constant tables    */
/****************************************************************/

void printsaolopcodetables(tnode * optr, char * prefix, int type)


{
  sigsym * sptr;
  char newprefix[STRSIZE];
  char name[STRSIZE];

  while (optr != NULL)
    {
      sprintf(newprefix,"%s_%s%i", prefix, optr->val, optr->arrayidx);
      if ((optr->ttype == S_OPCALL) || (optr->ttype == S_OPARRAYCALL))
	{
	  sptr = optr->sptr->defnode->sptr;
	  while (sptr != NULL)
	    {
	      if ((sptr->vartype == TABLETYPE) &&
		  (sptr->kind == K_NORMAL) &&
		  (sptr->defnode->vol == CONSTANT) &&
		  (sptr->defnode->usesinput))
		{
		  if (type == S_DATA)
		    {	  
		      sprintf(name, "table_%s_%s", newprefix, sptr->val);
		      printtablestring(sptr, name);
		    }
		  else
		    fprintf(outfile,"extern %s table_%s_%s[];\n",
			    hexstrings ? "char" : "float", 
			    newprefix, sptr->val);
		}
	      sptr = sptr->next;
	    }
	  printsaolopcodetables(optr->sptr->defnode->optr, newprefix, type);
	}
      optr = optr->next;
    }
}


extern void printsaolopcodecatalog(tnode * optr, char * prefix, 
				   int * catsize_ptr);

/****************************************************************/
/*   prints a catalog of all constant tables defined in sa.c    */
/****************************************************************/

void printtablecatalog(void)

{
  tnode * tptr;
  sigsym * sptr;
  sigsym * iptr;
  int catsize = 0;

  fprintf(outfile,"csys_table_ptr csys_table_catalog[] = {");

  sptr = globalsymtable;

  while (sptr != NULL)
    {
      if ((sptr->vartype == TABLETYPE) && (sptr->kind == K_NORMAL) &&
	  (sptr->defnode->vol == CONSTANT) && (sptr->defnode->usesinput))
	{
	  if (catsize++)
	    fprintf(outfile, ",\n\t");
	  else
	    fprintf(outfile,  "\n\t");
	      
	  fprintf(outfile,"%i,", sptr->defnode->arrayidx + 1);

	  if (hexstrings)
	    fprintf(outfile," (float *)");

	  fprintf(outfile," table_global_%s", sptr->val);
	}
      sptr = sptr->next;
    }

  printsaolopcodecatalog(globalopcodecalls, "global", &catsize);

  iptr = instrnametable;
  while (iptr != NULL)
    {
      if (reachableinstrexeff(iptr))
	{
	  sptr = iptr->defnode->sptr;
	  while (sptr != NULL)
	    {
	      if ((sptr->vartype == TABLETYPE) &&
		  (sptr->kind == K_NORMAL) &&
		  (sptr->defnode->vol == CONSTANT) &&
		  (sptr->defnode->usesinput))
		{
		  if (catsize++)
		    fprintf(outfile, ",\n\t");
		  else
		    fprintf(outfile,  "\n\t");
		  
		  fprintf(outfile,"%i,", sptr->defnode->arrayidx + 1);

		  if (hexstrings)
		    fprintf(outfile," (float *)");

		  fprintf(outfile," table_%s_%s", iptr->val, sptr->val);
		}
	      sptr = sptr->next;
	    }
	  printsaolopcodecatalog(iptr->defnode->optr, iptr->val, &catsize);
	}
      iptr = iptr->next;
    }

  tptr = instances;
  while (tptr != NULL)
    {
      sptr = tptr->sptr->defnode->sptr;
      while (sptr != NULL)
	{
	  if ((sptr->vartype == TABLETYPE) &&
	      (sptr->kind == K_NORMAL) &&
	      (sptr->defnode->vol == CONSTANT) &&
	      (sptr->defnode->usesinput))
	    {
	      if (catsize++)
		fprintf(outfile, ",\n\t");
	      else
		fprintf(outfile,  "\n\t");
	      
	      fprintf(outfile,"%i,", sptr->defnode->arrayidx + 1);
	      
	      if (hexstrings)
		fprintf(outfile," (float *)");
	      
	      fprintf(outfile," table_%s_%s", tptr->val, sptr->val);
	    }
	  sptr = sptr->next;
	}

      printsaolopcodecatalog(tptr->sptr->defnode->optr, tptr->val, &catsize);
      tptr = tptr->next;
    }

  if (!catsize)
    fprintf(outfile, " 0, NULL");

  fprintf(outfile," };\n\n");
  fprintf(outfile,"#define CSYS_TABLE_CATALOG_SIZE %i\n\n", catsize);
}

/****************************************************************/
/*          print catalog data for opcall constant tables       */
/****************************************************************/

void printsaolopcodecatalog(tnode * optr, char * prefix, int * catsize_ptr)


{
  sigsym * sptr;
  char newprefix[STRSIZE];

  while (optr != NULL)
    {
      sprintf(newprefix,"%s_%s%i", prefix, optr->val, optr->arrayidx);
      if ((optr->ttype == S_OPCALL) || (optr->ttype == S_OPARRAYCALL))
	{
	  sptr = optr->sptr->defnode->sptr;
	  while (sptr != NULL)
	    {
	      if ((sptr->vartype == TABLETYPE) &&
		  (sptr->kind == K_NORMAL) &&
		  (sptr->defnode->vol == CONSTANT) &&
		  (sptr->defnode->usesinput))
		{
		  if ((*catsize_ptr)++)
		    fprintf(outfile, ",\n\t");
		  else
		    fprintf(outfile,  "\n\t");
		  
		  fprintf(outfile,"%i,", sptr->defnode->arrayidx + 1);
		  
		  if (hexstrings)
		    fprintf(outfile," (float *)");
		  
		  fprintf(outfile," table_%s_%s", newprefix, sptr->val);
		}
	      sptr = sptr->next;
	    }
	  printsaolopcodecatalog(optr->sptr->defnode->optr, newprefix, catsize_ptr);
	}
      optr = optr->next;
    }
}


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*   This function family handles the reflection interface      */
/*         that prints out SAOL program structure               */
/*        printreflection() is main routine, at end             */  
/*______________________________________________________________*/

/****************************************************************/
/*                 print SAOL global variables                  */
/****************************************************************/

void printreflectionglobalvars(void)

{
  int numglobal = 0;
  int i, gcount, tbl;
  sigsym * sptr, * bptr, * token;
  int au_read = 0, au_write = 0;
  char * nptr;

  sptr = globalsymtable;
  while (sptr)
    {
      if ((sptr->vartype == SCALARTYPE) ||
	  (sptr->vartype == VECTORTYPE) ||
	  ((sptr->vartype == TABLETYPE) && (sptr->val[0] != '_')))
	numglobal++;
      sptr = sptr->next;
    }

  i =  MIDIFRAMELEN*totmidichan;

  fprintf(outfile,"#define CSYS_GLOBALNUM %i\n\n",numglobal);
  if (!numglobal)
    {
      fprintf(outfile,"csys_varstruct csys_global[1];\n\n");
    }
  else
    {
      fprintf(outfile,"csys_varstruct csys_global[CSYS_GLOBALNUM] = {\n");
      sptr = globalsymtable;
      gcount = numglobal;
      currinputwidth = inchannels;
      currinstrwidth = outchannels;
      tbl = 0;
      while (sptr)
	{      
	  if ((sptr->vartype == SCALARTYPE) ||
	      (sptr->vartype == VECTORTYPE) ||
	      ((sptr->vartype == TABLETYPE) && (sptr->val[0] != '_')))
	    {
	      /* int index */

	      if (sptr->vartype == TABLETYPE)
		fprintf(outfile,"%i, ",tbl);  
	      else
		fprintf(outfile,"%i, ",i);  

	      /* char * name */
	      
	      if (bitfile && bitsymin && (bptr = getvsym(&bitsymin,sptr->val)))
		fprintf(outfile,"\"%s\", ",bptr->defnode->val);
	      else
		fprintf(outfile,"\"%s\", ",sptr->val);

	      /* int token */

	      if (bitfile)
		fprintf(outfile,"%s, ", &(sptr->val[1]));
	      else
		{
		  token = getvsym(&mpegtokens, sptr->val);
		  fprintf(outfile, "%i, ", token ? token->width : -1);
		}

	      /* int type and int tag*/

	      switch (sptr->vartype) {
	      case SCALARTYPE: 
	      case VECTORTYPE: 
		if (sptr->rate == IRATETYPE)
		  fprintf(outfile,"CSYS_IRATE, CSYS_NORMAL, ");
		else
		  fprintf(outfile,"CSYS_KRATE, CSYS_NORMAL, ");
		break;
	      case TABLETYPE:
		fprintf(outfile,"CSYS_TABLE, CSYS_NORMAL, ");
		break;
	      }
	      
	      /* int width */

	      fprintf(outfile,"%i, ", truewidth(sptr->width));

	      /* int use */

	      if (sptr->tref->assigntot)
		{
		  if (sptr->tref->accesstot)
		    fprintf(outfile,"CSYS_WRITTEN_AND_READ");
		  else
		    fprintf(outfile,"CSYS_WRITTEN");
		}
	      else
		{
		  if (sptr->tref->accesstot)
		    fprintf(outfile,"CSYS_READ");
		  else
		    fprintf(outfile,"CSYS_UNUSED");
		}

	      if (--gcount)
		fprintf(outfile,", \n");
	      else
		fprintf(outfile," };\n\n");
	    }

	  if (!(bitfile && bitsymin && getvsym(&bitsymin,sptr->val)))
	    do {

	      /* code below must track audiounit.c line 1800 changes */

	      if ((strncmp(sptr->val, "aup_", 4)) ||
		  (strlen(sptr->val) <= 4) || 
		  (truewidth(sptr->width) != 1) ||
		  ((sptr->rate != IRATETYPE) && (sptr->rate != KRATETYPE)))
		break;

	      if ((nptr = strstr(&(sptr->val[4]), "_idx")) && 
		  (strlen(nptr) > 6) && (nptr[4] >= '0') && (nptr[4] <= '9'))
		break;

	      if (strstr(&(sptr->val[4]), "_unit_"))
		break;

	      if ((strstr(&(sptr->val[4]), "_slider_squarelaw")) ||
		  (strstr(&(sptr->val[4]), "_slider_cubic")) ||
		  (strstr(&(sptr->val[4]), "_slider_squareroot")) ||
		  (strstr(&(sptr->val[4]), "_slider_cuberoot")) ||
		  (strstr(&(sptr->val[4]), "_slider_log")) ||
		  (strstr(&(sptr->val[4]), "_slider_exp")) ||
		  (strstr(&(sptr->val[4]), "_slider_linear")) ||
		  (strstr(&(sptr->val[4]), "_slider")) ||
		  (strstr(&(sptr->val[4]), "_checkbox")) ||
		  (strstr(&(sptr->val[4]), "_menu")) || 
		  (strstr(&(sptr->val[4]), "_display_number")) || 
		  (strstr(&(sptr->val[4]), "_display_checkbox")) || 
		  (strstr(&(sptr->val[4]), "_display_menu")))
		break;

	      if (sptr->tref->assigntot)
		au_write = 1;

	      if (sptr->tref->accesstot)
		au_read = 1;

	    } while (0);

	  i = i + truewidth(sptr->width);

	  if (sptr->vartype == TABLETYPE)
	    tbl++;

	  sptr = sptr->next;
	}
    }

  if (aout && (!strcmp(aoutname,"audiounit") || !strcmp(aoutname,"audiounit_debug")))
    {
      fprintf(outfile,"#define ASYS_AUDIOUNIT_PARAMETERS_WRITTEN %i\n",
	      au_write);
      fprintf(outfile,"#define ASYS_AUDIOUNIT_PARAMETERS_READ %i\n\n",
	      au_read);
    }
}


/****************************************************************/
/*   computes position of labelled control variable in stack    */
/****************************************************************/

int targetvarposition(sigsym * sptr, sigsym * vptr)

{
  int ret = 0;

  while (sptr)
    {
      if (sptr == vptr)
	return ret;
      else
	if ((sptr->vartype != TMAPTYPE) && (sptr->tref->mirror == REQUIRED))
	  ret += truewidth(sptr->width);
      sptr = sptr->next;
    }

  /* should never execute */

  return 0;
}


/****************************************************************/
/*   print target variables for labelled SASL control statement */
/****************************************************************/

void printreflectiontarget(void)

{
  char * name;
  int numtarget, numinstr;
  int i, tcount;
  int * instrindex, * varindex;
  sigsym * sptr, * bptr, * iptr, * token;

  sptr = targetsymtable;
  numtarget = 0;
  vmcheck(instrindex = (int *) calloc(numinstrnames ? numinstrnames : 1,
				      sizeof(int)));
  vmcheck(varindex = (int *) calloc(numinstrnames ? numinstrnames : 1,
				    sizeof(int)));
  while (sptr)
    {
      if (bitfile && bitsymin && (bptr = getvsym(&bitsymin,sptr->val)))
	name = bptr->defnode->val;
      else
	name = sptr->val;

      iptr = instrnametable;
      numinstr = 0;
      while (iptr)
	{
	  if ((bptr = getvsym(&(iptr->defnode->sptr),sptr->val)) &&
	      (bptr->kind == K_IMPORT))
	    {
	      instrindex[numinstr] = iptr->vol;

	      currinputwidth = 1;
	      currinstrwidth = iptr->width;
	      varindex[numinstr] = targetvarposition(iptr->defnode->sptr,
						     bptr);
	      numinstr++;
	    }
	  iptr = iptr->next;
	}
      fprintf(outfile,"#define CSYS_TARGET_INSTR_%s %i\n\n",
	      name, numinstr);
      if (!numinstr)
	{
	  fprintf(outfile, "int csys_targetinstr_%s[1];\n", name);
	  fprintf(outfile, "int csys_targetvar_%s[1];\n\n",  name);
	}
      else
	{      
	  fprintf(outfile,
		  "int csys_targetinstr_%s[CSYS_TARGET_INSTR_%s] = {\n %i",
		  name, name, instrindex[0]);
	  for (i=1; i< numinstr; i++)
	    fprintf(outfile, ", %i",instrindex[i]);
	  fprintf(outfile, "};\n\n");
	  fprintf(outfile,
		  "int csys_targetvar_%s[CSYS_TARGET_INSTR_%s] = {\n %i",
		  name, name, varindex[0]);
	  for (i=1; i< numinstr; i++)
	    fprintf(outfile, ", %i",varindex[i]);
	  fprintf(outfile, "};\n\n");
	}
      numtarget++;
      sptr = sptr->next;
    }

  fprintf(outfile,"#define CSYS_TARGETNUM %i\n\n",numtarget);
  if (!numtarget)
    {
      fprintf(outfile,"csys_targetstruct csys_target[1];\n\n");
    }
  else
    {
      fprintf(outfile,"csys_targetstruct csys_target[CSYS_TARGETNUM] = {\n");
      sptr = targetsymtable;
      tcount = 0;
      while (sptr)
	{      

	  /* char * name */
	      
	  if (bitfile && bitsymin && (bptr = getvsym(&bitsymin,sptr->val)))
	    name = bptr->defnode->val;
	  else
	    name = sptr->val;

	  fprintf(outfile,"\"%s\", ",name);

	  /* int token */
	  
	  if (bitfile)
	    fprintf(outfile,"%s, ", &(sptr->val[1]));
	  else
	    {
	      token = getvsym(&mpegtokens, sptr->val);
	      fprintf(outfile, "%i, ", token ? token->width : -1);
	    }

	  /* int numinstr */

	  fprintf(outfile,"CSYS_TARGET_INSTR_%s, ", name);

	  /* int numinstr */

	  fprintf(outfile,"&(csys_targetinstr_%s[0]), ", name);

	  /* int numinstr */

	  fprintf(outfile,"&(csys_targetvar_%s[0]) ", name);
	  if ((++tcount) == numtarget)
	     fprintf(outfile,"};\n\n");
	  else
	    fprintf(outfile,",\n");
	  sptr = sptr->next;
	}
    }

  free(instrindex);
  free(varindex);

}


/****************************************************************/
/*                     print SASL labels                        */
/****************************************************************/

void printreflectionlabels(void)

{
  sigsym * sptr, * bptr, * iptr, * token;
  tnode * tptr;
  int found, totlabels;

  totlabels = allsasl->numlabels + abssasl->numlabels;

  fprintf(outfile,"#define CSYS_NOLABEL 0\n\n");
  fprintf(outfile,"#define CSYS_LABELNUM %i\n\n", totlabels);
  
  if (!(totlabels)) 
    fprintf(outfile,"csys_labelstruct csys_labels[1];\n\n");
  else
    {
      sptr = allsasl->labeltable;
      fprintf(outfile,"csys_labelstruct csys_labels[CSYS_LABELNUM] = {\n");
      while (sptr)
	{
	  /* int index */

	  fprintf(outfile, "%i, ",sptr->special+1); 

	  /* char * name */

	  if (bitfile && bitsymin && (bptr = getvsym(&bitsymin,sptr->val)))
	    fprintf(outfile,"\"%s\", ",bptr->defnode->val);
	  else
	    fprintf(outfile,"\"%s\", ",sptr->val);

	  /* int token */

	  if (bitfile)
	    fprintf(outfile,"%s, ", &(sptr->val[1]));
	  else
	    {
	      token = getvsym(&mpegtokens, sptr->val);
	      fprintf(outfile, "%i, ", token ? token->width : -1);
	    }

	  iptr = instrnametable;
	  while (iptr)
	    {
	      found = 0;
	      tptr = sptr->defnode;
	      while (tptr)
		{
		  if (tptr->down && (tptr->down->sptr == iptr))
		    {
		      found = 1;
		      break;
		    }
		  tptr = tptr->next;
		}
	      fprintf(outfile, "%i ",found);
	      iptr = iptr->next;
	      if (iptr)
		fprintf(outfile, ", ");
	    }
	  sptr = sptr->next;
	  if (sptr)
	    fprintf(outfile,",\n");
	  else
	    fprintf(outfile,"};\n\n");
	}
    }

}

/****************************************************************/
/*            print preset numbers used in instrs               */
/****************************************************************/

void printreflectionpresets(void)

{
  sigsym * sptr;
  int i;

  i = 0;
  sptr = instrpresets;
  while (sptr)
    {
      i++;
      sptr = sptr->next;
    }

  fprintf(outfile,"#define CSYS_PRESETNUM %i\n\n", i);
  
  if (!i) 
    fprintf(outfile,"csys_presetstruct csys_presets[1];\n\n");
  else
    {
      sptr = instrpresets;
      fprintf(outfile,"csys_presetstruct csys_presets[CSYS_PRESETNUM] = {\n");
      while (sptr)
	{
	  /* int index */

	  fprintf(outfile, "%i, ",sptr->defnode->sptr->vol); 

	  /* int preset */

	  fprintf(outfile, "%i ",sptr->width); 

	  sptr = sptr->next;
	  if (sptr)
	    fprintf(outfile,",\n");
	  else
	    fprintf(outfile,"};\n\n");
	}
    }

}

/****************************************************************/
/*       print sample files used in configuration block         */
/****************************************************************/

void printreflectionsamples(void)

{
  sigsym * sptr, * bptr, * token;
  char * name;
  int i;

  i = 0;
  sptr = bitsamplein;
  while (sptr)
    {
      i++;
      sptr = sptr->next;
    }

  fprintf(outfile,"#define CSYS_SAMPLENUM %i\n\n", i);
  
  if (!i) 
    fprintf(outfile,"csys_samplestruct csys_samples[1];\n\n");
  else
    {
      i = 0;
      sptr = bitsamplein;
      fprintf(outfile,"csys_samplestruct csys_samples[CSYS_SAMPLENUM] = {\n");
      while (sptr)
	{
	  /* int index */

	  fprintf(outfile, "%i, ",i++); 

	  /* int token */

	  if (bitfile)
	    fprintf(outfile,"%s, ", &(sptr->val[1]));
	  else
	    {
	      token = getvsym(&mpegtokens, sptr->val);
	      fprintf(outfile, "%i, ", token ? token->width : -1);
	    }

	  /* char * name */

	  if (bitfile && bitsymin && (bptr = getvsym(&bitsymin,sptr->val)))
	    name = bptr->defnode->val;
	  else
	    name = sptr->val;
  
	  fprintf(outfile, "\"%s\", ", name); 

	  /* char * fname */

	  fprintf(outfile, "\"%s.wav\" ", sptr->val); 

	  sptr = sptr->next;
	  if (sptr)
	    fprintf(outfile,",\n");
	  else
	    fprintf(outfile,"};\n\n");
	}
    }

}

/****************************************************************/
/*              prints csys_varstruct for instrs                */
/****************************************************************/

void printreflectionvars(sigsym * instr)

{

  sigsym * sptr, * bptr, * token;
  int numvars, i, comma;
  char * name, * varname;

  sptr = instr->defnode->sptr;
  numvars = 0;
  while (sptr)
    {
      /* don't include tablemaps or internal variables */
      
      if ((sptr->vartype != TMAPTYPE) && (sptr->val[0] != '_')) 
	numvars++;
      sptr = sptr->next;
    }
  
  if (bitfile && bitsymin && (bptr = getvsym(&bitsymin,instr->val)))
    name = bptr->defnode->val;
  else
    name = instr->val;
  
  fprintf(outfile,"#define CSYS_%s_VARNUM %i\n\n",name,numvars);
  if (!numvars) 
    fprintf(outfile,"csys_varstruct csys_%s_vars[1];\n\n",name);
  else
    {
      fprintf(outfile,"csys_varstruct csys_%s_vars[CSYS_%s_VARNUM] = {\n",
	      name, name);

      comma = i = 0;
      currinputwidth = 1;              
      currinstrwidth = instr->width;

      sptr = instr->defnode->sptr;

      while (sptr)
	{
	  if ((sptr->vartype != TMAPTYPE) && (sptr->val[0] != '_'))
	    {	
	      if (comma)
		fprintf(outfile,",\n");
	      comma = 1;

	      if (bitfile && bitsymin && (bptr = getvsym(&bitsymin,sptr->val)))
		varname = bptr->defnode->val;
	      else
		varname = sptr->val;

	      /* int index */

	      if (sptr->tref->mirror == REQUIRED)
		fprintf(outfile,"%i, ",sptr->vol);  
	      else
		fprintf(outfile, "-1, ");

	      /* char * name */

	      fprintf(outfile,"\"%s\", ", varname);

	      /* int token */

	      if (bitfile)
		fprintf(outfile,"%s, ", &(sptr->val[1]));
	      else
		{
		  token = getvsym(&mpegtokens, sptr->val);
		  fprintf(outfile, "%i, ", token ? token->width : -1);
		}

	      /* int type */

	      switch (sptr->vartype) {
	      case SCALARTYPE: 
	      case VECTORTYPE: 
		switch (sptr->rate) {
		case IRATETYPE:
		  fprintf(outfile,"CSYS_IRATE, ");
		  break;
		case KRATETYPE:
		  fprintf(outfile,"CSYS_KRATE, ");
		  break;
		case ARATETYPE:
		  fprintf(outfile,"CSYS_ARATE, ");
		  break;
		}
		break;
	      case TABLETYPE:
		fprintf(outfile,"CSYS_TABLE, ");
		break;
	      }

	      /* int tag */

	      switch (sptr->kind) {
	      case K_NORMAL:
		fprintf(outfile,"CSYS_NORMAL, ");
		break;
	      case K_PFIELD:
		fprintf(outfile,"CSYS_PFIELD, ");
		break;
	      case K_IMPORT:
		fprintf(outfile,"CSYS_IMPORT, ");
		break;
	      case K_EXPORT:
		fprintf(outfile,"CSYS_EXPORT, ");
		break;
	      case K_IMPORTEXPORT:
		fprintf(outfile,"CSYS_IMPORTEXPORT, ");
		break;
	      default:
		fprintf(outfile,"CSYS_INTERNAL, ");
		break;
	      }

	      /* int width */
	      fprintf(outfile,"%i, ",truewidth(sptr->width));  

	      /* int use */

	      if (sptr->tref->assigntot)
		{
		  if (sptr->tref->accesstot)
		    fprintf(outfile,"CSYS_WRITTEN_AND_READ ");
		  else
		    fprintf(outfile,"CSYS_READ ");
		}
	      else
		{
		  if (sptr->tref->accesstot)
		    fprintf(outfile,"CSYS_READ ");
		  else
		    fprintf(outfile,"CSYS_UNUSED ");
		}

	    }

	  if ((sptr->vartype != TMAPTYPE) && (sptr->tref->mirror == REQUIRED))
	    i += truewidth(sptr->width);

	  sptr = sptr->next;
	}
      fprintf(outfile," };\n\n");
    }
}

/****************************************************************/
/*              prints csys_inststruct for instrs               */
/****************************************************************/

void printreflectioninstr(sigsym * sptr)

{
  char * name;
  sigsym * bptr, * token;
  int status;

  if (sptr->vol)
    fprintf(outfile,",\n");

  if (bitfile && bitsymin && (bptr = getvsym(&bitsymin,sptr->val)))
    name = bptr->defnode->val;
  else
    name = sptr->val;
  
  /* int index, char * name */

  fprintf(outfile,"%i, \"%s\",",sptr->vol, name);  
      
  /* int token */

  if (bitfile)
    fprintf(outfile,"%s, ", !(sptr->startup) ? &(sptr->val[1]) : "-1");
  else
    {
      token = getvsym(&mpegtokens, sptr->val);
      fprintf(outfile, "%i, ", token ? token->width : -1);
    }

  /* numvars, * vars */

  fprintf(outfile,"CSYS_%s_VARNUM, &(csys_%s_vars[0]),",
	  name, name);

  /* output width */

  fprintf(outfile,"%i, ",sptr->width);

  /* status word */

  status = 0;
  if (sptr->effects)
    status |=  STATWORD_EFFECTS;

  if (sptr->score || sptr->ascore)
    status |=  STATWORD_SCORE;

  if (sptr->midi || sptr->amidi)
    status |=  STATWORD_MIDI;

  if (sptr->dyn)
    status |=  STATWORD_DYNAMIC;

  if (sptr->startup)
    status |=  STATWORD_STARTUP;


  fprintf(outfile,"%i ",status);

}

/****************************************************************/
/*           print buses in SAOL program                        */
/****************************************************************/

void printreflectionbus(void)

{
  char * name;
  sigsym * sptr, * bptr;
  tnode * tptr;
  int i;

  i = 0;
  sptr = busnametable;
  while (sptr)
    {
      i++;
      sptr = sptr->next;
    }

  fprintf(outfile,"#define CSYS_BUSNUM %i\n\n", i);
  
  if (!i) 
    fprintf(outfile,"csys_busstruct csys_bus[1];\n\n");
  else
    {
      sptr = busnametable;
      fprintf(outfile,"csys_busstruct csys_bus[CSYS_BUSNUM] = {\n");
      while (sptr)
	{
	  /* int index */

	  fprintf(outfile, "%i, ",sptr->vol);
 
	  if (bitfile && bitsymin && (bptr = getvsym(&bitsymin,sptr->val)))
	    name = bptr->defnode->val;
	  else
	    name = sptr->val;
  
	  /* char * name */

	  fprintf(outfile, "\"%s\",", name); 

	  /* int width */

	  fprintf(outfile,"%i, ",sptr->width);

	  /* int oflag */

	  i = 0;
	  tptr = outbustable;
	  while (tptr)
	    {
	      if (tptr->sptr == sptr)
		i++;
	      tptr = tptr->next;
	    }
	  fprintf(outfile,"%i ", i);
	  
	  sptr = sptr->next;
	  if (sptr)
	    fprintf(outfile,",\n");
	  else
	    fprintf(outfile,"};\n\n");
	}
    }

}

/****************************************************************/
/*           print route statements in SAOL program             */
/****************************************************************/

void printreflectionroute(void)

{
  tnode * tptr, * iptr;
  int i, j;

  i = 0;
  tptr = groot;
  while (tptr)
    {
      if (tptr->ttype == S_ROUTEDEF)
	{
	  j = 0;
	  iptr = tptr->down->next->next->next->next->down;
	  while (iptr)
	    {
	      if (iptr->ttype == S_IDENT)
		j++;
	      iptr = iptr->next;
	    }
	  fprintf(outfile,"#define CSYS_ROUTE_INUM_%i %i\n\n",
		  i, j);
	  fprintf(outfile,"int csys_route%i_instr[CSYS_ROUTE_INUM_%i] = {\n",
		  i,i);
	  iptr = tptr->down->next->next->next->next->down;
	  while (iptr)
	    {
	      if (iptr->ttype == S_IDENT)
		{
		  fprintf(outfile," %i",tptr->sptr->vol);
		  if (iptr->next)
		    fprintf(outfile,",");
		  else
		    fprintf(outfile,"};\n\n");
		}
	      iptr = iptr->next;
	    }
	  i++;
	}
      tptr = tptr->next;
    }

  fprintf(outfile,"#define CSYS_ROUTENUM %i\n\n", i);
  
  if (!i) 
    fprintf(outfile,"csys_routestruct csys_route[1];\n\n");
  else
    {
      tptr = groot;
      i = 0;
      fprintf(outfile,"csys_routestruct csys_route[CSYS_ROUTENUM] = {\n");
      while (tptr)
	{
	  if (tptr->ttype == S_ROUTEDEF)
	    {
	      if (i)
		fprintf(outfile, "\n, ");

	      /* int bus */

	      fprintf(outfile, "%i, ",tptr->sptr->vol);
 
	      /* int ninstr */

	      fprintf(outfile, "CSYS_ROUTE_INUM_%i, ",i);
	      
	      /* int * instr */

	      fprintf(outfile, "&(csys_route%i_instr[0]) ",i);

	      i++;
	    }
	  tptr = tptr->next;
	}
      fprintf(outfile,"};\n\n");
    }

}

/****************************************************************/
/*           print send statements in SAOL program             */
/****************************************************************/

void printreflectionsend(void)

{
  tnode * tptr, * iptr;
  int i, j;

  i = 0;
  tptr = groot;
  while (tptr)
    {
      if (tptr->ttype == S_SENDDEF)
	{
	  j = 0;
	  iptr = tptr->down->next->next->next->next->next->next->down;
	  while (iptr)
	    {
	      if (iptr->ttype == S_NAME)
		j++;
	      iptr = iptr->next;
	    }
	  fprintf(outfile,"#define CSYS_SEND_BNUM_%i %i\n\n",
		  i, j);
	  fprintf(outfile,"int csys_send%i_bus[CSYS_SEND_BNUM_%i] = {\n",
		  i,i);
	  iptr = tptr->down->next->next->next->next->next->next->down;
	  while (iptr)
	    {
	      if (iptr->ttype == S_NAME)
		{
		  fprintf(outfile," %i", iptr->sptr->vol);
		  if (iptr->next)
		    fprintf(outfile,",");
		  else
		    fprintf(outfile,"};\n\n");
		}
	      iptr = iptr->next;
	    }
	  i++;
	}
      tptr = tptr->next;
    }

  fprintf(outfile,"#define CSYS_SENDNUM %i\n\n", i);
  
  if (!i) 
    fprintf(outfile,"csys_sendstruct csys_send[1];\n\n");
  else
    {
      tptr = groot;
      i = 0;
      fprintf(outfile,"csys_sendstruct csys_send[CSYS_SENDNUM] = {\n");
      while (tptr)
	{
	  if (tptr->ttype == S_SENDDEF)
	    {
	      if (i)
		fprintf(outfile, "\n, ");

	      /* int instr */

	      fprintf(outfile, "%i, ",tptr->sptr->vol);
 
	      /* int nbus */

	      fprintf(outfile, "CSYS_SEND_BNUM_%i, ",i);
	      
	      /* int * instr */

	      fprintf(outfile, "&(csys_send%i_bus[0]) ",i);

	      i++;
	    }
	  tptr = tptr->next;
	}
      fprintf(outfile,"};\n\n");
    }

}

/****************************************************************/
/*    top-level routine to print the reflection interface       */
/****************************************************************/

void printreflection(void)

{
  sigsym * sptr;

  if (!cin && !session && !adriver_reflection(ain))
    return;

  fprintf(outfile, "\n/*  Reflection interface starts here */\n\n");

  printreflectionglobalvars();   /* fill global variable info      */
  printreflectiontarget();       /* fill target info               */
  printreflectionlabels();       /* fill SASL label info           */
  printreflectionpresets();      /* fill MIDI preset info          */
  printreflectionsamples();      /* fill sample file info          */
  printreflectionbus();          /* fill bus info                  */
  printreflectionroute();        /* fill route info                */
  printreflectionsend();         /* fill send info                 */

  /* during this pass, make vars array and param array */

  sptr = instrnametable;
  while (sptr != NULL)
    {
      printreflectionvars(sptr);
      sptr = sptr->next;
    }      

  /* during this pass, fill in complete data structure */

  fprintf(outfile,"csys_instrstruct csys_instr[CSYS_INSTRNUM] = {\n");

  sptr = instrnametable;
  while (sptr != NULL)
    {
      printreflectioninstr(sptr);
      sptr = sptr->next;
    }     

  fprintf(outfile,"};\n\n");

  fprintf(outfile, "\n/*  Reflection interface ends here */\n\n");
}


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*  This function family prints out data lists for the          */
/*     various kinds of instrument data sources.                */
/*______________________________________________________________*/

/****************************************************************/
/*      engine variable declarations for all instrs            */
/****************************************************************/

void printinstrlists(int * totlines, int * usesdyn, int * instrnum)

{
  sigsym * sptr;
  sigsym * bptr;
  char * val;

  sptr = instrnametable;
  while (sptr != NULL)
    {
      if (sptr->score)
	{
	  initscoreinstr(RELTSTAMP, sptr);
	  (*totlines) += sptr->score;
	}
      if (sptr->ascore)
	{
	  initscoreinstr(ABSTSTAMP, sptr);
	  (*totlines) += sptr->ascore;
	}
      if (sptr->midi)
	{
	  initmidiinstr(RELTSTAMP, sptr, totlines);
	}
      if (sptr->amidi)
	{
	  initmidiinstr(ABSTSTAMP, sptr, totlines);
	}
      if (sptr->dyn)
	{
	  *usesdyn = 1;
	}
      if (sptr->effects)
	{
	  fprintf(outfile,"instr_line e_%s[%i];\n\n",sptr->val,
		  sptr->effects);
	  (*totlines) += sptr->effects;
	}
      if (sptr->startup)
	{
	  fprintf(outfile,"instr_line u_startup[1];\n");
	  (*totlines)++;
	}
      if ((cmidi || session) && sptr->miditag)
	{
	  val = dupunderscore(sptr->val);
	  fprintf(outfile,"instr_line cm_%s[MAXCNOTES];\n", sptr->val);
	  fprintf(outfile,"instr_line * cm_%sfirst;\n", val);
	  fprintf(outfile,"instr_line * cm_%slast;\n", val);
	  fprintf(outfile,"instr_line * cm_%send;\n", val);
	  fprintf(outfile,"instr_line * cm_%snext;\n\n", val);
	  free(val);
	}

      if (csasl)
	{
	  if (bitfile && bitsymin)
	    {
	      bptr = getvsym(&bitsymin,sptr->val);
	      if (bptr)
		fprintf(outfile,"#define CSYS_SASL_INSTR_%s %i\n\n",
			bptr->defnode->val, (*instrnum)++);
	      else
		internalerror("writepre.c", "bitsymin");
	    }
	  else
	    {
	      fprintf(outfile,"#define CSYS_SASL_INSTR_%s %i\n\n",
		      sptr->val,(*instrnum)++);
	    }
	  val = dupunderscore(sptr->val);
	  fprintf(outfile,"instr_line cs_%s[MAXCNOTES];\n", sptr->val);
	  fprintf(outfile,"instr_line * cs_%sfirst;\n", val);
	  fprintf(outfile,"instr_line * cs_%slast;\n", val);
	  fprintf(outfile,"instr_line * cs_%send;\n", val);
	  fprintf(outfile,"instr_line * cs_%snext;\n\n", val);
	  free(val);
	}
      sptr = sptr->next;
    }
}


/****************************************************************/
/*      engine_init() assignments for all instr variables      */
/****************************************************************/

void printinstrlistassign(void)

{
  sigsym * sptr;
  char * val;

  sptr = instrnametable;
  while (sptr != NULL)
    {
      if (sptr->score)
	{
	  initscoreinstrassign(RELTSTAMP, sptr);
	}
      if (sptr->ascore)
	{
	  initscoreinstrassign(ABSTSTAMP, sptr);
	}
      if (sptr->midi)
	{
	  initmidiinstrassign(RELTSTAMP, sptr);
	}
      if (sptr->amidi)
	{
	  initmidiinstrassign(ABSTSTAMP, sptr);
	}
      if (sptr->dyn)
	{
	  val = dupunderscore(sptr->val); 
	  fprintf(outfile,"  EV(d_%sfirst) = &(EV(d_%s)[1]);\n",val,sptr->val);
	  fprintf(outfile,"  EV(d_%slast) = &(EV(d_%s)[0]);\n",val,sptr->val);
	  fprintf(outfile,"  EV(d_%send) = &(EV(d_%s)[MAXLINES-1]);\n",val,sptr->val);
	  free(val); 
	}
      if ((cmidi || session) && sptr->miditag)
	{
	  val = dupunderscore(sptr->val);
	  fprintf(outfile,"  EV(cm_%sfirst) = &EV(cm_%s[1]);\n", val,sptr->val);
	  fprintf(outfile,"  EV(cm_%slast) = &EV(cm_%s[0]);\n", val,sptr->val);
	  fprintf(outfile,"  EV(cm_%send) = &EV(cm_%s[MAXCNOTES-1]);\n",val,sptr->val);
	  free(val);
	}
      if (csasl)
	{
	  val = dupunderscore(sptr->val);
	  fprintf(outfile,"  EV(cs_%sfirst) = &EV(cs_%s)[1];\n", val,sptr->val);
	  fprintf(outfile,"  EV(cs_%slast) = &EV(cs_%s)[0];\n", val,sptr->val);
	  fprintf(outfile,"  EV(cs_%send) = &EV(cs_%s)[MAXCNOTES-1];\n", val,sptr->val);
	  free(val);
	}
      sptr = sptr->next;
    }
}


/****************************************************************/
/*     declare and init true constant vars for all instrs       */
/****************************************************************/

void printinstrlistconstant(void)

{
  sigsym * sptr;

  sptr = instrnametable;
  while (sptr != NULL)
    {
      if (sptr->score)
	{
	  initscoreinstrconstant(RELTSTAMP, sptr);
	}
      if (sptr->ascore)
	{
	  initscoreinstrconstant(ABSTSTAMP, sptr);
	}
      if (sptr->midi)
	{
	  initmidiinstrconstant(RELTSTAMP, sptr);
	}
      if (sptr->amidi)
	{
	  initmidiinstrconstant(ABSTSTAMP, sptr);
	}
      sptr = sptr->next;
    }
}


/****************************************************************/
/*                 prints cm declarations for presets           */
/****************************************************************/

void printpresetinit(char * name)

{
  int i;
  sigsym * sptr = instrpresets;
  char * val;

  for (i = 0; i <= maxmidipreset;i++)
    {
      fprintf(outfile,"  EV(cmp_%s)[%i] = ", name, i);
      if ((sptr == NULL) || (i != sptr->width))
	fprintf(outfile,"NULL;\n");
      else
	{
	  val = dupunderscore(sptr->defnode->sptr->val);
	  fprintf(outfile,"&EV(cm_%s%s);\n",val,name);
	  free(val);
	  sptr = sptr->next;
	}
    }
  fprintf(outfile,"\n");
}


/****************************************************************/
/*                 prints cm declarations for presets           */
/****************************************************************/

void printextchaninit(char * name)

{
  int i;
  sigsym * sptr = NULL;
  char * val;

  for (i = 0; i < cmaxchan; i++)
    {
      fprintf(outfile,"  EV(cme_%s)[%i] = ", name, i);
      if ((i & 15) == 0)
	sptr = instrpresets;
      if (sptr == NULL)
	fprintf(outfile,"NULL;\n");
      else
	{
	  val = dupunderscore(sptr->defnode->sptr->val);
	  fprintf(outfile,"&EV(cm_%s%s);\n",val,name);
	  free(val);
	  sptr = sptr->next ? sptr->next : instrpresets;
	}
    }
  fprintf(outfile,"\n");

}

/****************************************************************/
/*            prints cm declaration for preset number           */
/****************************************************************/

void printextchanpreset(void)

{
  int i;
  int j = 0;
  sigsym * sptr = instrpresets;

  fprintf(outfile,"int cme_preset[CSYS_MAXEXTCHAN] = {\n");
  for (i = 0; i < cmaxchan; i++)
    {
      if ((sptr == NULL) || (i != sptr->width))
	fprintf(outfile,"CSYS_MAXPRESETS");
      else
	{
	  fprintf(outfile,"%i",i);
	  sptr = sptr->next;
	}
      if (i != cmaxchan-1)
	fprintf(outfile,",");
      if (!((++j)%5))
	fprintf(outfile,"\n");
    }
  fprintf(outfile,"};\n\n");

}

/****************************************************************/
/*                 prints cs declarations for presets           */
/****************************************************************/

void printsaslinit(char * name)

{
  int j = 0;
  sigsym * sptr = instrnametable;
  char * val;

  while (sptr != NULL)
    {
      fprintf(outfile,"  EV(cs_%s)[%i] = ",name, j);
      val = dupunderscore(sptr->val);
      fprintf(outfile,"&EV(cs_%s%s);\n",val,name);
      free(val);
      sptr = sptr->next;
      j++;
    }
  fprintf(outfile,"\n");

  if (strcmp(name,"first") && strcmp(name,"last"))
    return;

  if (abssasl->instrroot)
    {
      j = 0;
      sptr = instrnametable;
      while (sptr != NULL)
	{
	  fprintf(outfile, "  EV(csa_%s)[%i] = ", name, j);
	  if (sptr->ascore)
	    {
	      val = dupunderscore(sptr->val);
	      fprintf(outfile,"&EV(sa_%s%s);\n",val,name);
	      free(val);
	    }
	  else
	    fprintf(outfile,"NULL;\n");

	  sptr = sptr->next;
	  j++;
	}
      fprintf(outfile,"\n");
    }

  if (allsasl->instrroot)
    {
      j = 0;
      sptr = instrnametable;
      while (sptr != NULL)
	{
	  fprintf(outfile, "  EV(css_%s)[%i] = ", name, j);
	  if (sptr->score)
	    {
	      val = dupunderscore(sptr->val);
	      fprintf(outfile,"&EV(s_%s%s);\n",val,name);
	      free(val);
	    }
	  else
	    fprintf(outfile,"NULL;\n");
	  sptr = sptr->next;
	  j++;
	}
      fprintf(outfile,"\n");
    }
}

/****************************************************************/
/*      prints cs declarations for table and variable size      */
/****************************************************************/

void printsaslsizes(void)

{ 
  int i;
  sigsym * iptr;

  fprintf(outfile,"int csys_instrtablesize[CSYS_MAXSASLINSTR] = {\n");

  iptr = instrnametable;
  i = 0;

  while (iptr != NULL)
    {
      fprintf(outfile,"%s_ENDTBL%s",iptr->val,
	      iptr->next ? "," : "};\n\n");

      iptr = iptr->next;
      if ((i++ == 4) && iptr)
	{
	  fprintf(outfile,"\n");
	  i = 0;
	}
    }

  fprintf(outfile,"int csys_instrvarsize[CSYS_MAXSASLINSTR] = {\n");

  iptr = instrnametable;
  i = 0;

  while (iptr != NULL)
    {
      fprintf(outfile,"%s_ENDVAR%s",iptr->val,
	      iptr->next ? "," : "};\n\n");

      iptr = iptr->next;
      if ((i++ == 4) && iptr)
	{
	  fprintf(outfile,"\n");
	  i = 0;
	}
    }

}


/****************************************************************/
/*   print declarations for MIDI and SASL control drivers       */
/****************************************************************/

void printcdrivervars(int * instrnum)

{

  if (cmidi || session)
    {
      fprintf(outfile,"#define CSYS_CCPOS %i\n", MIDICTRLPOS);
      fprintf(outfile,"#define CSYS_TOUCHPOS %i\n", MIDITOUCHPOS);
      fprintf(outfile,"#define CSYS_CHTOUCHPOS %i\n", MIDICHTOUCHPOS);
      fprintf(outfile,"#define CSYS_BENDPOS %i\n", MIDIBENDPOS);
      fprintf(outfile,"#define CSYS_EXTPOS %i\n", MIDIEXTPOS);
      fprintf(outfile,"#define CSYS_FRAMELEN %i\n", MIDIFRAMELEN);

      fprintf(outfile,"#define CSYS_MAXPRESETS %i\n\n",maxmidipreset+1);
      fprintf(outfile,"#define CSYS_NULLPROGRAM %i\n\n", null_program);

      fprintf(outfile,"#define CSYS_MAXCINCHAN %i\n\n", cinmaxchan);
      fprintf(outfile,"#define CSYS_MAXEXTCHAN %i\n\n", cmaxchan);

      fprintf(outfile,"instr_line **cmp_first[CSYS_MAXPRESETS];\n");
      fprintf(outfile,"instr_line **cmp_last[CSYS_MAXPRESETS];\n");
      fprintf(outfile,"instr_line **cmp_end[CSYS_MAXPRESETS];\n");
      fprintf(outfile,"instr_line **cmp_next[CSYS_MAXPRESETS];\n\n");

      fprintf(outfile,"instr_line **cme_first[CSYS_MAXEXTCHAN];\n");
      fprintf(outfile,"instr_line **cme_last[CSYS_MAXEXTCHAN];\n");
      fprintf(outfile,"instr_line **cme_end[CSYS_MAXEXTCHAN];\n");
      fprintf(outfile,"instr_line **cme_next[CSYS_MAXEXTCHAN];\n\n");

      fprintf(outfile,"int csys_bank;\n");
      fprintf(outfile,"int csys_banklsb;\n");
      fprintf(outfile,"int csys_bankmsb;\n\n");
    }

  if (csasl)
    {
      fprintf(outfile,"\n#define CSYS_MAXSASLINSTR %i\n",*instrnum);
      fprintf(outfile,"#define CSYS_SASL_NOINSTR %i\n",*instrnum);
      fprintf(outfile,"#define CSYS_SASL_MAXCONTROL %i\n",MAXDCONTROL);
      fprintf(outfile,"#define CSYS_SASL_MAXTABLES %i\n\n",MAXDTABLES);

      fprintf(outfile,"instr_line **cs_first[CSYS_MAXSASLINSTR];\n");
      fprintf(outfile,"instr_line **cs_last[CSYS_MAXSASLINSTR];\n");
      fprintf(outfile,"instr_line **cs_end[CSYS_MAXSASLINSTR];\n");
      fprintf(outfile,"instr_line **cs_next[CSYS_MAXSASLINSTR];\n\n");

      if (abssasl->instrroot)
	{
	  fprintf(outfile, "instr_line **csa_end[CSYS_MAXSASLINSTR];\n");
	  fprintf(outfile, "instr_line **csa_next[CSYS_MAXSASLINSTR];\n\n");
	}

      if (allsasl->instrroot)
	{
	  fprintf(outfile, "instr_line **css_end[CSYS_MAXSASLINSTR];\n");
	  fprintf(outfile, "instr_line **css_next[CSYS_MAXSASLINSTR];\n\n");
	}

      fprintf(outfile,"struct {\n");
      fprintf(outfile,"unsigned short id;\n");
      fprintf(outfile,"unsigned short label;\n");
      fprintf(outfile,"unsigned int fptr;\n");
      fprintf(outfile,"float fval;\n");
      fprintf(outfile,"} saslcontrol[CSYS_SASL_MAXCONTROL];\n\n");
      fprintf(outfile,"int maxsc;\n\n");
      fprintf(outfile,"struct table_cdtype {\n");
      fprintf(outfile,"unsigned short index; /* gtables index */\n");
      fprintf(outfile,"unsigned short tgen;  /* table generator */\n");
      fprintf(outfile,"unsigned int pnum;  /* number of params */\n");
      fprintf(outfile,"float * p;            /* param list */\n");
      fprintf(outfile,"} sasltable[CSYS_SASL_MAXTABLES];\n\n");
      fprintf(outfile,"int maxtb;\n\n");
    }
}


/****************************************************************/
/*   print true constants for MIDI and SASL control drivers     */
/****************************************************************/

void printcdriverconstant(void)

{
  if (cmidi || session)
    printextchanpreset();

  if (csasl)
    printsaslsizes();         
}


/****************************************************************/
/*   engine_init() assigns for MIDI and SASL control drivers   */
/****************************************************************/

void printcdriverassign(void)

{
  if (cmidi || session)
    {
      printpresetinit("first");
      printpresetinit("last");
      printpresetinit("end");
      printpresetinit("next");
      printextchaninit("first");
      printextchaninit("last");
      printextchaninit("end");
      printextchaninit("next");
    }

  if (csasl)
    {
      printsaslinit("first");   
      printsaslinit("last");
      printsaslinit("end");
      printsaslinit("next");
    }
}

/****************************************************************/
/*   print declarations for dynamic instruments                 */
/****************************************************************/

void printdynamiclists(void)

{
  sigsym * sptr;
  char * val;

  sptr = instrnametable;
  while (sptr != NULL)
    {
      if (sptr->dyn)
	{
	  val = dupunderscore(sptr->val); 
	  fprintf(outfile,"instr_line d_%s[MAXLINES];\n", sptr->val);
	  fprintf(outfile, "instr_line * d_%sfirst;\n", val);
	  fprintf(outfile, "instr_line * d_%slast;\n", val);
	  fprintf(outfile, "instr_line * d_%send;\n", val);
	  fprintf(outfile, "instr_line * d_%snext;\n", val);
	  free(val); 
	}
      sptr = sptr->next;
    }
}


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*   These functions print out audio input and output driver.   */
/*    The order in which these functions are flow-dependent.    */
/*______________________________________________________________*/

/****************************************************************/
/*               make the output audio driver                   */
/****************************************************************/

void makeoutputaudiodriver(sigsym * inputbus)

{ 
  int dosync;

  fprintf(outfile,"#define ASYS_HASOUTPUT\n"); 

  if ((ain == aout) && (inputbus != NULL) && (inchannels > 0))
    fprintf(outfile,"#define ASYS_HASINPUT\n");

  dosync = makeaoutsync(aout);

  if (dosync)
    fprintf(outfile,"#define ASYS_KSYNC\n");

  fprintf(outfile,"\n");

  makeaudiodriver(aout);

  fprintf(outfile,"#undef ASYS_HASOUTPUT\n");

  if (dosync)
    fprintf(outfile,"#undef ASYS_KSYNC\n");
}

/****************************************************************/
/*               make the output audio driver                   */
/****************************************************************/

void makeinputaudiodriver(sigsym * inputbus)

{  
  int dosync;

  if ((ain != aout) && (inputbus != NULL) && (inchannels > 0))
    {
      fprintf(outfile,"#define ASYS_HASINPUT\n");

      dosync = ((!makeaoutsync(aout)) && makeainsync(ain));
      
      if (dosync)
	fprintf(outfile,"#define ASYS_KSYNC\n");
      
      fprintf(outfile,"\n");
      makeaudiodriver(ain);
      fprintf(outfile,"\n");
      fprintf(outfile,"#undef ASYS_HASINPUT\n");

      if (dosync)
	fprintf(outfile,"#undef ASYS_KSYNC\n");
    }
}

