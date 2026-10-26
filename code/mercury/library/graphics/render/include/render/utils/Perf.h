#ifndef RENDER_PERF_H
#define RENDER_PERF_H

#ifdef _MSC_VER
#pragma once
#endif

enum E_RenderQuality
{
	E_RenderQuality_NormalMaps		= 1,
	E_RenderQuality_Highlights		= 2,
	E_RenderQuality_CubeMaps		= 3,
	E_RenderQuality_ShadowMaps		= 4,
	E_RenderQuality_PointLights_1	= 5,
	E_RenderQuality_PointLights_4	= 6,
};

#endif // RENDER_PERF_H
