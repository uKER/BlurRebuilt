#include "AudioPch.h"

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <malloc.h>
#include <crtdbg.h>


CommandLineApp*	CommandLineApp::m_pApp = 0;



class XMLtoDat : public CommandLineApp
{

public:
	XMLtoDat() { }
	XMLtoDat(const char* const strName) { Init(strName); }

	void Run(char* sFileToParse, char* sComLineOutputFile, char* sDestinationDirectory, bool bStrict, bool bBigEndian, bool bPauseAtEnd, bool bWriteBankList)
	{
		AudioDataFile audioDataFile;

		if (! bBigEndian)
			SetLittleEndian();

		audioDataFile.SetFrameSkip(6);

		audioDataFile.GetDiagnostics().SetStrict(bStrict);

		//
		if (! strlen(sFileToParse))
		{
			strcpy(sFileToParse, "data\\ui.xml");
		}

		char strError[256] = {0};

		if (audioDataFile.LoadXml(sFileToParse, strError))
		{
			char sOutputFile[512] = {0};
			if (! strlen(sComLineOutputFile))
				GetOutputFilename(sOutputFile, sDestinationDirectory, sFileToParse, "dat");
			else
				sprintf(sDestinationDirectory, sOutputFile, sComLineOutputFile);

			if (!audioDataFile.Save(sOutputFile, (CommandLineApp*)this, bWriteBankList))
			{
				char strErrorString[1024] = {0};
				sprintf(strErrorString, "Error writing file %s\n", sOutputFile);

				ErrorData errorData;
				errorData.m_eErrorLevel = eError;
				errorData.m_strErrorString = strErrorString;
				audioDataFile.GetDiagnostics().ReceiveError(&errorData);
			}
		}
		else
		{
			char strErrorString[1024] = {0};
			sprintf(strErrorString, "%s\n", strError);

			ErrorData errorData;
			errorData.m_eErrorLevel = eError;
			errorData.m_strErrorString = strErrorString;
			audioDataFile.GetDiagnostics().ReceiveError(&errorData);
		}

		audioDataFile.GetDiagnostics().FinalReport(bPauseAtEnd);

		return;
	}
};


//------------------------------------------------------------------------------
//	main
//------------------------------------------------------------------------------
int _tmain(int argc, _TCHAR* argv[])
{
	printf("\t\t\t************************\n\t\t\t%c XMLtoDat Parser v%d.%d %c\n\t\t\t************************\n", 0xe, DAT_FILE_VERSION_MAJOR, DAT_FILE_VERSION_MINOR, 0xe);

	XMLtoDat xmlToDat("XMLtoDat");

	xmlToDat.Begin(argc, argv);

	return 0;
}

