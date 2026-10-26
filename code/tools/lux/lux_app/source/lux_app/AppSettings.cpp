#include "stdafx.h"
#include <lux_app/AppSettings.h>

CAppSettings::CAppSettings(void)
{
	m_RendererSettings.bRealtimeLighting = true;
	m_RendererSettings.bViewLightPaths = false;
	m_RendererSettings.bViewSamplePoints = false;
	m_RendererSettings.iLightmapType = SRD::SceneRendererSettings::LMType_FullColour;
}

CAppSettings::~CAppSettings(void)
{
}
