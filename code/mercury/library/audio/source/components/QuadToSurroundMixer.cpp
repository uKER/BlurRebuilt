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
void* Audio::QuadToSurroundMixer::Init(void* pData)
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
U32 Audio::QuadToSurroundMixer::CoreMemory(U32 uOffset)
{
	Data* pInitData = (Data*)m_pObject->GetInitData(m_uInitOffset);

	m_uMixer = uOffset;

	//  mixer
	uOffset += Core::Mixer::MixerSize( pInitData->uInputs, 4, 5, GetNumDSPSlots() );

	return uOffset;
}

////////////////////////////////////////////////////////////////////////////////
//	InitContext
////////////////////////////////////////////////////////////////////////////////
void Audio::QuadToSurroundMixer::InitContext(Instance* pInstance)
{
	Data* pInitData = (Data*)m_pObject->GetInitData( m_uInitOffset );
    State* pState = (State*)pInstance->GetStateData( m_uStateOffset );
    Core::Mixer* pMixer = (Core::Mixer*)pInstance->GetCoreData( m_uMixer );

    //  create mixer
    new( pMixer ) Core::Mixer( Name(), pInitData->uInputs, 4, 5, GetNumDSPSlots(), false );

    pState->afAzimuth[ 0 ] = -0.25f;
    pState->afAzimuth[ 1 ] = 0.25f;
    pState->afAzimuth[ 2 ] = -0.75f;
    pState->afAzimuth[ 3 ] = 0.75f;

	for ( U32 uChIn = 0; uChIn < 4; uChIn++ )
	{
		pState->afElevation[ uChIn ] = 0.f;
		pState->afDistance[ uChIn ] = 5.f;
		pState->afWidth[ uChIn ] = -1.f;

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
void Audio::QuadToSurroundMixer::ReleaseContext(Instance* pInstance)
{
    Core::Mixer* pMixer = (Core::Mixer*)pInstance->GetCoreData( m_uMixer );

	pMixer->~Mixer();
}
#endif	//	#ifndef PLATFORM_SPU

////////////////////////////////////////////////////////////////////////////////
//	Update
////////////////////////////////////////////////////////////////////////////////
void Audio::QuadToSurroundMixer::Update(Instance* pInstance)
{
	AUDIO_PERF_MICRO_EVENT("Audio.Update.QuadToSurroundMixer");

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
		F32 afAzimuth[ 4 ];
		F32 afElevation[ 4 ];
		F32 afDistance[ 4 ];
		F32 afWidth[ 4 ];

		//  get azimuth
		afAzimuth[ 0 ] = GetOptionValue( pInstance, pInitData->optFrontLeftAzimuth );
		afAzimuth[ 1 ] = GetOptionValue( pInstance, pInitData->optFrontRightAzimuth );
		afAzimuth[ 2 ] = GetOptionValue( pInstance, pInitData->optRearLeftAzimuth );
		afAzimuth[ 3 ] = GetOptionValue( pInstance, pInitData->optRearRightAzimuth );

		//  get elevation
		afElevation[ 0 ] = GetOptionValue( pInstance, pInitData->optFrontLeftElevation );
		afElevation[ 1 ] = GetOptionValue( pInstance, pInitData->optFrontRightElevation );
		afElevation[ 2 ] = GetOptionValue( pInstance, pInitData->optRearLeftElevation );
		afElevation[ 3 ] = GetOptionValue( pInstance, pInitData->optRearRightElevation );

		//  get arc width
		afWidth[ 0 ] = GetOptionValue( pInstance, pInitData->optFrontLeftWidth );
		afWidth[ 1 ] = GetOptionValue( pInstance, pInitData->optFrontRightWidth );
		afWidth[ 2 ] = GetOptionValue( pInstance, pInitData->optRearLeftWidth );
		afWidth[ 3 ] = GetOptionValue( pInstance, pInitData->optRearRightWidth );

		//  get distance
		afDistance[ 0 ] = GetOptionValue( pInstance, pInitData->optFrontLeftDistance );
		afDistance[ 1 ] = GetOptionValue( pInstance, pInitData->optFrontRightDistance );
		afDistance[ 2 ] = GetOptionValue( pInstance, pInitData->optRearLeftDistance );
		afDistance[ 3 ] = GetOptionValue( pInstance, pInitData->optRearRightDistance );

		for( U32 uChIn = 0; uChIn < 4; uChIn++ )
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
			F32 fFrontLeftInputGain = GetLevel( pInstance, aInputs[ uInput ].optFrontLeftLevel, aInputs[ uInput ].optFrontLeftGain );
			F32 fFrontRightInputGain = GetLevel( pInstance, aInputs[ uInput ].optFrontRightLevel, aInputs[ uInput ].optFrontRightGain );
			F32 fRearLeftInputGain = GetLevel( pInstance, aInputs[ uInput ].optRearLeftLevel, aInputs[ uInput ].optRearLeftGain );
			F32 fRearRightInputGain = GetLevel( pInstance, aInputs[ uInput ].optRearRightLevel, aInputs[ uInput ].optRearRightGain );

			//	set FL gains
			pMixer->SetGain( uInput, 0, 0, fFrontLeftInputGain * pState->asVolumes[ 0 ].fFrontLeft );
			pMixer->SetGain( uInput, 0, 1, fFrontLeftInputGain * pState->asVolumes[ 0 ].fFrontRight );
			pMixer->SetGain( uInput, 0, 2, fFrontLeftInputGain * pState->asVolumes[ 0 ].fCentre );
			pMixer->SetGain( uInput, 0, 3, fFrontLeftInputGain * pState->asVolumes[ 0 ].fSurroundLeft );
			pMixer->SetGain( uInput, 0, 4, fFrontLeftInputGain * pState->asVolumes[ 0 ].fSurroundRight );

			//	set FR gains
			pMixer->SetGain( uInput, 1, 0, fFrontRightInputGain * pState->asVolumes[ 1 ].fFrontLeft );
			pMixer->SetGain( uInput, 1, 1, fFrontRightInputGain * pState->asVolumes[ 1 ].fFrontRight );
			pMixer->SetGain( uInput, 1, 2, fFrontRightInputGain * pState->asVolumes[ 1 ].fCentre );
			pMixer->SetGain( uInput, 1, 3, fFrontRightInputGain * pState->asVolumes[ 1 ].fSurroundLeft );
			pMixer->SetGain( uInput, 1, 4, fFrontRightInputGain * pState->asVolumes[ 1 ].fSurroundRight );

			//	set RL gains
			pMixer->SetGain( uInput, 2, 0, fRearLeftInputGain * pState->asVolumes[ 2 ].fFrontLeft );
			pMixer->SetGain( uInput, 2, 1, fRearLeftInputGain * pState->asVolumes[ 2 ].fFrontRight );
			pMixer->SetGain( uInput, 2, 2, fRearLeftInputGain * pState->asVolumes[ 2 ].fCentre );
			pMixer->SetGain( uInput, 2, 3, fRearLeftInputGain * pState->asVolumes[ 2 ].fSurroundLeft );
			pMixer->SetGain( uInput, 2, 4, fRearLeftInputGain * pState->asVolumes[ 2 ].fSurroundRight );

			//	set RR gains
			pMixer->SetGain( uInput, 3, 0, fRearRightInputGain * pState->asVolumes[ 3 ].fFrontLeft );
			pMixer->SetGain( uInput, 3, 1, fRearRightInputGain * pState->asVolumes[ 3 ].fFrontRight );
			pMixer->SetGain( uInput, 3, 2, fRearRightInputGain * pState->asVolumes[ 3 ].fCentre );
			pMixer->SetGain( uInput, 3, 3, fRearRightInputGain * pState->asVolumes[ 3 ].fSurroundLeft );
			pMixer->SetGain( uInput, 3, 4, fRearRightInputGain * pState->asVolumes[ 3 ].fSurroundRight );
		}
		else
		{
			//	zero gains
			for( U32 uChIn = 0; uChIn < 4; uChIn++ )
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
F32 Audio::QuadToSurroundMixer::Value( Instance* pInstance, U32 uOutput )
{
	State* pState = (State*)pInstance->GetStateData( m_uStateOffset );

	return pState->bActive ? 1.f : 0.f;
}

////////////////////////////////////////////////////////////////////////////////
//	AudioOutType
////////////////////////////////////////////////////////////////////////////////
const U32 Audio::QuadToSurroundMixer::AudioOutType(Instance* pInstance) const
{
	return Core::eMixer;
}

////////////////////////////////////////////////////////////////////////////////
//	AudioOutType
////////////////////////////////////////////////////////////////////////////////
Audio::Com Audio::QuadToSurroundMixer::GetInput( U8* pInitData, U32 uInput )
{
	InputGain* aInputs = (InputGain*)m_pObject->GetInitData( m_uInputOffset );
	return aInputs[ uInput ].sInputCom;
}

#if defined (AUDIO_ENABLE_PROFILING)
////////////////////////////////////////////////////////////////////////////////
//	GetNumInputs
////////////////////////////////////////////////////////////////////////////////
U16 Audio::QuadToSurroundMixer::GetNumInputs()
{
	Data* pInitData = (Data*)m_pObject->GetInitData(m_uInitOffset);

	return pInitData->uInputs;
}

////////////////////////////////////////////////////////////////////////////////
//	GetInputCom
////////////////////////////////////////////////////////////////////////////////
Audio::Com Audio::QuadToSurroundMixer::GetInputCom(U32 uIndex)
{
	InputGain* aInputs = (InputGain*)m_pObject->GetInitData(m_uInputOffset);

	return aInputs[uIndex].sInputCom;
}

////////////////////////////////////////////////////////////////////////////////
//	IsActive
////////////////////////////////////////////////////////////////////////////////
bool Audio::QuadToSurroundMixer::IsActive(Instance* pInstance)
{
	State* pState = (State*)pInstance->GetStateData(m_uStateOffset);

	return pState->bActive;
}

////////////////////////////////////////////////////////////////////////////////
//	SetMute
////////////////////////////////////////////////////////////////////////////////
void Audio::QuadToSurroundMixer::SetMute(Instance* pInstance, bool bMuted)
{
	State* pState = (State*)pInstance->GetStateData(m_uStateOffset);

	pState->bMuted = bMuted;
}
#endif	//	AUDIO_ENABLE_PROFILING

#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", on) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF
