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
void* Audio::MonoBitCrusher::Init(void* pData)
{
	Data* pInitData = PreInit<Data>(pData);
    return (void*)(pInitData + 1);
}

////////////////////////////////////////////////////////////////////////////////
//	CoreMemory
////////////////////////////////////////////////////////////////////////////////
U32 Audio::MonoBitCrusher::CoreMemory(U32 uOffset)
{
	m_uDSP = (ixCore)uOffset;

	uOffset += AUDIO_CORE_ALIGN_16(sizeof(Core::DSP::BitCrusher));

	return uOffset;
}

////////////////////////////////////////////////////////////////////////////////
//	InitContext
////////////////////////////////////////////////////////////////////////////////
void Audio::MonoBitCrusher::InitContext(Instance* pInstance)
{
    State* pState = (State*)pInstance->GetStateData(m_uStateOffset);

	new( pInstance->GetCoreData( m_uDSP ) ) Core::DSP::BitCrusher( Name(), 1 );

	pState->fDepth = Maths::fInvalid;
	pState->fLevel = Maths::fInvalid;
}

////////////////////////////////////////////////////////////////////////////////
//	ReleaseContext
////////////////////////////////////////////////////////////////////////////////
void Audio::MonoBitCrusher::ReleaseContext(Instance* pInstance)
{
	delete (Core::DSP::BitCrusher*)pInstance->GetCoreData( m_uDSP );
}
#endif	//	#ifndef PLATFORM_SPU

////////////////////////////////////////////////////////////////////////////////
//	Update
////////////////////////////////////////////////////////////////////////////////
void Audio::MonoBitCrusher::Update(Instance* pInstance)
{
	AUDIO_PERF_MICRO_EVENT("Audio.Update.MonoBitCrusher");

	Data* pInitData = (Data*)m_pObject->GetInitData(m_uInitOffset);
    State* pState = (State*)pInstance->GetNewStateData(m_uStateOffset);
	Core::DSP::BitCrusher* pDSP = (Core::DSP::BitCrusher*)pInstance->GetCoreData( m_uDSP );

    //  get depth
	F32 fDepth = GetOptionValue( pInstance, pInitData->optDepth );

    //  set depth
	if( pState->fDepth != fDepth )
    {
        pState->fDepth = fDepth;
		pDSP->SetParam( Core::DSP::BitCrusher::eDepth, fDepth );
    }

    //  get level
	F32 fLevel = GetLevel( pInstance, pInitData->optLevel, pInitData->optGain );

    //  has changed
    if( pState->fLevel != fLevel )
    {
        pState->fLevel = fLevel;
		pDSP->SetParam( Core::DSP::BitCrusher::eLevel, fLevel );
    }
}

////////////////////////////////////////////////////////////////////////////////
//	DoMessage
////////////////////////////////////////////////////////////////////////////////
void Audio::MonoBitCrusher::DoMessage( Instance* pInstance, U32 uMessage )
{
	Core::DSP::BitCrusher* pDSP = (Core::DSP::BitCrusher*)pInstance->GetCoreData( m_uDSP );

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


