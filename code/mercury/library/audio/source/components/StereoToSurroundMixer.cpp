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
void* Audio::StereoToSurroundMixer::Init(void* pData)
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
U32 Audio::StereoToSurroundMixer::CoreMemory(U32 uOffset)
{
	Data* pInitData = (Data*)m_pObject->GetInitData(m_uInitOffset);

	m_uMixer = uOffset;

	//  mixer
	uOffset += Core::Mixer::MixerSize( pInitData->uInputs, 2, 5, GetNumDSPSlots() );

	return uOffset;
}

////////////////////////////////////////////////////////////////////////////////
//	InitContext
////////////////////////////////////////////////////////////////////////////////
void Audio::StereoToSurroundMixer::InitContext(Instance* pInstance)
{
	Data* pInitData = (Data*)m_pObject->GetInitData( m_uInitOffset );
    State* pState = (State*)pInstance->GetStateData( m_uStateOffset );
    Core::Mixer* pMixer = (Core::Mixer*)pInstance->GetCoreData( m_uMixer );

    //  create mixer
    new( pMixer ) Core::Mixer( Name(), pInitData->uInputs, 2, 5, GetNumDSPSlots(), false );

	pState->fOutputGain = -1.f;

	for ( U32 uChIn = 0; uChIn < 2; uChIn++ )
	{
		pState->afAzimuth[ uChIn ] = -1.f;
		pState->afElevation[ uChIn ] = -1.f;
		pState->afWidth[ uChIn ] = -1.f;
		pState->afDistance[ uChIn ] = -1.f;

		pState->asVolumes[ uChIn ].fCentre = 0.f;
		pState->asVolumes[ uChIn ].fFrontLeft = 0.f;
		pState->asVolumes[ uChIn ].fFrontRight = 0.f;
		pState->asVolumes[ uChIn ].fSurroundLeft = 0.f;
		pState->asVolumes[ uChIn ].fSurroundRight = 0.f;
		pState->asVolumes[ uChIn ].fBackLeft = 0.f;
		pState->asVolumes[ uChIn ].fBackRight = 0.f;
	}

	pState->bActive = false;
	pState->bMuted = false;
}

////////////////////////////////////////////////////////////////////////////////
//	ReleaseContext
////////////////////////////////////////////////////////////////////////////////
void Audio::StereoToSurroundMixer::ReleaseContext(Instance* pInstance)
{
	U8* pCoreData = pInstance->GetCoreData();
    Core::Mixer* pMixer = (Core::Mixer*)&pCoreData[m_uMixer];

	pMixer->~Mixer();
}
#endif	//	#ifndef PLATFORM_SPU

////////////////////////////////////////////////////////////////////////////////
//	Update
////////////////////////////////////////////////////////////////////////////////
void Audio::StereoToSurroundMixer::Update(Instance* pInstance)
{
    AUDIO_PERF_MICRO_EVENT("Audio.Update.StereoToSurroundMixer");

	Data* pInitData = (Data*)m_pObject->GetInitData( m_uInitOffset );
	InputGain* aInputs = (InputGain*)m_pObject->GetInitData( m_uInputOffset );
    State* pState = (State*)pInstance->GetNewStateData( m_uStateOffset );
    Core::Mixer* pMixer = (Core::Mixer*)pInstance->GetCoreData( m_uMixer );

	//	set if active
	pState->bActive = pMixer->IsActive();

	//  get output gain
	F32 fOutputGain = pState->bMuted ? 0.f : GetLevel( pInstance, pInitData->optLevel, pInitData->optGain );
	bool bOutputSilent = Maths::IsVolumeLessThan( fOutputGain, SoundComponent::iMinVolume );

	if ( !bOutputSilent )
	{
		F32 afAzimuth[ 2 ];
		F32 afElevation[ 2 ];
		F32 afDistance[ 2 ];
		F32 afWidth[ 2 ];

		//  get azimuth
		afAzimuth[ 0 ] = GetOptionValue( pInstance, pInitData->optLeftAzimuth );
		afAzimuth[ 1 ] = GetOptionValue( pInstance, pInitData->optRightAzimuth );

		//  get elevation
		afElevation[ 0 ] = GetOptionValue( pInstance, pInitData->optLeftElevation );
		afElevation[ 1 ] = GetOptionValue( pInstance, pInitData->optRightElevation );

		//  get arc width
		afWidth[ 0 ] = GetOptionValue( pInstance, pInitData->optLeftWidth );
		afWidth[ 1 ] = GetOptionValue( pInstance, pInitData->optRightWidth );

		//  get distance
		afDistance[ 0 ] = GetOptionValue( pInstance, pInitData->optLeftDistance );
		afDistance[ 1 ] = GetOptionValue( pInstance, pInitData->optRightDistance );

		for( U32 uChIn = 0; uChIn < 2; uChIn++ )
		{
			//  recalculate levels, if changed
			if( pState->fOutputGain != fOutputGain ||
				pState->afAzimuth[ uChIn ] != afAzimuth[ uChIn ] ||
				pState->afElevation[ uChIn ] != afElevation[ uChIn ] ||
				pState->afWidth[ uChIn ] != afWidth[ uChIn ] ||
				pState->afDistance[ uChIn ] != afDistance[ uChIn ] ||
				System::Get()->AnglesChanged() )
			{
				pState->afAzimuth[ uChIn ] = afAzimuth[ uChIn ];
				pState->afElevation[ uChIn ] = afElevation[ uChIn ];
				pState->afWidth[ uChIn ] = afWidth[ uChIn ];
				pState->afDistance[ uChIn ] = afDistance[ uChIn ];

				GetSurroundVolumes( System::Get()->GetPanningMode(), (SurroundMode)pInitData->bfFlags.nSurroundMode, afAzimuth[ uChIn ], afElevation[ uChIn ], afDistance[ uChIn ], afWidth[ uChIn ], fOutputGain, pState->asVolumes[ uChIn ] );
			}
		}
	}

	//	update output gain
	pState->fOutputGain = fOutputGain;

	//  do each input
	for( U32 uInput = 0; uInput < pInitData->uInputs; uInput++ )
	{
		if( !bOutputSilent )
		{
			//  get input gain
			F32 fLeftInputGain = GetLevel( pInstance, aInputs[uInput].optLeftLevel, aInputs[uInput].optLeftGain );
			F32 fRightInputGain = GetLevel( pInstance, aInputs[uInput].optRightLevel, aInputs[uInput].optRightGain );

			//	set left gains
			pMixer->SetGain( uInput, 0, 0, fLeftInputGain * pState->asVolumes[ 0 ].fFrontLeft );
			pMixer->SetGain( uInput, 0, 1, fLeftInputGain * pState->asVolumes[ 0 ].fFrontRight );
			pMixer->SetGain( uInput, 0, 2, fLeftInputGain * pState->asVolumes[ 0 ].fCentre );
			pMixer->SetGain( uInput, 0, 3, fLeftInputGain * pState->asVolumes[ 0 ].fSurroundLeft );
			pMixer->SetGain( uInput, 0, 4, fLeftInputGain * pState->asVolumes[ 0 ].fSurroundRight );

			pMixer->SetGain( uInput, 1, 0, fRightInputGain * pState->asVolumes[ 1 ].fFrontLeft );
			pMixer->SetGain( uInput, 1, 1, fRightInputGain * pState->asVolumes[ 1 ].fFrontRight );
			pMixer->SetGain( uInput, 1, 2, fRightInputGain * pState->asVolumes[ 1 ].fCentre );
			pMixer->SetGain( uInput, 1, 3, fRightInputGain * pState->asVolumes[ 1 ].fSurroundLeft );
			pMixer->SetGain( uInput, 1, 4, fRightInputGain * pState->asVolumes[ 1 ].fSurroundRight );
		}
		else
		{
			//	zero gains
			for( U32 uChIn = 0; uChIn < 2; uChIn++ )
			{
				pMixer->SetGain( uInput, uChIn, 0, 0.f ); // L
				pMixer->SetGain( uInput, uChIn, 1, 0.f ); // R
				pMixer->SetGain( uInput, uChIn, 2, 0.f ); // C
				pMixer->SetGain( uInput, uChIn, 3, 0.f ); // SL
				pMixer->SetGain( uInput, uChIn, 4, 0.f ); // SR
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
//	Value
////////////////////////////////////////////////////////////////////////////////
F32 Audio::StereoToSurroundMixer::Value( Instance* pInstance, U32 uOutput )
{
	State* pState = (State*)pInstance->GetStateData( m_uStateOffset );

	return pState->bActive ? 1.f : 0.f;
}

////////////////////////////////////////////////////////////////////////////////
//	AudioOutType
////////////////////////////////////////////////////////////////////////////////
const U32 Audio::StereoToSurroundMixer::AudioOutType(Instance* pInstance) const
{
	return Core::eMixer;
}

////////////////////////////////////////////////////////////////////////////////
//	AudioOutType
////////////////////////////////////////////////////////////////////////////////
Audio::Com Audio::StereoToSurroundMixer::GetInput( U8* pInitData, U32 uInput )
{
	InputGain* aInputs = (InputGain*)m_pObject->GetInitData( m_uInputOffset );
	return aInputs[ uInput ].sInputCom;
}

#if defined (AUDIO_ENABLE_PROFILING)
////////////////////////////////////////////////////////////////////////////////
//	GetNumInputs
////////////////////////////////////////////////////////////////////////////////
U16 Audio::StereoToSurroundMixer::GetNumInputs()
{
	Data* pInitData = (Data*)m_pObject->GetInitData(m_uInitOffset);

	return pInitData->uInputs;
}

////////////////////////////////////////////////////////////////////////////////
//	GetInputCom
////////////////////////////////////////////////////////////////////////////////
Audio::Com Audio::StereoToSurroundMixer::GetInputCom(U32 uIndex)
{
	InputGain* aInputs = (InputGain*)m_pObject->GetInitData(m_uInputOffset);

	return aInputs[uIndex].sInputCom;
}

////////////////////////////////////////////////////////////////////////////////
//	IsActive
////////////////////////////////////////////////////////////////////////////////
bool Audio::StereoToSurroundMixer::IsActive(Instance* pInstance)
{
	State* pState = (State*)pInstance->GetStateData(m_uStateOffset);

	return pState->bActive;
}

////////////////////////////////////////////////////////////////////////////////
//	SetMute
////////////////////////////////////////////////////////////////////////////////
void Audio::StereoToSurroundMixer::SetMute(Instance* pInstance, bool bMuted)
{
	State* pState = (State*)pInstance->GetStateData(m_uStateOffset);

	pState->bMuted = bMuted;
}
#endif	//	AUDIO_ENABLE_PROFILING

#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", on) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF
