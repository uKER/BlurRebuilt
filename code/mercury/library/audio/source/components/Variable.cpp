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
void* Audio::Variable::Init(void* pData)
{
	Data* pInitData = PreInit<Data>(pData);
    return (void*)(pInitData + 1);
}

////////////////////////////////////////////////////////////////////////////////
//	InitContext
////////////////////////////////////////////////////////////////////////////////
void Audio::Variable::InitContext(Instance* pInstance)
{
	Data* pInitData = (Data*)m_pObject->GetInitData(m_uInitOffset);
	U8* pStateData = pInstance->GetStateData();
    State* pState = (State*)&pStateData[m_uStateOffset];

    pState->fValue = pInitData->fDefault;
}
#endif	//	#ifndef PLATFORM_SPU

////////////////////////////////////////////////////////////////////////////////
//	Value
////////////////////////////////////////////////////////////////////////////////
F32 Audio::Variable::Value(Instance* pInstance, U32 uOutput)
{
	U8* pStateData = pInstance->GetStateData();
    State* pState = (State*)&pStateData[m_uStateOffset];

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

////////////////////////////////////////////////////////////////////////////////
//	Name
////////////////////////////////////////////////////////////////////////////////
void Audio::Variable::SetValueDirect(Instance* pInstance, F32 fValue)
{
    State* pState = (State*)pInstance->GetNewStateData( m_uStateOffset );

	pState->fValue = fValue;
}

#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", on) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF
