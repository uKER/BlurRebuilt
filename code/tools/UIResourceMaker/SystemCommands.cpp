#include "stdafx.h"
#include "config.h"
#include "SystemCommands.h"

SystemCommands gSystemCommands;

void SystemCommands::AddCommand(char *comm)
{
	if(strlen(comm) > 255) 
	{
		ASSERT(0);
	}

	strcpy(mCommands[mNumCommands],comm);
	mNumCommands++;

	ASSERT(mNumCommands < MAX_SYSTEM_COMMANDS);
}

void SystemCommands::Execute()
{
	FILE *fp;
	char name[256];

	sprintf(name,"runit.bat");
	fp=fopen(name,"w+");

	fprintf(fp,"call \"C:\\Program Files\\Microsoft Xbox 360 SDK\\bin\\win32\\xdkvars.bat\"\n");

	if(fp)
	{
		for(int a=0;a<mNumCommands;a++)
		{
			fprintf(fp,"%s\n",mCommands[a]);
		}

		fclose(fp);

		system(name);

		DeleteFile("runit.bat");
	}
	else
	{
		AfxMessageBox("Error Writing System Command File",MB_OK|MB_ICONINFORMATION);
	}

	mNumCommands=0;
}