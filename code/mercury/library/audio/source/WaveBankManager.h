#pragma once

#include "..\include\Audio.h"
#include "Common\wavebank.h"
#include "Common\wavebankreader.h"
#include <btl/utility/string.h>
#include <System/Serialise/FilePath.h>

namespace Audio
{
    class WaveBankManager : public Audio::IWaveManager
    {
		friend struct WavebankDebugEntry;

	private:
		static const S32 hFileStreamInvalid = -1;

		struct BankEntry
		{
		public:
			BankEntry(WaveBankManager* pMan) : nRefCount(0),
						  hBankHandle(INVALID_HANDLE),
						  pWaveBank(0),
						  hFileStream(hFileStreamInvalid),
						  pManager(pMan),
						  eCreation(eWaveBankCreationType_NotSet) {}
		public:
			WaveBankManager*				pManager;
			int								nRefCount;		// how many times is this bank referenced?
			Audio::WaveBank*				pWaveBank;		// the Audio::WaveBank itself which holds the wave data
			NODE_HANDLE						hBankHandle;	// handle to the paged array
			StreamSourceHandle				hFileStream;	// wave server handle (if bank is streaming, MP3 etc)
			Core::Array<StreamHandle>		m_vhStream;		// all stream handles for this bank
			eWaveBankCreationType			eCreation;		// was this created from the audiosystem or the game?
			Sys::C_FilePath					strFullPath;	// full path to bank
		};

    public:
		static void* operator new(size_t sz);
		static void operator delete(void* ptr);
		

    public:
        WaveBankManager();
        ~WaveBankManager();
		
		#if 0
		// WARNING. only use these if necessary. they are slow as they use string compares
		S32 GetFormat(const char* pchBank, const char* pchWave);	
		Audio::Result GetWave(const char* strBank, const char* strName, void* pCoreWave, U32* pu32StreamIndex, Core::WaveStreamData* pStreamInfo = 0);
		#endif

	    bool IsBankLoaded(const char* pchBankName, const char* pchPath = 0) const;
	    void SetBankFilePath(const Sys::C_FilePath& filePath);

		// these functions can be called externally, ie from the game
		WM_BANK_HANDLE LoadBank(const char* pchFullPath);
		Audio::Result ReleaseBank(const WM_BANK_HANDLE hBank, const eWaveBankCreationType eCreation);

		// debug functions
		S32 GetMemoryLoad(S32 s32SampleType=-1);
		void LogLoadedBanks();
		void RenderStats();

		// these functions are designed to be called internally, ie from the audio system
		S32 GetFormat(const WM_WAVE_HANDLE hWave) const;
		Audio::Result GetWave(const WM_WAVE_HANDLE hWave, void* pCoreWave, U32* pu32StreamIndex, Core::WaveStreamData* pStreamInfo = 0) const;
		Audio::Result Prime(const char* pchBank, const char* pchWave, WM_WAVE_HANDLE& hWave, WaveExtendedInfo* pExInfo = 0);
		Audio::Result Release(const WM_WAVE_HANDLE hWave);

		void SetMapFilenameFunc(WbmMapFilenameFunc func) { m_cbMapFilename = func; }

    private:
		Core::Lock& GetLock() { return m_Lock; }

		Audio::Result SetupCoreWave(Audio::WaveBank::Wave* pBankWave, void* pCoreWave, BankEntry* pBank, U32* pu32StreamIndex, Core::WaveStreamData* pStreamInfo = 0) const;

		Audio::Result CreateBank(BankEntry* pEntry);
		BankEntry* GetEntry(const Btl::String& pchBank);

		BankEntry* LookupByName(const char* pchBankName) const;
		BankEntry* LookupByPath(const char* pchPathName) const;
		
		const Sys::C_FilePath& GetDefaultPath() const { return m_strPath; }

		Btl::String GetFormatString(S32 sFormat);

		WbmMapFilenameFunc m_cbMapFilename;

    private:

		Core::Lock m_Lock;

		Sys::C_FilePath m_strPath;

		Audio::BlPagedArray<BankEntry*, 10, 10>	m_vpWaveBanks;
    };
}
