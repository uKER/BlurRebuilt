#include "stdafx.h"
#include "AppHeaders.h"

namespace Audio
{
	bool WaveBankWriter::ReadWavFromDisk(const char* pchFilename, Audio::CWaveFile* waveFile)
	{
		if (Util::ExtractFileExtension(pchFilename) != Util::C_String("wav"))
		{
			PrintError("'%s' is not a wav file.\n", pchFilename);
			return false;
		}

		// open the file
		if(FAILED(waveFile->OpenForRead(pchFilename)))
		{
			PrintError("Wave file '%s' not found\n", pchFilename);
			return false;
		}

		if(FAILED(waveFile->ReadFormat()))
		{
			PrintError("Could not read wave format for '%s'\n", pchFilename);
			return false;
		}

		if (FAILED(waveFile->ReadSample()))
		{
			PrintError("Reading sample data.\n");
			return false;
		}
		
		if (FAILED(waveFile->ReadLoopInfo()))
		{
			PrintError("Reading loop information.\n");
			return false;
		}

		if (FAILED(waveFile->ReadCueInfo()))
		{
			PrintError("Reading cue information.\n");
			return false;
		}

		if (FAILED(waveFile->ReadListInfo()))
		{
			PrintError("Reading list information.\n");
			return false;
		}


		#ifdef _DEBUG
		waveFile->LogFormatChunk();
		waveFile->LogSamplerChunk();
		#endif

		return true;
	}

	bool WaveBankWriter::ProcessWavInMemory(Audio::CWaveFile* waveFile, WaveBankIniFile::Wave* pIniFileWave, WaveBank::Wave* pBankWave, U32 uRequiredFormat)
	{
		// get the format	
		WAVEFORMATEX* waveFormat = waveFile->GetFormatEx();
		
		if(waveFormat==NULL)
		{
			PrintError("Could not get wave format for '%s'\n", pIniFileWave->Filename().Chars());
			return false;
		}

		// check this is the number of bits we expected
		int uNumberBitsExpected=16;

		if (uRequiredFormat!=Core::uPCM16BitSignedInt)
		{
			PrintError("Unknown format for '%s'. Only 16 bit PCM supported at the moment.\n", pIniFileWave->Filename().Chars());
			return false;
		}
		
		if (waveFormat->wBitsPerSample != uNumberBitsExpected)
		{
			PrintError("'%s' is %d bits but we expected it to be %d\n", pIniFileWave->Filename().Chars(), waveFormat->wBitsPerSample, uNumberBitsExpected);
			return false;
		}

		// fill in the WaveBank headers...
		pBankWave->m_baseHeader.uFormat = uRequiredFormat;

		pBankWave->m_baseHeader.fDefaultLevel = pIniFileWave->DefaultLevel();
		strncpy(pBankWave->m_baseHeader.pchName, pIniFileWave->InternalName().Chars(), g_WbWaveNameSize);
		pBankWave->m_baseHeader.pchName[g_WbWaveNameSize-1] = 0; // make sure null termination

		waveFile->GetDataSize((DWORD*)&pBankWave->m_baseHeader.uDataLength);
		
		pBankWave->m_baseHeader.uFlags |= pIniFileWave->PitchShiftUp();
		pBankWave->m_baseHeader.uFlags |= pIniFileWave->PitchShiftDown() << PITCHSHIFT_DOWN_SHIFT;
		pBankWave->m_baseHeader.uFlags |= pIniFileWave->PitchShiftQuality() << PITCHSHIFT_QUALITY_SHIFT;
		
		pBankWave->m_baseHeader.uSize = sizeof(WaveBank::WaveBaseHeader)+sizeof(WaveBank::PCMWave);		

		// allocate the extra header 
		pBankWave->m_pExtraHeader = Audio::System::Alloc(sizeof(WaveBank::PCMWave));
		if (!pBankWave->m_pExtraHeader)
		{
			PrintError("could not allocate RAM for PCM Wave header\n");
			return false;
		}
	
		WaveBank::PCMWave* hdr=(WaveBank::PCMWave*)pBankWave->m_pExtraHeader;

		memset(hdr, 0, sizeof(WaveBank::PCMWave));

		hdr->uBitsPerSample = waveFormat->wBitsPerSample;
		hdr->uNumChannels = waveFormat->nChannels;
		hdr->uSampleRate = waveFormat->nSamplesPerSec;
		
		// if they havent specified looping in the ini file then take it from the wav file
		if (pIniFileWave->Loop() == WaveBankIniFile::E_LOOP_NOT_SPECIFIED)
		{
			if (waveFile->IsLoop())
				hdr->uNumLoops = 255;
			else
				hdr->uNumLoops = 0;
		}
		else
		if (pIniFileWave->Loop() == WaveBankIniFile::E_LOOP_YES)
			hdr->uNumLoops = 255;
		else
			hdr->uNumLoops = 0;

		// TODO. stride is how big each interleaved stereo segment is. ie stride of 1 is 1 left sample then 1 right sample
		// stride of 256 is 256 l samples, then 256 r samples, then 256 l samples etc. 
		// nickb said it might not be needed but good to have
		hdr->uStride = 0;
	
		// copy the data
		pBankWave->m_pvWaveData = (char*) Audio::System::Alloc(pBankWave->m_baseHeader.uDataLength);
		if (!pBankWave->m_pvWaveData)
		{
			PrintError("could not allocate RAM for PCM Wave data\n");
			return false;
		}

		char* pchSampleData = (char*)waveFile->GetSampleData();

		if (pchSampleData==NULL)
		{
			PrintError("Sample data does not exist for WAV Wave\n");
			return false;
		}

		// PERF: can we avoid this memcpy?
		// store pointer to it instead?

		// copy the wav sample data
		memcpy(pBankWave->m_pvWaveData, pchSampleData, pBankWave->m_baseHeader.uDataLength);

		//pBankWave->m_pvWaveData = (char*)waveFile->GetSampleData();

		return true;
	}

	// open the file, get the header details and read the data
	bool WaveBankWriter::ReadWav(const char* pchFilename, WaveBankIniFile::Wave* pIniFileWave, WaveBank::Wave* pBankWave, U32 uRequiredFormat)
	{
		Audio::CWaveFile wavFile;
		#ifdef DISPLAY_PROFILE_INFO
		StopWatch sw;
		#endif

		if (ReadWavFromDisk(pchFilename, &wavFile)==false)
		{
			return false;
		}

		// we should have read the cue data if there is any whilst reading the wav, so pass that data into the bank wave
		if (wavFile.m_CueChunk.IsValid())
			wavFile.CopyInCueData(pBankWave);

		DoSampleProcessing(pchFilename, wavFile, pIniFileWave, uRequiredFormat);

		if (ProcessWavInMemory(&wavFile, pIniFileWave, pBankWave, uRequiredFormat)==false)
		{
			return false;
		}

		#ifdef DISPLAY_PROFILE_INFO
		Audio::Print(TXTCOLOUR_BLUE, "ProcessWavInMemory took %.4f seconds.\n", sw.Time());
		#endif

		return true;
	}
};
