#include "AudioPch.h"
#include <string.h>
#include <stdio.h>
#include "SystemHeaders.h"
#include "WavebankManager.h"
#include "AudioBufferSizes.h"
#include <System/Serialise/FilePath.h>
#include <System/Serialise/I_FileSystem.h>

// remove this when #ifdef AUDIO_DEFERRED_LOADING has gone
#include "XtAudio.h"

#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", off) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF

// format for BANK/WAVE HANDLE
// 12 bits for wave index, 20 bits for bank paged array handle
// wwwwwwwwwwwwbbbbbbbbbbbbbbbbbbbb
static const U32 WBM_BANK_BITS = 20;
static const U32 WBM_WAVE_BITS = 12;
static const U32 WBM_BANK_MASK = ((1 << WBM_BANK_BITS)-1);
static const U32 WBM_WAVE_MASK_UNSHIFTED = ((1 << WBM_WAVE_BITS)-1);
static const U32 WBM_WAVE_MASK = (WBM_WAVE_MASK_UNSHIFTED<<WBM_BANK_BITS);
#define WBM_GET_WAVE(h) ((h & WBM_WAVE_MASK) >> WBM_BANK_BITS)
#define WBM_GET_BANK(h) (h & WBM_BANK_MASK)

Debug::Int gAudioDebugXmaReadChunkSize("Audio.System.Streamer.StreamingXmaReadChunkSizeInK", AUDIO_XMA_BUFFER_SIZE_K, Debug::Range(4, 128), Debug::Step(4));
Debug::Int g_DebugAdpcmFileReadChunkSize("Audio.System.Streamer.DebugAdpcmFileReadChunkSizeInK", AUDIO_ADPCM_BUFFER_SIZE_K, Debug::Range(4, 128), Debug::Step(4), Debug::Persist());
Debug::Bool gbAudioDebugShowWaveBankPath("Audio.System.Wavebanks.ShowBankPathOnScreen", false);
Debug::Bool gbAudioDebugSortWaveBanks("Audio.System.Wavebanks.SortWaveBanksOnScreen", true);
Debug::Float g_AudioWavebankStatsYOffset("Audio.System.Wavebanks.YOffset", 140.f, Debug::Range(-3000.0f, 600.f));

////////////////////////////////////////////////////////////////////////////////
//	new
////////////////////////////////////////////////////////////////////////////////
void* Audio::WaveBankManager::operator new(size_t sz)
{
	System::Get()->PushCurrentMemoryAllocationId(1);
	void* pRet = Audio::System::Alloc(sz);
	System::Get()->PopCurrentMemoryAllocationId();
	return pRet;
}

////////////////////////////////////////////////////////////////////////////////
//	delete
////////////////////////////////////////////////////////////////////////////////
void Audio::WaveBankManager::operator delete(void* ptr)
{
	System::Get()->PushCurrentMemoryAllocationId(1);
	Audio::System::Free(ptr);
	System::Get()->PopCurrentMemoryAllocationId();
}

////////////////////////////////////////////////////////////////////////////////
//  WaveBankManager
////////////////////////////////////////////////////////////////////////////////
Audio::WaveBankManager::WaveBankManager()
{
	m_cbMapFilename = 0;
}

////////////////////////////////////////////////////////////////////////////////
//  ~WaveBankManager
////////////////////////////////////////////////////////////////////////////////
Audio::WaveBankManager::~WaveBankManager()
{
	for (U32 i=0; i<m_vpWaveBanks.GetHighestNodeIndex(); i++)
	{
		if (m_vpWaveBanks[i])
		{
			if(m_vpWaveBanks[i]->pWaveBank)
			{
				delete m_vpWaveBanks[i]->pWaveBank;
			}
			m_vpWaveBanks.FreeByIndex(i);

		//	Audio::System::Free(m_vpWaveBanks[i]);
		}
	}

//	for (U32 i=0; i<m_vpWaveBanks.GetHighestNodeIndex(); i++)
//	{
//		if (m_vpWaveBanks[i])
//			m_vpWaveBanks.FreeByIndex(i);
//	}
}

////////////////////////////////////////////////////////////////////////////////
//  SetBankFilePath()
//	Set default path where banks are loaded from.
//	Note: LoadBank does not use this but instead passes its own full path in.
////////////////////////////////////////////////////////////////////////////////
void Audio::WaveBankManager::SetBankFilePath(const Sys::C_FilePath& pchPath)
{
	if (pchPath.Length() == 0)
		return;

	m_strPath = pchPath;
	m_strPath += '\\';
}

////////////////////////////////////////////////////////////////////////////////
//  CreateBank()
//	Given a wavebank entry, create the wavebank class and load the bank data.
////////////////////////////////////////////////////////////////////////////////
Audio::Result Audio::WaveBankManager::CreateBank(BankEntry* pEntry) 
{
	if ((pEntry->hBankHandle != INVALID_HANDLE) || (pEntry->nRefCount != 0) || (pEntry->pWaveBank != 0))
		return Failed;

	WaveBank* pBank = new WaveBank();

	if (!pBank)
		return OutOfMemory;
		
	pBank->SetFilename(pEntry->strFullPath.Chars());	
	
	Audio::WaveBankReader reader(pBank);
	Audio::Result res = reader.Read();

	if (res == Audio::WaveBankNotFound)
	{
		#if defined (AUDIO_ENABLE_PROFILING)
		Profiler::Instance().PostWarningMessage(NULL, Profiler::eWaveBankNotFound);
		#endif	
		AUDIO_SYS_WARN(DBSTR "could not find '%s'\n", pBank->Filename());
		delete pBank;
		return WaveBankNotFound;
	}
	else
	if (res != Success)
	{
		delete pBank;
		return res;
	}

	// add bank and get handle to paged array
	GetLock().Acquire();
	pEntry->hBankHandle = m_vpWaveBanks.AddEntry(pEntry);
	GetLock().Release();

	if (pEntry->hBankHandle == INVALID_HANDLE || pEntry->hBankHandle & WBM_WAVE_MASK)
	{
		AUDIO_SYS_WARN(DBSTR "Could not create add entry for bank '%s'\n", pEntry->strFullPath.Chars());

		if (pEntry->hBankHandle != INVALID_HANDLE)
		{
			GetLock().Acquire();
			pEntry->hBankHandle = m_vpWaveBanks.Free(pEntry->hBankHandle);
			GetLock().Release();
		}
		
		delete pBank;
		return Failed;
	}

	if (pBank->IsStreamingBank())
	{
		Core::WaveServerRegisterSourceParams sourceParams;

		sourceParams.m_eType = Core::eFile;
		sourceParams.m_pSourceData = (void*)(pEntry->strFullPath.Chars());

		bool bSuccess = Audio::System::Get()->GetWaveServer()->RegisterStreamSource(pEntry->hFileStream, sourceParams);
	}

	pEntry->pWaveBank = pBank;

	return Success;
}

////////////////////////////////////////////////////////////////////////////////
//  CreateEntry
//	If entry exists returns that otherwise creates a new one.
////////////////////////////////////////////////////////////////////////////////
Audio::WaveBankManager::BankEntry* Audio::WaveBankManager::GetEntry(const Btl::String& strFullBankPath)
{
	BankEntry* pEntry = 0;
	
	#ifdef AUDIO_DEFERRED_LOADING
	Btl::String strLookup = strFullBankPath.Lower();
	#else
	Btl::String strLookup = strFullBankPath;
	#endif

	if (m_cbMapFilename)
	{
		// see if we need a mapped filename
		Btl::String strMappedName = m_cbMapFilename(Btl::String(strFullBankPath));

		if (strMappedName != "")
			strLookup = strMappedName;
	}

	pEntry = LookupByPath(strLookup.Chars());
	
	if (pEntry)
		return pEntry;

	AUDIO_SYS_INFO("Load bank %s [%s]", strLookup.Chars(), strFullBankPath.Chars());

	// entry not found so create one
	pEntry = new BankEntry(this);

	if (pEntry)
	{
		pEntry->strFullPath = strLookup;
	}

	return pEntry;	 
}

////////////////////////////////////////////////////////////////////////////////
//  Prime
//	Returns handle to wave in hWave.
//  pchBank is the bank internal name (minus .baf extension).
//	Load banks from default path.
//	called from the main game thread.
////////////////////////////////////////////////////////////////////////////////
Audio::Result Audio::WaveBankManager::Prime(const char* pchBank, const char* pchWave, WM_WAVE_HANDLE& hWave, WaveExtendedInfo* pExInfo)
{
	hWave = INVALID_HANDLE;

	// if entry exists return that otherwise create one
	BankEntry* pEntry = GetEntry(Sys::ConstructFilePath(GetDefaultPath(), pchBank, "baf").String());

	if (!pEntry)
		return OutOfMemory;

	if (!pEntry->pWaveBank && pEntry->nRefCount == 0)
	{
		Audio::Result res = CreateBank(pEntry);
		
		if (res != Audio::Success)
		{
			delete pEntry;
			return res;
		}

		BIZ_ASSERT(pEntry->eCreation == eWaveBankCreationType_NotSet);

		if (pEntry->eCreation == eWaveBankCreationType_NotSet)
			pEntry->eCreation = eWaveBankCreationType_Internal;
	}

	// at this point pEntry either contains a bank which we have just loaded or one loaded before.
	// note: could have been loaded externally or internally

	// bank loaded ok, now look for the wave
	for (U32 w=0; w<pEntry->pWaveBank->NumWaves(); w++)
	{
		if (strnicmp(pEntry->pWaveBank->GetWave(w)->m_baseHeader.pchName, pchWave, Audio::g_WbWaveNameSize)==0)
		{
			pEntry->nRefCount++;

			//AUDIO_SYS_WARN("INTERNAL, %d, bank '%s' : wave '%s'", pEntry->nRefCount, pchBank, pchWave);

			Assert(pEntry->hBankHandle != INVALID_HANDLE);

			hWave = w<<WBM_BANK_BITS | pEntry->hBankHandle;

			if (w > WBM_WAVE_MASK_UNSHIFTED)
			{
				AUDIO_SYS_ERROR("WaveBankManager: bank '%s' : wave '%s' exceeds wave handle limit of %d waves.", pchBank, pchWave, WBM_WAVE_MASK_UNSHIFTED+1);
				return Audio::Failed;
			}

			if (pExInfo)
			{
				if (pEntry->pWaveBank->IsStreamingBank() || pEntry->pWaveBank->GetWave(w)->m_baseHeader.uFormat == Audio::Core::uMP3)
					pExInfo->bCreatedStream = true;
				else
					pExInfo->bCreatedStream = false;
			}

			return Audio::Success;
		}
	}

	AUDIO_SYS_WARN("WaveBankManager: could not find wave '%s' in bank '%s'\n", pchWave, pchBank);
	return Audio::WaveNotFound;
}

////////////////////////////////////////////////////////////////////////////////
//  LoadBank
//	Load bank, given the full path.
//	Use externally from the audio system.
////////////////////////////////////////////////////////////////////////////////
Audio::WM_BANK_HANDLE Audio::WaveBankManager::LoadBank(const char* pchFullPath)
{
	// if entry exists return that otherwise create one
	BankEntry* pEntry = GetEntry(pchFullPath);

	if (!pEntry)
		return WM_INVALID_HANDLE;

	if (!pEntry->pWaveBank && pEntry->nRefCount == 0)
	{
		// if bank has never been loaded then load it
		Audio::Result res = CreateBank(pEntry);
		
		if (res != Audio::Success)
		{
			delete pEntry;
			return WM_INVALID_HANDLE;
		}

		BIZ_ASSERT(pEntry->eCreation == eWaveBankCreationType_NotSet);

		if (pEntry->eCreation == eWaveBankCreationType_NotSet)
			pEntry->eCreation = eWaveBankCreationType_External;
	}

	// at this point pEntry either contains a bank which we have just loaded or one loaded before.
	// if it is flagged as eWaveBankCreationType_Internal then we've looked up a bank
	// loaded internally by the audio system.
	
	pEntry->nRefCount++;

	//AUDIO_SYS_WARN("EXTERNAL, %d, bank '%s'", pEntry->nRefCount, pchFullPath);

	return pEntry->hBankHandle;
}

////////////////////////////////////////////////////////////////////////////////
//  Release
//	Release by wave handle
//	called from game thread
////////////////////////////////////////////////////////////////////////////////
Audio::Result Audio::WaveBankManager::Release(const WM_WAVE_HANDLE hWave)
{
	if (hWave == INVALID_HANDLE)
		return Audio::InvalidHandle;

	U32 hBank = WBM_GET_BANK(hWave);

	return ReleaseBank(hBank, eWaveBankCreationType_Internal);
}

////////////////////////////////////////////////////////////////////////////////
//  ReleaseBank
//	Release by bank handle
////////////////////////////////////////////////////////////////////////////////
Audio::Result Audio::WaveBankManager::ReleaseBank(const NODE_HANDLE hBank, const eWaveBankCreationType eCreation)
{
	if (hBank == INVALID_HANDLE)
		return Audio::InvalidHandle;

	// make sure hBank is not using any of the wave bits. if it is its not a bank handle
	if (hBank & WBM_WAVE_MASK)
		return Audio::InvalidHandle;

	BankEntry* pEntry = m_vpWaveBanks.Get(hBank);

	if (pEntry==NULL)
	{
		AUDIO_SYS_ERROR("Could not release bank! Bank handle 0x%x", hBank);
		return Audio::WaveBankNotFound;
	}

	// only let a bank be released from where it was created.
	// we dont want a bank being loaded on the resource thread and then freed on the game thread.
	// reason: if we are using physical memory (ie xma) then we cannot free memory on a different
	// thread than it was created on.

	/*if (eCreation != pEntry->eCreation)
	{
		AUDIO_SYS_WARN("%d Trying to release bank '%s' from different thread than it was created on!", pEntry->nRefCount, pEntry->strFullPath.Chars());
		return Audio::Success;
	}*/

	pEntry->nRefCount--;

	if (!pEntry->nRefCount)
	{
		// this bank is not referenced any more so free it and remove it from the list

		if (pEntry->pWaveBank->IsStreamingBank())
		{
			// release streaming file
			if (pEntry->hFileStream.IsValid())
			{
				Audio::System::Get()->GetWaveServer()->DeregisterStreamSource(pEntry->hFileStream);
				pEntry->hFileStream = hFileStreamInvalid;
			}
			
			// destroy all streams for this bank
			for (U32 i=0; i<pEntry->m_vhStream.Size(); i++)
			{
				Audio::System::Get()->GetWaveServer()->DestroyStream(pEntry->m_vhStream[i]);
			}
			pEntry->m_vhStream.Clear();
		}

		//printf("delete %s, %x\n", pchBank, pEntry->pWaveBank);
		delete pEntry->pWaveBank;
		delete pEntry;
		
		GetLock().Acquire();
		m_vpWaveBanks.Free(hBank);
		GetLock().Release();

		/*for (U32 i=0; i<m_vpWaveBanks.GetHighestNodeIndex(); i++)
		{
			if (m_vpWaveBanks[i])
				printf("%d - %s\n", i, m_vpWaveBanks[i]->pWaveBank->m_BankHeader.pchName);
		}*/
	}

	return Audio::Success;
}

//////////////////////////////////////////////////////////////////////////////////
////  ReleaseBank
////	called from resource thread
//////////////////////////////////////////////////////////////////////////////////
//Audio::Result Audio::WaveBankManager::ReleaseBank(NODE_HANDLE hBank)
//{
//	if (hBank == INVALID_HANDLE)
//		return Audio::InvalidHandle;
//
//	BankEntry* pEntry = m_vpWaveBanks.Get(hBank);
//
//	if (pEntry == NULL)
//	{
//		AUDIO_SYS_ERROR(DBSTR "Could not release bank. Could not lookup bank from handle", hWave, uWave);
//		return Audio::WaveBankNotFound;
//	}
//
//	return Audio::Success;
//}

#if 0
////////////////////////////////////////////////////////////////////////////////
//  GetFormat
//	lookup by string. 
////////////////////////////////////////////////////////////////////////////////
S32 Audio::WaveBankManager::GetFormat(const char* pchBank, const char* pchWave)
{
	BankEntry* entry = BankLookup(pchBank);

	if (entry==NULL)
		return 0xff;

	for (U32 w=0; w<entry->pWaveBank->NumWaves(); w++)
	{
		Audio::WaveBank::Wave* wave=entry->pWaveBank->GetWave(w);
		if (!stricmp(pchWave, wave->m_baseHeader.pchName))
			return wave->m_baseHeader.uFormat;
	}

	return 0xff;
}
#endif

////////////////////////////////////////////////////////////////////////////////
//  GetFormat
//	lookup by handle
//	called from main game thread AND audio thread
////////////////////////////////////////////////////////////////////////////////
S32 Audio::WaveBankManager::GetFormat(WM_WAVE_HANDLE hWave) const
{
	if (hWave == INVALID_HANDLE)
		return Core::uInvalidFormat;

	U32 hBank = WBM_GET_BANK(hWave);
	U32 uWave = WBM_GET_WAVE(hWave);
	
	BankEntry* entry = m_vpWaveBanks.Get(hBank);

	if (entry == NULL)
	{
		AUDIO_SYS_WARN("Audio::WaveBankManager::GetFormat() - wave bank not found! hWave is 0x%x, wave index is %d", hWave, uWave);
		return Core::uInvalidFormat;
	}

	Audio::WaveBank::Wave* pBankWave=entry->pWaveBank->GetWave(uWave);
	if (pBankWave)
		return pBankWave->m_baseHeader.uFormat;

	AUDIO_SYS_WARN("Audio::WaveBankManager::GetFormat() - bank found but could not find wave!");
	return Core::uInvalidFormat;
}

////////////////////////////////////////////////////////////////////////////////
//  GetWave
//	Lookup by handle. 
//	called from audio thread
////////////////////////////////////////////////////////////////////////////////
Audio::Result Audio::WaveBankManager::GetWave(WM_WAVE_HANDLE hWave, void* pCoreWave, U32* pu32StreamIndex, Core::WaveStreamData* pStreamInfo) const
{
	if (hWave == INVALID_HANDLE)
		return Audio::InvalidHandle;

	U32 hBank = WBM_GET_BANK(hWave);
	U32 uWave = WBM_GET_WAVE(hWave);

	BankEntry* pBankEntry = m_vpWaveBanks.Get(hBank);

	if (pBankEntry)
	{
		#ifndef CFG_SPEED
		if (uWave >= pBankEntry->pWaveBank->NumWaves())
		{
			AUDIO_SYS_WARN("WaveBankManager::GetWave() : wave index %d is out of range\n", uWave);
			return Audio::WaveNotFound;
		}
		#endif
		Audio::WaveBank::Wave* pBankWave=pBankEntry->pWaveBank->GetWave(uWave);
		if (pBankWave)
		{
			return SetupCoreWave(pBankWave, pCoreWave, pBankEntry, pu32StreamIndex, pStreamInfo);
		}

		return Audio::WaveBankNotFound;
	}

	return Audio::WaveBankNotFound;
}

////////////////////////////////////////////////////////////////////////////////
//  SetupCoreWave
//	
////////////////////////////////////////////////////////////////////////////////
Audio::Result Audio::WaveBankManager::SetupCoreWave(Audio::WaveBank::Wave* pBankWave, void* pCoreWave, BankEntry* pBankEntry, U32* pu32StreamIndex, Core::WaveStreamData* pStreamInfo) const
{
	switch (pBankWave->m_baseHeader.uFormat)
	{
		case Audio::Core::uPCM16BitSignedInt:
		{
			Audio::Core::PCMS16Decoder::WaveData* pPcmS16Wave=(Audio::Core::PCMS16Decoder::WaveData*)pCoreWave;
			pPcmS16Wave->psData = (S16*)pBankWave->m_pvWaveData; 
			pPcmS16Wave->uChannels = (U8)pBankWave->PcmWave()->uNumChannels;
            pPcmS16Wave->uSampleRate = pBankWave->PcmWave()->uSampleRate;
			pPcmS16Wave->uLength = pBankWave->m_baseHeader.uDataLength; 
			pPcmS16Wave->bLoop = pBankWave->PcmWave()->uNumLoops == 255 ? true : false;

			if (pBankEntry->hFileStream.IsValid() && pStreamInfo)
			{
				//return Audio::Success;

				Core::WaveServerStreamCreateParams streamCreateParams;

				streamCreateParams.m_hSourceHandle = pBankEntry->hFileStream;
				streamCreateParams.m_u32DataProcessMaxChunkSize = 256 * 4 * 2;
#ifdef PLATFORM_PS3
				streamCreateParams.m_u32FileReadChunkSize = 1024 * 64;
#else
				streamCreateParams.m_u32FileReadChunkSize = 1024 * 16;
#endif
				streamCreateParams.m_u32NumInitialWriteChunks = 3;
				streamCreateParams.m_u32BufferSizeInReadChunks = 3;
				streamCreateParams.m_u32FileStartPosition = pBankWave->m_baseHeader.uDataStart;
				streamCreateParams.m_u32FileEndPosition = pBankWave->m_baseHeader.uDataStart + pBankWave->m_baseHeader.uDataLength;
				streamCreateParams.m_u32ReturnBufferAlignment = 128;
				streamCreateParams.m_u32HeaderSkipSize = 0;
				streamCreateParams.m_bReturnBufferPhysicalMemory = false;
				streamCreateParams.m_bLoop = pPcmS16Wave->bLoop;
				streamCreateParams.m_bPrecache = false;
				streamCreateParams.m_u32MaxReturnBufferSize = 2048;
				streamCreateParams.m_u32StreamEncodedDataFormat = Audio::Core::uPCM16BitSignedInt;
				streamCreateParams.m_bTempAmaxNullFileReadHack = false;

				// hack for Amax, stops PCM actually doing disk reads-
				// for the silent PCM sample used for interactive music syncronisation-
				// saves hammering the disk just to read silence
#if defined(PLATFORM_PS3)
				streamCreateParams.m_bTempAmaxNullFileReadHack = true;
#endif

				#ifndef CFG_SPEED
				streamCreateParams.m_strBankFilename = Misc::ExtractFilename(pBankEntry->strFullPath.Chars());
				streamCreateParams.m_strWaveName = pBankWave->m_baseHeader.pchName;
				#endif

				pStreamInfo->Init();

				pStreamInfo->u32SourceDataLength = pBankWave->m_baseHeader.uDataLength;

				bool bNewStream = Audio::System::Get()->GetWaveServer()->CreateStream(pStreamInfo->hWaveData, streamCreateParams);

//				Assert (bNewStream && pStreamInfo->hWaveData.IsValid());

				if (bNewStream && pStreamInfo->hWaveData.IsValid())
				{
					pPcmS16Wave->hWaveServerIndex = *(pu32StreamIndex);
					*(pu32StreamIndex) = *(pu32StreamIndex) + 1;

					pBankEntry->m_vhStream.PushBack(pStreamInfo->hWaveData);
				}

			}
			else
			{
				pPcmS16Wave->hWaveServerIndex.SetInvalid();
#ifdef PLATFORM_PS3
				AUDIO_CORE_ERROR("Trying to set up memory resident PCM wavebank- this is no longer supported in the Blur release branch");
				return Audio::Failed;
#endif
			}
		}
		break;
#ifdef PLATFORM_PS3
		case Audio::Core::uMP3:
		{
			Audio::Core::PCMS16Decoder::WaveData* pPcmS16Wave=(Audio::Core::PCMS16Decoder::WaveData*)pCoreWave;
			pPcmS16Wave->psData = 0;
			pPcmS16Wave->uChannels = (U8)pBankWave->Mp3Wave()->uNumChannels;
            pPcmS16Wave->uSampleRate = pBankWave->Mp3Wave()->uSampleRate;
			pPcmS16Wave->uLength = pBankWave->m_baseHeader.uDataLength; 
			pPcmS16Wave->bLoop = (pBankWave->Mp3Wave()->uLoop > 0);

			Core::WaveServerStreamCreateParams streamCreateParams;

			streamCreateParams.m_pMemoryResidentData = (U8*)pBankWave->m_pvWaveData; 
			streamCreateParams.m_u32DataProcessMaxChunkSize = 256 * 4 * 2;
			streamCreateParams.m_u32FileReadChunkSize = 1024 * 150;		
			streamCreateParams.m_u32NumInitialWriteChunks = 2;
			streamCreateParams.m_u32BufferSizeInReadChunks = 4;
			streamCreateParams.m_u32FileStartPosition = 0;
			streamCreateParams.m_u32FileEndPosition = pBankWave->m_baseHeader.uDataLength;
			streamCreateParams.m_u32ReturnBufferAlignment = 128;
			streamCreateParams.m_u32HeaderSkipSize = 0;
			streamCreateParams.m_bReturnBufferPhysicalMemory = false;
			streamCreateParams.m_bLoop = pPcmS16Wave->bLoop;
			streamCreateParams.m_bPrecache = false;
			streamCreateParams.m_u32MaxReturnBufferSize = 2048;
			streamCreateParams.m_u32StreamEncodedDataFormat = Audio::Core::uMP3;
			streamCreateParams.m_u8NumChannels = pPcmS16Wave->uChannels;

			#ifndef CFG_SPEED
			streamCreateParams.m_strBankFilename = Misc::ExtractFilename(pBankEntry->strFullPath.Chars());
			streamCreateParams.m_strWaveName = pBankWave->m_baseHeader.pchName;
			#endif

			pStreamInfo->Init();

			bool bNewStream = Audio::System::Get()->GetWaveServer()->CreateStream(pStreamInfo->hWaveData, streamCreateParams);

			Assert (bNewStream && pStreamInfo->hWaveData.IsValid());

			pPcmS16Wave->hWaveServerIndex = *(pu32StreamIndex);
			*(pu32StreamIndex) = *(pu32StreamIndex) + 1;

			pBankEntry->m_vhStream.PushBack(pStreamInfo->hWaveData);
		}
		break;
#endif
		case Audio::Core::uADPCM4Bit:
		{
			Audio::Core::ADPCM4bitDecoder::WaveData* pAdpcmWave=(Audio::Core::ADPCM4bitDecoder::WaveData*)pCoreWave;
			pAdpcmWave->bLoop = pBankWave->AdpcmWave()->uLoop ? true : false; 
			pAdpcmWave->psData = (S8*)pBankWave->m_pvWaveData; 
			pAdpcmWave->uChannels = (U8)pBankWave->AdpcmWave()->uNumChannels;
			pAdpcmWave->uLength = pBankWave->m_baseHeader.uDataLength; 
			pAdpcmWave->uSampleRate = pBankWave->AdpcmWave()->uSampleRate;
			pAdpcmWave->uPCMLoopPosEnd = pBankWave->AdpcmWave()->uNrSamples;
			
			if (pBankEntry->hFileStream.IsValid() && pStreamInfo)
			{
				Core::WaveServerStreamCreateParams streamCreateParams;

				streamCreateParams.m_hSourceHandle = pBankEntry->hFileStream;
				streamCreateParams.m_u32DataProcessMaxChunkSize = 256 * 4 * 2;
				streamCreateParams.m_u32FileReadChunkSize = 1024 * g_DebugAdpcmFileReadChunkSize;		
				streamCreateParams.m_u32NumInitialWriteChunks = 2;
				streamCreateParams.m_u32BufferSizeInReadChunks = 3;
				streamCreateParams.m_u32FileStartPosition = pBankWave->m_baseHeader.uDataStart;
				streamCreateParams.m_u32FileEndPosition = pBankWave->m_baseHeader.uDataStart + pBankWave->m_baseHeader.uDataLength;
				streamCreateParams.m_u32ReturnBufferAlignment = 128;
				streamCreateParams.m_u32HeaderSkipSize = 0;
				streamCreateParams.m_bReturnBufferPhysicalMemory = false;
				streamCreateParams.m_bLoop = pAdpcmWave->bLoop;
				streamCreateParams.m_bPrecache = false;
				streamCreateParams.m_u32MaxReturnBufferSize = 2048;
				streamCreateParams.m_u32StreamEncodedDataFormat = Audio::Core::uADPCM4Bit;

				#ifndef CFG_SPEED
				streamCreateParams.m_strBankFilename = Misc::ExtractFilename(pBankEntry->strFullPath.Chars());
				streamCreateParams.m_strWaveName = pBankWave->m_baseHeader.pchName;
				#endif
				
				pStreamInfo->Init();
				pStreamInfo->u32SourceDataLength = pBankWave->m_baseHeader.uDataLength;
				bool bNewStream = Audio::System::Get()->GetWaveServer()->CreateStream(pStreamInfo->hWaveData, streamCreateParams);
				if (bNewStream && pStreamInfo->hWaveData.IsValid())
				{
					pAdpcmWave->hWaveServerIndex = *(pu32StreamIndex);
					*(pu32StreamIndex) = *(pu32StreamIndex) + 1;

					pBankEntry->m_vhStream.PushBack(pStreamInfo->hWaveData);
				}				
			}
			else
				pAdpcmWave->hWaveServerIndex.SetInvalid();
		}
		break;
		default:
			Assert(0);
			return Audio::Failed;
	}

	return Audio::Success;
}

#if 0
////////////////////////////////////////////////////////////////////////////////
//  GetWave
//	Lookup by string. 
//	NOTE: this function is slow! only use this function if you cannot use the 
//		  handle version
////////////////////////////////////////////////////////////////////////////////
Audio::Result Audio::WaveBankManager::GetWave(const char* pchBank, const char* pchWave, void* pCoreWave, U32* pu32StreamIndex/*, Core::FileStreamInfoPS3* pStreamInfo*/)
{
	for (U32 b=0; b<m_vpWaveBanks.GetHighestNodeIndex(); b++)
	{
		if (m_vpWaveBanks[b] && !strnicmp(pchBank, m_vpWaveBanks[b]->pWaveBank->m_BankHeader.pchName, Audio::bankNameSize))
		{
			for (U32 w=0; w<m_vpWaveBanks[b]->pWaveBank->NumWaves(); w++)
			{
				Audio::WaveBank::Wave* wave=m_vpWaveBanks[b]->pWaveBank->GetWave(w);

				if (!strnicmp(pchWave, wave->m_baseHeader.pchName, Audio::waveNameSize))
				{
					return SetupCoreWave(wave, pCoreWave, m_vpWaveBanks[b], pu32StreamIndex/*, pStreamInfo*/);
				}
			}

			return Audio::WaveNotFound;
		}
	}

	return Audio::WaveBankNotFound;
}
#endif

