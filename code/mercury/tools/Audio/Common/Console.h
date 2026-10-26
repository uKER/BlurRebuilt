#if !defined(__CONSOLE_H_)
#define __CONSOLE_H_


class CConsole
{
public:
	CConsole() 
	{ 
		hConsole = NULL; 
	}
	~CConsole() 
	{ 
		Close();
	};

public:
	// create the console
	bool   Create(const char* szTitle, bool bNoClose = false);
	
	// get colour of current output
	WORD   GetColor();
	// set color for output
	void   SetColor(WORD wColor = NULL);
	// write output to console
	void   Output(const char* szOutput = NULL, ...);
	
	// set and get title of console
	void   SetTitle(const char* szTitle);
	char*  GetTitle();

	// get HWND and/or HANDLE of console
	HWND   GetHWND();
	HANDLE GetHandle();

	// show/hide the console
	void   Show(bool bShow = true);
	// disable the [x] button of the console
	void   DisableClose();
	// clear all output
	void   Clear();

	// close the console and delete it
	void   Close();

private:
	HANDLE hConsole;
};

extern CConsole gConsole;

#endif 
