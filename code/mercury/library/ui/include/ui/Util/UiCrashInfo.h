#ifndef UICRASHINFO_H
#define UICRASHINFO_H

#ifdef _MSC_VER
#pragma once
#endif

#include <system\debug\cyclicdebuglogger.h>

namespace UI
{
	struct CrashInfo_s
	{
		CyclicDebugLogger<40, 60>*	luaScreenLogger;
	};

	extern CrashInfo_s CrashInfo;
}

#endif