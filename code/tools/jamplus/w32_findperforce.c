#include "w32_findperforce.h"

#include "jam.h"
#include "lists.h"
#include "w32_getreg.h"
#include "newstr.h"
#include "filesys.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>



char *g_pszP4Cfg = NULL;
int g_iP4Found = 0;
int g_iFilesFound = 0;

static void
findp4callback( 
	void		*closure,
	const char	*target,
	int		found,
	time_t		time )
{
	if (!strcmp(g_pszP4Cfg, target))
	{
		g_iP4Found = 1;
		return;
	}
}

char g_p4ConfigFilename[1024]="";
int g_isPerforceEnabledCalculated=0;
int g_isPerforceEnabled=0;

int is_perforce_enabled()
{
	FILE *f;
	int i;

	if (g_isPerforceEnabledCalculated)
		return g_isPerforceEnabled;

	g_isPerforceEnabledCalculated=1;

	// get perforce environment item
	g_pszP4Cfg = getenv("P4CONFIG");
	if (!g_pszP4Cfg)
		return 0;

	strcpy(g_p4ConfigFilename, ".");
	for (i=0; i<10; i++)
	{
		char buffer2[1024];
		sprintf(buffer2, "%s/%s", g_p4ConfigFilename, g_pszP4Cfg);
		f = fopen(buffer2, "r");
		if (f)
		{
			fclose(f);
			g_isPerforceEnabled=1;
			strcpy(g_p4ConfigFilename, buffer2);
			return 1;
		}
		strcat(g_p4ConfigFilename, "/..");
	}

	g_isPerforceEnabled=0;
	return 0;
}

const char* get_perforce_config_filename()
{
	if (!g_isPerforceEnabledCalculated)
		is_perforce_enabled();

	return g_p4ConfigFilename;
}
