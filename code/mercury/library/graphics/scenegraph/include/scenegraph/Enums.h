#ifndef SCENE_GRAPH_ENUMS_H
#define SCENE_GRAPH_ENUMS_H

#ifdef _MSC_VER
#pragma once
#endif

namespace SceneGraph
{

enum E_RenderPriority
{
	E_RenderPriority_Low = 0,
	E_RenderPriority_Medium,
	E_RenderPriority_High,

	E_RenderPriority_Count
};

enum E_VolatileNodeInfo
{
	E_VolatileNodeInfo_Distance		= 0x00000001,
	E_VolatileNodeInfo_ScreenSize	= 0x00000002,
	E_VolatileNodeInfo_Predication	= 0x00000004,
	E_VolatileNodeInfo_All			= 0xFFFFFFFF
};

} // namespace SceneGraph

#endif // SCENE_GRAPH_ENUMS_H
