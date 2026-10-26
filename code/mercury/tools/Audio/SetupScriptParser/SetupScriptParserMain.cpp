#include "stdafx.h"


CommandLineApp * CommandLineApp::m_pApp = 0;


class ControlScriptParserMain : public CommandLineApp
{

public:
	ControlScriptParserMain() { }
	ControlScriptParserMain(const char* const strName) { Init(strName); }

	void Run(char* sFileToParse, char* sComLineOutputFile, char* sDestinationDirectory, bool bStrict, bool bBigEndian, bool bPauseAtEnd)
	{
		if (! bBigEndian)
			SetLittleEndian();

		// TODO: set strictness within app
		bool bLoadedFile = m_parser.Load(sFileToParse);

		if (bLoadedFile)
		{
			bool bParsed = m_parser.Parse();

			if (bParsed)
			{
				char sOutputFile[512] = {0};
				if (! strlen(sComLineOutputFile))
					GetOutputFilename(sOutputFile, sDestinationDirectory, sFileToParse, "ssc");
				else
					sprintf(sDestinationDirectory, sOutputFile, sComLineOutputFile);

				m_parser.Output(sOutputFile, this);
			}
			else
				return;
		}
		else
		{
			printf("Couldn't load file %s\n", sFileToParse);
		}

		return;
	}

	Audio::ControlScriptParser m_parser;

};


//------------------------------------------------------------------------------
//	main
//------------------------------------------------------------------------------
int main(int argc, char* argv[])
{
	printf("\t\t\t***********************\n\t\t\t%c Setup Script Parser %c\n\t\t\t***********************\n", 0xe, 0xe);

	ControlScriptParserMain ControlScriptParserMain("ControlScriptParserMain");

	ControlScriptParserMain.Begin(argc, argv);

	system("pause 0");

	return 0;
}
