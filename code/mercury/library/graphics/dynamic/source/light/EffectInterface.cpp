#include "stdafx.h"

#include "Dynamic/Renderer.h"
#include "Dynamic/Light/EffectInterface.h"

namespace DynamicEffect
{
namespace Light
{

I_EffectBase::I_EffectBase(C_Renderer* pRenderer, I_DynamicEffectManager* pManager, const U32 iIndex, const U32 iTypeIndex)
	: I_DynamicEffect(pRenderer, pManager, iIndex, iTypeIndex)
{
	pRenderer->AddEffect(this);
}

I_EffectBase::~I_EffectBase()
{
	m_pRenderer->RemoveEffect(this);
}

void I_EffectBase::Destroy()
{
	if (m_RenderSettings.pTexture)
		m_RenderSettings.pTexture->Release();
}

}; // namespace Light
}; // namespace DynamicEffect
