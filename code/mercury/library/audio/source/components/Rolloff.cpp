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
void* Audio::Rolloff::Init(void* pData)
{
	Data* pInitData = PreInit<Data>(pData);
    return (void*)(pInitData + 1);
}

////////////////////////////////////////////////////////////////////////////////
//	InitContext
////////////////////////////////////////////////////////////////////////////////
void Audio::Rolloff::InitContext(Instance* pInstance)
{
    State* pState = (State*)pInstance->GetStateData(m_uStateOffset);

    pState->fRolloffLevel = 1.f;
    pState->uLastFrame = (U32)-1;
}
#endif	//	#ifndef PLATFORM_SPU

////////////////////////////////////////////////////////////////////////////////
//	Value
////////////////////////////////////////////////////////////////////////////////
F32 Audio::Rolloff::Value(Instance* pInstance, U32 uOutput)
{
	Data* pInitData = (Data*)m_pObject->GetInitData(m_uInitOffset);
    State* pState = (State*)pInstance->GetStateData(m_uStateOffset);

    switch(uOutput)
    {
    case ValueCtrl:
        {
            //  only recalc if 1st request this frame
            if (pState->uLastFrame == System::Get()->GetFrameNum())
			    return pState->fRolloffLevel;

            pState->uLastFrame = System::Get()->GetFrameNum();

            //  get distance
			F32 fDistance = GetControlValue( pInstance, pInitData->ctrlDistance );

            //  get rolloff factor
			F32 fRolloffFactor = GetOptionValue( pInstance, pInitData->optRolloffFactor );

            //  get max level
            F32 fMaxLevel = GetOptionValue( pInstance, pInitData->optMaxLevel );

            //  get reference level
            F32 fRefLevel = GetOptionValue( pInstance, pInitData->optRefLevel );

            //  get reference distance
            F32 fRefDistance = GetOptionValue( pInstance, pInitData->optRefDistance );

            //  get max distance
            F32 fMaxDistance = GetOptionValue( pInstance, pInitData->optMaxDistance );

			//	get rolloff amount
			F32 fRolloff = 1.f + fRolloffFactor * ( fDistance - fRefDistance ) / fRefDistance;

            //  calculate gain
			if( fRolloff < 0.f )
				pState->fRolloffLevel = Maths::fMax;
			else
				pState->fRolloffLevel = fRefLevel / fRolloff;

            //  fade if using max distance
            if( fMaxDistance < Maths::fMax )
            {
                //  scale to max distance
                if( ( fDistance < fMaxDistance ) && ( fDistance > fRefDistance ) )
                {
                    F32 fMaxDistanceLevel = 1.f / ( 1.f + fRolloffFactor * ( fMaxDistance - fRefDistance ) / fRefDistance );
                    pState->fRolloffLevel -= fMaxDistanceLevel * ( ( fDistance - fRefDistance ) / ( fMaxDistance - fRefDistance ) );
                }
                //  zero after max distance
                else if( fDistance > fMaxDistance )
                    pState->fRolloffLevel = 0.f;
            }

            //  clamp to max level
            if( pState->fRolloffLevel > fMaxLevel )
                pState->fRolloffLevel = fMaxLevel;

            return pState->fRolloffLevel;
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
