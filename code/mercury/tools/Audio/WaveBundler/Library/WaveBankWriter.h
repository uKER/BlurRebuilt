#pragma once

#include "AppHeaders.h"

typedef void (*FuncSetAsyncOpProgress)(F32 fProgress, void* pContext);

#include <string> 
#include "WaveBank.h"

//class WaveBank;

namespace Audio
{
	class WaveBankWriter
	{
		friend class WaveBank;

	public:
		WaveBankWriter(WaveBank* pWaveBank) : 
			m_setProgressFn(0), 
			m_fProgressMin(0), 
			m_fProgressMax(1.f), 
			m_pSetProgressContext(0)
			{ m_pWaveBank=pWaveBank; }

		bool Write(WaveBankIniFile& iniFile);					// create the wavebank file, given an inifile object
		
		void SetOutputPath(Util::C_String& strPath) 
		{ 
			m_gstrOutputPath = strPath;
			if (strPath.Length() && strPath[strPath.Length()-1] != '\\' && strPath[strPath.Length()-1] != '/')
				m_gstrOutputPath += '\\';
		}

		void SetProgressUpdateCallback(FuncSetAsyncOpProgress func, void* pContext)
		{
			m_setProgressFn = func;
			m_pSetProgressContext = pContext;
		}
		void SetProgressMinMax(F32 fMin, F32 fMax) { m_fProgressMin = fMin; m_fProgressMax = fMax; }

	private:
		WaveBankWriter();

		// readers for different formats
		bool ReadWav(const char* pchFilename, WaveBankIniFile::Wave* pIniFileWave, WaveBank::Wave* pBankWave, U32 uRequiredFormat);
		bool ReadXma(const char* pchFilename, WaveBankIniFile::Wave* pIniFileWave, WaveBank::Wave* pBankWave, U32 uRequiredFormat);
		bool ReadAdpcm(const char* pchFilename, WaveBankIniFile::Wave* pIniFileWave, WaveBank::Wave* pBankWave, U32 uRequiredFormat);
		bool ReadMp3(const char* pchFilename, WaveBankIniFile::Wave* pIniFileWave, WaveBank::Wave* pBankWave, U32 uRequiredFormat);

		bool DoSampleProcessing(const char* pchFilename, CWaveFile &wavFile, WaveBankIniFile::Wave* pIniFileWave, U32 uRequiredFormat, int iSampleIsLoop=-1);
		bool WriteIntermediateWav(Util::C_String strOrigFilename, Util::C_String strNewFilename, CWaveFile &wavFile);

		void DeleteTempFiles(WaveBankIniFile& iniFile);

		bool NeedToEncodeAsLoop(CWaveFile &wavFile, WaveBankIniFile::Wave* pIniFileWave);

		// utility functions
		bool ReadWavFromDisk(const char* pchFilename, Audio::CWaveFile* waveFile);
		bool ProcessWavInMemory(Audio::CWaveFile* waveFile, WaveBankIniFile::Wave* pIniFileWave, WaveBank::Wave* pBankWave, U32 uRequiredFormat);
		
		bool ReadXmaFromDisk(const char* pchFilename, Audio::XMAFile &xmaFile);
		bool ProcessXmaInMemory(Audio::XMAFile *xmaFile, WaveBankIniFile::Wave* pIniFileWave, WaveBank::Wave* pBankWave, U32 uRequiredFormat, U32 uNumFrames);

		bool ConvertToXma(const char* pchFilename, WaveBankIniFile::Wave* pIniFileWave, WaveBank::Wave* pBankWave, U32 uRequiredFormat);
		bool ConvertToAdpcm(const char* pchFilename, WaveBankIniFile::Wave* pIniFileWave, WaveBank::Wave* pBankWave, U32 uRequiredFormat);
		
		bool ReadAdpcmFromDisk(const char* pchFilename, CWaveFile& adpcmFile);
		bool ProcessAdpcmInMemory(CWaveFile* adpcmFile, WaveBankIniFile::Wave* pIniFileWave, WaveBank::Wave* pBankWave, U32 uRequiredFormat);
		bool PcmToAdpcm(CWaveFile* pPcmWavIn, CWaveFile* pAdpcmWavOut);

		bool ReadMp3FromDisk(const char* pchFilename, C_Mp3File& adpcmFile);
		bool ProcessMp3InMemory(C_Mp3File* adpcmFile, WaveBankIniFile::Wave* pIniFileWave, WaveBank::Wave* pBankWave, U32 uRequiredFormat);

		bool WaveDataEndianSwap();
		bool NeedToEndianSwapDataFormat(U32 uFormat);

		bool WriteBankFile(WaveBankIniFile& iniFile);

		bool PcmSigned16ToFloat32(WaveBank::Wave* pWave);
		bool Float32ToPcmSigned16(WaveBank::Wave* pWave);

		bool GetPathFromFilename(std::string strFilename, std::string& strPath);
		
		Util::C_String& OutputPath() { return m_gstrOutputPath; }
		
		void SetProgress(F32 fProgress)
		{ 
			if (m_setProgressFn)
				m_setProgressFn(m_fProgressMin+((m_fProgressMax-m_fProgressMin)*fProgress), m_pSetProgressContext); 
		}
	private:
		FuncSetAsyncOpProgress	m_setProgressFn;
		F32						m_fProgressMin;
		F32						m_fProgressMax;
		void*					m_pSetProgressContext;

		Util::C_String			m_gstrOutputPath;						// the output path for the bank

		WaveBank*				m_pWaveBank;							// the bank the writer is associated with
	};
};	
