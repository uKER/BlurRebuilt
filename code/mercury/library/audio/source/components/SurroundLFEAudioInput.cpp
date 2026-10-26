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

//using namespace Audio;

////////////////////////////////////////////////////////////////////////////////
//	Init
////////////////////////////////////////////////////////////////////////////////
#ifndef PLATFORM_SPU
void* Audio::SurroundLFEAudioInput::Init(void* pData)
{
	Data* pInitData = PreInit<Data>(pData);
    return (void*)(pInitData + 1);
}

////////////////////////////////////////////////////////////////////////////////
//	CoreMemory
////////////////////////////////////////////////////////////////////////////////
U32 Audio::SurroundLFEAudioInput::CoreMemory( U32 uOffset )
{
	m_uInput = uOffset;

	return uOffset + Core::Input::InputSize( 6, 0 );
}

////////////////////////////////////////////////////////////////////////////////
//	InitContext
////////////////////////////////////////////////////////////////////////////////
void Audio::SurroundLFEAudioInput::InitContext( Instance* pInstance )
{
    State* pState = (State*)pInstance->GetStateData( m_uStateOffset );

	Core::Input* pInput = new( pInstance->GetCoreData( m_uInput ) ) Core::Input( Name(), 6, 0 );

	pState->pFrameInput = (F32*)System::Get()->Alloc( 6 * Core::uFrameLength * sizeof(F32), 128 );
	pInput->SetAltBuffer( pState->pFrameInput );

	for( U32 uCh = 0; uCh < 6; uCh++ )
		Frame::Zero( &pState->pFrameInput[ uCh * Core::uFrameLength ] );

	pState->bActive = true;
	pState->bMuted = false;
}

////////////////////////////////////////////////////////////////////////////////
//	ReleaseContext
////////////////////////////////////////////////////////////////////////////////
void Audio::SurroundLFEAudioInput::ReleaseContext(Instance* pInstance)
{
	U8* pStateData = pInstance->GetStateData();
    State* pState = (State*)&pStateData[m_uStateOffset];
	U8* pCoreData = pInstance->GetCoreData();

	System::Get()->Free(pState->pFrameInput);

	Core::Input* pInput = (Core::Input*)&pCoreData[m_uInput];
	pInput->~Input();
}
#endif	//	#ifndef PLATFORM_SPU

////////////////////////////////////////////////////////////////////////////////
//	Update
////////////////////////////////////////////////////////////////////////////////
void Audio::SurroundLFEAudioInput::Update( Instance* pInstance )
{
    AUDIO_PERF_MICRO_EVENT("Audio.Update.SurroundLFEAudioInput");

	State* pState = (State*)pInstance->GetNewStateData( m_uStateOffset );
	Core::Input* pInput = (Core::Input*)pInstance->GetCoreData( m_uInput );

	pState->bActive = pInput->IsActive();

	pInput->SetMuted( pState->bMuted );
}

////////////////////////////////////////////////////////////////////////////////
//	AudioInput
////////////////////////////////////////////////////////////////////////////////
Audio::Core::Input* Audio::SurroundLFEAudioInput::AudioInput(Instance* pInstance)
{
    return (Core::Input*)pInstance->GetCoreData( m_uInput );
}

#if defined (AUDIO_ENABLE_PROFILING)
////////////////////////////////////////////////////////////////////////////////
//	GetOtherInstance
////////////////////////////////////////////////////////////////////////////////
Audio::Instance* Audio::SurroundLFEAudioInput::GetOtherInstance( Instance* pThisInstance )
{
    State* pState = (State*)pThisInstance->GetStateData( m_uStateOffset );

	return NULL;//pState->pInstanceOut;
}

////////////////////////////////////////////////////////////////////////////////
//	GetOtherComponent
////////////////////////////////////////////////////////////////////////////////
Audio::Com Audio::SurroundLFEAudioInput::GetOtherComponent( Instance* pThisInstance )
{
	return 0;
}

////////////////////////////////////////////////////////////////////////////////
//	IsActive
////////////////////////////////////////////////////////////////////////////////
bool Audio::SurroundLFEAudioInput::IsActive( Instance* pInstance )
{
	State* pState = (State*)pInstance->GetStateData( m_uStateOffset );

	return pState->bActive;
}

////////////////////////////////////////////////////////////////////////////////
//	SetMute
////////////////////////////////////////////////////////////////////////////////
void Audio::SurroundLFEAudioInput::SetMute( Instance* pInstance, bool bMuted )
{
	State* pState = (State*)pInstance->GetStateData( m_uStateOffset );

	pState->bMuted = bMuted;
}
#endif

#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", on) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF
