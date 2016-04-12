
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
"piano.saol",
"-sco",
"piano.sasl"};



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

#define SAOL_SRATE 22050.0F
#define SAOL_KRATE 100.0F

#define ARATE 22050.0F
#define ATIME 4.535147e-05F
#define ACYCLE 210

#define KRATE 105.0F
#define KTIME 9.523810e-03F
#define KMTIME 9.523809e+00F
#define KUTIME 9524
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

#define piano_fr 0
#define piano_cyc 1
#define piano_env01 2
#define piano_env02 3
#define piano_env03 4
#define piano_env04 5
#define piano_env05 6
#define piano_env06 7
#define piano_env07 8
#define piano_env08 9
#define piano_env09 10
#define piano_env10 11
#define piano_env11 12
#define piano_env12 13
#define piano_env13 14
#define piano_env14 15
#define piano_env15 16
#define piano_env16 17
#define piano_env17 18
#define piano_env18 19
#define piano_env19 20
#define piano_env20 21
#define piano_env21 22
#define piano_env22 23
#define piano_env23 24
#define piano_env24 25
#define piano_env25 26
#define piano_env26 27
#define piano_env27 28
#define piano_env28 29
#define piano_env29 30
#define piano_env30 31
#define piano_y01 32
#define piano_y02 33
#define piano_y03 34
#define piano_y04 35
#define piano_y05 36
#define piano_y06 37
#define piano_y07 38
#define piano_y08 39
#define piano_y09 40
#define piano_y10 41
#define piano_y11 42
#define piano_y12 43
#define piano_y13 44
#define piano_y14 45
#define piano_y15 46
#define piano_y16 47
#define piano_y17 48
#define piano_y18 49
#define piano_y19 50
#define piano_y20 51
#define piano_y21 52
#define piano_y22 53
#define piano_y23 54
#define piano_y24 55
#define piano_y25 56
#define piano_y26 57
#define piano_y27 58
#define piano_y28 59
#define piano_y29 60
#define piano_y30 61
#define piano__tvr29 62
#define piano__tvr28 63
#define piano__tvr27 64
#define piano__tvr26 65
#define piano__tvr25 66
#define piano__tvr24 67
#define piano__tvr23 68
#define piano__tvr22 69
#define piano__tvr21 70
#define piano__tvr20 71
#define piano__tvr19 72
#define piano__tvr18 73
#define piano__tvr17 74
#define piano__tvr16 75
#define piano__tvr15 76
#define piano__tvr14 77
#define piano__tvr13 78
#define piano__tvr12 79
#define piano__tvr11 80
#define piano__tvr10 81
#define piano__tvr9 82
#define piano__tvr8 83
#define piano__tvr7 84
#define piano__tvr6 85
#define piano__tvr5 86
#define piano__tvr4 87
#define piano__tvr3 88
#define piano__tvr2 89
#define piano__tvr1 90
#define piano__tvr0 91
#define piano_kline1_first 92
#define piano_kline1_addK 93
#define piano_kline1_outT 94
#define piano_kline1_mult 95
#define piano_kline1_cdur 96
#define piano_kline1_crp 97
#define piano_kline1_clp 98
#define piano_kline1_t 99
#define piano_kline1_x2 100
#define piano_kline1_dur1 101
#define piano_kline1_x1 102
#define piano_kline1_return 103
#define piano_kline2_first 104
#define piano_kline2_addK 105
#define piano_kline2_outT 106
#define piano_kline2_mult 107
#define piano_kline2_cdur 108
#define piano_kline2_crp 109
#define piano_kline2_clp 110
#define piano_kline2_t 111
#define piano_kline2_x2 112
#define piano_kline2_dur1 113
#define piano_kline2_x1 114
#define piano_kline2_return 115
#define piano_kline3_first 116
#define piano_kline3_addK 117
#define piano_kline3_outT 118
#define piano_kline3_mult 119
#define piano_kline3_cdur 120
#define piano_kline3_crp 121
#define piano_kline3_clp 122
#define piano_kline3_t 123
#define piano_kline3_x2 124
#define piano_kline3_dur1 125
#define piano_kline3_x1 126
#define piano_kline3_return 127
#define piano_kline4_first 128
#define piano_kline4_addK 129
#define piano_kline4_outT 130
#define piano_kline4_mult 131
#define piano_kline4_cdur 132
#define piano_kline4_crp 133
#define piano_kline4_clp 134
#define piano_kline4_t 135
#define piano_kline4_x2 136
#define piano_kline4_dur1 137
#define piano_kline4_x1 138
#define piano_kline4_return 139
#define piano_kline5_first 140
#define piano_kline5_addK 141
#define piano_kline5_outT 142
#define piano_kline5_mult 143
#define piano_kline5_cdur 144
#define piano_kline5_crp 145
#define piano_kline5_clp 146
#define piano_kline5_t 147
#define piano_kline5_x2 148
#define piano_kline5_dur1 149
#define piano_kline5_x1 150
#define piano_kline5_return 151
#define piano_kline6_first 152
#define piano_kline6_addK 153
#define piano_kline6_outT 154
#define piano_kline6_mult 155
#define piano_kline6_cdur 156
#define piano_kline6_crp 157
#define piano_kline6_clp 158
#define piano_kline6_t 159
#define piano_kline6_x2 160
#define piano_kline6_dur1 161
#define piano_kline6_x1 162
#define piano_kline6_return 163
#define piano_kline7_first 164
#define piano_kline7_addK 165
#define piano_kline7_outT 166
#define piano_kline7_mult 167
#define piano_kline7_cdur 168
#define piano_kline7_crp 169
#define piano_kline7_clp 170
#define piano_kline7_t 171
#define piano_kline7_x2 172
#define piano_kline7_dur1 173
#define piano_kline7_x1 174
#define piano_kline7_return 175
#define piano_kline8_first 176
#define piano_kline8_addK 177
#define piano_kline8_outT 178
#define piano_kline8_mult 179
#define piano_kline8_cdur 180
#define piano_kline8_crp 181
#define piano_kline8_clp 182
#define piano_kline8_t 183
#define piano_kline8_x2 184
#define piano_kline8_dur1 185
#define piano_kline8_x1 186
#define piano_kline8_return 187
#define piano_kline9_first 188
#define piano_kline9_addK 189
#define piano_kline9_outT 190
#define piano_kline9_mult 191
#define piano_kline9_cdur 192
#define piano_kline9_crp 193
#define piano_kline9_clp 194
#define piano_kline9_t 195
#define piano_kline9_x2 196
#define piano_kline9_dur1 197
#define piano_kline9_x1 198
#define piano_kline9_return 199
#define piano_kline10_first 200
#define piano_kline10_addK 201
#define piano_kline10_outT 202
#define piano_kline10_mult 203
#define piano_kline10_cdur 204
#define piano_kline10_crp 205
#define piano_kline10_clp 206
#define piano_kline10_t 207
#define piano_kline10_x2 208
#define piano_kline10_dur1 209
#define piano_kline10_x1 210
#define piano_kline10_return 211
#define piano_kline11_first 212
#define piano_kline11_addK 213
#define piano_kline11_outT 214
#define piano_kline11_mult 215
#define piano_kline11_cdur 216
#define piano_kline11_crp 217
#define piano_kline11_clp 218
#define piano_kline11_t 219
#define piano_kline11_x2 220
#define piano_kline11_dur1 221
#define piano_kline11_x1 222
#define piano_kline11_return 223
#define piano_kline12_first 224
#define piano_kline12_addK 225
#define piano_kline12_outT 226
#define piano_kline12_mult 227
#define piano_kline12_cdur 228
#define piano_kline12_crp 229
#define piano_kline12_clp 230
#define piano_kline12_t 231
#define piano_kline12_x2 232
#define piano_kline12_dur1 233
#define piano_kline12_x1 234
#define piano_kline12_return 235
#define piano_kline13_first 236
#define piano_kline13_addK 237
#define piano_kline13_outT 238
#define piano_kline13_mult 239
#define piano_kline13_cdur 240
#define piano_kline13_crp 241
#define piano_kline13_clp 242
#define piano_kline13_t 243
#define piano_kline13_x2 244
#define piano_kline13_dur1 245
#define piano_kline13_x1 246
#define piano_kline13_return 247
#define piano_kline14_first 248
#define piano_kline14_addK 249
#define piano_kline14_outT 250
#define piano_kline14_mult 251
#define piano_kline14_cdur 252
#define piano_kline14_crp 253
#define piano_kline14_clp 254
#define piano_kline14_t 255
#define piano_kline14_x2 256
#define piano_kline14_dur1 257
#define piano_kline14_x1 258
#define piano_kline14_return 259
#define piano_kline15_first 260
#define piano_kline15_addK 261
#define piano_kline15_outT 262
#define piano_kline15_mult 263
#define piano_kline15_cdur 264
#define piano_kline15_crp 265
#define piano_kline15_clp 266
#define piano_kline15_t 267
#define piano_kline15_x2 268
#define piano_kline15_dur1 269
#define piano_kline15_x1 270
#define piano_kline15_return 271
#define piano_kline16_first 272
#define piano_kline16_addK 273
#define piano_kline16_outT 274
#define piano_kline16_mult 275
#define piano_kline16_cdur 276
#define piano_kline16_crp 277
#define piano_kline16_clp 278
#define piano_kline16_t 279
#define piano_kline16_x2 280
#define piano_kline16_dur1 281
#define piano_kline16_x1 282
#define piano_kline16_return 283
#define piano_kline17_first 284
#define piano_kline17_addK 285
#define piano_kline17_outT 286
#define piano_kline17_mult 287
#define piano_kline17_cdur 288
#define piano_kline17_crp 289
#define piano_kline17_clp 290
#define piano_kline17_t 291
#define piano_kline17_x2 292
#define piano_kline17_dur1 293
#define piano_kline17_x1 294
#define piano_kline17_return 295
#define piano_kline18_first 296
#define piano_kline18_addK 297
#define piano_kline18_outT 298
#define piano_kline18_mult 299
#define piano_kline18_cdur 300
#define piano_kline18_crp 301
#define piano_kline18_clp 302
#define piano_kline18_t 303
#define piano_kline18_x2 304
#define piano_kline18_dur1 305
#define piano_kline18_x1 306
#define piano_kline18_return 307
#define piano_kline19_first 308
#define piano_kline19_addK 309
#define piano_kline19_outT 310
#define piano_kline19_mult 311
#define piano_kline19_cdur 312
#define piano_kline19_crp 313
#define piano_kline19_clp 314
#define piano_kline19_t 315
#define piano_kline19_x2 316
#define piano_kline19_dur1 317
#define piano_kline19_x1 318
#define piano_kline19_return 319
#define piano_kline20_first 320
#define piano_kline20_addK 321
#define piano_kline20_outT 322
#define piano_kline20_mult 323
#define piano_kline20_cdur 324
#define piano_kline20_crp 325
#define piano_kline20_clp 326
#define piano_kline20_t 327
#define piano_kline20_x2 328
#define piano_kline20_dur1 329
#define piano_kline20_x1 330
#define piano_kline20_return 331
#define piano_kline21_first 332
#define piano_kline21_addK 333
#define piano_kline21_outT 334
#define piano_kline21_mult 335
#define piano_kline21_cdur 336
#define piano_kline21_crp 337
#define piano_kline21_clp 338
#define piano_kline21_t 339
#define piano_kline21_x2 340
#define piano_kline21_dur1 341
#define piano_kline21_x1 342
#define piano_kline21_return 343
#define piano_kline22_first 344
#define piano_kline22_addK 345
#define piano_kline22_outT 346
#define piano_kline22_mult 347
#define piano_kline22_cdur 348
#define piano_kline22_crp 349
#define piano_kline22_clp 350
#define piano_kline22_t 351
#define piano_kline22_x2 352
#define piano_kline22_dur1 353
#define piano_kline22_x1 354
#define piano_kline22_return 355
#define piano_kline23_first 356
#define piano_kline23_addK 357
#define piano_kline23_outT 358
#define piano_kline23_mult 359
#define piano_kline23_cdur 360
#define piano_kline23_crp 361
#define piano_kline23_clp 362
#define piano_kline23_t 363
#define piano_kline23_x2 364
#define piano_kline23_dur1 365
#define piano_kline23_x1 366
#define piano_kline23_return 367
#define piano_kline24_first 368
#define piano_kline24_addK 369
#define piano_kline24_outT 370
#define piano_kline24_mult 371
#define piano_kline24_cdur 372
#define piano_kline24_crp 373
#define piano_kline24_clp 374
#define piano_kline24_t 375
#define piano_kline24_x2 376
#define piano_kline24_dur1 377
#define piano_kline24_x1 378
#define piano_kline24_return 379
#define piano_kline25_first 380
#define piano_kline25_addK 381
#define piano_kline25_outT 382
#define piano_kline25_mult 383
#define piano_kline25_cdur 384
#define piano_kline25_crp 385
#define piano_kline25_clp 386
#define piano_kline25_t 387
#define piano_kline25_x2 388
#define piano_kline25_dur1 389
#define piano_kline25_x1 390
#define piano_kline25_return 391
#define piano_kline26_first 392
#define piano_kline26_addK 393
#define piano_kline26_outT 394
#define piano_kline26_mult 395
#define piano_kline26_cdur 396
#define piano_kline26_crp 397
#define piano_kline26_clp 398
#define piano_kline26_t 399
#define piano_kline26_x2 400
#define piano_kline26_dur1 401
#define piano_kline26_x1 402
#define piano_kline26_return 403
#define piano_kline27_first 404
#define piano_kline27_addK 405
#define piano_kline27_outT 406
#define piano_kline27_mult 407
#define piano_kline27_cdur 408
#define piano_kline27_crp 409
#define piano_kline27_clp 410
#define piano_kline27_t 411
#define piano_kline27_x2 412
#define piano_kline27_dur1 413
#define piano_kline27_x1 414
#define piano_kline27_return 415
#define piano_kline28_first 416
#define piano_kline28_addK 417
#define piano_kline28_outT 418
#define piano_kline28_mult 419
#define piano_kline28_cdur 420
#define piano_kline28_crp 421
#define piano_kline28_clp 422
#define piano_kline28_t 423
#define piano_kline28_x2 424
#define piano_kline28_dur1 425
#define piano_kline28_x1 426
#define piano_kline28_return 427
#define piano_kline29_first 428
#define piano_kline29_addK 429
#define piano_kline29_outT 430
#define piano_kline29_mult 431
#define piano_kline29_cdur 432
#define piano_kline29_crp 433
#define piano_kline29_clp 434
#define piano_kline29_t 435
#define piano_kline29_x2 436
#define piano_kline29_dur1 437
#define piano_kline29_x1 438
#define piano_kline29_return 439
#define piano_kline30_first 440
#define piano_kline30_addK 441
#define piano_kline30_outT 442
#define piano_kline30_mult 443
#define piano_kline30_cdur 444
#define piano_kline30_crp 445
#define piano_kline30_clp 446
#define piano_kline30_t 447
#define piano_kline30_x2 448
#define piano_kline30_dur1 449
#define piano_kline30_x1 450
#define piano_kline30_return 451
#define piano_oscil31_fsign 452
#define piano_oscil31_kfrac 453
#define piano_oscil31_kint 454
#define piano_oscil31_pfrac 455
#define piano_oscil31_pint 456
#define piano_oscil31_kcyc 457
#define piano_oscil31_iloops 458
#define piano_oscil31_freq 459
#define piano_oscil31_t 460
#define piano_oscil31_return 461
#define piano_oscil32_fsign 462
#define piano_oscil32_kfrac 463
#define piano_oscil32_kint 464
#define piano_oscil32_pfrac 465
#define piano_oscil32_pint 466
#define piano_oscil32_kcyc 467
#define piano_oscil32_iloops 468
#define piano_oscil32_freq 469
#define piano_oscil32_t 470
#define piano_oscil32_return 471
#define piano_oscil33_fsign 472
#define piano_oscil33_kfrac 473
#define piano_oscil33_kint 474
#define piano_oscil33_pfrac 475
#define piano_oscil33_pint 476
#define piano_oscil33_kcyc 477
#define piano_oscil33_iloops 478
#define piano_oscil33_freq 479
#define piano_oscil33_t 480
#define piano_oscil33_return 481
#define piano_oscil34_fsign 482
#define piano_oscil34_kfrac 483
#define piano_oscil34_kint 484
#define piano_oscil34_pfrac 485
#define piano_oscil34_pint 486
#define piano_oscil34_kcyc 487
#define piano_oscil34_iloops 488
#define piano_oscil34_freq 489
#define piano_oscil34_t 490
#define piano_oscil34_return 491
#define piano_oscil35_fsign 492
#define piano_oscil35_kfrac 493
#define piano_oscil35_kint 494
#define piano_oscil35_pfrac 495
#define piano_oscil35_pint 496
#define piano_oscil35_kcyc 497
#define piano_oscil35_iloops 498
#define piano_oscil35_freq 499
#define piano_oscil35_t 500
#define piano_oscil35_return 501
#define piano_oscil36_fsign 502
#define piano_oscil36_kfrac 503
#define piano_oscil36_kint 504
#define piano_oscil36_pfrac 505
#define piano_oscil36_pint 506
#define piano_oscil36_kcyc 507
#define piano_oscil36_iloops 508
#define piano_oscil36_freq 509
#define piano_oscil36_t 510
#define piano_oscil36_return 511
#define piano_oscil37_fsign 512
#define piano_oscil37_kfrac 513
#define piano_oscil37_kint 514
#define piano_oscil37_pfrac 515
#define piano_oscil37_pint 516
#define piano_oscil37_kcyc 517
#define piano_oscil37_iloops 518
#define piano_oscil37_freq 519
#define piano_oscil37_t 520
#define piano_oscil37_return 521
#define piano_oscil38_fsign 522
#define piano_oscil38_kfrac 523
#define piano_oscil38_kint 524
#define piano_oscil38_pfrac 525
#define piano_oscil38_pint 526
#define piano_oscil38_kcyc 527
#define piano_oscil38_iloops 528
#define piano_oscil38_freq 529
#define piano_oscil38_t 530
#define piano_oscil38_return 531
#define piano_oscil39_fsign 532
#define piano_oscil39_kfrac 533
#define piano_oscil39_kint 534
#define piano_oscil39_pfrac 535
#define piano_oscil39_pint 536
#define piano_oscil39_kcyc 537
#define piano_oscil39_iloops 538
#define piano_oscil39_freq 539
#define piano_oscil39_t 540
#define piano_oscil39_return 541
#define piano_oscil40_fsign 542
#define piano_oscil40_kfrac 543
#define piano_oscil40_kint 544
#define piano_oscil40_pfrac 545
#define piano_oscil40_pint 546
#define piano_oscil40_kcyc 547
#define piano_oscil40_iloops 548
#define piano_oscil40_freq 549
#define piano_oscil40_t 550
#define piano_oscil40_return 551
#define piano_oscil41_fsign 552
#define piano_oscil41_kfrac 553
#define piano_oscil41_kint 554
#define piano_oscil41_pfrac 555
#define piano_oscil41_pint 556
#define piano_oscil41_kcyc 557
#define piano_oscil41_iloops 558
#define piano_oscil41_freq 559
#define piano_oscil41_t 560
#define piano_oscil41_return 561
#define piano_oscil42_fsign 562
#define piano_oscil42_kfrac 563
#define piano_oscil42_kint 564
#define piano_oscil42_pfrac 565
#define piano_oscil42_pint 566
#define piano_oscil42_kcyc 567
#define piano_oscil42_iloops 568
#define piano_oscil42_freq 569
#define piano_oscil42_t 570
#define piano_oscil42_return 571
#define piano_oscil43_fsign 572
#define piano_oscil43_kfrac 573
#define piano_oscil43_kint 574
#define piano_oscil43_pfrac 575
#define piano_oscil43_pint 576
#define piano_oscil43_kcyc 577
#define piano_oscil43_iloops 578
#define piano_oscil43_freq 579
#define piano_oscil43_t 580
#define piano_oscil43_return 581
#define piano_oscil44_fsign 582
#define piano_oscil44_kfrac 583
#define piano_oscil44_kint 584
#define piano_oscil44_pfrac 585
#define piano_oscil44_pint 586
#define piano_oscil44_kcyc 587
#define piano_oscil44_iloops 588
#define piano_oscil44_freq 589
#define piano_oscil44_t 590
#define piano_oscil44_return 591
#define piano_oscil45_fsign 592
#define piano_oscil45_kfrac 593
#define piano_oscil45_kint 594
#define piano_oscil45_pfrac 595
#define piano_oscil45_pint 596
#define piano_oscil45_kcyc 597
#define piano_oscil45_iloops 598
#define piano_oscil45_freq 599
#define piano_oscil45_t 600
#define piano_oscil45_return 601
#define piano_oscil46_fsign 602
#define piano_oscil46_kfrac 603
#define piano_oscil46_kint 604
#define piano_oscil46_pfrac 605
#define piano_oscil46_pint 606
#define piano_oscil46_kcyc 607
#define piano_oscil46_iloops 608
#define piano_oscil46_freq 609
#define piano_oscil46_t 610
#define piano_oscil46_return 611
#define piano_oscil47_fsign 612
#define piano_oscil47_kfrac 613
#define piano_oscil47_kint 614
#define piano_oscil47_pfrac 615
#define piano_oscil47_pint 616
#define piano_oscil47_kcyc 617
#define piano_oscil47_iloops 618
#define piano_oscil47_freq 619
#define piano_oscil47_t 620
#define piano_oscil47_return 621
#define piano_oscil48_fsign 622
#define piano_oscil48_kfrac 623
#define piano_oscil48_kint 624
#define piano_oscil48_pfrac 625
#define piano_oscil48_pint 626
#define piano_oscil48_kcyc 627
#define piano_oscil48_iloops 628
#define piano_oscil48_freq 629
#define piano_oscil48_t 630
#define piano_oscil48_return 631
#define piano_oscil49_fsign 632
#define piano_oscil49_kfrac 633
#define piano_oscil49_kint 634
#define piano_oscil49_pfrac 635
#define piano_oscil49_pint 636
#define piano_oscil49_kcyc 637
#define piano_oscil49_iloops 638
#define piano_oscil49_freq 639
#define piano_oscil49_t 640
#define piano_oscil49_return 641
#define piano_oscil50_fsign 642
#define piano_oscil50_kfrac 643
#define piano_oscil50_kint 644
#define piano_oscil50_pfrac 645
#define piano_oscil50_pint 646
#define piano_oscil50_kcyc 647
#define piano_oscil50_iloops 648
#define piano_oscil50_freq 649
#define piano_oscil50_t 650
#define piano_oscil50_return 651
#define piano_oscil51_fsign 652
#define piano_oscil51_kfrac 653
#define piano_oscil51_kint 654
#define piano_oscil51_pfrac 655
#define piano_oscil51_pint 656
#define piano_oscil51_kcyc 657
#define piano_oscil51_iloops 658
#define piano_oscil51_freq 659
#define piano_oscil51_t 660
#define piano_oscil51_return 661
#define piano_oscil52_fsign 662
#define piano_oscil52_kfrac 663
#define piano_oscil52_kint 664
#define piano_oscil52_pfrac 665
#define piano_oscil52_pint 666
#define piano_oscil52_kcyc 667
#define piano_oscil52_iloops 668
#define piano_oscil52_freq 669
#define piano_oscil52_t 670
#define piano_oscil52_return 671
#define piano_oscil53_fsign 672
#define piano_oscil53_kfrac 673
#define piano_oscil53_kint 674
#define piano_oscil53_pfrac 675
#define piano_oscil53_pint 676
#define piano_oscil53_kcyc 677
#define piano_oscil53_iloops 678
#define piano_oscil53_freq 679
#define piano_oscil53_t 680
#define piano_oscil53_return 681
#define piano_oscil54_fsign 682
#define piano_oscil54_kfrac 683
#define piano_oscil54_kint 684
#define piano_oscil54_pfrac 685
#define piano_oscil54_pint 686
#define piano_oscil54_kcyc 687
#define piano_oscil54_iloops 688
#define piano_oscil54_freq 689
#define piano_oscil54_t 690
#define piano_oscil54_return 691
#define piano_oscil55_fsign 692
#define piano_oscil55_kfrac 693
#define piano_oscil55_kint 694
#define piano_oscil55_pfrac 695
#define piano_oscil55_pint 696
#define piano_oscil55_kcyc 697
#define piano_oscil55_iloops 698
#define piano_oscil55_freq 699
#define piano_oscil55_t 700
#define piano_oscil55_return 701
#define piano_oscil56_fsign 702
#define piano_oscil56_kfrac 703
#define piano_oscil56_kint 704
#define piano_oscil56_pfrac 705
#define piano_oscil56_pint 706
#define piano_oscil56_kcyc 707
#define piano_oscil56_iloops 708
#define piano_oscil56_freq 709
#define piano_oscil56_t 710
#define piano_oscil56_return 711
#define piano_oscil57_fsign 712
#define piano_oscil57_kfrac 713
#define piano_oscil57_kint 714
#define piano_oscil57_pfrac 715
#define piano_oscil57_pint 716
#define piano_oscil57_kcyc 717
#define piano_oscil57_iloops 718
#define piano_oscil57_freq 719
#define piano_oscil57_t 720
#define piano_oscil57_return 721
#define piano_oscil58_fsign 722
#define piano_oscil58_kfrac 723
#define piano_oscil58_kint 724
#define piano_oscil58_pfrac 725
#define piano_oscil58_pint 726
#define piano_oscil58_kcyc 727
#define piano_oscil58_iloops 728
#define piano_oscil58_freq 729
#define piano_oscil58_t 730
#define piano_oscil58_return 731
#define piano_oscil59_fsign 732
#define piano_oscil59_kfrac 733
#define piano_oscil59_kint 734
#define piano_oscil59_pfrac 735
#define piano_oscil59_pint 736
#define piano_oscil59_kcyc 737
#define piano_oscil59_iloops 738
#define piano_oscil59_freq 739
#define piano_oscil59_t 740
#define piano_oscil59_return 741
#define piano_oscil60_fsign 742
#define piano_oscil60_kfrac 743
#define piano_oscil60_kint 744
#define piano_oscil60_pfrac 745
#define piano_oscil60_pint 746
#define piano_oscil60_kcyc 747
#define piano_oscil60_iloops 748
#define piano_oscil60_freq 749
#define piano_oscil60_t 750
#define piano_oscil60_return 751
#define piano_ENDVAR 752

#define TBL_piano_cyc 0
#define piano_ENDTBL 1

#define MAXENDTIME 1E+37

float endtime;
instr_line s_piano[1];
#define MAXSTATE 2


#define MAXVARSTATE 752
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

float piano__sym_kline1(ENGINE_PTR_TYPE_COMMA struct ninstr_types *);
float piano__sym_kline2(ENGINE_PTR_TYPE_COMMA struct ninstr_types *);
float piano__sym_kline3(ENGINE_PTR_TYPE_COMMA struct ninstr_types *);
float piano__sym_kline4(ENGINE_PTR_TYPE_COMMA struct ninstr_types *);
float piano__sym_kline5(ENGINE_PTR_TYPE_COMMA struct ninstr_types *);
float piano__sym_kline6(ENGINE_PTR_TYPE_COMMA struct ninstr_types *);
float piano__sym_kline7(ENGINE_PTR_TYPE_COMMA struct ninstr_types *);
float piano__sym_kline8(ENGINE_PTR_TYPE_COMMA struct ninstr_types *);
float piano__sym_kline9(ENGINE_PTR_TYPE_COMMA struct ninstr_types *);
float piano__sym_kline10(ENGINE_PTR_TYPE_COMMA struct ninstr_types *);
float piano__sym_kline11(ENGINE_PTR_TYPE_COMMA struct ninstr_types *);
float piano__sym_kline12(ENGINE_PTR_TYPE_COMMA struct ninstr_types *);
float piano__sym_kline13(ENGINE_PTR_TYPE_COMMA struct ninstr_types *);
float piano__sym_kline14(ENGINE_PTR_TYPE_COMMA struct ninstr_types *);
float piano__sym_kline15(ENGINE_PTR_TYPE_COMMA struct ninstr_types *);
float piano__sym_kline16(ENGINE_PTR_TYPE_COMMA struct ninstr_types *);
float piano__sym_kline17(ENGINE_PTR_TYPE_COMMA struct ninstr_types *);
float piano__sym_kline18(ENGINE_PTR_TYPE_COMMA struct ninstr_types *);
float piano__sym_kline19(ENGINE_PTR_TYPE_COMMA struct ninstr_types *);
float piano__sym_kline20(ENGINE_PTR_TYPE_COMMA struct ninstr_types *);
float piano__sym_kline21(ENGINE_PTR_TYPE_COMMA struct ninstr_types *);
float piano__sym_kline22(ENGINE_PTR_TYPE_COMMA struct ninstr_types *);
float piano__sym_kline23(ENGINE_PTR_TYPE_COMMA struct ninstr_types *);
float piano__sym_kline24(ENGINE_PTR_TYPE_COMMA struct ninstr_types *);
float piano__sym_kline25(ENGINE_PTR_TYPE_COMMA struct ninstr_types *);
float piano__sym_kline26(ENGINE_PTR_TYPE_COMMA struct ninstr_types *);
float piano__sym_kline27(ENGINE_PTR_TYPE_COMMA struct ninstr_types *);
float piano__sym_kline28(ENGINE_PTR_TYPE_COMMA struct ninstr_types *);
float piano__sym_kline29(ENGINE_PTR_TYPE_COMMA struct ninstr_types *);
float piano__sym_kline30(ENGINE_PTR_TYPE_COMMA struct ninstr_types *);
float piano__sym_oscil31(ENGINE_PTR_TYPE_COMMA struct ninstr_types *);
float piano__sym_oscil32(ENGINE_PTR_TYPE_COMMA struct ninstr_types *);
float piano__sym_oscil33(ENGINE_PTR_TYPE_COMMA struct ninstr_types *);
float piano__sym_oscil34(ENGINE_PTR_TYPE_COMMA struct ninstr_types *);
float piano__sym_oscil35(ENGINE_PTR_TYPE_COMMA struct ninstr_types *);
float piano__sym_oscil36(ENGINE_PTR_TYPE_COMMA struct ninstr_types *);
float piano__sym_oscil37(ENGINE_PTR_TYPE_COMMA struct ninstr_types *);
float piano__sym_oscil38(ENGINE_PTR_TYPE_COMMA struct ninstr_types *);
float piano__sym_oscil39(ENGINE_PTR_TYPE_COMMA struct ninstr_types *);
float piano__sym_oscil40(ENGINE_PTR_TYPE_COMMA struct ninstr_types *);
float piano__sym_oscil41(ENGINE_PTR_TYPE_COMMA struct ninstr_types *);
float piano__sym_oscil42(ENGINE_PTR_TYPE_COMMA struct ninstr_types *);
float piano__sym_oscil43(ENGINE_PTR_TYPE_COMMA struct ninstr_types *);
float piano__sym_oscil44(ENGINE_PTR_TYPE_COMMA struct ninstr_types *);
float piano__sym_oscil45(ENGINE_PTR_TYPE_COMMA struct ninstr_types *);
float piano__sym_oscil46(ENGINE_PTR_TYPE_COMMA struct ninstr_types *);
float piano__sym_oscil47(ENGINE_PTR_TYPE_COMMA struct ninstr_types *);
float piano__sym_oscil48(ENGINE_PTR_TYPE_COMMA struct ninstr_types *);
float piano__sym_oscil49(ENGINE_PTR_TYPE_COMMA struct ninstr_types *);
float piano__sym_oscil50(ENGINE_PTR_TYPE_COMMA struct ninstr_types *);
float piano__sym_oscil51(ENGINE_PTR_TYPE_COMMA struct ninstr_types *);
float piano__sym_oscil52(ENGINE_PTR_TYPE_COMMA struct ninstr_types *);
float piano__sym_oscil53(ENGINE_PTR_TYPE_COMMA struct ninstr_types *);
float piano__sym_oscil54(ENGINE_PTR_TYPE_COMMA struct ninstr_types *);
float piano__sym_oscil55(ENGINE_PTR_TYPE_COMMA struct ninstr_types *);
float piano__sym_oscil56(ENGINE_PTR_TYPE_COMMA struct ninstr_types *);
float piano__sym_oscil57(ENGINE_PTR_TYPE_COMMA struct ninstr_types *);
float piano__sym_oscil58(ENGINE_PTR_TYPE_COMMA struct ninstr_types *);
float piano__sym_oscil59(ENGINE_PTR_TYPE_COMMA struct ninstr_types *);
float piano__sym_oscil60(ENGINE_PTR_TYPE_COMMA struct ninstr_types *);
void piano_ipass(ENGINE_PTR_TYPE_COMMA struct ninstr_types *);
void piano_kpass(ENGINE_PTR_TYPE_COMMA struct ninstr_types *);
void piano_apass(ENGINE_PTR_TYPE_COMMA struct ninstr_types *);



extern float table_global_cyc[];

csys_table_ptr csys_table_catalog[] = {
	129, table_global_cyc };

#define CSYS_TABLE_CATALOG_SIZE 1

instr_line s_piano_init[1] = {
{ 0.0F, MAXENDTIME, MAXENDTIME, MAXENDTIME,  0.0F, 0.0F, 0.0F,  2.49F, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, { 311.0F }, NULL }};


void engine_init(ENGINE_PTR_DECLARE_COMMA float sample_rate)
{
  EV(globaltune) = 440.0F;
  EV(invglobaltune) = 2.272727e-03F;
  EV(kbase) = 1;
  EV(kcycleidx) = 1;
  EV(pass) = IPASS;
  EV(tempo) = 60.0F;

  EV(scoremult) = EV(KTIME);
  EV(endtime) = 2.49F;

  memcpy(EV(s_piano), s_piano_init, sizeof EV(s_piano));
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



float piano__sym_kline1(ENGINE_PTR_DECLARE_COMMA struct ninstr_types * nstate)
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
   float va_dur20;
   float va_x21;
   float x2;
   float dur1;
   float x1;
   float ret;

   x1 =  0.0F ;
   dur1 =  0.001608F ;
   x2 =  0.336494F ;
   va_dur2 =  0.004823F  ;
   va_x3 =  0.258039F  ;
   va_dur3 =  0.001608F  ;
   va_x4 =  0.260641F  ;
   va_dur4 =  0.003215F  ;
   va_x5 =  0.316632F  ;
   va_dur5 =  0.004823F  ;
   va_x6 =  0.306330F  ;
   va_dur6 =  0.003215F  ;
   va_x7 =  0.272150F  ;
   va_dur7 =  0.004823F  ;
   va_x8 =  0.301878F  ;
   va_dur8 =  0.004823F  ;
   va_x9 =  0.286735F  ;
   va_dur9 =  0.003215F  ;
   va_x10 =  0.252793F  ;
   va_dur10 =  0.003215F  ;
   va_x11 =  0.263369F  ;
   va_dur11 =  0.008039F  ;
   va_x12 =  0.227911F  ;
   va_dur12 =  0.004823F  ;
   va_x13 =  0.257629F  ;
   va_dur13 =  0.024116F  ;
   va_x14 =  0.273146F  ;
   va_dur14 =  0.014469F  ;
   va_x15 =  0.246980F  ;
   va_dur15 =  0.062701F  ;
   va_x16 =  0.243235F  ;
   va_dur16 =  0.077170F  ;
   va_x17 =  0.163925F  ;
   va_dur17 =  0.127010F  ;
   va_x18 =  0.106779F  ;
   va_dur18 =  0.147910F  ;
   va_x19 =  0.069834F  ;
   va_dur19 =  0.697749F  ;
   va_x20 =  0.011686F  ;
   va_dur20 =  1.292604F  ;
   va_x21 =  0.0F  ;
   ret = 0.0F;
   if (NVI(piano_kline1_first)>0)
   {
      NV(piano_kline1_t) += EV(KTIME);
      ret = (NV(piano_kline1_outT) += NV(piano_kline1_addK));
      if (NV(piano_kline1_t) > NV(piano_kline1_cdur))
       {
        while (NV(piano_kline1_t) > NV(piano_kline1_cdur))
         {
           NV(piano_kline1_t) -= NV(piano_kline1_cdur);
           switch(NVI(piano_kline1_first))
      {
         case 1:
         NV(piano_kline1_cdur) = va_dur2;
           NV(piano_kline1_clp) = NV(piano_kline1_crp);
           NV(piano_kline1_crp) = va_x3;
           break;
         case 2:
         NV(piano_kline1_cdur) = va_dur3;
           NV(piano_kline1_clp) = NV(piano_kline1_crp);
           NV(piano_kline1_crp) = va_x4;
           break;
         case 3:
         NV(piano_kline1_cdur) = va_dur4;
           NV(piano_kline1_clp) = NV(piano_kline1_crp);
           NV(piano_kline1_crp) = va_x5;
           break;
         case 4:
         NV(piano_kline1_cdur) = va_dur5;
           NV(piano_kline1_clp) = NV(piano_kline1_crp);
           NV(piano_kline1_crp) = va_x6;
           break;
         case 5:
         NV(piano_kline1_cdur) = va_dur6;
           NV(piano_kline1_clp) = NV(piano_kline1_crp);
           NV(piano_kline1_crp) = va_x7;
           break;
         case 6:
         NV(piano_kline1_cdur) = va_dur7;
           NV(piano_kline1_clp) = NV(piano_kline1_crp);
           NV(piano_kline1_crp) = va_x8;
           break;
         case 7:
         NV(piano_kline1_cdur) = va_dur8;
           NV(piano_kline1_clp) = NV(piano_kline1_crp);
           NV(piano_kline1_crp) = va_x9;
           break;
         case 8:
         NV(piano_kline1_cdur) = va_dur9;
           NV(piano_kline1_clp) = NV(piano_kline1_crp);
           NV(piano_kline1_crp) = va_x10;
           break;
         case 9:
         NV(piano_kline1_cdur) = va_dur10;
           NV(piano_kline1_clp) = NV(piano_kline1_crp);
           NV(piano_kline1_crp) = va_x11;
           break;
         case 10:
         NV(piano_kline1_cdur) = va_dur11;
           NV(piano_kline1_clp) = NV(piano_kline1_crp);
           NV(piano_kline1_crp) = va_x12;
           break;
         case 11:
         NV(piano_kline1_cdur) = va_dur12;
           NV(piano_kline1_clp) = NV(piano_kline1_crp);
           NV(piano_kline1_crp) = va_x13;
           break;
         case 12:
         NV(piano_kline1_cdur) = va_dur13;
           NV(piano_kline1_clp) = NV(piano_kline1_crp);
           NV(piano_kline1_crp) = va_x14;
           break;
         case 13:
         NV(piano_kline1_cdur) = va_dur14;
           NV(piano_kline1_clp) = NV(piano_kline1_crp);
           NV(piano_kline1_crp) = va_x15;
           break;
         case 14:
         NV(piano_kline1_cdur) = va_dur15;
           NV(piano_kline1_clp) = NV(piano_kline1_crp);
           NV(piano_kline1_crp) = va_x16;
           break;
         case 15:
         NV(piano_kline1_cdur) = va_dur16;
           NV(piano_kline1_clp) = NV(piano_kline1_crp);
           NV(piano_kline1_crp) = va_x17;
           break;
         case 16:
         NV(piano_kline1_cdur) = va_dur17;
           NV(piano_kline1_clp) = NV(piano_kline1_crp);
           NV(piano_kline1_crp) = va_x18;
           break;
         case 17:
         NV(piano_kline1_cdur) = va_dur18;
           NV(piano_kline1_clp) = NV(piano_kline1_crp);
           NV(piano_kline1_crp) = va_x19;
           break;
         case 18:
         NV(piano_kline1_cdur) = va_dur19;
           NV(piano_kline1_clp) = NV(piano_kline1_crp);
           NV(piano_kline1_crp) = va_x20;
           break;
         case 19:
         NV(piano_kline1_cdur) = va_dur20;
           NV(piano_kline1_clp) = NV(piano_kline1_crp);
           NV(piano_kline1_crp) = va_x21;
           break;
           default:
           NVI(piano_kline1_first) = -100;
           NV(piano_kline1_cdur) = NV(piano_kline1_t) + 10000.0F;
           break;
           }
         NVI(piano_kline1_first)++;
        }
        NV(piano_kline1_mult)=(NV(piano_kline1_crp) - NV(piano_kline1_clp))/NV(piano_kline1_cdur);
        ret = NV(piano_kline1_outT) = NV(piano_kline1_clp)+NV(piano_kline1_mult)*NV(piano_kline1_t);
        NV(piano_kline1_addK) = NV(piano_kline1_mult)*EV(KTIME);
        if (NVI(piano_kline1_first)<0)
          ret = 0.0F;
      }
   }
   if (NVI(piano_kline1_first)==0)
     {
       NVI(piano_kline1_first) = 1;
       ret = NV(piano_kline1_outT) = NV(piano_kline1_clp) = x1;
       NV(piano_kline1_crp) = x2;
       NV(piano_kline1_cdur) = dur1;
       if (dur1 > 0.0F)
         NV(piano_kline1_addK) = EV(KTIME)*((x2 - x1)/dur1);
     }
   return((NV(piano_kline1_return) = ret));

}



float piano__sym_kline2(ENGINE_PTR_DECLARE_COMMA struct ninstr_types * nstate)
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
   float va_dur20;
   float va_x21;
   float x2;
   float dur1;
   float x1;
   float ret;

   x1 =  0.0F ;
   dur1 =  0.001608F ;
   x2 =  0.097396F ;
   va_dur2 =  0.001608F  ;
   va_x3 =  0.081971F  ;
   va_dur3 =  0.001608F  ;
   va_x4 =  0.078928F  ;
   va_dur4 =  0.001608F  ;
   va_x5 =  0.081134F  ;
   va_dur5 =  0.003215F  ;
   va_x6 =  0.116144F  ;
   va_dur6 =  0.003215F  ;
   va_x7 =  0.091460F  ;
   va_dur7 =  0.004823F  ;
   va_x8 =  0.113417F  ;
   va_dur8 =  0.006431F  ;
   va_x9 =  0.093345F  ;
   va_dur9 =  0.004823F  ;
   va_x10 =  0.112132F  ;
   va_dur10 =  0.006431F  ;
   va_x11 =  0.120713F  ;
   va_dur11 =  0.024116F  ;
   va_x12 =  0.108884F  ;
   va_dur12 =  0.016077F  ;
   va_x13 =  0.116759F  ;
   va_dur13 =  0.012862F  ;
   va_x14 =  0.101062F  ;
   va_dur14 =  0.032154F  ;
   va_x15 =  0.097333F  ;
   va_dur15 =  0.036977F  ;
   va_x16 =  0.077621F  ;
   va_dur16 =  0.011254F  ;
   va_x17 =  0.090239F  ;
   va_dur17 =  0.085209F  ;
   va_x18 =  0.058597F  ;
   va_dur18 =  0.331190F  ;
   va_x19 =  0.018250F  ;
   va_dur19 =  0.215434F  ;
   va_x20 =  0.003887F  ;
   va_dur20 =  1.691318F  ;
   va_x21 =  0.0F  ;
   ret = 0.0F;
   if (NVI(piano_kline2_first)>0)
   {
      NV(piano_kline2_t) += EV(KTIME);
      ret = (NV(piano_kline2_outT) += NV(piano_kline2_addK));
      if (NV(piano_kline2_t) > NV(piano_kline2_cdur))
       {
        while (NV(piano_kline2_t) > NV(piano_kline2_cdur))
         {
           NV(piano_kline2_t) -= NV(piano_kline2_cdur);
           switch(NVI(piano_kline2_first))
      {
         case 1:
         NV(piano_kline2_cdur) = va_dur2;
           NV(piano_kline2_clp) = NV(piano_kline2_crp);
           NV(piano_kline2_crp) = va_x3;
           break;
         case 2:
         NV(piano_kline2_cdur) = va_dur3;
           NV(piano_kline2_clp) = NV(piano_kline2_crp);
           NV(piano_kline2_crp) = va_x4;
           break;
         case 3:
         NV(piano_kline2_cdur) = va_dur4;
           NV(piano_kline2_clp) = NV(piano_kline2_crp);
           NV(piano_kline2_crp) = va_x5;
           break;
         case 4:
         NV(piano_kline2_cdur) = va_dur5;
           NV(piano_kline2_clp) = NV(piano_kline2_crp);
           NV(piano_kline2_crp) = va_x6;
           break;
         case 5:
         NV(piano_kline2_cdur) = va_dur6;
           NV(piano_kline2_clp) = NV(piano_kline2_crp);
           NV(piano_kline2_crp) = va_x7;
           break;
         case 6:
         NV(piano_kline2_cdur) = va_dur7;
           NV(piano_kline2_clp) = NV(piano_kline2_crp);
           NV(piano_kline2_crp) = va_x8;
           break;
         case 7:
         NV(piano_kline2_cdur) = va_dur8;
           NV(piano_kline2_clp) = NV(piano_kline2_crp);
           NV(piano_kline2_crp) = va_x9;
           break;
         case 8:
         NV(piano_kline2_cdur) = va_dur9;
           NV(piano_kline2_clp) = NV(piano_kline2_crp);
           NV(piano_kline2_crp) = va_x10;
           break;
         case 9:
         NV(piano_kline2_cdur) = va_dur10;
           NV(piano_kline2_clp) = NV(piano_kline2_crp);
           NV(piano_kline2_crp) = va_x11;
           break;
         case 10:
         NV(piano_kline2_cdur) = va_dur11;
           NV(piano_kline2_clp) = NV(piano_kline2_crp);
           NV(piano_kline2_crp) = va_x12;
           break;
         case 11:
         NV(piano_kline2_cdur) = va_dur12;
           NV(piano_kline2_clp) = NV(piano_kline2_crp);
           NV(piano_kline2_crp) = va_x13;
           break;
         case 12:
         NV(piano_kline2_cdur) = va_dur13;
           NV(piano_kline2_clp) = NV(piano_kline2_crp);
           NV(piano_kline2_crp) = va_x14;
           break;
         case 13:
         NV(piano_kline2_cdur) = va_dur14;
           NV(piano_kline2_clp) = NV(piano_kline2_crp);
           NV(piano_kline2_crp) = va_x15;
           break;
         case 14:
         NV(piano_kline2_cdur) = va_dur15;
           NV(piano_kline2_clp) = NV(piano_kline2_crp);
           NV(piano_kline2_crp) = va_x16;
           break;
         case 15:
         NV(piano_kline2_cdur) = va_dur16;
           NV(piano_kline2_clp) = NV(piano_kline2_crp);
           NV(piano_kline2_crp) = va_x17;
           break;
         case 16:
         NV(piano_kline2_cdur) = va_dur17;
           NV(piano_kline2_clp) = NV(piano_kline2_crp);
           NV(piano_kline2_crp) = va_x18;
           break;
         case 17:
         NV(piano_kline2_cdur) = va_dur18;
           NV(piano_kline2_clp) = NV(piano_kline2_crp);
           NV(piano_kline2_crp) = va_x19;
           break;
         case 18:
         NV(piano_kline2_cdur) = va_dur19;
           NV(piano_kline2_clp) = NV(piano_kline2_crp);
           NV(piano_kline2_crp) = va_x20;
           break;
         case 19:
         NV(piano_kline2_cdur) = va_dur20;
           NV(piano_kline2_clp) = NV(piano_kline2_crp);
           NV(piano_kline2_crp) = va_x21;
           break;
           default:
           NVI(piano_kline2_first) = -100;
           NV(piano_kline2_cdur) = NV(piano_kline2_t) + 10000.0F;
           break;
           }
         NVI(piano_kline2_first)++;
        }
        NV(piano_kline2_mult)=(NV(piano_kline2_crp) - NV(piano_kline2_clp))/NV(piano_kline2_cdur);
        ret = NV(piano_kline2_outT) = NV(piano_kline2_clp)+NV(piano_kline2_mult)*NV(piano_kline2_t);
        NV(piano_kline2_addK) = NV(piano_kline2_mult)*EV(KTIME);
        if (NVI(piano_kline2_first)<0)
          ret = 0.0F;
      }
   }
   if (NVI(piano_kline2_first)==0)
     {
       NVI(piano_kline2_first) = 1;
       ret = NV(piano_kline2_outT) = NV(piano_kline2_clp) = x1;
       NV(piano_kline2_crp) = x2;
       NV(piano_kline2_cdur) = dur1;
       if (dur1 > 0.0F)
         NV(piano_kline2_addK) = EV(KTIME)*((x2 - x1)/dur1);
     }
   return((NV(piano_kline2_return) = ret));

}



float piano__sym_kline3(ENGINE_PTR_DECLARE_COMMA struct ninstr_types * nstate)
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
   float va_dur20;
   float va_x21;
   float x2;
   float dur1;
   float x1;
   float ret;

   x1 =  0.0F ;
   dur1 =  0.001608F ;
   x2 =  0.103600F ;
   va_dur2 =  0.006431F  ;
   va_x3 =  0.123191F  ;
   va_dur3 =  0.003215F  ;
   va_x4 =  0.119140F  ;
   va_dur4 =  0.003215F  ;
   va_x5 =  0.122436F  ;
   va_dur5 =  0.004823F  ;
   va_x6 =  0.143915F  ;
   va_dur6 =  0.001608F  ;
   va_x7 =  0.146819F  ;
   va_dur7 =  0.004823F  ;
   va_x8 =  0.140883F  ;
   va_dur8 =  0.011254F  ;
   va_x9 =  0.137932F  ;
   va_dur9 =  0.011254F  ;
   va_x10 =  0.125505F  ;
   va_dur10 =  0.024116F  ;
   va_x11 =  0.125184F  ;
   va_dur11 =  0.038585F  ;
   va_x12 =  0.107072F  ;
   va_dur12 =  0.012862F  ;
   va_x13 =  0.106261F  ;
   va_dur13 =  0.122186F  ;
   va_x14 =  0.064693F  ;
   va_dur14 =  0.159164F  ;
   va_x15 =  0.032198F  ;
   va_dur15 =  0.127010F  ;
   va_x16 =  0.015985F  ;
   va_dur16 =  0.093248F  ;
   va_x17 =  0.009119F  ;
   va_dur17 =  0.567524F  ;
   va_x18 =  0.014227F  ;
   va_dur18 =  1.295820F  ;
   va_x19 =  0.005881F  ;
   va_dur19 =  0.003215F  ;
   va_x20 =  0.005852F  ;
   va_dur20 =  0.0F  ;
   va_x21 =  0.0F  ;
   ret = 0.0F;
   if (NVI(piano_kline3_first)>0)
   {
      NV(piano_kline3_t) += EV(KTIME);
      ret = (NV(piano_kline3_outT) += NV(piano_kline3_addK));
      if (NV(piano_kline3_t) > NV(piano_kline3_cdur))
       {
        while (NV(piano_kline3_t) > NV(piano_kline3_cdur))
         {
           NV(piano_kline3_t) -= NV(piano_kline3_cdur);
           switch(NVI(piano_kline3_first))
      {
         case 1:
         NV(piano_kline3_cdur) = va_dur2;
           NV(piano_kline3_clp) = NV(piano_kline3_crp);
           NV(piano_kline3_crp) = va_x3;
           break;
         case 2:
         NV(piano_kline3_cdur) = va_dur3;
           NV(piano_kline3_clp) = NV(piano_kline3_crp);
           NV(piano_kline3_crp) = va_x4;
           break;
         case 3:
         NV(piano_kline3_cdur) = va_dur4;
           NV(piano_kline3_clp) = NV(piano_kline3_crp);
           NV(piano_kline3_crp) = va_x5;
           break;
         case 4:
         NV(piano_kline3_cdur) = va_dur5;
           NV(piano_kline3_clp) = NV(piano_kline3_crp);
           NV(piano_kline3_crp) = va_x6;
           break;
         case 5:
         NV(piano_kline3_cdur) = va_dur6;
           NV(piano_kline3_clp) = NV(piano_kline3_crp);
           NV(piano_kline3_crp) = va_x7;
           break;
         case 6:
         NV(piano_kline3_cdur) = va_dur7;
           NV(piano_kline3_clp) = NV(piano_kline3_crp);
           NV(piano_kline3_crp) = va_x8;
           break;
         case 7:
         NV(piano_kline3_cdur) = va_dur8;
           NV(piano_kline3_clp) = NV(piano_kline3_crp);
           NV(piano_kline3_crp) = va_x9;
           break;
         case 8:
         NV(piano_kline3_cdur) = va_dur9;
           NV(piano_kline3_clp) = NV(piano_kline3_crp);
           NV(piano_kline3_crp) = va_x10;
           break;
         case 9:
         NV(piano_kline3_cdur) = va_dur10;
           NV(piano_kline3_clp) = NV(piano_kline3_crp);
           NV(piano_kline3_crp) = va_x11;
           break;
         case 10:
         NV(piano_kline3_cdur) = va_dur11;
           NV(piano_kline3_clp) = NV(piano_kline3_crp);
           NV(piano_kline3_crp) = va_x12;
           break;
         case 11:
         NV(piano_kline3_cdur) = va_dur12;
           NV(piano_kline3_clp) = NV(piano_kline3_crp);
           NV(piano_kline3_crp) = va_x13;
           break;
         case 12:
         NV(piano_kline3_cdur) = va_dur13;
           NV(piano_kline3_clp) = NV(piano_kline3_crp);
           NV(piano_kline3_crp) = va_x14;
           break;
         case 13:
         NV(piano_kline3_cdur) = va_dur14;
           NV(piano_kline3_clp) = NV(piano_kline3_crp);
           NV(piano_kline3_crp) = va_x15;
           break;
         case 14:
         NV(piano_kline3_cdur) = va_dur15;
           NV(piano_kline3_clp) = NV(piano_kline3_crp);
           NV(piano_kline3_crp) = va_x16;
           break;
         case 15:
         NV(piano_kline3_cdur) = va_dur16;
           NV(piano_kline3_clp) = NV(piano_kline3_crp);
           NV(piano_kline3_crp) = va_x17;
           break;
         case 16:
         NV(piano_kline3_cdur) = va_dur17;
           NV(piano_kline3_clp) = NV(piano_kline3_crp);
           NV(piano_kline3_crp) = va_x18;
           break;
         case 17:
         NV(piano_kline3_cdur) = va_dur18;
           NV(piano_kline3_clp) = NV(piano_kline3_crp);
           NV(piano_kline3_crp) = va_x19;
           break;
         case 18:
         NV(piano_kline3_cdur) = va_dur19;
           NV(piano_kline3_clp) = NV(piano_kline3_crp);
           NV(piano_kline3_crp) = va_x20;
           break;
         case 19:
         NV(piano_kline3_cdur) = va_dur20;
           NV(piano_kline3_clp) = NV(piano_kline3_crp);
           NV(piano_kline3_crp) = va_x21;
           break;
           default:
           NVI(piano_kline3_first) = -100;
           NV(piano_kline3_cdur) = NV(piano_kline3_t) + 10000.0F;
           break;
           }
         NVI(piano_kline3_first)++;
        }
        NV(piano_kline3_mult)=(NV(piano_kline3_crp) - NV(piano_kline3_clp))/NV(piano_kline3_cdur);
        ret = NV(piano_kline3_outT) = NV(piano_kline3_clp)+NV(piano_kline3_mult)*NV(piano_kline3_t);
        NV(piano_kline3_addK) = NV(piano_kline3_mult)*EV(KTIME);
        if (NVI(piano_kline3_first)<0)
          ret = 0.0F;
      }
   }
   if (NVI(piano_kline3_first)==0)
     {
       NVI(piano_kline3_first) = 1;
       ret = NV(piano_kline3_outT) = NV(piano_kline3_clp) = x1;
       NV(piano_kline3_crp) = x2;
       NV(piano_kline3_cdur) = dur1;
       if (dur1 > 0.0F)
         NV(piano_kline3_addK) = EV(KTIME)*((x2 - x1)/dur1);
     }
   return((NV(piano_kline3_return) = ret));

}



float piano__sym_kline4(ENGINE_PTR_DECLARE_COMMA struct ninstr_types * nstate)
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
   float va_dur20;
   float va_x21;
   float x2;
   float dur1;
   float x1;
   float ret;

   x1 =  0.0F ;
   dur1 =  0.001608F ;
   x2 =  0.065044F ;
   va_dur2 =  0.001608F  ;
   va_x3 =  0.066805F  ;
   va_dur3 =  0.003215F  ;
   va_x4 =  0.061627F  ;
   va_dur4 =  0.003215F  ;
   va_x5 =  0.075400F  ;
   va_dur5 =  0.001608F  ;
   va_x6 =  0.077307F  ;
   va_dur6 =  0.004823F  ;
   va_x7 =  0.072471F  ;
   va_dur7 =  0.001608F  ;
   va_x8 =  0.074500F  ;
   va_dur8 =  0.009646F  ;
   va_x9 =  0.067854F  ;
   va_dur9 =  0.009646F  ;
   va_x10 =  0.070116F  ;
   va_dur10 =  0.004823F  ;
   va_x11 =  0.074675F  ;
   va_dur11 =  0.017685F  ;
   va_x12 =  0.067747F  ;
   va_dur12 =  0.011254F  ;
   va_x13 =  0.067912F  ;
   va_dur13 =  0.099678F  ;
   va_x14 =  0.035034F  ;
   va_dur14 =  0.011254F  ;
   va_x15 =  0.034537F  ;
   va_dur15 =  0.033762F  ;
   va_x16 =  0.026122F  ;
   va_dur16 =  0.043408F  ;
   va_x17 =  0.022119F  ;
   va_dur17 =  0.046624F  ;
   va_x18 =  0.021864F  ;
   va_dur18 =  0.155949F  ;
   va_x19 =  0.012716F  ;
   va_dur19 =  0.842444F  ;
   va_x20 =  0.002154F  ;
   va_dur20 =  1.188103F  ;
   va_x21 =  0.0F  ;
   ret = 0.0F;
   if (NVI(piano_kline4_first)>0)
   {
      NV(piano_kline4_t) += EV(KTIME);
      ret = (NV(piano_kline4_outT) += NV(piano_kline4_addK));
      if (NV(piano_kline4_t) > NV(piano_kline4_cdur))
       {
        while (NV(piano_kline4_t) > NV(piano_kline4_cdur))
         {
           NV(piano_kline4_t) -= NV(piano_kline4_cdur);
           switch(NVI(piano_kline4_first))
      {
         case 1:
         NV(piano_kline4_cdur) = va_dur2;
           NV(piano_kline4_clp) = NV(piano_kline4_crp);
           NV(piano_kline4_crp) = va_x3;
           break;
         case 2:
         NV(piano_kline4_cdur) = va_dur3;
           NV(piano_kline4_clp) = NV(piano_kline4_crp);
           NV(piano_kline4_crp) = va_x4;
           break;
         case 3:
         NV(piano_kline4_cdur) = va_dur4;
           NV(piano_kline4_clp) = NV(piano_kline4_crp);
           NV(piano_kline4_crp) = va_x5;
           break;
         case 4:
         NV(piano_kline4_cdur) = va_dur5;
           NV(piano_kline4_clp) = NV(piano_kline4_crp);
           NV(piano_kline4_crp) = va_x6;
           break;
         case 5:
         NV(piano_kline4_cdur) = va_dur6;
           NV(piano_kline4_clp) = NV(piano_kline4_crp);
           NV(piano_kline4_crp) = va_x7;
           break;
         case 6:
         NV(piano_kline4_cdur) = va_dur7;
           NV(piano_kline4_clp) = NV(piano_kline4_crp);
           NV(piano_kline4_crp) = va_x8;
           break;
         case 7:
         NV(piano_kline4_cdur) = va_dur8;
           NV(piano_kline4_clp) = NV(piano_kline4_crp);
           NV(piano_kline4_crp) = va_x9;
           break;
         case 8:
         NV(piano_kline4_cdur) = va_dur9;
           NV(piano_kline4_clp) = NV(piano_kline4_crp);
           NV(piano_kline4_crp) = va_x10;
           break;
         case 9:
         NV(piano_kline4_cdur) = va_dur10;
           NV(piano_kline4_clp) = NV(piano_kline4_crp);
           NV(piano_kline4_crp) = va_x11;
           break;
         case 10:
         NV(piano_kline4_cdur) = va_dur11;
           NV(piano_kline4_clp) = NV(piano_kline4_crp);
           NV(piano_kline4_crp) = va_x12;
           break;
         case 11:
         NV(piano_kline4_cdur) = va_dur12;
           NV(piano_kline4_clp) = NV(piano_kline4_crp);
           NV(piano_kline4_crp) = va_x13;
           break;
         case 12:
         NV(piano_kline4_cdur) = va_dur13;
           NV(piano_kline4_clp) = NV(piano_kline4_crp);
           NV(piano_kline4_crp) = va_x14;
           break;
         case 13:
         NV(piano_kline4_cdur) = va_dur14;
           NV(piano_kline4_clp) = NV(piano_kline4_crp);
           NV(piano_kline4_crp) = va_x15;
           break;
         case 14:
         NV(piano_kline4_cdur) = va_dur15;
           NV(piano_kline4_clp) = NV(piano_kline4_crp);
           NV(piano_kline4_crp) = va_x16;
           break;
         case 15:
         NV(piano_kline4_cdur) = va_dur16;
           NV(piano_kline4_clp) = NV(piano_kline4_crp);
           NV(piano_kline4_crp) = va_x17;
           break;
         case 16:
         NV(piano_kline4_cdur) = va_dur17;
           NV(piano_kline4_clp) = NV(piano_kline4_crp);
           NV(piano_kline4_crp) = va_x18;
           break;
         case 17:
         NV(piano_kline4_cdur) = va_dur18;
           NV(piano_kline4_clp) = NV(piano_kline4_crp);
           NV(piano_kline4_crp) = va_x19;
           break;
         case 18:
         NV(piano_kline4_cdur) = va_dur19;
           NV(piano_kline4_clp) = NV(piano_kline4_crp);
           NV(piano_kline4_crp) = va_x20;
           break;
         case 19:
         NV(piano_kline4_cdur) = va_dur20;
           NV(piano_kline4_clp) = NV(piano_kline4_crp);
           NV(piano_kline4_crp) = va_x21;
           break;
           default:
           NVI(piano_kline4_first) = -100;
           NV(piano_kline4_cdur) = NV(piano_kline4_t) + 10000.0F;
           break;
           }
         NVI(piano_kline4_first)++;
        }
        NV(piano_kline4_mult)=(NV(piano_kline4_crp) - NV(piano_kline4_clp))/NV(piano_kline4_cdur);
        ret = NV(piano_kline4_outT) = NV(piano_kline4_clp)+NV(piano_kline4_mult)*NV(piano_kline4_t);
        NV(piano_kline4_addK) = NV(piano_kline4_mult)*EV(KTIME);
        if (NVI(piano_kline4_first)<0)
          ret = 0.0F;
      }
   }
   if (NVI(piano_kline4_first)==0)
     {
       NVI(piano_kline4_first) = 1;
       ret = NV(piano_kline4_outT) = NV(piano_kline4_clp) = x1;
       NV(piano_kline4_crp) = x2;
       NV(piano_kline4_cdur) = dur1;
       if (dur1 > 0.0F)
         NV(piano_kline4_addK) = EV(KTIME)*((x2 - x1)/dur1);
     }
   return((NV(piano_kline4_return) = ret));

}



float piano__sym_kline5(ENGINE_PTR_DECLARE_COMMA struct ninstr_types * nstate)
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
   float va_dur20;
   float va_x21;
   float x2;
   float dur1;
   float x1;
   float ret;

   x1 =  0.0F ;
   dur1 =  0.001608F ;
   x2 =  0.079765F ;
   va_dur2 =  0.001608F  ;
   va_x3 =  0.091854F  ;
   va_dur3 =  0.004823F  ;
   va_x4 =  0.103992F  ;
   va_dur4 =  0.006431F  ;
   va_x5 =  0.102229F  ;
   va_dur5 =  0.008039F  ;
   va_x6 =  0.089347F  ;
   va_dur6 =  0.046624F  ;
   va_x7 =  0.080054F  ;
   va_dur7 =  0.025723F  ;
   va_x8 =  0.071244F  ;
   va_dur8 =  0.046624F  ;
   va_x9 =  0.062437F  ;
   va_dur9 =  0.024116F  ;
   va_x10 =  0.054804F  ;
   va_dur10 =  0.024116F  ;
   va_x11 =  0.053358F  ;
   va_dur11 =  0.069132F  ;
   va_x12 =  0.035083F  ;
   va_dur12 =  0.141479F  ;
   va_x13 =  0.016152F  ;
   va_dur13 =  0.090032F  ;
   va_x14 =  0.009605F  ;
   va_dur14 =  0.281350F  ;
   va_x15 =  0.015397F  ;
   va_dur15 =  0.109325F  ;
   va_x16 =  0.014919F  ;
   va_dur16 =  0.635048F  ;
   va_x17 =  0.004381F  ;
   va_dur17 =  0.967846F  ;
   va_x18 =  0.003670F  ;
   va_dur18 =  0.006431F  ;
   va_x19 =  0.003570F  ;
   va_dur19 =  0.001608F  ;
   va_x20 =  0.003554F  ;
   va_dur20 =  0.0F  ;
   va_x21 =  0.0F  ;
   ret = 0.0F;
   if (NVI(piano_kline5_first)>0)
   {
      NV(piano_kline5_t) += EV(KTIME);
      ret = (NV(piano_kline5_outT) += NV(piano_kline5_addK));
      if (NV(piano_kline5_t) > NV(piano_kline5_cdur))
       {
        while (NV(piano_kline5_t) > NV(piano_kline5_cdur))
         {
           NV(piano_kline5_t) -= NV(piano_kline5_cdur);
           switch(NVI(piano_kline5_first))
      {
         case 1:
         NV(piano_kline5_cdur) = va_dur2;
           NV(piano_kline5_clp) = NV(piano_kline5_crp);
           NV(piano_kline5_crp) = va_x3;
           break;
         case 2:
         NV(piano_kline5_cdur) = va_dur3;
           NV(piano_kline5_clp) = NV(piano_kline5_crp);
           NV(piano_kline5_crp) = va_x4;
           break;
         case 3:
         NV(piano_kline5_cdur) = va_dur4;
           NV(piano_kline5_clp) = NV(piano_kline5_crp);
           NV(piano_kline5_crp) = va_x5;
           break;
         case 4:
         NV(piano_kline5_cdur) = va_dur5;
           NV(piano_kline5_clp) = NV(piano_kline5_crp);
           NV(piano_kline5_crp) = va_x6;
           break;
         case 5:
         NV(piano_kline5_cdur) = va_dur6;
           NV(piano_kline5_clp) = NV(piano_kline5_crp);
           NV(piano_kline5_crp) = va_x7;
           break;
         case 6:
         NV(piano_kline5_cdur) = va_dur7;
           NV(piano_kline5_clp) = NV(piano_kline5_crp);
           NV(piano_kline5_crp) = va_x8;
           break;
         case 7:
         NV(piano_kline5_cdur) = va_dur8;
           NV(piano_kline5_clp) = NV(piano_kline5_crp);
           NV(piano_kline5_crp) = va_x9;
           break;
         case 8:
         NV(piano_kline5_cdur) = va_dur9;
           NV(piano_kline5_clp) = NV(piano_kline5_crp);
           NV(piano_kline5_crp) = va_x10;
           break;
         case 9:
         NV(piano_kline5_cdur) = va_dur10;
           NV(piano_kline5_clp) = NV(piano_kline5_crp);
           NV(piano_kline5_crp) = va_x11;
           break;
         case 10:
         NV(piano_kline5_cdur) = va_dur11;
           NV(piano_kline5_clp) = NV(piano_kline5_crp);
           NV(piano_kline5_crp) = va_x12;
           break;
         case 11:
         NV(piano_kline5_cdur) = va_dur12;
           NV(piano_kline5_clp) = NV(piano_kline5_crp);
           NV(piano_kline5_crp) = va_x13;
           break;
         case 12:
         NV(piano_kline5_cdur) = va_dur13;
           NV(piano_kline5_clp) = NV(piano_kline5_crp);
           NV(piano_kline5_crp) = va_x14;
           break;
         case 13:
         NV(piano_kline5_cdur) = va_dur14;
           NV(piano_kline5_clp) = NV(piano_kline5_crp);
           NV(piano_kline5_crp) = va_x15;
           break;
         case 14:
         NV(piano_kline5_cdur) = va_dur15;
           NV(piano_kline5_clp) = NV(piano_kline5_crp);
           NV(piano_kline5_crp) = va_x16;
           break;
         case 15:
         NV(piano_kline5_cdur) = va_dur16;
           NV(piano_kline5_clp) = NV(piano_kline5_crp);
           NV(piano_kline5_crp) = va_x17;
           break;
         case 16:
         NV(piano_kline5_cdur) = va_dur17;
           NV(piano_kline5_clp) = NV(piano_kline5_crp);
           NV(piano_kline5_crp) = va_x18;
           break;
         case 17:
         NV(piano_kline5_cdur) = va_dur18;
           NV(piano_kline5_clp) = NV(piano_kline5_crp);
           NV(piano_kline5_crp) = va_x19;
           break;
         case 18:
         NV(piano_kline5_cdur) = va_dur19;
           NV(piano_kline5_clp) = NV(piano_kline5_crp);
           NV(piano_kline5_crp) = va_x20;
           break;
         case 19:
         NV(piano_kline5_cdur) = va_dur20;
           NV(piano_kline5_clp) = NV(piano_kline5_crp);
           NV(piano_kline5_crp) = va_x21;
           break;
           default:
           NVI(piano_kline5_first) = -100;
           NV(piano_kline5_cdur) = NV(piano_kline5_t) + 10000.0F;
           break;
           }
         NVI(piano_kline5_first)++;
        }
        NV(piano_kline5_mult)=(NV(piano_kline5_crp) - NV(piano_kline5_clp))/NV(piano_kline5_cdur);
        ret = NV(piano_kline5_outT) = NV(piano_kline5_clp)+NV(piano_kline5_mult)*NV(piano_kline5_t);
        NV(piano_kline5_addK) = NV(piano_kline5_mult)*EV(KTIME);
        if (NVI(piano_kline5_first)<0)
          ret = 0.0F;
      }
   }
   if (NVI(piano_kline5_first)==0)
     {
       NVI(piano_kline5_first) = 1;
       ret = NV(piano_kline5_outT) = NV(piano_kline5_clp) = x1;
       NV(piano_kline5_crp) = x2;
       NV(piano_kline5_cdur) = dur1;
       if (dur1 > 0.0F)
         NV(piano_kline5_addK) = EV(KTIME)*((x2 - x1)/dur1);
     }
   return((NV(piano_kline5_return) = ret));

}



float piano__sym_kline6(ENGINE_PTR_DECLARE_COMMA struct ninstr_types * nstate)
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
   float va_dur20;
   float va_x21;
   float x2;
   float dur1;
   float x1;
   float ret;

   x1 =  0.0F ;
   dur1 =  0.001608F ;
   x2 =  0.026125F ;
   va_dur2 =  0.001608F  ;
   va_x3 =  0.032187F  ;
   va_dur3 =  0.001608F  ;
   va_x4 =  0.033016F  ;
   va_dur4 =  0.001608F  ;
   va_x5 =  0.032131F  ;
   va_dur5 =  0.001608F  ;
   va_x6 =  0.025887F  ;
   va_dur6 =  0.001608F  ;
   va_x7 =  0.026114F  ;
   va_dur7 =  0.003215F  ;
   va_x8 =  0.031280F  ;
   va_dur8 =  0.003215F  ;
   va_x9 =  0.032152F  ;
   va_dur9 =  0.004823F  ;
   va_x10 =  0.028888F  ;
   va_dur10 =  0.009646F  ;
   va_x11 =  0.027340F  ;
   va_dur11 =  0.001608F  ;
   va_x12 =  0.028969F  ;
   va_dur12 =  0.004823F  ;
   va_x13 =  0.026172F  ;
   va_dur13 =  0.020900F  ;
   va_x14 =  0.022047F  ;
   va_dur14 =  0.049839F  ;
   va_x15 =  0.023855F  ;
   va_dur15 =  0.112540F  ;
   va_x16 =  0.012179F  ;
   va_dur16 =  0.141479F  ;
   va_x17 =  0.006274F  ;
   va_dur17 =  0.361736F  ;
   va_x18 =  0.009030F  ;
   va_dur18 =  0.811897F  ;
   va_x19 =  0.000498F  ;
   va_dur19 =  0.427653F  ;
   va_x20 =  0.001979F  ;
   va_dur20 =  0.528939F  ;
   va_x21 =  0.0F  ;
   ret = 0.0F;
   if (NVI(piano_kline6_first)>0)
   {
      NV(piano_kline6_t) += EV(KTIME);
      ret = (NV(piano_kline6_outT) += NV(piano_kline6_addK));
      if (NV(piano_kline6_t) > NV(piano_kline6_cdur))
       {
        while (NV(piano_kline6_t) > NV(piano_kline6_cdur))
         {
           NV(piano_kline6_t) -= NV(piano_kline6_cdur);
           switch(NVI(piano_kline6_first))
      {
         case 1:
         NV(piano_kline6_cdur) = va_dur2;
           NV(piano_kline6_clp) = NV(piano_kline6_crp);
           NV(piano_kline6_crp) = va_x3;
           break;
         case 2:
         NV(piano_kline6_cdur) = va_dur3;
           NV(piano_kline6_clp) = NV(piano_kline6_crp);
           NV(piano_kline6_crp) = va_x4;
           break;
         case 3:
         NV(piano_kline6_cdur) = va_dur4;
           NV(piano_kline6_clp) = NV(piano_kline6_crp);
           NV(piano_kline6_crp) = va_x5;
           break;
         case 4:
         NV(piano_kline6_cdur) = va_dur5;
           NV(piano_kline6_clp) = NV(piano_kline6_crp);
           NV(piano_kline6_crp) = va_x6;
           break;
         case 5:
         NV(piano_kline6_cdur) = va_dur6;
           NV(piano_kline6_clp) = NV(piano_kline6_crp);
           NV(piano_kline6_crp) = va_x7;
           break;
         case 6:
         NV(piano_kline6_cdur) = va_dur7;
           NV(piano_kline6_clp) = NV(piano_kline6_crp);
           NV(piano_kline6_crp) = va_x8;
           break;
         case 7:
         NV(piano_kline6_cdur) = va_dur8;
           NV(piano_kline6_clp) = NV(piano_kline6_crp);
           NV(piano_kline6_crp) = va_x9;
           break;
         case 8:
         NV(piano_kline6_cdur) = va_dur9;
           NV(piano_kline6_clp) = NV(piano_kline6_crp);
           NV(piano_kline6_crp) = va_x10;
           break;
         case 9:
         NV(piano_kline6_cdur) = va_dur10;
           NV(piano_kline6_clp) = NV(piano_kline6_crp);
           NV(piano_kline6_crp) = va_x11;
           break;
         case 10:
         NV(piano_kline6_cdur) = va_dur11;
           NV(piano_kline6_clp) = NV(piano_kline6_crp);
           NV(piano_kline6_crp) = va_x12;
           break;
         case 11:
         NV(piano_kline6_cdur) = va_dur12;
           NV(piano_kline6_clp) = NV(piano_kline6_crp);
           NV(piano_kline6_crp) = va_x13;
           break;
         case 12:
         NV(piano_kline6_cdur) = va_dur13;
           NV(piano_kline6_clp) = NV(piano_kline6_crp);
           NV(piano_kline6_crp) = va_x14;
           break;
         case 13:
         NV(piano_kline6_cdur) = va_dur14;
           NV(piano_kline6_clp) = NV(piano_kline6_crp);
           NV(piano_kline6_crp) = va_x15;
           break;
         case 14:
         NV(piano_kline6_cdur) = va_dur15;
           NV(piano_kline6_clp) = NV(piano_kline6_crp);
           NV(piano_kline6_crp) = va_x16;
           break;
         case 15:
         NV(piano_kline6_cdur) = va_dur16;
           NV(piano_kline6_clp) = NV(piano_kline6_crp);
           NV(piano_kline6_crp) = va_x17;
           break;
         case 16:
         NV(piano_kline6_cdur) = va_dur17;
           NV(piano_kline6_clp) = NV(piano_kline6_crp);
           NV(piano_kline6_crp) = va_x18;
           break;
         case 17:
         NV(piano_kline6_cdur) = va_dur18;
           NV(piano_kline6_clp) = NV(piano_kline6_crp);
           NV(piano_kline6_crp) = va_x19;
           break;
         case 18:
         NV(piano_kline6_cdur) = va_dur19;
           NV(piano_kline6_clp) = NV(piano_kline6_crp);
           NV(piano_kline6_crp) = va_x20;
           break;
         case 19:
         NV(piano_kline6_cdur) = va_dur20;
           NV(piano_kline6_clp) = NV(piano_kline6_crp);
           NV(piano_kline6_crp) = va_x21;
           break;
           default:
           NVI(piano_kline6_first) = -100;
           NV(piano_kline6_cdur) = NV(piano_kline6_t) + 10000.0F;
           break;
           }
         NVI(piano_kline6_first)++;
        }
        NV(piano_kline6_mult)=(NV(piano_kline6_crp) - NV(piano_kline6_clp))/NV(piano_kline6_cdur);
        ret = NV(piano_kline6_outT) = NV(piano_kline6_clp)+NV(piano_kline6_mult)*NV(piano_kline6_t);
        NV(piano_kline6_addK) = NV(piano_kline6_mult)*EV(KTIME);
        if (NVI(piano_kline6_first)<0)
          ret = 0.0F;
      }
   }
   if (NVI(piano_kline6_first)==0)
     {
       NVI(piano_kline6_first) = 1;
       ret = NV(piano_kline6_outT) = NV(piano_kline6_clp) = x1;
       NV(piano_kline6_crp) = x2;
       NV(piano_kline6_cdur) = dur1;
       if (dur1 > 0.0F)
         NV(piano_kline6_addK) = EV(KTIME)*((x2 - x1)/dur1);
     }
   return((NV(piano_kline6_return) = ret));

}



float piano__sym_kline7(ENGINE_PTR_DECLARE_COMMA struct ninstr_types * nstate)
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
   float va_dur20;
   float va_x21;
   float x2;
   float dur1;
   float x1;
   float ret;

   x1 =  0.0F ;
   dur1 =  0.001608F ;
   x2 =  0.009863F ;
   va_dur2 =  0.001608F  ;
   va_x3 =  0.010075F  ;
   va_dur3 =  0.001608F  ;
   va_x4 =  0.016387F  ;
   va_dur4 =  0.001608F  ;
   va_x5 =  0.018609F  ;
   va_dur5 =  0.004823F  ;
   va_x6 =  0.021923F  ;
   va_dur6 =  0.001608F  ;
   va_x7 =  0.020323F  ;
   va_dur7 =  0.003215F  ;
   va_x8 =  0.021566F  ;
   va_dur8 =  0.001608F  ;
   va_x9 =  0.023717F  ;
   va_dur9 =  0.011254F  ;
   va_x10 =  0.017165F  ;
   va_dur10 =  0.035370F  ;
   va_x11 =  0.018974F  ;
   va_dur11 =  0.012862F  ;
   va_x12 =  0.017836F  ;
   va_dur12 =  0.008039F  ;
   va_x13 =  0.014402F  ;
   va_dur13 =  0.022508F  ;
   va_x14 =  0.014410F  ;
   va_dur14 =  0.033762F  ;
   va_x15 =  0.010147F  ;
   va_dur15 =  0.022508F  ;
   va_x16 =  0.011305F  ;
   va_dur16 =  0.099678F  ;
   va_x17 =  0.006955F  ;
   va_dur17 =  0.233119F  ;
   va_x18 =  0.008012F  ;
   va_dur18 =  0.707395F  ;
   va_x19 =  0.001255F  ;
   va_dur19 =  0.501608F  ;
   va_x20 =  0.002735F  ;
   va_dur20 =  0.786174F  ;
   va_x21 =  0.0F  ;
   ret = 0.0F;
   if (NVI(piano_kline7_first)>0)
   {
      NV(piano_kline7_t) += EV(KTIME);
      ret = (NV(piano_kline7_outT) += NV(piano_kline7_addK));
      if (NV(piano_kline7_t) > NV(piano_kline7_cdur))
       {
        while (NV(piano_kline7_t) > NV(piano_kline7_cdur))
         {
           NV(piano_kline7_t) -= NV(piano_kline7_cdur);
           switch(NVI(piano_kline7_first))
      {
         case 1:
         NV(piano_kline7_cdur) = va_dur2;
           NV(piano_kline7_clp) = NV(piano_kline7_crp);
           NV(piano_kline7_crp) = va_x3;
           break;
         case 2:
         NV(piano_kline7_cdur) = va_dur3;
           NV(piano_kline7_clp) = NV(piano_kline7_crp);
           NV(piano_kline7_crp) = va_x4;
           break;
         case 3:
         NV(piano_kline7_cdur) = va_dur4;
           NV(piano_kline7_clp) = NV(piano_kline7_crp);
           NV(piano_kline7_crp) = va_x5;
           break;
         case 4:
         NV(piano_kline7_cdur) = va_dur5;
           NV(piano_kline7_clp) = NV(piano_kline7_crp);
           NV(piano_kline7_crp) = va_x6;
           break;
         case 5:
         NV(piano_kline7_cdur) = va_dur6;
           NV(piano_kline7_clp) = NV(piano_kline7_crp);
           NV(piano_kline7_crp) = va_x7;
           break;
         case 6:
         NV(piano_kline7_cdur) = va_dur7;
           NV(piano_kline7_clp) = NV(piano_kline7_crp);
           NV(piano_kline7_crp) = va_x8;
           break;
         case 7:
         NV(piano_kline7_cdur) = va_dur8;
           NV(piano_kline7_clp) = NV(piano_kline7_crp);
           NV(piano_kline7_crp) = va_x9;
           break;
         case 8:
         NV(piano_kline7_cdur) = va_dur9;
           NV(piano_kline7_clp) = NV(piano_kline7_crp);
           NV(piano_kline7_crp) = va_x10;
           break;
         case 9:
         NV(piano_kline7_cdur) = va_dur10;
           NV(piano_kline7_clp) = NV(piano_kline7_crp);
           NV(piano_kline7_crp) = va_x11;
           break;
         case 10:
         NV(piano_kline7_cdur) = va_dur11;
           NV(piano_kline7_clp) = NV(piano_kline7_crp);
           NV(piano_kline7_crp) = va_x12;
           break;
         case 11:
         NV(piano_kline7_cdur) = va_dur12;
           NV(piano_kline7_clp) = NV(piano_kline7_crp);
           NV(piano_kline7_crp) = va_x13;
           break;
         case 12:
         NV(piano_kline7_cdur) = va_dur13;
           NV(piano_kline7_clp) = NV(piano_kline7_crp);
           NV(piano_kline7_crp) = va_x14;
           break;
         case 13:
         NV(piano_kline7_cdur) = va_dur14;
           NV(piano_kline7_clp) = NV(piano_kline7_crp);
           NV(piano_kline7_crp) = va_x15;
           break;
         case 14:
         NV(piano_kline7_cdur) = va_dur15;
           NV(piano_kline7_clp) = NV(piano_kline7_crp);
           NV(piano_kline7_crp) = va_x16;
           break;
         case 15:
         NV(piano_kline7_cdur) = va_dur16;
           NV(piano_kline7_clp) = NV(piano_kline7_crp);
           NV(piano_kline7_crp) = va_x17;
           break;
         case 16:
         NV(piano_kline7_cdur) = va_dur17;
           NV(piano_kline7_clp) = NV(piano_kline7_crp);
           NV(piano_kline7_crp) = va_x18;
           break;
         case 17:
         NV(piano_kline7_cdur) = va_dur18;
           NV(piano_kline7_clp) = NV(piano_kline7_crp);
           NV(piano_kline7_crp) = va_x19;
           break;
         case 18:
         NV(piano_kline7_cdur) = va_dur19;
           NV(piano_kline7_clp) = NV(piano_kline7_crp);
           NV(piano_kline7_crp) = va_x20;
           break;
         case 19:
         NV(piano_kline7_cdur) = va_dur20;
           NV(piano_kline7_clp) = NV(piano_kline7_crp);
           NV(piano_kline7_crp) = va_x21;
           break;
           default:
           NVI(piano_kline7_first) = -100;
           NV(piano_kline7_cdur) = NV(piano_kline7_t) + 10000.0F;
           break;
           }
         NVI(piano_kline7_first)++;
        }
        NV(piano_kline7_mult)=(NV(piano_kline7_crp) - NV(piano_kline7_clp))/NV(piano_kline7_cdur);
        ret = NV(piano_kline7_outT) = NV(piano_kline7_clp)+NV(piano_kline7_mult)*NV(piano_kline7_t);
        NV(piano_kline7_addK) = NV(piano_kline7_mult)*EV(KTIME);
        if (NVI(piano_kline7_first)<0)
          ret = 0.0F;
      }
   }
   if (NVI(piano_kline7_first)==0)
     {
       NVI(piano_kline7_first) = 1;
       ret = NV(piano_kline7_outT) = NV(piano_kline7_clp) = x1;
       NV(piano_kline7_crp) = x2;
       NV(piano_kline7_cdur) = dur1;
       if (dur1 > 0.0F)
         NV(piano_kline7_addK) = EV(KTIME)*((x2 - x1)/dur1);
     }
   return((NV(piano_kline7_return) = ret));

}



float piano__sym_kline8(ENGINE_PTR_DECLARE_COMMA struct ninstr_types * nstate)
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
   float va_dur20;
   float va_x21;
   float x2;
   float dur1;
   float x1;
   float ret;

   x1 =  0.0F ;
   dur1 =  0.003215F ;
   x2 =  0.046764F ;
   va_dur2 =  0.001608F  ;
   va_x3 =  0.056532F  ;
   va_dur3 =  0.003215F  ;
   va_x4 =  0.062370F  ;
   va_dur4 =  0.009646F  ;
   va_x5 =  0.052692F  ;
   va_dur5 =  0.009646F  ;
   va_x6 =  0.050197F  ;
   va_dur6 =  0.008039F  ;
   va_x7 =  0.051058F  ;
   va_dur7 =  0.022508F  ;
   va_x8 =  0.037180F  ;
   va_dur8 =  0.011254F  ;
   va_x9 =  0.035406F  ;
   va_dur9 =  0.011254F  ;
   va_x10 =  0.029706F  ;
   va_dur10 =  0.011254F  ;
   va_x11 =  0.029972F  ;
   va_dur11 =  0.062701F  ;
   va_x12 =  0.011875F  ;
   va_dur12 =  0.065916F  ;
   va_x13 =  0.001461F  ;
   va_dur13 =  0.073955F  ;
   va_x14 =  0.007250F  ;
   va_dur14 =  0.083601F  ;
   va_x15 =  0.009777F  ;
   va_dur15 =  0.202572F  ;
   va_x16 =  0.008572F  ;
   va_dur16 =  0.191318F  ;
   va_x17 =  0.010924F  ;
   va_dur17 =  0.435691F  ;
   va_x18 =  0.001027F  ;
   va_dur18 =  0.252412F  ;
   va_x19 =  0.003685F  ;
   va_dur19 =  0.385852F  ;
   va_x20 =  0.000021F  ;
   va_dur20 =  0.646302F  ;
   va_x21 =  0.0F  ;
   ret = 0.0F;
   if (NVI(piano_kline8_first)>0)
   {
      NV(piano_kline8_t) += EV(KTIME);
      ret = (NV(piano_kline8_outT) += NV(piano_kline8_addK));
      if (NV(piano_kline8_t) > NV(piano_kline8_cdur))
       {
        while (NV(piano_kline8_t) > NV(piano_kline8_cdur))
         {
           NV(piano_kline8_t) -= NV(piano_kline8_cdur);
           switch(NVI(piano_kline8_first))
      {
         case 1:
         NV(piano_kline8_cdur) = va_dur2;
           NV(piano_kline8_clp) = NV(piano_kline8_crp);
           NV(piano_kline8_crp) = va_x3;
           break;
         case 2:
         NV(piano_kline8_cdur) = va_dur3;
           NV(piano_kline8_clp) = NV(piano_kline8_crp);
           NV(piano_kline8_crp) = va_x4;
           break;
         case 3:
         NV(piano_kline8_cdur) = va_dur4;
           NV(piano_kline8_clp) = NV(piano_kline8_crp);
           NV(piano_kline8_crp) = va_x5;
           break;
         case 4:
         NV(piano_kline8_cdur) = va_dur5;
           NV(piano_kline8_clp) = NV(piano_kline8_crp);
           NV(piano_kline8_crp) = va_x6;
           break;
         case 5:
         NV(piano_kline8_cdur) = va_dur6;
           NV(piano_kline8_clp) = NV(piano_kline8_crp);
           NV(piano_kline8_crp) = va_x7;
           break;
         case 6:
         NV(piano_kline8_cdur) = va_dur7;
           NV(piano_kline8_clp) = NV(piano_kline8_crp);
           NV(piano_kline8_crp) = va_x8;
           break;
         case 7:
         NV(piano_kline8_cdur) = va_dur8;
           NV(piano_kline8_clp) = NV(piano_kline8_crp);
           NV(piano_kline8_crp) = va_x9;
           break;
         case 8:
         NV(piano_kline8_cdur) = va_dur9;
           NV(piano_kline8_clp) = NV(piano_kline8_crp);
           NV(piano_kline8_crp) = va_x10;
           break;
         case 9:
         NV(piano_kline8_cdur) = va_dur10;
           NV(piano_kline8_clp) = NV(piano_kline8_crp);
           NV(piano_kline8_crp) = va_x11;
           break;
         case 10:
         NV(piano_kline8_cdur) = va_dur11;
           NV(piano_kline8_clp) = NV(piano_kline8_crp);
           NV(piano_kline8_crp) = va_x12;
           break;
         case 11:
         NV(piano_kline8_cdur) = va_dur12;
           NV(piano_kline8_clp) = NV(piano_kline8_crp);
           NV(piano_kline8_crp) = va_x13;
           break;
         case 12:
         NV(piano_kline8_cdur) = va_dur13;
           NV(piano_kline8_clp) = NV(piano_kline8_crp);
           NV(piano_kline8_crp) = va_x14;
           break;
         case 13:
         NV(piano_kline8_cdur) = va_dur14;
           NV(piano_kline8_clp) = NV(piano_kline8_crp);
           NV(piano_kline8_crp) = va_x15;
           break;
         case 14:
         NV(piano_kline8_cdur) = va_dur15;
           NV(piano_kline8_clp) = NV(piano_kline8_crp);
           NV(piano_kline8_crp) = va_x16;
           break;
         case 15:
         NV(piano_kline8_cdur) = va_dur16;
           NV(piano_kline8_clp) = NV(piano_kline8_crp);
           NV(piano_kline8_crp) = va_x17;
           break;
         case 16:
         NV(piano_kline8_cdur) = va_dur17;
           NV(piano_kline8_clp) = NV(piano_kline8_crp);
           NV(piano_kline8_crp) = va_x18;
           break;
         case 17:
         NV(piano_kline8_cdur) = va_dur18;
           NV(piano_kline8_clp) = NV(piano_kline8_crp);
           NV(piano_kline8_crp) = va_x19;
           break;
         case 18:
         NV(piano_kline8_cdur) = va_dur19;
           NV(piano_kline8_clp) = NV(piano_kline8_crp);
           NV(piano_kline8_crp) = va_x20;
           break;
         case 19:
         NV(piano_kline8_cdur) = va_dur20;
           NV(piano_kline8_clp) = NV(piano_kline8_crp);
           NV(piano_kline8_crp) = va_x21;
           break;
           default:
           NVI(piano_kline8_first) = -100;
           NV(piano_kline8_cdur) = NV(piano_kline8_t) + 10000.0F;
           break;
           }
         NVI(piano_kline8_first)++;
        }
        NV(piano_kline8_mult)=(NV(piano_kline8_crp) - NV(piano_kline8_clp))/NV(piano_kline8_cdur);
        ret = NV(piano_kline8_outT) = NV(piano_kline8_clp)+NV(piano_kline8_mult)*NV(piano_kline8_t);
        NV(piano_kline8_addK) = NV(piano_kline8_mult)*EV(KTIME);
        if (NVI(piano_kline8_first)<0)
          ret = 0.0F;
      }
   }
   if (NVI(piano_kline8_first)==0)
     {
       NVI(piano_kline8_first) = 1;
       ret = NV(piano_kline8_outT) = NV(piano_kline8_clp) = x1;
       NV(piano_kline8_crp) = x2;
       NV(piano_kline8_cdur) = dur1;
       if (dur1 > 0.0F)
         NV(piano_kline8_addK) = EV(KTIME)*((x2 - x1)/dur1);
     }
   return((NV(piano_kline8_return) = ret));

}



float piano__sym_kline9(ENGINE_PTR_DECLARE_COMMA struct ninstr_types * nstate)
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
   float va_dur20;
   float va_x21;
   float x2;
   float dur1;
   float x1;
   float ret;

   x1 =  0.0F ;
   dur1 =  0.008039F ;
   x2 =  0.004127F ;
   va_dur2 =  0.001608F  ;
   va_x3 =  0.007745F  ;
   va_dur3 =  0.004823F  ;
   va_x4 =  0.002790F  ;
   va_dur4 =  0.001608F  ;
   va_x5 =  0.006780F  ;
   va_dur5 =  0.001608F  ;
   va_x6 =  0.003239F  ;
   va_dur6 =  0.001608F  ;
   va_x7 =  0.006901F  ;
   va_dur7 =  0.001608F  ;
   va_x8 =  0.004103F  ;
   va_dur8 =  0.001608F  ;
   va_x9 =  0.007379F  ;
   va_dur9 =  0.001608F  ;
   va_x10 =  0.003306F  ;
   va_dur10 =  0.001608F  ;
   va_x11 =  0.005225F  ;
   va_dur11 =  0.001608F  ;
   va_x12 =  0.001935F  ;
   va_dur12 =  0.001608F  ;
   va_x13 =  0.004744F  ;
   va_dur13 =  0.004823F  ;
   va_x14 =  0.001782F  ;
   va_dur14 =  0.020900F  ;
   va_x15 =  0.003379F  ;
   va_dur15 =  0.012862F  ;
   va_x16 =  0.000967F  ;
   va_dur16 =  0.004823F  ;
   va_x17 =  0.003175F  ;
   va_dur17 =  0.022508F  ;
   va_x18 =  0.000332F  ;
   va_dur18 =  0.210611F  ;
   va_x19 =  0.002355F  ;
   va_dur19 =  0.461415F  ;
   va_x20 =  0.000033F  ;
   va_dur20 =  1.725080F  ;
   va_x21 =  0.0F  ;
   ret = 0.0F;
   if (NVI(piano_kline9_first)>0)
   {
      NV(piano_kline9_t) += EV(KTIME);
      ret = (NV(piano_kline9_outT) += NV(piano_kline9_addK));
      if (NV(piano_kline9_t) > NV(piano_kline9_cdur))
       {
        while (NV(piano_kline9_t) > NV(piano_kline9_cdur))
         {
           NV(piano_kline9_t) -= NV(piano_kline9_cdur);
           switch(NVI(piano_kline9_first))
      {
         case 1:
         NV(piano_kline9_cdur) = va_dur2;
           NV(piano_kline9_clp) = NV(piano_kline9_crp);
           NV(piano_kline9_crp) = va_x3;
           break;
         case 2:
         NV(piano_kline9_cdur) = va_dur3;
           NV(piano_kline9_clp) = NV(piano_kline9_crp);
           NV(piano_kline9_crp) = va_x4;
           break;
         case 3:
         NV(piano_kline9_cdur) = va_dur4;
           NV(piano_kline9_clp) = NV(piano_kline9_crp);
           NV(piano_kline9_crp) = va_x5;
           break;
         case 4:
         NV(piano_kline9_cdur) = va_dur5;
           NV(piano_kline9_clp) = NV(piano_kline9_crp);
           NV(piano_kline9_crp) = va_x6;
           break;
         case 5:
         NV(piano_kline9_cdur) = va_dur6;
           NV(piano_kline9_clp) = NV(piano_kline9_crp);
           NV(piano_kline9_crp) = va_x7;
           break;
         case 6:
         NV(piano_kline9_cdur) = va_dur7;
           NV(piano_kline9_clp) = NV(piano_kline9_crp);
           NV(piano_kline9_crp) = va_x8;
           break;
         case 7:
         NV(piano_kline9_cdur) = va_dur8;
           NV(piano_kline9_clp) = NV(piano_kline9_crp);
           NV(piano_kline9_crp) = va_x9;
           break;
         case 8:
         NV(piano_kline9_cdur) = va_dur9;
           NV(piano_kline9_clp) = NV(piano_kline9_crp);
           NV(piano_kline9_crp) = va_x10;
           break;
         case 9:
         NV(piano_kline9_cdur) = va_dur10;
           NV(piano_kline9_clp) = NV(piano_kline9_crp);
           NV(piano_kline9_crp) = va_x11;
           break;
         case 10:
         NV(piano_kline9_cdur) = va_dur11;
           NV(piano_kline9_clp) = NV(piano_kline9_crp);
           NV(piano_kline9_crp) = va_x12;
           break;
         case 11:
         NV(piano_kline9_cdur) = va_dur12;
           NV(piano_kline9_clp) = NV(piano_kline9_crp);
           NV(piano_kline9_crp) = va_x13;
           break;
         case 12:
         NV(piano_kline9_cdur) = va_dur13;
           NV(piano_kline9_clp) = NV(piano_kline9_crp);
           NV(piano_kline9_crp) = va_x14;
           break;
         case 13:
         NV(piano_kline9_cdur) = va_dur14;
           NV(piano_kline9_clp) = NV(piano_kline9_crp);
           NV(piano_kline9_crp) = va_x15;
           break;
         case 14:
         NV(piano_kline9_cdur) = va_dur15;
           NV(piano_kline9_clp) = NV(piano_kline9_crp);
           NV(piano_kline9_crp) = va_x16;
           break;
         case 15:
         NV(piano_kline9_cdur) = va_dur16;
           NV(piano_kline9_clp) = NV(piano_kline9_crp);
           NV(piano_kline9_crp) = va_x17;
           break;
         case 16:
         NV(piano_kline9_cdur) = va_dur17;
           NV(piano_kline9_clp) = NV(piano_kline9_crp);
           NV(piano_kline9_crp) = va_x18;
           break;
         case 17:
         NV(piano_kline9_cdur) = va_dur18;
           NV(piano_kline9_clp) = NV(piano_kline9_crp);
           NV(piano_kline9_crp) = va_x19;
           break;
         case 18:
         NV(piano_kline9_cdur) = va_dur19;
           NV(piano_kline9_clp) = NV(piano_kline9_crp);
           NV(piano_kline9_crp) = va_x20;
           break;
         case 19:
         NV(piano_kline9_cdur) = va_dur20;
           NV(piano_kline9_clp) = NV(piano_kline9_crp);
           NV(piano_kline9_crp) = va_x21;
           break;
           default:
           NVI(piano_kline9_first) = -100;
           NV(piano_kline9_cdur) = NV(piano_kline9_t) + 10000.0F;
           break;
           }
         NVI(piano_kline9_first)++;
        }
        NV(piano_kline9_mult)=(NV(piano_kline9_crp) - NV(piano_kline9_clp))/NV(piano_kline9_cdur);
        ret = NV(piano_kline9_outT) = NV(piano_kline9_clp)+NV(piano_kline9_mult)*NV(piano_kline9_t);
        NV(piano_kline9_addK) = NV(piano_kline9_mult)*EV(KTIME);
        if (NVI(piano_kline9_first)<0)
          ret = 0.0F;
      }
   }
   if (NVI(piano_kline9_first)==0)
     {
       NVI(piano_kline9_first) = 1;
       ret = NV(piano_kline9_outT) = NV(piano_kline9_clp) = x1;
       NV(piano_kline9_crp) = x2;
       NV(piano_kline9_cdur) = dur1;
       if (dur1 > 0.0F)
         NV(piano_kline9_addK) = EV(KTIME)*((x2 - x1)/dur1);
     }
   return((NV(piano_kline9_return) = ret));

}



float piano__sym_kline10(ENGINE_PTR_DECLARE_COMMA struct ninstr_types * nstate)
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
   float va_dur20;
   float va_x21;
   float x2;
   float dur1;
   float x1;
   float ret;

   x1 =  0.0F ;
   dur1 =  0.001608F ;
   x2 =  0.003914F ;
   va_dur2 =  0.004823F  ;
   va_x3 =  0.008239F  ;
   va_dur3 =  0.001608F  ;
   va_x4 =  0.006549F  ;
   va_dur4 =  0.003215F  ;
   va_x5 =  0.005450F  ;
   va_dur5 =  0.011254F  ;
   va_x6 =  0.002857F  ;
   va_dur6 =  0.008039F  ;
   va_x7 =  0.003216F  ;
   va_dur7 =  0.004823F  ;
   va_x8 =  0.002220F  ;
   va_dur8 =  0.012862F  ;
   va_x9 =  0.004226F  ;
   va_dur9 =  0.012862F  ;
   va_x10 =  0.001520F  ;
   va_dur10 =  0.014469F  ;
   va_x11 =  0.003314F  ;
   va_dur11 =  0.014469F  ;
   va_x12 =  0.001304F  ;
   va_dur12 =  0.009646F  ;
   va_x13 =  0.002365F  ;
   va_dur13 =  0.009646F  ;
   va_x14 =  0.001265F  ;
   va_dur14 =  0.028939F  ;
   va_x15 =  0.001750F  ;
   va_dur15 =  0.009646F  ;
   va_x16 =  0.000774F  ;
   va_dur16 =  0.008039F  ;
   va_x17 =  0.001624F  ;
   va_dur17 =  0.011254F  ;
   va_x18 =  0.000869F  ;
   va_dur18 =  0.118971F  ;
   va_x19 =  0.002248F  ;
   va_dur19 =  0.479100F  ;
   va_x20 =  0.000163F  ;
   va_dur20 =  1.726688F  ;
   va_x21 =  0.0F  ;
   ret = 0.0F;
   if (NVI(piano_kline10_first)>0)
   {
      NV(piano_kline10_t) += EV(KTIME);
      ret = (NV(piano_kline10_outT) += NV(piano_kline10_addK));
      if (NV(piano_kline10_t) > NV(piano_kline10_cdur))
       {
        while (NV(piano_kline10_t) > NV(piano_kline10_cdur))
         {
           NV(piano_kline10_t) -= NV(piano_kline10_cdur);
           switch(NVI(piano_kline10_first))
      {
         case 1:
         NV(piano_kline10_cdur) = va_dur2;
           NV(piano_kline10_clp) = NV(piano_kline10_crp);
           NV(piano_kline10_crp) = va_x3;
           break;
         case 2:
         NV(piano_kline10_cdur) = va_dur3;
           NV(piano_kline10_clp) = NV(piano_kline10_crp);
           NV(piano_kline10_crp) = va_x4;
           break;
         case 3:
         NV(piano_kline10_cdur) = va_dur4;
           NV(piano_kline10_clp) = NV(piano_kline10_crp);
           NV(piano_kline10_crp) = va_x5;
           break;
         case 4:
         NV(piano_kline10_cdur) = va_dur5;
           NV(piano_kline10_clp) = NV(piano_kline10_crp);
           NV(piano_kline10_crp) = va_x6;
           break;
         case 5:
         NV(piano_kline10_cdur) = va_dur6;
           NV(piano_kline10_clp) = NV(piano_kline10_crp);
           NV(piano_kline10_crp) = va_x7;
           break;
         case 6:
         NV(piano_kline10_cdur) = va_dur7;
           NV(piano_kline10_clp) = NV(piano_kline10_crp);
           NV(piano_kline10_crp) = va_x8;
           break;
         case 7:
         NV(piano_kline10_cdur) = va_dur8;
           NV(piano_kline10_clp) = NV(piano_kline10_crp);
           NV(piano_kline10_crp) = va_x9;
           break;
         case 8:
         NV(piano_kline10_cdur) = va_dur9;
           NV(piano_kline10_clp) = NV(piano_kline10_crp);
           NV(piano_kline10_crp) = va_x10;
           break;
         case 9:
         NV(piano_kline10_cdur) = va_dur10;
           NV(piano_kline10_clp) = NV(piano_kline10_crp);
           NV(piano_kline10_crp) = va_x11;
           break;
         case 10:
         NV(piano_kline10_cdur) = va_dur11;
           NV(piano_kline10_clp) = NV(piano_kline10_crp);
           NV(piano_kline10_crp) = va_x12;
           break;
         case 11:
         NV(piano_kline10_cdur) = va_dur12;
           NV(piano_kline10_clp) = NV(piano_kline10_crp);
           NV(piano_kline10_crp) = va_x13;
           break;
         case 12:
         NV(piano_kline10_cdur) = va_dur13;
           NV(piano_kline10_clp) = NV(piano_kline10_crp);
           NV(piano_kline10_crp) = va_x14;
           break;
         case 13:
         NV(piano_kline10_cdur) = va_dur14;
           NV(piano_kline10_clp) = NV(piano_kline10_crp);
           NV(piano_kline10_crp) = va_x15;
           break;
         case 14:
         NV(piano_kline10_cdur) = va_dur15;
           NV(piano_kline10_clp) = NV(piano_kline10_crp);
           NV(piano_kline10_crp) = va_x16;
           break;
         case 15:
         NV(piano_kline10_cdur) = va_dur16;
           NV(piano_kline10_clp) = NV(piano_kline10_crp);
           NV(piano_kline10_crp) = va_x17;
           break;
         case 16:
         NV(piano_kline10_cdur) = va_dur17;
           NV(piano_kline10_clp) = NV(piano_kline10_crp);
           NV(piano_kline10_crp) = va_x18;
           break;
         case 17:
         NV(piano_kline10_cdur) = va_dur18;
           NV(piano_kline10_clp) = NV(piano_kline10_crp);
           NV(piano_kline10_crp) = va_x19;
           break;
         case 18:
         NV(piano_kline10_cdur) = va_dur19;
           NV(piano_kline10_clp) = NV(piano_kline10_crp);
           NV(piano_kline10_crp) = va_x20;
           break;
         case 19:
         NV(piano_kline10_cdur) = va_dur20;
           NV(piano_kline10_clp) = NV(piano_kline10_crp);
           NV(piano_kline10_crp) = va_x21;
           break;
           default:
           NVI(piano_kline10_first) = -100;
           NV(piano_kline10_cdur) = NV(piano_kline10_t) + 10000.0F;
           break;
           }
         NVI(piano_kline10_first)++;
        }
        NV(piano_kline10_mult)=(NV(piano_kline10_crp) - NV(piano_kline10_clp))/NV(piano_kline10_cdur);
        ret = NV(piano_kline10_outT) = NV(piano_kline10_clp)+NV(piano_kline10_mult)*NV(piano_kline10_t);
        NV(piano_kline10_addK) = NV(piano_kline10_mult)*EV(KTIME);
        if (NVI(piano_kline10_first)<0)
          ret = 0.0F;
      }
   }
   if (NVI(piano_kline10_first)==0)
     {
       NVI(piano_kline10_first) = 1;
       ret = NV(piano_kline10_outT) = NV(piano_kline10_clp) = x1;
       NV(piano_kline10_crp) = x2;
       NV(piano_kline10_cdur) = dur1;
       if (dur1 > 0.0F)
         NV(piano_kline10_addK) = EV(KTIME)*((x2 - x1)/dur1);
     }
   return((NV(piano_kline10_return) = ret));

}



float piano__sym_kline11(ENGINE_PTR_DECLARE_COMMA struct ninstr_types * nstate)
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
   float va_dur20;
   float va_x21;
   float x2;
   float dur1;
   float x1;
   float ret;

   x1 =  0.0F ;
   dur1 =  0.003215F ;
   x2 =  0.016506F ;
   va_dur2 =  0.001608F  ;
   va_x3 =  0.017322F  ;
   va_dur3 =  0.006431F  ;
   va_x4 =  0.012493F  ;
   va_dur4 =  0.003215F  ;
   va_x5 =  0.012187F  ;
   va_dur5 =  0.004823F  ;
   va_x6 =  0.015584F  ;
   va_dur6 =  0.004823F  ;
   va_x7 =  0.010866F  ;
   va_dur7 =  0.003215F  ;
   va_x8 =  0.010155F  ;
   va_dur8 =  0.006431F  ;
   va_x9 =  0.011660F  ;
   va_dur9 =  0.008039F  ;
   va_x10 =  0.008720F  ;
   va_dur10 =  0.069132F  ;
   va_x11 =  0.002613F  ;
   va_dur11 =  0.041801F  ;
   va_x12 =  0.000894F  ;
   va_dur12 =  0.086817F  ;
   va_x13 =  0.004317F  ;
   va_dur13 =  0.006431F  ;
   va_x14 =  0.002992F  ;
   va_dur14 =  0.056270F  ;
   va_x15 =  0.004118F  ;
   va_dur15 =  0.122186F  ;
   va_x16 =  0.001368F  ;
   va_dur16 =  0.160772F  ;
   va_x17 =  0.001791F  ;
   va_dur17 =  0.085209F  ;
   va_x18 =  0.000122F  ;
   va_dur18 =  0.186495F  ;
   va_x19 =  0.002486F  ;
   va_dur19 =  0.303859F  ;
   va_x20 =  0.000278F  ;
   va_dur20 =  1.331190F  ;
   va_x21 =  0.0F  ;
   ret = 0.0F;
   if (NVI(piano_kline11_first)>0)
   {
      NV(piano_kline11_t) += EV(KTIME);
      ret = (NV(piano_kline11_outT) += NV(piano_kline11_addK));
      if (NV(piano_kline11_t) > NV(piano_kline11_cdur))
       {
        while (NV(piano_kline11_t) > NV(piano_kline11_cdur))
         {
           NV(piano_kline11_t) -= NV(piano_kline11_cdur);
           switch(NVI(piano_kline11_first))
      {
         case 1:
         NV(piano_kline11_cdur) = va_dur2;
           NV(piano_kline11_clp) = NV(piano_kline11_crp);
           NV(piano_kline11_crp) = va_x3;
           break;
         case 2:
         NV(piano_kline11_cdur) = va_dur3;
           NV(piano_kline11_clp) = NV(piano_kline11_crp);
           NV(piano_kline11_crp) = va_x4;
           break;
         case 3:
         NV(piano_kline11_cdur) = va_dur4;
           NV(piano_kline11_clp) = NV(piano_kline11_crp);
           NV(piano_kline11_crp) = va_x5;
           break;
         case 4:
         NV(piano_kline11_cdur) = va_dur5;
           NV(piano_kline11_clp) = NV(piano_kline11_crp);
           NV(piano_kline11_crp) = va_x6;
           break;
         case 5:
         NV(piano_kline11_cdur) = va_dur6;
           NV(piano_kline11_clp) = NV(piano_kline11_crp);
           NV(piano_kline11_crp) = va_x7;
           break;
         case 6:
         NV(piano_kline11_cdur) = va_dur7;
           NV(piano_kline11_clp) = NV(piano_kline11_crp);
           NV(piano_kline11_crp) = va_x8;
           break;
         case 7:
         NV(piano_kline11_cdur) = va_dur8;
           NV(piano_kline11_clp) = NV(piano_kline11_crp);
           NV(piano_kline11_crp) = va_x9;
           break;
         case 8:
         NV(piano_kline11_cdur) = va_dur9;
           NV(piano_kline11_clp) = NV(piano_kline11_crp);
           NV(piano_kline11_crp) = va_x10;
           break;
         case 9:
         NV(piano_kline11_cdur) = va_dur10;
           NV(piano_kline11_clp) = NV(piano_kline11_crp);
           NV(piano_kline11_crp) = va_x11;
           break;
         case 10:
         NV(piano_kline11_cdur) = va_dur11;
           NV(piano_kline11_clp) = NV(piano_kline11_crp);
           NV(piano_kline11_crp) = va_x12;
           break;
         case 11:
         NV(piano_kline11_cdur) = va_dur12;
           NV(piano_kline11_clp) = NV(piano_kline11_crp);
           NV(piano_kline11_crp) = va_x13;
           break;
         case 12:
         NV(piano_kline11_cdur) = va_dur13;
           NV(piano_kline11_clp) = NV(piano_kline11_crp);
           NV(piano_kline11_crp) = va_x14;
           break;
         case 13:
         NV(piano_kline11_cdur) = va_dur14;
           NV(piano_kline11_clp) = NV(piano_kline11_crp);
           NV(piano_kline11_crp) = va_x15;
           break;
         case 14:
         NV(piano_kline11_cdur) = va_dur15;
           NV(piano_kline11_clp) = NV(piano_kline11_crp);
           NV(piano_kline11_crp) = va_x16;
           break;
         case 15:
         NV(piano_kline11_cdur) = va_dur16;
           NV(piano_kline11_clp) = NV(piano_kline11_crp);
           NV(piano_kline11_crp) = va_x17;
           break;
         case 16:
         NV(piano_kline11_cdur) = va_dur17;
           NV(piano_kline11_clp) = NV(piano_kline11_crp);
           NV(piano_kline11_crp) = va_x18;
           break;
         case 17:
         NV(piano_kline11_cdur) = va_dur18;
           NV(piano_kline11_clp) = NV(piano_kline11_crp);
           NV(piano_kline11_crp) = va_x19;
           break;
         case 18:
         NV(piano_kline11_cdur) = va_dur19;
           NV(piano_kline11_clp) = NV(piano_kline11_crp);
           NV(piano_kline11_crp) = va_x20;
           break;
         case 19:
         NV(piano_kline11_cdur) = va_dur20;
           NV(piano_kline11_clp) = NV(piano_kline11_crp);
           NV(piano_kline11_crp) = va_x21;
           break;
           default:
           NVI(piano_kline11_first) = -100;
           NV(piano_kline11_cdur) = NV(piano_kline11_t) + 10000.0F;
           break;
           }
         NVI(piano_kline11_first)++;
        }
        NV(piano_kline11_mult)=(NV(piano_kline11_crp) - NV(piano_kline11_clp))/NV(piano_kline11_cdur);
        ret = NV(piano_kline11_outT) = NV(piano_kline11_clp)+NV(piano_kline11_mult)*NV(piano_kline11_t);
        NV(piano_kline11_addK) = NV(piano_kline11_mult)*EV(KTIME);
        if (NVI(piano_kline11_first)<0)
          ret = 0.0F;
      }
   }
   if (NVI(piano_kline11_first)==0)
     {
       NVI(piano_kline11_first) = 1;
       ret = NV(piano_kline11_outT) = NV(piano_kline11_clp) = x1;
       NV(piano_kline11_crp) = x2;
       NV(piano_kline11_cdur) = dur1;
       if (dur1 > 0.0F)
         NV(piano_kline11_addK) = EV(KTIME)*((x2 - x1)/dur1);
     }
   return((NV(piano_kline11_return) = ret));

}



float piano__sym_kline12(ENGINE_PTR_DECLARE_COMMA struct ninstr_types * nstate)
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
   float va_dur20;
   float va_x21;
   float x2;
   float dur1;
   float x1;
   float ret;

   x1 =  0.0F ;
   dur1 =  0.004823F ;
   x2 =  0.007332F ;
   va_dur2 =  0.009646F  ;
   va_x3 =  0.002024F  ;
   va_dur3 =  0.001608F  ;
   va_x4 =  0.005050F  ;
   va_dur4 =  0.001608F  ;
   va_x5 =  0.001119F  ;
   va_dur5 =  0.001608F  ;
   va_x6 =  0.006028F  ;
   va_dur6 =  0.001608F  ;
   va_x7 =  0.001035F  ;
   va_dur7 =  0.001608F  ;
   va_x8 =  0.004930F  ;
   va_dur8 =  0.001608F  ;
   va_x9 =  0.001264F  ;
   va_dur9 =  0.001608F  ;
   va_x10 =  0.004307F  ;
   va_dur10 =  0.009646F  ;
   va_x11 =  0.002126F  ;
   va_dur11 =  0.001608F  ;
   va_x12 =  0.004609F  ;
   va_dur12 =  0.001608F  ;
   va_x13 =  0.001601F  ;
   va_dur13 =  0.001608F  ;
   va_x14 =  0.005054F  ;
   va_dur14 =  0.001608F  ;
   va_x15 =  0.001540F  ;
   va_dur15 =  0.001608F  ;
   va_x16 =  0.004153F  ;
   va_dur16 =  0.017685F  ;
   va_x17 =  0.003993F  ;
   va_dur17 =  0.001608F  ;
   va_x18 =  0.000891F  ;
   va_dur18 =  0.017685F  ;
   va_x19 =  0.000154F  ;
   va_dur19 =  0.271704F  ;
   va_x20 =  0.002710F  ;
   va_dur20 =  2.139871F  ;
   va_x21 =  0.0F  ;
   ret = 0.0F;
   if (NVI(piano_kline12_first)>0)
   {
      NV(piano_kline12_t) += EV(KTIME);
      ret = (NV(piano_kline12_outT) += NV(piano_kline12_addK));
      if (NV(piano_kline12_t) > NV(piano_kline12_cdur))
       {
        while (NV(piano_kline12_t) > NV(piano_kline12_cdur))
         {
           NV(piano_kline12_t) -= NV(piano_kline12_cdur);
           switch(NVI(piano_kline12_first))
      {
         case 1:
         NV(piano_kline12_cdur) = va_dur2;
           NV(piano_kline12_clp) = NV(piano_kline12_crp);
           NV(piano_kline12_crp) = va_x3;
           break;
         case 2:
         NV(piano_kline12_cdur) = va_dur3;
           NV(piano_kline12_clp) = NV(piano_kline12_crp);
           NV(piano_kline12_crp) = va_x4;
           break;
         case 3:
         NV(piano_kline12_cdur) = va_dur4;
           NV(piano_kline12_clp) = NV(piano_kline12_crp);
           NV(piano_kline12_crp) = va_x5;
           break;
         case 4:
         NV(piano_kline12_cdur) = va_dur5;
           NV(piano_kline12_clp) = NV(piano_kline12_crp);
           NV(piano_kline12_crp) = va_x6;
           break;
         case 5:
         NV(piano_kline12_cdur) = va_dur6;
           NV(piano_kline12_clp) = NV(piano_kline12_crp);
           NV(piano_kline12_crp) = va_x7;
           break;
         case 6:
         NV(piano_kline12_cdur) = va_dur7;
           NV(piano_kline12_clp) = NV(piano_kline12_crp);
           NV(piano_kline12_crp) = va_x8;
           break;
         case 7:
         NV(piano_kline12_cdur) = va_dur8;
           NV(piano_kline12_clp) = NV(piano_kline12_crp);
           NV(piano_kline12_crp) = va_x9;
           break;
         case 8:
         NV(piano_kline12_cdur) = va_dur9;
           NV(piano_kline12_clp) = NV(piano_kline12_crp);
           NV(piano_kline12_crp) = va_x10;
           break;
         case 9:
         NV(piano_kline12_cdur) = va_dur10;
           NV(piano_kline12_clp) = NV(piano_kline12_crp);
           NV(piano_kline12_crp) = va_x11;
           break;
         case 10:
         NV(piano_kline12_cdur) = va_dur11;
           NV(piano_kline12_clp) = NV(piano_kline12_crp);
           NV(piano_kline12_crp) = va_x12;
           break;
         case 11:
         NV(piano_kline12_cdur) = va_dur12;
           NV(piano_kline12_clp) = NV(piano_kline12_crp);
           NV(piano_kline12_crp) = va_x13;
           break;
         case 12:
         NV(piano_kline12_cdur) = va_dur13;
           NV(piano_kline12_clp) = NV(piano_kline12_crp);
           NV(piano_kline12_crp) = va_x14;
           break;
         case 13:
         NV(piano_kline12_cdur) = va_dur14;
           NV(piano_kline12_clp) = NV(piano_kline12_crp);
           NV(piano_kline12_crp) = va_x15;
           break;
         case 14:
         NV(piano_kline12_cdur) = va_dur15;
           NV(piano_kline12_clp) = NV(piano_kline12_crp);
           NV(piano_kline12_crp) = va_x16;
           break;
         case 15:
         NV(piano_kline12_cdur) = va_dur16;
           NV(piano_kline12_clp) = NV(piano_kline12_crp);
           NV(piano_kline12_crp) = va_x17;
           break;
         case 16:
         NV(piano_kline12_cdur) = va_dur17;
           NV(piano_kline12_clp) = NV(piano_kline12_crp);
           NV(piano_kline12_crp) = va_x18;
           break;
         case 17:
         NV(piano_kline12_cdur) = va_dur18;
           NV(piano_kline12_clp) = NV(piano_kline12_crp);
           NV(piano_kline12_crp) = va_x19;
           break;
         case 18:
         NV(piano_kline12_cdur) = va_dur19;
           NV(piano_kline12_clp) = NV(piano_kline12_crp);
           NV(piano_kline12_crp) = va_x20;
           break;
         case 19:
         NV(piano_kline12_cdur) = va_dur20;
           NV(piano_kline12_clp) = NV(piano_kline12_crp);
           NV(piano_kline12_crp) = va_x21;
           break;
           default:
           NVI(piano_kline12_first) = -100;
           NV(piano_kline12_cdur) = NV(piano_kline12_t) + 10000.0F;
           break;
           }
         NVI(piano_kline12_first)++;
        }
        NV(piano_kline12_mult)=(NV(piano_kline12_crp) - NV(piano_kline12_clp))/NV(piano_kline12_cdur);
        ret = NV(piano_kline12_outT) = NV(piano_kline12_clp)+NV(piano_kline12_mult)*NV(piano_kline12_t);
        NV(piano_kline12_addK) = NV(piano_kline12_mult)*EV(KTIME);
        if (NVI(piano_kline12_first)<0)
          ret = 0.0F;
      }
   }
   if (NVI(piano_kline12_first)==0)
     {
       NVI(piano_kline12_first) = 1;
       ret = NV(piano_kline12_outT) = NV(piano_kline12_clp) = x1;
       NV(piano_kline12_crp) = x2;
       NV(piano_kline12_cdur) = dur1;
       if (dur1 > 0.0F)
         NV(piano_kline12_addK) = EV(KTIME)*((x2 - x1)/dur1);
     }
   return((NV(piano_kline12_return) = ret));

}



float piano__sym_kline13(ENGINE_PTR_DECLARE_COMMA struct ninstr_types * nstate)
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
   float va_dur20;
   float va_x21;
   float x2;
   float dur1;
   float x1;
   float ret;

   x1 =  0.0F ;
   dur1 =  0.001608F ;
   x2 =  0.003971F ;
   va_dur2 =  0.004823F  ;
   va_x3 =  0.001150F  ;
   va_dur3 =  0.001608F  ;
   va_x4 =  0.002629F  ;
   va_dur4 =  0.001608F  ;
   va_x5 =  0.000343F  ;
   va_dur5 =  0.004823F  ;
   va_x6 =  0.004946F  ;
   va_dur6 =  0.004823F  ;
   va_x7 =  0.001225F  ;
   va_dur7 =  0.003215F  ;
   va_x8 =  0.004100F  ;
   va_dur8 =  0.004823F  ;
   va_x9 =  0.000788F  ;
   va_dur9 =  0.001608F  ;
   va_x10 =  0.003162F  ;
   va_dur10 =  0.003215F  ;
   va_x11 =  0.003146F  ;
   va_dur11 =  0.001608F  ;
   va_x12 =  0.000529F  ;
   va_dur12 =  0.012862F  ;
   va_x13 =  0.002922F  ;
   va_dur13 =  0.001608F  ;
   va_x14 =  0.000871F  ;
   va_dur14 =  0.016077F  ;
   va_x15 =  0.002607F  ;
   va_dur15 =  0.035370F  ;
   va_x16 =  0.000228F  ;
   va_dur16 =  0.212219F  ;
   va_x17 =  0.001903F  ;
   va_dur17 =  0.006431F  ;
   va_x18 =  0.000261F  ;
   va_dur18 =  0.006431F  ;
   va_x19 =  0.001763F  ;
   va_dur19 =  0.072347F  ;
   va_x20 =  0.000111F  ;
   va_dur20 =  2.094855F  ;
   va_x21 =  0.0F  ;
   ret = 0.0F;
   if (NVI(piano_kline13_first)>0)
   {
      NV(piano_kline13_t) += EV(KTIME);
      ret = (NV(piano_kline13_outT) += NV(piano_kline13_addK));
      if (NV(piano_kline13_t) > NV(piano_kline13_cdur))
       {
        while (NV(piano_kline13_t) > NV(piano_kline13_cdur))
         {
           NV(piano_kline13_t) -= NV(piano_kline13_cdur);
           switch(NVI(piano_kline13_first))
      {
         case 1:
         NV(piano_kline13_cdur) = va_dur2;
           NV(piano_kline13_clp) = NV(piano_kline13_crp);
           NV(piano_kline13_crp) = va_x3;
           break;
         case 2:
         NV(piano_kline13_cdur) = va_dur3;
           NV(piano_kline13_clp) = NV(piano_kline13_crp);
           NV(piano_kline13_crp) = va_x4;
           break;
         case 3:
         NV(piano_kline13_cdur) = va_dur4;
           NV(piano_kline13_clp) = NV(piano_kline13_crp);
           NV(piano_kline13_crp) = va_x5;
           break;
         case 4:
         NV(piano_kline13_cdur) = va_dur5;
           NV(piano_kline13_clp) = NV(piano_kline13_crp);
           NV(piano_kline13_crp) = va_x6;
           break;
         case 5:
         NV(piano_kline13_cdur) = va_dur6;
           NV(piano_kline13_clp) = NV(piano_kline13_crp);
           NV(piano_kline13_crp) = va_x7;
           break;
         case 6:
         NV(piano_kline13_cdur) = va_dur7;
           NV(piano_kline13_clp) = NV(piano_kline13_crp);
           NV(piano_kline13_crp) = va_x8;
           break;
         case 7:
         NV(piano_kline13_cdur) = va_dur8;
           NV(piano_kline13_clp) = NV(piano_kline13_crp);
           NV(piano_kline13_crp) = va_x9;
           break;
         case 8:
         NV(piano_kline13_cdur) = va_dur9;
           NV(piano_kline13_clp) = NV(piano_kline13_crp);
           NV(piano_kline13_crp) = va_x10;
           break;
         case 9:
         NV(piano_kline13_cdur) = va_dur10;
           NV(piano_kline13_clp) = NV(piano_kline13_crp);
           NV(piano_kline13_crp) = va_x11;
           break;
         case 10:
         NV(piano_kline13_cdur) = va_dur11;
           NV(piano_kline13_clp) = NV(piano_kline13_crp);
           NV(piano_kline13_crp) = va_x12;
           break;
         case 11:
         NV(piano_kline13_cdur) = va_dur12;
           NV(piano_kline13_clp) = NV(piano_kline13_crp);
           NV(piano_kline13_crp) = va_x13;
           break;
         case 12:
         NV(piano_kline13_cdur) = va_dur13;
           NV(piano_kline13_clp) = NV(piano_kline13_crp);
           NV(piano_kline13_crp) = va_x14;
           break;
         case 13:
         NV(piano_kline13_cdur) = va_dur14;
           NV(piano_kline13_clp) = NV(piano_kline13_crp);
           NV(piano_kline13_crp) = va_x15;
           break;
         case 14:
         NV(piano_kline13_cdur) = va_dur15;
           NV(piano_kline13_clp) = NV(piano_kline13_crp);
           NV(piano_kline13_crp) = va_x16;
           break;
         case 15:
         NV(piano_kline13_cdur) = va_dur16;
           NV(piano_kline13_clp) = NV(piano_kline13_crp);
           NV(piano_kline13_crp) = va_x17;
           break;
         case 16:
         NV(piano_kline13_cdur) = va_dur17;
           NV(piano_kline13_clp) = NV(piano_kline13_crp);
           NV(piano_kline13_crp) = va_x18;
           break;
         case 17:
         NV(piano_kline13_cdur) = va_dur18;
           NV(piano_kline13_clp) = NV(piano_kline13_crp);
           NV(piano_kline13_crp) = va_x19;
           break;
         case 18:
         NV(piano_kline13_cdur) = va_dur19;
           NV(piano_kline13_clp) = NV(piano_kline13_crp);
           NV(piano_kline13_crp) = va_x20;
           break;
         case 19:
         NV(piano_kline13_cdur) = va_dur20;
           NV(piano_kline13_clp) = NV(piano_kline13_crp);
           NV(piano_kline13_crp) = va_x21;
           break;
           default:
           NVI(piano_kline13_first) = -100;
           NV(piano_kline13_cdur) = NV(piano_kline13_t) + 10000.0F;
           break;
           }
         NVI(piano_kline13_first)++;
        }
        NV(piano_kline13_mult)=(NV(piano_kline13_crp) - NV(piano_kline13_clp))/NV(piano_kline13_cdur);
        ret = NV(piano_kline13_outT) = NV(piano_kline13_clp)+NV(piano_kline13_mult)*NV(piano_kline13_t);
        NV(piano_kline13_addK) = NV(piano_kline13_mult)*EV(KTIME);
        if (NVI(piano_kline13_first)<0)
          ret = 0.0F;
      }
   }
   if (NVI(piano_kline13_first)==0)
     {
       NVI(piano_kline13_first) = 1;
       ret = NV(piano_kline13_outT) = NV(piano_kline13_clp) = x1;
       NV(piano_kline13_crp) = x2;
       NV(piano_kline13_cdur) = dur1;
       if (dur1 > 0.0F)
         NV(piano_kline13_addK) = EV(KTIME)*((x2 - x1)/dur1);
     }
   return((NV(piano_kline13_return) = ret));

}



float piano__sym_kline14(ENGINE_PTR_DECLARE_COMMA struct ninstr_types * nstate)
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
   float va_dur20;
   float va_x21;
   float x2;
   float dur1;
   float x1;
   float ret;

   x1 =  0.0F ;
   dur1 =  0.008039F ;
   x2 =  0.002020F ;
   va_dur2 =  0.003215F  ;
   va_x3 =  0.003791F  ;
   va_dur3 =  0.003215F  ;
   va_x4 =  0.001646F  ;
   va_dur4 =  0.003215F  ;
   va_x5 =  0.003148F  ;
   va_dur5 =  0.008039F  ;
   va_x6 =  0.000336F  ;
   va_dur6 =  0.001608F  ;
   va_x7 =  0.002098F  ;
   va_dur7 =  0.003215F  ;
   va_x8 =  0.000179F  ;
   va_dur8 =  0.001608F  ;
   va_x9 =  0.001720F  ;
   va_dur9 =  0.004823F  ;
   va_x10 =  0.002207F  ;
   va_dur10 =  0.003215F  ;
   va_x11 =  0.000292F  ;
   va_dur11 =  0.011254F  ;
   va_x12 =  0.001372F  ;
   va_dur12 =  0.003215F  ;
   va_x13 =  0.000174F  ;
   va_dur13 =  0.009646F  ;
   va_x14 =  0.002101F  ;
   va_dur14 =  0.014469F  ;
   va_x15 =  0.000602F  ;
   va_dur15 =  0.001608F  ;
   va_x16 =  0.002307F  ;
   va_dur16 =  0.008039F  ;
   va_x17 =  0.000264F  ;
   va_dur17 =  0.001608F  ;
   va_x18 =  0.001441F  ;
   va_dur18 =  0.011254F  ;
   va_x19 =  0.001514F  ;
   va_dur19 =  0.123794F  ;
   va_x20 =  0.000121F  ;
   va_dur20 =  2.266881F  ;
   va_x21 =  0.0F  ;
   ret = 0.0F;
   if (NVI(piano_kline14_first)>0)
   {
      NV(piano_kline14_t) += EV(KTIME);
      ret = (NV(piano_kline14_outT) += NV(piano_kline14_addK));
      if (NV(piano_kline14_t) > NV(piano_kline14_cdur))
       {
        while (NV(piano_kline14_t) > NV(piano_kline14_cdur))
         {
           NV(piano_kline14_t) -= NV(piano_kline14_cdur);
           switch(NVI(piano_kline14_first))
      {
         case 1:
         NV(piano_kline14_cdur) = va_dur2;
           NV(piano_kline14_clp) = NV(piano_kline14_crp);
           NV(piano_kline14_crp) = va_x3;
           break;
         case 2:
         NV(piano_kline14_cdur) = va_dur3;
           NV(piano_kline14_clp) = NV(piano_kline14_crp);
           NV(piano_kline14_crp) = va_x4;
           break;
         case 3:
         NV(piano_kline14_cdur) = va_dur4;
           NV(piano_kline14_clp) = NV(piano_kline14_crp);
           NV(piano_kline14_crp) = va_x5;
           break;
         case 4:
         NV(piano_kline14_cdur) = va_dur5;
           NV(piano_kline14_clp) = NV(piano_kline14_crp);
           NV(piano_kline14_crp) = va_x6;
           break;
         case 5:
         NV(piano_kline14_cdur) = va_dur6;
           NV(piano_kline14_clp) = NV(piano_kline14_crp);
           NV(piano_kline14_crp) = va_x7;
           break;
         case 6:
         NV(piano_kline14_cdur) = va_dur7;
           NV(piano_kline14_clp) = NV(piano_kline14_crp);
           NV(piano_kline14_crp) = va_x8;
           break;
         case 7:
         NV(piano_kline14_cdur) = va_dur8;
           NV(piano_kline14_clp) = NV(piano_kline14_crp);
           NV(piano_kline14_crp) = va_x9;
           break;
         case 8:
         NV(piano_kline14_cdur) = va_dur9;
           NV(piano_kline14_clp) = NV(piano_kline14_crp);
           NV(piano_kline14_crp) = va_x10;
           break;
         case 9:
         NV(piano_kline14_cdur) = va_dur10;
           NV(piano_kline14_clp) = NV(piano_kline14_crp);
           NV(piano_kline14_crp) = va_x11;
           break;
         case 10:
         NV(piano_kline14_cdur) = va_dur11;
           NV(piano_kline14_clp) = NV(piano_kline14_crp);
           NV(piano_kline14_crp) = va_x12;
           break;
         case 11:
         NV(piano_kline14_cdur) = va_dur12;
           NV(piano_kline14_clp) = NV(piano_kline14_crp);
           NV(piano_kline14_crp) = va_x13;
           break;
         case 12:
         NV(piano_kline14_cdur) = va_dur13;
           NV(piano_kline14_clp) = NV(piano_kline14_crp);
           NV(piano_kline14_crp) = va_x14;
           break;
         case 13:
         NV(piano_kline14_cdur) = va_dur14;
           NV(piano_kline14_clp) = NV(piano_kline14_crp);
           NV(piano_kline14_crp) = va_x15;
           break;
         case 14:
         NV(piano_kline14_cdur) = va_dur15;
           NV(piano_kline14_clp) = NV(piano_kline14_crp);
           NV(piano_kline14_crp) = va_x16;
           break;
         case 15:
         NV(piano_kline14_cdur) = va_dur16;
           NV(piano_kline14_clp) = NV(piano_kline14_crp);
           NV(piano_kline14_crp) = va_x17;
           break;
         case 16:
         NV(piano_kline14_cdur) = va_dur17;
           NV(piano_kline14_clp) = NV(piano_kline14_crp);
           NV(piano_kline14_crp) = va_x18;
           break;
         case 17:
         NV(piano_kline14_cdur) = va_dur18;
           NV(piano_kline14_clp) = NV(piano_kline14_crp);
           NV(piano_kline14_crp) = va_x19;
           break;
         case 18:
         NV(piano_kline14_cdur) = va_dur19;
           NV(piano_kline14_clp) = NV(piano_kline14_crp);
           NV(piano_kline14_crp) = va_x20;
           break;
         case 19:
         NV(piano_kline14_cdur) = va_dur20;
           NV(piano_kline14_clp) = NV(piano_kline14_crp);
           NV(piano_kline14_crp) = va_x21;
           break;
           default:
           NVI(piano_kline14_first) = -100;
           NV(piano_kline14_cdur) = NV(piano_kline14_t) + 10000.0F;
           break;
           }
         NVI(piano_kline14_first)++;
        }
        NV(piano_kline14_mult)=(NV(piano_kline14_crp) - NV(piano_kline14_clp))/NV(piano_kline14_cdur);
        ret = NV(piano_kline14_outT) = NV(piano_kline14_clp)+NV(piano_kline14_mult)*NV(piano_kline14_t);
        NV(piano_kline14_addK) = NV(piano_kline14_mult)*EV(KTIME);
        if (NVI(piano_kline14_first)<0)
          ret = 0.0F;
      }
   }
   if (NVI(piano_kline14_first)==0)
     {
       NVI(piano_kline14_first) = 1;
       ret = NV(piano_kline14_outT) = NV(piano_kline14_clp) = x1;
       NV(piano_kline14_crp) = x2;
       NV(piano_kline14_cdur) = dur1;
       if (dur1 > 0.0F)
         NV(piano_kline14_addK) = EV(KTIME)*((x2 - x1)/dur1);
     }
   return((NV(piano_kline14_return) = ret));

}



float piano__sym_kline15(ENGINE_PTR_DECLARE_COMMA struct ninstr_types * nstate)
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
   float va_dur20;
   float va_x21;
   float x2;
   float dur1;
   float x1;
   float ret;

   x1 =  0.0F ;
   dur1 =  0.004823F ;
   x2 =  0.002273F ;
   va_dur2 =  0.001608F  ;
   va_x3 =  0.000118F  ;
   va_dur3 =  0.001608F  ;
   va_x4 =  0.001936F  ;
   va_dur4 =  0.001608F  ;
   va_x5 =  0.000916F  ;
   va_dur5 =  0.001608F  ;
   va_x6 =  0.001899F  ;
   va_dur6 =  0.001608F  ;
   va_x7 =  0.000347F  ;
   va_dur7 =  0.008039F  ;
   va_x8 =  0.001905F  ;
   va_dur8 =  0.001608F  ;
   va_x9 =  0.000736F  ;
   va_dur9 =  0.004823F  ;
   va_x10 =  0.001234F  ;
   va_dur10 =  0.006431F  ;
   va_x11 =  0.000059F  ;
   va_dur11 =  0.001608F  ;
   va_x12 =  0.000977F  ;
   va_dur12 =  0.028939F  ;
   va_x13 =  0.000234F  ;
   va_dur13 =  0.024116F  ;
   va_x14 =  0.001823F  ;
   va_dur14 =  0.024116F  ;
   va_x15 =  0.001210F  ;
   va_dur15 =  0.009646F  ;
   va_x16 =  0.002041F  ;
   va_dur16 =  0.020900F  ;
   va_x17 =  0.000951F  ;
   va_dur17 =  0.027331F  ;
   va_x18 =  0.001302F  ;
   va_dur18 =  0.001608F  ;
   va_x19 =  0.000448F  ;
   va_dur19 =  0.149518F  ;
   va_x20 =  0.000086F  ;
   va_dur20 =  2.170418F  ;
   va_x21 =  0.0F  ;
   ret = 0.0F;
   if (NVI(piano_kline15_first)>0)
   {
      NV(piano_kline15_t) += EV(KTIME);
      ret = (NV(piano_kline15_outT) += NV(piano_kline15_addK));
      if (NV(piano_kline15_t) > NV(piano_kline15_cdur))
       {
        while (NV(piano_kline15_t) > NV(piano_kline15_cdur))
         {
           NV(piano_kline15_t) -= NV(piano_kline15_cdur);
           switch(NVI(piano_kline15_first))
      {
         case 1:
         NV(piano_kline15_cdur) = va_dur2;
           NV(piano_kline15_clp) = NV(piano_kline15_crp);
           NV(piano_kline15_crp) = va_x3;
           break;
         case 2:
         NV(piano_kline15_cdur) = va_dur3;
           NV(piano_kline15_clp) = NV(piano_kline15_crp);
           NV(piano_kline15_crp) = va_x4;
           break;
         case 3:
         NV(piano_kline15_cdur) = va_dur4;
           NV(piano_kline15_clp) = NV(piano_kline15_crp);
           NV(piano_kline15_crp) = va_x5;
           break;
         case 4:
         NV(piano_kline15_cdur) = va_dur5;
           NV(piano_kline15_clp) = NV(piano_kline15_crp);
           NV(piano_kline15_crp) = va_x6;
           break;
         case 5:
         NV(piano_kline15_cdur) = va_dur6;
           NV(piano_kline15_clp) = NV(piano_kline15_crp);
           NV(piano_kline15_crp) = va_x7;
           break;
         case 6:
         NV(piano_kline15_cdur) = va_dur7;
           NV(piano_kline15_clp) = NV(piano_kline15_crp);
           NV(piano_kline15_crp) = va_x8;
           break;
         case 7:
         NV(piano_kline15_cdur) = va_dur8;
           NV(piano_kline15_clp) = NV(piano_kline15_crp);
           NV(piano_kline15_crp) = va_x9;
           break;
         case 8:
         NV(piano_kline15_cdur) = va_dur9;
           NV(piano_kline15_clp) = NV(piano_kline15_crp);
           NV(piano_kline15_crp) = va_x10;
           break;
         case 9:
         NV(piano_kline15_cdur) = va_dur10;
           NV(piano_kline15_clp) = NV(piano_kline15_crp);
           NV(piano_kline15_crp) = va_x11;
           break;
         case 10:
         NV(piano_kline15_cdur) = va_dur11;
           NV(piano_kline15_clp) = NV(piano_kline15_crp);
           NV(piano_kline15_crp) = va_x12;
           break;
         case 11:
         NV(piano_kline15_cdur) = va_dur12;
           NV(piano_kline15_clp) = NV(piano_kline15_crp);
           NV(piano_kline15_crp) = va_x13;
           break;
         case 12:
         NV(piano_kline15_cdur) = va_dur13;
           NV(piano_kline15_clp) = NV(piano_kline15_crp);
           NV(piano_kline15_crp) = va_x14;
           break;
         case 13:
         NV(piano_kline15_cdur) = va_dur14;
           NV(piano_kline15_clp) = NV(piano_kline15_crp);
           NV(piano_kline15_crp) = va_x15;
           break;
         case 14:
         NV(piano_kline15_cdur) = va_dur15;
           NV(piano_kline15_clp) = NV(piano_kline15_crp);
           NV(piano_kline15_crp) = va_x16;
           break;
         case 15:
         NV(piano_kline15_cdur) = va_dur16;
           NV(piano_kline15_clp) = NV(piano_kline15_crp);
           NV(piano_kline15_crp) = va_x17;
           break;
         case 16:
         NV(piano_kline15_cdur) = va_dur17;
           NV(piano_kline15_clp) = NV(piano_kline15_crp);
           NV(piano_kline15_crp) = va_x18;
           break;
         case 17:
         NV(piano_kline15_cdur) = va_dur18;
           NV(piano_kline15_clp) = NV(piano_kline15_crp);
           NV(piano_kline15_crp) = va_x19;
           break;
         case 18:
         NV(piano_kline15_cdur) = va_dur19;
           NV(piano_kline15_clp) = NV(piano_kline15_crp);
           NV(piano_kline15_crp) = va_x20;
           break;
         case 19:
         NV(piano_kline15_cdur) = va_dur20;
           NV(piano_kline15_clp) = NV(piano_kline15_crp);
           NV(piano_kline15_crp) = va_x21;
           break;
           default:
           NVI(piano_kline15_first) = -100;
           NV(piano_kline15_cdur) = NV(piano_kline15_t) + 10000.0F;
           break;
           }
         NVI(piano_kline15_first)++;
        }
        NV(piano_kline15_mult)=(NV(piano_kline15_crp) - NV(piano_kline15_clp))/NV(piano_kline15_cdur);
        ret = NV(piano_kline15_outT) = NV(piano_kline15_clp)+NV(piano_kline15_mult)*NV(piano_kline15_t);
        NV(piano_kline15_addK) = NV(piano_kline15_mult)*EV(KTIME);
        if (NVI(piano_kline15_first)<0)
          ret = 0.0F;
      }
   }
   if (NVI(piano_kline15_first)==0)
     {
       NVI(piano_kline15_first) = 1;
       ret = NV(piano_kline15_outT) = NV(piano_kline15_clp) = x1;
       NV(piano_kline15_crp) = x2;
       NV(piano_kline15_cdur) = dur1;
       if (dur1 > 0.0F)
         NV(piano_kline15_addK) = EV(KTIME)*((x2 - x1)/dur1);
     }
   return((NV(piano_kline15_return) = ret));

}



float piano__sym_kline16(ENGINE_PTR_DECLARE_COMMA struct ninstr_types * nstate)
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
   float va_dur20;
   float va_x21;
   float x2;
   float dur1;
   float x1;
   float ret;

   x1 =  0.0F ;
   dur1 =  0.001608F ;
   x2 =  0.003429F ;
   va_dur2 =  0.001608F  ;
   va_x3 =  0.002397F  ;
   va_dur3 =  0.009646F  ;
   va_x4 =  0.003121F  ;
   va_dur4 =  0.006431F  ;
   va_x5 =  0.002265F  ;
   va_dur5 =  0.001608F  ;
   va_x6 =  0.002992F  ;
   va_dur6 =  0.043408F  ;
   va_x7 =  0.001078F  ;
   va_dur7 =  0.009646F  ;
   va_x8 =  0.001671F  ;
   va_dur8 =  0.028939F  ;
   va_x9 =  0.000791F  ;
   va_dur9 =  0.004823F  ;
   va_x10 =  0.001557F  ;
   va_dur10 =  0.001608F  ;
   va_x11 =  0.000819F  ;
   va_dur11 =  0.019293F  ;
   va_x12 =  0.000697F  ;
   va_dur12 =  0.009646F  ;
   va_x13 =  0.001461F  ;
   va_dur13 =  0.001608F  ;
   va_x14 =  0.000467F  ;
   va_dur14 =  0.001608F  ;
   va_x15 =  0.001215F  ;
   va_dur15 =  0.011254F  ;
   va_x16 =  0.001459F  ;
   va_dur16 =  0.051447F  ;
   va_x17 =  0.000876F  ;
   va_dur17 =  0.004823F  ;
   va_x18 =  0.001703F  ;
   va_dur18 =  0.006431F  ;
   va_x19 =  0.000850F  ;
   va_dur19 =  0.282958F  ;
   va_x20 =  0.000045F  ;
   va_dur20 =  1.993569F  ;
   va_x21 =  0.0F  ;
   ret = 0.0F;
   if (NVI(piano_kline16_first)>0)
   {
      NV(piano_kline16_t) += EV(KTIME);
      ret = (NV(piano_kline16_outT) += NV(piano_kline16_addK));
      if (NV(piano_kline16_t) > NV(piano_kline16_cdur))
       {
        while (NV(piano_kline16_t) > NV(piano_kline16_cdur))
         {
           NV(piano_kline16_t) -= NV(piano_kline16_cdur);
           switch(NVI(piano_kline16_first))
      {
         case 1:
         NV(piano_kline16_cdur) = va_dur2;
           NV(piano_kline16_clp) = NV(piano_kline16_crp);
           NV(piano_kline16_crp) = va_x3;
           break;
         case 2:
         NV(piano_kline16_cdur) = va_dur3;
           NV(piano_kline16_clp) = NV(piano_kline16_crp);
           NV(piano_kline16_crp) = va_x4;
           break;
         case 3:
         NV(piano_kline16_cdur) = va_dur4;
           NV(piano_kline16_clp) = NV(piano_kline16_crp);
           NV(piano_kline16_crp) = va_x5;
           break;
         case 4:
         NV(piano_kline16_cdur) = va_dur5;
           NV(piano_kline16_clp) = NV(piano_kline16_crp);
           NV(piano_kline16_crp) = va_x6;
           break;
         case 5:
         NV(piano_kline16_cdur) = va_dur6;
           NV(piano_kline16_clp) = NV(piano_kline16_crp);
           NV(piano_kline16_crp) = va_x7;
           break;
         case 6:
         NV(piano_kline16_cdur) = va_dur7;
           NV(piano_kline16_clp) = NV(piano_kline16_crp);
           NV(piano_kline16_crp) = va_x8;
           break;
         case 7:
         NV(piano_kline16_cdur) = va_dur8;
           NV(piano_kline16_clp) = NV(piano_kline16_crp);
           NV(piano_kline16_crp) = va_x9;
           break;
         case 8:
         NV(piano_kline16_cdur) = va_dur9;
           NV(piano_kline16_clp) = NV(piano_kline16_crp);
           NV(piano_kline16_crp) = va_x10;
           break;
         case 9:
         NV(piano_kline16_cdur) = va_dur10;
           NV(piano_kline16_clp) = NV(piano_kline16_crp);
           NV(piano_kline16_crp) = va_x11;
           break;
         case 10:
         NV(piano_kline16_cdur) = va_dur11;
           NV(piano_kline16_clp) = NV(piano_kline16_crp);
           NV(piano_kline16_crp) = va_x12;
           break;
         case 11:
         NV(piano_kline16_cdur) = va_dur12;
           NV(piano_kline16_clp) = NV(piano_kline16_crp);
           NV(piano_kline16_crp) = va_x13;
           break;
         case 12:
         NV(piano_kline16_cdur) = va_dur13;
           NV(piano_kline16_clp) = NV(piano_kline16_crp);
           NV(piano_kline16_crp) = va_x14;
           break;
         case 13:
         NV(piano_kline16_cdur) = va_dur14;
           NV(piano_kline16_clp) = NV(piano_kline16_crp);
           NV(piano_kline16_crp) = va_x15;
           break;
         case 14:
         NV(piano_kline16_cdur) = va_dur15;
           NV(piano_kline16_clp) = NV(piano_kline16_crp);
           NV(piano_kline16_crp) = va_x16;
           break;
         case 15:
         NV(piano_kline16_cdur) = va_dur16;
           NV(piano_kline16_clp) = NV(piano_kline16_crp);
           NV(piano_kline16_crp) = va_x17;
           break;
         case 16:
         NV(piano_kline16_cdur) = va_dur17;
           NV(piano_kline16_clp) = NV(piano_kline16_crp);
           NV(piano_kline16_crp) = va_x18;
           break;
         case 17:
         NV(piano_kline16_cdur) = va_dur18;
           NV(piano_kline16_clp) = NV(piano_kline16_crp);
           NV(piano_kline16_crp) = va_x19;
           break;
         case 18:
         NV(piano_kline16_cdur) = va_dur19;
           NV(piano_kline16_clp) = NV(piano_kline16_crp);
           NV(piano_kline16_crp) = va_x20;
           break;
         case 19:
         NV(piano_kline16_cdur) = va_dur20;
           NV(piano_kline16_clp) = NV(piano_kline16_crp);
           NV(piano_kline16_crp) = va_x21;
           break;
           default:
           NVI(piano_kline16_first) = -100;
           NV(piano_kline16_cdur) = NV(piano_kline16_t) + 10000.0F;
           break;
           }
         NVI(piano_kline16_first)++;
        }
        NV(piano_kline16_mult)=(NV(piano_kline16_crp) - NV(piano_kline16_clp))/NV(piano_kline16_cdur);
        ret = NV(piano_kline16_outT) = NV(piano_kline16_clp)+NV(piano_kline16_mult)*NV(piano_kline16_t);
        NV(piano_kline16_addK) = NV(piano_kline16_mult)*EV(KTIME);
        if (NVI(piano_kline16_first)<0)
          ret = 0.0F;
      }
   }
   if (NVI(piano_kline16_first)==0)
     {
       NVI(piano_kline16_first) = 1;
       ret = NV(piano_kline16_outT) = NV(piano_kline16_clp) = x1;
       NV(piano_kline16_crp) = x2;
       NV(piano_kline16_cdur) = dur1;
       if (dur1 > 0.0F)
         NV(piano_kline16_addK) = EV(KTIME)*((x2 - x1)/dur1);
     }
   return((NV(piano_kline16_return) = ret));

}



float piano__sym_kline17(ENGINE_PTR_DECLARE_COMMA struct ninstr_types * nstate)
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
   float va_dur20;
   float va_x21;
   float x2;
   float dur1;
   float x1;
   float ret;

   x1 =  0.0F ;
   dur1 =  0.003215F ;
   x2 =  0.005754F ;
   va_dur2 =  0.006431F  ;
   va_x3 =  0.004108F  ;
   va_dur3 =  0.003215F  ;
   va_x4 =  0.004503F  ;
   va_dur4 =  0.003215F  ;
   va_x5 =  0.003749F  ;
   va_dur5 =  0.008039F  ;
   va_x6 =  0.003208F  ;
   va_dur6 =  0.022508F  ;
   va_x7 =  0.002241F  ;
   va_dur7 =  0.024116F  ;
   va_x8 =  0.003647F  ;
   va_dur8 =  0.003215F  ;
   va_x9 =  0.004181F  ;
   va_dur9 =  0.035370F  ;
   va_x10 =  0.005996F  ;
   va_dur10 =  0.028939F  ;
   va_x11 =  0.006136F  ;
   va_dur11 =  0.038585F  ;
   va_x12 =  0.005347F  ;
   va_dur12 =  0.009646F  ;
   va_x13 =  0.004598F  ;
   va_dur13 =  0.032154F  ;
   va_x14 =  0.003484F  ;
   va_dur14 =  0.065916F  ;
   va_x15 =  0.001774F  ;
   va_dur15 =  0.049839F  ;
   va_x16 =  0.001424F  ;
   va_dur16 =  0.138264F  ;
   va_x17 =  0.001732F  ;
   va_dur17 =  0.139871F  ;
   va_x18 =  0.000677F  ;
   va_dur18 =  0.147910F  ;
   va_x19 =  0.000915F  ;
   va_dur19 =  0.485531F  ;
   va_x20 =  0.000070F  ;
   va_dur20 =  1.245981F  ;
   va_x21 =  0.0F  ;
   ret = 0.0F;
   if (NVI(piano_kline17_first)>0)
   {
      NV(piano_kline17_t) += EV(KTIME);
      ret = (NV(piano_kline17_outT) += NV(piano_kline17_addK));
      if (NV(piano_kline17_t) > NV(piano_kline17_cdur))
       {
        while (NV(piano_kline17_t) > NV(piano_kline17_cdur))
         {
           NV(piano_kline17_t) -= NV(piano_kline17_cdur);
           switch(NVI(piano_kline17_first))
      {
         case 1:
         NV(piano_kline17_cdur) = va_dur2;
           NV(piano_kline17_clp) = NV(piano_kline17_crp);
           NV(piano_kline17_crp) = va_x3;
           break;
         case 2:
         NV(piano_kline17_cdur) = va_dur3;
           NV(piano_kline17_clp) = NV(piano_kline17_crp);
           NV(piano_kline17_crp) = va_x4;
           break;
         case 3:
         NV(piano_kline17_cdur) = va_dur4;
           NV(piano_kline17_clp) = NV(piano_kline17_crp);
           NV(piano_kline17_crp) = va_x5;
           break;
         case 4:
         NV(piano_kline17_cdur) = va_dur5;
           NV(piano_kline17_clp) = NV(piano_kline17_crp);
           NV(piano_kline17_crp) = va_x6;
           break;
         case 5:
         NV(piano_kline17_cdur) = va_dur6;
           NV(piano_kline17_clp) = NV(piano_kline17_crp);
           NV(piano_kline17_crp) = va_x7;
           break;
         case 6:
         NV(piano_kline17_cdur) = va_dur7;
           NV(piano_kline17_clp) = NV(piano_kline17_crp);
           NV(piano_kline17_crp) = va_x8;
           break;
         case 7:
         NV(piano_kline17_cdur) = va_dur8;
           NV(piano_kline17_clp) = NV(piano_kline17_crp);
           NV(piano_kline17_crp) = va_x9;
           break;
         case 8:
         NV(piano_kline17_cdur) = va_dur9;
           NV(piano_kline17_clp) = NV(piano_kline17_crp);
           NV(piano_kline17_crp) = va_x10;
           break;
         case 9:
         NV(piano_kline17_cdur) = va_dur10;
           NV(piano_kline17_clp) = NV(piano_kline17_crp);
           NV(piano_kline17_crp) = va_x11;
           break;
         case 10:
         NV(piano_kline17_cdur) = va_dur11;
           NV(piano_kline17_clp) = NV(piano_kline17_crp);
           NV(piano_kline17_crp) = va_x12;
           break;
         case 11:
         NV(piano_kline17_cdur) = va_dur12;
           NV(piano_kline17_clp) = NV(piano_kline17_crp);
           NV(piano_kline17_crp) = va_x13;
           break;
         case 12:
         NV(piano_kline17_cdur) = va_dur13;
           NV(piano_kline17_clp) = NV(piano_kline17_crp);
           NV(piano_kline17_crp) = va_x14;
           break;
         case 13:
         NV(piano_kline17_cdur) = va_dur14;
           NV(piano_kline17_clp) = NV(piano_kline17_crp);
           NV(piano_kline17_crp) = va_x15;
           break;
         case 14:
         NV(piano_kline17_cdur) = va_dur15;
           NV(piano_kline17_clp) = NV(piano_kline17_crp);
           NV(piano_kline17_crp) = va_x16;
           break;
         case 15:
         NV(piano_kline17_cdur) = va_dur16;
           NV(piano_kline17_clp) = NV(piano_kline17_crp);
           NV(piano_kline17_crp) = va_x17;
           break;
         case 16:
         NV(piano_kline17_cdur) = va_dur17;
           NV(piano_kline17_clp) = NV(piano_kline17_crp);
           NV(piano_kline17_crp) = va_x18;
           break;
         case 17:
         NV(piano_kline17_cdur) = va_dur18;
           NV(piano_kline17_clp) = NV(piano_kline17_crp);
           NV(piano_kline17_crp) = va_x19;
           break;
         case 18:
         NV(piano_kline17_cdur) = va_dur19;
           NV(piano_kline17_clp) = NV(piano_kline17_crp);
           NV(piano_kline17_crp) = va_x20;
           break;
         case 19:
         NV(piano_kline17_cdur) = va_dur20;
           NV(piano_kline17_clp) = NV(piano_kline17_crp);
           NV(piano_kline17_crp) = va_x21;
           break;
           default:
           NVI(piano_kline17_first) = -100;
           NV(piano_kline17_cdur) = NV(piano_kline17_t) + 10000.0F;
           break;
           }
         NVI(piano_kline17_first)++;
        }
        NV(piano_kline17_mult)=(NV(piano_kline17_crp) - NV(piano_kline17_clp))/NV(piano_kline17_cdur);
        ret = NV(piano_kline17_outT) = NV(piano_kline17_clp)+NV(piano_kline17_mult)*NV(piano_kline17_t);
        NV(piano_kline17_addK) = NV(piano_kline17_mult)*EV(KTIME);
        if (NVI(piano_kline17_first)<0)
          ret = 0.0F;
      }
   }
   if (NVI(piano_kline17_first)==0)
     {
       NVI(piano_kline17_first) = 1;
       ret = NV(piano_kline17_outT) = NV(piano_kline17_clp) = x1;
       NV(piano_kline17_crp) = x2;
       NV(piano_kline17_cdur) = dur1;
       if (dur1 > 0.0F)
         NV(piano_kline17_addK) = EV(KTIME)*((x2 - x1)/dur1);
     }
   return((NV(piano_kline17_return) = ret));

}



float piano__sym_kline18(ENGINE_PTR_DECLARE_COMMA struct ninstr_types * nstate)
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
   float va_dur20;
   float va_x21;
   float x2;
   float dur1;
   float x1;
   float ret;

   x1 =  0.0F ;
   dur1 =  0.001608F ;
   x2 =  0.001431F ;
   va_dur2 =  0.004823F  ;
   va_x3 =  0.001291F  ;
   va_dur3 =  0.001608F  ;
   va_x4 =  0.000856F  ;
   va_dur4 =  0.004823F  ;
   va_x5 =  0.001181F  ;
   va_dur5 =  0.001608F  ;
   va_x6 =  0.001137F  ;
   va_dur6 =  0.001608F  ;
   va_x7 =  0.000707F  ;
   va_dur7 =  0.006431F  ;
   va_x8 =  0.001008F  ;
   va_dur8 =  0.003215F  ;
   va_x9 =  0.000663F  ;
   va_dur9 =  0.012862F  ;
   va_x10 =  0.000717F  ;
   va_dur10 =  0.004823F  ;
   va_x11 =  0.000453F  ;
   va_dur11 =  0.001608F  ;
   va_x12 =  0.000675F  ;
   va_dur12 =  0.004823F  ;
   va_x13 =  0.000303F  ;
   va_dur13 =  0.004823F  ;
   va_x14 =  0.000514F  ;
   va_dur14 =  0.019293F  ;
   va_x15 =  0.000437F  ;
   va_dur15 =  0.001608F  ;
   va_x16 =  0.000670F  ;
   va_dur16 =  0.001608F  ;
   va_x17 =  0.000494F  ;
   va_dur17 =  0.081994F  ;
   va_x18 =  0.000998F  ;
   va_dur18 =  0.139871F  ;
   va_x19 =  0.000226F  ;
   va_dur19 =  0.273312F  ;
   va_x20 =  0.000016F  ;
   va_dur20 =  1.919614F  ;
   va_x21 =  0.0F  ;
   ret = 0.0F;
   if (NVI(piano_kline18_first)>0)
   {
      NV(piano_kline18_t) += EV(KTIME);
      ret = (NV(piano_kline18_outT) += NV(piano_kline18_addK));
      if (NV(piano_kline18_t) > NV(piano_kline18_cdur))
       {
        while (NV(piano_kline18_t) > NV(piano_kline18_cdur))
         {
           NV(piano_kline18_t) -= NV(piano_kline18_cdur);
           switch(NVI(piano_kline18_first))
      {
         case 1:
         NV(piano_kline18_cdur) = va_dur2;
           NV(piano_kline18_clp) = NV(piano_kline18_crp);
           NV(piano_kline18_crp) = va_x3;
           break;
         case 2:
         NV(piano_kline18_cdur) = va_dur3;
           NV(piano_kline18_clp) = NV(piano_kline18_crp);
           NV(piano_kline18_crp) = va_x4;
           break;
         case 3:
         NV(piano_kline18_cdur) = va_dur4;
           NV(piano_kline18_clp) = NV(piano_kline18_crp);
           NV(piano_kline18_crp) = va_x5;
           break;
         case 4:
         NV(piano_kline18_cdur) = va_dur5;
           NV(piano_kline18_clp) = NV(piano_kline18_crp);
           NV(piano_kline18_crp) = va_x6;
           break;
         case 5:
         NV(piano_kline18_cdur) = va_dur6;
           NV(piano_kline18_clp) = NV(piano_kline18_crp);
           NV(piano_kline18_crp) = va_x7;
           break;
         case 6:
         NV(piano_kline18_cdur) = va_dur7;
           NV(piano_kline18_clp) = NV(piano_kline18_crp);
           NV(piano_kline18_crp) = va_x8;
           break;
         case 7:
         NV(piano_kline18_cdur) = va_dur8;
           NV(piano_kline18_clp) = NV(piano_kline18_crp);
           NV(piano_kline18_crp) = va_x9;
           break;
         case 8:
         NV(piano_kline18_cdur) = va_dur9;
           NV(piano_kline18_clp) = NV(piano_kline18_crp);
           NV(piano_kline18_crp) = va_x10;
           break;
         case 9:
         NV(piano_kline18_cdur) = va_dur10;
           NV(piano_kline18_clp) = NV(piano_kline18_crp);
           NV(piano_kline18_crp) = va_x11;
           break;
         case 10:
         NV(piano_kline18_cdur) = va_dur11;
           NV(piano_kline18_clp) = NV(piano_kline18_crp);
           NV(piano_kline18_crp) = va_x12;
           break;
         case 11:
         NV(piano_kline18_cdur) = va_dur12;
           NV(piano_kline18_clp) = NV(piano_kline18_crp);
           NV(piano_kline18_crp) = va_x13;
           break;
         case 12:
         NV(piano_kline18_cdur) = va_dur13;
           NV(piano_kline18_clp) = NV(piano_kline18_crp);
           NV(piano_kline18_crp) = va_x14;
           break;
         case 13:
         NV(piano_kline18_cdur) = va_dur14;
           NV(piano_kline18_clp) = NV(piano_kline18_crp);
           NV(piano_kline18_crp) = va_x15;
           break;
         case 14:
         NV(piano_kline18_cdur) = va_dur15;
           NV(piano_kline18_clp) = NV(piano_kline18_crp);
           NV(piano_kline18_crp) = va_x16;
           break;
         case 15:
         NV(piano_kline18_cdur) = va_dur16;
           NV(piano_kline18_clp) = NV(piano_kline18_crp);
           NV(piano_kline18_crp) = va_x17;
           break;
         case 16:
         NV(piano_kline18_cdur) = va_dur17;
           NV(piano_kline18_clp) = NV(piano_kline18_crp);
           NV(piano_kline18_crp) = va_x18;
           break;
         case 17:
         NV(piano_kline18_cdur) = va_dur18;
           NV(piano_kline18_clp) = NV(piano_kline18_crp);
           NV(piano_kline18_crp) = va_x19;
           break;
         case 18:
         NV(piano_kline18_cdur) = va_dur19;
           NV(piano_kline18_clp) = NV(piano_kline18_crp);
           NV(piano_kline18_crp) = va_x20;
           break;
         case 19:
         NV(piano_kline18_cdur) = va_dur20;
           NV(piano_kline18_clp) = NV(piano_kline18_crp);
           NV(piano_kline18_crp) = va_x21;
           break;
           default:
           NVI(piano_kline18_first) = -100;
           NV(piano_kline18_cdur) = NV(piano_kline18_t) + 10000.0F;
           break;
           }
         NVI(piano_kline18_first)++;
        }
        NV(piano_kline18_mult)=(NV(piano_kline18_crp) - NV(piano_kline18_clp))/NV(piano_kline18_cdur);
        ret = NV(piano_kline18_outT) = NV(piano_kline18_clp)+NV(piano_kline18_mult)*NV(piano_kline18_t);
        NV(piano_kline18_addK) = NV(piano_kline18_mult)*EV(KTIME);
        if (NVI(piano_kline18_first)<0)
          ret = 0.0F;
      }
   }
   if (NVI(piano_kline18_first)==0)
     {
       NVI(piano_kline18_first) = 1;
       ret = NV(piano_kline18_outT) = NV(piano_kline18_clp) = x1;
       NV(piano_kline18_crp) = x2;
       NV(piano_kline18_cdur) = dur1;
       if (dur1 > 0.0F)
         NV(piano_kline18_addK) = EV(KTIME)*((x2 - x1)/dur1);
     }
   return((NV(piano_kline18_return) = ret));

}



float piano__sym_kline19(ENGINE_PTR_DECLARE_COMMA struct ninstr_types * nstate)
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
   float va_dur20;
   float va_x21;
   float x2;
   float dur1;
   float x1;
   float ret;

   x1 =  0.0F ;
   dur1 =  0.003215F ;
   x2 =  0.000800F ;
   va_dur2 =  0.004823F  ;
   va_x3 =  0.000910F  ;
   va_dur3 =  0.006431F  ;
   va_x4 =  0.000543F  ;
   va_dur4 =  0.020900F  ;
   va_x5 =  0.000849F  ;
   va_dur5 =  0.006431F  ;
   va_x6 =  0.000472F  ;
   va_dur6 =  0.019293F  ;
   va_x7 =  0.000841F  ;
   va_dur7 =  0.004823F  ;
   va_x8 =  0.000546F  ;
   va_dur8 =  0.009646F  ;
   va_x9 =  0.000870F  ;
   va_dur9 =  0.028939F  ;
   va_x10 =  0.000772F  ;
   va_dur10 =  0.004823F  ;
   va_x11 =  0.001081F  ;
   va_dur11 =  0.006431F  ;
   va_x12 =  0.000675F  ;
   va_dur12 =  0.022508F  ;
   va_x13 =  0.000987F  ;
   va_dur13 =  0.004823F  ;
   va_x14 =  0.000702F  ;
   va_dur14 =  0.025723F  ;
   va_x15 =  0.000579F  ;
   va_dur15 =  0.006431F  ;
   va_x16 =  0.000805F  ;
   va_dur16 =  0.004823F  ;
   va_x17 =  0.000489F  ;
   va_dur17 =  0.028939F  ;
   va_x18 =  0.000254F  ;
   va_dur18 =  0.072347F  ;
   va_x19 =  0.000047F  ;
   va_dur19 =  0.107717F  ;
   va_x20 =  0.000245F  ;
   va_dur20 =  2.102894F  ;
   va_x21 =  0.0F  ;
   ret = 0.0F;
   if (NVI(piano_kline19_first)>0)
   {
      NV(piano_kline19_t) += EV(KTIME);
      ret = (NV(piano_kline19_outT) += NV(piano_kline19_addK));
      if (NV(piano_kline19_t) > NV(piano_kline19_cdur))
       {
        while (NV(piano_kline19_t) > NV(piano_kline19_cdur))
         {
           NV(piano_kline19_t) -= NV(piano_kline19_cdur);
           switch(NVI(piano_kline19_first))
      {
         case 1:
         NV(piano_kline19_cdur) = va_dur2;
           NV(piano_kline19_clp) = NV(piano_kline19_crp);
           NV(piano_kline19_crp) = va_x3;
           break;
         case 2:
         NV(piano_kline19_cdur) = va_dur3;
           NV(piano_kline19_clp) = NV(piano_kline19_crp);
           NV(piano_kline19_crp) = va_x4;
           break;
         case 3:
         NV(piano_kline19_cdur) = va_dur4;
           NV(piano_kline19_clp) = NV(piano_kline19_crp);
           NV(piano_kline19_crp) = va_x5;
           break;
         case 4:
         NV(piano_kline19_cdur) = va_dur5;
           NV(piano_kline19_clp) = NV(piano_kline19_crp);
           NV(piano_kline19_crp) = va_x6;
           break;
         case 5:
         NV(piano_kline19_cdur) = va_dur6;
           NV(piano_kline19_clp) = NV(piano_kline19_crp);
           NV(piano_kline19_crp) = va_x7;
           break;
         case 6:
         NV(piano_kline19_cdur) = va_dur7;
           NV(piano_kline19_clp) = NV(piano_kline19_crp);
           NV(piano_kline19_crp) = va_x8;
           break;
         case 7:
         NV(piano_kline19_cdur) = va_dur8;
           NV(piano_kline19_clp) = NV(piano_kline19_crp);
           NV(piano_kline19_crp) = va_x9;
           break;
         case 8:
         NV(piano_kline19_cdur) = va_dur9;
           NV(piano_kline19_clp) = NV(piano_kline19_crp);
           NV(piano_kline19_crp) = va_x10;
           break;
         case 9:
         NV(piano_kline19_cdur) = va_dur10;
           NV(piano_kline19_clp) = NV(piano_kline19_crp);
           NV(piano_kline19_crp) = va_x11;
           break;
         case 10:
         NV(piano_kline19_cdur) = va_dur11;
           NV(piano_kline19_clp) = NV(piano_kline19_crp);
           NV(piano_kline19_crp) = va_x12;
           break;
         case 11:
         NV(piano_kline19_cdur) = va_dur12;
           NV(piano_kline19_clp) = NV(piano_kline19_crp);
           NV(piano_kline19_crp) = va_x13;
           break;
         case 12:
         NV(piano_kline19_cdur) = va_dur13;
           NV(piano_kline19_clp) = NV(piano_kline19_crp);
           NV(piano_kline19_crp) = va_x14;
           break;
         case 13:
         NV(piano_kline19_cdur) = va_dur14;
           NV(piano_kline19_clp) = NV(piano_kline19_crp);
           NV(piano_kline19_crp) = va_x15;
           break;
         case 14:
         NV(piano_kline19_cdur) = va_dur15;
           NV(piano_kline19_clp) = NV(piano_kline19_crp);
           NV(piano_kline19_crp) = va_x16;
           break;
         case 15:
         NV(piano_kline19_cdur) = va_dur16;
           NV(piano_kline19_clp) = NV(piano_kline19_crp);
           NV(piano_kline19_crp) = va_x17;
           break;
         case 16:
         NV(piano_kline19_cdur) = va_dur17;
           NV(piano_kline19_clp) = NV(piano_kline19_crp);
           NV(piano_kline19_crp) = va_x18;
           break;
         case 17:
         NV(piano_kline19_cdur) = va_dur18;
           NV(piano_kline19_clp) = NV(piano_kline19_crp);
           NV(piano_kline19_crp) = va_x19;
           break;
         case 18:
         NV(piano_kline19_cdur) = va_dur19;
           NV(piano_kline19_clp) = NV(piano_kline19_crp);
           NV(piano_kline19_crp) = va_x20;
           break;
         case 19:
         NV(piano_kline19_cdur) = va_dur20;
           NV(piano_kline19_clp) = NV(piano_kline19_crp);
           NV(piano_kline19_crp) = va_x21;
           break;
           default:
           NVI(piano_kline19_first) = -100;
           NV(piano_kline19_cdur) = NV(piano_kline19_t) + 10000.0F;
           break;
           }
         NVI(piano_kline19_first)++;
        }
        NV(piano_kline19_mult)=(NV(piano_kline19_crp) - NV(piano_kline19_clp))/NV(piano_kline19_cdur);
        ret = NV(piano_kline19_outT) = NV(piano_kline19_clp)+NV(piano_kline19_mult)*NV(piano_kline19_t);
        NV(piano_kline19_addK) = NV(piano_kline19_mult)*EV(KTIME);
        if (NVI(piano_kline19_first)<0)
          ret = 0.0F;
      }
   }
   if (NVI(piano_kline19_first)==0)
     {
       NVI(piano_kline19_first) = 1;
       ret = NV(piano_kline19_outT) = NV(piano_kline19_clp) = x1;
       NV(piano_kline19_crp) = x2;
       NV(piano_kline19_cdur) = dur1;
       if (dur1 > 0.0F)
         NV(piano_kline19_addK) = EV(KTIME)*((x2 - x1)/dur1);
     }
   return((NV(piano_kline19_return) = ret));

}



float piano__sym_kline20(ENGINE_PTR_DECLARE_COMMA struct ninstr_types * nstate)
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
   float va_dur20;
   float va_x21;
   float x2;
   float dur1;
   float x1;
   float ret;

   x1 =  0.0F ;
   dur1 =  0.001608F ;
   x2 =  0.000526F ;
   va_dur2 =  0.008039F  ;
   va_x3 =  0.001014F  ;
   va_dur3 =  0.008039F  ;
   va_x4 =  0.000609F  ;
   va_dur4 =  0.017685F  ;
   va_x5 =  0.001022F  ;
   va_dur5 =  0.008039F  ;
   va_x6 =  0.000581F  ;
   va_dur6 =  0.003215F  ;
   va_x7 =  0.001109F  ;
   va_dur7 =  0.008039F  ;
   va_x8 =  0.001350F  ;
   va_dur8 =  0.009646F  ;
   va_x9 =  0.000648F  ;
   va_dur9 =  0.019293F  ;
   va_x10 =  0.000923F  ;
   va_dur10 =  0.004823F  ;
   va_x11 =  0.000646F  ;
   va_dur11 =  0.006431F  ;
   va_x12 =  0.001035F  ;
   va_dur12 =  0.004823F  ;
   va_x13 =  0.000416F  ;
   va_dur13 =  0.003215F  ;
   va_x14 =  0.000826F  ;
   va_dur14 =  0.003215F  ;
   va_x15 =  0.000432F  ;
   va_dur15 =  0.001608F  ;
   va_x16 =  0.000704F  ;
   va_dur16 =  0.014469F  ;
   va_x17 =  0.000518F  ;
   va_dur17 =  0.008039F  ;
   va_x18 =  0.001072F  ;
   va_dur18 =  0.027331F  ;
   va_x19 =  0.000634F  ;
   va_dur19 =  0.368167F  ;
   va_x20 =  0.000007F  ;
   va_dur20 =  1.966238F  ;
   va_x21 =  0.0F  ;
   ret = 0.0F;
   if (NVI(piano_kline20_first)>0)
   {
      NV(piano_kline20_t) += EV(KTIME);
      ret = (NV(piano_kline20_outT) += NV(piano_kline20_addK));
      if (NV(piano_kline20_t) > NV(piano_kline20_cdur))
       {
        while (NV(piano_kline20_t) > NV(piano_kline20_cdur))
         {
           NV(piano_kline20_t) -= NV(piano_kline20_cdur);
           switch(NVI(piano_kline20_first))
      {
         case 1:
         NV(piano_kline20_cdur) = va_dur2;
           NV(piano_kline20_clp) = NV(piano_kline20_crp);
           NV(piano_kline20_crp) = va_x3;
           break;
         case 2:
         NV(piano_kline20_cdur) = va_dur3;
           NV(piano_kline20_clp) = NV(piano_kline20_crp);
           NV(piano_kline20_crp) = va_x4;
           break;
         case 3:
         NV(piano_kline20_cdur) = va_dur4;
           NV(piano_kline20_clp) = NV(piano_kline20_crp);
           NV(piano_kline20_crp) = va_x5;
           break;
         case 4:
         NV(piano_kline20_cdur) = va_dur5;
           NV(piano_kline20_clp) = NV(piano_kline20_crp);
           NV(piano_kline20_crp) = va_x6;
           break;
         case 5:
         NV(piano_kline20_cdur) = va_dur6;
           NV(piano_kline20_clp) = NV(piano_kline20_crp);
           NV(piano_kline20_crp) = va_x7;
           break;
         case 6:
         NV(piano_kline20_cdur) = va_dur7;
           NV(piano_kline20_clp) = NV(piano_kline20_crp);
           NV(piano_kline20_crp) = va_x8;
           break;
         case 7:
         NV(piano_kline20_cdur) = va_dur8;
           NV(piano_kline20_clp) = NV(piano_kline20_crp);
           NV(piano_kline20_crp) = va_x9;
           break;
         case 8:
         NV(piano_kline20_cdur) = va_dur9;
           NV(piano_kline20_clp) = NV(piano_kline20_crp);
           NV(piano_kline20_crp) = va_x10;
           break;
         case 9:
         NV(piano_kline20_cdur) = va_dur10;
           NV(piano_kline20_clp) = NV(piano_kline20_crp);
           NV(piano_kline20_crp) = va_x11;
           break;
         case 10:
         NV(piano_kline20_cdur) = va_dur11;
           NV(piano_kline20_clp) = NV(piano_kline20_crp);
           NV(piano_kline20_crp) = va_x12;
           break;
         case 11:
         NV(piano_kline20_cdur) = va_dur12;
           NV(piano_kline20_clp) = NV(piano_kline20_crp);
           NV(piano_kline20_crp) = va_x13;
           break;
         case 12:
         NV(piano_kline20_cdur) = va_dur13;
           NV(piano_kline20_clp) = NV(piano_kline20_crp);
           NV(piano_kline20_crp) = va_x14;
           break;
         case 13:
         NV(piano_kline20_cdur) = va_dur14;
           NV(piano_kline20_clp) = NV(piano_kline20_crp);
           NV(piano_kline20_crp) = va_x15;
           break;
         case 14:
         NV(piano_kline20_cdur) = va_dur15;
           NV(piano_kline20_clp) = NV(piano_kline20_crp);
           NV(piano_kline20_crp) = va_x16;
           break;
         case 15:
         NV(piano_kline20_cdur) = va_dur16;
           NV(piano_kline20_clp) = NV(piano_kline20_crp);
           NV(piano_kline20_crp) = va_x17;
           break;
         case 16:
         NV(piano_kline20_cdur) = va_dur17;
           NV(piano_kline20_clp) = NV(piano_kline20_crp);
           NV(piano_kline20_crp) = va_x18;
           break;
         case 17:
         NV(piano_kline20_cdur) = va_dur18;
           NV(piano_kline20_clp) = NV(piano_kline20_crp);
           NV(piano_kline20_crp) = va_x19;
           break;
         case 18:
         NV(piano_kline20_cdur) = va_dur19;
           NV(piano_kline20_clp) = NV(piano_kline20_crp);
           NV(piano_kline20_crp) = va_x20;
           break;
         case 19:
         NV(piano_kline20_cdur) = va_dur20;
           NV(piano_kline20_clp) = NV(piano_kline20_crp);
           NV(piano_kline20_crp) = va_x21;
           break;
           default:
           NVI(piano_kline20_first) = -100;
           NV(piano_kline20_cdur) = NV(piano_kline20_t) + 10000.0F;
           break;
           }
         NVI(piano_kline20_first)++;
        }
        NV(piano_kline20_mult)=(NV(piano_kline20_crp) - NV(piano_kline20_clp))/NV(piano_kline20_cdur);
        ret = NV(piano_kline20_outT) = NV(piano_kline20_clp)+NV(piano_kline20_mult)*NV(piano_kline20_t);
        NV(piano_kline20_addK) = NV(piano_kline20_mult)*EV(KTIME);
        if (NVI(piano_kline20_first)<0)
          ret = 0.0F;
      }
   }
   if (NVI(piano_kline20_first)==0)
     {
       NVI(piano_kline20_first) = 1;
       ret = NV(piano_kline20_outT) = NV(piano_kline20_clp) = x1;
       NV(piano_kline20_crp) = x2;
       NV(piano_kline20_cdur) = dur1;
       if (dur1 > 0.0F)
         NV(piano_kline20_addK) = EV(KTIME)*((x2 - x1)/dur1);
     }
   return((NV(piano_kline20_return) = ret));

}



float piano__sym_kline21(ENGINE_PTR_DECLARE_COMMA struct ninstr_types * nstate)
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
   float va_dur20;
   float va_x21;
   float x2;
   float dur1;
   float x1;
   float ret;

   x1 =  0.0F ;
   dur1 =  0.009646F ;
   x2 =  0.000087F ;
   va_dur2 =  0.022508F  ;
   va_x3 =  0.000452F  ;
   va_dur3 =  0.001608F  ;
   va_x4 =  0.001202F  ;
   va_dur4 =  0.004823F  ;
   va_x5 =  0.000134F  ;
   va_dur5 =  0.003215F  ;
   va_x6 =  0.000999F  ;
   va_dur6 =  0.006431F  ;
   va_x7 =  0.000277F  ;
   va_dur7 =  0.001608F  ;
   va_x8 =  0.000954F  ;
   va_dur8 =  0.001608F  ;
   va_x9 =  0.000103F  ;
   va_dur9 =  0.003215F  ;
   va_x10 =  0.000831F  ;
   va_dur10 =  0.001608F  ;
   va_x11 =  0.000026F  ;
   va_dur11 =  0.001608F  ;
   va_x12 =  0.000766F  ;
   va_dur12 =  0.008039F  ;
   va_x13 =  0.000836F  ;
   va_dur13 =  0.011254F  ;
   va_x14 =  0.000145F  ;
   va_dur14 =  0.001608F  ;
   va_x15 =  0.000979F  ;
   va_dur15 =  0.011254F  ;
   va_x16 =  0.000367F  ;
   va_dur16 =  0.001608F  ;
   va_x17 =  0.001238F  ;
   va_dur17 =  0.003215F  ;
   va_x18 =  0.000142F  ;
   va_dur18 =  0.004823F  ;
   va_x19 =  0.000785F  ;
   va_dur19 =  0.125402F  ;
   va_x20 =  0.000066F  ;
   va_dur20 =  2.263666F  ;
   va_x21 =  0.0F  ;
   ret = 0.0F;
   if (NVI(piano_kline21_first)>0)
   {
      NV(piano_kline21_t) += EV(KTIME);
      ret = (NV(piano_kline21_outT) += NV(piano_kline21_addK));
      if (NV(piano_kline21_t) > NV(piano_kline21_cdur))
       {
        while (NV(piano_kline21_t) > NV(piano_kline21_cdur))
         {
           NV(piano_kline21_t) -= NV(piano_kline21_cdur);
           switch(NVI(piano_kline21_first))
      {
         case 1:
         NV(piano_kline21_cdur) = va_dur2;
           NV(piano_kline21_clp) = NV(piano_kline21_crp);
           NV(piano_kline21_crp) = va_x3;
           break;
         case 2:
         NV(piano_kline21_cdur) = va_dur3;
           NV(piano_kline21_clp) = NV(piano_kline21_crp);
           NV(piano_kline21_crp) = va_x4;
           break;
         case 3:
         NV(piano_kline21_cdur) = va_dur4;
           NV(piano_kline21_clp) = NV(piano_kline21_crp);
           NV(piano_kline21_crp) = va_x5;
           break;
         case 4:
         NV(piano_kline21_cdur) = va_dur5;
           NV(piano_kline21_clp) = NV(piano_kline21_crp);
           NV(piano_kline21_crp) = va_x6;
           break;
         case 5:
         NV(piano_kline21_cdur) = va_dur6;
           NV(piano_kline21_clp) = NV(piano_kline21_crp);
           NV(piano_kline21_crp) = va_x7;
           break;
         case 6:
         NV(piano_kline21_cdur) = va_dur7;
           NV(piano_kline21_clp) = NV(piano_kline21_crp);
           NV(piano_kline21_crp) = va_x8;
           break;
         case 7:
         NV(piano_kline21_cdur) = va_dur8;
           NV(piano_kline21_clp) = NV(piano_kline21_crp);
           NV(piano_kline21_crp) = va_x9;
           break;
         case 8:
         NV(piano_kline21_cdur) = va_dur9;
           NV(piano_kline21_clp) = NV(piano_kline21_crp);
           NV(piano_kline21_crp) = va_x10;
           break;
         case 9:
         NV(piano_kline21_cdur) = va_dur10;
           NV(piano_kline21_clp) = NV(piano_kline21_crp);
           NV(piano_kline21_crp) = va_x11;
           break;
         case 10:
         NV(piano_kline21_cdur) = va_dur11;
           NV(piano_kline21_clp) = NV(piano_kline21_crp);
           NV(piano_kline21_crp) = va_x12;
           break;
         case 11:
         NV(piano_kline21_cdur) = va_dur12;
           NV(piano_kline21_clp) = NV(piano_kline21_crp);
           NV(piano_kline21_crp) = va_x13;
           break;
         case 12:
         NV(piano_kline21_cdur) = va_dur13;
           NV(piano_kline21_clp) = NV(piano_kline21_crp);
           NV(piano_kline21_crp) = va_x14;
           break;
         case 13:
         NV(piano_kline21_cdur) = va_dur14;
           NV(piano_kline21_clp) = NV(piano_kline21_crp);
           NV(piano_kline21_crp) = va_x15;
           break;
         case 14:
         NV(piano_kline21_cdur) = va_dur15;
           NV(piano_kline21_clp) = NV(piano_kline21_crp);
           NV(piano_kline21_crp) = va_x16;
           break;
         case 15:
         NV(piano_kline21_cdur) = va_dur16;
           NV(piano_kline21_clp) = NV(piano_kline21_crp);
           NV(piano_kline21_crp) = va_x17;
           break;
         case 16:
         NV(piano_kline21_cdur) = va_dur17;
           NV(piano_kline21_clp) = NV(piano_kline21_crp);
           NV(piano_kline21_crp) = va_x18;
           break;
         case 17:
         NV(piano_kline21_cdur) = va_dur18;
           NV(piano_kline21_clp) = NV(piano_kline21_crp);
           NV(piano_kline21_crp) = va_x19;
           break;
         case 18:
         NV(piano_kline21_cdur) = va_dur19;
           NV(piano_kline21_clp) = NV(piano_kline21_crp);
           NV(piano_kline21_crp) = va_x20;
           break;
         case 19:
         NV(piano_kline21_cdur) = va_dur20;
           NV(piano_kline21_clp) = NV(piano_kline21_crp);
           NV(piano_kline21_crp) = va_x21;
           break;
           default:
           NVI(piano_kline21_first) = -100;
           NV(piano_kline21_cdur) = NV(piano_kline21_t) + 10000.0F;
           break;
           }
         NVI(piano_kline21_first)++;
        }
        NV(piano_kline21_mult)=(NV(piano_kline21_crp) - NV(piano_kline21_clp))/NV(piano_kline21_cdur);
        ret = NV(piano_kline21_outT) = NV(piano_kline21_clp)+NV(piano_kline21_mult)*NV(piano_kline21_t);
        NV(piano_kline21_addK) = NV(piano_kline21_mult)*EV(KTIME);
        if (NVI(piano_kline21_first)<0)
          ret = 0.0F;
      }
   }
   if (NVI(piano_kline21_first)==0)
     {
       NVI(piano_kline21_first) = 1;
       ret = NV(piano_kline21_outT) = NV(piano_kline21_clp) = x1;
       NV(piano_kline21_crp) = x2;
       NV(piano_kline21_cdur) = dur1;
       if (dur1 > 0.0F)
         NV(piano_kline21_addK) = EV(KTIME)*((x2 - x1)/dur1);
     }
   return((NV(piano_kline21_return) = ret));

}



float piano__sym_kline22(ENGINE_PTR_DECLARE_COMMA struct ninstr_types * nstate)
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
   float va_dur20;
   float va_x21;
   float x2;
   float dur1;
   float x1;
   float ret;

   x1 =  0.0F ;
   dur1 =  0.001608F ;
   x2 =  0.001299F ;
   va_dur2 =  0.001608F  ;
   va_x3 =  0.000842F  ;
   va_dur3 =  0.011254F  ;
   va_x4 =  0.000420F  ;
   va_dur4 =  0.014469F  ;
   va_x5 =  0.001146F  ;
   va_dur5 =  0.012862F  ;
   va_x6 =  0.000399F  ;
   va_dur6 =  0.003215F  ;
   va_x7 =  0.000954F  ;
   va_dur7 =  0.003215F  ;
   va_x8 =  0.000552F  ;
   va_dur8 =  0.001608F  ;
   va_x9 =  0.001107F  ;
   va_dur9 =  0.003215F  ;
   va_x10 =  0.000734F  ;
   va_dur10 =  0.004823F  ;
   va_x11 =  0.001235F  ;
   va_dur11 =  0.003215F  ;
   va_x12 =  0.000826F  ;
   va_dur12 =  0.022508F  ;
   va_x13 =  0.001213F  ;
   va_dur13 =  0.001608F  ;
   va_x14 =  0.000810F  ;
   va_dur14 =  0.001608F  ;
   va_x15 =  0.001179F  ;
   va_dur15 =  0.008039F  ;
   va_x16 =  0.000938F  ;
   va_dur16 =  0.003215F  ;
   va_x17 =  0.000363F  ;
   va_dur17 =  0.024116F  ;
   va_x18 =  0.000796F  ;
   va_dur18 =  0.035370F  ;
   va_x19 =  0.000144F  ;
   va_dur19 =  0.143087F  ;
   va_x20 =  0.000005F  ;
   va_dur20 =  2.191318F  ;
   va_x21 =  0.0F  ;
   ret = 0.0F;
   if (NVI(piano_kline22_first)>0)
   {
      NV(piano_kline22_t) += EV(KTIME);
      ret = (NV(piano_kline22_outT) += NV(piano_kline22_addK));
      if (NV(piano_kline22_t) > NV(piano_kline22_cdur))
       {
        while (NV(piano_kline22_t) > NV(piano_kline22_cdur))
         {
           NV(piano_kline22_t) -= NV(piano_kline22_cdur);
           switch(NVI(piano_kline22_first))
      {
         case 1:
         NV(piano_kline22_cdur) = va_dur2;
           NV(piano_kline22_clp) = NV(piano_kline22_crp);
           NV(piano_kline22_crp) = va_x3;
           break;
         case 2:
         NV(piano_kline22_cdur) = va_dur3;
           NV(piano_kline22_clp) = NV(piano_kline22_crp);
           NV(piano_kline22_crp) = va_x4;
           break;
         case 3:
         NV(piano_kline22_cdur) = va_dur4;
           NV(piano_kline22_clp) = NV(piano_kline22_crp);
           NV(piano_kline22_crp) = va_x5;
           break;
         case 4:
         NV(piano_kline22_cdur) = va_dur5;
           NV(piano_kline22_clp) = NV(piano_kline22_crp);
           NV(piano_kline22_crp) = va_x6;
           break;
         case 5:
         NV(piano_kline22_cdur) = va_dur6;
           NV(piano_kline22_clp) = NV(piano_kline22_crp);
           NV(piano_kline22_crp) = va_x7;
           break;
         case 6:
         NV(piano_kline22_cdur) = va_dur7;
           NV(piano_kline22_clp) = NV(piano_kline22_crp);
           NV(piano_kline22_crp) = va_x8;
           break;
         case 7:
         NV(piano_kline22_cdur) = va_dur8;
           NV(piano_kline22_clp) = NV(piano_kline22_crp);
           NV(piano_kline22_crp) = va_x9;
           break;
         case 8:
         NV(piano_kline22_cdur) = va_dur9;
           NV(piano_kline22_clp) = NV(piano_kline22_crp);
           NV(piano_kline22_crp) = va_x10;
           break;
         case 9:
         NV(piano_kline22_cdur) = va_dur10;
           NV(piano_kline22_clp) = NV(piano_kline22_crp);
           NV(piano_kline22_crp) = va_x11;
           break;
         case 10:
         NV(piano_kline22_cdur) = va_dur11;
           NV(piano_kline22_clp) = NV(piano_kline22_crp);
           NV(piano_kline22_crp) = va_x12;
           break;
         case 11:
         NV(piano_kline22_cdur) = va_dur12;
           NV(piano_kline22_clp) = NV(piano_kline22_crp);
           NV(piano_kline22_crp) = va_x13;
           break;
         case 12:
         NV(piano_kline22_cdur) = va_dur13;
           NV(piano_kline22_clp) = NV(piano_kline22_crp);
           NV(piano_kline22_crp) = va_x14;
           break;
         case 13:
         NV(piano_kline22_cdur) = va_dur14;
           NV(piano_kline22_clp) = NV(piano_kline22_crp);
           NV(piano_kline22_crp) = va_x15;
           break;
         case 14:
         NV(piano_kline22_cdur) = va_dur15;
           NV(piano_kline22_clp) = NV(piano_kline22_crp);
           NV(piano_kline22_crp) = va_x16;
           break;
         case 15:
         NV(piano_kline22_cdur) = va_dur16;
           NV(piano_kline22_clp) = NV(piano_kline22_crp);
           NV(piano_kline22_crp) = va_x17;
           break;
         case 16:
         NV(piano_kline22_cdur) = va_dur17;
           NV(piano_kline22_clp) = NV(piano_kline22_crp);
           NV(piano_kline22_crp) = va_x18;
           break;
         case 17:
         NV(piano_kline22_cdur) = va_dur18;
           NV(piano_kline22_clp) = NV(piano_kline22_crp);
           NV(piano_kline22_crp) = va_x19;
           break;
         case 18:
         NV(piano_kline22_cdur) = va_dur19;
           NV(piano_kline22_clp) = NV(piano_kline22_crp);
           NV(piano_kline22_crp) = va_x20;
           break;
         case 19:
         NV(piano_kline22_cdur) = va_dur20;
           NV(piano_kline22_clp) = NV(piano_kline22_crp);
           NV(piano_kline22_crp) = va_x21;
           break;
           default:
           NVI(piano_kline22_first) = -100;
           NV(piano_kline22_cdur) = NV(piano_kline22_t) + 10000.0F;
           break;
           }
         NVI(piano_kline22_first)++;
        }
        NV(piano_kline22_mult)=(NV(piano_kline22_crp) - NV(piano_kline22_clp))/NV(piano_kline22_cdur);
        ret = NV(piano_kline22_outT) = NV(piano_kline22_clp)+NV(piano_kline22_mult)*NV(piano_kline22_t);
        NV(piano_kline22_addK) = NV(piano_kline22_mult)*EV(KTIME);
        if (NVI(piano_kline22_first)<0)
          ret = 0.0F;
      }
   }
   if (NVI(piano_kline22_first)==0)
     {
       NVI(piano_kline22_first) = 1;
       ret = NV(piano_kline22_outT) = NV(piano_kline22_clp) = x1;
       NV(piano_kline22_crp) = x2;
       NV(piano_kline22_cdur) = dur1;
       if (dur1 > 0.0F)
         NV(piano_kline22_addK) = EV(KTIME)*((x2 - x1)/dur1);
     }
   return((NV(piano_kline22_return) = ret));

}



float piano__sym_kline23(ENGINE_PTR_DECLARE_COMMA struct ninstr_types * nstate)
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
   float va_dur20;
   float va_x21;
   float x2;
   float dur1;
   float x1;
   float ret;

   x1 =  0.0F ;
   dur1 =  0.004823F ;
   x2 =  0.000600F ;
   va_dur2 =  0.004823F  ;
   va_x3 =  0.000153F  ;
   va_dur3 =  0.040193F  ;
   va_x4 =  0.002135F  ;
   va_dur4 =  0.009646F  ;
   va_x5 =  0.002063F  ;
   va_dur5 =  0.008039F  ;
   va_x6 =  0.002378F  ;
   va_dur6 =  0.001608F  ;
   va_x7 =  0.002162F  ;
   va_dur7 =  0.006431F  ;
   va_x8 =  0.002435F  ;
   va_dur8 =  0.003215F  ;
   va_x9 =  0.002069F  ;
   va_dur9 =  0.006431F  ;
   va_x10 =  0.002222F  ;
   va_dur10 =  0.006431F  ;
   va_x11 =  0.001675F  ;
   va_dur11 =  0.009646F  ;
   va_x12 =  0.001701F  ;
   va_dur12 =  0.020900F  ;
   va_x13 =  0.000717F  ;
   va_dur13 =  0.020900F  ;
   va_x14 =  0.000180F  ;
   va_dur14 =  0.038585F  ;
   va_x15 =  0.000278F  ;
   va_dur15 =  0.036977F  ;
   va_x16 =  0.000030F  ;
   va_dur16 =  0.059486F  ;
   va_x17 =  0.000037F  ;
   va_dur17 =  0.061093F  ;
   va_x18 =  0.000450F  ;
   va_dur18 =  0.072347F  ;
   va_x19 =  0.000125F  ;
   va_dur19 =  0.176849F  ;
   va_x20 =  0.000009F  ;
   va_dur20 =  1.903537F  ;
   va_x21 =  0.0F  ;
   ret = 0.0F;
   if (NVI(piano_kline23_first)>0)
   {
      NV(piano_kline23_t) += EV(KTIME);
      ret = (NV(piano_kline23_outT) += NV(piano_kline23_addK));
      if (NV(piano_kline23_t) > NV(piano_kline23_cdur))
       {
        while (NV(piano_kline23_t) > NV(piano_kline23_cdur))
         {
           NV(piano_kline23_t) -= NV(piano_kline23_cdur);
           switch(NVI(piano_kline23_first))
      {
         case 1:
         NV(piano_kline23_cdur) = va_dur2;
           NV(piano_kline23_clp) = NV(piano_kline23_crp);
           NV(piano_kline23_crp) = va_x3;
           break;
         case 2:
         NV(piano_kline23_cdur) = va_dur3;
           NV(piano_kline23_clp) = NV(piano_kline23_crp);
           NV(piano_kline23_crp) = va_x4;
           break;
         case 3:
         NV(piano_kline23_cdur) = va_dur4;
           NV(piano_kline23_clp) = NV(piano_kline23_crp);
           NV(piano_kline23_crp) = va_x5;
           break;
         case 4:
         NV(piano_kline23_cdur) = va_dur5;
           NV(piano_kline23_clp) = NV(piano_kline23_crp);
           NV(piano_kline23_crp) = va_x6;
           break;
         case 5:
         NV(piano_kline23_cdur) = va_dur6;
           NV(piano_kline23_clp) = NV(piano_kline23_crp);
           NV(piano_kline23_crp) = va_x7;
           break;
         case 6:
         NV(piano_kline23_cdur) = va_dur7;
           NV(piano_kline23_clp) = NV(piano_kline23_crp);
           NV(piano_kline23_crp) = va_x8;
           break;
         case 7:
         NV(piano_kline23_cdur) = va_dur8;
           NV(piano_kline23_clp) = NV(piano_kline23_crp);
           NV(piano_kline23_crp) = va_x9;
           break;
         case 8:
         NV(piano_kline23_cdur) = va_dur9;
           NV(piano_kline23_clp) = NV(piano_kline23_crp);
           NV(piano_kline23_crp) = va_x10;
           break;
         case 9:
         NV(piano_kline23_cdur) = va_dur10;
           NV(piano_kline23_clp) = NV(piano_kline23_crp);
           NV(piano_kline23_crp) = va_x11;
           break;
         case 10:
         NV(piano_kline23_cdur) = va_dur11;
           NV(piano_kline23_clp) = NV(piano_kline23_crp);
           NV(piano_kline23_crp) = va_x12;
           break;
         case 11:
         NV(piano_kline23_cdur) = va_dur12;
           NV(piano_kline23_clp) = NV(piano_kline23_crp);
           NV(piano_kline23_crp) = va_x13;
           break;
         case 12:
         NV(piano_kline23_cdur) = va_dur13;
           NV(piano_kline23_clp) = NV(piano_kline23_crp);
           NV(piano_kline23_crp) = va_x14;
           break;
         case 13:
         NV(piano_kline23_cdur) = va_dur14;
           NV(piano_kline23_clp) = NV(piano_kline23_crp);
           NV(piano_kline23_crp) = va_x15;
           break;
         case 14:
         NV(piano_kline23_cdur) = va_dur15;
           NV(piano_kline23_clp) = NV(piano_kline23_crp);
           NV(piano_kline23_crp) = va_x16;
           break;
         case 15:
         NV(piano_kline23_cdur) = va_dur16;
           NV(piano_kline23_clp) = NV(piano_kline23_crp);
           NV(piano_kline23_crp) = va_x17;
           break;
         case 16:
         NV(piano_kline23_cdur) = va_dur17;
           NV(piano_kline23_clp) = NV(piano_kline23_crp);
           NV(piano_kline23_crp) = va_x18;
           break;
         case 17:
         NV(piano_kline23_cdur) = va_dur18;
           NV(piano_kline23_clp) = NV(piano_kline23_crp);
           NV(piano_kline23_crp) = va_x19;
           break;
         case 18:
         NV(piano_kline23_cdur) = va_dur19;
           NV(piano_kline23_clp) = NV(piano_kline23_crp);
           NV(piano_kline23_crp) = va_x20;
           break;
         case 19:
         NV(piano_kline23_cdur) = va_dur20;
           NV(piano_kline23_clp) = NV(piano_kline23_crp);
           NV(piano_kline23_crp) = va_x21;
           break;
           default:
           NVI(piano_kline23_first) = -100;
           NV(piano_kline23_cdur) = NV(piano_kline23_t) + 10000.0F;
           break;
           }
         NVI(piano_kline23_first)++;
        }
        NV(piano_kline23_mult)=(NV(piano_kline23_crp) - NV(piano_kline23_clp))/NV(piano_kline23_cdur);
        ret = NV(piano_kline23_outT) = NV(piano_kline23_clp)+NV(piano_kline23_mult)*NV(piano_kline23_t);
        NV(piano_kline23_addK) = NV(piano_kline23_mult)*EV(KTIME);
        if (NVI(piano_kline23_first)<0)
          ret = 0.0F;
      }
   }
   if (NVI(piano_kline23_first)==0)
     {
       NVI(piano_kline23_first) = 1;
       ret = NV(piano_kline23_outT) = NV(piano_kline23_clp) = x1;
       NV(piano_kline23_crp) = x2;
       NV(piano_kline23_cdur) = dur1;
       if (dur1 > 0.0F)
         NV(piano_kline23_addK) = EV(KTIME)*((x2 - x1)/dur1);
     }
   return((NV(piano_kline23_return) = ret));

}



float piano__sym_kline24(ENGINE_PTR_DECLARE_COMMA struct ninstr_types * nstate)
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
   float va_dur20;
   float va_x21;
   float x2;
   float dur1;
   float x1;
   float ret;

   x1 =  0.0F ;
   dur1 =  0.003215F ;
   x2 =  0.001891F ;
   va_dur2 =  0.012862F  ;
   va_x3 =  0.000834F  ;
   va_dur3 =  0.001608F  ;
   va_x4 =  0.000916F  ;
   va_dur4 =  0.006431F  ;
   va_x5 =  0.000468F  ;
   va_dur5 =  0.017685F  ;
   va_x6 =  0.000498F  ;
   va_dur6 =  0.004823F  ;
   va_x7 =  0.000669F  ;
   va_dur7 =  0.012862F  ;
   va_x8 =  0.000462F  ;
   va_dur8 =  0.006431F  ;
   va_x9 =  0.000652F  ;
   va_dur9 =  0.004823F  ;
   va_x10 =  0.000338F  ;
   va_dur10 =  0.028939F  ;
   va_x11 =  0.001070F  ;
   va_dur11 =  0.012862F  ;
   va_x12 =  0.001196F  ;
   va_dur12 =  0.051447F  ;
   va_x13 =  0.000136F  ;
   va_dur13 =  0.014469F  ;
   va_x14 =  0.000054F  ;
   va_dur14 =  0.016077F  ;
   va_x15 =  0.000056F  ;
   va_dur15 =  0.053055F  ;
   va_x16 =  0.000697F  ;
   va_dur16 =  0.067524F  ;
   va_x17 =  0.000183F  ;
   va_dur17 =  0.080386F  ;
   va_x18 =  0.000331F  ;
   va_dur18 =  0.049839F  ;
   va_x19 =  0.000106F  ;
   va_dur19 =  0.205788F  ;
   va_x20 =  0.000017F  ;
   va_dur20 =  1.840836F  ;
   va_x21 =  0.0F  ;
   ret = 0.0F;
   if (NVI(piano_kline24_first)>0)
   {
      NV(piano_kline24_t) += EV(KTIME);
      ret = (NV(piano_kline24_outT) += NV(piano_kline24_addK));
      if (NV(piano_kline24_t) > NV(piano_kline24_cdur))
       {
        while (NV(piano_kline24_t) > NV(piano_kline24_cdur))
         {
           NV(piano_kline24_t) -= NV(piano_kline24_cdur);
           switch(NVI(piano_kline24_first))
      {
         case 1:
         NV(piano_kline24_cdur) = va_dur2;
           NV(piano_kline24_clp) = NV(piano_kline24_crp);
           NV(piano_kline24_crp) = va_x3;
           break;
         case 2:
         NV(piano_kline24_cdur) = va_dur3;
           NV(piano_kline24_clp) = NV(piano_kline24_crp);
           NV(piano_kline24_crp) = va_x4;
           break;
         case 3:
         NV(piano_kline24_cdur) = va_dur4;
           NV(piano_kline24_clp) = NV(piano_kline24_crp);
           NV(piano_kline24_crp) = va_x5;
           break;
         case 4:
         NV(piano_kline24_cdur) = va_dur5;
           NV(piano_kline24_clp) = NV(piano_kline24_crp);
           NV(piano_kline24_crp) = va_x6;
           break;
         case 5:
         NV(piano_kline24_cdur) = va_dur6;
           NV(piano_kline24_clp) = NV(piano_kline24_crp);
           NV(piano_kline24_crp) = va_x7;
           break;
         case 6:
         NV(piano_kline24_cdur) = va_dur7;
           NV(piano_kline24_clp) = NV(piano_kline24_crp);
           NV(piano_kline24_crp) = va_x8;
           break;
         case 7:
         NV(piano_kline24_cdur) = va_dur8;
           NV(piano_kline24_clp) = NV(piano_kline24_crp);
           NV(piano_kline24_crp) = va_x9;
           break;
         case 8:
         NV(piano_kline24_cdur) = va_dur9;
           NV(piano_kline24_clp) = NV(piano_kline24_crp);
           NV(piano_kline24_crp) = va_x10;
           break;
         case 9:
         NV(piano_kline24_cdur) = va_dur10;
           NV(piano_kline24_clp) = NV(piano_kline24_crp);
           NV(piano_kline24_crp) = va_x11;
           break;
         case 10:
         NV(piano_kline24_cdur) = va_dur11;
           NV(piano_kline24_clp) = NV(piano_kline24_crp);
           NV(piano_kline24_crp) = va_x12;
           break;
         case 11:
         NV(piano_kline24_cdur) = va_dur12;
           NV(piano_kline24_clp) = NV(piano_kline24_crp);
           NV(piano_kline24_crp) = va_x13;
           break;
         case 12:
         NV(piano_kline24_cdur) = va_dur13;
           NV(piano_kline24_clp) = NV(piano_kline24_crp);
           NV(piano_kline24_crp) = va_x14;
           break;
         case 13:
         NV(piano_kline24_cdur) = va_dur14;
           NV(piano_kline24_clp) = NV(piano_kline24_crp);
           NV(piano_kline24_crp) = va_x15;
           break;
         case 14:
         NV(piano_kline24_cdur) = va_dur15;
           NV(piano_kline24_clp) = NV(piano_kline24_crp);
           NV(piano_kline24_crp) = va_x16;
           break;
         case 15:
         NV(piano_kline24_cdur) = va_dur16;
           NV(piano_kline24_clp) = NV(piano_kline24_crp);
           NV(piano_kline24_crp) = va_x17;
           break;
         case 16:
         NV(piano_kline24_cdur) = va_dur17;
           NV(piano_kline24_clp) = NV(piano_kline24_crp);
           NV(piano_kline24_crp) = va_x18;
           break;
         case 17:
         NV(piano_kline24_cdur) = va_dur18;
           NV(piano_kline24_clp) = NV(piano_kline24_crp);
           NV(piano_kline24_crp) = va_x19;
           break;
         case 18:
         NV(piano_kline24_cdur) = va_dur19;
           NV(piano_kline24_clp) = NV(piano_kline24_crp);
           NV(piano_kline24_crp) = va_x20;
           break;
         case 19:
         NV(piano_kline24_cdur) = va_dur20;
           NV(piano_kline24_clp) = NV(piano_kline24_crp);
           NV(piano_kline24_crp) = va_x21;
           break;
           default:
           NVI(piano_kline24_first) = -100;
           NV(piano_kline24_cdur) = NV(piano_kline24_t) + 10000.0F;
           break;
           }
         NVI(piano_kline24_first)++;
        }
        NV(piano_kline24_mult)=(NV(piano_kline24_crp) - NV(piano_kline24_clp))/NV(piano_kline24_cdur);
        ret = NV(piano_kline24_outT) = NV(piano_kline24_clp)+NV(piano_kline24_mult)*NV(piano_kline24_t);
        NV(piano_kline24_addK) = NV(piano_kline24_mult)*EV(KTIME);
        if (NVI(piano_kline24_first)<0)
          ret = 0.0F;
      }
   }
   if (NVI(piano_kline24_first)==0)
     {
       NVI(piano_kline24_first) = 1;
       ret = NV(piano_kline24_outT) = NV(piano_kline24_clp) = x1;
       NV(piano_kline24_crp) = x2;
       NV(piano_kline24_cdur) = dur1;
       if (dur1 > 0.0F)
         NV(piano_kline24_addK) = EV(KTIME)*((x2 - x1)/dur1);
     }
   return((NV(piano_kline24_return) = ret));

}



float piano__sym_kline25(ENGINE_PTR_DECLARE_COMMA struct ninstr_types * nstate)
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
   float va_dur20;
   float va_x21;
   float x2;
   float dur1;
   float x1;
   float ret;

   x1 =  0.0F ;
   dur1 =  0.001608F ;
   x2 =  0.000396F ;
   va_dur2 =  0.004823F  ;
   va_x3 =  0.000054F  ;
   va_dur3 =  0.003215F  ;
   va_x4 =  0.000381F  ;
   va_dur4 =  0.001608F  ;
   va_x5 =  0.000129F  ;
   va_dur5 =  0.001608F  ;
   va_x6 =  0.000374F  ;
   va_dur6 =  0.003215F  ;
   va_x7 =  0.000385F  ;
   va_dur7 =  0.012862F  ;
   va_x8 =  0.000153F  ;
   va_dur8 =  0.006431F  ;
   va_x9 =  0.000453F  ;
   va_dur9 =  0.003215F  ;
   va_x10 =  0.000204F  ;
   va_dur10 =  0.001608F  ;
   va_x11 =  0.000466F  ;
   va_dur11 =  0.001608F  ;
   va_x12 =  0.000266F  ;
   va_dur12 =  0.008039F  ;
   va_x13 =  0.000418F  ;
   va_dur13 =  0.009646F  ;
   va_x14 =  0.000121F  ;
   va_dur14 =  0.003215F  ;
   va_x15 =  0.000264F  ;
   va_dur15 =  0.032154F  ;
   va_x16 =  0.000028F  ;
   va_dur16 =  0.004823F  ;
   va_x17 =  0.000265F  ;
   va_dur17 =  0.016077F  ;
   va_x18 =  0.000016F  ;
   va_dur18 =  0.012862F  ;
   va_x19 =  0.000169F  ;
   va_dur19 =  0.067524F  ;
   va_x20 =  0.000007F  ;
   va_dur20 =  2.295820F  ;
   va_x21 =  0.0F  ;
   ret = 0.0F;
   if (NVI(piano_kline25_first)>0)
   {
      NV(piano_kline25_t) += EV(KTIME);
      ret = (NV(piano_kline25_outT) += NV(piano_kline25_addK));
      if (NV(piano_kline25_t) > NV(piano_kline25_cdur))
       {
        while (NV(piano_kline25_t) > NV(piano_kline25_cdur))
         {
           NV(piano_kline25_t) -= NV(piano_kline25_cdur);
           switch(NVI(piano_kline25_first))
      {
         case 1:
         NV(piano_kline25_cdur) = va_dur2;
           NV(piano_kline25_clp) = NV(piano_kline25_crp);
           NV(piano_kline25_crp) = va_x3;
           break;
         case 2:
         NV(piano_kline25_cdur) = va_dur3;
           NV(piano_kline25_clp) = NV(piano_kline25_crp);
           NV(piano_kline25_crp) = va_x4;
           break;
         case 3:
         NV(piano_kline25_cdur) = va_dur4;
           NV(piano_kline25_clp) = NV(piano_kline25_crp);
           NV(piano_kline25_crp) = va_x5;
           break;
         case 4:
         NV(piano_kline25_cdur) = va_dur5;
           NV(piano_kline25_clp) = NV(piano_kline25_crp);
           NV(piano_kline25_crp) = va_x6;
           break;
         case 5:
         NV(piano_kline25_cdur) = va_dur6;
           NV(piano_kline25_clp) = NV(piano_kline25_crp);
           NV(piano_kline25_crp) = va_x7;
           break;
         case 6:
         NV(piano_kline25_cdur) = va_dur7;
           NV(piano_kline25_clp) = NV(piano_kline25_crp);
           NV(piano_kline25_crp) = va_x8;
           break;
         case 7:
         NV(piano_kline25_cdur) = va_dur8;
           NV(piano_kline25_clp) = NV(piano_kline25_crp);
           NV(piano_kline25_crp) = va_x9;
           break;
         case 8:
         NV(piano_kline25_cdur) = va_dur9;
           NV(piano_kline25_clp) = NV(piano_kline25_crp);
           NV(piano_kline25_crp) = va_x10;
           break;
         case 9:
         NV(piano_kline25_cdur) = va_dur10;
           NV(piano_kline25_clp) = NV(piano_kline25_crp);
           NV(piano_kline25_crp) = va_x11;
           break;
         case 10:
         NV(piano_kline25_cdur) = va_dur11;
           NV(piano_kline25_clp) = NV(piano_kline25_crp);
           NV(piano_kline25_crp) = va_x12;
           break;
         case 11:
         NV(piano_kline25_cdur) = va_dur12;
           NV(piano_kline25_clp) = NV(piano_kline25_crp);
           NV(piano_kline25_crp) = va_x13;
           break;
         case 12:
         NV(piano_kline25_cdur) = va_dur13;
           NV(piano_kline25_clp) = NV(piano_kline25_crp);
           NV(piano_kline25_crp) = va_x14;
           break;
         case 13:
         NV(piano_kline25_cdur) = va_dur14;
           NV(piano_kline25_clp) = NV(piano_kline25_crp);
           NV(piano_kline25_crp) = va_x15;
           break;
         case 14:
         NV(piano_kline25_cdur) = va_dur15;
           NV(piano_kline25_clp) = NV(piano_kline25_crp);
           NV(piano_kline25_crp) = va_x16;
           break;
         case 15:
         NV(piano_kline25_cdur) = va_dur16;
           NV(piano_kline25_clp) = NV(piano_kline25_crp);
           NV(piano_kline25_crp) = va_x17;
           break;
         case 16:
         NV(piano_kline25_cdur) = va_dur17;
           NV(piano_kline25_clp) = NV(piano_kline25_crp);
           NV(piano_kline25_crp) = va_x18;
           break;
         case 17:
         NV(piano_kline25_cdur) = va_dur18;
           NV(piano_kline25_clp) = NV(piano_kline25_crp);
           NV(piano_kline25_crp) = va_x19;
           break;
         case 18:
         NV(piano_kline25_cdur) = va_dur19;
           NV(piano_kline25_clp) = NV(piano_kline25_crp);
           NV(piano_kline25_crp) = va_x20;
           break;
         case 19:
         NV(piano_kline25_cdur) = va_dur20;
           NV(piano_kline25_clp) = NV(piano_kline25_crp);
           NV(piano_kline25_crp) = va_x21;
           break;
           default:
           NVI(piano_kline25_first) = -100;
           NV(piano_kline25_cdur) = NV(piano_kline25_t) + 10000.0F;
           break;
           }
         NVI(piano_kline25_first)++;
        }
        NV(piano_kline25_mult)=(NV(piano_kline25_crp) - NV(piano_kline25_clp))/NV(piano_kline25_cdur);
        ret = NV(piano_kline25_outT) = NV(piano_kline25_clp)+NV(piano_kline25_mult)*NV(piano_kline25_t);
        NV(piano_kline25_addK) = NV(piano_kline25_mult)*EV(KTIME);
        if (NVI(piano_kline25_first)<0)
          ret = 0.0F;
      }
   }
   if (NVI(piano_kline25_first)==0)
     {
       NVI(piano_kline25_first) = 1;
       ret = NV(piano_kline25_outT) = NV(piano_kline25_clp) = x1;
       NV(piano_kline25_crp) = x2;
       NV(piano_kline25_cdur) = dur1;
       if (dur1 > 0.0F)
         NV(piano_kline25_addK) = EV(KTIME)*((x2 - x1)/dur1);
     }
   return((NV(piano_kline25_return) = ret));

}



float piano__sym_kline26(ENGINE_PTR_DECLARE_COMMA struct ninstr_types * nstate)
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
   float va_dur20;
   float va_x21;
   float x2;
   float dur1;
   float x1;
   float ret;

   x1 =  0.0F ;
   dur1 =  0.001608F ;
   x2 =  0.000189F ;
   va_dur2 =  0.001608F  ;
   va_x3 =  0.000505F  ;
   va_dur3 =  0.003215F  ;
   va_x4 =  0.000060F  ;
   va_dur4 =  0.004823F  ;
   va_x5 =  0.000345F  ;
   va_dur5 =  0.003215F  ;
   va_x6 =  0.000031F  ;
   va_dur6 =  0.001608F  ;
   va_x7 =  0.000309F  ;
   va_dur7 =  0.003215F  ;
   va_x8 =  0.000034F  ;
   va_dur8 =  0.003215F  ;
   va_x9 =  0.000335F  ;
   va_dur9 =  0.006431F  ;
   va_x10 =  0.000059F  ;
   va_dur10 =  0.004823F  ;
   va_x11 =  0.000261F  ;
   va_dur11 =  0.001608F  ;
   va_x12 =  0.000592F  ;
   va_dur12 =  0.001608F  ;
   va_x13 =  0.000120F  ;
   va_dur13 =  0.008039F  ;
   va_x14 =  0.000352F  ;
   va_dur14 =  0.003215F  ;
   va_x15 =  0.000040F  ;
   va_dur15 =  0.001608F  ;
   va_x16 =  0.000326F  ;
   va_dur16 =  0.001608F  ;
   va_x17 =  0.000043F  ;
   va_dur17 =  0.003215F  ;
   va_x18 =  0.000300F  ;
   va_dur18 =  0.011254F  ;
   va_x19 =  0.000018F  ;
   va_dur19 =  0.028939F  ;
   va_x20 =  0.000214F  ;
   va_dur20 =  2.397106F  ;
   va_x21 =  0.0F  ;
   ret = 0.0F;
   if (NVI(piano_kline26_first)>0)
   {
      NV(piano_kline26_t) += EV(KTIME);
      ret = (NV(piano_kline26_outT) += NV(piano_kline26_addK));
      if (NV(piano_kline26_t) > NV(piano_kline26_cdur))
       {
        while (NV(piano_kline26_t) > NV(piano_kline26_cdur))
         {
           NV(piano_kline26_t) -= NV(piano_kline26_cdur);
           switch(NVI(piano_kline26_first))
      {
         case 1:
         NV(piano_kline26_cdur) = va_dur2;
           NV(piano_kline26_clp) = NV(piano_kline26_crp);
           NV(piano_kline26_crp) = va_x3;
           break;
         case 2:
         NV(piano_kline26_cdur) = va_dur3;
           NV(piano_kline26_clp) = NV(piano_kline26_crp);
           NV(piano_kline26_crp) = va_x4;
           break;
         case 3:
         NV(piano_kline26_cdur) = va_dur4;
           NV(piano_kline26_clp) = NV(piano_kline26_crp);
           NV(piano_kline26_crp) = va_x5;
           break;
         case 4:
         NV(piano_kline26_cdur) = va_dur5;
           NV(piano_kline26_clp) = NV(piano_kline26_crp);
           NV(piano_kline26_crp) = va_x6;
           break;
         case 5:
         NV(piano_kline26_cdur) = va_dur6;
           NV(piano_kline26_clp) = NV(piano_kline26_crp);
           NV(piano_kline26_crp) = va_x7;
           break;
         case 6:
         NV(piano_kline26_cdur) = va_dur7;
           NV(piano_kline26_clp) = NV(piano_kline26_crp);
           NV(piano_kline26_crp) = va_x8;
           break;
         case 7:
         NV(piano_kline26_cdur) = va_dur8;
           NV(piano_kline26_clp) = NV(piano_kline26_crp);
           NV(piano_kline26_crp) = va_x9;
           break;
         case 8:
         NV(piano_kline26_cdur) = va_dur9;
           NV(piano_kline26_clp) = NV(piano_kline26_crp);
           NV(piano_kline26_crp) = va_x10;
           break;
         case 9:
         NV(piano_kline26_cdur) = va_dur10;
           NV(piano_kline26_clp) = NV(piano_kline26_crp);
           NV(piano_kline26_crp) = va_x11;
           break;
         case 10:
         NV(piano_kline26_cdur) = va_dur11;
           NV(piano_kline26_clp) = NV(piano_kline26_crp);
           NV(piano_kline26_crp) = va_x12;
           break;
         case 11:
         NV(piano_kline26_cdur) = va_dur12;
           NV(piano_kline26_clp) = NV(piano_kline26_crp);
           NV(piano_kline26_crp) = va_x13;
           break;
         case 12:
         NV(piano_kline26_cdur) = va_dur13;
           NV(piano_kline26_clp) = NV(piano_kline26_crp);
           NV(piano_kline26_crp) = va_x14;
           break;
         case 13:
         NV(piano_kline26_cdur) = va_dur14;
           NV(piano_kline26_clp) = NV(piano_kline26_crp);
           NV(piano_kline26_crp) = va_x15;
           break;
         case 14:
         NV(piano_kline26_cdur) = va_dur15;
           NV(piano_kline26_clp) = NV(piano_kline26_crp);
           NV(piano_kline26_crp) = va_x16;
           break;
         case 15:
         NV(piano_kline26_cdur) = va_dur16;
           NV(piano_kline26_clp) = NV(piano_kline26_crp);
           NV(piano_kline26_crp) = va_x17;
           break;
         case 16:
         NV(piano_kline26_cdur) = va_dur17;
           NV(piano_kline26_clp) = NV(piano_kline26_crp);
           NV(piano_kline26_crp) = va_x18;
           break;
         case 17:
         NV(piano_kline26_cdur) = va_dur18;
           NV(piano_kline26_clp) = NV(piano_kline26_crp);
           NV(piano_kline26_crp) = va_x19;
           break;
         case 18:
         NV(piano_kline26_cdur) = va_dur19;
           NV(piano_kline26_clp) = NV(piano_kline26_crp);
           NV(piano_kline26_crp) = va_x20;
           break;
         case 19:
         NV(piano_kline26_cdur) = va_dur20;
           NV(piano_kline26_clp) = NV(piano_kline26_crp);
           NV(piano_kline26_crp) = va_x21;
           break;
           default:
           NVI(piano_kline26_first) = -100;
           NV(piano_kline26_cdur) = NV(piano_kline26_t) + 10000.0F;
           break;
           }
         NVI(piano_kline26_first)++;
        }
        NV(piano_kline26_mult)=(NV(piano_kline26_crp) - NV(piano_kline26_clp))/NV(piano_kline26_cdur);
        ret = NV(piano_kline26_outT) = NV(piano_kline26_clp)+NV(piano_kline26_mult)*NV(piano_kline26_t);
        NV(piano_kline26_addK) = NV(piano_kline26_mult)*EV(KTIME);
        if (NVI(piano_kline26_first)<0)
          ret = 0.0F;
      }
   }
   if (NVI(piano_kline26_first)==0)
     {
       NVI(piano_kline26_first) = 1;
       ret = NV(piano_kline26_outT) = NV(piano_kline26_clp) = x1;
       NV(piano_kline26_crp) = x2;
       NV(piano_kline26_cdur) = dur1;
       if (dur1 > 0.0F)
         NV(piano_kline26_addK) = EV(KTIME)*((x2 - x1)/dur1);
     }
   return((NV(piano_kline26_return) = ret));

}



float piano__sym_kline27(ENGINE_PTR_DECLARE_COMMA struct ninstr_types * nstate)
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
   float va_dur20;
   float va_x21;
   float x2;
   float dur1;
   float x1;
   float ret;

   x1 =  0.0F ;
   dur1 =  0.004823F ;
   x2 =  0.001116F ;
   va_dur2 =  0.003215F  ;
   va_x3 =  0.000727F  ;
   va_dur3 =  0.011254F  ;
   va_x4 =  0.001184F  ;
   va_dur4 =  0.004823F  ;
   va_x5 =  0.000303F  ;
   va_dur5 =  0.006431F  ;
   va_x6 =  0.001015F  ;
   va_dur6 =  0.006431F  ;
   va_x7 =  0.000213F  ;
   va_dur7 =  0.006431F  ;
   va_x8 =  0.000811F  ;
   va_dur8 =  0.008039F  ;
   va_x9 =  0.000182F  ;
   va_dur9 =  0.003215F  ;
   va_x10 =  0.000860F  ;
   va_dur10 =  0.009646F  ;
   va_x11 =  0.000271F  ;
   va_dur11 =  0.008039F  ;
   va_x12 =  0.000848F  ;
   va_dur12 =  0.006431F  ;
   va_x13 =  0.000215F  ;
   va_dur13 =  0.006431F  ;
   va_x14 =  0.000828F  ;
   va_dur14 =  0.008039F  ;
   va_x15 =  0.000213F  ;
   va_dur15 =  0.006431F  ;
   va_x16 =  0.000748F  ;
   va_dur16 =  0.004823F  ;
   va_x17 =  0.000222F  ;
   va_dur17 =  0.008039F  ;
   va_x18 =  0.000649F  ;
   va_dur18 =  0.006431F  ;
   va_x19 =  0.000259F  ;
   va_dur19 =  0.086817F  ;
   va_x20 =  0.000011F  ;
   va_dur20 =  2.286174F  ;
   va_x21 =  0.0F  ;
   ret = 0.0F;
   if (NVI(piano_kline27_first)>0)
   {
      NV(piano_kline27_t) += EV(KTIME);
      ret = (NV(piano_kline27_outT) += NV(piano_kline27_addK));
      if (NV(piano_kline27_t) > NV(piano_kline27_cdur))
       {
        while (NV(piano_kline27_t) > NV(piano_kline27_cdur))
         {
           NV(piano_kline27_t) -= NV(piano_kline27_cdur);
           switch(NVI(piano_kline27_first))
      {
         case 1:
         NV(piano_kline27_cdur) = va_dur2;
           NV(piano_kline27_clp) = NV(piano_kline27_crp);
           NV(piano_kline27_crp) = va_x3;
           break;
         case 2:
         NV(piano_kline27_cdur) = va_dur3;
           NV(piano_kline27_clp) = NV(piano_kline27_crp);
           NV(piano_kline27_crp) = va_x4;
           break;
         case 3:
         NV(piano_kline27_cdur) = va_dur4;
           NV(piano_kline27_clp) = NV(piano_kline27_crp);
           NV(piano_kline27_crp) = va_x5;
           break;
         case 4:
         NV(piano_kline27_cdur) = va_dur5;
           NV(piano_kline27_clp) = NV(piano_kline27_crp);
           NV(piano_kline27_crp) = va_x6;
           break;
         case 5:
         NV(piano_kline27_cdur) = va_dur6;
           NV(piano_kline27_clp) = NV(piano_kline27_crp);
           NV(piano_kline27_crp) = va_x7;
           break;
         case 6:
         NV(piano_kline27_cdur) = va_dur7;
           NV(piano_kline27_clp) = NV(piano_kline27_crp);
           NV(piano_kline27_crp) = va_x8;
           break;
         case 7:
         NV(piano_kline27_cdur) = va_dur8;
           NV(piano_kline27_clp) = NV(piano_kline27_crp);
           NV(piano_kline27_crp) = va_x9;
           break;
         case 8:
         NV(piano_kline27_cdur) = va_dur9;
           NV(piano_kline27_clp) = NV(piano_kline27_crp);
           NV(piano_kline27_crp) = va_x10;
           break;
         case 9:
         NV(piano_kline27_cdur) = va_dur10;
           NV(piano_kline27_clp) = NV(piano_kline27_crp);
           NV(piano_kline27_crp) = va_x11;
           break;
         case 10:
         NV(piano_kline27_cdur) = va_dur11;
           NV(piano_kline27_clp) = NV(piano_kline27_crp);
           NV(piano_kline27_crp) = va_x12;
           break;
         case 11:
         NV(piano_kline27_cdur) = va_dur12;
           NV(piano_kline27_clp) = NV(piano_kline27_crp);
           NV(piano_kline27_crp) = va_x13;
           break;
         case 12:
         NV(piano_kline27_cdur) = va_dur13;
           NV(piano_kline27_clp) = NV(piano_kline27_crp);
           NV(piano_kline27_crp) = va_x14;
           break;
         case 13:
         NV(piano_kline27_cdur) = va_dur14;
           NV(piano_kline27_clp) = NV(piano_kline27_crp);
           NV(piano_kline27_crp) = va_x15;
           break;
         case 14:
         NV(piano_kline27_cdur) = va_dur15;
           NV(piano_kline27_clp) = NV(piano_kline27_crp);
           NV(piano_kline27_crp) = va_x16;
           break;
         case 15:
         NV(piano_kline27_cdur) = va_dur16;
           NV(piano_kline27_clp) = NV(piano_kline27_crp);
           NV(piano_kline27_crp) = va_x17;
           break;
         case 16:
         NV(piano_kline27_cdur) = va_dur17;
           NV(piano_kline27_clp) = NV(piano_kline27_crp);
           NV(piano_kline27_crp) = va_x18;
           break;
         case 17:
         NV(piano_kline27_cdur) = va_dur18;
           NV(piano_kline27_clp) = NV(piano_kline27_crp);
           NV(piano_kline27_crp) = va_x19;
           break;
         case 18:
         NV(piano_kline27_cdur) = va_dur19;
           NV(piano_kline27_clp) = NV(piano_kline27_crp);
           NV(piano_kline27_crp) = va_x20;
           break;
         case 19:
         NV(piano_kline27_cdur) = va_dur20;
           NV(piano_kline27_clp) = NV(piano_kline27_crp);
           NV(piano_kline27_crp) = va_x21;
           break;
           default:
           NVI(piano_kline27_first) = -100;
           NV(piano_kline27_cdur) = NV(piano_kline27_t) + 10000.0F;
           break;
           }
         NVI(piano_kline27_first)++;
        }
        NV(piano_kline27_mult)=(NV(piano_kline27_crp) - NV(piano_kline27_clp))/NV(piano_kline27_cdur);
        ret = NV(piano_kline27_outT) = NV(piano_kline27_clp)+NV(piano_kline27_mult)*NV(piano_kline27_t);
        NV(piano_kline27_addK) = NV(piano_kline27_mult)*EV(KTIME);
        if (NVI(piano_kline27_first)<0)
          ret = 0.0F;
      }
   }
   if (NVI(piano_kline27_first)==0)
     {
       NVI(piano_kline27_first) = 1;
       ret = NV(piano_kline27_outT) = NV(piano_kline27_clp) = x1;
       NV(piano_kline27_crp) = x2;
       NV(piano_kline27_cdur) = dur1;
       if (dur1 > 0.0F)
         NV(piano_kline27_addK) = EV(KTIME)*((x2 - x1)/dur1);
     }
   return((NV(piano_kline27_return) = ret));

}



float piano__sym_kline28(ENGINE_PTR_DECLARE_COMMA struct ninstr_types * nstate)
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
   float va_dur20;
   float va_x21;
   float x2;
   float dur1;
   float x1;
   float ret;

   x1 =  0.0F ;
   dur1 =  0.001608F ;
   x2 =  0.000256F ;
   va_dur2 =  0.003215F  ;
   va_x3 =  0.000221F  ;
   va_dur3 =  0.004823F  ;
   va_x4 =  0.000532F  ;
   va_dur4 =  0.003215F  ;
   va_x5 =  0.000292F  ;
   va_dur5 =  0.008039F  ;
   va_x6 =  0.000905F  ;
   va_dur6 =  0.014469F  ;
   va_x7 =  0.001259F  ;
   va_dur7 =  0.011254F  ;
   va_x8 =  0.000676F  ;
   va_dur8 =  0.011254F  ;
   va_x9 =  0.000309F  ;
   va_dur9 =  0.012862F  ;
   va_x10 =  0.000233F  ;
   va_dur10 =  0.001608F  ;
   va_x11 =  0.000367F  ;
   va_dur11 =  0.012862F  ;
   va_x12 =  0.000172F  ;
   va_dur12 =  0.011254F  ;
   va_x13 =  0.000279F  ;
   va_dur13 =  0.001608F  ;
   va_x14 =  0.000122F  ;
   va_dur14 =  0.009646F  ;
   va_x15 =  0.000332F  ;
   va_dur15 =  0.004823F  ;
   va_x16 =  0.000202F  ;
   va_dur16 =  0.006431F  ;
   va_x17 =  0.000330F  ;
   va_dur17 =  0.043408F  ;
   va_x18 =  0.000027F  ;
   va_dur18 =  0.032154F  ;
   va_x19 =  0.000145F  ;
   va_dur19 =  0.117363F  ;
   va_x20 =  0.000005F  ;
   va_dur20 =  2.180064F  ;
   va_x21 =  0.0F  ;
   ret = 0.0F;
   if (NVI(piano_kline28_first)>0)
   {
      NV(piano_kline28_t) += EV(KTIME);
      ret = (NV(piano_kline28_outT) += NV(piano_kline28_addK));
      if (NV(piano_kline28_t) > NV(piano_kline28_cdur))
       {
        while (NV(piano_kline28_t) > NV(piano_kline28_cdur))
         {
           NV(piano_kline28_t) -= NV(piano_kline28_cdur);
           switch(NVI(piano_kline28_first))
      {
         case 1:
         NV(piano_kline28_cdur) = va_dur2;
           NV(piano_kline28_clp) = NV(piano_kline28_crp);
           NV(piano_kline28_crp) = va_x3;
           break;
         case 2:
         NV(piano_kline28_cdur) = va_dur3;
           NV(piano_kline28_clp) = NV(piano_kline28_crp);
           NV(piano_kline28_crp) = va_x4;
           break;
         case 3:
         NV(piano_kline28_cdur) = va_dur4;
           NV(piano_kline28_clp) = NV(piano_kline28_crp);
           NV(piano_kline28_crp) = va_x5;
           break;
         case 4:
         NV(piano_kline28_cdur) = va_dur5;
           NV(piano_kline28_clp) = NV(piano_kline28_crp);
           NV(piano_kline28_crp) = va_x6;
           break;
         case 5:
         NV(piano_kline28_cdur) = va_dur6;
           NV(piano_kline28_clp) = NV(piano_kline28_crp);
           NV(piano_kline28_crp) = va_x7;
           break;
         case 6:
         NV(piano_kline28_cdur) = va_dur7;
           NV(piano_kline28_clp) = NV(piano_kline28_crp);
           NV(piano_kline28_crp) = va_x8;
           break;
         case 7:
         NV(piano_kline28_cdur) = va_dur8;
           NV(piano_kline28_clp) = NV(piano_kline28_crp);
           NV(piano_kline28_crp) = va_x9;
           break;
         case 8:
         NV(piano_kline28_cdur) = va_dur9;
           NV(piano_kline28_clp) = NV(piano_kline28_crp);
           NV(piano_kline28_crp) = va_x10;
           break;
         case 9:
         NV(piano_kline28_cdur) = va_dur10;
           NV(piano_kline28_clp) = NV(piano_kline28_crp);
           NV(piano_kline28_crp) = va_x11;
           break;
         case 10:
         NV(piano_kline28_cdur) = va_dur11;
           NV(piano_kline28_clp) = NV(piano_kline28_crp);
           NV(piano_kline28_crp) = va_x12;
           break;
         case 11:
         NV(piano_kline28_cdur) = va_dur12;
           NV(piano_kline28_clp) = NV(piano_kline28_crp);
           NV(piano_kline28_crp) = va_x13;
           break;
         case 12:
         NV(piano_kline28_cdur) = va_dur13;
           NV(piano_kline28_clp) = NV(piano_kline28_crp);
           NV(piano_kline28_crp) = va_x14;
           break;
         case 13:
         NV(piano_kline28_cdur) = va_dur14;
           NV(piano_kline28_clp) = NV(piano_kline28_crp);
           NV(piano_kline28_crp) = va_x15;
           break;
         case 14:
         NV(piano_kline28_cdur) = va_dur15;
           NV(piano_kline28_clp) = NV(piano_kline28_crp);
           NV(piano_kline28_crp) = va_x16;
           break;
         case 15:
         NV(piano_kline28_cdur) = va_dur16;
           NV(piano_kline28_clp) = NV(piano_kline28_crp);
           NV(piano_kline28_crp) = va_x17;
           break;
         case 16:
         NV(piano_kline28_cdur) = va_dur17;
           NV(piano_kline28_clp) = NV(piano_kline28_crp);
           NV(piano_kline28_crp) = va_x18;
           break;
         case 17:
         NV(piano_kline28_cdur) = va_dur18;
           NV(piano_kline28_clp) = NV(piano_kline28_crp);
           NV(piano_kline28_crp) = va_x19;
           break;
         case 18:
         NV(piano_kline28_cdur) = va_dur19;
           NV(piano_kline28_clp) = NV(piano_kline28_crp);
           NV(piano_kline28_crp) = va_x20;
           break;
         case 19:
         NV(piano_kline28_cdur) = va_dur20;
           NV(piano_kline28_clp) = NV(piano_kline28_crp);
           NV(piano_kline28_crp) = va_x21;
           break;
           default:
           NVI(piano_kline28_first) = -100;
           NV(piano_kline28_cdur) = NV(piano_kline28_t) + 10000.0F;
           break;
           }
         NVI(piano_kline28_first)++;
        }
        NV(piano_kline28_mult)=(NV(piano_kline28_crp) - NV(piano_kline28_clp))/NV(piano_kline28_cdur);
        ret = NV(piano_kline28_outT) = NV(piano_kline28_clp)+NV(piano_kline28_mult)*NV(piano_kline28_t);
        NV(piano_kline28_addK) = NV(piano_kline28_mult)*EV(KTIME);
        if (NVI(piano_kline28_first)<0)
          ret = 0.0F;
      }
   }
   if (NVI(piano_kline28_first)==0)
     {
       NVI(piano_kline28_first) = 1;
       ret = NV(piano_kline28_outT) = NV(piano_kline28_clp) = x1;
       NV(piano_kline28_crp) = x2;
       NV(piano_kline28_cdur) = dur1;
       if (dur1 > 0.0F)
         NV(piano_kline28_addK) = EV(KTIME)*((x2 - x1)/dur1);
     }
   return((NV(piano_kline28_return) = ret));

}



float piano__sym_kline29(ENGINE_PTR_DECLARE_COMMA struct ninstr_types * nstate)
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
   float va_dur20;
   float va_x21;
   float x2;
   float dur1;
   float x1;
   float ret;

   x1 =  0.0F ;
   dur1 =  0.001608F ;
   x2 =  0.000438F ;
   va_dur2 =  0.003215F  ;
   va_x3 =  0.000175F  ;
   va_dur3 =  0.004823F  ;
   va_x4 =  0.000187F  ;
   va_dur4 =  0.004823F  ;
   va_x5 =  0.000040F  ;
   va_dur5 =  0.012862F  ;
   va_x6 =  0.000070F  ;
   va_dur6 =  0.001608F  ;
   va_x7 =  0.000192F  ;
   va_dur7 =  0.004823F  ;
   va_x8 =  0.000036F  ;
   va_dur8 =  0.001608F  ;
   va_x9 =  0.000220F  ;
   va_dur9 =  0.001608F  ;
   va_x10 =  0.000105F  ;
   va_dur10 =  0.001608F  ;
   va_x11 =  0.000240F  ;
   va_dur11 =  0.001608F  ;
   va_x12 =  0.000056F  ;
   va_dur12 =  0.003215F  ;
   va_x13 =  0.000122F  ;
   va_dur13 =  0.001608F  ;
   va_x14 =  0.000018F  ;
   va_dur14 =  0.008039F  ;
   va_x15 =  0.000031F  ;
   va_dur15 =  0.006431F  ;
   va_x16 =  0.000174F  ;
   va_dur16 =  0.008039F  ;
   va_x17 =  0.000022F  ;
   va_dur17 =  0.004823F  ;
   va_x18 =  0.000157F  ;
   va_dur18 =  0.006431F  ;
   va_x19 =  0.000013F  ;
   va_dur19 =  0.032154F  ;
   va_x20 =  0.000008F  ;
   va_dur20 =  2.381029F  ;
   va_x21 =  0.0F  ;
   ret = 0.0F;
   if (NVI(piano_kline29_first)>0)
   {
      NV(piano_kline29_t) += EV(KTIME);
      ret = (NV(piano_kline29_outT) += NV(piano_kline29_addK));
      if (NV(piano_kline29_t) > NV(piano_kline29_cdur))
       {
        while (NV(piano_kline29_t) > NV(piano_kline29_cdur))
         {
           NV(piano_kline29_t) -= NV(piano_kline29_cdur);
           switch(NVI(piano_kline29_first))
      {
         case 1:
         NV(piano_kline29_cdur) = va_dur2;
           NV(piano_kline29_clp) = NV(piano_kline29_crp);
           NV(piano_kline29_crp) = va_x3;
           break;
         case 2:
         NV(piano_kline29_cdur) = va_dur3;
           NV(piano_kline29_clp) = NV(piano_kline29_crp);
           NV(piano_kline29_crp) = va_x4;
           break;
         case 3:
         NV(piano_kline29_cdur) = va_dur4;
           NV(piano_kline29_clp) = NV(piano_kline29_crp);
           NV(piano_kline29_crp) = va_x5;
           break;
         case 4:
         NV(piano_kline29_cdur) = va_dur5;
           NV(piano_kline29_clp) = NV(piano_kline29_crp);
           NV(piano_kline29_crp) = va_x6;
           break;
         case 5:
         NV(piano_kline29_cdur) = va_dur6;
           NV(piano_kline29_clp) = NV(piano_kline29_crp);
           NV(piano_kline29_crp) = va_x7;
           break;
         case 6:
         NV(piano_kline29_cdur) = va_dur7;
           NV(piano_kline29_clp) = NV(piano_kline29_crp);
           NV(piano_kline29_crp) = va_x8;
           break;
         case 7:
         NV(piano_kline29_cdur) = va_dur8;
           NV(piano_kline29_clp) = NV(piano_kline29_crp);
           NV(piano_kline29_crp) = va_x9;
           break;
         case 8:
         NV(piano_kline29_cdur) = va_dur9;
           NV(piano_kline29_clp) = NV(piano_kline29_crp);
           NV(piano_kline29_crp) = va_x10;
           break;
         case 9:
         NV(piano_kline29_cdur) = va_dur10;
           NV(piano_kline29_clp) = NV(piano_kline29_crp);
           NV(piano_kline29_crp) = va_x11;
           break;
         case 10:
         NV(piano_kline29_cdur) = va_dur11;
           NV(piano_kline29_clp) = NV(piano_kline29_crp);
           NV(piano_kline29_crp) = va_x12;
           break;
         case 11:
         NV(piano_kline29_cdur) = va_dur12;
           NV(piano_kline29_clp) = NV(piano_kline29_crp);
           NV(piano_kline29_crp) = va_x13;
           break;
         case 12:
         NV(piano_kline29_cdur) = va_dur13;
           NV(piano_kline29_clp) = NV(piano_kline29_crp);
           NV(piano_kline29_crp) = va_x14;
           break;
         case 13:
         NV(piano_kline29_cdur) = va_dur14;
           NV(piano_kline29_clp) = NV(piano_kline29_crp);
           NV(piano_kline29_crp) = va_x15;
           break;
         case 14:
         NV(piano_kline29_cdur) = va_dur15;
           NV(piano_kline29_clp) = NV(piano_kline29_crp);
           NV(piano_kline29_crp) = va_x16;
           break;
         case 15:
         NV(piano_kline29_cdur) = va_dur16;
           NV(piano_kline29_clp) = NV(piano_kline29_crp);
           NV(piano_kline29_crp) = va_x17;
           break;
         case 16:
         NV(piano_kline29_cdur) = va_dur17;
           NV(piano_kline29_clp) = NV(piano_kline29_crp);
           NV(piano_kline29_crp) = va_x18;
           break;
         case 17:
         NV(piano_kline29_cdur) = va_dur18;
           NV(piano_kline29_clp) = NV(piano_kline29_crp);
           NV(piano_kline29_crp) = va_x19;
           break;
         case 18:
         NV(piano_kline29_cdur) = va_dur19;
           NV(piano_kline29_clp) = NV(piano_kline29_crp);
           NV(piano_kline29_crp) = va_x20;
           break;
         case 19:
         NV(piano_kline29_cdur) = va_dur20;
           NV(piano_kline29_clp) = NV(piano_kline29_crp);
           NV(piano_kline29_crp) = va_x21;
           break;
           default:
           NVI(piano_kline29_first) = -100;
           NV(piano_kline29_cdur) = NV(piano_kline29_t) + 10000.0F;
           break;
           }
         NVI(piano_kline29_first)++;
        }
        NV(piano_kline29_mult)=(NV(piano_kline29_crp) - NV(piano_kline29_clp))/NV(piano_kline29_cdur);
        ret = NV(piano_kline29_outT) = NV(piano_kline29_clp)+NV(piano_kline29_mult)*NV(piano_kline29_t);
        NV(piano_kline29_addK) = NV(piano_kline29_mult)*EV(KTIME);
        if (NVI(piano_kline29_first)<0)
          ret = 0.0F;
      }
   }
   if (NVI(piano_kline29_first)==0)
     {
       NVI(piano_kline29_first) = 1;
       ret = NV(piano_kline29_outT) = NV(piano_kline29_clp) = x1;
       NV(piano_kline29_crp) = x2;
       NV(piano_kline29_cdur) = dur1;
       if (dur1 > 0.0F)
         NV(piano_kline29_addK) = EV(KTIME)*((x2 - x1)/dur1);
     }
   return((NV(piano_kline29_return) = ret));

}



float piano__sym_kline30(ENGINE_PTR_DECLARE_COMMA struct ninstr_types * nstate)
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
   float va_dur20;
   float va_x21;
   float x2;
   float dur1;
   float x1;
   float ret;

   x1 =  0.0F ;
   dur1 =  0.001608F ;
   x2 =  0.000028F ;
   va_dur2 =  0.003215F  ;
   va_x3 =  0.000243F  ;
   va_dur3 =  0.003215F  ;
   va_x4 =  0.000039F  ;
   va_dur4 =  0.006431F  ;
   va_x5 =  0.000217F  ;
   va_dur5 =  0.003215F  ;
   va_x6 =  0.000033F  ;
   va_dur6 =  0.009646F  ;
   va_x7 =  0.000130F  ;
   va_dur7 =  0.006431F  ;
   va_x8 =  0.000023F  ;
   va_dur8 =  0.001608F  ;
   va_x9 =  0.000121F  ;
   va_dur9 =  0.025723F  ;
   va_x10 =  0.000108F  ;
   va_dur10 =  0.003215F  ;
   va_x11 =  0.000010F  ;
   va_dur11 =  0.011254F  ;
   va_x12 =  0.000146F  ;
   va_dur12 =  0.011254F  ;
   va_x13 =  0.000018F  ;
   va_dur13 =  0.008039F  ;
   va_x14 =  0.000138F  ;
   va_dur14 =  0.004823F  ;
   va_x15 =  0.000018F  ;
   va_dur15 =  0.006431F  ;
   va_x16 =  0.000118F  ;
   va_dur16 =  0.001608F  ;
   va_x17 =  0.000011F  ;
   va_dur17 =  0.009646F  ;
   va_x18 =  0.000006F  ;
   va_dur18 =  0.011254F  ;
   va_x19 =  0.000097F  ;
   va_dur19 =  0.038585F  ;
   va_x20 =  0.000004F  ;
   va_dur20 =  2.324759F  ;
   va_x21 =  0.0F  ;
   ret = 0.0F;
   if (NVI(piano_kline30_first)>0)
   {
      NV(piano_kline30_t) += EV(KTIME);
      ret = (NV(piano_kline30_outT) += NV(piano_kline30_addK));
      if (NV(piano_kline30_t) > NV(piano_kline30_cdur))
       {
        while (NV(piano_kline30_t) > NV(piano_kline30_cdur))
         {
           NV(piano_kline30_t) -= NV(piano_kline30_cdur);
           switch(NVI(piano_kline30_first))
      {
         case 1:
         NV(piano_kline30_cdur) = va_dur2;
           NV(piano_kline30_clp) = NV(piano_kline30_crp);
           NV(piano_kline30_crp) = va_x3;
           break;
         case 2:
         NV(piano_kline30_cdur) = va_dur3;
           NV(piano_kline30_clp) = NV(piano_kline30_crp);
           NV(piano_kline30_crp) = va_x4;
           break;
         case 3:
         NV(piano_kline30_cdur) = va_dur4;
           NV(piano_kline30_clp) = NV(piano_kline30_crp);
           NV(piano_kline30_crp) = va_x5;
           break;
         case 4:
         NV(piano_kline30_cdur) = va_dur5;
           NV(piano_kline30_clp) = NV(piano_kline30_crp);
           NV(piano_kline30_crp) = va_x6;
           break;
         case 5:
         NV(piano_kline30_cdur) = va_dur6;
           NV(piano_kline30_clp) = NV(piano_kline30_crp);
           NV(piano_kline30_crp) = va_x7;
           break;
         case 6:
         NV(piano_kline30_cdur) = va_dur7;
           NV(piano_kline30_clp) = NV(piano_kline30_crp);
           NV(piano_kline30_crp) = va_x8;
           break;
         case 7:
         NV(piano_kline30_cdur) = va_dur8;
           NV(piano_kline30_clp) = NV(piano_kline30_crp);
           NV(piano_kline30_crp) = va_x9;
           break;
         case 8:
         NV(piano_kline30_cdur) = va_dur9;
           NV(piano_kline30_clp) = NV(piano_kline30_crp);
           NV(piano_kline30_crp) = va_x10;
           break;
         case 9:
         NV(piano_kline30_cdur) = va_dur10;
           NV(piano_kline30_clp) = NV(piano_kline30_crp);
           NV(piano_kline30_crp) = va_x11;
           break;
         case 10:
         NV(piano_kline30_cdur) = va_dur11;
           NV(piano_kline30_clp) = NV(piano_kline30_crp);
           NV(piano_kline30_crp) = va_x12;
           break;
         case 11:
         NV(piano_kline30_cdur) = va_dur12;
           NV(piano_kline30_clp) = NV(piano_kline30_crp);
           NV(piano_kline30_crp) = va_x13;
           break;
         case 12:
         NV(piano_kline30_cdur) = va_dur13;
           NV(piano_kline30_clp) = NV(piano_kline30_crp);
           NV(piano_kline30_crp) = va_x14;
           break;
         case 13:
         NV(piano_kline30_cdur) = va_dur14;
           NV(piano_kline30_clp) = NV(piano_kline30_crp);
           NV(piano_kline30_crp) = va_x15;
           break;
         case 14:
         NV(piano_kline30_cdur) = va_dur15;
           NV(piano_kline30_clp) = NV(piano_kline30_crp);
           NV(piano_kline30_crp) = va_x16;
           break;
         case 15:
         NV(piano_kline30_cdur) = va_dur16;
           NV(piano_kline30_clp) = NV(piano_kline30_crp);
           NV(piano_kline30_crp) = va_x17;
           break;
         case 16:
         NV(piano_kline30_cdur) = va_dur17;
           NV(piano_kline30_clp) = NV(piano_kline30_crp);
           NV(piano_kline30_crp) = va_x18;
           break;
         case 17:
         NV(piano_kline30_cdur) = va_dur18;
           NV(piano_kline30_clp) = NV(piano_kline30_crp);
           NV(piano_kline30_crp) = va_x19;
           break;
         case 18:
         NV(piano_kline30_cdur) = va_dur19;
           NV(piano_kline30_clp) = NV(piano_kline30_crp);
           NV(piano_kline30_crp) = va_x20;
           break;
         case 19:
         NV(piano_kline30_cdur) = va_dur20;
           NV(piano_kline30_clp) = NV(piano_kline30_crp);
           NV(piano_kline30_crp) = va_x21;
           break;
           default:
           NVI(piano_kline30_first) = -100;
           NV(piano_kline30_cdur) = NV(piano_kline30_t) + 10000.0F;
           break;
           }
         NVI(piano_kline30_first)++;
        }
        NV(piano_kline30_mult)=(NV(piano_kline30_crp) - NV(piano_kline30_clp))/NV(piano_kline30_cdur);
        ret = NV(piano_kline30_outT) = NV(piano_kline30_clp)+NV(piano_kline30_mult)*NV(piano_kline30_t);
        NV(piano_kline30_addK) = NV(piano_kline30_mult)*EV(KTIME);
        if (NVI(piano_kline30_first)<0)
          ret = 0.0F;
      }
   }
   if (NVI(piano_kline30_first)==0)
     {
       NVI(piano_kline30_first) = 1;
       ret = NV(piano_kline30_outT) = NV(piano_kline30_clp) = x1;
       NV(piano_kline30_crp) = x2;
       NV(piano_kline30_cdur) = dur1;
       if (dur1 > 0.0F)
         NV(piano_kline30_addK) = EV(KTIME)*((x2 - x1)/dur1);
     }
   return((NV(piano_kline30_return) = ret));

}



float piano__sym_oscil31(ENGINE_PTR_DECLARE_COMMA struct ninstr_types * nstate)
{
   float freq;
   PSIZE t;
   float ret;
   float index;
   unsigned int nint, nfrac, i, j;

   t =  TBL_GBL_cyc ;

#undef AP1
#define AP1 EV(gtables)[t]

   freq = NV(piano__tvr0);
   if (NVI(piano_oscil31_kcyc))
   {
     if (NVI(piano_oscil31_fsign))
      {
       NVUI(piano_oscil31_pfrac) = (j = NVUI(piano_oscil31_pfrac)) + NVUI(piano_oscil31_kfrac);
       NVUI(piano_oscil31_pint) += NVUI(piano_oscil31_kint) + (NVUI(piano_oscil31_pfrac) < j);
       if ((i = NVUI(piano_oscil31_pint)) >= AP1.len)
         i = (NVUI(piano_oscil31_pint) -= AP1.len);
      }
     else
      {
       NVUI(piano_oscil31_pfrac) = (j = NVUI(piano_oscil31_pfrac)) - NVUI(piano_oscil31_kfrac);
       NVUI(piano_oscil31_pint) -= NVUI(piano_oscil31_kint) + (NVUI(piano_oscil31_pfrac) > j);
       if ((i = NVUI(piano_oscil31_pint)) >= AP1.len)
         i = (NVUI(piano_oscil31_pint) += AP1.len);
      }
     ret = AP1.t[i] + NVUI(piano_oscil31_pfrac)*
           ((float)(1.0/4294967296.0))*(AP1.t[i+1] - AP1.t[i]);
   }
   else
   {
     if ((NVI(piano_oscil31_fsign) = (freq >= 0)))
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
     NVUI(piano_oscil31_kint) = nint;
     NVUI(piano_oscil31_kfrac) = nfrac;
     NVI(piano_oscil31_kcyc) = 1;
     ret = AP1.t[0];
   }
   return((NV(piano_oscil31_return) = ret));

}



float piano__sym_oscil32(ENGINE_PTR_DECLARE_COMMA struct ninstr_types * nstate)
{
   float freq;
   PSIZE t;
   float ret;
   float index;
   unsigned int nint, nfrac, i, j;

   t =  TBL_GBL_cyc ;

#undef AP1
#define AP1 EV(gtables)[t]

   freq = NV(piano__tvr1);
   if (NVI(piano_oscil32_kcyc))
   {
     if (NVI(piano_oscil32_fsign))
      {
       NVUI(piano_oscil32_pfrac) = (j = NVUI(piano_oscil32_pfrac)) + NVUI(piano_oscil32_kfrac);
       NVUI(piano_oscil32_pint) += NVUI(piano_oscil32_kint) + (NVUI(piano_oscil32_pfrac) < j);
       if ((i = NVUI(piano_oscil32_pint)) >= AP1.len)
         i = (NVUI(piano_oscil32_pint) -= AP1.len);
      }
     else
      {
       NVUI(piano_oscil32_pfrac) = (j = NVUI(piano_oscil32_pfrac)) - NVUI(piano_oscil32_kfrac);
       NVUI(piano_oscil32_pint) -= NVUI(piano_oscil32_kint) + (NVUI(piano_oscil32_pfrac) > j);
       if ((i = NVUI(piano_oscil32_pint)) >= AP1.len)
         i = (NVUI(piano_oscil32_pint) += AP1.len);
      }
     ret = AP1.t[i] + NVUI(piano_oscil32_pfrac)*
           ((float)(1.0/4294967296.0))*(AP1.t[i+1] - AP1.t[i]);
   }
   else
   {
     if ((NVI(piano_oscil32_fsign) = (freq >= 0)))
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
     NVUI(piano_oscil32_kint) = nint;
     NVUI(piano_oscil32_kfrac) = nfrac;
     NVI(piano_oscil32_kcyc) = 1;
     ret = AP1.t[0];
   }
   return((NV(piano_oscil32_return) = ret));

}



float piano__sym_oscil33(ENGINE_PTR_DECLARE_COMMA struct ninstr_types * nstate)
{
   float freq;
   PSIZE t;
   float ret;
   float index;
   unsigned int nint, nfrac, i, j;

   t =  TBL_GBL_cyc ;

#undef AP1
#define AP1 EV(gtables)[t]

   freq = NV(piano__tvr2);
   if (NVI(piano_oscil33_kcyc))
   {
     if (NVI(piano_oscil33_fsign))
      {
       NVUI(piano_oscil33_pfrac) = (j = NVUI(piano_oscil33_pfrac)) + NVUI(piano_oscil33_kfrac);
       NVUI(piano_oscil33_pint) += NVUI(piano_oscil33_kint) + (NVUI(piano_oscil33_pfrac) < j);
       if ((i = NVUI(piano_oscil33_pint)) >= AP1.len)
         i = (NVUI(piano_oscil33_pint) -= AP1.len);
      }
     else
      {
       NVUI(piano_oscil33_pfrac) = (j = NVUI(piano_oscil33_pfrac)) - NVUI(piano_oscil33_kfrac);
       NVUI(piano_oscil33_pint) -= NVUI(piano_oscil33_kint) + (NVUI(piano_oscil33_pfrac) > j);
       if ((i = NVUI(piano_oscil33_pint)) >= AP1.len)
         i = (NVUI(piano_oscil33_pint) += AP1.len);
      }
     ret = AP1.t[i] + NVUI(piano_oscil33_pfrac)*
           ((float)(1.0/4294967296.0))*(AP1.t[i+1] - AP1.t[i]);
   }
   else
   {
     if ((NVI(piano_oscil33_fsign) = (freq >= 0)))
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
     NVUI(piano_oscil33_kint) = nint;
     NVUI(piano_oscil33_kfrac) = nfrac;
     NVI(piano_oscil33_kcyc) = 1;
     ret = AP1.t[0];
   }
   return((NV(piano_oscil33_return) = ret));

}



float piano__sym_oscil34(ENGINE_PTR_DECLARE_COMMA struct ninstr_types * nstate)
{
   float freq;
   PSIZE t;
   float ret;
   float index;
   unsigned int nint, nfrac, i, j;

   t =  TBL_GBL_cyc ;

#undef AP1
#define AP1 EV(gtables)[t]

   freq = NV(piano__tvr3);
   if (NVI(piano_oscil34_kcyc))
   {
     if (NVI(piano_oscil34_fsign))
      {
       NVUI(piano_oscil34_pfrac) = (j = NVUI(piano_oscil34_pfrac)) + NVUI(piano_oscil34_kfrac);
       NVUI(piano_oscil34_pint) += NVUI(piano_oscil34_kint) + (NVUI(piano_oscil34_pfrac) < j);
       if ((i = NVUI(piano_oscil34_pint)) >= AP1.len)
         i = (NVUI(piano_oscil34_pint) -= AP1.len);
      }
     else
      {
       NVUI(piano_oscil34_pfrac) = (j = NVUI(piano_oscil34_pfrac)) - NVUI(piano_oscil34_kfrac);
       NVUI(piano_oscil34_pint) -= NVUI(piano_oscil34_kint) + (NVUI(piano_oscil34_pfrac) > j);
       if ((i = NVUI(piano_oscil34_pint)) >= AP1.len)
         i = (NVUI(piano_oscil34_pint) += AP1.len);
      }
     ret = AP1.t[i] + NVUI(piano_oscil34_pfrac)*
           ((float)(1.0/4294967296.0))*(AP1.t[i+1] - AP1.t[i]);
   }
   else
   {
     if ((NVI(piano_oscil34_fsign) = (freq >= 0)))
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
     NVUI(piano_oscil34_kint) = nint;
     NVUI(piano_oscil34_kfrac) = nfrac;
     NVI(piano_oscil34_kcyc) = 1;
     ret = AP1.t[0];
   }
   return((NV(piano_oscil34_return) = ret));

}



float piano__sym_oscil35(ENGINE_PTR_DECLARE_COMMA struct ninstr_types * nstate)
{
   float freq;
   PSIZE t;
   float ret;
   float index;
   unsigned int nint, nfrac, i, j;

   t =  TBL_GBL_cyc ;

#undef AP1
#define AP1 EV(gtables)[t]

   freq = NV(piano__tvr4);
   if (NVI(piano_oscil35_kcyc))
   {
     if (NVI(piano_oscil35_fsign))
      {
       NVUI(piano_oscil35_pfrac) = (j = NVUI(piano_oscil35_pfrac)) + NVUI(piano_oscil35_kfrac);
       NVUI(piano_oscil35_pint) += NVUI(piano_oscil35_kint) + (NVUI(piano_oscil35_pfrac) < j);
       if ((i = NVUI(piano_oscil35_pint)) >= AP1.len)
         i = (NVUI(piano_oscil35_pint) -= AP1.len);
      }
     else
      {
       NVUI(piano_oscil35_pfrac) = (j = NVUI(piano_oscil35_pfrac)) - NVUI(piano_oscil35_kfrac);
       NVUI(piano_oscil35_pint) -= NVUI(piano_oscil35_kint) + (NVUI(piano_oscil35_pfrac) > j);
       if ((i = NVUI(piano_oscil35_pint)) >= AP1.len)
         i = (NVUI(piano_oscil35_pint) += AP1.len);
      }
     ret = AP1.t[i] + NVUI(piano_oscil35_pfrac)*
           ((float)(1.0/4294967296.0))*(AP1.t[i+1] - AP1.t[i]);
   }
   else
   {
     if ((NVI(piano_oscil35_fsign) = (freq >= 0)))
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
     NVUI(piano_oscil35_kint) = nint;
     NVUI(piano_oscil35_kfrac) = nfrac;
     NVI(piano_oscil35_kcyc) = 1;
     ret = AP1.t[0];
   }
   return((NV(piano_oscil35_return) = ret));

}



float piano__sym_oscil36(ENGINE_PTR_DECLARE_COMMA struct ninstr_types * nstate)
{
   float freq;
   PSIZE t;
   float ret;
   float index;
   unsigned int nint, nfrac, i, j;

   t =  TBL_GBL_cyc ;

#undef AP1
#define AP1 EV(gtables)[t]

   freq = NV(piano__tvr5);
   if (NVI(piano_oscil36_kcyc))
   {
     if (NVI(piano_oscil36_fsign))
      {
       NVUI(piano_oscil36_pfrac) = (j = NVUI(piano_oscil36_pfrac)) + NVUI(piano_oscil36_kfrac);
       NVUI(piano_oscil36_pint) += NVUI(piano_oscil36_kint) + (NVUI(piano_oscil36_pfrac) < j);
       if ((i = NVUI(piano_oscil36_pint)) >= AP1.len)
         i = (NVUI(piano_oscil36_pint) -= AP1.len);
      }
     else
      {
       NVUI(piano_oscil36_pfrac) = (j = NVUI(piano_oscil36_pfrac)) - NVUI(piano_oscil36_kfrac);
       NVUI(piano_oscil36_pint) -= NVUI(piano_oscil36_kint) + (NVUI(piano_oscil36_pfrac) > j);
       if ((i = NVUI(piano_oscil36_pint)) >= AP1.len)
         i = (NVUI(piano_oscil36_pint) += AP1.len);
      }
     ret = AP1.t[i] + NVUI(piano_oscil36_pfrac)*
           ((float)(1.0/4294967296.0))*(AP1.t[i+1] - AP1.t[i]);
   }
   else
   {
     if ((NVI(piano_oscil36_fsign) = (freq >= 0)))
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
     NVUI(piano_oscil36_kint) = nint;
     NVUI(piano_oscil36_kfrac) = nfrac;
     NVI(piano_oscil36_kcyc) = 1;
     ret = AP1.t[0];
   }
   return((NV(piano_oscil36_return) = ret));

}



float piano__sym_oscil37(ENGINE_PTR_DECLARE_COMMA struct ninstr_types * nstate)
{
   float freq;
   PSIZE t;
   float ret;
   float index;
   unsigned int nint, nfrac, i, j;

   t =  TBL_GBL_cyc ;

#undef AP1
#define AP1 EV(gtables)[t]

   freq = NV(piano__tvr6);
   if (NVI(piano_oscil37_kcyc))
   {
     if (NVI(piano_oscil37_fsign))
      {
       NVUI(piano_oscil37_pfrac) = (j = NVUI(piano_oscil37_pfrac)) + NVUI(piano_oscil37_kfrac);
       NVUI(piano_oscil37_pint) += NVUI(piano_oscil37_kint) + (NVUI(piano_oscil37_pfrac) < j);
       if ((i = NVUI(piano_oscil37_pint)) >= AP1.len)
         i = (NVUI(piano_oscil37_pint) -= AP1.len);
      }
     else
      {
       NVUI(piano_oscil37_pfrac) = (j = NVUI(piano_oscil37_pfrac)) - NVUI(piano_oscil37_kfrac);
       NVUI(piano_oscil37_pint) -= NVUI(piano_oscil37_kint) + (NVUI(piano_oscil37_pfrac) > j);
       if ((i = NVUI(piano_oscil37_pint)) >= AP1.len)
         i = (NVUI(piano_oscil37_pint) += AP1.len);
      }
     ret = AP1.t[i] + NVUI(piano_oscil37_pfrac)*
           ((float)(1.0/4294967296.0))*(AP1.t[i+1] - AP1.t[i]);
   }
   else
   {
     if ((NVI(piano_oscil37_fsign) = (freq >= 0)))
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
     NVUI(piano_oscil37_kint) = nint;
     NVUI(piano_oscil37_kfrac) = nfrac;
     NVI(piano_oscil37_kcyc) = 1;
     ret = AP1.t[0];
   }
   return((NV(piano_oscil37_return) = ret));

}



float piano__sym_oscil38(ENGINE_PTR_DECLARE_COMMA struct ninstr_types * nstate)
{
   float freq;
   PSIZE t;
   float ret;
   float index;
   unsigned int nint, nfrac, i, j;

   t =  TBL_GBL_cyc ;

#undef AP1
#define AP1 EV(gtables)[t]

   freq = NV(piano__tvr7);
   if (NVI(piano_oscil38_kcyc))
   {
     if (NVI(piano_oscil38_fsign))
      {
       NVUI(piano_oscil38_pfrac) = (j = NVUI(piano_oscil38_pfrac)) + NVUI(piano_oscil38_kfrac);
       NVUI(piano_oscil38_pint) += NVUI(piano_oscil38_kint) + (NVUI(piano_oscil38_pfrac) < j);
       if ((i = NVUI(piano_oscil38_pint)) >= AP1.len)
         i = (NVUI(piano_oscil38_pint) -= AP1.len);
      }
     else
      {
       NVUI(piano_oscil38_pfrac) = (j = NVUI(piano_oscil38_pfrac)) - NVUI(piano_oscil38_kfrac);
       NVUI(piano_oscil38_pint) -= NVUI(piano_oscil38_kint) + (NVUI(piano_oscil38_pfrac) > j);
       if ((i = NVUI(piano_oscil38_pint)) >= AP1.len)
         i = (NVUI(piano_oscil38_pint) += AP1.len);
      }
     ret = AP1.t[i] + NVUI(piano_oscil38_pfrac)*
           ((float)(1.0/4294967296.0))*(AP1.t[i+1] - AP1.t[i]);
   }
   else
   {
     if ((NVI(piano_oscil38_fsign) = (freq >= 0)))
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
     NVUI(piano_oscil38_kint) = nint;
     NVUI(piano_oscil38_kfrac) = nfrac;
     NVI(piano_oscil38_kcyc) = 1;
     ret = AP1.t[0];
   }
   return((NV(piano_oscil38_return) = ret));

}



float piano__sym_oscil39(ENGINE_PTR_DECLARE_COMMA struct ninstr_types * nstate)
{
   float freq;
   PSIZE t;
   float ret;
   float index;
   unsigned int nint, nfrac, i, j;

   t =  TBL_GBL_cyc ;

#undef AP1
#define AP1 EV(gtables)[t]

   freq = NV(piano__tvr8);
   if (NVI(piano_oscil39_kcyc))
   {
     if (NVI(piano_oscil39_fsign))
      {
       NVUI(piano_oscil39_pfrac) = (j = NVUI(piano_oscil39_pfrac)) + NVUI(piano_oscil39_kfrac);
       NVUI(piano_oscil39_pint) += NVUI(piano_oscil39_kint) + (NVUI(piano_oscil39_pfrac) < j);
       if ((i = NVUI(piano_oscil39_pint)) >= AP1.len)
         i = (NVUI(piano_oscil39_pint) -= AP1.len);
      }
     else
      {
       NVUI(piano_oscil39_pfrac) = (j = NVUI(piano_oscil39_pfrac)) - NVUI(piano_oscil39_kfrac);
       NVUI(piano_oscil39_pint) -= NVUI(piano_oscil39_kint) + (NVUI(piano_oscil39_pfrac) > j);
       if ((i = NVUI(piano_oscil39_pint)) >= AP1.len)
         i = (NVUI(piano_oscil39_pint) += AP1.len);
      }
     ret = AP1.t[i] + NVUI(piano_oscil39_pfrac)*
           ((float)(1.0/4294967296.0))*(AP1.t[i+1] - AP1.t[i]);
   }
   else
   {
     if ((NVI(piano_oscil39_fsign) = (freq >= 0)))
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
     NVUI(piano_oscil39_kint) = nint;
     NVUI(piano_oscil39_kfrac) = nfrac;
     NVI(piano_oscil39_kcyc) = 1;
     ret = AP1.t[0];
   }
   return((NV(piano_oscil39_return) = ret));

}



float piano__sym_oscil40(ENGINE_PTR_DECLARE_COMMA struct ninstr_types * nstate)
{
   float freq;
   PSIZE t;
   float ret;
   float index;
   unsigned int nint, nfrac, i, j;

   t =  TBL_GBL_cyc ;

#undef AP1
#define AP1 EV(gtables)[t]

   freq = NV(piano__tvr9);
   if (NVI(piano_oscil40_kcyc))
   {
     if (NVI(piano_oscil40_fsign))
      {
       NVUI(piano_oscil40_pfrac) = (j = NVUI(piano_oscil40_pfrac)) + NVUI(piano_oscil40_kfrac);
       NVUI(piano_oscil40_pint) += NVUI(piano_oscil40_kint) + (NVUI(piano_oscil40_pfrac) < j);
       if ((i = NVUI(piano_oscil40_pint)) >= AP1.len)
         i = (NVUI(piano_oscil40_pint) -= AP1.len);
      }
     else
      {
       NVUI(piano_oscil40_pfrac) = (j = NVUI(piano_oscil40_pfrac)) - NVUI(piano_oscil40_kfrac);
       NVUI(piano_oscil40_pint) -= NVUI(piano_oscil40_kint) + (NVUI(piano_oscil40_pfrac) > j);
       if ((i = NVUI(piano_oscil40_pint)) >= AP1.len)
         i = (NVUI(piano_oscil40_pint) += AP1.len);
      }
     ret = AP1.t[i] + NVUI(piano_oscil40_pfrac)*
           ((float)(1.0/4294967296.0))*(AP1.t[i+1] - AP1.t[i]);
   }
   else
   {
     if ((NVI(piano_oscil40_fsign) = (freq >= 0)))
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
     NVUI(piano_oscil40_kint) = nint;
     NVUI(piano_oscil40_kfrac) = nfrac;
     NVI(piano_oscil40_kcyc) = 1;
     ret = AP1.t[0];
   }
   return((NV(piano_oscil40_return) = ret));

}



float piano__sym_oscil41(ENGINE_PTR_DECLARE_COMMA struct ninstr_types * nstate)
{
   float freq;
   PSIZE t;
   float ret;
   float index;
   unsigned int nint, nfrac, i, j;

   t =  TBL_GBL_cyc ;

#undef AP1
#define AP1 EV(gtables)[t]

   freq = NV(piano__tvr10);
   if (NVI(piano_oscil41_kcyc))
   {
     if (NVI(piano_oscil41_fsign))
      {
       NVUI(piano_oscil41_pfrac) = (j = NVUI(piano_oscil41_pfrac)) + NVUI(piano_oscil41_kfrac);
       NVUI(piano_oscil41_pint) += NVUI(piano_oscil41_kint) + (NVUI(piano_oscil41_pfrac) < j);
       if ((i = NVUI(piano_oscil41_pint)) >= AP1.len)
         i = (NVUI(piano_oscil41_pint) -= AP1.len);
      }
     else
      {
       NVUI(piano_oscil41_pfrac) = (j = NVUI(piano_oscil41_pfrac)) - NVUI(piano_oscil41_kfrac);
       NVUI(piano_oscil41_pint) -= NVUI(piano_oscil41_kint) + (NVUI(piano_oscil41_pfrac) > j);
       if ((i = NVUI(piano_oscil41_pint)) >= AP1.len)
         i = (NVUI(piano_oscil41_pint) += AP1.len);
      }
     ret = AP1.t[i] + NVUI(piano_oscil41_pfrac)*
           ((float)(1.0/4294967296.0))*(AP1.t[i+1] - AP1.t[i]);
   }
   else
   {
     if ((NVI(piano_oscil41_fsign) = (freq >= 0)))
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
     NVUI(piano_oscil41_kint) = nint;
     NVUI(piano_oscil41_kfrac) = nfrac;
     NVI(piano_oscil41_kcyc) = 1;
     ret = AP1.t[0];
   }
   return((NV(piano_oscil41_return) = ret));

}



float piano__sym_oscil42(ENGINE_PTR_DECLARE_COMMA struct ninstr_types * nstate)
{
   float freq;
   PSIZE t;
   float ret;
   float index;
   unsigned int nint, nfrac, i, j;

   t =  TBL_GBL_cyc ;

#undef AP1
#define AP1 EV(gtables)[t]

   freq = NV(piano__tvr11);
   if (NVI(piano_oscil42_kcyc))
   {
     if (NVI(piano_oscil42_fsign))
      {
       NVUI(piano_oscil42_pfrac) = (j = NVUI(piano_oscil42_pfrac)) + NVUI(piano_oscil42_kfrac);
       NVUI(piano_oscil42_pint) += NVUI(piano_oscil42_kint) + (NVUI(piano_oscil42_pfrac) < j);
       if ((i = NVUI(piano_oscil42_pint)) >= AP1.len)
         i = (NVUI(piano_oscil42_pint) -= AP1.len);
      }
     else
      {
       NVUI(piano_oscil42_pfrac) = (j = NVUI(piano_oscil42_pfrac)) - NVUI(piano_oscil42_kfrac);
       NVUI(piano_oscil42_pint) -= NVUI(piano_oscil42_kint) + (NVUI(piano_oscil42_pfrac) > j);
       if ((i = NVUI(piano_oscil42_pint)) >= AP1.len)
         i = (NVUI(piano_oscil42_pint) += AP1.len);
      }
     ret = AP1.t[i] + NVUI(piano_oscil42_pfrac)*
           ((float)(1.0/4294967296.0))*(AP1.t[i+1] - AP1.t[i]);
   }
   else
   {
     if ((NVI(piano_oscil42_fsign) = (freq >= 0)))
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
     NVUI(piano_oscil42_kint) = nint;
     NVUI(piano_oscil42_kfrac) = nfrac;
     NVI(piano_oscil42_kcyc) = 1;
     ret = AP1.t[0];
   }
   return((NV(piano_oscil42_return) = ret));

}



float piano__sym_oscil43(ENGINE_PTR_DECLARE_COMMA struct ninstr_types * nstate)
{
   float freq;
   PSIZE t;
   float ret;
   float index;
   unsigned int nint, nfrac, i, j;

   t =  TBL_GBL_cyc ;

#undef AP1
#define AP1 EV(gtables)[t]

   freq = NV(piano__tvr12);
   if (NVI(piano_oscil43_kcyc))
   {
     if (NVI(piano_oscil43_fsign))
      {
       NVUI(piano_oscil43_pfrac) = (j = NVUI(piano_oscil43_pfrac)) + NVUI(piano_oscil43_kfrac);
       NVUI(piano_oscil43_pint) += NVUI(piano_oscil43_kint) + (NVUI(piano_oscil43_pfrac) < j);
       if ((i = NVUI(piano_oscil43_pint)) >= AP1.len)
         i = (NVUI(piano_oscil43_pint) -= AP1.len);
      }
     else
      {
       NVUI(piano_oscil43_pfrac) = (j = NVUI(piano_oscil43_pfrac)) - NVUI(piano_oscil43_kfrac);
       NVUI(piano_oscil43_pint) -= NVUI(piano_oscil43_kint) + (NVUI(piano_oscil43_pfrac) > j);
       if ((i = NVUI(piano_oscil43_pint)) >= AP1.len)
         i = (NVUI(piano_oscil43_pint) += AP1.len);
      }
     ret = AP1.t[i] + NVUI(piano_oscil43_pfrac)*
           ((float)(1.0/4294967296.0))*(AP1.t[i+1] - AP1.t[i]);
   }
   else
   {
     if ((NVI(piano_oscil43_fsign) = (freq >= 0)))
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
     NVUI(piano_oscil43_kint) = nint;
     NVUI(piano_oscil43_kfrac) = nfrac;
     NVI(piano_oscil43_kcyc) = 1;
     ret = AP1.t[0];
   }
   return((NV(piano_oscil43_return) = ret));

}



float piano__sym_oscil44(ENGINE_PTR_DECLARE_COMMA struct ninstr_types * nstate)
{
   float freq;
   PSIZE t;
   float ret;
   float index;
   unsigned int nint, nfrac, i, j;

   t =  TBL_GBL_cyc ;

#undef AP1
#define AP1 EV(gtables)[t]

   freq = NV(piano__tvr13);
   if (NVI(piano_oscil44_kcyc))
   {
     if (NVI(piano_oscil44_fsign))
      {
       NVUI(piano_oscil44_pfrac) = (j = NVUI(piano_oscil44_pfrac)) + NVUI(piano_oscil44_kfrac);
       NVUI(piano_oscil44_pint) += NVUI(piano_oscil44_kint) + (NVUI(piano_oscil44_pfrac) < j);
       if ((i = NVUI(piano_oscil44_pint)) >= AP1.len)
         i = (NVUI(piano_oscil44_pint) -= AP1.len);
      }
     else
      {
       NVUI(piano_oscil44_pfrac) = (j = NVUI(piano_oscil44_pfrac)) - NVUI(piano_oscil44_kfrac);
       NVUI(piano_oscil44_pint) -= NVUI(piano_oscil44_kint) + (NVUI(piano_oscil44_pfrac) > j);
       if ((i = NVUI(piano_oscil44_pint)) >= AP1.len)
         i = (NVUI(piano_oscil44_pint) += AP1.len);
      }
     ret = AP1.t[i] + NVUI(piano_oscil44_pfrac)*
           ((float)(1.0/4294967296.0))*(AP1.t[i+1] - AP1.t[i]);
   }
   else
   {
     if ((NVI(piano_oscil44_fsign) = (freq >= 0)))
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
     NVUI(piano_oscil44_kint) = nint;
     NVUI(piano_oscil44_kfrac) = nfrac;
     NVI(piano_oscil44_kcyc) = 1;
     ret = AP1.t[0];
   }
   return((NV(piano_oscil44_return) = ret));

}



float piano__sym_oscil45(ENGINE_PTR_DECLARE_COMMA struct ninstr_types * nstate)
{
   float freq;
   PSIZE t;
   float ret;
   float index;
   unsigned int nint, nfrac, i, j;

   t =  TBL_GBL_cyc ;

#undef AP1
#define AP1 EV(gtables)[t]

   freq = NV(piano__tvr14);
   if (NVI(piano_oscil45_kcyc))
   {
     if (NVI(piano_oscil45_fsign))
      {
       NVUI(piano_oscil45_pfrac) = (j = NVUI(piano_oscil45_pfrac)) + NVUI(piano_oscil45_kfrac);
       NVUI(piano_oscil45_pint) += NVUI(piano_oscil45_kint) + (NVUI(piano_oscil45_pfrac) < j);
       if ((i = NVUI(piano_oscil45_pint)) >= AP1.len)
         i = (NVUI(piano_oscil45_pint) -= AP1.len);
      }
     else
      {
       NVUI(piano_oscil45_pfrac) = (j = NVUI(piano_oscil45_pfrac)) - NVUI(piano_oscil45_kfrac);
       NVUI(piano_oscil45_pint) -= NVUI(piano_oscil45_kint) + (NVUI(piano_oscil45_pfrac) > j);
       if ((i = NVUI(piano_oscil45_pint)) >= AP1.len)
         i = (NVUI(piano_oscil45_pint) += AP1.len);
      }
     ret = AP1.t[i] + NVUI(piano_oscil45_pfrac)*
           ((float)(1.0/4294967296.0))*(AP1.t[i+1] - AP1.t[i]);
   }
   else
   {
     if ((NVI(piano_oscil45_fsign) = (freq >= 0)))
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
     NVUI(piano_oscil45_kint) = nint;
     NVUI(piano_oscil45_kfrac) = nfrac;
     NVI(piano_oscil45_kcyc) = 1;
     ret = AP1.t[0];
   }
   return((NV(piano_oscil45_return) = ret));

}



float piano__sym_oscil46(ENGINE_PTR_DECLARE_COMMA struct ninstr_types * nstate)
{
   float freq;
   PSIZE t;
   float ret;
   float index;
   unsigned int nint, nfrac, i, j;

   t =  TBL_GBL_cyc ;

#undef AP1
#define AP1 EV(gtables)[t]

   freq = NV(piano__tvr15);
   if (NVI(piano_oscil46_kcyc))
   {
     if (NVI(piano_oscil46_fsign))
      {
       NVUI(piano_oscil46_pfrac) = (j = NVUI(piano_oscil46_pfrac)) + NVUI(piano_oscil46_kfrac);
       NVUI(piano_oscil46_pint) += NVUI(piano_oscil46_kint) + (NVUI(piano_oscil46_pfrac) < j);
       if ((i = NVUI(piano_oscil46_pint)) >= AP1.len)
         i = (NVUI(piano_oscil46_pint) -= AP1.len);
      }
     else
      {
       NVUI(piano_oscil46_pfrac) = (j = NVUI(piano_oscil46_pfrac)) - NVUI(piano_oscil46_kfrac);
       NVUI(piano_oscil46_pint) -= NVUI(piano_oscil46_kint) + (NVUI(piano_oscil46_pfrac) > j);
       if ((i = NVUI(piano_oscil46_pint)) >= AP1.len)
         i = (NVUI(piano_oscil46_pint) += AP1.len);
      }
     ret = AP1.t[i] + NVUI(piano_oscil46_pfrac)*
           ((float)(1.0/4294967296.0))*(AP1.t[i+1] - AP1.t[i]);
   }
   else
   {
     if ((NVI(piano_oscil46_fsign) = (freq >= 0)))
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
     NVUI(piano_oscil46_kint) = nint;
     NVUI(piano_oscil46_kfrac) = nfrac;
     NVI(piano_oscil46_kcyc) = 1;
     ret = AP1.t[0];
   }
   return((NV(piano_oscil46_return) = ret));

}



float piano__sym_oscil47(ENGINE_PTR_DECLARE_COMMA struct ninstr_types * nstate)
{
   float freq;
   PSIZE t;
   float ret;
   float index;
   unsigned int nint, nfrac, i, j;

   t =  TBL_GBL_cyc ;

#undef AP1
#define AP1 EV(gtables)[t]

   freq = NV(piano__tvr16);
   if (NVI(piano_oscil47_kcyc))
   {
     if (NVI(piano_oscil47_fsign))
      {
       NVUI(piano_oscil47_pfrac) = (j = NVUI(piano_oscil47_pfrac)) + NVUI(piano_oscil47_kfrac);
       NVUI(piano_oscil47_pint) += NVUI(piano_oscil47_kint) + (NVUI(piano_oscil47_pfrac) < j);
       if ((i = NVUI(piano_oscil47_pint)) >= AP1.len)
         i = (NVUI(piano_oscil47_pint) -= AP1.len);
      }
     else
      {
       NVUI(piano_oscil47_pfrac) = (j = NVUI(piano_oscil47_pfrac)) - NVUI(piano_oscil47_kfrac);
       NVUI(piano_oscil47_pint) -= NVUI(piano_oscil47_kint) + (NVUI(piano_oscil47_pfrac) > j);
       if ((i = NVUI(piano_oscil47_pint)) >= AP1.len)
         i = (NVUI(piano_oscil47_pint) += AP1.len);
      }
     ret = AP1.t[i] + NVUI(piano_oscil47_pfrac)*
           ((float)(1.0/4294967296.0))*(AP1.t[i+1] - AP1.t[i]);
   }
   else
   {
     if ((NVI(piano_oscil47_fsign) = (freq >= 0)))
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
     NVUI(piano_oscil47_kint) = nint;
     NVUI(piano_oscil47_kfrac) = nfrac;
     NVI(piano_oscil47_kcyc) = 1;
     ret = AP1.t[0];
   }
   return((NV(piano_oscil47_return) = ret));

}



float piano__sym_oscil48(ENGINE_PTR_DECLARE_COMMA struct ninstr_types * nstate)
{
   float freq;
   PSIZE t;
   float ret;
   float index;
   unsigned int nint, nfrac, i, j;

   t =  TBL_GBL_cyc ;

#undef AP1
#define AP1 EV(gtables)[t]

   freq = NV(piano__tvr17);
   if (NVI(piano_oscil48_kcyc))
   {
     if (NVI(piano_oscil48_fsign))
      {
       NVUI(piano_oscil48_pfrac) = (j = NVUI(piano_oscil48_pfrac)) + NVUI(piano_oscil48_kfrac);
       NVUI(piano_oscil48_pint) += NVUI(piano_oscil48_kint) + (NVUI(piano_oscil48_pfrac) < j);
       if ((i = NVUI(piano_oscil48_pint)) >= AP1.len)
         i = (NVUI(piano_oscil48_pint) -= AP1.len);
      }
     else
      {
       NVUI(piano_oscil48_pfrac) = (j = NVUI(piano_oscil48_pfrac)) - NVUI(piano_oscil48_kfrac);
       NVUI(piano_oscil48_pint) -= NVUI(piano_oscil48_kint) + (NVUI(piano_oscil48_pfrac) > j);
       if ((i = NVUI(piano_oscil48_pint)) >= AP1.len)
         i = (NVUI(piano_oscil48_pint) += AP1.len);
      }
     ret = AP1.t[i] + NVUI(piano_oscil48_pfrac)*
           ((float)(1.0/4294967296.0))*(AP1.t[i+1] - AP1.t[i]);
   }
   else
   {
     if ((NVI(piano_oscil48_fsign) = (freq >= 0)))
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
     NVUI(piano_oscil48_kint) = nint;
     NVUI(piano_oscil48_kfrac) = nfrac;
     NVI(piano_oscil48_kcyc) = 1;
     ret = AP1.t[0];
   }
   return((NV(piano_oscil48_return) = ret));

}



float piano__sym_oscil49(ENGINE_PTR_DECLARE_COMMA struct ninstr_types * nstate)
{
   float freq;
   PSIZE t;
   float ret;
   float index;
   unsigned int nint, nfrac, i, j;

   t =  TBL_GBL_cyc ;

#undef AP1
#define AP1 EV(gtables)[t]

   freq = NV(piano__tvr18);
   if (NVI(piano_oscil49_kcyc))
   {
     if (NVI(piano_oscil49_fsign))
      {
       NVUI(piano_oscil49_pfrac) = (j = NVUI(piano_oscil49_pfrac)) + NVUI(piano_oscil49_kfrac);
       NVUI(piano_oscil49_pint) += NVUI(piano_oscil49_kint) + (NVUI(piano_oscil49_pfrac) < j);
       if ((i = NVUI(piano_oscil49_pint)) >= AP1.len)
         i = (NVUI(piano_oscil49_pint) -= AP1.len);
      }
     else
      {
       NVUI(piano_oscil49_pfrac) = (j = NVUI(piano_oscil49_pfrac)) - NVUI(piano_oscil49_kfrac);
       NVUI(piano_oscil49_pint) -= NVUI(piano_oscil49_kint) + (NVUI(piano_oscil49_pfrac) > j);
       if ((i = NVUI(piano_oscil49_pint)) >= AP1.len)
         i = (NVUI(piano_oscil49_pint) += AP1.len);
      }
     ret = AP1.t[i] + NVUI(piano_oscil49_pfrac)*
           ((float)(1.0/4294967296.0))*(AP1.t[i+1] - AP1.t[i]);
   }
   else
   {
     if ((NVI(piano_oscil49_fsign) = (freq >= 0)))
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
     NVUI(piano_oscil49_kint) = nint;
     NVUI(piano_oscil49_kfrac) = nfrac;
     NVI(piano_oscil49_kcyc) = 1;
     ret = AP1.t[0];
   }
   return((NV(piano_oscil49_return) = ret));

}



float piano__sym_oscil50(ENGINE_PTR_DECLARE_COMMA struct ninstr_types * nstate)
{
   float freq;
   PSIZE t;
   float ret;
   float index;
   unsigned int nint, nfrac, i, j;

   t =  TBL_GBL_cyc ;

#undef AP1
#define AP1 EV(gtables)[t]

   freq = NV(piano__tvr19);
   if (NVI(piano_oscil50_kcyc))
   {
     if (NVI(piano_oscil50_fsign))
      {
       NVUI(piano_oscil50_pfrac) = (j = NVUI(piano_oscil50_pfrac)) + NVUI(piano_oscil50_kfrac);
       NVUI(piano_oscil50_pint) += NVUI(piano_oscil50_kint) + (NVUI(piano_oscil50_pfrac) < j);
       if ((i = NVUI(piano_oscil50_pint)) >= AP1.len)
         i = (NVUI(piano_oscil50_pint) -= AP1.len);
      }
     else
      {
       NVUI(piano_oscil50_pfrac) = (j = NVUI(piano_oscil50_pfrac)) - NVUI(piano_oscil50_kfrac);
       NVUI(piano_oscil50_pint) -= NVUI(piano_oscil50_kint) + (NVUI(piano_oscil50_pfrac) > j);
       if ((i = NVUI(piano_oscil50_pint)) >= AP1.len)
         i = (NVUI(piano_oscil50_pint) += AP1.len);
      }
     ret = AP1.t[i] + NVUI(piano_oscil50_pfrac)*
           ((float)(1.0/4294967296.0))*(AP1.t[i+1] - AP1.t[i]);
   }
   else
   {
     if ((NVI(piano_oscil50_fsign) = (freq >= 0)))
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
     NVUI(piano_oscil50_kint) = nint;
     NVUI(piano_oscil50_kfrac) = nfrac;
     NVI(piano_oscil50_kcyc) = 1;
     ret = AP1.t[0];
   }
   return((NV(piano_oscil50_return) = ret));

}



float piano__sym_oscil51(ENGINE_PTR_DECLARE_COMMA struct ninstr_types * nstate)
{
   float freq;
   PSIZE t;
   float ret;
   float index;
   unsigned int nint, nfrac, i, j;

   t =  TBL_GBL_cyc ;

#undef AP1
#define AP1 EV(gtables)[t]

   freq = NV(piano__tvr20);
   if (NVI(piano_oscil51_kcyc))
   {
     if (NVI(piano_oscil51_fsign))
      {
       NVUI(piano_oscil51_pfrac) = (j = NVUI(piano_oscil51_pfrac)) + NVUI(piano_oscil51_kfrac);
       NVUI(piano_oscil51_pint) += NVUI(piano_oscil51_kint) + (NVUI(piano_oscil51_pfrac) < j);
       if ((i = NVUI(piano_oscil51_pint)) >= AP1.len)
         i = (NVUI(piano_oscil51_pint) -= AP1.len);
      }
     else
      {
       NVUI(piano_oscil51_pfrac) = (j = NVUI(piano_oscil51_pfrac)) - NVUI(piano_oscil51_kfrac);
       NVUI(piano_oscil51_pint) -= NVUI(piano_oscil51_kint) + (NVUI(piano_oscil51_pfrac) > j);
       if ((i = NVUI(piano_oscil51_pint)) >= AP1.len)
         i = (NVUI(piano_oscil51_pint) += AP1.len);
      }
     ret = AP1.t[i] + NVUI(piano_oscil51_pfrac)*
           ((float)(1.0/4294967296.0))*(AP1.t[i+1] - AP1.t[i]);
   }
   else
   {
     if ((NVI(piano_oscil51_fsign) = (freq >= 0)))
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
     NVUI(piano_oscil51_kint) = nint;
     NVUI(piano_oscil51_kfrac) = nfrac;
     NVI(piano_oscil51_kcyc) = 1;
     ret = AP1.t[0];
   }
   return((NV(piano_oscil51_return) = ret));

}



float piano__sym_oscil52(ENGINE_PTR_DECLARE_COMMA struct ninstr_types * nstate)
{
   float freq;
   PSIZE t;
   float ret;
   float index;
   unsigned int nint, nfrac, i, j;

   t =  TBL_GBL_cyc ;

#undef AP1
#define AP1 EV(gtables)[t]

   freq = NV(piano__tvr21);
   if (NVI(piano_oscil52_kcyc))
   {
     if (NVI(piano_oscil52_fsign))
      {
       NVUI(piano_oscil52_pfrac) = (j = NVUI(piano_oscil52_pfrac)) + NVUI(piano_oscil52_kfrac);
       NVUI(piano_oscil52_pint) += NVUI(piano_oscil52_kint) + (NVUI(piano_oscil52_pfrac) < j);
       if ((i = NVUI(piano_oscil52_pint)) >= AP1.len)
         i = (NVUI(piano_oscil52_pint) -= AP1.len);
      }
     else
      {
       NVUI(piano_oscil52_pfrac) = (j = NVUI(piano_oscil52_pfrac)) - NVUI(piano_oscil52_kfrac);
       NVUI(piano_oscil52_pint) -= NVUI(piano_oscil52_kint) + (NVUI(piano_oscil52_pfrac) > j);
       if ((i = NVUI(piano_oscil52_pint)) >= AP1.len)
         i = (NVUI(piano_oscil52_pint) += AP1.len);
      }
     ret = AP1.t[i] + NVUI(piano_oscil52_pfrac)*
           ((float)(1.0/4294967296.0))*(AP1.t[i+1] - AP1.t[i]);
   }
   else
   {
     if ((NVI(piano_oscil52_fsign) = (freq >= 0)))
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
     NVUI(piano_oscil52_kint) = nint;
     NVUI(piano_oscil52_kfrac) = nfrac;
     NVI(piano_oscil52_kcyc) = 1;
     ret = AP1.t[0];
   }
   return((NV(piano_oscil52_return) = ret));

}



float piano__sym_oscil53(ENGINE_PTR_DECLARE_COMMA struct ninstr_types * nstate)
{
   float freq;
   PSIZE t;
   float ret;
   float index;
   unsigned int nint, nfrac, i, j;

   t =  TBL_GBL_cyc ;

#undef AP1
#define AP1 EV(gtables)[t]

   freq = NV(piano__tvr22);
   if (NVI(piano_oscil53_kcyc))
   {
     if (NVI(piano_oscil53_fsign))
      {
       NVUI(piano_oscil53_pfrac) = (j = NVUI(piano_oscil53_pfrac)) + NVUI(piano_oscil53_kfrac);
       NVUI(piano_oscil53_pint) += NVUI(piano_oscil53_kint) + (NVUI(piano_oscil53_pfrac) < j);
       if ((i = NVUI(piano_oscil53_pint)) >= AP1.len)
         i = (NVUI(piano_oscil53_pint) -= AP1.len);
      }
     else
      {
       NVUI(piano_oscil53_pfrac) = (j = NVUI(piano_oscil53_pfrac)) - NVUI(piano_oscil53_kfrac);
       NVUI(piano_oscil53_pint) -= NVUI(piano_oscil53_kint) + (NVUI(piano_oscil53_pfrac) > j);
       if ((i = NVUI(piano_oscil53_pint)) >= AP1.len)
         i = (NVUI(piano_oscil53_pint) += AP1.len);
      }
     ret = AP1.t[i] + NVUI(piano_oscil53_pfrac)*
           ((float)(1.0/4294967296.0))*(AP1.t[i+1] - AP1.t[i]);
   }
   else
   {
     if ((NVI(piano_oscil53_fsign) = (freq >= 0)))
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
     NVUI(piano_oscil53_kint) = nint;
     NVUI(piano_oscil53_kfrac) = nfrac;
     NVI(piano_oscil53_kcyc) = 1;
     ret = AP1.t[0];
   }
   return((NV(piano_oscil53_return) = ret));

}



float piano__sym_oscil54(ENGINE_PTR_DECLARE_COMMA struct ninstr_types * nstate)
{
   float freq;
   PSIZE t;
   float ret;
   float index;
   unsigned int nint, nfrac, i, j;

   t =  TBL_GBL_cyc ;

#undef AP1
#define AP1 EV(gtables)[t]

   freq = NV(piano__tvr23);
   if (NVI(piano_oscil54_kcyc))
   {
     if (NVI(piano_oscil54_fsign))
      {
       NVUI(piano_oscil54_pfrac) = (j = NVUI(piano_oscil54_pfrac)) + NVUI(piano_oscil54_kfrac);
       NVUI(piano_oscil54_pint) += NVUI(piano_oscil54_kint) + (NVUI(piano_oscil54_pfrac) < j);
       if ((i = NVUI(piano_oscil54_pint)) >= AP1.len)
         i = (NVUI(piano_oscil54_pint) -= AP1.len);
      }
     else
      {
       NVUI(piano_oscil54_pfrac) = (j = NVUI(piano_oscil54_pfrac)) - NVUI(piano_oscil54_kfrac);
       NVUI(piano_oscil54_pint) -= NVUI(piano_oscil54_kint) + (NVUI(piano_oscil54_pfrac) > j);
       if ((i = NVUI(piano_oscil54_pint)) >= AP1.len)
         i = (NVUI(piano_oscil54_pint) += AP1.len);
      }
     ret = AP1.t[i] + NVUI(piano_oscil54_pfrac)*
           ((float)(1.0/4294967296.0))*(AP1.t[i+1] - AP1.t[i]);
   }
   else
   {
     if ((NVI(piano_oscil54_fsign) = (freq >= 0)))
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
     NVUI(piano_oscil54_kint) = nint;
     NVUI(piano_oscil54_kfrac) = nfrac;
     NVI(piano_oscil54_kcyc) = 1;
     ret = AP1.t[0];
   }
   return((NV(piano_oscil54_return) = ret));

}



float piano__sym_oscil55(ENGINE_PTR_DECLARE_COMMA struct ninstr_types * nstate)
{
   float freq;
   PSIZE t;
   float ret;
   float index;
   unsigned int nint, nfrac, i, j;

   t =  TBL_GBL_cyc ;

#undef AP1
#define AP1 EV(gtables)[t]

   freq = NV(piano__tvr24);
   if (NVI(piano_oscil55_kcyc))
   {
     if (NVI(piano_oscil55_fsign))
      {
       NVUI(piano_oscil55_pfrac) = (j = NVUI(piano_oscil55_pfrac)) + NVUI(piano_oscil55_kfrac);
       NVUI(piano_oscil55_pint) += NVUI(piano_oscil55_kint) + (NVUI(piano_oscil55_pfrac) < j);
       if ((i = NVUI(piano_oscil55_pint)) >= AP1.len)
         i = (NVUI(piano_oscil55_pint) -= AP1.len);
      }
     else
      {
       NVUI(piano_oscil55_pfrac) = (j = NVUI(piano_oscil55_pfrac)) - NVUI(piano_oscil55_kfrac);
       NVUI(piano_oscil55_pint) -= NVUI(piano_oscil55_kint) + (NVUI(piano_oscil55_pfrac) > j);
       if ((i = NVUI(piano_oscil55_pint)) >= AP1.len)
         i = (NVUI(piano_oscil55_pint) += AP1.len);
      }
     ret = AP1.t[i] + NVUI(piano_oscil55_pfrac)*
           ((float)(1.0/4294967296.0))*(AP1.t[i+1] - AP1.t[i]);
   }
   else
   {
     if ((NVI(piano_oscil55_fsign) = (freq >= 0)))
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
     NVUI(piano_oscil55_kint) = nint;
     NVUI(piano_oscil55_kfrac) = nfrac;
     NVI(piano_oscil55_kcyc) = 1;
     ret = AP1.t[0];
   }
   return((NV(piano_oscil55_return) = ret));

}



float piano__sym_oscil56(ENGINE_PTR_DECLARE_COMMA struct ninstr_types * nstate)
{
   float freq;
   PSIZE t;
   float ret;
   float index;
   unsigned int nint, nfrac, i, j;

   t =  TBL_GBL_cyc ;

#undef AP1
#define AP1 EV(gtables)[t]

   freq = NV(piano__tvr25);
   if (NVI(piano_oscil56_kcyc))
   {
     if (NVI(piano_oscil56_fsign))
      {
       NVUI(piano_oscil56_pfrac) = (j = NVUI(piano_oscil56_pfrac)) + NVUI(piano_oscil56_kfrac);
       NVUI(piano_oscil56_pint) += NVUI(piano_oscil56_kint) + (NVUI(piano_oscil56_pfrac) < j);
       if ((i = NVUI(piano_oscil56_pint)) >= AP1.len)
         i = (NVUI(piano_oscil56_pint) -= AP1.len);
      }
     else
      {
       NVUI(piano_oscil56_pfrac) = (j = NVUI(piano_oscil56_pfrac)) - NVUI(piano_oscil56_kfrac);
       NVUI(piano_oscil56_pint) -= NVUI(piano_oscil56_kint) + (NVUI(piano_oscil56_pfrac) > j);
       if ((i = NVUI(piano_oscil56_pint)) >= AP1.len)
         i = (NVUI(piano_oscil56_pint) += AP1.len);
      }
     ret = AP1.t[i] + NVUI(piano_oscil56_pfrac)*
           ((float)(1.0/4294967296.0))*(AP1.t[i+1] - AP1.t[i]);
   }
   else
   {
     if ((NVI(piano_oscil56_fsign) = (freq >= 0)))
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
     NVUI(piano_oscil56_kint) = nint;
     NVUI(piano_oscil56_kfrac) = nfrac;
     NVI(piano_oscil56_kcyc) = 1;
     ret = AP1.t[0];
   }
   return((NV(piano_oscil56_return) = ret));

}



float piano__sym_oscil57(ENGINE_PTR_DECLARE_COMMA struct ninstr_types * nstate)
{
   float freq;
   PSIZE t;
   float ret;
   float index;
   unsigned int nint, nfrac, i, j;

   t =  TBL_GBL_cyc ;

#undef AP1
#define AP1 EV(gtables)[t]

   freq = NV(piano__tvr26);
   if (NVI(piano_oscil57_kcyc))
   {
     if (NVI(piano_oscil57_fsign))
      {
       NVUI(piano_oscil57_pfrac) = (j = NVUI(piano_oscil57_pfrac)) + NVUI(piano_oscil57_kfrac);
       NVUI(piano_oscil57_pint) += NVUI(piano_oscil57_kint) + (NVUI(piano_oscil57_pfrac) < j);
       if ((i = NVUI(piano_oscil57_pint)) >= AP1.len)
         i = (NVUI(piano_oscil57_pint) -= AP1.len);
      }
     else
      {
       NVUI(piano_oscil57_pfrac) = (j = NVUI(piano_oscil57_pfrac)) - NVUI(piano_oscil57_kfrac);
       NVUI(piano_oscil57_pint) -= NVUI(piano_oscil57_kint) + (NVUI(piano_oscil57_pfrac) > j);
       if ((i = NVUI(piano_oscil57_pint)) >= AP1.len)
         i = (NVUI(piano_oscil57_pint) += AP1.len);
      }
     ret = AP1.t[i] + NVUI(piano_oscil57_pfrac)*
           ((float)(1.0/4294967296.0))*(AP1.t[i+1] - AP1.t[i]);
   }
   else
   {
     if ((NVI(piano_oscil57_fsign) = (freq >= 0)))
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
     NVUI(piano_oscil57_kint) = nint;
     NVUI(piano_oscil57_kfrac) = nfrac;
     NVI(piano_oscil57_kcyc) = 1;
     ret = AP1.t[0];
   }
   return((NV(piano_oscil57_return) = ret));

}



float piano__sym_oscil58(ENGINE_PTR_DECLARE_COMMA struct ninstr_types * nstate)
{
   float freq;
   PSIZE t;
   float ret;
   float index;
   unsigned int nint, nfrac, i, j;

   t =  TBL_GBL_cyc ;

#undef AP1
#define AP1 EV(gtables)[t]

   freq = NV(piano__tvr27);
   if (NVI(piano_oscil58_kcyc))
   {
     if (NVI(piano_oscil58_fsign))
      {
       NVUI(piano_oscil58_pfrac) = (j = NVUI(piano_oscil58_pfrac)) + NVUI(piano_oscil58_kfrac);
       NVUI(piano_oscil58_pint) += NVUI(piano_oscil58_kint) + (NVUI(piano_oscil58_pfrac) < j);
       if ((i = NVUI(piano_oscil58_pint)) >= AP1.len)
         i = (NVUI(piano_oscil58_pint) -= AP1.len);
      }
     else
      {
       NVUI(piano_oscil58_pfrac) = (j = NVUI(piano_oscil58_pfrac)) - NVUI(piano_oscil58_kfrac);
       NVUI(piano_oscil58_pint) -= NVUI(piano_oscil58_kint) + (NVUI(piano_oscil58_pfrac) > j);
       if ((i = NVUI(piano_oscil58_pint)) >= AP1.len)
         i = (NVUI(piano_oscil58_pint) += AP1.len);
      }
     ret = AP1.t[i] + NVUI(piano_oscil58_pfrac)*
           ((float)(1.0/4294967296.0))*(AP1.t[i+1] - AP1.t[i]);
   }
   else
   {
     if ((NVI(piano_oscil58_fsign) = (freq >= 0)))
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
     NVUI(piano_oscil58_kint) = nint;
     NVUI(piano_oscil58_kfrac) = nfrac;
     NVI(piano_oscil58_kcyc) = 1;
     ret = AP1.t[0];
   }
   return((NV(piano_oscil58_return) = ret));

}



float piano__sym_oscil59(ENGINE_PTR_DECLARE_COMMA struct ninstr_types * nstate)
{
   float freq;
   PSIZE t;
   float ret;
   float index;
   unsigned int nint, nfrac, i, j;

   t =  TBL_GBL_cyc ;

#undef AP1
#define AP1 EV(gtables)[t]

   freq = NV(piano__tvr28);
   if (NVI(piano_oscil59_kcyc))
   {
     if (NVI(piano_oscil59_fsign))
      {
       NVUI(piano_oscil59_pfrac) = (j = NVUI(piano_oscil59_pfrac)) + NVUI(piano_oscil59_kfrac);
       NVUI(piano_oscil59_pint) += NVUI(piano_oscil59_kint) + (NVUI(piano_oscil59_pfrac) < j);
       if ((i = NVUI(piano_oscil59_pint)) >= AP1.len)
         i = (NVUI(piano_oscil59_pint) -= AP1.len);
      }
     else
      {
       NVUI(piano_oscil59_pfrac) = (j = NVUI(piano_oscil59_pfrac)) - NVUI(piano_oscil59_kfrac);
       NVUI(piano_oscil59_pint) -= NVUI(piano_oscil59_kint) + (NVUI(piano_oscil59_pfrac) > j);
       if ((i = NVUI(piano_oscil59_pint)) >= AP1.len)
         i = (NVUI(piano_oscil59_pint) += AP1.len);
      }
     ret = AP1.t[i] + NVUI(piano_oscil59_pfrac)*
           ((float)(1.0/4294967296.0))*(AP1.t[i+1] - AP1.t[i]);
   }
   else
   {
     if ((NVI(piano_oscil59_fsign) = (freq >= 0)))
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
     NVUI(piano_oscil59_kint) = nint;
     NVUI(piano_oscil59_kfrac) = nfrac;
     NVI(piano_oscil59_kcyc) = 1;
     ret = AP1.t[0];
   }
   return((NV(piano_oscil59_return) = ret));

}



float piano__sym_oscil60(ENGINE_PTR_DECLARE_COMMA struct ninstr_types * nstate)
{
   float freq;
   PSIZE t;
   float ret;
   float index;
   unsigned int nint, nfrac, i, j;

   t =  TBL_GBL_cyc ;

#undef AP1
#define AP1 EV(gtables)[t]

   freq = NV(piano__tvr29);
   if (NVI(piano_oscil60_kcyc))
   {
     if (NVI(piano_oscil60_fsign))
      {
       NVUI(piano_oscil60_pfrac) = (j = NVUI(piano_oscil60_pfrac)) + NVUI(piano_oscil60_kfrac);
       NVUI(piano_oscil60_pint) += NVUI(piano_oscil60_kint) + (NVUI(piano_oscil60_pfrac) < j);
       if ((i = NVUI(piano_oscil60_pint)) >= AP1.len)
         i = (NVUI(piano_oscil60_pint) -= AP1.len);
      }
     else
      {
       NVUI(piano_oscil60_pfrac) = (j = NVUI(piano_oscil60_pfrac)) - NVUI(piano_oscil60_kfrac);
       NVUI(piano_oscil60_pint) -= NVUI(piano_oscil60_kint) + (NVUI(piano_oscil60_pfrac) > j);
       if ((i = NVUI(piano_oscil60_pint)) >= AP1.len)
         i = (NVUI(piano_oscil60_pint) += AP1.len);
      }
     ret = AP1.t[i] + NVUI(piano_oscil60_pfrac)*
           ((float)(1.0/4294967296.0))*(AP1.t[i+1] - AP1.t[i]);
   }
   else
   {
     if ((NVI(piano_oscil60_fsign) = (freq >= 0)))
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
     NVUI(piano_oscil60_kint) = nint;
     NVUI(piano_oscil60_kfrac) = nfrac;
     NVI(piano_oscil60_kcyc) = 1;
     ret = AP1.t[0];
   }
   return((NV(piano_oscil60_return) = ret));

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

void piano_ipass(ENGINE_PTR_DECLARE_COMMA struct ninstr_types * nstate)
{
   int i;

memset(&(NVU(0)), 0, piano_ENDVAR*sizeof(NVU(0)));
memset(&(NT(0)), 0, piano_ENDTBL*sizeof(NT(0)));
   NV(piano_fr) = 
   NS(iline->p[piano_fr]);
NV(piano__tvr0) = NV(piano_fr) *  1.0F ;
 NV(piano__tvr1) = NV(piano_fr) *  2.0F ;
 NV(piano__tvr2) = NV(piano_fr) *  3.0F ;
 NV(piano__tvr3) = NV(piano_fr) *  4.0F ;
 NV(piano__tvr4) = NV(piano_fr) *  5.0F ;
 NV(piano__tvr5) = NV(piano_fr) *  6.0F ;
 NV(piano__tvr6) = NV(piano_fr) *  7.0F ;
 NV(piano__tvr7) = NV(piano_fr) *  8.0F ;
 NV(piano__tvr8) = NV(piano_fr) *  9.0F ;
 NV(piano__tvr9) = NV(piano_fr) *  10.0F ;
 NV(piano__tvr10) = NV(piano_fr) *  11.0F ;
 NV(piano__tvr11) = NV(piano_fr) *  12.0F ;
 NV(piano__tvr12) = NV(piano_fr) *  13.0F ;
 NV(piano__tvr13) = NV(piano_fr) *  14.0F ;
 NV(piano__tvr14) = NV(piano_fr) *  15.0F ;
 NV(piano__tvr15) = NV(piano_fr) *  16.0F ;
 NV(piano__tvr16) = NV(piano_fr) *  17.0F ;
 NV(piano__tvr17) = NV(piano_fr) *  18.0F ;
 NV(piano__tvr18) = NV(piano_fr) *  19.0F ;
 NV(piano__tvr19) = NV(piano_fr) *  20.0F ;
 NV(piano__tvr20) = NV(piano_fr) *  21.0F ;
 NV(piano__tvr21) = NV(piano_fr) *  22.0F ;
 NV(piano__tvr22) = NV(piano_fr) *  23.0F ;
 NV(piano__tvr23) = NV(piano_fr) *  24.0F ;
 NV(piano__tvr24) = NV(piano_fr) *  25.0F ;
 NV(piano__tvr25) = NV(piano_fr) *  26.0F ;
 NV(piano__tvr26) = NV(piano_fr) *  27.0F ;
 NV(piano__tvr27) = NV(piano_fr) *  28.0F ;
 NV(piano__tvr28) = NV(piano_fr) *  29.0F ;
 NV(piano__tvr29) = NV(piano_fr) *  30.0F ;
 
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

void piano_kpass(ENGINE_PTR_DECLARE_COMMA struct ninstr_types * nstate)
{

   int i;

NV(piano_env01) = piano__sym_kline1(NSP);
 NV(piano_env02) = piano__sym_kline2(NSP);
 NV(piano_env03) = piano__sym_kline3(NSP);
 NV(piano_env04) = piano__sym_kline4(NSP);
 NV(piano_env05) = piano__sym_kline5(NSP);
 NV(piano_env06) = piano__sym_kline6(NSP);
 NV(piano_env07) = piano__sym_kline7(NSP);
 NV(piano_env08) = piano__sym_kline8(NSP);
 NV(piano_env09) = piano__sym_kline9(NSP);
 NV(piano_env10) = piano__sym_kline10(NSP);
 NV(piano_env11) = piano__sym_kline11(NSP);
 NV(piano_env12) = piano__sym_kline12(NSP);
 NV(piano_env13) = piano__sym_kline13(NSP);
 NV(piano_env14) = piano__sym_kline14(NSP);
 NV(piano_env15) = piano__sym_kline15(NSP);
 NV(piano_env16) = piano__sym_kline16(NSP);
 NV(piano_env17) = piano__sym_kline17(NSP);
 NV(piano_env18) = piano__sym_kline18(NSP);
 NV(piano_env19) = piano__sym_kline19(NSP);
 NV(piano_env20) = piano__sym_kline20(NSP);
 NV(piano_env21) = piano__sym_kline21(NSP);
 NV(piano_env22) = piano__sym_kline22(NSP);
 NV(piano_env23) = piano__sym_kline23(NSP);
 NV(piano_env24) = piano__sym_kline24(NSP);
 NV(piano_env25) = piano__sym_kline25(NSP);
 NV(piano_env26) = piano__sym_kline26(NSP);
 NV(piano_env27) = piano__sym_kline27(NSP);
 NV(piano_env28) = piano__sym_kline28(NSP);
 NV(piano_env29) = piano__sym_kline29(NSP);
 NV(piano_env30) = piano__sym_kline30(NSP);
 
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

void piano_apass(ENGINE_PTR_DECLARE_COMMA struct ninstr_types * nstate)
{

NV(piano_y01) = NV(piano_env01) * piano__sym_oscil31(NSP);
 NV(piano_y02) = NV(piano_env02) * piano__sym_oscil32(NSP);
 NV(piano_y03) = NV(piano_env03) * piano__sym_oscil33(NSP);
 NV(piano_y04) = NV(piano_env04) * piano__sym_oscil34(NSP);
 NV(piano_y05) = NV(piano_env05) * piano__sym_oscil35(NSP);
 NV(piano_y06) = NV(piano_env06) * piano__sym_oscil36(NSP);
 NV(piano_y07) = NV(piano_env07) * piano__sym_oscil37(NSP);
 NV(piano_y08) = NV(piano_env08) * piano__sym_oscil38(NSP);
 NV(piano_y09) = NV(piano_env09) * piano__sym_oscil39(NSP);
 NV(piano_y10) = NV(piano_env10) * piano__sym_oscil40(NSP);
 NV(piano_y11) = NV(piano_env11) * piano__sym_oscil41(NSP);
 NV(piano_y12) = NV(piano_env12) * piano__sym_oscil42(NSP);
 NV(piano_y13) = NV(piano_env13) * piano__sym_oscil43(NSP);
 NV(piano_y14) = NV(piano_env14) * piano__sym_oscil44(NSP);
 NV(piano_y15) = NV(piano_env15) * piano__sym_oscil45(NSP);
 NV(piano_y16) = NV(piano_env16) * piano__sym_oscil46(NSP);
 NV(piano_y17) = NV(piano_env17) * piano__sym_oscil47(NSP);
 NV(piano_y18) = NV(piano_env18) * piano__sym_oscil48(NSP);
 NV(piano_y19) = NV(piano_env19) * piano__sym_oscil49(NSP);
 NV(piano_y20) = NV(piano_env20) * piano__sym_oscil50(NSP);
 NV(piano_y21) = NV(piano_env21) * piano__sym_oscil51(NSP);
 NV(piano_y22) = NV(piano_env22) * piano__sym_oscil52(NSP);
 NV(piano_y23) = NV(piano_env23) * piano__sym_oscil53(NSP);
 NV(piano_y24) = NV(piano_env24) * piano__sym_oscil54(NSP);
 NV(piano_y25) = NV(piano_env25) * piano__sym_oscil55(NSP);
 NV(piano_y26) = NV(piano_env26) * piano__sym_oscil56(NSP);
 NV(piano_y27) = NV(piano_env27) * piano__sym_oscil57(NSP);
 NV(piano_y28) = NV(piano_env28) * piano__sym_oscil58(NSP);
 NV(piano_y29) = NV(piano_env29) * piano__sym_oscil59(NSP);
 NV(piano_y30) = NV(piano_env30) * piano__sym_oscil60(NSP);
 TB(BUS_output_bus + 0) += NV(piano_y01) + NV(piano_y02) + NV(piano_y03) + NV(piano_y04) + NV(piano_y05) + NV(piano_y06) + NV(piano_y07) + NV(piano_y08) + NV(piano_y09) + NV(piano_y10) + NV(piano_y11) + NV(piano_y12) + NV(piano_y13) + NV(piano_y14) + NV(piano_y15) + NV(piano_y16) + NV(piano_y17) + NV(piano_y18) + NV(piano_y19) + NV(piano_y20) + NV(piano_y21) + NV(piano_y22) + NV(piano_y23) + NV(piano_y24) + NV(piano_y25) + NV(piano_y26) + NV(piano_y27) + NV(piano_y28) + NV(piano_y29) + NV(piano_y30);
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

  if (EV(s_piano)[0].noteon == PLAYING)
   piano_apass(ENGINE_PTR_COMMA EV(s_piano)[0].nstate);

}

int main_kpass(ENGINE_PTR_DECLARE)

{
  instr_line * sysidx;

  if (EV(s_piano)[0].noteon == PLAYING)
   piano_kpass(ENGINE_PTR_COMMA EV(s_piano)[0].nstate);

  return EV(graceful_exit);
}

void main_ipass(ENGINE_PTR_DECLARE)

{
  int i;
  instr_line * sysidx;

  sysidx = &EV(s_piano)[0];
  switch(sysidx->noteon) {
   case PLAYING:
   if (sysidx->released)
    {
     if (sysidx->turnoff)
      {
        sysidx->noteon = ALLDONE;
        for (i = 0; i < piano_ENDTBL; i++)
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
           for (i = 0; i < piano_ENDTBL; i++)
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
      piano_ipass(ENGINE_PTR_COMMA sysidx->nstate);
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





