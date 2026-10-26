
#include "stdafx.h"

#include <scene_renderer/Common.h>
#include <scene_renderer/SinCos.h>
#include <scene_renderer/D3dWindow.h>

#include <scene_renderer/VertexBuffer.h>

#include <spatial_hierarchy/spatialDividerHeaders.h>

#include <scene_renderer/HierachyRenderer.h>

using namespace SRD;
using namespace SD;

HierachyRenderer::HierachyRenderer()
{
	Reset();
}

HierachyRenderer::HierachyRenderer(D3dWindow* pD3dWindow, const SD::SpatialHierarchy* pSpatialHierarchy)
{
	Reset();

	m_pD3dWindow = pD3dWindow;
	m_pSpatialHierarchy = pSpatialHierarchy;
}

HierachyRenderer::~HierachyRenderer()
{
	Clear();
}

void HierachyRenderer::SetD3dWindowPtr(D3dWindow* pD3dWindow)
{
	m_pD3dWindow = pD3dWindow;
}

void HierachyRenderer::SetSpatialHierarchyPtr(const SpatialHierarchy* pSpatialHierarchy)
{
	m_pSpatialHierarchy = pSpatialHierarchy;
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
	m_pSpatialHierarchy = 0;
}

#endif