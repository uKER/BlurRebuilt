#include "SystemPch.h"

#include "system/debug/log/DebugLog.h"
#include <system/xml/XMLParser.h>
#include <system/debug/log/ConfigReader.h>
#include <system/serialise.h>
#include <system/debug/log/FileSystem.h>

/// read and interpret a config xml file
namespace Debug
{

using namespace Sys;
using namespace Util;

// load a text file and parse it
bool ParseXMLConfigFile(const char *pszFilename)
{
	Util::C_String filename(pszFilename);

	if (!g_pFileSystem->FileExists(filename))
		return false;

	// open file for reading
	I_File *pFile = g_pFileSystem->Open(filename);
	if (!pFile)
		return false;

	// read file into buffer
	U32 c = (U32)pFile->Length();
	char *pBuffer = new char[c];
	pFile->Read(pBuffer, c);

	// parse xml data
	XMLConfigFile xmlContent;
	XmlParse( xmlContent, pBuffer, c );

	// cleanup
	delete [] pBuffer;
	delete pFile;
	return true;
}

// parse an element
bool XMLConfigFile::StartElement(const C_String& _name, const C_XmlAttributeList& attributes)
{
	C_String name = _name.Lower();
	if (name == "root")
		// process rootlogger
		ProcessLoggerElement(Debug::GetRootLogger(), attributes);
	else if (name == "subscriber")
		// process subscriber
		ProcessSubscriberElement(attributes);
	else
		// assume it's a logger
		ProcessLoggerElement(Debug::GetLogger(name), attributes);

	return true; 
}

// parse a logger element
void XMLConfigFile::ProcessLoggerElement(Debug::Logger &logger, const C_XmlAttributeList &attributes)
{
	for (int i=0; i<attributes.Elements(); i++)
	{
		const C_XmlAttribute *pA = attributes[i];
		C_String name = pA->name.Lower();

		// interpret level
		if (pA->name == "level")
			logger.SetLevel( DebugLevelFromString(pA->value) );
		else
		{
			// here when an invalid attribute was specified
		}
	}
}

/// parse a subscriber
void XMLConfigFile::ProcessSubscriberElement(const C_XmlAttributeList &attributes)
{
	// scan for "type", and create the subscriber
	I_LogSubscriber *pSubscriber = NULL;
	for (int i=0; i<attributes.Elements(); i++)
	{
		if (attributes[i]->name.Lower() == "type")
		{
			pSubscriber = SubscriberManager::Instance().CreateInstanceOf(attributes[i]->value);
			break;
		}
	}

	/// bail out if an invalid subscriber type was specified
	if (!pSubscriber)
		return;

	// scan through other attributes
	bool bAttached = false;
	for (int i=0; i<attributes.Elements(); i++)
	{
		C_String name = attributes[i]->name.Lower();
		// skip type
		if (!stricmp(name, "type"))
			continue;
		// starts with "attach"?
		else if (!strncmp(name, "attach", 6))
		{
			// attach to a logger
			Debug::Logger &logger = GetLogger(attributes[i]->value);
			logger.AddSubscriber(pSubscriber);
			bAttached = true;
		}
		else
			// pass unrecognized attributes to the subscriber itself
			pSubscriber->SetXMLParameter(name, attributes[i]->value);
	}

	// if it wasn't attached to a logger, delete it
	if (!bAttached)
	{
		delete pSubscriber;
		return;
	}

	pSubscriber->Start();
}


}
