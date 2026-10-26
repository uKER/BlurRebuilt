#include "AudioPch.h"
#ifdef PLATFORM_SPU
#include "SystemHeadersSPU.h"
#else
#include "../SystemHeaders.h"
#include "ComponentHelper.h"
#endif


#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", off) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF

////////////////////////////////////////////////////////////////////////////////
//	Init
////////////////////////////////////////////////////////////////////////////////
#ifndef PLATFORM_SPU
void* Audio::SurroundLFECompressor::Init(void* pData)
{
	Data* pInitData = PreInit<Data>(pData);
    return (void*)(pInitData + 1);
}

////////////////////////////////////////////////////////////////////////////////
//	CoreMemory
////////////////////////////////////////////////////////////////////////////////
U32 Audio::SurroundLFECompressor::CoreMemory(U32 uOffset)
{
	m_uDSP = (ixCore)uOffset;

	uOffset += AUDIO_CORE_ALIGN_16(sizeof(Core::DSP::Compressor));

	return uOffset;
}

////////////////////////////////////////////////////////////////////////////////
//	InitContext
////////////////////////////////////////////////////////////////////////////////
void Audio::SurroundLFECompressor::InitContext(Instance* pInstance)
{
	Data* pInitData = (Data*)m_pObject->GetInitData(m_uInitOffset);
    State* pState = (State*)pInstance->GetStateData(m_uStateOffset);

	Core::DSP::Compressor* pDSP = new( pInstance->GetCoreData( m_uDSP ) ) Core::DSP::Compressor( Name(), 6 );

	pState->fValue = 0.f;
	pState->fAttack = -1.f;
	pState->fRelease = -1.f;
	pState->fThreshold = -1.f;
	pState->fRatio = -1.f;
	pState->fGain = -1.f;
	pState->fSideChainRMS = -1.f;

	if( pInitData->ctrlSideChainRMS.sCom != eUndefined )
		pDSP->SetParam( Core::DSP::Compressor::eSideChainEnabled, 1.f );
}

////////////////////////////////////////////////////////////////////////////////
//	ReleaseContext
////////////////////////////////////////////////////////////////////////////////
void Audio::SurroundLFECompressor::ReleaseContext(Instance* pInstance)
{
	U8* pCoreData = pInstance->GetCoreData();

	Core::DSP::Compressor* pDSP = (Core::DSP::Compressor*)&pCoreData[m_uDSP];

	delete pDSP;
}
#endif	//	#ifndef PLATFORM_SPU

////////////////////////////////////////////////////////////////////////////////
//	Update
////////////////////////////////////////////////////////////////////////////////
void Audio::SurroundLFECompressor::Update(Instance* pInstance)
{
    AUDIO_PERF_MICRO_EVENT("Audio.Update.SurroundLFECompressor");

	Data* pInitData = (Data*)m_pObject->GetInitData( m_uInitOffset );
    State* pState = (State*)pInstance->GetNewStateData( m_uStateOffset );
	Core::DSP::Compressor* pDSP = (Core::DSP::Compressor*)pInstance->GetCoreData( m_uDSP );

	//  get attack
	F32 fAttack = Maths::Max( GetOptionValue( pInstance, pInitData->optAttack ), 0.f );

    //  has changed
    if( pState->fAttack != fAttack )
    {
        pState->fAttack = fAttack;
		pDSP->SetParam( Core::DSP::Compressor::eAttack, fAttack );
    }

    //  get release
	F32 fRelease = Maths::Max( GetOptionValue( pInstance, pInitData->optRelease ), 0.f );

	//  has changed
    if( pState->fRelease != fRelease )
    {
		pState->fRelease = fRelease;
		pDSP->SetParam( Core::DSP::Compressor::eRelease, fRelease );
	}

    //  get threshold
	F32 fThreshold = Maths::Max( GetLevel( pInstance, pInitData->optThresholdLevel, pInitData->optThresholdGain ), 0.f );

	//  has changed
    if( pState->fThreshold != fThreshold )
    {
		pState->fThreshold = fThreshold;
		pDSP->SetParam( Core::DSP::Compressor::eThreshold, fThreshold );
	}

    //  get ratio
	F32 fRatio = Maths::Clamp( GetOptionValue( pInstance, pInitData->optRatio ), 0.1f, 10000.f );

	//  has changed
    if( pState->fRatio != fRatio )
    {
		pState->fRatio = fRatio;
		pDSP->SetParam( Core::DSP::Compressor::eRatio, fRatio );
	}

    //  get gain make up
	F32 fGain = GetLevel( pInstance, pInitData->optMakeUpLevel, pInitData->optMakeUpGain );

	//  has changed
    if( pState->fGain != fGain )
    {
		pState->fGain = fGain;
		pDSP->SetParam( Core::DSP::Compressor::eGain, fGain );
	}

	if( pInitData->ctrlSideChainRMS.sCom != eUndefined )
	{
		//  get side chain rms
		F32 fSideChainRMS = GetControlValue( pInstance, pInitData->ctrlSideChainRMS );

		//  has changed
		if( pState->fSideChainRMS != fSideChainRMS )
		{
			pState->fSideChainRMS = fSideChainRMS;
			pDSP->SetParam( Core::DSP::Compressor::eSideChainRMS, fSideChainRMS );
		}
	}

	//	get current attenuation
	pState->fValue = pDSP->GetParam( Core::DSP::Compressor::eAttenuation );
}

////////////////////////////////////////////////////////////////////////////////
//	Value
////////////////////////////////////////////////////////////////////////////////
F32 Audio::Audio::SurroundLFECompressor::Value(Instance* pInstance, U32 uOutput)
{
    State* pState = (State*)pInstance->GetStateData(m_uStateOffset);

	switch(uOutput)
    {
    case ValueCtrl:
		return pState->fValue;
        break;
	default:			
		Assert(0);
        break;
    }

    return 0.f;
}

////////////////////////////////////////////////////////////////////////////////
//	DoMessage
////////////////////////////////////////////////////////////////////////////////
void Audio::Audio::SurroundLFECompressor::DoMessage( Instance* pInstance, U32 uMessage )
{
	Core::DSP::Effect* pDSP = (Core::DSP::Effect*)pInstance->GetCoreData( m_uDSP );

	switch( uMessage )
    {
    case EnableMsg:
		pDSP->SetActive( true );
        break;

    case BypassMsg:
		pDSP->SetActive( false );
        break;

    default:
        Assert(0);
        break;
    }
}

#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", on) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF

