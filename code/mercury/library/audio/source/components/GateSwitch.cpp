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
void* Audio::GateSwitch::Init(void* pData)
{
	Data* pInitData = PreInit<Data>(pData);

    Assert(pInitData->ctrlInput.sCom != -1);

    return (void*)(pInitData + 1);
}

////////////////////////////////////////////////////////////////////////////////
//	InitContext
////////////////////////////////////////////////////////////////////////////////
void Audio::GateSwitch::InitContext(Instance* pInstance)
{
    State* pState = (State*)pInstance->GetStateData(m_uStateOffset);

    pState->bState = false;
    pState->fPrevious = Audio::Maths::fMax;
}
#endif	//	#ifndef PLATFORM_SPU

////////////////////////////////////////////////////////////////////////////////
//	Update
////////////////////////////////////////////////////////////////////////////////
void Audio::GateSwitch::Update(Instance* pInstance)
{
	AUDIO_PERF_MICRO_EVENT("Audio.Update.GateSwitch");

	Data* pInitData = (Data*)m_pObject->GetInitData(m_uInitOffset);
    State* pState = (State*)pInstance->GetNewStateData(m_uStateOffset);

    //  input
    F32 fInput = GetControlValue( pInstance, pInitData->ctrlInput );

    //  threshold
    F32 fThreshold = GetOptionValue( pInstance, pInitData->optThreshold );

    //  hystresis
    F32 fHystresis = GetOptionValue( pInstance, pInitData->optHystresis );

    //  get absolute val
    if (fHystresis < 0)
        fHystresis *= -1.f;

    //  if gate is currently low
    if (pState->bState == false)
    {
        //  and input above threshold
        if (fInput > fThreshold + fHystresis)
        {
            //  go high
            pState->bState = true;

            //  do event
			DoEvent(pInstance, pInitData->evRising);
        }
    }
    //  if gate is currently high
    else
    {
        //  and input below threshold
        if (fInput < fThreshold - fHystresis)
        {
            //  go high
            pState->bState = false;

            //  do event
			DoEvent(pInstance, pInitData->evFalling);
        }
    }

    pState->fPrevious = fInput;
}

////////////////////////////////////////////////////////////////////////////////
//	Value
////////////////////////////////////////////////////////////////////////////////
F32 Audio::GateSwitch::Value(Instance* pInstance, U32 uOutput)
{
	Data* pInitData = (Data*)m_pObject->GetInitData( m_uInitOffset );
    State* pState = (State*)pInstance->GetStateData( m_uStateOffset );

    switch(uOutput)
    {
    case ValueCtrl:
        {
            //  only recalc if 1st request this frame
            if ( pState->uLastFrame == System::Get( )->GetFrameNum( ) )
                return pState->fPrevious;

            pState->uLastFrame = System::Get( )->GetFrameNum( );

            F32 fValue;
            if ( pState->bState )
				fValue = GetOptionValue( pInstance, pInitData->optHighLevel );
            else
				fValue = GetOptionValue( pInstance, pInitData->optLowLevel );

			return fValue;
        }
        break;

    default:
        Assert( 0 );
        break;
    }

    return 0.f;
}

#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", on) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF
