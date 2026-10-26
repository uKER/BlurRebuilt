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
void* Audio::ImpactFilter::Init( void* pData )
{
	Data* pInitData = PreInit<Data>(pData);
    return (void*)(pInitData + 1);
}

////////////////////////////////////////////////////////////////////////////////
//	InitContext
////////////////////////////////////////////////////////////////////////////////
void Audio::ImpactFilter::InitContext(Instance* pInstance)
{
	Data* pInitData = ( Data* )m_pObject->GetInitData( m_uInitOffset );
	State* pState = ( State* )pInstance->GetStateData( m_uStateOffset );

	pState->bImpact = false;
	pState->fThreshold = 0.f;
	pState->fDelay = 0.f;
}
#endif	//	#ifndef PLATFORM_SPU

////////////////////////////////////////////////////////////////////////////////
//	Update
////////////////////////////////////////////////////////////////////////////////
void Audio::ImpactFilter::Update( Instance* pInstance )
{
	AUDIO_PERF_MICRO_EVENT("Audio.Update.ImpactFilter");

	Data* pInitData = ( Data* )m_pObject->GetInitData( m_uInitOffset );
	State* pState = ( State* )pInstance->GetNewStateData( m_uStateOffset );

	if( pState->fDelay > 0.f )
	{
		//	delay before decaying force
		pState->fDelay -= Core::fFrameLength * Core::fRcpSampleRate * (F32)pInstance->m_coreInfo.uFrameInt;

		if( pState->fDelay < 0.f )
			pState->fDelay = 0.f;
	}
	else
	{
		if( pState->bImpact )
		{
			//	clear impact flag
			pState->bImpact = false;

			//	get force
			F32 fForce = GetControlValue( pInstance, pInitData->ctrlForce );

			//	if force larger than current threshold
			if( fForce > pState->fThreshold )
			{
				//	set new threshold to impact force
				pState->fThreshold = fForce;

				//	reset delay time
				pState->fDelay = GetOptionValue( pInstance, pInitData->optDelay );

				//  trigger impact event
				DoEvent( pInstance, pInitData->evImpact );
			}
		}

		//	get half life
		F32 fHalfLife = GetOptionValue( pInstance, pInitData->optHalfLife );

		//	adjust threshold
		pState->fThreshold *= Maths::Power( 0.5f, fHalfLife * Core::fFrameLength * Core::fRcpSampleRate * (F32)pInstance->m_coreInfo.uFrameInt );

		//	clamp to min value
		if( pState->fThreshold < Maths::fMin )
			pState->fThreshold = 0.f;
	}
}

////////////////////////////////////////////////////////////////////////////////
//	Value
////////////////////////////////////////////////////////////////////////////////
F32 Audio::ImpactFilter::Value(Instance* pInstance, U32 uOutput)
{
	State* pState = (State*)pInstance->GetStateData(m_uStateOffset);

	switch( uOutput )
    {
    case ValueCtrl:
        return pState->fThreshold;

	default:
        Assert(0);
    }

    return 0.f;
}

////////////////////////////////////////////////////////////////////////////////
//	DoMessage
////////////////////////////////////////////////////////////////////////////////
void Audio::ImpactFilter::DoMessage( Instance* pInstance, U32 uMessage )
{
	switch( uMessage )
    {
    case ImpactMsg:
        Impact( pInstance );
        break;

    case ResetMsg:
        Reset( pInstance );
        break;

    default:
        Assert(0);
        break;
    }
}

////////////////////////////////////////////////////////////////////////////////
//	Impact
////////////////////////////////////////////////////////////////////////////////
void Audio::ImpactFilter::Impact( Instance* pInstance )
{
	Data* pInitData = ( Data* )m_pObject->GetInitData( m_uInitOffset );
    State* pState = ( State* )pInstance->GetNewStateData( m_uStateOffset );

	pState->bImpact = true;
}

////////////////////////////////////////////////////////////////////////////////
//	Reset
////////////////////////////////////////////////////////////////////////////////
void Audio::ImpactFilter::Reset(Instance* pInstance)
{
	Data* pInitData = ( Data* )m_pObject->GetInitData( m_uInitOffset );
    State* pState = ( State* )pInstance->GetNewStateData( m_uStateOffset );

	pState->bImpact = false;
	pState->fThreshold = 0.f;
	pState->fDelay = 0.f;
}

#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", on) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF
