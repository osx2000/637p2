
/*
#    Sfront, a SAOL to C translator    
#    This file: ascii kbd control driver for sfront
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


#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>

/****************************************************************/
/****************************************************************/
/*             ascii keyboard control driver for sfront         */ 
/****************************************************************/

/****************************/
/* terminal and signal vars */
/****************************/

struct termios csysi_term_default;     /* to restore stdin */
sig_atomic_t csysi_no_interrupt = 1;   /* flags [cntrl-c]  */

/*************************/
/* keyboard input buffer */
/*************************/

#define CSYSI_INBUFF_SIZE 32

char csysi_inbuff[CSYSI_INBUFF_SIZE];   /* holds new keypresses */
int csysi_inbuff_cnt;                   
int csysi_inbuff_len;

/***********************/
/* keyboard action map */
/***********************/

#define CSYSI_MAPSIZE  256

typedef struct csysi_kinfo {
  unsigned char cmd;          /* CSYS_MIDI_{NOTEON,PROGRAM,CC,NOOP} */
  unsigned char ndata;        /* note number or parameter           */
} csysi_kinfo;

csysi_kinfo csysi_map[CSYSI_MAPSIZE];

/**********************/
/* current note state */
/**********************/

#define CSYSI_NOTESIZE 128
#define CSYSI_DURATION 0.1F

typedef struct csysi_noteinfo {

  unsigned char cmd;  
  float time;

} csysi_noteinfo;

csysi_noteinfo csysi_notestate[CSYSI_NOTESIZE];

int csysi_noteoff_min;   /* lowest pending noteoff     */
int csysi_noteoff_max;   /* highest pending noteoff    */
int csysi_noteoff_num;   /* number of pending noteoffs */

/****************/
/* timeout list */
/****************/

int csysi_noteready[CSYSI_NOTESIZE];
int csysi_noteready_len;

/***********/
/* volume  */
/***********/

#define CSYSI_VOLUME_DEFAULT 64
#define CSYSI_VOLUME_MAX 112
#define CSYSI_VOLUME_MIN 32
#define CSYSI_VOLUME_INCREMENT 8

int csysi_volume = CSYSI_VOLUME_DEFAULT;

/********************/
/* function externs */
/********************/

extern void csysi_signal_handler(int signum);
extern void csysi_kbdmap_init(void);


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*     high-level functions: called by sfront engine            */
/*______________________________________________________________*/

/****************************************************************/
/*             initialization routine for control               */
/****************************************************************/

int csys_setup(void)
     
{
  struct termios term_info;
  int i;

  /******************************/
  /* initialize data structures */
  /******************************/

  csysi_kbdmap_init();

  for (i = 0; i < CSYSI_NOTESIZE; i++)
    csysi_notestate[i].cmd = CSYS_MIDI_NOTEOFF;

  /*************************/
  /* set up signal handler */
  /*************************/

  if ((NSYS_NET == 0) && (signal(SIGINT, csysi_signal_handler) == SIG_ERR))
    {
      printf("Error: Can't set up SIGINT signal handler\n");
      return CSYS_ERROR;
    }

  /****************************************************/
  /* set up terminal: no echo, single-character reads */
  /****************************************************/

  if (tcgetattr(STDIN_FILENO, &csysi_term_default))
    {
      printf("Error: Can't set up terminal (1).\n");
      return CSYS_ERROR;
    }

  term_info = csysi_term_default;

  term_info.c_lflag &= (~ICANON);   
  term_info.c_lflag &= (~ECHO);
  term_info.c_cc[VTIME] = 0;        
  term_info.c_cc[VMIN] = 0;

  if (tcsetattr(STDIN_FILENO, TCSANOW, &term_info))
    {
      printf("Error: Can't set up terminal (2).\n");
      return CSYS_ERROR;
    }

  if (fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK))
    {
      printf("Error: Can't set stdin to O_NONBLOCK.\n");
      return CSYS_ERROR;
    }

  /*********************/
  /* user instructions */
  /*********************/

#if (!defined(ASYS_OUTDRIVER_LINUX))

  fprintf(stderr, 
	  "\nInput Driver Instructions for -cin ascii:\n");
  fprintf(stderr, 
	  "{a-z}: notes, {0-9}: MIDI presets, {+,-} volume, cntrl-C exits.\n");

#if (!defined(ASYS_OUTDRIVER_COREAUDIO))
  fprintf(stderr, 
	  "If autorepeat interferes, try 'xset -r' to disable it.\n\n");
#endif

#endif
  
  return CSYS_DONE;
}

/****************************************************************/
/*             polling routine for new data                     */
/****************************************************************/

int csys_newdata(void)
     
{
  int i, bottom, top;
  int ret = CSYS_NONE;

  if (csysi_no_interrupt)
    {
      /* see if any NoteOn's ready to be turned off */

      if (csysi_noteoff_num)
	{
	  csysi_noteready_len = 0;
	  top = bottom = -1;
	  for (i = csysi_noteoff_min; i <= csysi_noteoff_max; i++)
	    if (csysi_notestate[i].cmd == CSYS_MIDI_NOTEON)
	      {
		if (csysi_notestate[i].time > EV(absolutetime))
		  {
		    if (bottom < 0)
		      bottom = i;
		    top = i;
		  }
		else
		  {
		    csysi_notestate[i].cmd = CSYS_MIDI_NOTEOFF;
		    csysi_noteready[csysi_noteready_len++] = i;
		    csysi_noteoff_num--;
		    ret = CSYS_MIDIEVENTS;
		  }
	      }
	  csysi_noteoff_min = bottom;
	  csysi_noteoff_max = top;
	}

      /* check for new keypresses */

      csysi_inbuff_cnt = 0;
      csysi_inbuff_len = read(STDIN_FILENO, csysi_inbuff, CSYSI_INBUFF_SIZE);
      csysi_inbuff_len = (csysi_inbuff_len >= 0) ? csysi_inbuff_len : 0;
      return (csysi_inbuff_len ? CSYS_MIDIEVENTS : ret);
    }
  else
    return CSYS_MIDIEVENTS;
}

/****************************************************************/
/*                 processes a MIDI event                       */
/****************************************************************/

int csys_midievent(unsigned char * cmd,   unsigned char * ndata, 
	           unsigned char * vdata, unsigned short * extchan,
		   float * fval)

{
  unsigned char i;

  if (csysi_no_interrupt)
    {
      *extchan = 0;

      /* check for new NoteOffs */

      if (csysi_noteready_len)
	{
	  csysi_noteready_len--;
	  *cmd = CSYS_MIDI_NOTEOFF;
	  *ndata = csysi_noteready[csysi_noteready_len];
	  *vdata = 0;
	  return ((csysi_inbuff_len || csysi_noteready_len) ? 
		  CSYS_MIDIEVENTS : CSYS_NONE);
	}

      /* handle new keypresses */

      *cmd = CSYS_MIDI_NOOP;

      while (csysi_inbuff_cnt < csysi_inbuff_len)
	switch (csysi_map[(i = csysi_inbuff[csysi_inbuff_cnt++])].cmd) {
	case CSYS_MIDI_NOTEON:

	  *ndata = csysi_map[i].ndata;

	  if (csysi_notestate[*ndata].cmd == CSYS_MIDI_NOTEOFF)
	    {
	      *cmd = CSYS_MIDI_NOTEON;
	      *vdata = csysi_volume;
	      
	      csysi_notestate[*ndata].cmd = CSYS_MIDI_NOTEON;
	      csysi_notestate[*ndata].time = EV(absolutetime) + CSYSI_DURATION;
	      
	      if (csysi_noteoff_num++)
		{
		  if (*ndata > csysi_noteoff_max)
		    csysi_noteoff_max = *ndata;
		  else
		    if (*ndata < csysi_noteoff_min)
		      csysi_noteoff_min = *ndata;
		}
	      else
		csysi_noteoff_min = csysi_noteoff_max = *ndata;
	      return ((csysi_inbuff_cnt < csysi_inbuff_len) ? 
		      CSYS_MIDIEVENTS : CSYS_NONE);
	    }

	  break;
	case CSYS_MIDI_PROGRAM:
	  *cmd = CSYS_MIDI_PROGRAM;
	  *ndata = csysi_map[i].ndata;
	  return ((csysi_inbuff_cnt < csysi_inbuff_len) ? 
		  CSYS_MIDIEVENTS : CSYS_NONE);
	  break;	
	case CSYS_MIDI_CC:
	  if (csysi_map[i].ndata)
	    {
	      csysi_volume += CSYSI_VOLUME_INCREMENT;
	      if (csysi_volume > CSYSI_VOLUME_MAX)
		csysi_volume = CSYSI_VOLUME_MAX;
	    }
	  else
	    {
	      csysi_volume -= CSYSI_VOLUME_INCREMENT;
	      if (csysi_volume < CSYSI_VOLUME_MIN)
		csysi_volume = CSYSI_VOLUME_MIN;
	    }
	  break;	
	case CSYS_MIDI_NOOP:
	  break;
	default:
	  break;
	}

      return CSYS_NONE;
    }

  /* end session if a cntrl-C clears csysi_no_interrupt */

  *cmd = CSYS_MIDI_ENDTIME;
  *fval = EV(scorebeats);
  return CSYS_NONE;
}

/****************************************************************/
/*                  closing routine for control                 */
/****************************************************************/

void csys_shutdown(void)
     
{
  tcsetattr(STDIN_FILENO, TCSANOW, &csysi_term_default);
}


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*   mid-level functions: called by top-level driver functions  */
/*______________________________________________________________*/

/****************************************************************/
/*             initializes keyboard map                         */
/****************************************************************/

void csysi_kbdmap_init(void)

{
  int i;

  /* MIDI note number map */

  csysi_map['z'].ndata = csysi_map['a'].ndata = csysi_map['q'].ndata = 48; 
  csysi_map['Z'].ndata = csysi_map['A'].ndata = csysi_map['Q'].ndata = 48; 
  csysi_map['x'].ndata = csysi_map['s'].ndata = csysi_map['w'].ndata = 50; 
  csysi_map['X'].ndata = csysi_map['S'].ndata = csysi_map['W'].ndata = 50; 
  csysi_map['c'].ndata = csysi_map['d'].ndata = csysi_map['e'].ndata = 52; 
  csysi_map['C'].ndata = csysi_map['D'].ndata = csysi_map['E'].ndata = 52; 
  csysi_map['v'].ndata = csysi_map['f'].ndata = csysi_map['r'].ndata = 55; 
  csysi_map['V'].ndata = csysi_map['F'].ndata = csysi_map['R'].ndata = 55; 
  csysi_map['b'].ndata = csysi_map['g'].ndata = csysi_map['t'].ndata = 57;
  csysi_map['B'].ndata = csysi_map['G'].ndata = csysi_map['T'].ndata = 57;
  csysi_map['n'].ndata = csysi_map['h'].ndata = csysi_map['y'].ndata = 60; 
  csysi_map['N'].ndata = csysi_map['H'].ndata = csysi_map['Y'].ndata = 60; 
  csysi_map['m'].ndata = csysi_map['j'].ndata = csysi_map['u'].ndata = 62; 
  csysi_map['M'].ndata = csysi_map['J'].ndata = csysi_map['U'].ndata = 62; 
  csysi_map[','].ndata = csysi_map['k'].ndata = csysi_map['i'].ndata = 64; 
  csysi_map['<'].ndata = csysi_map['K'].ndata = csysi_map['I'].ndata = 64; 
  csysi_map['.'].ndata = csysi_map['l'].ndata = csysi_map['o'].ndata = 67; 
  csysi_map['>'].ndata = csysi_map['L'].ndata = csysi_map['O'].ndata = 67; 
  csysi_map['/'].ndata = csysi_map[';'].ndata = csysi_map['p'].ndata = 69;
  csysi_map['\?'].ndata = csysi_map[':'].ndata = csysi_map['P'].ndata = 69;
  csysi_map['\''].ndata = csysi_map['['].ndata = 72; 
  csysi_map['"'].ndata = csysi_map['{'].ndata = 72; 
  csysi_map[']'].ndata = 74; 
  csysi_map['}'].ndata = 74; 
  csysi_map['\\'].ndata = csysi_map['\n'].ndata = 76; 
  csysi_map['|'].ndata = 76; 
  csysi_map[' '].ndata = 48;

  /* set command type */

  for (i = 0; i < CSYSI_MAPSIZE; i++)
    {
      if (csysi_map[i].ndata)
	csysi_map[i].cmd = CSYS_MIDI_NOTEON;
      else
	if (isdigit(i))
	  {
	    csysi_map[i].cmd = CSYS_MIDI_PROGRAM;
	    csysi_map[i].ndata = (i - '0');
	  }
	else
	  if ((i == '+') || (i == '_') || (i == '-') || (i == '='))
	    {
	      csysi_map[i].cmd = CSYS_MIDI_CC;
	      csysi_map[i].ndata = ((i == '+') || (i == '='));
	    }
	  else
	    csysi_map[i].cmd = CSYS_MIDI_NOOP;
    }

}



/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*                 low-level functions                          */
/*______________________________________________________________*/

/****************************************************************/
/*                 SIGINT signal handler                        */
/****************************************************************/

void csysi_signal_handler(int signum)

{

  if (csysi_no_interrupt)
    {
      csysi_no_interrupt = 0;
    }
  else
    {
      exit(129); /* emergency shutdown */
    }

}


