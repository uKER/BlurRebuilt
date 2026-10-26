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
void* Audio::Timer::Init(void* pData)
{
	Data* pInitData = PreInit<Data>(pData);
    return (void*)(pInitData + 1);
}

////////////////////////////////////////////////////////////////////////////////
//	InitContext
////////////////////////////////////////////////////////////////////////////////
void Audio::Timer::InitContext(Instance* pInstance)
{
	Data* pInitData = (Data*)m_pObject->GetInitData(m_uInitOffset);
	State* pState = (State*)pInstance->GetStateData(m_uStateOffset);

    pState->fPrevious = 0.f;
    pState->fCurrent = 0.f;

    pState->bActive = false;
    pState->bAlarmed = true;
    pState->bInitialized = false;
}
#endif	//	#ifndef PLATFORM_SPU

////////////////////////////////////////////////////////////////////////////////
//	Update
////////////////////////////////////////////////////////////////////////////////
void Audio::Timer::Update(Instance* pInstance)
{
    AUDIO_PERF_MICRO_EVENT("Audio.Update.Timer");

	Data* pInitData = (Data*)m_pObject->GetInitData(m_uInitOffset);
	State* pState = (State*)pInstance->GetNewStateData(m_uStateOffset);

	//	initalize
	if (!pState->bInitialized)
	{
		pState->fCurrent = GetOptionValue( pInstance, pInitData->optStartAt );
		pState->bInitialized = true;
	}

	//	if active
	if (pState->bActive)
	{
		//  increment
		F32 fIncrement = GetOptionValue( pInstance, pInitData->optIncrement );

		pState->fPrevious = pState->fCurrent;
		pState->fCurrent += fIncrement * Core::fFrameLength * Core::fRcpSampleRate * (F32)pInstance->m_coreInfo.uFrameInt;

		//  if alarmed
		if (pState->bAlarmed && (pInitData->evAlarm.sCom != -1))
		{
			//  alarm
			F32 fAlarm = GetOptionValue( pInstance, pInitData->optAlarm );

			//  if triggered (in either direction)
			if ((pState->fPrevious < fAlarm && pState->fCurrent >= fAlarm) || (pState->fCurrent < fAlarm && pState->fPrevious >= fAlarm))
			{
				pState->bAlarmed = false;

				//  do alarm event
				DoEvent(pInstance, pInitData->evAlarm);
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
//	Value
////////////////////////////////////////////////////////////////////////////////
F32 Audio::Timer::Value(Instance* pInstance, U32 uOutput)
{
	State* pState = (State*)pInstance->GetStateData(m_uStateOffset);

	switch(uOutput)
    {
    case ValueCtrl:
        return pState->fCurrent;
        break;
	case ActiveCtrl:
		return pState->bActive ? 1.f : 0.f;
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
void Audio::Timer::DoMessage(Instance* pInstance, U32 uMessage)
{
	switch (uMessage)
    {
    case StartMsg:
        Start(pInstance);
        break;

    case StopMsg:
        Stop(pInstance);
        break;

    case ResetMsg:
        Reset(pInstance);
        break;

    case ZeroMsg:
        ResetToZero(pInstance);
        break;

    default:
        Assert(0);
        break;
    }
}

////////////////////////////////////////////////////////////////////////////////
//	Start
////////////////////////////////////////////////////////////////////////////////
void Audio::Timer::Start(Instance* pInstance)
{
    State* pState = (State*)pInstance->GetNewStateData(m_uStateOffset);

    pState->bActive = true;
}

////////////////////////////////////////////////////////////////////////////////
//	Stop
////////////////////////////////////////////////////////////////////////////////
void Audio::Timer::Stop(Instance* pInstance)
{
    State* pState = (State*)pInstance->GetNewStateData(m_uStateOffset);

    pState->bActive = false;
}

////////////////////////////////////////////////////////////////////////////////
//	Reset
////////////////////////////////////////////////////////////////////////////////
void Audio::Timer::Reset(Instance* pInstance)
{
	Data* pInitData = (Data*)m_pObject->GetInitData(m_uInitOffset);
    State* pState = (State*)pInstance->GetNewStateData(m_uStateOffset);

	if (pInitData->optStartAt.ctrl.sCom != -1)
		pState->fCurrent = m_pObject->GetComponentValue(pInitData->optStartAt.ctrl.sCom, pInstance, pInitData->optStartAt.ctrl.uOut);
	else
	    pState->fCurrent = pInitData->optStartAt.fVal;

    pState->bAlarmed = true;   
}

////////////////////////////////////////////////////////////////////////////////
//	Zero
////////////////////////////////////////////////////////////////////////////////
void Audio::Timer::ResetToZero(Instance* pInstance)
{
    State* pState = (State*)pInstance->GetNewStateData(m_uStateOffset);

    pState->fCurrent = 0.f;

    pState->bAlarmed = true;
}

#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", on) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF
