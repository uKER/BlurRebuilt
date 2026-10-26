#ifndef SCENE_GRAPH_VIEWDATA_H
#define SCENE_GRAPH_VIEWDATA_H

#ifdef _MSC_VER
#pragma once
#endif

#include <display/view/ViewFrustum.h>
#include <display/view/ViewSettings.h>

namespace Display
{
	struct C_TileSetup;
}

namespace SceneGraph
{

/// List of potential cull modes that can be used when determining
/// a list of visible objects for this view..
enum E_CullMethod
{
	E_CullMethod_None		= 0x0,
	E_CullMethod_Frustum	= 0x00000001,
	E_CullMethod_Portal		= 0x00000002,
	E_CullMethod_Pvs		= 0x00000004,
	E_CullMethod_All		= 0xffffffff
};

struct ALIGN16 LightSphere
{
	MathsStorage::Float4A16		unitSphere;
	MathsStorage::Float4A16		sphere;
	MathsStorage::Float4A16		depth;
};

struct ALIGN16 ViewData
{
	Display::C_ViewFrustum		viewFrustum;
	Display::C_ViewSettings3D	viewSettings3D;
	MathsStorage::Float4A16		forwardZ;
	MathsStorage::Float4A16		forwardZscaled;
	F32							farClip;
	F32							depthScale;
	F32							instanceDrawDistanceScale;
	F32							forwardMultiplier;
	F32							instanceDrawDistanceScaleRcp;
	F32							modelInstanceDrawDistanceScale;
	F32							modelInstanceDrawDistanceScaleRcp;
	U32							type;
	U32							typeFlag;
	const Display::C_TileSetup*	pTileSetup;
	U32							cullFlags;
	bool						distanceSubtechniqueEnable;
	F32							distanceSubtechniqueDistance;
	const LightSphere*			pLightSpheres;
	U32							lightSpheresCount;
};

} // namespace SceneGraph

#endif // SCENE_GRAPH_VIEWDATA_H
