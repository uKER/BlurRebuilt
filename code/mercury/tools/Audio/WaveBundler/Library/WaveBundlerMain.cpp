// WaveBundler.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "AppHeaders.h"
#include "Wavebundler.h"

using namespace Audio;

WbSettings gSettings;

void ShowUsage();

void CreateXmlFromFiles(char* pchFname);
int IsDir(const char* pchStr);

//-----------------------------------------------------------------------------
// Name: WaveBundlerMain
// Desc: 
//-----------------------------------------------------------------------------
int WaveBundlerMain(int argc, _TCHAR* argv[])
{
	#ifdef CHECK_FOR_MEMORY_LEAKS
	// _CRTDBG_LEAK_CHECK_DF = Perform automatic leak checking at program exit 
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF | _CRTDBG_CHECK_ALWAYS_DF /* _CRTDBG_CHECK_ALWAYS_DF = CHECK HEAP _EVERY_ ALLOC. SLOW! */);
	//_CrtSetBreakAlloc(10059);	// break on specific alloc
	#endif

	StopWatch sw;

	gConsole.Create("Wavebundler");

	#ifdef _DEBUG
	Print(TXTCOLOUR_PURPLE, "[DEBUG BUILD][DEBUG BUILD][DEBUG BUILD][DEBUG BUILD][DEBUG BUILD][DEBUG BUILD]\n");
	#endif

	#ifdef PRINT_ARGS
	printf("%s ", argv[0]);
	for (int i = 1; i < argc; i++)
		printf("%s ", argv[i]);
	printf("\n");
		
	printf("num args = %d\n", argc);
	#endif	

	bool bCreateXmlFileHack=false;
	char* pchFname=0;

	// if there are no arguments then show the usage
	if (argc==1)
	{	
		ShowUsage();
		return 0;
	}

	// see if we need to auto-create an xml file. 
	// auto creating xml files will take priority over creating banks
	if (argc > 1)
	{
		for (int i = 1; i < argc; i++)
		{
			if (!strnicmp(".wav", argv[i]+strlen(argv[i])-4, 4) ||
			!strnicmp(".adpcm", argv[i]+strlen(argv[i])-6, 6) ||
			!strnicmp(".xma", argv[i]+strlen(argv[i])-4, 4)) 
			{
				bCreateXmlFileHack = true;
				pchFname = argv[i];
				break;
			}
			
			if (IsDir(argv[i]))
			{
				bCreateXmlFileHack = true;
				pchFname = argv[i];
				break;
			}
		}
	}

	if (bCreateXmlFileHack && pchFname)
	{
		CreateXmlFromFiles(pchFname);
		system("pause");
		return 0;
	}

	if (argc == 1 || (argc > 1 && (*argv[1] == '?' || stricmp(argv[1], "/?") == 0 || stricmp(argv[1], "-help") == 0)))
	{
		ShowUsage();
		return 0;
	}
	
	// look for switches 
	
	for (int i = 1; i < argc; i++)
	{
		if ((argv[i][0] == '/') || (argv[i][0] == '-'))
		{
			if (argv[i][1] == 'v' || argv[i][1] == 'V')
				gSettings.gbVerboseMode = 1;
			else
			if (argv[i][1] == 'p' || argv[i][1] == 'P')
				gSettings.gbDoSystemPause = false;
			else
			if (argv[i][1] == 'w' || argv[i][1] == 'W')
			{
				gSettings.gbWriteIntermediateBitDepthFiles = true;
				gSettings.gbWriteIntermediateQuantizedFiles = true;
				gSettings.gbWriteIntermediateResampledFiles = true;
				gSettings.gbWriteIntermediateEncodedFiles = true;
			}
			else 
			if (argv[i][1] == 'd' || argv[i][1] == 'D')
			{
				gSettings.gstrOutputPath = Util::C_String(argv[i] + strlen("/d="));
				gSettings.gstrOutputPath = gSettings.gstrOutputPath.Replace("/", "\\");
				if (gSettings.gstrOutputPath[gSettings.gstrOutputPath.Length()-1] != '\\')
					gSettings.gstrOutputPath += '\\';
				Audio::Print(0, "Destination directory set to %s\n", gSettings.gstrOutputPath.Chars());
				gSettings.gbWriteBankToDest = true;
			}
		}
	}

	// process each xml file in turn
	int iFilesProcessed = 0;
	
	for (int i = 1; i < argc; i++)
	{
		if (stricmp(argv[i] + strlen(argv[i]) - strlen(".xml"), ".xml") == 0)
		{
			WaveBankIniFile iniFile;
			iniFile.SetFilename(Util::C_String(argv[i]));

			#ifdef DISPLAY_PROFILE_INFO
			StopWatch sw2;
			#endif

			// load the xml ini file and parse it
			if (iniFile.Load() == false)
			{
				//system("color c"); // change text to red
				Audio::Print(0, "\n");
				continue;
			}

			#ifdef DISPLAY_PROFILE_INFO
			Audio::Print(TXTCOLOUR_BLUE, "\nLoading and parsing ini file took %.4f seconds.\n", sw2.Time());
			#endif

			WaveBank bank(iniFile.BankName().Chars());
			
			WaveBankWriter writer(&bank);

			if (gSettings.gbWriteBankToDest)
				writer.SetOutputPath(gSettings.gstrOutputPath);

			if (writer.Write(iniFile)==false)
			{
				Audio::Print(0, "\n");
				continue;
			}
			iFilesProcessed++;
		}
	}
	
	Audio::Print(0, "\n%d bank files written.\n", iFilesProcessed);

	Audio::Print(0, "\nTook %.2f seconds.\n", sw.Time());

	if (gSettings.gbDoSystemPause)
	{
		Audio::Print(0, "\n");
		system("pause");
	}

	return 1;
}

//-----------------------------------------------------------------------------
// Name: ShowUsage
// Desc: Display progam command line usage
//-----------------------------------------------------------------------------
void ShowUsage()
{
	Audio::Print(0, "Usage: WaveBundler [/w] [/v] [/p] [/d=] xmlfile [xmlfile2] [xmlfile3] [...]\n\n  /w\t\tWrite intermediate files\n  /v\t\tVerbose mode\n  /p\t\tSuppress pause\n  /d=dest\tWrite Bank to directory 'dest'\n\n");
	Audio::Print(0, "WaveBundler version %d. Built %s %s.\n", Audio::g_WbBankVersion, __TIME__, __DATE__);
	system("pause");
}

