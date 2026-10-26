#include "AudioPch.h"
#ifdef PLATFORM_SPU
#include "SystemHeadersSPU.h"
#else
#include "../SystemHeaders.h"
#include "ComponentHelper.h"
#endif

//using namespace Audio;

#ifndef CFG_SPEED
#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", off)
#endif	//	AUDIO_OPTIMIZE_OFF
#endif

////////////////////////////////////////////////////////////////////////////////
//	Init
////////////////////////////////////////////////////////////////////////////////
#ifndef PLATFORM_SPU
void* Audio::Selector::Init(void* pData)
{
	Data* pInitData = PreInit<Data>(pData);

    Assert(pInitData->uNumElements >= 2);

	m_uElementOffset = m_uInitOffset + sizeof(Data);

	U32 uNext = m_uElementOffset + pInitData->uNumElements * sizeof(Element);

	return (void*)((U8*)m_pObject->GetInitData() + uNext);
}

////////////////////////////////////////////////////////////////////////////////
//	InitContext
////////////////////////////////////////////////////////////////////////////////
void Audio::Selector::InitContext(Instance* pInstance)
{
	Data* pInitData = (Data*)m_pObject->GetInitData(m_uInitOffset);
	Element* aElements = (Element*)m_pObject->GetInitData(m_uElementOffset);
    State* pState = (State*)pInstance->GetStateData(m_uStateOffset);

    pState->fCurrent = 0.f;

	for (int i=0; i<AUDIO_SELECTOR_MAX_WEIGHTS; i++)
		pState->afWeights[i] = 0.f;

    pState->fTotal = 0.f;

	Assert(pInitData->uNumElements <= AUDIO_SELECTOR_MAX_WEIGHTS);

	//	reset all weights
    for (U32 uIndex = 0; uIndex < pInitData->uNumElements; uIndex++)
    {
	    //	set default weights
	    if (aElements[uIndex].optWeight.ctrl.sCom != -1)
		    pState->afWeights[uIndex] = m_pObject->GetComponentValue(aElements[uIndex].optWeight.ctrl.sCom, pInstance, aElements[uIndex].optWeight.ctrl.uOut);
	    else
		    pState->afWeights[uIndex] = aElements[uIndex].optWeight.fVal;
    }
}

////////////////////////////////////////////////////////////////////////////////
//	ReleaseContext
////////////////////////////////////////////////////////////////////////////////
void Audio::Selector::ReleaseContext(Instance* pInstance)
{
	U8* pStateData = pInstance->GetStateData();
    Selector::State* pState = (Selector::State*)&pStateData[m_uStateOffset];
}
#endif	//	#ifndef PLATFORM_SPU

////////////////////////////////////////////////////////////////////////////////
//	Value
////////////////////////////////////////////////////////////////////////////////
F32 Audio::Selector::Value(Instance* pInstance, U32 uOutput)
{
    State* pState = (State*)pInstance->GetStateData(m_uStateOffset);

    switch(uOutput)
    {
    case ValueCtrl:
		return pState->fCurrent;
        break;

    case WeightCtrl:
		return pState->fTotal;
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
void Audio::Selector::DoMessage(Instance* pInstance, U32 uMessage)
{
	switch (uMessage)
    {
    case SelectMsg:
        Select(pInstance);
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
void Audio::Selector::Select(Instance* pInstance)
{
	U32 uIndex;

	Data* pInitData = (Data*)m_pObject->GetInitData(m_uInitOffset);
	Element* aElements = (Element*)m_pObject->GetInitData(m_uElementOffset);
    State* pState = (State*)pInstance->GetNewStateData(m_uStateOffset);

	Assert(pInitData->uNumElements > 1 && "Not enough elements in audio selector");

	//	sum current weights
	pState->fTotal = 0.f;
    for (U32 uCount = 0; uCount < pInitData->uNumElements; uCount++)
	    pState->fTotal += pState->afWeights[uCount];
    
	//	get reset level
	F32 fReset = GetOptionValue( pInstance, pInitData->optReset );

	//	clamp
	if (fReset < 0.f)
		fReset = 0.f;

	F32 fCompare = pState->fTotal;

	//	need to reset?
	if (fCompare <= fReset)
	{
		fCompare = 0.f;

		//	reset all weights
		Reset(pInstance);
	}

	//	pick a random weight band
	fCompare *= System::Get()->GetRandom();

	//	get element index for weight
    for (uIndex = 0; uIndex < pInitData->uNumElements; uIndex++)
    {
	    if (fCompare <= pState->afWeights[uIndex])
		    break;
	    else
		    fCompare -= pState->afWeights[uIndex];
    }
    
	Assert(uIndex < pInitData->uNumElements);

	//	get new value
	pState->fCurrent = GetOptionValue( pInstance, aElements[uIndex].optElement );

	//	get scale factor for selected elements
	F32 fScale = GetOptionValue( pInstance, aElements[uIndex].optScale );

	//	rescale element weight
    pState->afWeights[uIndex] *= fScale;

	//	add offset to all other weights
    for (U32 i = (uIndex + 1) % pInitData->uNumElements; i != uIndex; i = (i + 1) % pInitData->uNumElements)
		pState->afWeights[i] += GetOptionValue( pInstance, aElements[uIndex].optOffset );
}

////////////////////////////////////////////////////////////////////////////////
//	Reset
////////////////////////////////////////////////////////////////////////////////
void Audio::Selector::Reset(Instance* pInstance)
{
	Data* pInitData = (Data*)m_pObject->GetInitData(m_uInitOffset);
	Element* aElements = (Element*)m_pObject->GetInitData(m_uElementOffset);
    State* pState = (State*)pInstance->GetNewStateData(m_uStateOffset);

	//	reset all weights
    for (U32 uIndex = 0; uIndex < pInitData->uNumElements; uIndex++)
    {
	    //	set default weights
	    if (aElements[uIndex].optWeight.ctrl.sCom != -1)
		    pState->afWeights[uIndex] = m_pObject->GetComponentValue(aElements[uIndex].optWeight.ctrl.sCom, pInstance, aElements[uIndex].optWeight.ctrl.uOut);
	    else
		    pState->afWeights[uIndex] = aElements[uIndex].optWeight.fVal;
    }
}

#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", on) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF
