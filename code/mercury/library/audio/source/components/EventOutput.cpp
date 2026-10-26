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
Audio::EventOutput::~EventOutput()
{
}

////////////////////////////////////////////////////////////////////////////////
//	Init
////////////////////////////////////////////////////////////////////////////////
#ifndef PLATFORM_SPU
void* Audio::EventOutput::Init(void* pData)
{
	Data* pInitData = PreInit<Data>(pData);
    return (void*)(pInitData + 1);
}

////////////////////////////////////////////////////////////////////////////////
//	InitContext
////////////////////////////////////////////////////////////////////////////////
void Audio::EventOutput::InitContext(Instance* pInstance)
{
    State* pState = (State*)pInstance->GetStateData(m_uStateOffset);

    pState->pToInstance = NULL;
    pState->uEventInput = -1;
	pState->bExecuteInput = false;
}
#endif	//	#ifndef PLATFORM_SPU

////////////////////////////////////////////////////////////////////////////////
//	DoMessage
////////////////////////////////////////////////////////////////////////////////
void Audio::EventOutput::DoMessage(Instance* pInstance, U32 uMessage)
{
    switch (uMessage)
    {
    case ExecuteMsg:
        Execute(pInstance);
        break;

    default:
        Assert(0);
        break;
    }
}

////////////////////////////////////////////////////////////////////////////////
//	Execute
////////////////////////////////////////////////////////////////////////////////
void Audio::EventOutput::Execute(Instance* pInstance)
{
    State* pState = (State*)pInstance->GetNewStateData(m_uStateOffset);

	pState->bExecuteInput = true;
}

////////////////////////////////////////////////////////////////////////////////
//	ProcessConnection
////////////////////////////////////////////////////////////////////////////////
void Audio::EventOutput::ProcessConnection(Instance* pInstance)
{
    State* pState = (State*)pInstance->GetStateData(m_uStateOffset);

	if (pState->bExecuteInput)
	{
		pState->bExecuteInput = false;

		if( pState->uEventInput != -1 && pState->pToInstance )
		{
			EventInput* pEvntIn = (EventInput*)pState->pToInstance->GetObject()->GetComponent( pState->uEventInput );
			pEvntIn->Execute( pState->pToInstance );
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
//	SetOutput
////////////////////////////////////////////////////////////////////////////////
void Audio::EventOutput::SetInput(Instance* pThisInstance, Instance* pThatInstance, Audio::Com uEventInput)
{
    State* pState = (State*)pThisInstance->GetStateData(m_uStateOffset);

    pState->pToInstance = pThatInstance;
    pState->uEventInput = uEventInput;
}

#if defined (AUDIO_ENABLE_PROFILING)
////////////////////////////////////////////////////////////////////////////////
//	GetOtherInstance
////////////////////////////////////////////////////////////////////////////////
Audio::Instance* Audio::EventOutput::GetOtherInstance(Instance* pThisInstance)
{
    State* pState = (State*)pThisInstance->GetStateData(m_uStateOffset);

	return pState->pToInstance;
}

////////////////////////////////////////////////////////////////////////////////
//	GetOtherComponent
////////////////////////////////////////////////////////////////////////////////
Audio::Com Audio::EventOutput::GetOtherComponent(Instance* pThisInstance)
{
    State* pState = (State*)pThisInstance->GetStateData(m_uStateOffset);

	return pState->uEventInput;
}
#endif

#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", on) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF
