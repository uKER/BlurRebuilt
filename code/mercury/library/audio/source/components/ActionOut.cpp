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
void* Audio::ActionOut::Init(void* pData)
{ 
	Data* pInitData = PreInit<Data>(pData);
    return (void*)(pInitData + 1);
}

////////////////////////////////////////////////////////////////////////////////
//	InitContext
////////////////////////////////////////////////////////////////////////////////
void Audio::ActionOut::InitContext(Instance* pInstance)
{
    State* pState = (State*)pInstance->GetStateData(m_uStateOffset);

    pState->uAppData = 0;
#if defined (PLATFORM_SPU) || defined (PLATFORM_PS3)
	pState->uCount = 0;
#endif
}

////////////////////////////////////////////////////////////////////////////////
//	DoMessage
////////////////////////////////////////////////////////////////////////////////
void Audio::ActionOut::DoMessage(Instance* pInstance, U32 uMessage)
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
//	SetCallbackData
////////////////////////////////////////////////////////////////////////////////
void Audio::ActionOut::SetCallbackData(Instance* pInstance, U32 u32Data)
{
    State* pState = (State*)pInstance->GetStateData(m_uStateOffset);

    pState->uAppData = u32Data;
}
#endif	//	#ifndef PLATFORM_SPU

////////////////////////////////////////////////////////////////////////////////
//	Execute
////////////////////////////////////////////////////////////////////////////////
void Audio::ActionOut::Execute(Instance* pInstance)
{
#ifndef PLATFORM_SPU
	CallbackQueueMessage msg;
    State* pState = (State*)pInstance->GetStateData(m_uStateOffset);

	msg.bSuccess = true;
	msg.pCallback = m_pCallback;
	msg.u32AppData = pState->uAppData;

	System::Get()->GetReturnQueue()->SendItem(Audio::Core::ReturnQueue::eActionOut, &msg, sizeof(CallbackQueueMessage));
#else
    State* pState = (State*)pInstance->GetNewStateData(m_uStateOffset);

	pState->uCount++;
#endif
}

#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", on) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF
