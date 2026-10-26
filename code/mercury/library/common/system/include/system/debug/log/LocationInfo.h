#ifndef __SYSTEM_LOCATIONINFO_H__
#define __SYSTEM_LOCATIONINFO_H__

namespace Debug
{

struct StaticLocationInfo
{
	int				m_LineNumber;
	const char		*m_FileName;
	const char		*m_FunctionName;
};

class StaticLocationInfoClass : public StaticLocationInfo
{
public:
	inline StaticLocationInfoClass(int line, const char *pFile, const char *pFunction)
	{
		m_LineNumber = line;
		m_FileName = pFile;
		m_FunctionName = pFunction;
	}
};

}

#endif

