#include "SystemPch.h"

#include "system/debug/log/DebugLog.h"
#include <system/debug/log/Subscribers/LogFileSubscriber.h>
#include <system/Serialise.h>
#include <system/debug/log/filesystem.h>
#include <system/serialise/textserialise.h>
#include <system/misc/stringutil.h>

namespace Debug
{

IMPLEMENT_SUBSCRIBER(LogFileSubscriber, "logfile");

// constructor
LogFileSubscriber::LogFileSubscriber() :
	m_File(NULL)
{
}

// constructor with filename
LogFileSubscriber::LogFileSubscriber( const char *pFilename ) :
	m_File(NULL)
{
	LogFileSubscriber::SetXMLParameter("file", pFilename);

	LogFileSubscriber::Start();
}


// destructor
LogFileSubscriber::~LogFileSubscriber()
{
	// close log file
	if (m_File)
		delete m_File;
}

// send a log event to this subscriber
void LogFileSubscriber::LogString(const Util::C_String &string)
{
	if (!m_File)
		return;

	m_pMutex->Wait();

	Sys::TextSerialiser s(*m_File);
	
	// write text to file
	s << string;

	// flush immediately
	m_File->Overflow();
	m_File->Flush();

	m_pMutex->Release();
}

/// parse a configuration parameter for this subscriber
void LogFileSubscriber::SetXMLParameter(const char *pName, const char *pValue)
{
	// open file
	if (!stricmp(pName,"file"))
	{
		m_sFilename = Util::C_String(pValue);
	}
	else
		SubscriberCommon::SetXMLParameter(pName, pValue);
}

// open the log file
bool LogFileSubscriber::Start()
{
	//Create folder for log file.
	Util::C_String folder = Sys::ExtractDirectory(m_sFilename).Chars();

	if (folder != "")
		if (!g_pFileSystem->FileExists(folder) && !g_pFileSystem->CreateDirectory(folder))
			return false;

	m_File = g_pFileSystem->Open(m_sFilename, Sys::E_FileAccess_Write);
	if (!m_File)
		return false;

	return true;
}

};
