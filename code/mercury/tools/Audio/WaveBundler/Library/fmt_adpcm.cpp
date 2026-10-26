#include "stdafx.h"
#include "AppHeaders.h"
#include "adpcm64.h"

namespace Audio
{	
	bool WaveBankWriter::ReadAdpcmFromDisk(const char* pchFilename, CWaveFile& adpcmFile)
	{
		if (Util::ExtractFileExtension(pchFilename) != Util::C_String("adpcm"))
		{
			PrintError("'%s' is not an adpcm file.\n", pchFilename);
			return false;
		}

		// open the file
		if(FAILED(adpcmFile.OpenForRead(pchFilename)))
		{
			PrintError("Could not open Adpcm file '%s'\n", pchFilename);
			return false;
		}
		
		// get the format	
		if(FAILED(adpcmFile.ReadFormat()))
		{
			PrintError("Could not get wave format for '%s'\n", pchFilename);
			return false;
		}

		if (adpcmFile.ReadSample()==S_FALSE)
		{
			PrintError("Reading adpcm sample data.\n");
			return false;
		}

		return true;
	}
	// open the file, get the header details and read the data
	//bool WaveBankWriter::ProcessAdpcmInMemory(const char* pchFilename, WaveBankIniFile::Wave* pIniFileWave, WaveBank::Wave* pBankWave)
	bool WaveBankWriter::ProcessAdpcmInMemory(CWaveFile* adpcmFile, WaveBankIniFile::Wave* pIniFileWave, WaveBank::Wave* pBankWave, U32 uRequiredFormat/*, bool bLoop*/)
	{
		WAVEFORMATEX* waveFormat = adpcmFile->GetFormatEx();
		if (waveFormat==NULL)
		{
			PrintError("Could not get wave format for '%s'\n", pIniFileWave->Filename().Chars());
			return false;
		}

		if (waveFormat->cbSize != 2)
		{
			PrintError("wavformat extra loop bytes size is %d  and not 2 as expected!\n", waveFormat->cbSize);
			return false;
		}

		// fill in the WaveBank headers...
		pBankWave->m_baseHeader.uFormat = Core::uADPCM4Bit;

		pBankWave->m_baseHeader.fDefaultLevel = pIniFileWave->DefaultLevel();
		strncpy(pBankWave->m_baseHeader.pchName, pIniFileWave->InternalName().Chars(), g_WbWaveNameSize);
		pBankWave->m_baseHeader.pchName[g_WbWaveNameSize-1] = 0; // make sure null termination

		adpcmFile->GetDataSize((DWORD*)&pBankWave->m_baseHeader.uDataLength);
		
		pBankWave->m_baseHeader.uFlags |= pIniFileWave->PitchShiftUp();
		pBankWave->m_baseHeader.uFlags |= pIniFileWave->PitchShiftDown() << PITCHSHIFT_DOWN_SHIFT;
		pBankWave->m_baseHeader.uFlags |= pIniFileWave->PitchShiftQuality() << PITCHSHIFT_QUALITY_SHIFT;
		
		pBankWave->m_baseHeader.uSize = sizeof(WaveBank::WaveBaseHeader)+sizeof(WaveBank::ADPCMWave);			

		// allocate the extra header
		pBankWave->m_pExtraHeader = Audio::System::Alloc(sizeof(WaveBank::ADPCMWave));
		if (!pBankWave->m_pExtraHeader)
		{
			PrintError("Could not allocate RAM for APCM extra header\n");
			return false;
		}
		
		WaveBank::ADPCMWave* hdr=(WaveBank::ADPCMWave*)pBankWave->m_pExtraHeader;
		memset(hdr, 0, sizeof(WaveBank::ADPCMWave));

		hdr->uBitsPerSample = waveFormat->wBitsPerSample;
		hdr->uNumChannels = waveFormat->nChannels;
		hdr->uSampleRate = waveFormat->nSamplesPerSec;
		
		// get the extra format bytes - the 2 bytes after the WAVEFORMATEX struct
		hdr->uLoop = *((U16*)((char*)adpcmFile->m_FormatChunk.m_pData + sizeof(WAVEFORMATEX)));

		pBankWave->m_pvWaveData = (char*) Audio::System::Alloc(pBankWave->m_baseHeader.uDataLength);
		if (!pBankWave->m_pvWaveData)
		{
			PrintError("Could not allocate RAM for ADPCM Wave data\n");
			return false;
		}


		void* pSampleData = adpcmFile->GetSampleData();
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
	bool WaveBankWriter::ReadAdpcm(const char* pchFilename, WaveBankIniFile::Wave* pIniFileWave, WaveBank::Wave* pBankWave, U32 uRequiredFormat)
	{
		CWaveFile adpcmFile;

		if (ReadAdpcmFromDisk(pchFilename, adpcmFile)==false)
		{
			return false;
		}
		
		if (ProcessAdpcmInMemory(&adpcmFile, pIniFileWave, pBankWave, uRequiredFormat)==false)
		{
			return false;
		}

		return true;
	}

	bool WaveBankWriter::PcmToAdpcm(CWaveFile* pPcmWavIn, CWaveFile* pAdpcmWavOut)
	{
		DWORD dwNumSamples;
		WAVEFORMATEX *pWavInFormat = pPcmWavIn->GetFormatEx();

		if (!pWavInFormat)
		{
			PrintError("WaveBankWriter::PcmToAdpcm() - could not get format for PCM wave\n");
			return false;
		}

		if (pPcmWavIn->GetNumSamples(&dwNumSamples) != S_OK)
		{
			PrintError("WaveBankWriter::PcmToAdpcm() - Could not get number of samples for wavfile\n");
			return false;
		}

		short* pInData = (short*)pPcmWavIn->GetSampleData();
		if (!pInData)
		{
			PrintError("WaveBankWriter::PcmToAdpcm() Sample data does not exist for PCM wavfile\n");
			return false;
		}

		// alloc using riff alloc function because it will be freed by the riff free function when the riff chunk is deleted
		unsigned char *outData = (unsigned char*)pAdpcmWavOut->m_DataChunk.Alloc(pWavInFormat->nChannels * dwNumSamples);

		memset(outData, 0, pWavInFormat->nChannels * dwNumSamples);

		if (!outData)
		{
			PrintError("WaveBankWriter::PcmToAdpcm() - Could not allocate memory for Adpcm sample\n");
			return false;
		}

		// do the encode
		#define READ_SIZE_IN_SAMPLES 8192
		short buffer[ READ_SIZE_IN_SAMPLES  ];
		short *pIn = pInData;
		unsigned char *pOut = (unsigned char *)outData;
		unsigned int *pHeader = (unsigned int *)pOut;
		pHeader[0] = 8; // Index of first sample data
		pHeader[1] = (dwNumSamples) * pWavInFormat->nChannels; // Number of Samples
		pHeader[1] = (pHeader[1] + 1) & ~1;
		pOut = pOut + pHeader[0];
		int nrSamplesRemaining = dwNumSamples * pWavInFormat->nChannels;
		int nrSamplesBatchAlign = (nrSamplesRemaining + 63) & ~63;
		int nrWritten = 0;
		int totalWritten = 0;
		adpcm64 adpcm;

		adpcm._StartPack( dwNumSamples, pWavInFormat->nChannels );
		
		while( nrSamplesRemaining > 0)
		{

			int readSize = 0;
			if (nrSamplesRemaining > READ_SIZE_IN_SAMPLES)
			{
				readSize = READ_SIZE_IN_SAMPLES;
				memcpy( buffer, pIn, readSize * sizeof(short));
			}
			else
			{ 
				memset( buffer, 0x00, sizeof(buffer));
				memcpy( buffer , pIn , nrSamplesRemaining * sizeof(short) );
				readSize = nrSamplesRemaining;
				if (pWavInFormat->nChannels == 1)
				{
					readSize = (readSize + 63) &~ 63; 
				}
				else
				{
					readSize = (readSize + 127) &~ 127; 
				}
			}
			nrWritten = adpcm._PackWrite( buffer, readSize, pOut, READ_SIZE_IN_SAMPLES );
			totalWritten += nrWritten;
			pIn = pIn + readSize;
			pOut = pOut + nrWritten;
			nrSamplesRemaining -= readSize;
			nrSamplesBatchAlign -= readSize;
		}
		
		// set the data of the encoded wav file
		pAdpcmWavOut->m_DataChunk.FreeDataAndSetNew((void *)outData, totalWritten, false);

		// allcate enough mem for the waveformatex and the extra 2 bytes. 
		// these bytes are not currently used [they are now!] but we kept this format because
		// its the same as cian's adpcm format and meant we could package up
		// easier by 'piggybacking' using the same format.
		U32 uFormatSize = sizeof(WAVEFORMATEX)+2;
		WAVEFORMATEX* pFormat = (WAVEFORMATEX*)pAdpcmWavOut->m_FormatChunk.Alloc(uFormatSize);

		memset(pFormat, 0, uFormatSize);
		pFormat->wFormatTag = 0xaaab;
		pFormat->nChannels = adpcm.GetNrChannels();
		pFormat->wBitsPerSample = 4;
		pFormat->nSamplesPerSec = pWavInFormat->nSamplesPerSec;
		pFormat->nAvgBytesPerSec = pWavInFormat->nSamplesPerSec * adpcm.GetNrChannels() * 8 / 4;
		pFormat->nBlockAlign = 1;
		pFormat->cbSize = 2;

		*((U16*)((U8*)pFormat + sizeof(WAVEFORMATEX))) = pPcmWavIn->IsLoop() ? 1 : 0;

		pAdpcmWavOut->m_FormatChunk.FreeDataAndSetNew(pFormat, uFormatSize, false);
		adpcm._PackClose();

		return true;
	}

//#ifdef DONKEY_TROUSERS
	bool WaveBankWriter::ConvertToAdpcm(const char* pchFilename, WaveBankIniFile::Wave* pIniFileWave, WaveBank::Wave* pBankWave, U32 uRequiredFormat)
	{
		// read the wav in and encode it to adpcm in memory
		CWaveFile wavFile;
		CWaveFile wavFileAdpcm;

		#ifdef DISPLAY_PROFILE_INFO
		StopWatch sw;
		#endif

		if (ReadWavFromDisk(pchFilename, &wavFile)==false)
			return false;
	
		#ifdef DISPLAY_PROFILE_INFO
		Audio::Print(TXTCOLOUR_BLUE, "Reading file from disk took %.4f seconds.\n", sw.Time());
		#endif

		// we should have read the cue data if there is any whilst reading the wav, so pass that data into the bank wave
		if (wavFile.m_CueChunk.IsValid())
			wavFile.CopyInCueData(pBankWave);

		DoSampleProcessing(pchFilename, wavFile, pIniFileWave, uRequiredFormat);

		#ifdef DISPLAY_PROFILE_INFO
		sw.Start();
		#endif

		Print(0, "'%s': Converting to adpcm...", GetRawFilename((char*)pchFilename).Chars());
		
		// encode to adpcm
		PcmToAdpcm(&wavFile, &wavFileAdpcm);

		#ifdef DISPLAY_PROFILE_INFO
		Audio::Print(TXTCOLOUR_BLUE, "\nEncoding to adpcm took %.4f seconds.\n", sw.Time());

		sw.Start();
		#endif

		if (ProcessAdpcmInMemory(&wavFileAdpcm, pIniFileWave, pBankWave, Core::uADPCM4Bit)==false)
			return false;

		#ifdef DISPLAY_PROFILE_INFO
		Audio::Print(TXTCOLOUR_BLUE, "ProcessAdpcmInMemory took %.4f seconds.\n", sw.Time());
		#endif

		if (gSettings.gbWriteIntermediateEncodedFiles)
			WriteIntermediateWav(pchFilename, Util::C_String().Printf("_WaveBundlerTemp\\%s_adpcm.wav", GetRawFilename(pchFilename)), wavFileAdpcm);
		
		Print(TXTCOLOUR_GREEN, "ok\n");

		return true;
	}
//#endif
};
