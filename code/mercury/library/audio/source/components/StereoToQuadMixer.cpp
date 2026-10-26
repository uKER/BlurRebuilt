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
void* Audio::StereoToQuadMixer::Init(void* pData)
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
U32 Audio::StereoToQuadMixer::CoreMemory(U32 uOffset)
{
	Data* pInitData = (Data*)m_pObject->GetInitData(m_uInitOffset);

	m_uMixer = uOffset;

    //  mixer
	uOffset += Core::Mixer::MixerSize( pInitData->uInputs, 2, 4, GetNumDSPSlots() );

	return uOffset;
}

////////////////////////////////////////////////////////////////////////////////
//	InitContext
////////////////////////////////////////////////////////////////////////////////
void Audio::StereoToQuadMixer::InitContext(Instance* pInstance)
{
	Data* pInitData = (Data*)m_pObject->GetInitData(m_uInitOffset);
	U8* pStateData = pInstance->GetStateData();
	U8* pCoreData = pInstance->GetCoreData();
    State* pState = (State*)&pStateData[m_uStateOffset];
    Core::Mixer* pMixer = (Core::Mixer*)&pCoreData[m_uMixer];

    new( pMixer ) Core::Mixer( Name(), pInitData->uInputs, 2, 4, GetNumDSPSlots(), false );
	pCoreData += Core::Mixer::MixerSize( pInitData->uInputs, 2, 4, GetNumDSPSlots() );

	memset( pState->afGain, 0, 2 * 4 * sizeof(F32) );

	pState->bActive = false;
	pState->bMuted = false;
}

////////////////////////////////////////////////////////////////////////////////
//	ReleaseContext
////////////////////////////////////////////////////////////////////////////////
void Audio::StereoToQuadMixer::ReleaseContext(Instance* pInstance)
{
	U8* pCoreData = pInstance->GetCoreData();
    Core::Mixer* pMixer = (Core::Mixer*)&pCoreData[m_uMixer];

	pMixer->~Mixer();
}
#endif	//	#ifndef PLATFORM_SPU

////////////////////////////////////////////////////////////////////////////////
//	Update
////////////////////////////////////////////////////////////////////////////////
void Audio::StereoToQuadMixer::Update(Instance* pInstance)
{
    AUDIO_PERF_MICRO_EVENT("Audio.Update.StereoToQuadMixer");

	Data* pInitData = (Data*)m_pObject->GetInitData(m_uInitOffset);
	InputGain* aInputs = (InputGain*)m_pObject->GetInitData(m_uInputOffset);
	U8* pStateData = pInstance->GetNewStateData();
	U8* pCoreData = pInstance->GetCoreData();
    State* pState = (State*)&pStateData[m_uStateOffset];
    Core::Mixer* pMixer = (Core::Mixer*)&pCoreData[m_uMixer];

	//	set if active
	pState->bActive = pMixer->IsActive();

	//  get output gain
	F32 fOutputGain = pState->bMuted ? 0.f : GetLevel( pInstance, pInitData->optLevel, pInitData->optGain );
	bool bOutputSilent = Maths::IsVolumeLessThan( fOutputGain, SoundComponent::iMinVolume );

	//  do each input
	for (U32 uInput = 0; uInput < pInitData->uInputs; uInput++)
	{
		F32 afGains[ 2 ][ 4 ];

		if ( !bOutputSilent )
		{
			//  get input gains
			afGains[ 0 ][ 0 ] = fOutputGain * GetLevel( pInstance, aInputs[ uInput ].optLeftToFrontLeftLevel, aInputs[ uInput ].optLeftToFrontLeftGain );
			afGains[ 1 ][ 0 ] = fOutputGain * GetLevel( pInstance, aInputs[ uInput ].optRightToFrontLeftLevel, aInputs[ uInput ].optRightToFrontLeftGain );
			afGains[ 0 ][ 1 ] = fOutputGain * GetLevel( pInstance, aInputs[ uInput ].optLeftToFrontRightLevel, aInputs[ uInput ].optLeftToFrontRightGain );
			afGains[ 1 ][ 1 ] = fOutputGain * GetLevel( pInstance, aInputs[ uInput ].optRightToFrontRightLevel, aInputs[ uInput ].optRightToFrontRightGain );
			afGains[ 0 ][ 2 ] = fOutputGain * GetLevel( pInstance, aInputs[ uInput ].optLeftToRearLeftLevel, aInputs[ uInput ].optLeftToRearLeftGain );
			afGains[ 1 ][ 2 ] = fOutputGain * GetLevel( pInstance, aInputs[ uInput ].optRightToRearLeftLevel, aInputs[ uInput ].optRightToRearLeftGain );
			afGains[ 0 ][ 3 ] = fOutputGain * GetLevel( pInstance, aInputs[ uInput ].optLeftToRearRightLevel, aInputs[ uInput ].optLeftToRearRightGain );
			afGains[ 1 ][ 3 ] = fOutputGain * GetLevel( pInstance, aInputs[ uInput ].optRightToRearRightLevel, aInputs [uInput ].optRightToRearRightGain );
		}
		else
		{
			//	zero gains
			memset( afGains, 0, 2 * 4 * sizeof(F32) );
		}

		//  set gains
		pMixer->SetGains( uInput, (F32*)afGains );
	}
}

////////////////////////////////////////////////////////////////////////////////
//	Value
////////////////////////////////////////////////////////////////////////////////
F32 Audio::StereoToQuadMixer::Value( Instance* pInstance, U32 uOutput )
{
	State* pState = (State*)pInstance->GetStateData( m_uStateOffset );

	return pState->bActive ? 1.f : 0.f;
}

////////////////////////////////////////////////////////////////////////////////
//	AudioOutType
////////////////////////////////////////////////////////////////////////////////
const U32 Audio::StereoToQuadMixer::AudioOutType(Instance* pInstance) const
{
	return Core::eMixer;
}

////////////////////////////////////////////////////////////////////////////////
//	AudioOutType
////////////////////////////////////////////////////////////////////////////////
Audio::Com Audio::StereoToQuadMixer::GetInput( U8* pInitData, U32 uInput )
{
	InputGain* aInputs = (InputGain*)m_pObject->GetInitData( m_uInputOffset );
	return aInputs[ uInput ].sInputCom;
}

#if defined (AUDIO_ENABLE_PROFILING)
////////////////////////////////////////////////////////////////////////////////
//	GetNumInputs
////////////////////////////////////////////////////////////////////////////////
U16 Audio::StereoToQuadMixer::GetNumInputs()
{
	Data* pInitData = (Data*)m_pObject->GetInitData(m_uInitOffset);

	return pInitData->uInputs;
}

////////////////////////////////////////////////////////////////////////////////
//	GetInputCom
////////////////////////////////////////////////////////////////////////////////
Audio::Com Audio::StereoToQuadMixer::GetInputCom(U32 uIndex)
{
	InputGain* aInputs = (InputGain*)m_pObject->GetInitData(m_uInputOffset);

	return aInputs[uIndex].sInputCom;
}

////////////////////////////////////////////////////////////////////////////////
//	IsActive
////////////////////////////////////////////////////////////////////////////////
bool Audio::StereoToQuadMixer::IsActive(Instance* pInstance)
{
	State* pState = (State*)pInstance->GetStateData(m_uStateOffset);

	return pState->bActive;
}

////////////////////////////////////////////////////////////////////////////////
//	SetMute
////////////////////////////////////////////////////////////////////////////////
void Audio::StereoToQuadMixer::SetMute(Instance* pInstance, bool bMuted)
{
	State* pState = (State*)pInstance->GetStateData(m_uStateOffset);

	pState->bMuted = bMuted;
}
#endif	//	AUDIO_ENABLE_PROFILING

#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", on) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF

