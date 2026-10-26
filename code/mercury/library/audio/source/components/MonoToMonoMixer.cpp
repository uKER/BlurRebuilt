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
void* Audio::MonoToMonoMixer::Init(void* pData)
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
U32 Audio::MonoToMonoMixer::CoreMemory(U32 uOffset)
{
	Data* pInitData = (Data*)m_pObject->GetInitData(m_uInitOffset);

	m_uMixer = (ixCore)uOffset;

    //  mixer
	uOffset += Core::Mixer::MixerSize( pInitData->uInputs, 1, 1, GetNumDSPSlots() );

	return uOffset;
}

////////////////////////////////////////////////////////////////////////////////
//	InitContext
////////////////////////////////////////////////////////////////////////////////
void Audio::MonoToMonoMixer::InitContext(Instance* pInstance)
{
	Data* pInitData = (Data*)m_pObject->GetInitData(m_uInitOffset);
	State* pState = (State*)pInstance->GetStateData(m_uStateOffset);
    Core::Mixer* pMixer = (Core::Mixer*)pInstance->GetCoreData( m_uMixer );

    //  create mixer
    new( pMixer ) Core::Mixer( Name(), pInitData->uInputs, 1, 1, GetNumDSPSlots(), false );

	pState->bActive = false;
	pState->bMuted = false;
}

////////////////////////////////////////////////////////////////////////////////
//	ReleaseContext
////////////////////////////////////////////////////////////////////////////////
void Audio::MonoToMonoMixer::ReleaseContext(Instance* pInstance)
{
    Core::Mixer* pMixer = (Core::Mixer*)pInstance->GetCoreData( m_uMixer );

	pMixer->~Mixer();
}
#endif	//	#ifndef PLATFORM_SPU

////////////////////////////////////////////////////////////////////////////////
//	Update
////////////////////////////////////////////////////////////////////////////////
void Audio::MonoToMonoMixer::Update(Instance* pInstance)
{
	AUDIO_PERF_MICRO_EVENT("Audio.Update.MonoToMonoMixer");

	Data* pInitData = (Data*)m_pObject->GetInitData( m_uInitOffset );
	InputGain* aInputs = (InputGain*)m_pObject->GetInitData( m_uInputOffset );
	State* pState = (State*)pInstance->GetNewStateData( m_uStateOffset );
    Core::Mixer* pMixer = (Core::Mixer*)pInstance->GetCoreData( m_uMixer );

	//	set if active
	pState->bActive = pMixer->IsActive();

	//  get output gain
	F32 fOutputGain = pState->bMuted ? 0.f : GetLevel( pInstance, pInitData->optLevel, pInitData->optGain );
	bool bOutputSilent = Maths::IsVolumeLessThan( fOutputGain, SoundComponent::iMinVolume );

	//  do each input
	for ( U32 uInput = 0; uInput < pInitData->uInputs; uInput++ )
	{
		if ( !bOutputSilent )
		{
			//  get input gain
			F32 fInputGain = GetLevel( pInstance, aInputs[ uInput ].optLevel, aInputs[ uInput ].optGain );

			//	set gains
			pMixer->SetGain( uInput, 0, 0, fInputGain * fOutputGain );
		}
		else
		{
			//	zero gains
			pMixer->SetGain( uInput, 0, 0, 0.f );
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
//	Value
////////////////////////////////////////////////////////////////////////////////
F32 Audio::MonoToMonoMixer::Value( Instance* pInstance, U32 uOutput )
{
	State* pState = (State*)pInstance->GetStateData( m_uStateOffset );

	return pState->bActive ? 1.f : 0.f;
}

////////////////////////////////////////////////////////////////////////////////
//	AudioOutType
////////////////////////////////////////////////////////////////////////////////
const U32 Audio::MonoToMonoMixer::AudioOutType(Instance* pInstance) const
{
	return Core::eMixer;
}

////////////////////////////////////////////////////////////////////////////////
//	AudioOutType
////////////////////////////////////////////////////////////////////////////////
Audio::Com Audio::MonoToMonoMixer::GetInput( U8* pInitData, U32 uInput )
{
	InputGain* aInputs = (InputGain*)m_pObject->GetInitData( m_uInputOffset );
	return aInputs[ uInput ].sInputCom;
}

#if defined (AUDIO_ENABLE_PROFILING)
////////////////////////////////////////////////////////////////////////////////
//	GetNumInputs
////////////////////////////////////////////////////////////////////////////////
U16 Audio::MonoToMonoMixer::GetNumInputs()
{
	Data* pInitData = (Data*)m_pObject->GetInitData(m_uInitOffset);

	return pInitData->uInputs;
}

////////////////////////////////////////////////////////////////////////////////
//	GetInputCom
////////////////////////////////////////////////////////////////////////////////
Audio::Com Audio::MonoToMonoMixer::GetInputCom(U32 uIndex)
{
	InputGain* aInputs = (InputGain*)m_pObject->GetInitData(m_uInputOffset);

	return aInputs[uIndex].sInputCom;
}

////////////////////////////////////////////////////////////////////////////////
//	IsActive
////////////////////////////////////////////////////////////////////////////////
bool Audio::MonoToMonoMixer::IsActive(Instance* pInstance)
{
	State* pState = (State*)pInstance->GetStateData(m_uStateOffset);

	return pState->bActive;
}

////////////////////////////////////////////////////////////////////////////////
//	SetMute
////////////////////////////////////////////////////////////////////////////////
void Audio::MonoToMonoMixer::SetMute(Instance* pInstance, bool bMuted)
{
	State* pState = (State*)pInstance->GetStateData(m_uStateOffset);

	pState->bMuted = bMuted;
}
#endif	//	AUDIO_ENABLE_PROFILING

#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", on) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF
