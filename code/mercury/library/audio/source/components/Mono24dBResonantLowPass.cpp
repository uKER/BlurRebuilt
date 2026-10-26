#include "AudioPch.h"
#ifdef PLATFORM_SPU
#include "SystemHeadersSPU.h"
#else
#include "../SystemHeaders.h"
#include "ComponentHelper.h"
#endif

//using namespace Audio;

////////////////////////////////////////////////////////////////////////////////
//	Init
////////////////////////////////////////////////////////////////////////////////
#ifndef PLATFORM_SPU
void* Audio::Mono24dBResonantLowPass::Init(void* pData)
{
	Data* pInitData = PreInit<Data>(pData);
    return (void*)(pInitData + 1);
}

////////////////////////////////////////////////////////////////////////////////
//	CoreMemory
////////////////////////////////////////////////////////////////////////////////
U32 Audio::Mono24dBResonantLowPass::CoreMemory(U32 uOffset)
{	
	m_uDSP = (ixCore)uOffset;

	uOffset += AUDIO_CORE_ALIGN_16(sizeof(Core::DSP::IIRLowPass4thOrder));

	return uOffset;
}

////////////////////////////////////////////////////////////////////////////////
//	InitContext
////////////////////////////////////////////////////////////////////////////////
void Audio::Mono24dBResonantLowPass::InitContext(Instance* pInstance)
{
    State* pState = (State*)pInstance->GetStateData(m_uStateOffset);

	new( pInstance->GetCoreData( m_uDSP ) ) Core::DSP::IIRLowPass4thOrder( Name(), 1 );

    pState->fCutoff = -1.f;
    pState->fQ = -1.f;
    pState->fLevel = -1.f;
    pState->fClamp = -1.f;
}

////////////////////////////////////////////////////////////////////////////////
//	ReleaseContext
////////////////////////////////////////////////////////////////////////////////
void Audio::Mono24dBResonantLowPass::ReleaseContext(Instance* pInstance)
{
	Core::DSP::IIRLowPass4thOrder* pDSP = (Core::DSP::IIRLowPass4thOrder*)pInstance->GetCoreData( m_uDSP );

	delete pDSP;
}
#endif	//	#ifndef PLATFORM_SPU

////////////////////////////////////////////////////////////////////////////////
//	Update
////////////////////////////////////////////////////////////////////////////////
void Audio::Mono24dBResonantLowPass::Update(Instance* pInstance)
{
	AUDIO_PERF_MICRO_EVENT("Audio.Update.Mono24dBResonantLowPass");

	bool bCutoffChanged = false;
	bool bLevelChanged = false;

	Data* pInitData = (Data*)m_pObject->GetInitData(m_uInitOffset);
    State* pState = (State*)pInstance->GetNewStateData(m_uStateOffset);
	Core::DSP::IIRLowPass4thOrder* pDSP = (Core::DSP::IIRLowPass4thOrder*)pInstance->GetCoreData( m_uDSP );

    //  get cutoff
	F32 fCutoff = GetOptionValue( pInstance, pInitData->optCutoff );

    //  adjust freq
	fCutoff *= GetFrequency( pInstance, pInitData->optFreqShift, pInitData->optPitchShift );

    //  has changed
    if( pState->fCutoff != fCutoff )
    {
        pState->fCutoff = fCutoff;
		pDSP->SetParam( Core::DSP::IIRLowPass4thOrder::eCutoff, fCutoff );
    }

    //  get q
	F32 fQ = GetOptionValue( pInstance, pInitData->optQ );

    //  has changed
    if( pState->fQ != fQ )
    {
        pState->fQ = fQ;
		pDSP->SetParam( Core::DSP::IIRLowPass4thOrder::eQ, fQ );
    }

    //  get level
	F32 fLevel = GetLevel( pInstance, pInitData->optLevel, pInitData->optGain );

    //  has changed
    if( pState->fLevel != fLevel )
    {
        pState->fLevel = fLevel;
		pDSP->SetParam( Core::DSP::IIRLowPass4thOrder::eGain, fLevel );
    }

    //  get clamp
	F32 fClamp = GetOptionValue( pInstance, pInitData->optClamp );

    //  has changed
    if( pState->fClamp != fClamp )
    {
        pState->fClamp = fClamp;
		pDSP->SetParam( Core::DSP::IIRLowPass4thOrder::eClamp, fClamp );
    }
}

////////////////////////////////////////////////////////////////////////////////
//	DoMessage
////////////////////////////////////////////////////////////////////////////////
void Audio::Mono24dBResonantLowPass::DoMessage( Instance* pInstance, U32 uMessage )
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

