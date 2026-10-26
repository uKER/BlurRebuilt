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
void* Audio::MonoDelay::Init(void* pData)
{
	Data* pInitData = PreInit<Data>(pData);
    return (void*)(pInitData + 1);
}

////////////////////////////////////////////////////////////////////////////////
//	CoreMemory
////////////////////////////////////////////////////////////////////////////////
U32 Audio::MonoDelay::CoreMemory(U32 uOffset)
{
	m_uDSP = (ixCore)uOffset;

	uOffset += AUDIO_CORE_ALIGN_16(sizeof(Core::DSP::Delay));

	return uOffset;
}

////////////////////////////////////////////////////////////////////////////////
//	InitContext
////////////////////////////////////////////////////////////////////////////////
void Audio::MonoDelay::InitContext(Instance* pInstance)
{
    State* pState = (State*)pInstance->GetStateData(m_uStateOffset);
	Data* pInitData = (Data*)m_pObject->GetInitData(m_uInitOffset);

	U32 uMaxLength = (U32)Maths::Max( pInitData->fMaxLength * (F32)Core::uSampleRate * Core::fRcpFrameLength, 0.1f );
	U32 uDownSample = (U32)Maths::Max( Maths::Floor( pInitData->fDownSample ), 0.f );

	new( pInstance->GetCoreData( m_uDSP ) ) Core::DSP::Delay( Name(), 1, uMaxLength, uDownSample );

    pState->fLength = 0.f;
    pState->fWet = 1.f;
    pState->fDry = 0.f;
}

////////////////////////////////////////////////////////////////////////////////
//	ReleaseContext
////////////////////////////////////////////////////////////////////////////////
void Audio::MonoDelay::ReleaseContext(Instance* pInstance)
{
	Core::DSP::Delay* pDSP = (Core::DSP::Delay*)pInstance->GetCoreData( m_uDSP );

	delete pDSP;
}
#endif	//	#ifndef PLATFORM_SPU

////////////////////////////////////////////////////////////////////////////////
//	Update
////////////////////////////////////////////////////////////////////////////////
void Audio::MonoDelay::Update(Instance* pInstance)
{
	AUDIO_PERF_MICRO_EVENT("Audio.Update.MonoDelay");

	Data* pInitData = (Data*)m_pObject->GetInitData(m_uInitOffset);
    State* pState = (State*)pInstance->GetNewStateData(m_uStateOffset);
	Core::DSP::Delay* pDSP = (Core::DSP::Delay*)pInstance->GetCoreData( m_uDSP );

    //  get length
	F32 fLength = GetOptionValue( pInstance, pInitData->optLength );

    //  set length
    if( pState->fLength != fLength )
    {
        pState->fLength = fLength;
		pDSP->SetParam(Core::DSP::Delay::eLength, fLength);
    }

    //  get wet
	F32 fWet = GetOptionValue( pInstance, pInitData->optWet );

    //  set wet
    if( pState->fWet != fWet )
    {
        pState->fWet = fWet;
		pDSP->SetParam( Core::DSP::Delay::eWet, fWet );
    }

    //  get dry
	F32 fDry = GetOptionValue( pInstance, pInitData->optDry );

    //  set dry
    if( pState->fDry != fDry )
    {
        pState->fDry = fDry;
		pDSP->SetParam( Core::DSP::Delay::eDry, fDry );
    }
}

////////////////////////////////////////////////////////////////////////////////
//	DoMessage
////////////////////////////////////////////////////////////////////////////////
void Audio::MonoDelay::DoMessage( Instance* pInstance, U32 uMessage )
{
	Core::DSP::Delay* pDSP = (Core::DSP::Delay*)pInstance->GetCoreData( m_uDSP );

	switch( uMessage )
    {
    case EnableMsg:
		pDSP->SetActive( true );
        break;

    case BypassMsg:
		pDSP->SetActive( false );
        break;

    case ResetMsg:
		pDSP->Reset();
        break;

    default:
        Assert(0);
        break;
    }
}

