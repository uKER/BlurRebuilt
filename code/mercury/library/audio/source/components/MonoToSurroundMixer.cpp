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
void* Audio::MonoToSurroundMixer::Init(void* pData)
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
U32 Audio::MonoToSurroundMixer::CoreMemory(U32 uOffset)
{
	Data* pInitData = (Data*)m_pObject->GetInitData(m_uInitOffset);

	m_uMixer = (ixCore)uOffset;

    //  mixer
	uOffset += Core::Mixer::MixerSize( pInitData->uInputs, 1, 5, GetNumDSPSlots() );

	return uOffset;
}

////////////////////////////////////////////////////////////////////////////////
//	InitContext
////////////////////////////////////////////////////////////////////////////////
void Audio::MonoToSurroundMixer::InitContext(Instance* pInstance)
{
	Data* pInitData = (Data*)m_pObject->GetInitData( m_uInitOffset );
    State* pState = (State*)pInstance->GetStateData( m_uStateOffset );
    Core::Mixer* pMixer = (Core::Mixer*)pInstance->GetCoreData( m_uMixer );

    //  create mixer
    new( pMixer ) Core::Mixer( Name(), pInitData->uInputs, 1, 5, GetNumDSPSlots(), false );

    pState->fOutputGain = -1.f;
    pState->fAzimuth = -1.f;
    pState->fElevation = -1.f;
    pState->fWidth = -1.f;
    pState->fDistance = -1.f;

	pState->sVolumes.fCentre = 0.f;
	pState->sVolumes.fFrontLeft = 0.f;
	pState->sVolumes.fFrontRight = 0.f;
	pState->sVolumes.fSurroundLeft = 0.f;
	pState->sVolumes.fSurroundRight = 0.f;
	pState->sVolumes.fBackLeft = 0.f;
	pState->sVolumes.fBackRight = 0.f;

	pState->bActive = false;
	pState->bMuted = false;
}

////////////////////////////////////////////////////////////////////////////////
//	ReleaseContext
////////////////////////////////////////////////////////////////////////////////
void Audio::MonoToSurroundMixer::ReleaseContext(Instance* pInstance)
{
    Core::Mixer* pMixer = (Core::Mixer*)pInstance->GetCoreData( m_uMixer );

	pMixer->~Mixer();
}
#endif	//	#ifndef PLATFORM_SPU

////////////////////////////////////////////////////////////////////////////////
//	Update
////////////////////////////////////////////////////////////////////////////////
void Audio::MonoToSurroundMixer::Update(Instance* pInstance)
{
	AUDIO_PERF_MICRO_EVENT("Audio.Update.MonoToSurroundMixer");

	Data* pInitData = (Data*)m_pObject->GetInitData( m_uInitOffset );
	InputGain* aInputs = (InputGain*)m_pObject->GetInitData( m_uInputOffset );
    State* pState = (State*)pInstance->GetNewStateData( m_uStateOffset );
    Core::Mixer* pMixer = (Core::Mixer*)pInstance->GetCoreData( m_uMixer );

	//	set if active
	pState->bActive = pMixer->IsActive();

	//  get output gain
	F32 fOutputGain = pState->bMuted ? 0.f : GetLevel( pInstance, pInitData->optLevel, pInitData->optGain );
	bool bOutputSilent = Maths::IsVolumeLessThan( fOutputGain, SoundComponent::iMinVolume );

	if( !bOutputSilent )
	{
		//  get azimuth
		F32 fAzimuth = GetOptionValue( pInstance, pInitData->optAzimuth );

		//  get elevation
		F32 fElevation = GetOptionValue( pInstance, pInitData->optElevation );

		//  get arc width
		F32 fWidth = Maths::Clamp( GetOptionValue( pInstance, pInitData->optWidth ), 0.f, 1.f );

		//  get distance
		F32 fDistance = GetOptionValue( pInstance, pInitData->optDistance );

		//  recalculate levels, if changed
		if( pState->fOutputGain != fOutputGain ||
			pState->fAzimuth != fAzimuth ||
			pState->fDistance != fDistance ||
			pState->fElevation != fElevation ||
			pState->fWidth != fWidth ||
			System::Get()->AnglesChanged() )
		{
			pState->fOutputGain = fOutputGain;
			pState->fAzimuth = fAzimuth;
			pState->fElevation = fElevation;
			pState->fWidth = fWidth;
			pState->fDistance = fDistance;

			GetSurroundVolumes( System::Get()->GetPanningMode(), (SurroundMode)pInitData->bfFlags.nSurroundMode, fAzimuth, fElevation, fDistance, fWidth, fOutputGain, pState->sVolumes );
		}
	}

	//  do each input
	for( U32 uInput = 0; uInput < pInitData->uInputs; uInput++ )
	{
		if( !bOutputSilent )
		{
			//  get input gain
			F32 fInputGain = GetLevel( pInstance, aInputs[ uInput ].optLevel, aInputs[ uInput ].optGain );

			//	set gains
			pMixer->SetGain( uInput, 0, 0, fInputGain * pState->sVolumes.fFrontLeft );
			pMixer->SetGain( uInput, 0, 1, fInputGain * pState->sVolumes.fFrontRight );
			pMixer->SetGain( uInput, 0, 2, fInputGain * pState->sVolumes.fCentre );
			pMixer->SetGain( uInput, 0, 3, fInputGain * pState->sVolumes.fSurroundLeft );
			pMixer->SetGain( uInput, 0, 4, fInputGain * pState->sVolumes.fSurroundRight );
		}
		else
		{
			//	zero gains
			pMixer->SetGain( uInput, 0, 0, 0.f ); // L
			pMixer->SetGain( uInput, 0, 1, 0.f ); // R
			pMixer->SetGain( uInput, 0, 2, 0.f ); // C
			pMixer->SetGain( uInput, 0, 3, 0.f ); // SL
			pMixer->SetGain( uInput, 0, 4, 0.f ); // SR
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
//	Value
////////////////////////////////////////////////////////////////////////////////
F32 Audio::MonoToSurroundMixer::Value( Instance* pInstance, U32 uOutput )
{
	State* pState = (State*)pInstance->GetStateData( m_uStateOffset );

	return pState->bActive ? 1.f : 0.f;
}

////////////////////////////////////////////////////////////////////////////////
//	AudioOutType
////////////////////////////////////////////////////////////////////////////////
const U32 Audio::MonoToSurroundMixer::AudioOutType(Instance* pInstance) const
{
	return Core::eMixer;
}

////////////////////////////////////////////////////////////////////////////////
//	AudioOutType
////////////////////////////////////////////////////////////////////////////////
Audio::Com Audio::MonoToSurroundMixer::GetInput( U8* pInitData, U32 uInput )
{
	InputGain* aInputs = (InputGain*)m_pObject->GetInitData( m_uInputOffset );
	return aInputs[ uInput ].sInputCom;
}

#if defined (AUDIO_ENABLE_PROFILING)
////////////////////////////////////////////////////////////////////////////////
//	GetNumInputs
////////////////////////////////////////////////////////////////////////////////
U16 Audio::MonoToSurroundMixer::GetNumInputs()
{
	Data* pInitData = (Data*)m_pObject->GetInitData(m_uInitOffset);

	return pInitData->uInputs;
}

////////////////////////////////////////////////////////////////////////////////
//	GetInputCom
////////////////////////////////////////////////////////////////////////////////
Audio::Com Audio::MonoToSurroundMixer::GetInputCom(U32 uIndex)
{
	InputGain* aInputs = (InputGain*)m_pObject->GetInitData(m_uInputOffset);

	return aInputs[uIndex].sInputCom;
}

////////////////////////////////////////////////////////////////////////////////
//	IsActive
////////////////////////////////////////////////////////////////////////////////
bool Audio::MonoToSurroundMixer::IsActive(Instance* pInstance)
{
	State* pState = (State*)pInstance->GetStateData(m_uStateOffset);

	return pState->bActive;
}

////////////////////////////////////////////////////////////////////////////////
//	SetMute
////////////////////////////////////////////////////////////////////////////////
void Audio::MonoToSurroundMixer::SetMute(Instance* pInstance, bool bMuted)
{
	State* pState = (State*)pInstance->GetStateData(m_uStateOffset);

	pState->bMuted = bMuted;
}
#endif	//	AUDIO_ENABLE_PROFILING

#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", on) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF



