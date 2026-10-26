#include "AudioPch.h"
#ifdef PLATFORM_SPU
#include "SystemHeadersSPU.h"
#else
#include "../SystemHeaders.h"
#include "ComponentHelper.h"
#endif

#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", off) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF

////////////////////////////////////////////////////////////////////////////////
//	Init
////////////////////////////////////////////////////////////////////////////////
#ifndef PLATFORM_SPU
void* Audio::SurroundFlanger::Init( void* pData )
{
	Data* pInitData = PreInit<Data>(pData);
    return (void*)(pInitData + 1);
}

////////////////////////////////////////////////////////////////////////////////
//	CoreMemory
////////////////////////////////////////////////////////////////////////////////
U32 Audio::SurroundFlanger::CoreMemory(U32 uOffset)
{
	m_uDSP = (ixCore)uOffset;

	uOffset += AUDIO_CORE_ALIGN_16( sizeof( Core::DSP::Flanger ) );

	return uOffset;
}

////////////////////////////////////////////////////////////////////////////////
//	InitContext
////////////////////////////////////////////////////////////////////////////////
void Audio::SurroundFlanger::InitContext( Instance* pInstance )
{
    State* pState = (State*)pInstance->GetStateData( m_uStateOffset );
	Data* pInitData = (Data*)m_pObject->GetInitData( m_uInitOffset );

	U32 uMaxLength = (U32)Maths::Max( 0.001f * pInitData->fMaxLength * (F32)Core::uSampleRate * Core::fRcpFrameLength, 0.f );

	new( pInstance->GetCoreData( m_uDSP ) ) Core::DSP::Flanger( Name(), 5, 1 + uMaxLength );

	pState->fDelay = Maths::fInvalid;
    pState->fDepth = Maths::fInvalid;
    pState->fRate = Maths::fInvalid;
    pState->fFeedback = Maths::fInvalid;
    pState->fWet = Maths::fInvalid;
    pState->fDry = Maths::fInvalid;
}

////////////////////////////////////////////////////////////////////////////////
//	ReleaseContext
////////////////////////////////////////////////////////////////////////////////
void Audio::SurroundFlanger::ReleaseContext( Instance* pInstance )
{
	Core::DSP::Flanger* pDSP = (Core::DSP::Flanger*)pInstance->GetCoreData( m_uDSP );

	delete pDSP;
}
#endif	//	#ifndef PLATFORM_SPU

#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", on) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF
