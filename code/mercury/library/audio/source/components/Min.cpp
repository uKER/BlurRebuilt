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
void* Audio::Min::Init(void* pData)
{
	Data* pInitData = PreInit<Data>(pData);

    Assert(pInitData->uNumInputs >= 2);

	m_uInputsOffset = m_uInitOffset + sizeof(Data);

	U32 uNext = m_uInputsOffset + pInitData->uNumInputs * sizeof(Input);

	return (void*)((U8*)m_pObject->GetInitData() + uNext);
}

////////////////////////////////////////////////////////////////////////////////
//	InitContext
////////////////////////////////////////////////////////////////////////////////
void Audio::Min::InitContext(Instance* pInstance)
{
    State* pState = (State*)pInstance->GetStateData(m_uStateOffset);

    pState->fValue = -1.f;
    pState->uLastFrame = (U32)-1;
}
#endif

////////////////////////////////////////////////////////////////////////////////
//	Value
////////////////////////////////////////////////////////////////////////////////
F32 Audio::Min::Value(Instance* pInstance, U32 uOutput)
{
	Data* pInitData = (Data*)m_pObject->GetInitData( m_uInitOffset );
	Input* aInputs = (Input*)m_pObject->GetInitData( m_uInputsOffset );
    State* pState = (State*)pInstance->GetStateData( m_uStateOffset );

    switch(uOutput)
    {
    case ValueCtrl:
        {
            //  only recalc if 1st request this frame
            if (pState->uLastFrame == System::Get()->GetFrameNum())
                return pState->fValue;

            pState->uLastFrame = System::Get()->GetFrameNum();

            //  1st
			pState->fValue = GetOptionValue( pInstance, aInputs[ 0 ].optInput );

            //  2nd
			pState->fValue = Maths::Min( pState->fValue, GetOptionValue( pInstance, aInputs[ 1 ].optInput ) );

            //  remainder
            for (U32 uInput = 2; uInput < pInitData->uNumInputs; uInput++)
				pState->fValue = Maths::Min( pState->fValue, GetOptionValue( pInstance, aInputs[ uInput ].optInput ) );

            return pState->fValue;
        }
        break;

    default:
        Assert( 0 );
        break;
    }

    return 0.f;
}

