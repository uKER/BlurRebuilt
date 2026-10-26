
#include "stdafx.h"
#include "app.h"

static char *ConfigName="XenWatch.ini";

static char *LabelCheck(char *string,char *label)
{
	int len=0;
	len=(int)strlen(label);
	
	if (strncmp(string,label,len)!=0) return NULL; // Doesn't match
	
	return string+len;
}

int ConfigLoad()
{
	char line[256]="";
	FILE *f=NULL;
	
	memset(line,0,sizeof(line));
	
	f=fopen(ConfigName,"rt"); if (f==NULL) return 1;
	
	// Go through each line:
	for (;;)
	{
		int len=0;
		if (fgets(line,sizeof(line)-1,f)==NULL) break; // End of config file
		
		len=(int)strcspn(line,"\r\n");
		line[len]=0;

#define STR(x) { char *val=LabelCheck(line,#x "="); if (val) strcpy(x,val); }
	STR(MainPath)
#undef STR
	}
	
	fclose(f);
	
	return 0;
}

// Save variables to config file
int ConfigSave()
{
	FILE *f=NULL;
	
	f=fopen(ConfigName,"wt"); if (f==NULL) return 1;
	
	fprintf(f,"// %s - config file\n\n",ConfigName);

#define STR(x) fprintf(f,#x "=%s\n",x);	
	STR(MainPath)
#undef STR
	
	fclose(f);
	
	return 0;
}
