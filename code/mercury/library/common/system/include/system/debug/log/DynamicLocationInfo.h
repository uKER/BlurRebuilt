#ifndef __SYSTEM_DYNAMICLOCATIONINFO_H__
#define __SYSTEM_DYNAMICLOCATIONINFO_H__

#include <core\containers\string.h>

namespace Debug
{

// Container for a debug log context.
struct LocationInfo
{
	inline LocationInfo(const StaticLocationInfo &in) :
		m_LineNumber(in.m_LineNumber), 
		m_FileName(in.m_FileName), 
		m_FunctionName(in.m_FunctionName)
	{
	}
	inline LocationInfo() {}

	int				m_LineNumber;
	Util::C_String	m_FileName;
	Util::C_String	m_FunctionName;
};

};

#endif
