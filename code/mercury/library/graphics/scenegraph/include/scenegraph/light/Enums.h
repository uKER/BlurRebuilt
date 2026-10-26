#ifndef SCENE_GRAPH_LIGHT_ENUMS_H
#define SCENE_GRAPH_LIGHT_ENUMS_H

#ifdef _MSC_VER
#pragma once
#endif

namespace SceneGraph
{

enum E_SpotLightType
{
	E_SpotLightType_Standard = 0,
	E_SpotLightType_Projected,
	E_SpotLightType_Volume,
	E_SpotLightType_Count
};

} // namespace SceneGraph

#endif // SCENE_GRAPH_LIGHT_ENUMS_H
