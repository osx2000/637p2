
/*
#    Sfront, a SAOL to C translator    
#    This file: Creates new ../*lib.{c,h} files for sfront
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


#include "libmaker.h"

/* following declarations formerly in globals.c */

int libtype = DIRSIZE;  /* CSRC, ASYS, CSYS, NSYS, PSYS */

char * libdirs[DIRSIZE] = {"csrc", "asys", "csys", "nsys", "psys"};

char * libnames[NAMESIZE]; /* library file names */

int numnames = 0;

FILE * infile = NULL;
FILE * outfile = NULL;

/* end of global declarations */

/****************************************************************/
/*             prints help screen                               */
/****************************************************************/

void exitlibmaker(void)

{

  exit(0);

}

/****************************************************************/
/*             prints help screen                               */
/****************************************************************/

void printhelp(void)

{

  printf("Usage: libmaker [options] [filenames]\n");
  printf("Specify one of the following options:\n");
  printf("-csrc   Make csrclib.{c,h} files.\n");
  printf("-asys   Make asyslib.{c,h} files.\n");
  printf("-csys   Make csyslib.{c,h} files.\n");
  printf("-nsys   Make nsyslib.{c,h} files.\n");
  printf("-psys   Make psyslib.{c,h} files.\n");
  printf("Followed by the filenames to include in the\n");
  printf("library (without paths or suffixes).\n");
  exitlibmaker();

}

/****************************************************************/
/*             parses command line arguments                    */
/****************************************************************/

void commandlineargs(int argc, char ** argv)

{
  int i = 1;
  char name[255];
  int done;

  printf("libmaker, compresses sfront libraries. Version %s.\n",IDSTRING);
  printf("Run libmaker with -license option for Copyright/License info.\n\n");

  if (argc == 1)
    printhelp();
  else
    while (i<argc)
      {
	if ((!strcmp(argv[i],"-license")))
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

	    i++;
	    exitlibmaker();
	  }
	if ((!strcmp(argv[i],"-csrc")) || (!strcmp(argv[i],"-asys")) ||
	    (!strcmp(argv[i],"-csys")) || (!strcmp(argv[i],"-nsys")) ||
	    (!strcmp(argv[i],"-psys")))
	  {
	    if (libtype != DIRSIZE)
	      {
		printf("error -- multiple lib options specified\n");
		printhelp();
	      }
	    if (!strcmp(argv[i],"-csrc"))
	      libtype = CSRC;
	    if (!strcmp(argv[i],"-asys"))
	      libtype = ASYS;
	    if (!strcmp(argv[i],"-csys"))
	      libtype = CSYS;
	    if (!strcmp(argv[i],"-psys"))
	      libtype = PSYS;
	    if (!strcmp(argv[i],"-nsys"))
	      libtype = NSYS;
	    i++;
	    continue;
	  }
	if (argv[i][0] == '-')
	  {
	    printf("unknown option %s\n\n",argv[i]);
	    printhelp();
	  }
	libnames[numnames] = argv[i];
	numnames++;
	i++;
      }

  if (libtype == DIRSIZE)
    {
      printf("error -- multiple lib options specified\n\n");
      printhelp();
    }

}

/****************************************************************/
/*             writes preamble to library file                  */
/****************************************************************/

void writepreamble()

{
  fprintf(outfile,"\n/*\n");

  fprintf(outfile," * Copyright (c) 1999-2006, Regents of the University of California\n");
  fprintf(outfile," * All rights reserved.\n");
  fprintf(outfile," * \n");
  fprintf(outfile," * Redistribution and use in source and binary forms, with or without\n");
  fprintf(outfile," * modification, are permitted provided that the following conditions are\n");
  fprintf(outfile," * met:\n");
  fprintf(outfile," * \n");
  fprintf(outfile," *  Redistributions of source code must retain the above copyright\n");
  fprintf(outfile," *  notice, this list of conditions and the following disclaimer.\n");
  fprintf(outfile," * \n");
  fprintf(outfile," *  Redistributions in binary form must reproduce the above copyright\n");
  fprintf(outfile," *  notice, this list of conditions and the following disclaimer in the\n");
  fprintf(outfile," *  documentation and/or other materials provided with the distribution.\n");
  fprintf(outfile," * \n");
  fprintf(outfile," *  Neither the name of the University of California, Berkeley nor the\n");
  fprintf(outfile," *  names of its contributors may be used to endorse or promote products\n");
  fprintf(outfile," *  derived from this software without specific prior written permission.\n");
  fprintf(outfile," * \n");
  fprintf(outfile," * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS\n");
  fprintf(outfile," * \"AS IS\" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT\n");
  fprintf(outfile," * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR\n");
  fprintf(outfile," * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT\n");
  fprintf(outfile," * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,\n");
  fprintf(outfile," * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT\n");
  fprintf(outfile," * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,\n");
  fprintf(outfile," * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY\n");
  fprintf(outfile," * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT\n");
  fprintf(outfile," * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE\n");
  fprintf(outfile," * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.\n");
  fprintf(outfile," * \n");
  fprintf(outfile," * Automatically generated library file %s\n\n",libdirs[libtype]);
  fprintf(outfile," * The drivers embedded in this file may be covered by a different\n");
  fprintf(outfile," * license.  Scroll down to see the license appearing before each\n");
  fprintf(outfile," * driver, or see sfront/src/lib/ directories for driver source file.\n\n");
  fprintf(outfile," */\n\n");
}


/****************************************************************/
/*                  writes library files                        */
/****************************************************************/

void writefiles(void)

{
  int i, j, done;
  char name[256];
  char c;

  /* open library .c file */

  sprintf(name,"../%slib.c",libdirs[libtype]);
  outfile = fopen(name,"w");
  if (outfile == NULL)
    {
      printf("error -- %s can not be opened\n\n", name);
      printhelp();
    }
  writepreamble();
  fprintf(outfile,"#include \"tree.h\"\n\n");

  /* fill the library file */

  for (i = 0; i < numnames; i++)
    {
      sprintf(name,"%s/%s.c",libdirs[libtype],libnames[i]);
      infile = fopen(name,"r");
      if (infile == NULL)
	{
	  printf("error -- %s can not be opened\n\n", name);
	  printhelp();
	}
      done = fread(&c,sizeof(char),1,infile);

      j = 0;

      fprintf(outfile,"\n\n");
      fprintf(outfile,"void make%s(void)\n",libnames[i]);
      fprintf(outfile,"{\n");
      fprintf(outfile,"  int lc = 0;\n\n");

      while (done)
	{
	  j++;
	  fprintf(outfile,"  z[lc++]=\"");
	  while (done && (c != '\n'))
	    {
	      if ((c == '"')||(c == '\\'))
		putc('\\',outfile);
	      if (c != 13)               /* ^M, i.e. CR */
		putc(c,outfile);
	      done = fread(&c,sizeof(char),1,infile);
	    }
	  fprintf(outfile,"\";\n");
	  done = fread(&c,sizeof(char),1,infile);
	}
      fclose(infile);

      fprintf(outfile,"  printlib(lc);\n");
      fprintf(outfile,"}\n");

      if (j > ZSIZE)
	{
	  printf("Error: %s overwriting z[], increase ZSIZE (%i)\n", 
		 libnames[i], j);
	  exit(-1);
	}

    }
  fprintf(outfile,"\n\n");
  fclose(outfile);

  /* open library .h file */

  sprintf(name,"../%slib.h",libdirs[libtype]);
  outfile = fopen(name,"w");
  if (outfile == NULL)
    {
      printf("error -- %s can not be opened\n\n", name);
      printhelp();
    }
  writepreamble();

  for (i = 0; i < numnames; i++)
    fprintf(outfile,"extern void make%s(void);\n",libnames[i]);

  fprintf(outfile,"\n\n");
  fclose(outfile);

}


/****************************************************************/
/*                     main() for libmaker                        */
/****************************************************************/

int main(int argc, char ** argv)

{

  commandlineargs(argc, argv);
  writefiles();
  return 0;
}





