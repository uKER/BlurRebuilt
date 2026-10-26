
#include "stdafx.h"
#include "xbdm.h"

#include <iostream>
#include <tools_shared/VersionStrings.inl>

#include "ubercopy/UberCopyApp.h"
#include "ubercopy/UberCopy.h"

#include <CmdLine.h> 

Debug::Logger& g_Log = Debug::GetLogger("tools.filecopy");

int main(int argc, char **argv)
{
	UberCopyApp ubercopy;
	return ubercopy.Run(argc, argv);

}


U32 ParsePlatformList(Util::C_String platformList)
{
	S32 separatorIndex;
	U32 platformBits = 0;

	Util::C_String partialList = platformList;

	separatorIndex = partialList.Search(',');
	while (separatorIndex > 0)
	{
		platformBits |= Tools::FileCopy::GetBitForPlatform(partialList(0, separatorIndex));
		partialList = partialList(separatorIndex+1, partialList.Length());
		separatorIndex = partialList.Search(',');
	}
	platformBits |= Tools::FileCopy::GetBitForPlatform(partialList);

	return platformBits;
}

/*
U32 ParseConsoleList(Util::C_String consoleList, Ut::UtVector<Ut::DynamicVector<Util::C_String> >& consolenames)
{
	S32 separatorIndex;
	U32 numConsoles = 0;

	Util::C_String partialList = consoleList;

	separatorIndex = partialList.Search(',');
	while (separatorIndex > 0)
	{
		consolenames.PushBack(partialList(0,separatorIndex));
		partialList = partialList(separatorIndex+1, partialList.Length());
		separatorIndex = partialList.Search(',');
		numConsoles++;
	}
	consolenames.PushBack(partialList);
	numConsoles++;
	return numConsoles;
}
*/

//ensures the default console is preserved as calls to xbcp will override this
class DefaultConsoleHelper
{
public:
	DefaultConsoleHelper()
	{
		const int kMaxChars = 256;
		char buf[kMaxChars];
		DWORD length = kMaxChars;

		HRESULT hr = DmGetXboxName(buf, &length);
		if (FAILED(hr))
		{
			_snprintf_s(buf, kMaxChars, _TRUNCATE, "");
			LOGERROR(g_Log, "Unable to determine default Xbox name (hr = %i)", hr);
		}

		m_defaultConsole = buf;
		LOGDEBUG(g_Log, "Default console name: %s", m_defaultConsole.Chars());
	}

	~DefaultConsoleHelper()
	{
		HRESULT hr = DmSetXboxName(m_defaultConsole.Chars());
		LOGDEBUG(g_Log, "Reverting default console name: %s", m_defaultConsole.Chars());
	}

private:
	Util::C_String m_defaultConsole;
};




int UberCopyApp::Main(int argc, char **argv, Progress::I_ProgressBar *pProgress)
{
	DefaultConsoleHelper helper;

	bool success = true;
	// Initialise WinSock so we can do network stuff...
	if (WSAStartup(WINSOCK_VERSION, &WSADATA()) != 0)
	{
		printf("Winsock error.\r\n");
		goto fail;
	}

	{
		Tools::FileCopy::FileCopyUtils360::Instance();

		Util::C_String xmlConfig;
		U32 platforms = Tools::FileCopy::E_FileCopyPlatformBit_All;
		Util::C_String consoleList;
		bool skipPlatformOnFailure = false;

		try
		{
			//Define the command line object.
			TCLAP::CmdLine cmd("Ubercopy", ' ', "");

			TCLAP::UnlabeledValueArg<std::string> xmlFileArg(
				"xml","XML file","", "Path to xml config file");
			TCLAP::ValueArg<std::string> platformArg(
				"p","platform","Platforms",false,"PC,Xenon,PS3","List of platforms {PC|Xenon|PS3},{PC|Xenon|PS3},...");
			TCLAP::ValueArg<std::string> consolesArg(
				"a","consoles","Console override list",false,"","List of consoles console1,console2,...");
			TCLAP::SwitchArg skipPlatformArg(
				"s","skipPlatformOnFailure","Skip Platform on Failure","");

			cmd.add(xmlFileArg);
			cmd.add(platformArg);
			cmd.add(consolesArg);
			cmd.add(skipPlatformArg);

			cmd.parse(argc, argv);

			if (xmlFileArg.getValue().length() != 0)
				xmlConfig = xmlFileArg.getValue().c_str();
			
			if (platformArg.getValue().length() != 0)
				platforms = ParsePlatformList(platformArg.getValue().c_str());

			if (consolesArg.getValue().length() != 0)
				consoleList = consolesArg.getValue().c_str();

			skipPlatformOnFailure = skipPlatformArg.getValue();
		}
		catch (TCLAP::CmdLineParseException &e)
		{
			std::cerr << "Failed to parse arguments: " << e.argId() << std::endl;
			return false;
		}
		catch (TCLAP::ArgException &e)  // catch any exceptions
		{ 
			std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
			return false;
		}


		Tools::FileCopy::C_FileCopyListXml xmlList;
		//read xml
		{
			Sys::C_FileSystem& filesystem = Sys::C_FileSystem::Instance();
			Sys::I_File *pFile = filesystem.Open(xmlConfig);

			if (!pFile)
			{
				LOGERROR(g_Log, "Unable to open xml file %s\n", xmlConfig.Chars());
				goto fail;
			}

			U32 c = (U32)pFile->Length();
			char *pBuffer = new char[c];
			pFile->Read(pBuffer, c);

			if (Sys::XmlParse(xmlList, pBuffer, c) != Sys::E_XmlResult_Success)
			{
				LOGERROR(g_Log, "Failed to parse the xml file %s\n", xmlConfig.Chars());
				delete pFile;
				delete[] pBuffer;
				goto fail;
			}

			delete pFile;
			delete[] pBuffer;
		}

		Tools::FileCopy::C_FileCopyList fileList;
		if (!xmlList.CopyToFileCopyList(fileList))
		{
			LOGERROR(g_Log, "Failed to parse the xml file %s\n", xmlConfig.Chars());
			goto fail;
		}

		if (consoleList.Length())
		{
			Tools::FileCopy::C_FileCopyOption *pConsoleList = fileList.FindFileCopyOptionByName(Util::E_Platform_360, "console");
			if (pConsoleList)
				pConsoleList->m_Value = consoleList;
			else
				fileList.AddFileCopyOption(Util::E_Platform_360, Tools::FileCopy::C_FileCopyOption("console", consoleList));
		}

		success = UberCopy::C_UberCopy::CopyList(fileList, skipPlatformOnFailure, platforms);
	}


//This is one of the two ways in which goto is justified!
exit:
	LOGINFO(g_Log, "UberCopy - Done - %s", (success) ? "Success" : "Failure");
	WSACleanup();
	if (success)
		return 0;
	//Fail
	return 1;

//And this is the only justification I can think for a goto which jumps up!
//Maybe I've been writing to many evil scripts recently.
fail:
	success = false;
	goto exit;
}

