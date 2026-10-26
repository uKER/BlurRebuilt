#include "AudioPch.h"
#ifdef PLATFORM_SPU
#include "SystemHeadersSPU.h"
#else
#include "../SystemHeaders.h"
#include "ComponentHelper.h"
#endif

//using namespace Audio;

////////////////////////////////////////////////////////////////////////////////
//	Init
////////////////////////////////////////////////////////////////////////////////
#ifndef PLATFORM_SPU
void* Audio::Fraction::Init(void* pData)
{
	Data* pInitData = PreInit<Data>(pData);
    return (void*)(pInitData + 1);
}

////////////////////////////////////////////////////////////////////////////////
//	InitContext
////////////////////////////////////////////////////////////////////////////////
void Audio::Fraction::InitContext(Instance* pInstance)
{
    State* pState = (State*)pInstance->GetStateData(m_uStateOffset);

    pState->uLastFrame = (U32)-1;
    pState->fPrevious = 0.f;
}
#endif	//	#ifndef PLATFORM_SPU

////////////////////////////////////////////////////////////////////////////////
//	Value
////////////////////////////////////////////////////////////////////////////////
F32 Audio::Fraction::Value(Instance* pInstance, U32 uOutput)
{
	Data* pInitData = (Data*)m_pObject->GetInitData(m_uInitOffset);
    State* pState = (State*)pInstance->GetStateData(m_uStateOffset);

    switch(uOutput)
    {
    case ValueCtrl:
        {
            //  only recalc if 1st request this frame
            if (pState->uLastFrame == System::Get()->GetFrameNum())
                return pState->fPrevious;

            pState->uLastFrame = System::Get()->GetFrameNum();

            //  get input
			F32 fInput = GetControlValue( pInstance, pInitData->ctrlInput );

            //  generate output
			pState->fPrevious = Maths::Select( fInput, fInput - Maths::Floor( fInput ), fInput - Maths::Floor( fInput ) + 1.f );
            
            return pState->fPrevious;
        }
        break;

    default:
        Assert(0);
        break;
    }

    return 0.f;
}


