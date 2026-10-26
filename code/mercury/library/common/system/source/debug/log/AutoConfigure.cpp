#include "SystemPch.h"

#include "system/debug/log/debuglog.h"
#include <system/debug/log/AutoConfigure.h>
#include <system/debug/log/Subscribers/ConsoleSubscriber.h>
#include <system/debug/log/Subscribers/SocketSubscriber.h>

#if PLATFORM_PS3
#include <sys/paths.h>
#endif

namespace Debug
{

using namespace Util;

C_String DirFromString(
	const Util::C_String& inString)
{
	// Copy incoming string.
	Util::C_String newInString(inString);
	// Change to unix style directory separators.
	newInString = newInString.Replace("\\", "/");

	// Find the last directory separator.
	int pos = newInString.RevSearch('/');
	if (pos == -1)
	{
		//No directory separator, just return the string.
		return inString;
	}
    
	//else we return the path substring.
	return newInString(0, pos+1);
}

AutoConfigure::AutoConfigure()
{
}

AutoConfigure::AutoConfigure(Util::C_String directory)
{
	m_sDirectory = directory;
}


#if defined( PLATFORM_PC )

int	AutoConfigure::GetNumPaths()
{
	return 2;
}

Util::C_String AutoConfigure::GetPath(int i)
{
	if (i == 0)
	{
		// try the current path
		return Util::C_String(".\\");
	}
	else
	{
		char szFilePath[_MAX_PATH];
		GetModuleFileName(NULL, szFilePath, _MAX_PATH);

		Util::C_String sExePath(szFilePath);
		return DirFromString(sExePath);
	}
}

#elif defined( PLATFORM_PS3 )

int AutoConfigure::GetNumPaths()
{
	return 1;
}

Util::C_String AutoConfigure::GetPath(int i)
{
	// try game root path
	return Util::C_String(SYS_APP_HOME "/");
}

#else

int	AutoConfigure::GetNumPaths()
{
	return 1;
}

Util::C_String AutoConfigure::GetPath(int i)
{
	// try game root path
	return Util::C_String("./");
}

#endif

//lint -sem(AddSubscriber,custodial(1))
bool AutoConfigure::Go(bool bSetupDefaultLoggers)
{
	// try to load the config file
	if (m_sDirectory == "")
	{
		// try default paths
		for (int i=0; i<GetNumPaths(); i++)
		{
			Util::C_String file = GetPath(i) + "logcfg.xml";

			if (ParseXMLConfigFile(file))
				return true; // success
		}
	}
	else
	{
		// try specified path
		Util::C_String file = m_sDirectory + "logcfg.xml";

		if (ParseXMLConfigFile(file))
			return true; // success
	}

	if (!bSetupDefaultLoggers)
		return false;

	// couldn't load a config file
	// add a default console subscriber to the root logger
	Debug::ConsoleSubscriber *p1 = new Debug::ConsoleSubscriber();
	Debug::GetRootLogger().AddSubscriber( p1 );

#ifdef PLATFORM_PC
	// automatically try to connect to a local log server (used for farm)
	Debug::SocketSubscriber *p2 = new Debug::SocketSubscriber();
	p2->SetXMLParameter("host", "127.0.0.1");
	// if we connected to a logging host, turn off OutputDebugString logging
	if (p2->Start())
	{
		Debug::GetRootLogger().AddSubscriber( p2 );
		Debug::GetRootLogger().SetLevel( DebugLevel::Debug );	// if connected to a logging host, default to Debug
		if (IsDebuggerPresent())
			p1->SetXMLParameter("mode","both");
		else
			p1->SetXMLParameter("mode","console");
	}
	else
	{
		p1->SetXMLParameter("mode","both");
		delete p2;
	}

	// suppress executer progress messages if a debugger is attached
	if (IsDebuggerPresent())
		Debug::GetLogger("executer").SetLevel(DebugLevel::Fatal);

	p1->Start();
#endif

	return false;
}

/// Try and load an XML config file in the same directory as the .exe, or initialize some default subscribers.
bool AutoConfigureDebugLog(Util::C_String directory, bool bSetupDefaultLoggers)
{
	AutoConfigure ac(directory);
	return ac.Go(bSetupDefaultLoggers);
}

}

