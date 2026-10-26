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
void* Audio::MonoPhaser::Init(void* pData)
{
	Data* pInitData = PreInit<Data>(pData);
    return (void*)(pInitData + 1);
}

////////////////////////////////////////////////////////////////////////////////
//	CoreMemory
////////////////////////////////////////////////////////////////////////////////
U32 Audio::MonoPhaser::CoreMemory(U32 uOffset)
{
	m_uDSP = (ixCore)uOffset;

	uOffset += AUDIO_CORE_ALIGN_16(sizeof(Core::DSP::Phaser));

	return uOffset;
}

////////////////////////////////////////////////////////////////////////////////
//	InitContext
////////////////////////////////////////////////////////////////////////////////
void Audio::MonoPhaser::InitContext(Instance* pInstance)
{
    State* pState = (State*)pInstance->GetStateData(m_uStateOffset);

	new( pInstance->GetCoreData( m_uDSP ) ) Core::DSP::Phaser( Name(), 1 );

	pState->fDry = 1.0f;
    pState->fWet = -1.f;
	pState->fAngle = 0.f;
	pState->fSpeed = 100.f;
	pState->fWidth = 0.5f;
	pState->fFeedback = 0.0f;
}

////////////////////////////////////////////////////////////////////////////////
//	ReleaseContext
////////////////////////////////////////////////////////////////////////////////
void Audio::MonoPhaser::ReleaseContext(Instance* pInstance)
{
	Core::DSP::Phaser* pDSP = (Core::DSP::Phaser*)pInstance->GetCoreData( m_uDSP );

	delete pDSP;
}
#endif	//	#ifndef PLATFORM_SPU

////////////////////////////////////////////////////////////////////////////////
//	Update
////////////////////////////////////////////////////////////////////////////////
void Audio::MonoPhaser::Update(Instance* pInstance)
{
	AUDIO_PERF_MICRO_EVENT("Audio.Update.MonoPhaser");

	Data* pInitData = (Data*)m_pObject->GetInitData(m_uInitOffset);
    State* pState = (State*)pInstance->GetNewStateData(m_uStateOffset);
	Core::DSP::Phaser* pDSP = (Core::DSP::Phaser*)pInstance->GetCoreData( m_uDSP );

    //  get dry
	F32 fDry = GetOptionValue( pInstance, pInitData->optDry );

	//  set dry
	if (pState->fDry != fDry)
	{
		pState->fDry = fDry;
		pDSP->SetParam(Core::DSP::Phaser::eDry, fDry);
	}

	//  get cutoff
	F32 fWet = GetOptionValue( pInstance, pInitData->optWet );

	//  set wet
	if (pState->fWet != fWet)
	{
		pState->fWet = fWet;
		pDSP->SetParam(Core::DSP::Phaser::eWet, fWet);
	}

	//  get angle
	F32 fAngle = GetOptionValue( pInstance, pInitData->optAngle );

	//  set angle
	if (pState->fAngle != fAngle)
	{
		pState->fAngle = fAngle;
		pDSP->SetParam(Core::DSP::Phaser::eAngle, fAngle);
	}

	//  get speed
	F32 fSpeed = GetOptionValue( pInstance, pInitData->optSpeed );

	//  set blendRMS
	if (pState->fSpeed != fSpeed)
	{
		pState->fSpeed = fSpeed;
		pDSP->SetParam(Core::DSP::Phaser::eSpeed, fSpeed);
	}

	//  get width
	F32 fWidth = GetOptionValue( pInstance, pInitData->optWidth );

	//  set width
	if (pState->fWidth != fWidth)
	{
		pState->fWidth = fWidth;
		pDSP->SetParam(Core::DSP::Phaser::eWidth, fWidth);
	}

	//  get feedback
	F32 fFeedback = GetOptionValue( pInstance, pInitData->optFeedback );

	//  set feedback
	if (pState->fFeedback != fFeedback)
	{
		pState->fFeedback = fFeedback;
		pDSP->SetParam(Core::DSP::Phaser::eFeedback, fFeedback);
	}
}

////////////////////////////////////////////////////////////////////////////////
//	DoMessage
////////////////////////////////////////////////////////////////////////////////
void Audio::MonoPhaser::DoMessage( Instance* pInstance, U32 uMessage )
{
	Core::DSP::Effect* pDSP = (Core::DSP::Effect*)pInstance->GetCoreData( m_uDSP );

	switch( uMessage )
    {
    case EnableMsg:
		pDSP->SetActive( true );
        break;

    case BypassMsg:
		pDSP->SetActive( false );
        break;

    default:
        Assert(0);
        break;
    }
}

#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", on) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF
