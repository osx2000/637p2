
/*
#    Sfront, a SAOL to C translator    
#    This file: Code generaton: core wavetables
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


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*                                                              */
/* Top-level functions for wavetable logging and identification */
/*                                                              */
/*______________________________________________________________*/


/*********************************************************/
/* returns 1 if S_IDENT is a wavetable generator, else 0 */
/*********************************************************/

int wavegeneratorname(tnode * ident)

{
 
 switch (ident->val[0]) {
  case 'b':
    if (!(strcmp(ident->val,"buzz")))
      return 1;
    return 0;
  case 'c':
    if (!(strcmp(ident->val,"concat")))
      return 1;
    if (!(strcmp(ident->val,"cubicseg")))
      return 1;
    return 0;
  case 'd':
    if (!(strcmp(ident->val,"data")))
      return 1;
    return 0;
  case 'e':
    if (!(strcmp(ident->val,"empty")))
      return 1;
    if (!(strcmp(ident->val,"expseg")))
      return 1;
    return 0;
  case 'h':
    if (!(strcmp(ident->val,"harm")))
      return 1;
    if (!(strcmp(ident->val,"harm_phase")))
      return 1;
    return 0;
  case 'l':
    if (!(strcmp(ident->val,"lineseg")))
      return 1;
    return 0;
  case 'p':
    if (!(strcmp(ident->val,"periodic")))
      return 1;
    if (!(strcmp(ident->val,"polynomial")))
      return 1;
    return 0;
  case 'r':
    if (!(strcmp(ident->val,"random")))
      return 1;
    return 0;
  case 's':
    if (!(strcmp(ident->val,"sample")))
      return 1;
    if (!(strcmp(ident->val,"spline")))
      return 1;
    if (!(strcmp(ident->val,"step")))
      return 1;
    return 0;
  case 'w':
    if (!(strcmp(ident->val,"window")))
      return 1;
    return 0;
  }
 return 0; /* will never happen */
}

/*********************************************************/
/*        incremenets counter in has structure           */
/*********************************************************/

void haswavegenerator(tnode * ident)

{
 
  switch (ident->val[0]) {
  case 'b':
    if (!(strcmp(ident->val,"buzz")))
      {
	has.w_buzz++;
	return;
      }
    return;
  case 'c':
    if (!(strcmp(ident->val,"concat")))
      {
	has.w_concat++;
	return;
      }
    if (!(strcmp(ident->val,"cubicseg")))
      {
	has.w_cubicseg++;
	return;
      }
    return;
  case 'd':
    if (!(strcmp(ident->val,"data")))
      {
	has.w_data++;
	return;
      }
    return;
  case 'e':
    if (!(strcmp(ident->val,"empty")))
      {
	has.w_empty++;
	return;
      }
    if (!(strcmp(ident->val,"expseg")))
      {
	has.w_expseg++;
	return;
      }
    return;
  case 'h':
    if (!(strcmp(ident->val,"harm")))
      {
	has.w_harm++;
	return;
      }
    if (!(strcmp(ident->val,"harm_phase")))
      {
	has.w_harm_phase++;
	return;
      }
    return;
  case 'l':
    if (!(strcmp(ident->val,"lineseg")))
      {
	has.w_lineseg++;
	return;
      }
    return;
  case 'p':
    if (!(strcmp(ident->val,"periodic")))
      {
	has.w_periodic++;
	return;
      }
    if (!(strcmp(ident->val,"polynomial")))
      {
	has.w_polynomial++;
	return;
      }
    return;
  case 'r':
    if (!(strcmp(ident->val,"random")))
      {
	has.w_random++;
	return;
      }
    return;
  case 's':
    if (!(strcmp(ident->val,"sample")))
      {
	has.w_sample++;
	return;
      }
    if (!(strcmp(ident->val,"spline")))
      {
	has.w_spline++;
	return;
      }
    if (!(strcmp(ident->val,"step")))
      {
	has.w_step++;
	return;
      }
    return;
  case 'w':
    if (!(strcmp(ident->val,"window")))
      {
	has.w_window++;
	return;
      }
    return;
  default:
    return;
  }
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*                                                              */
/*      Top-level functions for wavetable code generation.      */
/*                                                              */
/*______________________________________________________________*/

extern char * makewstring(char *);

/*********************************************************/
/*  declares local variables for table generators        */
/*                                                       */
/* modes values:                                         */
/*                                                       */
/*        S_SASLFILE: SASL table commands (readscore.c)  */
/*        S_OPCODE  : opcode tables       (writeop.c)    */
/*        S_INSTR   : instr tables        (writeorc.c)   */
/*        S_GLOBAL  : global tables       (writeorc.c)   */
/*                                                       */
/*********************************************************/

void wavegeneratorvar(sigsym * sptr)

{
  tnode * wname = sptr->defnode->down->next;
  tnode * ident = sptr->defnode->down->next->next->next;
  tnode * tptr =  sptr->defnode->down->next->next->next->next->next->down;
  sampleinfo * sinfo;
  int i,j;
  int lc = 0;
  char * lname = makewstring(wname->val);

  /* no variables declarations needed for constant tables */

  if ((sptr->defnode->vol == CONSTANT) && (sptr->defnode->usesinput))
    return;

  if (tptr == NULL)
    {
      printf("Error: Generator needs size parameter.\n");
      showerrorplace(sptr->defnode->down->linenum,
		     sptr->defnode->down->filename);
    }

  z[lc++] = "int %2$s_size;";
  z[lc++] = "float %2$s_rounding;";
  tptr = tptr->next;
  if (tptr != NULL)
    tptr = tptr->next;

  switch (ident->val[0]) {
  case 'b':
    if (!(strcmp(ident->val,"buzz")))
      {
	z[lc++]="float %2$s_scale;";
	z[lc++]="float %2$s_base;";
	z[lc++]="float %2$s_index;";
	z[lc++]="float %2$s_acc;";
	i = 1;
	while (tptr != NULL)
	  {
	    if (tptr->ttype == S_EXPR)
	      {
		switch (i) {
		case 1:
		  z[lc++]="int %2$s_nharm;";
		  break;
		case 2:
		  z[lc++]="int %2$s_lowharm;";
		  break;
		case 3:
		  z[lc++]="float %2$s_rolloff;";
		  break;
		default:
		  printf("Error: Too many parameters for buzz generator.\n");
		  showerrorplace(sptr->defnode->down->linenum,
				 sptr->defnode->down->filename);
		}
		i++;
	      }
	    tptr = tptr->next;
	  }
	if (i<4)
	  {
	    printf("Error: Too few parameters for buzz generator.\n");
	    showerrorplace(sptr->defnode->down->linenum,
			   sptr->defnode->down->filename);
	  }
	printwavesymblock2(lc, wname); 
	break;
      }
    break;
  case 'c':
    if (!(strcmp(ident->val,"concat")))
      {
	i = 1;
	while (tptr != NULL)
	  {
	    if (tptr->ttype == S_EXPR)
	      {
		mz(lc); sprintf(z[lc++],"int %s_ft%i;",lname,i);
		i++;
	      }
	    tptr = tptr->next;
	  }
	printwavesymblock2(lc, wname); 
	break;
      }
    if (!(strcmp(ident->val,"cubicseg")))
      {
	i = 1; j = 0;
	z[lc++]="float %2$s_xf;";
	z[lc++]="float %2$s_Q;";
	z[lc++]="float %2$s_R;";
	z[lc++]="float %2$s_S;";
	z[lc++]="float %2$s_T;";
	while (tptr != NULL)
	  {
	    if (tptr->ttype == S_EXPR)
	      {
		switch (j) {
		case 0:
		  mz(lc); sprintf(z[lc++],"float %s_infl%i;",lname,i);
		  mz(lc); sprintf(z[lc++],"float %s_a%i;",lname,i);
		  mz(lc); sprintf(z[lc++],"float %s_b%i;",lname,i);
		  mz(lc); sprintf(z[lc++],"float %s_c%i;",lname,i);
		  mz(lc); sprintf(z[lc++],"float %s_d%i;",lname,i);
		  break;
		case 1:
		  mz(lc); sprintf(z[lc++],"float %s_y%i;",lname, 2*i-1);
		  break;
		case 2:
		  mz(lc); sprintf(z[lc++],"float %s_x%i;",lname,i);
		  break;
		case 3:
		  mz(lc); sprintf(z[lc++],"float %s_y%i;",lname, 2*i);
		  break;
		}
		j++;
		if (j == 4)
		  {
		    j = 0; i++;
		  }
	      }
	    tptr = tptr->next;
	  }

	/* for correct compilation for 'not enough x' error condition */

	mz(lc);
	if (j != 0)
	  i++;
	sprintf(z[lc++],"float %s_infl%i;",lname,i);

	printwavesymblock2(lc, wname); 
	break;
      }
    break;
  case 'd':
    if (!(strcmp(ident->val,"data")))
      {
	i = 1;
	while (tptr != NULL)
	  {
	    if (tptr->ttype == S_EXPR)
	      {
		mz(lc); sprintf(z[lc++],"float %s_p%i;",lname,i);
		i++;
	      }
	    tptr = tptr->next;
	  }
	printwavesymblock2(lc, wname); 
	break;
      }
    break;
  case 'e':
    if (!(strcmp(ident->val,"empty")))
      {
	if (tptr != NULL)
	  {
	    printf("Error: Too many parameters for empty generator.\n");
	    showerrorplace(sptr->defnode->down->linenum,
			   sptr->defnode->down->filename);
	  }
	printwavesymblock2(lc, wname); 
	break;
      }
    if (!(strcmp(ident->val,"expseg")))
      {
	i = 1;
	while (tptr != NULL)
	  {
	    if (tptr->ttype == S_EXPR)
	      {
		if ((i % 2) == 1)
		  {
		    mz(lc); sprintf(z[lc++],"float %s_x%i;",lname,(i/2)+1);
		    mz(lc); sprintf(z[lc++],"float %s_d%i;",lname,(i/2)+1);
		    mz(lc); sprintf(z[lc++],"float %s_e%i;",lname,(i/2)+1);
		  }
		else
		  {
		    mz(lc); sprintf(z[lc++],"float %s_y%i;",lname,(i/2));
		  }
		i++;
	      }
	    tptr = tptr->next;
	  }
	if ((i % 2) == 0)
	  {
	    mz(lc); sprintf(z[lc++],"float %s_y%i;",lname,(i/2));
	  }
	printwavesymblock2(lc, wname); 
	break;
      }
    break;
  case 'h':
    if (!(strcmp(ident->val,"harm")))
      {
	z[lc++]="float %2$s_base;";
	z[lc++]="float %2$s_index;";
	i = 1;
	while (tptr != NULL)
	  {
	    if (tptr->ttype == S_EXPR)
	      {
		mz(lc); sprintf(z[lc++],"float %s_f%i;",lname,i);
		i++;
	      }
	    tptr = tptr->next;
	  }
	printwavesymblock2(lc, wname); 
	break;
      }
    if (!(strcmp(ident->val,"harm_phase")))
      {
	z[lc++]="float %2$s_base;";
	z[lc++]="float %2$s_index;";
	i = 1;
	while (tptr != NULL)
	  {
	    if (tptr->ttype == S_EXPR)
	      {
		if ((i % 2) == 1)
		  {
		    mz(lc); sprintf(z[lc++],"float %s_f%i;",lname,(i/2)+1);
		  }
		else
		  {
		    mz(lc); sprintf(z[lc++],"float %s_ph%i;",lname,(i/2));
		  }
		i++;
	      }
	    tptr = tptr->next;
	  }
	if ((i % 2) == 0)
	  {
	    printf("Error: F parameter w/o a ph in harm_phase generator.\n");
	    showerrorplace(sptr->defnode->down->linenum,
			   sptr->defnode->down->filename);
	  }
	printwavesymblock2(lc, wname); 
	break;
      }
    break;
  case 'l':
    if (!(strcmp(ident->val,"lineseg")))
      {
	i = 1;
	while (tptr != NULL)
	  {
	    if (tptr->ttype == S_EXPR)
	      {
		if ((i % 2) == 1)
		  {
		    mz(lc); sprintf(z[lc++],"float %s_x%i;",lname,(i/2)+1);
		    mz(lc); sprintf(z[lc++],"float %s_d%i;",lname,(i/2)+1);
		  }
		else
		  {
		    mz(lc); sprintf(z[lc++],"float %s_y%i;",lname,(i/2));
		  }
		i++;
	      }
	    tptr = tptr->next;
	  }
	if ((i % 2) == 0)
	  {
	    mz(lc); sprintf(z[lc++],"float %s_y%i;",lname,(i/2));
	  }
	printwavesymblock2(lc, wname); 
	break;
      }
    break;
  case 'p':
    if (!(strcmp(ident->val,"periodic")))
      {
	z[lc++]="float %2$s_base;";
	z[lc++]="float %2$s_index;";
	i = 1;
	while (tptr != NULL)
	  {
	    if (tptr->ttype == S_EXPR)
	      {
		switch(i % 3) {
		case 1:
		  mz(lc); sprintf(z[lc++],"float %s_p%i;",lname,(i/3)+1);
		  break;
		case 2:
		  mz(lc); sprintf(z[lc++],"float %s_f%i;",lname,(i/3)+1);
		  break;
		case 0:
		  mz(lc); sprintf(z[lc++],"float %s_ph%i;",lname,(i/3));
		  break;
		}
		i++;
	      }
	    tptr = tptr->next;
	  }
	if ((i % 3) != 1)
	  {
	    printf("Error: Incomplete data triplet in periodic generator.\n");
	    showerrorplace(sptr->defnode->down->linenum,
			   sptr->defnode->down->filename);
	  }
	printwavesymblock2(lc, wname); 
	break;
      }
    if (!(strcmp(ident->val,"polynomial")))
      {
	i = 1;
	z[lc++]="float %2$s_scale;";
	z[lc++]="float %2$s_index;";
	z[lc++]="float %2$s_acc;";
	while (tptr != NULL)
	  {
	    if (tptr->ttype == S_EXPR)
	      {
		switch (i) {
		case 1:
		  z[lc++]="float %2$s_xmin;";
		  break;
		case 2:
		  z[lc++]="float %2$s_xmax;";
		  break;
		default:
		  mz(lc); sprintf(z[lc++],"float %s_a%i;",lname,i-3);
		  break;
		}
		i++;
	      }
	    tptr = tptr->next;
	  }
	printwavesymblock2(lc, wname); 
	break;
      }
    break;
  case 'r':
    if (!(strcmp(ident->val,"random")))
      {
	i = 1;
	z[lc++]="float %2$s_c1;";
	z[lc++]="float %2$s_x;";
	z[lc++]="float %2$s_y;";
	while (tptr != NULL)
	  {
	    if (tptr->ttype == S_EXPR)
	      {
		switch (i) {
		case 1:
		  z[lc++]="int %2$s_dist;";
		  break;
		case 2:
		  z[lc++]="float %2$s_p1;";
		  break;
		case 3:
		  z[lc++]="float %2$s_p2;";
		  break;
		default:
		  printf("Error: Too many parameters for random generator.\n");
		  showerrorplace(sptr->defnode->down->linenum,
				 sptr->defnode->down->filename);
		}
		i++;
	      }
	    tptr = tptr->next;
	  }
	if (i<3)
	  {
	    printf("Error: Insufficient parameters for random generator.\n");
	    showerrorplace(sptr->defnode->down->linenum,
			   sptr->defnode->down->filename);
	  }
	printwavesymblock2(lc, wname); 
	break;
      }
    break;
  case 's':
    if (!(strcmp(ident->val,"sample")))
      {
	z[lc++]="int %2$s_skip;";

	sinfo = (sampleinfo *) 
	  sptr->defnode->down->next->next->next->next->next->down->next->next->ibus;

	mz(lc);  
	sprintf(z[lc++],"unsigned char %s_c[%i];",lname, sinfo->framebytes);

	printwavesymblock2(lc, wname); 
	break;
      }
    if (!(strcmp(ident->val,"spline")))
      {
	z[lc++]="float %2$s_xf;";
	z[lc++]="float %2$s_Q;";
	z[lc++]="float %2$s_R;";
	z[lc++]="float %2$s_S;";
	z[lc++]="float %2$s_x1;";
	z[lc++]="float %2$s_y1;";
	z[lc++]="float %2$s_k1;";
	i = 1;
	while (tptr != NULL)
	  {
	    if (tptr->ttype == S_EXPR)
	      {
		if (!(i % 3))
		  {
		    mz(lc);
		    sprintf(z[lc++],"float %s_y%i;",lname,(i/3)+1);
		    mz(lc);
		    sprintf(z[lc++],"float %s_x%i;",lname,(i/3)+1);
		    mz(lc);
		    sprintf(z[lc++],"float %s_k%i;",lname,(i/3)+1);
		    mz(lc);
		    sprintf(z[lc++],"float %s_a%i;",lname,(i/3));
		    mz(lc);
		    sprintf(z[lc++],"float %s_b%i;",lname,(i/3));
		    mz(lc);
		    sprintf(z[lc++],"float %s_c%i;",lname,(i/3));
		    mz(lc);
		    sprintf(z[lc++],"float %s_d%i;",lname,(i/3));
		  }
		i++;
	      }
	    tptr = tptr->next;
	  }
	printwavesymblock2(lc, wname); 
	break;
      }
    if (!(strcmp(ident->val,"step")))
      {
	i = 1;
	z[lc++]="float %2$s_x1;";
	while (tptr != NULL)
	  {
	    if (tptr->ttype == S_EXPR)
	      {
		if (!(i % 2))
		  {
		    mz(lc); sprintf(z[lc++],"float %s_y%i;",lname,(i/2));
		    mz(lc); sprintf(z[lc++],"float %s_x%i;",lname,(i/2)+1);
		  }
		i++;
	      }
	    tptr = tptr->next;
	  }
	printwavesymblock2(lc, wname); 
	break;
      }
    break;
  case 'w':
    if (!(strcmp(ident->val,"window")))
      {
	z[lc++]="float %2$s_c1;";
	z[lc++]="float %2$s_c2;";
	z[lc++]="double %2$s_d1;";
	z[lc++]="double %2$s_d2;";
	z[lc++]="double %2$s_d3;";
	i = 1;
	while (tptr != NULL)
	  {
	    if (tptr->ttype == S_EXPR)
	      {
		switch (i) {
		case 1:
		  z[lc++]="int %2$s_type;";
		  break;
		case 2:
		  z[lc++]="float %2$s_p;";
		  break;
		default:
		  printf("Error: Too many parameters for window generator.\n");
		  showerrorplace(sptr->defnode->down->linenum,
				 sptr->defnode->down->filename);
		}
		i++;
	      }
	    tptr = tptr->next;
	  }
	if (i == 1)
	  {
	    printf("Error: Type parameter needed for window generator.\n");
	    showerrorplace(sptr->defnode->down->linenum,
			   sptr->defnode->down->filename);
	  }
	printwavesymblock2(lc, wname); 
	break;
      }
    break;
  default:
    break;
  }
  free(lname);
}

extern void createwavetable(sigsym *, char *, int);


/*********************************************************/
/*       high-level routine that creates table code      */
/*                                                       */
/* modes values:                                         */
/*                                                       */
/*        S_SASLFILE: SASL table commands (readscore.c)  */
/*        S_OPCODE  : opcode tables       (writeop.c)    */
/*        S_INSTR   : instr tables        (writeorc.c)   */
/*        S_GLOBAL  : global tables       (writeorc.c)   */
/*        S_FUTURE  : tstamp check        (writeorc.c)   */
/*                                                       */
/*********************************************************/

void createtable(sigsym * ident, char * prefix, int mode)


{
  int lc = 0;
  sigsym * gptr;

  switch(ident->kind) {
  case K_NORMAL:
    if ((ident->defnode->vol == CONSTANT) && (ident->defnode->usesinput))
      createconstanttable(ident, prefix, mode);
    else
      createwavetable(ident, prefix, mode);
    break;
  case K_IMPORT:
    if ((gptr = getvsym(&globalsymtable, ident->val)) == NULL)
      internalerror("wtparse.c","createtable");
    if ((ident->tref->assigntot == 0) && (gptr->tref->assigntot == 0))
      {
	/* hash-define statements alias it to global wavetable */

	if (startupinstr && (currinstrument == startupinstr))
	  tablestartupcheck(ident, mode, K_IMPORTEXPORT); 
	break;
      }
    if (startupinstr && (currinstrument == startupinstr))
      tablestartupcheck(ident, mode, K_IMPORT); 
    z[lc++]= "i = NT(%1$s).len = EV(gtables)[TBL_GBL_%2$s].len;";
    z[lc++]= "NT(%1$s).lenf = EV(gtables)[TBL_GBL_%2$s].lenf;";
    z[lc++]= "NT(%1$s).start = EV(gtables)[TBL_GBL_%2$s].start;";
    z[lc++]= "NT(%1$s).end = EV(gtables)[TBL_GBL_%2$s].end;";
    z[lc++]= "NT(%1$s).base = EV(gtables)[TBL_GBL_%2$s].base;";
    z[lc++]= "NT(%1$s).stamp = EV(gtables)[TBL_GBL_%2$s].stamp;";
    z[lc++]= "NT(%1$s).sr = EV(gtables)[TBL_GBL_%2$s].sr;";
    z[lc++]= "NT(%1$s).tend = EV(gtables)[TBL_GBL_%2$s].tend;";
    z[lc++]= "NT(%1$s).oconst = EV(gtables)[TBL_GBL_%2$s].oconst;";
    z[lc++]= "NT(%1$s).dint = EV(gtables)[TBL_GBL_%2$s].dint;";
    z[lc++]= "NT(%1$s).dfrac = EV(gtables)[TBL_GBL_%2$s].dfrac;";
    z[lc++]= "NT(%1$s).sffl = EV(gtables)[TBL_GBL_%2$s].sffl;";
    z[lc++]= "NT(%1$s).sfui = EV(gtables)[TBL_GBL_%2$s].sfui;";
    z[lc++]= "NT(%1$s).dsincr = EV(gtables)[TBL_GBL_%2$s].dsincr;";
    if ((ident->tref->assigntval == 0) &&
	(gptr->tref->assigntval == 0))
      {
	z[lc++]= "NT(%1$s).t = EV(gtables)[TBL_GBL_%2$s].t;";
      }
    else
      {
	z[lc++]= "NT(%1$s).t = (float *) malloc((i+1)*sizeof(float));";
	z[lc++]= "for(;i >= 0;i--)";
	z[lc++]= "  NT(%1$s).t[i] = EV(gtables)[TBL_GBL_%2$s].t[i];";
	z[lc++]= "NT(%1$s).llmem = 1;";
      }
    printwaveblock(lc,ident,prefix);
    break;
  case K_IMPORTEXPORT:
    /* hash-define statements alias it to global wavetable */

    if (startupinstr && (currinstrument == startupinstr))
      tablestartupcheck(ident, mode, K_IMPORTEXPORT); 
    break;
  default:
    break;
  }
}

/*********************************************************/
/*     prints warnings and errors for startup tables     */
/*     called in createtable() above, and in writeop.c   */
/*********************************************************/

void tablestartupcheck(sigsym * ident, int mode, int kind)

{
  switch(kind) {
  case K_IMPORT:
    switch (mode) {
    case S_INSTR:
      printf("Error: Importing table \"%s\" in startup instrument\n"
	     "       would cause run-time error, as tables do\n"
	     "       not exist until after startup's i-cycle.\n\n",
	     ident->val);
      showerrorplace(ident->defnode->linenum,
		     ident->defnode->filename);
      break;
    case S_OPCODE:
      printf("Warning: Importing table \"%s\" in an opcode called\n" 
	     "         by the startup instrument may cause a run-time\n"
	     "         error, as tables do not exist during startup's i-cycle.\n\n",
	     ident->val);
      break;
    }
    break;
  case K_IMPORTEXPORT:
    switch (mode) {
    case S_INSTR:
      printf("Warning: %s table \"%s\" in startup instrument\n"
	     "         may cause run-time error, as tables do\n"
	     "         not exist until after startup's i-cycle.\n\n",
	     ident->kind == K_IMPORT ? "Importing" : "Import/export of",
	     ident->val);
      break;
    case S_OPCODE:
      printf("Warning: %s table \"%s\" in an opcode called\n" 
	     "         by the startup instrument may cause a run-time\n"
	     "         error, as tables do not exist during startup's i-cycle.\n\n",
	     ident->kind == K_IMPORT ? "Importing" : "Import/export of",
	     ident->val);
      break;
    }
    break;
  }
  return;
}


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*                                                              */
/* Second-level functions to generate table initialization code */
/*                                                              */
/*______________________________________________________________*/

extern void wavevarinit(sigsym *, char *);
extern void tablepreamble(sigsym *, char *, int, int);
extern int tablelistlength(tnode *);
extern void samplefilecode(sigsym *, char *, int *);
extern void randomconstcode(sigsym *, char *, tnode *, int *);

/*********************************************************/
/*        adds code that creates a table                 */
/*                                                       */
/*     TABLE IDENT LP IDENT COM exprstrlist RP           */
/*********************************************************/

void createwavetable(sigsym * ident, char * prefix, int mode)

{
 
 tnode * genptr = ident->defnode->down->next->next->next;
 tnode * aptr = ident->defnode->down->next->next->next->next->next->down;
 int i,j,k;
 int lc=0;
 char * lname = makewstring(ident->val);

 wavevarinit(ident,prefix);

 switch (genptr->val[0]) {
  case 'b':
    if (!(strcmp(genptr->val,"buzz")))
      {
	tablepreamble(ident,prefix,mode,GENBUZZ);

	z[lc++] = "if ((%2$s_rolloff == 1.0F)||(%2$s_rolloff == 0.0F)||";
	z[lc++] = "    (%2$s_rolloff == -1.0F))";
	z[lc++] = " {";
	z[lc++] = "   %2$s_scale = 1.0F;";
	z[lc++] = "   if ((%2$s_rolloff == 1.0F)||(%2$s_rolloff == -1.0F))";
	z[lc++] = "    %2$s_scale = 1.0F/(1 + %2$s_nharm);";
	z[lc++] = " }";
	z[lc++] = "else";
	z[lc++] = "  %2$s_scale = (1.0F-(float)fabs(%2$s_rolloff))/";
	z[lc++] ="        (1-(float)fabs((float)pow(%2$s_rolloff,%2$s_nharm+1)));";
	z[lc++] = "%2$s_base = 6.283185F/%2$s_size;";

	z[lc++] = "while (i >= 0)";
	z[lc++] = " {";
	z[lc++] = "  NT(%1$s).t[i] = 0.0F;";

	z[lc++] = "  %2$s_index = i*%2$s_base;";
	z[lc++] = "  %2$s_acc = 1.0F;";
	z[lc++] = "  j = %2$s_lowharm + 1;";
	z[lc++] = "  while (j <= (%2$s_lowharm + %2$s_nharm))";
	z[lc++] = "   {";
	z[lc++] = "     NT(%1$s).t[i] += %2$s_acc*(float)cos(%2$s_index*j);";
	z[lc++] = "     %2$s_acc *= %2$s_rolloff;";
	z[lc++] = "     j++;";
	z[lc++] = "   }";
	z[lc++] = "  NT(%1$s).t[i] *= %2$s_scale;";
	z[lc++] = "  i--;";
	z[lc++] = " }";

	z[lc++] = "  NT(%1$s).t[NT(%1$s).len] = NT(%1$s).t[0];";
	break;
      }
    break;
  case 'c':
    if (!(strcmp(genptr->val,"concat")))
      {
	tablepreamble(ident, prefix, mode, GENCONCAT);

	z[lc++] = "while (i >= 0)";
	z[lc++] = " NT(%1$s).t[i--] = 0.0F;";
	z[lc++] = "i = 0;";
	j = 1;
	i = tablelistlength(aptr);
	while (j <= i)
	  { 
	    z[lc++] = "j = 0;";
	    mz(lc); sprintf(z[lc++],
	    "while ((j < AP%i.len) && (i+j < %s_size))",
			    j,lname);
	    z[lc++] = " {";
	    mz(lc); sprintf(z[lc++],
			    "   NT(%s_%s).t[i+j] = AP%i.t[j];",
			    prefix,ident->val,j);
	    z[lc++] = "   j++;";
	    z[lc++] = " }";
	    mz(lc); sprintf(z[lc++],"i += AP%i.len;", j);
	    j++;
	  }

	z[lc++] = "  NT(%1$s).t[NT(%1$s).len] = NT(%1$s).t[0];";
	break;
      }
    if (!(strcmp(genptr->val,"cubicseg")))
      {
	tablepreamble(ident,prefix,mode,GENCUBIC);
	k = j = tablelistlength(aptr)/4;

	z[lc++]= "while (i >= 0)";
	z[lc++]= "{";
	z[lc++]= "  NT(%1$s).t[i] = 0.0F;";

	mz(lc); sprintf(z[lc++], "  %s_xf = i;", lname);

	while (j >= 1)
	  {
	    mz(lc); 
	    if (j == k)
	      sprintf(z[lc++],
	    "  if ((i >= %s_infl%i) && (i <= %s_infl%i) )",
		   lname,j,lname,j+1);
	    else
	      sprintf(z[lc++],
	    "  if ((i >= %s_infl%i) && (i < %s_infl%i) )",
		   lname,j,lname,j+1);
	    mz(lc); sprintf(z[lc++],
			    "    NT(%s_%s).t[i] =  %s_xf*%s_xf*%s_xf*%s_a%i + "
			    "%s_xf*%s_xf*%s_b%i + %s_xf*%s_c%i + %s_d%i;",
			    prefix,ident->val, 
			    lname, lname, lname, lname,j,
			    lname, lname, lname, j,
			    lname, lname, j,
			    lname, j);

	    j--;
	  }
	z[lc++]= "  i--;";
	z[lc++]= "}";

	z[lc++] = "  NT(%1$s).t[NT(%1$s).len] = NT(%1$s).t[0];";
	break;
      }
    break;
  case 'd':
    if (!(strcmp(genptr->val,"data")))
      {
	j = tablelistlength(aptr) - 1;
	tablepreamble(ident,prefix,mode,GENNUMDATA);

	z[lc++]= "   while (i >= 0)";
	z[lc++]= "   switch(i) {";
	while (j>=0)
	  {
	    mz(lc); sprintf(z[lc++],"    case %i:",j);
	    mz(lc); sprintf(z[lc++],"    NT(%s_%s).t[i--] = %s_p%i;",
		     prefix,ident->val,lname,j+1);
	    z[lc++]="     break;";
	    j--;
	  }
	z[lc++]= "     default :";
	z[lc++]= "     NT(%1$s).t[i--] = 0.0F;";
	z[lc++]= "}";

	z[lc++] = "  NT(%1$s).t[NT(%1$s).len] = NT(%1$s).t[0];";
      }
    break;
  case 'e':
    if (!(strcmp(genptr->val,"empty")))
      {
	if ((aptr->next != NULL))
	  {
	    printf("Error: Incorrect number of args.\n");
	    showerrorplace(ident->defnode->down->linenum,
			   ident->defnode->down->filename);
	  }
	tablepreamble(ident,prefix,mode,GENILLEGAL);

	z[lc++]= "while (i >= 0)";
	z[lc++]= "  NT(%1$s).t[i--] = 0.0F;";

	z[lc++] = "  NT(%1$s).t[NT(%1$s).len] = NT(%1$s).t[0];";

	break;
      }
    if (!(strcmp(genptr->val,"expseg")))
      {
	tablepreamble(ident,prefix,mode,GENPAIRS);
	j = tablelistlength(aptr)/2;

	z[lc++]= "while (i >= 0)";
	z[lc++]= "{";
	z[lc++]= "  NT(%1$s).t[i] = 0.0F;";
	while (j >= 2)
	  {
	    mz(lc); sprintf(z[lc++],
	    "  if ((i >= %s_x%i) && (i < %s_x%i) && (%s_x%i != %s_x%i))",
		   lname,j-1,lname,j,lname,j-1,lname,j);
	    mz(lc); sprintf(z[lc++],
           "    NT(%s_%s).t[i] =  %s_y%i*(float)pow((double)%s_d%i,",
                prefix,ident->val,lname,j-1,lname,j-1);

	    mz(lc); sprintf(z[lc++],
           "                      (double) %s_e%i*(i -  %s_x%i));",
                lname,j-1,lname,j-1);

	    j--;
	  }
	z[lc++]= "  i--;";
	z[lc++]= "}";

	z[lc++] = "  NT(%1$s).t[NT(%1$s).len] = NT(%1$s).t[0];";
	break;
      }
    break;
  case 'h':
    if (!(strcmp(genptr->val,"harm")))
      {
	tablepreamble(ident,prefix,mode,GENILLEGAL);

	z[lc++] = "%2$s_base = 6.283185F/%2$s_size;";
	j = tablelistlength(aptr);
	
	z[lc++] = "while (i >= 0)";
	z[lc++] = " {";
	z[lc++] = "  NT(%1$s).t[i] = 0.0F;";
	z[lc++] = "  %2$s_index = i*%2$s_base;";
	while (j>=1)
	  {
	    z[lc++] = "  NT(%1$s).t[i] += ";
	    mz(lc); sprintf(z[lc++],"   %s_f%i*(float)sin(%i.0F*%s_index);",
			    lname,j,j,lname);
	    j--;
	  }
	z[lc++] = "  i--;";
	z[lc++] = " }";

	z[lc++] = "  NT(%1$s).t[NT(%1$s).len] = NT(%1$s).t[0];";
	break;
      }
    if (!(strcmp(genptr->val,"harm_phase")))
      {
	j = tablelistlength(aptr);
	if ( (j % 2) == 1)
	  genex(&lc, ident->defnode->down,
		"Odd number of parameters (not including size)");
	else
	  {
	    tablepreamble(ident,prefix,mode,GENILLEGAL);

	    z[lc++] = "%2$s_base = 6.283185F/%2$s_size;";
	    z[lc++] = "while (i >= 0)";
	    z[lc++] = " {";
	    z[lc++] = "  NT(%1$s).t[i] = 0.0F;";
	    z[lc++] = "  %2$s_index = i*%2$s_base;";
	    while (j>=2)
	      {
		z[lc++] = "  NT(%1$s).t[i] += ";

		mz(lc); sprintf(z[lc++],
		"   %s_f%i*(float)sin((%i.0F*%s_index+ %s_ph%i));",
		       lname,j/2,j/2,lname,lname,j/2);

		j= j - 2;
	      }
	    z[lc++] = "  i--;";
	    z[lc++] = " }";
	  }

	z[lc++] = "  NT(%1$s).t[NT(%1$s).len] = NT(%1$s).t[0];";
	break;
      }
    break;
  case 'l':
    if (!(strcmp(genptr->val,"lineseg")))
      {

	tablepreamble(ident,prefix,mode,GENPAIRS);
	j = tablelistlength(aptr)/2;

	z[lc++]= "while (i >= 0)";
	z[lc++]= "{";
	z[lc++]= "  NT(%1$s).t[i] = 0.0F;";
	while (j >= 2)
	  {
	    mz(lc); sprintf(z[lc++],
	    "  if ((i >= %s_x%i) && (i < %s_x%i) && (%s_x%i != %s_x%i))",
		   lname,j-1,lname,j,lname,j-1,lname,j);
	    mz(lc); sprintf(z[lc++],
	    "    NT(%s_%s).t[i] =  %s_y%i + %s_d%i*(i -  %s_x%i);",
                prefix,ident->val,lname,j-1,lname,j-1,lname,j-1);

	    j--;
	  }
	z[lc++]= "  i--;";
	z[lc++]= "}";

	z[lc++] = "  NT(%1$s).t[NT(%1$s).len] = NT(%1$s).t[0];";
	break;
      }
    break;
  case 'p':
    if (!(strcmp(genptr->val,"periodic")))
      {
	j = tablelistlength(aptr);
	if ( (j % 3) != 0)
	  genex(&lc, ident->defnode->down, 
		"Number of parameters not divisible by 3");
	else
	  {
	    tablepreamble(ident,prefix,mode,GENILLEGAL);

	    z[lc++] = "%2$s_base = 6.283185F/%2$s_size;";
	    z[lc++] = "while (i >= 0)";
	    z[lc++] = " {";
	    z[lc++] = "  NT(%1$s).t[i] = 0.0F;";
	    z[lc++] = "  %2$s_index = i*%2$s_base;";

	    while (j>=3)
	      {
		z[lc++] = "  NT(%1$s).t[i] += ";

		mz(lc); sprintf(z[lc++],
		"   %s_f%i*(float)sin(%s_p%i*%s_index+ %s_ph%i);",
		lname,j/3,lname,j/3, lname,lname,j/3);

		j= j - 3;
	      }
	    z[lc++] = "  i--;";
	    z[lc++] = " }";
	  }

	z[lc++] = "  NT(%1$s).t[NT(%1$s).len] = NT(%1$s).t[0];";
	break;
      }
    if (!(strcmp(genptr->val,"polynomial")))
      {
	tablepreamble(ident,prefix,mode,GENILLEGAL);

	i = tablelistlength(aptr) -3;
	j = 0;
	z[lc++] = "%2$s_scale = (1.0F/%2$s_size)*(%2$s_xmax-%2$s_xmin);";
	z[lc++] = "while (i >= 0)";
	z[lc++] = " {";
	z[lc++] = "  NT(%1$s).t[i] = 0.0F;";
	z[lc++] = "  %2$s_index = %2$s_xmin + %2$s_scale*(%2$s_size - i);";
	z[lc++] = "  %2$s_acc = 1.0F;";
	z[lc++] = "  NT(%1$s).t[i] = %2$s_a0;";
	while (j < i)
	  {
	    z[lc++] = "  %2$s_acc *= %2$s_index;";
	    mz(lc); sprintf(z[lc++],"   NT(%s_%s).t[i] += %s_a%i*%s_acc;",
			prefix,ident->val,lname,j+1,lname);
	    j++;
	  }
	z[lc++] = "  i--;";
	z[lc++] = " }";

	z[lc++] = "  NT(%1$s).t[NT(%1$s).len] = NT(%1$s).t[0];";
	break;
      }
    break;
  case 'r':
    if (!(strcmp(genptr->val,"random")))
      {
	j = tablelistlength(aptr);
	tablepreamble(ident,prefix,mode,GENILLEGAL);

	if (aptr->next->next->vol == CONSTANT)
	  {
	    randomconstcode(ident, prefix, aptr, &lc);
	    break;
	  }

	/* general-purpose code: only used if dist is a variable */

	z[lc++] = "switch(%2$s_dist) {";
	z[lc++] = " case 1:";
	if (j==3)
	  z[lc++] = "  %2$s_c1 = %2$s_p2 - %2$s_p1;";
	else
	  genex(&lc, ident->defnode->down, "Wrong number of parameters (dist 1)");

	z[lc++] = "  break;";
	z[lc++] = " case 2:";
	if (j==3)
	  {
	    z[lc++] = "  %2$s_c1 = %2$s_p2 - %2$s_p1;";
	    z[lc++] = "if (%2$s_p2 == %2$s_p1)";
	    genex(&lc, ident->defnode->down, "p1 == p2 (dist 2)");
	  }
	else
	  genex(&lc, ident->defnode->down, "Wrong number of parameters (dist 2)");

	z[lc++] = "  break;";
	z[lc++] = " case 3:";
	z[lc++] = "  break;";
	z[lc++] = " case 4:";
	if (j==3)
	  {
	    z[lc++] = "  if (%2$s_p2 <= 0.0F)";
	    genex(&lc, ident->defnode->down, "p2 <= 0 (dist 4)");
	    z[lc++] = "  %2$s_c1 = (float)sqrt(2*%2$s_p2);";
	  }
	else
	  genex(&lc, ident->defnode->down, "Wrong number of parameters (dist 4)");

	z[lc++] = "  break;";
	z[lc++] = " case 5:";
	z[lc++] = "  break;";
	z[lc++] = " default:";
	genex(&lc, ident->defnode->down, "Illegal distribution (not 1,2,3,4,5)");
	z[lc++] = "}";

	z[lc++] = "j = 0; i = 0;";
	z[lc++] = "while (i < %2$s_size)";
	z[lc++] = " {";
	z[lc++] = "   switch(%2$s_dist) {";
	z[lc++] = "    case 1:";
	z[lc++] = "     NT(%1$s).t[i] = %2$s_c1*RMULT*((float)rand()) + %2$s_p1;";
	z[lc++] = "     break;";
	z[lc++] = "    case 2:";
	z[lc++] = "     %2$s_x = RMULT*((float)rand());";
	z[lc++] = "     %2$s_y = RMULT*((float)rand());";
	z[lc++] = "     if (%2$s_x > %2$s_y)";
	z[lc++] = "       NT(%1$s).t[i] =  %2$s_c1*%2$s_x + %2$s_p1;";
	z[lc++] = "     else";
	z[lc++] = "       NT(%1$s).t[i] =  %2$s_c1*%2$s_y + %2$s_p1;";
	z[lc++] = "     break;";
	z[lc++] = "    case 3:";
	z[lc++] = "     NT(%1$s).t[i] = -%2$s_p1*(float)log(RMULT*((float)rand())+1e-45F);";
	z[lc++] = "     break;";
	z[lc++] = "    case 4:";
	z[lc++] = "     NT(%1$s).t[i] = %2$s_p1+%2$s_c1*";
        z[lc++] = "             (float)sqrt(-2.0F*(float)log(RMULT*((float)rand())+1e-45F))";
        z[lc++] = "                   *(float)cos(6.283185F*RMULT*((float)rand()));";
	z[lc++] = "     break;";
	z[lc++] = "    case 5:";
	z[lc++] = "     NT(%1$s).t[i] = 0;";
	z[lc++] = "     if (j == 0)";
	z[lc++] = "      {";
	z[lc++] = "        j = ROUND(-%2$s_p1*(float)log(RMULT*((float)rand())+1e-45F))+1;";
	z[lc++] = "        if (i != 0)";
	z[lc++] = "         NT(%1$s).t[i] = 1.0F;";
	z[lc++] = "      }";
	z[lc++] = "      j--;";
	z[lc++] = "     break;";
	z[lc++] = "   }";
	z[lc++] = "  i++;";
	z[lc++] = " }";
	z[lc++] = "  NT(%1$s).t[NT(%1$s).len] = NT(%1$s).t[0];";
      }
    break;
  case 's':
    if (!(strcmp(genptr->val,"sample")))
      {
	samplefilecode(ident, prefix, &lc); 
	break;
      }
    if (!(strcmp(genptr->val,"spline")))
      {
	tablepreamble(ident,prefix,mode,GENSPLINE);
	k = j = ((tablelistlength(aptr)+1)/3)-1;

	z[lc++]= "while (i >= 0)";
	z[lc++]= "{";
	z[lc++]= "  NT(%1$s).t[i] = 0.0F;";

	mz(lc); sprintf(z[lc++], "  %s_xf = i;", lname);
	    
	while (j >= 1)
	  {
	    mz(lc);
	    if (j == k)
	      sprintf(z[lc++],
		 "  if ((i >= %s_x%i) && (i <= %s_x%i) && (%s_x%i != %s_x%i))",
		      lname,j,lname,j+1,lname,j,lname,j+1);
	    else
	      sprintf(z[lc++],
		 "  if ((i >= %s_x%i) && (i < %s_x%i) && (%s_x%i != %s_x%i))",
		      lname,j,lname,j+1,lname,j,lname,j+1);
	    mz(lc); sprintf(z[lc++],
			    "    NT(%s_%s).t[i] =  %s_xf*%s_xf*%s_xf*%s_a%i + " 
			    "%s_xf*%s_xf*%s_b%i + %s_xf*%s_c%i + %s_d%i;",
			    
			    prefix,ident->val, lname, lname, lname, lname, j,
			    lname, lname, lname,j,
			    lname, lname,j,
			    lname,j);

	    j--;
	  }
	z[lc++]= "  i--;";
	z[lc++]= "}";
	z[lc++]= "  NT(%1$s).t[NT(%1$s).len] = NT(%1$s).t[0];";
	break;
      }
    if (!(strcmp(genptr->val,"step")))
      {

	tablepreamble(ident,prefix,mode,GENSTEP);
	j = (tablelistlength(aptr)/2)+1;

	z[lc++]= "while (i >= 0)";
	z[lc++]= "{";
	z[lc++]= "  NT(%1$s).t[i] = 0.0F;";
	while (j >= 2)
	  {
	    mz(lc); sprintf(z[lc++],
	    "  if ((i >= %s_x%i) && (i < %s_x%i))",
			    lname,j-1,lname,j);
	    mz(lc); sprintf(z[lc++],
	    "    NT(%s_%s).t[i] =  %s_y%i;", prefix,ident->val,lname,j-1);

	    j--;
	  }
	z[lc++]= "  i--;";
	z[lc++]= "}";

	z[lc++]= "  NT(%1$s).t[NT(%1$s).len] = NT(%1$s).t[0];";
	break;
      }
    break;
  case 'w':
    if (!(strcmp(genptr->val,"window")))
      {
	j = tablelistlength(aptr);
	tablepreamble(ident,prefix,mode,GENILLEGAL);


	z[lc++] = "switch(%2$s_type) {";
	z[lc++] = " case 1:";
	z[lc++] = " case 2:";
	z[lc++] = "  %2$s_c1 = 6.283185F/(%2$s_size-1.0F);";
	z[lc++] = "  break;";
	z[lc++] = " case 3:";
	z[lc++] = "  %2$s_c1 = 2.0F/(%2$s_size-1.0F);";
	z[lc++] = "  %2$s_c2 = 1.0F/%2$s_c1;";
	z[lc++] = "  break;";
	z[lc++] = " case 4:";
	z[lc++] = "  %2$s_c1 = - 18.0F/(%2$s_size*%2$s_size);";
	z[lc++] = "  %2$s_c2 = - 0.5F*%2$s_size;";  
	z[lc++] = "  break;";
	z[lc++] = " case 5:";
	if (j==2)
	  {
	    z[lc++] = "  %2$s_d1 = (%2$s_size-1.0)/2.0;"; 
	    z[lc++] = "  %2$s_d2 = %2$s_d1*%2$s_d1;";
	    z[lc++] = "  %2$s_d3 = 1.0/modbessel(%2$s_p*%2$s_d1);";
	  }
	else
	  genex(&lc, ident->defnode->down, "Parameter p needed (type 5)");
	z[lc++] = "  break;";
	z[lc++] = " case 6:";
	z[lc++] = "  break;";
	z[lc++] = " default:";
	genex(&lc, ident->defnode->down, "Illegal window type (not 1,2,3,4,5,6)");

	z[lc++] = "}";

	
	z[lc++] = "while (i >= 0)";
	z[lc++] = " {";
	z[lc++] = "   switch(%2$s_type) {";
	z[lc++] = "    case 1:";
	z[lc++] = "     NT(%1$s).t[i] = 0.54F - 0.46F*(float)cos(%2$s_c1*i);";
	z[lc++] = "     break;";
	z[lc++] = "    case 2:";
	z[lc++] = "     NT(%1$s).t[i] = 0.5F*(1.0F - (float)cos(%2$s_c1*i));";
	z[lc++] = "     break;";
	z[lc++] = "    case 3:";
	z[lc++] = "     NT(%1$s).t[i] = 1.0F - %2$s_c1*(float)fabs(i - %2$s_c2);";
	z[lc++] = "     break;";
	z[lc++] = "    case 4:";
	z[lc++] = "     NT(%1$s).t[i] = ";
	z[lc++] = "         (float)exp(%2$s_c1*(%2$s_c2+i)*(%2$s_c2+i));";
	z[lc++] = "     break;";
	z[lc++] = "    case 5:";
	if (j==2)
	  {
	    z[lc++] = "     NT(%1$s).t[i] = (float)(%2$s_d3*modbessel(%2$s_p*";
	    z[lc++] = "      sqrt(%2$s_d2 - (i-%2$s_d1)*(i-%2$s_d1))));";
	  }
	z[lc++] = "     break;";
	z[lc++] = "    case 6:";
	z[lc++] = "     NT(%1$s).t[i] = 1.0F;";
	z[lc++] = "     break;";
	z[lc++] = "   }";
	z[lc++] = "  i--;";
	z[lc++] = " }";

	z[lc++]= "  NT(%1$s).t[NT(%1$s).len] = NT(%1$s).t[0];";
      }
    break;
  default:
    break;
  }
 printwavesymblock(lc,ident,prefix);
 free(lname);
}


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*                                                              */
/* Lower-level functions wavevarinit() and tablepreamble()      */
/* do large code-generation subtasks for createwavetable().     */
/*                                                              */
/*______________________________________________________________*/

extern void wavevarinitcheck(tnode *);

/*********************************************************/
/*  initializes variables for generator execution        */
/*********************************************************/

void wavevarinit(sigsym * sptr, char * prefix)


{
  tnode * wname = sptr->defnode->down->next;
  tnode * ident = sptr->defnode->down->next->next->next;
  tnode * tptr =  sptr->defnode->down->next->next->next->next->next->down;
  int i, j, depth, currblocksafe;
  int lc = 0;
  int tabletype;   /* for concat */
  char tablename[STRSIZE];
  char * idxstr;
  char * lname = makewstring(wname->val);

  currblocksafe = currblockrate;
  currblockrate = IRATETYPE;

  if (tptr == NULL)
    {
      printf("Error: Generator needs size parameter.\n");
      showerrorplace(wname->linenum, wname->filename);
    }
  wavevarinitcheck(tptr);

  fprintf(outfile,"   %s_rounding = ", lname);
  blocktree(tptr->down, PRINTTOKENS);
  fprintf(outfile,";\n");

  fprintf(outfile,"   %s_size = ROUND(%s_rounding);\n",
	  lname, lname);

  tptr = tptr->next;
  if (tptr != NULL)
    tptr = tptr->next;

  switch (ident->val[0]) {
  case 'b':
    if (!(strcmp(ident->val,"buzz")))
      {
	i = 1;
	while (tptr != NULL)
	  {
	    if (tptr->ttype == S_EXPR)
	      {
		wavevarinitcheck(tptr);
		switch (i) {
		case 1:
		  fprintf(outfile,"   %s_rounding = ", lname);
		  blocktree(tptr->down, PRINTTOKENS);
		  fprintf(outfile,";\n");
		  fprintf(outfile,"   %s_nharm = ROUND(%s_rounding);\n",
			  lname, lname);
		  break;
		case 2:
		  fprintf(outfile,"   %s_rounding = ", lname);
		  blocktree(tptr->down, PRINTTOKENS);
		  fprintf(outfile,";\n");
		  fprintf(outfile,"   %s_lowharm = ROUND(%s_rounding);\n",
			  lname, lname);
		  break;
		case 3:
		  fprintf(outfile,"   %s_rolloff = ",lname);
		  blocktree(tptr->down, PRINTTOKENS);
		  fprintf(outfile,";\n");
		  break;
		}
		i++;
	      }
	    tptr = tptr->next;
	  }
	break;
      }
    break;
  case 'c':
    if (!(strcmp(ident->val,"concat")))
      {
	i = 1;
	while (tptr != NULL)
	  {
	    if (tptr->ttype == S_EXPR)
	      {
		if (!(tptr->vartype == TABLETYPE))
		  {
		    printf("Error: Concat ft args must be tables.\n");
		    showerrorplace(wname->linenum, wname->filename);
		  }
		currarrayindex = 0;
		currscalarflag = 1;
		sprintf(tablename,"%s_ft%i",lname,i);
		if (tptr->down->next == NULL)  
		  {
		    /* a table */

		    fprintf(outfile,"   %s = ", tablename);
		    tabletype = maketableindex(tptr->down, curropcodestack,
					       &idxstr, &depth);
		    fprintf(outfile,"%s;\n",idxstr);
		    printtabledefine(i++, tablename, tabletype, depth);
		  }
		else  
		  {

		    /* a tablemap */

		    printtmapcase(tptr->down, 
				  tptr->down->next->next, tablename);
		    printtabledefine(i++, tablename, S_TABLEMAP, -1);
		  }
	      }
	    tptr = tptr->next;
	  }
	break;
      }
    if (!(strcmp(ident->val,"cubicseg")))
      {
	i = 1; j = 0;
	while (tptr != NULL)
	  {
	    if (tptr->ttype == S_EXPR)
	      {
		wavevarinitcheck(tptr);
		switch (j) {
		case 0:
		  fprintf(outfile,"   %s_rounding = ", lname);
		  blocktree(tptr->down, PRINTTOKENS);
		  fprintf(outfile,";\n");
		  fprintf(outfile,"   %s_infl%i = ROUND(%s_rounding);\n",
			  lname,i,lname);
		  break;
		case 1:
		  fprintf(outfile,"   %s_y%i = ",lname, 2*i-1);
		  blocktree(tptr->down, PRINTTOKENS);
		  fprintf(outfile,";\n");
		  break;
		case 2:
		  fprintf(outfile,"   %s_rounding = ", lname);
		  blocktree(tptr->down, PRINTTOKENS);
		  fprintf(outfile,";\n");
		  fprintf(outfile,"   %s_x%i = ROUND(%s_rounding);\n",
			  lname,i,lname);
		  break;
		case 3:
		  fprintf(outfile,"   %s_y%i = ",lname, 2*i);
		  blocktree(tptr->down, PRINTTOKENS);
		  fprintf(outfile,";\n");
		  break;
		}
		j++;
		if (j == 4)
		  {
		    j = 0; i++;
		  }
	      }
	    tptr = tptr->next;
	  }
	z[lc++] = "if (%2$s_infl1 != 0.0F)";
	genex(&lc, wname, "First inflection point != 0");
	i--;
	if ((j != 2)|| (i<2))
	  genex(&lc, wname, "Not enough x values");
	else
	  {
	    j = 1;
	    while (j <= i)
	      {
		z[lc++] = "";
		mz(lc); sprintf(z[lc++],
		"if (%s_x%i <= %s_infl%i)",
				lname,j,lname,j);
		genex(&lc, wname, "Infl not strictly between surrounding xvals");
		mz(lc); sprintf(z[lc++],
		"if (%s_infl%i <= %s_x%i)",
				lname,j+1,lname,j);
		genex(&lc, wname, "Infl not strictly between surrounding xvals");

		mz(lc); sprintf(z[lc++], 
                "%s_Q = %s_infl%i*%s_infl%i*%s_infl%i;",
				lname,lname,j,lname,j,lname,j);
		mz(lc); sprintf(z[lc++], 
		"%s_Q += - %s_x%i*%s_x%i*%s_x%i;",
				lname,lname,j,lname,j,lname,j);
		mz(lc); sprintf(z[lc++],
		"%s_Q += - 3.0F*%s_x%i*%s_x%i*(%s_infl%i-%s_x%i);",
				lname,lname,j,lname,
				j,lname,j,lname,j);
		mz(lc); sprintf(z[lc++],
		"if (%s_Q == 0.0F)",
				lname);
		genex(&lc, wname, "No cubic solution for these parameters");

		mz(lc); sprintf(z[lc++],
	        "%s_R = %s_infl%i*%s_infl%i;",
				lname,lname,j,lname,j);
		mz(lc); sprintf(z[lc++],
	        "%s_R += - %s_x%i*%s_x%i;",
				lname,lname,j,lname,j);
		mz(lc); sprintf(z[lc++],
                "%s_R += - 2.0F*%s_x%i*(%s_infl%i-%s_x%i);",
				lname,lname,j,lname,j
				,lname,j);
		mz(lc); sprintf(z[lc++],
                "if (%s_R == 0.0F)",
				lname);
		genex(&lc, wname, "No cubic solution for these parameters");

		mz(lc); sprintf(z[lc++],
	        "%s_S = %s_x%i*%s_x%i*%s_x%i;",
				lname,lname,j,lname,j
				,lname,j);
		mz(lc); sprintf(z[lc++],
	        "%s_S += - %s_infl%i*%s_infl%i*%s_infl%i;",
				lname,lname,j+1,lname,j+1
				,lname,j+1);
		mz(lc); sprintf(z[lc++],
                "%s_S += - 3.0F*%s_x%i*%s_x%i*(%s_x%i-%s_infl%i);"
				,lname,lname,j,lname,j
				,lname,j,lname,j+1);
		mz(lc); sprintf(z[lc++],
                "if (%s_S == 0.0F)",
				lname);
		genex(&lc, wname, "No cubic solution for these parameters");

		mz(lc); sprintf(z[lc++],
	        "%s_T = %s_x%i*%s_x%i;",
				lname,lname,j,lname,j);
		mz(lc); sprintf(z[lc++],
	        "%s_T += - %s_infl%i*%s_infl%i;",
				lname,lname,j+1,lname,j+1);
		mz(lc); sprintf(z[lc++],
                "%s_T += - 2.0F*%s_x%i*(%s_x%i-%s_infl%i);"
				,lname,lname,j,lname,j
				,lname,j+1);
		mz(lc); sprintf(z[lc++],
                "if (%s_T == 0.0F)",
				lname);
		genex(&lc, wname, "No cubic solution for these parameters");

		mz(lc); sprintf(z[lc++],
	        "if ((%s_Q/%s_R) == (%s_S/%s_T))",
				lname,lname,lname,lname);
		genex(&lc, wname, "No cubic solution for these parameters");

		mz(lc); sprintf(z[lc++],
                "%s_a%i = 1.0F/((%s_Q/%s_R) - (%s_S/%s_T));",
				lname,j,lname,lname,
				lname,lname);
		mz(lc); sprintf(z[lc++],
  "%s_a%i *= (%s_y%i-%s_y%i)/%s_R - (%s_y%i-%s_y%i)/%s_T;",
				lname,j,
				lname,2*j-1,
				lname,2*j,
				lname,
				lname,2*j,
				lname,2*j+1,
				lname);

		mz(lc); sprintf(z[lc++],
                "%s_b%i = 1.0F/((%s_R/%s_Q) - (%s_T/%s_S));",
				lname,j,lname,lname,
				lname,lname);
		mz(lc); sprintf(z[lc++],
  "%s_b%i *= (%s_y%i-%s_y%i)/%s_Q - (%s_y%i-%s_y%i)/%s_S;",
				lname,j,
				lname,2*j-1,
				lname,2*j,
				lname,
				lname,2*j,
				lname,2*j+1,
				lname);

		mz(lc); sprintf(z[lc++],
  "%s_c%i = - 3.0F*%s_a%i*%s_x%i*%s_x%i - 2.0F*%s_b%i*%s_x%i;",
				lname,j, lname,j,
				lname,j, lname,j,
				lname,j, lname,j);
		mz(lc); sprintf(z[lc++],
  "%s_d%i = %s_y%i - %s_a%i*%s_x%i*%s_x%i*%s_x%i;",
				lname,j, lname,2*j,
				lname,j, lname,j,
				lname,j, lname,j);

		mz(lc); sprintf(z[lc++],
"%s_d%i += - %s_b%i*%s_x%i*%s_x%i - %s_c%i*%s_x%i;"
				,lname,j,lname,j
				,lname,j,lname,j
				,lname,j,lname,j);
		
		j++;
	      }
	   z[lc++]="";
	    
	  }
	printwavesymblock2(lc,wname);
	break;
      }
    break;
  case 'd':
    if (!(strcmp(ident->val,"data")))
      {
	i = 1;
	while (tptr != NULL)
	  {
	    if (tptr->ttype == S_EXPR)
	      {
		wavevarinitcheck(tptr);
		fprintf(outfile,"   %s_p%i = ",lname,i);
		blocktree(tptr->down, PRINTTOKENS);
		fprintf(outfile,";\n");
		i++;
	      }
	    tptr = tptr->next;
	  }
	break;
      }
    break;
  case 'e':
    if (!(strcmp(ident->val,"empty")))
      {
	break;
      }
    if (!(strcmp(ident->val,"expseg")))
      {
	i = 1;
	while (tptr != NULL)
	  {
	    if (tptr->ttype == S_EXPR)
	      {
		wavevarinitcheck(tptr);
		if ((i % 2) == 1)
		  {
		    fprintf(outfile,"   %s_rounding = ", lname);
		    blocktree(tptr->down, PRINTTOKENS);
		    fprintf(outfile,";\n");
		    fprintf(outfile,"   %s_x%i = ROUND(%s_rounding);\n",
			    lname, (i/2)+1, lname);
		  }
		else
		  {
		    fprintf(outfile,"   %s_y%i = ",lname,(i/2));
		    blocktree(tptr->down, PRINTTOKENS);
		    fprintf(outfile,";\n");
		  }
		i++;
	      }
	    tptr = tptr->next;
	  }
	i--;
	if (i % 2 == 1)
	  genex(&lc, wname, "Odd number of parameters (not including size)");
	else
	  {
	    i = i/2;
	    while (i >= 2)
	      {
		mz(lc); sprintf(z[lc++],"if (%s_x%i > %s_x%i)",
				lname,i-1,lname,i);
		genex(&lc, wname, "xvals not a non-decreasing sequence");
		mz(lc); sprintf(z[lc++],"if (%s_y%i == 0.0F)",
				lname,i);
		genex(&lc, wname, "yval == 0.0F");
		mz(lc); sprintf(z[lc++],"if ((%s_y%i>0.0F) != (%s_y%i>0.0F))",
				lname,i-1,lname,i);
		genex(&lc, wname, "yvals not all the same sign");
		mz(lc); sprintf(z[lc++],"%s_d%i = %s_y%i/%s_y%i;",
				lname,i-1,lname,i,lname,i-1);
		mz(lc); sprintf(z[lc++],"if (%s_x%i != %s_x%i)",
				lname,i-1,lname,i);
		mz(lc); sprintf(z[lc++],"%s_e%i= 1.0F/(%s_x%i-%s_x%i);",
				lname,i-1,
				lname,i,
				lname,i-1);
		i--;
	      }
	    z[lc++]="if (%2$s_x1 != 0.0F)";
	    genex(&lc, wname, "x1 is not equal to zero");
	    z[lc++]="if (%2$s_y1 == 0.0F)";
	    genex(&lc, wname, "y1 is equal to zero");
	  }
	printwavesymblock2(lc,wname);
	break;
      }
    break;
  case 'h':
    if (!(strcmp(ident->val,"harm")))
      {
	i = 1;
	while (tptr != NULL)
	  {
	    if (tptr->ttype == S_EXPR)
	      {
		wavevarinitcheck(tptr);
		fprintf(outfile,"   %s_f%i = ",lname,i);
		i++;
		blocktree(tptr->down, PRINTTOKENS);
		fprintf(outfile,";\n");
	      }
	    tptr = tptr->next;
	  }
	break;
      }
    if (!(strcmp(ident->val,"harm_phase")))
      {
	i = 1;
	while (tptr != NULL)
	  {
	    if (tptr->ttype == S_EXPR)
	      {
		wavevarinitcheck(tptr);
		if ((i % 2) == 1)
		  fprintf(outfile,"   %s_f%i = ",lname,(i/2)+1);
		else
		  fprintf(outfile,"   %s_ph%i = ",lname,(i/2));
		blocktree(tptr->down, PRINTTOKENS);
		fprintf(outfile,";\n");
		i++;
	      }
	    tptr = tptr->next;
	  }
	break;
      }
    break;
  case 'l':
    if (!(strcmp(ident->val,"lineseg")))
      {
	i = 1;
	while (tptr != NULL)
	  {
	    if (tptr->ttype == S_EXPR)
	      {
		wavevarinitcheck(tptr);
		if ((i % 2) == 1)
		  {
		    fprintf(outfile,"   %s_rounding = ", lname);
		    blocktree(tptr->down, PRINTTOKENS);
		    fprintf(outfile,";\n");
		    fprintf(outfile,"   %s_x%i = ROUND(%s_rounding);\n",
			    lname, (i/2)+1, lname);
		  }
		else
		  {
		    fprintf(outfile,"   %s_y%i = (",lname,(i/2));
		    blocktree(tptr->down, PRINTTOKENS);
		    fprintf(outfile,");\n");
		  }
		i++;
	      }
	    tptr = tptr->next;
	  }
	i--;
	if (i % 2 == 1)
	  genex(&lc, wname, "Odd number of parameters (not including size)");
	else
	  {
	    i = i/2;
	    while (i >= 2)
	      {
		mz(lc); sprintf(z[lc++],
                "if (%s_x%i > %s_x%i)",
				lname,i-1,lname,i);
		genex(&lc, wname, "xvals not a non-decreasing sequence");
		mz(lc); sprintf(z[lc++],
                "if (%s_x%i != %s_x%i)",
				lname,i-1,lname,i);
		mz(lc); sprintf(z[lc++],
	        "%s_d%i=(%s_y%i-%s_y%i)/(%s_x%i-%s_x%i);",
			lname,i-1, lname,i,
			lname,i-1, lname,i,lname,i-1);
		i--;
	      }
	    z[lc++]="if (%2$s_x1 != 0.0F)";
	    genex(&lc, wname, "x1 != 0");
	  }
	printwavesymblock2(lc,wname);
	break;
      }
    break;
  case 'p':
    if (!(strcmp(ident->val,"periodic")))
      {
	i = 1;
	while (tptr != NULL)
	  {
	    if (tptr->ttype == S_EXPR)
	      {
		wavevarinitcheck(tptr);
		switch(i % 3) {
		case 1:
		  fprintf(outfile,"   %s_p%i = ",lname,(i/3)+1);
		  break;
		case 2:
		  fprintf(outfile,"   %s_f%i = ",lname,(i/3)+1);
		  break;
		case 0:
		  fprintf(outfile,"   %s_ph%i = ",lname,(i/3));
		  break;
		}
		blocktree(tptr->down, PRINTTOKENS);
		fprintf(outfile,";\n");
		i++;
	      }
	    tptr = tptr->next;
	  }
	break;
      }
    if (!(strcmp(ident->val,"polynomial")))
      {
	i = 1;
	while (tptr != NULL)
	  {
	    if (tptr->ttype == S_EXPR)
	      {
		switch (i) {
		case 1:
		  fprintf(outfile,"   %s_xmin = ",lname);
		  break;
		case 2:
		  fprintf(outfile,"   %s_xmax = ",lname);
		  break;
		default:
		  fprintf(outfile,"   %s_a%i = ",lname,i-3);
		  break;
		}
		blocktree(tptr->down, PRINTTOKENS);
		fprintf(outfile,";\n");
		i++;
	      }
	    tptr = tptr->next;
	  }
	if (i<4)
	  genex(&lc, wname, "Not enough parameters");
	z[lc++]="if (%2$s_xmax == %2$s_xmin)";
	genex(&lc, wname, "xmin = xmax");
	printwavesymblock2(lc,wname);
	break;
      }
    break;
  case 'r':
    if (!(strcmp(ident->val,"random")))
      {
	i = 1;
	while (tptr != NULL)
	  {
	    if (tptr->ttype == S_EXPR)
	      {
		wavevarinitcheck(tptr);
		switch (i) {
		case 1:
		  fprintf(outfile,"   %s_rounding = ", lname);
		  blocktree(tptr->down, PRINTTOKENS);
		  fprintf(outfile,";\n");
		  fprintf(outfile,"   %s_dist = ROUND(%s_rounding);\n",
			  lname, lname);
		  break;
		case 2:
		  fprintf(outfile,"   %s_p1 = ",lname);
		  blocktree(tptr->down, PRINTTOKENS);
		  fprintf(outfile,";\n");
		  break;
		case 3:
		  fprintf(outfile,"   %s_p2 = ",lname);
		  blocktree(tptr->down, PRINTTOKENS);
		  fprintf(outfile,";\n");
		  break;
		default:
		  printf("Error: Too many parameters.\n");
		  showerrorplace(wname->linenum, wname->filename);
		}
		i++;
	      }
	    tptr = tptr->next;
	  }
	break;
      }
    break;
  case 's':
    if (!(strcmp(ident->val,"sample")))
      {
	/* check skip if present, codegen in samplefilecode */

	tptr = (tptr->next) ? tptr->next->next : NULL;

	if (tptr)
	  wavevarinitcheck(tptr);

	break;
      }
    if (!(strcmp(ident->val,"spline")))
      {
	i = 1; j = 1;
	while (tptr != NULL)
	  {
	    if (tptr->ttype == S_EXPR)
	      {
		wavevarinitcheck(tptr);
		switch(i % 3) {
		case 0:
		  fprintf(outfile,"   %s_k%i = ",lname,(i/3)+1);
		  blocktree(tptr->down, PRINTTOKENS);
		  fprintf(outfile,";\n");
		  break;
		case 1:
		  fprintf(outfile,"   %s_rounding = ", lname);
		  blocktree(tptr->down, PRINTTOKENS);
		  fprintf(outfile,";\n");
		  fprintf(outfile,"   %s_x%i = ROUND(%s_rounding);\n",
			  lname,(i/3)+1,lname);
		  break;
		case 2:
		  fprintf(outfile,"   %s_y%i = ",lname,(i/3)+1);
		  blocktree(tptr->down, PRINTTOKENS);
		  fprintf(outfile,";\n");
		  j++;
		  break;
		}
		i++;
	      }
	    tptr = tptr->next;
	  }
	j--;i--;

	z[lc++]="%2$s_k1 = 0.0F;";
	mz(lc); sprintf(z[lc++],"%s_k%i = 0.0F;", lname,j);

	if (j<2)
	  genex(&lc, wname, "Less than 2 parameters");
	if ((i % 3) != 2)
	  genex(&lc, wname, "Sequence must end with yn.");

	else
	  {
	    i = 1;
	    while (i < j)
	      {
		mz(lc); sprintf(z[lc++],"if (%s_x%i > %s_x%i)",
			lname,i,lname,i+1);
		genex(&lc, wname, "xvals not a non-decreasing sequence");

		mz(lc); sprintf(z[lc++],"if (%s_x%i != %s_x%i)",
			lname,i,lname,i+1);
		z[lc++]="{";

		mz(lc); sprintf(z[lc++],"%s_Q = 1.0F/(%s_x%i - %s_x%i);",
			lname,lname,i,lname,i+1);

		mz(lc); sprintf(z[lc++],
	       "%s_R = %s_Q*(%s_x%i*%s_x%i - %s_x%i*%s_x%i) - 2.0F*%s_x%i;",
				lname,lname,lname,i,lname,i,
				lname,i+1,lname,i+1,lname,i);

		z[lc++]="if (%2$s_R == 0.0F)";
		genex(&lc, wname, "No spline solution for these parameters");

		z[lc++]="%2$s_R =1.0F/%2$s_R;";

		mz(lc); sprintf(z[lc++],
	       "%s_S = %s_Q*(%s_x%i*%s_x%i - %s_x%i*%s_x%i) - 2.0F*%s_x%i;",
				lname,lname,lname,i,lname,i,
				lname,i+1,lname,i+1,lname,i+1);

		z[lc++]="if (%2$s_S == 0.0F)";
		genex(&lc, wname, "No spline solution for these parameters");

		z[lc++]="%2$s_S =1.0F/%2$s_S;";

		mz(lc); sprintf(z[lc++],
		"%s_a%i = %s_Q*(%s_x%i*%s_x%i*%s_x%i-%s_x%i*%s_x%i*%s_x%i)*(%s_R-%s_S);",
				lname,i,lname,
				lname,i,lname,i,lname,i,
				lname,i+1,lname,i+1,lname,i+1,
				lname,lname);

		mz(lc); sprintf(z[lc++],
		"%s_a%i += -3.0F*(%s_R*%s_x%i*%s_x%i - %s_S*%s_x%i*%s_x%i);",
				lname,i,
				lname, lname,i,lname,i,
				lname, lname,i+1,lname,i+1);

		mz(lc); sprintf(z[lc++],"if (%s_a%i==0.0F)",lname,i);
		genex(&lc, wname, "No spline solution for these parameters");

		mz(lc); sprintf(z[lc++], 
		"%s_a%i =1.0F/%s_a%i;",
				lname,i,lname,i);

		mz(lc); sprintf(z[lc++],
		"%s_a%i *= %s_Q*(%s_y%i-%s_y%i)*(%s_R-%s_S) - %s_R*%s_k%i + %s_S*%s_k%i;",
				lname,i,lname,lname,i,lname,i+1,
				lname,lname,lname,lname,i,
				lname,lname,i+1);

		mz(lc); sprintf(z[lc++],
		"%s_b%i = %s_Q*(%s_k%i-%s_k%i)*0.5F;",
				lname,i,
				lname, lname,i,lname,i+1);
		mz(lc); sprintf(z[lc++],
		"%s_b%i += - 1.5F*%s_a%i*%s_Q*(%s_x%i*%s_x%i - %s_x%i*%s_x%i);",
				lname,i,lname,i,lname,
				lname,i,lname,i,lname,i+1,lname,i+1);

		mz(lc); sprintf(z[lc++],
		"%s_c%i = %s_k%i - 3.0F*%s_a%i*%s_x%i*%s_x%i -2.0F*%s_b%i*%s_x%i;",
				lname,i,lname,i,lname,i,lname,i,
				lname,i,lname,i,lname,i);

		mz(lc); sprintf(z[lc++],
	    "%s_d%i=%s_y%i-%s_a%i*%s_x%i*%s_x%i*%s_x%i-%s_b%i*%s_x%i*%s_x%i-%s_c%i*%s_x%i;"
				,lname,i,lname,i,lname,i,lname,i
				,lname,i,lname,i,lname,i,lname,i
				,lname,i,lname,i,lname,i);

		z[lc++]="}";
		i++;
	      }
 
	  }
	printwavesymblock2(lc,wname);
	break;

      }
    if (!(strcmp(ident->val,"step")))
      {
	i = 1;
	while (tptr != NULL)
	  {
	    if (tptr->ttype == S_EXPR)
	      {
		wavevarinitcheck(tptr);
		if ((i % 2) == 1)
		  {
		    fprintf(outfile,"   %s_rounding = ", lname);
		    blocktree(tptr->down, PRINTTOKENS);
		    fprintf(outfile,";\n");
		    fprintf(outfile,"   %s_x%i = ROUND(%s_rounding);\n",
			    lname, (i/2)+1, lname);
		  }
		else
		  {
		    fprintf(outfile,"   %s_y%i = ",lname,(i/2));
		    blocktree(tptr->down, PRINTTOKENS);
		    fprintf(outfile,";\n");
		  }
		i++;
	      }
	    tptr = tptr->next;
	  }
	i--;
	if (i % 2 == 0)
	  genex(&lc, wname, "Not enough parameters");
	else
	  {
	    i = (i/2) + 1;
	    while (i >= 2)
	      {
		mz(lc); sprintf(z[lc++],"if (%s_x%i > %s_x%i)",
				lname,i-1,lname,i);
		genex(&lc, wname, "xvals not a non-decreasing sequence");

		i--;
	      }
	    z[lc++]="if (%2$s_x1 != 0.0F)";
	    genex(&lc, wname, "x1 != 0");
	  }
	printwavesymblock2(lc,wname);
	break;
      }
    break;
  case 'w':
    if (!(strcmp(ident->val,"window")))
      {
	i = 1;
	while (tptr != NULL)
	  {
	    if (tptr->ttype == S_EXPR)
	      {
		wavevarinitcheck(tptr);
		switch (i) {
		case 1:
		  fprintf(outfile,"   %s_rounding = ", lname);
		  blocktree(tptr->down, PRINTTOKENS);
		  fprintf(outfile,";\n");

		  fprintf(outfile,"   %s_type = ROUND(%s_rounding);\n",
			  lname, lname);
		  break;
		case 2:
		  fprintf(outfile,"   %s_p = ",lname);
		  blocktree(tptr->down, PRINTTOKENS);
		  fprintf(outfile,";\n");
		  break;
		default:
		  printf("Error: Too many parameters.\n");
		  showerrorplace(wname->linenum, wname->filename);
		}
		i++;
	      }
	    tptr = tptr->next;
	  }
	break;
      }
    break;
  default:
    break;
  }
  free(lname);  
  currblockrate = currblocksafe;
}

extern void samplefilepreamble(sigsym *, int *);

/*********************************************************/
/* adds code common to all tables to wavegenerator       */
/*********************************************************/

void tablepreamble(sigsym * ident, char * prefix, int mode, int action)

{

  int lc = 0;
  int j;
  char * lname = makewstring(ident->val);

  tnode * aptr = ident->defnode->down->next->next->next->next->next->down;

  z[lc++]= " ";

  if (0)
    {
      z[lc++]= "NT(%1$s).start = 0;";
      z[lc++]= "NT(%1$s).end = 0;";
      z[lc++]= "NT(%1$s).sr = 0;";
      z[lc++]= "NT(%1$s).dint = 0;";
      z[lc++]= "NT(%1$s).dfrac = 0;";
      z[lc++]= "NT(%1$s).sffl = 0;";
      z[lc++]= "NT(%1$s).sfui = 0;";
      z[lc++]= "NT(%1$s).dsincr = 0;";
      z[lc++]= "NT(%1$s).base = 0;";
      z[lc++]= "NT(%1$s).oconst = 0;";
      z[lc++]= "NT(%1$s).tend = 0;";
    }

  z[lc++]= "i = NT(%1$s).len = %2$s_size;";

  switch (action) {
  case GENILLEGAL:
    z[lc++]= "if (i < 1)";
    genex(&lc, ident->defnode->down, "Table length < 1");
    break;
  case GENNUMDATA:
    z[lc++]= "if (i == -1) ";
    mz(lc); sprintf(z[lc++],"  %s_size = i = NT(%s_%s).len = %i;",
		    lname, prefix,ident->val, tablelistlength(aptr));
    z[lc++]= "if (i < 1)";
    genex(&lc, ident->defnode->down, "Table length < 1");
    break;
  case GENSTEP:
    z[lc++]= "if (i == -1) ";
    mz(lc);sprintf(z[lc++],"  %s_size = i = NT(%s_%s).len = %s_x%i;",
		   lname,prefix,ident->val,lname,
		   (tablelistlength(aptr)/2)+1);
    z[lc++]= "if (i < 1)";
    genex(&lc, ident->defnode->down, "Table length < 1");
    break;
  case GENPAIRS:
    z[lc++]= "if (i == -1) ";
    mz(lc);sprintf(z[lc++],"  %s_size = i = NT(%s_%s).len = %s_x%i;",
		   lname,prefix,ident->val,lname,
		   tablelistlength(aptr)/2);
    z[lc++]= "if (i < 1)";
    genex(&lc, ident->defnode->down, "Table length < 1");
    break;
  case GENCUBIC:
    z[lc++]= "if (i == -1) ";
    j = tablelistlength(aptr);
    mz(lc);
    if (4*(j/4) == j) 
      sprintf(z[lc++],"  %s_size = i = NT(%s_%s).len = %s_x%i;",
		     lname,prefix,ident->val,
		     lname,tablelistlength(aptr)/4);
    else
      sprintf(z[lc++],"  %s_size = i = NT(%s_%s).len = %s_infl%i;",
		     lname,prefix,ident->val,
		     lname,(tablelistlength(aptr)/4)+1);
    z[lc++]= "if (i < 1)";
    genex(&lc, ident->defnode->down, "Table length < 1");
    break;
  case GENSPLINE:
    z[lc++]= "if (i == -1) ";
    mz(lc);sprintf(z[lc++],"  %s_size = i = NT(%s_%s).len = %s_x%i;",
		   lname, prefix,ident->val,
		   lname,((tablelistlength(aptr)+1)/3));
    z[lc++]= "if (i < 1)";
    genex(&lc, ident->defnode->down, "Table length < 1");
    break;
  case GENBUZZ:
    z[lc++]="if ((%2$s_lowharm<0))";
    genex(&lc, ident->defnode->down, "Low harm must be >= 0");
    z[lc++]="if ((i < 1) && (%2$s_nharm<1))";
    genex(&lc, ident->defnode->down, "Buzz size and nharm both < 1");
    z[lc++]="if ((i < 1) || (%2$s_nharm<1))";
    z[lc++]="{";
    z[lc++]="  if (i < 1)";
    z[lc++]="    NT(%1$s).len = %2$s_size = i = 2*(%2$s_lowharm + %2$s_nharm);";
    z[lc++]="  else";
    z[lc++]="   %2$s_nharm = (int)floor(i/2.0 - %2$s_lowharm);";
    z[lc++]="}";
    z[lc++]="if (((%2$s_acc=fabs(%2$s_rolloff))<1.0F)&&(%2$s_rolloff!=0.0F))";
    z[lc++]="{";
    z[lc++]=  "j = -(int)(17.0F/log(%2$s_acc)) + %2$s_lowharm + 1;";
    z[lc++]=  "%2$s_nharm = (%2$s_nharm > j) ? j : %2$s_nharm;";
    z[lc++]="}";

    break;
  case GENCONCAT:
    z[lc++]= "if (i < 1)";
    z[lc++]= "{";
    z[lc++]= "  i = 0;";
    j = tablelistlength(aptr);
    while (j >=1)
      {
	mz(lc); sprintf(z[lc++],"  if (AP%i.t == NULL)", j);
	genex(&lc, ident->defnode->down, "Can't concat an uninitialized table");
	mz(lc); sprintf(z[lc++],"  i += AP%i.len;", j);
	j--;
      }
    z[lc++]= "   if (i < 1)";
    genex(&lc, ident->defnode->down, "Table length < 1");
    z[lc++]= "   NT(%1$s).len = %2$s_size = i;";
    z[lc++]= "}";
    break;
  default:
    internalerror("wtparse.c", "case statement");
  }

  if (interp == INTERP_SINC)
    {
      z[lc++]= "NT(%1$s).sffl = 1.0F;";
      z[lc++]= "NT(%1$s).sfui = 0x00010000;";
      z[lc++]= "NT(%1$s).dsincr = SINC_PILEN;";
    }

  z[lc++]= "NT(%1$s).stamp = EV(scorebeats);";
  z[lc++]= "NT(%1$s).lenf = (float)(i);";
  z[lc++]= "NT(%1$s).oconst = EV(ATIME)*((float)i);";
  z[lc++]= "if (i>0)";
  z[lc++]= "{";
  z[lc++]= "  NT(%1$s).tend = i - 1;";
  z[lc++]= "}";
  z[lc++]= "NT(%1$s).t = (float *) malloc((i+1)*sizeof(float));";
  z[lc++]= "NT(%1$s).llmem = 1;";
  z[lc++]= "i--;";
  z[lc++]= " ";
  printwavesymblock(lc,ident,prefix);
  free(lname);
  return;

}


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*                                                              */
/* Simpler utility functions used by code generation routines.  */
/*                                                              */
/*______________________________________________________________*/

/*********************************************************/
/*     do variable checks for generator execution        */
/*********************************************************/

void wavevarinitcheck(tnode * tptr)

{
  widthupdate(tptr);
  if (truewidth(tptr->width) != 1)
    {
      while (tptr->down != NULL)
	tptr = tptr->down;
      printf("Error: Generator arguments must be width 1.\n");
      showerrorplace(tptr->linenum, tptr->filename);
    }
  curropcoderate = ARATETYPE;
  currtreerate = UNKNOWN;
  rateupdate(tptr);
  if (tptr->rate != IRATETYPE)
    {
      while (tptr->down != NULL)
	tptr = tptr->down;
      printf("Error: Generators arguments must run at i-rate.\n");
      showerrorplace(tptr->linenum, tptr->filename);
    }
  if ((tptr->vartype == TMAPTYPE)||(tptr->vartype == TABLETYPE))
    {
      while (tptr->down != NULL)
	tptr = tptr->down;
      printf("Error: Table(map) an inappropriate argument.\n");
      showerrorplace(tptr->linenum, tptr->filename);
    }
}

/*********************************************************/
/* computes length of argument list for table (w/o size) */
/*********************************************************/

int tablelistlength(tnode * tptr)

{
  int j=0;

  tptr = tptr->next;
  if (tptr != NULL)
    tptr = tptr->next;
  while (tptr != NULL)
    {
      j++;
      tptr = tptr->next;
      if (tptr != NULL)
	tptr = tptr->next;
    }
  return j;
}

/*********************************************************/
/*       creates alias-free local variable name          */
/*********************************************************/

char * makewstring(char * val)

{
  char * lname;

  vmcheck(lname = calloc(strlen(val)+strlen("_sym_") + 1,
                         sizeof(char)));
  strcpy(lname, val);
  strcat(lname, "__sym");
  return(lname);

}


/*********************************************************/
/*          adds aiff/wav sample reading code            */
/*********************************************************/

void samplefilecode(sigsym * ident, char * prefix, int * lcptr)

{
  char newval[STRSIZE];
  int lc = *lcptr;
  sampleinfo * sinfo;
  double intdummy;
  tnode * sizeptr;
  tnode * fileptr;
  tnode * skipptr = NULL;
  unsigned int size = 0;
  int tsize = 0;
  unsigned int skip = 0;
  int known, currblocksafe;
  char * lname = makewstring(ident->val);
  char * suffix;
  char bytecount;

  /* see wtconst.c for regularization of constant size and skip */

  sizeptr = ident->defnode->down->next->next->next->next->next->down;
  fileptr = sizeptr->next->next;
  skipptr = (fileptr->next) ? fileptr->next->next : NULL;

  sinfo = (sampleinfo *) fileptr->ibus;

  /***********************************************/
  /* set variables independent of size and skip  */
  /***********************************************/

  z[lc++]= "NT(%1$s).stamp = EV(scorebeats);";

  mz(lc); sprintf(z[lc++], "NT(%s_%s).sr = %i;", prefix, ident->val,
		  sinfo->srate);

  if (reentrant)
    {
      mz(lc); sprintf(z[lc++], "if (EV(ARATE) == %u.0F)", sinfo->srate);
      mz(lc); sprintf(z[lc++], "  {");
      mz(lc); sprintf(z[lc++], "    NT(%s_%s).dint = 1;", prefix, ident->val);
      mz(lc); sprintf(z[lc++], "    NT(%s_%s).dfrac = 0;", prefix, ident->val);
      mz(lc); sprintf(z[lc++], "  }");
      mz(lc); sprintf(z[lc++], "else");
      mz(lc); sprintf(z[lc++], "  {");
      mz(lc); sprintf(z[lc++], "     NT(%s_%s).dfrac = (unsigned int)"
		      "(4294967296.0*((%u.0/EV(ARATE)) - ((int)(%u.0/EV(ARATE)))));",
		      prefix, ident->val, sinfo->srate, sinfo->srate);
      mz(lc); sprintf(z[lc++], "     NT(%s_%s).dint = (unsigned int)(%u.0/EV(ARATE));",
		      prefix, ident->val, sinfo->srate);
      mz(lc); sprintf(z[lc++], "  }");
    }
  else
    {
      if (srate == sinfo->srate)
	{
	  mz(lc); sprintf(z[lc++], "NT(%s_%s).dint = 1;", prefix, ident->val);
	  mz(lc); sprintf(z[lc++], "NT(%s_%s).dfrac = 0;", prefix, ident->val);
	}
      else
	{
	  mz(lc); sprintf(z[lc++], "NT(%s_%s).dfrac = %uU;", prefix, ident->val,
			  (unsigned int) (4294967296.0*
					   modf(((double)sinfo->srate)/((double)srate), 
						&intdummy)));
	  mz(lc); sprintf(z[lc++], "NT(%s_%s).dint = %uU;", prefix, ident->val,
			  (unsigned int) intdummy);
	}
    }

  if (interp == INTERP_SINC)
    {
      if (reentrant)
	{
	  mz(lc); sprintf(z[lc++], "if (EV(ARATE) >= %u.0F)", sinfo->srate);
	  mz(lc); sprintf(z[lc++], "  {");
	  mz(lc); sprintf(z[lc++],    "NT(%s_%s).sffl = 1.0F;", prefix, ident->val);
	  mz(lc); sprintf(z[lc++],    "NT(%s_%s).sfui = 0x00010000;", prefix, ident->val);
	  mz(lc); sprintf(z[lc++],    "NT(%s_%s).dsincr = SINC_PILEN;", prefix, ident->val);
	  mz(lc); sprintf(z[lc++], "  }");
	  mz(lc); sprintf(z[lc++], "else");
	  mz(lc); sprintf(z[lc++], "  {");
	  mz(lc); sprintf(z[lc++], "   if (SINC_UPMAX*EV(ARATE) > %u.0F)", 
			  sinfo->srate);
	  mz(lc); sprintf(z[lc++], "     NT(%s_%s).sffl = (EV(ARATE)/NT(%s_%s).sr);", 
			  prefix, ident->val, prefix, ident->val);
	  mz(lc); sprintf(z[lc++], "   else");
	  mz(lc); sprintf(z[lc++], "     NT(%s_%s).sffl = (1.0F/SINC_UPMAX);", 
			  prefix, ident->val);
	  mz(lc); sprintf(z[lc++], "   NT(%s_%s).sfui = ((float)(pow(2,16)))*NT(%s_%s).sffl + 0.5F;", 
			  prefix, ident->val, prefix, ident->val);
	  mz(lc); sprintf(z[lc++], "   NT(%s_%s).dsincr = (SINC_PILEN*NT(%s_%s).sfui) >> 16;", 
			  prefix, ident->val, prefix, ident->val);
	  mz(lc); sprintf(z[lc++], "  }");
	}
      else
	{
	  if (srate >= sinfo->srate)
	    {
	      mz(lc); sprintf(z[lc++], "NT(%s_%s).sffl = 1.0F;", prefix, ident->val);
	      mz(lc); sprintf(z[lc++], "NT(%s_%s).sfui = 0x00010000;", prefix, ident->val);
	      mz(lc); sprintf(z[lc++], "NT(%s_%s).dsincr = SINC_PILEN;", prefix, ident->val);
	    }
	  else
	    {
	      if (srate*sinc_upmax > sinfo->srate)
		{
		  mz(lc); sprintf(z[lc++], "NT(%s_%s).sffl = (EV(ARATE)/NT(%s_%s).sr);", 
				  prefix, ident->val, prefix, ident->val);
		}
	      else
		{
		  mz(lc); sprintf(z[lc++], "NT(%s_%s).sffl = (1.0F/SINC_UPMAX);", 
				  prefix, ident->val);
		}
	      
	      mz(lc); sprintf(z[lc++], 
			      "NT(%s_%s).sfui = ((float)(pow(2,16)))*NT(%s_%s).sffl + 0.5F;", 
			      prefix, ident->val, prefix, ident->val);
	      mz(lc); sprintf(z[lc++], 
			      "NT(%s_%s).dsincr = (SINC_PILEN*NT(%s_%s).sfui) >> 16;", 
			      prefix, ident->val, prefix, ident->val);
	    }	
	}
    }
  
  if (sinfo->hasbase)
    {      
      mz(lc); sprintf(z[lc++], "NT(%s_%s).base = %s;", prefix, ident->val,
		      compactfloat(newval, sinfo->base));
    }

  if (sinfo->hasloop)
    {	  
      mz(lc); sprintf(z[lc++], "NT(%s_%s).start = %i;", 
		      prefix, ident->val, sinfo->start);
      if (sinfo->end)
	{
	  mz(lc); sprintf(z[lc++], "NT(%s_%s).tend = NT(%s_%s).end = %i;", 
			  prefix, ident->val, prefix, ident->val, sinfo->end);
	}
    }

  /***********************************************/
  /* table parameters that depend on size/skip   */
  /***********************************************/

  if ((known = (sizeptr->vol == CONSTANT) && 
      ((skipptr == NULL) || (skipptr->vol == CONSTANT))))
    {
      /******************************/
      /* for constant size and skip */
      /******************************/

      if (skipptr)
	skip = atoi(skipptr->down->val);

      if (atoi(sizeptr->down->val) <= 0)
	{
	  tsize = 1;
	  size = sinfo->len - skip;
	}
      else
	size = atoi(sizeptr->down->val);

      mz(lc); sprintf(z[lc++], 
		      "NT(%s_%s).lenf = (float)(NT(%s_%s).len = %i);",
		      prefix,ident->val, prefix, ident->val, size);

      mz(lc); sprintf(z[lc++], "NT(%s_%s).oconst = NT(%s_%s).lenf*EV(ATIME);",
		      prefix,ident->val, prefix, ident->val);

      mz(lc); sprintf(z[lc++], "NT(%s_%s).t = (float *)(calloc(%i, %i));",
		      prefix, ident->val, size + 1, (int)sizeof(float));

      if (sinfo->hasloop)
	{	  
	  if (!(sinfo->end))
	    {
	      mz(lc); sprintf(z[lc++], "NT(%s_%s).tend = %i;",
			      prefix,ident->val, size - 1);
	    }
	}
      else
	{
	  mz(lc); sprintf(z[lc++], "NT(%s_%s).tend = %i;",
			  prefix,ident->val, size - 1);
	}
    }
  else
    {

      /**********************************/
      /* for variable size and/or skip */
      /*********************************/

      if ((skipptr == NULL) || (skipptr->vol == CONSTANT))
	{      
	  if (skipptr)
	    skip = atoi(skipptr->down->val);

	  mz(lc); sprintf(z[lc++], "%s_skip = %i;", lname, skip);
	}
      else
	{ 
	  currblocksafe = currblockrate;
	  currblockrate = IRATETYPE;	    
	  fprintf(outfile,"   %s_rounding = ", lname);
	  blocktree(skipptr->down, PRINTTOKENS);
	  fprintf(outfile,";\n");
	  fprintf(outfile,"   %s_skip = ROUND(%s_rounding);\n",
		  lname, lname);
	  fprintf(outfile,"   if (%s_skip < 0)\n",lname);
	  fprintf(outfile,"     %s_skip = 0;\n",lname);
	  currblockrate = currblocksafe;
	}

      mz(lc); sprintf(z[lc++], 
		      "NT(%s_%s).lenf = (float)(NT(%s_%s).len = "
		      "((%s_size <= 0) ? (%i - %s_skip) : %s_size));",
		      prefix, ident->val, prefix, ident->val, lname,
		      sinfo->len, lname, lname);

      mz(lc); sprintf(z[lc++], "NT(%s_%s).oconst = NT(%s_%s).lenf*EV(ATIME);",
		      prefix,ident->val, prefix, ident->val);

      z[lc++]= "NT(%1$s).t = (float *)(calloc(NT(%1$s).len +1,";
      z[lc++]= "                       sizeof(float)));";

      if (sinfo->hasloop)
	{
	  if (!(sinfo->end))
	    {
	      z[lc++]="NT(%1$s).tend = NT(%1$s).len - 1;";
	    }
	}
      else
	{
	  z[lc++]="NT(%1$s).tend = NT(%1$s).len - 1;";
	}
    }

  /***********************************/
  /* code for reading data from file */
  /***********************************/

  if ((suffix = strrchr(fileptr->val, '.')) && (suffix = strrchr(suffix, '@')))
    suffix[0] = '\0';

  mz(lc); sprintf(z[lc++], "EV(sfile) = fopen(\"%s\", \"rb\");", fileptr->val);

  if (suffix)
    suffix[0] = '@';

  z[lc++]= "if (EV(sfile) == NULL)";
  genex(&lc, ident->defnode->down, "Samplefile not found");

  if (known)
    {
      /* we don't fseek() to simplify signal handling for real-time */

      mz(lc); sprintf(z[lc++], "for (i = 0; i < %i; i++)", 
		      sinfo->point + skip*sinfo->framebytes);
      z[lc++]= "if (!rread(%2$s_c,1,1,EV(sfile)))";
      genex(&lc, ident->defnode->down, "Corrupt samplefile");

      mz(lc); sprintf(z[lc++], "for (i=0; i < %i; i++)", 
		      tsize ? size : ((size < (sinfo->len - skip)) ? 
				      size : (sinfo->len - skip)));  
      z[lc++]= "{";

    }
  else
    {

      mz(lc); sprintf(z[lc++], "for (i = 0; i < %i + %i*%s_skip; i++)", 
		      sinfo->point, sinfo->framebytes, lname);
      z[lc++]= "if (!rread(%2$s_c,1,1,EV(sfile)))";
      genex(&lc, ident->defnode->down, "Corrupt samplefile");

      z[lc++]="for (i=0; i < NT(%1$s).len; i++)";
      z[lc++]= "{";
      mz(lc); sprintf(z[lc++], "if (i >= (%i - %s_skip))", 
		      sinfo->len, lname);
      z[lc++]="  break;";
    }

  mz(lc); sprintf(z[lc++], "if (!rread(%s_c,1,%i,EV(sfile)))", 
		  lname, sinfo->framebytes);
  genex(&lc, ident->defnode->down, "Corrupt samplefile");

  if (sinfo->wav)
    {
      if (sinfo->chanpoint > -1)
	{
	  switch (sinfo->numbytes) {
	  case 3:
	    z[lc++]= "     NT(%1$s).t[i] = ((float)pow(2, -23))*(";
	    mz(lc);  sprintf(z[lc++], "    ((int)(%s_c[%hhi])) + ", 
			     lname, sinfo->chanpoint);
	    mz(lc);  sprintf(z[lc++], "    (((int)(%s_c[%hhi])) << 8) +", 
			     lname, sinfo->chanpoint + 1);
	    mz(lc);  sprintf(z[lc++], "    (((int)((char)(%s_c[%hhi]))) << 16));", 
			     lname, sinfo->chanpoint + 2);
	    break;
	  case 2:
	    z[lc++]= "     NT(%1$s).t[i] = ((float)pow(2, -15))*(";
	    mz(lc);  sprintf(z[lc++], "    ((int)(%s_c[%hhi])) + ", 
			     lname, sinfo->chanpoint);
	    mz(lc);  sprintf(z[lc++], "    (((int)((char)(%s_c[%hhi]))) << 8));", 
			     lname, sinfo->chanpoint + 1);
	    break;
	  case 1:
	    z[lc++]= "     NT(%1$s).t[i] = ((float)pow(2, -7))*(";
	    mz(lc);  sprintf(z[lc++], "    ((short)(%s_c[%hhi])) - 128); ", 
			     lname, sinfo->chanpoint);
	    break;
	  }
	}
      else
	{
	  switch (sinfo->numbytes) {
	  case 3:
	    z[lc++]= "     NT(%1$s).t[i] = ((float)pow(2, -23))*(";
	    
	    mz(lc);  sprintf(z[lc++], "     1.0F/%i.0F)*(", 
			     (int)(sinfo->framebytes/sinfo->numbytes));
	    break;
	  case 2:
	    z[lc++]= "     NT(%1$s).t[i] = ((float)pow(2, -15))*(";
	    
	    mz(lc);  sprintf(z[lc++], "     1.0F/%i.0F)*(", 
			     (int)(sinfo->framebytes/sinfo->numbytes));
	    break;
	  case 1:
	    z[lc++]= "     NT(%1$s).t[i] = ((float)pow(2, -7))*(";
	    
	    mz(lc);  sprintf(z[lc++], "     1.0F/%i.0F)*(", 
			     (int)(sinfo->framebytes/sinfo->numbytes));
	    break;
	  }

	  bytecount = 0;
	  while (bytecount < sinfo->framebytes)
	    {
	      switch (sinfo->numbytes) {
	      case 3:
		mz(lc);  sprintf(z[lc++], "    %c ((int)(%s_c[%hhi])) + ", 
				 bytecount ? '+' : ' ', lname, bytecount);
		mz(lc);  sprintf(z[lc++], "    (((int)(%s_c[%hhi])) << 8) +", 
				 lname, bytecount + 1);
		mz(lc);  sprintf(z[lc++], "    (((int)((char)(%s_c[%hhi]))) << 16)", 
				 lname, bytecount + 2);
		break;
	      case 2:
		mz(lc);  sprintf(z[lc++], "    %c ((int)(%s_c[%hhi])) + ", 
				 bytecount ? '+' : ' ', lname, bytecount);
		mz(lc);  sprintf(z[lc++], "    (((int)((char)(%s_c[%hhi]))) << 8)", 
				 lname, bytecount + 1);
		break;
	      case 1:
		mz(lc);  sprintf(z[lc++], "    %c ((short)(%s_c[%hhi])) - 128 ", 
				 bytecount ? '+' : ' ', lname, bytecount);
		break;
	      }
	      bytecount += sinfo->numbytes;
	    }
	  z[lc++]= "     );";
	}
    }
  else
    {
      if (sinfo->chanpoint > -1)
	{
	  switch (sinfo->numbytes) {
	  case 3:
	    z[lc++]= "     NT(%1$s).t[i] = ((float)pow(2, -23))*(";
	    mz(lc);  sprintf(z[lc++], "    ((int)(%s_c[%hhi])) + ", 
			     lname, sinfo->chanpoint + 2);
	    mz(lc);  sprintf(z[lc++], "    (((int)(%s_c[%hhi])) << 8) +", 
			     lname, sinfo->chanpoint + 1);
	    mz(lc);  sprintf(z[lc++], "    (((int)((char)(%s_c[%hhi]))) << 16));", 
			     lname, sinfo->chanpoint);
	    break;
	  case 2:
	    z[lc++]= "     NT(%1$s).t[i] = ((float)pow(2, -15))*(";
	    mz(lc);  sprintf(z[lc++], "    ((int)(%s_c[%hhi])) + ", 
			     lname, sinfo->chanpoint + 1);
	    mz(lc);  sprintf(z[lc++], "    (((int)((char)(%s_c[%hhi]))) << 8));", 
			     lname, sinfo->chanpoint);
	    break;
	  case 1:
	    z[lc++]= "     NT(%1$s).t[i] = ((float)pow(2, -7))*(";
	    mz(lc);  sprintf(z[lc++], "    ((short)(%s_c[%hhi]))); ", 
			     lname, sinfo->chanpoint);
	    break;
	  }
	}
      else
	{
	  switch (sinfo->numbytes) {
	  case 3:
	    z[lc++]= "     NT(%1$s).t[i] = ((float)pow(2, -23))*(";

	    mz(lc);  sprintf(z[lc++], "     1.0F/%i.0F)*(", 
			     (int)(sinfo->framebytes/sinfo->numbytes));
	    break;
	  case 2:
	    z[lc++]= "     NT(%1$s).t[i] = ((float)pow(2, -15))*(";
	    
	    mz(lc);  sprintf(z[lc++], "     1.0F/%i.0F)*(", 
			     (int)(sinfo->framebytes/sinfo->numbytes));
	    break;
	  case 1:
	    z[lc++]= "     NT(%1$s).t[i] = ((float)pow(2, -7))*(";
	    
	    mz(lc);  sprintf(z[lc++], "     1.0F/%i.0F)*(", 
			     (int)(sinfo->framebytes/sinfo->numbytes));
	    break;
	  }

	  bytecount = 0;
	  while (bytecount < sinfo->framebytes)
	    {
	      switch (sinfo->numbytes) {
	      case 3:
		mz(lc);  sprintf(z[lc++], "    %c ((int)(%s_c[%hhi])) + ", 
				 bytecount ? '+': ' ', lname, bytecount + 2);
		mz(lc);  sprintf(z[lc++], "    (((int)(%s_c[%hhi])) << 8) +", 
				 lname, bytecount + 1);
		mz(lc);  sprintf(z[lc++], "    (((int)((char)(%s_c[%hhi]))) << 16)", 
				 lname, bytecount);
		break;
	      case 2:
		mz(lc);  sprintf(z[lc++], "    %c ((int)(%s_c[%hhi])) + ", 
				 bytecount ? '+': ' ', lname, bytecount + 1);
		mz(lc);  sprintf(z[lc++], "    (((int)((char)(%s_c[%hhi]))) << 8)", 
				 lname, bytecount);
		break;
	      case 1:
		mz(lc);  sprintf(z[lc++], "    %c ((short)(%s_c[%hhi])) ", 
				 bytecount ? '+': ' ', lname, bytecount);
		break;
	      }
	      bytecount += sinfo->numbytes;
	    }
	  z[lc++]= "     );";
	}
    }	

  z[lc++]= "}";

  z[lc++]= " fclose(EV(sfile));";
  if (known)
    {
      mz(lc); sprintf(z[lc++], "NT(%s_%s).t[%i] = NT(%s_%s).t[0];",
		      prefix, ident->val, size, prefix, ident->val);
    }
  else
    {
      z[lc++]= "NT(%1$s).t[NT(%1$s).len] = NT(%1$s).t[0];";
    }

  *lcptr = lc;
  free(lname);

}

/*********************************************************/
/*          simplifies random if dist is a constant      */
/*********************************************************/

void randomconstcode(sigsym * ident, char * prefix, 
		     tnode * aptr, int * lcptr)

{
  int lc = *lcptr;
  tnode * distptr = aptr->next->next;
  tnode * p1ptr;
  tnode * p2ptr;
  int dist;

  dist = make_int(distptr->down);

  if ((dist < 1) || (dist > 5))
    {      
      printf("Error: Illegal random wavetable distribution %i.\n", dist);
      showerrorplace(aptr->down->linenum, aptr->down->filename);
    }

  p1ptr = distptr->next->next;
  p2ptr = p1ptr->next ? p1ptr->next->next : NULL;

  if ((p2ptr == NULL) && ((dist == 1) || (dist == 2)|| (dist == 4)))
    {      
      printf("Error: Random wavetable distribution %i needs p2.\n", dist);
      showerrorplace(aptr->down->linenum, aptr->down->filename);
    }

  switch (dist) {
  case 1:
    z[lc++] = "  %2$s_c1 = %2$s_p2 - %2$s_p1;";
    break;
  case 2:	    
    z[lc++] = "  %2$s_c1 = %2$s_p2 - %2$s_p1;";
    z[lc++] = "if (%2$s_p2 == %2$s_p1)";
    genex(&lc, ident->defnode->down, "p1 == p2 (dist 2)");
    break;
  case 3:	    
    break;
  case 4:
    z[lc++] = "  if (%2$s_p2 <= 0.0F)";
    genex(&lc, ident->defnode->down, "p2 <= 0 (dist 4)");
    z[lc++] = "  %2$s_c1 = (float)sqrt(2*%2$s_p2);";
    break;
  case 5:
    z[lc++] = "j = 0;";
    break;
  }

  z[lc++] = "i = 0;";
  z[lc++] = "while (i < %2$s_size)";
  z[lc++] = " {";

  switch (dist) {
  case 1:	
    z[lc++] = "     NT(%1$s).t[i] = %2$s_c1*RMULT*((float)rand()) + %2$s_p1;";
    break;
  case 2:		
    z[lc++] = "     %2$s_x = RMULT*((float)rand());";
    z[lc++] = "     %2$s_y = RMULT*((float)rand());";
    z[lc++] = "     if (%2$s_x > %2$s_y)";
    z[lc++] = "       NT(%1$s).t[i] =  %2$s_c1*%2$s_x + %2$s_p1;";
    z[lc++] = "     else";
    z[lc++] = "       NT(%1$s).t[i] =  %2$s_c1*%2$s_y + %2$s_p1;";    
    break;
  case 3:	
    z[lc++] = "     NT(%1$s).t[i] = -%2$s_p1*(float)log(RMULT*((float)rand())+1e-45F);";    
    break;
  case 4:	
    z[lc++] = "     NT(%1$s).t[i] = %2$s_p1+%2$s_c1*";
    z[lc++] = "             (float)sqrt(-2.0F*(float)log(RMULT*((float)rand())+1e-45F))";
    z[lc++] = "               *(float)cos(6.283185F*RMULT*((float)rand()));";
    break;
  case 5:	
    z[lc++] = "     NT(%1$s).t[i] = 0;";
    z[lc++] = "     if (j == 0)";
    z[lc++] = "      {";
    z[lc++] = "        j = ROUND(-%2$s_p1*(float)log(RMULT*((float)rand())+1e-45F))+1;";
    z[lc++] = "        if (i != 0)";
    z[lc++] = "         NT(%1$s).t[i] = 1.0F;";
    z[lc++] = "      }";
    z[lc++] = "      j--;";
    break;
  }
  z[lc++] = "  i++;";
  z[lc++] = " }";
  z[lc++] = "  NT(%1$s).t[NT(%1$s).len] = NT(%1$s).t[0];";

  *lcptr = lc;

}
