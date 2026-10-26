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

extern bool g_bDumpStats;

////////////////////////////////////////////////////////////////////////////////
//	Init
////////////////////////////////////////////////////////////////////////////////
#ifndef PLATFORM_SPU
void* Audio::SurroundToSurroundMixer::Init(void* pData)
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
U32 Audio::SurroundToSurroundMixer::CoreMemory(U32 uOffset)
{
	Data* pInitData = (Data*)m_pObject->GetInitData(m_uInitOffset);

	m_uMixer = uOffset;

    //  mixer
	uOffset += Core::Mixer::MixerSize( pInitData->uInputs, 5, 5, GetNumDSPSlots() );

	return uOffset;
}

////////////////////////////////////////////////////////////////////////////////
//	InitContext
////////////////////////////////////////////////////////////////////////////////
void Audio::SurroundToSurroundMixer::InitContext(Instance* pInstance)
{
	Data* pInitData = (Data*)m_pObject->GetInitData(m_uInitOffset);
	U8* pStateData = pInstance->GetStateData();
	U8* pCoreData = pInstance->GetCoreData();
    State* pState = (State*)&pStateData[m_uStateOffset];
    Core::Mixer* pMixer = (Core::Mixer*)&pCoreData[m_uMixer];

    //  create mixer
    new( pMixer ) Core::Mixer( Name(), pInitData->uInputs, 5, 5, GetNumDSPSlots(), false );
	pCoreData += Core::Mixer::MixerSize( pInitData->uInputs, 5, 5, GetNumDSPSlots() );

    pState->fWidth = 2.f;
    pState->fGain = -1.f;

	pState->bActive = false;
	pState->bMuted = false;
}

////////////////////////////////////////////////////////////////////////////////
//	ReleaseContext
////////////////////////////////////////////////////////////////////////////////
void Audio::SurroundToSurroundMixer::ReleaseContext(Instance* pInstance)
{
	U8* pCoreData = pInstance->GetCoreData();
    Core::Mixer* pMixer = (Core::Mixer*)&pCoreData[m_uMixer];

	pMixer->~Mixer();
}
#endif	//	#ifndef PLATFORM_SPU

////////////////////////////////////////////////////////////////////////////////
//	Update
////////////////////////////////////////////////////////////////////////////////
void Audio::SurroundToSurroundMixer::Update(Instance* pInstance)
{
    AUDIO_PERF_MICRO_EVENT("Audio.Update.SurroundToSurroundMixer");

	Data* pInitData = (Data*)m_pObject->GetInitData(m_uInitOffset);
	InputGain* aInputs = (InputGain*)m_pObject->GetInitData(m_uInputOffset);
    State* pState = (State*)pInstance->GetNewStateData(m_uStateOffset);
    Core::Mixer* pMixer = (Core::Mixer*)pInstance->GetCoreData( m_uMixer );

	//	set if active
	pState->bActive = pMixer->IsActive();

	//  get output gain
	F32 fOutputGain = pState->bMuted ? 0.f : GetLevel( pInstance, pInitData->optLevel, pInitData->optGain );
	bool bOutputSilent = Maths::IsVolumeLessThan( fOutputGain, SoundComponent::iMinVolume );

	//  do each input
	for (U32 uInput = 0; uInput < pInitData->uInputs; uInput++)
	{
		F32 afGains[ 5 ][ 5 ];

		//	zero gains
		memset( afGains, 0, 5 * 5 * sizeof(F32) );

		if ( !bOutputSilent )
		{
			//  get input gain
			F32 fInputGain = GetLevel( pInstance, aInputs[ uInput ].optLevel, aInputs[ uInput ].optGain );

			F32 fGain = fInputGain * fOutputGain;

			afGains[ 0 ][ 0 ] = fGain;
			afGains[ 1 ][ 1 ] = fGain;
			afGains[ 2 ][ 2 ] = fGain;
			afGains[ 3 ][ 3 ] = fGain;
			afGains[ 4 ][ 4 ] = fGain;
		}

		//  set gains
		pMixer->SetGains( uInput, (F32*)afGains );
	}
}

////////////////////////////////////////////////////////////////////////////////
//	Value
////////////////////////////////////////////////////////////////////////////////
F32 Audio::SurroundToSurroundMixer::Value( Instance* pInstance, U32 uOutput )
{
	State* pState = (State*)pInstance->GetStateData( m_uStateOffset );

	return pState->bActive ? 1.f : 0.f;
}

////////////////////////////////////////////////////////////////////////////////
//	AudioOutType
////////////////////////////////////////////////////////////////////////////////
const U32 Audio::SurroundToSurroundMixer::AudioOutType(Instance* pInstance) const
{
	return Core::eMixer;
}

////////////////////////////////////////////////////////////////////////////////
//	AudioOutType
////////////////////////////////////////////////////////////////////////////////
Audio::Com Audio::SurroundToSurroundMixer::GetInput( U8* pInitData, U32 uInput )
{
	InputGain* aInputs = (InputGain*)m_pObject->GetInitData( m_uInputOffset );
	return aInputs[ uInput ].sInputCom;
}

#if defined (AUDIO_ENABLE_PROFILING)
////////////////////////////////////////////////////////////////////////////////
//	GetNumInputs
////////////////////////////////////////////////////////////////////////////////
U16 Audio::SurroundToSurroundMixer::GetNumInputs()
{
	Data* pInitData = (Data*)m_pObject->GetInitData(m_uInitOffset);

	return pInitData->uInputs;
}

////////////////////////////////////////////////////////////////////////////////
//	GetInputCom
////////////////////////////////////////////////////////////////////////////////
Audio::Com Audio::SurroundToSurroundMixer::GetInputCom(U32 uIndex)
{
	InputGain* aInputs = (InputGain*)m_pObject->GetInitData(m_uInputOffset);

	return aInputs[uIndex].sInputCom;
}

////////////////////////////////////////////////////////////////////////////////
//	IsActive
////////////////////////////////////////////////////////////////////////////////
bool Audio::SurroundToSurroundMixer::IsActive(Instance* pInstance)
{
	State* pState = (State*)pInstance->GetStateData(m_uStateOffset);

	return pState->bActive;
}

////////////////////////////////////////////////////////////////////////////////
//	SetMute
////////////////////////////////////////////////////////////////////////////////
void Audio::SurroundToSurroundMixer::SetMute(Instance* pInstance, bool bMuted)
{
	State* pState = (State*)pInstance->GetStateData(m_uStateOffset);

	pState->bMuted = bMuted;
}
#endif	//	AUDIO_ENABLE_PROFILING

#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", on) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF
