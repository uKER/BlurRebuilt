/*
 * Copyright 1993, 1995 Christopher Seiwald.
 *
 * This file is part of Jam - see jam.c for Copyright information.
 */

/*
 * command.c - maintain lists of commands
 *
 * 01/20/00 (seiwald) - Upgraded from K&R to ANSI C
 * 09/08/00 (seiwald) - bulletproof PIECEMEAL size computation
 */

# include "jam.h"

# include "lists.h"
# include "parse.h"
# include "variable.h"
# include "rules.h"
#ifdef OPT_RESPONSE_FILES
# include "expand.h"
# include "tmpfile.h"
#endif

# include "command.h"

#ifdef OPT_RESPONSE_FILES
static int cmd_string( RULE* rule, char *out, int outsize,
		       LOL *lol, TMPLIST **response_files);
#endif
/*
 * cmd_new() - return a new CMD or 0 if too many args
 */

CMD *
cmd_new(
	RULE	*rule,
	LIST	*targets,
	LIST	*sources,
	LIST	*shell,
	int	maxline )
{
	CMD *cmd = (CMD *)malloc( sizeof( CMD ) );

	cmd->rule = rule;
	cmd->shell = shell;
	cmd->next = 0;
#ifdef OPT_RESPONSE_FILES
	cmd->response_files = 0;
#endif

	lol_init( &cmd->args );
	lol_add( &cmd->args, targets );
	lol_add( &cmd->args, sources );

	/* Bail if the result won't fit in maxline */
	/* We don't free targets/sources/shell if bailing. */

#ifdef OPT_RESPONSE_FILES
    	if( cmd_string( rule, cmd->buf, maxline, &cmd->args,
			&cmd->response_files ) < 0 )
#else
	if( var_string( rule->actions, cmd->buf, maxline, &cmd->args ) < 0 )
#endif
	{
	    cmd_free( cmd );
	    return 0;
	}
#ifdef OPT_PIECEMEAL_PUNT_EXT
        /* if the command was too long and we can possibly make it
           shorter, try.  Otherwise hope for the best. */
        if ( strlen(cmd->buf) > maxline && rule->flags & RULE_PIECEMEAL )
        {
			printf("too big: %s\n", cmd->buf);
            cmd_free( cmd );
            return 0;
        }
#endif

	return cmd;
}

/*
 * cmd_free() - free a CMD
 */

void
cmd_free( CMD *cmd )
{
	lol_free( &cmd->args );
	list_free( cmd->shell );
#ifdef OPT_RESPONSE_FILES
	while( cmd->response_files )
	{
	    TMPLIST *t;

	    t = cmd->response_files;
	    cmd->response_files = t->next;
	    tmp_release( t->file );
	    free( t );
	}
#endif
	free( (char *)cmd );
}

#ifdef OPT_RESPONSE_FILES
/*
 * cmd_string() - expand a string with the magic 'expand to file'
 * syntax, replacing the syntax with the names of temporary files.
 *
 * Copies in to out; doesn't modify targets & sources.  Returns -1
 * when out is too small.
 *
 * Adds files created into the response_files linked list.  The caller
 * is responsible for deletion, even in failure conditions.
 */
static int
cmd_string(
    RULE     *rule,
    char     *out,
    int	      outsize,
    LOL	     *lol,
    TMPLIST **response_files)
{
    const char  *in   = rule->actions;
    char    	*out0 = out;
    char    	*oute = out + outsize - 1;

    while (*in && out < oute) {
	int 	dollar = 0;
	char* 	lastword;

	/* Copy white space */

	while (isspace(*in)) {
	    if (out >= oute) {
		return -1;
	    }
	    *out++ = *in++;
	}

	lastword = out;
	
	/* Copy non-white space, watching for variables and optionally
	 * for response file indicators. */

	while (*in && !isspace(*in)) {
	    if (out >= oute) {
		return -1;
	    }

	    if (in[0] == '$' && in[1] == '(') {
		dollar++;
	    } else if ((rule->flags & RULE_RESPONSE)
		       && in[0] == '@' && in[1] == '(') {
		const char *ine;
		int depth;
		TMPLIST *r;
		int tlen;

		r = (TMPLIST*)malloc(sizeof(*r));
		r->next = *response_files;
		*response_files = r;
		r->file = tmp_new(0);
		if (!r->file)
		{
		    printf("jam: Could not create temporary file\n");
		    exit(EXITBAD);
		}

		tlen = strlen(r->file->name);
		if (out + tlen >= oute) {
		    /* no room for file name + trailing '\0' */
		    return -1;
		}
		memcpy(out, r->file->name, tlen);
		out += tlen;

		ine = in + 2;
		depth = 1;
		while (*ine && depth > 0) {
		    switch (*ine) {
			case '(':
			    ++depth;
			    break;
			case ')':
			    --depth;
			    break;
		    }
		    ++ine;
		}
		if (depth == 0) {
		    char save;
		    char* buffer;
		    int bufferSize;
		    int expandedSize;

		    save = ine[-1];
		    ((char*)ine)[-1] = '\0';

		    bufferSize = CMDBUF * 4;
		    buffer = (char*)malloc(bufferSize);
		    if (!buffer) {
			printf("jam: out of memory");
			exit(EXITBAD);
		    }
		    while (0 > (expandedSize = var_string(
				     in + 2, buffer, bufferSize, lol))) {
			bufferSize *= 2;
			buffer = (char*)realloc(buffer, bufferSize);
			if (!buffer) {
			    printf("jam: out of memory");
			    exit(EXITBAD);
			}
		    }

		    ((char*)ine)[-1] = save;

		    tmp_write(r->file, buffer, expandedSize - 1);
		    free(buffer);

		    if (!tmp_flush(r->file)) {
			printf("jam: I/O error on temporary file\n");
			exit(EXITBAD);
		    }
		    tmp_close(r->file);
		}
		in = ine;
		break;
	    }

	    *out++ = *in++;
	}

	if (dollar) {
	    LIST *l = var_expand(L0, lastword, out, lol, 0);
	    LIST *head = l;

	    out = lastword;

	    while (l)
	    {
		int so = strlen(l->string);

		if (out + so >= oute) {
		    return -1;
		}

		memcpy(out, l->string, so);
		out += so;

		/* Separate with space */

		l = list_next(l);
		if (l) {
		    *out++ = ' ';
		}
	    }
	    list_free(head);
	}
    }

    if (out >= oute)
	return -1;
    *out++ = '\0';
    return out - out0;
}
#endif
