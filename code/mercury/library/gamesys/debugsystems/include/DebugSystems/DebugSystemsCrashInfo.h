#ifndef DEBUGSYSTEMSCRASHINFO_H
#define DEBUGSYSTEMSCRASHINFO_H

#ifdef _MSC_VER
#pragma once
#endif

namespace DebugSystems
{
	struct C_DebugHUDLogMessages;

	struct CrashInfo_s
	{
		C_DebugHUDLogMessages*	debugOutput;
	};

	extern CrashInfo_s CrashInfo;
}

#endif