#include "stdafx.h"
//#include "AppHeaders.h"
#include "ErrorReport.h"

bool bFirstError=true;

const int DEBUG_BUF_SIZE=256;
char tmp_str[DEBUG_BUF_SIZE];
bool bLastPrintHadCarriageReturn=false;

// function pointers to override the string printing functions with your own
FuncErrorReport customErrorReportFunc = 0;
void* pErrorReportContext = 0;
FuncErrorReport customWarningReportFunc = 0;
void* pWarningReportContext = 0;

//-----------------------------------------------------------------------------
// Name: SetErrorReportCallback
// Desc: override function for printing errors.
//		 context is passed in case you need to pass a class or other data
//-----------------------------------------------------------------------------
void SetErrorReportCallback(FuncErrorReport funcPtr, void* pContext)
{
	customErrorReportFunc = funcPtr;
	pErrorReportContext = pContext;
}

//-----------------------------------------------------------------------------
// Name: SetWarningReportCallback
// Desc: override function for printing warnings
//-----------------------------------------------------------------------------
void SetWarningReportCallback(FuncErrorReport funcPtr, void* pContext)
{
	customWarningReportFunc = funcPtr;
	pWarningReportContext = pContext;
}

//-----------------------------------------------------------------------------
// Name: PrintError
// Desc: Print error to command line with colour highlighting
//		 
//-----------------------------------------------------------------------------
int PrintError(const char *format, ...)
{
	if (!gConsole.GetHandle())
		gConsole.Create(NULL);

	if (format == NULL)
		return 1;

	// save the old colours
	WORD wOldColours = gConsole.GetColor();

	va_list val=NULL;

	va_start(val,format);

#ifdef AUDIO_TOOLS_CMD_LINE
//	if (bFirstError==true)
#endif
	{
		//gConsole.Create();

		// IDEA. HAVE FLAG FOR IF LAST LINE ENDED WITH A \n. IF SO DONT DO THIS!?
		if (bLastPrintHadCarriageReturn==false)
			printf("\n");
		bFirstError = false;
	}

	_vsnprintf(tmp_str, DEBUG_BUF_SIZE, format, val);

	bLastPrintHadCarriageReturn = tmp_str[strlen(tmp_str)-1] == '\n' ? true : false;

	if (customErrorReportFunc)
	{
		customErrorReportFunc(tmp_str, pErrorReportContext);
	}
	else
	{
		gConsole.SetColor(TXTCOLOUR_RED);
		gConsole.Output("ERROR : ");
		OutputDebugString("ERROR : ");

		gConsole.Output(tmp_str);
		OutputDebugString(tmp_str);
	}
	
	va_end(val);

	// restore old colours
	gConsole.SetColor(wOldColours);

	// pause on errors. only if they supress the main pause at the end of wavebundler operation.
	// this way they can never miss an error as they always either get the main pause or this one.
	#ifdef AUDIO_TOOLS_CMD_LINE
	if (!gSettings.gbDoSystemPause)
		system("pause");
	#endif

	return 0;
}

//-----------------------------------------------------------------------------
// Name: PrintWarning
// Desc: Print warning to command line with colour highlighting
//-----------------------------------------------------------------------------
int PrintWarning(const char *format, ...)
{
	if (!gConsole.GetHandle())
		gConsole.Create(NULL);

	// save the old colours
	WORD wOldColours = gConsole.GetColor();

	va_list val=NULL;

	va_start(val,format);

#ifdef AUDIO_TOOLS_CMD_LINE
//	if (bFirstError==true)
#endif
	{
		//gConsole.Create();
		if (bLastPrintHadCarriageReturn==false)
			printf("\n");
		bFirstError = false;
	}
	gConsole.SetColor(TXTCOLOUR_YELLOW);
	//gConsole.Output("\n");
	gConsole.Output("WARNING : ");
	OutputDebugString("WARNING : ");

	_vsnprintf(tmp_str, DEBUG_BUF_SIZE, format, val);

	bLastPrintHadCarriageReturn = tmp_str[strlen(tmp_str)-1] == '\n' ? true : false;

	if (customWarningReportFunc)
	{
		customWarningReportFunc(tmp_str, pWarningReportContext);
		//customWarningReportFunc("\n", pWarningReportContext);
	}
	else
	{
		gConsole.Output(tmp_str);
		//gConsole.Output("\n");
		OutputDebugString(tmp_str);
	}

	
	va_end(val);

	// restore old colours
	gConsole.SetColor(wOldColours);

	//system("pause");

	return 0;
}
