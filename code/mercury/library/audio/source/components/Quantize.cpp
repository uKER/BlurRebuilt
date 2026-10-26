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
void* Audio::Quantize::Init(void* pData)
{
	Data* pInitData = PreInit<Data>(pData);
    return (void*)(pInitData + 1);
}

////////////////////////////////////////////////////////////////////////////////
//	InitContext
////////////////////////////////////////////////////////////////////////////////
void Audio::Quantize::InitContext(Instance* pInstance)
{
    State* pState = (State*)pInstance->GetStateData(m_uStateOffset);

    pState->uLastFrame = (U32)-1;
    pState->fPrevious = 0.f;
}
#endif	//	#ifndef PLATFORM_SPU

////////////////////////////////////////////////////////////////////////////////
//	Value
////////////////////////////////////////////////////////////////////////////////
F32 Audio::Quantize::Value(Instance* pInstance, U32 uOutput)
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

            //  get step
            F32 fStep = GetOptionValue( pInstance, pInitData->optStep );

            //  get offset
            F32 fOffset = GetOptionValue( pInstance, pInitData->optOffset );

			//	rounding direction is sign dependant
			F32 fRound = Maths::Select( fInput, 0.5f, -0.5f );

            //  generate output
			if ( fStep == 1.f )
				pState->fPrevious = Maths::Floor( fRound + fInput);
			else
			{
				if (fOffset == 0.f)
					pState->fPrevious = fStep * ( Maths::Floor( fRound + fInput / fStep) );
				else
					pState->fPrevious = fOffset + fStep * ( Maths::Floor( fRound + ( fInput - fOffset ) / fStep ) );
			}
            
            return pState->fPrevious;
        }
        break;

    default:
        Assert(0);
        break;
    }

    return 0.f;
}

