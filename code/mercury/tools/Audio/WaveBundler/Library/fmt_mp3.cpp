#include "stdafx.h"
#include "AppHeaders.h"

namespace Audio
{	
	// read the mp3 file into memory ready for bundling
	bool WaveBankWriter::ReadMp3FromDisk(const char* pchFilename, C_Mp3File& mp3File)
	{
		if (Util::ExtractFileExtension(pchFilename) != Util::C_String("mp3"))
		{
			PrintError("'%s' is not an mp3 file.\n", pchFilename);
			return false;
		}

		// open the file
		if (mp3File.Open(pchFilename) == false)
		{
			PrintError("Could not open Mp3 file '%s'\n", pchFilename);
			return false;
		}
		
		if (mp3File.Read() == false)
		{
			PrintError("Reading data and getting format from Mp3 file '%s'.\n", pchFilename);
			return false;
		}

		return true;
	}

	// open the file, get the header details and read the data
	bool WaveBankWriter::ProcessMp3InMemory(C_Mp3File* mp3File, WaveBankIniFile::Wave* pIniFileWave, WaveBank::Wave* pBankWave, U32 uRequiredFormat)
	{
		const Mp3Format* mp3Format = mp3File->GetFormat();
		if (mp3Format==NULL)
		{
			PrintError("Could not get mp3 format for '%s'\n", pIniFileWave->Filename().Chars());
			return false;
		}
		
		if (mp3File->DataSize() == 0)
		{
			PrintError("Data size is 0 for '%s'\n", pIniFileWave->Filename().Chars());
			return false;
		}

		// fill in the WaveBank headers...
		pBankWave->m_baseHeader.uFormat = Core::uMP3;

		pBankWave->m_baseHeader.fDefaultLevel = pIniFileWave->DefaultLevel();
		strncpy(pBankWave->m_baseHeader.pchName, pIniFileWave->InternalName().Chars(), g_WbWaveNameSize);
		pBankWave->m_baseHeader.pchName[g_WbWaveNameSize-1] = 0; // make sure null termination

		pBankWave->m_baseHeader.uDataLength = mp3File->DataSize();
		
		pBankWave->m_baseHeader.uFlags |= pIniFileWave->PitchShiftUp();
		pBankWave->m_baseHeader.uFlags |= pIniFileWave->PitchShiftDown() << PITCHSHIFT_DOWN_SHIFT;
		pBankWave->m_baseHeader.uFlags |= pIniFileWave->PitchShiftQuality() << PITCHSHIFT_QUALITY_SHIFT;
		
		pBankWave->m_baseHeader.uSize = sizeof(WaveBank::WaveBaseHeader) + sizeof(WaveBank::MP3Wave);			

		// allocate the extra header
		pBankWave->m_pExtraHeader = Audio::System::Alloc(sizeof(WaveBank::MP3Wave));
		if (!pBankWave->m_pExtraHeader)
		{
			PrintError("Could not allocate RAM for MP3 extra header\n");
			return false;
		}
		
		WaveBank::MP3Wave* hdr=(WaveBank::MP3Wave*)pBankWave->m_pExtraHeader;
		memset(hdr, 0, sizeof(WaveBank::MP3Wave));

		hdr->uLoop = pIniFileWave->Loop() == WaveBankIniFile::E_LOOP_YES ? true : false;
		hdr->uNumChannels = mp3Format->uNumChannels;
		hdr->uSampleRate = mp3Format->uSampleRate;

		hdr->uStartSample = pIniFileWave->NumStartPaddingSamples();
		
		pBankWave->m_pvWaveData = (char*) Audio::System::Alloc(pBankWave->m_baseHeader.uDataLength);
		if (!pBankWave->m_pvWaveData)
		{
			PrintError("Could not allocate RAM for MP3 Wave data\n");
			return false;
		}

		const void* pSampleData = mp3File->GetData();
		// copy it into the Wavebank wave
		if (pSampleData)
		{
			memcpy(pBankWave->m_pvWaveData, pSampleData, pBankWave->m_baseHeader.uDataLength);
		}
		else
		{
			PrintError("Read sample OK but sample data was NULL\n");
			return false;
		}

		return true;
	}

	// open the file, get the header details and read the data
	bool WaveBankWriter::ReadMp3(const char* pchFilename, WaveBankIniFile::Wave* pIniFileWave, WaveBank::Wave* pBankWave, U32 uRequiredFormat)
	{	
		C_Mp3File mp3File;

		if (ReadMp3FromDisk(pchFilename, mp3File)==false)
		{
			return false;
		}
	
		// temp. read corresponding wav file for the mp3 and get the length in samples. 
		// we need this to achieve seamless looping in the engine when decoding.
		CWaveFile wavFile;
		DWORD uNumSourceSamples = 0;	// how many samples are in the source file?
		if (wavFile.OpenForRead(Util::ReplaceFileExtension(pchFilename, "wav").Chars()) == S_OK)
		{
			if (wavFile.ReadFormat() != S_OK)
			{
				PrintWarning("Could not find read format from mp3 source wav file for '%s'.\n", pchFilename);
			}
			else
			{
				WAVEFORMATEX* pFormat =	wavFile.GetFormatEx();
				if (pFormat)
				{
					if (wavFile.GetNumSamples(&uNumSourceSamples) != S_OK)
					{
						PrintWarning("Could not get number of samples from mp3 source wav file for '%s'.\n", pchFilename);
					}
				}
			}
		}
		else
		{
			PrintWarning("Could not find mp3 source wav file for '%s'.\n", pchFilename);
		}

		if (ProcessMp3InMemory(&mp3File, pIniFileWave, pBankWave, uRequiredFormat)==false)
		{
			return false;
		}

		// we need to set uEndSample
		// uStartSample will have been set by this point. its got from the ini file wave.
		WaveBank::MP3Wave* pWave = pBankWave->Mp3Wave();
		if (pWave)
		{
			WAVEFORMATEX* pFormat =	wavFile.GetFormatEx();
			if (pFormat)
			{
				if (pFormat->nSamplesPerSec != pWave->uSampleRate)
				{
					PrintError("Sample rate of source wav file does not match sample rate of mp3 for '%s'. mp3 is %d hz & wav is %d hz\n", pchFilename, pWave->uSampleRate, pFormat->nSamplesPerSec);
					return false;
				}
			}

			// default to the same value. this signifies we havent been able to read the num samples from the source file
			pWave->uEndSample = pWave->uStartSample;

			if (uNumSourceSamples)
				pWave->uEndSample = pWave->uStartSample + uNumSourceSamples;

			Print(0, "start %d end %d\n", pWave->uStartSample, pWave->uEndSample);
		}
		else
			return false;

		return true;
	}

	// TODO:
	//bool WaveBankWriter::PcmToMp3(CWaveFile* pPcmWavIn, CWaveFile* pMp3WavOut)
	//{
	//	DWORD dwNumSamples;
	//	WAVEFORMATEX *pWavInFormat = pPcmWavIn->GetFormatEx();

	//	if (!pWavInFormat)
	//	{
	//		PrintError("WaveBankWriter::PcmToMp3() - could not get format for PCM wave\n");
	//		return false;
	//	}

	//	if (pPcmWavIn->GetNumSamples(&dwNumSamples) != S_OK)
	//	{
	//		PrintError("WaveBankWriter::PcmToMp3() - Could not get number of samples for wavfile\n");
	//		return false;
	//	}

	//	short* pInData = (short*)pPcmWavIn->GetSampleData();
	//	if (!pInData)
	//	{
	//		PrintError("WaveBankWriter::PcmToMp3() Sample data does not exist for PCM wavfile\n");
	//		return false;
	//	}

	//	// alloc using riff alloc function because it will be freed by the riff free function when the riff chunk is deleted
	//	unsigned char *outData = (unsigned char*)pMp3WavOut->m_DataChunk.Alloc(pWavInFormat->nChannels * dwNumSamples);//new unsigned char [ dwNumChannels * dwNumSamples ];

	//	memset(outData, 0, pWavInFormat->nChannels * dwNumSamples);

