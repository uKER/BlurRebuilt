#include "stdafx.h"
#include "Display/Utils/DebugLog.h"
#include <System/Serialise.h>
#include <System/Serialise/TextSerialise.h>

namespace Display
{

void Output(char* output, ...)
{
	char buffer[1024];

	va_list arglist;
	va_start(arglist, output);
	vsprintf_s(buffer, 1024, output, arglist);
	va_end(arglist);

}

#ifdef USER_LLOYDW

C_DebugLog C_DebugLog::s_Instance;

C_DebugLog::C_DebugLog() : m_StringIndex(0)
{
}

C_DebugLog::~C_DebugLog()
{
	Dump();
}

void C_DebugLog::Dump()
{
	if(m_StringIndex == 0)
		return;

	#if defined(PLATFORM_PS3)
	Util::C_String filePath = "/app_home/User/";
	#else
	Util::C_String filePath = "";
	#endif

	filePath += "DebugLog.txt";

	Sys::I_File *pFile = Sys::GetDefaultFileSystem()->Open(filePath.Chars(), Sys::E_FileAccess_Append);
	if(pFile)
	{
		Sys::TextSerialiser textSerialiser(*pFile);

		for(S32 i=0; i<m_StringIndex; i++)
		{
			textSerialiser << m_StringList[i] << "\n";
		}

		SAFE_DELETE_PTR(pFile);
	}
	else
	{
		LOGWARN(g_Log, "Failed to open file '%s'\n", filePath.Chars());
	}

	m_StringIndex = 0;
}

void C_DebugLog::Add(char* output, ...)
{
	// List is full so so dump what we have to file.
	if(m_StringIndex >= m_StringCount-1)
	{
		Dump();
	}

	// Add the string to the list.
	static char temp[256];
	va_list arglist;
	va_start(arglist, output);
	vsprintf_s(temp, sizeof(temp), output, arglist);
	va_end(arglist);
	m_StringList[m_StringIndex++] = temp;
}

#endif // USER_LLOYDW

Debug::Logger& g_Log = Debug::GetLogger("mercury.display");

}; // namespace Display
