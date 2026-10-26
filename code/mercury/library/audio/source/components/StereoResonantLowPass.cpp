#include "AudioPch.h"
#ifdef PLATFORM_SPU
#include "SystemHeadersSPU.h"
#else
#include "../SystemHeaders.h"
#include "ComponentHelper.h"
#endif

////////////////////////////////////////////////////////////////////////////////
//	Init
////////////////////////////////////////////////////////////////////////////////
#ifndef PLATFORM_SPU
void* Audio::StereoResonantLowPass::Init(void* pData)
{
	Data* pInitData = PreInit<Data>(pData);
    return (void*)(pInitData + 1);
}

////////////////////////////////////////////////////////////////////////////////
//	CoreMemory
////////////////////////////////////////////////////////////////////////////////
U32 Audio::StereoResonantLowPass::CoreMemory(U32 uOffset)
{
	m_uDSP = (ixCore)uOffset;

	uOffset += AUDIO_CORE_ALIGN_16(sizeof(Core::DSP::IIRLowPass2ndOrder));

	return uOffset;
}

////////////////////////////////////////////////////////////////////////////////
//	InitContext
////////////////////////////////////////////////////////////////////////////////
void Audio::StereoResonantLowPass::InitContext(Instance* pInstance)
{
    State* pState = (State*)pInstance->GetStateData(m_uStateOffset);

	new( pInstance->GetCoreData( m_uDSP ) ) Core::DSP::IIRLowPass2ndOrder( Name(), 2 );

    pState->fCutoff = -1.f;
    pState->fQ = -1.f;
    pState->fLevel = -1.f;
    pState->fClamp = -1.f;
}

////////////////////////////////////////////////////////////////////////////////
//	ReleaseContext
////////////////////////////////////////////////////////////////////////////////
void Audio::StereoResonantLowPass::ReleaseContext(Instance* pInstance)
{
	Core::DSP::IIRLowPass2ndOrder* pDSP = (Core::DSP::IIRLowPass2ndOrder*)pInstance->GetCoreData( m_uDSP );

	delete pDSP;
}
#endif	//	#ifndef PLATFORM_SPU

////////////////////////////////////////////////////////////////////////////////
//	Update
////////////////////////////////////////////////////////////////////////////////
void Audio::StereoResonantLowPass::Update(Instance* pInstance)
{
    AUDIO_PERF_MICRO_EVENT("Audio.Update.StereoResonantLowPass");

	Data* pInitData = (Data*)m_pObject->GetInitData(m_uInitOffset);
    State* pState = (State*)pInstance->GetNewStateData(m_uStateOffset);
	Core::DSP::IIRLowPass2ndOrder* pDSP = (Core::DSP::IIRLowPass2ndOrder*)pInstance->GetCoreData( m_uDSP );

    //  get cutoff
	F32 fCutoff = GetOptionValue( pInstance, pInitData->optCutoff );

    //  adjust freq
	fCutoff *= GetFrequency( pInstance, pInitData->optFreqShift, pInitData->optPitchShift );

    //  has changed
    if( pState->fCutoff != fCutoff )
    {
        pState->fCutoff = fCutoff;
		pDSP->SetParam( Core::DSP::IIRLowPass2ndOrder::eCutoff, fCutoff );
    }

    //  get q
	F32 fQ = GetOptionValue( pInstance, pInitData->optQ );

    //  has changed
    if( pState->fQ != fQ )
    {
        pState->fQ = fQ;
		pDSP->SetParam( Core::DSP::IIRLowPass2ndOrder::eQ, fQ );
    }

    //  get level
	F32 fLevel = GetLevel( pInstance, pInitData->optLevel, pInitData->optGain );

    //  has changed
    if( pState->fLevel != fLevel )
    {
        pState->fLevel = fLevel;
		pDSP->SetParam( Core::DSP::IIRLowPass2ndOrder::eGain, fLevel );
    }

    //  get clamp
	F32 fClamp = GetOptionValue( pInstance, pInitData->optClamp );

    //  has changed
    if( pState->fClamp != fClamp )
    {
        pState->fClamp = fClamp;
		pDSP->SetParam( Core::DSP::IIRLowPass2ndOrder::eClamp, fClamp );
    }
}

////////////////////////////////////////////////////////////////////////////////
//	DoMessage
////////////////////////////////////////////////////////////////////////////////
void Audio::StereoResonantLowPass::DoMessage( Instance* pInstance, U32 uMessage )
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

