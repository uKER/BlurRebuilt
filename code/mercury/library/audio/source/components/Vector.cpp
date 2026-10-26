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
void* Audio::Vector::Init(void* pData)
{
	Data* pInitData = PreInit<Data>(pData);
    return (void*)(pInitData + 1);
}

////////////////////////////////////////////////////////////////////////////////
//	InitContext
////////////////////////////////////////////////////////////////////////////////
void Audio::Vector::InitContext(Instance* pInstance)
{
	Data* pInitData = (Data*)m_pObject->GetInitData(m_uInitOffset);
	U8* pStateData = pInstance->GetStateData();
    State* pState = (State*)&pStateData[m_uStateOffset];

    pState->fX = pInitData->optX.fVal;
    pState->fY = pInitData->optY.fVal;
    pState->fZ = pInitData->optZ.fVal;
    pState->fMagnitude = Maths::Sqrt(pState->fX * pState->fX + pState->fY * pState->fY + pState->fZ * pState->fZ);
}
#endif	//	#ifndef PLATFORM_SPU

////////////////////////////////////////////////////////////////////////////////
//	Value
////////////////////////////////////////////////////////////////////////////////
F32 Audio::Vector::Value(Instance* pInstance, U32 uOutput)
{
	U8* pStateData = pInstance->GetStateData();
    State* pState = (State*)&pStateData[m_uStateOffset];

    switch(uOutput)
    {
    case MagnitudeCtrl:
        return pState->fMagnitude;
        break;

    case XCtrl:
        return pState->fX;
        break;

    case YCtrl:
        return pState->fY;
        break;

    case ZCtrl:
        return pState->fZ;
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
void Audio::Vector::Coords(Instance* pInstance, Audio::Maths::Vector* pVector)
{
    Assert(pVector != NULL);

	U8* pStateData = pInstance->GetStateData();
    State* pState = (State*)&pStateData[m_uStateOffset];

    if (pVector)
    {
        *pVector = Maths::MakeVector3( pState->fX, pState->fY, pState->fZ );
    }
}

////////////////////////////////////////////////////////////////////////////////
//	Update
////////////////////////////////////////////////////////////////////////////////
void Audio::Vector::Update(Instance* pInstance)
{
    AUDIO_PERF_MICRO_EVENT("Audio.Update.Vector");

    using namespace Audio::Maths;
    
	Data* pInitData = (Data*)m_pObject->GetInitData(m_uInitOffset);
	U8* pStateData = pInstance->GetNewStateData();
    State* pState = (State*)&pStateData[m_uStateOffset];

	if (pInitData->sInputVec != -1)
    {
        //  get vector
        Maths::Vector vPos;//( UNDEFINED );
        m_pObject->GetComponentCoords(pInitData->sInputVec, pInstance, &vPos);

		pState->fX = _float( X( vPos ) );
		pState->fY = _float( Y( vPos ) );
		pState->fZ = _float( Z( vPos ) );
    }

	pState->fX = GetOptionValue( pInstance, pInitData->optX );
	pState->fY = GetOptionValue( pInstance, pInitData->optY );
	pState->fZ = GetOptionValue( pInstance, pInitData->optZ );

    pState->fMagnitude = Maths::Sqrt(pState->fX * pState->fX + pState->fY * pState->fY + pState->fZ * pState->fZ);
}

#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", on) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF

