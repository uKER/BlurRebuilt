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
void* Audio::PiecewiseLinearFunction::Init(void* pData)
{
	Data* pInitData = PreInit<Data>(pData);

	Assert(pInitData->ctrlInput.sCom != -1);
    Assert(pInitData->uNumPoints >= 2);

	m_uPointOffset = m_uInitOffset + sizeof(Data);

	U32 uNext = m_uPointOffset + pInitData->uNumPoints * sizeof(Point);

    return (void*)((U8*)m_pObject->GetInitData() + uNext);
}

////////////////////////////////////////////////////////////////////////////////
//	InitContext
////////////////////////////////////////////////////////////////////////////////
void Audio::PiecewiseLinearFunction::InitContext(Instance* pInstance)
{
	Data* pInitData = (Data*)m_pObject->GetInitData(m_uInitOffset);
	Point* aPoints = (Point*)m_pObject->GetInitData(m_uPointOffset);
    State* pState = (State*)pInstance->GetStateData(m_uStateOffset);

    pState->bIsDynamic = false;

    //  check for dynamic points
    for (U32 uCount = 0; uCount < pInitData->uNumPoints; uCount++)
    {
		if (aPoints[uCount].optY.ctrl.sCom != -1)
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
F32 Audio::PiecewiseLinearFunction::Value(Instance* pInstance, U32 uOutput)
{
	Data* pInitData = (Data*)m_pObject->GetInitData( m_uInitOffset );
	Point* aPoints = (Point*)m_pObject->GetInitData( m_uPointOffset );
    State* pState = (State*)pInstance->GetStateData( m_uStateOffset );

    switch(uOutput)
    {
    case ValueCtrl:
        {
            //  only recalc if 1st request this frame
            if ( pState->uLastFrame == System::Get()->GetFrameNum( ) )
			    return pState->fPrevOut;

            pState->uLastFrame = System::Get( )->GetFrameNum( );

            //  input
			F32 fInput = GetControlValue( pInstance, pInitData->ctrlInput );

            //  if no change in input and no dynamic points/curve, return previous output
			if ( ( pState->fPrevIn == fInput ) && !pState->bIsDynamic && ( pInitData->optCurve.ctrl.sCom == -1 ) )
                return pState->fPrevOut;

            //  get curve
            F32 fCurve = GetOptionValue( pInstance, pInitData->optCurve );

            pState->fPrevIn = fInput;

		    if ( fInput <= aPoints[ 0 ].fXVal )
            {
                //  before 1st point
				pState->fPrevOut = GetOptionValue( pInstance, aPoints[ 0 ].optY );
			    return pState->fPrevOut;
            }
            else if ( fInput >= aPoints[pInitData->uNumPoints - 1].fXVal )
		    {
                //  after last point
				pState->fPrevOut = GetOptionValue( pInstance, aPoints[ pInitData->uNumPoints - 1 ].optY );
			    return pState->fPrevOut;
		    }
		    else
		    {
                //  find nearest points
			    U32 i = 1;
            	while ( aPoints[ i ].fXVal < fInput )
				    ++i;

                //  get start blend ratio
                F32 fStart = GetOptionValue( pInstance, aPoints[ i - 1 ].optY );

                //  if no interpolation, just use start
                if ( fInput == aPoints[ i - 1 ].fXVal )
                    pState->fPrevOut = fStart;
                else
                {
                    //	interpolation blend ratios
			        F32 fEndRatio = ( fInput - aPoints[ i - 1 ].fXVal ) / ( aPoints[ i ].fXVal - aPoints[ i - 1 ].fXVal );
                    F32 fStartRatio = 1.f - fEndRatio;

                    if ( fCurve != 1.f )
                    {
                        //  not linear
                        fStartRatio = powf( fStartRatio, fCurve );
                        fEndRatio = powf( fEndRatio, fCurve );
                    }

                    //  get end blend value
                    F32 fEnd = GetOptionValue( pInstance, aPoints[ i ].optY );

                    //  get output
                    pState->fPrevOut = fStart * fStartRatio + fEnd * fEndRatio;
                }

			    return pState->fPrevOut;
            }
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

