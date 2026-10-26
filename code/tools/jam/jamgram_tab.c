
/*  A Bison parser, made from jamgram.y with Bison version GNU Bison version 1.24
  */

#define YYBISON 1  /* Identify Bison output.  */

#define	_BANG_t	258
#define	_BANG_EQUALS_t	259
#define	_AMPER_t	260
#define	_AMPERAMPER_t	261
#define	_LPAREN_t	262
#define	_RPAREN_t	263
#define	_PLUS_EQUALS_t	264
#define	_COLON_t	265
#define	_SEMIC_t	266
#define	_LANGLE_t	267
#define	_LANGLE_EQUALS_t	268
#define	_EQUALS_t	269
#define	_RANGLE_t	270
#define	_RANGLE_EQUALS_t	271
#define	_QUESTION_EQUALS_t	272
#define	_LBRACKET_t	273
#define	_RBRACKET_t	274
#define	ACTIONS_t	275
#define	BIND_t	276
#define	BREAK_t	277
#define	CASE_t	278
#define	CONTINUE_t	279
#define	DEFAULT_t	280
#define	ELSE_t	281
#define	EXISTING_t	282
#define	FOR_t	283
#define	IF_t	284
#define	IGNORE_t	285
#define	IN_t	286
#define	INCLUDE_t	287
#define	LOCAL_t	288
#define	MAXLINE_t	289
#define	ON_t	290
#define	PIECEMEAL_t	291
#define	QUIETLY_t	292
#define	VCERRORCONV_t	293
#define	RESPONSE_t	294
#define	RETURN_t	295
#define	RULE_t	296
#define	SWITCH_t	297
#define	TOGETHER_t	298
#define	UPDATED_t	299
#define	WHILE_t	300
#define	_LBRACE_t	301
#define	_BAR_t	302
#define	_BARBAR_t	303
#define	_RBRACE_t	304
#define	ARG	305
#define	STRING	306

#line 86 "jamgram.y"

#include "jam.h"

#include "lists.h"
#include "variable.h"
#include "parse.h"
#include "scan.h"
#include "compile.h"
#include "newstr.h"
#include "rules.h"

# define YYMAXDEPTH 10000	/* for OSF and other less endowed yaccs */

# define F0 (LIST *(*)(PARSE *, LOL *, int *))0
# define P0 (PARSE *)0
# define S0 (char *)0

# define pappend( l,r )    	parse_make( compile_append,l,r,P0,S0,S0,0 )
# define pbreak( l,f )     	parse_make( compile_break,l,P0,P0,S0,S0,f )
# define peval( c,l,r )		parse_make( compile_eval,l,r,P0,S0,S0,c )
# define pfor( s,l,r )    	parse_make( compile_foreach,l,r,P0,s,S0,0 )
# define pif( l,r,t )	  	parse_make( compile_if,l,r,t,S0,S0,0 )
# define pincl( l )       	parse_make( compile_include,l,P0,P0,S0,S0,0 )
# define plist( s )	  	parse_make( compile_list,P0,P0,P0,s,S0,0 )
# define plocal( l,r,t )  	parse_make( compile_local,l,r,t,S0,S0,0 )
# define pnull()	  	parse_make( compile_null,P0,P0,P0,S0,S0,0 )
# define pon( l,r )	  	parse_make( compile_on,l,r,P0,S0,S0,0 )
# define prule( a,p )     	parse_make( compile_rule,a,p,P0,S0,S0,0 )
# define prules( l,r )	  	parse_make( compile_rules,l,r,P0,S0,S0,0 )
# define pset( l,r,a ) 	  	parse_make( compile_set,l,r,P0,S0,S0,a )
# define pset1( l,r,t,a )	parse_make( compile_settings,l,r,t,S0,S0,a )
# define psetc( s,l,r )     	parse_make( compile_setcomp,l,r,P0,s,S0,0 )
# define psete( s,l,s1,f ) 	parse_make( compile_setexec,l,P0,P0,s,s1,f )
# define pswitch( l,r )   	parse_make( compile_switch,l,r,P0,S0,S0,0 )
# define pwhile( l,r )   	parse_make( compile_while,l,r,P0,S0,S0,0 )

# define pnode( l,r )    	parse_make( F0,l,r,P0,S0,S0,0 )
# define psnode( s,l )     	parse_make( F0,l,P0,P0,s,S0,0 )


#ifndef YYLTYPE
typedef
  struct yyltype
    {
      int timestamp;
      int first_line;
      int first_column;
      int last_line;
      int last_column;
      char *text;
   }
  yyltype;

#define YYLTYPE yyltype
#endif

#ifndef YYSTYPE
#define YYSTYPE int
#endif
#include <stdio.h>

#ifndef __cplusplus
#ifndef __STDC__
#define const
#endif
#endif



#define	YYFINAL		156
#define	YYFLAG		-32768
#define	YYNTBASE	52

#define YYTRANSLATE(x) ((unsigned)(x) <= 306 ? yytranslate[x] : 72)

static const char yytranslate[] = {     0,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     1,     2,     3,     4,     5,
     6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
    16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
    26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
    36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
    46,    47,    48,    49,    50,    51
};

#if YYDEBUG != 0
static const short yyprhs[] = {     0,
     0,     1,     3,     4,     6,     8,    11,    16,    23,    27,
    31,    35,    40,    47,    51,    55,    59,    67,    73,    79,
    87,    93,   100,   104,   105,   106,   116,   118,   120,   122,
   125,   127,   131,   135,   139,   143,   147,   151,   155,   159,
   163,   167,   171,   174,   178,   179,   182,   187,   188,   192,
   194,   196,   200,   202,   203,   206,   208,   209,   214,   217,
   222,   227,   228,   231,   233,   235,   237,   239,   241,   243,
   245,   248,   250,   251
};

static const short yyrhs[] = {    -1,
    54,     0,     0,    54,     0,    55,     0,    55,    54,     0,
    33,    64,    11,    53,     0,    33,    64,    14,    64,    11,
    53,     0,    46,    53,    49,     0,    32,    64,    11,     0,
    66,    63,    11,     0,    66,    58,    64,    11,     0,    66,
    35,    64,    58,    64,    11,     0,    22,    64,    11,     0,
    24,    64,    11,     0,    40,    64,    11,     0,    28,    50,
    31,    64,    46,    53,    49,     0,    42,    64,    46,    60,
    49,     0,    29,    59,    46,    53,    49,     0,    29,    59,
    46,    53,    49,    26,    55,     0,    45,    59,    46,    53,
    49,     0,    41,    50,    62,    46,    53,    49,     0,    35,
    66,    55,     0,     0,     0,    20,    69,    50,    71,    46,
    56,    51,    57,    49,     0,    14,     0,     9,     0,    17,
     0,    25,    14,     0,    66,     0,    59,    14,    59,     0,
    59,     4,    59,     0,    59,    12,    59,     0,    59,    13,
    59,     0,    59,    15,    59,     0,    59,    16,    59,     0,
    59,     5,    59,     0,    59,     6,    59,     0,    59,    47,
    59,     0,    59,    48,    59,     0,    66,    31,    64,     0,
     3,    59,     0,     7,    59,     8,     0,     0,    61,    60,
     0,    23,    50,    10,    53,     0,     0,    50,    10,    62,
     0,    50,     0,    64,     0,    64,    10,    63,     0,    65,
     0,     0,    65,    66,     0,    50,     0,     0,    18,    67,
    68,    19,     0,    66,    63,     0,    35,    66,    66,    63,
     0,    35,    66,    40,    64,     0,     0,    69,    70,     0,
    44,     0,    43,     0,    30,     0,    37,     0,    38,     0,
    36,     0,    27,     0,    34,    50,     0,    39,     0,     0,
    21,    64,     0
};

#endif

#if YYDEBUG != 0
static const short yyrline[] = { 0,
   129,   131,   142,   144,   148,   150,   152,   154,   158,   160,
   162,   164,   166,   168,   170,   172,   174,   176,   178,   180,
   182,   184,   186,   188,   191,   193,   200,   202,   204,   206,
   214,   216,   218,   220,   222,   224,   226,   228,   230,   232,
   234,   236,   238,   240,   250,   252,   256,   265,   267,   269,
   278,   280,   290,   294,   296,   300,   302,   302,   311,   313,
   315,   324,   326,   330,   332,   334,   336,   338,   340,   342,
   344,   346,   355,   357
};

static const char * const yytname[] = {   "$","error","$undefined.","_BANG_t",
"_BANG_EQUALS_t","_AMPER_t","_AMPERAMPER_t","_LPAREN_t","_RPAREN_t","_PLUS_EQUALS_t",
"_COLON_t","_SEMIC_t","_LANGLE_t","_LANGLE_EQUALS_t","_EQUALS_t","_RANGLE_t",
"_RANGLE_EQUALS_t","_QUESTION_EQUALS_t","_LBRACKET_t","_RBRACKET_t","ACTIONS_t",
"BIND_t","BREAK_t","CASE_t","CONTINUE_t","DEFAULT_t","ELSE_t","EXISTING_t","FOR_t",
"IF_t","IGNORE_t","IN_t","INCLUDE_t","LOCAL_t","MAXLINE_t","ON_t","PIECEMEAL_t",
"QUIETLY_t","VCERRORCONV_t","RESPONSE_t","RETURN_t","RULE_t","SWITCH_t","TOGETHER_t",
"UPDATED_t","WHILE_t","_LBRACE_t","_BAR_t","_BARBAR_t","_RBRACE_t","ARG","STRING",
"run","block","rules","rule","@1","@2","assign","expr","cases","case","params",
"lol","list","listp","arg","@3","func","eflags","eflag","bindlist",""
};
#endif

static const short yyr1[] = {     0,
    52,    52,    53,    53,    54,    54,    54,    54,    55,    55,
    55,    55,    55,    55,    55,    55,    55,    55,    55,    55,
    55,    55,    55,    56,    57,    55,    58,    58,    58,    58,
    59,    59,    59,    59,    59,    59,    59,    59,    59,    59,
    59,    59,    59,    59,    60,    60,    61,    62,    62,    62,
    63,    63,    64,    65,    65,    66,    67,    66,    68,    68,
    68,    69,    69,    70,    70,    70,    70,    70,    70,    70,
    70,    70,    71,    71
};

static const short yyr2[] = {     0,
     0,     1,     0,     1,     1,     2,     4,     6,     3,     3,
     3,     4,     6,     3,     3,     3,     7,     5,     5,     7,
     5,     6,     3,     0,     0,     9,     1,     1,     1,     2,
     1,     3,     3,     3,     3,     3,     3,     3,     3,     3,
     3,     3,     2,     3,     0,     2,     4,     0,     3,     1,
     1,     3,     1,     0,     2,     1,     0,     4,     2,     4,
     4,     0,     2,     1,     1,     1,     1,     1,     1,     1,
     2,     1,     0,     2
};

static const short yydefact[] = {     1,
    57,    62,    54,    54,     0,     0,    54,    54,     0,    54,
     0,    54,     0,     3,    56,     2,     5,    54,     0,     0,
     0,    53,     0,     0,     0,     0,     0,    31,     0,     0,
     0,     0,    48,     0,     0,     0,     4,     6,    28,    27,
    29,     0,    54,    54,     0,    51,     0,    54,     0,    70,
    66,     0,    69,    67,    68,    72,    65,    64,    73,    63,
    14,    55,    15,    54,    43,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     3,     0,     0,    54,    10,     3,
    54,    23,    16,    50,     0,    45,     3,     9,    30,     0,
     0,    11,    54,     0,    59,    58,    71,    54,     0,     0,
    44,    33,    38,    39,    34,    35,    32,    36,    37,     0,
    40,    41,    42,     7,     0,    48,     3,     0,     0,    45,
     0,    54,    12,    52,    54,    54,    74,    24,     3,    19,
     3,    49,     0,     0,    18,    46,    21,     0,    61,    60,
     0,     0,     0,     8,    22,     3,    13,    25,    17,    20,
    47,     0,    26,     0,     0,     0
};

static const short yydefgoto[] = {   154,
    36,    37,    17,   141,   152,    44,    27,   119,   120,    85,
    45,    46,    22,    28,    19,    49,    20,    60,    99
};

static const short yypact[] = {   153,
-32768,-32768,-32768,-32768,   -38,     8,-32768,-32768,     2,-32768,
   -34,-32768,     8,   153,-32768,-32768,   153,   102,    15,   126,
    12,     2,    16,    14,     8,     8,    24,    17,    23,    21,
   182,    35,     1,    13,   120,     5,-32768,-32768,-32768,-32768,
-32768,    52,-32768,-32768,    57,    67,     2,-32768,    55,-32768,
-32768,    33,-32768,-32768,-32768,-32768,-32768,-32768,    65,-32768,
-32768,-32768,-32768,-32768,-32768,   136,     8,     8,     8,     8,
     8,     8,     8,     8,   153,     8,     8,-32768,-32768,   153,
-32768,-32768,-32768,    75,    42,    68,   153,-32768,-32768,    64,
    79,-32768,-32768,     3,-32768,-32768,-32768,-32768,    46,    47,
-32768,    48,   230,   230,-32768,-32768,    48,-32768,-32768,    60,
   225,   225,-32768,-32768,    84,     1,   153,    56,    61,    68,
    63,-32768,-32768,-32768,-32768,-32768,-32768,-32768,   153,    71,
   153,-32768,    69,   103,-32768,-32768,-32768,   110,-32768,-32768,
    77,    73,   182,-32768,-32768,   153,-32768,-32768,-32768,-32768,
-32768,    81,-32768,   114,   138,-32768
};

static const short yypgoto[] = {-32768,
   -62,     7,   -28,-32768,-32768,    49,    31,    25,-32768,    38,
   -44,    -2,-32768,     0,-32768,-32768,-32768,-32768,-32768
};


#define	YYLAST		246


static const short yytable[] = {    18,
    21,    23,    82,    95,    29,    30,    16,    32,    31,    34,
    25,    24,   110,    18,    26,    33,    18,   114,    48,     1,
     1,    62,    61,    38,   121,     1,    63,    67,    68,    69,
    18,    80,     1,    79,    81,    70,    71,    72,    73,    74,
    90,    91,   125,    35,    64,    83,    94,    78,   124,    47,
    84,    15,    15,    88,   133,    65,    66,    15,    86,    70,
    71,   100,    73,    74,    15,    89,   142,    92,   144,    75,
    76,    77,    39,    96,    18,   113,    93,    40,   115,    18,
    41,   140,    97,   151,   116,    98,    18,   117,    42,   123,
   118,   128,   129,   126,   131,   127,   143,   102,   103,   104,
   105,   106,   107,   108,   109,   134,   111,   112,   130,   135,
    39,   137,   146,   155,   150,    40,    18,   145,    41,   138,
   147,   149,   139,    67,    68,    69,    42,   148,    18,   153,
    18,    70,    71,    72,    73,    74,    43,   156,   122,    67,
    68,    69,    18,   101,   136,    18,     0,    70,    71,    72,
    73,    74,    50,   132,     0,    51,     0,     0,     0,    52,
     0,    53,    54,    55,    56,    87,    76,    77,    57,    58,
     1,     0,     2,     0,     3,    59,     4,     0,     0,     0,
     5,     6,    76,    77,     7,     8,     0,     9,     0,     0,
     0,     0,    10,    11,    12,     0,     0,    13,    14,     1,
     0,     2,    15,     3,     0,     4,     0,     0,     0,     5,
     6,     0,     0,     7,     0,     0,     9,     0,     0,     0,
     0,    10,    11,    12,     0,     0,    13,    14,    67,    68,
    69,    15,     0,    67,     0,     0,    70,    71,    72,    73,
    74,    70,    71,    72,    73,    74
};

static const short yycheck[] = {     0,
     3,     4,    31,    48,     7,     8,     0,    10,     9,    12,
     3,    50,    75,    14,     7,    50,    17,    80,    19,    18,
    18,    22,    11,    17,    87,    18,    11,     4,     5,     6,
    31,    11,    18,    11,    14,    12,    13,    14,    15,    16,
    43,    44,    40,    13,    31,    11,    47,    31,    93,    35,
    50,    50,    50,    49,   117,    25,    26,    50,    46,    12,
    13,    64,    15,    16,    50,    14,   129,    11,   131,    46,
    47,    48,     9,    19,    75,    78,    10,    14,    81,    80,
    17,   126,    50,   146,    10,    21,    87,    46,    25,    11,
    23,    46,    46,    94,    11,    98,    26,    67,    68,    69,
    70,    71,    72,    73,    74,    50,    76,    77,    49,    49,
     9,    49,    10,     0,   143,    14,   117,    49,    17,   122,
    11,    49,   125,     4,     5,     6,    25,    51,   129,    49,
   131,    12,    13,    14,    15,    16,    35,     0,    90,     4,
     5,     6,   143,     8,   120,   146,    -1,    12,    13,    14,
    15,    16,    27,   116,    -1,    30,    -1,    -1,    -1,    34,
    -1,    36,    37,    38,    39,    46,    47,    48,    43,    44,
    18,    -1,    20,    -1,    22,    50,    24,    -1,    -1,    -1,
    28,    29,    47,    48,    32,    33,    -1,    35,    -1,    -1,
    -1,    -1,    40,    41,    42,    -1,    -1,    45,    46,    18,
    -1,    20,    50,    22,    -1,    24,    -1,    -1,    -1,    28,
    29,    -1,    -1,    32,    -1,    -1,    35,    -1,    -1,    -1,
    -1,    40,    41,    42,    -1,    -1,    45,    46,     4,     5,
     6,    50,    -1,     4,    -1,    -1,    12,    13,    14,    15,
    16,    12,    13,    14,    15,    16
};
/* -*-C-*-  Note some compilers choke on comments on `#line' lines.  */
#line 3 "bison.simple"

/* Skeleton output parser for bison,
   Copyright (C) 1984, 1989, 1990 Free Software Foundation, Inc.

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
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.  */

/* As a special exception, when this file is copied by Bison into a
   Bison output file, you may use that output file without restriction.
   This special exception was added by the Free Software Foundation
   in version 1.24 of Bison.  */

#ifndef alloca
#ifdef __GNUC__
#define alloca __builtin_alloca
#else /* not GNU C.  */
#if (!defined (__STDC__) && defined (sparc)) || defined (__sparc__) || defined (__sparc) || defined (__sgi)
#include <alloca.h>
#else /* not sparc */
#if defined (MSDOS) && !defined (__TURBOC__)
#include <malloc.h>
#else /* not MSDOS, or __TURBOC__ */
#if defined(_AIX)
#include <malloc.h>
 #pragma alloca
#else /* not MSDOS, __TURBOC__, or _AIX */
#ifdef __hpux
#ifdef __cplusplus
extern "C" {
void *alloca (unsigned int);
};
#else /* not __cplusplus */
void *alloca ();
#endif /* not __cplusplus */
#endif /* __hpux */
#endif /* not _AIX */
#endif /* not MSDOS, or __TURBOC__ */
#endif /* not sparc.  */
#endif /* not GNU C.  */
#endif /* alloca not defined.  */

/* This is the parser code that is written into each bison parser
  when the %semantic_parser declaration is not specified in the grammar.
  It was written by Richard Stallman by simplifying the hairy parser
  used when %semantic_parser is specified.  */

/* Note: there must be only one dollar sign in this file.
   It is replaced by the list of actions, each action
   as one case of the switch.  */

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		-2
#define YYEOF		0
#define YYACCEPT	return(0)
#define YYABORT 	return(1)
#define YYERROR		goto yyerrlab1
/* Like YYERROR except do call yyerror.
   This remains here temporarily to ease the
   transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */
#define YYFAIL		goto yyerrlab
#define YYRECOVERING()  (!!yyerrstatus)
#define YYBACKUP(token, value) \
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    { yychar = (token), yylval = (value);			\
      yychar1 = YYTRANSLATE (yychar);				\
      YYPOPSTACK;						\
      goto yybackup;						\
    }								\
  else								\
    { yyerror ("syntax error: cannot back up"); YYERROR; }	\
while (0)

#define YYTERROR	1
#define YYERRCODE	256

#ifndef YYPURE
#define YYLEX		yylex()
#endif

#ifdef YYPURE
#ifdef YYLSP_NEEDED
#ifdef YYLEX_PARAM
#define YYLEX		yylex(&yylval, &yylloc, YYLEX_PARAM)
#else
#define YYLEX		yylex(&yylval, &yylloc)
#endif
#else /* not YYLSP_NEEDED */
#ifdef YYLEX_PARAM
#define YYLEX		yylex(&yylval, YYLEX_PARAM)
#else
#define YYLEX		yylex(&yylval)
#endif
#endif /* not YYLSP_NEEDED */
#endif

/* If nonreentrant, generate the variables here */

#ifndef YYPURE

int	yychar;			/*  the lookahead symbol		*/
YYSTYPE	yylval;			/*  the semantic value of the		*/
				/*  lookahead symbol			*/

#ifdef YYLSP_NEEDED
YYLTYPE yylloc;			/*  location data for the lookahead	*/
				/*  symbol				*/
#endif

int yynerrs;			/*  number of parse errors so far       */
#endif  /* not YYPURE */

#if YYDEBUG != 0
int yydebug;			/*  nonzero means print parse trace	*/
/* Since this is uninitialized, it does not stop multiple parsers
   from coexisting.  */
#endif

/*  YYINITDEPTH indicates the initial size of the parser's stacks	*/

#ifndef	YYINITDEPTH
#define YYINITDEPTH 200
#endif

/*  YYMAXDEPTH is the maximum size the stacks can grow to
    (effective only if the built-in stack extension method is used).  */

#if YYMAXDEPTH == 0
#undef YYMAXDEPTH
#endif

#ifndef YYMAXDEPTH
#define YYMAXDEPTH 10000
#endif

/* Prevent warning if -Wstrict-prototypes.  */
#ifdef __GNUC__
int yyparse (void);
#endif

#if __GNUC__ > 1		/* GNU C and GNU C++ define this.  */
#define __yy_memcpy(FROM,TO,COUNT)	__builtin_memcpy(TO,FROM,COUNT)
#else				/* not GNU C or C++ */
#ifndef __cplusplus

/* This is the most reliable way to avoid incompatibilities
   in available built-in functions on various systems.  */
static void
__yy_memcpy (from, to, count)
     char *from;
     char *to;
     int count;
{
  register char *f = from;
  register char *t = to;
  register int i = count;

  while (i-- > 0)
    *t++ = *f++;
}

#else /* __cplusplus */

/* This is the most reliable way to avoid incompatibilities
   in available built-in functions on various systems.  */
static void
__yy_memcpy (char *from, char *to, int count)
{
  register char *f = from;
  register char *t = to;
  register int i = count;

  while (i-- > 0)
    *t++ = *f++;
}

#endif
#endif

#line 192 "bison.simple"

/* The user can define YYPARSE_PARAM as the name of an argument to be passed
   into yyparse.  The argument should have type void *.
   It should actually point to an object.
   Grammar actions can access the variable by casting it
   to the proper pointer type.  */

#ifdef YYPARSE_PARAM
#define YYPARSE_PARAM_DECL void *YYPARSE_PARAM;
#else
#define YYPARSE_PARAM
#define YYPARSE_PARAM_DECL
#endif

int
yyparse(YYPARSE_PARAM)
     YYPARSE_PARAM_DECL
{
  register int yystate;
  register int yyn;
  register short *yyssp;
  register YYSTYPE *yyvsp;
  int yyerrstatus;	/*  number of tokens to shift before error messages enabled */
  int yychar1 = 0;		/*  lookahead token as an internal (translated) token number */

  short	yyssa[YYINITDEPTH];	/*  the state stack			*/
  YYSTYPE yyvsa[YYINITDEPTH];	/*  the semantic value stack		*/

  short *yyss = yyssa;		/*  refer to the stacks thru separate pointers */
  YYSTYPE *yyvs = yyvsa;	/*  to allow yyoverflow to reallocate them elsewhere */

#ifdef YYLSP_NEEDED
  YYLTYPE yylsa[YYINITDEPTH];	/*  the location stack			*/
  YYLTYPE *yyls = yylsa;
  YYLTYPE *yylsp;

#define YYPOPSTACK   (yyvsp--, yyssp--, yylsp--)
#else
#define YYPOPSTACK   (yyvsp--, yyssp--)
#endif

  int yystacksize = YYINITDEPTH;

#ifdef YYPURE
  int yychar;
  YYSTYPE yylval;
  int yynerrs;
#ifdef YYLSP_NEEDED
  YYLTYPE yylloc;
#endif
#endif

  YYSTYPE yyval;		/*  the variable used to return		*/
				/*  semantic values from the action	*/
				/*  routines				*/

  int yylen;

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Starting parse\n");
#endif

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss - 1;
  yyvsp = yyvs;
#ifdef YYLSP_NEEDED
  yylsp = yyls;
#endif

/* Push a new state, which is found in  yystate  .  */
/* In all cases, when you get here, the value and location stacks
   have just been pushed. so pushing a state here evens the stacks.  */
yynewstate:

  *++yyssp = yystate;

  if (yyssp >= yyss + yystacksize - 1)
    {
      /* Give user a chance to reallocate the stack */
      /* Use copies of these so that the &'s don't force the real ones into memory. */
      YYSTYPE *yyvs1 = yyvs;
      short *yyss1 = yyss;
#ifdef YYLSP_NEEDED
      YYLTYPE *yyls1 = yyls;
#endif

      /* Get the current used size of the three stacks, in elements.  */
      int size = yyssp - yyss + 1;

#ifdef yyoverflow
      /* Each stack pointer address is followed by the size of
	 the data in use in that stack, in bytes.  */
#ifdef YYLSP_NEEDED
      /* This used to be a conditional around just the two extra args,
	 but that might be undefined if yyoverflow is a macro.  */
      yyoverflow("parser stack overflow",
		 &yyss1, size * sizeof (*yyssp),
		 &yyvs1, size * sizeof (*yyvsp),
		 &yyls1, size * sizeof (*yylsp),
		 &yystacksize);
#else
      yyoverflow("parser stack overflow",
		 &yyss1, size * sizeof (*yyssp),
		 &yyvs1, size * sizeof (*yyvsp),
		 &yystacksize);
#endif

      yyss = yyss1; yyvs = yyvs1;
#ifdef YYLSP_NEEDED
      yyls = yyls1;
#endif
#else /* no yyoverflow */
      /* Extend the stack our own way.  */
      if (yystacksize >= YYMAXDEPTH)
	{
	  yyerror("parser stack overflow");
	  return 2;
	}
      yystacksize *= 2;
      if (yystacksize > YYMAXDEPTH)
	yystacksize = YYMAXDEPTH;
      yyss = (short *) alloca (yystacksize * sizeof (*yyssp));
      __yy_memcpy ((char *)yyss1, (char *)yyss, size * sizeof (*yyssp));
      yyvs = (YYSTYPE *) alloca (yystacksize * sizeof (*yyvsp));
      __yy_memcpy ((char *)yyvs1, (char *)yyvs, size * sizeof (*yyvsp));
#ifdef YYLSP_NEEDED
      yyls = (YYLTYPE *) alloca (yystacksize * sizeof (*yylsp));
      __yy_memcpy ((char *)yyls1, (char *)yyls, size * sizeof (*yylsp));
#endif
#endif /* no yyoverflow */

      yyssp = yyss + size - 1;
      yyvsp = yyvs + size - 1;
#ifdef YYLSP_NEEDED
      yylsp = yyls + size - 1;
#endif

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Stack size increased to %d\n", yystacksize);
#endif

      if (yyssp >= yyss + yystacksize - 1)
	YYABORT;
    }

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Entering state %d\n", yystate);
#endif

  goto yybackup;
 yybackup:

/* Do appropriate processing given the current state.  */
/* Read a lookahead token if we need one and don't already have one.  */
/* yyresume: */

  /* First try to decide what to do without reference to lookahead token.  */

  yyn = yypact[yystate];
  if (yyn == YYFLAG)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* yychar is either YYEMPTY or YYEOF
     or a valid token in external form.  */

  if (yychar == YYEMPTY)
    {
#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Reading a token: ");
#endif
      yychar = YYLEX;
    }

  /* Convert token to internal form (in yychar1) for indexing tables with */

  if (yychar <= 0)		/* This means end of input. */
    {
      yychar1 = 0;
      yychar = YYEOF;		/* Don't call YYLEX any more */

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Now at end of input.\n");
#endif
    }
  else
    {
      yychar1 = YYTRANSLATE(yychar);

#if YYDEBUG != 0
      if (yydebug)
	{
	  fprintf (stderr, "Next token is %d (%s", yychar, yytname[yychar1]);
	  /* Give the individual parser a way to print the precise meaning
	     of a token, for further debugging info.  */
#ifdef YYPRINT
	  YYPRINT (stderr, yychar, yylval);
#endif
	  fprintf (stderr, ")\n");
	}
#endif
    }

  yyn += yychar1;
  if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != yychar1)
    goto yydefault;

  yyn = yytable[yyn];

  /* yyn is what to do for this token type in this state.
     Negative => reduce, -yyn is rule number.
     Positive => shift, yyn is new state.
       New state is final state => don't bother to shift,
       just return success.
     0, or most negative number => error.  */

  if (yyn < 0)
    {
      if (yyn == YYFLAG)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }
  else if (yyn == 0)
    goto yyerrlab;

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Shift the lookahead token.  */

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Shifting token %d (%s), ", yychar, yytname[yychar1]);
#endif

  /* Discard the token being shifted unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  *++yyvsp = yylval;
#ifdef YYLSP_NEEDED
  *++yylsp = yylloc;
#endif

  /* count tokens shifted since error; after three, turn off error status.  */
  if (yyerrstatus) yyerrstatus--;

  yystate = yyn;
  goto yynewstate;

/* Do the default action for the current state.  */
yydefault:

  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;

/* Do a reduction.  yyn is the number of a rule to reduce with.  */
yyreduce:
  yylen = yyr2[yyn];
  if (yylen > 0)
    yyval = yyvsp[1-yylen]; /* implement default value of the action */

#if YYDEBUG != 0
  if (yydebug)
    {
      int i;

      fprintf (stderr, "Reducing via rule %d (line %d), ",
	       yyn, yyrline[yyn]);

      /* Print the symbols being reduced, and their result.  */
      for (i = yyprhs[yyn]; yyrhs[i] > 0; i++)
	fprintf (stderr, "%s ", yytname[yyrhs[i]]);
      fprintf (stderr, " -> %s\n", yytname[yyr1[yyn]]);
    }
#endif


  switch (yyn) {

case 2:
#line 132 "jamgram.y"
{ parse_save( yyvsp[0].parse ); ;
    break;}
case 3:
#line 143 "jamgram.y"
{ yyval.parse = pnull(); ;
    break;}
case 4:
#line 145 "jamgram.y"
{ yyval.parse = yyvsp[0].parse; ;
    break;}
case 5:
#line 149 "jamgram.y"
{ yyval.parse = yyvsp[0].parse; ;
    break;}
case 6:
#line 151 "jamgram.y"
{ yyval.parse = prules( yyvsp[-1].parse, yyvsp[0].parse ); ;
    break;}
case 7:
#line 153 "jamgram.y"
{ yyval.parse = plocal( yyvsp[-2].parse, pnull(), yyvsp[0].parse ); ;
    break;}
case 8:
#line 155 "jamgram.y"
{ yyval.parse = plocal( yyvsp[-4].parse, yyvsp[-2].parse, yyvsp[0].parse ); ;
    break;}
case 9:
#line 159 "jamgram.y"
{ yyval.parse = yyvsp[-1].parse; ;
    break;}
case 10:
#line 161 "jamgram.y"
{ yyval.parse = pincl( yyvsp[-1].parse ); ;
    break;}
case 11:
#line 163 "jamgram.y"
{ yyval.parse = prule( yyvsp[-2].parse, yyvsp[-1].parse ); ;
    break;}
case 12:
#line 165 "jamgram.y"
{ yyval.parse = pset( yyvsp[-3].parse, yyvsp[-1].parse, yyvsp[-2].number ); ;
    break;}
case 13:
#line 167 "jamgram.y"
{ yyval.parse = pset1( yyvsp[-5].parse, yyvsp[-3].parse, yyvsp[-1].parse, yyvsp[-2].number ); ;
    break;}
case 14:
#line 169 "jamgram.y"
{ yyval.parse = pbreak( yyvsp[-1].parse, JMP_BREAK ); ;
    break;}
case 15:
#line 171 "jamgram.y"
{ yyval.parse = pbreak( yyvsp[-1].parse, JMP_CONTINUE ); ;
    break;}
case 16:
#line 173 "jamgram.y"
{ yyval.parse = pbreak( yyvsp[-1].parse, JMP_RETURN ); ;
    break;}
case 17:
#line 175 "jamgram.y"
{ yyval.parse = pfor( yyvsp[-5].string, yyvsp[-3].parse, yyvsp[-1].parse ); ;
    break;}
case 18:
#line 177 "jamgram.y"
{ yyval.parse = pswitch( yyvsp[-3].parse, yyvsp[-1].parse ); ;
    break;}
case 19:
#line 179 "jamgram.y"
{ yyval.parse = pif( yyvsp[-3].parse, yyvsp[-1].parse, pnull() ); ;
    break;}
case 20:
#line 181 "jamgram.y"
{ yyval.parse = pif( yyvsp[-5].parse, yyvsp[-3].parse, yyvsp[0].parse ); ;
    break;}
case 21:
#line 183 "jamgram.y"
{ yyval.parse = pwhile( yyvsp[-3].parse, yyvsp[-1].parse ); ;
    break;}
case 22:
#line 185 "jamgram.y"
{ yyval.parse = psetc( yyvsp[-4].string, yyvsp[-3].parse, yyvsp[-1].parse ); ;
    break;}
case 23:
#line 187 "jamgram.y"
{ yyval.parse = pon( yyvsp[-1].parse, yyvsp[0].parse ); ;
    break;}
case 24:
#line 189 "jamgram.y"
{ yymode( SCAN_STRING ); ;
    break;}
case 25:
#line 191 "jamgram.y"
{ yymode( SCAN_NORMAL ); ;
    break;}
case 26:
#line 193 "jamgram.y"
{ yyval.parse = psete( yyvsp[-6].string,yyvsp[-5].parse,yyvsp[-2].string,yyvsp[-7].number ); ;
    break;}
case 27:
#line 201 "jamgram.y"
{ yyval.number = VAR_SET; ;
    break;}
case 28:
#line 203 "jamgram.y"
{ yyval.number = VAR_APPEND; ;
    break;}
case 29:
#line 205 "jamgram.y"
{ yyval.number = VAR_DEFAULT; ;
    break;}
case 30:
#line 207 "jamgram.y"
{ yyval.number = VAR_DEFAULT; ;
    break;}
case 31:
#line 215 "jamgram.y"
{ yyval.parse = peval( EXPR_EXISTS, yyvsp[0].parse, pnull() ); ;
    break;}
case 32:
#line 217 "jamgram.y"
{ yyval.parse = peval( EXPR_EQUALS, yyvsp[-2].parse, yyvsp[0].parse ); ;
    break;}
case 33:
#line 219 "jamgram.y"
{ yyval.parse = peval( EXPR_NOTEQ, yyvsp[-2].parse, yyvsp[0].parse ); ;
    break;}
case 34:
#line 221 "jamgram.y"
{ yyval.parse = peval( EXPR_LESS, yyvsp[-2].parse, yyvsp[0].parse ); ;
    break;}
case 35:
#line 223 "jamgram.y"
{ yyval.parse = peval( EXPR_LESSEQ, yyvsp[-2].parse, yyvsp[0].parse ); ;
    break;}
case 36:
#line 225 "jamgram.y"
{ yyval.parse = peval( EXPR_MORE, yyvsp[-2].parse, yyvsp[0].parse ); ;
    break;}
case 37:
#line 227 "jamgram.y"
{ yyval.parse = peval( EXPR_MOREEQ, yyvsp[-2].parse, yyvsp[0].parse ); ;
    break;}
case 38:
#line 229 "jamgram.y"
{ yyval.parse = peval( EXPR_AND, yyvsp[-2].parse, yyvsp[0].parse ); ;
    break;}
case 39:
#line 231 "jamgram.y"
{ yyval.parse = peval( EXPR_AND, yyvsp[-2].parse, yyvsp[0].parse ); ;
    break;}
case 40:
#line 233 "jamgram.y"
{ yyval.parse = peval( EXPR_OR, yyvsp[-2].parse, yyvsp[0].parse ); ;
    break;}
case 41:
#line 235 "jamgram.y"
{ yyval.parse = peval( EXPR_OR, yyvsp[-2].parse, yyvsp[0].parse ); ;
    break;}
case 42:
#line 237 "jamgram.y"
{ yyval.parse = peval( EXPR_IN, yyvsp[-2].parse, yyvsp[0].parse ); ;
    break;}
case 43:
#line 239 "jamgram.y"
{ yyval.parse = peval( EXPR_NOT, yyvsp[0].parse, pnull() ); ;
    break;}
case 44:
#line 241 "jamgram.y"
{ yyval.parse = yyvsp[-1].parse; ;
    break;}
case 45:
#line 251 "jamgram.y"
{ yyval.parse = P0; ;
    break;}
case 46:
#line 253 "jamgram.y"
{ yyval.parse = pnode( yyvsp[-1].parse, yyvsp[0].parse ); ;
    break;}
case 47:
#line 257 "jamgram.y"
{ yyval.parse = psnode( yyvsp[-2].string, yyvsp[0].parse ); ;
    break;}
case 48:
#line 266 "jamgram.y"
{ yyval.parse = P0; ;
    break;}
case 49:
#line 268 "jamgram.y"
{ yyval.parse = psnode( yyvsp[-2].string, yyvsp[0].parse ); ;
    break;}
case 50:
#line 270 "jamgram.y"
{ yyval.parse = psnode( yyvsp[0].string, P0 ); ;
    break;}
case 51:
#line 279 "jamgram.y"
{ yyval.parse = pnode( P0, yyvsp[0].parse ); ;
    break;}
case 52:
#line 281 "jamgram.y"
{ yyval.parse = pnode( yyvsp[0].parse, yyvsp[-2].parse ); ;
    break;}
case 53:
#line 291 "jamgram.y"
{ yyval.parse = yyvsp[0].parse; yymode( SCAN_NORMAL ); ;
    break;}
case 54:
#line 295 "jamgram.y"
{ yyval.parse = pnull(); yymode( SCAN_PUNCT ); ;
    break;}
case 55:
#line 297 "jamgram.y"
{ yyval.parse = pappend( yyvsp[-1].parse, yyvsp[0].parse ); ;
    break;}
case 56:
#line 301 "jamgram.y"
{ yyval.parse = plist( yyvsp[0].string ); ;
    break;}
case 57:
#line 302 "jamgram.y"
{ yymode( SCAN_NORMAL ); ;
    break;}
case 58:
#line 303 "jamgram.y"
{ yyval.parse = yyvsp[-1].parse; ;
    break;}
case 59:
#line 312 "jamgram.y"
{ yyval.parse = prule( yyvsp[-1].parse, yyvsp[0].parse ); ;
    break;}
case 60:
#line 314 "jamgram.y"
{ yyval.parse = pon( yyvsp[-2].parse, prule( yyvsp[-1].parse, yyvsp[0].parse ) ); ;
    break;}
case 61:
#line 316 "jamgram.y"
{ yyval.parse = pon( yyvsp[-2].parse, yyvsp[0].parse ); ;
    break;}
case 62:
#line 325 "jamgram.y"
{ yyval.number = 0; ;
    break;}
case 63:
#line 327 "jamgram.y"
{ yyval.number = yyvsp[-1].number | yyvsp[0].number; ;
    break;}
case 64:
#line 331 "jamgram.y"
{ yyval.number = RULE_UPDATED; ;
    break;}
case 65:
#line 333 "jamgram.y"
{ yyval.number = RULE_TOGETHER; ;
    break;}
case 66:
#line 335 "jamgram.y"
{ yyval.number = RULE_IGNORE; ;
    break;}
case 67:
#line 337 "jamgram.y"
{ yyval.number = RULE_QUIETLY; ;
    break;}
case 68:
#line 339 "jamgram.y"
{ yyval.number = RULE_VCERRORCONV; ;
    break;}
case 69:
#line 341 "jamgram.y"
{ yyval.number = RULE_PIECEMEAL; ;
    break;}
case 70:
#line 343 "jamgram.y"
{ yyval.number = RULE_EXISTING; ;
    break;}
case 71:
#line 345 "jamgram.y"
{ yyval.number = atoi( yyvsp[0].string ) * RULE_MAXLINE; ;
    break;}
case 72:
#line 347 "jamgram.y"
{ yyval.number = RULE_RESPONSE; ;
    break;}
case 73:
#line 356 "jamgram.y"
{ yyval.parse = pnull(); ;
    break;}
case 74:
#line 358 "jamgram.y"
{ yyval.parse = yyvsp[0].parse; ;
    break;}
}
   /* the action file gets copied in in place of this dollarsign */
#line 487 "bison.simple"

  yyvsp -= yylen;
  yyssp -= yylen;
#ifdef YYLSP_NEEDED
  yylsp -= yylen;
#endif

#if YYDEBUG != 0
  if (yydebug)
    {
      short *ssp1 = yyss - 1;
      fprintf (stderr, "state stack now");
      while (ssp1 != yyssp)
	fprintf (stderr, " %d", *++ssp1);
      fprintf (stderr, "\n");
    }
#endif

  *++yyvsp = yyval;

#ifdef YYLSP_NEEDED
  yylsp++;
  if (yylen == 0)
    {
      yylsp->first_line = yylloc.first_line;
      yylsp->first_column = yylloc.first_column;
      yylsp->last_line = (yylsp-1)->last_line;
      yylsp->last_column = (yylsp-1)->last_column;
      yylsp->text = 0;
    }
  else
    {
      yylsp->last_line = (yylsp+yylen-1)->last_line;
      yylsp->last_column = (yylsp+yylen-1)->last_column;
    }
#endif

  /* Now "shift" the result of the reduction.
     Determine what state that goes to,
     based on the state we popped back to
     and the rule number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTBASE] + *yyssp;
  if (yystate >= 0 && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTBASE];

  goto yynewstate;

yyerrlab:   /* here on detecting error */

  if (! yyerrstatus)
    /* If not already recovering from an error, report this error.  */
    {
      ++yynerrs;

#ifdef YYERROR_VERBOSE
      yyn = yypact[yystate];

      if (yyn > YYFLAG && yyn < YYLAST)
	{
	  int size = 0;
	  char *msg;
	  int x, count;

	  count = 0;
	  /* Start X at -yyn if nec to avoid negative indexes in yycheck.  */
	  for (x = (yyn < 0 ? -yyn : 0);
	       x < (sizeof(yytname) / sizeof(char *)); x++)
	    if (yycheck[x + yyn] == x)
	      size += strlen(yytname[x]) + 15, count++;
	  msg = (char *) malloc(size + 15);
	  if (msg != 0)
	    {
	      strcpy(msg, "parse error");

	      if (count < 5)
		{
		  count = 0;
		  for (x = (yyn < 0 ? -yyn : 0);
		       x < (sizeof(yytname) / sizeof(char *)); x++)
		    if (yycheck[x + yyn] == x)
		      {
			strcat(msg, count == 0 ? ", expecting `" : " or `");
			strcat(msg, yytname[x]);
			strcat(msg, "'");
			count++;
		      }
		}
	      yyerror(msg);
	      free(msg);
	    }
	  else
	    yyerror ("parse error; also virtual memory exceeded");
	}
      else
#endif /* YYERROR_VERBOSE */
	yyerror("parse error");
    }

  goto yyerrlab1;
yyerrlab1:   /* here on error raised explicitly by an action */

  if (yyerrstatus == 3)
    {
      /* if just tried and failed to reuse lookahead token after an error, discard it.  */

      /* return failure if at end of input */
      if (yychar == YYEOF)
	YYABORT;

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Discarding token %d (%s).\n", yychar, yytname[yychar1]);
#endif

      yychar = YYEMPTY;
    }

  /* Else will try to reuse lookahead token
     after shifting the error token.  */

  yyerrstatus = 3;		/* Each real token shifted decrements this */

  goto yyerrhandle;

yyerrdefault:  /* current state does not do anything special for the error token. */

#if 0
  /* This is wrong; only states that explicitly want error tokens
     should shift them.  */
  yyn = yydefact[yystate];  /* If its default is to accept any token, ok.  Otherwise pop it.*/
  if (yyn) goto yydefault;
#endif

yyerrpop:   /* pop the current state because it cannot handle the error token */

  if (yyssp == yyss) YYABORT;
  yyvsp--;
  yystate = *--yyssp;
#ifdef YYLSP_NEEDED
  yylsp--;
#endif

#if YYDEBUG != 0
  if (yydebug)
    {
      short *ssp1 = yyss - 1;
      fprintf (stderr, "Error: state stack now");
      while (ssp1 != yyssp)
	fprintf (stderr, " %d", *++ssp1);
      fprintf (stderr, "\n");
    }
#endif

yyerrhandle:

  yyn = yypact[yystate];
  if (yyn == YYFLAG)
    goto yyerrdefault;

  yyn += YYTERROR;
  if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != YYTERROR)
    goto yyerrdefault;

  yyn = yytable[yyn];
  if (yyn < 0)
    {
      if (yyn == YYFLAG)
	goto yyerrpop;
      yyn = -yyn;
      goto yyreduce;
    }
  else if (yyn == 0)
    goto yyerrpop;

  if (yyn == YYFINAL)
    YYACCEPT;

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Shifting error token, ");
#endif

  *++yyvsp = yylval;
#ifdef YYLSP_NEEDED
  *++yylsp = yylloc;
#endif

  yystate = yyn;
  goto yynewstate;
}
#line 362 "jamgram.y"
