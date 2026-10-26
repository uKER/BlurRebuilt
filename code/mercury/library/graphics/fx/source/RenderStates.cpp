#include "stdafx.h"

#include "fx/RenderStates.h"

namespace FX
{

S32 BlendStateManager::InsertRenderState(const BlendState& blendState)
{
	for (int i = 0; i < m_blendStates.Size(); i++)
	{
		if (blendState.srcBlend == m_blendStates[i].srcBlend
			&& blendState.destBlend == m_blendStates[i].destBlend
			&& blendState.blendOp == m_blendStates[i].blendOp
			&& blendState.alphaRef == m_blendStates[i].alphaRef
			&& blendState.alphaToMaskEnable == m_blendStates[i].alphaToMaskEnable
			&& blendState.cullMode == m_blendStates[i].cullMode
			&& blendState.alphaTestEnable == m_blendStates[i].alphaTestEnable)
			return i;
	}

	m_blendStates.PushBack(blendState);
	return m_blendStates.Size() - 1;
}

BlendStateManager ALIGN16 g_RenderStateMgr;

}
