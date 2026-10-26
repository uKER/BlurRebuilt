#pragma once

#define MAX_COMMAND_LINES 4098

enum TEXT_COLOUR
{
	TC_BLACK,
	TC_RED,
	TC_GREY,
	TC_BLUE,
};

class CommandLine
{
public:
	CommandLine(){num_lines=0;text_space=16;command_string_cursor=0;};

	void Update();
	void Render(CDC *cdc) const;
	void RenderCommandLine(HDC hdc) const;
	void AddLine(TEXT_COLOUR col,char *text);
	void AddLineArg(TEXT_COLOUR col,char *text,...);

	void AddCharacter(char c);

	void SetTextSpace(int value){text_space=value;};
	int GetTextSpace(){return text_space;};

	void GetParam(int start,char *temp_str);

	int GetSize(){return (10+(num_lines*text_space));};

	void Clear(){num_lines=0;};

protected:
	char strings[MAX_COMMAND_LINES][512];
	TEXT_COLOUR stringcolour[MAX_COMMAND_LINES];

	int scroll_pos;
	int num_lines;

	int text_space;

	char command_string[128];
	int command_string_cursor;

	int Blink;
};

extern CommandLine gCommandLine;
