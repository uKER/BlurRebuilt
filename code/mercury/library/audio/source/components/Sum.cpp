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
void* Audio::Sum::Init(void* pData)
{
	Data* pInitData = PreInit<Data>(pData);

    Assert(pInitData->uNumInputs >= 2);

	m_uInputOffset = m_uInitOffset + sizeof(Data);

	U32 uNext = m_uInputOffset + pInitData->uNumInputs * sizeof(Input);

	return (void*)((U8*)m_pObject->GetInitData() + uNext);
}

////////////////////////////////////////////////////////////////////////////////
//	InitContext
////////////////////////////////////////////////////////////////////////////////
void Audio::Sum::InitContext(Instance* pInstance)
{
	U8* pStateData = pInstance->GetStateData();
    State* pState = (State*)&pStateData[m_uStateOffset];

    pState->fValue = -1.f;
    pState->uLastFrame = (U32)-1;
}
#endif	//	#ifndef PLATFORM_SPU

////////////////////////////////////////////////////////////////////////////////
//	Value
////////////////////////////////////////////////////////////////////////////////
F32 Audio::Sum::Value(Instance* pInstance, U32 uOutput)
{
	Data* pInitData = (Data*)m_pObject->GetInitData(m_uInitOffset);
	Input* aInputs = (Input*)m_pObject->GetInitData(m_uInputOffset);
	U8* pStateData = pInstance->GetStateData();
    State* pState = (State*)&pStateData[m_uStateOffset];

    switch(uOutput)
    {
    case ValueCtrl:
        {
            //  only recalc if 1st request this frame
            if (pState->uLastFrame == System::Get()->GetFrameNum())
                return pState->fValue;

            pState->uLastFrame = System::Get()->GetFrameNum();

            //  1st
            if (aInputs[0].optInput.ctrl.sCom != -1)
                pState->fValue = m_pObject->GetComponentValue(aInputs[0].optInput.ctrl.sCom, pInstance, aInputs[0].optInput.ctrl.uOut);
            else
                pState->fValue = aInputs[0].optInput.fVal;

            //  2nd
            if (aInputs[1].optInput.ctrl.sCom != -1)
                pState->fValue += m_pObject->GetComponentValue(aInputs[1].optInput.ctrl.sCom, pInstance, aInputs[1].optInput.ctrl.uOut);
            else
                pState->fValue += aInputs[1].optInput.fVal;

            //  remainder
            for (U32 uInput = 2; uInput < pInitData->uNumInputs; uInput++)
            {
                if (aInputs[uInput].optInput.ctrl.sCom != -1)
                    pState->fValue += m_pObject->GetComponentValue(aInputs[uInput].optInput.ctrl.sCom, pInstance, aInputs[uInput].optInput.ctrl.uOut);
                else
                    pState->fValue += aInputs[uInput].optInput.fVal;
            }

            return pState->fValue;
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
