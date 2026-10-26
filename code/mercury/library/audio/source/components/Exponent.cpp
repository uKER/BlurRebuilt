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
void* Audio::Exponent::Init(void* pData)
{
	Data* pInitData = PreInit<Data>(pData);
    return (void*)(pInitData + 1);
}

////////////////////////////////////////////////////////////////////////////////
//	InitContext
////////////////////////////////////////////////////////////////////////////////
void Audio::Exponent::InitContext(Instance* pInstance)
{
    State* pState = (State*)pInstance->GetStateData(m_uStateOffset);

    pState->uLastFrame = (S32)-1;
    pState->fPrevIn = Maths::fMax;
    pState->fPrevOut = Maths::fMax;
}
#endif	//	#ifndef PLATFORM_SPU

////////////////////////////////////////////////////////////////////////////////
//	Value
////////////////////////////////////////////////////////////////////////////////
F32 Audio::Exponent::Value(Instance* pInstance, U32 uOutput)
{
	Data* pInitData = (Data*)m_pObject->GetInitData(m_uInitOffset);
    State* pState = (State*)pInstance->GetStateData(m_uStateOffset);

    switch(uOutput)
    {
    case ValueCtrl:
        {
            //  only recalc if 1st request this frame
            if (pState->uLastFrame == System::Get()->GetFrameNum())
			    return pState->fPrevOut;

            pState->uLastFrame = System::Get()->GetFrameNum();

            //  input
			F32 fInput = GetControlValue( pInstance, pInitData->ctrlInput );

            //  if no change in input, return previous output
            if (pState->fPrevIn == fInput)
                return pState->fPrevOut;

            //  get power
			F32 fPower = GetOptionValue( pInstance, pInitData->optPower );

            pState->fPrevIn = fInput;

            //  get output
            pState->fPrevOut = powf(fInput, fPower);
			    
			return pState->fPrevOut;
        }
        break;

    default:
        Assert(0);
        break;
    }

    return 0.f;
}

#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", on) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF
