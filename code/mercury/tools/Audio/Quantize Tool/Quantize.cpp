#include "stdafx.h"

#include "Resample.h"
#include "InsertDelete.h"


#define DEFAULT_SAMPLE_BOUNDARY 128
#define MODE_PITCH_SHIFT 0
#define MODE_INSERT_REMOVE 1

bool g_bVerbose=false;

bool ProcessFile(char* strWavePath, unsigned int uQuantize, unsigned int uThreshold, unsigned int uMode);
bool ParseFolder (char* folder, char* strSuffix, unsigned int uQuantize, unsigned int uThreshold, unsigned int uMode);
void ShowUsage();

Util::C_String strDestFilename;

using namespace Audio;

//------------------------------------------------------------------------------
//	Command line arguments:
//		filename
//		mode switch '-p' or '-i"
//		boundary length
//		threshold
//------------------------------------------------------------------------------
int _tmain(int argc, _TCHAR* argv[])
{
	#ifdef CHECK_FOR_MEMORY_LEAKS
	// _CRTDBG_LEAK_CHECK_DF = Perform automatic leak checking at program exit 
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	_CrtSetBreakAlloc(171);	// break on specific alloc
	#endif

	gConsole.Create("Quantize");

	char strWavePath[MAX_PATH] = { 0 };
	unsigned int uQuantize = DEFAULT_SAMPLE_BOUNDARY;
	unsigned int uThreshold;
	unsigned int uMode = MODE_INSERT_REMOVE;

	if (argc == 1
		|| (argc > 1 && (*argv[1] == '?' || _stricmp(argv[1], "/?") == 0 || _stricmp(argv[1], "-help") == 0)))
	{
		ShowUsage();
		return 0;
	}

	for (int i = 1; i < argc; i++)
	{
		if (_strnicmp(argv[i], "quantize=", strlen("quantize=")) == 0)
		{
			sscanf(argv[i] + strlen("quantize="), "%d", &uQuantize);
		}
		else if (_strnicmp(argv[i], "threshold=", strlen("threshold=")) == 0)
		{
			sscanf(argv[i] + strlen("threshold="), "%d", &uThreshold);
		}
		else if (_stricmp(argv[i], "-i") == 0
				|| _stricmp(argv[i], "/i") == 0)
		{
			uMode = MODE_INSERT_REMOVE;
		}
		else if (_stricmp(argv[i], "-p") == 0
				|| _stricmp(argv[i], "/p") == 0)
		{
			uMode = MODE_PITCH_SHIFT;
		}
		else if ((_strnicmp(argv[i], "-d=", 3) == 0))
		{
			strDestFilename = argv[i]+3;
		}
		else if (_stricmp(argv[i], "-v") == 0
				|| _stricmp(argv[i], "/v") == 0)
		{
			g_bVerbose = true;
		}
		else if (_stricmp(argv[i] + strlen(argv[i]) - strlen(".wav"), ".wav") != 0)
		{
			PrintError("unrecognised parameter- %s\n", argv[i]);
			system("pause 0");
			exit(0);
		}
	}


	uThreshold = uQuantize/2;

	Audio::Print(0, "Using %s method\n", uMode==MODE_INSERT_REMOVE?"insert delete":"pitch shift");
	Audio::Print(0, "Quantize is %d\n", uQuantize);
	Audio::Print(0, "Threshold is %d\n\n", uThreshold);

	int iFilesProcessed = 0;
	for (int i = 1; i < argc; i++)
	{
		if ((_strnicmp(argv[i], "-d=", 3) && _stricmp(argv[i] + strlen(argv[i]) - strlen(".wav"), ".wav") == 0))
		{
			Audio::Print(0, "Attempting to process file '%s'\n", argv[i]);
			ProcessFile(argv[i], uQuantize, uThreshold, uMode);
			iFilesProcessed++;
		}
	}


	if (iFilesProcessed>1)
		Audio::Print(FOREGROUND_GREEN | FOREGROUND_INTENSITY, "%d files processed sucessfully.\n", iFilesProcessed);
	else
	if (!iFilesProcessed)
		PrintError("no files processed.\n");
	
	system ("pause");
	return 0;
}

bool ProcessFile(char *strWavePath, unsigned int uQuantize, unsigned int uThreshold, unsigned int uMode)
{
	//DWORD dwSizeIn=0;
	//DWORD dwSizeOut=0;
	//BYTE* pDataOut=0;
	//WAVEFORMATEX* waveFormat;

	//if (uQuantize <= uThreshold)
	//{
	//	PrintError("Threshold must be less than block size\n");
 //       return 0;
	//}

	// Open the .wav file
	Audio::CWaveFile waveFile;

	if (strstr(strWavePath, ".wav") == NULL)
	{
		ShowUsage();
        return 0;
	}

    if(FAILED(waveFile.OpenForRead(strWavePath)))
	{
		PrintError("Wave file '%s' not found\n", strWavePath);
        return 0;
	}

	if(FAILED(waveFile.ReadFormat()))
	{
		PrintError("Could not read format\n");
        return 0;
	}

	/*waveFormat = waveFile.GetFormat();
	if (waveFormat==NULL)
		return 0;

	if (waveFormat->wBitsPerSample != 16)
	{
		PrintError("Aborting, sample must be 16 bit\n");
		return 0;
	}
	else if (waveFormat->nChannels < 1 || waveFormat->nChannels > 2)
	{
		PrintError("Aborting, illegal number of channels\n");
		return 0;
	}*/

	// Read the wave file data. this needs to be done before we can get the number of samples 
    /*if (FAILED(waveFile.ReadSample()))
        return 0;*/

	// ====================
	// all units in SAMPLES
	// ====================

	//DWORD dwNumSamplesIn=0;
	//DWORD dwNumSamplesOut=0;
 //
	//if (waveFile.GetNumSamples(&dwNumSamplesIn) == E_FAIL)
	//{
	//	PrintError("Aborting, could not get number of samples of input wav file\n");
	//	return 0;
	//}

	//if (dwNumSamplesIn % uQuantize == 0)
	//{
	//	Audio::Print(0, "Sample is already quantized to %d samples (%d samples). Nothing to do.\n", uQuantize, dwNumSamplesIn);
	//	return 1;
	//}

	////int sNumBytesPerSample = (waveFormat->wBitsPerSample / 8);
	//U32 uChannels = waveFormat->nChannels;
	//U32 uQuantizeBlockSize = uQuantize * uChannels;

	////U32 uDestSampleRate = 24000;
	//U32 uDestSampleRate = 0;

	//F32 fSampleRateDifRatio;
	//if (uDestSampleRate)
	//	fSampleRateDifRatio= (F32)uDestSampleRate / (F32)waveFormat->nSamplesPerSec;
	//else
	//	fSampleRateDifRatio = 1.0f;

	//U32 uDstNumSamplesUnaligned = (U32)(dwNumSamplesIn * fSampleRateDifRatio);

	//// Get new data chunk size to the nearest quantize block size
	//dwNumSamplesOut = uQuantizeBlockSize * (int)(uDstNumSamplesUnaligned / uQuantizeBlockSize);

	//unsigned int uSamplesDiscarded = uDstNumSamplesUnaligned % uQuantizeBlockSize;

	//// if the bytes we have got rid of is less than the threshold then we round down (delete).
	//// if there are more bytes than the threshold then we round up to the nearest quantize block (insert)
	//if (uSamplesDiscarded > uThreshold)
	//	dwNumSamplesOut += uQuantizeBlockSize;


	// OLD
	//Audio::Print(0, "* Need %d samples to be aligned *\n", dwNumSamplesOut);

	//   // Get the size of the .wav file
	/*DWORD dwSizeIn=0;
	DWORD dwSizeOut=0;*/
    //waveFile.GetDuration(&dwSizeIn);
	//int sNumBytesPerSample = (waveFormat->wBitsPerSample / 8);
	//int uChannels = waveFormat->nChannels;
	//int QuantizeBlockSizeInBytes = ((uQuantize * uChannels) * sNumBytesPerSample);

	//U32 uDestSampleRate = 24000;
	//F32 fSampleRateDifRatio = (F32)uDestSampleRate / (F32)waveFormat->nSamplesPerSec;

	//U32 uDestSampleRateNumBytesUnaligned = (U32)(dwSizeIn * fSampleRateDifRatio);

	//// Get new data chunk size to the nearest quantize block size
	//dwSizeOut = QuantizeBlockSizeInBytes * (int)(uDestSampleRateNumBytesUnaligned / QuantizeBlockSizeInBytes);

	//unsigned int uRemainder = uDestSampleRateNumBytesUnaligned % QuantizeBlockSizeInBytes;

	//// if the bytes we have got rid of is less than the threshold then we round down (delete).
	//// if there are more bytes than the threshold then we round up to the nearest quantize block (insert)
	//if (uRemainder > (uThreshold * sNumBytesPerSample))
	//	dwSizeOut += QuantizeBlockSizeInBytes;



	//if (uMode == MODE_INSERT_REMOVE)
	//{
	//	pDataOut = (BYTE*)malloc(dwSizeOut * sizeof(BYTE));
	//	
	//	if (pDataOut==NULL)
	//	{
	//		PrintError("FAILED to allocate %d bytes. ABORTING\n", dwSizeOut);
	//		return 0;
	//	}
	//}

	//Audio::Print(0, "Processing to %d samples\n", uQuantize);

	//if (dwSizeOut > dwSizeIn)
	//	Audio::Print(0, "Input length %d, new length %d, inserting %d samples ", dwSizeIn/2, dwSizeOut/2, (dwSizeOut/2 - dwSizeIn/2) / uChannels);
	//else if (dwSizeOut < dwSizeIn)
	//	Audio::Print(0, "Input length %d, new length %d, removing %d samples ", dwSizeIn/2, dwSizeOut/2, (dwSizeIn/2 - dwSizeOut/2) / uChannels);
	//else
	//	Audio::Print(0, "Input length %d, new length %d, no change", dwSizeIn/2, dwSizeOut/2);

	//if (uMode == MODE_INSERT_REMOVE)
	//	Modify((BYTE*)waveFile.GetSampleData(), dwSizeIn, pDataOut, dwSizeOut, uChannels);
	//else
	//{
	//	F64 fRatio = (F64)dwNumSamplesOut / (F64)dwNumSamplesIn;
	//	//PitchShift((BYTE*)waveFile.GetSampleData(), dwSizeIn, pDataOut, dwSizeOut, uChannels);
	//	waveFile.SampleRateConvertF64(fRatio, true);

	//	waveFormat->nSamplesPerSec = uDestSampleRate;
	//	waveFormat->nAvgBytesPerSec = uDestSampleRate;
	//}

	//Audio::Print(FOREGROUND_GREEN | FOREGROUND_INTENSITY, "\ndone\n\n");

	if (uMode == MODE_INSERT_REMOVE)
	{
		if (waveFile.Quantize(CWaveFile::eQuantizeInsertDelete, uQuantize, uThreshold, 0) != S_OK)
		{
			PrintError("Could not quantize %s.\n\n", strWavePath);
			waveFile.Close();
			return false;
		}
	}
	else
	if (waveFile.Quantize(CWaveFile::eQuantizeResample, uQuantize, uThreshold, 0) != S_OK)
	{
		PrintError("Could not quantize %s.\n\n", strWavePath);
		waveFile.Close();
		return false;
	}

	waveFile.Close();

	//if (FAILED(waveFile.OpenForWrite(strDestFilename != "" ? strDestFilename : strWavePath)))
	Util::C_String strStripped = Util::RemoveFileExtension(Util::C_String(strWavePath));
	if (FAILED(waveFile.OpenForWrite(Util::C_String(strStripped + Util::C_String("_QUANTIZED") + Util::C_String(".wav")).Chars())))
	{
		PrintError("failed to write file %s.\n\n", strWavePath);

		//free(pDataIn);
		//free(pDataOut);

		return false;
	}

	//if (uMode == MODE_INSERT_REMOVE)
	//	waveFile.m_DataChunk.SetData(pDataOut, dwSizeOut, false);
	
	if (FAILED(waveFile.Write()))
	{
		PrintError("failed to write file %s.\n\n", strWavePath);

		waveFile.Close();
		
		return false;
	}

	waveFile.Close();

	return true;
}

void ShowUsage()
{
	printf("Resamples .wav files to a specified number of samples.\n\n");

	printf("Quantize ? | [-switch] [quantize=[..]] [threshold=[..]] [wavefile(s)]\n\n");

	printf("  ?             Show usage.\n");
	printf("  quantize      Length of 1 boundary region, in samples. Default is 128.\n");
	printf("  switch        -i uses insert/remove method.\n");
	printf("                -p uses pitch shift method.\n");
	printf("                -v verbose output.\n");
	printf("  threshold     Rounding threshold. Remainder less than threshold rounds\n");
	printf("                down, remainder greater than threshold rounds up.\n");
	printf("                Default is half of the quantize value.\n");
	printf("  wavefile(s)   File(s) to be processed.\n");
	printf("\n");
	printf("Note: There is a limit (from windows) on how many wavefiles can be processed. If encountered, try selecting less files and try again.\n");
	
	return;
}