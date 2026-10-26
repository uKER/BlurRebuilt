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
//	destructor
////////////////////////////////////////////////////////////////////////////////
Audio::EventInput::~EventInput()
{
}

////////////////////////////////////////////////////////////////////////////////
//	Init
////////////////////////////////////////////////////////////////////////////////
#ifndef PLATFORM_SPU
void* Audio::EventInput::Init(void* pData)
{
	Data* pInitData = PreInit<Data>(pData);
    return (void*)(pInitData + 1);
}

////////////////////////////////////////////////////////////////////////////////
//	ContextSize
////////////////////////////////////////////////////////////////////////////////
U32 Audio::EventInput::ContextSize()
{
	return sizeof(State);
}

////////////////////////////////////////////////////////////////////////////////
//	InitContext
////////////////////////////////////////////////////////////////////////////////
void Audio::EventInput::InitContext(Instance* pInstance)
{
    State* pState = (State*)pInstance->GetStateData(m_uStateOffset);

    pState->pInstanceOut = NULL;
    pState->uEventOutput = -1;
	pState->bExecute = false;
}
#endif	//	#ifndef PLATFORM_SPU

////////////////////////////////////////////////////////////////////////////////
//	Update
////////////////////////////////////////////////////////////////////////////////
void Audio::EventInput::Update(Instance* pInstance)
{
	AUDIO_PERF_MICRO_EVENT("Audio.Update.EventInput");

	U8* pObjContext = pInstance->GetNewStateData();
	Data* pInitData = (Data*)m_pObject->GetInitData(m_uInitOffset);
    State* pState = (State*)pInstance->GetNewStateData(m_uStateOffset);

	if (pState->bExecute)
	{
		pState->bExecute = false;
		DoEvent(pInstance, pInitData->evOutput);
	}
}

////////////////////////////////////////////////////////////////////////////////
//	Execute
////////////////////////////////////////////////////////////////////////////////
void Audio::EventInput::Execute(Instance* pInstance)
{
    State* pState = (State*)pInstance->GetStateData(m_uStateOffset);

	pState->bExecute = true;
}

////////////////////////////////////////////////////////////////////////////////
//	SetOutput
////////////////////////////////////////////////////////////////////////////////
void Audio::EventInput::SetOutput(Instance* pThisInstance, Instance* pThatInstance, Audio::Com uEventOutput)
{
    State* pState = (State*)pThisInstance->GetStateData(m_uStateOffset);

    pState->pInstanceOut = pThatInstance;
    pState->uEventOutput = uEventOutput;
}

#if defined (AUDIO_ENABLE_PROFILING)
////////////////////////////////////////////////////////////////////////////////
//	GetOtherInstance
////////////////////////////////////////////////////////////////////////////////
Audio::Instance* Audio::EventInput::GetOtherInstance(Instance* pThisInstance)
{
    State* pState = (State*)pThisInstance->GetStateData(m_uStateOffset);

	return pState->pInstanceOut;
}

////////////////////////////////////////////////////////////////////////////////
//	GetOtherComponent
////////////////////////////////////////////////////////////////////////////////
Audio::Com Audio::EventInput::GetOtherComponent(Instance* pThisInstance)
{
    State* pState = (State*)pThisInstance->GetStateData(m_uStateOffset);

	return pState->uEventOutput;
}
#endif

#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", on) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF
