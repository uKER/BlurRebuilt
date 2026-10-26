
#ifndef HELPER_SCREENS_H
#define HELPER_SCREENS_H

namespace helper_screens
{
	bool BizAssertHandler(BizAssertKind kind, const char* expr, const char* msg, const char* file, int line);

	void InitialiseVisualAssertScreen(bool enableLogFatalCapture=true);
	void ShutdownVisualAssertScreen();
}

#endif
