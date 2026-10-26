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
void* Audio::StereoHighPass::Init(void* pData)
{
	Data* pInitData = PreInit<Data>(pData);
    return (void*)(pInitData + 1);
}

////////////////////////////////////////////////////////////////////////////////
//	CoreMemory
////////////////////////////////////////////////////////////////////////////////
U32 Audio::StereoHighPass::CoreMemory(U32 uOffset)
{
	m_uDSP = (ixCore)uOffset;

	uOffset += AUDIO_CORE_ALIGN_16(sizeof(Core::DSP::IIRHighPass1stOrder));

	return uOffset;
}

////////////////////////////////////////////////////////////////////////////////
//	InitContext
////////////////////////////////////////////////////////////////////////////////
void Audio::StereoHighPass::InitContext(Instance* pInstance)
{
    State* pState = (State*)pInstance->GetStateData(m_uStateOffset);

	new( pInstance->GetCoreData( m_uDSP ) ) Core::DSP::IIRHighPass1stOrder( Name(), 2 );

    pState->fCutoff = 1.f;
    pState->fLevel = 1.f;
}

////////////////////////////////////////////////////////////////////////////////
//	ReleaseContext
////////////////////////////////////////////////////////////////////////////////
void Audio::StereoHighPass::ReleaseContext(Instance* pInstance)
{
	Core::DSP::IIRHighPass1stOrder* pDSP = (Core::DSP::IIRHighPass1stOrder*)pInstance->GetCoreData( m_uDSP );

	delete pDSP;
}
#endif	//	#ifndef PLATFORM_SPU

////////////////////////////////////////////////////////////////////////////////
//	Update
////////////////////////////////////////////////////////////////////////////////
void Audio::StereoHighPass::Update(Instance* pInstance)
{
    AUDIO_PERF_MICRO_EVENT("Audio.Update.StereoHighPass");

	Data* pInitData = (Data*)m_pObject->GetInitData(m_uInitOffset);
    State* pState = (State*)pInstance->GetNewStateData(m_uStateOffset);
	Core::DSP::IIRHighPass1stOrder* pDSP = (Core::DSP::IIRHighPass1stOrder*)pInstance->GetCoreData( m_uDSP );

    //  get cutoff
	F32 fCutoff = GetOptionValue( pInstance, pInitData->optCutoff );

    //  adjust freq
	fCutoff *= GetFrequency( pInstance, pInitData->optFreqShift, pInitData->optPitchShift );

    //  has changed
    if( pState->fCutoff != fCutoff )
    {
        pState->fCutoff = fCutoff;
		pDSP->SetParam( Core::DSP::IIRHighPass1stOrder::eCutoff, fCutoff );
    }

    //  get level
	F32 fLevel = GetLevel( pInstance, pInitData->optLevel, pInitData->optGain );

    //  has changed
    if( pState->fLevel != fLevel )
    {
        pState->fLevel = fLevel;
		pDSP->SetParam( Core::DSP::IIRHighPass1stOrder::eGain, fLevel );
    }
}

////////////////////////////////////////////////////////////////////////////////
//	DoMessage
////////////////////////////////////////////////////////////////////////////////
void Audio::StereoHighPass::DoMessage( Instance* pInstance, U32 uMessage )
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
