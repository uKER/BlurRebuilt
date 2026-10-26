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
void* Audio::MonoFeedbackDelay::Init(void* pData)
{
	Data* pInitData = PreInit<Data>(pData);
    return (void*)(pInitData + 1);
}

////////////////////////////////////////////////////////////////////////////////
//	CoreMemory
////////////////////////////////////////////////////////////////////////////////
U32 Audio::MonoFeedbackDelay::CoreMemory(U32 uOffset)
{
	m_uDSP = (ixCore)uOffset;

	uOffset += AUDIO_CORE_ALIGN_16(sizeof(Core::DSP::FeedbackDelay));

	return uOffset;
}

////////////////////////////////////////////////////////////////////////////////
//	InitContext
////////////////////////////////////////////////////////////////////////////////
void Audio::MonoFeedbackDelay::InitContext(Instance* pInstance)
{
	Data* pInitData = (Data*)m_pObject->GetInitData(m_uInitOffset);
    State* pState = (State*)pInstance->GetStateData(m_uStateOffset);

	//	ignore -ve lengths
	F32 fMaxLength = Maths::Max(pInitData->fMaxLength, 0.f);

	//	get delay buffer size in frames
	U32 uBufferSize = (U32)(fMaxLength * Core::fSampleRate * Core::fRcpFrameLength);

	new( pInstance->GetCoreData( m_uDSP ) ) Core::DSP::FeedbackDelay( Name(), 1, uBufferSize );

    pState->fLength = 0.f;
}

////////////////////////////////////////////////////////////////////////////////
//	ReleaseContext
////////////////////////////////////////////////////////////////////////////////
void Audio::MonoFeedbackDelay::ReleaseContext(Instance* pInstance)
{
	Core::DSP::FeedbackDelay* pDSP = (Core::DSP::FeedbackDelay*)pInstance->GetCoreData( m_uDSP );

	delete pDSP;
}
#endif	//	#ifndef PLATFORM_SPU

////////////////////////////////////////////////////////////////////////////////
//	Update
////////////////////////////////////////////////////////////////////////////////
void Audio::MonoFeedbackDelay::Update(Instance* pInstance)
{
	AUDIO_PERF_MICRO_EVENT("Audio.Update.MonoFeedbackDelay");

	Data* pInitData = (Data*)m_pObject->GetInitData(m_uInitOffset);
    State* pState = (State*)pInstance->GetNewStateData(m_uStateOffset);

	Core::DSP::FeedbackDelay* pDSP = (Core::DSP::FeedbackDelay*)pInstance->GetCoreData( m_uDSP );

    //  get length
    F32 fLength = GetOptionValue( pInstance, pInitData->optLength );

    //  set length
    if (pState->fLength != fLength)
    {
        pState->fLength = fLength;
		pDSP->SetParam(Core::DSP::FeedbackDelay::eLength, fLength);
    }

    //  get feedback
    F32 fFeedback = GetOptionValue( pInstance, pInitData->optFeedback );

    //  set feedback
    if (pState->fFeedback != fFeedback)
    {
        pState->fFeedback = fFeedback;
		pDSP->SetParam(Core::DSP::FeedbackDelay::eFeedback, fFeedback);
    }

    //  get wet
    F32 fWet = GetLevel( pInstance, pInitData->optWetLevel, pInitData->optWetGain );

    //  set wet
    if (pState->fWet != fWet)
    {
        pState->fWet = fWet;
		pDSP->SetParam(Core::DSP::FeedbackDelay::eWet, fWet);
    }

    //  get dry
    F32 fDry = GetLevel( pInstance, pInitData->optDryLevel, pInitData->optDryGain );

    //  set dry
    if (pState->fDry != fDry)
    {
        pState->fDry = fDry;
		pDSP->SetParam(Core::DSP::FeedbackDelay::eDry, fDry);
    }
}

////////////////////////////////////////////////////////////////////////////////
//	DoMessage
////////////////////////////////////////////////////////////////////////////////
void Audio::MonoFeedbackDelay::DoMessage( Instance* pInstance, U32 uMessage )
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

