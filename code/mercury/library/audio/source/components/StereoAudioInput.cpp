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
void* Audio::StereoAudioInput::Init(void* pData)
{
	Data* pInitData = PreInit<Data>(pData);
    return (void*)(pInitData + 1);
}

////////////////////////////////////////////////////////////////////////////////
//	CoreMemory
////////////////////////////////////////////////////////////////////////////////
U32 Audio::StereoAudioInput::CoreMemory( U32 uOffset )
{
	m_uInput = uOffset;

	return uOffset + Core::Input::InputSize( 2, 0 );
}

////////////////////////////////////////////////////////////////////////////////
//	InitContext
////////////////////////////////////////////////////////////////////////////////
void Audio::StereoAudioInput::InitContext( Instance* pInstance )
{
    State* pState = (State*)pInstance->GetStateData( m_uStateOffset );

	Core::Input* pInput = new( pInstance->GetCoreData( m_uInput ) ) Core::Input( Name(), 2, 0 );

	pState->pFrameInput = (F32*)System::Get()->Alloc( 2 * Core::uFrameLength * sizeof(F32), 128 );
	pInput->SetAltBuffer( pState->pFrameInput );

	for( U32 uCh = 0; uCh < 2; uCh++ )
		Frame::Zero( &pState->pFrameInput[ uCh * Core::uFrameLength ] );

	pState->bActive = true;
	pState->bMuted = false;
}

////////////////////////////////////////////////////////////////////////////////
//	ReleaseContext
////////////////////////////////////////////////////////////////////////////////
void Audio::StereoAudioInput::ReleaseContext(Instance* pInstance)
{
    State* pState = (State*)pInstance->GetStateData(m_uStateOffset);
	Core::Input* pInput = (Core::Input*)pInstance->GetCoreData( m_uInput );

	System::Get()->Free(pState->pFrameInput);

	pInput->~Input();
}
#endif	//	#ifndef PLATFORM_SPU

////////////////////////////////////////////////////////////////////////////////
//	Update
////////////////////////////////////////////////////////////////////////////////
void Audio::StereoAudioInput::Update( Instance* pInstance )
{
    AUDIO_PERF_MICRO_EVENT("Audio.Update.StereoAudioInput");

	State* pState = (State*)pInstance->GetNewStateData( m_uStateOffset );
	Core::Input* pInput = (Core::Input*)pInstance->GetCoreData( m_uInput );

	pState->bActive = pInput->IsActive();

	pInput->SetMuted( pState->bMuted );
}

////////////////////////////////////////////////////////////////////////////////
//	AudioInput
////////////////////////////////////////////////////////////////////////////////
Audio::Core::Input* Audio::StereoAudioInput::AudioInput(Instance* pInstance)
{
    return (Core::Input*)pInstance->GetCoreData( m_uInput );
}

#if defined (AUDIO_ENABLE_PROFILING)
////////////////////////////////////////////////////////////////////////////////
//	GetOtherInstance
////////////////////////////////////////////////////////////////////////////////
Audio::Instance* Audio::StereoAudioInput::GetOtherInstance( Instance* pThisInstance )
{
    State* pState = (State*)pThisInstance->GetStateData( m_uStateOffset );

	return NULL;//pState->pInstanceOut;
}

////////////////////////////////////////////////////////////////////////////////
//	GetOtherComponent
////////////////////////////////////////////////////////////////////////////////
Audio::Com Audio::StereoAudioInput::GetOtherComponent( Instance* pThisInstance )
{
	return 0;
}

////////////////////////////////////////////////////////////////////////////////
//	IsActive
////////////////////////////////////////////////////////////////////////////////
bool Audio::StereoAudioInput::IsActive( Instance* pInstance )
{
	State* pState = (State*)pInstance->GetStateData( m_uStateOffset );

	return pState->bActive;
}

////////////////////////////////////////////////////////////////////////////////
//	SetMute
////////////////////////////////////////////////////////////////////////////////
void Audio::StereoAudioInput::SetMute( Instance* pInstance, bool bMuted )
{
	State* pState = (State*)pInstance->GetStateData( m_uStateOffset );

	pState->bMuted = bMuted;
}
#endif

#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", on) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF


