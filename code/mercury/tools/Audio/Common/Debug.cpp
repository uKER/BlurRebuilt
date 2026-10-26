#include "stdafx.h"

#include "console.h"
#include "ErrorReport.h"

const int DEBUG_BUF_SIZE=512;

char db_tmp_str[DEBUG_BUF_SIZE];
namespace Audio
{

//-----------------------------------------------------------------------------
// Name: LogSubscriber
// Desc: class to pipe mercury log prints to the PrintError function
//		 this class subscribes itself by declaring an instance of itself.
//		 [if this is not desired we can make a SubscribeToMercuryLogger() func
//		 and call that instead?]
//-----------------------------------------------------------------------------
class LogSubscriber : public Debug::SubscriberCommon
{
public:
	LogSubscriber() 
	{
		static bool bSubscribed = false;
		if (!bSubscribed)
		{
			bSubscribed = true;
			LogSubscriber *subscriber = new LogSubscriber();
			Debug::GetRootLogger().AddSubscriber( subscriber );
		}
	}
	void		LogString(const Util::C_String &string) { PrintError(string.Chars()); }
};

LogSubscriber theSubscriber;

// function pointers to override the string printing functions with your own
FuncPrintString gCustomPrintStringFunc = 0;
void* gpPrintStringContext = 0;

//-----------------------------------------------------------------------------
// Name: SetPrintCallback
// Desc: override function for printing errors
//-----------------------------------------------------------------------------
void SetPrintStringCallback(FuncPrintString funcPtr, void* pContext)
{
	gCustomPrintStringFunc = funcPtr;
	gpPrintStringContext = pContext;
}

int Print(U32 uAttribs, const char *format, ...)
{
	if (!gConsole.GetHandle())
		gConsole.Create(NULL);

	WORD wOldColours;
	if (uAttribs)
	{
		// save the old colours
		wOldColours = gConsole.GetColor();
		gConsole.SetColor(uAttribs);
	}

	va_list val=NULL;

#ifdef USE_FILE
	char *name=NULL;

	name="d:\\log.txt";

	if (log_file==NULL) 
		log_file=fopen(name,"wt");
	if (log_file==NULL) 
		return 1;
#endif

	va_start(val,format);

	vsnprintf_s(db_tmp_str, sizeof(db_tmp_str), DEBUG_BUF_SIZE, format, val);

	bLastPrintHadCarriageReturn = db_tmp_str[strlen(db_tmp_str)-1] == '\n' ? true : false;

	if (!gCustomPrintStringFunc)
		OutputDebugStringA(db_tmp_str);

#ifdef USE_FILE
	vfprintf(log_file, format, val);
	fprintf(log_file, "\n");
	fflush(log_file);
#endif

	va_end(val);

	if (gCustomPrintStringFunc)
		gCustomPrintStringFunc(uAttribs, db_tmp_str, gpPrintStringContext);
	else
		gConsole.Output(db_tmp_str);

	if (uAttribs)
	{
		// restore old colours
		gConsole.SetColor(wOldColours);
	}

	return 0;
}

}	// namespace Audio
