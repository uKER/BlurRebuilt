#ifndef __SYSTEM_LOG_AUTOCONFIGURE_H__
#define __SYSTEM_LOG_AUTOCONFIGURE_H__

#pragma once

namespace Debug
{

/// Try and load an XML config file in the same directory as the .exe, or initialize some default subscribers.
bool AutoConfigureDebugLog(Util::C_String directory = "", bool bSetupDefaultLoggers = true);

class AutoConfigure
{
public:
	AutoConfigure();
	AutoConfigure(Util::C_String directory);

	int				GetNumPaths();
	Util::C_String	GetPath(int i);
	bool			Go(bool bSetupDefaultLoggers);

	Util::C_String	m_sDirectory;
};

}

#endif
