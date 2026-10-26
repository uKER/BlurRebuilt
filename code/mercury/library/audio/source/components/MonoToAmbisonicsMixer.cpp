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
void* Audio::MonoToAmbisonicsMixer::Init(void* pData)
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
U32 Audio::MonoToAmbisonicsMixer::CoreMemory(U32 uOffset)
{
	Data* pInitData = (Data*)m_pObject->GetInitData(m_uInitOffset);

	m_uMixer = (ixCore)uOffset;

    //  mixer
#if defined (AMBISONICS_1ST_ORDER)
	uOffset += Core::Mixer::MixerSize( pInitData->uInputs, 1, 3, GetNumDSPSlots() );
#endif
#if defined (AMBISONICS_2ND_ORDER)
	uOffset += Core::Mixer::MixerSize( pInitData->uInputs, 1, 5, GetNumDSPSlots() );
#endif
#if defined (AMBISONICS_3RD_ORDER)
	uOffset += Core::Mixer::MixerSize( pInitData->uInputs, 1, 8, GetNumDSPSlots() );
#endif

	return uOffset;
}

////////////////////////////////////////////////////////////////////////////////
//	InitContext
////////////////////////////////////////////////////////////////////////////////
void Audio::MonoToAmbisonicsMixer::InitContext(Instance* pInstance)
{
	Data* pInitData = (Data*)m_pObject->GetInitData(m_uInitOffset);
    State* pState = (State*)pInstance->GetStateData(m_uStateOffset);
    Core::Mixer* pMixer = (Core::Mixer*)pInstance->GetCoreData( m_uMixer );

    //  create mixer
#if defined (AMBISONICS_1ST_ORDER)
    new( pMixer ) Core::Mixer( Name(), pInitData->uInputs, 1, 3, GetNumDSPSlots(), false );
#endif
#if defined (AMBISONICS_2ND_ORDER)
    new( pMixer ) Core::Mixer( Name(), pInitData->uInputs, 1, 5, GetNumDSPSlots(), false );
#endif
#if defined (AMBISONICS_3RD_ORDER)
    new( pMixer ) Core::Mixer( Name(), pInitData->uInputs, 1, 8, GetNumDSPSlots(), false );
#endif

    pState->fOutputGain = -1.f;
    pState->fAzimuth = 0.f;
    pState->fElevation = 0.f;

	pState->bActive = false;
	pState->bMuted = false;
}

////////////////////////////////////////////////////////////////////////////////
//	ReleaseContext
////////////////////////////////////////////////////////////////////////////////
void Audio::MonoToAmbisonicsMixer::ReleaseContext(Instance* pInstance)
{
    Core::Mixer* pMixer = (Core::Mixer*)pInstance->GetCoreData( m_uMixer );

	pMixer->~Mixer();
}
#endif	//	#ifndef PLATFORM_SPU

////////////////////////////////////////////////////////////////////////////////
//	Update
////////////////////////////////////////////////////////////////////////////////
void Audio::MonoToAmbisonicsMixer::Update(Instance* pInstance)
{
	AUDIO_PERF_MICRO_EVENT("Audio.Update.MonoToAmbisonicsMixer");

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

		//  recalculate levels, if changed
		if( pState->fOutputGain != fOutputGain ||
			pState->fAzimuth != fAzimuth ||
			pState->fElevation != fElevation ||
			System::Get()->AnglesChanged() )
		{
			pState->fOutputGain = fOutputGain;
			pState->fAzimuth = fAzimuth;
			pState->fElevation = fElevation;

			F32 fAzimuthRad = fAzimuth * Maths::fPi;
			F32 fElevationRad = fElevation * Maths::fPi;

#if defined (AMBISONICS_1ST_ORDER) || defined (AMBISONICS_2ND_ORDER) || defined (AMBISONICS_3RD_ORDER)
			pState->afEncoder[ 0 ] = Maths::fMinus3dB;											//	W
			pState->afEncoder[ 1 ] = Maths::Cos( fAzimuthRad ) * Maths::Cos( fElevationRad );	//	X
			pState->afEncoder[ 2 ] = Maths::Sin( fAzimuthRad ) * Maths::Cos( fElevationRad );	//	Y
#endif
#if defined (AMBISONICS_2ND_ORDER) || defined (AMBISONICS_3RD_ORDER)
			F32 fCosElevation2 = Maths::Cos( fElevationRad );
			F32 f2Azimuth = 2.f * fAzimuthRad;
			fCosElevation2 *= fCosElevation2;
			pState->afEncoder[ 3 ] = 1.1547f * Maths::Cos( f2Azimuth ) * fCosElevation2;		//	U
			pState->afEncoder[ 4 ] = 1.1547f * Maths::Sin( f2Azimuth ) * fCosElevation2;		//	V
#endif
#if defined (AMBISONICS_3RD_ORDER)
			//F32 fCosElevation2 = Maths::Cos( fElevationRad );
			//fCosElevation2 *= fCosElevation2;
			//pState->afEncoder[ 5 ] = Maths::Cos( 2.f * fAzimuthRad ) * fCosElevation2;			//	U
			//pState->afEncoder[ 6 ] = Maths::Sin( 2.f * fAzimuthRad ) * fCosElevation2;			//	V
			//pState->afEncoder[ 7] = Maths::Sin( 2.f * fAzimuthRad ) * fCosElevation2;			//	V
#endif

			//printf("(a=%.3f, e=%.3f) W=%.3f X=%.3f Y=%.3f \n", fAzimuth, fElevation, pState->afEncoder[ 0 ], pState->afEncoder[ 1 ], pState->afEncoder[ 2 ]);
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
			pMixer->SetGain( uInput, 0, 0, fOutputGain * fInputGain * pState->afEncoder[ 0 ] );
			pMixer->SetGain( uInput, 0, 1, fOutputGain * fInputGain * pState->afEncoder[ 1 ] );
			pMixer->SetGain( uInput, 0, 2, fOutputGain * fInputGain * pState->afEncoder[ 2 ] );
		}
		else
		{
			//	zero gains
			pMixer->SetGain( uInput, 0, 0, 0.f );
			pMixer->SetGain( uInput, 0, 1, 0.f );
			pMixer->SetGain( uInput, 0, 2, 0.f );
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
//	AudioOutType
////////////////////////////////////////////////////////////////////////////////
const U32 Audio::MonoToAmbisonicsMixer::AudioOutType(Instance* pInstance) const
{
	return Core::eMixer;
}

////////////////////////////////////////////////////////////////////////////////
//	AudioOutType
////////////////////////////////////////////////////////////////////////////////
Audio::Com Audio::MonoToAmbisonicsMixer::GetInput( U8* pInitData, U32 uInput )
{
	InputGain* aInputs = (InputGain*)m_pObject->GetInitData( m_uInputOffset );
	return aInputs[ uInput ].sInputCom;
}

#if defined (AUDIO_ENABLE_PROFILING)
////////////////////////////////////////////////////////////////////////////////
//	GetNumInputs
////////////////////////////////////////////////////////////////////////////////
U16 Audio::MonoToAmbisonicsMixer::GetNumInputs()
{
	Data* pInitData = (Data*)m_pObject->GetInitData(m_uInitOffset);

	return pInitData->uInputs;
}

////////////////////////////////////////////////////////////////////////////////
//	GetInputCom
////////////////////////////////////////////////////////////////////////////////
Audio::Com Audio::MonoToAmbisonicsMixer::GetInputCom(U32 uIndex)
{
	InputGain* aInputs = (InputGain*)m_pObject->GetInitData(m_uInputOffset);

	return aInputs[uIndex].sInputCom;
}

////////////////////////////////////////////////////////////////////////////////
//	IsActive
////////////////////////////////////////////////////////////////////////////////
bool Audio::MonoToAmbisonicsMixer::IsActive(Instance* pInstance)
{
	State* pState = (State*)pInstance->GetStateData(m_uStateOffset);

	return pState->bActive;
}

////////////////////////////////////////////////////////////////////////////////
//	SetMute
////////////////////////////////////////////////////////////////////////////////
void Audio::MonoToAmbisonicsMixer::SetMute(Instance* pInstance, bool bMuted)
{
	State* pState = (State*)pInstance->GetStateData(m_uStateOffset);

	pState->bMuted = bMuted;
}
#endif	//	AUDIO_ENABLE_PROFILING

#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", on) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF

