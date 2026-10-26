#include "StdAfx.h"
#include "Samples/System/Config.h"

namespace Samples
{

C_Config::C_Config()
{
}

C_Config::~C_Config()
{
	m_Stream.close();
}

bool C_Config::Open(const Util::C_String& fileName)
{
	const Util::C_String& filePath = Display::FormatString(Sys::C_FileSystem::Instance().GetWorkingDirectory() + "/" + fileName);

	m_Stream.open(filePath.Chars(), std::ios::in);

	if (m_Stream.is_open())
	{
		return true;
	}
	else
	{
		LOGWARN(g_Log, "Failed to open file '%s'\n", filePath.Chars());
		return false;
	}
}

void C_Config::Close()
{
	m_Stream.close();
}

}; // namespace Samples
