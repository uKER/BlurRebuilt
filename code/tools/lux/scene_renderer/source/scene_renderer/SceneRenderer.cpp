#include "stdafx.h"

#include <scene_renderer/Common.h>
#include <scene_renderer/SinCos.h>
#include <scene_renderer/D3dWindow.h>

#include <scene_renderer/VertexBuffer.h>

#include <scene_renderer/Camera.h>

#include <scene_renderer/SceneRenderer.h>
#include <scene_renderer/VertexFormats.h>

using namespace SRD;

SceneRenderer::SceneRenderer(
		D3dWindow* pD3dWindow,
		const Aftk::C_Scene* pAftkScene
	)
{
	m_pD3dWindow = pD3dWindow;
	m_pScene = pAftkScene;
	m_vertexBuffersBuiltFlag = false;
	m_pCamera = 0;
}

SceneRenderer::~SceneRenderer()
{
}

void SceneRenderer::SetCameraPtr(const Camera* pCamera)
{
	m_pCamera = pCamera;
}

#ifndef DOXYGEN_SHOULD_SKIP_THIS

void SceneRenderer::Reset()
{
	m_pD3dWindow = 0;
	m_pScene = 0;
	m_vertexBuffersBuiltFlag = false;
	m_pCamera = 0;
}

#endif
