#include "w32_getperforceworkspace.h"
#include "w32_findperforce.h"

#include "jam.h"

#include <stdio.h>

#define MAX_FILE_SIZE 512

char g_fileBuffer[MAX_FILE_SIZE];
char* g_workspaceName=NULL;

const char* get_perforce_workspace()
{
	FILE* f;
	char* nextToken;
	char* token;

	if (g_workspaceName!=NULL)
		return g_workspaceName;

	if (is_perforce_enabled())
	{
		char delimiters[4];

		delimiters[0]='=';
		delimiters[1]=13;
		delimiters[2]=10;
		delimiters[3]=0;

		f = fopen(get_perforce_config_filename(), "r");
		fread(g_fileBuffer, MAX_FILE_SIZE, 1, f);
		fclose(f);

		token=strtok_s(g_fileBuffer, delimiters, &nextToken);
		while(token)
		{
			if (strcmp(token, "P4CLIENT")==0)
			{
				g_workspaceName=strtok_s(NULL,delimiters, &nextToken);
				break;
			}
			token=strtok_s(NULL,delimiters, &nextToken);
		}
		return g_workspaceName;
	}

	return NULL;
}
