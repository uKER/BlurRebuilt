#include "AudioPch.h"
#ifdef PLATFORM_SPU
#include "SystemHeadersSPU.h"
#else
#include "../SystemHeaders.h"
#include "ComponentHelper.h"
#endif

//using namespace Audio;

#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", off) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF

////////////////////////////////////////////////////////////////////////////////
//	Init
////////////////////////////////////////////////////////////////////////////////
#ifndef PLATFORM_SPU
void* Audio::Envelope::Init(void* pData)
{
	Data* pInitData = PreInit<Data>(pData);
    return (void*)(pInitData + 1);
}

////////////////////////////////////////////////////////////////////////////////
//	Envelope
////////////////////////////////////////////////////////////////////////////////
U32 Audio::Envelope::ContextSize()
{
	return sizeof(State);
}

////////////////////////////////////////////////////////////////////////////////
//	InitContext
////////////////////////////////////////////////////////////////////////////////
void Audio::Envelope::InitContext(Instance* pInstance)
{
    State* pState = (State*)pInstance->GetStateData(m_uStateOffset);

    pState->fCurrent = 0.f;
	pState->fHold = 0.f;
	pState->fDelay = 0.f;
}
#endif	//	#ifndef PLATFORM_SPU

////////////////////////////////////////////////////////////////////////////////
//	Update
////////////////////////////////////////////////////////////////////////////////
void Audio::Envelope::Update(Instance* pInstance)
{
	AUDIO_PERF_MICRO_EVENT("Audio.Update.Envelope");

	Data* pInitData = (Data*)m_pObject->GetInitData(m_uInitOffset);
    State* pState = (State*)pInstance->GetNewStateData(m_uStateOffset);

	//	frame length in seconds
	F32 fFrameTime = Core::fFrameLength * Core::fRcpSampleRate * (F32)pInstance->m_coreInfo.uFrameInt;

    //  input
    F32 fInput = GetControlValue( pInstance, pInitData->ctrlInput );

    if( pState->fCurrent > fInput )
    {
		//	above threshold
        if( pState->fHold > 0.f )
        {
			//	holding
            pState->fHold -= fFrameTime;

            if( pState->fHold <= 0.f )
			{
				//	finished hold
				pState->fHold = 0.f;

				//  execute hold phase complete script
				DoEvent( pInstance, pInitData->evHoldEnd );
			}
        }
        else
        {
			//  get release and clamp
			F32 fRelease = Maths::Max( GetOptionValue( pInstance, pInitData->optRelease ), Maths::fEpsilon );

			//	do delta release
			pState->fCurrent -= fFrameTime * Maths::RcpEst( fRelease );

			if( pState->fCurrent < fInput )
			{
				//	finished release
				pState->fCurrent = fInput;

				//  execute release phase complete script
				DoEvent( pInstance, pInitData->evReleaseEnd );

				//  set delay time
				pState->fDelay = Maths::Max( GetOptionValue( pInstance, pInitData->optDelay ), 0.f );
			}
		}
    }
    else if( pState->fCurrent < fInput )
    {
		//	below threshold
        if( pState->fDelay > 0.f )
        {
			//	delaying
            pState->fDelay -= fFrameTime;

            if( pState->fDelay <= 0.f )
			{
				//	finshed delay
				pState->fDelay = 0.f;

				//  execute delay phase complete script
				DoEvent( pInstance, pInitData->evDelayEnd );
			}
        }
        else
        {
			//  get attack and clamp
			F32 fAttack = Maths::Max( GetOptionValue( pInstance, pInitData->optAttack ), Maths::fEpsilon );

			//	do delta attack
			pState->fCurrent += fFrameTime * Maths::RcpEst( fAttack );

			if( pState->fCurrent >= fInput )
			{
				//	finished attack
				pState->fCurrent = fInput;

				//  execute attack phase complete script
				DoEvent( pInstance, pInitData->evAttackEnd );

				//  set hold time
				pState->fHold = Maths::Max( GetOptionValue( pInstance, pInitData->optHold ), 0.f );
			}
		}
    }
}

////////////////////////////////////////////////////////////////////////////////
//	Value
////////////////////////////////////////////////////////////////////////////////
F32 Audio::Envelope::Value(Instance* pInstance, U32 uOutput)
{
	U8* pStateData = pInstance->GetStateData();
    State* pState = (State*)&pStateData[m_uStateOffset];

    switch(uOutput)
    {
    case ValueCtrl:
        return pState->fCurrent;
        break;

    default:
        Assert(0);
        break;
    }

    return 0.f;
}

#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", on) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF
