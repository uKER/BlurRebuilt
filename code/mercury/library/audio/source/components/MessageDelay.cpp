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
void* Audio::MessageDelay::Init(void* pData)
{
	Data* pInitData = PreInit<Data>(pData);
    return (void*)(pInitData + 1);
}

////////////////////////////////////////////////////////////////////////////////
//	InitContext
////////////////////////////////////////////////////////////////////////////////
void Audio::MessageDelay::InitContext(Instance* pInstance)
{
	State* pState = (State*)pInstance->GetStateData( m_uStateOffset );

    pState->fDelay = 0.f;
}
#endif	//	#ifndef PLATFORM_SPU

////////////////////////////////////////////////////////////////////////////////
//	Update
////////////////////////////////////////////////////////////////////////////////
void Audio::MessageDelay::Update( Instance* pInstance )
{
	AUDIO_PERF_MICRO_EVENT("Audio.Update.MessageDelay");

	Data* pInitData = (Data*)m_pObject->GetInitData( m_uInitOffset );
	State* pState = (State*)pInstance->GetNewStateData( m_uStateOffset );

	//	if active
	if( pState->fDelay != 0.f )
	{
		pState->fDelay -= Core::fFrameLength * Core::fRcpSampleRate * (F32)pInstance->m_coreInfo.uFrameInt;

		//  if time's up
		if( pState->fDelay <= 0.f )
		{
			pState->fDelay = 0.f;

			//  trigger delayed event
			DoEvent( pInstance, pInitData->evDelayed );
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
//	DoMessage
////////////////////////////////////////////////////////////////////////////////
void Audio::MessageDelay::DoMessage( Instance* pInstance, U32 uMessage )
{
	switch( uMessage )
    {
    case DelayMsg:
        Delay( pInstance );
        break;

    case CancelMsg:
        Cancel( pInstance );
        break;

    default:
        Assert(0);
        break;
    }
}

////////////////////////////////////////////////////////////////////////////////
//	Delay
////////////////////////////////////////////////////////////////////////////////
void Audio::MessageDelay::Delay( Instance* pInstance )
{
	Data* pInitData = (Data*)m_pObject->GetInitData( m_uInitOffset );
    State* pState = (State*)pInstance->GetNewStateData( m_uStateOffset );

    pState->fDelay = GetOptionValue( pInstance, pInitData->optLength );
}

////////////////////////////////////////////////////////////////////////////////
//	Cancel
////////////////////////////////////////////////////////////////////////////////
void Audio::MessageDelay::Cancel( Instance* pInstance )
{
    State* pState = (State*)pInstance->GetNewStateData( m_uStateOffset );

    pState->fDelay = 0.f;
}

#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", on) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF
