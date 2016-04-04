
/*
#    Sfront, a SAOL to C translator    
#    This file: Grammar for Bison
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



%{

#include "tree.h"

%}

%expect 1

%token STRCONST
%token IDENT
%token INTGR
%token NUMBER
%token AOPCODE
%token ASIG
%token ELSE
%token EXPORTS
%token EXTEND
%token GLOBAL
%token IF
%token IMPORTS
%token INCHANNELS
%token INSTR
%token INTERP
%token IOPCODE
%token IVAR
%token KOPCODE
%token KRATE
%token KSIG
%token MAP
%token OPARRAY
%token OPCODE
%token OUTBUS
%token OUTCHANNELS
%token OUTPUT
%token PRINTF
%token RETURN
%token ROUTE
%token SASBF
%token SEND
%token SEQUENCE
%token SPATIALIZE
%token SRATE
%token TABLE
%token TABLEMAP
%token TEMPLATE
%token TURNOFF
%token WHILE
%token WITH
%token XSIG
%token AND
%token OR
%token GEQ
%token LEQ
%token NEQ
%token EQEQ
%token MINUS
%token STAR
%token SLASH
%token PLUS
%token GT
%token LT
%token Q
%token COL
%token LP
%token RP
%token LC
%token RC
%token LB
%token RB
%token SEM
%token COM
%token EQ
%token NOT
%token BADCHAR
%token BADNUMBER
%token LTT
%token GTT

%start orcfile
%right Q
%left  OR
%left  AND 
%left  EQEQ NEQ
%left  LT GT LEQ GEQ
%left  PLUS MINUS
%left  STAR SLASH
%right UNOT UMINUS
%token HIGHEST


%% /* grammer rules and actions follow */

orcfile         : proclist { troot = $1;}
                ;

proclist        : proclist instrdecl     {$$ = leftrecurse($1,$2);}
                | proclist opcodedecl    {$$ = leftrecurse($1,$2);}
                | proclist globaldecl    {$$ = leftrecurse($1,$2);}
                | proclist templatedecl  {$$ = leftrecurse($1,$2);}
                | /* null */             {$$ = NULL; }
                ;

instrdecl       : INSTR IDENT LP identlist {make_instrpfields($4);} 
                  RP miditag LC vardecls block RC
                  {$$ = make_instrdecl($1,$2,$3,$4,$6,$7,$8,$9,$10,$11);}
                ;

miditag         : IDENT int_list  {$$ = make_miditag($1,$2);} 
                | /* null */ {$$ = NULL;}
                ;

int_list        : int_list INTGR   {$$ = leftrecurse($1,$2);}
                | INTGR
                ;


opcodedecl      : optype IDENT {make_opcodetype($1,$2);} 
                  LP paramlist RP LC opvardecls block RC
                 {$$ = make_opcodedecl($1,$2,$4,$5,$6,$7,$8,$9,$10);}
                ;

globaldecl      : GLOBAL {suspendvarchecks = 1;} LC globalblock RC 
                  {$$=make_globaldecl($1,$3,$4,$5);}
                ;

templatedecl    : TEMPLATE LT identlist GT LP identlist RP 
                  {make_templatepfields(NULL,$6);} 
                  MAP LC identlist RC
                  WITH LC mapblock RC
                  {templateopcodepatch();}
                  LC vardecls block RC
                  {$$=make_templatedecl($3,NULL,$6,$11,$15,$19,$20);}
                | TEMPLATE LT identlist GT 
		  IDENT {suspendvarchecks = 1;} mapblock LP identlist RP
                  {make_templatepfields($5, $9);}
                  MAP LC identlist RC
                  WITH LC mapblock RC
                  {templateopcodepatch();}
                  LC vardecls block RC
                  {$$=make_templatedecl($3,$7,$9,$14,$18,$22,$23);}
                ;


mapblock        : mapblock COM LTT exprlist GTT {$$=make_mapblock($1,$4);}
                | LTT exprlist GTT              {$$=make_mapblock(NULL,$2);}
                | /* null */ { $$ = NULL; }
                ;

globalblock     : globalblock globaldef {$$ = leftrecurse($1,$2);}
                | /* null */            { $$ = NULL; }
                ;

globaldef       : rtparam
                | vardecl
                | routedef
                | senddef
                | seqdef
                ;

rtparam         : SRATE INTGR SEM        {$$ = make_rtparam($1,$2,$3);} 
                | KRATE INTGR SEM        {$$ = make_rtparam($1,$2,$3);} 
                | INCHANNELS INTGR SEM   {$$ = make_rtparam($1,$2,$3);} 
                | OUTCHANNELS INTGR SEM  {$$ = make_rtparam($1,$2,$3);}
                | INTERP INTGR SEM       {$$ = make_rtparam($1,$2,$3);}
                ;

routedef        : ROUTE LP IDENT COM identlist RP SEM
                  {$$ = make_routedef($1,$2,$3,$4,$5,$6,$7);}
                ;

senddef         : SEND LP IDENT SEM exprlist SEM namelist RP SEM
                  {$$ = make_senddef($1,$2,$3,$4,$5,$6,$7,$8,$9);}
                ;

seqdef          : SEQUENCE LP identlist RP SEM
                  {$$ = make_seqdef($1,$2,$3,$4,$5);}
                ;

block           : block statement     {$$ = leftrecurse($1,$2);}
                | /* null */          {$$ = NULL; }
                ;

statement       : lvalue EQ expr SEM
                  {$$ = make_statement($1,$2,$3,$4,NULL,NULL,
                                       NULL,NULL,NULL,NULL,NULL);}
                | expr SEM
                  {$$ = make_statement($1,$2,NULL,NULL,NULL,NULL,
                                       NULL,NULL,NULL,NULL,NULL);}
                | IF LP expr RP LC block RC
                  {$$ = make_statement($1,$2,$3,$4,$5,$6,
                                       $7,NULL,NULL,NULL,NULL);}
                | IF LP expr RP LC block RC ELSE LC block RC
                  {$$ = make_statement($1,$2,$3,$4,$5,$6,
                                       $7,$8,$9,$10,$11);}
                | WHILE LP expr RP LC block RC
                  {$$ = make_statement($1,$2,$3,$4,$5,$6,
                                       $7,NULL,NULL,NULL,NULL);}
                | INSTR IDENT LP exprlist RP SEM
                  {$$ = make_statement($1,$2,$3,$4,$5,$6,
                                       NULL,NULL,NULL,NULL,NULL);}
                | OUTPUT LP exprlist RP SEM
                  {$$ = make_statement($1,$2,$3,$4,$5,NULL,
                                       NULL,NULL,NULL,NULL,NULL);}
                | SPATIALIZE LP exprlist RP SEM
                  {$$ = make_statement($1,$2,$3,$4,$5,NULL,
                                       NULL,NULL,NULL,NULL,NULL);}
                | OUTBUS LP IDENT COM exprlist RP SEM
                  {$$ = make_statement($1,$2,$3,$4,$5,$6,
                                       $7,NULL,NULL,NULL,NULL);}
                | EXTEND LP expr RP SEM
                  {$$ = make_statement($1,$2,$3,$4,$5,NULL,
                                       NULL,NULL,NULL,NULL,NULL);}
                | TURNOFF SEM
                  {$$ = make_statement($1,$2,NULL,NULL,NULL,NULL,
                                       NULL,NULL,NULL,NULL,NULL);}
                | RETURN LP exprlist RP SEM
                  {$$ = make_statement($1,$2,$3,$4,$5,NULL,
                                       NULL,NULL,NULL,NULL,NULL);}
                | PRINTF LP exprstrlist RP SEM
                  {$$ = make_statement($1,$2,$3,$4,$5,NULL,
                                       NULL,NULL,NULL,NULL,NULL);}
                ;

lvalue          : IDENT               {$$ = make_lval($1,NULL,NULL,NULL);}
                | IDENT LB expr RB    {$$ = make_lval($1,$2,$3,$4);}
                ;

identlist       : identlist COM IDENT {$$ = leftsrecurse($1,$2,$3);}
                | IDENT
                | /* null */          {$$ = NULL;}
                ;

paramlist       : paramlist COM paramdecl  {$$ = leftsrecurse($1,$2,$3);}
                | paramdecl
                | /* null */               { $$ = NULL; }
                ;

vardecls        : vardecls vardecl    {$$ = leftrecurse($1,$2);}
                | /* null */          {$$ = NULL;}
                ;

vardecl         : taglist stype namelist SEM 
                  {$$=make_simplevar($1,$2,$3,$4, "<vardecl>" ,S_VARDECL);}
                | stype namelist SEM 
                  {$$=make_simplevar(NULL,$1,$2,$3, "<vardecl>", S_VARDECL);}
                | tabledecl SEM    
                  {$$ = $1;}          /* SEM part of tabledecl */
                | TABLEMAP IDENT LP identlist RP SEM
                  {$$=make_tablemap($1,$2,$3,$4,$5,$6);}
                ;

opvardecls      : opvardecls opvardecl {$$ = leftrecurse($1,$2);}
                | /* null */           {$$ = NULL; }
                ;

opvardecl       : taglist otype namelist SEM
                {$$=make_simplevar($1,$2,$3,$4, "<opvardecl>" ,S_OPVARDECL);}
                | otype namelist SEM
                {$$=make_simplevar(NULL,$1,$2,$3, "<opvardecl>", S_OPVARDECL);}
                | tabledecl SEM
                {$$ = $1;}  /* SEM part of tabledecl */
                | TABLEMAP IDENT LP identlist RP SEM
                  {$$=make_tablemap($1,$2,$3,$4,$5,$6);}
                ;

paramdecl       : otype name              {$$ = make_paramdecl($1,$2);}   
                ;

namelist        : namelist COM name       {$$ = leftsrecurse($1,$2,$3);}
                | name
                ;

name            : IDENT                   {$$ = make_name($1,NULL,NULL,NULL);}
                | IDENT LB INTGR RB       {$$ = make_name($1,$2,$3,$4);}
                | IDENT LB INCHANNELS RB  {$$ = make_name($1,$2,$3,$4);}
		| IDENT LB OUTCHANNELS RB {$$ = make_name($1,$2,$3,$4);}
                ;

stype           : IVAR
                | KSIG 
                | ASIG
                | TABLE
                | OPARRAY
                ;

otype           : XSIG
                | stype
                ;


tabledecl       : TABLE IDENT LP IDENT COM exprstrlist RP
                  {$$ = make_tabledecl($1,$2,$3,$4,$5,$6,$7);}
                ;

taglist         : IMPORTS  
                  {$$=make_stree($1,NULL,NULL,NULL,"<taglist>",S_TAGLIST);}
                | EXPORTS           
                  {$$=make_stree($1,NULL,NULL,NULL,"<taglist>",S_TAGLIST);}
                | IMPORTS EXPORTS
                  {$$=make_stree($1,$2,NULL,NULL,"<taglist>",S_TAGLIST);}
                | EXPORTS IMPORTS
                  {$$=make_stree($1,$2,NULL,NULL,"<taglist>",S_TAGLIST);}
                ;

optype          : AOPCODE
                | KOPCODE
                | IOPCODE
                | OPCODE
                ;

expr            : IDENT
                  {$$ = make_expr($1,NULL,NULL,NULL,NULL,NULL,NULL);}
                | const
                  {$$ = make_expr($1,NULL,NULL,NULL,NULL,NULL,NULL);}
                | IDENT LB expr RB
                  {$$ = make_expr($1,$2,$3,$4,NULL,NULL,NULL);}
                | SASBF LP exprlist RP
                  {$$ = make_expr($1,$2,$3,$4,NULL,NULL,NULL);}
                | IDENT LP exprlist RP
                  {$$ = make_expr($1,$2,$3,$4,NULL,NULL,NULL);}
                | IDENT LB expr RB LP exprlist RP
                  {$$ = make_expr($1,$2,$3,$4,$5,$6,$7);}
                | expr Q expr COL expr %prec Q
                  {$$ = make_expr($1,$2,$3,$4,$5,NULL,NULL);}
                | expr LEQ expr
                  {$$ = make_expr($1,$2,$3,NULL,NULL,NULL,NULL);}
                | expr GEQ expr
                  {$$ = make_expr($1,$2,$3,NULL,NULL,NULL,NULL);}
                | expr NEQ expr
                  {$$ = make_expr($1,$2,$3,NULL,NULL,NULL,NULL);}
                | expr EQEQ expr
                  {$$ = make_expr($1,$2,$3,NULL,NULL,NULL,NULL);}
                | expr GT expr
                  {$$ = make_expr($1,$2,$3,NULL,NULL,NULL,NULL);}
                | expr LT expr
                  {$$ = make_expr($1,$2,$3,NULL,NULL,NULL,NULL);}
                | expr AND expr
                  {$$ = make_expr($1,$2,$3,NULL,NULL,NULL,NULL);}
                | expr OR expr
                  {$$ = make_expr($1,$2,$3,NULL,NULL,NULL,NULL);}
                | expr PLUS expr
                  {$$ = make_expr($1,$2,$3,NULL,NULL,NULL,NULL);}
                | expr MINUS expr
                  {$$ = make_expr($1,$2,$3,NULL,NULL,NULL,NULL);}
                | expr STAR expr
                  {$$ = make_expr($1,$2,$3,NULL,NULL,NULL,NULL);}
                | expr SLASH expr
                  {$$ = make_expr($1,$2,$3,NULL,NULL,NULL,NULL);}
                | NOT expr %prec UNOT
                  {$$ = make_expr($1,$2,NULL,NULL,NULL,NULL,NULL);}
                | MINUS expr %prec UMINUS
                  {$$ = make_expr($1,$2,NULL,NULL,NULL,NULL,NULL);}
                | LP expr RP
                  {$$ = make_expr($1,$2,$3,NULL,NULL,NULL,NULL);}
                ;

exprlist        : exprlist COM expr      {$$ = leftsrecurse($1,$2,$3);}
                | expr
                | /* null */             { $$ = NULL; }
                ;

exprstrlist     : exprstrlist COM expr     {$$ = leftsrecurse($1,$2,$3);}
                | exprstrlist COM STRCONST {$$ = leftsrecurse($1,$2,$3);}
                | STRCONST
                | expr 
                ;

const           : INTGR
                | NUMBER
                ;


%%         


