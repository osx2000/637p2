/* A Bison parser, made by GNU Bison 2.3.  */

/* Skeleton interface for Bison's Yacc-like parsers in C

   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     STRCONST = 258,
     IDENT = 259,
     INTGR = 260,
     NUMBER = 261,
     AOPCODE = 262,
     ASIG = 263,
     ELSE = 264,
     EXPORTS = 265,
     EXTEND = 266,
     GLOBAL = 267,
     IF = 268,
     IMPORTS = 269,
     INCHANNELS = 270,
     INSTR = 271,
     INTERP = 272,
     IOPCODE = 273,
     IVAR = 274,
     KOPCODE = 275,
     KRATE = 276,
     KSIG = 277,
     MAP = 278,
     OPARRAY = 279,
     OPCODE = 280,
     OUTBUS = 281,
     OUTCHANNELS = 282,
     OUTPUT = 283,
     PRINTF = 284,
     RETURN = 285,
     ROUTE = 286,
     SASBF = 287,
     SEND = 288,
     SEQUENCE = 289,
     SPATIALIZE = 290,
     SRATE = 291,
     TABLE = 292,
     TABLEMAP = 293,
     TEMPLATE = 294,
     TURNOFF = 295,
     WHILE = 296,
     WITH = 297,
     XSIG = 298,
     AND = 299,
     OR = 300,
     GEQ = 301,
     LEQ = 302,
     NEQ = 303,
     EQEQ = 304,
     MINUS = 305,
     STAR = 306,
     SLASH = 307,
     PLUS = 308,
     GT = 309,
     LT = 310,
     Q = 311,
     COL = 312,
     LP = 313,
     RP = 314,
     LC = 315,
     RC = 316,
     LB = 317,
     RB = 318,
     SEM = 319,
     COM = 320,
     EQ = 321,
     NOT = 322,
     BADCHAR = 323,
     BADNUMBER = 324,
     LTT = 325,
     GTT = 326,
     UMINUS = 327,
     UNOT = 328,
     HIGHEST = 329
   };
#endif
/* Tokens.  */
#define STRCONST 258
#define IDENT 259
#define INTGR 260
#define NUMBER 261
#define AOPCODE 262
#define ASIG 263
#define ELSE 264
#define EXPORTS 265
#define EXTEND 266
#define GLOBAL 267
#define IF 268
#define IMPORTS 269
#define INCHANNELS 270
#define INSTR 271
#define INTERP 272
#define IOPCODE 273
#define IVAR 274
#define KOPCODE 275
#define KRATE 276
#define KSIG 277
#define MAP 278
#define OPARRAY 279
#define OPCODE 280
#define OUTBUS 281
#define OUTCHANNELS 282
#define OUTPUT 283
#define PRINTF 284
#define RETURN 285
#define ROUTE 286
#define SASBF 287
#define SEND 288
#define SEQUENCE 289
#define SPATIALIZE 290
#define SRATE 291
#define TABLE 292
#define TABLEMAP 293
#define TEMPLATE 294
#define TURNOFF 295
#define WHILE 296
#define WITH 297
#define XSIG 298
#define AND 299
#define OR 300
#define GEQ 301
#define LEQ 302
#define NEQ 303
#define EQEQ 304
#define MINUS 305
#define STAR 306
#define SLASH 307
#define PLUS 308
#define GT 309
#define LT 310
#define Q 311
#define COL 312
#define LP 313
#define RP 314
#define LC 315
#define RC 316
#define LB 317
#define RB 318
#define SEM 319
#define COM 320
#define EQ 321
#define NOT 322
#define BADCHAR 323
#define BADNUMBER 324
#define LTT 325
#define GTT 326
#define UMINUS 327
#define UNOT 328
#define HIGHEST 329




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef int YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE yylval;

