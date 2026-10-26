#include "stdafx.h"

#include "Dynamic\Interface.h"
#include "Dynamic\Renderer.h"


#include "Dynamic\Shaders\Constants.h"

using namespace DynamicEffect;

I_DynamicEffect::I_DynamicEffect(C_Renderer* pRenderer, I_DynamicEffectManager* pManager, const U32 iIndex, const U32 iTypeIndex)
	: m_pRenderer(pRenderer),
	m_pManagerInterface(pManager),
	m_iIndex(iIndex),
	m_iTypeIndex(iTypeIndex),
	m_nBlocks(0),
	m_nBlocksLocal(0),
	m_bBegin(false),
	m_bAlwaysUpdate(false)
{
	if(!m_pManagerInterface)
		m_pRenderer->AddEffect(this);
}

I_DynamicEffect::~I_DynamicEffect()
{
	if(!m_pManagerInterface)
		m_pRenderer->RemoveEffect(this);
}

void I_DynamicEffect::SetWorldMatrix(const Display::C_Matrix* pMatrix) const
{
	Display::Device()->SetVertexShaderConstantF(CV_DISPLAY_WORLD_MATRIX, (F32*)pMatrix, 4);
	m_pRenderer->m_bIdentity = false;
}






