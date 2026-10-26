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
//	ParameterIn
////////////////////////////////////////////////////////////////////////////////
Audio::ParameterIn::~ParameterIn()
{
}

////////////////////////////////////////////////////////////////////////////////
//	Init
////////////////////////////////////////////////////////////////////////////////
#ifndef PLATFORM_SPU
void* Audio::ParameterIn::Init(void* pData)
{
	Data* pInitData = PreInit<Data>(pData);

#ifndef PLATFORM_SPU
    //  init queue item
#endif

    return (void*)(pInitData + 1);
}

////////////////////////////////////////////////////////////////////////////////
//	InitContext
////////////////////////////////////////////////////////////////////////////////
void Audio::ParameterIn::InitContext(Instance* pInstance)
{
	Data* pInitData = (Data*)m_pObject->GetInitData(m_uInitOffset);
    State* pState = (State*)pInstance->GetStateData(m_uStateOffset);

    pState->bDisable = false;
    pState->fValue = pInitData->fDefault;
#if defined(ENABLE_DEBUG_EXTENSIONS)
	pState->bDebug = false;
#endif
}
#endif	//	#ifndef PLATFORM_SPU

////////////////////////////////////////////////////////////////////////////////
//	Value
////////////////////////////////////////////////////////////////////////////////
F32 Audio::ParameterIn::Value(Instance* pInstance, U32 uOutput)
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

////////////////////////////////////////////////////////////////////////////////
//	DoMessage
////////////////////////////////////////////////////////////////////////////////
void Audio::ParameterIn::DoMessage( Instance* pInstance, U32 uMessage )
{
	switch( uMessage )
    {
    case ResetMsg:
        Reset( pInstance );
        break;

    default:
        Assert( 0 );
        break;
    }
}

////////////////////////////////////////////////////////////////////////////////
//	Reset
////////////////////////////////////////////////////////////////////////////////
void Audio::ParameterIn::Reset( Instance* pInstance )
{
	Data* pInitData = (Data*)m_pObject->GetInitData( m_uInitOffset );
    State* pState = (State*)pInstance->GetNewStateData( m_uStateOffset );

    pState->fValue = pInitData->fDefault;
}

////////////////////////////////////////////////////////////////////////////////
//	SetValue
////////////////////////////////////////////////////////////////////////////////
bool Audio::ParameterIn::SetValue(Instance* pInstance, F32 fValue)
{
    State* pState = (State*)pInstance->GetStateData(m_uStateOffset);

	if (!pState->bDisable)
		pState->fValue = fValue;

	return true;
}

////////////////////////////////////////////////////////////////////////////////
//	ForceSetValue
////////////////////////////////////////////////////////////////////////////////
bool Audio::ParameterIn::ForceSetValue(Instance* pInstance, F32 fValue)
{
    State* pState = (State*)pInstance->GetStateData(m_uStateOffset);

	pState->fValue = fValue;

	return true;
}

////////////////////////////////////////////////////////////////////////////////
//	SetDisableFlag
////////////////////////////////////////////////////////////////////////////////
void Audio::ParameterIn::SetDisableFlag(Instance* pInstance, bool bDisable)
{
    State* pState = (State*)pInstance->GetStateData(m_uStateOffset);

	pState->bDisable = bDisable;
}

////////////////////////////////////////////////////////////////////////////////
//	ReceiveData
////////////////////////////////////////////////////////////////////////////////
F32 Audio::ParameterIn::GetDefaultValue()
{
	Data* pInitData = (Data*)m_pObject->GetInitData(m_uInitOffset);
	return pInitData->fDefault;
}

#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", on) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF
