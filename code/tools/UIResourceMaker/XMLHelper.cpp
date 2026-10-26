#include "stdafx.h"
#include "XMLHelper.h"
#include "ui.h"

XMLHelper gXMLHelper;

void XMLHelper::Init()
{
	int a=0;
	int b=0;
	bool store_keyword=true;
	TCHAR c;
	FILE *fp;
	char filename[128];
	sprintf(filename,"%s\\help.txt",gUI.Config.UIMakerDir);

	fp=fopen(filename,"rb");

	if(fp)
	{
		c=fgetc(fp);
		while(c != EOF)
		{	
			if(c == '=')
			{
				a=0;
				c=fgetc(fp);
				store_keyword=false;
			}

			if(c == 0x0d) //EOL
			{
				b++;
				a=0;
				store_keyword=true;
				c=fgetc(fp);		//clear line break
				c=fgetc(fp);		// get next character
			}

			if(store_keyword) help_keyword[b][a]=c;
			else help_text[b][a]=c;

			c=fgetc(fp);
			a++;
		}

		number_of_help=b;

		fclose(fp);
	}
	else
	{
		AfxMessageBox("Error Reading Help File",MB_OK|MB_ICONINFORMATION);
	}

	sprintf(filename,"%s\\templates.txt",gUI.Config.UIMakerDir);
	fp=fopen(filename,"rb");

	if(fp)
	{
		a=0;
		c=fgetc(fp);
		c=fgetc(fp);
		while(c != EOF)
		{
			char string[32];

			a=0;
			while(c != '#')
			{
				string[a]=c;
				c=fgetc(fp);
				a++;
			}

			string[a]='\0';

			if(!strcmp(string,"STATICGRAPHIC")) b=HELP_TEMPLATES_STATICGRAPHIC;
			if(!strcmp(string,"STATICLINE")) b=HELP_TEMPLATES_STATICLINE;
			if(!strcmp(string,"STATICTEXT")) b=HELP_TEMPLATES_STATICTEXT;
			if(!strcmp(string,"STATICMODEL")) b=HELP_TEMPLATES_STATICMODEL;
			if(!strcmp(string,"TIMER")) b=HELP_TEMPLATES_TIMER;

			if(!strcmp(string,"VARIABLESELECT")) b=HELP_TEMPLATES_VARIABLESELECT;
			if(!strcmp(string,"VARIABLESELECT_HORIZ")) b=HELP_TEMPLATES_VARIABLESELECT_HORIZ;
			if(!strcmp(string,"VARIABLESELECT_VERT")) b=HELP_TEMPLATES_VARIABLESELECT_VERT;
			if(!strcmp(string,"VARIABLESELECT_RADIAL")) b=HELP_TEMPLATES_VARIABLESELECT_RADIAL;

			if(!strcmp(string,"EFFECTMOVE")) b=HELP_TEMPLATES_EFFECTMOVE;
			if(!strcmp(string,"EFFECTSIZE")) b=HELP_TEMPLATES_EFFECTSIZE;
			if(!strcmp(string,"EFFECTCOLOUR")) b=HELP_TEMPLATES_EFFECTCOLOUR;
			if(!strcmp(string,"EFFECTTEXTURESIZE")) b=HELP_TEMPLATES_EFFECTTEXTURESIZE;
			if(!strcmp(string,"EFFECTTEXTUREMOVE")) b=HELP_TEMPLATES_EFFECTTEXTUREMOVE;
			if(!strcmp(string,"EFFECTBLUR")) b=HELP_TEMPLATES_EFFECTBLUR;
			if(!strcmp(string,"EFFECTROTATE")) b=HELP_TEMPLATES_EFFECTROTATE;

			c=fgetc(fp);
			a=0;

			while(c != '#')
			{
				template_text[b][a]=c;
				c=fgetc(fp);
				a++;
			}

			c=fgetc(fp);
		}

		fclose(fp);
	}
	else
	{
		AfxMessageBox("Error Reading Templates File",MB_OK|MB_ICONINFORMATION);
	}
}

TCHAR *XMLHelper::GetCurrentStringHelp()
{
	for(int a=0;a<number_of_help;a++)
	{
		if(!strcmp(help_keyword[a],current_string))
		{
			return help_text[a];
		}
	}

	return NULL;
}

bool XMLHelper::DoHelp(CRichEditCtrl *control,int orig_charpos)
{
	int a=0;

	control->SetRedraw(false);

	int nTextLength = control->GetTextLength();
	control->SetSel(0,-1);
	
	control->GetSelText(buffer);

	bool do_variable=false;
	bool finished=false;
	
	while(buffer[a] && !finished)
	{
		if(buffer[a] == '"')
		{
			do_variable=!do_variable;
		}

		if(orig_charpos == a)
		{
			if(do_variable || buffer[a] == '"')
			{
				int b=a;
				int c=0;
				char string[128];

				while(buffer[b] != 0x20 && b)
				{
					b--;
				}

				b++;

				while(buffer[b] != '=')
				{
					string[c]=buffer[b];
					c++;
					b++;
				}

				string[c]='\0';
				strcpy(current_string,string);

				c=0;
				b+=2;
				while(buffer[b] != '"')
				{
					string[c]=buffer[b];
					c++;
					b++;
				}

				string[c]='\0';
				strcpy(current_value,string);

				return true;
			}

			finished=true;
		}

		a++;
	}

	return false;
}