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
void* Audio::Spring::Init(void* pData)
{
	Data* pInitData = PreInit<Data>(pData);
    return (void*)(pInitData + 1);
}

////////////////////////////////////////////////////////////////////////////////
//	Spring
////////////////////////////////////////////////////////////////////////////////
U32 Audio::Spring::ContextSize()
{
	return sizeof(State);
}

////////////////////////////////////////////////////////////////////////////////
//	InitContext
////////////////////////////////////////////////////////////////////////////////
void Audio::Spring::InitContext(Instance* pInstance)
{
    State* pState = (State*)pInstance->GetStateData(m_uStateOffset);

	pState->fPosition = 0.f;
	pState->fVelocity = 0.f;
}
#endif	//	#ifndef PLATFORM_SPU

////////////////////////////////////////////////////////////////////////////////
//	Update
////////////////////////////////////////////////////////////////////////////////
void Audio::Spring::Update(Instance* pInstance)
{
    AUDIO_PERF_MICRO_EVENT("Audio.Update.Spring");

	Data* pInitData = (Data*)m_pObject->GetInitData( m_uInitOffset );
    State* pState = (State*)pInstance->GetNewStateData( m_uStateOffset );

	//	frame length in seconds
	F32 fFrameTime = Core::fFrameLength * Core::fRcpSampleRate * (F32)pInstance->m_coreInfo.uFrameInt;

    //  input
	F32 fInput = GetControlValue( pInstance, pInitData->ctrlInput );

    //  get frequency and clamp
	F32 fFrequency = Maths::Clamp( fFrameTime * GetOptionValue( pInstance, pInitData->optFrequency ), 0.f, 2.f );

    //  get damping, clamp and scale
	F32 fDamping = Maths::Exp( -Maths::Max( GetOptionValue( pInstance, pInitData->optDamping ), 0.f ) );

	//	get acceleration
    F32 fOffset = fInput - pState->fPosition;
    F32 fAcc = fFrequency * fOffset;

	//	adjust velocity
    pState->fVelocity += fAcc;
	pState->fVelocity *= fDamping;

	//	get new position
    pState->fPosition += pState->fVelocity;
}

////////////////////////////////////////////////////////////////////////////////
//	Value
////////////////////////////////////////////////////////////////////////////////
F32 Audio::Spring::Value(Instance* pInstance, U32 uOutput)
{
    State* pState = (State*)pInstance->GetStateData(m_uStateOffset);

    switch(uOutput)
    {
    case PositionCtrl:
		return pState->fPosition;
        break;

    case SpeedCtrl:
        return pState->fVelocity;
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
