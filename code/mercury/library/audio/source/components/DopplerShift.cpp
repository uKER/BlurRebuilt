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
void* Audio::DopplerShift::Init(void* pData)
{
	Data* pInitData = PreInit<Data>(pData);
    return (void*)(pInitData + 1);
}

////////////////////////////////////////////////////////////////////////////////
//	InitContext
////////////////////////////////////////////////////////////////////////////////
void Audio::DopplerShift::InitContext(Instance* pInstance)
{
    State* pState = (State*)pInstance->GetStateData(m_uStateOffset);

    pState->fDopplerShiftFactor = 1.f;
}
#endif	//	#ifndef PLATFORM_SPU

////////////////////////////////////////////////////////////////////////////////
//	Value
////////////////////////////////////////////////////////////////////////////////
F32 Audio::DopplerShift::Value(Instance* pInstance, U32 uOutput)
{
    State* pState = (State*)pInstance->GetStateData(m_uStateOffset);

    switch(uOutput)
    {
    case FrequencyCtrl:
        return pState->fDopplerShiftFactor;
        break;

	case ValueCtrl:
		return pState->fDopplerShiftPitchFactor;
        break;

    default:
        Assert(0);
        break;
    }

    return 0.f;
}

////////////////////////////////////////////////////////////////////////////////
//	Coords
////////////////////////////////////////////////////////////////////////////////
void Audio::DopplerShift::Coords(Instance* pInstance, Audio::Maths::Vector* pVector)
{
    Assert(pVector != NULL);

	U8* pStateData = pInstance->GetNewStateData();
    State* pState = (State*)&pStateData[m_uStateOffset];

    if (pVector)
	{
		const Maths::Vector vec( Audio::Maths::MakeVector3( pState->outputVecX,
															pState->outputVecY,
															pState->outputVecZ ) );
        *(pVector) = vec;
	}
}

////////////////////////////////////////////////////////////////////////////////
//	Update
////////////////////////////////////////////////////////////////////////////////
void Audio::DopplerShift::Update(Instance* pInstance)
{
	AUDIO_PERF_MICRO_EVENT("Audio.Update.DopplerShift");

    using namespace Audio::Maths;
	
	Data* pInitData = (Data*)m_pObject->GetInitData(m_uInitOffset);
	U8* pStateData = pInstance->GetNewStateData();
    State* pState = (State*)&pStateData[m_uStateOffset];

    Maths::Vector vSourceVel;
	if (pInitData->sInputSourceVelocityVec != -1)
		m_pObject->GetComponentCoords(pInitData->sInputSourceVelocityVec, pInstance, &vSourceVel);
	else
		vSourceVel = Audio::Maths::MakeVector3( 0.f, 0.f, 0.f );

    Maths::Vector vListenerVel;
	if (pInitData->sInputListenerVelocityVec != -1)
		m_pObject->GetComponentCoords(pInitData->sInputListenerVelocityVec, pInstance, &vListenerVel);
	else
		vListenerVel = Audio::Maths::MakeVector3( 0.f, 0.f, 0.f );

    Maths::Vector vSourcePos;
	if (pInitData->sInputSourcePositionVec != -1)
	    m_pObject->GetComponentCoords(pInitData->sInputSourcePositionVec, pInstance, &vSourcePos);
	else
		vSourcePos = Audio::Maths::MakeVector3( 0.f, 0.f, 0.f );

    Maths::Vector vListenerPos;
	if (pInitData->sInputListenerPositionVec != -1)
	    m_pObject->GetComponentCoords(pInitData->sInputListenerPositionVec, pInstance, &vListenerPos);
	else
		vListenerPos = Audio::Maths::MakeVector3( 0.f, 0.f, 0.f );

	//Speed of Sound
	F32 c = GetOptionValue( pInstance, pInitData->optSpeedOfSound );

	Maths::Vector relativeVel = vSourceVel - vListenerVel;
	F32 relativespeed = _float(!relativeVel);

	Maths::Vector vRelativeDir = vSourcePos - vListenerPos;

	const Scalar one( _Scalar( 1.0f ) );
	relativeVel |= one;
	vRelativeDir |= one;

	F32 dotProduct = _float(relativeVel % vRelativeDir);

	F32 DopplerSpeed = Maths::Max( relativespeed * dotProduct, -c );						

	//calculate doppler shift
	if( DopplerSpeed != -c )
	{
		pState->fDopplerShiftFactor = c / ( c + DopplerSpeed );
		pState->fDopplerShiftPitchFactor = Maths::HzToOct( pState->fDopplerShiftFactor );
	}
	else
	{
		pState->fDopplerShiftFactor = 0.f;
		pState->fDopplerShiftPitchFactor = -Maths::fMax;
	}
}

#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", on) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF
