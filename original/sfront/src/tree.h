
/*
#    Sfront, a SAOL to C translator    
#    This file: Include file for shared variables
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


#ifndef _SFRONT_TREE_H
#define _SFRONT_TREE_H 1

#define IDSTRING "0.99 08/30/11"

#include <stdio.h>
#include <math.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <float.h>
#include <limits.h>

#include "csrclib.h"
#include "csyslib.h"
#include "asyslib.h"
#include "psyslib.h"
#include "nsyslib.h"

extern void * alloca(size_t);

#define S_RESERVED0    0x00
#define S_AOPCODE      0x01
#define S_ASIG         0x02
#define S_ELSE         0x03
#define S_EXPORTS      0x04
#define S_EXTEND       0x05
#define S_GLOBAL       0x06     
#define S_IF           0x07
#define S_IMPORTS      0x08
#define S_INCHANNELS   0x09
#define S_INSTR        0x0A
#define S_IOPCODE      0x0B
#define S_IVAR         0x0C
#define S_KOPCODE      0x0D
#define S_KRATE        0x0E
#define S_KSIG         0x0F

#define S_MAP          0x10
#define S_OPARRAY      0x11
#define S_OPCODE       0x12
#define S_OUTBUS       0x13
#define S_OUTCHANNELS  0x14
#define S_OUTPUT       0x15
#define S_RETURN       0x16
#define S_ROUTE        0x17
#define S_SEND         0x18
#define S_SEQUENCE     0x19
#define S_SASBF        0x1A
#define S_SPATIALIZE   0x1B
#define S_SRATE        0x1C
#define S_TABLE        0x1D
#define S_TABLEMAP     0x1E
#define S_TEMPLATE     0x1F

#define S_TURNOFF      0x20
#define S_WHILE        0x21
#define S_WITH         0x22
#define S_XSIG         0x23
#define S_INTERP       0x24

#define S_PRESET       0x25
#define S_RESERVED1    0x26
#define S_RESERVED2    0x27
#define S_RESERVED3    0x28
#define S_RESERVED4    0x29
#define S_RESERVED5    0x2A
#define S_RESERVED6    0x2B
#define S_RESERVED7    0x2C
#define S_RESERVED8    0x2D
#define S_RESERVED9    0x2E
#define S_RESERVED10   0x2F

#define S_K_RATE       0x30
#define S_S_RATE       0x31
#define S_INCHAN       0x32
#define S_OUTCHAN      0x33
#define S_TIME         0x34
#define S_DUR          0x35
#define S_MIDICTRL     0x36
#define S_MIDITOUCH    0x37
#define S_MIDIBEND     0x38
#define S_INPUT        0x39
#define S_INGROUP      0x3A
#define S_RELEASED     0x3B
#define S_CPULOAD      0x3C
#define S_POSITION     0x3D
#define S_DIRECTION    0x3E
#define S_LISTENERPOSITION 0x3F

#define S_LISTENERDIRECTION 0x40
#define S_MINFRONT     0x41
#define S_MINBACK      0x42
#define S_MAXFRONT     0x43
#define S_MAXBACK      0x44
#define S_PARAMS       0x45
#define S_ITIME        0x46
#define S_RESERVED11   0x47
#define S_CHANNEL      0x48
#define S_INPUT_BUS    0x49
#define S_OUTPUT_BUS   0x4A
#define S_STARTUP      0x4B
#define S_RESERVED15   0x4C
#define S_RESERVED16   0x4D
#define S_RESERVED17   0x4E
#define S_RESERVED18   0x4F

#define S_AND          0x50
#define S_OR           0x51
#define S_GEQ          0x52
#define S_LEQ          0x53
#define S_NEQ          0x54
#define S_EQEQ         0x55
#define S_MINUS        0x56
#define S_STAR         0x57
#define S_SLASH        0x58
#define S_PLUS         0x59
#define S_GT           0x5A
#define S_LT           0x5B
#define S_Q            0x5C
#define S_COL          0x5D
#define S_LP           0x5E
#define S_RP           0x5F

#define S_LC           0x60
#define S_RC           0x61
#define S_LB           0x62
#define S_RB           0x63
#define S_SEM          0x64
#define S_COM          0x65
#define S_EQ           0x66
#define S_NOT          0x67
#define S_RESERVED19   0x68
#define S_RESERVED20   0x69
#define S_RESERVED21   0x6A
#define S_RESERVED22   0x6B
#define S_RESERVED23   0x6C
#define S_RESERVED24   0x6D
#define S_RESERVED25   0x6E
#define S_SAMPLE       0x6F

#define S_DATA         0x70
#define S_RANDOM       0x71
#define S_STEP         0x72
#define S_LINESEG      0x73
#define S_EXPSEG       0x74
#define S_CUBICSEG     0x75
#define S_POLYNOMIAL   0x76
#define S_SPLINE       0x77
#define S_WINDOW       0x78
#define S_HARM         0x79
#define S_HARM_PHASE   0x7A
#define S_PERIODIC     0x7B
#define S_BUZZWAVE     0x7C
#define S_CONCAT       0x7D
#define S_EMPTY        0x7E
#define S_DESTROY      0x7F

#define S_INT          0x80
#define S_FRAC         0x81
#define S_DBAMP        0x82
#define S_AMPDB        0x83
#define S_ABS          0x84
#define S_EXP          0x85
#define S_LOG          0x86
#define S_SQRT         0x87
#define S_SIN          0x88
#define S_COS          0x89
#define S_ATAN         0x8A
#define S_POW          0x8B
#define S_LOG10        0x8C
#define S_ASIN         0x8D
#define S_ACOS         0x8E
#define S_FLOOR        0x8F

#define S_CEIL            0x90
#define S_MIN             0x91
#define S_MAX             0x92
#define S_PCHOCT          0x93
#define S_OCTPCH          0x94
#define S_CPSPCH          0x95
#define S_PCHCPS          0x96
#define S_CPSOCT          0x97
#define S_OCTCPS          0x98
#define S_PCHMIDI         0x99
#define S_MIDIPCH         0x9A
#define S_OCTMIDI         0x9B
#define S_MIDIOCT         0x9C
#define S_CPSMIDI         0x9D
#define S_MIDICPS         0x9E
#define S_SGN             0x9F

#define S_FTLEN           0xA0
#define S_FTLOOP          0xA1
#define S_FTLOOPEND       0xA2
#define S_FTSETLOOP       0xA3
#define S_FTSETEND        0xA4
#define S_FTBASECPS       0xA5
#define S_FTSETBASE       0xA6
#define S_TABLEREAD       0xA7
#define S_TABLEWRITE      0xA8
#define S_OSCIL           0xA9
#define S_LOSCIL          0xAA
#define S_DOSCIL          0xAB
#define S_KOSCIL          0xAC
#define S_KLINE           0xAD
#define S_ALINE           0xAE
#define S_SBLOCK          0xAF

#define S_KEXPON          0xB0
#define S_AEXPON          0xB1
#define S_KPHASOR         0xB2
#define S_APHASOR         0xB3
#define S_PLUCK           0xB4
#define S_BUZZOPCODE      0xB5
#define S_GRAIN           0xB6
#define S_IRAND           0xB7
#define S_KRAND           0xB8
#define S_ARAND           0xB9
#define S_ILINRAND        0xBA
#define S_KLINRAND        0xBB
#define S_ALINRAND        0xBC
#define S_IEXPRAND        0xBD
#define S_KEXPRAND        0xBE
#define S_AEXPRAND        0xBF

#define S_KPOISSONRAND    0xC0
#define S_APOISSONRAND    0xC1
#define S_IGAUSSRAND      0xC2
#define S_KGAUSSRAND      0xC3
#define S_AGAUSSRAND      0xC4
#define S_PORT            0xC5
#define S_HIPASS          0xC6
#define S_LOPASS          0xC7
#define S_BANDPASS        0xC8
#define S_BANDSTOP        0xC9
#define S_FIR             0xCA
#define S_IIR             0xCB
#define S_FIRT            0xCC
#define S_IIRT            0xCD
#define S_BIQUAD          0xCE
#define S_FFT             0xCF

#define S_IFFT            0xD0
#define S_RMS             0xD1
#define S_GAIN            0xD2
#define S_BALANCE         0xD3
#define S_DECIMATE        0xD4
#define S_UPSAMP          0xD5
#define S_DOWNSAMP        0xD6
#define S_SAMPHOLD        0xD7
#define S_DELAY           0xD8
#define S_DELAY1          0xD9
#define S_FRACDELAY       0xDA
#define S_COMB            0xDB
#define S_ALLPASS         0xDC
#define S_CHORUS          0xDD
#define S_FLANGE          0xDE
#define S_REVERB          0xDF

#define S_COMPRESSOR      0xE0
#define S_GETTUNE         0xE1
#define S_SETTUNE         0xE2
#define S_FTSR            0xE3
#define S_FTSETSR         0xE4
#define S_GETTEMPO        0xE5
#define S_SETTEMPO        0xE6
#define S_FX_SPEEDC       0xE7
#define S_SPEEDT          0xE8
#define S_RESERVED26      0xE9
#define S_RESERVED27      0xEA
#define S_RESERVED28      0xEB
#define S_RESERVED29      0xEC
#define S_RESERVED30      0xED
#define S_RESERVED31      0xEE
#define S_RESERVED32      0xEF

#define S_IDENT        0xF0
#define S_NUMBER       0xF1
#define S_INTGR        0xF2
#define S_STRCONST     0xF3
#define S_BYTE         0xF4
#define S_FREE1        0xF5
#define S_FREE2        0xF6
#define S_FREE3        0xF7
#define S_FREE4        0xF8
#define S_FREE5        0xF9
#define S_FREE6        0xFA
#define S_FREE7        0xFB
#define S_FREE8        0xFC
#define S_FREE9        0xFD
#define S_FREE10       0xFE
#define S_EOO          0xFF

#define S_BADCHAR      0x0100    
#define S_BADNUMBER    0x0101
#define S_INSTRDECL    0x0102
#define S_OPCODEDECL   0x0103
#define S_GLOBALDECL   0x0104
#define S_TEMPLATEDECL 0x0105
#define S_IDENTLIST    0x0106
#define S_MIDITAG      0x0107
#define S_VARDECLS     0x0108
#define S_BLOCK        0x0109
#define S_NAME         0x010A
#define S_VARDECL      0x010B
#define S_NAMELIST     0x010C
#define S_TAGLIST      0x010D
#define S_LVALUE       0x010E
#define S_EXPR         0x010F
#define S_STATEMENT    0x0110    
#define S_FLOATCAST    0x0111    
#define S_GLOBALBLOCK  0x0112
#define S_ROUTEDEF     0x0113  
#define S_SENDDEF      0x0114  
#define S_SEQDEF       0x0115  
#define S_INSTANCE     0x0116
#define S_BUS          0x0117
#define S_EXPRLIST     0x0118
#define S_RTDEF        0x0119
#define S_UNION        0x011A
#define S_UNUSED2      0x011B
#define S_CONTROL      0x011C
#define S_LCONTROL     0x011D
#define S_LINSTR       0x011E
#define S_TEMPO        0x011F
#define S_END          0x0120
#define S_NEWLINE      0x0121
#define S_EOF          0x0122
#define S_PARAMDECL    0x0123
#define S_PARAMLIST    0x0124
#define S_OPVARDECL    0x0125
#define S_OPVARDECLS   0x0126
#define S_OPCALL       0x0127
#define S_OPARRAYCALL  0x0128
#define S_OPARRAYDECL  0x0129
#define S_EXPRSTRLIST  0x012A
#define S_INTLIST      0x012B
#define S_SOPCODE      0x012C
#define S_TMAPIDX      0x012D
#define S_PRINTF       0x012E
#define S_SASLFILE     0x012F
#define S_SAOLFILE     0x0130
#define S_OUTBUSNAME   0x0131
#define S_INSTRNAME    0x0132
#define S_FUTURE       0x0133


/* codes returned for adding to symbol table */

#define INSTALLED 0
#define DUPLICATE 1
#define DELETED 2
#define NOTPRESENT 3

#define SYSERROR1 -1

/* types of rates */

#define UNKNOWN   0
#define IRATETYPE 1
#define KRATETYPE 2
#define ARATETYPE 3
#define XRATETYPE 4

/* types of widths */

/* #define UNKNOWN   0 */
#define INCHANNELSWIDTH -1
#define OUTCHANNELSWIDTH -2
#define CHANNELSWIDTH -3
#define NOTDEFINED -4

/* types of volatility */
#define VARIABLE 0
#define CONSTANT 1

/* types of expressions */
#define ASFLOAT 0
#define ASINT   1

/* types of variables */
#define SCALARTYPE 1
#define VECTORTYPE 2
#define TABLETYPE  3
#define TMAPTYPE   4

/* kinds of symbols */
#define K_NORMAL 0
#define K_PFIELD 1
#define K_IMPORT 2
#define K_EXPORT 3
#define K_IMPORTEXPORT 4
#define K_INSTRNAME 5
#define K_BUSNAME 6
#define K_OPCODENAME 7
#define K_INTERNAL   8
#define K_PRESET 9

/* states for typechecking */

#define CHECKINPROGRESS 0
#define CHECKDONE 1

/* printing options */

#define PRINTIPASS 1
#define PRINTKPASS 2
#define PRINTAPASS 3
#define PRINTTOKENS 4

/* types of opcode call printouts */

#define SAMERATETYPE 0
#define SLOWRATETYPE 1

/* actions to take if wavegenerator SIZE = -1 */

#define GENILLEGAL 0
#define GENNUMDATA 1
#define GENLARGESTX 2
#define GENBUZZ 3
#define GENCONCAT 4
#define GENSTEP 5
#define GENPAIRS 6
#define GENCUBIC 7
#define GENSPLINE 8

/* refer-count table changing options */

#define TPARAMCHANGE 0
#define TVALCHANGE   1

/* types of IF-ELSE sections */

#define IRATESECTION 1
#define KRATESECTION 2
#define BOTHSECTIONS 3

/* spatialize */

#define HEADSIZE   0.2F /* size of head, in feet */
#define SPEEDSOUND 1087.49F
#define DZERO 1.0F
#define ROOMDELAY 0.020F
#define ROOMGAIN 0.177828F

/* fft and ifft */

#define FFTTABSIZE 14

/* sample-rate interpolation */

#define INTERP_LINEAR  0
#define INTERP_SINC    1

#define PI   (3.14159265358979323846)

/* print buffer size */

#define ZSIZE        8196

/* MIDI defines */

#define MCHAN 16
#define MNOTE 128

#define MIDIMASKCOM  0xF0
#define MIDIMASKCHAN 0x0F

#define MIDINOTEOFF  0x80
#define MIDINOTEON   0x90
#define MIDIKEYTOUCH 0xA0
#define MIDICONTROL  0xB0
#define MIDIPATCH    0xC0
#define MIDICHTOUCH  0xD0
#define MIDIWHEEL    0xE0
#define MIDISYSTEM   0xF0
#define MIDISYSEX0   0xF0
#define MIDISPP      0xF2
#define MIDISSP      0xF3
#define MIDITUNE     0xF6
#define MIDISYSEX7   0xF7
#define MIDITIME     0xF8
#define MIDISTART    0xFA
#define MIDICONT     0xFB
#define MIDISTOP     0xFC
#define MIDISENSE    0xFE
#define MIDIMETA     0xFF

#define METASEQNUM   0x00
#define METATEXT     0x01
#define METACOPYR    0x02
#define METASEQNAME  0x03
#define METAINSTR    0x04
#define METALYRIC    0x05
#define METAMARKER   0x06
#define METACUEPT    0x07
#define METACHANNEL  0x20
#define METAPORT     0x21
#define METATEMPO    0x51
#define METASMPTE    0x54
#define METATIMESIG  0x58
#define METAKEYSIG   0x59

/* for globals[] */

#define MIDICTRLPOS      0
#define MIDIVOLUMEPOS   7
#define MIDIPANPOS     10
#define MIDIEXPRPOS    11
#define MIDITOUCHPOS   128
#define MIDICHTOUCHPOS 256
#define MIDIBENDPOS    257
#define MIDIEXTPOS     258
#define MIDIFRAMELEN   259

/* for binary files */

/* 3-bit code for config datastructure */

#define BINORC   0
#define BINSCORE 1
#define BINMIDI  2
#define BINSAMP  3
#define BINSBF   4
#define BINSYM   5
#define BINRES1  6
#define BINRES2  7

/* and a few extra for coding purposes */

#define BINSSTR  8

/* 2-bit code for streaming events */

#define EVSCORE  0
#define EVMIDI   1
#define EVSAMPLE 2
#define EVRES    3

/* 3-bit code for score datastructure */

#define BININSTR     0
#define BINCONTROL   1
#define BINTABLE     2
#define BINEND       4
#define BINTEMPO     5

/* code for type of MIDI/SASL to read */

#define FCONFSCORE  0
#define FCONFMIDI   1
#define FSSTRSCORE  2
#define FSSTRMIDI   3
#define BCONFSCORE  4
#define BCONFMIDI   5
#define BSSTRSCORE  6
#define BSSTRMIDI   7

/* code for type of timestamps in a list */

#define RELTSTAMP 0
#define ABSTSTAMP 1

/* template cloning states */

#define NOSUB 0
#define DOSUB 1

/* time-management states */

#define RENDER 1
#define PLAYBACK 2
#define TIMESYNC 3

/* new latency limits, in seconds */

#define HIGH_LATENCY_MIN      0.0001F
#define HIGH_LATENCY_DEFAULT  0.300F
#define HIGH_LATENCY_MAX      1.0F

#define LOW_LATENCY_MIN      0.0001F
#define LOW_LATENCY_DEFAULT  0.002F
#define LOW_LATENCY_MAX      0.050F

/* types of latency, for control and ain drivers */

#define LOW_LATENCY_DRIVER   0
#define HIGH_LATENCY_DRIVER  1

/* types of networking available */

#define NO_NETWORKING  0
#define HAS_NETWORKING 1
#define NET_STATUS     NO_NETWORKING

/* default SIP location */

#define SIP_IP          "169.229.59.210"
#define SIP_RTP_PORT    5060

/* transposition for mirror session */

#define MSESSION_INTERVAL  5

/* maximum number of dynamic/controldevices notes, control lines, tables */

#define MAXDNOTES   256
#define MAXDCONTROL 64
#define MAXDTABLES  64

/* size of static strings -- eventually replace */

#define STRSIZE 2048

/* maximum depth of user-defined opcode calls (catches recursive calls) */

#define MAXOPCODEDEPTH 30

/* types of system compilers */

#define UNKNOWN_COMPILER 0
#define GCC_COMPILER     1

/* directory pre-prcoesss looks for SAOL library by default */

#define SAOLLIBDIR  "/usr/share/sfront"

/* types of input/output samples for drivers */

#define SAMPLE_SHORT 0
#define SAMPLE_FLOAT 1

/* active/passive samples for drivers */

#define PASSIVE_FLOW 0
#define  ACTIVE_FLOW 1

/* output file wordsize hint for drivers */

#define WORDSIZE_8BIT    0
#define WORDSIZE_16BIT   1
#define WORDSIZE_24BIT   2

/* bit positions for instr status word */

#define STATWORD_EFFECTS    1
#define STATWORD_SCORE      2
#define STATWORD_MIDI       4
#define STATWORD_DYNAMIC    8
#define STATWORD_STARTUP   16

/* types of signal variable mirroring */

#define REQUIRED      0
#define GLOBALMIRROR  1
#define OPCODEMIRROR  2

/* types of oparray code generation */

#define OPARRAY_GENERAL  0
#define OPARRAY_CONSTANT 1

/* maximum size for computing constant tables */

#define MAXTABLECONSTSIZE 8192

/* number of players in a session */

#define DEFAULTBANDSIZE 2
#define     MAXBANDSIZE 31 

/* levels of forward-error-correction */

#define FEC_NONE     0   /* must be zero */
#define FEC_NOGUARD  1  
#define FEC_MINIMAL  2  
#define FEC_STANDARD 3  
#define FEC_EXTRA    4  

/* default timing limit for late notes */

#define LATETIME_LIMIT   0.040F

/* minimum pass-phrase length */

#define MINIMUM_SESSIONKEY 20


/*******************************/
/* main symbol table structure */
/*******************************/

typedef struct sigsym {

/* these fields valid for all types of symbols */

char * val;             /* IDENT associated with symbol */ 
int kind;               /* kind of symbol*/
struct tnode * defnode; /* S_NAME tnode where IDENT is defined */
struct sigsym * next;   /* linked list ptr */
int width;              /* width (for instrs, output width) */
int effects;            /* instruments targeted by a send */
int score;              /* instrument used in score */
int ascore;             /* instrument used in score w/ absolute time*/
int dyn;                /* instrument used dynamically */
int midi;               /* instrument used in MIDI */
int amidi;              /* instrument used in MIDI w/ absolute time*/
int miditag;            /* instr definition has a preset tag */
int startup;            /* instrument used as startup() */
int outputbus;          /* instrument writing to output bus */

/* valid only for identifiers */

int rate;               /* RATETYPE,KRATETYPE,ARATETYPE,XRATETYPE */
int special;            /* 1 if a specialop                       */
int res;                /* ASFLOAT or ASINT */
int vartype;            /* SCALARTYPE,VECTORTYPE,TABLETYPE,TMAPTYPE */
int vol;                /* CONSTANT or VARIABLE */
int numinst;            /* number of instances created by send */
int calrate;            /* calculated rate for xsig pfields */

/* valid only for instr definitions */

struct tnode * obus;  /* output bus for this instrument */

/* valid for opcode and instr definitions */

int maxifstate;         /* maximum number of if-else state variables */

/* for optimization pass use */

char * consval;         /* holds calculated value */ 

/* reference count struct pointers */

struct trefer * tref;  /* reference counts for tokens        */
struct crefer * cref;  /* reference counts for instr/opcodes */

} sigsym;


/*****************************/
/* main parse tree structure */
/*****************************/

typedef struct tnode {

char * val;             /* string for terminals */

int ttype;              /* S_* number           */
int rate;               /* IRATETYPE,KRATETYPE,ARATETYPE,UNKNOWN */
int special;            /* 1 if a specialop                       */
int width;              /* width for arrays */
int res;                /* ASFLOAT or ASINT */
int vartype;            /* SCALARTYPE or VECTORTYPE */
int vol;                /* CONSTANT or VARIABLE */

struct sigsym * sptr;   /* symbol table entry for terminals */
struct tnode  * optr;   /* symbol table entry for opcode calls */
struct tnode  * dptr;   /* symbol table entry for dynamic instr calls */
int opwidth;            /* width for oparray calls */
int staterate;          /* rate of controlling statement, for opcode calls */
struct tnode * extra;   /* extra formal parameters for varargs opcode calls */
int extrarate;          /* maximum formal parameter rate for varargs */

struct tnode *ibus;     /* input bus list for this instance */
int arrayidx;           /* position in effects[] */
int usesinput;          /* needs a function for input[] */
int usesingroup;        /* needs a function for inGroup[] */
float time;             /* start time -- for sasl links */
int inwidth;            /* instance input width */

struct tnode * next;    /* tree pointers -- across */
struct tnode * down;    /* tree pointers -- down */
int linenum;            /* position of token for error reporting */
char * filename;        /* filename for error reporting */

} tnode;


/*******************************/
/*   reference count structs   */
/*******************************/


/*    reference counts for each token              */
/* assignments include call-by-reference uses      */
/* (u) implies refcount used for something useful  */
/* other counts were done for future optimizations */

typedef struct trefer {

  /* information about assignments */

  int assigntot;    /* number of assignments overall (u)  */
  int assignif;     /* number of assignments in if/else block/guard  (u) */
  int assignwhile;  /* number of assignments in while block/guard  (u) */
  int assignbyref;  /* number of assignments made in call-by-refer (u) */
  int assigntval;   /* number of assignments made to values of tables (u) */
                    /* contrast with assigntot, which also detects params */
  int assignrate;   /* rate of fastest assignment to the variable (u) */

  /* information about accesses */
  
  int accesstot;    /* total number of expression references  */
  int accessrate;   /* fastest rate variable is accessed at   */

  /* variable status */

  int varstate;    /* i/k/a variable potentially read before written */
  int dynaccess;   /* does a dyn instr statement import this ivar  */
  int totexport;   /* total number of times exported in an instr   */
  int totimport;   /* total number of times imported in an instr   */
  int mirror;      /* mirror status of imports/exports signal var  */
 
  /* in use for globals only */

  int finalinstr;   /* the last instr in execution seq that modifies var */

} trefer;



/* reference counts for each instr or opcode */
/* includes effects of all called opcodes    */
/* (u) implies refcount used for something useful  */
/* other counts were done for future optimizations */

typedef struct crefer {

  int MIDIctrl;  /* rate of fastest assignment to MIDIctrl (u)     */
  int params;    /* rate of fastest assignment to params (u)       */
  int settune;   /* rate of fastest execution of settune   (u)     */
  int kadur;     /* number of times dur accessed at k/a rates (u)  */
  int idur;      /* number of times dur accessed at i rate  (u)    */
  int itime;     /* number of times itime accessed     (u)         */
  int statevars; /* number of i/k/a sigs read before written       */
  int statewave; /* number of wavetables written                   */
  int callparam; /* number of parameter assigns (call-by-ref only) */
  int syslines;  /* number of instr/output/outbus/spat/ext/toff    */
  int conlines;  /* number of extend + turnoff lines               */
  
  /* for instrs and user-defined opcodes */
  
  int ilines;    /* number of i-rate statements at top level       */
  int klines;    /* number of k-rate statements at top level       */
  int alines;    /* number of a-rate statements at top level       */
  int ifstate;   /* number of if-else state variables at top level */
  int inmirror;  /* set if input[] buses need mirroring            */
 
  /* for opcodes only */
  
  int callif;     /* set if opcode call happens inside if block    */
  int callwhile;  /* set if opcode call happens inside while block */
  int callswitch; /* set if opcode call happens inside switch      */
  int callrate;   /* rate opcode is called at                      */

} crefer;


struct hasarray {

int o_abs;
int o_acos;
int o_aexpon;
int o_aexprand;
int o_agaussrand;
int o_aline;
int o_alinrand;
int o_allpass;
int o_ampdb;
int o_aphasor;
int o_apoissonrand;
int o_arand;
int o_asin;
int o_atan;
int o_balance;
int o_bandpass;
int o_bandstop;
int o_biquad;
int o_buzz;
int o_ceil;
int o_chorus;
int o_comb;
int o_compressor;
int o_cos;
int o_cpsmidi;
int o_cpsoct;
int o_cpspch;
int o_dbamp;
int o_decimate;
int o_delay;
int o_delay1;
int o_doscil;
int o_downsamp;
int o_exp;
int o_fft;
int o_fir;
int o_firt;
int o_flange;
int o_floor;
int o_frac;
int o_fracdelay;
int o_ftbasecps;
int o_ftlen;
int o_ftloop;
int o_ftloopend;
int o_ftsetbase;
int o_ftsetend;
int o_ftsetloop;
int o_ftsetsr;
int o_ftsr;
int o_gain;
int o_gettempo;
int o_gettune;
int o_grain;
int o_hipass;
int o_iexprand;
int o_ifft;
int o_igaussrand;
int o_iir;
int o_iirt;
int o_ilinrand;
int o_int;
int o_irand;
int o_kexpon;
int o_kexprand;
int o_kgaussrand;
int o_kline;
int o_klinrand;
int o_koscil;
int o_kphasor;
int o_kpoissonrand;
int o_krand;
int o_log;
int o_log10;
int o_lopass;
int o_loscil;
int o_max;
int o_midicps;
int o_midioct;
int o_midipch;
int o_min;
int o_octcps;
int o_octmidi;
int o_octpch;
int o_oscil;
int o_pchcps;
int o_pchmidi;
int o_pchoct;
int o_pluck;
int o_port;
int o_pow;
int o_reverb;
int o_rms;
int o_samphold;
int o_sblock;
int o_settempo;
int o_settune;
int o_sgn;
int o_sin;
int o_speedt;
int o_sqrt;
int o_tableread;
int o_tablewrite;
int o_upsamp;
int w_buzz;
int w_concat;
int w_cubicseg;
int w_data;
int w_empty;
int w_expseg;
int w_harm;
int w_harm_phase;
int w_lineseg;
int w_periodic;
int w_polynomial;
int w_random;
int w_sample;
int w_spline;
int w_step;
int w_window;
int spatialize;
int s_k_rate;
int s_s_rate;
int s_inchan;
int s_outchan;
int s_time;
int s_dur;
int s_itime;
int s_preset;
int s_channel;
int s_MIDIctrl;
int s_MIDItouch;
int s_MIDIbend;
int s_input;
int s_inGroup;
int s_released;
int s_cpuload;
int s_position;
int s_direction;
int s_listenerPosition;
int s_listenerDirection;
int s_minFront;
int s_maxFront;
int s_minBack;
int s_maxBack;
int s_params;
};

/* markerlist for mp4write */

typedef struct aiffmark {

short id;
unsigned int position;
struct aiffmark * next;

} aiffmark;


/* compiler flags */

extern int midiverbose;
extern int isocheck;
extern int rateoptimize;
extern int constoptimize;

extern FILE * saolfile;
extern FILE * saslfile;
extern FILE * sstrfile;
extern FILE * midifile;
extern FILE * mstrfile;
extern FILE * outfile;
extern FILE * boutfile;
extern FILE * orcoutfile;
extern FILE * scooutfile;
extern FILE * midoutfile;
extern FILE * bitfile;
extern FILE * soundfile;

extern int sfront_argc;
extern char ** sfront_argv;

extern int aout;
extern int aoutflow;
extern char * aoutname;

extern int ain;
extern int ainlatency;
extern int ainflow;
extern char * ainname;

extern int outfile_wordsize;

extern int cin;
extern int cmidi;
extern int csasl;
extern int cmaxchan;
extern int cinmaxchan;
extern int clatency;
extern char * cinname;

extern int reentrant;
extern int creentrant;
extern int nomain;
extern int adebug;

extern int timeoptions;
extern float latency;
extern int catchsignals;
extern int fixedseed;
extern int isocompliant;
extern int compilertype;
extern int cppsaol;
extern char * cppincludes;
extern int systemshell;
extern int hexstrings;
extern char * session;
extern char * sessionkey;
extern int feclevel;
extern int lateplay;
extern float latetime;
extern int null_program;
extern int netstart;
extern int netmsets;
extern unsigned short sip_port;
extern char sip_ip[];
extern int msession_interval;
extern char * au_component_type;
extern char * au_component_manu;
extern char * au_component_subtype;
extern char * au_filesystem_name;
extern char * au_ui_name;
extern char * au_ui_manu;
extern char * au_manu_url;
extern char * au_view_bundlename;
extern char * au_view_baseclass;

extern tnode * saolfilelist;
extern tnode * saslfilelist;
extern tnode * sstrfilelist;
extern tnode * currsaolfile;
extern int ascsaolptree;

extern char * z[]; 

/* for free'ing dynamically-created lines */

typedef struct znode {

char * zchar;
struct znode * next;

} znode;

extern znode * zlist;

extern struct hasarray has;

extern int srate;
extern int krate;
extern int saol_krate;
extern int twocycle;
extern int inchannels;
extern int outchannels;
extern int interp;
extern float globaltune;

extern int interp_cmdline;
extern unsigned int sinc_pilen;
extern unsigned int sinc_zcross;
extern float sinc_upmax;

extern tnode * troot;
extern tnode * groot;
extern tnode * instances;
extern tnode * outputbusinstance;
extern tnode * outbustable;
extern tnode * printfunctions;
extern tnode * locopcodecalls;
extern tnode * tlocopcodecalls;
extern tnode * locdyncalls;
extern tnode * tlocdyncalls;
extern tnode * globalopcodecalls;

extern tnode maplistopcall;
extern tnode maplistoparraycall;

extern sigsym * bitsamplein; 
extern sigsym * bitsampleout; 
extern sigsym * bitsymtable;
extern sigsym * bitsymin;
extern sigsym * locsymtable;
extern sigsym * tlocsymtable;
extern sigsym * globalsymtable;
extern sigsym * instrnametable;
extern sigsym * unusedinstrtable;
extern sigsym * opcodenametable;
extern sigsym * dinstrnametable;
extern sigsym * busnametable;
extern sigsym * outputbus;
extern sigsym * startupinstr;
extern sigsym * instrpresets;
extern sigsym * targetsymtable;
extern sigsym * mpegtokens;

extern char   * currinstancename;
extern char   * curropcodeprefix;
extern tnode  * currinstance;
extern tnode  * curropcodeinstance;
extern sigsym * curropcodestack;
extern sigsym * currinstrument;
extern sigsym * currconstoptlevel;

extern int currinstrwidth;
extern int currinputwidth;
extern int curropcoderate;
extern int currspecialrate;
extern int currtreerate;
extern int currblockrate;
extern int currintprint;
extern int currconstoptif;
extern int currconstoptwhile;
extern int currconstwhilerate;
extern int currrateunguarded;

extern int globalblockcount;
extern int suspendvarchecks;
extern int conditionalblocks;
extern int currarrayindex;
extern int currscalarflag;
extern int outstrict;
extern int setbusnum;
extern int isaninstr;
extern int nonpolyparams;
extern int numinstrnames;
extern int curropcalldepth;
extern int maxoparraydepth;
extern int curroparraydepth;
extern int useshadowbus;
extern int mpegtokencount;

extern int ifrefdepth;
extern int whilerefdepth;
extern int ifrefglobaldepth;
extern int whilerefglobaldepth;

/* sasl reading stuff */

extern int numpfields;
extern int maxmidipreset;

typedef struct sasdata {

  tnode * temporoot;
  tnode * tempotail;
  int numtempo;
  
  tnode * tableroot;
  tnode * tabletail;
  int numtable;

  tnode * controlroot;
  tnode * controltail;
  int numcontrol;

  tnode * instrroot;
  tnode * instrtail;
  
  sigsym * labeltable;
  int numlabels;

  unsigned int scorefsize;
  
  char * endtimeval;

  float compendtime;

} sasdata;


extern sasdata * confsasl;
extern sasdata * sstrsasl;
extern sasdata * allsasl;
extern sasdata * abssasl;

extern tnode * tempomap;

/* midi reading stuff */

/* state for reading a midi file */

typedef struct midata {

  tnode * imidiroot;         /* list of midi events */
  tnode * imiditail;

  unsigned int midifsize;   /* midi file size, for mp4write */

  unsigned int miditracks;  /* track counter for extchan */
  unsigned int miditicks;   /* ticks per quarter note */
  unsigned int midimaxtime; /* maximum timestamp in file */
  int midinumchan;          /* extended channels in use */

} midata;

extern midata * confmidi;
extern midata * sstrmidi;
extern int totmidichan;
extern int midiallsoundsoff;

/* temporary midi variables */

extern unsigned char midirunstat;
extern unsigned int midictime;
extern unsigned int midibank[];
extern int midiext[];
extern int midifirst[];
extern tnode * midicurrinstr[];
extern tnode * midicurrnote[];
extern tnode * midilastnote[][MNOTE];

extern int midihasctrlflag[][MNOTE];
extern int midihastouchc[];
extern int midihastouchk[];
extern int midihaswheel[];


/* mp4 bit writing stuff */

extern unsigned char bitstowrite; /* buffers for mp4 binary write */
extern int bitwritepos;           /* keeps track of bit position */

extern unsigned char bitstoread;  /* buffers for mp4 binary read */
extern int bitreadpos;            /* keeps track of bit position */
                                
                                  /* for mp4 binary intrachunk read */
extern unsigned int bitreadlen;  /* number of bytes left in chunk */
extern int bitscoretype;          /* type of line read in progress */
extern int bitlinecount;          /* score line finished */
extern float bitscotime;          /* score time */
extern int bitscohastime;         /* score line hastime bit */
extern int bitscolabel;           /* score label (-1 if no label) */
extern int bitscopfields;         /* score number of pfields */
extern int bitscopfieldsmax;      /* score number of pfields max */
extern int bittabletoken;         /* tabletype token for sample table gen */
extern int bitsampletoken;        /* score token for sample table gen */
extern int bitsamplefirst;        /* counter for sample table gen */
extern float bitaccesstime;       /* time of current access unit */

extern unsigned char bitstowrite;
extern int bitwritepos;

extern unsigned char bitstoread;
extern int bitreadpos;

extern int bitwritenosymbols;

extern int bitreadaccessunits;

extern int ffttables[];          /* checklist for including fft tables */ 

/*****************************/
/* lexical/parsing variables */
/*****************************/

extern int saollinenumber;
extern char * saolsourcefile;

#define YYSTYPE tnode *

/* maximum size of context-dependent lexical look-ahead */

#define LEXSTACKSIZE 256
#define LEXBUFSIZE   256

extern int lexstackret[];
extern tnode * lexstacktnode[];
extern int lexstackptr;

extern int lexholdret[];
extern tnode * lexholdtnode[];
extern int lexholdptr;

/* possible values for look-ahead state machine */

#define TEMPLATE_REST 0    
#define TEMPLATE_ACTIVE 1  
#define TEMPLATE_PRESET 2
#define TEMPLATE_PMAPLIST 3
#define TEMPLATE_WITH 4
#define TEMPLATE_WMAPLIST 5
#define TEMPLATE_LOOKAHEAD 6

extern int lexstatemachine;

/* possible values for mp4write global state machine */

#define GLOBAL_DORMANT  0
#define GLOBAL_FIRST    1
#define GLOBAL_REST     2
#define GLOBAL_ACTIVE   3

extern int lexttl;
extern int sendsemicoloncount;


/* for describing a wav or aiff sample */

typedef struct sampleinfo {

  char wav;             /* 1 if from wav file, 0 if from aif file */
  char numbytes;        /* number of bytes in a data word         */
  char framebytes;      /* number of bytes in one N-channel frame */
  char chanpoint;       /* 1st byte of channel in frame; -1 sum to mono */
  unsigned int len;    /* number of samples                      */
  unsigned int point;  /* number of bytes before first data byte */
  unsigned int srate;  /* sampling rate */
  char hasbase;         /* has a base frequency */
  char hasloop;         /* has loop points */
  float base;           /* base frequency */
  unsigned int start;  /* start of loop */
  unsigned int end;    /* end of loop */

} sampleinfo;


/**********************/
/* external functions */
/**********************/

/* from ascwrite.c */

extern void ascsaolwrite(void);
extern void ascsaslwrite(sasdata *);

/* from audio.c */

extern void printaudiohelp(void);
extern int ainfilecheck(char *);
extern int aoutfilecheck(char *);
extern int makeaudiotypeout(int);
extern int stdoutused(int);
extern int stdinused(int);
extern int wiretap_logging(int);
extern int adriver_reflection(int);
extern int makeaudiotypein(int);
extern void audiodefaults(void);
extern void makeainparams(int, int *, int *);
extern void makeaudiodriver(int);
extern int makeaoutsync(int);
extern int makeainsync(int);
extern int makeaouttimedefault(int);
extern int makeaintimedefault(int);
extern int makeaoutrobust(int);
extern int makeainrobust(int);

/* from blocktree.c */

extern void nextstateupdate(int *);
extern void blocktree(tnode *, int);

/* from csrclib.c  */

extern void makepreamble(void);

/* from cmainpass.c */

extern void makecontrolsys(void);
extern void makesaslcontrolsys(void);
extern void makesasltablesys(void);
extern void makesaslcrosscontrol(char *);

/* from collapse.c */

extern void co_constcollapse(tnode *);

/* from control.c */

extern void printcontrolhelp(void);
extern int cinfilecheck(char *);
extern void makecontroldriver(int);
extern void makenetworkdriver(void);

/* from corecode.c */

extern tnode * firstopcodearg(tnode *);
extern void printblock(int);
extern void printraw(int);
extern void printlib(int);
extern void printwaveblock(int, sigsym *, char *);
extern void printwaveblock2(int, tnode *);
extern void printwavesymblock(int, sigsym *, char *);
extern void printwavesymblock2(int, tnode *);
extern void printcontrolblock(int, char *);
extern void mz(int);
extern void vmcheck(void *);
extern void coreopcodebody(tnode *);
extern void genex(int*, tnode*, char *);
extern void gened(tnode*, char *);

/* from coreinline.c */
 
extern int  coreopcodecaninline(tnode *);
extern void coreopcodedoinline(tnode *);

/* from corevars.c */

extern void coreopcodevars(tnode *);
extern int coreopcodename(tnode *);
extern int coreopcodehasextras(tnode *);
extern int coreopcodespeedtrap(tnode *);
extern int delicatepolyops(tnode * tptr);
extern int polyopcallexcept(tnode * tptr);
extern int coreopcodeprecompute(tnode *);
extern void coreopcodecollapse(tnode *, tnode *);
extern void hascoreopcode(tnode *, int);
extern void corerefer(sigsym *, tnode *, tnode *, int);
extern int coreopcodespecial(tnode *);
extern int coreopcodeasint(tnode *);
extern sigsym * coreopcodeadd(tnode *, sigsym **);
extern int coreopcodeargs(tnode *,tnode *);
extern void coreopcodevarargs(tnode * tcall);

/* from lex.c */

extern void yyerror(char *);
extern int yylex(void);
extern void showerrorplace(int, char *);
extern void internalerror(char *, char *);
extern void noerrorplace(void);
extern void warningmessage(tnode *, char *);
extern int lexstate_pmap(void);
extern int lexstate_wmap(void);


/* from mp4read.c */

extern unsigned int readbit(unsigned int);
extern void readflush(unsigned int);
extern int readprepare(int);
extern int orclex(void);
extern tnode * binconflex(void);
extern tnode * binsstrlex(void);
extern void readsampleset(void);
extern void readsymboltable(void);

/* from mp4write.c */

extern void mp4write(void);

/* from oclone.c */

extern void installopnames(tnode *);
extern void installdyninstr(tnode *);
extern tnode * eclone(tnode *);
extern tnode * tclone(tnode *);
extern sigsym * sclone (sigsym *);
extern tnode *  treeclone(tnode *, sigsym **, int);

/* from optconst.c */

extern void stateoptconst(int, tnode **);
extern void exprcollapse(int, tnode *);

/* from optmain.c */

extern void optmain(void);

/* from optrate.c */

extern int looseopcoderules(tnode *);
extern void stateoptrate(int, tnode *, tnode **, int *);

/* from optrefer.c */

extern void exprrefer(sigsym *, tnode *, int);
extern void staterefer(sigsym *, tnode *, int);
extern void tmaprefer(sigsym *, int, int);
extern void refermirror(sigsym *);

/* from parsehelp.c */

extern void tablecheck(tnode *);
extern void tablelistcheck(tnode *);
extern tnode * leftrecurse(tnode *, tnode *);
extern tnode * leftsrecurse(tnode *, tnode *, tnode *);
extern void make_instrpfields(tnode *);
extern tnode * make_instrdecl(tnode *, tnode *, tnode *, tnode *, tnode *,
                       tnode *, tnode *, tnode *, tnode *, tnode *);
extern tnode * make_miditag(tnode *, tnode *);
extern void make_opcodetype(tnode *, tnode *);
extern tnode * make_opcodedecl(tnode *, tnode *, tnode *, tnode *, tnode *,
			       tnode *, tnode *, tnode *, tnode *);
extern tnode * make_globaldecl(tnode *, tnode *, tnode *, tnode *);
extern void make_templatepfields(tnode *, tnode *);
extern tnode * make_templatedecl(tnode *, tnode *, tnode *, tnode *,
				 tnode *, tnode *, tnode *);
extern void templateopcodepatch(void);
extern tnode * make_mapblock(tnode *, tnode *);
extern tnode * make_rtparam(tnode *, tnode *, tnode *);
extern tnode * make_routedef(tnode *, tnode *, tnode *, tnode *, tnode *,
			     tnode *, tnode *);
extern tnode * make_senddef(tnode *, tnode *, tnode *, tnode *, tnode *,
			    tnode *, tnode *, tnode *, tnode *);
extern tnode * make_seqdef(tnode *, tnode *, tnode *, tnode *, tnode *);
extern tnode * make_statement(tnode *, tnode *, tnode *, tnode *, tnode *,
			      tnode *, tnode *, tnode *, tnode *,
			      tnode *, tnode *);
extern tnode * make_lval(tnode *, tnode *, tnode *, tnode *);
extern tnode * make_simplevar(tnode *, tnode *, tnode *, tnode *, 
			      char *, int);
extern tnode * make_tabledecl(tnode *, tnode *, tnode *, tnode *, tnode *, 
			      tnode *, tnode *);
extern tnode * make_tablemap(tnode *, tnode *, tnode *, tnode *,
			     tnode *, tnode *);
extern tnode * make_paramdecl(tnode *, tnode *);
extern tnode * make_name(tnode *, tnode *, tnode *, tnode *);
extern tnode * make_stree(tnode *, tnode *, tnode *, tnode *, char *, int);
extern tnode * make_expr(tnode *, tnode *, tnode *, tnode *, tnode *, tnode *,
			 tnode *);


/* from parser.tab.c */

extern int yyparse(void);

/* from postparse.c */

extern void checkopcodeargswidth(tnode *);
extern void saolparse(void);
extern void varupdate(tnode *, sigsym**);

/* from readmidi.c */

extern void readmidi(midata *, sasdata *, int type);
extern void initmidiinstr(int, sigsym *, int *);
extern void initmidiinstrassign(int, sigsym *);
extern void initmidiinstrconstant(int, sigsym *);
extern void binmidiwrite(int);
extern void midieventread(void);

/* from readscore.c */

extern void readscore(int);
extern void printtablefunctions(void);
extern void initendtime(void);
extern void initendtimeassign(void);
extern void initscoreinstr(int, sigsym * sptr);
extern void initscoreinstrassign(int, sigsym * sptr);
extern void initscoreinstrconstant(int, sigsym * sptr);
extern void initscorecontrol(int);
extern void initscorecontrolassign(int);
extern void initscorecontrolconstant(int);
extern void initscoretempo(int);
extern void initscoretempoassign(int);
extern void initscoretempoconstant(int);
extern void initscoretableexterns(int);
extern void initscoretablevars(int);
extern void initscoretableassign(int);
extern void initscoretableconstant(int);
extern int parsetempo(sasdata *, tnode *, int);
extern int parsecontrol(sasdata *, tnode *, int);
extern void mergescores(void);
extern void renumberabs(void);
extern void showbadline(tnode *);
extern void badline(tnode *);

/* from sfmain.c */

extern void deletecppfiles(void);

/* from special.c */

extern int specialupdate(tnode *);

/* from stparse.c */

extern int constdur(void);
extern void hasstandardname(tnode *);
extern void printdurassign(void);
extern void printstandardname(tnode **);
extern int standardcollapse(tnode *);
extern int standardname(tnode *);
extern int standardrate(tnode *);
extern int standardres(tnode *);
extern int standardvartype(tnode *);
extern int standardwidth(tnode *);

/* from symbols.c */

extern int      addsym(sigsym **, tnode*);
extern int      addvsym(sigsym **, char *, int);
extern sigsym * addvsymend(sigsym **, char *, int);
extern int      addvsymsort(sigsym **, char *, int);
extern sigsym * getsym(sigsym **, tnode*);
extern sigsym * getvsym(sigsym **, char*);
extern void     symcheck(int, tnode *);
extern int      deletesym(sigsym **, sigsym*);
extern sigsym * reversetable (sigsym *);
extern sigsym * findlast(sigsym **,tnode *);
extern void     moveafter(sigsym **,sigsym *,sigsym *);
extern int      movebefore(sigsym **,sigsym *,sigsym *);
extern char *   dupval(char * val);
extern char *   dupunderscore(char * val);
extern tnode *  make_tnode(char *, int);
extern int      make_int(tnode * tptr);
extern int      largeinteger(char * s);
extern int      reachableinstrexeff(sigsym *);
extern int      reachableinstrexstart(sigsym *);
extern int      reachableinstrexeffexstart(sigsym *);
extern int      reachableinstr(sigsym *);

/* from tokens.c */

extern int identtoken (tnode * );
extern char * strfortoken(char *, int);
extern int tokenmap(int);
extern int parsetokenmap(int);

/* from treeupdate.c */

extern int  truewidth(int);
extern int  widthupdate(tnode *);
extern void opraterecurse(tnode *);
extern void inrateupdate(sigsym *);
extern void rateupdate(tnode *);

/* from writemain.c */

extern void printmainloops(void);

/* from writeop.c */

extern void redefnormal(void);
extern void redefglobal(void);
extern void redefstatic(int);
extern void opcodefunctions(void);
extern void printopcodes(tnode *);
extern void printtmapcase(tnode *, tnode *, char *);
extern void printtabledefine(int, char *, int, int);
extern void printinlinetable(tnode * tptr);
extern  int maketableindex(tnode *, sigsym *, char **, int *);
extern char * namingprefix(sigsym *, char *);
extern char * stackstring(int, int, char * idxstr);
extern  int indexed_cbr(tnode * aptr);
extern  int stname_cbr(tnode * aptr);

/* from writeorc.c */

extern void toptree(int);
extern void postscript(void);
extern int shadowcheck(void);
extern char * inputbusmacro(void);

/* from writepre.c */

extern void preamble(void);
extern void postcorefunctions(void);
extern void printsaoltables(int);
extern void printtablecatalog(void);

/* from wtparse.c */

extern int wavegeneratorname(tnode *);
extern void haswavegenerator(tnode *);
extern float * wavereduceconstants(tnode *, tnode *);
extern void wavegeneratorvar(sigsym *);
extern void createtable(sigsym *, char *, int);
extern void tablestartupcheck(sigsym * ident, int mode, int kind);
/* from wtconst.c */

extern float * wavereduceconstants(tnode *, tnode *);
extern void createconstanttable(sigsym *, char *, int);
extern void printtablestring(sigsym *, char *);
extern char * compactfloat(char *, float);

#endif /* _SFRONT_TREE_H */













