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
void* Audio::StereoAudioOutput::Init(void* pData)
{
	Data* pInitData = PreInit<Data>(pData);

    Assert( pInitData->sInputCom != -1 );

    return (void*)( pInitData + 1 );
}

////////////////////////////////////////////////////////////////////////////////
//	CoreMemory
////////////////////////////////////////////////////////////////////////////////
U32 Audio::StereoAudioOutput::CoreMemory(U32 uOffset)
{
	m_uOutput = uOffset;

	return uOffset + Core::Output::OutputSize( 2, 0 );
}

////////////////////////////////////////////////////////////////////////////////
//	InitContext
////////////////////////////////////////////////////////////////////////////////
void Audio::StereoAudioOutput::InitContext(Instance* pInstance)
{
    State* pState = (State*)pInstance->GetStateData( m_uStateOffset );

	new( pInstance->GetCoreData( m_uOutput ) ) Core::Output( Name(), 2, 0 );

    pState->bActive = false;
    pState->bMuted = false;
}

////////////////////////////////////////////////////////////////////////////////
//	ReleaseContext
////////////////////////////////////////////////////////////////////////////////
void Audio::StereoAudioOutput::ReleaseContext(Instance* pInstance)
{
	Core::Output* pOutput = (Core::Output*)pInstance->GetCoreData( m_uOutput );

	pOutput->~Output();
}
#endif	//	#ifndef PLATFORM_SPU

////////////////////////////////////////////////////////////////////////////////
//	Generate
////////////////////////////////////////////////////////////////////////////////
void Audio::StereoAudioOutput::Generate( Instance* pInstance )
{
	State* pState = (State*)pInstance->GetStateData( m_uStateOffset );
	Core::Output* pOutput = (Core::Output*)pInstance->GetCoreData( m_uOutput );

	pOutput->Process( pInstance->m_coreInfo );

	//	TODO: move to Update()
	pState->bActive = pOutput->IsActive();

	pOutput->SetMuted( pState->bMuted );
}

////////////////////////////////////////////////////////////////////////////////
//	OutputTo
////////////////////////////////////////////////////////////////////////////////
#ifndef PLATFORM_SPU
void Audio::StereoAudioOutput::OutputTo(Instance* pInstance, Core::Input* pInput)
{
	Core::Output* pOutput = (Core::Output*)pInstance->GetCoreData( m_uOutput );

	Assert( pInput != NULL );

	pOutput->AddInput( pInput );
}

////////////////////////////////////////////////////////////////////////////////
//	DisconnectFrom
////////////////////////////////////////////////////////////////////////////////
void Audio::StereoAudioOutput::DisconnectFrom(Instance* pInstance, Core::Input* pInput)
{
	Core::Output* pOutput = (Core::Output*)pInstance->GetCoreData( m_uOutput );

	Assert( pInput != NULL );

	pOutput->RemoveInput( pInput );
}

////////////////////////////////////////////////////////////////////////////////
//	ConfigureAudio
////////////////////////////////////////////////////////////////////////////////
void Audio::StereoAudioOutput::ConfigureAudio(Instance* pInstance)
{
	Data* pInitData = (Data*)m_pObject->GetInitData( m_uInitOffset );
	Core::Output* pOutput = (Core::Output*)pInstance->GetCoreData( m_uOutput );

	Component* pCom = m_pObject->GetComponent( pInitData->sInputCom );

	Assert( pCom->IsAudio() );

	AudioComponent* pAudioCom = (AudioComponent*)pCom;

	pOutput->SetInput( pAudioCom->AudioOutType( pInstance ), pAudioCom->AudioOutOffset( pInstance ) );
}

////////////////////////////////////////////////////////////////////////////////
//	ProcessConnection
////////////////////////////////////////////////////////////////////////////////
void Audio::StereoAudioOutput::ProcessConnection( Instance* pInstance )
{
	Core::Output* pOutput = (Core::Output*)pInstance->GetCoreData( m_uOutput );

	pOutput->MoveAudioBatch( pInstance->m_coreInfo );
}

////////////////////////////////////////////////////////////////////////////////
//	DumpProcessFlags
////////////////////////////////////////////////////////////////////////////////
#ifndef CFG_GOLD
void Audio::StereoAudioOutput::DumpProcessFlags( Instance* pInstance )
{
#ifdef DUMP_PROCESSING_STATS
	Core::Output* pOutput = (Core::Output*)pInstance->GetCoreData( m_uOutput );

	pOutput->DumpProcessingStatus( pInstance->m_coreInfo, pInstance->GetObject()->Name(), 0 );
#endif
}
#endif

#if defined (AUDIO_ENABLE_PROFILING)
////////////////////////////////////////////////////////////////////////////////
//	GetInputCom
////////////////////////////////////////////////////////////////////////////////
Audio::Com Audio::StereoAudioOutput::GetInputCom(U32 uIndex)
{
	Data* pInitData = (Data*)m_pObject->GetInitData( m_uInitOffset );

	return pInitData->sInputCom;
}

////////////////////////////////////////////////////////////////////////////////
//	IsActive
////////////////////////////////////////////////////////////////////////////////
bool Audio::StereoAudioOutput::IsActive(Instance* pInstance)
{
	State* pState = (State*)pInstance->GetStateData( m_uStateOffset );

	return pState->bActive;
}

////////////////////////////////////////////////////////////////////////////////
//	SetMute
////////////////////////////////////////////////////////////////////////////////
void Audio::StereoAudioOutput::SetMute( Instance* pInstance, bool bMuted )
{
	State* pState = (State*)pInstance->GetStateData( m_uStateOffset );

	pState->bMuted = bMuted;
}

////////////////////////////////////////////////////////////////////////////////
//	GetOtherInstance
////////////////////////////////////////////////////////////////////////////////
Audio::Instance* Audio::StereoAudioOutput::GetOtherInstance( Instance* pThisInstance )
{
    State* pState = (State*)pThisInstance->GetStateData( m_uStateOffset );

	return NULL;//pState->pInstanceIn;
}

////////////////////////////////////////////////////////////////////////////////
//	GetOtherComponent
////////////////////////////////////////////////////////////////////////////////
Audio::Com Audio::StereoAudioOutput::GetOtherComponent( Instance* pThisInstance )
{
	return 0;
}
#endif

#endif	//	#ifndef PLATFORM_SPU

#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", on)
#endif	//	AUDIO_OPTIMIZE_OFF
