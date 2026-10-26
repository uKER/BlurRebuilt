#pragma once

#include "scene_renderer/SceneRenderer.h"

class CAppSettings
{
public:

	SRD::SceneRendererSettings	m_RendererSettings;

	CAppSettings(void);
	~CAppSettings(void);
};
