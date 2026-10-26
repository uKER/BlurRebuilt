/*
 * Copyright 1993, 1995 Christopher Seiwald.
 *
 * This file is part of Jam - see jam.c for Copyright information.
 */

/*
 * execunix.c - execute a shell script on UNIX/WinNT/OS2/AmigaOS
 *
 * If $(JAMSHELL) is defined, uses that to formulate execvp()/spawnvp().
 * The default is:
 *
 *	/bin/sh -c %		[ on UNIX/AmigaOS ]
 *	cmd.exe /c %		[ on OS2/WinNT ]
 *
 * Each word must be an individual element in a jam variable value.
 *
 * In $(JAMSHELL), % expands to the command string and ! expands to 
 * the slot number (starting at 1) for multiprocess (-j) invocations.
 * If $(JAMSHELL) doesn't include a %, it is tacked on as the last
 * argument.
 *
 * Don't just set JAMSHELL to /bin/sh or cmd.exe - it won't work!
 *
 * External routines:
 *	execcmd() - launch an async command execution
 * 	execwait() - wait and drive at most one execution completion
 *
 * Internal routines:
 *	onintr() - bump intr to note command interruption
 *
 * 04/08/94 (seiwald) - Coherent/386 support added.
 * 05/04/94 (seiwald) - async multiprocess interface
 * 01/22/95 (seiwald) - $(JAMSHELL) support
 * 06/02/97 (gsar)    - full async multiprocess support for Win32
 * 01/20/00 (seiwald) - Upgraded from K&R to ANSI C
 * 11/04/02 (seiwald) - const-ing for string literals
 * 12/27/02 (seiwald) - grist .bat file with pid for system uniqueness
 */

# include "jam.h"
# include "lists.h"
# include "execcmd.h"
# include <errno.h>
#include <string>
#include <iostream>

# ifdef USE_EXECUNIX

# ifdef OS_OS2
# define USE_EXECNT
# include <process.h>
# endif

# ifdef OS_NT 
# define USE_EXECNT
# include <process.h>
# define WIN32_LEAN_AND_MEAN
# include <windows.h>		/* do the ugly deed */
# define USE_MYWAIT
# endif

#include <string>

#include "redirect_output.h"

int intr;

static int cmdsrunning = 0;
static void (*istat)( int );

int g_iCurrentlyOutputtingProcess = -1;

static struct
{
	int		order;
	HANDLE	pid; /* on win32, a real process handle */
#ifdef OPT_SERIAL_OUTPUT_EXT
	void (*func)( const char* outputname, void *closure, int status );
#else
	void (*func)( void *closure, int status );
#endif
	void 	*closure;

	RedirectedProcess process;

	char command[1024];
	char friendly_name[1024];
# ifdef USE_EXECNT
	char	*tempfile;
# endif
# ifdef OPT_SERIAL_OUTPUT_EXT
	char	*outputFilename;
	int	outputFilenameUsed;
# endif

} cmdtab[ MAXJOBS ] = {{0}};

/*
 * execcmd() - launch an async command execution
 */

//#define ENABLE_BETTER_LOGGING
#ifdef OPT_SERIAL_OUTPUT_EXT
void
exec_init()
{
    char 	*tempdir;
    int		i;

# ifdef USE_EXECNT
    tempdir = "\\temp";
# else
    tempdir = "/tmp";
# endif

    if( getenv( "TEMP" ) )
	tempdir = getenv( "TEMP" );
    else if( getenv( "TMP" ) )
	tempdir = getenv( "TMP" );

    for( i = 0; i < globs.jobs; ++i )
    {
	cmdtab[ i ].outputFilename = (char*)malloc( strlen( tempdir ) + 32 );
	sprintf( cmdtab[ i ].outputFilename, "%s%cjam%dout%d", 
		tempdir, PATH_DELIM, getpid(), i );
    }
}

void
exec_done()
{
    int		i;

    for( i = 0; i < globs.jobs; ++i)
    {
	if ( cmdtab[ i ].outputFilenameUsed )
	    unlink( cmdtab[ i ].outputFilename );

/* # ifdef USE_EXECNT *//* CWM - need on unix too */
	if ( cmdtab[ i ].tempfile )
	    unlink( cmdtab[ i ].tempfile );
/* # endif *//* CWM */
    }
}
#endif

void deletetempfiles()
{
	for (int i=0; i<MAXJOBS; i++)
	{
		unlink( cmdtab[i].tempfile );
	}
}

void
execcmd( 
	char *string,
#ifdef OPT_SERIAL_OUTPUT_EXT
	void (*func)( const char* outputname, void *closure, int status ),
#else
	void (*func)( void *closure, int status ),
#endif
	void *closure,
	LIST *shell,
	char bConvertErrors)
{
	int pid;
	int slot;
	const char *argv[ MAXARGC + 1 ];	/* +1 for NULL */

	const char friendly_name[] = "friendlyname";

# ifdef USE_EXECNT
	char *p;
# endif

	/* Find a slot in the running commands table for this one. */

	for( slot = 0; slot < MAXJOBS; slot++ )
	    if( !cmdtab[ slot ].pid )
		break;

	std::string leftover = cmdtab[slot].process.GetOutput().c_str();
	if (leftover.length() > 0)
	{
		printf("LEFTOVER: (%s)\n", leftover.c_str());
	}

	if( slot == MAXJOBS )
	{
	    printf( "no slots for child!\n" );
	    exit( EXITBAD );
	}

# ifdef USE_EXECNT
	if( !cmdtab[ slot ].tempfile )
	{
	    char *tempdir;

	    if( !( tempdir = getenv( "TEMP" ) ) &&
		!( tempdir = getenv( "TMP" ) ) )
		    tempdir = "\\temp";

	    /* +32 is room for \jamXXXXXtSS.bat (at least) */

	    cmdtab[ slot ].tempfile = (char*)malloc( strlen( tempdir ) + 32 );

	    sprintf( cmdtab[ slot ].tempfile, "%s\\jam%dt%d.bat", 
				tempdir, GetCurrentProcessId(), slot );
	}

	/* Trim leading, ending white space */

	while( isspace( *string ) )
		++string;

	p = strchr( string, '\n' );

	while( p && isspace( *p ) )
		++p;

	// find instances of $(#) and replace them with the current job number
	char *psz = string;
	while (psz[0] && psz[1] && psz[2] && psz[3])
	{
		if (psz[0] == '$' && psz[1] == '$' && psz[2] == '!' && psz[3] == '!' )
		{
			char jobno[4];
			sprintf(jobno,"%d", slot+1);
			int n = strlen(jobno);
			memcpy(psz, jobno, n);
			strcpy(psz+n, psz+4);
		}
		psz++;
	}

	/* If multi line, or too long, or JAMSHELL is set, write to bat file. */
	/* Otherwise, exec directly. */
	/* Frankly, if it is a single long line I don't think the */
	/* command interpreter will do any better -- it will fail. */

	if( p && *p || strlen( string ) > MAXLINE || shell )
	{
	    FILE *f;

	    /* Write command to bat file. */
		for (int x=0; x<10; x++)
		{
		    f = fopen( cmdtab[ slot ].tempfile, "w" );
			if (f)
				break;
			Sleep(1000);
		}

		if (!f) {
		perror( "fopen" );
	        printf( "Could not open tempfile %s after 10 retries.\n",
			cmdtab[ slot ].tempfile );
		exit( EXITBAD );
	    }

	    fputs( string, f );
	    fclose( f );

	    string = cmdtab[ slot ].tempfile;
	}
# endif

	/* Forumulate argv */
	/* If shell was defined, be prepared for % and ! subs. */
	/* Otherwise, use stock /bin/sh (on unix) or cmd.exe (on NT). */

	if( shell )
	{
	    int i;
	    char jobno[4];
	    int gotpercent = 0;

	    sprintf( jobno, "%d", slot + 1 );

	    for( i = 0; shell && i < MAXARGC; i++, shell = list_next( shell ) )
	    {
		switch( shell->string[0] )
		{
		case '%':	argv[i] = string; gotpercent++; break;
		case '!':	argv[i] = jobno; break;
		default:	argv[i] = shell->string;
		}
		if( DEBUG_EXECCMD )
		    printf( "argv[%d] = '%s'\n", i, argv[i] );
	    }

	    if( !gotpercent )
		argv[i++] = string;

	    argv[i] = 0;
	}
	else
	{
	    argv[0] = "cmd.exe";
	    argv[1] = "/Q/C";		/* anything more is non-portable */

	    argv[2] = string;
	    argv[3] = 0;
	}

	/* Catch interrupts whenever commands are running. */
	cmdsrunning++;

	std::string cmdline = "";
	int i=0;
	while (true)
	{
		cmdline += argv[i];
		if (argv[i+1] == NULL)
			break;
		cmdline += " ";
		i++;
	}

	/* Start the command */
	cmdtab[slot].process.RunCommandlineRedirect(cmdline, bConvertErrors?true:false);

	/* Save the operation for execwait() to find. */
	strncpy(cmdtab[slot].command, cmdline.c_str(), 70);

	cmdtab[ slot ].pid = cmdtab[ slot ].process.GetHandle();
	cmdtab[ slot ].func = func;
	cmdtab[ slot ].closure = closure;
	static int iGlobalOrder = 0;
//	printf("%d = %s\n", iGlobalOrder, cmdline.c_str());
	cmdtab[ slot ].order = iGlobalOrder++;
	strcpy(cmdtab[ slot ].friendly_name, friendly_name);

	/* Wait until we're under the limit of concurrent commands. */
	/* Don't trust globs.jobs alone. */
	while( cmdsrunning >= MAXJOBS || cmdsrunning >= globs.jobs )
	    if( !execwait() )
		break;
}

void DripFeedOutput(const char *psz)
{
	printf("%s", psz);
	return;
	if (*psz == 0)
		return;
	bool bStop = false;
	while (1)
	{
		const char *pszStart = psz;
		// count out 5 lines of text
		int ncrlf = 0;
		while (1)
		{
			if (*psz == '\n')
			{
				ncrlf++;
				if (ncrlf>=5)
				{
					psz++;
					break;
				}
			}
			if (*psz == 0)
			{
				bStop = true;
				break;
			}
			psz++;
		}

		char buffer[1024*64];
		int n = psz-pszStart;
		memcpy(buffer, pszStart, n);
		buffer[n] = 0;

		printf("%s", buffer);
		fflush(stdout);

		if (bStop)
			return;

		Sleep(100);
	}
}

/*
 * execwait() - wait and drive at most one execution completion
 */

int FindOldestProcess()
{
	int iOldestProcess = -1;
	int iOldestOrder = 0xfffffff;

	for (int i=0; i<MAXJOBS; i++)
	{
		if (cmdtab[i].pid != NULL && cmdtab[i].order<iOldestOrder)
		{
			iOldestOrder = cmdtab[i].order;
			iOldestProcess = i;
		}
	}

	return iOldestProcess;
}

std::string g_OutputBuffer = "";

int
execwait()
{
	int i;
	int status, w;
	int rstat;

	/* Handle naive make1() which doesn't know if cmds are running. */
	if( !cmdsrunning )
	    return 0;

	int _iChildProcessThatExited = -1;

	while (true)
	{
		// collect handles to all currently running child processes
		HANDLE childHandles[MAXJOBS];
		int nCmdRunning = 0;
		for (int i=0; i<MAXJOBS; i++)
		{
			HANDLE h = cmdtab[i].pid;
			if (h != NULL)
			{
				childHandles[nCmdRunning] = h;
				nCmdRunning++;

				if (intr == 0)
				{
					cmdtab[i].process.ReadAndHandleOutput();
					if (!globs.inorderlogging)
					{
						printf("%s", cmdtab[i].process.GetOutput().c_str());
					}
				}
			}
		}
		if (nCmdRunning == 0)
			return 0;

		// wait for a bit to see if any child processes will exit
		DWORD r = WaitForMultipleObjects(nCmdRunning, childHandles, FALSE, 100);
		if (r != WAIT_TIMEOUT)
		{
			_iChildProcessThatExited = r - WAIT_OBJECT_0;
			break;
		}
	}

	// here when one of the child processes exited
	// collect handles to all currently running child processes
	int iChildProcessThatExited = -1;
	int iCmdRunning = 0;
	for (int i=0; i<MAXJOBS; i++)
	{
		HANDLE h = cmdtab[i].pid;
		if (h != NULL)
		{
			if (iCmdRunning == _iChildProcessThatExited)
			{
				iChildProcessThatExited = i;
				break;
			}
			iCmdRunning++;
		}
	}

	i = iChildProcessThatExited;
	if (i == -1)
	{
		printf("ERROR: CHILD PROCESS THAT EXITED == -1??!!\n");
	}

	// get any remaining output for the process that exited
	cmdtab[i].process.ReadAndHandleOutput();
	cmdtab[i].process.StopTiming();
	cmdtab[i].pid = NULL;

	if( i == -1 )
	{
	    printf( "waif child found!\n" );
	    exit( EXITBAD );
	}

	// if the oldest process exited, print the remaining text, plus any output from other processes that completed in the meantime
	if (globs.inorderlogging)
	{
		if (g_iCurrentlyOutputtingProcess == i || g_iCurrentlyOutputtingProcess == -1)
		{
			DripFeedOutput(cmdtab[i].process.GetOutput().c_str());
			DripFeedOutput(g_OutputBuffer.c_str());
			g_OutputBuffer = "";
			g_iCurrentlyOutputtingProcess = -1;
		}
		else
		{
			// if this is not the oldest process, buffer the output until the oldest process exits
			std::string tmp = cmdtab[i].process.GetOutput();
//			if (tmp.length() > 0)
//				printf("BUFFERING (%s)\n", tmp.c_str());
			g_OutputBuffer = g_OutputBuffer + tmp;
		}
	}
	else
	{
		printf("%s", cmdtab[i].process.GetOutput().c_str());
	}

	DWORD exitcode = cmdtab[i].process.Close();
	cmdsrunning--;

# ifdef USE_EXECNT
	/* Clear the temp file */

	unlink( cmdtab[ i ].tempfile );
# endif

	if (intr)
		rstat = EXEC_CMD_INTR;
	else
	{
		switch (exitcode)
		{
		case 0:				rstat = EXEC_CMD_OK; break;
		case 0xc000013a:	rstat = EXEC_CMD_INTR; break;
		default:			rstat = EXEC_CMD_FAIL; break;
		}
	}

	// if a task failed, dump it's output and then exit
	if (rstat == EXEC_CMD_FAIL)
	{
		DripFeedOutput(g_OutputBuffer.c_str());
	}

	cmdtab[ i ].pid = 0;

	(*cmdtab[ i ].func)( cmdtab[ i ].closure, rstat );

	return 1;
}

# endif /* USE_EXECUNIX */
