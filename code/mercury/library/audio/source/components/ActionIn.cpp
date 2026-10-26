#include "AudioPch.h"
#ifdef PLATFORM_SPU
#include "SystemHeadersSPU.h"
#else
#include "../SystemHeaders.h"
#include "ComponentHelper.h"
#endif

#ifndef PLATFORM_SPU
Debug::Bool gAudioLogActionIns("Audio.System.LogActionIns", false);
#endif

//using namespace Audio;

#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", off) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF

////////////////////////////////////////////////////////////////////////////////
//	Init
////////////////////////////////////////////////////////////////////////////////
#ifndef PLATFORM_SPU
void* Audio::ActionIn::Init(void* pData)
{ 
	Data* pInitData = PreInit<Data>(pData);
	
#ifndef PLATFORM_SPU
	//  init queue item
#else
	// TODO: SPU_TODO: something here
#endif

    return (void*)(pInitData + 1);
}

////////////////////////////////////////////////////////////////////////////////
//	InitContext
////////////////////////////////////////////////////////////////////////////////
void Audio::ActionIn::InitContext(Instance* pInstance)
{
    State* pState = (State*)pInstance->GetStateData(m_uStateOffset);

	pState->bActive = false;
}
#endif	//	#ifndef PLATFORM_SPU

////////////////////////////////////////////////////////////////////////////////
//	Execute
////////////////////////////////////////////////////////////////////////////////
bool Audio::ActionIn::DoExecute(Instance* pInstance)
{
    State* pState = (State*)pInstance->GetStateData(m_uStateOffset);

    pState->bActive = true;

	return true;
}

////////////////////////////////////////////////////////////////////////////////
//	Update
////////////////////////////////////////////////////////////////////////////////
void Audio::ActionIn::Update(Instance* pInstance)
{
	AUDIO_PERF_MICRO_EVENT("Audio.Update.ActionIn");
	
	Data* pInitData = (Data*)m_pObject->GetInitData(m_uInitOffset);
    State* pState = (State*)pInstance->GetNewStateData(m_uStateOffset);

	//	if active
	if (pState->bActive)
	{
		#ifndef PLATFORM_SPU
		if (gAudioLogActionIns)
			AUDIO_SYS_DEBUG("Trigger ActionIn: '%s'->'%s' [hInst 0x%x]", GetObject()->Name(), Name(), pInstance->GetHandle().Value());
		#endif	

		DoEvent(pInstance, pInitData->evTrigger);
		pState->bActive = false;
	}
}

#ifndef PLATFORM_SPU
////////////////////////////////////////////////////////////////////////////////
//	TriggerFromScript
////////////////////////////////////////////////////////////////////////////////
bool Audio::ActionIn::TriggerFromScript(Instance* pInstance)
{
	Data* pInitData = (Data*)m_pObject->GetInitData(m_uInitOffset);

	if (gAudioLogActionIns)
		AUDIO_SYS_DEBUG("Trigger ActionIn from script: '%s'->'%s'", GetObject()->Name(), Name());

	DoEvent(pInstance, pInitData->evTrigger);
	return true;
}
#endif	//	ndef PLATFORM_SPU

#ifndef PLATFORM_SPU
////////////////////////////////////////////////////////////////////////////////
//	Flags
////////////////////////////////////////////////////////////////////////////////
const U32 Audio::ActionIn::Flags()
{
	return StaticFlags();
}
#endif	//	#ifndef PLATFORM_SPU

#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", on) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF

