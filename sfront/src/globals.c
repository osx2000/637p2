
/*
#    Sfront, a SAOL to C translator    
#    This file: Global variable initialization
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

FILE * saolfile = NULL;
FILE * saslfile = NULL;
FILE * sstrfile = NULL;
FILE * midifile = NULL;
FILE * mstrfile = NULL;
FILE * outfile  = NULL;
FILE * boutfile  = NULL;
FILE * orcoutfile = NULL;
FILE * scooutfile = NULL;
FILE * midoutfile = NULL;
FILE * bitfile = NULL;
FILE * soundfile = NULL;

int sfront_argc = 0;
char ** sfront_argv = NULL;

int aout = 0;
char * aoutname = NULL;
int aoutflow = PASSIVE_FLOW;

int ain = 0;
int ainlatency = HIGH_LATENCY_DRIVER;
int ainflow = PASSIVE_FLOW;
char * ainname = NULL;

int outfile_wordsize = WORDSIZE_16BIT;

int cin = 0;
int cmidi = 0;
int csasl = 0;
int cmaxchan = 0;
int cinmaxchan = 0;
int clatency = HIGH_LATENCY_DRIVER;
char * cinname = NULL;

int reentrant = 0;
int creentrant = 0;
int nomain = 0;
int adebug = 0;

char * session = NULL;
char * sessionkey = NULL;
int feclevel = FEC_STANDARD;
int lateplay = 0;
float latetime = LATETIME_LIMIT;
int null_program = 0;
int netstart;
int netmsets = DEFAULTBANDSIZE;
unsigned short sip_port = SIP_RTP_PORT;
char sip_ip[16] = SIP_IP;
int msession_interval = MSESSION_INTERVAL;

char * au_component_type = NULL;
char * au_component_manu = NULL;
char * au_component_subtype = NULL;
char * au_filesystem_name = NULL;
char * au_ui_name = NULL;
char * au_ui_manu = NULL;
char * au_manu_url = NULL;
char * au_view_bundlename = NULL;
char * au_view_baseclass = NULL;

int timeoptions = UNKNOWN;
float latency = -1.0F;
int catchsignals = 0;
int fixedseed = 0;
int isocompliant = 0;
int compilertype = UNKNOWN_COMPILER;
int cppsaol = 0;
char * cppincludes = NULL;
int systemshell;
int hexstrings = 0;

tnode * saolfilelist = NULL;
tnode * saslfilelist = NULL;
tnode * sstrfilelist = NULL;
tnode * currsaolfile = NULL;

int ascsaolptree = 0;

char * z[ZSIZE]; 
znode * zlist = NULL;

struct hasarray has;

/* compiler flags */

int midiverbose = 0;
int isocheck = 0;
int rateoptimize = 1;
int constoptimize = 1;

int srate = -1;
int krate = -1;
int saol_krate = -1;
int twocycle = -1;
int inchannels = -1;
int outchannels = -1;
int interp = -1;
float globaltune = 440.0;

int interp_cmdline = -1;
unsigned int sinc_pilen = 128;
unsigned int sinc_zcross = 3;
float sinc_upmax = 4.0F;

tnode  * troot = NULL;
tnode  * groot = NULL;
tnode  * instances = NULL;
tnode  * outputbusinstance = NULL;
tnode  * outbustable = NULL;
tnode  * printfunctions = NULL;
tnode  * locopcodecalls = NULL;
tnode  * tlocopcodecalls = NULL;
tnode  * locdyncalls = NULL;
tnode  * tlocdyncalls = NULL;
tnode  * globalopcodecalls = NULL;

tnode maplistopcall;
tnode maplistoparraycall;

sigsym * bitsampleout = NULL; 
sigsym * bitsamplein = NULL;
sigsym * bitsymtable = NULL;
sigsym * bitsymin = NULL;
sigsym * locsymtable = NULL;
sigsym * tlocsymtable = NULL;
sigsym * globalsymtable = NULL;
sigsym * instrnametable = NULL;
sigsym * unusedinstrtable = NULL;
sigsym * opcodenametable = NULL; 
sigsym * busnametable = NULL;
sigsym * outputbus = NULL;
sigsym * startupinstr = NULL;
sigsym * instrpresets = NULL;
sigsym * targetsymtable = NULL;
sigsym * mpegtokens = NULL;

char   * currinstancename = NULL;
char   * curropcodeprefix = NULL;
tnode  * currinstance = NULL;
tnode  * curropcodeinstance = NULL;
sigsym * curropcodestack = NULL;
sigsym * currinstrument = NULL;
sigsym * currconstoptlevel = NULL;

int currinstrwidth = 1;
int currinputwidth = 0;
int curropcoderate = IRATETYPE;
int currspecialrate = UNKNOWN;
int currtreerate = UNKNOWN;
int currblockrate = IRATETYPE;
int currintprint = ASFLOAT;
int currconstoptif = 0;
int currconstoptwhile = 0;
int currconstwhilerate = UNKNOWN;
int currrateunguarded = 0;

int globalblockcount = 0;
int suspendvarchecks = 0;
int conditionalblocks = 0;
int currarrayindex = 0;
int currscalarflag = 1;
int outstrict = 0;
int setbusnum = 1;   /* always know output bus */
int isaninstr = 0;
int nonpolyparams = 0;
int numinstrnames = 0;
int maxoparraydepth = 0;
int curroparraydepth = 0;
int curropcalldepth = 0;
int useshadowbus = 0;
int mpegtokencount;

int ifrefdepth = 0;
int whilerefdepth = 0;
int ifrefglobaldepth = 0;
int whilerefglobaldepth = 0;

int saollinenumber = 1;
char * saolsourcefile = NULL;

int lexstackret[LEXSTACKSIZE];
tnode * lexstacktnode[LEXSTACKSIZE];
int lexstackptr = -1;

int lexholdret[LEXSTACKSIZE];
tnode * lexholdtnode[LEXSTACKSIZE];
int lexholdptr = -1;

int lexstatemachine = TEMPLATE_REST;
int lexttl = S_SEM;   
int sendsemicoloncount = 0;

int numpfields = 1;
int maxmidipreset = 0;

/* sasl reading stuff */

sasdata * confsasl = NULL;
sasdata * sstrsasl = NULL;
sasdata * allsasl = NULL;
sasdata * abssasl = NULL;
   
tnode * tempomap = NULL;

/* midi reading stuff */

midata * confmidi = NULL;       /* state for conf midi file      */
midata * sstrmidi = NULL;       /* state for streaming midi file */
int totmidichan = -1;           /* cmidi+confmidi+sstrmidi chans */ 
int midiallsoundsoff = 0;       /* uses the MIDI all sounds off command */

                                /* temporary midi variables */

unsigned int midictime = 0;    /* current time (during parse) */
unsigned char midirunstat = 0;  /* running status for streaming MIDI */

int midiext[MCHAN];
unsigned int midibank[MCHAN];   /* note state for each note/chan */
int midifirst[MCHAN];           /* flags first event in new channel */
tnode * midicurrinstr[MCHAN];   /* current instr for each channel */
tnode * midicurrnote[MCHAN];    /* last note for each channel */
tnode * midilastnote[MCHAN][MNOTE]; /* note state for each note/chan */

int midihasctrlflag[MCHAN][MNOTE]; /* flags controller use for verbose mode */
int midihastouchc[MCHAN];
int midihastouchk[MCHAN];
int midihaswheel[MCHAN];


unsigned char bitstowrite = 0;  /* buffers for mp4 binary write */
int bitwritepos = 7;            /* keeps track of bit position */

unsigned char bitstoread = 0;   /* buffers for mp4 binary read */
int bitreadpos = -1;            /* keeps track of bit position */
                                
                                /* for mp4 binary intrachunk read */
unsigned int bitreadlen = 0;   /* number of bytes left in chunk */
int bitscoretype = 0;           /* type of line read in progress */
int bitlinecount = 0;           /* score line finished */
float bitscotime = 0;           /* score time */
int bitscohastime = 0;          /* score line hastime bit */
int bitscolabel = -1;           /* score label (-1 if no label) */
int bitscopfields = -1;         /* score number of pfields */
int bitscopfieldsmax = -1;      /* score number of pfields */
int bitsampletoken = -1;        /* score token for sample table gen */
int bittabletoken = -1;         /* score token for sample table gen */
int bitsamplefirst = 0;         /* counter for sample table gen */
float bitaccesstime = 0.0F;     /* time of current access unit */
int bitwritenosymbols = 1;
int bitreadaccessunits = 0;

int ffttables[FFTTABSIZE];      /* checklist for including fft tables */

