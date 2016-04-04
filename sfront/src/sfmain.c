
/*
#    Sfront, a SAOL to C translator    
#    This file: Contains main()
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

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*    welcome to the sfront source tree. we begin with externs  */
/*    for all of the functions called by main(), and then main  */
/*    itself. the remainder of this file is the command line    */
/*    parsing part of sfront. the guidance comments, like this  */
/*    one will be surronded with ~~~ and ___ instead of ***.    */
/*______________________________________________________________*/


extern void sfrontinitialize(int, char **);
extern void commandlineargs(int, char **);
extern void sfrontshutdown(void);

/****************************************************************/
/*                     main() for sfront                        */
/****************************************************************/

int main(int argc, char ** argv)

{

  /*~~~~~~~~~~~~~~~*/
  /* part 1: setup */
  /*_______________*/

  /* initialize sfront */

  sfrontinitialize(argc, argv);               /* sfmain.c    */
  commandlineargs(argc, argv);                /* sfmain.c    */

  /* read parts of mp4 file */

  if (saolfilelist == NULL)                 
    {
      readsampleset();                        /* mp4read.c   */
      if (orcoutfile || scooutfile || csasl)
	readsymboltable();                    /* mp4read.c   */
    }

  /* write diagnostic SAOL file */

  if (orcoutfile)
    ascsaolwrite();                           /* ascwrite.c  */           


  /*~~~~~~~~~~~~~~~~~~~*/
  /* part 2: front end */
  /*___________________*/

  /* parse SAOL/SASL/MIDI */

  saolparse();                                /* postparse.c */

  /* SAOL optimization */

  optmain();                                  /* optmain.c   */

  /* combine scores, write mp4 file */

  renumberabs();                              /* readscore.c */
  if (boutfile)
    mp4write();                               /* mp4write.c  */
  mergescores();                              /* readscore.c */


  /*~~~~~~~~~~~~~~~~~~~~~~~~~*/
  /* part 3: code generation */
  /*_________________________*/

  preamble();                                 /* writepre.c  */
  opcodefunctions();                          /* writeop.c   */
  toptree(PRINTIPASS);                        /* writeorc.c */
  toptree(PRINTKPASS);
  toptree(PRINTAPASS);
  printtablefunctions();                      /* readscore.c */
  postscript();                               /* writeorc.c */


  /*~~~~~~~~~~~~~~~~~~*/
  /* part 4: shutdown */
  /*__________________*/

  sfrontshutdown();                           /* sfmain.c    */
  return 0;

}


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/* below are the functions called above that are in the file:   */
/*    sfrontinitialize   commandlineargs   sfrontshutdown       */ 
/*______________________________________________________________*/

extern void printhelpshort(void);

/****************************************************************/
/*            sfront initialization before cmdline parsing      */
/****************************************************************/

void sfrontinitialize(int argc, char ** argv)

{

  sfront_argc = argc;
  sfront_argv = argv;
  systemshell = system(NULL);

  vmcheck(confsasl = calloc(1, sizeof(sasdata)));
  vmcheck(sstrsasl = calloc(1, sizeof(sasdata)));
  vmcheck(abssasl  = calloc(1, sizeof(sasdata)));
  vmcheck(confmidi = calloc(1, sizeof(midata)));
  vmcheck(sstrmidi = calloc(1, sizeof(midata)));
  cppincludes = dupval(" ");

  printf("sfront, a SAOL to C translator. Version %s.\n",IDSTRING);
  printf("Run sfront with -license option for Copyright/License info.\n\n");

  if (argc == 1)
    printhelpshort();

}


extern int singleflags(char *);
extern int oneparamflags(char *, char *);
extern void cmdlineorchestra(int *, int, char **);
extern void sfrontpostparsecheck(void);
extern void saolcppcheck(void);
extern void audiodefaults(void);


/****************************************************************/
/*             parses command line arguments                    */
/****************************************************************/

void commandlineargs(int argc, char ** argv)

{
  int i = 1;

  while (i<argc)
    {
      if (singleflags(argv[i]))
	{
	  i++;
	  continue;
	}
      if (oneparamflags(argv[i], (argc == (i + 1)) ? NULL : argv[i+1]))
	{
	  i+=2;
	  continue;
	}
      if (!strcmp(argv[i],"-orc"))
	{
	  cmdlineorchestra(&i, argc, argv);
	  continue;
	}
      printf("Error: Command-line option %s unknown or incorrectly used.\n\n",
	     argv[i]); 
      printhelpshort();
    }

  sfrontpostparsecheck();
  saolcppcheck();
  audiodefaults();
}


/****************************************************************/
/*            what sfront does before exiting normally          */
/****************************************************************/

void sfrontshutdown(void)

{
  if (outfile != NULL)
    fclose(outfile);
  if (orcoutfile != NULL)
    fclose(orcoutfile);
  if (scooutfile != NULL)
    fclose(scooutfile);
  if (midoutfile != NULL)
    fclose(midoutfile);
  deletecppfiles();

}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/* this ends the part of the file that has externs. from now on */
/* functions are declared in the order needed. the rest of the  */
/* file implements the command-line parsing functions -- the    */
/* only exported function is deletecppfiles(), that cleans up   */
/* temporary files used by the preprocessor.                    */
/*______________________________________________________________*/



/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/* the first set of functions print out extended user messages  */
/*______________________________________________________________*/



/****************************************************************/
/*                print bsd license                           */
/****************************************************************/

void printbsd(void)

{

  printf("Copyright (c) 1999-2006, Regents of the University of California\n");
  printf("All rights reserved.\n");
  printf("\n");
  printf("Redistribution and use in source and binary forms, with or without\n");
  printf("modification, are permitted provided that the following conditions are\n");
  printf("met:\n");
  printf("\n");
  printf(" Redistributions of source code must retain the above copyright\n");
  printf(" notice, this list of conditions and the following disclaimer.\n");
  printf("\n");
  printf(" Redistributions in binary form must reproduce the above copyright\n");
  printf(" notice, this list of conditions and the following disclaimer in the\n");
  printf(" documentation and/or other materials provided with the distribution.\n");
  printf("\n");
  printf(" Neither the name of the University of California, Berkeley nor the\n");
  printf(" names of its contributors may be used to endorse or promote products\n");
  printf(" derived from this software without specific prior written permission.\n");
  printf("\n");
  printf("THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS\n");
  printf("\"AS IS\" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT\n");
  printf("LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR\n");
  printf("A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT\n");
  printf("OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,\n");
  printf("SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT\n");
  printf("LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,\n");
  printf("DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY\n");
  printf("THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT\n");
  printf("(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE\n");
  printf("OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.\n");

}

/****************************************************************/
/*             prints top part of help screen                   */
/****************************************************************/

void printhelptop(void)

{

  printf("Usage: sfront [options]\n");
  printf("For input, either:\n");
  printf("       [-bit mp4file]      MP4 binary file\n");
  printf("       [-bitc mp4file]     MP4 binary file: skip streaming data.\n");
  printf("Or:\n");
  printf("       [-orc f1 [f2 ...]]  SAOL orchestra file(s)\n");
  printf("SAOL file(s) may be combined with:\n");
  printf("       [-sco scorefile]    SASL score file\n");
  printf("       [-sstr scorefile]   SASL score file (encode as stream)\n");
  printf("       [-midi midifile]    MIDI score file\n");
  printf("       [-mstr midifile]    MIDI score file (encode as stream)\n");

}

/****************************************************************/
/*             prints short version of help screen              */
/****************************************************************/

void printhelpshort(void)

{

  printhelptop();
  printf("Run 'sfront -help' for more options.\n");
  noerrorplace();

}


/****************************************************************/
/*             prints help screen                               */
/****************************************************************/

void printhelp(void)

{

  printhelptop();
  printf("Produces C output as: \n");
  printf("       [-o cfile]          C output file (default: sa.c)\n");
  printaudiohelp();
  printcontrolhelp();
  printf("Time-management options (last option on command-line used):\n");
  printf("       [-render]           Accurately compute audio output.\n");
  printf("       [-playback]         Real-time streaming audio output.\n");
  printf("       [-timesync]         Real-time interactive option.\n");
  printf("Audio output latency (caution: default value is usually best):\n");
  printf("       [-latency time]     Soundcard latency, in seconds.\n");
  printf("To create an MP4 file, use:\n");
  printf("       [-bitout mp4file]   Generate mp4 bitstream\n");
  printf("       [-symtab]           Include symbol table in file\n");
  printf("To extract component files from MP4 file, use:\n");
  printf("       [-orcout orcfile]   Generate SAOL file\n");
  printf("       [-scoout scorefile] Generate SASL file\n");
  printf("       [-midout midifile]  Generate MIDI bitstream\n");
  printf("SAOL file parsing directives:\n");
  printf("       [-isosyntax]        Reject sfront SAOL enhancements.\n");
  printf("       [-cpp]              Run pre-processor on SAOL files.\n");
  printf("       [-gcc]              System pre-processor is -gcc.\n");
  printf("       [-Is dirname]       Library directory for pre-processor.\n");
  printf("C output file generation directives:\n");
  printf("       [-except]           Generate signal handler in C file.\n");
  printf("       [-hexdata]          Initialize arrays using hex strings.\n");
  printf("       [-fixedseed]        Use same random seed for each run.\n");
  printf("       [-00]               Skip all optimizations.\n");

  if (NET_STATUS == HAS_NETWORKING)
    {
      printf("Networking options (UNIX-only):\n");
      printf("       [-session name]     Join network session [name].\n");
      printf("       [-passphrase key]   Session secret (at least %i characters).\n",
	     MINIMUM_SESSIONKEY);
      printf("       [-bandsize num]     Max number of other players [default %i].\n", DEFAULTBANDSIZE);
      printf("       [-latetime t]       Mute notes arriving t seconds late"
	     " (default %gs)\n", LATETIME_LIMIT);
      printf("       [-lateplay]         Never mute notes (even if very late).\n");
      printf("       [-fec standard]     Full forward-error correction (default).\n");
      printf("       [-fec extra]        Extra-strength FEC, protects NoteOns.\n");
      printf("       [-fec minimal]      Reduced FEC, for low-bandwidth links.\n");
      printf("       [-fec noguard]      FEC w/o last note guard (for debug).\n");
      printf("       [-fec none]         No FEC, for use with a proxy.\n");
      printf("       [-sip_ip dotquad]   SIP IP number (default %s).\n",SIP_IP);
      printf("       [-sip_port num]     SIP server port (default %hu).\n",
	     SIP_RTP_PORT);
      printf("       [-m_semitones num]  Mirror session pitch shift (default %hu).\n",
	     MSESSION_INTERVAL);
    }
  printf("Sample-rate interpolation options:\n");
  printf("       [-interp [linear, sinc]]  Override SAOL global interp parameter.\n");
  printf("       [-sinc_pilen n]     Lookup table size for 1 cycle of sinc sinusoid.\n");
  printf("       [-sinc_zcross n]    Number of zero-crossings on sinc-function lobe.\n");
  printf("Miscellanous options:\n");
  printf("       [-null-program]     Map unassigned MIDI presets to silence.\n");
  printf("       [-mv]               Verbose -midi or -mstr file read.\n");
  printf("       [-pporc]            -orcout shows processed parse tree\n");
  printf("       [-license]          Warantee/License info\n");
  printf("See http://www.cs.berkeley.edu/~lazzaro/sa/sfman/ for full list.\n");
  noerrorplace();

}


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/* the next set of functions handles the pre-processor calls    */
/*______________________________________________________________*/



/****************************************************************/
/*      run cpp: tptr->ttype codes S_SAOLFILE or S_SASLFILE     */
/****************************************************************/

void cppsystemcall(tnode * tptr)

{
  char * command = NULL;
  char * prefix;
  char * lastinclude;
  FILE * fd = NULL;

  /* formulate cpp file name */

  vmcheck(tptr->filename = calloc(strlen(tptr->val)+5, sizeof(char)));
  strcat(strcpy(tptr->filename, tptr->val),".tpp");
  remove(tptr->filename);

  /* generate command string for each compiler */

  switch (compilertype) {
  case GCC_COMPILER:
    if (tptr->ttype == S_SAOLFILE)
      prefix = "gcc -nostdinc -x c++ -E -o ";
    else
      prefix = "gcc -P -nostdinc -x c++ -E -o ";
    lastinclude = " -I " SAOLLIBDIR " ";
    vmcheck(command = calloc(strlen(prefix) + strlen(tptr->filename) 
			     + strlen(cppincludes) + strlen(lastinclude)
			     + strlen(tptr->val) + 1,
		     sizeof(char)));
    sprintf(command,"%s%s%s%s%s", prefix, tptr->filename,
	    cppincludes, lastinclude, tptr->val);
    break;
  default:
    internalerror("sfmain.c","cppsystemcall()");
  }

  /* execute preprocessor */

  if ( system(command) || ! (fd = fopen(tptr->filename,"r")))
    {
      printf("\nError: Pre-processing for file %s was not successful.\n", 
	     tptr->val);
      printf("See diagnostics message above (if any) for details.\n\n");
      noerrorplace();
    }

  free(command);
  fclose(fd);

}

/****************************************************************/
/*             run cpp on the SASL file                         */
/****************************************************************/

void cppsaslfile(FILE ** fd, tnode ** flist, char * option)

{
  int i = 0;
  char * name;
  int argc = sfront_argc;
  char ** argv = sfront_argv;

  /* find SASL file */

  fclose(*fd);
  while ((i < argc) && strcmp(argv[i], option))
    i++;
  
  /* make ????filelist for it */

  if ((*fd = fopen((name = argv[++i]),"r")))
    fclose(*fd);
  else
    {
      vmcheck(name = calloc(strlen(argv[i]) + 6, 1));
      strcat(strcpy(name, argv[i]), ".sasl");
    }

  /* run cpp, make new saslfile on result */
      
  cppsystemcall((*flist = make_tnode(name, S_SASLFILE)));
  *fd = fopen((*flist)->filename,"r");

}

/****************************************************************/
/*             adds SFRONT_INCLUDE_PATH to cppincludes          */
/****************************************************************/

void cppenvironment(void)

{
  char * envstr, * newlib, * oldstr;
  int idx, i, len, lenname;


  if (!(envstr = getenv("SFRONT_INCLUDE_PATH")))
    return;

  /* some systems include name in return string */

  lenname = strlen("SFRONT_INCLUDE_PATH");
  len = strlen(envstr);
  idx = 0; 
  if (len >= lenname)
    {
      while ((idx < len) && (isspace((int)envstr[idx]) || (envstr[idx] == ':')
			      || (envstr[idx] == '=')))
	idx++;
      if ((len - idx) >= lenname)
	if (!strncmp("SFRONT_INCLUDE_PATH",&(envstr[idx]),lenname))
	  idx += lenname;
    }

  /* remove trailing = sign, if any */

  while ((idx < len) && (isspace((int)envstr[idx]) || (envstr[idx] == ':')
			 || (envstr[idx] == '=')))
    idx++;

  /* parse include directories */

  while (idx < len)
    {
      i = 0;
      while ((idx + i < len) && (!isspace((int)envstr[idx+i])) && 
	     (envstr[idx+i] != ':'))
	i++;
      vmcheck(newlib = calloc(i+6, sizeof(char)));
      strcat(newlib," -I ");
      strncat(newlib,&(envstr[idx]),i);
      strcat(newlib," ");
      idx += i;
      oldstr = cppincludes;
      vmcheck(cppincludes = calloc(strlen(oldstr) + strlen(newlib) + 1,
			   sizeof(char)));
      strcat(cppincludes, oldstr);
      strcat(cppincludes, newlib);
      free(oldstr);
      free(newlib);
      while ((idx < len) && (isspace((int)envstr[idx])||(envstr[idx] == ':')))
	idx++;
    }
}


/****************************************************************/
/*    run cpp on saolfilelist,saslfilelist,sstrfilelist         */
/****************************************************************/

void saolcppcheck(void)

{
  tnode * tptr;

  if ((!cppsaol) || (!saolfilelist))
    return;

  if (!systemshell)
    {
      printf("Error: -cpp option not supported on your platform.\n");
      printf("       No shell available for system() command.\n\n");
      printhelpshort();
    }

  if (compilertype != GCC_COMPILER)
    {
      printf("Error: -cpp option requires a companion compiler flag.\n");
      printf("       Supported compiler flags: -gcc\n\n"); 
      printf("Run 'sfront -help' for more information.\n");
      noerrorplace();
    }

  cppenvironment();
  tptr = saolfilelist;
  while (tptr)
    {
      cppsystemcall(tptr);
      tptr = tptr->next;
    }

  if (saslfile)
    cppsaslfile(&saslfile, &saslfilelist, "-sco");
  if (sstrfile)
    cppsaslfile(&sstrfile, &sstrfilelist, "-sstr");


}

/******************************************************************/
/*                deletes any cpp temporary files                 */
/******************************************************************/

void deletecppfiles(void)

{
  tnode * tptr;

  /* delete temporary cpp files */

  if (cppsaol)
    {
      tptr = saolfilelist;
      while (tptr)
	{
	  if (tptr->filename)
	    remove(tptr->filename);
	  tptr = tptr->next;
	}
      if (saslfilelist && saslfilelist->filename)
	remove(saslfilelist->filename);
      if (sstrfilelist && sstrfilelist->filename)
	remove(sstrfilelist->filename);
    }

}


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/* this function runs post-parse, checks for conflicting state  */
/*______________________________________________________________*/


/****************************************************************/
/*            sfront state checking post cmdline parsing        */
/****************************************************************/

void sfrontpostparsecheck(void)

{
  int len;

  if ((saolfilelist == NULL)&&(bitfile == NULL))
    {
      printf("Error:  -orc, -bit, or -bitc option is required.\n");
      printhelpshort();
    }
  if ((bitfile != NULL) &&  ((saolfilelist != NULL) || (saslfile != NULL)||
			      (midifile != NULL) || (mstrfile != NULL)))
    {
      printf("Error: -bit file specified with other input files.\n");
      printhelpshort();
    }
  if ( (bitfile != NULL) && (boutfile != NULL))
    {
      printf("Error: -bitout option cannot be used with -bit option \n");
      printhelpshort();
    }
  if (outfile == NULL)
    {
      outfile = fopen("sa.c","w");
      if (outfile == NULL)
	{
	  printf("Error: Cannot open output file sa.c\n\n");
	  printhelpshort();
	}
    }

  if (ain && aout && (ain != aout) &&
      (ainflow == ACTIVE_FLOW) && (aoutflow == ACTIVE_FLOW))
    {      
      printf("Error: -ain and -aout drivers are both active-flow drivers.\n\n");
      printhelpshort();
    }

  if (reentrant)
    {
      if (cin && !creentrant)
	{      
	  printf("Error: The non-reentrant -cin driver is incompatible with\n");
	  printf("       the re-entrant -aout or -ain driver.\n\n");
	  printhelpshort();
	}
      if (session)
	{      
	  printf("Error: Sfront networking is incompatible with\n");
	  printf("       the re-entrant -aout or -ain driver.\n\n");
	  printhelpshort();
	}
    }

  if ((cin && (!strcmp(cinname,"aucontrol") || !strcmp(cinname,"aucontrolm"))) ||
      (ain && (!strcmp(ainname,"audiounit") || !strcmp(ainname,"audiounit_debug"))) ||
      (aout && (!strcmp(aoutname,"audiounit") || !strcmp(aoutname,"audiounit_debug"))))
    {
      if (ain && aout && cin && !strcmp(cinname, "aucontrol"))
	au_component_type = dupval("aufx");

      if (ain && aout && cin && !strcmp(cinname, "aucontrolm"))
	au_component_type = dupval("aumf");

      if (!ain && aout && cin && !strcmp(cinname, "aucontrolm"))
	au_component_type = dupval("aumu");

      if (!au_component_type)
	{
	  printf("Error: Unsupported combination of AudioUnit drivers:\n");

	  printf("       ");
	  if (aout &&(!strcmp(aoutname,"audiounit") || !strcmp(aoutname,"audiounit_debug")))
	    printf("-aout %s ", aoutname);
	  if ((cin && (!strcmp(cinname,"aucontrol") || !strcmp(cinname,"aucontrolm"))))
	    printf("-cin %s ", cinname);
	  if (ain && (!strcmp(ainname,"audiounit") || !strcmp(ainname,"audiounit_debug")))
	    printf("-ain %s", ainname);
	  printf("\n\n");
	  
	  printf("       Supported combinations:\n");
	  printf("       -aout audiounit -cin aucontrol  -ain audiounit (Effect)\n");
	  printf("       -aout audiounit -cin aucontrolm -ain audiounit (MusicEffect)\n");
	  printf("       -aout audiounit -cin aucontrolm                (MusicDevice)\n");
	  printf("\n       (audiounit_debug may substitute for audiounit)\n");
	  noerrorplace();
	}

      if (!au_filesystem_name)
	{
	  if (!saolfilelist)
	    {
	      printf("Error: -au_filesystem_name flag is required for AUs built"
		     "from -bit/-bitc files.\n");
	      noerrorplace();
	    }

	  au_filesystem_name = dupval(saolfilelist->val);
	  len = strlen(au_filesystem_name);

	  if ((len > 5) && (!strcmp(&(au_filesystem_name[len - 5]), ".saol") ||
			    !strcmp(&(au_filesystem_name[len - 5]),".SAOL")))
	    {
	      au_filesystem_name[len - 5] = '\0';
	      au_filesystem_name[len - 4] = '\0';
	      au_filesystem_name[len - 3] = '\0';
	      au_filesystem_name[len - 2] = '\0';
	      au_filesystem_name[len - 1] = '\0';
	    }
	}

      /* au_component_subtype */

      if (!au_component_subtype)
	{
	  au_component_subtype = dupval("wxyz");
	  au_component_subtype[0] = au_filesystem_name[0];
	  len = strlen(au_filesystem_name);
	  au_component_subtype[1] = ((len > 1) ? 
				     au_filesystem_name[1] : au_component_subtype[1]);
	  au_component_subtype[2] = ((len > 2) ? 
				     au_filesystem_name[2] : au_component_subtype[2]);
	  au_component_subtype[3] = ((len > 3) ? 
				     au_filesystem_name[3] : au_component_subtype[3]);
	}

      if (!au_ui_name)
	au_ui_name = dupval(au_filesystem_name);
      if (!au_component_manu)
	au_component_manu = dupval("Xmpl");
      if (!au_ui_manu)
	au_ui_manu = dupval("Example");
      if (!au_manu_url)
	au_manu_url = dupval("com.example");

      if (!strcmp(au_component_type, "aufx"))
	printf("Making an Effect AudioUnit\n\n");
      if (!strcmp(au_component_type, "aumf"))
	printf("Making a MusicEffect AudioUnit\n\n");
      if (!strcmp(au_component_type, "aumu"))
	printf("Making a MusicDevice AudioUnit\n\n");
    }

  cinmaxchan = cmaxchan;

  if (session)
    {
      if ((sessionkey == NULL) && strcmp("mirror", session))
	{
	  printf("Error: -passphrase option missing.\n\n");
	  printhelpshort();
	}

      if (!strcmp("mirror", session))
	netmsets = 1;                   /* for security reasons */

      netstart = cmaxchan;
      cmaxchan += 16*netmsets;    /* 16 channels per netuser  */
      if (fixedseed)
	{
	  fixedseed = 0;
	  printf("Warning: -fixedseed ignored due to network requirements.\n");
	}
    }
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/* the remaining files actually parse cmdline flags             */
/*______________________________________________________________*/

/****************************************************************/
/*             opens a command-line file                        */
/****************************************************************/

int cmdlinefileopen(FILE ** stream, char * command, char * name, 
		    char * mode, char * ext)

{
  char * suffix;

  if (*stream != NULL)
    {
      printf("Error: sfront only processes one %s file\n\n", command);
      printhelpshort();
    }
  if (!name)
    {
      printf("Error: No filename specified for %s option\n", command);
      printhelpshort();
    }
  *stream = fopen(name, mode);
  if (*stream == NULL)
    {
      vmcheck(suffix = calloc(strlen(name)+strlen(ext)+2, sizeof(char)));
      sprintf(suffix,"%s.%s", name, ext);
      *stream = fopen(suffix, mode);
      if (*stream == NULL)
	{
	  printf("Error: Cannot open %s file `%s' [also tried `%s'\n\n",
		 command, name, suffix);
	  printhelpshort();
	}
      free(suffix);
    }
  return 1;
}

/****************************************************************/
/*             opens a command-line file                        */
/****************************************************************/

int cmddriveropen(int od, char * command, char * name, int check)

{
  if (od)
    {
      printf("Error: sfront only processes one %s file/device\n\n", command);
      printhelpshort();
    }
  if (!name)
    {
      printf("Error: No %s file/device specified\n\n", command);
      printhelpshort();
    }
  if (check)
    {
      printf("Error: Bad %s file/device: %s\n\n", command, name);
      printhelpshort();
    }
  return 1;
}

/****************************************************************/
/*             parses one-parameter flags                    */
/****************************************************************/

int oneparamflags(char * s1, char * s2)

{  
  int od, i, q1, q2, q3, q4; 
  float f;
  char * name1, * name2, * sptr;

  /* all files */

  if ((!strcmp(s1,"-bit"))||(!strcmp(s1,"-bitc")))
    {
      if (saolfilelist != NULL)
	{
	  printf("sfront can't combine -orc and -bit files\n\n");
	  printhelpshort();
	}
      bitreadaccessunits = strcmp(s1,"-bitc") ? 1 : 0;
      isocompliant = 1;
      return cmdlinefileopen(&bitfile, s1, s2, "rb", "mp4"); 
    }
  if (!strcmp(s1,"-bitout"))
    return cmdlinefileopen(&boutfile, s1, s2, "wb", "mp4"); 
  if (!strcmp(s1,"-midi"))
    return cmdlinefileopen(&midifile, s1, s2, "rb", "mid"); 
  if (!strcmp(s1,"-midout"))
    return cmdlinefileopen(&midoutfile, s1, s2, "wb", "mid"); 
  if (!strcmp(s1,"-mstr"))
    return cmdlinefileopen(&mstrfile, s1, s2, "rb", "mid"); 
  if (!strcmp(s1,"-o"))
    return cmdlinefileopen(&outfile, s1, s2, "w", "c"); 
  if (!strcmp(s1,"-orcout"))
    return cmdlinefileopen(&orcoutfile, s1, s2, "w", "saol"); 
  if (!strcmp(s1,"-sco"))
    return cmdlinefileopen(&saslfile, s1, s2, "r", "sasl"); 
  if (!strcmp(s1,"-scoout"))
    return cmdlinefileopen(&scooutfile, s1, s2, "w", "sasl"); 
  if (!strcmp(s1,"-sstr"))
    return cmdlinefileopen(&sstrfile, s1, s2, "r", "sasl"); 

  /* all drivers */

  if (!strcmp(s1,"-ain"))
    {
      od = ain;
      return cmddriveropen(od, s1, s2, (!ain && s2) ? ainfilecheck(s2) : 0);
    }
  if (!strcmp(s1,"-aout"))
    {
      od = aout;
      return cmddriveropen(od, s1, s2, (!aout && s2) ? aoutfilecheck(s2) : 0);
    }
  if (!strcmp(s1,"-cin"))
    {
      od = cin;
      return cmddriveropen(od, s1, s2, (!cin && s2) ? cinfilecheck(s2) : 0);
    }

  /* other parameters */

  if ((NET_STATUS == HAS_NETWORKING) && (!strcmp(s1,"-bandsize")))
    {
       
      if (!s2 || (sscanf(s2, "%i", &netmsets) != 1) || 
	  (netmsets < 0) ||(netmsets > MAXBANDSIZE))
	{
	  if (s2)
	    printf("Error: Illegal -bandsize parameter %s \n\n", s2);
	  else
	    printf("Error: Missing -bandsize parameter.\n\n");
	  printhelpshort();
	}
      return 1;
    }

  if ((NET_STATUS == HAS_NETWORKING) && (!strcmp(s1,"-fec")))
    {
       
      if (!s2)
	{
	  printf("Error: Missing -fec level "
		 "[extra, standard, minimal, noguard, none].\n\n");
	  printhelpshort();
	}

      if (!strcmp("extra", s2))
	feclevel = FEC_EXTRA;
      else
	if (!strcmp("standard", s2))
	  feclevel = FEC_STANDARD;
	else
	  if (!strcmp("minimal", s2))
	    feclevel = FEC_MINIMAL;
	  else
	    if (!strcmp("noguard", s2))
	      feclevel = FEC_NOGUARD;
	    else
	      if (!strcmp("none", s2))
		feclevel = FEC_NONE;
	      else
		{
		  printf("Error: Invalid -fec level %s; use one of\n"
			 "[extra, standard, minimal, noguard, none]\n\n",
			 s2);
		  printhelpshort();
		} 

      return 1;
    }

  if ((!strcmp(s1,"-interp")))
    {
      if (!s2)
	{
	  printf("Error: Missing -interp type [linear | sinc].\n\n");
	  printhelpshort();
	}

      if (!strcmp("linear", s2))
	interp_cmdline = INTERP_LINEAR;
      else
	if (!strcmp("sinc", s2))
	  interp_cmdline = INTERP_SINC;
	else
	  {
	    printf("Error: Invalid -interp level %s; use one of\n"
		   "[linear, sinc]\n\n", s2);
	    printhelpshort();
	  } 
      
      return 1;
    }

  if (!strcmp(s1,"-latency"))
    {
       
      if (!s2 || (sscanf(s2, "%f", &latency) != 1))
	{
	  printf("Error: Missing or illegal -latency time parameter.\n\n");
	  printhelpshort();
	}
      return 1;
    }

  if ((NET_STATUS == HAS_NETWORKING) &&(!strcmp(s1,"-latetime")))
    {
       
      if (!s2 || (sscanf(s2, "%f", &latetime) != 1))
	{
	  printf("Error: Missing or illegal -latetime time parameter.\n\n");
	  printhelpshort();
	}
      return 1;
    }

  if ((NET_STATUS == HAS_NETWORKING) && ((!strcmp(s1,"-m_semitones")) || 
					 (!strcmp(s1,"-m_semitone"))))
    {
       
      if (!s2 || (sscanf(s2, "%i", &i) != 1) || (i < 0) || (i > 24))
	{
	  printf("Error: Missing or illegal -m_semitones parameter.\n");
	  printf("     : Use a positive integer between 0 and 24.\n\n");
	  printhelpshort();
	}
      msession_interval = i;
      return 1;
    }

  if ((NET_STATUS == HAS_NETWORKING) && (!strcmp(s1,"-passphrase")))
    {
       
      if (!s2)
	{
	  printf("Error: Missing -passphrase parameter.\n\n");
	  printhelpshort();
	}

      if (strlen(s2) < MINIMUM_SESSIONKEY)
	{
	  printf("Error: Passphrase \"%s\" less than %i characters in length.\n\n",
		 s2, MINIMUM_SESSIONKEY);
	  printhelpshort();
	}

      sessionkey = dupval(s2);
      return 1;
    }

  if ((NET_STATUS == HAS_NETWORKING) && (!strcmp(s1,"-session")))
    {
       
      if (!s2)
	{
	  printf("Error: Missing -session name parameter.\n\n");
	  printhelpshort();
	}

      for (i=0; i < ((int)strlen(s2)); i++)
	if ( !isalnum((int)s2[i]) && 
	     (s2[i] != '-') && (s2[i] != '_') && (s2[i] != '.') && 
	     (s2[i] != '!') && (s2[i] != '~') && (s2[i] != '*') &&
	     (s2[i] != '\'') && (s2[i] != '(') && (s2[i] != ')'))
	{
	  if (s2[i] == ' ')
	    printf("Error: Space character not allowed in session name\n");
	  else
	    printf("Error: Character '%c' not allowed in session name.\n",
		   s2[i]);

	  printf("     : Use letters, numbers" 
		 "and - _ . ! ~ * ' ( ) only.\n\n");
	  printhelpshort();
	}

      session = dupval(s2);
      return 1;
    }

  if (!strcmp(s1,"-au_component_manu"))
    {
       
      if (!s2)
	{
	  printf("Error: Missing -au_component_manu parameter.\n");
	  noerrorplace();
	}

      if (strlen(s2) != 4)
	{
	  printf("Error: -au_component_manu code \"%s\" is not exactly 4 characters in length.\n",
		 s2);
	  noerrorplace();
	}

      au_component_manu = dupval(s2);
      return 1;
    }

  if (!strcmp(s1,"-au_component_subtype"))
    {
       
      if (!s2)
	{
	  printf("Error: Missing -au_component_subtype parameter.\n");
	  noerrorplace();
	}

      if (strlen(s2) != 4)
	{
	  printf("Error: -au_component_subtype code \"%s\" is not exactly 4 characters in length.\n",
		 s2);
	  noerrorplace();
	}

      au_component_subtype = dupval(s2);
      return 1;
    }

  if (!strcmp(s1,"-au_filesystem_name"))
    {
       
      if (!s2)
	{
	  printf("Error: Missing -au_filesystem_name parameter.\n");
	  noerrorplace();
	}

      au_filesystem_name = dupval(s2);
      return 1;
    }

  if (!strcmp(s1,"-au_manu_url"))
    {
       
      if (!s2)
	{
	  printf("Error: Missing -au_manu_url parameter.\n");
	  noerrorplace();
	}

      au_manu_url = dupval(s2);

      if ((sptr = strstr(au_manu_url, ".audiounit")))
	*sptr = '\0';

      return 1;
    }

  if (!strcmp(s1,"-au_ui_name"))
    {
       
      if (!s2)
	{
	  printf("Error: Missing -au_ui_name parameter.\n");
	  noerrorplace();
	}

      au_ui_name = dupval(s2);
      return 1;
    }

  if (!strcmp(s1,"-au_ui_manu"))
    {
       
      if (!s2)
	{
	  printf("Error: Missing -au_ui_manu parameter.\n");
	  noerrorplace();
	}

      au_ui_manu = dupval(s2);
      return 1;
    }

  if (!strcmp(s1,"-au_view_bundlename"))
    {
       
      if (!s2)
	{
	  printf("Error: Missing -au_view_bundlename parameter.\n");
	  noerrorplace();
	}

      au_view_bundlename = dupval(s2);

      if ((sptr = strstr(au_view_bundlename, ".bundle")))
	*sptr = '\0';

      return 1;
    }

  if (!strcmp(s1,"-au_view_baseclass"))
    {
       
      if (!s2)
	{
	  printf("Error: Missing -au_view_baseclass parameter.\n");
	  noerrorplace();
	}

      au_view_baseclass = dupval(s2);

      return 1;
    }

  if (!strcmp(s1,"-sinc_zcross"))
    {
      if (!s2 || (sscanf(s2, "%f", &f) != 1))
	{
	  printf("Error: Missing or illegal -sinc_zcross parameter.\n\n");
	  printhelpshort();
	}
      sinc_zcross = (unsigned int) f;
      if ((f != sinc_zcross) || (sinc_zcross < 1) || (sinc_zcross > 30))
	{
	  printf("Error: -sinc_zcross parameter must be integer in 1-30 range.\n\n");
	  printhelpshort();
	}
      return 1;
    }

  if (!strcmp(s1,"-sinc_pilen"))
    {
      if (!s2 || (sscanf(s2, "%f", &f) != 1))
	{
	  printf("Error: Missing or illegal -sinc_pilen parameter.\n\n");
	  printhelpshort();
	}
      sinc_pilen = (unsigned int) f;
      if ((f != sinc_pilen) || 
	  ((sinc_pilen != 2) && (sinc_pilen != 4) && (sinc_pilen != 8) && 
	   (sinc_pilen != 16) && (sinc_pilen != 32) && (sinc_pilen != 64) &&
	   (sinc_pilen != 128) && (sinc_pilen != 256) && (sinc_pilen != 512) &&
	   (sinc_pilen != 1024) && (sinc_pilen != 2048) && (sinc_pilen != 4096) &&
	   (sinc_pilen != 8192)))
	{
	  printf("Error: -sinc_pilen parameter must be <= 8192 and a power of 2.\n\n");
	  printhelpshort();
	}
      return 1;
    }

  if ((NET_STATUS == HAS_NETWORKING) && (!strcmp(s1,"-sip_ip")))
    {
      if (!s2)
	{
	  printf("Error: Missing or illegal -sip_ip parameter.\n\n");
	  printhelpshort();
	}

      q1 = q2 = q3 = q4 = -1;

      if (sscanf(s2, "%3i.%3i.%3i.%3i", &q1, &q2, &q3, &q4) != 4)
	{
	  printf("Error: Bad format (1) for -sip_ip" 
		 " dotted-quad: %s.\n\n", s2);
	  printhelpshort();
	}

      if ((q1 < 0) || (q1 > 255) || (q2 < 0) || (q2 > 255) ||
	  (q3 < 0) || (q3 > 255) || (q4 < 0) || (q4 > 255))
	{
	  printf("Error: Bad format (2) for -sip_ip" 
		 " dotted-quad: %s.\n\n", s2);
	  printhelpshort();
	}

      strcpy(sip_ip, s2);
      return 1;
    }

  if ((NET_STATUS == HAS_NETWORKING) &&(!strcmp(s1,"-sip_port")))
    {
       
      if (!s2 || (sscanf(s2, "%i", &i) != 1) || (i < 0) || (i > 65535))
	{
	  printf("Error: Missing or illegal -sip_port parameter.\n\n");
	  printhelpshort();
	}
      sip_port = (unsigned short) i;
      return 1;
    }

  if (!strcmp(s1,"-Is"))
    {
      if (!s2)
	{
	  printf("Error: -Is option not followed by directory name.\n\n");
	  printhelpshort();
	}
      vmcheck(name1 = calloc(strlen(s2)+6, sizeof(char)));
      sprintf(name1," -I %s ", s2);
      name2 = cppincludes;
      vmcheck(cppincludes = calloc(strlen(name1)+strlen(name2)+1, 
				   sizeof(char)));
      strcpy(cppincludes,name2);
      strcat(cppincludes,name1);
      free(name1);
      free(name2);
      return 1;
    }

  return 0;
}


/****************************************************************/
/*             parses orchestra files                           */
/****************************************************************/

void cmdlineorchestra(int * idx, int argc, char ** argv)

{	
  char * name;
  tnode * tptr;
  int done = 0;
  int i;

  if ((bitfile != NULL))
    {
      printf("Error: Both -orc and -bit files specified.\n\n");
      printhelpshort();
    }
  if ((i = ((*idx) + 1)) == argc)
    {
      printf("Error: -orc option without a SAOL file.\n");
      printhelpshort();
    }
  while (!done)
    {
      vmcheck(name = calloc(strlen(argv[i])+6, sizeof(char)));
      saolfile = fopen(strcpy(name, argv[i]),"r");
      if (saolfile == NULL)
	{
	  saolfile = fopen(strcat(name,".saol"),"r");
	  if (saolfile == NULL)
	    {
	      printf("Error: Cannot open -orc file `%s' [also tried `%s'\n\n"
		     ,argv[i], name);
	      printhelpshort();
	    }
	}
      fclose(saolfile);
      if (saolfilelist == NULL)
	saolfilelist = make_tnode(name, S_SAOLFILE);
      else
	{
	  tptr = saolfilelist;
	  while (tptr->next != NULL)
	    tptr = tptr->next;
	  tptr->next = make_tnode(name, S_SAOLFILE);
	}
      i++;
      if ((argc == i) || (argv[i][0] == '-'))
	done = 1;
    }

  *idx = i;

}

/****************************************************************/
/*             parses parameter-free flags                    */
/****************************************************************/

int singleflags(char * s)

{

  if (!strcmp(s,"-cpp"))
    {
      cppsaol = 1;
      return 1;
    }
  if (!strcmp(s,"-except"))
    {
      catchsignals = 1;
      return 1;
    }
  if (!strcmp(s,"-fixedseed"))
    {
      fixedseed = 1;
      return 1;
    }
  if (!strcmp(s,"-gcc"))
    {
      compilertype = GCC_COMPILER;
      return 1;
    }
  if (!strcmp(s,"-help"))
    {
      printhelp();
    }
  if (!strcmp(s,"-hexdata"))
    {
      hexstrings = 1;
      return 1;
    }
  if (!strcmp(s,"-isosyntax"))
    {
      isocompliant = 1;
      return 1;
    }
  if ((NET_STATUS == HAS_NETWORKING) && (!strcmp(s,"-lateplay")))
    {
      lateplay = 1;
      return 1;
    }
  if ((!strcmp(s,"-lics"))||(!strcmp(s,"-lisc"))||
      (!strcmp(s,"-license")))
    {
      printbsd();
      noerrorplace();
    }
  if (!strcmp(s,"-mv"))
    {
      midiverbose = 1;
      return 1;
    }
  if ((!strcmp(s,"-isocheck"))||(!strcmp(s,"-no-inline")))
    {
      isocheck = 1;
      return 1;
    }
  if (!strcmp(s,"-no-rateopt"))
    {
      rateoptimize = 0;
      return 1;
    }
  if (!strcmp(s,"-no-constopt"))
    {
      constoptimize = 0;
      return 1;
    }
  if (!strcmp(s,"-null-program"))
    {
      null_program = 1;
      return 1;
    }
  if (!strcmp(s,"-O0"))
    {
      isocheck = 1;
      rateoptimize = 0;
      constoptimize = 0;
      return 1;
    }
  if (!strcmp(s,"-playback"))
    {
      timeoptions = PLAYBACK;
      return 1;
    }
  if (!strcmp(s,"-pporc"))
    {
      ascsaolptree = 1;
      return 1;
    }
  if (!strcmp(s,"-render"))
    {
      timeoptions = RENDER;
      return 1;
    }
  if (!strcmp(s,"-timesync"))
    {
      timeoptions = TIMESYNC;
      return 1;
    }
  if (!strcmp(s,"-symtab"))
    {
      bitwritenosymbols = 0;
      return 1;
    }
  
  return 0;
}

