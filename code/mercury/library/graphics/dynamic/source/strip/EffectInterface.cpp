#include "stdafx.h"


#include "Dynamic/Renderer.h"
#include "Dynamic/Strip/EffectInterface.h"
#include "Dynamic/Strip/Manager.h"

namespace DynamicEffect
{
namespace Strip
{

I_StripEffectBase::I_StripEffectBase(C_Renderer* pRenderer, C_Manager* pManager, const U32 iIndex, const U32 iTypeIndex, const U32 iElementSize) : I_DynamicEffect(pRenderer, pManager, iIndex, iTypeIndex), m_pManager(pManager), m_iElementSize(iElementSize)
{
	pRenderer->AddEffect(this);
}

I_StripEffectBase::~I_StripEffectBase()
{
	m_pRenderer->RemoveEffect(this);
}

}
}

