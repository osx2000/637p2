
/*
#    Sfront, a SAOL to C translator    
#    This file: Lexical analyzer for SAOL
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


/******************************************************************/
/*             errors without a place: closes sfront              */
/******************************************************************/

void noerrorplace(void)

{

  printf("\nEnding sfront.\n");
  if (outfile != NULL)
    fclose(outfile);
  if (boutfile != NULL)
    fclose(boutfile);
  if (orcoutfile != NULL)
    fclose(orcoutfile);
  if (scooutfile != NULL)
    fclose(scooutfile);
  if (midoutfile != NULL)
    fclose(midoutfile);
  deletecppfiles();

  exit(-1);
}


/******************************************************************/
/*     prints out offending region of saolfile, closes sfront     */
/******************************************************************/

void showerrorplace(int badline, char * filename)

{
  int c;

  if (filename == NULL)
    {
      printf("Syntax error parsing orc_file SA block of -bit file.\n");
      printf("Use -orcout to generate ASCII saolfile(s) and\n");
      printf("run sfront on these files to pinpoint error.\n");
      noerrorplace();
    }

  saolfile = fopen(filename,"r");
  saollinenumber = 1;

  printf("Error occured near line %i in file %s:\n\n", badline, filename);

  while (badline - 2 > saollinenumber)
    {
      c = getc(saolfile);
      if (c == '\n')
	saollinenumber++;
    }

  while (badline + 2 > saollinenumber)
    {
      c = getc(saolfile);
      if (c == '\n')
	saollinenumber++;
      if (c == EOF)
	break;
      putchar(c);
    }
  noerrorplace();
}


/******************************************************************/
/*                    reports compiler bugs                       */
/******************************************************************/

void internalerror(char * file, char * desc)

{

  printf("Internal compiler error -- source file %s\n",file);
  printf("Problem description: %s\n",desc);
  printf("Send a bug report to lazzaro@cs.berkeley.edu --\n");
  printf("If possible, include input files. Thanks! --jl\n");
  noerrorplace();

}

/******************************************************************/
/*                    reports compiler bugs                       */
/******************************************************************/

void warningmessage(tnode * tptr, char * desc)

{

  printf("Warning: %s\n", desc);
  if (tptr != NULL)
    printf("       : File %s near line %i (%s):\n",
	   tptr->filename,tptr->linenum,tptr->val);

}

/******************************************************************/
/*          reports syntax error and leaves sfront                 */
/******************************************************************/

void yyerror (char * s)  

{
  if (saolfile)
    {
      printf("Syntax error during -saolfile parsing.\n\n");
    }
  else
    {
      printf("Syntax error parsing orc_file SA block of -bit file.\n");
      printf("Use -orcout to generate ASCII saolfile(s) and\n");
      printf("run sfront on these files to pinpoint error.\n");
      noerrorplace();
    }
  showerrorplace(saollinenumber, saolsourcefile);

}


/******************************************************************/
/*         intelligent "white space" detector, strips extra ;'s   */
/******************************************************************/

int iswhitespace(int c)

{

  /* This function strips out superfluous ;'s and also white space. */
  /* For non-semicolons, check for whitespace and return.           */

  if (c != ';')
    return isspace(c);

  /* disable semicolon stripping in iso-compliant mode, or no cpp    */

  if (isocompliant || (!cppsaol))
    return 0;

  /* send statement is unique, two semicolons can be adjacent        */

  if (sendsemicoloncount)
    {
      sendsemicoloncount--;
      return 0;
    }

  /* places to strip semicolons -- lexttl is last token              */

  if ((lexttl == S_LC) || (lexttl == S_RC) || (lexttl == S_SEM))
    return 1;

  /* let all other semicolons through */

  return 0;
  
}

/******************************************************************/
/*          points parser to next saol file                       */
/******************************************************************/

int getnextsaolfile(void)

{
  int ret = 1;

  if (currsaolfile->next == NULL)
    {
      fclose(saolfile);
      ret = 0;
    }
  else
    {  
      fclose(saolfile);
      currsaolfile = currsaolfile->next;
      if (currsaolfile->filename)
	saolfile = fopen(currsaolfile->filename,"r");
      else
	saolfile = fopen(currsaolfile->val,"r");
      yylval->linenum  = saollinenumber = 1;
      yylval->filename = saolsourcefile = currsaolfile->val;
    }
  return ret;

}

/******************************************************************/
/*          handles pre-processor line commands                   */
/******************************************************************/

int preprocesslinenum(int c)

{
  char * buffer, * filename;
  int num, linenum;

  if (compilertype == GCC_COMPILER)
    {
      if (c != '#')
	return 0;

      vmcheck(buffer = calloc(1024, sizeof(char)));
      vmcheck(filename = calloc(1024, sizeof(char)));

      fgets(buffer, 1024, saolfile);

      /* scanf strips leading double-quote */

      if ((num = sscanf(buffer," %i \"%1023s ", &linenum, filename)))
	{
	  yylval->linenum = saollinenumber = linenum;
	  if (--num)
	    {
	      /* stripping trailing double-quote */

	      filename[strlen(filename)-1] = '\0'; 
	      yylval->filename = saolsourcefile = filename;
	    }
	}
      free(buffer);
      return 1;
    }

  /* return should never execute */

  return 0;
  
}

/******************************************************************/
/*          template state machine: wmap, gt transition          */
/******************************************************************/

int lexstate_wmap(void)

{	    
  tnode * holdyy;
  int lookret;

  /* look one ahead */

  holdyy = yylval;

  lexstatemachine = TEMPLATE_LOOKAHEAD;
  lookret = yylex();  
  if (++lexstackptr == LEXSTACKSIZE)
    {
      printf("Error: Template-related syntax error.\n");
      showerrorplace(saollinenumber, saolsourcefile);
    }
  lexstackret[lexstackptr] = lookret;
  lexstacktnode[lexstackptr] = yylval;
  
  yylval = holdyy;
  
  if (lookret == COM)
    {
      lexstatemachine = TEMPLATE_WITH;
      return GTT; 
    }
  if (lookret == RC)
    {
      lexstatemachine = TEMPLATE_REST;
      return GTT; 
    }
  lexstatemachine = TEMPLATE_WMAPLIST;
  return GT;

}


/******************************************************************/
/*          template state machine: pmap, gt transition          */
/******************************************************************/

int lexstate_pmap(void)

{	    
  tnode * holdyy;
  int lookret;
  int i;
	    
  holdyy = yylval;
  
  lexstatemachine = TEMPLATE_LOOKAHEAD;
  lookret = yylex(); 
  if (lookret != LP)
    {
      if (++lexstackptr == LEXSTACKSIZE)
	{
	  printf("Error: Template-related syntax error.\n");
	  showerrorplace(saollinenumber, saolsourcefile);
	}
      lexstackret[lexstackptr] = lookret;
      lexstacktnode[lexstackptr] = yylval;	    
      yylval = holdyy;
      if (lookret == COM)
	{
	  lexstatemachine = TEMPLATE_PRESET;
	  return GTT; 
	}
      lexstatemachine = TEMPLATE_PMAPLIST;
      return GT;
    }
  else
    {
      lexholdptr = 0;
      lexholdret[lexholdptr] = lookret; 
      lexholdtnode[lexholdptr] = yylval;
      while ( (!lexholdptr) || (lookret == IDENT) || 
	      (lookret == RP))
	{
	  if (++lexholdptr == LEXSTACKSIZE)
	    {
	      printf("Error: Template-related syntax error.\n");
	      showerrorplace(saollinenumber, saolsourcefile);
	    }
	  lookret = lexholdret[lexholdptr] = yylex();
	  lexholdtnode[lexholdptr] = yylval;
	}
      if ((lookret == MAP)||(lookret == COM))
	lexstatemachine = TEMPLATE_ACTIVE;
      else 
	{
	  lexstatemachine = TEMPLATE_PMAPLIST;
	  if (lookret == GT)
	    {	  
	      printf("Error: Preset expr uses a variable.\n");
	      showerrorplace(yylval->linenum, yylval->filename);
	    }
	}
      for (i = 0; i <= lexholdptr; i++)
	{
	  if (++lexstackptr == LEXSTACKSIZE)
	    {
	      printf("Error: Template-related syntax error.\n");
	      showerrorplace(saollinenumber, saolsourcefile);
	    }
	  lexstackret[lexstackptr] = lexholdret[i];
	  lexstacktnode[lexstackptr] = lexholdtnode[i];
	}
      yylval = holdyy;
      if (lexstatemachine == TEMPLATE_ACTIVE)
	return GTT;
      else
	return GT;
    }
}


/******************************************************************/
/*          lexical analyzer for SAOL files                       */
/******************************************************************/

int yylex (void)

{

  int c;
  int i;
  int foundit;
  int strstart;
  int nonzero;    /* for returning 1.00 as an INT */
  char buf[LEXBUFSIZE] = {'\0'};

  /* check lookahead buffer first */

  if (lexstackptr >= 0)
    {
      yylval = lexstacktnode[0];
      c = lexstackret[0];
      i = 1;
      while (i <= lexstackptr)
	{
	  lexstacktnode[i-1] = lexstacktnode[i];
	  lexstackret[i-1] = lexstackret[i];
	  i++;
	}
      lexstackptr--;
      return c;
    }

  /* lexical analaysis begins here */

  if (bitfile != NULL)
    return orclex();

  yylval = make_tnode("", S_BADCHAR);

  yylval->linenum  = saollinenumber;
  yylval->filename = saolsourcefile;

  /* delete whitespace and comments, handle SLASH */
 
  foundit = 0;
  while (!foundit)
    {
      while (iswhitespace(c = getc(saolfile)))  
	if (c == '\n') 
	  yylval->linenum = ++saollinenumber;
      if (c == EOF)
	{
	  if (!getnextsaolfile())
	    return 0;
	  else
	    continue;
	}
      if (cppsaol && preprocesslinenum(c))
	continue;
      if (c != '/')
	foundit = 1;
      else
	{
	  c = getc(saolfile);
	  if (c != '/')
	    {
	      ungetc(c,saolfile);
	      buf[0]='/'; buf[1]='\0';
	      yylval->val =  dupval(buf);
	      yylval->ttype = lexttl = S_SLASH;
	      return SLASH;
	    }
	  else
	    {
	      while ((c != '\n'))
		{
		  c = getc(saolfile);
		  if ((c == EOF) && (!getnextsaolfile()))
		    return 0;
		}
	      yylval->linenum = ++saollinenumber;
	    }
	}
    }

  /* string constant */

  if (c == '"')
    {
      strstart = saollinenumber;
      i=0; c = getc(saolfile);
      while (c != EOF)
	{
	  if (c == '\n') 
	    yylval->linenum = ++saollinenumber;
	  if (c != '"')
	    {
	      if (i < LEXBUFSIZE)
		buf[i++]=(char)c; 
	      c = getc(saolfile);
	    }
	  else
	    {
	      if ((i!=0)&&buf[i-1]=='\\')
		{
		  buf[i-1]= '"';
		  c = getc(saolfile);
		}
	      else
		{
		  if (i >= LEXBUFSIZE)
		    {	     
		      printf("Error: Sfront string const size limit (%i chars)" 
			     " exceeded.\n", LEXBUFSIZE - 1);
		      showerrorplace(saollinenumber, saolsourcefile);
		    }
		  buf[i]='\0';
		  yylval->val = dupval(buf);
		  yylval->ttype = lexttl = S_STRCONST;
		  return STRCONST;
		}
	    }
	}
      printf("Error: Unterminated string constant in SAOL file.\n\n");
      showerrorplace(strstart, saolsourcefile);
    }

  /* parse integers and numbers */

  if (isdigit(c)||(c == '.'))    
    {
      nonzero = 0;
      i=0; 
      if (c == '.')
	{
	  buf[0]= '0'; ; i=1;
	  c = getc(saolfile);
	  if (!isdigit(c))
	    {
	      ungetc(c,saolfile);
	      buf[1]='\0';
	      yylval->val =  dupval(buf);
	      yylval->ttype = lexttl = S_BADNUMBER;
	      return BADNUMBER;
	    }
	  else
	    {
	      ungetc(c,saolfile);
	      c = '.';
	    }
	}
      else
	{
	  while (isdigit(c))
	    {
	      if (i < LEXBUFSIZE)
		buf[i++]=(char)c; 
	      c = getc(saolfile);
	    }
	  if ((c != 'e') && (c != '.'))      /* an integer */
	    {
	      ungetc(c,saolfile);
	      yylval->rate = IRATETYPE;
	      yylval->vol = CONSTANT;

	      if ((i < 10) || ((i == 10) && (strcmp(buf,"4294967295") <= 0)))
		{
		  buf[i]='\0';
		  yylval->val = dupval(buf);
		  yylval->ttype = lexttl = S_INTGR;
		  yylval->res = ASINT;
		  return INTGR;
		}
	      else
		{
		  if (i + 2 >= LEXBUFSIZE)
		    {		      
		      printf("Error: Sfront number const size limit " 
			     "(%i digits) exceeded.\n", LEXBUFSIZE - 1);
		      showerrorplace(saollinenumber, saolsourcefile);
		    }
		  buf[i++] = '.';
		  buf[i++] = '0';
		  buf[i] = '\0';
		  yylval->val = dupval(buf);
		  yylval->ttype = lexttl = S_NUMBER;
		  return NUMBER;
		}
	    }
	}
      if (c == '.')
	{
	  if (i < LEXBUFSIZE)
	    buf[i++]=(char)c; 
	  c = getc(saolfile);
	  if (!isdigit(c))
	    {
	      if (i < LEXBUFSIZE)
		buf[i++]='0'; 
	    }
	  else
	    {
	      while (isdigit(c))
		{
		  if (i < LEXBUFSIZE)
		    buf[i++]=(char)c;
		  nonzero = ((c != '0') || nonzero);
		  c = getc(saolfile);
		}
	    }
	}
      if (c != 'e')
	{
	  if (i >= LEXBUFSIZE)
	    {	     
	      printf("Error: Sfront number const size limit (%i chars)" 
		     " exceeded.\n", LEXBUFSIZE - 1);
	      showerrorplace(saollinenumber, saolsourcefile);
	    }

	  ungetc(c,saolfile);
	  if (nonzero)
	    { 
	      buf[i]='\0';
	      yylval->val =  dupval(buf);
	      yylval->ttype = lexttl = S_NUMBER;
	      yylval->rate =  IRATETYPE;
	      yylval->vol = CONSTANT;
	      return NUMBER;
	    }
	  else
	    {
	      while ((buf[i] != '.') && (i != 0))
		i--;
	      buf[i] = '\0';

	      yylval->rate =  IRATETYPE;
	      yylval->vol = CONSTANT;

	      if (i == 0)
		{
		  yylval->val = dupval(buf);
		  yylval->ttype = lexttl = S_BADNUMBER;
		  return BADNUMBER;
		}

	      if ((i < 10) || ((i == 10) && (strcmp(buf,"4294967295") <= 0)))
		{
		  yylval->val = dupval(buf);
		  yylval->ttype = lexttl = S_INTGR;
		  yylval->res = ASINT;
		  return INTGR;
		}
	      else
		{		
		  if (i + 2 >= LEXBUFSIZE)
		    {		      
		      printf("Error: Sfront number const size limit " 
			     "(%i digits) exceeded.\n", LEXBUFSIZE - 1);
		      showerrorplace(saollinenumber, saolsourcefile);
		    }
		  buf[i++] = '.';
		  buf[i++] = '0';
		  buf[i] = '\0';
		  yylval->val = dupval(buf);
		  yylval->ttype = lexttl = S_NUMBER;
		  return NUMBER;
		}
	    }
	}
      else
	{	
	  if (i < LEXBUFSIZE)
	    buf[i++]=(char)c; 
	  c = getc(saolfile);
	  if ((c=='+')||(c=='-'))
	    {
	      if (i < LEXBUFSIZE)
		buf[i++]=(char)c; 
	      c = getc(saolfile);
	    }
	  if (isdigit(c))
	    {
	      while (isdigit(c))
		{
		  if (i < LEXBUFSIZE)
		    buf[i++]=(char)c;
		  c = getc(saolfile);
		}
	      if (i >= LEXBUFSIZE)
		{	     
		  printf("Error: Sfront number const size limit (%i chars)" 
			 " exceeded.\n", LEXBUFSIZE - 1);
		  showerrorplace(saollinenumber, saolsourcefile);
		}
	      ungetc(c,saolfile);
	      buf[i]='\0';
	      yylval->val =  dupval(buf);
	      yylval->ttype = lexttl = S_NUMBER;
	      yylval->rate =  IRATETYPE;
	      yylval->vol = CONSTANT;
	      return NUMBER;
	    }
	  else
	    {
	      if (i >= LEXBUFSIZE)
		{	     
		  printf("Error: Sfront number const size limit (%i chars)" 
			 " exceeded.\n", LEXBUFSIZE - 1);
		  showerrorplace(saollinenumber, saolsourcefile);
		}
	      ungetc(c,saolfile);
	      buf[i]='\0';
	      yylval->val =  dupval(buf);
	      yylval->ttype = lexttl = S_BADNUMBER;
	      return BADNUMBER;
	    }
	}
    }

  if ((isalpha(c))||(c == '_'))    /* keywords and identifiers */
    {

      i = 0;
      while (((isalnum(c)) || (c == '_')))
	{
	  if (i < LEXBUFSIZE - 1)   /* only first 16 chars matter in spec */
	    buf[i++] = (char)c;
	  c = getc(saolfile);
	}
      buf[i] = '\0';
      ungetc(c,saolfile);
      yylval->val = dupval(buf);
      if (strstr(buf,"_sym_") != NULL)
	{
	  printf("Error: Identifiers beginning with _sym_ not permitted.\n");
	  showerrorplace(yylval->linenum, yylval->filename);
	}
      switch (buf[0]) {

      case 'a':
	if (!strcmp(buf,"aopcode"))
	  { 
	    yylval->ttype = lexttl = S_AOPCODE;
	    yylval->rate = ARATETYPE;
	    return AOPCODE;
	  }
	if (!strcmp(buf,"asig"))
	  { 
	    yylval->rate = ARATETYPE;
	    yylval->ttype = lexttl = S_ASIG;
	    return ASIG;
	  }
	break;

      case 'e':
	if (!strcmp(buf,"else"))
	  { 
	    yylval->ttype = lexttl = S_ELSE;
	    return ELSE;
	  }
	if (!strcmp(buf,"exports"))
	  { 
	    yylval->ttype = lexttl = S_EXPORTS;
	    return EXPORTS;
	  }
	if (!strcmp(buf,"extend"))
	  { 
	    yylval->ttype = lexttl = S_EXTEND;
	    return EXTEND;
	  }
	break;

      case 'g':
	if (!strcmp(buf,"global"))
	  { 
	    yylval->ttype = lexttl = S_GLOBAL;
	    return GLOBAL;
	  }
	break;

      case 'i':
	if (!strcmp(buf,"if"))
	  { 
	    yylval->ttype = lexttl = S_IF;
	    return IF;
	  }
	if (!strcmp(buf,"imports"))
	  { 
	    yylval->ttype = lexttl = S_IMPORTS;
	    return IMPORTS;
	  }
	if (!strcmp(buf,"inchannels"))
	  { 
	    yylval->res = ASINT;
	    yylval->ttype = lexttl = S_INCHANNELS;
	    return INCHANNELS;
	  }
	if (!strcmp(buf,"instr"))
	  { 
	    yylval->ttype = lexttl = S_INSTR;
	    return INSTR;
	  }
	if (!strcmp(buf,"interp"))
	  { 
	    yylval->res = ASINT;
	    yylval->ttype = lexttl = S_INTERP;
	    return INTERP;
	  }
	if (!strcmp(buf,"iopcode"))
	  { 
	    yylval->ttype = lexttl = S_IOPCODE;
	    yylval->rate = IRATETYPE;
	    return IOPCODE;
	  }
	if (!strcmp(buf,"ivar"))
	  { 
	    yylval->rate = IRATETYPE;
	    yylval->ttype = lexttl = S_IVAR;
	    return IVAR;
	  }
	break;

      case 'k':
	if (!strcmp(buf,"kopcode"))
	  { 
	    yylval->ttype = lexttl = S_KOPCODE;
	    yylval->rate = KRATETYPE;
	    return KOPCODE;
	  }
	if (!strcmp(buf,"krate"))
	  { 
	    yylval->ttype = lexttl = S_KRATE;
	    return KRATE;
	  }
	if (!strcmp(buf,"ksig"))
	  { 
	    yylval->rate = KRATETYPE;
	    yylval->ttype = lexttl = S_KSIG;
	    return KSIG;
	  }
	break;

      case 'm':
	if (!strcmp(buf,"map"))
	  { 
	    yylval->ttype = lexttl = S_MAP;
	    return MAP;
	  }
	break;

      case 'o':
	if (!strcmp(buf,"oparray"))
	  { 
	    yylval->ttype = lexttl = S_OPARRAY;
	    return OPARRAY;
	  }
	if (!strcmp(buf,"opcode"))
	  { 
	    yylval->ttype = lexttl = S_OPCODE;
	    yylval->rate = XRATETYPE;
	    return OPCODE;
	  }
	if (!strcmp(buf,"outbus"))
	  { 
	    yylval->ttype = lexttl = S_OUTBUS;
	    return OUTBUS;
	  }
	if (!strcmp(buf,"outchannels"))
	  { 
	    yylval->res = ASINT;
	    yylval->ttype = lexttl = S_OUTCHANNELS;
	    return OUTCHANNELS;
	  }
	if (!strcmp(buf,"output"))
	  { 
	    yylval->ttype = lexttl = S_OUTPUT;
	    return OUTPUT;
	  }
	break;

      case 'p':
	if (!strcmp(buf,"preset"))
	  { 
	    if (lexstatemachine == TEMPLATE_ACTIVE)
              lexstatemachine = TEMPLATE_PRESET;
	    yylval->ttype = lexttl = S_IDENT;
	    return IDENT;
	  }	
	if (!strcmp(buf,"printf"))
	  { 
	    if (!isocompliant)
	      {
		yylval->ttype = lexttl = S_PRINTF;
		return PRINTF;
	      }
	  }
	break;

      case 'r':
	if (!strcmp(buf,"return"))
	  { 
	    yylval->ttype = lexttl = S_RETURN;
	    return RETURN;
	  }
	if (!strcmp(buf,"route"))
	  { 
	    yylval->ttype = lexttl = S_ROUTE;
	    return ROUTE;
	  }
	break;

      case 's':
	if (!strcmp(buf,"send"))
	  { 
	    yylval->ttype = lexttl = S_SEND;
	    sendsemicoloncount = 3;
	    return SEND;
	  }
	if (!strcmp(buf,"sequence"))
	  { 
	    yylval->ttype = lexttl = S_SEQUENCE;
	    return SEQUENCE;
	  }
	if (!strcmp(buf,"sasbf"))
	  { 
	    yylval->ttype = lexttl = S_SASBF;
	    return SASBF;
	  }
	if (!strcmp(buf,"spatialize"))
	  { 
	    yylval->ttype = lexttl = S_SPATIALIZE;
	    return SPATIALIZE;
	  }
	if (!strcmp(buf,"srate"))
	  { 
	    yylval->ttype = lexttl = S_SRATE;
	    return SRATE;
	  }
	break;

      case 't':
	if (!strcmp(buf,"table"))
	  { 
	    yylval->ttype = lexttl = S_TABLE;
	    yylval->rate = IRATETYPE;
	    return TABLE;
	  }
	if (!strcmp(buf,"tablemap"))
	  { 
	    yylval->ttype = lexttl = S_TABLEMAP;
	    yylval->rate = IRATETYPE;
	    return TABLEMAP;
	  }
	if (!strcmp(buf,"template"))
	  { 
	    yylval->ttype = lexttl = S_TEMPLATE;
	    if (lexstatemachine == TEMPLATE_REST)
	      lexstatemachine = TEMPLATE_ACTIVE;
	    return TEMPLATE;
	  }
	if (!strcmp(buf,"turnoff"))
	  { 
	    yylval->ttype = lexttl = S_TURNOFF;
	    return TURNOFF;
	  }
	break;

      case 'w':
	if (!strcmp(buf,"while"))
	  { 
	    yylval->ttype = lexttl = S_WHILE;
	    return WHILE;
	  }
	if (!strcmp(buf,"with"))
	  { 
	    yylval->ttype = lexttl = S_WITH;
	    if (lexstatemachine == TEMPLATE_ACTIVE)
	      lexstatemachine = TEMPLATE_WITH;
	    return WITH;
	  }
	break;

      case 'x':
	if (!strcmp(buf,"xsig"))
	  { 
	    yylval->ttype = lexttl = S_XSIG;
	    yylval->rate = XRATETYPE;
	    return XSIG;
	  }
	break;
      }

      if (cin || session)
	if (addvsym(&mpegtokens, buf, S_IDENT) == INSTALLED)
	  mpegtokens->width = mpegtokencount++;

      yylval->ttype = lexttl = S_IDENT;
      return IDENT;
    }


  buf[0]=(char)c;
  buf[1]='\0';
  yylval->val = dupval(buf);
  yylval->ttype = lexttl = c;

  switch (c) {

  case EOF:
    internalerror("lex.c", "c == EOF happened in switch statement");
    break;
  case '>':
    c = getc(saolfile);
    if (c != '=')
      {
	ungetc(c,saolfile);
	yylval->ttype = lexttl = S_GT;
	if (lexstatemachine == TEMPLATE_WMAPLIST)
	  return lexstate_wmap();
	if (lexstatemachine == TEMPLATE_PMAPLIST)
	  return lexstate_pmap();
	return GT;
      }
    else
      {
	buf[1] = '=';
	buf[2] = '\0';
	yylval->val = dupval(buf);
	yylval->ttype = lexttl = S_GEQ;
	return GEQ;
      }
  case '<':
    c = getc(saolfile);
    if (c != '=')
      {
	ungetc(c,saolfile);
	yylval->ttype = lexttl = S_LT;
	if (lexstatemachine == TEMPLATE_WITH)
	  {
	    lexstatemachine = TEMPLATE_WMAPLIST;
	    return LTT; 
	  }
	if (lexstatemachine == TEMPLATE_PRESET)
	  {
	    lexstatemachine = TEMPLATE_PMAPLIST;
	    return LTT; 
	  }
	return LT;     
      }
    else
      {
	buf[1] = '=';
	buf[2] = '\0';
	yylval->val = dupval(buf);
	yylval->ttype = lexttl = S_LEQ;
	return LEQ;
      }
  case '=':
    c = getc(saolfile);
    if (c != '=')
      {
	ungetc(c,saolfile);
	yylval->ttype = lexttl = S_EQ;
	return EQ;
      }
    else
      {
	buf[1] = '=';
	buf[2] = '\0';
	yylval->val = dupval(buf);
	yylval->ttype = lexttl = S_EQEQ;
	return EQEQ;
      }
  case '!':
    c = getc(saolfile);
    if (c != '=')
      {
	ungetc(c,saolfile);
	yylval->ttype = lexttl = S_NOT;
	return NOT;
      }
    else
      {
	buf[1] = '=';
	buf[2] = '\0';
	yylval->val = dupval(buf);
	yylval->ttype = lexttl = S_NEQ;
	return NEQ;
      }
  case '&':
    c = getc(saolfile);
    if (c != '&')
      {
	ungetc(c,saolfile);
	yylval->ttype = lexttl = S_BADCHAR;
	return BADCHAR;
      }
    else
      {
	buf[1] = '&';
	buf[2] = '\0';
	yylval->val = dupval(buf);
	yylval->ttype = lexttl = S_AND;
	return AND;
      }
  case '|':
    c = getc(saolfile);
    if (c != '|')
      {
	ungetc(c,saolfile);
	yylval->ttype = lexttl = S_BADCHAR;
	return BADCHAR;
      }
    else
      {
	buf[1] = '|';
	buf[2] = '\0';
	yylval->val = dupval(buf);
	yylval->ttype = lexttl = S_OR;
	return OR;
      }
  case '-':
    yylval->ttype = lexttl = S_MINUS;
    return MINUS;
   case '*':
    yylval->ttype = lexttl = S_STAR;
    return STAR;
   case '/':    /* should never run -- see/update whitespace code above */
    yylval->ttype = lexttl = S_SLASH;
    return SLASH;
   case '+':
    yylval->ttype = lexttl = S_PLUS;
    return PLUS;
   case '?':
    yylval->ttype = lexttl = S_Q;
    return Q;
   case ':':
    yylval->ttype = lexttl = S_COL;
    return COL;
   case '(':
    yylval->ttype = lexttl = S_LP;
    return LP;
   case ')':
    yylval->ttype = lexttl = S_RP;
    return RP;
   case '{':
    yylval->ttype = lexttl = S_LC;
    return LC;
   case '}':
    yylval->ttype = lexttl = S_RC;
    return RC;
   case '[':
    yylval->ttype = lexttl = S_LB;
    return LB;
   case ']':
    yylval->ttype = lexttl = S_RB;
    return RB;
   case ';':
    yylval->ttype = lexttl = S_SEM;
    return SEM;
   case ',':
    yylval->ttype = lexttl = S_COM;
    return COM;
   default:
    yylval->ttype = lexttl = S_BADCHAR;
    return BADCHAR;
   }

  return BADCHAR; /* will never execute */
}


