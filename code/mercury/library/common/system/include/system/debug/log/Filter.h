#ifndef __SYSTEM_LOG_FILTER_H__
#define __SYSTEM_LOG_FILTER_H__

namespace Debug
{

/**
 * filter log message according to a specified set
 **/

class LogFilter
{
public:
							LogFilter();

	virtual					~LogFilter();

							/// set string to use for filtering
	void					SetFilterString(Util::C_String &s);

							/// returns true if the event passes the filter
	bool					Filter(LogEvent *pEvent);    

private:
							/// interpret a part of the filter string
	void					InterpretString(Util::C_String str);

	bool					m_bLevels[6];
};

}

#endif
