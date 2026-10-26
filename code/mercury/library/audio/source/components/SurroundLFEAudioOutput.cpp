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
void* Audio::SurroundLFEAudioOutput::Init(void* pData)
{
	Data* pInitData = PreInit<Data>(pData);

    Assert( pInitData->sInputCom != -1 );

    return (void*)( pInitData + 1 );
}

////////////////////////////////////////////////////////////////////////////////
//	CoreMemory
////////////////////////////////////////////////////////////////////////////////
U32 Audio::SurroundLFEAudioOutput::CoreMemory(U32 uOffset)
{
	m_uOutput = uOffset;

	return uOffset + Core::Output::OutputSize( 6, 0 );
}

////////////////////////////////////////////////////////////////////////////////
//	InitContext
////////////////////////////////////////////////////////////////////////////////
void Audio::SurroundLFEAudioOutput::InitContext(Instance* pInstance)
{
	State* pState = (State*)pInstance->GetStateData( m_uStateOffset );
	U8* pCoreData = pInstance->GetCoreData();

	new( pInstance->GetCoreData( m_uOutput ) ) Core::Output( Name(), 6, 0 );

	pState->bActive = true;
	pState->bMuted = false;
}

////////////////////////////////////////////////////////////////////////////////
//	ReleaseContext
////////////////////////////////////////////////////////////////////////////////
void Audio::SurroundLFEAudioOutput::ReleaseContext(Instance* pInstance)
{
	Core::Output* pOutput = (Core::Output*)pInstance->GetCoreData( m_uOutput );

	pOutput->~Output();
}
#endif	//	#ifndef PLATFORM_SPU

////////////////////////////////////////////////////////////////////////////////
//	Generate
////////////////////////////////////////////////////////////////////////////////
void Audio::SurroundLFEAudioOutput::Generate( Instance* pInstance )
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
void Audio::SurroundLFEAudioOutput::OutputTo(Instance* pInstance, Core::Input* pInput)
{
	Core::Output* pOutput = (Core::Output*)pInstance->GetCoreData( m_uOutput );

	pOutput->AddInput( pInput );
}

////////////////////////////////////////////////////////////////////////////////
//	DisconnectFrom
////////////////////////////////////////////////////////////////////////////////
void Audio::SurroundLFEAudioOutput::DisconnectFrom(Instance* pInstance, Core::Input* pInput)
{
	Core::Output* pOutput = (Core::Output*)pInstance->GetCoreData( m_uOutput );

	pOutput->RemoveInput( pInput );
}

////////////////////////////////////////////////////////////////////////////////
//	ConfigureAudio
////////////////////////////////////////////////////////////////////////////////
void Audio::SurroundLFEAudioOutput::ConfigureAudio( Instance* pInstance )
{
	Data* pInitData = (Data*)m_pObject->GetInitData( m_uInitOffset );
	Core::Output* pOutput = (Core::Output*)pInstance->GetCoreData( m_uOutput );

	Component* pCom = m_pObject->GetComponent( pInitData->sInputCom );

	Assert( pCom->IsAudio( ) );

	AudioComponent* pAudioCom = (AudioComponent*)pCom;

	pOutput->SetInput( pAudioCom->AudioOutType( pInstance ), pAudioCom->AudioOutOffset( pInstance ) );
}

////////////////////////////////////////////////////////////////////////////////
//	ProcessConnection
////////////////////////////////////////////////////////////////////////////////
void Audio::SurroundLFEAudioOutput::ProcessConnection( Instance* pInstance )
{
	Core::Output* pOutput = (Core::Output*)pInstance->GetCoreData( m_uOutput );

	pOutput->MoveAudioBatch( pInstance->m_coreInfo );
}

////////////////////////////////////////////////////////////////////////////////
//	DumpProcessFlags
////////////////////////////////////////////////////////////////////////////////
#ifndef CFG_GOLD
void Audio::SurroundLFEAudioOutput::DumpProcessFlags( Instance* pInstance )
{
#ifdef DUMP_PROCESSING_STATS
	Core::Output* pOutput = (Core::Output*)pInstance->GetCoreData( m_uOutput );

	pOutput->DumpProcessingStatus( pInstance->m_coreInfo, pInstance->GetObject()->Name(), 0 );
#endif
}
#endif

#if defined (AUDIO_ENABLE_PROFILING)
////////////////////////////////////////////////////////////////////////////////
//	GetNumInputs
////////////////////////////////////////////////////////////////////////////////
U16 Audio::SurroundLFEAudioOutput::GetNumInputs()
{
	return 1;
}

////////////////////////////////////////////////////////////////////////////////
//	GetInputCom
////////////////////////////////////////////////////////////////////////////////
Audio::Com Audio::SurroundLFEAudioOutput::GetInputCom( U32 uIndex )
{
	Data* pInitData = (Data*)m_pObject->GetInitData( m_uInitOffset );

	return pInitData->sInputCom;
}

////////////////////////////////////////////////////////////////////////////////
//	IsActive
////////////////////////////////////////////////////////////////////////////////
bool Audio::SurroundLFEAudioOutput::IsActive( Instance* pInstance )
{
	State* pState = (State*)pInstance->GetStateData( m_uStateOffset );

	return pState->bActive;
}

////////////////////////////////////////////////////////////////////////////////
//	SetMute
////////////////////////////////////////////////////////////////////////////////
void Audio::SurroundLFEAudioOutput::SetMute( Instance* pInstance, bool bMuted )
{
	State* pState = (State*)pInstance->GetStateData( m_uStateOffset );

	pState->bMuted = bMuted;
}
#endif	//	AUDIO_ENABLE_PROFILING

#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", on) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF
