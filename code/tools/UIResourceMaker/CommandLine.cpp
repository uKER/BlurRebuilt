#include "stdafx.h"
#include "commandline.h"
#include "xmlparser.h"

CommandLine gCommandLine;

void CommandLine::Update()
{
	Blink^=1;
}

void CommandLine::Render(CDC *cdc) const
{
	for(int a=0;a<num_lines;a++)
	{
		switch(stringcolour[a])
		{
			case TC_BLACK:
				cdc->SetTextColor(RGB(0,0,0));
				break;
			case TC_RED:
				cdc->SetTextColor(RGB(255,0,0));
				break;
			case TC_GREY:
				cdc->SetTextColor(RGB(192,192,192));
				break;
			case TC_BLUE:
				cdc->SetTextColor(RGB(0,0,255));
				break;
			default:
				cdc->SetTextColor(RGB(0,0,0));
				break;
		}
	
		cdc->TextOut(10,(10+(a*text_space)),strings[a],(int)strlen(strings[a]));
	}
}

void CommandLine::RenderCommandLine(HDC hdc) const
{
	SetTextColor(hdc,RGB(0,0,0));
	if(Blink)
	{
		TextOut(hdc,10,10,command_string,(int)strlen(command_string));
	}
	else
	{
		char text[129];
		sprintf(text,"%s_",command_string);
		TextOut(hdc,10,10,text,(int)strlen(text));
	}
}

void CommandLine::AddLine(TEXT_COLOUR col,char *text)
{
	strcpy(strings[num_lines],text);
	stringcolour[num_lines]=col;
	num_lines++;
}

void CommandLine::AddLineArg(TEXT_COLOUR col,char *text,...)
{
	static char string[256];

	va_list arglist;

	va_start(arglist, text);
	vsprintf(string, text, arglist);
	va_end(arglist);

	strcpy(strings[num_lines],string);
	stringcolour[num_lines]=col;

	num_lines++;
}

void CommandLine::GetParam(int start,char *temp_str)
{
	int cur=start;
	int cur1=0;

	while(command_string[cur])
	{
		temp_str[cur1]=command_string[cur];

		cur++;
		cur1++;
	}

	temp_str[cur1]=NULL;
}

void CommandLine::AddCharacter(char c)
{

}