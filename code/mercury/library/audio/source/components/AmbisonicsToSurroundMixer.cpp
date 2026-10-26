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
void* Audio::AmbisonicsToSurroundMixer::Init(void* pData)
{
	Data* pInitData = PreInit<Data>(pData);

	m_uEffectOffset = m_uInitOffset + sizeof(Data);

	m_uInputOffset = m_uEffectOffset + pInitData->uEffects * sizeof(U32);

	U32 uNext = m_uInputOffset + pInitData->uInputs * sizeof(InputGain);

	m_uNumInputs = pInitData->uInputs;
	m_uNumEffects = pInitData->uEffects;

	return (void*)((U8*)m_pObject->GetInitData() + uNext);
}

////////////////////////////////////////////////////////////////////////////////
//	CoreMemory
////////////////////////////////////////////////////////////////////////////////
U32 Audio::AmbisonicsToSurroundMixer::CoreMemory(U32 uOffset)
{
	Data* pInitData = (Data*)m_pObject->GetInitData(m_uInitOffset);

	m_uMixer = (ixCore)uOffset;

    //  mixer
#if defined (AMBISONICS_1ST_ORDER)
	uOffset += Core::Mixer::MixerSize( pInitData->uInputs, 3, 5, GetNumDSPSlots() );
#endif
#if defined (AMBISONICS_2ND_ORDER)
	uOffset += Core::Mixer::MixerSize( pInitData->uInputs, 5, 5, GetNumDSPSlots() );
#endif
#if defined (AMBISONICS_3RD_ORDER)
	uOffset += Core::Mixer::MixerSize( pInitData->uInputs, 8, 5, GetNumDSPSlots() );
#endif

	return uOffset;
}

////////////////////////////////////////////////////////////////////////////////
//	InitContext
////////////////////////////////////////////////////////////////////////////////
void Audio::AmbisonicsToSurroundMixer::InitContext(Instance* pInstance)
{
	Data* pInitData = (Data*)m_pObject->GetInitData( m_uInitOffset );
    State* pState = (State*)pInstance->GetStateData( m_uStateOffset );
    Core::Mixer* pMixer = (Core::Mixer*)pInstance->GetCoreData( m_uMixer );

    //  create mixer
#if defined (AMBISONICS_1ST_ORDER)
    new( pMixer ) Core::Mixer( Name(), pInitData->uInputs, 3, 5, GetNumDSPSlots(), false );
#endif
#if defined (AMBISONICS_2ND_ORDER)
    new( pMixer ) Core::Mixer( Name(), pInitData->uInputs, 5, 5, GetNumDSPSlots(), false );
#endif
#if defined (AMBISONICS_3RD_ORDER)
    new( pMixer ) Core::Mixer( Name(), pInitData->uInputs, 8, 5, GetNumDSPSlots(), false );
#endif

    pState->fOutputGain = -1.f;
    pState->fDistance = -1.f;

	for ( U32 uCount = 0; uCount < MAX_NUM_SURROUND_SPEAKERS; uCount++ )
		pState->afSpeakerVolumes[ uCount ] = 0.f;

	pState->bActive = false;
	pState->bMuted = false;
}

////////////////////////////////////////////////////////////////////////////////
//	ReleaseContext
////////////////////////////////////////////////////////////////////////////////
void Audio::AmbisonicsToSurroundMixer::ReleaseContext(Instance* pInstance)
{
    Core::Mixer* pMixer = (Core::Mixer*)pInstance->GetCoreData( m_uMixer );

	pMixer->~Mixer();
}
#endif	//	#ifndef PLATFORM_SPU

////////////////////////////////////////////////////////////////////////////////
//	Update
////////////////////////////////////////////////////////////////////////////////
void Audio::AmbisonicsToSurroundMixer::Update(Instance* pInstance)
{
	AUDIO_PERF_MICRO_EVENT("Audio.Update.AmbisonicsToSurroundMixer");

	Data* pInitData = (Data*)m_pObject->GetInitData( m_uInitOffset );
	InputGain* aInputs = (InputGain*)m_pObject->GetInitData( m_uInputOffset );
    State* pState = (State*)pInstance->GetNewStateData( m_uStateOffset );
    Core::Mixer* pMixer = (Core::Mixer*)pInstance->GetCoreData( m_uMixer );

#if defined (AMBISONICS_1ST_ORDER)
	U32 uNumSignals = 3;
	static F32 afEncoder[ 3 ][ 5 ] = { {  0.4563f,  0.1690f, 0.1635f, 0.1690f,  0.4563f },		//	W
									   { -0.1259f,  0.0797f, 0.0923f, 0.0797f, -0.1259f },		//	X
									   { -0.1543f, -0.0891f, 0.0000f, 0.0891f,  0.1543f } };	//	Y
#endif
#if defined (AMBISONICS_2ND_ORDER)
	U32 uNumSignals = 5;
	static F32 afEncoder[ 5 ][ 5 ] = { {  0.9101f,  0.4724f, 0.3226f,  0.4724f,  0.9101f }, 	//	W
									   { -0.7834f,  0.7143f, 0.7719f,  0.7143f, -0.7834f },		//	X
									   {  0.9562f,  0.7258f, 0.0000f, -0.7258f, -0.9562f },		//	Y
									   { -0.0806f,  0.0000f, 0.0000f,  0.0000f, -0.0806f },		//	U
									   {  0.0000f,  0.3456f, 0.4724f, -0.3456f,  0.0000f } };	//	V
#endif
#if defined (AMBISONICS_3RD_ORDER)
	U32 uNumSignals = 5;
	static F32 afEncoder[ 8 ][ 5 ] = { {  0.9101f,  0.4724f, 0.3226f,  0.4724f,  0.9101f }, 	//	W
									   { -0.7834f,  0.7143f, 0.7719f,  0.7143f, -0.7834f },		//	X
									   {  0.9562f,  0.7258f, 0.0000f, -0.7258f, -0.9562f },		//	Y
									   { -0.0806f,  0.0000f, 0.0000f,  0.0000f, -0.0806f },		//	U
									   {  0.0000f,  0.3456f, 0.4724f, -0.3456f,  0.0000f },		//	V
									   { -0.0806f,  0.0000f, 0.0000f,  0.0000f, -0.0806f },		//	
									   { -0.0806f,  0.0000f, 0.0000f,  0.0000f, -0.0806f },		//	
									   { -0.0806f,  0.0000f, 0.0000f,  0.0000f, -0.0806f } };	//	
#endif

	//	set if active
	pState->bActive = pMixer->IsActive();

	//  get output gain
	F32 fOutputGain = pState->bMuted ? 0.f : GetLevel( pInstance, pInitData->optLevel, pInitData->optGain );
	bool bOutputSilent = Maths::IsVolumeLessThan( fOutputGain, SoundComponent::iMinVolume );

	//  do each input
	for(U32 uInput = 0; uInput < pInitData->uInputs; uInput++)
	{
		if( !bOutputSilent )
		{
			//  get input gain
			F32 fInputGain = GetLevel( pInstance, aInputs[ uInput ].optLevel, aInputs[ uInput ].optGain );

			//	set gains
			for( U32 uSignal = 0; uSignal < uNumSignals; uSignal++ )
			{
				pMixer->SetGain( uInput, uSignal, 0, fInputGain * afEncoder[ uSignal ][ 1 ] ); // L
				pMixer->SetGain( uInput, uSignal, 1, fInputGain * afEncoder[ uSignal ][ 3 ] ); // R
				pMixer->SetGain( uInput, uSignal, 2, fInputGain * afEncoder[ uSignal ][ 2 ] ); // C
				pMixer->SetGain( uInput, uSignal, 3, fInputGain * afEncoder[ uSignal ][ 0 ] ); // SL
				pMixer->SetGain( uInput, uSignal, 4, fInputGain * afEncoder[ uSignal ][ 4 ] ); // SR
			}
		}
		else
		{
			//	zero gains
			for( U32 uSignal = 0; uSignal < uNumSignals; uSignal++ )
			{
				pMixer->SetGain( uInput, uSignal, 0, 0.f ); // L
				pMixer->SetGain( uInput, uSignal, 1, 0.f ); // R
				pMixer->SetGain( uInput, uSignal, 2, 0.f ); // C
				pMixer->SetGain( uInput, uSignal, 3, 0.f ); // SL
				pMixer->SetGain( uInput, uSignal, 4, 0.f ); // SR
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
//	AudioOutType
////////////////////////////////////////////////////////////////////////////////
const U32 Audio::AmbisonicsToSurroundMixer::AudioOutType(Instance* pInstance) const
{
	return Core::eMixer;
}

////////////////////////////////////////////////////////////////////////////////
//	AudioOutType
////////////////////////////////////////////////////////////////////////////////
Audio::Com Audio::AmbisonicsToSurroundMixer::GetInput( U8* pInitData, U32 uInput )
{
	InputGain* aInputs = (InputGain*)m_pObject->GetInitData( m_uInputOffset );
	return aInputs[ uInput ].sInputCom;
}

#if defined (AUDIO_ENABLE_PROFILING)
////////////////////////////////////////////////////////////////////////////////
//	GetNumInputs
////////////////////////////////////////////////////////////////////////////////
U16 Audio::AmbisonicsToSurroundMixer::GetNumInputs()
{
	Data* pInitData = (Data*)m_pObject->GetInitData(m_uInitOffset);

	return pInitData->uInputs;
}

////////////////////////////////////////////////////////////////////////////////
//	GetInputCom
////////////////////////////////////////////////////////////////////////////////
Audio::Com Audio::AmbisonicsToSurroundMixer::GetInputCom(U32 uIndex)
{
	InputGain* aInputs = (InputGain*)m_pObject->GetInitData(m_uInputOffset);

	return aInputs[uIndex].sInputCom;
}

////////////////////////////////////////////////////////////////////////////////
//	IsActive
////////////////////////////////////////////////////////////////////////////////
bool Audio::AmbisonicsToSurroundMixer::IsActive(Instance* pInstance)
{
	State* pState = (State*)pInstance->GetStateData(m_uStateOffset);

	return pState->bActive;
}

////////////////////////////////////////////////////////////////////////////////
//	SetMute
////////////////////////////////////////////////////////////////////////////////
void Audio::AmbisonicsToSurroundMixer::SetMute(Instance* pInstance, bool bMuted)
{
	State* pState = (State*)pInstance->GetStateData(m_uStateOffset);

	pState->bMuted = bMuted;
}
#endif	//	AUDIO_ENABLE_PROFILING

#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", on) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF



