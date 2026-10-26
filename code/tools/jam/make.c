/*
 * Copyright 1993, 1995 Christopher Seiwald.
 *
 * This file is part of Jam - see jam.c for Copyright information.
 */

/*
 * make.c - bring a target up to date, once rules are in place
 *
 * This modules controls the execution of rules to bring a target and
 * its dependencies up to date.  It is invoked after the targets, rules,
 * et. al. described in rules.h are created by the interpreting of the
 * jam files.
 *
 * This file contains the main make() entry point and the first pass
 * make0().  The second pass, make1(), which actually does the command
 * execution, is in make1.c.
 *
 * External routines:
 *	make() - make a target, given its name
 *
 * Internal routines:
 * 	make0() - calculate fates for everything needed to make a TARGET
 * 	make0sort() - reorder TARGETS chain by their time (newest to oldest)
 * 	make0bscan() - bind and scan everything needed to make a TARGET
 *
 * 12/26/93 (seiwald) - allow NOTIME targets to be expanded via $(<), $(>)
 * 01/04/94 (seiwald) - print all targets, bounded, when tracing commands
 * 04/08/94 (seiwald) - progress report now reflects only targets with actions
 * 04/11/94 (seiwald) - Combined deps & headers into deps[2] in TARGET.
 * 12/20/94 (seiwald) - NOTIME renamed NOTFILE.
 * 12/20/94 (seiwald) - make0() headers after determining fate of target, so 
 *			that headers aren't seen as dependents on themselves.
 * 01/19/95 (seiwald) - distinguish between CANTFIND/CANTMAKE targets.
 * 02/02/95 (seiwald) - propagate leaf source time for new LEAVES rule.
 * 02/14/95 (seiwald) - NOUPDATE rule means don't update existing target.
 * 08/22/95 (seiwald) - NOUPDATE targets immune to anyhow (-a) flag.
 * 09/06/00 (seiwald) - NOCARE affects targets with sources/actions.
 * 03/02/01 (seiwald) - reverse NOCARE change.
 * 03/14/02 (seiwald) - TEMPORARY targets no longer take on parents age
 * 03/16/02 (seiwald) - support for -g (reorder builds by source time)
 * 07/17/02 (seiwald) - TEMPORARY sources for headers now get built
 * 09/19/02 (seiwald) - new -d displays
 * 09/23/02 (seiwald) - suppress "...using temp..." in default output
 * 09/28/02 (seiwald) - make0() takes parent pointer; new -dc display
 * 11/04/02 (seiwald) - const-ing for string literals
 * 12/03/02 (seiwald) - fix odd includes support by grafting them onto depends
 * 12/17/02 (seiwald) - new copysettings() to protect target-specific vars
 * 01/03/03 (seiwald) - T_FATE_NEWER once again gets set with missing parent
 */

# include "jam.h"

# include "lists.h"
# include "parse.h"
# include "variable.h"
# include "rules.h"

# include "search.h"
# include "newstr.h"
# include "make.h"
# include "headers.h"
# include "command.h"
#ifdef OPT_HEADER_CACHE_EXT
#include "hcache.h"
#endif
#ifdef OPT_IMPROVED_PROGRESS_EXT
#include "progress.h"
#endif

# ifndef max
# define max( a,b ) ((a)>(b)?(a):(b))
# endif

typedef struct {
	int	temp;
	int	updating;
	int	cantfind;
	int	cantmake;
	int	targets;
	int	made;
} COUNTS ;

static void make0( TARGET *t, TARGET *p, int depth, 
		COUNTS *counts, int anyhow );

static TARGETS *make0sort( TARGETS *c );
#ifdef OPT_GRAPH_DEBUG_EXT
static void dependGraphOutput( TARGET *t, int depth );
#endif

static void make0bscan( TARGET *t );
static void make0includes( TARGET *t );

static void make0gatherincludes( TARGET *t );
static TARGETS* make0gatherincludes2( const char* tname,
				      TARGETS *chain, TARGET* t );
static long mark_generation = 0;

static const char *target_fate[] = 
{
	"init",		/* T_FATE_INIT */
	"making", 	/* T_FATE_MAKING */
	"stable", 	/* T_FATE_STABLE */
	"newer",	/* T_FATE_NEWER */
	"temp", 	/* T_FATE_ISTMP */
	"touched", 	/* T_FATE_TOUCHED */
	"missing", 	/* T_FATE_MISSING */
	"needtmp", 	/* T_FATE_NEEDTMP */
	"old", 		/* T_FATE_OUTDATED */
	"update", 	/* T_FATE_UPDATE */
	"nofind", 	/* T_FATE_CANTFIND */
	"nomake" 	/* T_FATE_CANTMAKE */
} ;

static const char *target_bind[] = 
{
	"unbound",
	"missing",
	"parents",
	"exists",
} ;

# define spaces(x) ( "                " + 16 - ( x > 16 ? 16 : x ) )


/*
 * make() - make a target, given its name
 */

int
make( 
	int		n_targets,
	const char	**targets,
	int		anyhow )
{
	int i;
	COUNTS counts[1];
	int status = 0;		/* 1 if anything fails */

#ifdef OPT_HEADER_CACHE_EXT
	hcache_init();
#endif
	memset( (char *)counts, 0, sizeof( *counts ) );

	for( i = 0; i < n_targets; i++ )
	{
	    TARGET *t = bindtarget( targets[i] );

	    make0bscan( t );
	}

	for( i = 0; i < n_targets; i++ )
	{
	    TARGET *t = bindtarget( targets[i] );

	    make0includes( t );
	}

	/* if -b is used but the target was not found, quit out */
	if (globs.singletarget[0] && !globs.foundsingletarget)
	{
		printf( "The specified target '%s' was not found in any of the JAM files.\n", globs.singletarget);
		exit( EXITBAD );
	}

	if (globs.foundsingletarget)
	{
		// find direct dependants of single target and build those
		const char *mytargets[1024];
		int nmytargets = 0;

		for( i = 0; i < n_targets; i++ )
		{
			TARGET *t = bindtarget( targets[i] );

			makegetdirectdependents( t, globs.singletarget, mytargets, &nmytargets);
		}

		targets = mytargets;
		n_targets = nmytargets;

		if (n_targets == 0)
		{
			printf( "The specified target '%s' has no dependents in any of the JAM files.\n", globs.singletarget);
			exit( EXITBAD );
		}
	}

	for( i = 0; i < n_targets; i++ )
	{
	    TARGET *t = bindtarget( targets[i] );

	    make0( t, 0, 0, counts, anyhow );
	}
#ifdef OPT_GRAPH_DEBUG_EXT
	if( DEBUG_GRAPH )
	{
	    for( i = 0; i < n_targets; i++ )
	    {
		TARGET *t = bindtarget( targets[i] );
		dependGraphOutput( t, 0 );
	    }
	}
#endif

	if( DEBUG_MAKE )
	{
	    if( counts->targets )
		printf( "...found %d target(s)...\n", counts->targets );
	    if( counts->temp )
		printf( "...using %d temp target(s)...\n", counts->temp );
	    if( counts->updating )
		printf( "...updating %d target(s)...\n", counts->updating );
	    if( counts->cantfind )
		printf( "...can't find %d target(s)...\n", counts->cantfind );
	    if( counts->cantmake )
		printf( "...can't make %d target(s)...\n", counts->cantmake );
	}

#ifdef OPT_HEADER_CACHE_EXT
	hcache_done();
#endif
#ifdef OPT_IMPROVED_PROGRESS_EXT
	globs.updating = counts->updating;
	globs.progress = progress_start(counts->updating);
#endif
	status = counts->cantfind || counts->cantmake;

	make_initcounts();

	for( i = 0; i < n_targets; i++ )
	    make1( bindtarget( targets[i] ) );

	status |= make_finish();

	return status;
}

/*
 * make0() - bind and scan everything to make a TARGET
 *
 * Make0() recursively binds a target, searches for #included headers,
 * calls itself on those headers, and calls itself on any dependents.
 */

static void
make0( 
	TARGET	*t,
	TARGET  *p,		/* parent */
	int	depth,		/* for display purposes */
	COUNTS	*counts,	/* for reporting */
	int	anyhow )	/* forcibly touch all (real) targets */
{
	TARGETS	*c, *d, *incs;
	TARGET 	*ptime = t;
	time_t	last, leaf, hlast;
	int	fate;
	const char *flag = "";
	SETTINGS *s;
#ifdef OPT_GRAPH_DEBUG_EXT
	int	savedFate, oldTimeStamp;
#endif

	/* 
	 * Step 1: initialize
	 */

	if( DEBUG_MAKEPROG )
	    printf( "make\t--\t%s%s\n", spaces( depth ), t->name );

	t->fate = T_FATE_MAKING;

	/*
	 * Step 2: under the influence of "on target" variables,
	 * bind the target and search for headers.
	 */

	/* If temp file doesn't exist but parent does, use parent */

	if( p && t->flags & T_FLAG_TEMP && 
	    t->binding == T_BIND_MISSING && 
	    p->binding != T_BIND_MISSING )
	{
	    t->binding = T_BIND_PARENTS;
	    ptime = p;
	}

	/* 
	 * Pause for a little progress reporting 
	 */

	if( DEBUG_MAKEPROG )
	{
	    if( strcmp( t->name, t->boundname ) )
	    {
		printf( "bind\t--\t%s%s: %s\n",
			spaces( depth ), t->name, t->boundname );
	    }

	    switch( t->binding )
	    {
	    case T_BIND_UNBOUND:
	    case T_BIND_MISSING:
	    case T_BIND_PARENTS:
		printf( "time\t--\t%s%s: %s\n",
			spaces( depth ), t->name, target_bind[ t->binding ] );
		break;

	    case T_BIND_EXISTS:
		printf( "time\t--\t%s%s: %s",
			spaces( depth ), t->name, ctime( &t->time ) );
		break;
	    }
	}

	/* 
	 * Step 3: recursively make0() dependents & headers
	 */

	/* Step 3a: recursively make0() dependents */

	/* Warn about circular deps. */

	for( c = t->depends; c; c = c->next )
	{
	    if( DEBUG_DEPENDS )
		printf( "Depends \"%s\" : \"%s\" ;\n", 
		    t->name, c->target->name );

	    if( c->target->fate == T_FATE_MAKING )
		printf( "warning: %s depends on itself\n", c->target->name );
	    else if( c->target->fate == T_FATE_INIT )
		make0( c->target, ptime, depth + 1, counts, anyhow );
#ifdef OPT_FIX_UPDATED
	    else if( ptime && ptime->binding != T_BIND_UNBOUND &&
		     c->target->time > ptime->time &&
		     c->target->fate < T_FATE_NEWER )
	    {
		/*
		 * BUG FIX:
		 *
		 * If you have a rule with flag RULE_UPDATED, then any
		 * dependents must have fate greater than
		 * T_FATE_STABLE to be included.
		 *
		 * However, make.c can get confused for dependency
		 * trees like this:
		 *
		 * a --> b --> d
		 *   \-> c --> d
		 *
		 * In this case, make.c can set the fate of "d" before
		 * it ever gets to "c".  So you will end up with a
		 * T_FATE_MISSING target "c" with dependents with
		 * T_FATE_STABLE.
		 *
		 * If "c" happens to have a RULE_UPDATED action,
		 * RULE_UPDATED, make1list() will refrain from
		 * including it in the list of targets.
		 *
		 * We hack around this here by explicitly checking for
		 * this case and manually tweaking the dependents fate
		 * to at least T_FATE_NEWER.
		 *
		 * An alternate fix is to modify make1cmds() to take a
		 * TARGET* instead of an ACTIONS* and, when the target
		 * is T_FATE_MISSING, have it mask off the
		 * RULE_UPDATED flag when calling make1list().
		 */
#ifdef OPT_GRAPH_DEBUG_EXT
		if( DEBUG_FATE ) {
		    if( c->target->fate == T_FATE_STABLE ) {
			printf("fate change  %s set to %s (parent %s)\n",
			       c->target->name,
			       target_fate[T_FATE_NEWER],
			       ptime->name);
		    } else {
			printf("fate change  %s adjusted from %s to %s\n",
			       c->target->name,
			       target_fate[c->target->fate],
			       target_fate[T_FATE_NEWER]);
		    }
		}
#endif
		c->target->fate = T_FATE_NEWER;
	    }
#endif
	}

	/* Step 3b: recursively make0() headers */

	/* Ignore circular deps: headers include themselves a lot. */

	for( c = t->includes; c; c = c->next )
	{
	    if( DEBUG_DEPENDS )
		printf( "Includes \"%s\" : \"%s\" ;\n",
		    t->name, c->target->name );

	    if( c->target->fate == T_FATE_INIT )
		make0( c->target, p, depth + 1, counts, anyhow );
	}

	/* Step 3c: add dependents' includes to our direct dependencies */

	incs = 0;

	t->mark = ++mark_generation;
	for( c = t->depends; c; c = c->next )
	    c->target->mark = mark_generation;

	for( c = t->depends; c; c = c->next )
	    for( d = c->target->includes; d; d = d->next )
	    {
		TARGET *temp = bindtarget( d->target->name );
		
		if ( temp->mark != mark_generation )
		{
		    if( DEBUG_DEPENDS )
			printf( "# (implicit) Depends \"%s\" : \"%s\" ;\n",
				t->name, d->target->name );
		    temp->mark = mark_generation;
		    incs = targetentry( incs, temp );
		}
	    }

	t->depends = targetchain( t->depends, incs );

	/*
	 * Step 4: compute time & fate 
	 */

	/* Step 4a: pick up dependents' time and fate */

	last = 0;
	leaf = 0;
	fate = T_FATE_STABLE;

	for( c = t->depends; c; c = c->next )
	{
	    /* If LEAVES has been applied, we only heed the timestamps of */
	    /* the leaf source nodes. */

	    leaf = max( leaf, c->target->leaf );

	    if( t->flags & T_FLAG_LEAVES )
	    {
		last = leaf;
		continue;
	    }

	    last = max( last, c->target->time );
#ifdef OPT_GRAPH_DEBUG_EXT
	    if( DEBUG_FATE )
		if( fate < c->target->fate )
		    printf( "fate change  %s from %s to %s by dependency %s\n",
			    t->name,
			    target_fate[fate], target_fate[c->target->fate],
			    c->target->name);
#endif
#ifdef OPT_GRAPH_DEBUG_EXT
	    if( DEBUG_FATE && fate < c->target->fate ) {
		printf( "fate change  %s from %s to %s by dependency %s\n",
			t->name,
			target_fate[fate], target_fate[c->target->fate],
			c->target->name);
	    }
#endif
	    fate = max( fate, c->target->fate );
	}

	/* Step 4b: pick up included headers time */

	/* 
	 * If a header is newer than a temp source that includes it, 
	 * the temp source will need building.   Only bother for temp
	 * sources.
	 */

	hlast = 0;

	if( t->binding == T_BIND_PARENTS )
	    for( c = t->includes; c; c = c->next )
		hlast = max( hlast, c->target->time );

	/* Step 4c: handle NOUPDATE oddity */

	/*
	 * If a NOUPDATE file exists, make dependents eternally old.
	 * Don't inherit our fate from our dependents.  Decide fate
	 * based only upon other flags and our binding (done later).
	 */

	if( t->flags & T_FLAG_NOUPDATE )
	{
#ifdef OPT_GRAPH_DEBUG_EXT
	    if( DEBUG_FATE && fate != T_FATE_STABLE ) {
		printf("fate change  %s back to stable by NOUPDATE.\n",
		       t->name);
	    }
#endif
	    last = 0;
	    t->time = 0;
	    fate = T_FATE_STABLE;
	}

	/* Step 4d: determine fate: rebuild target or what? */

	/* 
	    In English:
		If can't find or make child, can't make target.
		If children changed, make target.
		If target missing, make it.
		If children newer, make target.
		If temp's children newer than parent, make temp.
		If temp's headers newer than parent, make temp.
		If deliberately touched, make it.
		If up-to-date temp file present, use it.
		If target newer than non-notfile parent, mark target newer.
		Otherwise, stable!

		Note this block runs from least to most stable:
		as we make it further down the list, the target's
		fate is getting stabler.
	*/
#ifdef OPT_GRAPH_DEBUG_EXT
	savedFate = fate;
	oldTimeStamp = 0;
#endif

	if( fate >= T_FATE_BROKEN )
	{
	    fate = T_FATE_CANTMAKE;
	}
	else if( fate >= T_FATE_SPOIL )
	{
	    fate = T_FATE_UPDATE;
	}
	else if( t->binding == T_BIND_MISSING )
	{
	    fate = T_FATE_MISSING;
	}
	else if( t->binding == T_BIND_EXISTS && last > t->time )
	{
#ifdef OPT_GRAPH_DEBUG_EXT
	    oldTimeStamp = 1;
#endif
	    fate = T_FATE_OUTDATED;
	}
	else if( t->binding == T_BIND_PARENTS && last > p->time )
	{
	    fate = T_FATE_NEEDTMP;
	}
	else if( t->binding == T_BIND_PARENTS && hlast > p->time )
	{
#ifdef OPT_GRAPH_DEBUG_EXT
	    oldTimeStamp = 1;
#endif
	    fate = T_FATE_NEEDTMP;
	}
	else if( t->flags & T_FLAG_TOUCHED )
	{
	    fate = T_FATE_TOUCHED;
	}
	else if( anyhow && !( t->flags & T_FLAG_NOUPDATE ) )
	{
	    fate = T_FATE_TOUCHED;
	}
	else if( t->binding == T_BIND_EXISTS && t->flags & T_FLAG_TEMP )
	{
	    fate = T_FATE_ISTMP;
	}
	else if( t->binding == T_BIND_EXISTS && p && 
		 p->binding != T_BIND_UNBOUND && t->time > p->time )
	{
	    fate = T_FATE_NEWER;
	}
	else
	{
	    fate = T_FATE_STABLE;
	}
#ifdef OPT_GRAPH_DEBUG_EXT
	if( DEBUG_FATE && fate != savedFate )
	    if( savedFate == T_FATE_STABLE )
		printf( "fate change  %s set to %s%s\n",
		       t->name, target_fate[fate],
		       oldTimeStamp ? " (by timestamp)" : "" );
	    else
		printf( "fate change  %s adjusted from %s to %s%s\n",
		       t->name, target_fate[savedFate], target_fate[fate],
		       oldTimeStamp ? " (by timestamp)" : "" );
#endif

	/* Step 4e: handle missing files */
	/* If it's missing and there are no actions to create it, boom. */
	/* If we can't make a target we don't care about, 'sokay */
	/* We could insist that there are updating actions for all missing */
	/* files, but if they have dependents we just pretend it's NOTFILE. */

	if( fate == T_FATE_MISSING && !t->actions && !t->depends )
	{
	    if( t->flags & T_FLAG_NOCARE )
	    {
#ifdef OPT_GRAPH_DEBUG_EXT
	    if( DEBUG_FATE )
		printf( "fate change  %s to STABLE from %s, "
		        "no actions, no dependents and don't care\n",
		       t->name, target_fate[fate]);
#endif
		fate = T_FATE_STABLE;
	    }
	    else
	    {
		printf( "don't know how to make %s\n", t->name );

		fate = T_FATE_CANTFIND;
	    }
	}

	/* Step 4f: propagate dependents' time & fate. */
	/* Set leaf time to be our time only if this is a leaf. */

	t->time = max( t->time, last );
	t->leaf = leaf ? leaf : t->time ;
	t->fate = fate;

	/* 
	 * Step 5: sort dependents by their update time. 
	 */

#ifdef OPT_FIX_NOTFILE_NEWESTFIRST
	if( globs.newestfirst && !( t->flags & T_FLAG_NOTFILE ) )
	    t->depends = make0sort( t->depends );
#else
	if( globs.newestfirst )
	    t->depends = make0sort( t->depends );
#endif

	/* 
	 * Step 6: a little harmless tabulating for tracing purposes 
	 */

#ifdef OPT_IMPROVED_PATIENCE_EXT
	++counts->targets;
#else	
	if( !( ++counts->targets % 1000 ) && DEBUG_MAKE )
	    printf( "...patience...\n" );
#endif

	if( fate == T_FATE_ISTMP )
	    counts->temp++;
	else if( fate == T_FATE_CANTFIND )
	    counts->cantfind++;
	else if( fate == T_FATE_CANTMAKE && t->actions )
	    counts->cantmake++;
	else if( fate >= T_FATE_BUILD && fate < T_FATE_BROKEN && t->actions )
	    counts->updating++;

	if( !( t->flags & T_FLAG_NOTFILE ) && fate >= T_FATE_SPOIL )
	    flag = "+";
	else if( t->binding == T_BIND_EXISTS && p && t->time > p->time )
	    flag = "*";

	if( DEBUG_MAKEPROG )
	    printf( "made%s\t%s\t%s%s\n", 
		flag, target_fate[ t->fate ], 
		spaces( depth ), t->name );

	if( DEBUG_CAUSES && 
	    t->fate >= T_FATE_NEWER && 
	    t->fate <= T_FATE_MISSING )
		printf( "%s %s\n", target_fate[ t->fate ], t->name );
}

/*
 * make0bscan() - bind and scan everything to make a TARGET
 *
 * make0bscan() recursively binds a target, searches for #included
 * headers, calls itself on those headers, and calls itself on any
 * dependents.
 */

static void
make0bscan( TARGET *t )
{
	TARGETS *c;
	SETTINGS *s;

	/* 
	 * Step 1: initialize
	 */

	if ( t->makeflags & T_MAKEFLAG_SCANNED )
	    return;
	t->makeflags |= T_MAKEFLAG_SCANNED;

	/*
	 * Step 2: under the influence of "on target" variables,
	 * bind the target and search for headers.
	 */

	/* Step 2a: set "on target" variables. */

	s = copysettings( t->settings );
	pushsettings( s );

	/* Step 2b: find the target file (if it's a file). */

	if( t->binding == T_BIND_UNBOUND && !( t->flags & T_FLAG_NOTFILE ) )
	{
	    t->boundname = search( t->name, &t->time );
	    t->binding = t->time ? T_BIND_EXISTS : T_BIND_MISSING;
	}

	/* Step 2c: If its a file, search for headers. */

	if( t->binding == T_BIND_EXISTS )
	    headers( t );
#ifdef OPT_SEMAPHORE
	{
	    LIST *var = var_get( "SEMAPHORE" );
	    if( var )
	    {
		TARGET *semaphore = bindtarget( var->string );

		semaphore->progress = T_MAKE_SEMAPHORE;
		t->semaphore = semaphore;
	    }
	}
#endif

	/* Step 2d: reset "on target" variables */

	popsettings( s );
	freesettings( s );

	/* 
	 * Step 3: recursively make0bscan() dependents & headers
	 */

	/* Step 3a: recursively make0bscan() dependents */

	for( c = t->depends; c; c = c->next )
	    make0bscan( c->target );

	/* Step 3b: recursively make0() includes */

	for( c = t->includes; c; c = c->next )
	    make0bscan( c->target );
}

/*
 * make0includes() - propagate INCLUDES through the tree
 */

static void
make0includes( TARGET *t )
{
	TARGETS *c;
	SETTINGS *s;

	/* 
	 * Step 1: initialize
	 */

	if ( t->makeflags & T_MAKEFLAG_INCLUDES )
	    return;
	t->makeflags |= T_MAKEFLAG_INCLUDES;

	/* 
	 * Step 3: recursively make0includes() dependents & headers
	 */

	/* Step 3a: recursively make0includes() dependents */

	for( c = t->depends; c; c = c->next )
	    make0includes( c->target );

	/* Step 3b: recursively make0() internal includes node */

	for( c = t->includes; c; c = c->next )
	    make0includes( c->target );

	/*
	 * Step 2: gather all the INCLUDES of this target.
	 */
	make0gatherincludes(t);
}

/*
 * make0gatherincludes() - gather all of the target's internal nodes
 * (INCLUDES) nodes into a list of t's dependencies that are not in
 * the T_MAKE_DONE state.
 */


extern int gbPrintStuff;

static void
make0gatherincludes( TARGET *t )
{
	TARGETS *chain = 0;
	TARGETS *c;

//	printf("make0gatherincludes: %s\n", t->name);

//	gbPrintStuff = 1;

	/* Increment the gather generation.  Each generation
	 * represents a given target's INCLUDES list. */
	t->mark = ++mark_generation;

	/* Mark this target's includes as being in the current
	 * INCLUDES list */
	for(c = t->includes; c; c = c->next)
	{
	    c->target = bindtarget(c->target->name);
	    c->target->mark = mark_generation;
	}

	/* For each of the target's direct INCLUDES, go off and find
	 * the INCLUDES's INCLUDES, etc., recursively. */
	for(c = t->includes; c; c = c->next)
	{
	    chain = make0gatherincludes2(t->name, chain, c->target);
	}

	/* Append the newly found INCLUDES to this target's list of
	 * direct INCLUDES.  Now t->includes contains all INCLUDES
	 * targets, direct and indirect. */
	t->includes = targetchain(t->includes, chain);
}

static TARGETS*
make0gatherincludes2( const char* tname, TARGETS *chain, TARGET* t )
{
    TARGETS *c;

//	printf("----> %s\n", t->name);

    /* For each of this target's INCLUDES that aren't already part of
       this gather generation, append it to the chain and recurse. */
    for (c = t->includes; c; c = c->next)
    {
	c->target = bindtarget(c->target->name);
	if (c->target->mark != mark_generation)
	{
	    c->target->mark = mark_generation;
	    chain = targetentry(chain, c->target);

//	    chain = make0gatherincludes2(tname, chain, c->target);
	}
    }

    return chain;
}

/*
 make0sort() - reorder TARGETS chain by their time (newest to oldest)
 */

static TARGETS *
make0sort( TARGETS *chain )
{
	TARGETS *result = 0;

	/* We walk chain, taking each item and inserting it on the */
	/* sorted result, with newest items at the front.  This involves */
	/* updating each TARGETS' c->next and c->tail.  Note that we */
	/* make c->tail a valid prev pointer for every entry.  Normally, */
	/* it is only valid at the head, where prev == tail.  Note also */
	/* that while tail is a loop, next ends at the end of the chain. */

	/* Walk current target list */

	while( chain )
	{
	    TARGETS *c = chain;
	    TARGETS *s = result;

	    chain = chain->next;

	    /* Find point s in result for c */

	    while( s && s->target->time > c->target->time )
		s = s->next;

	    /* Insert c in front of s (might be 0). */
	    /* Don't even think of deciphering this. */

	    c->next = s;			/* good even if s = 0 */
	    if( result == s ) result = c;	/* new head of chain? */
	    if( !s ) s = result;		/* wrap to ensure a next */
	    if( result != c ) s->tail->next = c; /* not head? be prev's next */
	    c->tail = s->tail;			/* take on next's prev */
	    s->tail = c;			/* make next's prev us */
	}

	return result;
}

void adddirectdependent(const char **pTargets, int *pnTargets, const char *pszTargetName)
{
	if (*pnTargets > 1023)
		return;

	// don't add 'all'
	if (!strcmp(pszTargetName, "all"))
		return;

	// check for duplicates
	for (int i=0; i<*pnTargets; i++)
	{
		if (!strcmp(pTargets[i], pszTargetName))
			return;
	}

	if (*pnTargets == 1022)
	{
		printf("WARNING: More than 1024 dependent targets. Ignoring extra targets.\n");
	}

	// add new
	pTargets[*pnTargets] = _strdup(pszTargetName);
	(*pnTargets)++;
}

int fileextensionsarethesame(const char *p1, const char *p2)
{
	const char *pe1 = strrchr(p1, '.');
	const char *pe2 = strrchr(p2, '.');
	return pe1 && pe2 && !stricmp(pe1, pe2);
}

int makegetdirectdependents( TARGET *t, const char *pszSingleTarget, const char **ppTargets, int *pnTargets)
{
    TARGETS *c;

	// this will be set to 2 if a dependent is found, then decremented each time the file extension changes
	// when the file extension changes for the 2nd time, that's when we stop rebuilding stuff
	int directdependent = 0;
	int iFileExtensionChanges = 0;
	int iFileExtSame = 0;

	// this is a match for the single target we wanted to build
	if (arepathsthesame(t->name,pszSingleTarget) || (t->boundname && arepathsthesame(t->boundname, pszSingleTarget)))
		return 2;

    for (c = t->depends; c; c = c->next)
    {
		int x;
		c->target = bindtarget(c->target->name);
		x = makegetdirectdependents(c->target, pszSingleTarget, ppTargets, pnTargets);
		if (x > iFileExtensionChanges)
			iFileExtensionChanges = x;
		else
		{
			// if a direct dependent is copied about, then perform that action as well
			if (fileextensionsarethesame(c->target->name, t->name))
				iFileExtSame = 1;
		}
	}

	if (!iFileExtSame)
		iFileExtensionChanges--;

	if (iFileExtensionChanges>0)
		adddirectdependent(ppTargets, pnTargets, t->name);

	return iFileExtensionChanges;
}

#ifdef OPT_GRAPH_DEBUG_EXT

/*
 * dependGraphOutput() - output the DG after make0 has run
 */

static void
dependGraphOutputTimes( int time )
{
    printf( "(time:%d)\n", time );
}

static void
dependGraphOutput( TARGET *t, int depth )
{
    TARGETS	*c;
    TARGET	*include;

    if (   (t->flags & T_FLAG_VISITED) != 0
	|| !t->name
	|| !t->boundname)
	return;

    t->flags |= T_FLAG_VISITED;

    switch (t->fate)
    {
      case T_FATE_TOUCHED:
      case T_FATE_MISSING:
      case T_FATE_OUTDATED:
      case T_FATE_UPDATE:
	printf( "->" );
	break;
      default:
	printf( "  " );
	break;
    }

    printf( "%s%2d Name: %s\n", spaces(depth), depth, t->name );

    if( strcmp (t->name, t->boundname) )
    {
	printf( "  %s    Loc: %s\n", spaces(depth), t->boundname );
    }

    switch( t->fate )
    {
      case T_FATE_STABLE:
        printf( "  %s       : Stable\n", spaces(depth) );
        break;
      case T_FATE_NEWER:
        printf( "  %s       : Newer\n", spaces(depth) );
        break;
      case T_FATE_ISTMP:
        printf( "  %s       : Up to date temp file\n", spaces(depth) );
        break;
      case T_FATE_TOUCHED:
        printf( "  %s       : Been touched, updating it\n", spaces(depth) );
        break;
      case T_FATE_MISSING:
        printf( "  %s       : Missing, creating it\n", spaces(depth) );
        break;
      case T_FATE_OUTDATED:
        printf( "  %s       : Outdated, updating it\n", spaces(depth) );
        break;
      case T_FATE_UPDATE:
        printf( "  %s       : Updating it\n", spaces(depth) );
        break;
      case T_FATE_CANTFIND:
        printf( "  %s       : Can't find it\n", spaces(depth) );
        break;
      case T_FATE_CANTMAKE:
        printf( "  %s       : Can't make it\n", spaces(depth) );
        break;
    }

    printf( "  %s  Times: ", spaces(depth) );
    dependGraphOutputTimes( t->time );

    if( t->flags & ~T_FLAG_VISITED )
    {
	printf( "  %s       : ", spaces(depth) );
	if( t->flags & T_FLAG_TEMP ) printf ("TEMPORARY ");
	if( t->flags & T_FLAG_NOCARE ) printf ("NOCARE ");
	if( t->flags & T_FLAG_NOTFILE ) printf ("NOTFILE ");
	if( t->flags & T_FLAG_TOUCHED ) printf ("TOUCHED ");
	if( t->flags & T_FLAG_LEAVES ) printf ("LEAVES ");
	if( t->flags & T_FLAG_NOUPDATE ) printf ("NOUPDATE ");
	printf( "\n" );
    }

    for( c = t->depends; c; c = c->next )
    {
	printf( "  %s       : Depends on %s (%s) ", spaces(depth),
	       c->target->name, target_fate[ c->target->fate ] );
	dependGraphOutputTimes( c->target->time );
    }

    for( c = t->includes; c; c = c->next )
    {
	printf( "  %s       : Includes %s (%s) ", spaces(depth),
	       c->target->name, target_fate[ c->target->fate ] );
	dependGraphOutputTimes( c->target->time );
    }

    for( c = t->depends; c; c = c->next )
    {
	dependGraphOutput( c->target, depth + 1 );
    }

    for( c = t->includes; c; c = c->next )
    {
	dependGraphOutput( c->target, depth + 1 );
    }
}

#endif
