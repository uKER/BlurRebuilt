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
void* Audio::DebugPrint::Init(void* pData)
{
	Data* pInitData = PreInit<Data>(pData);
    return (void*)(pInitData + 1);
}

////////////////////////////////////////////////////////////////////////////////
//	InitContext
////////////////////////////////////////////////////////////////////////////////
void Audio::DebugPrint::InitContext(Instance* pInstance)
{
#ifndef CFG_GOLD
    State* pState = (State*)pInstance->GetStateData( m_uStateOffset );
    pState->bFired = false;
#endif
}
#endif	//	#ifndef PLATFORM_SPU

////////////////////////////////////////////////////////////////////////////////
//	DoMessage
////////////////////////////////////////////////////////////////////////////////
void Audio::DebugPrint::DoMessage(Instance* pInstance, U32 uMessage)
{
	switch (uMessage)
    {
    case PrintMsg:
	{
#ifndef CFG_GOLD
	    State* pState = (State*)pInstance->GetNewStateData( m_uStateOffset );
		pState->bFired = true;
#endif
	}
    break;

    default:
        Assert(0);
        break;
    }
}

////////////////////////////////////////////////////////////////////////////////
//	ProcessConnection
////////////////////////////////////////////////////////////////////////////////
#ifndef PLATFORM_SPU
#ifndef CFG_GOLD
void Audio::DebugPrint::ProcessConnection(Instance* pInstance)
{
    State* pState = (State*)pInstance->GetStateData( m_uStateOffset );

	if (pState->bFired)
	{
		pState->bFired = false;
		Print(pInstance);
	}
}
#endif
#endif	//	#ifndef PLATFORM_SPU

////////////////////////////////////////////////////////////////////////////////
//	Print
////////////////////////////////////////////////////////////////////////////////
void Audio::DebugPrint::Print(Instance* pInstance)
{
#ifndef PLATFORM_SPU
	Data* pInitData = (Data*)m_pObject->GetInitData(m_uInitOffset);

	float fValue = m_pObject->GetComponentValue(pInitData->ctrlInput.sCom, pInstance, pInitData->ctrlInput.uOut);

	AUDIO_SYS_WARN("[Frame %d] %s = %f %s", System::Get()->GetFrameNum(), Name(), fValue, pInitData->strName==-1?"":m_pObject->GetString(pInitData->strName));
#endif
}

#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", on) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF
