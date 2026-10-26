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
void* Audio::LowPass::Init(void* pData)
{
	Data* pInitData = PreInit<Data>(pData);
    return (void*)(pInitData + 1);
}

////////////////////////////////////////////////////////////////////////////////
//	InitContext
////////////////////////////////////////////////////////////////////////////////
void Audio::LowPass::InitContext(Instance* pInstance)
{
    State* pState = (State*)pInstance->GetStateData(m_uStateOffset);

	pState->fPrevInput = 0.f;
    pState->fValue = 0.f;
	pState->fPrevCutoff = -1.f;
}
#endif	//	#ifndef PLATFORM_SPU

////////////////////////////////////////////////////////////////////////////////
//	Update
////////////////////////////////////////////////////////////////////////////////
void Audio::LowPass::Update(Instance* pInstance)
{
	AUDIO_PERF_MICRO_EVENT("Audio.Update.LowPass");

	Data* pInitData = (Data*)m_pObject->GetInitData(m_uInitOffset);
    State* pState = (State*)pInstance->GetNewStateData(m_uStateOffset);

    //  cutoff
	F32 fCutoff = Maths::Max( GetOptionValue( pInstance, pInitData->optCutoff ), Maths::fEpsilon );

	//	has changed
	if (fCutoff != pState->fPrevCutoff)
	{
		pState->fB = Maths::ArcTan(Maths::fPi * fCutoff * Core::fFrameLength * Core::fRcpSampleRate * (F32)pInstance->m_coreInfo.uFrameInt);
        pState->fB = -(pState->fB - 1.f) / (1.f + pState->fB);
        pState->fA = 0.5f * (1.f - pState->fB);

        pState->fPrevCutoff = fCutoff;
	}

	//  input
    F32 fInput = GetControlValue( pInstance, pInitData->ctrlInput );

	//	get new value
    pState->fValue = pState->fA * (fInput + pState->fPrevInput) + pState->fB * pState->fValue;

    pState->fPrevInput = fInput;
}

////////////////////////////////////////////////////////////////////////////////
//	Value
////////////////////////////////////////////////////////////////////////////////
F32 Audio::LowPass::Value(Instance* pInstance, U32 uOutput)
{
    State* pState = (State*)pInstance->GetStateData(m_uStateOffset);

    switch(uOutput)
    {
    case ValueCtrl:
        return pState->fValue;
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
