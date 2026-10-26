#ifndef __SYSTEM_LOG_INTERFACE_H__
#define __SYSTEM_LOG_INTERFACE_H__

namespace Debug
{

class Logger;

/// retrieve a logger reference given a name
Logger &GetLogger(const char *psName);

/// retrieve root-logger
Logger &GetRootLogger();

/// load a text file and parse it
bool ParseXMLConfigFile(const char *pszFilename);

/// parse a block of text as xml ini file
void ParseXMLConfig(const char *pBuffer);

/// get the effective (cached) debug level of a logger
inline ::DebugLevel::Level GetEffectiveLevel(Logger &logger)
{
	return ((_DebugLevel*)&logger)->m_DebugLevel;
}

}

#endif
