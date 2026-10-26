
#include "Scene Renderer/stdafx.h"

#include <Scene Renderer/Common.h>
#include <Scene Renderer/SinCos.h>
#include <Scene Renderer/D3dWindow.h>

#include <Scene Renderer/VertexBuffer.h>

#include <Scene Renderer/HierachyRenderer.h>

using namespace SRD;
using namespace SD;

HierachyRenderer::HierachyRenderer()
{
	Reset();
}

HierachyRenderer::HierachyRenderer(D3dWindow* pD3dWindow, const SD::SpatialHierachy* pSpatialHierachy)
{
	Reset();

	m_pD3dWindow = pD3dWindow;
	m_pSpatialHierachy = pSpatialHierachy;
}

HierachyRenderer::~HierachyRenderer()
{
	Clear();
}

void HierachyRenderer::SetD3dWindowPtr(D3dWindow* pD3dWindow)
{
	m_pD3dWindow = pD3dWindow;
}

void HierachyRenderer::SetSpatialHierachyPtr(const SpatialHierachy* pSpatialHierachy)
{
	m_pSpatialHierachy = pSpatialHierachy;
}

//void HierachyRenderer::OnUpdateDevice()
//{
	// do rendering
//}

void HierachyRenderer::Clear()
{
	Reset();
}

#ifndef DOXYGEN_SHOULD_SKIP_THIS

void HierachyRenderer::Reset()
{
	m_pD3dWindow = 0;
	m_pSpatialHierachy = 0;
}

#endif