////////////////////////////////////////////////////////////////////////////////
// LookupByPath
// Lookup by full path filename.
////////////////////////////////////////////////////////////////////////////////
Audio::WaveBankManager::BankEntry* Audio::WaveBankManager::LookupByPath(const char* pchPathName) const
{
	for (U32 b=0; b<m_vpWaveBanks.GetHighestNodeIndex(); b++)
	{
		BankEntry* pEntry = m_vpWaveBanks[b];
		
		if (pEntry && pEntry->pWaveBank && !stricmp(pchPathName, pEntry->pWaveBank->Filename()))
			return pEntry;
	}

	return (BankEntry*)NULL;
}

////////////////////////////////////////////////////////////////////////////////
// LookupByName
// Lookup by internal bank name.
////////////////////////////////////////////////////////////////////////////////
Audio::WaveBankManager::BankEntry* Audio::WaveBankManager::LookupByName(const char* pchBankName) const
{
	for (U32 b=0; b<m_vpWaveBanks.GetHighestNodeIndex(); b++)
	{
		BankEntry* pEntry = m_vpWaveBanks[b];
		
		if (pEntry && !strnicmp(pchBankName, pEntry->pWaveBank->m_BankHeader.pchName, Audio::g_WbNameSize))
			return pEntry;
	}

	return (BankEntry*)NULL;
}

////////////////////////////////////////////////////////////////////////////////
// IsBankLoaded
// See if a bank is loaded. Lookup via bank name (minus .baf extension) or optionally pass in the full path 
// of the bank.
////////////////////////////////////////////////////////////////////////////////
bool Audio::WaveBankManager::IsBankLoaded(const char* pchBankName, const char* pchPath) const
{
	if (pchPath)
		return LookupByPath(Sys::ConstructFilePath(pchPath, pchBankName, "baf").Chars()) ? true : false ;
	else
		return LookupByName(pchBankName) ? true : false;
}

////////////////////////////////////////////////////////////////////////////////
// RenderStats
// Show which banks are loaded onscreen
////////////////////////////////////////////////////////////////////////////////
namespace Audio 
{
	struct WavebankDebugEntry
	{
	public:
		WavebankDebugEntry(Audio::WaveBankManager::BankEntry* pEntry) { m_pEntry = pEntry; }
		Audio::WaveBankManager::BankEntry* m_pEntry;
		bool operator <= ( const WavebankDebugEntry& other ) const { return (stricmp(m_pEntry->strFullPath.Chars(), other.m_pEntry->strFullPath.Chars()) <= 0) ? true : false; }
	};
}

Btl::String Audio::WaveBankManager::GetFormatString(S32 sFormat)
{
	Btl::String strFormat;
	switch (sFormat)
	{
		case Audio::Core::uPCM16BitSignedInt: strFormat = "PCM"; break;
		case Audio::Core::uADPCM4Bit: strFormat = "ADPCM"; break;
		case Audio::Core::uXMA16Bit: strFormat = "XMA1"; break;
		case Audio::Core::uXMA216Bit: strFormat = "XMA2"; break;
		case -1: strFormat = "MIXED"; break;
		default: strFormat = "???";
	}
	return strFormat;
}

void Audio::WaveBankManager::RenderStats()
{
	#ifndef CFG_GOLD
	Btl::List<WavebankDebugEntry> list;
	for (U32 b = 0; b < m_vpWaveBanks.GetHighestNodeIndex(); b++)
	{
		if (m_vpWaveBanks[b])
			list.PushBack(WavebankDebugEntry(m_vpWaveBanks[b]));
	}

	if (gbAudioDebugSortWaveBanks)
		list.Sort();
	
	Btl::String str;
	static F32 fXCol1 = 40.f;
	static F32 fXCol2 = 400.f;
	static F32 fXCol3 = 430.f;
	static F32 fXCol4 = 470.f;
	static F32 fXCol5 = 550.f;
	static F32 fYInc = 20.f;
	
	F32 fY = g_AudioWavebankStatsYOffset;
	U32 c=0;

	Audio::System::DrawBox(fXCol1, fY-fYInc/2.f, 800.f, fYInc*2.f, 0x802020a0);

	Audio::System::PrintAt(fXCol1, fY, 0xffffffff, 16.f, "BANKNAME");
	Audio::System::PrintAt(fXCol2, fY, 0xffffffff, 16.f, "TYPE");
	Audio::System::PrintAt(fXCol4, fY, 0xffffffff, 16.f, "FORMAT");
	Audio::System::PrintAt(fXCol5, fY, 0xffffffff, 16.f, "MEMORY");
	//Audio::System::PrintAt(fXCol5, fY, 0xffffffff, 16.f, "ALL INST.");

	fY += fYInc*2.f;

	Btl::List<WavebankDebugEntry>::Iterator it;
	for (it = list.Begin(); it != list.End(); ++it, c++)
	{
		BankEntry* pEntry = it->m_pEntry;

		if (pEntry)
		{
			Audio::System::DrawBox(fXCol1, fY-fYInc/2.f, 800.f, fYInc, c&0x1 ? 0xc0404040 : 0xc0202020);

 			Audio::System::PrintAt(fXCol1, fY, 0xffffffff, 16.f, "%s", gbAudioDebugShowWaveBankPath ? pEntry->strFullPath.Chars() : Sys::ExtractFilename(pEntry->strFullPath).Chars());
			Audio::System::PrintAt(fXCol2, fY, 0xffffffff, 16.f, "%s", pEntry->eCreation == eWaveBankCreationType_Internal ? "" : "[R]" );
			Audio::System::PrintAt(fXCol3, fY, 0xffffffff, 16.f, "%s", pEntry->pWaveBank->IsStreamingBank() ? "[S]" : "" );

			Audio::System::PrintAt(fXCol4, fY, 0xffffffff, 16.f, "%s", GetFormatString(pEntry->pWaveBank->BankFormat()).Chars());
			
			if (pEntry->pWaveBank->MemoryFootprint() > 1024*1024)
				Audio::System::PrintAt(fXCol5, fY, 0xffffffff, 16.f, "%.2f mb", (F32)pEntry->pWaveBank->MemoryFootprint()/1024.f/1024.f);
			else
				Audio::System::PrintAt(fXCol5, fY, 0xffffffff, 16.f, "%d k", pEntry->pWaveBank->MemoryFootprint()/1024);

			fY += fYInc;
		}
	}

	Audio::System::DrawBox(fXCol1, fY-fYInc/2.f, 800.f, fYInc*2, c&0x1 ? 0xc0404040 : 0xc0202020);

	Audio::System::PrintAt(fXCol1, fY, 0xffffffff, 16.f, "---------------------------------------------------------------------------------------");
	fY += fYInc;
	Audio::System::PrintAt(fXCol1, fY, 0xffffffff, 16.f, "%d banks loaded. [R] = loaded async by resource system. [S] = streaming bank", list.Size());

	#endif
}

////////////////////////////////////////////////////////////////////////////////
// LogLoadedBanks
// Prints out which banks are loaded and how much memory they take up.
////////////////////////////////////////////////////////////////////////////////
void Audio::WaveBankManager::LogLoadedBanks()
{
	S32 totalMemLoad=0;
	#ifndef CFG_GOLD
	AUDIO_SYS_INFO("========WAVEBANK DUMP========");
	for (U32 b=0; b<m_vpWaveBanks.GetHighestNodeIndex(); b++)
	{
		S32 bankMemLoad = 0;
		BankEntry* pEntry = m_vpWaveBanks[b];
		if (pEntry)
		{
			bankMemLoad = pEntry->pWaveBank->MemoryFootprint();

			Btl::String strFormat = GetFormatString(pEntry->pWaveBank->BankFormat());
			AUDIO_SYS_INFO("%2d - %s [refcount %d] [%dk][%s]%s%s", b+1, pEntry->pWaveBank->Filename(), pEntry->nRefCount,  bankMemLoad/1024, strFormat.Chars(), pEntry->pWaveBank->IsStreamingBank() ? " [Streaming]":"", pEntry->eCreation == eWaveBankCreationType_Internal ? "" : " [R]");
		}
		totalMemLoad += bankMemLoad;
	}
	AUDIO_SYS_INFO("%d bank(s) loaded. [%.2f mb] [%d k] [%d bytes]\n[R] = Loaded by the resource system\n========DUMP COMPLETE========", m_vpWaveBanks.GetSize(), (F32)totalMemLoad/1024.f/1024.f, totalMemLoad/1024, totalMemLoad);
	#endif
}

#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", on) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF
