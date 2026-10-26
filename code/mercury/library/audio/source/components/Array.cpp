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
void* Audio::Array::Init(void* pData)
{
	Data* pInitData = PreInit<Data>(pData);

    Assert(pInitData->ctrlInput.sCom != -1);
    Assert(pInitData->uNumElements >= 2);

	m_uElementOffset = m_uInitOffset + sizeof(Data);

	U32 uNext = m_uElementOffset + pInitData->uNumElements * sizeof(Element);

	return (void*)((U8*)m_pObject->GetInitData() + uNext);
}

////////////////////////////////////////////////////////////////////////////////
//	InitContext
////////////////////////////////////////////////////////////////////////////////
void Audio::Array::InitContext(Instance* pInstance)
{
	Data* pInitData = (Data*)m_pObject->GetInitData(m_uInitOffset);
    State* pState = (State*)pInstance->GetStateData(m_uStateOffset);

    pState->bIsDynamic = false;

	Element* aElements = (Element*)m_pObject->GetInitData(m_uElementOffset);

    //  check for dynamic points
    for (U32 uCount = 0; uCount < pInitData->uNumElements; uCount++)
    {
        if (aElements[uCount].optElement.ctrl.sCom != -1)
        {
            pState->bIsDynamic = true;
            break;
        }
    }

    pState->uLastFrame = (S32)-1;
    pState->fPrevIn = Maths::fMax;
    pState->fPrevOut = Maths::fMax;
}
#endif	//	#ifndef PLATFORM_SPU

////////////////////////////////////////////////////////////////////////////////
//	Value
////////////////////////////////////////////////////////////////////////////////
F32 Audio::Array::Value(Instance* pInstance, U32 uOutput)
{
	Data* pInitData = (Data*)m_pObject->GetInitData(m_uInitOffset);
	Element* aElements = (Element*)m_pObject->GetInitData(m_uElementOffset);
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

            //  get offset
			F32 fOffset = GetOptionValue( pInstance, pInitData->optOffset );

            //  if no change in input or offest, return previous output
            if ( pState->fPrevIn == fInput && pState->fOffset == fOffset && !pState->bIsDynamic )
                return pState->fPrevOut;

            pState->fOffset = fOffset;
            pState->fPrevIn = fInput;

            F32 fIndex = fInput - fOffset;

            if (fIndex < 0.5f)
            {
                //  before 1st element
				pState->fPrevOut = GetOptionValue( pInstance, aElements[ 0 ].optElement );
            }
            else if (fIndex >= (F32)pInitData->uNumElements - 0.5f)
            {
                //  after last element
				pState->fPrevOut = GetOptionValue( pInstance, aElements[ pInitData->uNumElements - 1 ].optElement );
            }
            else
            {
				//	get index
				S32 sIndex = (S32)(fIndex + 0.5f);
				pState->fPrevOut = GetOptionValue( pInstance, aElements[ sIndex ].optElement );
            }

	        return pState->fPrevOut;
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

