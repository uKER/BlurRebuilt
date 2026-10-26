#ifndef AMAX_GRAPHICS_GPU_MARKERS_H
#define AMAX_GRAPHICS_GPU_MARKERS_H

#if _MSC_VER > 1000
#pragma once
#endif

enum E_GpuGroupMarker
{
	E_GpuGroupMarker_Unknown						= 0,
	E_GpuGroupMarker_CubeMap						= 1,
	E_GpuGroupMarker_BlobShadowsPreRender			= 2,
	E_GpuGroupMarker_Reflection						= 3,
	E_GpuGroupMarker_Mirror							= 4,
	E_GpuGroupMarker_Prepass						= 5,
	E_GpuGroupMarker_LightFlares					= 6,
	E_GpuGroupMarker_Refract						= 7,
	E_GpuGroupMarker_DropShadows					= 8,
	E_GpuGroupMarker_BlobShadowsScreenRender		= 9,
	E_GpuGroupMarker_MainViewAlpha					= 10,
	E_GpuGroupMarker_MainViewTwoDimensional			= 11,
	E_GpuGroupMarker_MainViewEffectDecal			= 12,
	E_GpuGroupMarker_MainViewGroup_Main				= 13,
	E_GpuGroupMarker_BeginRender					= 14,
	E_GpuGroupMarker_CommonRender					= 15,
	E_GpuGroupMarker_EndRender						= 16,
};

#endif // AMAX_GRAPHICS_GPU_MARKERS_H
