#include "stdafx.h"

#include "SceneGraph/RenderPass.h"
#include "Scenegraph/RenderStates.h"
#include "Scenegraph/Visible/ListSorter.h"

namespace SceneGraph
{

RenderStates::RenderStates()
{
	// alpha states etc
	colourWriteEnable = (Display::E_ColourWriteEnable)(((int)Display::E_ColourWriteEnable_Red) | 
																			((int)Display::E_ColourWriteEnable_Green) |
																			((int)Display::E_ColourWriteEnable_Blue));

	setBlendMode = false;
	setDepth = false;
	setPixelShader = false;
	pPixelShader = 0;
}

RenderStates::~RenderStates()
{
}

C_RenderPass::C_RenderPass() :
	m_Flags(0),
	m_Index(0),
	m_VolatileInfoFlags(0),
	m_DrawOrder(E_DrawOrder_PrimitivesInstances),
	m_RenderingDataPass(0),
	m_Subtechnique(0),
	m_SubtechniqueMask(0xFFFFFFFF),
	m_pRenderStates(0),
	m_pSorter(0)
{
	
}

C_RenderPass::~C_RenderPass()
{
}

void C_RenderPass::SetSorter(I_VisibleNodeListSorter* pSorter)
{
	if(pSorter)
		m_VolatileInfoFlags = pSorter->GetVolatileInfoFlags();
	else
		m_VolatileInfoFlags = 0;
	m_pSorter = pSorter;
}

} // namespace SceneGraph

