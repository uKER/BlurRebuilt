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
void* Audio::VectorNearestPointOnLine::Init(void* pData)
{
	Data* pInitData = PreInit<Data>(pData);
    return (void*)(pInitData + 1);
}

////////////////////////////////////////////////////////////////////////////////
//	InitContext
////////////////////////////////////////////////////////////////////////////////
void Audio::VectorNearestPointOnLine::InitContext(Instance* pInstance)
{
	U8* pStateData = pInstance->GetStateData();
    State* pState = (State*)&pStateData[m_uStateOffset];

	pState->vOutputVec = Maths::MakeVector3(); // Default constructor sets to zero!
}
#endif	//	#ifndef PLATFORM_SPU

////////////////////////////////////////////////////////////////////////////////
//	Value
////////////////////////////////////////////////////////////////////////////////
F32 Audio::VectorNearestPointOnLine::Value(Instance* pInstance, U32 uOutput)
{
	U8* pStateData = pInstance->GetStateData();
    State* pState = (State*)&pStateData[m_uStateOffset];

    switch(uOutput)
    {
    case MagnitudeCtrl:
        return _float( !pState->vOutputVec ); // Using magnitude operator
        break;

    case XCtrl:
        return _float( X( pState->vOutputVec ) );
        break;

    case YCtrl:
        return _float( Y( pState->vOutputVec ) );
        break;

    case ZCtrl:
        return _float( Z( pState->vOutputVec ) );
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
void Audio::VectorNearestPointOnLine::Coords(Instance* pInstance, Audio::Maths::Vector* pVector)
{
    Assert(pVector != NULL);

	U8* pStateData = pInstance->GetNewStateData();
    State* pState = (State*)&pStateData[m_uStateOffset];

    if (pVector)
        *(pVector) = pState->vOutputVec;
}

////////////////////////////////////////////////////////////////////////////////
//	Update
////////////////////////////////////////////////////////////////////////////////
void Audio::VectorNearestPointOnLine::Update(Instance* pInstance)
{
    AUDIO_PERF_MICRO_EVENT("Audio.Update.VectorNearestPointOnLine");

    using namespace Audio::Maths;
	
	Data* pInitData = (Data*)m_pObject->GetInitData(m_uInitOffset);
	U8* pStateData = pInstance->GetNewStateData();
    State* pState = (State*)&pStateData[m_uStateOffset];

    Maths::Vector vAPos;//( UNDEFINED );
    m_pObject->GetComponentCoords(pInitData->sInputAVec, pInstance, &vAPos);

    Maths::Vector vBPos;//( UNDEFINED );
    m_pObject->GetComponentCoords(pInitData->sInputBVec, pInstance, &vBPos);

    Maths::Vector vAlignPos;//( UNDEFINED );
    m_pObject->GetComponentCoords(pInitData->sInputAlignVec, pInstance, &vAlignPos);

	// remove a pos from equation..
	Maths::Vector vBPosShifted = (vBPos - vAPos);
	Maths::Vector vAlignPosShifted = (vAlignPos - vAPos);

	const Maths::Scalar zero( _Scalar( 0.0f ) );
	const Maths::Scalar one( _Scalar( 1.0f ) );
    
	if( _float( Magnitude2(vBPosShifted) ) > 0.f && _float( Magnitude2(vAlignPosShifted) ) > 0.f )
	{
		Scalar fPositionAlongLine = (! vAlignPosShifted * ((vAlignPosShifted | one) % (vBPosShifted | one))) / ! vBPosShifted;
	    
		// clamp so return vec doesn't extend beyond end of line
		fPositionAlongLine = Clamp( fPositionAlongLine, zero, one );
	    
		pState->vOutputVec = (vBPosShifted * fPositionAlongLine) + vAPos;
	}
	else
	{
		AUDIO_SYS_WARN("VectorNearestPointOnLine: Zero Magnitude");
		pState->vOutputVec = MakeVector3( 0.f, 0.f, 0.f );
	}
}

#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", on) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF
