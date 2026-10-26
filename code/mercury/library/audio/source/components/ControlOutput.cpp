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
void* Audio::ControlOutput::Init(void* pData)
{
	Data* pInitData = PreInit<Data>(pData);
    return (void*)(pInitData + 1);
}

////////////////////////////////////////////////////////////////////////////////
//	InitContext
////////////////////////////////////////////////////////////////////////////////
void Audio::ControlOutput::InitContext(Instance* pInstance)
{
    State* pState = (State*)pInstance->GetStateData(m_uStateOffset);

    pState->pInstanceIn = NULL;
    pState->uControlInput = -1;
}
#endif	//	#ifndef PLATFORM_SPU

////////////////////////////////////////////////////////////////////////////////
//	Value
////////////////////////////////////////////////////////////////////////////////
F32 Audio::ControlOutput::Value(Instance* pInstance, U32 uOutput)
{
	Data* pInitData = (Data*)m_pObject->GetInitData(m_uInitOffset);

#ifdef AUDIO_NEW_OUTPUT_OUTPUTS
	return GetOptionValue( pInstance, pInitData->optInput );
#else
    return m_pObject->GetComponentValue(pInitData->sInputCom, pInstance, 0);
#endif
}

////////////////////////////////////////////////////////////////////////////////
//	SetInput
////////////////////////////////////////////////////////////////////////////////
// this function is kind of meaningless since one control output can map to many control inputs.. so here
// it's only storing the most recent connection... could be removed?
void Audio::ControlOutput::SetInput(Instance* pThisInstance, Instance* pThatInstance, Audio::Com uControlInput)
{
    State* pState = (State*)pThisInstance->GetStateData(m_uStateOffset);

    pState->pInstanceIn = pThatInstance;
    pState->uControlInput = uControlInput;
}

#if defined (AUDIO_ENABLE_PROFILING)
////////////////////////////////////////////////////////////////////////////////
//	GetOtherInstance
////////////////////////////////////////////////////////////////////////////////
Audio::Instance* Audio::ControlOutput::GetOtherInstance(Instance* pThisInstance)
{
    State* pState = (State*)pThisInstance->GetStateData(m_uStateOffset);

	return pState->pInstanceIn;
}

////////////////////////////////////////////////////////////////////////////////
//	GetOtherComponent
////////////////////////////////////////////////////////////////////////////////
Audio::Com Audio::ControlOutput::GetOtherComponent(Instance* pThisInstance)
{
    State* pState = (State*)pThisInstance->GetStateData(m_uStateOffset);

	return pState->uControlInput;
}
#endif

#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", on) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF
