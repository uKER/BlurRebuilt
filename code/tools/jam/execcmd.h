/*
 * Copyright 1993, 1995 Christopher Seiwald.
 *
 * This file is part of Jam - see jam.c for Copyright information.
 */

/*
 * execcmd.h - execute a shell script
 *
 * 05/04/94 (seiwald) - async multiprocess interface
 */

void execcmd(
	char *string,
#ifdef OPT_SERIAL_OUTPUT_EXT
	void (*func)( const char* outputname, void *closure, int status ),
#else
	void (*func)( void *closure, int status ),
#endif
	void *closure,
	LIST *shell,
	bool bConvertErrors,
	char *friendly_name);

int execwait();

# define EXEC_CMD_OK	0
# define EXEC_CMD_FAIL	1
# define EXEC_CMD_INTR	2
