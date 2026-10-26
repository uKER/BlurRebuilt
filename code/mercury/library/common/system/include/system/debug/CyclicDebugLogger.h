
#ifndef __SYSTEM_CYCLICDEBUGLOGGER_H__
#define __SYSTEM_CYCLICDEBUGLOGGER_H__

#ifdef _MSC_VER
#pragma once
#endif

template<int MaxLines, int MaxLineLength>
class CyclicDebugLogger
{
#ifndef CFG_GOLD
	int		m_next;
	char	m_text[MaxLines][MaxLineLength];

	int AddJnr(const char* text, bool split)
	{
		int nLength = strlen(text);

		if (!nLength)
			return 0;

		if (!split)
		{
			strncpy_s(m_text[m_next], MaxLineLength, text, MaxLineLength-1);
		}
		else
		{
			strcpy_s(m_text[m_next], MaxLineLength, "->");
			#ifdef PLATFORM_PS3
			strncat(m_text[m_next], text, MaxLineLength-3);
			#else
			strncat_s(m_text[m_next], MaxLineLength, text, MaxLineLength-3);
			#endif
		}
			
		m_next = (m_next+1) % MaxLines;
	
		if (!split)
			return (nLength < MaxLineLength-1) ? 0 : MaxLineLength-1;
		else
			return (nLength < MaxLineLength-3) ? 0 : MaxLineLength-3;
	}
#endif

public:
	CyclicDebugLogger()
	{
#ifndef CFG_GOLD
		m_next = 0;
		memset(m_text, 0, sizeof(m_text));
#endif
	}

	void Add(const char* text)
	{
#ifndef CFG_GOLD
		bool splitLine(false);

		// If the line run's over, split it up
		while (int inc = AddJnr(text, splitLine))
		{
			text += inc;
			splitLine = true;
		}
#endif
	}
};

#endif
