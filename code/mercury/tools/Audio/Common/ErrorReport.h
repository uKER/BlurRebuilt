#if !defined(ERRORREPORT_H)
#define ERRORREPORT_H

#define TXTCOLOUR_YELLOW (FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY)
#define TXTCOLOUR_RED (FOREGROUND_RED | FOREGROUND_INTENSITY)
#define	TXTCOLOUR_BLUE (FOREGROUND_BLUE | FOREGROUND_INTENSITY)
#define TXTCOLOUR_GREEN (FOREGROUND_GREEN | FOREGROUND_INTENSITY)
#define TXTCOLOUR_PURPLE (FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY)

int PrintError(const char *format, ...);
int PrintWarning(const char *format, ...);

typedef void (*FuncErrorReport)(const char* pchText, void* pContext);

void SetErrorReportCallback(FuncErrorReport funcPtr, void* pContext=0);
void SetWarningReportCallback(FuncErrorReport funcPtr, void* pContext=0);

extern bool bLastPrintHadCarriageReturn;
#endif