
/*
#    Sfront, a SAOL to C translator    
#    This file: Included file in sfront runtime
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


#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <signal.h>

/********************************/
/* readabiliy-improving defines */
/********************************/

#define ROUND(x) ( ((x) > 0.0F) ? ((int) ((x) + 0.5F)) :  ((int) ((x) - 0.5F)))
#define POS(x)   (((x) > 0.0F) ? x : 0.0F)
#define RMULT ((float)(1.0F/(RAND_MAX + 1.0F)))

#define NOTUSEDYET 0
#define TOBEPLAYED 1
#define PAUSED     2
#define PLAYING    3
#define ALLDONE    4

#define NOTLAUNCHED 0
#define LAUNCHED 1

#define ASYS_DONE        0
#define ASYS_EXIT        1
#define ASYS_ERROR       2

#define IPASS 1
#define KPASS 2
#define APASS 3

#define IOERROR_RETRY 256 

/* integer type that holds a cast pointer */

#define PSIZE long 

/************************************/
/*  union for a data stack element  */
/************************************/

typedef union {

float f;
int i;
unsigned int ui;
PSIZE ps;

} dstack;

/************************************/
/* ntables: table entries for notes */
/************************************/

typedef struct tableinfo {

int    len;                /* length of table             */
float  lenf;               /* length of table, as a float */

int    start;              /* loop start position  */
int    end;                /* loop end position    */
float  sr;                 /* table sampling rate  */
float  base;               /* table base frequency */

                           /* precomputed constants       */
int tend;                  /* len -1 if end==0            */
float oconst;              /* len*ATIME                   */

unsigned int dint;         /* doscil: 64-bit phase incr   */
unsigned int dfrac;
                           /* doscil: sinc interpolation        */
unsigned int sfui;         /* scale_factor as unsigned int      */
float sffl;                /* scale_factor as a float           */
unsigned int dsincr;       /* sinc pointer increment (d=doscil) */

float  *t;                 /* pointer to table entries    */
float stamp;               /* timestamp on table contents */
char llmem;                /* 1 if *t was malloced        */

} tableinfo; 

/********************/
/*   tempo lines    */
/********************/

typedef struct stempo_lines {

  float t;          /* trigger time */
  float newtempo;   /* new tempo */ 

} stempo_lines;

/********************/
/*   table lines    */
/********************/

typedef struct stable_lines {

  float t;          /* trigger time */
  int gindex;       /* global table to target */
  int size;         /* size of data */
  void (*tmake) (); /* function   */
  void * data;      /* data block */

} stable_lines;

