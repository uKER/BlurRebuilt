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
//	(constructor)
////////////////////////////////////////////////////////////////////////////////
Audio::ControlInput::ControlInput(Object* pObject) : Component(pObject)
{
}

////////////////////////////////////////////////////////////////////////////////
//	(destructor)
////////////////////////////////////////////////////////////////////////////////
Audio::ControlInput::~ControlInput()
{
}

////////////////////////////////////////////////////////////////////////////////
//	Init
////////////////////////////////////////////////////////////////////////////////
#ifndef PLATFORM_SPU
void* Audio::ControlInput::Init(void* pData)
{
	Data* pInitData = PreInit<Data>(pData);
    return (void*)(pInitData + 1);
}

////////////////////////////////////////////////////////////////////////////////
//	InitContext
////////////////////////////////////////////////////////////////////////////////
void Audio::ControlInput::InitContext(Instance* pInstance)
{
    State* pState = (State*)pInstance->GetStateData(m_uStateOffset);

    pState->pInstanceOut = NULL;
    pState->uControlOutput = -1;
	pState->f32Value = 0.f;
}
#endif	//	#ifndef PLATFORM_SPU

////////////////////////////////////////////////////////////////////////////////
//	Value
////////////////////////////////////////////////////////////////////////////////
F32 Audio::ControlInput::Value(Instance* pInstance, U32 uOutput)
{
    State* pState = (State*)pInstance->GetStateData(m_uStateOffset);

	return pState->f32Value;
}

////////////////////////////////////////////////////////////////////////////////
//	ProcessConnection
////////////////////////////////////////////////////////////////////////////////
void Audio::ControlInput::ProcessConnection(Instance* pInstance)
{
    State* pState = (State*)pInstance->GetStateData(m_uStateOffset);

    if (pState->uControlOutput != -1 && pState->pInstanceOut != NULL)
		pState->f32Value = pState->pInstanceOut->GetObject()->GetComponentValue(pState->uControlOutput, pState->pInstanceOut, 0);
	else
	    pState->f32Value = 0.f;
}

////////////////////////////////////////////////////////////////////////////////
//	SetOutput
////////////////////////////////////////////////////////////////////////////////
void Audio::ControlInput::SetOutput(Instance* pThisInstance, Instance* pThatInstance, Audio::Com uControlOutput)
{
#ifndef CFG_GOLD
	if (pThatInstance == NULL && uControlOutput != -1)
		Assert( 0 );
#endif

    State* pState = (State*)pThisInstance->GetStateData(m_uStateOffset);

    pState->pInstanceOut = pThatInstance;
    pState->uControlOutput = uControlOutput;
}

#if defined (AUDIO_ENABLE_PROFILING)
////////////////////////////////////////////////////////////////////////////////
//	GetOtherInstance
////////////////////////////////////////////////////////////////////////////////
Audio::Instance* Audio::ControlInput::GetOtherInstance(Instance* pThisInstance)
{
    State* pState = (State*)pThisInstance->GetStateData(m_uStateOffset);

	return pState->pInstanceOut;
}

////////////////////////////////////////////////////////////////////////////////
//	GetOtherComponent
////////////////////////////////////////////////////////////////////////////////
Audio::Com Audio::ControlInput::GetOtherComponent(Instance* pThisInstance)
{
    State* pState = (State*)pThisInstance->GetStateData(m_uStateOffset);

	return pState->uControlOutput;
}
#endif

#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", on) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF
