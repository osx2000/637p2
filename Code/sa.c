
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


/* defines for a non-rentrant sa.c file */

#define ENGINE_NONREENTRANT  0
#define ENGINE_REENTRANT     1

#define ENGINE_STYLE  ENGINE_NONREENTRANT
#define ENGINE_PTR_TYPE  void
#define ENGINE_PTR_TYPE_COMMA  
#define ENGINE_PTR_DECLARE  void
#define ENGINE_PTR_DECLARE_COMMA  
#define ENGINE_PTR_DECLARE_SEMICOLON  
#define ENGINE_PTR 
#define ENGINE_PTR_COMMA 
#define ENGINE_PTR_ASSIGNED_TO  
#define ENGINE_PTR_CREATE_SEMICOLON  
#define ENGINE_SIZE 
#define ENGINE_PTR_NULLRETURN_SEMICOLON 
#define ENGINE_PTR_IS_NULL  (0)
#define ENGINE_PTR_IS_NOT_NULL  (1)
#define ENGINE_PTR_DESTROY_SEMICOLON 
#define ENGINE_PTR_RETURN_SEMICOLON  return;
#define EV(x)   x

#define NV(x)   nstate->v[x].f
#define NVI(x)  nstate->v[x].i
#define NVUI(x) nstate->v[x].ui
#define NVPS(x) nstate->v[x].ps
#define NVU(x)  nstate->v[x]
#define NT(x)   nstate->t[x]
#define NS(x)   nstate->x
#define NSP     ENGINE_PTR_COMMA nstate
#define NP(x)   nstate->v[x].f
#define NPI(x)  nstate->v[x].i
#define NPUI(x) nstate->v[x].ui

#define NPPS(x) nstate->v[x].ps

#define NG(x)   EV(global)[x].f
#define NGI(x)  EV(global)[x].i
#define NGUI(x) EV(global)[x].ui
#define NGU(x)  EV(global)[x]
#define TB(x)   EV(bus[x])
#define STB(x)  EV(sbus[x])

#define MAXPFIELDS 1

typedef struct instr_line { 

float starttime;  /* score start time of note */
float endtime;    /* score end time of note */
float startabs;   /* absolute start time of note */
float endabs;     /* absolute end time of note */
float abstime;    /* absolute time extension */
float time;       /* time of note start (absolute) */
float itime;      /* elapsed time (absolute) */
float sdur;       /* duration of note in score time*/

int kbirth;       /* kcycleidx for note launch */
int released;     /* flag for turnoff*/
int turnoff;      /* flag for turnoff */
int noteon;       /* NOTYETUSED,TOBEPLAYED,PAUSED,PLAYING,ALLDONE */
int notestate;    /* index into state array */
int launch;       /* only for dynamic instruments */
int numchan;      /* only for MIDI notes */
int preset;       /* only for MIDI notes */
int notenum;      /* only for MIDI notes */
int label;        /* SASL label index + 1, or 0 (no label) */
                  /* for static MIDI, all-sounds noteoff */

float p[MAXPFIELDS];  /* parameters */

struct ninstr_types * nstate; /* pointer into state array */

} instr_line;

typedef struct scontrol_lines {

float t;                  /* trigger time */
int label;                /* index into label array */
int siptr;                /* score instr line to control */
instr_line * iline;       /* pointer to score line */
int imptr;                /* position of variable in v[] */
float imval;              /* value to import into v[] */

} scontrol_lines;

typedef struct csys_table_ptr {

int num;                  /* number of floats in array */
float * t;                /* pointer to array */

} csys_table_ptr;



int csys_sfront_argc = 7;

char * csys_sfront_argv[7] = {
"/Users/SP/CS/637/sfront/bin/sfront",
"-aout",
"output.wav",
"-orc",
"flute.saol",
"-sco",
"flute.sasl"};



/* global engine begins here */

/* audio and control rates */

float globaltune;
float invglobaltune;
float tempo;
float scoremult;
float scorebeats;          /* current score beat */
float absolutetime;        /* current absolute time */
int kbase;                 /* kcycle of last tempo change */
float scorebase;           /* scorebeat of last tempo change */

/* counters & bounds acycles and kcycles */

int endkcycle;
int kcycleidx;
int acycleidx;
int pass;
int beginflag;
sig_atomic_t graceful_exit;

int nextstate;     /* counter for active instrument state */
int oldstate;      /* detects loops in nextstate updates  */
float cpuload;     /* current cpu-time value              */

#define APPNAME "sfront"
#define APPVERSION "0.99 08/30/11"
#define NSYS_NET 0
#define INTERP_LINEAR 0
#define INTERP_SINC 1
#define INTERP_TYPE INTERP_LINEAR

#define SAOL_SRATE 44100.0F
#define SAOL_KRATE 100.0F

#define ARATE 44100.0F
#define ATIME 2.267574e-05F
#define ACYCLE 441

#define KRATE 100.0F
#define KTIME 1.000000e-02F
#define KMTIME 1.000000e+01F
#define KUTIME 10000
#define ASYS_RENDER   0
#define ASYS_PLAYBACK 1
#define ASYS_TIMESYNC 2

#define ASYS_SHORT   0
#define ASYS_FLOAT   1

/* audio i/o */

#define ASYS_OCHAN 1
#define ASYS_OTYPENAME  ASYS_FLOAT
#define ASYS_OTYPE  float

int asys_osize;
int obusidx;

ASYS_OTYPE * asys_obuf;

#define ASYS_USERLATENCY  0
#define ASYS_LOWLATENCY   0
#define ASYS_HIGHLATENCY  1
#define ASYS_LATENCYTYPE  ASYS_HIGHLATENCY
#define ASYS_LATENCY 0.300000F
#define ASYS_TIMEOPTION ASYS_RENDER

/* AIF or WAV output file wordsize */

#define ASYS_OUTFILE_WORDSIZE_8BIT  0
#define ASYS_OUTFILE_WORDSIZE_16BIT  1
#define ASYS_OUTFILE_WORDSIZE_24BIT  2
#define ASYS_OUTFILE_WORDSIZE  1

int asys_argc;
char ** asys_argv;

#define BUS_output_bus 0
#define ENDBUS_output_bus 1
#define ENDBUS 1

float bus[ENDBUS];

float fakeMIDIctrl[256];

#define GBL_STARTVAR 0
#define GBL_cyc 0
#define GBL_ENDVAR 1

/* SAOL global block variables */

dstack global[GBL_ENDVAR+1];

#define TBL_GBL_cyc 0
#define GBL_ENDTBL 1

tableinfo gtables[GBL_ENDTBL+1];

#define flute_fr 0
#define flute_cyc 1
#define flute_env01 2
#define flute_env02 3
#define flute_env03 4
#define flute_env04 5
#define flute_env05 6
#define flute_env06 7
#define flute_env07 8
#define flute_env08 9
#define flute_env09 10
#define flute_env10 11
#define flute_env11 12
#define flute_env12 13
#define flute_env13 14
#define flute_env14 15
#define flute_env15 16
#define flute_env16 17
#define flute_env17 18
#define flute_env18 19
#define flute_env19 20
#define flute_env20 21
#define flute_y01 22
#define flute_y02 23
#define flute_y03 24
#define flute_y04 25
#define flute_y05 26
#define flute_y06 27
#define flute_y07 28
#define flute_y08 29
#define flute_y09 30
#define flute_y10 31
#define flute_y11 32
#define flute_y12 33
#define flute_y13 34
#define flute_y14 35
#define flute_y15 36
#define flute_y16 37
#define flute_y17 38
#define flute_y18 39
#define flute_y19 40
#define flute_y20 41
#define flute__tvr19 42
#define flute__tvr18 43
#define flute__tvr17 44
#define flute__tvr16 45
#define flute__tvr15 46
#define flute__tvr14 47
#define flute__tvr13 48
#define flute__tvr12 49
#define flute__tvr11 50
#define flute__tvr10 51
#define flute__tvr9 52
#define flute__tvr8 53
#define flute__tvr7 54
#define flute__tvr6 55
#define flute__tvr5 56
#define flute__tvr4 57
#define flute__tvr3 58
#define flute__tvr2 59
#define flute__tvr1 60
#define flute__tvr0 61
#define flute_kline1_first 62
#define flute_kline1_addK 63
#define flute_kline1_outT 64
#define flute_kline1_mult 65
#define flute_kline1_cdur 66
#define flute_kline1_crp 67
#define flute_kline1_clp 68
#define flute_kline1_t 69
#define flute_kline1_x2 70
#define flute_kline1_dur1 71
#define flute_kline1_x1 72
#define flute_kline1_return 73
#define flute_kline2_first 74
#define flute_kline2_addK 75
#define flute_kline2_outT 76
#define flute_kline2_mult 77
#define flute_kline2_cdur 78
#define flute_kline2_crp 79
#define flute_kline2_clp 80
#define flute_kline2_t 81
#define flute_kline2_x2 82
#define flute_kline2_dur1 83
#define flute_kline2_x1 84
#define flute_kline2_return 85
#define flute_kline3_first 86
#define flute_kline3_addK 87
#define flute_kline3_outT 88
#define flute_kline3_mult 89
#define flute_kline3_cdur 90
#define flute_kline3_crp 91
#define flute_kline3_clp 92
#define flute_kline3_t 93
#define flute_kline3_x2 94
#define flute_kline3_dur1 95
#define flute_kline3_x1 96
#define flute_kline3_return 97
#define flute_kline4_first 98
#define flute_kline4_addK 99
#define flute_kline4_outT 100
#define flute_kline4_mult 101
#define flute_kline4_cdur 102
#define flute_kline4_crp 103
#define flute_kline4_clp 104
#define flute_kline4_t 105
#define flute_kline4_x2 106
#define flute_kline4_dur1 107
#define flute_kline4_x1 108
#define flute_kline4_return 109
#define flute_kline5_first 110
#define flute_kline5_addK 111
#define flute_kline5_outT 112
#define flute_kline5_mult 113
#define flute_kline5_cdur 114
#define flute_kline5_crp 115
#define flute_kline5_clp 116
#define flute_kline5_t 117
#define flute_kline5_x2 118
#define flute_kline5_dur1 119
#define flute_kline5_x1 120
#define flute_kline5_return 121
#define flute_kline6_first 122
#define flute_kline6_addK 123
#define flute_kline6_outT 124
#define flute_kline6_mult 125
#define flute_kline6_cdur 126
#define flute_kline6_crp 127
#define flute_kline6_clp 128
#define flute_kline6_t 129
#define flute_kline6_x2 130
#define flute_kline6_dur1 131
#define flute_kline6_x1 132
#define flute_kline6_return 133
#define flute_kline7_first 134
#define flute_kline7_addK 135
#define flute_kline7_outT 136
#define flute_kline7_mult 137
#define flute_kline7_cdur 138
#define flute_kline7_crp 139
#define flute_kline7_clp 140
#define flute_kline7_t 141
#define flute_kline7_x2 142
#define flute_kline7_dur1 143
#define flute_kline7_x1 144
#define flute_kline7_return 145
#define flute_kline8_first 146
#define flute_kline8_addK 147
#define flute_kline8_outT 148
#define flute_kline8_mult 149
#define flute_kline8_cdur 150
#define flute_kline8_crp 151
#define flute_kline8_clp 152
#define flute_kline8_t 153
#define flute_kline8_x2 154
#define flute_kline8_dur1 155
#define flute_kline8_x1 156
#define flute_kline8_return 157
#define flute_kline9_first 158
#define flute_kline9_addK 159
#define flute_kline9_outT 160
#define flute_kline9_mult 161
#define flute_kline9_cdur 162
#define flute_kline9_crp 163
#define flute_kline9_clp 164
#define flute_kline9_t 165
#define flute_kline9_x2 166
#define flute_kline9_dur1 167
#define flute_kline9_x1 168
#define flute_kline9_return 169
#define flute_kline10_first 170
#define flute_kline10_addK 171
#define flute_kline10_outT 172
#define flute_kline10_mult 173
#define flute_kline10_cdur 174
#define flute_kline10_crp 175
#define flute_kline10_clp 176
#define flute_kline10_t 177
#define flute_kline10_x2 178
#define flute_kline10_dur1 179
#define flute_kline10_x1 180
#define flute_kline10_return 181
#define flute_kline11_first 182
#define flute_kline11_addK 183
#define flute_kline11_outT 184
#define flute_kline11_mult 185
#define flute_kline11_cdur 186
#define flute_kline11_crp 187
#define flute_kline11_clp 188
#define flute_kline11_t 189
#define flute_kline11_x2 190
#define flute_kline11_dur1 191
#define flute_kline11_x1 192
#define flute_kline11_return 193
#define flute_kline12_first 194
#define flute_kline12_addK 195
#define flute_kline12_outT 196
#define flute_kline12_mult 197
#define flute_kline12_cdur 198
#define flute_kline12_crp 199
#define flute_kline12_clp 200
#define flute_kline12_t 201
#define flute_kline12_x2 202
#define flute_kline12_dur1 203
#define flute_kline12_x1 204
#define flute_kline12_return 205
#define flute_kline13_first 206
#define flute_kline13_addK 207
#define flute_kline13_outT 208
#define flute_kline13_mult 209
#define flute_kline13_cdur 210
#define flute_kline13_crp 211
#define flute_kline13_clp 212
#define flute_kline13_t 213
#define flute_kline13_x2 214
#define flute_kline13_dur1 215
#define flute_kline13_x1 216
#define flute_kline13_return 217
#define flute_kline14_first 218
#define flute_kline14_addK 219
#define flute_kline14_outT 220
#define flute_kline14_mult 221
#define flute_kline14_cdur 222
#define flute_kline14_crp 223
#define flute_kline14_clp 224
#define flute_kline14_t 225
#define flute_kline14_x2 226
#define flute_kline14_dur1 227
#define flute_kline14_x1 228
#define flute_kline14_return 229
#define flute_kline15_first 230
#define flute_kline15_addK 231
#define flute_kline15_outT 232
#define flute_kline15_mult 233
#define flute_kline15_cdur 234
#define flute_kline15_crp 235
#define flute_kline15_clp 236
#define flute_kline15_t 237
#define flute_kline15_x2 238
#define flute_kline15_dur1 239
#define flute_kline15_x1 240
#define flute_kline15_return 241
#define flute_kline16_first 242
#define flute_kline16_addK 243
#define flute_kline16_outT 244
#define flute_kline16_mult 245
#define flute_kline16_cdur 246
#define flute_kline16_crp 247
#define flute_kline16_clp 248
#define flute_kline16_t 249
#define flute_kline16_x2 250
#define flute_kline16_dur1 251
#define flute_kline16_x1 252
#define flute_kline16_return 253
#define flute_kline17_first 254
#define flute_kline17_addK 255
#define flute_kline17_outT 256
#define flute_kline17_mult 257
#define flute_kline17_cdur 258
#define flute_kline17_crp 259
#define flute_kline17_clp 260
#define flute_kline17_t 261
#define flute_kline17_x2 262
#define flute_kline17_dur1 263
#define flute_kline17_x1 264
#define flute_kline17_return 265
#define flute_kline18_first 266
#define flute_kline18_addK 267
#define flute_kline18_outT 268
#define flute_kline18_mult 269
#define flute_kline18_cdur 270
#define flute_kline18_crp 271
#define flute_kline18_clp 272
#define flute_kline18_t 273
#define flute_kline18_x2 274
#define flute_kline18_dur1 275
#define flute_kline18_x1 276
#define flute_kline18_return 277
#define flute_kline19_first 278
#define flute_kline19_addK 279
#define flute_kline19_outT 280
#define flute_kline19_mult 281
#define flute_kline19_cdur 282
#define flute_kline19_crp 283
#define flute_kline19_clp 284
#define flute_kline19_t 285
#define flute_kline19_x2 286
#define flute_kline19_dur1 287
#define flute_kline19_x1 288
#define flute_kline19_return 289
#define flute_kline20_first 290
#define flute_kline20_addK 291
#define flute_kline20_outT 292
#define flute_kline20_mult 293
#define flute_kline20_cdur 294
#define flute_kline20_crp 295
#define flute_kline20_clp 296
#define flute_kline20_t 297
#define flute_kline20_x2 298
#define flute_kline20_dur1 299
#define flute_kline20_x1 300
#define flute_kline20_return 301
#define flute_oscil21_fsign 302
#define flute_oscil21_kfrac 303
#define flute_oscil21_kint 304
#define flute_oscil21_pfrac 305
#define flute_oscil21_pint 306
#define flute_oscil21_kcyc 307
#define flute_oscil21_iloops 308
#define flute_oscil21_freq 309
#define flute_oscil21_t 310
#define flute_oscil21_return 311
#define flute_oscil22_fsign 312
#define flute_oscil22_kfrac 313
#define flute_oscil22_kint 314
#define flute_oscil22_pfrac 315
#define flute_oscil22_pint 316
#define flute_oscil22_kcyc 317
#define flute_oscil22_iloops 318
#define flute_oscil22_freq 319
#define flute_oscil22_t 320
#define flute_oscil22_return 321
#define flute_oscil23_fsign 322
#define flute_oscil23_kfrac 323
#define flute_oscil23_kint 324
#define flute_oscil23_pfrac 325
#define flute_oscil23_pint 326
#define flute_oscil23_kcyc 327
#define flute_oscil23_iloops 328
#define flute_oscil23_freq 329
#define flute_oscil23_t 330
#define flute_oscil23_return 331
#define flute_oscil24_fsign 332
#define flute_oscil24_kfrac 333
#define flute_oscil24_kint 334
#define flute_oscil24_pfrac 335
#define flute_oscil24_pint 336
#define flute_oscil24_kcyc 337
#define flute_oscil24_iloops 338
#define flute_oscil24_freq 339
#define flute_oscil24_t 340
#define flute_oscil24_return 341
#define flute_oscil25_fsign 342
#define flute_oscil25_kfrac 343
#define flute_oscil25_kint 344
#define flute_oscil25_pfrac 345
#define flute_oscil25_pint 346
#define flute_oscil25_kcyc 347
#define flute_oscil25_iloops 348
#define flute_oscil25_freq 349
#define flute_oscil25_t 350
#define flute_oscil25_return 351
#define flute_oscil26_fsign 352
#define flute_oscil26_kfrac 353
#define flute_oscil26_kint 354
#define flute_oscil26_pfrac 355
#define flute_oscil26_pint 356
#define flute_oscil26_kcyc 357
#define flute_oscil26_iloops 358
#define flute_oscil26_freq 359
#define flute_oscil26_t 360
#define flute_oscil26_return 361
#define flute_oscil27_fsign 362
#define flute_oscil27_kfrac 363
#define flute_oscil27_kint 364
#define flute_oscil27_pfrac 365
#define flute_oscil27_pint 366
#define flute_oscil27_kcyc 367
#define flute_oscil27_iloops 368
#define flute_oscil27_freq 369
#define flute_oscil27_t 370
#define flute_oscil27_return 371
#define flute_oscil28_fsign 372
#define flute_oscil28_kfrac 373
#define flute_oscil28_kint 374
#define flute_oscil28_pfrac 375
#define flute_oscil28_pint 376
#define flute_oscil28_kcyc 377
#define flute_oscil28_iloops 378
#define flute_oscil28_freq 379
#define flute_oscil28_t 380
#define flute_oscil28_return 381
#define flute_oscil29_fsign 382
#define flute_oscil29_kfrac 383
#define flute_oscil29_kint 384
#define flute_oscil29_pfrac 385
#define flute_oscil29_pint 386
#define flute_oscil29_kcyc 387
#define flute_oscil29_iloops 388
#define flute_oscil29_freq 389
#define flute_oscil29_t 390
#define flute_oscil29_return 391
#define flute_oscil30_fsign 392
#define flute_oscil30_kfrac 393
#define flute_oscil30_kint 394
#define flute_oscil30_pfrac 395
#define flute_oscil30_pint 396
#define flute_oscil30_kcyc 397
#define flute_oscil30_iloops 398
#define flute_oscil30_freq 399
#define flute_oscil30_t 400
#define flute_oscil30_return 401
#define flute_oscil31_fsign 402
#define flute_oscil31_kfrac 403
#define flute_oscil31_kint 404
#define flute_oscil31_pfrac 405
#define flute_oscil31_pint 406
#define flute_oscil31_kcyc 407
#define flute_oscil31_iloops 408
#define flute_oscil31_freq 409
#define flute_oscil31_t 410
#define flute_oscil31_return 411
#define flute_oscil32_fsign 412
#define flute_oscil32_kfrac 413
#define flute_oscil32_kint 414
#define flute_oscil32_pfrac 415
#define flute_oscil32_pint 416
#define flute_oscil32_kcyc 417
#define flute_oscil32_iloops 418
#define flute_oscil32_freq 419
#define flute_oscil32_t 420
#define flute_oscil32_return 421
#define flute_oscil33_fsign 422
#define flute_oscil33_kfrac 423
#define flute_oscil33_kint 424
#define flute_oscil33_pfrac 425
#define flute_oscil33_pint 426
#define flute_oscil33_kcyc 427
#define flute_oscil33_iloops 428
#define flute_oscil33_freq 429
#define flute_oscil33_t 430
#define flute_oscil33_return 431
#define flute_oscil34_fsign 432
#define flute_oscil34_kfrac 433
#define flute_oscil34_kint 434
#define flute_oscil34_pfrac 435
#define flute_oscil34_pint 436
#define flute_oscil34_kcyc 437
#define flute_oscil34_iloops 438
#define flute_oscil34_freq 439
#define flute_oscil34_t 440
#define flute_oscil34_return 441
#define flute_oscil35_fsign 442
#define flute_oscil35_kfrac 443
#define flute_oscil35_kint 444
#define flute_oscil35_pfrac 445
#define flute_oscil35_pint 446
#define flute_oscil35_kcyc 447
#define flute_oscil35_iloops 448
#define flute_oscil35_freq 449
#define flute_oscil35_t 450
#define flute_oscil35_return 451
#define flute_oscil36_fsign 452
#define flute_oscil36_kfrac 453
#define flute_oscil36_kint 454
#define flute_oscil36_pfrac 455
#define flute_oscil36_pint 456
#define flute_oscil36_kcyc 457
#define flute_oscil36_iloops 458
#define flute_oscil36_freq 459
#define flute_oscil36_t 460
#define flute_oscil36_return 461
#define flute_oscil37_fsign 462
#define flute_oscil37_kfrac 463
#define flute_oscil37_kint 464
#define flute_oscil37_pfrac 465
#define flute_oscil37_pint 466
#define flute_oscil37_kcyc 467
#define flute_oscil37_iloops 468
#define flute_oscil37_freq 469
#define flute_oscil37_t 470
#define flute_oscil37_return 471
#define flute_oscil38_fsign 472
#define flute_oscil38_kfrac 473
#define flute_oscil38_kint 474
#define flute_oscil38_pfrac 475
#define flute_oscil38_pint 476
#define flute_oscil38_kcyc 477
#define flute_oscil38_iloops 478
#define flute_oscil38_freq 479
#define flute_oscil38_t 480
#define flute_oscil38_return 481
#define flute_oscil39_fsign 482
#define flute_oscil39_kfrac 483
#define flute_oscil39_kint 484
#define flute_oscil39_pfrac 485
#define flute_oscil39_pint 486
#define flute_oscil39_kcyc 487
#define flute_oscil39_iloops 488
#define flute_oscil39_freq 489
#define flute_oscil39_t 490
#define flute_oscil39_return 491
#define flute_oscil40_fsign 492
#define flute_oscil40_kfrac 493
#define flute_oscil40_kint 494
#define flute_oscil40_pfrac 495
#define flute_oscil40_pint 496
#define flute_oscil40_kcyc 497
#define flute_oscil40_iloops 498
#define flute_oscil40_freq 499
#define flute_oscil40_t 500
#define flute_oscil40_return 501
#define flute_ENDVAR 502

#define TBL_flute_cyc 0
#define flute_ENDTBL 1

#define MAXENDTIME 1E+37

float endtime;
instr_line s_flute[1];
#define MAXSTATE 2


#define MAXVARSTATE 502
#define MAXTABLESTATE 1

/* ninstr: used for score, effects, */
/* and dynamic instruments          */

struct ninstr_types {

instr_line * iline; /* pointer to score line */
dstack v[MAXVARSTATE];     /* parameters & variables*/
tableinfo t[MAXTABLESTATE]; /* tables */

} ninstr[MAXSTATE];


/* global engine ends here */

extern void sigint_handler(int);

extern ENGINE_PTR_TYPE system_init(int argc, char **argv, float sample_rate);
extern void effects_init(ENGINE_PTR_DECLARE);
extern void main_initpass(ENGINE_PTR_DECLARE);
extern void shut_down(ENGINE_PTR_DECLARE);

extern void main_apass(ENGINE_PTR_DECLARE);
extern int  main_kpass(ENGINE_PTR_DECLARE);
extern void main_ipass(ENGINE_PTR_DECLARE);
extern void main_control(ENGINE_PTR_DECLARE);

float flute__sym_kline1(ENGINE_PTR_TYPE_COMMA struct ninstr_types *);
float flute__sym_kline2(ENGINE_PTR_TYPE_COMMA struct ninstr_types *);
float flute__sym_kline3(ENGINE_PTR_TYPE_COMMA struct ninstr_types *);
float flute__sym_kline4(ENGINE_PTR_TYPE_COMMA struct ninstr_types *);
float flute__sym_kline5(ENGINE_PTR_TYPE_COMMA struct ninstr_types *);
float flute__sym_kline6(ENGINE_PTR_TYPE_COMMA struct ninstr_types *);
float flute__sym_kline7(ENGINE_PTR_TYPE_COMMA struct ninstr_types *);
float flute__sym_kline8(ENGINE_PTR_TYPE_COMMA struct ninstr_types *);
float flute__sym_kline9(ENGINE_PTR_TYPE_COMMA struct ninstr_types *);
float flute__sym_kline10(ENGINE_PTR_TYPE_COMMA struct ninstr_types *);
float flute__sym_kline11(ENGINE_PTR_TYPE_COMMA struct ninstr_types *);
float flute__sym_kline12(ENGINE_PTR_TYPE_COMMA struct ninstr_types *);
float flute__sym_kline13(ENGINE_PTR_TYPE_COMMA struct ninstr_types *);
float flute__sym_kline14(ENGINE_PTR_TYPE_COMMA struct ninstr_types *);
float flute__sym_kline15(ENGINE_PTR_TYPE_COMMA struct ninstr_types *);
float flute__sym_kline16(ENGINE_PTR_TYPE_COMMA struct ninstr_types *);
float flute__sym_kline17(ENGINE_PTR_TYPE_COMMA struct ninstr_types *);
float flute__sym_kline18(ENGINE_PTR_TYPE_COMMA struct ninstr_types *);
float flute__sym_kline19(ENGINE_PTR_TYPE_COMMA struct ninstr_types *);
float flute__sym_kline20(ENGINE_PTR_TYPE_COMMA struct ninstr_types *);
float flute__sym_oscil21(ENGINE_PTR_TYPE_COMMA struct ninstr_types *);
float flute__sym_oscil22(ENGINE_PTR_TYPE_COMMA struct ninstr_types *);
float flute__sym_oscil23(ENGINE_PTR_TYPE_COMMA struct ninstr_types *);
float flute__sym_oscil24(ENGINE_PTR_TYPE_COMMA struct ninstr_types *);
float flute__sym_oscil25(ENGINE_PTR_TYPE_COMMA struct ninstr_types *);
float flute__sym_oscil26(ENGINE_PTR_TYPE_COMMA struct ninstr_types *);
float flute__sym_oscil27(ENGINE_PTR_TYPE_COMMA struct ninstr_types *);
float flute__sym_oscil28(ENGINE_PTR_TYPE_COMMA struct ninstr_types *);
float flute__sym_oscil29(ENGINE_PTR_TYPE_COMMA struct ninstr_types *);
float flute__sym_oscil30(ENGINE_PTR_TYPE_COMMA struct ninstr_types *);
float flute__sym_oscil31(ENGINE_PTR_TYPE_COMMA struct ninstr_types *);
float flute__sym_oscil32(ENGINE_PTR_TYPE_COMMA struct ninstr_types *);
float flute__sym_oscil33(ENGINE_PTR_TYPE_COMMA struct ninstr_types *);
float flute__sym_oscil34(ENGINE_PTR_TYPE_COMMA struct ninstr_types *);
float flute__sym_oscil35(ENGINE_PTR_TYPE_COMMA struct ninstr_types *);
float flute__sym_oscil36(ENGINE_PTR_TYPE_COMMA struct ninstr_types *);
float flute__sym_oscil37(ENGINE_PTR_TYPE_COMMA struct ninstr_types *);
float flute__sym_oscil38(ENGINE_PTR_TYPE_COMMA struct ninstr_types *);
float flute__sym_oscil39(ENGINE_PTR_TYPE_COMMA struct ninstr_types *);
float flute__sym_oscil40(ENGINE_PTR_TYPE_COMMA struct ninstr_types *);
void flute_ipass(ENGINE_PTR_TYPE_COMMA struct ninstr_types *);
void flute_kpass(ENGINE_PTR_TYPE_COMMA struct ninstr_types *);
void flute_apass(ENGINE_PTR_TYPE_COMMA struct ninstr_types *);



extern float table_global_cyc[];

csys_table_ptr csys_table_catalog[] = {
	129, table_global_cyc };

#define CSYS_TABLE_CATALOG_SIZE 1

instr_line s_flute_init[1] = {
{ 0.0F, MAXENDTIME, MAXENDTIME, MAXENDTIME,  0.0F, 0.0F, 0.0F,  2.31F, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, { 260.0F }, NULL }};


void engine_init(ENGINE_PTR_DECLARE_COMMA float sample_rate)
{
  EV(globaltune) = 440.0F;
  EV(invglobaltune) = 2.272727e-03F;
  EV(kbase) = 1;
  EV(kcycleidx) = 1;
  EV(pass) = IPASS;
  EV(tempo) = 60.0F;

  EV(scoremult) = EV(KTIME);
  EV(endtime) = 2.31F;

  memcpy(EV(s_flute), s_flute_init, sizeof EV(s_flute));
}


/* handles termination in case of error */

extern void epr(int linenum, char * filename, char * token, char * message);

/*
#    Sfront, a SAOL to C translator    
#    This file: Robust file I/O
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


/* robust replacement for fread() */

size_t rread(void * ptr, size_t size, size_t nmemb, FILE * stream)

{
  int recv;
  int len;
  int retry;
  char * c;

  /* fast path */

  if ((recv = fread(ptr, size, nmemb, stream)) == nmemb)
    return nmemb;

  /* error recovery */
     
  c = (char *) ptr;
  len = retry = 0;

  do 
    {
      if (++retry > IOERROR_RETRY)
	{
	  len = recv = 0;
	  break;
	}

      if (feof(stream))
	{
	  clearerr(stream);
	  break;
	}

      /* ANSI, not POSIX, so can't look for EINTR/EAGAIN  */
      /* Assume it was one of these and retry.            */

      clearerr(stream);
      len += recv;
      nmemb -= recv;

    }
  while ((recv = fread(&(c[len]), size, nmemb, stream)) != nmemb);

  return (len += recv);

}

/* robust replacement for fwrite() */

size_t rwrite(void * ptr, size_t size, size_t nmemb, FILE * stream)

{
  int recv;
  int len;
  int retry;
  char * c;

  /* fast path */

  if ((recv = fwrite(ptr, size, nmemb, stream)) == nmemb)
    return nmemb;

  /* error recovery */
     
  c = (char *) ptr;
  len = retry = 0;

  do 
    {
      if (++retry > IOERROR_RETRY)
	{
	  len = recv = 0;
	  break;
	}

      /* ANSI, not POSIX, so can't look for EINTR/EAGAIN  */
      /* Assume it was one of these and retry.            */

      len += recv;
      nmemb -= recv;

    }
  while ((recv = fwrite(&(c[len]), size, nmemb, stream)) != nmemb);

  return (len += recv);

}

#define ASYS_OUTDRIVER_WAV
#define ASYS_HASOUTPUT


/*
#    Sfront, a SAOL to C translator    
#    This file: WAV audio driver for sfront
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


/****************************************************************/
/****************************************************************/
/*             wav file audio driver for sfront                 */ 
/****************************************************************/
        
#include <stdio.h>
#include <string.h>

#if defined(ASYS_HASOUTPUT)

/* default name for output audio file */
#define ASYSO_DEFAULTNAME "output.wav"

/* global variables, must start with asyso_ */

FILE * asyso_fd;     /* output file pointer */
char * asyso_name;   /* name of file  */        
int asyso_srate;    /* sampling rate */
int asyso_channels; /* number of channels */
int asyso_size;     /* number of float samples _buf */
int asyso_nsamp;    /* total number of samples written */
int asyso_bps;      /* number of bytes per sample, 1-3 */
int asyso_doswap;   /* needs byteswap on write */
float * asyso_buf;   /* output floats from sfront */ 
unsigned char * asyso_cbuf;  /* output chars to file */
#endif

#if defined(ASYS_HASINPUT)

/* default name for input audio file */

#define ASYSI_DEFAULTNAME "input.wav"

/* only used for asysi_soundtypecheck */

#define ASYSI_MATCH  0
#define ASYSI_EOF 1
#define ASYSI_NOMATCH 2

/* global variables, must start with asysi_ */

FILE * asysi_fd;     /* input file pointer */
char * asysi_name;   /* name of file  */        
int asysi_srate;    /* sampling rate */
int asysi_channels; /* number of channels */
int asysi_bytes;     /* number of bytes in a buffer */
int asysi_nsamp;    /* total number of samples read */
int asysi_bps;      /* number of bytes per sample, 1-3 */
int asysi_doswap;   /* needs byteswap on read */
unsigned char * asysi_cbuf;  /* buffer of WAV file bytes */
float * asysi_buf;   /* float buffer for sfront */ 

#endif

#if defined(ASYS_HASOUTPUT)

/*********************************************************/
/*        writes next block of WAV/AIFF bytes            */
/*********************************************************/

int asyso_putbytes(unsigned char * c, int numbytes)

{
  if (rwrite(c, sizeof(char), numbytes, asyso_fd) != numbytes)
    return ASYS_ERROR;
  return ASYS_DONE;
}

/*********************************************************/
/*        writes unsigned int to a WAV files             */
/*********************************************************/

int asyso_putint(unsigned int val, int numbytes)

{
  unsigned char c[4];

  if (numbytes > 4)
    return ASYS_ERROR;
  switch (numbytes) {
  case 4:
    c[0] = (unsigned char) (val&0x000000FF);
    c[1] = (unsigned char)((val >> 8)&0x000000FF);
    c[2] = (unsigned char)((val >> 16)&0x000000FF);
    c[3] = (unsigned char)((val >> 24)&0x000000FF);
    return asyso_putbytes(c, 4);
  case 3:
    c[0] = (unsigned char) (val&0x000000FF);
    c[1] = (unsigned char)((val >> 8)&0x000000FF);
    c[2] = (unsigned char)((val >> 16)&0x000000FF);
    return asyso_putbytes(c, 3);
  case 2:
    c[0] = (unsigned char) (val&0x000000FF);
    c[1] = (unsigned char)((val >> 8)&0x000000FF);
    return asyso_putbytes(c, 2);
  case 1:
    c[0] = (unsigned char) (val&0x000000FF);
    return asyso_putbytes(c,1);
  default:
    return ASYS_ERROR;
  }

}

/****************************************************************/
/*        core routine for audio output setup                   */
/****************************************************************/

int asyso_setup(int srate, int ochannels, int osize, char * name)


{
  short swaptest = 0x0100;
  char * val;

  asyso_doswap = *((char *)&swaptest);
  if (name == NULL)
    val = ASYSO_DEFAULTNAME;
  else
    val = name;

  switch (ASYS_OUTFILE_WORDSIZE) {
  case ASYS_OUTFILE_WORDSIZE_8BIT: 
    asyso_bps = 1;
    break;
  case ASYS_OUTFILE_WORDSIZE_16BIT:
    asyso_bps = 2;
    break;
  case ASYS_OUTFILE_WORDSIZE_24BIT:
    asyso_bps = 3;
    break;
  }

  asyso_name = strcpy((char *) calloc((strlen(val)+1),sizeof(char)), val);
  asyso_fd = fopen(asyso_name,"wb");
  if (asyso_fd == NULL)
    return ASYS_ERROR;

  /* preamble for wav file */

  asyso_putbytes((unsigned char *) "RIFF",4);
  asyso_putint(0,4);       /* patched later */
  asyso_putbytes((unsigned char *) "WAVEfmt ",8);
  asyso_putint(16,4);
  asyso_putint(1,2);                          /* PCM  */
  asyso_putint(ochannels,2);                  /* number of channels */
  asyso_putint(srate,4);                      /* srate */
  asyso_putint(srate*ochannels*asyso_bps, 4); /* bytes/sec */
  asyso_putint(ochannels*asyso_bps, 2);       /* block align */
  asyso_putint(8*asyso_bps, 2);               /* bits per sample */
  asyso_putbytes((unsigned char *) "data",4);
  asyso_putint(0,4);                          /* patched later */

  asyso_srate = srate;
  asyso_channels = ochannels;
  asyso_size = osize;
  asyso_nsamp = 0;
  asyso_cbuf = (unsigned char *) malloc(osize*asyso_bps);

  if (asyso_cbuf == NULL)
    {
      fprintf(stderr, "Can't allocate WAV byte output buffer (%s).\n",
	      strerror(errno));
      return ASYS_ERROR;
    }

  asyso_buf = (float *)calloc(osize, sizeof(float));

  if (asyso_buf == NULL)
    {
      fprintf(stderr, "Can't allocate WAV float output buffer (%s).\n",
	      strerror(errno));
      return ASYS_ERROR;
    }

  return ASYS_DONE;
}

#endif

#if defined(ASYS_HASINPUT)

/*********************************************************/
/*            gets next block of WAV bytes               */
/*********************************************************/

int asysi_getbytes(unsigned char * c, int numbytes)

{
  if ((int)rread(c, sizeof(char), numbytes, asysi_fd) != numbytes)
    return ASYS_ERROR;
  return ASYS_DONE;
}

/*********************************************************/
/*        flushes next block of WAV bytes                */
/*********************************************************/

int asysi_flushbytes(int numbytes)

{
  unsigned char c;

  while (numbytes > 0)
    {
      if (rread(&c, sizeof(char), 1, asysi_fd) != 1)
	return ASYS_ERROR;
      numbytes--;
    }
  return ASYS_DONE;

}

/*********************************************************/
/*     converts byte stream to an unsigned int           */
/*********************************************************/

int asysi_getint(int numbytes, unsigned int * ret)

{
  unsigned char c[4];

  if (numbytes > 4)
    return ASYS_ERROR;
  if (ASYS_DONE != asysi_getbytes(&c[0],numbytes))
    return ASYS_ERROR;
  switch (numbytes) {
  case 4:
    *ret  =  (unsigned int)c[0];
    *ret |=  (unsigned int)c[1] << 8;
    *ret |=  (unsigned int)c[2] << 16;
    *ret |=  (unsigned int)c[3] << 24;
    return ASYS_DONE;
  case 3:
    *ret  =  (unsigned int)c[0];
    *ret |=  (unsigned int)c[1] << 8;
    *ret |=  (unsigned int)c[2] << 16;
    return ASYS_DONE;
  case 2:
    *ret  =  (unsigned int)c[0];
    *ret |=  (unsigned int)c[1] << 8;
    return ASYS_DONE;
  case 1:
    *ret = (unsigned int)c[0];
    return ASYS_DONE;
  default:
    return ASYS_ERROR;
  }

}
  
/***********************************************************/
/*  checks byte stream for AIFF/WAV cookie --              */
/***********************************************************/

int asysi_soundtypecheck(char * d)

{
  char c[4];

  if (rread(c, sizeof(char), 4, asysi_fd) != 4)
    return ASYSI_EOF;
  if (strncmp(c,d,4))
    return ASYSI_NOMATCH;
  return ASYSI_MATCH;
}
  
/****************************************************************/
/*        core routine for audio input setup                   */
/****************************************************************/

int asysi_setup(int srate, int ichannels, int isize, char * name)


{
  short swaptest = 0x0100;
  unsigned int i, cookie;
  int len;
  char * val;

  asysi_doswap = *((char *)&swaptest);

  if (name == NULL)
    val = ASYSI_DEFAULTNAME;
  else
    val = name;
  asysi_name = strcpy((char *) calloc((strlen(val)+1),sizeof(char)), val);
  asysi_fd = fopen(asysi_name,"rb");
  if (asysi_fd == NULL)
    return ASYS_ERROR;

  if (asysi_soundtypecheck("RIFF")!= ASYSI_MATCH)
    return ASYS_ERROR;
  if (asysi_flushbytes(4)!= ASYS_DONE)
    return ASYS_ERROR;
  if (asysi_soundtypecheck("WAVE")!= ASYSI_MATCH)
    return ASYS_ERROR;
  while ((cookie = asysi_soundtypecheck("fmt "))!=ASYSI_MATCH)
    {
      if (cookie == ASYSI_EOF)
	return ASYS_ERROR;
      if (asysi_getint(4, &i) != ASYS_DONE)
	return ASYS_ERROR;
      if (asysi_flushbytes(i + (i % 2))!= ASYS_DONE)
	return ASYS_ERROR;
    }
  if (asysi_getint(4, &i) != ASYS_DONE)
    return ASYS_ERROR;
  len = i;
  if ((len -= 16) < 0)
    return ASYS_ERROR;
  if (asysi_getint(2, &i) != ASYS_DONE)
    return ASYS_ERROR;
  if (i != 1)
    {
      fprintf(stderr,"Error: Can only handle PCM WAV files\n");
      return ASYS_ERROR;
    }
  if (asysi_getint(2, &i) != ASYS_DONE)
    return ASYS_ERROR;
  if (i != ichannels)
    {
      fprintf(stderr,"Error: Inchannels doesn't match WAV file\n");
      return ASYS_ERROR;
    }
  if (asysi_getint(4, &i) != ASYS_DONE)
    return ASYS_ERROR;
  if (srate != i)
    fprintf(stderr,"Warning: SAOL srate %i mismatches WAV file srate %i\n",
	    srate,i);
  asysi_flushbytes(6);
  if (asysi_getint(2, &i) != ASYS_DONE)
    return ASYS_ERROR;
  if ((i < 8) || (i > 24))
    {
      fprintf(stderr,"Error: Can't handle %i bit data\n",i);
      return ASYS_ERROR;
    }
  asysi_bps = i/8;
  asysi_flushbytes(len + (len % 2));

  while ((cookie = asysi_soundtypecheck("data"))!=ASYSI_MATCH)
    {
      if (cookie == ASYSI_EOF)
	return ASYS_ERROR;
      if (asysi_getint(4, &i) != ASYS_DONE)
	return ASYS_ERROR;
      if (asysi_flushbytes(i + (i % 2))!= ASYS_DONE)
	return ASYS_ERROR;
    }
  if (asysi_getint(4, &i) != ASYS_DONE)
    return ASYS_ERROR;

  asysi_nsamp = i/asysi_bps;
  asysi_srate = srate;
  asysi_channels = ichannels;
  asysi_bytes = isize*asysi_bps;
  asysi_cbuf = (unsigned char *) malloc(asysi_bytes);

  if (asysi_cbuf == NULL)
    {
      fprintf(stderr, "Can't allocate WAV input byte buffer (%s).\n",
	      strerror(errno));
      return ASYS_ERROR;
    }

  asysi_buf = (float *) malloc(sizeof(float)*isize);

  if (asysi_buf == NULL)
    {
      fprintf(stderr, "Can't allocate WAV input float buffer (%s).\n",
	      strerror(errno));
      return ASYS_ERROR;
    }

  return ASYS_DONE;
}

#endif

#if (defined(ASYS_HASOUTPUT) && !defined(ASYS_HASINPUT))

/****************************************************************/
/*        sets up audio output for a given srate/channels       */
/****************************************************************/

int asys_osetup(int srate, int ochannels, int osample, 
		char * oname, int toption)

{
  return asyso_setup(srate, ochannels, ASYS_OCHAN*EV(ACYCLE), oname);
}

#endif


#if (!defined(ASYS_HASOUTPUT) && defined(ASYS_HASINPUT))

/****************************************************************/
/*        sets up audio input for a given srate/channels       */
/****************************************************************/

int asys_isetup(int srate, int ichannels, int isample, 
		char * iname, int toption)

{
  return asysi_setup(srate, ichannels, ASYS_ICHAN*EV(ACYCLE), iname);
}

#endif


#if (defined(ASYS_HASOUTPUT) && defined(ASYS_HASINPUT))

/****************************************************************/
/*   sets up audio input and output for a given srate/channels  */
/****************************************************************/

int asys_iosetup(int srate, int ichannels, int ochannels,
		 int isample, int osample, 
		 char * iname, char * oname, int toption)

{

  if (asysi_setup(srate, ichannels, ASYS_ICHAN*EV(ACYCLE), iname) != ASYS_DONE)
    return ASYS_ERROR;
  return asyso_setup(srate, ochannels, ASYS_OCHAN*EV(ACYCLE), oname);

}

#endif

#if defined(ASYS_HASOUTPUT)

/****************************************************************/
/*             shuts down audio output system                   */
/****************************************************************/

void asyso_shutdown(void)

{

  fseek(asyso_fd, 4, SEEK_SET);
  asyso_putint(asyso_bps*(unsigned int)asyso_nsamp+36,4);
  fseek(asyso_fd, 32, SEEK_CUR);
  asyso_putint(asyso_bps*(unsigned int)asyso_nsamp,4);
  fclose(asyso_fd);

}

#endif

#if defined(ASYS_HASINPUT)

/****************************************************************/
/*               shuts down audio input system                  */
/****************************************************************/

void asysi_shutdown(void)

{

  fclose(asysi_fd);
}

#endif


#if (defined(ASYS_HASOUTPUT)&&(!defined(ASYS_HASINPUT)))

/****************************************************************/
/*                    shuts down audio output                   */
/****************************************************************/

void asys_oshutdown(void)

{
  asyso_shutdown();
}

#endif

#if (!defined(ASYS_HASOUTPUT)&&(defined(ASYS_HASINPUT)))

/****************************************************************/
/*              shuts down audio input device                   */
/****************************************************************/

void asys_ishutdown(void)

{
  asysi_shutdown();
}

#endif

#if (defined(ASYS_HASOUTPUT)&&(defined(ASYS_HASINPUT)))

/****************************************************************/
/*              shuts down audio input and output device        */
/****************************************************************/

void asys_ioshutdown(void)

{
  asysi_shutdown();
  asyso_shutdown();
}

#endif


#if defined(ASYS_HASOUTPUT)


/****************************************************************/
/*        creates buffer, and generates starting silence        */
/****************************************************************/

int asys_preamble(ASYS_OTYPE * asys_obuf[], int * osize)

{
  *asys_obuf = asyso_buf;
  *osize = asyso_size;
  return ASYS_DONE;
}

/****************************************************************/
/*               sends one frame of audio to output             */
/****************************************************************/

int asys_putbuf(ASYS_OTYPE * asys_obuf[], int * osize)

{
  float * buf = *asys_obuf;
  float fval;
  int val;
  int i = 0;
  int j = 0;

  switch (asyso_bps) {
  case 3:
    while (i < *osize)
      {
	fval = ((float)(pow(2, 23) - 1))*buf[i++];
	val = (int)((fval >= 0.0F) ? (fval + 0.5F) : (fval - 0.5F));
	asyso_cbuf[j++] = (unsigned char) (val & 0x000000FF);
	asyso_cbuf[j++] = (unsigned char)((val >> 8) & 0x000000FF);
	asyso_cbuf[j++] = (unsigned char)((val >> 16) & 0x000000FF);
      }
    break;
  case 2:
    while (i < *osize)
      {
	fval = ((float)(pow(2, 15) - 1))*buf[i++];
	val = (int)((fval >= 0.0F) ? (fval + 0.5F) : (fval - 0.5F));
	asyso_cbuf[j++] = (unsigned char) (val & 0x000000FF);
	asyso_cbuf[j++] = (unsigned char)((val >> 8) & 0x000000FF);
      }
    break;
  case 1:
    while (i < *osize)
      {
	fval = ((float)(pow(2, 7) - 1))*buf[i++];
	asyso_cbuf[j++] = (unsigned char)
	  (128 + ((char)((fval >= 0.0F) ? (fval + 0.5F) : (fval - 0.5F))));
      }
    break;
  }
  
  if (rwrite(asyso_cbuf, sizeof(char), j, asyso_fd) != j)
    return ASYS_ERROR;

  asyso_nsamp += *osize;
  *osize = asyso_size;
  return ASYS_DONE;
}

#endif


#if defined(ASYS_HASINPUT)

/****************************************************************/
/*               get one frame of audio from input              */
/****************************************************************/

int asys_getbuf(ASYS_ITYPE * asys_ibuf[], int * isize)

{
  int i = 0;
  int j = 0;

  if (*asys_ibuf == NULL)
    *asys_ibuf = asysi_buf;
  
  if (asysi_nsamp <= 0)
    {
      *isize = 0;
      return ASYS_DONE;
    }

  *isize = (int)rread(asysi_cbuf, sizeof(unsigned char), asysi_bytes, asysi_fd);

  switch (asysi_bps) {
  case 1:                              /* 8-bit  */
    while (i < *isize)
      {
	asysi_buf[i] = ((float)pow(2, -7))*(((short) asysi_cbuf[i]) - 128);
	i++;
      }
    break;
  case 2:                              /* 9-16 bit */
    *isize = (*isize) / 2;
    while (i < *isize)
      {
	asysi_buf[i] = ((float)pow(2, -15))*((int)(asysi_cbuf[j]) + 
				    (((int)((char)(asysi_cbuf[j+1]))) << 8)); 
	i++;
	j += 2;
      }
    break;
  case 3:                            /* 17-24 bit */
    *isize = (*isize) / 3;
    while (i < *isize)
      {
	asysi_buf[i] = ((float)pow(2, -23))*((int)(asysi_cbuf[j]) + 
				    (((int)(asysi_cbuf[j+1])) << 8) + 
				    (((int)((char) asysi_cbuf[j+2])) << 16));
	i++; 
	j += 3;
      }
    break;
  } 

  asysi_nsamp -= *isize;
  return ASYS_DONE;
}

#endif


#undef ASYS_HASOUTPUT


float ksync() { return 0.0F; }


void ksyncinit() { }



/* handles termination in case of error */

void epr(int linenum, char * filename, char * token, char * message)

{

  fprintf(stderr, "\nRuntime Error.\n");
  if (filename != NULL)
    fprintf(stderr, "Location: File %s near line %i.\n",filename, linenum);
  if (token != NULL)
    fprintf(stderr, "While executing: %s.\n",token);
  if (message != NULL)
    fprintf(stderr, "Potential problem: %s.\n",message);
  fprintf(stderr, "Exiting program.\n\n");
  exit(-1);

}



float flute__sym_kline1(ENGINE_PTR_DECLARE_COMMA struct ninstr_types * nstate)
{
   float va_dur2;
   float va_x3;
   float va_dur3;
   float va_x4;
   float va_dur4;
   float va_x5;
   float va_dur5;
   float va_x6;
   float va_dur6;
   float va_x7;
   float va_dur7;
   float va_x8;
   float va_dur8;
   float va_x9;
   float va_dur9;
   float va_x10;
   float va_dur10;
   float va_x11;
   float va_dur11;
   float va_x12;
   float va_dur12;
   float va_x13;
   float va_dur13;
   float va_x14;
   float va_dur14;
   float va_x15;
   float va_dur15;
   float va_x16;
   float va_dur16;
   float va_x17;
   float va_dur17;
   float va_x18;
   float va_dur18;
   float va_x19;
   float va_dur19;
   float va_x20;
   float x2;
   float dur1;
   float x1;
   float ret;

   x1 =  0.0F ;
   dur1 =  0.121154F ;
   x2 =  0.126821F ;
   va_dur2 =  0.107692F  ;
   va_x3 =  0.185633F  ;
   va_dur3 =  0.096154F  ;
   va_x4 =  0.211073F  ;
   va_dur4 =  0.059615F  ;
   va_x5 =  0.255557F  ;
   va_dur5 =  0.019231F  ;
   va_x6 =  0.278097F  ;
   va_dur6 =  0.019231F  ;
   va_x7 =  0.292906F  ;
   va_dur7 =  0.040385F  ;
   va_x8 =  0.270017F  ;
   va_dur8 =  0.009615F  ;
   va_x9 =  0.254216F  ;
   va_dur9 =  0.015385F  ;
   va_x10 =  0.237838F  ;
   va_dur10 =  0.015385F  ;
   va_x11 =  0.229367F  ;
   va_dur11 =  0.026923F  ;
   va_x12 =  0.232896F  ;
   va_dur12 =  0.076923F  ;
   va_x13 =  0.313022F  ;
   va_dur13 =  0.026923F  ;
   va_x14 =  0.274067F  ;
   va_dur14 =  0.017308F  ;
   va_x15 =  0.217905F  ;
   va_dur15 =  0.009615F  ;
   va_x16 =  0.182745F  ;
   va_dur16 =  0.059615F  ;
   va_x17 =  0.117718F  ;
   va_dur17 =  0.175000F  ;
   va_x18 =  0.107359F  ;
   va_dur18 =  0.344231F  ;
   va_x19 =  0.098393F  ;
   va_dur19 =  1.065385F  ;
   va_x20 =  0.002078F  ;
   ret = 0.0F;
   if (NVI(flute_kline1_first)>0)
   {
      NV(flute_kline1_t) += EV(KTIME);
      ret = (NV(flute_kline1_outT) += NV(flute_kline1_addK));
      if (NV(flute_kline1_t) > NV(flute_kline1_cdur))
       {
        while (NV(flute_kline1_t) > NV(flute_kline1_cdur))
         {
           NV(flute_kline1_t) -= NV(flute_kline1_cdur);
           switch(NVI(flute_kline1_first))
      {
         case 1:
         NV(flute_kline1_cdur) = va_dur2;
           NV(flute_kline1_clp) = NV(flute_kline1_crp);
           NV(flute_kline1_crp) = va_x3;
           break;
         case 2:
         NV(flute_kline1_cdur) = va_dur3;
           NV(flute_kline1_clp) = NV(flute_kline1_crp);
           NV(flute_kline1_crp) = va_x4;
           break;
         case 3:
         NV(flute_kline1_cdur) = va_dur4;
           NV(flute_kline1_clp) = NV(flute_kline1_crp);
           NV(flute_kline1_crp) = va_x5;
           break;
         case 4:
         NV(flute_kline1_cdur) = va_dur5;
           NV(flute_kline1_clp) = NV(flute_kline1_crp);
           NV(flute_kline1_crp) = va_x6;
           break;
         case 5:
         NV(flute_kline1_cdur) = va_dur6;
           NV(flute_kline1_clp) = NV(flute_kline1_crp);
           NV(flute_kline1_crp) = va_x7;
           break;
         case 6:
         NV(flute_kline1_cdur) = va_dur7;
           NV(flute_kline1_clp) = NV(flute_kline1_crp);
           NV(flute_kline1_crp) = va_x8;
           break;
         case 7:
         NV(flute_kline1_cdur) = va_dur8;
           NV(flute_kline1_clp) = NV(flute_kline1_crp);
           NV(flute_kline1_crp) = va_x9;
           break;
         case 8:
         NV(flute_kline1_cdur) = va_dur9;
           NV(flute_kline1_clp) = NV(flute_kline1_crp);
           NV(flute_kline1_crp) = va_x10;
           break;
         case 9:
         NV(flute_kline1_cdur) = va_dur10;
           NV(flute_kline1_clp) = NV(flute_kline1_crp);
           NV(flute_kline1_crp) = va_x11;
           break;
         case 10:
         NV(flute_kline1_cdur) = va_dur11;
           NV(flute_kline1_clp) = NV(flute_kline1_crp);
           NV(flute_kline1_crp) = va_x12;
           break;
         case 11:
         NV(flute_kline1_cdur) = va_dur12;
           NV(flute_kline1_clp) = NV(flute_kline1_crp);
           NV(flute_kline1_crp) = va_x13;
           break;
         case 12:
         NV(flute_kline1_cdur) = va_dur13;
           NV(flute_kline1_clp) = NV(flute_kline1_crp);
           NV(flute_kline1_crp) = va_x14;
           break;
         case 13:
         NV(flute_kline1_cdur) = va_dur14;
           NV(flute_kline1_clp) = NV(flute_kline1_crp);
           NV(flute_kline1_crp) = va_x15;
           break;
         case 14:
         NV(flute_kline1_cdur) = va_dur15;
           NV(flute_kline1_clp) = NV(flute_kline1_crp);
           NV(flute_kline1_crp) = va_x16;
           break;
         case 15:
         NV(flute_kline1_cdur) = va_dur16;
           NV(flute_kline1_clp) = NV(flute_kline1_crp);
           NV(flute_kline1_crp) = va_x17;
           break;
         case 16:
         NV(flute_kline1_cdur) = va_dur17;
           NV(flute_kline1_clp) = NV(flute_kline1_crp);
           NV(flute_kline1_crp) = va_x18;
           break;
         case 17:
         NV(flute_kline1_cdur) = va_dur18;
           NV(flute_kline1_clp) = NV(flute_kline1_crp);
           NV(flute_kline1_crp) = va_x19;
           break;
         case 18:
         NV(flute_kline1_cdur) = va_dur19;
           NV(flute_kline1_clp) = NV(flute_kline1_crp);
           NV(flute_kline1_crp) = va_x20;
           break;
           default:
           NVI(flute_kline1_first) = -100;
           NV(flute_kline1_cdur) = NV(flute_kline1_t) + 10000.0F;
           break;
           }
         NVI(flute_kline1_first)++;
        }
        NV(flute_kline1_mult)=(NV(flute_kline1_crp) - NV(flute_kline1_clp))/NV(flute_kline1_cdur);
        ret = NV(flute_kline1_outT) = NV(flute_kline1_clp)+NV(flute_kline1_mult)*NV(flute_kline1_t);
        NV(flute_kline1_addK) = NV(flute_kline1_mult)*EV(KTIME);
        if (NVI(flute_kline1_first)<0)
          ret = 0.0F;
      }
   }
   if (NVI(flute_kline1_first)==0)
     {
       NVI(flute_kline1_first) = 1;
       ret = NV(flute_kline1_outT) = NV(flute_kline1_clp) = x1;
       NV(flute_kline1_crp) = x2;
       NV(flute_kline1_cdur) = dur1;
       if (dur1 > 0.0F)
         NV(flute_kline1_addK) = EV(KTIME)*((x2 - x1)/dur1);
     }
   return((NV(flute_kline1_return) = ret));

}



float flute__sym_kline2(ENGINE_PTR_DECLARE_COMMA struct ninstr_types * nstate)
{
   float va_dur2;
   float va_x3;
   float va_dur3;
   float va_x4;
   float va_dur4;
   float va_x5;
   float va_dur5;
   float va_x6;
   float va_dur6;
   float va_x7;
   float va_dur7;
   float va_x8;
   float va_dur8;
   float va_x9;
   float va_dur9;
   float va_x10;
   float va_dur10;
   float va_x11;
   float va_dur11;
   float va_x12;
   float va_dur12;
   float va_x13;
   float va_dur13;
   float va_x14;
   float va_dur14;
   float va_x15;
   float va_dur15;
   float va_x16;
   float va_dur16;
   float va_x17;
   float va_dur17;
   float va_x18;
   float va_dur18;
   float va_x19;
   float va_dur19;
   float va_x20;
   float x2;
   float dur1;
   float x1;
   float ret;

   x1 =  0.0F ;
   dur1 =  0.115385F ;
   x2 =  0.109963F ;
   va_dur2 =  0.215385F  ;
   va_x3 =  0.240874F  ;
   va_dur3 =  0.086538F  ;
   va_x4 =  0.328835F  ;
   va_dur4 =  0.044231F  ;
   va_x5 =  0.314505F  ;
   va_dur5 =  0.011538F  ;
   va_x6 =  0.294773F  ;
   va_dur6 =  0.013462F  ;
   va_x7 =  0.278730F  ;
   va_dur7 =  0.015385F  ;
   va_x8 =  0.267327F  ;
   va_dur8 =  0.019231F  ;
   va_x9 =  0.264604F  ;
   va_dur9 =  0.076923F  ;
   va_x10 =  0.378284F  ;
   va_dur10 =  0.011538F  ;
   va_x11 =  0.377367F  ;
   va_dur11 =  0.019231F  ;
   va_x12 =  0.349473F  ;
   va_dur12 =  0.025000F  ;
   va_x13 =  0.255729F  ;
   va_dur13 =  0.009615F  ;
   va_x14 =  0.230729F  ;
   va_dur14 =  0.017308F  ;
   va_x15 =  0.202978F  ;
   va_dur15 =  0.013462F  ;
   va_x16 =  0.186313F  ;
   va_dur16 =  0.036538F  ;
   va_x17 =  0.232262F  ;
   va_dur17 =  0.738462F  ;
   va_x18 =  0.241885F  ;
   va_dur18 =  0.544231F  ;
   va_x19 =  0.194498F  ;
   va_dur19 =  0.292308F  ;
   va_x20 =  0.003575F  ;
   ret = 0.0F;
   if (NVI(flute_kline2_first)>0)
   {
      NV(flute_kline2_t) += EV(KTIME);
      ret = (NV(flute_kline2_outT) += NV(flute_kline2_addK));
      if (NV(flute_kline2_t) > NV(flute_kline2_cdur))
       {
        while (NV(flute_kline2_t) > NV(flute_kline2_cdur))
         {
           NV(flute_kline2_t) -= NV(flute_kline2_cdur);
           switch(NVI(flute_kline2_first))
      {
         case 1:
         NV(flute_kline2_cdur) = va_dur2;
           NV(flute_kline2_clp) = NV(flute_kline2_crp);
           NV(flute_kline2_crp) = va_x3;
           break;
         case 2:
         NV(flute_kline2_cdur) = va_dur3;
           NV(flute_kline2_clp) = NV(flute_kline2_crp);
           NV(flute_kline2_crp) = va_x4;
           break;
         case 3:
         NV(flute_kline2_cdur) = va_dur4;
           NV(flute_kline2_clp) = NV(flute_kline2_crp);
           NV(flute_kline2_crp) = va_x5;
           break;
         case 4:
         NV(flute_kline2_cdur) = va_dur5;
           NV(flute_kline2_clp) = NV(flute_kline2_crp);
           NV(flute_kline2_crp) = va_x6;
           break;
         case 5:
         NV(flute_kline2_cdur) = va_dur6;
           NV(flute_kline2_clp) = NV(flute_kline2_crp);
           NV(flute_kline2_crp) = va_x7;
           break;
         case 6:
         NV(flute_kline2_cdur) = va_dur7;
           NV(flute_kline2_clp) = NV(flute_kline2_crp);
           NV(flute_kline2_crp) = va_x8;
           break;
         case 7:
         NV(flute_kline2_cdur) = va_dur8;
           NV(flute_kline2_clp) = NV(flute_kline2_crp);
           NV(flute_kline2_crp) = va_x9;
           break;
         case 8:
         NV(flute_kline2_cdur) = va_dur9;
           NV(flute_kline2_clp) = NV(flute_kline2_crp);
           NV(flute_kline2_crp) = va_x10;
           break;
         case 9:
         NV(flute_kline2_cdur) = va_dur10;
           NV(flute_kline2_clp) = NV(flute_kline2_crp);
           NV(flute_kline2_crp) = va_x11;
           break;
         case 10:
         NV(flute_kline2_cdur) = va_dur11;
           NV(flute_kline2_clp) = NV(flute_kline2_crp);
           NV(flute_kline2_crp) = va_x12;
           break;
         case 11:
         NV(flute_kline2_cdur) = va_dur12;
           NV(flute_kline2_clp) = NV(flute_kline2_crp);
           NV(flute_kline2_crp) = va_x13;
           break;
         case 12:
         NV(flute_kline2_cdur) = va_dur13;
           NV(flute_kline2_clp) = NV(flute_kline2_crp);
           NV(flute_kline2_crp) = va_x14;
           break;
         case 13:
         NV(flute_kline2_cdur) = va_dur14;
           NV(flute_kline2_clp) = NV(flute_kline2_crp);
           NV(flute_kline2_crp) = va_x15;
           break;
         case 14:
         NV(flute_kline2_cdur) = va_dur15;
           NV(flute_kline2_clp) = NV(flute_kline2_crp);
           NV(flute_kline2_crp) = va_x16;
           break;
         case 15:
         NV(flute_kline2_cdur) = va_dur16;
           NV(flute_kline2_clp) = NV(flute_kline2_crp);
           NV(flute_kline2_crp) = va_x17;
           break;
         case 16:
         NV(flute_kline2_cdur) = va_dur17;
           NV(flute_kline2_clp) = NV(flute_kline2_crp);
           NV(flute_kline2_crp) = va_x18;
           break;
         case 17:
         NV(flute_kline2_cdur) = va_dur18;
           NV(flute_kline2_clp) = NV(flute_kline2_crp);
           NV(flute_kline2_crp) = va_x19;
           break;
         case 18:
         NV(flute_kline2_cdur) = va_dur19;
           NV(flute_kline2_clp) = NV(flute_kline2_crp);
           NV(flute_kline2_crp) = va_x20;
           break;
           default:
           NVI(flute_kline2_first) = -100;
           NV(flute_kline2_cdur) = NV(flute_kline2_t) + 10000.0F;
           break;
           }
         NVI(flute_kline2_first)++;
        }
        NV(flute_kline2_mult)=(NV(flute_kline2_crp) - NV(flute_kline2_clp))/NV(flute_kline2_cdur);
        ret = NV(flute_kline2_outT) = NV(flute_kline2_clp)+NV(flute_kline2_mult)*NV(flute_kline2_t);
        NV(flute_kline2_addK) = NV(flute_kline2_mult)*EV(KTIME);
        if (NVI(flute_kline2_first)<0)
          ret = 0.0F;
      }
   }
   if (NVI(flute_kline2_first)==0)
     {
       NVI(flute_kline2_first) = 1;
       ret = NV(flute_kline2_outT) = NV(flute_kline2_clp) = x1;
       NV(flute_kline2_crp) = x2;
       NV(flute_kline2_cdur) = dur1;
       if (dur1 > 0.0F)
         NV(flute_kline2_addK) = EV(KTIME)*((x2 - x1)/dur1);
     }
   return((NV(flute_kline2_return) = ret));

}



float flute__sym_kline3(ENGINE_PTR_DECLARE_COMMA struct ninstr_types * nstate)
{
   float va_dur2;
   float va_x3;
   float va_dur3;
   float va_x4;
   float va_dur4;
   float va_x5;
   float va_dur5;
   float va_x6;
   float va_dur6;
   float va_x7;
   float va_dur7;
   float va_x8;
   float va_dur8;
   float va_x9;
   float va_dur9;
   float va_x10;
   float va_dur10;
   float va_x11;
   float va_dur11;
   float va_x12;
   float va_dur12;
   float va_x13;
   float va_dur13;
   float va_x14;
   float va_dur14;
   float va_x15;
   float va_dur15;
   float va_x16;
   float va_dur16;
   float va_x17;
   float va_dur17;
   float va_x18;
   float va_dur18;
   float va_x19;
   float va_dur19;
   float va_x20;
   float x2;
   float dur1;
   float x1;
   float ret;

   x1 =  0.0F ;
   dur1 =  0.082692F ;
   x2 =  0.063851F ;
   va_dur2 =  0.175000F  ;
   va_x3 =  0.111520F  ;
   va_dur3 =  0.065385F  ;
   va_x4 =  0.107080F  ;
   va_dur4 =  0.082692F  ;
   va_x5 =  0.149256F  ;
   va_dur5 =  0.055769F  ;
   va_x6 =  0.129666F  ;
   va_dur6 =  0.026923F  ;
   va_x7 =  0.110505F  ;
   va_dur7 =  0.075000F  ;
   va_x8 =  0.135601F  ;
   va_dur8 =  0.061538F  ;
   va_x9 =  0.141322F  ;
   va_dur9 =  0.017308F  ;
   va_x10 =  0.116464F  ;
   va_dur10 =  0.026923F  ;
   va_x11 =  0.105707F  ;
   va_dur11 =  0.019231F  ;
   va_x12 =  0.094559F  ;
   va_dur12 =  0.023077F  ;
   va_x13 =  0.093558F  ;
   va_dur13 =  0.059615F  ;
   va_x14 =  0.167633F  ;
   va_dur14 =  0.063462F  ;
   va_x15 =  0.092797F  ;
   va_dur15 =  0.019231F  ;
   va_x16 =  0.078699F  ;
   va_dur16 =  0.057692F  ;
   va_x17 =  0.120449F  ;
   va_dur17 =  0.378846F  ;
   va_x18 =  0.129190F  ;
   va_dur18 =  0.871154F  ;
   va_x19 =  0.082799F  ;
   va_dur19 =  0.144231F  ;
   va_x20 =  0.001784F  ;
   ret = 0.0F;
   if (NVI(flute_kline3_first)>0)
   {
      NV(flute_kline3_t) += EV(KTIME);
      ret = (NV(flute_kline3_outT) += NV(flute_kline3_addK));
      if (NV(flute_kline3_t) > NV(flute_kline3_cdur))
       {
        while (NV(flute_kline3_t) > NV(flute_kline3_cdur))
         {
           NV(flute_kline3_t) -= NV(flute_kline3_cdur);
           switch(NVI(flute_kline3_first))
      {
         case 1:
         NV(flute_kline3_cdur) = va_dur2;
           NV(flute_kline3_clp) = NV(flute_kline3_crp);
           NV(flute_kline3_crp) = va_x3;
           break;
         case 2:
         NV(flute_kline3_cdur) = va_dur3;
           NV(flute_kline3_clp) = NV(flute_kline3_crp);
           NV(flute_kline3_crp) = va_x4;
           break;
         case 3:
         NV(flute_kline3_cdur) = va_dur4;
           NV(flute_kline3_clp) = NV(flute_kline3_crp);
           NV(flute_kline3_crp) = va_x5;
           break;
         case 4:
         NV(flute_kline3_cdur) = va_dur5;
           NV(flute_kline3_clp) = NV(flute_kline3_crp);
           NV(flute_kline3_crp) = va_x6;
           break;
         case 5:
         NV(flute_kline3_cdur) = va_dur6;
           NV(flute_kline3_clp) = NV(flute_kline3_crp);
           NV(flute_kline3_crp) = va_x7;
           break;
         case 6:
         NV(flute_kline3_cdur) = va_dur7;
           NV(flute_kline3_clp) = NV(flute_kline3_crp);
           NV(flute_kline3_crp) = va_x8;
           break;
         case 7:
         NV(flute_kline3_cdur) = va_dur8;
           NV(flute_kline3_clp) = NV(flute_kline3_crp);
           NV(flute_kline3_crp) = va_x9;
           break;
         case 8:
         NV(flute_kline3_cdur) = va_dur9;
           NV(flute_kline3_clp) = NV(flute_kline3_crp);
           NV(flute_kline3_crp) = va_x10;
           break;
         case 9:
         NV(flute_kline3_cdur) = va_dur10;
           NV(flute_kline3_clp) = NV(flute_kline3_crp);
           NV(flute_kline3_crp) = va_x11;
           break;
         case 10:
         NV(flute_kline3_cdur) = va_dur11;
           NV(flute_kline3_clp) = NV(flute_kline3_crp);
           NV(flute_kline3_crp) = va_x12;
           break;
         case 11:
         NV(flute_kline3_cdur) = va_dur12;
           NV(flute_kline3_clp) = NV(flute_kline3_crp);
           NV(flute_kline3_crp) = va_x13;
           break;
         case 12:
         NV(flute_kline3_cdur) = va_dur13;
           NV(flute_kline3_clp) = NV(flute_kline3_crp);
           NV(flute_kline3_crp) = va_x14;
           break;
         case 13:
         NV(flute_kline3_cdur) = va_dur14;
           NV(flute_kline3_clp) = NV(flute_kline3_crp);
           NV(flute_kline3_crp) = va_x15;
           break;
         case 14:
         NV(flute_kline3_cdur) = va_dur15;
           NV(flute_kline3_clp) = NV(flute_kline3_crp);
           NV(flute_kline3_crp) = va_x16;
           break;
         case 15:
         NV(flute_kline3_cdur) = va_dur16;
           NV(flute_kline3_clp) = NV(flute_kline3_crp);
           NV(flute_kline3_crp) = va_x17;
           break;
         case 16:
         NV(flute_kline3_cdur) = va_dur17;
           NV(flute_kline3_clp) = NV(flute_kline3_crp);
           NV(flute_kline3_crp) = va_x18;
           break;
         case 17:
         NV(flute_kline3_cdur) = va_dur18;
           NV(flute_kline3_clp) = NV(flute_kline3_crp);
           NV(flute_kline3_crp) = va_x19;
           break;
         case 18:
         NV(flute_kline3_cdur) = va_dur19;
           NV(flute_kline3_clp) = NV(flute_kline3_crp);
           NV(flute_kline3_crp) = va_x20;
           break;
           default:
           NVI(flute_kline3_first) = -100;
           NV(flute_kline3_cdur) = NV(flute_kline3_t) + 10000.0F;
           break;
           }
         NVI(flute_kline3_first)++;
        }
        NV(flute_kline3_mult)=(NV(flute_kline3_crp) - NV(flute_kline3_clp))/NV(flute_kline3_cdur);
        ret = NV(flute_kline3_outT) = NV(flute_kline3_clp)+NV(flute_kline3_mult)*NV(flute_kline3_t);
        NV(flute_kline3_addK) = NV(flute_kline3_mult)*EV(KTIME);
        if (NVI(flute_kline3_first)<0)
          ret = 0.0F;
      }
   }
   if (NVI(flute_kline3_first)==0)
     {
       NVI(flute_kline3_first) = 1;
       ret = NV(flute_kline3_outT) = NV(flute_kline3_clp) = x1;
       NV(flute_kline3_crp) = x2;
       NV(flute_kline3_cdur) = dur1;
       if (dur1 > 0.0F)
         NV(flute_kline3_addK) = EV(KTIME)*((x2 - x1)/dur1);
     }
   return((NV(flute_kline3_return) = ret));

}



float flute__sym_kline4(ENGINE_PTR_DECLARE_COMMA struct ninstr_types * nstate)
{
   float va_dur2;
   float va_x3;
   float va_dur3;
   float va_x4;
   float va_dur4;
   float va_x5;
   float va_dur5;
   float va_x6;
   float va_dur6;
   float va_x7;
   float va_dur7;
   float va_x8;
   float va_dur8;
   float va_x9;
   float va_dur9;
   float va_x10;
   float va_dur10;
   float va_x11;
   float va_dur11;
   float va_x12;
   float va_dur12;
   float va_x13;
   float va_dur13;
   float va_x14;
   float va_dur14;
   float va_x15;
   float va_dur15;
   float va_x16;
   float va_dur16;
   float va_x17;
   float va_dur17;
   float va_x18;
   float va_dur18;
   float va_x19;
   float va_dur19;
   float va_x20;
   float x2;
   float dur1;
   float x1;
   float ret;

   x1 =  0.0F ;
   dur1 =  0.121154F ;
   x2 =  0.104440F ;
   va_dur2 =  0.111538F  ;
   va_x3 =  0.160297F  ;
   va_dur3 =  0.103846F  ;
   va_x4 =  0.178152F  ;
   va_dur4 =  0.063462F  ;
   va_x5 =  0.239313F  ;
   va_dur5 =  0.057692F  ;
   va_x6 =  0.243839F  ;
   va_dur6 =  0.013462F  ;
   va_x7 =  0.206753F  ;
   va_dur7 =  0.046154F  ;
   va_x8 =  0.195542F  ;
   va_dur8 =  0.084615F  ;
   va_x9 =  0.262112F  ;
   va_dur9 =  0.015385F  ;
   va_x10 =  0.275729F  ;
   va_dur10 =  0.034615F  ;
   va_x11 =  0.188439F  ;
   va_dur11 =  0.040385F  ;
   va_x12 =  0.153378F  ;
   va_dur12 =  0.094231F  ;
   va_x13 =  0.278066F  ;
   va_dur13 =  0.050000F  ;
   va_x14 =  0.162670F  ;
   va_dur14 =  0.023077F  ;
   va_x15 =  0.134330F  ;
   va_dur15 =  0.040385F  ;
   va_x16 =  0.175186F  ;
   va_dur16 =  0.021154F  ;
   va_x17 =  0.218142F  ;
   va_dur17 =  0.369231F  ;
   va_x18 =  0.219114F  ;
   va_dur18 =  0.855769F  ;
   va_x19 =  0.140624F  ;
   va_dur19 =  0.159615F  ;
   va_x20 =  0.002817F  ;
   ret = 0.0F;
   if (NVI(flute_kline4_first)>0)
   {
      NV(flute_kline4_t) += EV(KTIME);
      ret = (NV(flute_kline4_outT) += NV(flute_kline4_addK));
      if (NV(flute_kline4_t) > NV(flute_kline4_cdur))
       {
        while (NV(flute_kline4_t) > NV(flute_kline4_cdur))
         {
           NV(flute_kline4_t) -= NV(flute_kline4_cdur);
           switch(NVI(flute_kline4_first))
      {
         case 1:
         NV(flute_kline4_cdur) = va_dur2;
           NV(flute_kline4_clp) = NV(flute_kline4_crp);
           NV(flute_kline4_crp) = va_x3;
           break;
         case 2:
         NV(flute_kline4_cdur) = va_dur3;
           NV(flute_kline4_clp) = NV(flute_kline4_crp);
           NV(flute_kline4_crp) = va_x4;
           break;
         case 3:
         NV(flute_kline4_cdur) = va_dur4;
           NV(flute_kline4_clp) = NV(flute_kline4_crp);
           NV(flute_kline4_crp) = va_x5;
           break;
         case 4:
         NV(flute_kline4_cdur) = va_dur5;
           NV(flute_kline4_clp) = NV(flute_kline4_crp);
           NV(flute_kline4_crp) = va_x6;
           break;
         case 5:
         NV(flute_kline4_cdur) = va_dur6;
           NV(flute_kline4_clp) = NV(flute_kline4_crp);
           NV(flute_kline4_crp) = va_x7;
           break;
         case 6:
         NV(flute_kline4_cdur) = va_dur7;
           NV(flute_kline4_clp) = NV(flute_kline4_crp);
           NV(flute_kline4_crp) = va_x8;
           break;
         case 7:
         NV(flute_kline4_cdur) = va_dur8;
           NV(flute_kline4_clp) = NV(flute_kline4_crp);
           NV(flute_kline4_crp) = va_x9;
           break;
         case 8:
         NV(flute_kline4_cdur) = va_dur9;
           NV(flute_kline4_clp) = NV(flute_kline4_crp);
           NV(flute_kline4_crp) = va_x10;
           break;
         case 9:
         NV(flute_kline4_cdur) = va_dur10;
           NV(flute_kline4_clp) = NV(flute_kline4_crp);
           NV(flute_kline4_crp) = va_x11;
           break;
         case 10:
         NV(flute_kline4_cdur) = va_dur11;
           NV(flute_kline4_clp) = NV(flute_kline4_crp);
           NV(flute_kline4_crp) = va_x12;
           break;
         case 11:
         NV(flute_kline4_cdur) = va_dur12;
           NV(flute_kline4_clp) = NV(flute_kline4_crp);
           NV(flute_kline4_crp) = va_x13;
           break;
         case 12:
         NV(flute_kline4_cdur) = va_dur13;
           NV(flute_kline4_clp) = NV(flute_kline4_crp);
           NV(flute_kline4_crp) = va_x14;
           break;
         case 13:
         NV(flute_kline4_cdur) = va_dur14;
           NV(flute_kline4_clp) = NV(flute_kline4_crp);
           NV(flute_kline4_crp) = va_x15;
           break;
         case 14:
         NV(flute_kline4_cdur) = va_dur15;
           NV(flute_kline4_clp) = NV(flute_kline4_crp);
           NV(flute_kline4_crp) = va_x16;
           break;
         case 15:
         NV(flute_kline4_cdur) = va_dur16;
           NV(flute_kline4_clp) = NV(flute_kline4_crp);
           NV(flute_kline4_crp) = va_x17;
           break;
         case 16:
         NV(flute_kline4_cdur) = va_dur17;
           NV(flute_kline4_clp) = NV(flute_kline4_crp);
           NV(flute_kline4_crp) = va_x18;
           break;
         case 17:
         NV(flute_kline4_cdur) = va_dur18;
           NV(flute_kline4_clp) = NV(flute_kline4_crp);
           NV(flute_kline4_crp) = va_x19;
           break;
         case 18:
         NV(flute_kline4_cdur) = va_dur19;
           NV(flute_kline4_clp) = NV(flute_kline4_crp);
           NV(flute_kline4_crp) = va_x20;
           break;
           default:
           NVI(flute_kline4_first) = -100;
           NV(flute_kline4_cdur) = NV(flute_kline4_t) + 10000.0F;
           break;
           }
         NVI(flute_kline4_first)++;
        }
        NV(flute_kline4_mult)=(NV(flute_kline4_crp) - NV(flute_kline4_clp))/NV(flute_kline4_cdur);
        ret = NV(flute_kline4_outT) = NV(flute_kline4_clp)+NV(flute_kline4_mult)*NV(flute_kline4_t);
        NV(flute_kline4_addK) = NV(flute_kline4_mult)*EV(KTIME);
        if (NVI(flute_kline4_first)<0)
          ret = 0.0F;
      }
   }
   if (NVI(flute_kline4_first)==0)
     {
       NVI(flute_kline4_first) = 1;
       ret = NV(flute_kline4_outT) = NV(flute_kline4_clp) = x1;
       NV(flute_kline4_crp) = x2;
       NV(flute_kline4_cdur) = dur1;
       if (dur1 > 0.0F)
         NV(flute_kline4_addK) = EV(KTIME)*((x2 - x1)/dur1);
     }
   return((NV(flute_kline4_return) = ret));

}



float flute__sym_kline5(ENGINE_PTR_DECLARE_COMMA struct ninstr_types * nstate)
{
   float va_dur2;
   float va_x3;
   float va_dur3;
   float va_x4;
   float va_dur4;
   float va_x5;
   float va_dur5;
   float va_x6;
   float va_dur6;
   float va_x7;
   float va_dur7;
   float va_x8;
   float va_dur8;
   float va_x9;
   float va_dur9;
   float va_x10;
   float va_dur10;
   float va_x11;
   float va_dur11;
   float va_x12;
   float va_dur12;
   float va_x13;
   float va_dur13;
   float va_x14;
   float va_dur14;
   float va_x15;
   float va_dur15;
   float va_x16;
   float va_dur16;
   float va_x17;
   float va_dur17;
   float va_x18;
   float va_dur18;
   float va_x19;
   float va_dur19;
   float va_x20;
   float x2;
   float dur1;
   float x1;
   float ret;

   x1 =  0.0F ;
   dur1 =  0.250000F ;
   x2 =  0.050610F ;
   va_dur2 =  0.236538F  ;
   va_x3 =  0.052573F  ;
   va_dur3 =  0.111538F  ;
   va_x4 =  0.091340F  ;
   va_dur4 =  0.032692F  ;
   va_x5 =  0.083642F  ;
   va_dur5 =  0.021154F  ;
   va_x6 =  0.038792F  ;
   va_dur6 =  0.036538F  ;
   va_x7 =  0.043367F  ;
   va_dur7 =  0.075000F  ;
   va_x8 =  0.080240F  ;
   va_dur8 =  0.046154F  ;
   va_x9 =  0.056172F  ;
   va_dur9 =  0.032692F  ;
   va_x10 =  0.039438F  ;
   va_dur10 =  0.113462F  ;
   va_x11 =  0.089941F  ;
   va_dur11 =  0.076923F  ;
   va_x12 =  0.031062F  ;
   va_dur12 =  0.032692F  ;
   va_x13 =  0.040738F  ;
   va_dur13 =  0.051923F  ;
   va_x14 =  0.065417F  ;
   va_dur14 =  0.073077F  ;
   va_x15 =  0.038719F  ;
   va_dur15 =  0.100000F  ;
   va_x16 =  0.065333F  ;
   va_dur16 =  0.121154F  ;
   va_x17 =  0.036677F  ;
   va_dur17 =  0.440385F  ;
   va_x18 =  0.054232F  ;
   va_dur18 =  0.311538F  ;
   va_x19 =  0.049932F  ;
   va_dur19 =  0.142308F  ;
   va_x20 =  0.000843F  ;
   ret = 0.0F;
   if (NVI(flute_kline5_first)>0)
   {
      NV(flute_kline5_t) += EV(KTIME);
      ret = (NV(flute_kline5_outT) += NV(flute_kline5_addK));
      if (NV(flute_kline5_t) > NV(flute_kline5_cdur))
       {
        while (NV(flute_kline5_t) > NV(flute_kline5_cdur))
         {
           NV(flute_kline5_t) -= NV(flute_kline5_cdur);
           switch(NVI(flute_kline5_first))
      {
         case 1:
         NV(flute_kline5_cdur) = va_dur2;
           NV(flute_kline5_clp) = NV(flute_kline5_crp);
           NV(flute_kline5_crp) = va_x3;
           break;
         case 2:
         NV(flute_kline5_cdur) = va_dur3;
           NV(flute_kline5_clp) = NV(flute_kline5_crp);
           NV(flute_kline5_crp) = va_x4;
           break;
         case 3:
         NV(flute_kline5_cdur) = va_dur4;
           NV(flute_kline5_clp) = NV(flute_kline5_crp);
           NV(flute_kline5_crp) = va_x5;
           break;
         case 4:
         NV(flute_kline5_cdur) = va_dur5;
           NV(flute_kline5_clp) = NV(flute_kline5_crp);
           NV(flute_kline5_crp) = va_x6;
           break;
         case 5:
         NV(flute_kline5_cdur) = va_dur6;
           NV(flute_kline5_clp) = NV(flute_kline5_crp);
           NV(flute_kline5_crp) = va_x7;
           break;
         case 6:
         NV(flute_kline5_cdur) = va_dur7;
           NV(flute_kline5_clp) = NV(flute_kline5_crp);
           NV(flute_kline5_crp) = va_x8;
           break;
         case 7:
         NV(flute_kline5_cdur) = va_dur8;
           NV(flute_kline5_clp) = NV(flute_kline5_crp);
           NV(flute_kline5_crp) = va_x9;
           break;
         case 8:
         NV(flute_kline5_cdur) = va_dur9;
           NV(flute_kline5_clp) = NV(flute_kline5_crp);
           NV(flute_kline5_crp) = va_x10;
           break;
         case 9:
         NV(flute_kline5_cdur) = va_dur10;
           NV(flute_kline5_clp) = NV(flute_kline5_crp);
           NV(flute_kline5_crp) = va_x11;
           break;
         case 10:
         NV(flute_kline5_cdur) = va_dur11;
           NV(flute_kline5_clp) = NV(flute_kline5_crp);
           NV(flute_kline5_crp) = va_x12;
           break;
         case 11:
         NV(flute_kline5_cdur) = va_dur12;
           NV(flute_kline5_clp) = NV(flute_kline5_crp);
           NV(flute_kline5_crp) = va_x13;
           break;
         case 12:
         NV(flute_kline5_cdur) = va_dur13;
           NV(flute_kline5_clp) = NV(flute_kline5_crp);
           NV(flute_kline5_crp) = va_x14;
           break;
         case 13:
         NV(flute_kline5_cdur) = va_dur14;
           NV(flute_kline5_clp) = NV(flute_kline5_crp);
           NV(flute_kline5_crp) = va_x15;
           break;
         case 14:
         NV(flute_kline5_cdur) = va_dur15;
           NV(flute_kline5_clp) = NV(flute_kline5_crp);
           NV(flute_kline5_crp) = va_x16;
           break;
         case 15:
         NV(flute_kline5_cdur) = va_dur16;
           NV(flute_kline5_clp) = NV(flute_kline5_crp);
           NV(flute_kline5_crp) = va_x17;
           break;
         case 16:
         NV(flute_kline5_cdur) = va_dur17;
           NV(flute_kline5_clp) = NV(flute_kline5_crp);
           NV(flute_kline5_crp) = va_x18;
           break;
         case 17:
         NV(flute_kline5_cdur) = va_dur18;
           NV(flute_kline5_clp) = NV(flute_kline5_crp);
           NV(flute_kline5_crp) = va_x19;
           break;
         case 18:
         NV(flute_kline5_cdur) = va_dur19;
           NV(flute_kline5_clp) = NV(flute_kline5_crp);
           NV(flute_kline5_crp) = va_x20;
           break;
           default:
           NVI(flute_kline5_first) = -100;
           NV(flute_kline5_cdur) = NV(flute_kline5_t) + 10000.0F;
           break;
           }
         NVI(flute_kline5_first)++;
        }
        NV(flute_kline5_mult)=(NV(flute_kline5_crp) - NV(flute_kline5_clp))/NV(flute_kline5_cdur);
        ret = NV(flute_kline5_outT) = NV(flute_kline5_clp)+NV(flute_kline5_mult)*NV(flute_kline5_t);
        NV(flute_kline5_addK) = NV(flute_kline5_mult)*EV(KTIME);
        if (NVI(flute_kline5_first)<0)
          ret = 0.0F;
      }
   }
   if (NVI(flute_kline5_first)==0)
     {
       NVI(flute_kline5_first) = 1;
       ret = NV(flute_kline5_outT) = NV(flute_kline5_clp) = x1;
       NV(flute_kline5_crp) = x2;
       NV(flute_kline5_cdur) = dur1;
       if (dur1 > 0.0F)
         NV(flute_kline5_addK) = EV(KTIME)*((x2 - x1)/dur1);
     }
   return((NV(flute_kline5_return) = ret));

}



float flute__sym_kline6(ENGINE_PTR_DECLARE_COMMA struct ninstr_types * nstate)
{
   float va_dur2;
   float va_x3;
   float va_dur3;
   float va_x4;
   float va_dur4;
   float va_x5;
   float va_dur5;
   float va_x6;
   float va_dur6;
   float va_x7;
   float va_dur7;
   float va_x8;
   float va_dur8;
   float va_x9;
   float va_dur9;
   float va_x10;
   float va_dur10;
   float va_x11;
   float va_dur11;
   float va_x12;
   float va_dur12;
   float va_x13;
   float va_dur13;
   float va_x14;
   float va_dur14;
   float va_x15;
   float va_dur15;
   float va_x16;
   float va_dur16;
   float va_x17;
   float va_dur17;
   float va_x18;
   float va_dur18;
   float va_x19;
   float va_dur19;
   float va_x20;
   float x2;
   float dur1;
   float x1;
   float ret;

   x1 =  0.0F ;
   dur1 =  0.111538F ;
   x2 =  0.017359F ;
   va_dur2 =  0.255769F  ;
   va_x3 =  0.042226F  ;
   va_dur3 =  0.196154F  ;
   va_x4 =  0.031286F  ;
   va_dur4 =  0.032692F  ;
   va_x5 =  0.050869F  ;
   va_dur5 =  0.088462F  ;
   va_x6 =  0.027569F  ;
   va_dur6 =  0.105769F  ;
   va_x7 =  0.060205F  ;
   va_dur7 =  0.040385F  ;
   va_x8 =  0.027966F  ;
   va_dur8 =  0.048077F  ;
   va_x9 =  0.028143F  ;
   va_dur9 =  0.048077F  ;
   va_x10 =  0.046221F  ;
   va_dur10 =  0.109615F  ;
   va_x11 =  0.024257F  ;
   va_dur11 =  0.100000F  ;
   va_x12 =  0.050589F  ;
   va_dur12 =  0.082692F  ;
   va_x13 =  0.025498F  ;
   va_dur13 =  0.075000F  ;
   va_x14 =  0.046186F  ;
   va_dur14 =  0.036538F  ;
   va_x15 =  0.027736F  ;
   va_dur15 =  0.269231F  ;
   va_x16 =  0.022008F  ;
   va_dur16 =  0.050000F  ;
   va_x17 =  0.041721F  ;
   va_dur17 =  0.094231F  ;
   va_x18 =  0.019350F  ;
   va_dur18 =  0.450000F  ;
   va_x19 =  0.029740F  ;
   va_dur19 =  0.111538F  ;
   va_x20 =  0.000499F  ;
   ret = 0.0F;
   if (NVI(flute_kline6_first)>0)
   {
      NV(flute_kline6_t) += EV(KTIME);
      ret = (NV(flute_kline6_outT) += NV(flute_kline6_addK));
      if (NV(flute_kline6_t) > NV(flute_kline6_cdur))
       {
        while (NV(flute_kline6_t) > NV(flute_kline6_cdur))
         {
           NV(flute_kline6_t) -= NV(flute_kline6_cdur);
           switch(NVI(flute_kline6_first))
      {
         case 1:
         NV(flute_kline6_cdur) = va_dur2;
           NV(flute_kline6_clp) = NV(flute_kline6_crp);
           NV(flute_kline6_crp) = va_x3;
           break;
         case 2:
         NV(flute_kline6_cdur) = va_dur3;
           NV(flute_kline6_clp) = NV(flute_kline6_crp);
           NV(flute_kline6_crp) = va_x4;
           break;
         case 3:
         NV(flute_kline6_cdur) = va_dur4;
           NV(flute_kline6_clp) = NV(flute_kline6_crp);
           NV(flute_kline6_crp) = va_x5;
           break;
         case 4:
         NV(flute_kline6_cdur) = va_dur5;
           NV(flute_kline6_clp) = NV(flute_kline6_crp);
           NV(flute_kline6_crp) = va_x6;
           break;
         case 5:
         NV(flute_kline6_cdur) = va_dur6;
           NV(flute_kline6_clp) = NV(flute_kline6_crp);
           NV(flute_kline6_crp) = va_x7;
           break;
         case 6:
         NV(flute_kline6_cdur) = va_dur7;
           NV(flute_kline6_clp) = NV(flute_kline6_crp);
           NV(flute_kline6_crp) = va_x8;
           break;
         case 7:
         NV(flute_kline6_cdur) = va_dur8;
           NV(flute_kline6_clp) = NV(flute_kline6_crp);
           NV(flute_kline6_crp) = va_x9;
           break;
         case 8:
         NV(flute_kline6_cdur) = va_dur9;
           NV(flute_kline6_clp) = NV(flute_kline6_crp);
           NV(flute_kline6_crp) = va_x10;
           break;
         case 9:
         NV(flute_kline6_cdur) = va_dur10;
           NV(flute_kline6_clp) = NV(flute_kline6_crp);
           NV(flute_kline6_crp) = va_x11;
           break;
         case 10:
         NV(flute_kline6_cdur) = va_dur11;
           NV(flute_kline6_clp) = NV(flute_kline6_crp);
           NV(flute_kline6_crp) = va_x12;
           break;
         case 11:
         NV(flute_kline6_cdur) = va_dur12;
           NV(flute_kline6_clp) = NV(flute_kline6_crp);
           NV(flute_kline6_crp) = va_x13;
           break;
         case 12:
         NV(flute_kline6_cdur) = va_dur13;
           NV(flute_kline6_clp) = NV(flute_kline6_crp);
           NV(flute_kline6_crp) = va_x14;
           break;
         case 13:
         NV(flute_kline6_cdur) = va_dur14;
           NV(flute_kline6_clp) = NV(flute_kline6_crp);
           NV(flute_kline6_crp) = va_x15;
           break;
         case 14:
         NV(flute_kline6_cdur) = va_dur15;
           NV(flute_kline6_clp) = NV(flute_kline6_crp);
           NV(flute_kline6_crp) = va_x16;
           break;
         case 15:
         NV(flute_kline6_cdur) = va_dur16;
           NV(flute_kline6_clp) = NV(flute_kline6_crp);
           NV(flute_kline6_crp) = va_x17;
           break;
         case 16:
         NV(flute_kline6_cdur) = va_dur17;
           NV(flute_kline6_clp) = NV(flute_kline6_crp);
           NV(flute_kline6_crp) = va_x18;
           break;
         case 17:
         NV(flute_kline6_cdur) = va_dur18;
           NV(flute_kline6_clp) = NV(flute_kline6_crp);
           NV(flute_kline6_crp) = va_x19;
           break;
         case 18:
         NV(flute_kline6_cdur) = va_dur19;
           NV(flute_kline6_clp) = NV(flute_kline6_crp);
           NV(flute_kline6_crp) = va_x20;
           break;
           default:
           NVI(flute_kline6_first) = -100;
           NV(flute_kline6_cdur) = NV(flute_kline6_t) + 10000.0F;
           break;
           }
         NVI(flute_kline6_first)++;
        }
        NV(flute_kline6_mult)=(NV(flute_kline6_crp) - NV(flute_kline6_clp))/NV(flute_kline6_cdur);
        ret = NV(flute_kline6_outT) = NV(flute_kline6_clp)+NV(flute_kline6_mult)*NV(flute_kline6_t);
        NV(flute_kline6_addK) = NV(flute_kline6_mult)*EV(KTIME);
        if (NVI(flute_kline6_first)<0)
          ret = 0.0F;
      }
   }
   if (NVI(flute_kline6_first)==0)
     {
       NVI(flute_kline6_first) = 1;
       ret = NV(flute_kline6_outT) = NV(flute_kline6_clp) = x1;
       NV(flute_kline6_crp) = x2;
       NV(flute_kline6_cdur) = dur1;
       if (dur1 > 0.0F)
         NV(flute_kline6_addK) = EV(KTIME)*((x2 - x1)/dur1);
     }
   return((NV(flute_kline6_return) = ret));

}



float flute__sym_kline7(ENGINE_PTR_DECLARE_COMMA struct ninstr_types * nstate)
{
   float va_dur2;
   float va_x3;
   float va_dur3;
   float va_x4;
   float va_dur4;
   float va_x5;
   float va_dur5;
   float va_x6;
   float va_dur6;
   float va_x7;
   float va_dur7;
   float va_x8;
   float va_dur8;
   float va_x9;
   float va_dur9;
   float va_x10;
   float va_dur10;
   float va_x11;
   float va_dur11;
   float va_x12;
   float va_dur12;
   float va_x13;
   float va_dur13;
   float va_x14;
   float va_dur14;
   float va_x15;
   float va_dur15;
   float va_x16;
   float va_dur16;
   float va_x17;
   float va_dur17;
   float va_x18;
   float va_dur18;
   float va_x19;
   float va_dur19;
   float va_x20;
   float x2;
   float dur1;
   float x1;
   float ret;

   x1 =  0.0F ;
   dur1 =  0.405769F ;
   x2 =  0.025024F ;
   va_dur2 =  0.380769F  ;
   va_x3 =  0.013352F  ;
   va_dur3 =  0.003846F  ;
   va_x4 =  0.029156F  ;
   va_dur4 =  0.003846F  ;
   va_x5 =  0.013638F  ;
   va_dur5 =  0.015385F  ;
   va_x6 =  0.014984F  ;
   va_dur6 =  0.098077F  ;
   va_x7 =  0.012338F  ;
   va_dur7 =  0.036538F  ;
   va_x8 =  0.024177F  ;
   va_dur8 =  0.176923F  ;
   va_x9 =  0.022102F  ;
   va_dur9 =  0.073077F  ;
   va_x10 =  0.008078F  ;
   va_dur10 =  0.230769F  ;
   va_x11 =  0.009086F  ;
   va_dur11 =  0.078846F  ;
   va_x12 =  0.017682F  ;
   va_dur12 =  0.073077F  ;
   va_x13 =  0.007015F  ;
   va_dur13 =  0.050000F  ;
   va_x14 =  0.016699F  ;
   va_dur14 =  0.178846F  ;
   va_x15 =  0.008888F  ;
   va_dur15 =  0.017308F  ;
   va_x16 =  0.017341F  ;
   va_dur16 =  0.138462F  ;
   va_x17 =  0.008060F  ;
   va_dur17 =  0.188462F  ;
   va_x18 =  0.015178F  ;
   va_dur18 =  0.036538F  ;
   va_x19 =  0.006738F  ;
   va_dur19 =  0.119231F  ;
   va_x20 =  0.000241F  ;
   ret = 0.0F;
   if (NVI(flute_kline7_first)>0)
   {
      NV(flute_kline7_t) += EV(KTIME);
      ret = (NV(flute_kline7_outT) += NV(flute_kline7_addK));
      if (NV(flute_kline7_t) > NV(flute_kline7_cdur))
       {
        while (NV(flute_kline7_t) > NV(flute_kline7_cdur))
         {
           NV(flute_kline7_t) -= NV(flute_kline7_cdur);
           switch(NVI(flute_kline7_first))
      {
         case 1:
         NV(flute_kline7_cdur) = va_dur2;
           NV(flute_kline7_clp) = NV(flute_kline7_crp);
           NV(flute_kline7_crp) = va_x3;
           break;
         case 2:
         NV(flute_kline7_cdur) = va_dur3;
           NV(flute_kline7_clp) = NV(flute_kline7_crp);
           NV(flute_kline7_crp) = va_x4;
           break;
         case 3:
         NV(flute_kline7_cdur) = va_dur4;
           NV(flute_kline7_clp) = NV(flute_kline7_crp);
           NV(flute_kline7_crp) = va_x5;
           break;
         case 4:
         NV(flute_kline7_cdur) = va_dur5;
           NV(flute_kline7_clp) = NV(flute_kline7_crp);
           NV(flute_kline7_crp) = va_x6;
           break;
         case 5:
         NV(flute_kline7_cdur) = va_dur6;
           NV(flute_kline7_clp) = NV(flute_kline7_crp);
           NV(flute_kline7_crp) = va_x7;
           break;
         case 6:
         NV(flute_kline7_cdur) = va_dur7;
           NV(flute_kline7_clp) = NV(flute_kline7_crp);
           NV(flute_kline7_crp) = va_x8;
           break;
         case 7:
         NV(flute_kline7_cdur) = va_dur8;
           NV(flute_kline7_clp) = NV(flute_kline7_crp);
           NV(flute_kline7_crp) = va_x9;
           break;
         case 8:
         NV(flute_kline7_cdur) = va_dur9;
           NV(flute_kline7_clp) = NV(flute_kline7_crp);
           NV(flute_kline7_crp) = va_x10;
           break;
         case 9:
         NV(flute_kline7_cdur) = va_dur10;
           NV(flute_kline7_clp) = NV(flute_kline7_crp);
           NV(flute_kline7_crp) = va_x11;
           break;
         case 10:
         NV(flute_kline7_cdur) = va_dur11;
           NV(flute_kline7_clp) = NV(flute_kline7_crp);
           NV(flute_kline7_crp) = va_x12;
           break;
         case 11:
         NV(flute_kline7_cdur) = va_dur12;
           NV(flute_kline7_clp) = NV(flute_kline7_crp);
           NV(flute_kline7_crp) = va_x13;
           break;
         case 12:
         NV(flute_kline7_cdur) = va_dur13;
           NV(flute_kline7_clp) = NV(flute_kline7_crp);
           NV(flute_kline7_crp) = va_x14;
           break;
         case 13:
         NV(flute_kline7_cdur) = va_dur14;
           NV(flute_kline7_clp) = NV(flute_kline7_crp);
           NV(flute_kline7_crp) = va_x15;
           break;
         case 14:
         NV(flute_kline7_cdur) = va_dur15;
           NV(flute_kline7_clp) = NV(flute_kline7_crp);
           NV(flute_kline7_crp) = va_x16;
           break;
         case 15:
         NV(flute_kline7_cdur) = va_dur16;
           NV(flute_kline7_clp) = NV(flute_kline7_crp);
           NV(flute_kline7_crp) = va_x17;
           break;
         case 16:
         NV(flute_kline7_cdur) = va_dur17;
           NV(flute_kline7_clp) = NV(flute_kline7_crp);
           NV(flute_kline7_crp) = va_x18;
           break;
         case 17:
         NV(flute_kline7_cdur) = va_dur18;
           NV(flute_kline7_clp) = NV(flute_kline7_crp);
           NV(flute_kline7_crp) = va_x19;
           break;
         case 18:
         NV(flute_kline7_cdur) = va_dur19;
           NV(flute_kline7_clp) = NV(flute_kline7_crp);
           NV(flute_kline7_crp) = va_x20;
           break;
           default:
           NVI(flute_kline7_first) = -100;
           NV(flute_kline7_cdur) = NV(flute_kline7_t) + 10000.0F;
           break;
           }
         NVI(flute_kline7_first)++;
        }
        NV(flute_kline7_mult)=(NV(flute_kline7_crp) - NV(flute_kline7_clp))/NV(flute_kline7_cdur);
        ret = NV(flute_kline7_outT) = NV(flute_kline7_clp)+NV(flute_kline7_mult)*NV(flute_kline7_t);
        NV(flute_kline7_addK) = NV(flute_kline7_mult)*EV(KTIME);
        if (NVI(flute_kline7_first)<0)
          ret = 0.0F;
      }
   }
   if (NVI(flute_kline7_first)==0)
     {
       NVI(flute_kline7_first) = 1;
       ret = NV(flute_kline7_outT) = NV(flute_kline7_clp) = x1;
       NV(flute_kline7_crp) = x2;
       NV(flute_kline7_cdur) = dur1;
       if (dur1 > 0.0F)
         NV(flute_kline7_addK) = EV(KTIME)*((x2 - x1)/dur1);
     }
   return((NV(flute_kline7_return) = ret));

}



float flute__sym_kline8(ENGINE_PTR_DECLARE_COMMA struct ninstr_types * nstate)
{
   float va_dur2;
   float va_x3;
   float va_dur3;
   float va_x4;
   float va_dur4;
   float va_x5;
   float va_dur5;
   float va_x6;
   float va_dur6;
   float va_x7;
   float va_dur7;
   float va_x8;
   float va_dur8;
   float va_x9;
   float va_dur9;
   float va_x10;
   float va_dur10;
   float va_x11;
   float va_dur11;
   float va_x12;
   float va_dur12;
   float va_x13;
   float va_dur13;
   float va_x14;
   float va_dur14;
   float va_x15;
   float va_dur15;
   float va_x16;
   float va_dur16;
   float va_x17;
   float va_dur17;
   float va_x18;
   float va_dur18;
   float va_x19;
   float va_dur19;
   float va_x20;
   float x2;
   float dur1;
   float x1;
   float ret;

   x1 =  0.0F ;
   dur1 =  0.801923F ;
   x2 =  0.020638F ;
   va_dur2 =  0.975000F  ;
   va_x3 =  0.011216F  ;
   va_dur3 =  0.271154F  ;
   va_x4 =  0.005008F  ;
   va_dur4 =  0.009615F  ;
   va_x5 =  0.010307F  ;
   va_dur5 =  0.026923F  ;
   va_x6 =  0.005870F  ;
   va_dur6 =  0.013462F  ;
   va_x7 =  0.005342F  ;
   va_dur7 =  0.001923F  ;
   va_x8 =  0.008010F  ;
   va_dur8 =  0.017308F  ;
   va_x9 =  0.007936F  ;
   va_dur9 =  0.005769F  ;
   va_x10 =  0.004772F  ;
   va_dur10 =  0.009615F  ;
   va_x11 =  0.009320F  ;
   va_dur11 =  0.003846F  ;
   va_x12 =  0.005457F  ;
   va_dur12 =  0.009615F  ;
   va_x13 =  0.006781F  ;
   va_dur13 =  0.003846F  ;
   va_x14 =  0.004190F  ;
   va_dur14 =  0.009615F  ;
   va_x15 =  0.011191F  ;
   va_dur15 =  0.005769F  ;
   va_x16 =  0.005481F  ;
   va_dur16 =  0.001923F  ;
   va_x17 =  0.009691F  ;
   va_dur17 =  0.003846F  ;
   va_x18 =  0.005682F  ;
   va_dur18 =  0.003846F  ;
   va_x19 =  0.007783F  ;
   va_dur19 =  0.130769F  ;
   va_x20 =  0.000094F  ;
   ret = 0.0F;
   if (NVI(flute_kline8_first)>0)
   {
      NV(flute_kline8_t) += EV(KTIME);
      ret = (NV(flute_kline8_outT) += NV(flute_kline8_addK));
      if (NV(flute_kline8_t) > NV(flute_kline8_cdur))
       {
        while (NV(flute_kline8_t) > NV(flute_kline8_cdur))
         {
           NV(flute_kline8_t) -= NV(flute_kline8_cdur);
           switch(NVI(flute_kline8_first))
      {
         case 1:
         NV(flute_kline8_cdur) = va_dur2;
           NV(flute_kline8_clp) = NV(flute_kline8_crp);
           NV(flute_kline8_crp) = va_x3;
           break;
         case 2:
         NV(flute_kline8_cdur) = va_dur3;
           NV(flute_kline8_clp) = NV(flute_kline8_crp);
           NV(flute_kline8_crp) = va_x4;
           break;
         case 3:
         NV(flute_kline8_cdur) = va_dur4;
           NV(flute_kline8_clp) = NV(flute_kline8_crp);
           NV(flute_kline8_crp) = va_x5;
           break;
         case 4:
         NV(flute_kline8_cdur) = va_dur5;
           NV(flute_kline8_clp) = NV(flute_kline8_crp);
           NV(flute_kline8_crp) = va_x6;
           break;
         case 5:
         NV(flute_kline8_cdur) = va_dur6;
           NV(flute_kline8_clp) = NV(flute_kline8_crp);
           NV(flute_kline8_crp) = va_x7;
           break;
         case 6:
         NV(flute_kline8_cdur) = va_dur7;
           NV(flute_kline8_clp) = NV(flute_kline8_crp);
           NV(flute_kline8_crp) = va_x8;
           break;
         case 7:
         NV(flute_kline8_cdur) = va_dur8;
           NV(flute_kline8_clp) = NV(flute_kline8_crp);
           NV(flute_kline8_crp) = va_x9;
           break;
         case 8:
         NV(flute_kline8_cdur) = va_dur9;
           NV(flute_kline8_clp) = NV(flute_kline8_crp);
           NV(flute_kline8_crp) = va_x10;
           break;
         case 9:
         NV(flute_kline8_cdur) = va_dur10;
           NV(flute_kline8_clp) = NV(flute_kline8_crp);
           NV(flute_kline8_crp) = va_x11;
           break;
         case 10:
         NV(flute_kline8_cdur) = va_dur11;
           NV(flute_kline8_clp) = NV(flute_kline8_crp);
           NV(flute_kline8_crp) = va_x12;
           break;
         case 11:
         NV(flute_kline8_cdur) = va_dur12;
           NV(flute_kline8_clp) = NV(flute_kline8_crp);
           NV(flute_kline8_crp) = va_x13;
           break;
         case 12:
         NV(flute_kline8_cdur) = va_dur13;
           NV(flute_kline8_clp) = NV(flute_kline8_crp);
           NV(flute_kline8_crp) = va_x14;
           break;
         case 13:
         NV(flute_kline8_cdur) = va_dur14;
           NV(flute_kline8_clp) = NV(flute_kline8_crp);
           NV(flute_kline8_crp) = va_x15;
           break;
         case 14:
         NV(flute_kline8_cdur) = va_dur15;
           NV(flute_kline8_clp) = NV(flute_kline8_crp);
           NV(flute_kline8_crp) = va_x16;
           break;
         case 15:
         NV(flute_kline8_cdur) = va_dur16;
           NV(flute_kline8_clp) = NV(flute_kline8_crp);
           NV(flute_kline8_crp) = va_x17;
           break;
         case 16:
         NV(flute_kline8_cdur) = va_dur17;
           NV(flute_kline8_clp) = NV(flute_kline8_crp);
           NV(flute_kline8_crp) = va_x18;
           break;
         case 17:
         NV(flute_kline8_cdur) = va_dur18;
           NV(flute_kline8_clp) = NV(flute_kline8_crp);
           NV(flute_kline8_crp) = va_x19;
           break;
         case 18:
         NV(flute_kline8_cdur) = va_dur19;
           NV(flute_kline8_clp) = NV(flute_kline8_crp);
           NV(flute_kline8_crp) = va_x20;
           break;
           default:
           NVI(flute_kline8_first) = -100;
           NV(flute_kline8_cdur) = NV(flute_kline8_t) + 10000.0F;
           break;
           }
         NVI(flute_kline8_first)++;
        }
        NV(flute_kline8_mult)=(NV(flute_kline8_crp) - NV(flute_kline8_clp))/NV(flute_kline8_cdur);
        ret = NV(flute_kline8_outT) = NV(flute_kline8_clp)+NV(flute_kline8_mult)*NV(flute_kline8_t);
        NV(flute_kline8_addK) = NV(flute_kline8_mult)*EV(KTIME);
        if (NVI(flute_kline8_first)<0)
          ret = 0.0F;
      }
   }
   if (NVI(flute_kline8_first)==0)
     {
       NVI(flute_kline8_first) = 1;
       ret = NV(flute_kline8_outT) = NV(flute_kline8_clp) = x1;
       NV(flute_kline8_crp) = x2;
       NV(flute_kline8_cdur) = dur1;
       if (dur1 > 0.0F)
         NV(flute_kline8_addK) = EV(KTIME)*((x2 - x1)/dur1);
     }
   return((NV(flute_kline8_return) = ret));

}



float flute__sym_kline9(ENGINE_PTR_DECLARE_COMMA struct ninstr_types * nstate)
{
   float va_dur2;
   float va_x3;
   float va_dur3;
   float va_x4;
   float va_dur4;
   float va_x5;
   float va_dur5;
   float va_x6;
   float va_dur6;
   float va_x7;
   float va_dur7;
   float va_x8;
   float va_dur8;
   float va_x9;
   float va_dur9;
   float va_x10;
   float va_dur10;
   float va_x11;
   float va_dur11;
   float va_x12;
   float va_dur12;
   float va_x13;
   float va_dur13;
   float va_x14;
   float va_dur14;
   float va_x15;
   float va_dur15;
   float va_x16;
   float va_dur16;
   float va_x17;
   float va_dur17;
   float va_x18;
   float va_dur18;
   float va_x19;
   float va_dur19;
   float va_x20;
   float x2;
   float dur1;
   float x1;
   float ret;

   x1 =  0.0F ;
   dur1 =  0.801923F ;
   x2 =  0.018672F ;
   va_dur2 =  1.005769F  ;
   va_x3 =  0.009949F  ;
   va_dur3 =  0.201923F  ;
   va_x4 =  0.008613F  ;
   va_dur4 =  0.128846F  ;
   va_x5 =  0.004551F  ;
   va_dur5 =  0.009615F  ;
   va_x6 =  0.008652F  ;
   va_dur6 =  0.007692F  ;
   va_x7 =  0.007908F  ;
   va_dur7 =  0.003846F  ;
   va_x8 =  0.007181F  ;
   va_dur8 =  0.001923F  ;
   va_x9 =  0.001933F  ;
   va_dur9 =  0.003846F  ;
   va_x10 =  0.003839F  ;
   va_dur10 =  0.003846F  ;
   va_x11 =  0.006452F  ;
   va_dur11 =  0.005769F  ;
   va_x12 =  0.003321F  ;
   va_dur12 =  0.011538F  ;
   va_x13 =  0.004510F  ;
   va_dur13 =  0.003846F  ;
   va_x14 =  0.008486F  ;
   va_dur14 =  0.007692F  ;
   va_x15 =  0.004728F  ;
   va_dur15 =  0.003846F  ;
   va_x16 =  0.006950F  ;
   va_dur16 =  0.005769F  ;
   va_x17 =  0.004227F  ;
   va_dur17 =  0.005769F  ;
   va_x18 =  0.006954F  ;
   va_dur18 =  0.009615F  ;
   va_x19 =  0.002542F  ;
   va_dur19 =  0.082692F  ;
   va_x20 =  0.000125F  ;
   ret = 0.0F;
   if (NVI(flute_kline9_first)>0)
   {
      NV(flute_kline9_t) += EV(KTIME);
      ret = (NV(flute_kline9_outT) += NV(flute_kline9_addK));
      if (NV(flute_kline9_t) > NV(flute_kline9_cdur))
       {
        while (NV(flute_kline9_t) > NV(flute_kline9_cdur))
         {
           NV(flute_kline9_t) -= NV(flute_kline9_cdur);
           switch(NVI(flute_kline9_first))
      {
         case 1:
         NV(flute_kline9_cdur) = va_dur2;
           NV(flute_kline9_clp) = NV(flute_kline9_crp);
           NV(flute_kline9_crp) = va_x3;
           break;
         case 2:
         NV(flute_kline9_cdur) = va_dur3;
           NV(flute_kline9_clp) = NV(flute_kline9_crp);
           NV(flute_kline9_crp) = va_x4;
           break;
         case 3:
         NV(flute_kline9_cdur) = va_dur4;
           NV(flute_kline9_clp) = NV(flute_kline9_crp);
           NV(flute_kline9_crp) = va_x5;
           break;
         case 4:
         NV(flute_kline9_cdur) = va_dur5;
           NV(flute_kline9_clp) = NV(flute_kline9_crp);
           NV(flute_kline9_crp) = va_x6;
           break;
         case 5:
         NV(flute_kline9_cdur) = va_dur6;
           NV(flute_kline9_clp) = NV(flute_kline9_crp);
           NV(flute_kline9_crp) = va_x7;
           break;
         case 6:
         NV(flute_kline9_cdur) = va_dur7;
           NV(flute_kline9_clp) = NV(flute_kline9_crp);
           NV(flute_kline9_crp) = va_x8;
           break;
         case 7:
         NV(flute_kline9_cdur) = va_dur8;
           NV(flute_kline9_clp) = NV(flute_kline9_crp);
           NV(flute_kline9_crp) = va_x9;
           break;
         case 8:
         NV(flute_kline9_cdur) = va_dur9;
           NV(flute_kline9_clp) = NV(flute_kline9_crp);
           NV(flute_kline9_crp) = va_x10;
           break;
         case 9:
         NV(flute_kline9_cdur) = va_dur10;
           NV(flute_kline9_clp) = NV(flute_kline9_crp);
           NV(flute_kline9_crp) = va_x11;
           break;
         case 10:
         NV(flute_kline9_cdur) = va_dur11;
           NV(flute_kline9_clp) = NV(flute_kline9_crp);
           NV(flute_kline9_crp) = va_x12;
           break;
         case 11:
         NV(flute_kline9_cdur) = va_dur12;
           NV(flute_kline9_clp) = NV(flute_kline9_crp);
           NV(flute_kline9_crp) = va_x13;
           break;
         case 12:
         NV(flute_kline9_cdur) = va_dur13;
           NV(flute_kline9_clp) = NV(flute_kline9_crp);
           NV(flute_kline9_crp) = va_x14;
           break;
         case 13:
         NV(flute_kline9_cdur) = va_dur14;
           NV(flute_kline9_clp) = NV(flute_kline9_crp);
           NV(flute_kline9_crp) = va_x15;
           break;
         case 14:
         NV(flute_kline9_cdur) = va_dur15;
           NV(flute_kline9_clp) = NV(flute_kline9_crp);
           NV(flute_kline9_crp) = va_x16;
           break;
         case 15:
         NV(flute_kline9_cdur) = va_dur16;
           NV(flute_kline9_clp) = NV(flute_kline9_crp);
           NV(flute_kline9_crp) = va_x17;
           break;
         case 16:
         NV(flute_kline9_cdur) = va_dur17;
           NV(flute_kline9_clp) = NV(flute_kline9_crp);
           NV(flute_kline9_crp) = va_x18;
           break;
         case 17:
         NV(flute_kline9_cdur) = va_dur18;
           NV(flute_kline9_clp) = NV(flute_kline9_crp);
           NV(flute_kline9_crp) = va_x19;
           break;
         case 18:
         NV(flute_kline9_cdur) = va_dur19;
           NV(flute_kline9_clp) = NV(flute_kline9_crp);
           NV(flute_kline9_crp) = va_x20;
           break;
           default:
           NVI(flute_kline9_first) = -100;
           NV(flute_kline9_cdur) = NV(flute_kline9_t) + 10000.0F;
           break;
           }
         NVI(flute_kline9_first)++;
        }
        NV(flute_kline9_mult)=(NV(flute_kline9_crp) - NV(flute_kline9_clp))/NV(flute_kline9_cdur);
        ret = NV(flute_kline9_outT) = NV(flute_kline9_clp)+NV(flute_kline9_mult)*NV(flute_kline9_t);
        NV(flute_kline9_addK) = NV(flute_kline9_mult)*EV(KTIME);
        if (NVI(flute_kline9_first)<0)
          ret = 0.0F;
      }
   }
   if (NVI(flute_kline9_first)==0)
     {
       NVI(flute_kline9_first) = 1;
       ret = NV(flute_kline9_outT) = NV(flute_kline9_clp) = x1;
       NV(flute_kline9_crp) = x2;
       NV(flute_kline9_cdur) = dur1;
       if (dur1 > 0.0F)
         NV(flute_kline9_addK) = EV(KTIME)*((x2 - x1)/dur1);
     }
   return((NV(flute_kline9_return) = ret));

}



float flute__sym_kline10(ENGINE_PTR_DECLARE_COMMA struct ninstr_types * nstate)
{
   float va_dur2;
   float va_x3;
   float va_dur3;
   float va_x4;
   float va_dur4;
   float va_x5;
   float va_dur5;
   float va_x6;
   float va_dur6;
   float va_x7;
   float va_dur7;
   float va_x8;
   float va_dur8;
   float va_x9;
   float va_dur9;
   float va_x10;
   float va_dur10;
   float va_x11;
   float va_dur11;
   float va_x12;
   float va_dur12;
   float va_x13;
   float va_dur13;
   float va_x14;
   float va_dur14;
   float va_x15;
   float va_dur15;
   float va_x16;
   float va_dur16;
   float va_x17;
   float va_dur17;
   float va_x18;
   float va_dur18;
   float va_x19;
   float va_dur19;
   float va_x20;
   float x2;
   float dur1;
   float x1;
   float ret;

   x1 =  0.0F ;
   dur1 =  0.801923F ;
   x2 =  0.013061F ;
   va_dur2 =  1.150000F  ;
   va_x3 =  0.006786F  ;
   va_dur3 =  0.023077F  ;
   va_x4 =  0.003822F  ;
   va_dur4 =  0.023077F  ;
   va_x5 =  0.003972F  ;
   va_dur5 =  0.111538F  ;
   va_x6 =  0.004655F  ;
   va_dur6 =  0.028846F  ;
   va_x7 =  0.003727F  ;
   va_dur7 =  0.001923F  ;
   va_x8 =  0.002607F  ;
   va_dur8 =  0.007692F  ;
   va_x9 =  0.003879F  ;
   va_dur9 =  0.046154F  ;
   va_x10 =  0.000264F  ;
   va_dur10 =  0.001923F  ;
   va_x11 =  0.002324F  ;
   va_dur11 =  0.001923F  ;
   va_x12 =  0.002877F  ;
   va_dur12 =  0.001923F  ;
   va_x13 =  0.002254F  ;
   va_dur13 =  0.001923F  ;
   va_x14 =  0.002207F  ;
   va_dur14 =  0.001923F  ;
   va_x15 =  0.003435F  ;
   va_dur15 =  0.001923F  ;
   va_x16 =  0.003418F  ;
   va_dur16 =  0.001923F  ;
   va_x17 =  0.002344F  ;
   va_dur17 =  0.015385F  ;
   va_x18 =  0.001891F  ;
   va_dur18 =  0.082692F  ;
   va_x19 =  0.000106F  ;
   va_dur19 =  0.0F  ;
   va_x20 =  0.000106F  ;
   ret = 0.0F;
   if (NVI(flute_kline10_first)>0)
   {
      NV(flute_kline10_t) += EV(KTIME);
      ret = (NV(flute_kline10_outT) += NV(flute_kline10_addK));
      if (NV(flute_kline10_t) > NV(flute_kline10_cdur))
       {
        while (NV(flute_kline10_t) > NV(flute_kline10_cdur))
         {
           NV(flute_kline10_t) -= NV(flute_kline10_cdur);
           switch(NVI(flute_kline10_first))
      {
         case 1:
         NV(flute_kline10_cdur) = va_dur2;
           NV(flute_kline10_clp) = NV(flute_kline10_crp);
           NV(flute_kline10_crp) = va_x3;
           break;
         case 2:
         NV(flute_kline10_cdur) = va_dur3;
           NV(flute_kline10_clp) = NV(flute_kline10_crp);
           NV(flute_kline10_crp) = va_x4;
           break;
         case 3:
         NV(flute_kline10_cdur) = va_dur4;
           NV(flute_kline10_clp) = NV(flute_kline10_crp);
           NV(flute_kline10_crp) = va_x5;
           break;
         case 4:
         NV(flute_kline10_cdur) = va_dur5;
           NV(flute_kline10_clp) = NV(flute_kline10_crp);
           NV(flute_kline10_crp) = va_x6;
           break;
         case 5:
         NV(flute_kline10_cdur) = va_dur6;
           NV(flute_kline10_clp) = NV(flute_kline10_crp);
           NV(flute_kline10_crp) = va_x7;
           break;
         case 6:
         NV(flute_kline10_cdur) = va_dur7;
           NV(flute_kline10_clp) = NV(flute_kline10_crp);
           NV(flute_kline10_crp) = va_x8;
           break;
         case 7:
         NV(flute_kline10_cdur) = va_dur8;
           NV(flute_kline10_clp) = NV(flute_kline10_crp);
           NV(flute_kline10_crp) = va_x9;
           break;
         case 8:
         NV(flute_kline10_cdur) = va_dur9;
           NV(flute_kline10_clp) = NV(flute_kline10_crp);
           NV(flute_kline10_crp) = va_x10;
           break;
         case 9:
         NV(flute_kline10_cdur) = va_dur10;
           NV(flute_kline10_clp) = NV(flute_kline10_crp);
           NV(flute_kline10_crp) = va_x11;
           break;
         case 10:
         NV(flute_kline10_cdur) = va_dur11;
           NV(flute_kline10_clp) = NV(flute_kline10_crp);
           NV(flute_kline10_crp) = va_x12;
           break;
         case 11:
         NV(flute_kline10_cdur) = va_dur12;
           NV(flute_kline10_clp) = NV(flute_kline10_crp);
           NV(flute_kline10_crp) = va_x13;
           break;
         case 12:
         NV(flute_kline10_cdur) = va_dur13;
           NV(flute_kline10_clp) = NV(flute_kline10_crp);
           NV(flute_kline10_crp) = va_x14;
           break;
         case 13:
         NV(flute_kline10_cdur) = va_dur14;
           NV(flute_kline10_clp) = NV(flute_kline10_crp);
           NV(flute_kline10_crp) = va_x15;
           break;
         case 14:
         NV(flute_kline10_cdur) = va_dur15;
           NV(flute_kline10_clp) = NV(flute_kline10_crp);
           NV(flute_kline10_crp) = va_x16;
           break;
         case 15:
         NV(flute_kline10_cdur) = va_dur16;
           NV(flute_kline10_clp) = NV(flute_kline10_crp);
           NV(flute_kline10_crp) = va_x17;
           break;
         case 16:
         NV(flute_kline10_cdur) = va_dur17;
           NV(flute_kline10_clp) = NV(flute_kline10_crp);
           NV(flute_kline10_crp) = va_x18;
           break;
         case 17:
         NV(flute_kline10_cdur) = va_dur18;
           NV(flute_kline10_clp) = NV(flute_kline10_crp);
           NV(flute_kline10_crp) = va_x19;
           break;
         case 18:
         NV(flute_kline10_cdur) = va_dur19;
           NV(flute_kline10_clp) = NV(flute_kline10_crp);
           NV(flute_kline10_crp) = va_x20;
           break;
           default:
           NVI(flute_kline10_first) = -100;
           NV(flute_kline10_cdur) = NV(flute_kline10_t) + 10000.0F;
           break;
           }
         NVI(flute_kline10_first)++;
        }
        NV(flute_kline10_mult)=(NV(flute_kline10_crp) - NV(flute_kline10_clp))/NV(flute_kline10_cdur);
        ret = NV(flute_kline10_outT) = NV(flute_kline10_clp)+NV(flute_kline10_mult)*NV(flute_kline10_t);
        NV(flute_kline10_addK) = NV(flute_kline10_mult)*EV(KTIME);
        if (NVI(flute_kline10_first)<0)
          ret = 0.0F;
      }
   }
   if (NVI(flute_kline10_first)==0)
     {
       NVI(flute_kline10_first) = 1;
       ret = NV(flute_kline10_outT) = NV(flute_kline10_clp) = x1;
       NV(flute_kline10_crp) = x2;
       NV(flute_kline10_cdur) = dur1;
       if (dur1 > 0.0F)
         NV(flute_kline10_addK) = EV(KTIME)*((x2 - x1)/dur1);
     }
   return((NV(flute_kline10_return) = ret));

}



float flute__sym_kline11(ENGINE_PTR_DECLARE_COMMA struct ninstr_types * nstate)
{
   float va_dur2;
   float va_x3;
   float va_dur3;
   float va_x4;
   float va_dur4;
   float va_x5;
   float va_dur5;
   float va_x6;
   float va_dur6;
   float va_x7;
   float va_dur7;
   float va_x8;
   float va_dur8;
   float va_x9;
   float va_dur9;
   float va_x10;
   float va_dur10;
   float va_x11;
   float va_dur11;
   float va_x12;
   float va_dur12;
   float va_x13;
   float va_dur13;
   float va_x14;
   float va_dur14;
   float va_x15;
   float va_dur15;
   float va_x16;
   float va_dur16;
   float va_x17;
   float va_dur17;
   float va_x18;
   float va_dur18;
   float va_x19;
   float va_dur19;
   float va_x20;
   float x2;
   float dur1;
   float x1;
   float ret;

   x1 =  0.0F ;
   dur1 =  0.600000F ;
   x2 =  0.010092F ;
   va_dur2 =  0.705769F  ;
   va_x3 =  0.008370F  ;
   va_dur3 =  0.136538F  ;
   va_x4 =  0.003648F  ;
   va_dur4 =  0.044231F  ;
   va_x5 =  0.007075F  ;
   va_dur5 =  0.065385F  ;
   va_x6 =  0.002992F  ;
   va_dur6 =  0.078846F  ;
   va_x7 =  0.006157F  ;
   va_dur7 =  0.036538F  ;
   va_x8 =  0.003177F  ;
   va_dur8 =  0.007692F  ;
   va_x9 =  0.006989F  ;
   va_dur9 =  0.057692F  ;
   va_x10 =  0.003068F  ;
   va_dur10 =  0.061538F  ;
   va_x11 =  0.002908F  ;
   va_dur11 =  0.176923F  ;
   va_x12 =  0.003112F  ;
   va_dur12 =  0.042308F  ;
   va_x13 =  0.005622F  ;
   va_dur13 =  0.117308F  ;
   va_x14 =  0.002243F  ;
   va_dur14 =  0.003846F  ;
   va_x15 =  0.005415F  ;
   va_dur15 =  0.003846F  ;
   va_x16 =  0.002770F  ;
   va_dur16 =  0.017308F  ;
   va_x17 =  0.005210F  ;
   va_dur17 =  0.015385F  ;
   va_x18 =  0.002411F  ;
   va_dur18 =  0.017308F  ;
   va_x19 =  0.004414F  ;
   va_dur19 =  0.117308F  ;
   va_x20 =  0.000091F  ;
   ret = 0.0F;
   if (NVI(flute_kline11_first)>0)
   {
      NV(flute_kline11_t) += EV(KTIME);
      ret = (NV(flute_kline11_outT) += NV(flute_kline11_addK));
      if (NV(flute_kline11_t) > NV(flute_kline11_cdur))
       {
        while (NV(flute_kline11_t) > NV(flute_kline11_cdur))
         {
           NV(flute_kline11_t) -= NV(flute_kline11_cdur);
           switch(NVI(flute_kline11_first))
      {
         case 1:
         NV(flute_kline11_cdur) = va_dur2;
           NV(flute_kline11_clp) = NV(flute_kline11_crp);
           NV(flute_kline11_crp) = va_x3;
           break;
         case 2:
         NV(flute_kline11_cdur) = va_dur3;
           NV(flute_kline11_clp) = NV(flute_kline11_crp);
           NV(flute_kline11_crp) = va_x4;
           break;
         case 3:
         NV(flute_kline11_cdur) = va_dur4;
           NV(flute_kline11_clp) = NV(flute_kline11_crp);
           NV(flute_kline11_crp) = va_x5;
           break;
         case 4:
         NV(flute_kline11_cdur) = va_dur5;
           NV(flute_kline11_clp) = NV(flute_kline11_crp);
           NV(flute_kline11_crp) = va_x6;
           break;
         case 5:
         NV(flute_kline11_cdur) = va_dur6;
           NV(flute_kline11_clp) = NV(flute_kline11_crp);
           NV(flute_kline11_crp) = va_x7;
           break;
         case 6:
         NV(flute_kline11_cdur) = va_dur7;
           NV(flute_kline11_clp) = NV(flute_kline11_crp);
           NV(flute_kline11_crp) = va_x8;
           break;
         case 7:
         NV(flute_kline11_cdur) = va_dur8;
           NV(flute_kline11_clp) = NV(flute_kline11_crp);
           NV(flute_kline11_crp) = va_x9;
           break;
         case 8:
         NV(flute_kline11_cdur) = va_dur9;
           NV(flute_kline11_clp) = NV(flute_kline11_crp);
           NV(flute_kline11_crp) = va_x10;
           break;
         case 9:
         NV(flute_kline11_cdur) = va_dur10;
           NV(flute_kline11_clp) = NV(flute_kline11_crp);
           NV(flute_kline11_crp) = va_x11;
           break;
         case 10:
         NV(flute_kline11_cdur) = va_dur11;
           NV(flute_kline11_clp) = NV(flute_kline11_crp);
           NV(flute_kline11_crp) = va_x12;
           break;
         case 11:
         NV(flute_kline11_cdur) = va_dur12;
           NV(flute_kline11_clp) = NV(flute_kline11_crp);
           NV(flute_kline11_crp) = va_x13;
           break;
         case 12:
         NV(flute_kline11_cdur) = va_dur13;
           NV(flute_kline11_clp) = NV(flute_kline11_crp);
           NV(flute_kline11_crp) = va_x14;
           break;
         case 13:
         NV(flute_kline11_cdur) = va_dur14;
           NV(flute_kline11_clp) = NV(flute_kline11_crp);
           NV(flute_kline11_crp) = va_x15;
           break;
         case 14:
         NV(flute_kline11_cdur) = va_dur15;
           NV(flute_kline11_clp) = NV(flute_kline11_crp);
           NV(flute_kline11_crp) = va_x16;
           break;
         case 15:
         NV(flute_kline11_cdur) = va_dur16;
           NV(flute_kline11_clp) = NV(flute_kline11_crp);
           NV(flute_kline11_crp) = va_x17;
           break;
         case 16:
         NV(flute_kline11_cdur) = va_dur17;
           NV(flute_kline11_clp) = NV(flute_kline11_crp);
           NV(flute_kline11_crp) = va_x18;
           break;
         case 17:
         NV(flute_kline11_cdur) = va_dur18;
           NV(flute_kline11_clp) = NV(flute_kline11_crp);
           NV(flute_kline11_crp) = va_x19;
           break;
         case 18:
         NV(flute_kline11_cdur) = va_dur19;
           NV(flute_kline11_clp) = NV(flute_kline11_crp);
           NV(flute_kline11_crp) = va_x20;
           break;
           default:
           NVI(flute_kline11_first) = -100;
           NV(flute_kline11_cdur) = NV(flute_kline11_t) + 10000.0F;
           break;
           }
         NVI(flute_kline11_first)++;
        }
        NV(flute_kline11_mult)=(NV(flute_kline11_crp) - NV(flute_kline11_clp))/NV(flute_kline11_cdur);
        ret = NV(flute_kline11_outT) = NV(flute_kline11_clp)+NV(flute_kline11_mult)*NV(flute_kline11_t);
        NV(flute_kline11_addK) = NV(flute_kline11_mult)*EV(KTIME);
        if (NVI(flute_kline11_first)<0)
          ret = 0.0F;
      }
   }
   if (NVI(flute_kline11_first)==0)
     {
       NVI(flute_kline11_first) = 1;
       ret = NV(flute_kline11_outT) = NV(flute_kline11_clp) = x1;
       NV(flute_kline11_crp) = x2;
       NV(flute_kline11_cdur) = dur1;
       if (dur1 > 0.0F)
         NV(flute_kline11_addK) = EV(KTIME)*((x2 - x1)/dur1);
     }
   return((NV(flute_kline11_return) = ret));

}



float flute__sym_kline12(ENGINE_PTR_DECLARE_COMMA struct ninstr_types * nstate)
{
   float va_dur2;
   float va_x3;
   float va_dur3;
   float va_x4;
   float va_dur4;
   float va_x5;
   float va_dur5;
   float va_x6;
   float va_dur6;
   float va_x7;
   float va_dur7;
   float va_x8;
   float va_dur8;
   float va_x9;
   float va_dur9;
   float va_x10;
   float va_dur10;
   float va_x11;
   float va_dur11;
   float va_x12;
   float va_dur12;
   float va_x13;
   float va_dur13;
   float va_x14;
   float va_dur14;
   float va_x15;
   float va_dur15;
   float va_x16;
   float va_dur16;
   float va_x17;
   float va_dur17;
   float va_x18;
   float va_dur18;
   float va_x19;
   float va_dur19;
   float va_x20;
   float x2;
   float dur1;
   float x1;
   float ret;

   x1 =  0.0F ;
   dur1 =  0.800000F ;
   x2 =  0.007518F ;
   va_dur2 =  1.009615F  ;
   va_x3 =  0.003943F  ;
   va_dur3 =  0.203846F  ;
   va_x4 =  0.002497F  ;
   va_dur4 =  0.011538F  ;
   va_x5 =  0.004112F  ;
   va_dur5 =  0.026923F  ;
   va_x6 =  0.001800F  ;
   va_dur6 =  0.038462F  ;
   va_x7 =  0.003358F  ;
   va_dur7 =  0.032692F  ;
   va_x8 =  0.001573F  ;
   va_dur8 =  0.003846F  ;
   va_x9 =  0.003125F  ;
   va_dur9 =  0.059615F  ;
   va_x10 =  0.001702F  ;
   va_dur10 =  0.005769F  ;
   va_x11 =  0.003526F  ;
   va_dur11 =  0.088462F  ;
   va_x12 =  0.000142F  ;
   va_dur12 =  0.007692F  ;
   va_x13 =  0.000091F  ;
   va_dur13 =  0.005769F  ;
   va_x14 =  0.000030F  ;
   va_dur14 =  0.001923F  ;
   va_x15 =  0.000044F  ;
   va_dur15 =  0.001923F  ;
   va_x16 =  0.000051F  ;
   va_dur16 =  0.001923F  ;
   va_x17 =  0.000077F  ;
   va_dur17 =  0.003846F  ;
   va_x18 =  0.000068F  ;
   va_dur18 =  0.001923F  ;
   va_x19 =  0.000038F  ;
   va_dur19 =  0.0F  ;
   va_x20 =  0.000038F  ;
   ret = 0.0F;
   if (NVI(flute_kline12_first)>0)
   {
      NV(flute_kline12_t) += EV(KTIME);
      ret = (NV(flute_kline12_outT) += NV(flute_kline12_addK));
      if (NV(flute_kline12_t) > NV(flute_kline12_cdur))
       {
        while (NV(flute_kline12_t) > NV(flute_kline12_cdur))
         {
           NV(flute_kline12_t) -= NV(flute_kline12_cdur);
           switch(NVI(flute_kline12_first))
      {
         case 1:
         NV(flute_kline12_cdur) = va_dur2;
           NV(flute_kline12_clp) = NV(flute_kline12_crp);
           NV(flute_kline12_crp) = va_x3;
           break;
         case 2:
         NV(flute_kline12_cdur) = va_dur3;
           NV(flute_kline12_clp) = NV(flute_kline12_crp);
           NV(flute_kline12_crp) = va_x4;
           break;
         case 3:
         NV(flute_kline12_cdur) = va_dur4;
           NV(flute_kline12_clp) = NV(flute_kline12_crp);
           NV(flute_kline12_crp) = va_x5;
           break;
         case 4:
         NV(flute_kline12_cdur) = va_dur5;
           NV(flute_kline12_clp) = NV(flute_kline12_crp);
           NV(flute_kline12_crp) = va_x6;
           break;
         case 5:
         NV(flute_kline12_cdur) = va_dur6;
           NV(flute_kline12_clp) = NV(flute_kline12_crp);
           NV(flute_kline12_crp) = va_x7;
           break;
         case 6:
         NV(flute_kline12_cdur) = va_dur7;
           NV(flute_kline12_clp) = NV(flute_kline12_crp);
           NV(flute_kline12_crp) = va_x8;
           break;
         case 7:
         NV(flute_kline12_cdur) = va_dur8;
           NV(flute_kline12_clp) = NV(flute_kline12_crp);
           NV(flute_kline12_crp) = va_x9;
           break;
         case 8:
         NV(flute_kline12_cdur) = va_dur9;
           NV(flute_kline12_clp) = NV(flute_kline12_crp);
           NV(flute_kline12_crp) = va_x10;
           break;
         case 9:
         NV(flute_kline12_cdur) = va_dur10;
           NV(flute_kline12_clp) = NV(flute_kline12_crp);
           NV(flute_kline12_crp) = va_x11;
           break;
         case 10:
         NV(flute_kline12_cdur) = va_dur11;
           NV(flute_kline12_clp) = NV(flute_kline12_crp);
           NV(flute_kline12_crp) = va_x12;
           break;
         case 11:
         NV(flute_kline12_cdur) = va_dur12;
           NV(flute_kline12_clp) = NV(flute_kline12_crp);
           NV(flute_kline12_crp) = va_x13;
           break;
         case 12:
         NV(flute_kline12_cdur) = va_dur13;
           NV(flute_kline12_clp) = NV(flute_kline12_crp);
           NV(flute_kline12_crp) = va_x14;
           break;
         case 13:
         NV(flute_kline12_cdur) = va_dur14;
           NV(flute_kline12_clp) = NV(flute_kline12_crp);
           NV(flute_kline12_crp) = va_x15;
           break;
         case 14:
         NV(flute_kline12_cdur) = va_dur15;
           NV(flute_kline12_clp) = NV(flute_kline12_crp);
           NV(flute_kline12_crp) = va_x16;
           break;
         case 15:
         NV(flute_kline12_cdur) = va_dur16;
           NV(flute_kline12_clp) = NV(flute_kline12_crp);
           NV(flute_kline12_crp) = va_x17;
           break;
         case 16:
         NV(flute_kline12_cdur) = va_dur17;
           NV(flute_kline12_clp) = NV(flute_kline12_crp);
           NV(flute_kline12_crp) = va_x18;
           break;
         case 17:
         NV(flute_kline12_cdur) = va_dur18;
           NV(flute_kline12_clp) = NV(flute_kline12_crp);
           NV(flute_kline12_crp) = va_x19;
           break;
         case 18:
         NV(flute_kline12_cdur) = va_dur19;
           NV(flute_kline12_clp) = NV(flute_kline12_crp);
           NV(flute_kline12_crp) = va_x20;
           break;
           default:
           NVI(flute_kline12_first) = -100;
           NV(flute_kline12_cdur) = NV(flute_kline12_t) + 10000.0F;
           break;
           }
         NVI(flute_kline12_first)++;
        }
        NV(flute_kline12_mult)=(NV(flute_kline12_crp) - NV(flute_kline12_clp))/NV(flute_kline12_cdur);
        ret = NV(flute_kline12_outT) = NV(flute_kline12_clp)+NV(flute_kline12_mult)*NV(flute_kline12_t);
        NV(flute_kline12_addK) = NV(flute_kline12_mult)*EV(KTIME);
        if (NVI(flute_kline12_first)<0)
          ret = 0.0F;
      }
   }
   if (NVI(flute_kline12_first)==0)
     {
       NVI(flute_kline12_first) = 1;
       ret = NV(flute_kline12_outT) = NV(flute_kline12_clp) = x1;
       NV(flute_kline12_crp) = x2;
       NV(flute_kline12_cdur) = dur1;
       if (dur1 > 0.0F)
         NV(flute_kline12_addK) = EV(KTIME)*((x2 - x1)/dur1);
     }
   return((NV(flute_kline12_return) = ret));

}



float flute__sym_kline13(ENGINE_PTR_DECLARE_COMMA struct ninstr_types * nstate)
{
   float va_dur2;
   float va_x3;
   float va_dur3;
   float va_x4;
   float va_dur4;
   float va_x5;
   float va_dur5;
   float va_x6;
   float va_dur6;
   float va_x7;
   float va_dur7;
   float va_x8;
   float va_dur8;
   float va_x9;
   float va_dur9;
   float va_x10;
   float va_dur10;
   float va_x11;
   float va_dur11;
   float va_x12;
   float va_dur12;
   float va_x13;
   float va_dur13;
   float va_x14;
   float va_dur14;
   float va_x15;
   float va_dur15;
   float va_x16;
   float va_dur16;
   float va_x17;
   float va_dur17;
   float va_x18;
   float va_dur18;
   float va_x19;
   float va_dur19;
   float va_x20;
   float x2;
   float dur1;
   float x1;
   float ret;

   x1 =  0.0F ;
   dur1 =  0.596154F ;
   x2 =  0.005469F ;
   va_dur2 =  1.151923F  ;
   va_x3 =  0.002501F  ;
   va_dur3 =  0.228846F  ;
   va_x4 =  0.001642F  ;
   va_dur4 =  0.013462F  ;
   va_x5 =  0.002939F  ;
   va_dur5 =  0.098077F  ;
   va_x6 =  0.001027F  ;
   va_dur6 =  0.028846F  ;
   va_x7 =  0.001479F  ;
   va_dur7 =  0.013462F  ;
   va_x8 =  0.002449F  ;
   va_dur8 =  0.015385F  ;
   va_x9 =  0.000816F  ;
   va_dur9 =  0.005769F  ;
   va_x10 =  0.002399F  ;
   va_dur10 =  0.001923F  ;
   va_x11 =  0.001424F  ;
   va_dur11 =  0.013462F  ;
   va_x12 =  0.001086F  ;
   va_dur12 =  0.007692F  ;
   va_x13 =  0.001845F  ;
   va_dur13 =  0.005769F  ;
   va_x14 =  0.000592F  ;
   va_dur14 =  0.005769F  ;
   va_x15 =  0.000634F  ;
   va_dur15 =  0.011538F  ;
   va_x16 =  0.000838F  ;
   va_dur16 =  0.005769F  ;
   va_x17 =  0.001480F  ;
   va_dur17 =  0.038462F  ;
   va_x18 =  0.000428F  ;
   va_dur18 =  0.042308F  ;
   va_x19 =  0.000147F  ;
   va_dur19 =  0.021154F  ;
   va_x20 =  0.000075F  ;
   ret = 0.0F;
   if (NVI(flute_kline13_first)>0)
   {
      NV(flute_kline13_t) += EV(KTIME);
      ret = (NV(flute_kline13_outT) += NV(flute_kline13_addK));
      if (NV(flute_kline13_t) > NV(flute_kline13_cdur))
       {
        while (NV(flute_kline13_t) > NV(flute_kline13_cdur))
         {
           NV(flute_kline13_t) -= NV(flute_kline13_cdur);
           switch(NVI(flute_kline13_first))
      {
         case 1:
         NV(flute_kline13_cdur) = va_dur2;
           NV(flute_kline13_clp) = NV(flute_kline13_crp);
           NV(flute_kline13_crp) = va_x3;
           break;
         case 2:
         NV(flute_kline13_cdur) = va_dur3;
           NV(flute_kline13_clp) = NV(flute_kline13_crp);
           NV(flute_kline13_crp) = va_x4;
           break;
         case 3:
         NV(flute_kline13_cdur) = va_dur4;
           NV(flute_kline13_clp) = NV(flute_kline13_crp);
           NV(flute_kline13_crp) = va_x5;
           break;
         case 4:
         NV(flute_kline13_cdur) = va_dur5;
           NV(flute_kline13_clp) = NV(flute_kline13_crp);
           NV(flute_kline13_crp) = va_x6;
           break;
         case 5:
         NV(flute_kline13_cdur) = va_dur6;
           NV(flute_kline13_clp) = NV(flute_kline13_crp);
           NV(flute_kline13_crp) = va_x7;
           break;
         case 6:
         NV(flute_kline13_cdur) = va_dur7;
           NV(flute_kline13_clp) = NV(flute_kline13_crp);
           NV(flute_kline13_crp) = va_x8;
           break;
         case 7:
         NV(flute_kline13_cdur) = va_dur8;
           NV(flute_kline13_clp) = NV(flute_kline13_crp);
           NV(flute_kline13_crp) = va_x9;
           break;
         case 8:
         NV(flute_kline13_cdur) = va_dur9;
           NV(flute_kline13_clp) = NV(flute_kline13_crp);
           NV(flute_kline13_crp) = va_x10;
           break;
         case 9:
         NV(flute_kline13_cdur) = va_dur10;
           NV(flute_kline13_clp) = NV(flute_kline13_crp);
           NV(flute_kline13_crp) = va_x11;
           break;
         case 10:
         NV(flute_kline13_cdur) = va_dur11;
           NV(flute_kline13_clp) = NV(flute_kline13_crp);
           NV(flute_kline13_crp) = va_x12;
           break;
         case 11:
         NV(flute_kline13_cdur) = va_dur12;
           NV(flute_kline13_clp) = NV(flute_kline13_crp);
           NV(flute_kline13_crp) = va_x13;
           break;
         case 12:
         NV(flute_kline13_cdur) = va_dur13;
           NV(flute_kline13_clp) = NV(flute_kline13_crp);
           NV(flute_kline13_crp) = va_x14;
           break;
         case 13:
         NV(flute_kline13_cdur) = va_dur14;
           NV(flute_kline13_clp) = NV(flute_kline13_crp);
           NV(flute_kline13_crp) = va_x15;
           break;
         case 14:
         NV(flute_kline13_cdur) = va_dur15;
           NV(flute_kline13_clp) = NV(flute_kline13_crp);
           NV(flute_kline13_crp) = va_x16;
           break;
         case 15:
         NV(flute_kline13_cdur) = va_dur16;
           NV(flute_kline13_clp) = NV(flute_kline13_crp);
           NV(flute_kline13_crp) = va_x17;
           break;
         case 16:
         NV(flute_kline13_cdur) = va_dur17;
           NV(flute_kline13_clp) = NV(flute_kline13_crp);
           NV(flute_kline13_crp) = va_x18;
           break;
         case 17:
         NV(flute_kline13_cdur) = va_dur18;
           NV(flute_kline13_clp) = NV(flute_kline13_crp);
           NV(flute_kline13_crp) = va_x19;
           break;
         case 18:
         NV(flute_kline13_cdur) = va_dur19;
           NV(flute_kline13_clp) = NV(flute_kline13_crp);
           NV(flute_kline13_crp) = va_x20;
           break;
           default:
           NVI(flute_kline13_first) = -100;
           NV(flute_kline13_cdur) = NV(flute_kline13_t) + 10000.0F;
           break;
           }
         NVI(flute_kline13_first)++;
        }
        NV(flute_kline13_mult)=(NV(flute_kline13_crp) - NV(flute_kline13_clp))/NV(flute_kline13_cdur);
        ret = NV(flute_kline13_outT) = NV(flute_kline13_clp)+NV(flute_kline13_mult)*NV(flute_kline13_t);
        NV(flute_kline13_addK) = NV(flute_kline13_mult)*EV(KTIME);
        if (NVI(flute_kline13_first)<0)
          ret = 0.0F;
      }
   }
   if (NVI(flute_kline13_first)==0)
     {
       NVI(flute_kline13_first) = 1;
       ret = NV(flute_kline13_outT) = NV(flute_kline13_clp) = x1;
       NV(flute_kline13_crp) = x2;
       NV(flute_kline13_cdur) = dur1;
       if (dur1 > 0.0F)
         NV(flute_kline13_addK) = EV(KTIME)*((x2 - x1)/dur1);
     }
   return((NV(flute_kline13_return) = ret));

}



float flute__sym_kline14(ENGINE_PTR_DECLARE_COMMA struct ninstr_types * nstate)
{
   float va_dur2;
   float va_x3;
   float va_dur3;
   float va_x4;
   float va_dur4;
   float va_x5;
   float va_dur5;
   float va_x6;
   float va_dur6;
   float va_x7;
   float va_dur7;
   float va_x8;
   float va_dur8;
   float va_x9;
   float va_dur9;
   float va_x10;
   float va_dur10;
   float va_x11;
   float va_dur11;
   float va_x12;
   float va_dur12;
   float va_x13;
   float va_dur13;
   float va_x14;
   float va_dur14;
   float va_x15;
   float va_dur15;
   float va_x16;
   float va_dur16;
   float va_x17;
   float va_dur17;
   float va_x18;
   float va_dur18;
   float va_x19;
   float va_dur19;
   float va_x20;
   float x2;
   float dur1;
   float x1;
   float ret;

   x1 =  0.0F ;
   dur1 =  0.625000F ;
   x2 =  0.003927F ;
   va_dur2 =  1.009615F  ;
   va_x3 =  0.003068F  ;
   va_dur3 =  0.380769F  ;
   va_x4 =  0.002379F  ;
   va_dur4 =  0.036538F  ;
   va_x5 =  0.001028F  ;
   va_dur5 =  0.030769F  ;
   va_x6 =  0.002257F  ;
   va_dur6 =  0.051923F  ;
   va_x7 =  0.000917F  ;
   va_dur7 =  0.001923F  ;
   va_x8 =  0.001498F  ;
   va_dur8 =  0.001923F  ;
   va_x9 =  0.001021F  ;
   va_dur9 =  0.009615F  ;
   va_x10 =  0.001988F  ;
   va_dur10 =  0.001923F  ;
   va_x11 =  0.000869F  ;
   va_dur11 =  0.003846F  ;
   va_x12 =  0.001233F  ;
   va_dur12 =  0.005769F  ;
   va_x13 =  0.002169F  ;
   va_dur13 =  0.001923F  ;
   va_x14 =  0.001128F  ;
   va_dur14 =  0.003846F  ;
   va_x15 =  0.002026F  ;
   va_dur15 =  0.003846F  ;
   va_x16 =  0.001271F  ;
   va_dur16 =  0.005769F  ;
   va_x17 =  0.001879F  ;
   va_dur17 =  0.003846F  ;
   va_x18 =  0.000833F  ;
   va_dur18 =  0.003846F  ;
   va_x19 =  0.001859F  ;
   va_dur19 =  0.123077F  ;
   va_x20 =  0.000012F  ;
   ret = 0.0F;
   if (NVI(flute_kline14_first)>0)
   {
      NV(flute_kline14_t) += EV(KTIME);
      ret = (NV(flute_kline14_outT) += NV(flute_kline14_addK));
      if (NV(flute_kline14_t) > NV(flute_kline14_cdur))
       {
        while (NV(flute_kline14_t) > NV(flute_kline14_cdur))
         {
           NV(flute_kline14_t) -= NV(flute_kline14_cdur);
           switch(NVI(flute_kline14_first))
      {
         case 1:
         NV(flute_kline14_cdur) = va_dur2;
           NV(flute_kline14_clp) = NV(flute_kline14_crp);
           NV(flute_kline14_crp) = va_x3;
           break;
         case 2:
         NV(flute_kline14_cdur) = va_dur3;
           NV(flute_kline14_clp) = NV(flute_kline14_crp);
           NV(flute_kline14_crp) = va_x4;
           break;
         case 3:
         NV(flute_kline14_cdur) = va_dur4;
           NV(flute_kline14_clp) = NV(flute_kline14_crp);
           NV(flute_kline14_crp) = va_x5;
           break;
         case 4:
         NV(flute_kline14_cdur) = va_dur5;
           NV(flute_kline14_clp) = NV(flute_kline14_crp);
           NV(flute_kline14_crp) = va_x6;
           break;
         case 5:
         NV(flute_kline14_cdur) = va_dur6;
           NV(flute_kline14_clp) = NV(flute_kline14_crp);
           NV(flute_kline14_crp) = va_x7;
           break;
         case 6:
         NV(flute_kline14_cdur) = va_dur7;
           NV(flute_kline14_clp) = NV(flute_kline14_crp);
           NV(flute_kline14_crp) = va_x8;
           break;
         case 7:
         NV(flute_kline14_cdur) = va_dur8;
           NV(flute_kline14_clp) = NV(flute_kline14_crp);
           NV(flute_kline14_crp) = va_x9;
           break;
         case 8:
         NV(flute_kline14_cdur) = va_dur9;
           NV(flute_kline14_clp) = NV(flute_kline14_crp);
           NV(flute_kline14_crp) = va_x10;
           break;
         case 9:
         NV(flute_kline14_cdur) = va_dur10;
           NV(flute_kline14_clp) = NV(flute_kline14_crp);
           NV(flute_kline14_crp) = va_x11;
           break;
         case 10:
         NV(flute_kline14_cdur) = va_dur11;
           NV(flute_kline14_clp) = NV(flute_kline14_crp);
           NV(flute_kline14_crp) = va_x12;
           break;
         case 11:
         NV(flute_kline14_cdur) = va_dur12;
           NV(flute_kline14_clp) = NV(flute_kline14_crp);
           NV(flute_kline14_crp) = va_x13;
           break;
         case 12:
         NV(flute_kline14_cdur) = va_dur13;
           NV(flute_kline14_clp) = NV(flute_kline14_crp);
           NV(flute_kline14_crp) = va_x14;
           break;
         case 13:
         NV(flute_kline14_cdur) = va_dur14;
           NV(flute_kline14_clp) = NV(flute_kline14_crp);
           NV(flute_kline14_crp) = va_x15;
           break;
         case 14:
         NV(flute_kline14_cdur) = va_dur15;
           NV(flute_kline14_clp) = NV(flute_kline14_crp);
           NV(flute_kline14_crp) = va_x16;
           break;
         case 15:
         NV(flute_kline14_cdur) = va_dur16;
           NV(flute_kline14_clp) = NV(flute_kline14_crp);
           NV(flute_kline14_crp) = va_x17;
           break;
         case 16:
         NV(flute_kline14_cdur) = va_dur17;
           NV(flute_kline14_clp) = NV(flute_kline14_crp);
           NV(flute_kline14_crp) = va_x18;
           break;
         case 17:
         NV(flute_kline14_cdur) = va_dur18;
           NV(flute_kline14_clp) = NV(flute_kline14_crp);
           NV(flute_kline14_crp) = va_x19;
           break;
         case 18:
         NV(flute_kline14_cdur) = va_dur19;
           NV(flute_kline14_clp) = NV(flute_kline14_crp);
           NV(flute_kline14_crp) = va_x20;
           break;
           default:
           NVI(flute_kline14_first) = -100;
           NV(flute_kline14_cdur) = NV(flute_kline14_t) + 10000.0F;
           break;
           }
         NVI(flute_kline14_first)++;
        }
        NV(flute_kline14_mult)=(NV(flute_kline14_crp) - NV(flute_kline14_clp))/NV(flute_kline14_cdur);
        ret = NV(flute_kline14_outT) = NV(flute_kline14_clp)+NV(flute_kline14_mult)*NV(flute_kline14_t);
        NV(flute_kline14_addK) = NV(flute_kline14_mult)*EV(KTIME);
        if (NVI(flute_kline14_first)<0)
          ret = 0.0F;
      }
   }
   if (NVI(flute_kline14_first)==0)
     {
       NVI(flute_kline14_first) = 1;
       ret = NV(flute_kline14_outT) = NV(flute_kline14_clp) = x1;
       NV(flute_kline14_crp) = x2;
       NV(flute_kline14_cdur) = dur1;
       if (dur1 > 0.0F)
         NV(flute_kline14_addK) = EV(KTIME)*((x2 - x1)/dur1);
     }
   return((NV(flute_kline14_return) = ret));

}



float flute__sym_kline15(ENGINE_PTR_DECLARE_COMMA struct ninstr_types * nstate)
{
   float va_dur2;
   float va_x3;
   float va_dur3;
   float va_x4;
   float va_dur4;
   float va_x5;
   float va_dur5;
   float va_x6;
   float va_dur6;
   float va_x7;
   float va_dur7;
   float va_x8;
   float va_dur8;
   float va_x9;
   float va_dur9;
   float va_x10;
   float va_dur10;
   float va_x11;
   float va_dur11;
   float va_x12;
   float va_dur12;
   float va_x13;
   float va_dur13;
   float va_x14;
   float va_dur14;
   float va_x15;
   float va_dur15;
   float va_x16;
   float va_dur16;
   float va_x17;
   float va_dur17;
   float va_x18;
   float va_dur18;
   float va_x19;
   float va_dur19;
   float va_x20;
   float x2;
   float dur1;
   float x1;
   float ret;

   x1 =  0.0F ;
   dur1 =  0.801923F ;
   x2 =  0.004476F ;
   va_dur2 =  0.840385F  ;
   va_x3 =  0.003504F  ;
   va_dur3 =  0.217308F  ;
   va_x4 =  0.001374F  ;
   va_dur4 =  0.119231F  ;
   va_x5 =  0.001838F  ;
   va_dur5 =  0.046154F  ;
   va_x6 =  0.002878F  ;
   va_dur6 =  0.155769F  ;
   va_x7 =  0.000431F  ;
   va_dur7 =  0.007692F  ;
   va_x8 =  0.001010F  ;
   va_dur8 =  0.007692F  ;
   va_x9 =  0.001186F  ;
   va_dur9 =  0.005769F  ;
   va_x10 =  0.002135F  ;
   va_dur10 =  0.005769F  ;
   va_x11 =  0.001198F  ;
   va_dur11 =  0.034615F  ;
   va_x12 =  0.000303F  ;
   va_dur12 =  0.017308F  ;
   va_x13 =  0.000206F  ;
   va_dur13 =  0.007692F  ;
   va_x14 =  0.000177F  ;
   va_dur14 =  0.011538F  ;
   va_x15 =  0.000093F  ;
   va_dur15 =  0.005769F  ;
   va_x16 =  0.000089F  ;
   va_dur16 =  0.007692F  ;
   va_x17 =  0.000066F  ;
   va_dur17 =  0.003846F  ;
   va_x18 =  0.000081F  ;
   va_dur18 =  0.009615F  ;
   va_x19 =  0.000068F  ;
   va_dur19 =  0.0F  ;
   va_x20 =  0.000068F  ;
   ret = 0.0F;
   if (NVI(flute_kline15_first)>0)
   {
      NV(flute_kline15_t) += EV(KTIME);
      ret = (NV(flute_kline15_outT) += NV(flute_kline15_addK));
      if (NV(flute_kline15_t) > NV(flute_kline15_cdur))
       {
        while (NV(flute_kline15_t) > NV(flute_kline15_cdur))
         {
           NV(flute_kline15_t) -= NV(flute_kline15_cdur);
           switch(NVI(flute_kline15_first))
      {
         case 1:
         NV(flute_kline15_cdur) = va_dur2;
           NV(flute_kline15_clp) = NV(flute_kline15_crp);
           NV(flute_kline15_crp) = va_x3;
           break;
         case 2:
         NV(flute_kline15_cdur) = va_dur3;
           NV(flute_kline15_clp) = NV(flute_kline15_crp);
           NV(flute_kline15_crp) = va_x4;
           break;
         case 3:
         NV(flute_kline15_cdur) = va_dur4;
           NV(flute_kline15_clp) = NV(flute_kline15_crp);
           NV(flute_kline15_crp) = va_x5;
           break;
         case 4:
         NV(flute_kline15_cdur) = va_dur5;
           NV(flute_kline15_clp) = NV(flute_kline15_crp);
           NV(flute_kline15_crp) = va_x6;
           break;
         case 5:
         NV(flute_kline15_cdur) = va_dur6;
           NV(flute_kline15_clp) = NV(flute_kline15_crp);
           NV(flute_kline15_crp) = va_x7;
           break;
         case 6:
         NV(flute_kline15_cdur) = va_dur7;
           NV(flute_kline15_clp) = NV(flute_kline15_crp);
           NV(flute_kline15_crp) = va_x8;
           break;
         case 7:
         NV(flute_kline15_cdur) = va_dur8;
           NV(flute_kline15_clp) = NV(flute_kline15_crp);
           NV(flute_kline15_crp) = va_x9;
           break;
         case 8:
         NV(flute_kline15_cdur) = va_dur9;
           NV(flute_kline15_clp) = NV(flute_kline15_crp);
           NV(flute_kline15_crp) = va_x10;
           break;
         case 9:
         NV(flute_kline15_cdur) = va_dur10;
           NV(flute_kline15_clp) = NV(flute_kline15_crp);
           NV(flute_kline15_crp) = va_x11;
           break;
         case 10:
         NV(flute_kline15_cdur) = va_dur11;
           NV(flute_kline15_clp) = NV(flute_kline15_crp);
           NV(flute_kline15_crp) = va_x12;
           break;
         case 11:
         NV(flute_kline15_cdur) = va_dur12;
           NV(flute_kline15_clp) = NV(flute_kline15_crp);
           NV(flute_kline15_crp) = va_x13;
           break;
         case 12:
         NV(flute_kline15_cdur) = va_dur13;
           NV(flute_kline15_clp) = NV(flute_kline15_crp);
           NV(flute_kline15_crp) = va_x14;
           break;
         case 13:
         NV(flute_kline15_cdur) = va_dur14;
           NV(flute_kline15_clp) = NV(flute_kline15_crp);
           NV(flute_kline15_crp) = va_x15;
           break;
         case 14:
         NV(flute_kline15_cdur) = va_dur15;
           NV(flute_kline15_clp) = NV(flute_kline15_crp);
           NV(flute_kline15_crp) = va_x16;
           break;
         case 15:
         NV(flute_kline15_cdur) = va_dur16;
           NV(flute_kline15_clp) = NV(flute_kline15_crp);
           NV(flute_kline15_crp) = va_x17;
           break;
         case 16:
         NV(flute_kline15_cdur) = va_dur17;
           NV(flute_kline15_clp) = NV(flute_kline15_crp);
           NV(flute_kline15_crp) = va_x18;
           break;
         case 17:
         NV(flute_kline15_cdur) = va_dur18;
           NV(flute_kline15_clp) = NV(flute_kline15_crp);
           NV(flute_kline15_crp) = va_x19;
           break;
         case 18:
         NV(flute_kline15_cdur) = va_dur19;
           NV(flute_kline15_clp) = NV(flute_kline15_crp);
           NV(flute_kline15_crp) = va_x20;
           break;
           default:
           NVI(flute_kline15_first) = -100;
           NV(flute_kline15_cdur) = NV(flute_kline15_t) + 10000.0F;
           break;
           }
         NVI(flute_kline15_first)++;
        }
        NV(flute_kline15_mult)=(NV(flute_kline15_crp) - NV(flute_kline15_clp))/NV(flute_kline15_cdur);
        ret = NV(flute_kline15_outT) = NV(flute_kline15_clp)+NV(flute_kline15_mult)*NV(flute_kline15_t);
        NV(flute_kline15_addK) = NV(flute_kline15_mult)*EV(KTIME);
        if (NVI(flute_kline15_first)<0)
          ret = 0.0F;
      }
   }
   if (NVI(flute_kline15_first)==0)
     {
       NVI(flute_kline15_first) = 1;
       ret = NV(flute_kline15_outT) = NV(flute_kline15_clp) = x1;
       NV(flute_kline15_crp) = x2;
       NV(flute_kline15_cdur) = dur1;
       if (dur1 > 0.0F)
         NV(flute_kline15_addK) = EV(KTIME)*((x2 - x1)/dur1);
     }
   return((NV(flute_kline15_return) = ret));

}



float flute__sym_kline16(ENGINE_PTR_DECLARE_COMMA struct ninstr_types * nstate)
{
   float va_dur2;
   float va_x3;
   float va_dur3;
   float va_x4;
   float va_dur4;
   float va_x5;
   float va_dur5;
   float va_x6;
   float va_dur6;
   float va_x7;
   float va_dur7;
   float va_x8;
   float va_dur8;
   float va_x9;
   float va_dur9;
   float va_x10;
   float va_dur10;
   float va_x11;
   float va_dur11;
   float va_x12;
   float va_dur12;
   float va_x13;
   float va_dur13;
   float va_x14;
   float va_dur14;
   float va_x15;
   float va_dur15;
   float va_x16;
   float va_dur16;
   float va_x17;
   float va_dur17;
   float va_x18;
   float va_dur18;
   float va_x19;
   float va_dur19;
   float va_x20;
   float x2;
   float dur1;
   float x1;
   float ret;

   x1 =  0.0F ;
   dur1 =  0.425000F ;
   x2 =  0.002606F ;
   va_dur2 =  0.719231F  ;
   va_x3 =  0.002413F  ;
   va_dur3 =  0.815385F  ;
   va_x4 =  0.001641F  ;
   va_dur4 =  0.184615F  ;
   va_x5 =  0.000917F  ;
   va_dur5 =  0.001923F  ;
   va_x6 =  0.000806F  ;
   va_dur6 =  0.021154F  ;
   va_x7 =  0.001132F  ;
   va_dur7 =  0.011538F  ;
   va_x8 =  0.000628F  ;
   va_dur8 =  0.001923F  ;
   va_x9 =  0.000954F  ;
   va_dur9 =  0.001923F  ;
   va_x10 =  0.000633F  ;
   va_dur10 =  0.001923F  ;
   va_x11 =  0.000542F  ;
   va_dur11 =  0.001923F  ;
   va_x12 =  0.000672F  ;
   va_dur12 =  0.001923F  ;
   va_x13 =  0.001025F  ;
   va_dur13 =  0.001923F  ;
   va_x14 =  0.000648F  ;
   va_dur14 =  0.001923F  ;
   va_x15 =  0.000571F  ;
   va_dur15 =  0.001923F  ;
   va_x16 =  0.000557F  ;
   va_dur16 =  0.003846F  ;
   va_x17 =  0.000760F  ;
   va_dur17 =  0.001923F  ;
   va_x18 =  0.000946F  ;
   va_dur18 =  0.001923F  ;
   va_x19 =  0.000797F  ;
   va_dur19 =  0.103846F  ;
   va_x20 =  0.000032F  ;
   ret = 0.0F;
   if (NVI(flute_kline16_first)>0)
   {
      NV(flute_kline16_t) += EV(KTIME);
      ret = (NV(flute_kline16_outT) += NV(flute_kline16_addK));
      if (NV(flute_kline16_t) > NV(flute_kline16_cdur))
       {
        while (NV(flute_kline16_t) > NV(flute_kline16_cdur))
         {
           NV(flute_kline16_t) -= NV(flute_kline16_cdur);
           switch(NVI(flute_kline16_first))
      {
         case 1:
         NV(flute_kline16_cdur) = va_dur2;
           NV(flute_kline16_clp) = NV(flute_kline16_crp);
           NV(flute_kline16_crp) = va_x3;
           break;
         case 2:
         NV(flute_kline16_cdur) = va_dur3;
           NV(flute_kline16_clp) = NV(flute_kline16_crp);
           NV(flute_kline16_crp) = va_x4;
           break;
         case 3:
         NV(flute_kline16_cdur) = va_dur4;
           NV(flute_kline16_clp) = NV(flute_kline16_crp);
           NV(flute_kline16_crp) = va_x5;
           break;
         case 4:
         NV(flute_kline16_cdur) = va_dur5;
           NV(flute_kline16_clp) = NV(flute_kline16_crp);
           NV(flute_kline16_crp) = va_x6;
           break;
         case 5:
         NV(flute_kline16_cdur) = va_dur6;
           NV(flute_kline16_clp) = NV(flute_kline16_crp);
           NV(flute_kline16_crp) = va_x7;
           break;
         case 6:
         NV(flute_kline16_cdur) = va_dur7;
           NV(flute_kline16_clp) = NV(flute_kline16_crp);
           NV(flute_kline16_crp) = va_x8;
           break;
         case 7:
         NV(flute_kline16_cdur) = va_dur8;
           NV(flute_kline16_clp) = NV(flute_kline16_crp);
           NV(flute_kline16_crp) = va_x9;
           break;
         case 8:
         NV(flute_kline16_cdur) = va_dur9;
           NV(flute_kline16_clp) = NV(flute_kline16_crp);
           NV(flute_kline16_crp) = va_x10;
           break;
         case 9:
         NV(flute_kline16_cdur) = va_dur10;
           NV(flute_kline16_clp) = NV(flute_kline16_crp);
           NV(flute_kline16_crp) = va_x11;
           break;
         case 10:
         NV(flute_kline16_cdur) = va_dur11;
           NV(flute_kline16_clp) = NV(flute_kline16_crp);
           NV(flute_kline16_crp) = va_x12;
           break;
         case 11:
         NV(flute_kline16_cdur) = va_dur12;
           NV(flute_kline16_clp) = NV(flute_kline16_crp);
           NV(flute_kline16_crp) = va_x13;
           break;
         case 12:
         NV(flute_kline16_cdur) = va_dur13;
           NV(flute_kline16_clp) = NV(flute_kline16_crp);
           NV(flute_kline16_crp) = va_x14;
           break;
         case 13:
         NV(flute_kline16_cdur) = va_dur14;
           NV(flute_kline16_clp) = NV(flute_kline16_crp);
           NV(flute_kline16_crp) = va_x15;
           break;
         case 14:
         NV(flute_kline16_cdur) = va_dur15;
           NV(flute_kline16_clp) = NV(flute_kline16_crp);
           NV(flute_kline16_crp) = va_x16;
           break;
         case 15:
         NV(flute_kline16_cdur) = va_dur16;
           NV(flute_kline16_clp) = NV(flute_kline16_crp);
           NV(flute_kline16_crp) = va_x17;
           break;
         case 16:
         NV(flute_kline16_cdur) = va_dur17;
           NV(flute_kline16_clp) = NV(flute_kline16_crp);
           NV(flute_kline16_crp) = va_x18;
           break;
         case 17:
         NV(flute_kline16_cdur) = va_dur18;
           NV(flute_kline16_clp) = NV(flute_kline16_crp);
           NV(flute_kline16_crp) = va_x19;
           break;
         case 18:
         NV(flute_kline16_cdur) = va_dur19;
           NV(flute_kline16_clp) = NV(flute_kline16_crp);
           NV(flute_kline16_crp) = va_x20;
           break;
           default:
           NVI(flute_kline16_first) = -100;
           NV(flute_kline16_cdur) = NV(flute_kline16_t) + 10000.0F;
           break;
           }
         NVI(flute_kline16_first)++;
        }
        NV(flute_kline16_mult)=(NV(flute_kline16_crp) - NV(flute_kline16_clp))/NV(flute_kline16_cdur);
        ret = NV(flute_kline16_outT) = NV(flute_kline16_clp)+NV(flute_kline16_mult)*NV(flute_kline16_t);
        NV(flute_kline16_addK) = NV(flute_kline16_mult)*EV(KTIME);
        if (NVI(flute_kline16_first)<0)
          ret = 0.0F;
      }
   }
   if (NVI(flute_kline16_first)==0)
     {
       NVI(flute_kline16_first) = 1;
       ret = NV(flute_kline16_outT) = NV(flute_kline16_clp) = x1;
       NV(flute_kline16_crp) = x2;
       NV(flute_kline16_cdur) = dur1;
       if (dur1 > 0.0F)
         NV(flute_kline16_addK) = EV(KTIME)*((x2 - x1)/dur1);
     }
   return((NV(flute_kline16_return) = ret));

}



float flute__sym_kline17(ENGINE_PTR_DECLARE_COMMA struct ninstr_types * nstate)
{
   float va_dur2;
   float va_x3;
   float va_dur3;
   float va_x4;
   float va_dur4;
   float va_x5;
   float va_dur5;
   float va_x6;
   float va_dur6;
   float va_x7;
   float va_dur7;
   float va_x8;
   float va_dur8;
   float va_x9;
   float va_dur9;
   float va_x10;
   float va_dur10;
   float va_x11;
   float va_dur11;
   float va_x12;
   float va_dur12;
   float va_x13;
   float va_dur13;
   float va_x14;
   float va_dur14;
   float va_x15;
   float va_dur15;
   float va_x16;
   float va_dur16;
   float va_x17;
   float va_dur17;
   float va_x18;
   float va_dur18;
   float va_x19;
   float va_dur19;
   float va_x20;
   float x2;
   float dur1;
   float x1;
   float ret;

   x1 =  0.0F ;
   dur1 =  0.450000F ;
   x2 =  0.001659F ;
   va_dur2 =  0.844231F  ;
   va_x3 =  0.001422F  ;
   va_dur3 =  0.394231F  ;
   va_x4 =  0.001229F  ;
   va_dur4 =  0.300000F  ;
   va_x5 =  0.001076F  ;
   va_dur5 =  0.182692F  ;
   va_x6 =  0.001114F  ;
   va_dur6 =  0.021154F  ;
   va_x7 =  0.000516F  ;
   va_dur7 =  0.009615F  ;
   va_x8 =  0.000827F  ;
   va_dur8 =  0.073077F  ;
   va_x9 =  0.000020F  ;
   va_dur9 =  0.001923F  ;
   va_x10 =  0.000035F  ;
   va_dur10 =  0.001923F  ;
   va_x11 =  0.000069F  ;
   va_dur11 =  0.003846F  ;
   va_x12 =  0.000023F  ;
   va_dur12 =  0.001923F  ;
   va_x13 =  0.000066F  ;
   va_dur13 =  0.001923F  ;
   va_x14 =  0.000042F  ;
   va_dur14 =  0.003846F  ;
   va_x15 =  0.000063F  ;
   va_dur15 =  0.005769F  ;
   va_x16 =  0.000071F  ;
   va_dur16 =  0.003846F  ;
   va_x17 =  0.000039F  ;
   va_dur17 =  0.001923F  ;
   va_x18 =  0.000110F  ;
   va_dur18 =  0.003846F  ;
   va_x19 =  0.000043F  ;
   va_dur19 =  0.0F  ;
   va_x20 =  0.000043F  ;
   ret = 0.0F;
   if (NVI(flute_kline17_first)>0)
   {
      NV(flute_kline17_t) += EV(KTIME);
      ret = (NV(flute_kline17_outT) += NV(flute_kline17_addK));
      if (NV(flute_kline17_t) > NV(flute_kline17_cdur))
       {
        while (NV(flute_kline17_t) > NV(flute_kline17_cdur))
         {
           NV(flute_kline17_t) -= NV(flute_kline17_cdur);
           switch(NVI(flute_kline17_first))
      {
         case 1:
         NV(flute_kline17_cdur) = va_dur2;
           NV(flute_kline17_clp) = NV(flute_kline17_crp);
           NV(flute_kline17_crp) = va_x3;
           break;
         case 2:
         NV(flute_kline17_cdur) = va_dur3;
           NV(flute_kline17_clp) = NV(flute_kline17_crp);
           NV(flute_kline17_crp) = va_x4;
           break;
         case 3:
         NV(flute_kline17_cdur) = va_dur4;
           NV(flute_kline17_clp) = NV(flute_kline17_crp);
           NV(flute_kline17_crp) = va_x5;
           break;
         case 4:
         NV(flute_kline17_cdur) = va_dur5;
           NV(flute_kline17_clp) = NV(flute_kline17_crp);
           NV(flute_kline17_crp) = va_x6;
           break;
         case 5:
         NV(flute_kline17_cdur) = va_dur6;
           NV(flute_kline17_clp) = NV(flute_kline17_crp);
           NV(flute_kline17_crp) = va_x7;
           break;
         case 6:
         NV(flute_kline17_cdur) = va_dur7;
           NV(flute_kline17_clp) = NV(flute_kline17_crp);
           NV(flute_kline17_crp) = va_x8;
           break;
         case 7:
         NV(flute_kline17_cdur) = va_dur8;
           NV(flute_kline17_clp) = NV(flute_kline17_crp);
           NV(flute_kline17_crp) = va_x9;
           break;
         case 8:
         NV(flute_kline17_cdur) = va_dur9;
           NV(flute_kline17_clp) = NV(flute_kline17_crp);
           NV(flute_kline17_crp) = va_x10;
           break;
         case 9:
         NV(flute_kline17_cdur) = va_dur10;
           NV(flute_kline17_clp) = NV(flute_kline17_crp);
           NV(flute_kline17_crp) = va_x11;
           break;
         case 10:
         NV(flute_kline17_cdur) = va_dur11;
           NV(flute_kline17_clp) = NV(flute_kline17_crp);
           NV(flute_kline17_crp) = va_x12;
           break;
         case 11:
         NV(flute_kline17_cdur) = va_dur12;
           NV(flute_kline17_clp) = NV(flute_kline17_crp);
           NV(flute_kline17_crp) = va_x13;
           break;
         case 12:
         NV(flute_kline17_cdur) = va_dur13;
           NV(flute_kline17_clp) = NV(flute_kline17_crp);
           NV(flute_kline17_crp) = va_x14;
           break;
         case 13:
         NV(flute_kline17_cdur) = va_dur14;
           NV(flute_kline17_clp) = NV(flute_kline17_crp);
           NV(flute_kline17_crp) = va_x15;
           break;
         case 14:
         NV(flute_kline17_cdur) = va_dur15;
           NV(flute_kline17_clp) = NV(flute_kline17_crp);
           NV(flute_kline17_crp) = va_x16;
           break;
         case 15:
         NV(flute_kline17_cdur) = va_dur16;
           NV(flute_kline17_clp) = NV(flute_kline17_crp);
           NV(flute_kline17_crp) = va_x17;
           break;
         case 16:
         NV(flute_kline17_cdur) = va_dur17;
           NV(flute_kline17_clp) = NV(flute_kline17_crp);
           NV(flute_kline17_crp) = va_x18;
           break;
         case 17:
         NV(flute_kline17_cdur) = va_dur18;
           NV(flute_kline17_clp) = NV(flute_kline17_crp);
           NV(flute_kline17_crp) = va_x19;
           break;
         case 18:
         NV(flute_kline17_cdur) = va_dur19;
           NV(flute_kline17_clp) = NV(flute_kline17_crp);
           NV(flute_kline17_crp) = va_x20;
           break;
           default:
           NVI(flute_kline17_first) = -100;
           NV(flute_kline17_cdur) = NV(flute_kline17_t) + 10000.0F;
           break;
           }
         NVI(flute_kline17_first)++;
        }
        NV(flute_kline17_mult)=(NV(flute_kline17_crp) - NV(flute_kline17_clp))/NV(flute_kline17_cdur);
        ret = NV(flute_kline17_outT) = NV(flute_kline17_clp)+NV(flute_kline17_mult)*NV(flute_kline17_t);
        NV(flute_kline17_addK) = NV(flute_kline17_mult)*EV(KTIME);
        if (NVI(flute_kline17_first)<0)
          ret = 0.0F;
      }
   }
   if (NVI(flute_kline17_first)==0)
     {
       NVI(flute_kline17_first) = 1;
       ret = NV(flute_kline17_outT) = NV(flute_kline17_clp) = x1;
       NV(flute_kline17_crp) = x2;
       NV(flute_kline17_cdur) = dur1;
       if (dur1 > 0.0F)
         NV(flute_kline17_addK) = EV(KTIME)*((x2 - x1)/dur1);
     }
   return((NV(flute_kline17_return) = ret));

}



float flute__sym_kline18(ENGINE_PTR_DECLARE_COMMA struct ninstr_types * nstate)
{
   float va_dur2;
   float va_x3;
   float va_dur3;
   float va_x4;
   float va_dur4;
   float va_x5;
   float va_dur5;
   float va_x6;
   float va_dur6;
   float va_x7;
   float va_dur7;
   float va_x8;
   float va_dur8;
   float va_x9;
   float va_dur9;
   float va_x10;
   float va_dur10;
   float va_x11;
   float va_dur11;
   float va_x12;
   float va_dur12;
   float va_x13;
   float va_dur13;
   float va_x14;
   float va_dur14;
   float va_x15;
   float va_dur15;
   float va_x16;
   float va_dur16;
   float va_x17;
   float va_dur17;
   float va_x18;
   float va_dur18;
   float va_x19;
   float va_dur19;
   float va_x20;
   float x2;
   float dur1;
   float x1;
   float ret;

   x1 =  0.0F ;
   dur1 =  0.603846F ;
   x2 =  0.003264F ;
   va_dur2 =  0.705769F  ;
   va_x3 =  0.002747F  ;
   va_dur3 =  0.684615F  ;
   va_x4 =  0.001681F  ;
   va_dur4 =  0.155769F  ;
   va_x5 =  0.000916F  ;
   va_dur5 =  0.001923F  ;
   va_x6 =  0.001134F  ;
   va_dur6 =  0.042308F  ;
   va_x7 =  0.000784F  ;
   va_dur7 =  0.005769F  ;
   va_x8 =  0.001465F  ;
   va_dur8 =  0.017308F  ;
   va_x9 =  0.000464F  ;
   va_dur9 =  0.036538F  ;
   va_x10 =  0.000110F  ;
   va_dur10 =  0.009615F  ;
   va_x11 =  0.000074F  ;
   va_dur11 =  0.003846F  ;
   va_x12 =  0.000114F  ;
   va_dur12 =  0.009615F  ;
   va_x13 =  0.000077F  ;
   va_dur13 =  0.005769F  ;
   va_x14 =  0.000053F  ;
   va_dur14 =  0.005769F  ;
   va_x15 =  0.000063F  ;
   va_dur15 =  0.009615F  ;
   va_x16 =  0.000016F  ;
   va_dur16 =  0.003846F  ;
   va_x17 =  0.000007F  ;
   va_dur17 =  0.001923F  ;
   va_x18 =  0.000035F  ;
   va_dur18 =  0.001923F  ;
   va_x19 =  0.000012F  ;
   va_dur19 =  0.0F  ;
   va_x20 =  0.000012F  ;
   ret = 0.0F;
   if (NVI(flute_kline18_first)>0)
   {
      NV(flute_kline18_t) += EV(KTIME);
      ret = (NV(flute_kline18_outT) += NV(flute_kline18_addK));
      if (NV(flute_kline18_t) > NV(flute_kline18_cdur))
       {
        while (NV(flute_kline18_t) > NV(flute_kline18_cdur))
         {
           NV(flute_kline18_t) -= NV(flute_kline18_cdur);
           switch(NVI(flute_kline18_first))
      {
         case 1:
         NV(flute_kline18_cdur) = va_dur2;
           NV(flute_kline18_clp) = NV(flute_kline18_crp);
           NV(flute_kline18_crp) = va_x3;
           break;
         case 2:
         NV(flute_kline18_cdur) = va_dur3;
           NV(flute_kline18_clp) = NV(flute_kline18_crp);
           NV(flute_kline18_crp) = va_x4;
           break;
         case 3:
         NV(flute_kline18_cdur) = va_dur4;
           NV(flute_kline18_clp) = NV(flute_kline18_crp);
           NV(flute_kline18_crp) = va_x5;
           break;
         case 4:
         NV(flute_kline18_cdur) = va_dur5;
           NV(flute_kline18_clp) = NV(flute_kline18_crp);
           NV(flute_kline18_crp) = va_x6;
           break;
         case 5:
         NV(flute_kline18_cdur) = va_dur6;
           NV(flute_kline18_clp) = NV(flute_kline18_crp);
           NV(flute_kline18_crp) = va_x7;
           break;
         case 6:
         NV(flute_kline18_cdur) = va_dur7;
           NV(flute_kline18_clp) = NV(flute_kline18_crp);
           NV(flute_kline18_crp) = va_x8;
           break;
         case 7:
         NV(flute_kline18_cdur) = va_dur8;
           NV(flute_kline18_clp) = NV(flute_kline18_crp);
           NV(flute_kline18_crp) = va_x9;
           break;
         case 8:
         NV(flute_kline18_cdur) = va_dur9;
           NV(flute_kline18_clp) = NV(flute_kline18_crp);
           NV(flute_kline18_crp) = va_x10;
           break;
         case 9:
         NV(flute_kline18_cdur) = va_dur10;
           NV(flute_kline18_clp) = NV(flute_kline18_crp);
           NV(flute_kline18_crp) = va_x11;
           break;
         case 10:
         NV(flute_kline18_cdur) = va_dur11;
           NV(flute_kline18_clp) = NV(flute_kline18_crp);
           NV(flute_kline18_crp) = va_x12;
           break;
         case 11:
         NV(flute_kline18_cdur) = va_dur12;
           NV(flute_kline18_clp) = NV(flute_kline18_crp);
           NV(flute_kline18_crp) = va_x13;
           break;
         case 12:
         NV(flute_kline18_cdur) = va_dur13;
           NV(flute_kline18_clp) = NV(flute_kline18_crp);
           NV(flute_kline18_crp) = va_x14;
           break;
         case 13:
         NV(flute_kline18_cdur) = va_dur14;
           NV(flute_kline18_clp) = NV(flute_kline18_crp);
           NV(flute_kline18_crp) = va_x15;
           break;
         case 14:
         NV(flute_kline18_cdur) = va_dur15;
           NV(flute_kline18_clp) = NV(flute_kline18_crp);
           NV(flute_kline18_crp) = va_x16;
           break;
         case 15:
         NV(flute_kline18_cdur) = va_dur16;
           NV(flute_kline18_clp) = NV(flute_kline18_crp);
           NV(flute_kline18_crp) = va_x17;
           break;
         case 16:
         NV(flute_kline18_cdur) = va_dur17;
           NV(flute_kline18_clp) = NV(flute_kline18_crp);
           NV(flute_kline18_crp) = va_x18;
           break;
         case 17:
         NV(flute_kline18_cdur) = va_dur18;
           NV(flute_kline18_clp) = NV(flute_kline18_crp);
           NV(flute_kline18_crp) = va_x19;
           break;
         case 18:
         NV(flute_kline18_cdur) = va_dur19;
           NV(flute_kline18_clp) = NV(flute_kline18_crp);
           NV(flute_kline18_crp) = va_x20;
           break;
           default:
           NVI(flute_kline18_first) = -100;
           NV(flute_kline18_cdur) = NV(flute_kline18_t) + 10000.0F;
           break;
           }
         NVI(flute_kline18_first)++;
        }
        NV(flute_kline18_mult)=(NV(flute_kline18_crp) - NV(flute_kline18_clp))/NV(flute_kline18_cdur);
        ret = NV(flute_kline18_outT) = NV(flute_kline18_clp)+NV(flute_kline18_mult)*NV(flute_kline18_t);
        NV(flute_kline18_addK) = NV(flute_kline18_mult)*EV(KTIME);
        if (NVI(flute_kline18_first)<0)
          ret = 0.0F;
      }
   }
   if (NVI(flute_kline18_first)==0)
     {
       NVI(flute_kline18_first) = 1;
       ret = NV(flute_kline18_outT) = NV(flute_kline18_clp) = x1;
       NV(flute_kline18_crp) = x2;
       NV(flute_kline18_cdur) = dur1;
       if (dur1 > 0.0F)
         NV(flute_kline18_addK) = EV(KTIME)*((x2 - x1)/dur1);
     }
   return((NV(flute_kline18_return) = ret));

}



float flute__sym_kline19(ENGINE_PTR_DECLARE_COMMA struct ninstr_types * nstate)
{
   float va_dur2;
   float va_x3;
   float va_dur3;
   float va_x4;
   float va_dur4;
   float va_x5;
   float va_dur5;
   float va_x6;
   float va_dur6;
   float va_x7;
   float va_dur7;
   float va_x8;
   float va_dur8;
   float va_x9;
   float va_dur9;
   float va_x10;
   float va_dur10;
   float va_x11;
   float va_dur11;
   float va_x12;
   float va_dur12;
   float va_x13;
   float va_dur13;
   float va_x14;
   float va_dur14;
   float va_x15;
   float va_dur15;
   float va_x16;
   float va_dur16;
   float va_x17;
   float va_dur17;
   float va_x18;
   float va_dur18;
   float va_x19;
   float va_dur19;
   float va_x20;
   float x2;
   float dur1;
   float x1;
   float ret;

   x1 =  0.0F ;
   dur1 =  0.959615F ;
   x2 =  0.002520F ;
   va_dur2 =  0.682692F  ;
   va_x3 =  0.001933F  ;
   va_dur3 =  0.340385F  ;
   va_x4 =  0.001026F  ;
   va_dur4 =  0.003846F  ;
   va_x5 =  0.001223F  ;
   va_dur5 =  0.153846F  ;
   va_x6 =  0.001286F  ;
   va_dur6 =  0.017308F  ;
   va_x7 =  0.000818F  ;
   va_dur7 =  0.001923F  ;
   va_x8 =  0.001332F  ;
   va_dur8 =  0.001923F  ;
   va_x9 =  0.000696F  ;
   va_dur9 =  0.013462F  ;
   va_x10 =  0.000625F  ;
   va_dur10 =  0.001923F  ;
   va_x11 =  0.000977F  ;
   va_dur11 =  0.003846F  ;
   va_x12 =  0.000558F  ;
   va_dur12 =  0.007692F  ;
   va_x13 =  0.000900F  ;
   va_dur13 =  0.003846F  ;
   va_x14 =  0.000547F  ;
   va_dur14 =  0.080769F  ;
   va_x15 =  0.000105F  ;
   va_dur15 =  0.017308F  ;
   va_x16 =  0.000022F  ;
   va_dur16 =  0.005769F  ;
   va_x17 =  0.000014F  ;
   va_dur17 =  0.001923F  ;
   va_x18 =  0.000041F  ;
   va_dur18 =  0.003846F  ;
   va_x19 =  0.000039F  ;
   va_dur19 =  0.003846F  ;
   va_x20 =  0.000014F  ;
   ret = 0.0F;
   if (NVI(flute_kline19_first)>0)
   {
      NV(flute_kline19_t) += EV(KTIME);
      ret = (NV(flute_kline19_outT) += NV(flute_kline19_addK));
      if (NV(flute_kline19_t) > NV(flute_kline19_cdur))
       {
        while (NV(flute_kline19_t) > NV(flute_kline19_cdur))
         {
           NV(flute_kline19_t) -= NV(flute_kline19_cdur);
           switch(NVI(flute_kline19_first))
      {
         case 1:
         NV(flute_kline19_cdur) = va_dur2;
           NV(flute_kline19_clp) = NV(flute_kline19_crp);
           NV(flute_kline19_crp) = va_x3;
           break;
         case 2:
         NV(flute_kline19_cdur) = va_dur3;
           NV(flute_kline19_clp) = NV(flute_kline19_crp);
           NV(flute_kline19_crp) = va_x4;
           break;
         case 3:
         NV(flute_kline19_cdur) = va_dur4;
           NV(flute_kline19_clp) = NV(flute_kline19_crp);
           NV(flute_kline19_crp) = va_x5;
           break;
         case 4:
         NV(flute_kline19_cdur) = va_dur5;
           NV(flute_kline19_clp) = NV(flute_kline19_crp);
           NV(flute_kline19_crp) = va_x6;
           break;
         case 5:
         NV(flute_kline19_cdur) = va_dur6;
           NV(flute_kline19_clp) = NV(flute_kline19_crp);
           NV(flute_kline19_crp) = va_x7;
           break;
         case 6:
         NV(flute_kline19_cdur) = va_dur7;
           NV(flute_kline19_clp) = NV(flute_kline19_crp);
           NV(flute_kline19_crp) = va_x8;
           break;
         case 7:
         NV(flute_kline19_cdur) = va_dur8;
           NV(flute_kline19_clp) = NV(flute_kline19_crp);
           NV(flute_kline19_crp) = va_x9;
           break;
         case 8:
         NV(flute_kline19_cdur) = va_dur9;
           NV(flute_kline19_clp) = NV(flute_kline19_crp);
           NV(flute_kline19_crp) = va_x10;
           break;
         case 9:
         NV(flute_kline19_cdur) = va_dur10;
           NV(flute_kline19_clp) = NV(flute_kline19_crp);
           NV(flute_kline19_crp) = va_x11;
           break;
         case 10:
         NV(flute_kline19_cdur) = va_dur11;
           NV(flute_kline19_clp) = NV(flute_kline19_crp);
           NV(flute_kline19_crp) = va_x12;
           break;
         case 11:
         NV(flute_kline19_cdur) = va_dur12;
           NV(flute_kline19_clp) = NV(flute_kline19_crp);
           NV(flute_kline19_crp) = va_x13;
           break;
         case 12:
         NV(flute_kline19_cdur) = va_dur13;
           NV(flute_kline19_clp) = NV(flute_kline19_crp);
           NV(flute_kline19_crp) = va_x14;
           break;
         case 13:
         NV(flute_kline19_cdur) = va_dur14;
           NV(flute_kline19_clp) = NV(flute_kline19_crp);
           NV(flute_kline19_crp) = va_x15;
           break;
         case 14:
         NV(flute_kline19_cdur) = va_dur15;
           NV(flute_kline19_clp) = NV(flute_kline19_crp);
           NV(flute_kline19_crp) = va_x16;
           break;
         case 15:
         NV(flute_kline19_cdur) = va_dur16;
           NV(flute_kline19_clp) = NV(flute_kline19_crp);
           NV(flute_kline19_crp) = va_x17;
           break;
         case 16:
         NV(flute_kline19_cdur) = va_dur17;
           NV(flute_kline19_clp) = NV(flute_kline19_crp);
           NV(flute_kline19_crp) = va_x18;
           break;
         case 17:
         NV(flute_kline19_cdur) = va_dur18;
           NV(flute_kline19_clp) = NV(flute_kline19_crp);
           NV(flute_kline19_crp) = va_x19;
           break;
         case 18:
         NV(flute_kline19_cdur) = va_dur19;
           NV(flute_kline19_clp) = NV(flute_kline19_crp);
           NV(flute_kline19_crp) = va_x20;
           break;
           default:
           NVI(flute_kline19_first) = -100;
           NV(flute_kline19_cdur) = NV(flute_kline19_t) + 10000.0F;
           break;
           }
         NVI(flute_kline19_first)++;
        }
        NV(flute_kline19_mult)=(NV(flute_kline19_crp) - NV(flute_kline19_clp))/NV(flute_kline19_cdur);
        ret = NV(flute_kline19_outT) = NV(flute_kline19_clp)+NV(flute_kline19_mult)*NV(flute_kline19_t);
        NV(flute_kline19_addK) = NV(flute_kline19_mult)*EV(KTIME);
        if (NVI(flute_kline19_first)<0)
          ret = 0.0F;
      }
   }
   if (NVI(flute_kline19_first)==0)
     {
       NVI(flute_kline19_first) = 1;
       ret = NV(flute_kline19_outT) = NV(flute_kline19_clp) = x1;
       NV(flute_kline19_crp) = x2;
       NV(flute_kline19_cdur) = dur1;
       if (dur1 > 0.0F)
         NV(flute_kline19_addK) = EV(KTIME)*((x2 - x1)/dur1);
     }
   return((NV(flute_kline19_return) = ret));

}



float flute__sym_kline20(ENGINE_PTR_DECLARE_COMMA struct ninstr_types * nstate)
{
   float va_dur2;
   float va_x3;
   float va_dur3;
   float va_x4;
   float va_dur4;
   float va_x5;
   float va_dur5;
   float va_x6;
   float va_dur6;
   float va_x7;
   float va_dur7;
   float va_x8;
   float va_dur8;
   float va_x9;
   float va_dur9;
   float va_x10;
   float va_dur10;
   float va_x11;
   float va_dur11;
   float va_x12;
   float va_dur12;
   float va_x13;
   float va_dur13;
   float va_x14;
   float va_dur14;
   float va_x15;
   float va_dur15;
   float va_x16;
   float va_dur16;
   float va_x17;
   float va_dur17;
   float va_x18;
   float va_dur18;
   float va_x19;
   float va_dur19;
   float va_x20;
   float x2;
   float dur1;
   float x1;
   float ret;

   x1 =  0.0F ;
   dur1 =  0.417308F ;
   x2 =  0.001685F ;
   va_dur2 =  0.888462F  ;
   va_x3 =  0.001340F  ;
   va_dur3 =  0.684615F  ;
   va_x4 =  0.001003F  ;
   va_dur4 =  0.150000F  ;
   va_x5 =  0.000779F  ;
   va_dur5 =  0.046154F  ;
   va_x6 =  0.000417F  ;
   va_dur6 =  0.001923F  ;
   va_x7 =  0.000560F  ;
   va_dur7 =  0.003846F  ;
   va_x8 =  0.000636F  ;
   va_dur8 =  0.001923F  ;
   va_x9 =  0.000485F  ;
   va_dur9 =  0.007692F  ;
   va_x10 =  0.000865F  ;
   va_dur10 =  0.071154F  ;
   va_x11 =  0.000015F  ;
   va_dur11 =  0.001923F  ;
   va_x12 =  0.000045F  ;
   va_dur12 =  0.001923F  ;
   va_x13 =  0.000060F  ;
   va_dur13 =  0.003846F  ;
   va_x14 =  0.000036F  ;
   va_dur14 =  0.003846F  ;
   va_x15 =  0.000042F  ;
   va_dur15 =  0.001923F  ;
   va_x16 =  0.000086F  ;
   va_dur16 =  0.001923F  ;
   va_x17 =  0.000091F  ;
   va_dur17 =  0.009615F  ;
   va_x18 =  0.000015F  ;
   va_dur18 =  0.007692F  ;
   va_x19 =  0.000045F  ;
   va_dur19 =  0.0F  ;
   va_x20 =  0.000045F  ;
   ret = 0.0F;
   if (NVI(flute_kline20_first)>0)
   {
      NV(flute_kline20_t) += EV(KTIME);
      ret = (NV(flute_kline20_outT) += NV(flute_kline20_addK));
      if (NV(flute_kline20_t) > NV(flute_kline20_cdur))
       {
        while (NV(flute_kline20_t) > NV(flute_kline20_cdur))
         {
           NV(flute_kline20_t) -= NV(flute_kline20_cdur);
           switch(NVI(flute_kline20_first))
      {
         case 1:
         NV(flute_kline20_cdur) = va_dur2;
           NV(flute_kline20_clp) = NV(flute_kline20_crp);
           NV(flute_kline20_crp) = va_x3;
           break;
         case 2:
         NV(flute_kline20_cdur) = va_dur3;
           NV(flute_kline20_clp) = NV(flute_kline20_crp);
           NV(flute_kline20_crp) = va_x4;
           break;
         case 3:
         NV(flute_kline20_cdur) = va_dur4;
           NV(flute_kline20_clp) = NV(flute_kline20_crp);
           NV(flute_kline20_crp) = va_x5;
           break;
         case 4:
         NV(flute_kline20_cdur) = va_dur5;
           NV(flute_kline20_clp) = NV(flute_kline20_crp);
           NV(flute_kline20_crp) = va_x6;
           break;
         case 5:
         NV(flute_kline20_cdur) = va_dur6;
           NV(flute_kline20_clp) = NV(flute_kline20_crp);
           NV(flute_kline20_crp) = va_x7;
           break;
         case 6:
         NV(flute_kline20_cdur) = va_dur7;
           NV(flute_kline20_clp) = NV(flute_kline20_crp);
           NV(flute_kline20_crp) = va_x8;
           break;
         case 7:
         NV(flute_kline20_cdur) = va_dur8;
           NV(flute_kline20_clp) = NV(flute_kline20_crp);
           NV(flute_kline20_crp) = va_x9;
           break;
         case 8:
         NV(flute_kline20_cdur) = va_dur9;
           NV(flute_kline20_clp) = NV(flute_kline20_crp);
           NV(flute_kline20_crp) = va_x10;
           break;
         case 9:
         NV(flute_kline20_cdur) = va_dur10;
           NV(flute_kline20_clp) = NV(flute_kline20_crp);
           NV(flute_kline20_crp) = va_x11;
           break;
         case 10:
         NV(flute_kline20_cdur) = va_dur11;
           NV(flute_kline20_clp) = NV(flute_kline20_crp);
           NV(flute_kline20_crp) = va_x12;
           break;
         case 11:
         NV(flute_kline20_cdur) = va_dur12;
           NV(flute_kline20_clp) = NV(flute_kline20_crp);
           NV(flute_kline20_crp) = va_x13;
           break;
         case 12:
         NV(flute_kline20_cdur) = va_dur13;
           NV(flute_kline20_clp) = NV(flute_kline20_crp);
           NV(flute_kline20_crp) = va_x14;
           break;
         case 13:
         NV(flute_kline20_cdur) = va_dur14;
           NV(flute_kline20_clp) = NV(flute_kline20_crp);
           NV(flute_kline20_crp) = va_x15;
           break;
         case 14:
         NV(flute_kline20_cdur) = va_dur15;
           NV(flute_kline20_clp) = NV(flute_kline20_crp);
           NV(flute_kline20_crp) = va_x16;
           break;
         case 15:
         NV(flute_kline20_cdur) = va_dur16;
           NV(flute_kline20_clp) = NV(flute_kline20_crp);
           NV(flute_kline20_crp) = va_x17;
           break;
         case 16:
         NV(flute_kline20_cdur) = va_dur17;
           NV(flute_kline20_clp) = NV(flute_kline20_crp);
           NV(flute_kline20_crp) = va_x18;
           break;
         case 17:
         NV(flute_kline20_cdur) = va_dur18;
           NV(flute_kline20_clp) = NV(flute_kline20_crp);
           NV(flute_kline20_crp) = va_x19;
           break;
         case 18:
         NV(flute_kline20_cdur) = va_dur19;
           NV(flute_kline20_clp) = NV(flute_kline20_crp);
           NV(flute_kline20_crp) = va_x20;
           break;
           default:
           NVI(flute_kline20_first) = -100;
           NV(flute_kline20_cdur) = NV(flute_kline20_t) + 10000.0F;
           break;
           }
         NVI(flute_kline20_first)++;
        }
        NV(flute_kline20_mult)=(NV(flute_kline20_crp) - NV(flute_kline20_clp))/NV(flute_kline20_cdur);
        ret = NV(flute_kline20_outT) = NV(flute_kline20_clp)+NV(flute_kline20_mult)*NV(flute_kline20_t);
        NV(flute_kline20_addK) = NV(flute_kline20_mult)*EV(KTIME);
        if (NVI(flute_kline20_first)<0)
          ret = 0.0F;
      }
   }
   if (NVI(flute_kline20_first)==0)
     {
       NVI(flute_kline20_first) = 1;
       ret = NV(flute_kline20_outT) = NV(flute_kline20_clp) = x1;
       NV(flute_kline20_crp) = x2;
       NV(flute_kline20_cdur) = dur1;
       if (dur1 > 0.0F)
         NV(flute_kline20_addK) = EV(KTIME)*((x2 - x1)/dur1);
     }
   return((NV(flute_kline20_return) = ret));

}



float flute__sym_oscil21(ENGINE_PTR_DECLARE_COMMA struct ninstr_types * nstate)
{
   float freq;
   PSIZE t;
   float ret;
   float index;
   unsigned int nint, nfrac, i, j;

   t =  TBL_GBL_cyc ;

#undef AP1
#define AP1 EV(gtables)[t]

   freq = NV(flute__tvr0);
   if (NVI(flute_oscil21_kcyc))
   {
     if (NVI(flute_oscil21_fsign))
      {
       NVUI(flute_oscil21_pfrac) = (j = NVUI(flute_oscil21_pfrac)) + NVUI(flute_oscil21_kfrac);
       NVUI(flute_oscil21_pint) += NVUI(flute_oscil21_kint) + (NVUI(flute_oscil21_pfrac) < j);
       if ((i = NVUI(flute_oscil21_pint)) >= AP1.len)
         i = (NVUI(flute_oscil21_pint) -= AP1.len);
      }
     else
      {
       NVUI(flute_oscil21_pfrac) = (j = NVUI(flute_oscil21_pfrac)) - NVUI(flute_oscil21_kfrac);
       NVUI(flute_oscil21_pint) -= NVUI(flute_oscil21_kint) + (NVUI(flute_oscil21_pfrac) > j);
       if ((i = NVUI(flute_oscil21_pint)) >= AP1.len)
         i = (NVUI(flute_oscil21_pint) += AP1.len);
      }
     ret = AP1.t[i] + NVUI(flute_oscil21_pfrac)*
           ((float)(1.0/4294967296.0))*(AP1.t[i+1] - AP1.t[i]);
   }
   else
   {
     if ((NVI(flute_oscil21_fsign) = (freq >= 0)))
      {
       nint = (unsigned int)(index = freq*AP1.oconst);
       while (index >= AP1.lenf)
         nint = (unsigned int)(index -= AP1.lenf);
       nfrac = (unsigned int)(4294967296.0F*(index - nint));
      }
     else
      {
       nint = (unsigned int)(index = -freq*AP1.oconst);
       while (index >= AP1.lenf)
         nint = (unsigned int)(index -= AP1.lenf);
       nfrac = (unsigned int)(4294967296.0F*(index - nint));
      }
     NVUI(flute_oscil21_kint) = nint;
     NVUI(flute_oscil21_kfrac) = nfrac;
     NVI(flute_oscil21_kcyc) = 1;
     ret = AP1.t[0];
   }
   return((NV(flute_oscil21_return) = ret));

}



float flute__sym_oscil22(ENGINE_PTR_DECLARE_COMMA struct ninstr_types * nstate)
{
   float freq;
   PSIZE t;
   float ret;
   float index;
   unsigned int nint, nfrac, i, j;

   t =  TBL_GBL_cyc ;

#undef AP1
#define AP1 EV(gtables)[t]

   freq = NV(flute__tvr1);
   if (NVI(flute_oscil22_kcyc))
   {
     if (NVI(flute_oscil22_fsign))
      {
       NVUI(flute_oscil22_pfrac) = (j = NVUI(flute_oscil22_pfrac)) + NVUI(flute_oscil22_kfrac);
       NVUI(flute_oscil22_pint) += NVUI(flute_oscil22_kint) + (NVUI(flute_oscil22_pfrac) < j);
       if ((i = NVUI(flute_oscil22_pint)) >= AP1.len)
         i = (NVUI(flute_oscil22_pint) -= AP1.len);
      }
     else
      {
       NVUI(flute_oscil22_pfrac) = (j = NVUI(flute_oscil22_pfrac)) - NVUI(flute_oscil22_kfrac);
       NVUI(flute_oscil22_pint) -= NVUI(flute_oscil22_kint) + (NVUI(flute_oscil22_pfrac) > j);
       if ((i = NVUI(flute_oscil22_pint)) >= AP1.len)
         i = (NVUI(flute_oscil22_pint) += AP1.len);
      }
     ret = AP1.t[i] + NVUI(flute_oscil22_pfrac)*
           ((float)(1.0/4294967296.0))*(AP1.t[i+1] - AP1.t[i]);
   }
   else
   {
     if ((NVI(flute_oscil22_fsign) = (freq >= 0)))
      {
       nint = (unsigned int)(index = freq*AP1.oconst);
       while (index >= AP1.lenf)
         nint = (unsigned int)(index -= AP1.lenf);
       nfrac = (unsigned int)(4294967296.0F*(index - nint));
      }
     else
      {
       nint = (unsigned int)(index = -freq*AP1.oconst);
       while (index >= AP1.lenf)
         nint = (unsigned int)(index -= AP1.lenf);
       nfrac = (unsigned int)(4294967296.0F*(index - nint));
      }
     NVUI(flute_oscil22_kint) = nint;
     NVUI(flute_oscil22_kfrac) = nfrac;
     NVI(flute_oscil22_kcyc) = 1;
     ret = AP1.t[0];
   }
   return((NV(flute_oscil22_return) = ret));

}



float flute__sym_oscil23(ENGINE_PTR_DECLARE_COMMA struct ninstr_types * nstate)
{
   float freq;
   PSIZE t;
   float ret;
   float index;
   unsigned int nint, nfrac, i, j;

   t =  TBL_GBL_cyc ;

#undef AP1
#define AP1 EV(gtables)[t]

   freq = NV(flute__tvr2);
   if (NVI(flute_oscil23_kcyc))
   {
     if (NVI(flute_oscil23_fsign))
      {
       NVUI(flute_oscil23_pfrac) = (j = NVUI(flute_oscil23_pfrac)) + NVUI(flute_oscil23_kfrac);
       NVUI(flute_oscil23_pint) += NVUI(flute_oscil23_kint) + (NVUI(flute_oscil23_pfrac) < j);
       if ((i = NVUI(flute_oscil23_pint)) >= AP1.len)
         i = (NVUI(flute_oscil23_pint) -= AP1.len);
      }
     else
      {
       NVUI(flute_oscil23_pfrac) = (j = NVUI(flute_oscil23_pfrac)) - NVUI(flute_oscil23_kfrac);
       NVUI(flute_oscil23_pint) -= NVUI(flute_oscil23_kint) + (NVUI(flute_oscil23_pfrac) > j);
       if ((i = NVUI(flute_oscil23_pint)) >= AP1.len)
         i = (NVUI(flute_oscil23_pint) += AP1.len);
      }
     ret = AP1.t[i] + NVUI(flute_oscil23_pfrac)*
           ((float)(1.0/4294967296.0))*(AP1.t[i+1] - AP1.t[i]);
   }
   else
   {
     if ((NVI(flute_oscil23_fsign) = (freq >= 0)))
      {
       nint = (unsigned int)(index = freq*AP1.oconst);
       while (index >= AP1.lenf)
         nint = (unsigned int)(index -= AP1.lenf);
       nfrac = (unsigned int)(4294967296.0F*(index - nint));
      }
     else
      {
       nint = (unsigned int)(index = -freq*AP1.oconst);
       while (index >= AP1.lenf)
         nint = (unsigned int)(index -= AP1.lenf);
       nfrac = (unsigned int)(4294967296.0F*(index - nint));
      }
     NVUI(flute_oscil23_kint) = nint;
     NVUI(flute_oscil23_kfrac) = nfrac;
     NVI(flute_oscil23_kcyc) = 1;
     ret = AP1.t[0];
   }
   return((NV(flute_oscil23_return) = ret));

}



float flute__sym_oscil24(ENGINE_PTR_DECLARE_COMMA struct ninstr_types * nstate)
{
   float freq;
   PSIZE t;
   float ret;
   float index;
   unsigned int nint, nfrac, i, j;

   t =  TBL_GBL_cyc ;

#undef AP1
#define AP1 EV(gtables)[t]

   freq = NV(flute__tvr3);
   if (NVI(flute_oscil24_kcyc))
   {
     if (NVI(flute_oscil24_fsign))
      {
       NVUI(flute_oscil24_pfrac) = (j = NVUI(flute_oscil24_pfrac)) + NVUI(flute_oscil24_kfrac);
       NVUI(flute_oscil24_pint) += NVUI(flute_oscil24_kint) + (NVUI(flute_oscil24_pfrac) < j);
       if ((i = NVUI(flute_oscil24_pint)) >= AP1.len)
         i = (NVUI(flute_oscil24_pint) -= AP1.len);
      }
     else
      {
       NVUI(flute_oscil24_pfrac) = (j = NVUI(flute_oscil24_pfrac)) - NVUI(flute_oscil24_kfrac);
       NVUI(flute_oscil24_pint) -= NVUI(flute_oscil24_kint) + (NVUI(flute_oscil24_pfrac) > j);
       if ((i = NVUI(flute_oscil24_pint)) >= AP1.len)
         i = (NVUI(flute_oscil24_pint) += AP1.len);
      }
     ret = AP1.t[i] + NVUI(flute_oscil24_pfrac)*
           ((float)(1.0/4294967296.0))*(AP1.t[i+1] - AP1.t[i]);
   }
   else
   {
     if ((NVI(flute_oscil24_fsign) = (freq >= 0)))
      {
       nint = (unsigned int)(index = freq*AP1.oconst);
       while (index >= AP1.lenf)
         nint = (unsigned int)(index -= AP1.lenf);
       nfrac = (unsigned int)(4294967296.0F*(index - nint));
      }
     else
      {
       nint = (unsigned int)(index = -freq*AP1.oconst);
       while (index >= AP1.lenf)
         nint = (unsigned int)(index -= AP1.lenf);
       nfrac = (unsigned int)(4294967296.0F*(index - nint));
      }
     NVUI(flute_oscil24_kint) = nint;
     NVUI(flute_oscil24_kfrac) = nfrac;
     NVI(flute_oscil24_kcyc) = 1;
     ret = AP1.t[0];
   }
   return((NV(flute_oscil24_return) = ret));

}



float flute__sym_oscil25(ENGINE_PTR_DECLARE_COMMA struct ninstr_types * nstate)
{
   float freq;
   PSIZE t;
   float ret;
   float index;
   unsigned int nint, nfrac, i, j;

   t =  TBL_GBL_cyc ;

#undef AP1
#define AP1 EV(gtables)[t]

   freq = NV(flute__tvr4);
   if (NVI(flute_oscil25_kcyc))
   {
     if (NVI(flute_oscil25_fsign))
      {
       NVUI(flute_oscil25_pfrac) = (j = NVUI(flute_oscil25_pfrac)) + NVUI(flute_oscil25_kfrac);
       NVUI(flute_oscil25_pint) += NVUI(flute_oscil25_kint) + (NVUI(flute_oscil25_pfrac) < j);
       if ((i = NVUI(flute_oscil25_pint)) >= AP1.len)
         i = (NVUI(flute_oscil25_pint) -= AP1.len);
      }
     else
      {
       NVUI(flute_oscil25_pfrac) = (j = NVUI(flute_oscil25_pfrac)) - NVUI(flute_oscil25_kfrac);
       NVUI(flute_oscil25_pint) -= NVUI(flute_oscil25_kint) + (NVUI(flute_oscil25_pfrac) > j);
       if ((i = NVUI(flute_oscil25_pint)) >= AP1.len)
         i = (NVUI(flute_oscil25_pint) += AP1.len);
      }
     ret = AP1.t[i] + NVUI(flute_oscil25_pfrac)*
           ((float)(1.0/4294967296.0))*(AP1.t[i+1] - AP1.t[i]);
   }
   else
   {
     if ((NVI(flute_oscil25_fsign) = (freq >= 0)))
      {
       nint = (unsigned int)(index = freq*AP1.oconst);
       while (index >= AP1.lenf)
         nint = (unsigned int)(index -= AP1.lenf);
       nfrac = (unsigned int)(4294967296.0F*(index - nint));
      }
     else
      {
       nint = (unsigned int)(index = -freq*AP1.oconst);
       while (index >= AP1.lenf)
         nint = (unsigned int)(index -= AP1.lenf);
       nfrac = (unsigned int)(4294967296.0F*(index - nint));
      }
     NVUI(flute_oscil25_kint) = nint;
     NVUI(flute_oscil25_kfrac) = nfrac;
     NVI(flute_oscil25_kcyc) = 1;
     ret = AP1.t[0];
   }
   return((NV(flute_oscil25_return) = ret));

}



float flute__sym_oscil26(ENGINE_PTR_DECLARE_COMMA struct ninstr_types * nstate)
{
   float freq;
   PSIZE t;
   float ret;
   float index;
   unsigned int nint, nfrac, i, j;

   t =  TBL_GBL_cyc ;

#undef AP1
#define AP1 EV(gtables)[t]

   freq = NV(flute__tvr5);
   if (NVI(flute_oscil26_kcyc))
   {
     if (NVI(flute_oscil26_fsign))
      {
       NVUI(flute_oscil26_pfrac) = (j = NVUI(flute_oscil26_pfrac)) + NVUI(flute_oscil26_kfrac);
       NVUI(flute_oscil26_pint) += NVUI(flute_oscil26_kint) + (NVUI(flute_oscil26_pfrac) < j);
       if ((i = NVUI(flute_oscil26_pint)) >= AP1.len)
         i = (NVUI(flute_oscil26_pint) -= AP1.len);
      }
     else
      {
       NVUI(flute_oscil26_pfrac) = (j = NVUI(flute_oscil26_pfrac)) - NVUI(flute_oscil26_kfrac);
       NVUI(flute_oscil26_pint) -= NVUI(flute_oscil26_kint) + (NVUI(flute_oscil26_pfrac) > j);
       if ((i = NVUI(flute_oscil26_pint)) >= AP1.len)
         i = (NVUI(flute_oscil26_pint) += AP1.len);
      }
     ret = AP1.t[i] + NVUI(flute_oscil26_pfrac)*
           ((float)(1.0/4294967296.0))*(AP1.t[i+1] - AP1.t[i]);
   }
   else
   {
     if ((NVI(flute_oscil26_fsign) = (freq >= 0)))
      {
       nint = (unsigned int)(index = freq*AP1.oconst);
       while (index >= AP1.lenf)
         nint = (unsigned int)(index -= AP1.lenf);
       nfrac = (unsigned int)(4294967296.0F*(index - nint));
      }
     else
      {
       nint = (unsigned int)(index = -freq*AP1.oconst);
       while (index >= AP1.lenf)
         nint = (unsigned int)(index -= AP1.lenf);
       nfrac = (unsigned int)(4294967296.0F*(index - nint));
      }
     NVUI(flute_oscil26_kint) = nint;
     NVUI(flute_oscil26_kfrac) = nfrac;
     NVI(flute_oscil26_kcyc) = 1;
     ret = AP1.t[0];
   }
   return((NV(flute_oscil26_return) = ret));

}



float flute__sym_oscil27(ENGINE_PTR_DECLARE_COMMA struct ninstr_types * nstate)
{
   float freq;
   PSIZE t;
   float ret;
   float index;
   unsigned int nint, nfrac, i, j;

   t =  TBL_GBL_cyc ;

#undef AP1
#define AP1 EV(gtables)[t]

   freq = NV(flute__tvr6);
   if (NVI(flute_oscil27_kcyc))
   {
     if (NVI(flute_oscil27_fsign))
      {
       NVUI(flute_oscil27_pfrac) = (j = NVUI(flute_oscil27_pfrac)) + NVUI(flute_oscil27_kfrac);
       NVUI(flute_oscil27_pint) += NVUI(flute_oscil27_kint) + (NVUI(flute_oscil27_pfrac) < j);
       if ((i = NVUI(flute_oscil27_pint)) >= AP1.len)
         i = (NVUI(flute_oscil27_pint) -= AP1.len);
      }
     else
      {
       NVUI(flute_oscil27_pfrac) = (j = NVUI(flute_oscil27_pfrac)) - NVUI(flute_oscil27_kfrac);
       NVUI(flute_oscil27_pint) -= NVUI(flute_oscil27_kint) + (NVUI(flute_oscil27_pfrac) > j);
       if ((i = NVUI(flute_oscil27_pint)) >= AP1.len)
         i = (NVUI(flute_oscil27_pint) += AP1.len);
      }
     ret = AP1.t[i] + NVUI(flute_oscil27_pfrac)*
           ((float)(1.0/4294967296.0))*(AP1.t[i+1] - AP1.t[i]);
   }
   else
   {
     if ((NVI(flute_oscil27_fsign) = (freq >= 0)))
      {
       nint = (unsigned int)(index = freq*AP1.oconst);
       while (index >= AP1.lenf)
         nint = (unsigned int)(index -= AP1.lenf);
       nfrac = (unsigned int)(4294967296.0F*(index - nint));
      }
     else
      {
       nint = (unsigned int)(index = -freq*AP1.oconst);
       while (index >= AP1.lenf)
         nint = (unsigned int)(index -= AP1.lenf);
       nfrac = (unsigned int)(4294967296.0F*(index - nint));
      }
     NVUI(flute_oscil27_kint) = nint;
     NVUI(flute_oscil27_kfrac) = nfrac;
     NVI(flute_oscil27_kcyc) = 1;
     ret = AP1.t[0];
   }
   return((NV(flute_oscil27_return) = ret));

}



float flute__sym_oscil28(ENGINE_PTR_DECLARE_COMMA struct ninstr_types * nstate)
{
   float freq;
   PSIZE t;
   float ret;
   float index;
   unsigned int nint, nfrac, i, j;

   t =  TBL_GBL_cyc ;

#undef AP1
#define AP1 EV(gtables)[t]

   freq = NV(flute__tvr7);
   if (NVI(flute_oscil28_kcyc))
   {
     if (NVI(flute_oscil28_fsign))
      {
       NVUI(flute_oscil28_pfrac) = (j = NVUI(flute_oscil28_pfrac)) + NVUI(flute_oscil28_kfrac);
       NVUI(flute_oscil28_pint) += NVUI(flute_oscil28_kint) + (NVUI(flute_oscil28_pfrac) < j);
       if ((i = NVUI(flute_oscil28_pint)) >= AP1.len)
         i = (NVUI(flute_oscil28_pint) -= AP1.len);
      }
     else
      {
       NVUI(flute_oscil28_pfrac) = (j = NVUI(flute_oscil28_pfrac)) - NVUI(flute_oscil28_kfrac);
       NVUI(flute_oscil28_pint) -= NVUI(flute_oscil28_kint) + (NVUI(flute_oscil28_pfrac) > j);
       if ((i = NVUI(flute_oscil28_pint)) >= AP1.len)
         i = (NVUI(flute_oscil28_pint) += AP1.len);
      }
     ret = AP1.t[i] + NVUI(flute_oscil28_pfrac)*
           ((float)(1.0/4294967296.0))*(AP1.t[i+1] - AP1.t[i]);
   }
   else
   {
     if ((NVI(flute_oscil28_fsign) = (freq >= 0)))
      {
       nint = (unsigned int)(index = freq*AP1.oconst);
       while (index >= AP1.lenf)
         nint = (unsigned int)(index -= AP1.lenf);
       nfrac = (unsigned int)(4294967296.0F*(index - nint));
      }
     else
      {
       nint = (unsigned int)(index = -freq*AP1.oconst);
       while (index >= AP1.lenf)
         nint = (unsigned int)(index -= AP1.lenf);
       nfrac = (unsigned int)(4294967296.0F*(index - nint));
      }
     NVUI(flute_oscil28_kint) = nint;
     NVUI(flute_oscil28_kfrac) = nfrac;
     NVI(flute_oscil28_kcyc) = 1;
     ret = AP1.t[0];
   }
   return((NV(flute_oscil28_return) = ret));

}



float flute__sym_oscil29(ENGINE_PTR_DECLARE_COMMA struct ninstr_types * nstate)
{
   float freq;
   PSIZE t;
   float ret;
   float index;
   unsigned int nint, nfrac, i, j;

   t =  TBL_GBL_cyc ;

#undef AP1
#define AP1 EV(gtables)[t]

   freq = NV(flute__tvr8);
   if (NVI(flute_oscil29_kcyc))
   {
     if (NVI(flute_oscil29_fsign))
      {
       NVUI(flute_oscil29_pfrac) = (j = NVUI(flute_oscil29_pfrac)) + NVUI(flute_oscil29_kfrac);
       NVUI(flute_oscil29_pint) += NVUI(flute_oscil29_kint) + (NVUI(flute_oscil29_pfrac) < j);
       if ((i = NVUI(flute_oscil29_pint)) >= AP1.len)
         i = (NVUI(flute_oscil29_pint) -= AP1.len);
      }
     else
      {
       NVUI(flute_oscil29_pfrac) = (j = NVUI(flute_oscil29_pfrac)) - NVUI(flute_oscil29_kfrac);
       NVUI(flute_oscil29_pint) -= NVUI(flute_oscil29_kint) + (NVUI(flute_oscil29_pfrac) > j);
       if ((i = NVUI(flute_oscil29_pint)) >= AP1.len)
         i = (NVUI(flute_oscil29_pint) += AP1.len);
      }
     ret = AP1.t[i] + NVUI(flute_oscil29_pfrac)*
           ((float)(1.0/4294967296.0))*(AP1.t[i+1] - AP1.t[i]);
   }
   else
   {
     if ((NVI(flute_oscil29_fsign) = (freq >= 0)))
      {
       nint = (unsigned int)(index = freq*AP1.oconst);
       while (index >= AP1.lenf)
         nint = (unsigned int)(index -= AP1.lenf);
       nfrac = (unsigned int)(4294967296.0F*(index - nint));
      }
     else
      {
       nint = (unsigned int)(index = -freq*AP1.oconst);
       while (index >= AP1.lenf)
         nint = (unsigned int)(index -= AP1.lenf);
       nfrac = (unsigned int)(4294967296.0F*(index - nint));
      }
     NVUI(flute_oscil29_kint) = nint;
     NVUI(flute_oscil29_kfrac) = nfrac;
     NVI(flute_oscil29_kcyc) = 1;
     ret = AP1.t[0];
   }
   return((NV(flute_oscil29_return) = ret));

}



float flute__sym_oscil30(ENGINE_PTR_DECLARE_COMMA struct ninstr_types * nstate)
{
   float freq;
   PSIZE t;
   float ret;
   float index;
   unsigned int nint, nfrac, i, j;

   t =  TBL_GBL_cyc ;

#undef AP1
#define AP1 EV(gtables)[t]

   freq = NV(flute__tvr9);
   if (NVI(flute_oscil30_kcyc))
   {
     if (NVI(flute_oscil30_fsign))
      {
       NVUI(flute_oscil30_pfrac) = (j = NVUI(flute_oscil30_pfrac)) + NVUI(flute_oscil30_kfrac);
       NVUI(flute_oscil30_pint) += NVUI(flute_oscil30_kint) + (NVUI(flute_oscil30_pfrac) < j);
       if ((i = NVUI(flute_oscil30_pint)) >= AP1.len)
         i = (NVUI(flute_oscil30_pint) -= AP1.len);
      }
     else
      {
       NVUI(flute_oscil30_pfrac) = (j = NVUI(flute_oscil30_pfrac)) - NVUI(flute_oscil30_kfrac);
       NVUI(flute_oscil30_pint) -= NVUI(flute_oscil30_kint) + (NVUI(flute_oscil30_pfrac) > j);
       if ((i = NVUI(flute_oscil30_pint)) >= AP1.len)
         i = (NVUI(flute_oscil30_pint) += AP1.len);
      }
     ret = AP1.t[i] + NVUI(flute_oscil30_pfrac)*
           ((float)(1.0/4294967296.0))*(AP1.t[i+1] - AP1.t[i]);
   }
   else
   {
     if ((NVI(flute_oscil30_fsign) = (freq >= 0)))
      {
       nint = (unsigned int)(index = freq*AP1.oconst);
       while (index >= AP1.lenf)
         nint = (unsigned int)(index -= AP1.lenf);
       nfrac = (unsigned int)(4294967296.0F*(index - nint));
      }
     else
      {
       nint = (unsigned int)(index = -freq*AP1.oconst);
       while (index >= AP1.lenf)
         nint = (unsigned int)(index -= AP1.lenf);
       nfrac = (unsigned int)(4294967296.0F*(index - nint));
      }
     NVUI(flute_oscil30_kint) = nint;
     NVUI(flute_oscil30_kfrac) = nfrac;
     NVI(flute_oscil30_kcyc) = 1;
     ret = AP1.t[0];
   }
   return((NV(flute_oscil30_return) = ret));

}



float flute__sym_oscil31(ENGINE_PTR_DECLARE_COMMA struct ninstr_types * nstate)
{
   float freq;
   PSIZE t;
   float ret;
   float index;
   unsigned int nint, nfrac, i, j;

   t =  TBL_GBL_cyc ;

#undef AP1
#define AP1 EV(gtables)[t]

   freq = NV(flute__tvr10);
   if (NVI(flute_oscil31_kcyc))
   {
     if (NVI(flute_oscil31_fsign))
      {
       NVUI(flute_oscil31_pfrac) = (j = NVUI(flute_oscil31_pfrac)) + NVUI(flute_oscil31_kfrac);
       NVUI(flute_oscil31_pint) += NVUI(flute_oscil31_kint) + (NVUI(flute_oscil31_pfrac) < j);
       if ((i = NVUI(flute_oscil31_pint)) >= AP1.len)
         i = (NVUI(flute_oscil31_pint) -= AP1.len);
      }
     else
      {
       NVUI(flute_oscil31_pfrac) = (j = NVUI(flute_oscil31_pfrac)) - NVUI(flute_oscil31_kfrac);
       NVUI(flute_oscil31_pint) -= NVUI(flute_oscil31_kint) + (NVUI(flute_oscil31_pfrac) > j);
       if ((i = NVUI(flute_oscil31_pint)) >= AP1.len)
         i = (NVUI(flute_oscil31_pint) += AP1.len);
      }
     ret = AP1.t[i] + NVUI(flute_oscil31_pfrac)*
           ((float)(1.0/4294967296.0))*(AP1.t[i+1] - AP1.t[i]);
   }
   else
   {
     if ((NVI(flute_oscil31_fsign) = (freq >= 0)))
      {
       nint = (unsigned int)(index = freq*AP1.oconst);
       while (index >= AP1.lenf)
         nint = (unsigned int)(index -= AP1.lenf);
       nfrac = (unsigned int)(4294967296.0F*(index - nint));
      }
     else
      {
       nint = (unsigned int)(index = -freq*AP1.oconst);
       while (index >= AP1.lenf)
         nint = (unsigned int)(index -= AP1.lenf);
       nfrac = (unsigned int)(4294967296.0F*(index - nint));
      }
     NVUI(flute_oscil31_kint) = nint;
     NVUI(flute_oscil31_kfrac) = nfrac;
     NVI(flute_oscil31_kcyc) = 1;
     ret = AP1.t[0];
   }
   return((NV(flute_oscil31_return) = ret));

}



float flute__sym_oscil32(ENGINE_PTR_DECLARE_COMMA struct ninstr_types * nstate)
{
   float freq;
   PSIZE t;
   float ret;
   float index;
   unsigned int nint, nfrac, i, j;

   t =  TBL_GBL_cyc ;

#undef AP1
#define AP1 EV(gtables)[t]

   freq = NV(flute__tvr11);
   if (NVI(flute_oscil32_kcyc))
   {
     if (NVI(flute_oscil32_fsign))
      {
       NVUI(flute_oscil32_pfrac) = (j = NVUI(flute_oscil32_pfrac)) + NVUI(flute_oscil32_kfrac);
       NVUI(flute_oscil32_pint) += NVUI(flute_oscil32_kint) + (NVUI(flute_oscil32_pfrac) < j);
       if ((i = NVUI(flute_oscil32_pint)) >= AP1.len)
         i = (NVUI(flute_oscil32_pint) -= AP1.len);
      }
     else
      {
       NVUI(flute_oscil32_pfrac) = (j = NVUI(flute_oscil32_pfrac)) - NVUI(flute_oscil32_kfrac);
       NVUI(flute_oscil32_pint) -= NVUI(flute_oscil32_kint) + (NVUI(flute_oscil32_pfrac) > j);
       if ((i = NVUI(flute_oscil32_pint)) >= AP1.len)
         i = (NVUI(flute_oscil32_pint) += AP1.len);
      }
     ret = AP1.t[i] + NVUI(flute_oscil32_pfrac)*
           ((float)(1.0/4294967296.0))*(AP1.t[i+1] - AP1.t[i]);
   }
   else
   {
     if ((NVI(flute_oscil32_fsign) = (freq >= 0)))
      {
       nint = (unsigned int)(index = freq*AP1.oconst);
       while (index >= AP1.lenf)
         nint = (unsigned int)(index -= AP1.lenf);
       nfrac = (unsigned int)(4294967296.0F*(index - nint));
      }
     else
      {
       nint = (unsigned int)(index = -freq*AP1.oconst);
       while (index >= AP1.lenf)
         nint = (unsigned int)(index -= AP1.lenf);
       nfrac = (unsigned int)(4294967296.0F*(index - nint));
      }
     NVUI(flute_oscil32_kint) = nint;
     NVUI(flute_oscil32_kfrac) = nfrac;
     NVI(flute_oscil32_kcyc) = 1;
     ret = AP1.t[0];
   }
   return((NV(flute_oscil32_return) = ret));

}



float flute__sym_oscil33(ENGINE_PTR_DECLARE_COMMA struct ninstr_types * nstate)
{
   float freq;
   PSIZE t;
   float ret;
   float index;
   unsigned int nint, nfrac, i, j;

   t =  TBL_GBL_cyc ;

#undef AP1
#define AP1 EV(gtables)[t]

   freq = NV(flute__tvr12);
   if (NVI(flute_oscil33_kcyc))
   {
     if (NVI(flute_oscil33_fsign))
      {
       NVUI(flute_oscil33_pfrac) = (j = NVUI(flute_oscil33_pfrac)) + NVUI(flute_oscil33_kfrac);
       NVUI(flute_oscil33_pint) += NVUI(flute_oscil33_kint) + (NVUI(flute_oscil33_pfrac) < j);
       if ((i = NVUI(flute_oscil33_pint)) >= AP1.len)
         i = (NVUI(flute_oscil33_pint) -= AP1.len);
      }
     else
      {
       NVUI(flute_oscil33_pfrac) = (j = NVUI(flute_oscil33_pfrac)) - NVUI(flute_oscil33_kfrac);
       NVUI(flute_oscil33_pint) -= NVUI(flute_oscil33_kint) + (NVUI(flute_oscil33_pfrac) > j);
       if ((i = NVUI(flute_oscil33_pint)) >= AP1.len)
         i = (NVUI(flute_oscil33_pint) += AP1.len);
      }
     ret = AP1.t[i] + NVUI(flute_oscil33_pfrac)*
           ((float)(1.0/4294967296.0))*(AP1.t[i+1] - AP1.t[i]);
   }
   else
   {
     if ((NVI(flute_oscil33_fsign) = (freq >= 0)))
      {
       nint = (unsigned int)(index = freq*AP1.oconst);
       while (index >= AP1.lenf)
         nint = (unsigned int)(index -= AP1.lenf);
       nfrac = (unsigned int)(4294967296.0F*(index - nint));
      }
     else
      {
       nint = (unsigned int)(index = -freq*AP1.oconst);
       while (index >= AP1.lenf)
         nint = (unsigned int)(index -= AP1.lenf);
       nfrac = (unsigned int)(4294967296.0F*(index - nint));
      }
     NVUI(flute_oscil33_kint) = nint;
     NVUI(flute_oscil33_kfrac) = nfrac;
     NVI(flute_oscil33_kcyc) = 1;
     ret = AP1.t[0];
   }
   return((NV(flute_oscil33_return) = ret));

}



float flute__sym_oscil34(ENGINE_PTR_DECLARE_COMMA struct ninstr_types * nstate)
{
   float freq;
   PSIZE t;
   float ret;
   float index;
   unsigned int nint, nfrac, i, j;

   t =  TBL_GBL_cyc ;

#undef AP1
#define AP1 EV(gtables)[t]

   freq = NV(flute__tvr13);
   if (NVI(flute_oscil34_kcyc))
   {
     if (NVI(flute_oscil34_fsign))
      {
       NVUI(flute_oscil34_pfrac) = (j = NVUI(flute_oscil34_pfrac)) + NVUI(flute_oscil34_kfrac);
       NVUI(flute_oscil34_pint) += NVUI(flute_oscil34_kint) + (NVUI(flute_oscil34_pfrac) < j);
       if ((i = NVUI(flute_oscil34_pint)) >= AP1.len)
         i = (NVUI(flute_oscil34_pint) -= AP1.len);
      }
     else
      {
       NVUI(flute_oscil34_pfrac) = (j = NVUI(flute_oscil34_pfrac)) - NVUI(flute_oscil34_kfrac);
       NVUI(flute_oscil34_pint) -= NVUI(flute_oscil34_kint) + (NVUI(flute_oscil34_pfrac) > j);
       if ((i = NVUI(flute_oscil34_pint)) >= AP1.len)
         i = (NVUI(flute_oscil34_pint) += AP1.len);
      }
     ret = AP1.t[i] + NVUI(flute_oscil34_pfrac)*
           ((float)(1.0/4294967296.0))*(AP1.t[i+1] - AP1.t[i]);
   }
   else
   {
     if ((NVI(flute_oscil34_fsign) = (freq >= 0)))
      {
       nint = (unsigned int)(index = freq*AP1.oconst);
       while (index >= AP1.lenf)
         nint = (unsigned int)(index -= AP1.lenf);
       nfrac = (unsigned int)(4294967296.0F*(index - nint));
      }
     else
      {
       nint = (unsigned int)(index = -freq*AP1.oconst);
       while (index >= AP1.lenf)
         nint = (unsigned int)(index -= AP1.lenf);
       nfrac = (unsigned int)(4294967296.0F*(index - nint));
      }
     NVUI(flute_oscil34_kint) = nint;
     NVUI(flute_oscil34_kfrac) = nfrac;
     NVI(flute_oscil34_kcyc) = 1;
     ret = AP1.t[0];
   }
   return((NV(flute_oscil34_return) = ret));

}



float flute__sym_oscil35(ENGINE_PTR_DECLARE_COMMA struct ninstr_types * nstate)
{
   float freq;
   PSIZE t;
   float ret;
   float index;
   unsigned int nint, nfrac, i, j;

   t =  TBL_GBL_cyc ;

#undef AP1
#define AP1 EV(gtables)[t]

   freq = NV(flute__tvr14);
   if (NVI(flute_oscil35_kcyc))
   {
     if (NVI(flute_oscil35_fsign))
      {
       NVUI(flute_oscil35_pfrac) = (j = NVUI(flute_oscil35_pfrac)) + NVUI(flute_oscil35_kfrac);
       NVUI(flute_oscil35_pint) += NVUI(flute_oscil35_kint) + (NVUI(flute_oscil35_pfrac) < j);
       if ((i = NVUI(flute_oscil35_pint)) >= AP1.len)
         i = (NVUI(flute_oscil35_pint) -= AP1.len);
      }
     else
      {
       NVUI(flute_oscil35_pfrac) = (j = NVUI(flute_oscil35_pfrac)) - NVUI(flute_oscil35_kfrac);
       NVUI(flute_oscil35_pint) -= NVUI(flute_oscil35_kint) + (NVUI(flute_oscil35_pfrac) > j);
       if ((i = NVUI(flute_oscil35_pint)) >= AP1.len)
         i = (NVUI(flute_oscil35_pint) += AP1.len);
      }
     ret = AP1.t[i] + NVUI(flute_oscil35_pfrac)*
           ((float)(1.0/4294967296.0))*(AP1.t[i+1] - AP1.t[i]);
   }
   else
   {
     if ((NVI(flute_oscil35_fsign) = (freq >= 0)))
      {
       nint = (unsigned int)(index = freq*AP1.oconst);
       while (index >= AP1.lenf)
         nint = (unsigned int)(index -= AP1.lenf);
       nfrac = (unsigned int)(4294967296.0F*(index - nint));
      }
     else
      {
       nint = (unsigned int)(index = -freq*AP1.oconst);
       while (index >= AP1.lenf)
         nint = (unsigned int)(index -= AP1.lenf);
       nfrac = (unsigned int)(4294967296.0F*(index - nint));
      }
     NVUI(flute_oscil35_kint) = nint;
     NVUI(flute_oscil35_kfrac) = nfrac;
     NVI(flute_oscil35_kcyc) = 1;
     ret = AP1.t[0];
   }
   return((NV(flute_oscil35_return) = ret));

}



float flute__sym_oscil36(ENGINE_PTR_DECLARE_COMMA struct ninstr_types * nstate)
{
   float freq;
   PSIZE t;
   float ret;
   float index;
   unsigned int nint, nfrac, i, j;

   t =  TBL_GBL_cyc ;

#undef AP1
#define AP1 EV(gtables)[t]

   freq = NV(flute__tvr15);
   if (NVI(flute_oscil36_kcyc))
   {
     if (NVI(flute_oscil36_fsign))
      {
       NVUI(flute_oscil36_pfrac) = (j = NVUI(flute_oscil36_pfrac)) + NVUI(flute_oscil36_kfrac);
       NVUI(flute_oscil36_pint) += NVUI(flute_oscil36_kint) + (NVUI(flute_oscil36_pfrac) < j);
       if ((i = NVUI(flute_oscil36_pint)) >= AP1.len)
         i = (NVUI(flute_oscil36_pint) -= AP1.len);
      }
     else
      {
       NVUI(flute_oscil36_pfrac) = (j = NVUI(flute_oscil36_pfrac)) - NVUI(flute_oscil36_kfrac);
       NVUI(flute_oscil36_pint) -= NVUI(flute_oscil36_kint) + (NVUI(flute_oscil36_pfrac) > j);
       if ((i = NVUI(flute_oscil36_pint)) >= AP1.len)
         i = (NVUI(flute_oscil36_pint) += AP1.len);
      }
     ret = AP1.t[i] + NVUI(flute_oscil36_pfrac)*
           ((float)(1.0/4294967296.0))*(AP1.t[i+1] - AP1.t[i]);
   }
   else
   {
     if ((NVI(flute_oscil36_fsign) = (freq >= 0)))
      {
       nint = (unsigned int)(index = freq*AP1.oconst);
       while (index >= AP1.lenf)
         nint = (unsigned int)(index -= AP1.lenf);
       nfrac = (unsigned int)(4294967296.0F*(index - nint));
      }
     else
      {
       nint = (unsigned int)(index = -freq*AP1.oconst);
       while (index >= AP1.lenf)
         nint = (unsigned int)(index -= AP1.lenf);
       nfrac = (unsigned int)(4294967296.0F*(index - nint));
      }
     NVUI(flute_oscil36_kint) = nint;
     NVUI(flute_oscil36_kfrac) = nfrac;
     NVI(flute_oscil36_kcyc) = 1;
     ret = AP1.t[0];
   }
   return((NV(flute_oscil36_return) = ret));

}



float flute__sym_oscil37(ENGINE_PTR_DECLARE_COMMA struct ninstr_types * nstate)
{
   float freq;
   PSIZE t;
   float ret;
   float index;
   unsigned int nint, nfrac, i, j;

   t =  TBL_GBL_cyc ;

#undef AP1
#define AP1 EV(gtables)[t]

   freq = NV(flute__tvr16);
   if (NVI(flute_oscil37_kcyc))
   {
     if (NVI(flute_oscil37_fsign))
      {
       NVUI(flute_oscil37_pfrac) = (j = NVUI(flute_oscil37_pfrac)) + NVUI(flute_oscil37_kfrac);
       NVUI(flute_oscil37_pint) += NVUI(flute_oscil37_kint) + (NVUI(flute_oscil37_pfrac) < j);
       if ((i = NVUI(flute_oscil37_pint)) >= AP1.len)
         i = (NVUI(flute_oscil37_pint) -= AP1.len);
      }
     else
      {
       NVUI(flute_oscil37_pfrac) = (j = NVUI(flute_oscil37_pfrac)) - NVUI(flute_oscil37_kfrac);
       NVUI(flute_oscil37_pint) -= NVUI(flute_oscil37_kint) + (NVUI(flute_oscil37_pfrac) > j);
       if ((i = NVUI(flute_oscil37_pint)) >= AP1.len)
         i = (NVUI(flute_oscil37_pint) += AP1.len);
      }
     ret = AP1.t[i] + NVUI(flute_oscil37_pfrac)*
           ((float)(1.0/4294967296.0))*(AP1.t[i+1] - AP1.t[i]);
   }
   else
   {
     if ((NVI(flute_oscil37_fsign) = (freq >= 0)))
      {
       nint = (unsigned int)(index = freq*AP1.oconst);
       while (index >= AP1.lenf)
         nint = (unsigned int)(index -= AP1.lenf);
       nfrac = (unsigned int)(4294967296.0F*(index - nint));
      }
     else
      {
       nint = (unsigned int)(index = -freq*AP1.oconst);
       while (index >= AP1.lenf)
         nint = (unsigned int)(index -= AP1.lenf);
       nfrac = (unsigned int)(4294967296.0F*(index - nint));
      }
     NVUI(flute_oscil37_kint) = nint;
     NVUI(flute_oscil37_kfrac) = nfrac;
     NVI(flute_oscil37_kcyc) = 1;
     ret = AP1.t[0];
   }
   return((NV(flute_oscil37_return) = ret));

}



float flute__sym_oscil38(ENGINE_PTR_DECLARE_COMMA struct ninstr_types * nstate)
{
   float freq;
   PSIZE t;
   float ret;
   float index;
   unsigned int nint, nfrac, i, j;

   t =  TBL_GBL_cyc ;

#undef AP1
#define AP1 EV(gtables)[t]

   freq = NV(flute__tvr17);
   if (NVI(flute_oscil38_kcyc))
   {
     if (NVI(flute_oscil38_fsign))
      {
       NVUI(flute_oscil38_pfrac) = (j = NVUI(flute_oscil38_pfrac)) + NVUI(flute_oscil38_kfrac);
       NVUI(flute_oscil38_pint) += NVUI(flute_oscil38_kint) + (NVUI(flute_oscil38_pfrac) < j);
       if ((i = NVUI(flute_oscil38_pint)) >= AP1.len)
         i = (NVUI(flute_oscil38_pint) -= AP1.len);
      }
     else
      {
       NVUI(flute_oscil38_pfrac) = (j = NVUI(flute_oscil38_pfrac)) - NVUI(flute_oscil38_kfrac);
       NVUI(flute_oscil38_pint) -= NVUI(flute_oscil38_kint) + (NVUI(flute_oscil38_pfrac) > j);
       if ((i = NVUI(flute_oscil38_pint)) >= AP1.len)
         i = (NVUI(flute_oscil38_pint) += AP1.len);
      }
     ret = AP1.t[i] + NVUI(flute_oscil38_pfrac)*
           ((float)(1.0/4294967296.0))*(AP1.t[i+1] - AP1.t[i]);
   }
   else
   {
     if ((NVI(flute_oscil38_fsign) = (freq >= 0)))
      {
       nint = (unsigned int)(index = freq*AP1.oconst);
       while (index >= AP1.lenf)
         nint = (unsigned int)(index -= AP1.lenf);
       nfrac = (unsigned int)(4294967296.0F*(index - nint));
      }
     else
      {
       nint = (unsigned int)(index = -freq*AP1.oconst);
       while (index >= AP1.lenf)
         nint = (unsigned int)(index -= AP1.lenf);
       nfrac = (unsigned int)(4294967296.0F*(index - nint));
      }
     NVUI(flute_oscil38_kint) = nint;
     NVUI(flute_oscil38_kfrac) = nfrac;
     NVI(flute_oscil38_kcyc) = 1;
     ret = AP1.t[0];
   }
   return((NV(flute_oscil38_return) = ret));

}



float flute__sym_oscil39(ENGINE_PTR_DECLARE_COMMA struct ninstr_types * nstate)
{
   float freq;
   PSIZE t;
   float ret;
   float index;
   unsigned int nint, nfrac, i, j;

   t =  TBL_GBL_cyc ;

#undef AP1
#define AP1 EV(gtables)[t]

   freq = NV(flute__tvr18);
   if (NVI(flute_oscil39_kcyc))
   {
     if (NVI(flute_oscil39_fsign))
      {
       NVUI(flute_oscil39_pfrac) = (j = NVUI(flute_oscil39_pfrac)) + NVUI(flute_oscil39_kfrac);
       NVUI(flute_oscil39_pint) += NVUI(flute_oscil39_kint) + (NVUI(flute_oscil39_pfrac) < j);
       if ((i = NVUI(flute_oscil39_pint)) >= AP1.len)
         i = (NVUI(flute_oscil39_pint) -= AP1.len);
      }
     else
      {
       NVUI(flute_oscil39_pfrac) = (j = NVUI(flute_oscil39_pfrac)) - NVUI(flute_oscil39_kfrac);
       NVUI(flute_oscil39_pint) -= NVUI(flute_oscil39_kint) + (NVUI(flute_oscil39_pfrac) > j);
       if ((i = NVUI(flute_oscil39_pint)) >= AP1.len)
         i = (NVUI(flute_oscil39_pint) += AP1.len);
      }
     ret = AP1.t[i] + NVUI(flute_oscil39_pfrac)*
           ((float)(1.0/4294967296.0))*(AP1.t[i+1] - AP1.t[i]);
   }
   else
   {
     if ((NVI(flute_oscil39_fsign) = (freq >= 0)))
      {
       nint = (unsigned int)(index = freq*AP1.oconst);
       while (index >= AP1.lenf)
         nint = (unsigned int)(index -= AP1.lenf);
       nfrac = (unsigned int)(4294967296.0F*(index - nint));
      }
     else
      {
       nint = (unsigned int)(index = -freq*AP1.oconst);
       while (index >= AP1.lenf)
         nint = (unsigned int)(index -= AP1.lenf);
       nfrac = (unsigned int)(4294967296.0F*(index - nint));
      }
     NVUI(flute_oscil39_kint) = nint;
     NVUI(flute_oscil39_kfrac) = nfrac;
     NVI(flute_oscil39_kcyc) = 1;
     ret = AP1.t[0];
   }
   return((NV(flute_oscil39_return) = ret));

}



float flute__sym_oscil40(ENGINE_PTR_DECLARE_COMMA struct ninstr_types * nstate)
{
   float freq;
   PSIZE t;
   float ret;
   float index;
   unsigned int nint, nfrac, i, j;

   t =  TBL_GBL_cyc ;

#undef AP1
#define AP1 EV(gtables)[t]

   freq = NV(flute__tvr19);
   if (NVI(flute_oscil40_kcyc))
   {
     if (NVI(flute_oscil40_fsign))
      {
       NVUI(flute_oscil40_pfrac) = (j = NVUI(flute_oscil40_pfrac)) + NVUI(flute_oscil40_kfrac);
       NVUI(flute_oscil40_pint) += NVUI(flute_oscil40_kint) + (NVUI(flute_oscil40_pfrac) < j);
       if ((i = NVUI(flute_oscil40_pint)) >= AP1.len)
         i = (NVUI(flute_oscil40_pint) -= AP1.len);
      }
     else
      {
       NVUI(flute_oscil40_pfrac) = (j = NVUI(flute_oscil40_pfrac)) - NVUI(flute_oscil40_kfrac);
       NVUI(flute_oscil40_pint) -= NVUI(flute_oscil40_kint) + (NVUI(flute_oscil40_pfrac) > j);
       if ((i = NVUI(flute_oscil40_pint)) >= AP1.len)
         i = (NVUI(flute_oscil40_pint) += AP1.len);
      }
     ret = AP1.t[i] + NVUI(flute_oscil40_pfrac)*
           ((float)(1.0/4294967296.0))*(AP1.t[i+1] - AP1.t[i]);
   }
   else
   {
     if ((NVI(flute_oscil40_fsign) = (freq >= 0)))
      {
       nint = (unsigned int)(index = freq*AP1.oconst);
       while (index >= AP1.lenf)
         nint = (unsigned int)(index -= AP1.lenf);
       nfrac = (unsigned int)(4294967296.0F*(index - nint));
      }
     else
      {
       nint = (unsigned int)(index = -freq*AP1.oconst);
       while (index >= AP1.lenf)
         nint = (unsigned int)(index -= AP1.lenf);
       nfrac = (unsigned int)(4294967296.0F*(index - nint));
      }
     NVUI(flute_oscil40_kint) = nint;
     NVUI(flute_oscil40_kfrac) = nfrac;
     NVI(flute_oscil40_kcyc) = 1;
     ret = AP1.t[0];
   }
   return((NV(flute_oscil40_return) = ret));

}


#undef NS
#define NS(x) nstate->x
#undef NSP
#define NSP  ENGINE_PTR_COMMA nstate
#undef NT
#define NT(x)  nstate->t[x]
#undef NV
#define NV(x)  nstate->v[x].f
#undef NVI
#define NVI(x)  nstate->v[x].i
#undef NVUI
#define NVUI(x)  nstate->v[x].ui
#undef NVPS
#define NVPS(x)  nstate->v[x].ps
#undef NVU
#define NVU(x)  nstate->v[x]
#undef NP
#define NP(x)  nstate->v[x].f
#undef NPI
#define NPI(x)  nstate->v[x].i
#undef NPUI
#define NPUI(x)  nstate->v[x].ui
#undef NPPS
#define NPPS(x)  nstate->v[x].ps

void flute_ipass(ENGINE_PTR_DECLARE_COMMA struct ninstr_types * nstate)
{
   int i;

memset(&(NVU(0)), 0, flute_ENDVAR*sizeof(NVU(0)));
memset(&(NT(0)), 0, flute_ENDTBL*sizeof(NT(0)));
   NV(flute_fr) = 
   NS(iline->p[flute_fr]);
NV(flute__tvr0) = NV(flute_fr) *  1.0F ;
 NV(flute__tvr1) = NV(flute_fr) *  2.0F ;
 NV(flute__tvr2) = NV(flute_fr) *  3.0F ;
 NV(flute__tvr3) = NV(flute_fr) *  4.0F ;
 NV(flute__tvr4) = NV(flute_fr) *  5.0F ;
 NV(flute__tvr5) = NV(flute_fr) *  6.0F ;
 NV(flute__tvr6) = NV(flute_fr) *  7.0F ;
 NV(flute__tvr7) = NV(flute_fr) *  8.0F ;
 NV(flute__tvr8) = NV(flute_fr) *  9.0F ;
 NV(flute__tvr9) = NV(flute_fr) *  10.0F ;
 NV(flute__tvr10) = NV(flute_fr) *  11.0F ;
 NV(flute__tvr11) = NV(flute_fr) *  12.0F ;
 NV(flute__tvr12) = NV(flute_fr) *  13.0F ;
 NV(flute__tvr13) = NV(flute_fr) *  14.0F ;
 NV(flute__tvr14) = NV(flute_fr) *  15.0F ;
 NV(flute__tvr15) = NV(flute_fr) *  16.0F ;
 NV(flute__tvr16) = NV(flute_fr) *  17.0F ;
 NV(flute__tvr17) = NV(flute_fr) *  18.0F ;
 NV(flute__tvr18) = NV(flute_fr) *  19.0F ;
 NV(flute__tvr19) = NV(flute_fr) *  20.0F ;
 
}


#undef NS
#define NS(x) nstate->x
#undef NSP
#define NSP  ENGINE_PTR_COMMA nstate
#undef NT
#define NT(x)  nstate->t[x]
#undef NV
#define NV(x)  nstate->v[x].f
#undef NVI
#define NVI(x)  nstate->v[x].i
#undef NVUI
#define NVUI(x)  nstate->v[x].ui
#undef NVPS
#define NVPS(x)  nstate->v[x].ps
#undef NVU
#define NVU(x)  nstate->v[x]
#undef NP
#define NP(x)  nstate->v[x].f
#undef NPI
#define NPI(x)  nstate->v[x].i
#undef NPUI
#define NPUI(x)  nstate->v[x].ui
#undef NPPS
#define NPPS(x)  nstate->v[x].ps

void flute_kpass(ENGINE_PTR_DECLARE_COMMA struct ninstr_types * nstate)
{

   int i;

NV(flute_env01) = flute__sym_kline1(NSP);
 NV(flute_env02) = flute__sym_kline2(NSP);
 NV(flute_env03) = flute__sym_kline3(NSP);
 NV(flute_env04) = flute__sym_kline4(NSP);
 NV(flute_env05) = flute__sym_kline5(NSP);
 NV(flute_env06) = flute__sym_kline6(NSP);
 NV(flute_env07) = flute__sym_kline7(NSP);
 NV(flute_env08) = flute__sym_kline8(NSP);
 NV(flute_env09) = flute__sym_kline9(NSP);
 NV(flute_env10) = flute__sym_kline10(NSP);
 NV(flute_env11) = flute__sym_kline11(NSP);
 NV(flute_env12) = flute__sym_kline12(NSP);
 NV(flute_env13) = flute__sym_kline13(NSP);
 NV(flute_env14) = flute__sym_kline14(NSP);
 NV(flute_env15) = flute__sym_kline15(NSP);
 NV(flute_env16) = flute__sym_kline16(NSP);
 NV(flute_env17) = flute__sym_kline17(NSP);
 NV(flute_env18) = flute__sym_kline18(NSP);
 NV(flute_env19) = flute__sym_kline19(NSP);
 NV(flute_env20) = flute__sym_kline20(NSP);
 
}


#undef NS
#define NS(x) nstate->x
#undef NSP
#define NSP  ENGINE_PTR_COMMA nstate
#undef NT
#define NT(x)  nstate->t[x]
#undef NV
#define NV(x)  nstate->v[x].f
#undef NVI
#define NVI(x)  nstate->v[x].i
#undef NVUI
#define NVUI(x)  nstate->v[x].ui
#undef NVPS
#define NVPS(x)  nstate->v[x].ps
#undef NVU
#define NVU(x)  nstate->v[x]
#undef NP
#define NP(x)  nstate->v[x].f
#undef NPI
#define NPI(x)  nstate->v[x].i
#undef NPUI
#define NPUI(x)  nstate->v[x].ui
#undef NPPS
#define NPPS(x)  nstate->v[x].ps

void flute_apass(ENGINE_PTR_DECLARE_COMMA struct ninstr_types * nstate)
{

NV(flute_y01) = NV(flute_env01) * flute__sym_oscil21(NSP);
 NV(flute_y02) = NV(flute_env02) * flute__sym_oscil22(NSP);
 NV(flute_y03) = NV(flute_env03) * flute__sym_oscil23(NSP);
 NV(flute_y04) = NV(flute_env04) * flute__sym_oscil24(NSP);
 NV(flute_y05) = NV(flute_env05) * flute__sym_oscil25(NSP);
 NV(flute_y06) = NV(flute_env06) * flute__sym_oscil26(NSP);
 NV(flute_y07) = NV(flute_env07) * flute__sym_oscil27(NSP);
 NV(flute_y08) = NV(flute_env08) * flute__sym_oscil28(NSP);
 NV(flute_y09) = NV(flute_env09) * flute__sym_oscil29(NSP);
 NV(flute_y10) = NV(flute_env10) * flute__sym_oscil30(NSP);
 NV(flute_y11) = NV(flute_env11) * flute__sym_oscil31(NSP);
 NV(flute_y12) = NV(flute_env12) * flute__sym_oscil32(NSP);
 NV(flute_y13) = NV(flute_env13) * flute__sym_oscil33(NSP);
 NV(flute_y14) = NV(flute_env14) * flute__sym_oscil34(NSP);
 NV(flute_y15) = NV(flute_env15) * flute__sym_oscil35(NSP);
 NV(flute_y16) = NV(flute_env16) * flute__sym_oscil36(NSP);
 NV(flute_y17) = NV(flute_env17) * flute__sym_oscil37(NSP);
 NV(flute_y18) = NV(flute_env18) * flute__sym_oscil38(NSP);
 NV(flute_y19) = NV(flute_env19) * flute__sym_oscil39(NSP);
 NV(flute_y20) = NV(flute_env20) * flute__sym_oscil40(NSP);
 TB(BUS_output_bus + 0) += NV(flute_y01) + NV(flute_y02) + NV(flute_y03) + NV(flute_y04) + NV(flute_y05) + NV(flute_y06) + NV(flute_y07) + NV(flute_y08) + NV(flute_y09) + NV(flute_y10) + NV(flute_y11) + NV(flute_y12) + NV(flute_y13) + NV(flute_y14) + NV(flute_y15) + NV(flute_y16) + NV(flute_y17) + NV(flute_y18) + NV(flute_y19) + NV(flute_y20);
}


#undef NS
#define NS(x) nstate->x
#undef NSP
#define NSP  ENGINE_PTR_COMMA nstate
#undef NT
#define NT(x)  nstate->t[x]
#undef NV
#define NV(x)  nstate->v[x].f
#undef NVI
#define NVI(x)  nstate->v[x].i
#undef NVUI
#define NVUI(x)  nstate->v[x].ui
#undef NVPS
#define NVPS(x)  nstate->v[x].ps
#undef NVU
#define NVU(x)  nstate->v[x]
#undef NP
#define NP(x)  nstate->v[x].f
#undef NPI
#define NPI(x)  nstate->v[x].i
#undef NPUI
#define NPUI(x)  nstate->v[x].ui
#undef NPPS
#define NPPS(x)  nstate->v[x].ps


#undef NS
#define NS(x) 0
#undef NSP
#define NSP  ENGINE_PTR_COMMA NULL
#undef NT
#define NT(x)  EV(gtables)[x]
#undef NV
#define NV(x)  EV(global)[x].f
#undef NVI
#define NVI(x)  EV(global)[x].i
#undef NVUI
#define NVUI(x)  EV(global)[x].ui
#undef NVPS
#define NVPS(x)  EV(global)[x].ps
#undef NVU
#define NVU(x)  EV(global)[x]
#undef NP
#define NP(x)  EV(global)[x].f
#undef NPI
#define NPI(x)  EV(global)[x].i
#undef NPUI
#define NPUI(x)  EV(global)[x].ui
#undef NPPS
#define NPPS(x)  EV(global)[x].ps


#undef NS
#define NS(x) nstate->x
#undef NSP
#define NSP  ENGINE_PTR_COMMA nstate
#undef NT
#define NT(x)  nstate->t[x]
#undef NV
#define NV(x)  nstate->v[x].f
#undef NVI
#define NVI(x)  nstate->v[x].i
#undef NVUI
#define NVUI(x)  nstate->v[x].ui
#undef NVPS
#define NVPS(x)  nstate->v[x].ps
#undef NVU
#define NVU(x)  nstate->v[x]
#undef NP
#define NP(x)  nstate->v[x].f
#undef NPI
#define NPI(x)  nstate->v[x].i
#undef NPUI
#define NPUI(x)  nstate->v[x].ui
#undef NPPS
#define NPPS(x)  nstate->v[x].ps

float table_global_cyc[129] = { 
0.0F,4.906767e-02F,9.801713e-02F,1.467305e-01F,1.950903e-01F,2.429802e-01F,2.902847e-01F,3.368898e-01F,
3.826834e-01F,4.275551e-01F,4.713967e-01F,5.141028e-01F,5.555702e-01F,5.956993e-01F,6.343933e-01F,6.715589e-01F,
7.071068e-01F,7.409511e-01F,7.730104e-01F,8.032075e-01F,8.314696e-01F,8.577286e-01F,8.819212e-01F,9.039893e-01F,
9.238795e-01F,9.415441e-01F,9.569403e-01F,9.700313e-01F,9.807853e-01F,9.891765e-01F,9.951847e-01F,9.987954e-01F,
1.0F,9.987954e-01F,9.951847e-01F,9.891765e-01F,9.807853e-01F,9.700313e-01F,9.569404e-01F,9.415441e-01F,
9.238796e-01F,9.039893e-01F,8.819213e-01F,8.577287e-01F,8.314697e-01F,8.032076e-01F,7.730105e-01F,7.409512e-01F,
7.071068e-01F,6.715590e-01F,6.343935e-01F,5.956994e-01F,5.555704e-01F,5.141028e-01F,4.713968e-01F,4.275553e-01F,
3.826835e-01F,3.368900e-01F,2.902847e-01F,2.429803e-01F,1.950905e-01F,1.467306e-01F,9.801733e-02F,4.906772e-02F,
1.509958e-07F,-4.906742e-02F,-9.801703e-02F,-1.467303e-01F,-1.950902e-01F,-2.429800e-01F,-2.902844e-01F,-3.368897e-01F,
-3.826832e-01F,-4.275550e-01F,-4.713966e-01F,-5.141025e-01F,-5.555701e-01F,-5.956991e-01F,-6.343932e-01F,-6.715588e-01F,
-7.071066e-01F,-7.409510e-01F,-7.730104e-01F,-8.032073e-01F,-8.314695e-01F,-8.577285e-01F,-8.819211e-01F,-9.039892e-01F,
-9.238795e-01F,-9.415441e-01F,-9.569402e-01F,-9.700312e-01F,-9.807853e-01F,-9.891765e-01F,-9.951847e-01F,-9.987954e-01F,
-1.0F,-9.987954e-01F,-9.951847e-01F,-9.891765e-01F,-9.807854e-01F,-9.700313e-01F,-9.569404e-01F,-9.415442e-01F,
-9.238796e-01F,-9.039894e-01F,-8.819213e-01F,-8.577288e-01F,-8.314697e-01F,-8.032076e-01F,-7.730107e-01F,-7.409513e-01F,
-7.071069e-01F,-6.715593e-01F,-6.343936e-01F,-5.956995e-01F,-5.555703e-01F,-5.141031e-01F,-4.713970e-01F,-4.275552e-01F,
-3.826839e-01F,-3.368902e-01F,-2.902848e-01F,-2.429807e-01F,-1.950907e-01F,-1.467307e-01F,-9.801725e-02F,-4.906812e-02F,
0.0F};



#undef NS
#define NS(x) 0
#undef NSP
#define NSP  ENGINE_PTR_COMMA NULL
#undef NT
#define NT(x)  EV(gtables)[x]
#undef NV
#define NV(x)  EV(global)[x].f
#undef NVI
#define NVI(x)  EV(global)[x].i
#undef NVUI
#define NVUI(x)  EV(global)[x].ui
#undef NVPS
#define NVPS(x)  EV(global)[x].ps
#undef NVU
#define NVU(x)  EV(global)[x]
#undef NP
#define NP(x)  EV(global)[x].f
#undef NPI
#define NPI(x)  EV(global)[x].i
#undef NPUI
#define NPUI(x)  EV(global)[x].ui
#undef NPPS
#define NPPS(x)  EV(global)[x].ps



ENGINE_PTR_TYPE system_init(int argc, char **argv, float sample_rate)
{
   int i;
   ENGINE_PTR_CREATE_SEMICOLON

   int j;

   ENGINE_PTR_NULLRETURN_SEMICOLON

   engine_init(ENGINE_PTR_COMMA sample_rate);

   srand(((unsigned int)time(0))|1);
   EV(asys_argc) = argc;
   EV(asys_argv) = argv;


   memset(&(NVU(GBL_STARTVAR)), 0, 
          (GBL_ENDVAR-GBL_STARTVAR)*sizeof(NVU(0)));
   memset(&(NT(0)), 0, GBL_ENDTBL*sizeof(NT(0)));
   NT(TBL_GBL_cyc).lenf = (float)(NT(TBL_GBL_cyc).len = 128);
   NT(TBL_GBL_cyc).tend = 127;
   NT(TBL_GBL_cyc).oconst = 128.0F*EV(ATIME);
   NT(TBL_GBL_cyc).t = (float *)(table_global_cyc);

#undef NS
#define NS(x) nstate->x
#undef NSP
#define NSP  ENGINE_PTR_COMMA nstate
#undef NT
#define NT(x)  nstate->t[x]
#undef NV
#define NV(x)  nstate->v[x].f
#undef NVI
#define NVI(x)  nstate->v[x].i
#undef NVUI
#define NVUI(x)  nstate->v[x].ui
#undef NVPS
#define NVPS(x)  nstate->v[x].ps
#undef NVU
#define NVU(x)  nstate->v[x]
#undef NP
#define NP(x)  nstate->v[x].f
#undef NPI
#define NPI(x)  nstate->v[x].i
#undef NPUI
#define NPUI(x)  nstate->v[x].ui
#undef NPPS
#define NPPS(x)  nstate->v[x].ps

      memset(&(TB(0)), 0, ENDBUS*sizeof(TB(0)));


  ENGINE_PTR_RETURN_SEMICOLON
}


#undef NS
#define NS(x) 0
#undef NSP
#define NSP  ENGINE_PTR_COMMA NULL
#undef NT
#define NT(x)  EV(gtables)[x]
#undef NV
#define NV(x)  EV(global)[x].f
#undef NVI
#define NVI(x)  EV(global)[x].i
#undef NVUI
#define NVUI(x)  EV(global)[x].ui
#undef NVPS
#define NVPS(x)  EV(global)[x].ps
#undef NVU
#define NVU(x)  EV(global)[x]
#undef NP
#define NP(x)  EV(global)[x].f
#undef NPI
#define NPI(x)  EV(global)[x].i
#undef NPUI
#define NPUI(x)  EV(global)[x].ui
#undef NPPS
#define NPPS(x)  EV(global)[x].ps



void effects_init(ENGINE_PTR_DECLARE)
{



}


#undef NS
#define NS(x) nstate->x
#undef NSP
#define NSP  ENGINE_PTR_COMMA nstate
#undef NT
#define NT(x)  nstate->t[x]
#undef NV
#define NV(x)  nstate->v[x].f
#undef NVI
#define NVI(x)  nstate->v[x].i
#undef NVUI
#define NVUI(x)  nstate->v[x].ui
#undef NVPS
#define NVPS(x)  nstate->v[x].ps
#undef NVU
#define NVU(x)  nstate->v[x]
#undef NP
#define NP(x)  nstate->v[x].f
#undef NPI
#define NPI(x)  nstate->v[x].i
#undef NPUI
#define NPUI(x)  nstate->v[x].ui
#undef NPPS
#define NPPS(x)  nstate->v[x].ps


#undef NS
#define NS(x) 0
#undef NSP
#define NSP  ENGINE_PTR_COMMA NULL
#undef NT
#define NT(x)  EV(gtables)[x]
#undef NV
#define NV(x)  EV(global)[x].f
#undef NVI
#define NVI(x)  EV(global)[x].i
#undef NVUI
#define NVUI(x)  EV(global)[x].ui
#undef NVPS
#define NVPS(x)  EV(global)[x].ps
#undef NVU
#define NVU(x)  EV(global)[x]
#undef NP
#define NP(x)  EV(global)[x].f
#undef NPI
#define NPI(x)  EV(global)[x].i
#undef NPUI
#define NPUI(x)  EV(global)[x].ui
#undef NPPS
#define NPPS(x)  EV(global)[x].ps



void shut_down(ENGINE_PTR_DECLARE)
   {

  if (EV(graceful_exit))
    {
      fprintf(stderr, "\nShutting down system ... please wait.\n");
      fprintf(stderr, "If no termination in 10 seconds, use Ctrl-C or Ctrl-\\ to force exit.\n");
      fflush(stderr);
    }
   asys_putbuf(&EV(asys_obuf), &EV(obusidx));
   asys_oshutdown();

   ENGINE_PTR_DESTROY_SEMICOLON
   }

void main_apass(ENGINE_PTR_DECLARE)

{
  int busidx;
  instr_line * sysidx;

  if (EV(s_flute)[0].noteon == PLAYING)
   flute_apass(ENGINE_PTR_COMMA EV(s_flute)[0].nstate);

}

int main_kpass(ENGINE_PTR_DECLARE)

{
  instr_line * sysidx;

  if (EV(s_flute)[0].noteon == PLAYING)
   flute_kpass(ENGINE_PTR_COMMA EV(s_flute)[0].nstate);

  return EV(graceful_exit);
}

void main_ipass(ENGINE_PTR_DECLARE)

{
  int i;
  instr_line * sysidx;

  sysidx = &EV(s_flute)[0];
  switch(sysidx->noteon) {
   case PLAYING:
   if (sysidx->released)
    {
     if (sysidx->turnoff)
      {
        sysidx->noteon = ALLDONE;
        for (i = 0; i < flute_ENDTBL; i++)
         if (sysidx->nstate->t[i].llmem)
           free(sysidx->nstate->t[i].t);
        sysidx->nstate->iline = NULL;
      }
     else
      {
        sysidx->abstime -= EV(KTIME);
        if (sysidx->abstime < 0.0F)
         {
           sysidx->noteon = ALLDONE;
           for (i = 0; i < flute_ENDTBL; i++)
            if (sysidx->nstate->t[i].llmem)
             free(sysidx->nstate->t[i].t);
           sysidx->nstate->iline = NULL;
         }
        else
         sysidx->turnoff = sysidx->released = 0;
      }
    }
   else
    {
     if (sysidx->turnoff)
      {
       sysidx->released = 1;
      }
     else
      {
        if (sysidx->endtime <= EV(scorebeats))
         {
           if (sysidx->abstime <= 0.0F)
             sysidx->turnoff = sysidx->released = 1;
           else
           {
             sysidx->abstime -= EV(KTIME);
             if (sysidx->abstime < 0.0F)
               sysidx->turnoff = sysidx->released = 1;
           }
         }
        else
          if ((sysidx->abstime < 0.0F) &&
           (1.666667e-2F*EV(tempo)*sysidx->abstime + 
               sysidx->endtime <= EV(scorebeats)))
            sysidx->turnoff = sysidx->released = 1;
      }
    }
   break;
   case TOBEPLAYED:
    if (sysidx->starttime <= EV(scorebeats))
     {
      sysidx->noteon = PLAYING;
      sysidx->notestate = EV(nextstate);
      sysidx->nstate = &(EV(ninstr)[EV(nextstate)]);
      if (sysidx->sdur >= 0.0F)
        sysidx->endtime = EV(scorebeats) + sysidx->sdur;
      sysidx->kbirth = EV(kcycleidx);
      EV(ninstr)[EV(nextstate)].iline = sysidx;
      sysidx->time = (EV(kcycleidx)-1)*EV(KTIME);
       EV(oldstate) = EV(nextstate);
       EV(nextstate) = (EV(nextstate)+1) % MAXSTATE;
       while ((EV(oldstate) != EV(nextstate)) && 
              (EV(ninstr)[EV(nextstate)].iline != NULL))
         EV(nextstate) = (EV(nextstate)+1) % MAXSTATE;
       if (EV(oldstate) == EV(nextstate))
       {
         EV(nextstate) = (EV(nextstate)+1) % MAXSTATE;
         while ((EV(oldstate) != EV(nextstate)) && 
             (EV(ninstr)[EV(nextstate)].iline->time == 0.0F) &&
             (EV(ninstr)[EV(nextstate)].iline->noteon == PLAYING))
           EV(nextstate) = (EV(nextstate)+1) % MAXSTATE;
         EV(ninstr)[EV(nextstate)].iline->noteon = ALLDONE;
         EV(ninstr)[EV(nextstate)].iline = NULL;
       }
      flute_ipass(ENGINE_PTR_COMMA sysidx->nstate);
    }
   break;
   }

}

void main_initpass(ENGINE_PTR_DECLARE)

{


   if (asys_osetup((int) EV(ARATE), ASYS_OCHAN, ASYS_OTYPENAME,  "output.wav",
ASYS_TIMEOPTION) != ASYS_DONE)
   epr(0,NULL,NULL,"audio output device unavailable");
   EV(endkcycle) = EV(kbase) + (int) 
      (EV(KRATE)*(EV(endtime) - EV(scorebase))*(60.0F/EV(tempo)));

   if (asys_preamble(&EV(asys_obuf), &EV(asys_osize)) != ASYS_DONE)
   epr(0,NULL,NULL,"audio output device unavailable");
   ksyncinit();


}

void main_control(ENGINE_PTR_DECLARE)

{

}

/*
#    Sfront, a SAOL to C translator    
#    This file: Main loop for runtime
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


int main(int argc, char *argv[])

{
  ENGINE_PTR_DECLARE_SEMICOLON
  int busidx;

  ENGINE_PTR_ASSIGNED_TO  system_init(argc, argv, SAOL_SRATE);
  if (ENGINE_PTR_IS_NULL) return -1;
  effects_init(ENGINE_PTR);
  main_initpass(ENGINE_PTR);
  for (EV(kcycleidx)=EV(kbase); EV(kcycleidx)<=EV(endkcycle); EV(kcycleidx)++)
    {
      EV(pass) = IPASS;
      EV(scorebeats) = EV(scoremult)*(EV(kcycleidx) - EV(kbase)) + EV(scorebase);
      EV(absolutetime) = (EV(kcycleidx) - 1)*EV(KTIME);
      main_ipass(ENGINE_PTR);
      EV(pass) = KPASS;
      main_control(ENGINE_PTR);
      if (main_kpass(ENGINE_PTR))
	break;
      EV(pass) = APASS;
      for (EV(acycleidx)=0; EV(acycleidx)<EV(ACYCLE); EV(acycleidx)++)
	{
	  memset(&(TB(0)), 0, sizeof(TB(0))*ENDBUS);
	  main_apass(ENGINE_PTR);
	  for (busidx=BUS_output_bus; busidx<ENDBUS_output_bus;busidx++)
	    {
	      TB(busidx) = (TB(busidx) >  1.0F) ?  1.0F : TB(busidx);
	      EV(asys_obuf)[EV(obusidx)++] = (TB(busidx) < -1.0F) ? -1.0F:TB(busidx);
	    }
	  if (EV(obusidx) >= EV(asys_osize))
	    {
	      EV(obusidx) = 0;
	      if (asys_putbuf(&EV(asys_obuf), &EV(asys_osize)))
		{
		  fprintf(stderr,"  Sfront: Audio output device problem\n\n");
		  EV(kcycleidx) = EV(endkcycle);
		  break;
		}
	    }
	}
      EV(acycleidx) = 0; 
      EV(cpuload) = ksync();
    }
  shut_down(ENGINE_PTR);
  return 0;
}





