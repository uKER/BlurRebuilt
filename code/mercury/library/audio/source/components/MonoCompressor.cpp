#include "AudioPch.h"
#ifdef PLATFORM_SPU
#include "SystemHeadersSPU.h"
#else
#include "../SystemHeaders.h"
#include "ComponentHelper.h"
#endif

//using namespace Audio;

#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", off) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF

////////////////////////////////////////////////////////////////////////////////
//	Init
////////////////////////////////////////////////////////////////////////////////
#ifndef PLATFORM_SPU
void* Audio::MonoCompressor::Init(void* pData)
{
	Data* pInitData = PreInit<Data>(pData);
	return (void*)(pInitData + 1);
}

////////////////////////////////////////////////////////////////////////////////
//	CoreMemory
////////////////////////////////////////////////////////////////////////////////
U32 Audio::MonoCompressor::CoreMemory(U32 uOffset)
{
	m_uDSP = (ixCore)uOffset;

	uOffset += AUDIO_CORE_ALIGN_16(sizeof(Core::DSP::Compressor));

	return uOffset;
}

////////////////////////////////////////////////////////////////////////////////
//	InitContext
////////////////////////////////////////////////////////////////////////////////
void Audio::MonoCompressor::InitContext(Instance* pInstance)
{
	Data* pInitData = (Data*)m_pObject->GetInitData(m_uInitOffset);
    State* pState = (State*)pInstance->GetStateData(m_uStateOffset);

	Core::DSP::Compressor* pDSP = new( pInstance->GetCoreData( m_uDSP ) ) Core::DSP::Compressor( Name(), 1 );

	pState->fValue = 0.f;
	pState->fAttack = -1.f;
	pState->fRelease = -1.f;
	pState->fThreshold = -1.f;
	pState->fRatio = -1.f;
	pState->fGain = -1.f;
	pState->fSideChainRMS = -1.f;

	if ( pInitData->ctrlSideChainRMS.sCom != eUndefined )
		pDSP->SetParam( Core::DSP::Compressor::eSideChainEnabled, 1.f );
}

////////////////////////////////////////////////////////////////////////////////
//	ReleaseContext
////////////////////////////////////////////////////////////////////////////////
void Audio::MonoCompressor::ReleaseContext(Instance* pInstance)
{
	Core::DSP::Compressor* pDSP = (Core::DSP::Compressor*)pInstance->GetCoreData( m_uDSP );

	delete pDSP;
}
#endif	//	#ifndef PLATFORM_SPU

#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", on) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF


