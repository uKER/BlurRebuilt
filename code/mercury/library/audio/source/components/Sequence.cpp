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
void* Audio::Sequence::Init(void* pData)
{
	Data* pInitData = PreInit<Data>(pData);

	m_uElementOffset = m_uInitOffset + sizeof(Data);

	U32 uNext = m_uElementOffset + pInitData->uNumElements * sizeof(Element);

    return (void*)((U8*)m_pObject->GetInitData() + uNext);
}

////////////////////////////////////////////////////////////////////////////////
//	InitContext
////////////////////////////////////////////////////////////////////////////////
void Audio::Sequence::InitContext(Instance* pInstance)
{
	Data* pInitData = (Data*)m_pObject->GetInitData(m_uInitOffset);
	State* pState = (State*)pInstance->GetStateData(m_uStateOffset);

    pState->uLastFrame = (U32)-1;
    pState->fCurrentIn = -1.f;
    pState->fCurrentOut = 0.f;
}
#endif	//	#ifndef PLATFORM_SPU

////////////////////////////////////////////////////////////////////////////////
//	Update
////////////////////////////////////////////////////////////////////////////////
void Audio::Sequence::Update(Instance* pInstance)
{
	AUDIO_PERF_MICRO_EVENT("Audio.Update.Sequence");

	Data* pInitData = (Data*)m_pObject->GetInitData(m_uInitOffset);
	Element* aElements = (Element*)m_pObject->GetInitData( m_uElementOffset );
	State* pState = (State*)pInstance->GetNewStateData(m_uStateOffset);

	F32 fPreviousIn = pState->fCurrentIn;

	//  get input
	pState->fCurrentIn = GetControlValue( pInstance, pInitData->ctrlInput );

	//	for each element
	for ( U32 uCount = 0; uCount < pInitData->uNumElements; uCount++ )
	{
		//	has been triggered?
		if ( fPreviousIn < aElements[ uCount ].fTime && pState->fCurrentIn >= aElements[ uCount ].fTime )
		{
			//	set output val - if multiple elements are triggered, output will be newest
			pState->fCurrentOut = GetOptionValue( pInstance, aElements[ uCount ].optValue );

			//	fire event
			DoEvent( pInstance, aElements[ uCount ].evEvent );
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
//	Value
////////////////////////////////////////////////////////////////////////////////
F32 Audio::Sequence::Value(Instance* pInstance, U32 uOutput)
{
	State* pState = (State*)pInstance->GetStateData(m_uStateOffset);

	switch(uOutput)
    {
    case ValueCtrl:
        return pState->fCurrentOut;
        break;
    default:
        Assert(0);
        break;
    }

    return 0.f;
}

#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", on) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF
