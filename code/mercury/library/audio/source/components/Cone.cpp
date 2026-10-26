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
//	(constructor)
////////////////////////////////////////////////////////////////////////////////
Audio::Cone::Cone(Object* pObject) : Component(pObject)
{
}

////////////////////////////////////////////////////////////////////////////////
//	Init
////////////////////////////////////////////////////////////////////////////////
#ifndef PLATFORM_SPU
void* Audio::Cone::Init(void* pData)
{ 
	Data* pInitData = PreInit<Data>(pData);

    Assert(pInitData->ctrlIncidentAngle.sCom != -1);

    return (void*)(pInitData + 1);
}

////////////////////////////////////////////////////////////////////////////////
//	InitContext
////////////////////////////////////////////////////////////////////////////////
void Audio::Cone::InitContext(Instance* pInstance)
{
	U8* pStateData = pInstance->GetStateData();
    State* pState = (State*)&pStateData[m_uStateOffset];

    pState->fConeLevel = 1.f;
    pState->uLastFrame = (U32)-1;
    pState->fIncidentAngle = -1.f;
    pState->fInnerAngle = -1.f;
    pState->fInnerLevel = -1.f;
    pState->fOuterAngle = -1.f;
    pState->fOuterLevel = -1.f;
    pState->fCurve = -1.f;
}
#endif	//	#ifndef PLATFORM_SPU

////////////////////////////////////////////////////////////////////////////////
//	Value
////////////////////////////////////////////////////////////////////////////////
F32 Audio::Cone::Value(Instance* pInstance, U32 uOutput)
{
	Data* pInitData = (Data*)m_pObject->GetInitData(m_uInitOffset);
    State* pState = (State*)pInstance->GetStateData(m_uStateOffset);

    switch(uOutput)
    {
    case ValueCtrl:
        {
            //  only recalc if 1st request this frame
            if (pState->uLastFrame == System::Get()->GetFrameNum())
			    return pState->fConeLevel;

            pState->uLastFrame = System::Get()->GetFrameNum();

            //  get incident angle
            F32 fIncidentAngle = GetControlValue( pInstance, pInitData->ctrlIncidentAngle );

            //  deal with incident angle wrap around
			fIncidentAngle = Maths::MakeMinusOneToOne( fIncidentAngle );

            //  correct sign
            if (fIncidentAngle < 0.f)
                fIncidentAngle *= -1.f;

            //  get inner angle
			F32 fInnerAngle = GetOptionValue( pInstance, pInitData->optInnerAngle );

            //  correct sign
            if (fInnerAngle < 0.f)
                fInnerAngle *= -1.f;

            //  get inner level
			F32 fInnerLevel = GetOptionValue( pInstance, pInitData->optInnerLevel );

            //  get outer angle
			F32 fOuterAngle = GetOptionValue( pInstance, pInitData->optOuterAngle );

            //  correct sign
            if (fOuterAngle < 0.f)
                fOuterAngle *= -1.f;

            //  get outer level
			F32 fOuterLevel = GetOptionValue( pInstance, pInitData->optOuterLevel );

            //  get curve
			F32 fCurve = GetOptionValue( pInstance, pInitData->optCurve );

            if (fCurve == 0.f)
                fCurve = Maths::fMin;

            if ((fIncidentAngle != pState->fIncidentAngle) ||
                (fInnerLevel != pState->fInnerLevel) ||
                (fOuterLevel != pState->fOuterLevel) ||
                (fInnerAngle != pState->fInnerAngle) ||
                (fOuterAngle != pState->fOuterAngle) ||
                (fCurve != pState->fCurve))
            {
                pState->fIncidentAngle = fIncidentAngle;
                pState->fInnerLevel = fInnerLevel;
                pState->fOuterLevel = fOuterLevel;
                pState->fInnerAngle = fInnerAngle;
                pState->fIncidentAngle = fIncidentAngle;
                pState->fCurve = fCurve;

                if (fIncidentAngle <= fInnerAngle)
                    pState->fConeLevel = fInnerLevel;
                else if (fIncidentAngle >= fOuterAngle)
                    pState->fConeLevel = fOuterLevel;
                else
                {
                    //  get blend value
                    F32 fBlend;
                    if (fCurve == 1.f)
                        fBlend = (fIncidentAngle - fOuterAngle) / (fInnerAngle - fOuterAngle);
                    else
                        fBlend = powf((fIncidentAngle - fOuterAngle) / (fInnerAngle - fOuterAngle), fCurve);

                    //  calculate gain
                    pState->fConeLevel = fOuterLevel + fBlend * (fInnerLevel - fOuterLevel);
                }
            }

            return pState->fConeLevel;
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


