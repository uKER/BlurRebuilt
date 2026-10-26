#ifndef __SYSTEM_LOG_FORMAT_H__
#define __SYSTEM_LOG_FORMAT_H__

namespace Debug
{
struct LogEvent;

/**
 * format a log message according to a format string
 * 
 * %c - message
 * %d - date						
 * %t - time						
 * %F - filename
 * %L - line number
 * %S - function name
 * %P - level
 * %r - seconds since last message	
 * %x - NDC							
 * %m - name of machine	
 * %l - name of logger
 *
 * %5d  - pad out to 5 characters and left-justify
 * %-5d - pad out to 5 characters and right-justify
 **/

class LogFormat
{
public:
	typedef const char *CLPSZ;

							LogFormat();

	virtual					~LogFormat();

							/// set string to use for formatting
	void					SetFormatString(Util::C_String &s);

							/// format a log event into a string
	Util::C_String			Format(LogEvent *pEvent);    

							/// set current timestamp. needed when repeating log messages
	void					SetTimeStamp(U64 time);
private:
	/// formatting helper functions
	struct Attribute
	{
		int		num;	// padding value
		char	c;		// character
	};

							/// get a char
	char					Char(CLPSZ &p);

							/// generate padded string from attribute info
	Util::C_String			FormatAttr(LogFormat::Attribute &attr, LogEvent *pEvent);
							/// generate string from attribute info
	Util::C_String			GetAttributeString(char c, LogEvent *pEvent);
							/// get attribute info from attribute string
	LogFormat::Attribute	GetAttr(CLPSZ &p);
							/// turn \n into carriage return, etc.
	Util::C_String			FormatEscapeCharacter(CLPSZ &p);
							/// generate date string
	Util::C_String			DateString(U64 time);
							/// generate time string
	Util::C_String			TimeString(U64 time);
							/// generate string with seconds elapsed since last message
	Util::C_String			SecsElapsedString(U64 time);
							/// retrieve the hostname
	Util::C_String			GetHostName();

	// our format string
	Util::C_String			m_FormatString;
	// last timestamp
	U64						m_LastTimeStamp;
};

}

#endif
