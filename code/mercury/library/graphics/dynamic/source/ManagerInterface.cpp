#include "stdafx.h"

#include "Dynamic\ManagerInterface.h"
#include "Dynamic\Renderer.h"

using namespace DynamicEffect;

I_DynamicEffectManager::I_DynamicEffectManager(C_Renderer* pRenderer, const U32 iTypeIndex)
	: I_GatherNode(0, NULL),
	m_pRenderer(pRenderer),
	m_iTypeIndex(iTypeIndex)
{
	m_pRenderer->AddManager(this);
}

I_DynamicEffectManager::~I_DynamicEffectManager()
{
	m_pRenderer->RemoveManager(this);
}





