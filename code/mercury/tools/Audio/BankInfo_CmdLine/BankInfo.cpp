// BankInfo.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "Common\WaveBankReader.h"

//using namespace Audio;

void ShowUsage();
void WriteOutWaveData(Audio::WaveBank& bank);

int _tmain(int argc, _TCHAR* argv[])
{
	#ifdef CHECK_FOR_MEMORY_LEAKS
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	#endif

	if (argc == 1 || (argc > 1 && (*argv[1] == '?' || _stricmp(argv[1], "/?") == 0 || _stricmp(argv[1], "-help") == 0)))
	{
		ShowUsage();
		return 0;
	}

	bool bWriteOutWaveData=false;
	int argNum=1;

	if ((argv[argNum][0] == '/') || (argv[argNum][0] == '-'))
	{
		if (argv[1][1] == 'w' || argv[1][1] == 'W')
			bWriteOutWaveData = true;
		else
		{
			printf("Unknown switch '%s'\n\n", argv[argNum]);
			ShowUsage();
			return 0;
		}

		argNum++;
	}

	if (_stricmp(argv[argNum] + strlen(argv[argNum]) - strlen(Audio::pchFileExt), Audio::pchFileExt) == 0)
	{
		Audio::WaveBank bank(argv[argNum]);
		Audio::WaveBankReader reader(&bank);

		if (reader.Read()!=Audio::Success)
		{
			printf("\n");
			system("pause");
 			return 0;
		}

		reader.Show();

		if (bWriteOutWaveData)
			WriteOutWaveData(bank);
	}
	else
	{
		printf("Unknown argument '%s'\n\n", argv[argNum]);
		ShowUsage();
	}

	printf("\n\n");
	system("pause");

	return 1;
}

void ShowUsage()
{
	printf("Usage: BankInfo [/w] bankfile\n\n  /w\t Write out raw wave data to individual files\n\n");
	system("pause");
}

void GetPathFromFilename(std::string strFilename, std::string& strPath)
{
	if (strFilename.data()==NULL)
		return;

	char pchDrive[_MAX_DRIVE]={0};
	char pchDir[_MAX_DIR]={0};

	_splitpath(strFilename.data(), pchDrive, pchDir, NULL, NULL);

	strPath.clear();
	strPath.append(pchDrive);
	strPath.append(pchDir);

	return;
}

void WriteOutWaveData(Audio::WaveBank& bank)
{
	printf("\nWriting out raw wav data...");

	std::string strPath;
	GetPathFromFilename(std::string(bank.Filename()), strPath);
	std::string strExt = "unknown";

	for (U32 i=0; i<bank.NumWaves(); i++)
	{
		Audio::WaveBank::Wave* wave = bank.GetWave(i);

		Audio::File f;

		switch (wave->m_baseHeader.uFormat)
		{
			case Audio::Core::uPCM16BitSignedInt: 
			case Audio::Core::uPCM32BitFloat: 
				strExt = "wav";
				break;
			case Audio::Core::uXMA16Bit: 
				strExt = "xma";
				break;
			case Audio::Core::uADPCM4Bit: 
				strExt = "adpcm";
				break;
		}

		char* fname = new char[strPath.length()+strlen(wave->m_baseHeader.pchName)+strExt.length()+5];
		sprintf(fname, "%s%s.%s.raw", strPath.data(), wave->m_baseHeader.pchName, strExt.data()); 
		printf("\n%s ...", fname);
		f.OpenWrite(fname);
		f.Write(wave->m_pvWaveData, wave->m_baseHeader.uDataLength);
		f.Close();

		printf("ok");
		
	}		
	printf("\nDone.\n\nWhy don't you go and do something less boring instead?\n");
}

//void Audio::Assert(bool bExp)
//{
//	assert(bExp);	
//}