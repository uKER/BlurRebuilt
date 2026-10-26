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
void* Audio::Metronome::Init(void* pData)
{
	Data* pInitData = PreInit<Data>(pData);
    return (void*)(pInitData + 1);
}

////////////////////////////////////////////////////////////////////////////////
//	InitContext
////////////////////////////////////////////////////////////////////////////////
void Audio::Metronome::InitContext(Instance* pInstance)
{
	Data* pInitData = (Data*)m_pObject->GetInitData(m_uInitOffset);
    State* pState = (State*)pInstance->GetStateData(m_uStateOffset);

    pState->bActive = false;

    pState->fFraction = 0.f;
    pState->fBeat = 0.f;
    pState->fBar = 0.f;
}
#endif	//	#ifndef PLATFORM_SPU

////////////////////////////////////////////////////////////////////////////////
//	Update
////////////////////////////////////////////////////////////////////////////////
void Audio::Metronome::Update(Instance* pInstance)
{
	AUDIO_PERF_MICRO_EVENT("Audio.Update.Metronome");

	Data* pInitData = (Data*)m_pObject->GetInitData(m_uInitOffset);
    State* pState = (State*)pInstance->GetNewStateData(m_uStateOffset);

	//	if active
	if( pState->bActive )
	{
		//	get tempo
		F32 fTempo = GetOptionValue( pInstance, pInitData->optTempo );
		fTempo = Maths::Clamp( fTempo, 20.f, 600.f );

		pState->fFraction += ( fTempo / 60.f ) * Core::fFrameLength * Core::fRcpSampleRate * (F32)pInstance->m_coreInfo.uFrameInt;

		//	beat boundary
		if( pState->fFraction >= 1.f )
		{
			pState->fFraction -= 1.f;

			//	do beat event
			DoEvent(pInstance, pInitData->evBeat);

			pState->fBeat += 1.f;

			//	get beats per bar
			F32 fBeatsPerBar = GetOptionValue( pInstance, pInitData->optBeatsPerBar );
			fBeatsPerBar = Maths::Clamp( Maths::Floor( fBeatsPerBar ), 2.f, 64.f );

			//	bar boundary
			if( pState->fBeat >= fBeatsPerBar )
			{
				pState->fBeat -= fBeatsPerBar;
				pState->fBar += 1.f;

				//	do bar event
				DoEvent(pInstance, pInitData->evBar);
			}

			F32 fLength = GetOptionValue( pInstance, pInitData->optLength );
			fLength = Maths::Max( Maths::Floor( fLength ), 1.f );

			//	song complete
			if( pState->fBar >= fLength )
			{
				//	stop
				pState->bActive = false;

				//	do complete event
				DoEvent(pInstance, pInitData->evFinished);

				//	reset
				pState->fFraction = 0.f;
				pState->fBeat = 0.f;
				pState->fBar = 0.f;
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
//	Value
////////////////////////////////////////////////////////////////////////////////
F32 Audio::Metronome::Value(Instance* pInstance, U32 uOutput)
{
    State* pState = (State*)pInstance->GetStateData(m_uStateOffset);

    switch(uOutput)
    {
    case FractionCtrl:
		return pState->fFraction;
        break;
    case BeatCtrl:
		return 1.f + pState->fBeat;
        break;
	case BarCtrl:
		return 1.f + pState->fBar;
        break;
    default:
        Assert(0);
        break;
    }

    return 0.f;
}

////////////////////////////////////////////////////////////////////////////////
//	DoMessage
////////////////////////////////////////////////////////////////////////////////
void Audio::Metronome::DoMessage(Instance* pInstance, U32 uMessage)
{
	switch (uMessage)
    {
    case StartMsg:
        Start(pInstance);
        break;

    case StopMsg:
        Stop(pInstance);
        break;

    case ResetMsg:
        Reset(pInstance);
        break;

    default:
        Assert(0);
        break;
    }
}

////////////////////////////////////////////////////////////////////////////////
//	Start
////////////////////////////////////////////////////////////////////////////////
void Audio::Metronome::Start(Instance* pInstance)
{
    State* pState = (State*)pInstance->GetNewStateData(m_uStateOffset);

    pState->bActive = true;
}

////////////////////////////////////////////////////////////////////////////////
//	Stop
////////////////////////////////////////////////////////////////////////////////
void Audio::Metronome::Stop(Instance* pInstance)
{
    State* pState = (State*)pInstance->GetNewStateData(m_uStateOffset);

    pState->bActive = false;
}

////////////////////////////////////////////////////////////////////////////////
//	Reset
////////////////////////////////////////////////////////////////////////////////
void Audio::Metronome::Reset(Instance* pInstance)
{
	Data* pInitData = (Data*)m_pObject->GetInitData(m_uInitOffset);
    State* pState = (State*)pInstance->GetNewStateData(m_uStateOffset);

	pState->fFraction = 0.f;
    pState->fBeat = 0.f;
    pState->fBar = 0.f;
}

#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", on) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF
