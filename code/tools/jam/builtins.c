/*
 * Copyright 1993-2002 Christopher Seiwald and Perforce Software, Inc.
 *
 * This file is part of Jam - see jam.c for Copyright information.
 */

/*
 * builtins.c - builtin jam rules
 *
 * External routines:
 *
 * 	load_builtin() - define builtin rules
 *
 * Internal routines:
 *
 *	builtin_depends() - DEPENDS/INCLUDES rule
 *	builtin_echo() - ECHO rule
 *	builtin_exit() - EXIT rule
 *	builtin_flags() - NOCARE, NOTFILE, TEMPORARY rule
 *	builtin_glob() - GLOB rule
 *	builtin_match() - MATCH rule
 *
 * 01/10/01 (seiwald) - split from compile.c
 * 01/08/01 (seiwald) - new 'Glob' (file expansion) builtin
 * 03/02/02 (seiwald) - new 'Match' (regexp match) builtin
 * 04/03/02 (seiwald) - Glob matches only filename, not directory
 * 10/22/02 (seiwald) - list_new() now does its own newstr()/copystr()
 * 10/22/02 (seiwald) - working return/break/continue statements
 * 11/04/02 (seiwald) - const-ing for string literals
 * 12/03/02 (seiwald) - fix odd includes support by grafting them onto depends
 */

# include "jam.h"

# include "lists.h"
# include "parse.h"
# include "builtins.h"
# include "rules.h"
# include "filesys.h"
# include "newstr.h"
# include "regexp.h"
# include "pathsys.h"
# include "w32_getreg.h"
# include "search.h"

/*
 * compile_builtin() - define builtin rules
 */

# define P0 (PARSE *)0
# define C0 (char *)0

LIST *builtin_depends( PARSE *parse, LOL *args, int *jmp );
LIST *builtin_echo( PARSE *parse, LOL *args, int *jmp );
LIST *builtin_exit( PARSE *parse, LOL *args, int *jmp );
LIST *builtin_flags( PARSE *parse, LOL *args, int *jmp );
LIST *builtin_glob( PARSE *parse, LOL *args, int *jmp );
LIST *builtin_match( PARSE *parse, LOL *args, int *jmp );
LIST *builtin_w32_getreg( PARSE *parse, LOL *args, int *jmp );

#ifdef OPT_BUILTIN_MD5_EXT
LIST *builtin_md5( PARSE *parse, LOL *args, int *jmp );
#endif /* OPT_BUILTIN_MD5_EXT */
LIST *builtin_putenv( PARSE *parse, LOL *args, int *jmp );
LIST *builtin_readfile(	PARSE	*parse,	LOL	*args,	int	*jmp );

int glob( const char *s, const char *c );

void
load_builtins()
{
    bindrule( "Always" )->procedure = 
    bindrule( "ALWAYS" )->procedure = 
	parse_make( builtin_flags, P0, P0, P0, C0, C0, T_FLAG_TOUCHED );

    bindrule( "Depends" )->procedure = 
    bindrule( "DEPENDS" )->procedure = 
	parse_make( builtin_depends, P0, P0, P0, C0, C0, 0 );

    bindrule( "echo" )->procedure = 
    bindrule( "Echo" )->procedure = 
    bindrule( "ECHO" )->procedure = 
	parse_make( builtin_echo, P0, P0, P0, C0, C0, 0 );

    bindrule( "exit" )->procedure = 
    bindrule( "Exit" )->procedure = 
    bindrule( "EXIT" )->procedure = 
	parse_make( builtin_exit, P0, P0, P0, C0, C0, 0 );

    bindrule( "Glob" )->procedure = 
    bindrule( "GLOB" )->procedure = 
	parse_make( builtin_glob, P0, P0, P0, C0, C0, 0 );

    bindrule( "Includes" )->procedure = 
    bindrule( "INCLUDES" )->procedure = 
	parse_make( builtin_depends, P0, P0, P0, C0, C0, 1 );

    bindrule( "Leaves" )->procedure = 
    bindrule( "LEAVES" )->procedure = 
	parse_make( builtin_flags, P0, P0, P0, C0, C0, T_FLAG_LEAVES );

    bindrule( "Match" )->procedure = 
    bindrule( "MATCH" )->procedure = 
	parse_make( builtin_match, P0, P0, P0, C0, C0, 0 );

    bindrule( "NoCare" )->procedure = 
    bindrule( "NOCARE" )->procedure = 
	parse_make( builtin_flags, P0, P0, P0, C0, C0, T_FLAG_NOCARE );

    bindrule( "NOTIME" )->procedure = 
    bindrule( "NotFile" )->procedure = 
    bindrule( "NOTFILE" )->procedure = 
	parse_make( builtin_flags, P0, P0, P0, C0, C0, T_FLAG_NOTFILE );

    bindrule( "NoUpdate" )->procedure = 
    bindrule( "NOUPDATE" )->procedure = 
	parse_make( builtin_flags, P0, P0, P0, C0, C0, T_FLAG_NOUPDATE );

    bindrule( "Temporary" )->procedure = 
    bindrule( "TEMPORARY" )->procedure = 
	parse_make( builtin_flags, P0, P0, P0, C0, C0, T_FLAG_TEMP );

    bindrule( "W32_GETREG" )->procedure =
	parse_make( builtin_w32_getreg, P0, P0, P0, C0, C0, 0 );

#ifdef OPT_BUILTIN_MD5_EXT
    bindrule( "MD5" )->procedure =
	parse_make( builtin_md5, P0, P0, P0, C0, C0, 0 );
#endif /* OPT_BUILTIN_MD5_EXT */

	bindrule( "PutEnv" )->procedure =
	bindrule( "PUTENV" )->procedure =
	parse_make( builtin_putenv, P0, P0, P0, C0, C0, 0 );

    bindrule( "ReadFile" )->procedure =
	parse_make( builtin_readfile, P0, P0, P0, C0, C0, 0 );
}

/*
 * builtin_w32_getreg() - W32_GETREG rule, returns a registry entry
 *          given a list of keys.
 *
 * Usage: result = [ W32_GETREG list ] ;
 */
static LIST*
builtin_w32_getreg( PARSE *parse, LOL *args, int *jmp )
{
    const char* result = w32_getreg(lol_get(args, 0));
    if (result)
		return list_new(L0, result, 0);
    return L0;
}


/*
 * builtin_depends() - DEPENDS/INCLUDES rule
 *
 * The DEPENDS builtin rule appends each of the listed sources on the 
 * dependency list of each of the listed targets.  It binds both the 
 * targets and sources as TARGETs.
 */

LIST *
builtin_depends(
	PARSE	*parse,
	LOL	*args,
	int	*jmp )
{
	LIST *targets = lol_get( args, 0 );
	LIST *sources = lol_get( args, 1 );
	int which = parse->num;
	LIST *l;

	for( l = targets; l; l = list_next( l ) )
	{
	    TARGET *t = bindtarget( l->string );

	    if( !which )
		t->depends = targetlist( t->depends, sources );
	    else
		t->includes = targetlist( t->includes, sources );
	}

	return L0;
}

/*
 * builtin_echo() - ECHO rule
 *
 * The ECHO builtin rule echoes the targets to the user.  No other 
 * actions are taken.
 */

LIST *
builtin_echo(
	PARSE	*parse,
	LOL	*args,
	int	*jmp )
{
	list_print( lol_get( args, 0 ) );
	printf( "\n" );
	return L0;
}

/*
 * builtin_putenv() - PUTENV rule
 *
 * The PUTENV builtin rule sets an environment item to the specified value.
 */

char *ConcatenateList(LIST *pList, char* pszSeparator)
{
	// count characters
	int n = 0;
	for (LIST *_pList=pList; _pList; _pList=_pList->next)
		n += strlen(_pList->string) + strlen(pszSeparator);
	char *psz = (char*)malloc(n+1);
	psz[0] = 0;
	for (LIST *_pList=pList; _pList; _pList=_pList->next)
	{
		strcat(psz, _pList->string);
		strcat(psz, pszSeparator);
	}
	psz[n-1] = 0;
	return psz;
}


LIST *
builtin_putenv(
	PARSE	*parse,
	LOL	*args,
	int	*jmp )
{
	LIST *env = lol_get( args, 0 );
	LIST *str = lol_get( args, 1 );
	char *pszEnv = ConcatenateList(env, " ");
	char *pszStr = ConcatenateList(str, ";");

	_putenv_s(pszEnv, pszStr);

	return L0;
}

/*
 * builtin_exit() - EXIT rule
 *
 * The EXIT builtin rule echoes the targets to the user and exits
 * the program with a failure status.
 */

LIST *
builtin_exit(
	PARSE	*parse,
	LOL	*args,
	int	*jmp )
{
	list_print( lol_get( args, 0 ) );
	printf( "\n" );
	exit( EXITBAD ); /* yeech */
	return L0;
}

/*
 * builtin_flags() - NOCARE, NOTFILE, TEMPORARY rule
 *
 * Builtin_flags() marks the target with the appropriate flag, for use
 * by make0().  It binds each target as a TARGET.
 */

LIST *
builtin_flags(
	PARSE	*parse,
	LOL	*args,
	int	*jmp )
{
	LIST *l = lol_get( args, 0 );

	for( ; l; l = list_next( l ) )
	    bindtarget( l->string )->flags |= parse->num;

	return L0;
}

/*
 * builtin_globbing() - GLOB rule
 */

struct globbing {
	LIST	*patterns;
	LIST	*results;
} ;

static void
builtin_glob_back(
	void	*closure,
	const char *file,
	int	status,
	time_t	time )
{
	struct globbing *globbing = (struct globbing *)closure;
	LIST		*l;
	PATHNAME	f;
	char		buf[ MAXJPATH ];

	/* Null out directory for matching. */
	/* We wish we had file_dirscan() pass up a PATHNAME. */

	path_parse( file, &f );
	f.f_dir.len = 0;
	path_build( &f, buf, 0 );

	for( l = globbing->patterns; l; l = l->next )
	    if( !glob( l->string, buf ) )
	{
	    globbing->results = list_new( globbing->results, file, 0 );
	    break;
	}
}

LIST *
builtin_glob(
	PARSE	*parse,
	LOL	*args,
	int	*jmp )
{
	LIST *l = lol_get( args, 0 );
	LIST *r = lol_get( args, 1 );

	struct globbing globbing;

	globbing.results = L0;
	globbing.patterns = r;

	for( ; l; l = list_next( l ) )
	    file_dirscan( l->string, builtin_glob_back, &globbing );

	return globbing.results;
}

/*
 * builtin_match() - MATCH rule, regexp matching
 */

LIST *
builtin_match(
	PARSE	*parse,
	LOL	*args,
	int	*jmp )
{
	LIST *l, *r;
	LIST *result = 0;

	/* For each pattern */

	for( l = lol_get( args, 0 ); l; l = l->next )
	{
	    regexp *re = regcomp( l->string );

	    /* For each string to match against */

	    for( r = lol_get( args, 1 ); r; r = r->next )
		if( regexec( re, r->string ) )
	    {
		int i, top;

		/* Find highest parameter */

		for( top = NSUBEXP; top-- > 1; )
		    if( re->startp[top] )
			break;

		/* And add all parameters up to highest onto list. */
		/* Must have parameters to have results! */

		for( i = 1; i <= top; i++ )
		{
		    char buf[ MAXSYM ];
		    int l = re->endp[i] - re->startp[i];
		    memcpy( buf, re->startp[i], l );
		    buf[ l ] = 0;
		    result = list_new( result, buf, 0 );
		}
	    }

	    free( (char *)re );
	}

	return result;
}
#ifdef OPT_BUILTIN_MD5_EXT
/*
 * builtin_md5() - Return the MD5 sum of the supplied lists
 */
#include "global.h"
#include "md5.h"

LIST *
builtin_md5(
	PARSE	*parse,
	LOL	*args,
	int	*jmp )
{
	static const unsigned char list_sep[] = { 0, 0 };
	static const unsigned char item_sep[] = { 0 };

	MD5_CTX context;
	unsigned char digest[16];
	unsigned char digest_string[33];
	unsigned char* p;
	int i;
	LIST *l;
	LIST *result;

	MD5Init(&context);

	/* For each argument */
	for (i = 0; i < args->count; ++i) {
	    if (i > 0) {
		/* separate lists with 2 NUL characters.  This
		 * guarantees that [ MD5 a b ] is different from [ MD5
		 * a : b ] */
		MD5Update(&context, list_sep, sizeof(list_sep));
	    }
	    l = lol_get(args, i);
	    if (l) {
		MD5Update(&context, (const unsigned char*)l->string, strlen(l->string));
		for (l = list_next(l); l; l = list_next(l)) {
		    /* separate list items with 1 NUL character.  This
		     * guarantees that [ MD5 a b ] is different from [
		     * MD5 ab ] */
		    MD5Update(&context, (const unsigned char*)item_sep, sizeof(item_sep));
		    MD5Update(&context, (const unsigned char*)l->string, strlen(l->string));
		}
	    }
	}

	MD5Final(digest, &context);
	p = digest_string;
	for (i = 0, p = digest_string; i < 16; i++, p += 2) {
	    sprintf((char*)p, "%02x", (char*)digest[i]);
	}
	*p = 0;

	result = list_new(L0, (char*)digest_string, 0);

	return result;
}
#endif /* OPT_BUILTIN_MD5_EXT */


// read the first line of a file into a variable
LIST *
builtin_readfile(
	PARSE	*parse,
	LOL	*args,
	int	*jmp )
{
	const int BUFFER_SIZE = 100 * 1024;
	unsigned char* buffer = (unsigned char*)malloc(BUFFER_SIZE);
	LIST *l;
	LIST *result;
	int x;

	/* only process the first argument */
	if (args->count>0)
	{
	    l = lol_get(args, 0);
	    if (l) {
		do {
			FILE* file;
			TARGET *t = bindtarget(l->string);
			pushsettings( t->settings );
			t->boundname = search( t->name, &t->time );
			popsettings( t->settings );
		    file = fopen(t->boundname, "rb");
		    if (file)
			{
				fread(buffer, 1, BUFFER_SIZE, file);

				for (x=0; x<BUFFER_SIZE; x++)
				{
					if (buffer[x] == 0)
						break;

					if (buffer[x] == 0x0d || buffer[x] == 0x0a)
					{
						buffer[x] = 0;
						break;
					}
				}

				fclose(file);
				break;
		    }

		    l = list_next(l);
		} while (l);
	    }
	}

	result = list_new(L0, (const char*)buffer, 0);

	free(buffer);

	return result;
}
