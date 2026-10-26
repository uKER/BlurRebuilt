#include "AudioPch.h"
#ifdef PLATFORM_SPU
#include "SystemHeadersSPU.h"
#else
#include "SystemHeaders.h"
#endif


#ifndef PLATFORM_SPU
Audio::Instance::Instance()
{
	m_hInstance = -1;
	m_bDumpStats = false;

	m_bSuspend = false;
	m_bMute = false;
	m_bPrevSuspend = false;
	m_bPrevMute = false;

	m_pContext = 0;
	m_pCoreData = 0;
	m_pBufferData = 0;

	m_u32ContextSize = 0;
	m_u32CoreDataSize = 0;
	m_u32BufferDataSize = 0;
	m_u32StreamInfoSize = 0;

	m_uWaveServerOffset = 0;

	m_coreInfo.bMutePrev = false;
	m_coreInfo.bMuteNext = false;
	m_bLinkedIntoActiveList = false;

	m_u32NumStreamsCreated = 0;
	m_u32NumAdpcmWavesCreated = 0;

	#if !defined (PLATFORM_PS3) && !defined (PLATFORM_SPU) && !defined CFG_GOLD
	memset(m_pchEntityName, 0, 16);
	memset(m_pchGroupName, 0, 16);
	m_uEntityNameHash = 0xffffffff;
	m_uGroupNameHash = 0xffffffff;
	#endif

#ifdef PLATFORM_PS3
//	m_pPpuData = new InstancePs3PpuData;

	m_u32BitmaskOffset = 16 - ((U32)m_au32AdpcmBitmaskPadded % 16);

	for (U32 i = 0; i < ADPCM_BUFFER_ARRAY_SIZE + 4; i++)
		m_au32AdpcmBitmaskPadded[i] = 0;
#endif

	//m_pDistributedTaskData = new InstanceUpdateParams;

#if defined (AUDIO_ENABLE_PROFILING)
	if( EnableProfiling )
		m_bLogging = false;
#endif
}


Audio::Instance::~Instance()
{
// NB this should never be called as core data will have been released by this point
}
#endif


void Audio::Instance::SetStateFlag(U32 uStateFlag)
{ 
//#ifndef PLATFORM_SPU
//	m_pObject->m_pManager->GetStateChangeLock()->Acquire(); 
	m_uStateFlags |= uStateFlag; 
//	m_pObject->m_pManager->GetStateChangeLock()->Release(); 
//#endif
}

void Audio::Instance::ResetStateFlag(U32 uStateFlag)
{ 
//#ifndef PLATFORM_SPU
//	m_pObject->m_pManager->GetStateChangeLock()->Acquire(); 
	m_uStateFlags &= ~uStateFlag; 
//	m_pObject->m_pManager->GetStateChangeLock()->Release(); 
//#endif
}

const char* Audio::Instance::GroupName() const 
{ 
	#if defined (PLATFORM_PS3) || defined (PLATFORM_SPU) || defined CFG_GOLD
	return "";
	#else
	return m_pchGroupName;
	#endif
}

const char* Audio::Instance::EntityName() const 
{ 
	#if defined (PLATFORM_PS3) || defined (PLATFORM_SPU) || defined CFG_GOLD
	return "";
	#else
	return m_pchEntityName;
	#endif
}

void Audio::Instance::SetEntityName(const char* pchName)
{
	#if defined (PLATFORM_PS3) || defined (PLATFORM_SPU) || defined CFG_GOLD
	#else
	if (pchName)
	{
		strncpy(m_pchEntityName, pchName, 15);
		m_pchEntityName[15] = 0;

		// cache the hash.
		m_uEntityNameHash = StringHash(pchName).GetValue();
	}
	#endif
}

void Audio::Instance::SetGroupName(const char* pchName)
{
	#if defined (PLATFORM_PS3) || defined (PLATFORM_SPU) || defined CFG_GOLD
	#else
	if (pchName)
	{
		strncpy(m_pchGroupName, pchName, 15);
		m_pchEntityName[15] = 0;

		// cache the hash.
		m_uGroupNameHash = StringHash(pchName).GetValue();
	}
	#endif
}

#ifdef PLATFORM_PS3
void Audio::Instance::Suspend(bool bSuspend)
{
	m_bSuspend = bSuspend;

	U32* pu32AdpcmBitmask = GetAdpcmBitmask();

	for (U32 i = 0; i < ADPCM_BUFFER_ARRAY_SIZE; i++)
		pu32AdpcmBitmask[i] = 0;
}
#endif

#ifdef PLATFORM_PS3
void Audio::Instance::Mute(bool bMute)
{
	m_bMute = bMute;

	U32* pu32AdpcmBitmask = GetAdpcmBitmask();

	for (U32 i = 0; i < ADPCM_BUFFER_ARRAY_SIZE; i++)
		pu32AdpcmBitmask[i] = 0;
}
#endif


Audio::Core::WaveStreamData* Audio::Instance::GetNextStreamInfoSlot()
{
	return &(((Core::WaveStreamData*)(GetWaveServerData() + 4))[m_u32NumStreamsCreated]);
}


Audio::Core::AdpcmWaveInfo* Audio::Instance::GetAdpcmWaveInfo(U32 u32WaveIndex)
{
	return &(((Core::AdpcmWaveInfo*)(GetAdpcmWaveInfoDataChunk() + 4))[u32WaveIndex]);
}


Audio::Core::WaveStreamData* Audio::Instance::GetWaveServerStreamData(U32 u32StreamIndex)
{
	return &(((Core::WaveStreamData*)(&m_pCoreData[m_uWaveServerOffset + 4]))[u32StreamIndex]);
}

////////////////////////////////////////////////////////////////////////////////
//	Reset
//////////////////////////////////////////////////////////////////////////////////
//void Audio::Instance::Reset()
//{
//    for (U32 uCount = 0; uCount < m_apWaves.Size(); uCount++)
//		m_apWaves[uCount]->Reset();
//
//    for (U32 uCount = 0; uCount < m_apMixers.Size(); uCount++)
//		m_apMixers[uCount]->Reset();
//
//    for (U32 uCount = 0; uCount < m_apOutputs.Size(); uCount++)
//		m_apOutputs[uCount]->Reset();
//}
//
//////////////////////////////////////////////////////////////////////////////////
////	SetActivated
//////////////////////////////////////////////////////////////////////////////////
//void Audio::Instance::SetActivated()
//{
//    for (U32 uCount = 0; uCount < m_apWaves.Size(); uCount++)
//		m_apWaves[uCount]->SetActivatedDown(true);
//}
//
//////////////////////////////////////////////////////////////////////////////////
////	SetDeactivated
//////////////////////////////////////////////////////////////////////////////////
//void Audio::Instance::SetDeactivated()
//{
//    for (U32 uCount = 0; uCount < m_apOutputs.Size(); uCount++)
//		m_apOutputs[uCount]->SetActivatedUp(true);
//}
//
