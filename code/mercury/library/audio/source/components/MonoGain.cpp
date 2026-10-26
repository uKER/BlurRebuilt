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
void* Audio::MonoGain::Init(void* pData)
{
	Data* pInitData = PreInit<Data>(pData);
    return (void*)(pInitData + 1);
}

////////////////////////////////////////////////////////////////////////////////
//	CoreMemory
////////////////////////////////////////////////////////////////////////////////
U32 Audio::MonoGain::CoreMemory(U32 uOffset)
{
	m_uDSP = (ixCore)uOffset;

	uOffset += AUDIO_CORE_ALIGN_16(sizeof(Core::DSP::Gain));

	return uOffset;
}

////////////////////////////////////////////////////////////////////////////////
//	InitContext
////////////////////////////////////////////////////////////////////////////////
void Audio::MonoGain::InitContext(Instance* pInstance)
{
    State* pState = (State*)pInstance->GetStateData(m_uStateOffset);

	new( pInstance->GetCoreData( m_uDSP ) ) Core::DSP::Gain( Name(), 1 );

    pState->fGain = -1.f;
}

////////////////////////////////////////////////////////////////////////////////
//	ReleaseContext
////////////////////////////////////////////////////////////////////////////////
void Audio::MonoGain::ReleaseContext(Instance* pInstance)
{
	Core::DSP::Gain* pDSP = (Core::DSP::Gain*)pInstance->GetCoreData( m_uDSP );

	delete pDSP;
}
#endif	//	#ifndef PLATFORM_SPU

////////////////////////////////////////////////////////////////////////////////
//	Update
////////////////////////////////////////////////////////////////////////////////
void Audio::MonoGain::Update(Instance* pInstance)
{
	AUDIO_PERF_MICRO_EVENT("Audio.Update.MonoGain");

	Data* pInitData = (Data*)m_pObject->GetInitData(m_uInitOffset);
    State* pState = (State*)pInstance->GetNewStateData(m_uStateOffset);
	Core::DSP::Gain* pDSP = (Core::DSP::Gain*)pInstance->GetCoreData( m_uDSP );

    //  get gain
    F32 fGain = GetOptionValue( pInstance, pInitData->optGain );

    //  set gain
    if( pState->fGain != fGain )
    {
        pState->fGain = fGain;
		pDSP->SetParam(Core::DSP::Gain::eGain, fGain);
    }
}

////////////////////////////////////////////////////////////////////////////////
//	DoMessage
////////////////////////////////////////////////////////////////////////////////
void Audio::MonoGain::DoMessage( Instance* pInstance, U32 uMessage )
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


