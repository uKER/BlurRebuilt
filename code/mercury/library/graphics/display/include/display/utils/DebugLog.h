#ifndef DISPLAY_UTILS_DEBUG_LOG_H
#define DISPLAY_UTILS_DEBUG_LOG_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Display
{

void Output(char* output, ...);

#ifdef USER_LLOYDW

class C_DebugLog
{
public:

	C_DebugLog();
	~C_DebugLog();

	void Dump();
	void Add(char* output, ...);

	FORCE_INLINE static C_DebugLog& Instance() { return s_Instance; }

public:

	S32														m_StringIndex;
	static const S32										m_StringCount = 1024;
	Ut::UtStackVector<Util::C_String256, m_StringCount>		m_StringList;

private:

	static C_DebugLog s_Instance;
};

#endif // USER_LLOYDW

extern Debug::Logger& g_Log;

}; // namespace Display

#endif // DISPLAY_UTILS_DEBUG_LOG_H
