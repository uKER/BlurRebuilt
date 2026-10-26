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
void* Audio::SurroundRingModulator::Init( void* pData )
{
	Data* pInitData = PreInit<Data>(pData);
    return (void*)(pInitData + 1);
}

////////////////////////////////////////////////////////////////////////////////
//	CoreMemory
////////////////////////////////////////////////////////////////////////////////
U32 Audio::SurroundRingModulator::CoreMemory(U32 uOffset)
{
	m_uDSP = (ixCore)uOffset;

	uOffset += AUDIO_CORE_ALIGN_16( sizeof( Core::DSP::RingModulator ) );

	return uOffset;
}

////////////////////////////////////////////////////////////////////////////////
//	InitContext
////////////////////////////////////////////////////////////////////////////////
void Audio::SurroundRingModulator::InitContext( Instance* pInstance )
{
    State* pState = (State*)pInstance->GetStateData( m_uStateOffset );
	Data* pInitData = (Data*)m_pObject->GetInitData( m_uInitOffset );

	new( pInstance->GetCoreData( m_uDSP ) ) Core::DSP::RingModulator( Name(), 5 );

    pState->fRate = Maths::fInvalid;
    pState->fWet = Maths::fInvalid;
    pState->fDry = Maths::fInvalid;
}

////////////////////////////////////////////////////////////////////////////////
//	ReleaseContext
////////////////////////////////////////////////////////////////////////////////
void Audio::SurroundRingModulator::ReleaseContext( Instance* pInstance )
{
	Core::DSP::RingModulator* pDSP = (Core::DSP::RingModulator*)pInstance->GetCoreData( m_uDSP );

	delete pDSP;
}
#endif // #ifndef PLATFORM_SPU

////////////////////////////////////////////////////////////////////////////////
//	Update
////////////////////////////////////////////////////////////////////////////////
void Audio::SurroundRingModulator::Update( Instance* pInstance )
{
    AUDIO_PERF_MICRO_EVENT("Audio.Update.SurroundRingModulator");

	Data* pInitData = (Data*)m_pObject->GetInitData( m_uInitOffset );
    State* pState = (State*)pInstance->GetNewStateData( m_uStateOffset );
	Core::DSP::RingModulator* pDSP = (Core::DSP::RingModulator*)pInstance->GetCoreData( m_uDSP );

    //  get rate
	F32 fRate = GetOptionValue( pInstance, pInitData->optRate );

    //  set rate
    if( pState->fRate != fRate )
    {
        pState->fRate = fRate;
		pDSP->SetParam( Core::DSP::RingModulator::eRate, fRate );
    }

    //  get wet
	F32 fWet = GetOptionValue( pInstance, pInitData->optWet );

    //  set wet
    if( pState->fWet != fWet )
    {
        pState->fWet = fWet;
		pDSP->SetParam( Core::DSP::RingModulator::eWet, fWet );
    }

    //  get dry
	F32 fDry = GetOptionValue( pInstance, pInitData->optDry );

    //  set dry
    if( pState->fDry != fDry )
    {
        pState->fDry = fDry;
		pDSP->SetParam( Core::DSP::RingModulator::eDry, fDry );
    }
}

////////////////////////////////////////////////////////////////////////////////
//	DoMessage
////////////////////////////////////////////////////////////////////////////////
void Audio::SurroundRingModulator::DoMessage( Instance* pInstance, U32 uMessage )
{
	Core::DSP::RingModulator* pDSP = (Core::DSP::RingModulator*)pInstance->GetCoreData( m_uDSP );

	switch( uMessage )
    {
    case EnableMsg:
		pDSP->SetActive( true );
        break;

    case BypassMsg:
		pDSP->SetActive( false );
        break;

    case ReSyncInPhaseMsg:
		pDSP->Sync( true );
        break;

    case ReSyncOutPhaseMsg:
		pDSP->Sync( false );
        break;

    default:
        Assert(0);
        break;
    }
}

