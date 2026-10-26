#include "stdafx.h"
#include "XMLColouriser.h"
#include "ui.h"

XMLColouriser gXMLColouriser;

#define MAX_KEYWORDS  9
char keywords[MAX_KEYWORDS][64] = {"STATICGRAPHIC","STATICLINE","VARIABLESELECT","STATICTEXT","STATICMOVIE","STATICMODEL","FONT","SETVARIABLE","ENVIRONMENT"};

void XMLColouriser::Colourise(CRichEditCtrl *control,int orig_charpos)
{
	CHARFORMAT cf;
	int a=0;

	control->SetRedraw(false);

	int nTextLength = control->GetTextLength();
	control->SetSel(0,-1);
	
	control->GetSelText(buffer);

	cf.dwMask = CFM_BOLD|CFM_COLOR|CFM_FACE;
	cf.dwEffects = CFE_BOLD;
	cf.crTextColor = RGB(0,0,0);
	strcpy(cf.szFaceName,"Times New Roman");
	control->SetSelectionCharFormat(cf);

	return;

	bool do_variable=false;
	bool do_keyword=false;
	bool do_comment=false;

	while(buffer[a])
	{
		/*
		if(buffer[a] == '"')
		{
			do_variable=!do_variable;
		}

		if(buffer[a] == '<')
		{
			char temp[64];
			int b=1;		//ignore the first <

			while(buffer[a+b] != 0x20 && buffer[a+b] != '>')
			{
				temp[b-1]=buffer[a+b];
				b++;
			}

			temp[b-1]='\0';

			for(int c=0;c<MAX_KEYWORDS;c++)
			{
				if(!strcmp(temp,keywords[c]))
				{
					do_keyword=true;
				}
			}
		}

		if(buffer[a] == '<' && buffer[a+1] == '!' && buffer[a+2] == '-' && buffer[a+3] == '-')
		{
			do_comment=true;
		}

		if(buffer[a] == '-' && buffer[a+1] == '-' && buffer[a+2] == '>')
		{
			do_comment=false;
		}

		if(do_keyword)
		{
			cf.dwMask = CFM_BOLD|CFM_COLOR|CFM_FACE;
			cf.dwEffects = CFE_BOLD;
			cf.crTextColor = RGB(gUI.Config.KeywordTextR,gUI.Config.KeywordTextG,gUI.Config.KeywordTextB);
			strcpy(cf.szFaceName,"Times New Roman");
			control->SetSel(a,a+1);
			control->SetSelectionCharFormat(cf);

			if(buffer[a] == 0x20 || buffer[a] == '>') do_keyword=false;
		}

		if(do_variable || buffer[a] == '"')
		{
			cf.dwMask = CFM_BOLD|CFM_COLOR|CFM_FACE;
			cf.dwEffects = CFE_BOLD;
			cf.crTextColor = RGB(gUI.Config.VariableTextR,gUI.Config.VariableTextG,gUI.Config.VariableTextB);
			strcpy(cf.szFaceName,"Times New Roman");
			control->SetSel(a,a+1);
			control->SetSelectionCharFormat(cf);

			if(orig_charpos == a)
			{
				int b=a;
				int c=0;
				char string[256];

				while(buffer[b] != 0x20 && b)
				{
					b--;
				}

				while(buffer[b] != '=')
				{
					string[c]=buffer[b];
					c++;
					b++;
				}

				string[c]='\0';

				if(!strcmp(string,"Name"))
				{

				}
			}
		}

		if(do_comment)
		{
			cf.dwMask = CFM_BOLD|CFM_COLOR|CFM_FACE;
			cf.dwEffects = CFE_BOLD;
			cf.crTextColor = RGB(gUI.Config.CommentTextR,gUI.Config.CommentTextG,gUI.Config.CommentTextB);
			strcpy(cf.szFaceName,"Times New Roman");
			control->SetSel(a,a+1);
			control->SetSelectionCharFormat(cf);
		}
		*/

		a++;
	}
}