	//	if (!outData)
	//	{
	//		PrintError("WaveBankWriter::PcmToMp3() - Could not allocate memory for Mp3 sample\n");
	//		return false;
	//	}

	//	// do the encode
	//	// TODO: encode mp3 here
	//	
	//	// set the data of the encoded wav file
	//	pAdpcmWavOut->m_DataChunk.SetData((void *)outData, totalWritten, false);

	//	// allcate enough mem for the waveformatex and the extra 2 bytes. 
	//	// these bytes are not currently used [they are now!] but we kept this format because
	//	// its the same as cian's adpcm format and meant we could package up
	//	// easier by 'piggybacking' using the same format.
	//	U32 uFormatSize = sizeof(WAVEFORMATEX)+2;
	//	WAVEFORMATEX* pFormat = (WAVEFORMATEX*)pAdpcmWavOut->m_FormatChunk.Alloc(uFormatSize);

	//	memset(pFormat, 0, uFormatSize);
	//	pFormat->wFormatTag = 0xaaab;
	//	pFormat->nChannels = adpcm.GetNrChannels();
	//	pFormat->wBitsPerSample = 4;
	//	pFormat->nSamplesPerSec = pWavInFormat->nSamplesPerSec;
	//	pFormat->nAvgBytesPerSec = pWavInFormat->nSamplesPerSec * adpcm.GetNrChannels() * 8 / 4;
	//	pFormat->nBlockAlign = 1;
	//	pFormat->cbSize = 2;

	//	*((U16*)((U8*)pFormat + sizeof(WAVEFORMATEX))) = pPcmWavIn->IsLoop() ? 1 : 0;

	//	pAdpcmWavOut->m_FormatChunk.SetData(pFormat, uFormatSize, false);
	//	adpcm._PackClose();

	//	return true;
	//}


	// TODO:
	//bool WaveBankWriter::ConvertToMp3(const char* pchFilename, WaveBankIniFile::Wave* pIniFileWave, WaveBank::Wave* pBankWave, U32 uRequiredFormat)
	//{
	//	// read the wav in and encode it to Mp3 in memory
	//	CWaveFile wavFile;
	//	CWaveFile wavFileMp3;

	//	#ifdef DISPLAY_PROFILE_INFO
	//	StopWatch sw;
	//	#endif

	//	if (ReadWavFromDisk(pchFilename, &wavFile)==false)
	//		return false;
	//
	//	#ifdef DISPLAY_PROFILE_INFO
	//	Audio::Print(TXTCOLOUR_BLUE, "Reading file from disk took %.4f seconds.\n", sw.Time());
	//	#endif

	//	DoSampleProcessing(pchFilename, wavFile, pIniFileWave, uRequiredFormat);

	//	#ifdef DISPLAY_PROFILE_INFO
	//	sw.Start();
	//	#endif

	//	Print(0, "'%s': Converting to Mp3...", GetRawFilename((char*)pchFilename).Chars());
	//	
	//	// encode to Mp3
	//	PcmToMp3(&wavFile, &wavFileMp3);

	//	#ifdef DISPLAY_PROFILE_INFO
	//	Audio::Print(TXTCOLOUR_BLUE, "\nEncoding to Mp3 took %.4f seconds.\n", sw.Time());

	//	sw.Start();
	//	#endif

	//	if (ProcessMp3InMemory(&wavFileMp3, pIniFileWave, pBankWave, Core::uMp34Bit)==false)
	//		return false;

	//	#ifdef DISPLAY_PROFILE_INFO
	//	Audio::Print(TXTCOLOUR_BLUE, "ProcessMp3InMemory took %.4f seconds.\n", sw.Time());
	//	#endif

	//	if (gSettings.gbWriteIntermediateEncodedFiles)
	//		WriteIntermediateWav(pchFilename, Util::C_String().Printf("_WaveBundlerTemp\\%s_Mp3.wav", GetRawFilename(pchFilename)), wavFileMp3);
	//	
	//	Print(TXTCOLOUR_GREEN, "ok\n");

	//	return true;
	//}
};
