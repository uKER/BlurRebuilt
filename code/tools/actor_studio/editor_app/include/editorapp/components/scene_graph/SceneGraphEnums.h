#ifndef EDITOR_SCENE_GRAPH_ENUMS_H
#define EDITOR_SCENE_GRAPH_ENUMS_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Editor
{
	enum E_SceneGraphDataType
	{
		E_SceneGraphDataType_Actor,
		E_SceneGraphDataType_SceneNode,
		E_SceneGraphDataType_Component,
		E_SceneGraphDataType_Property,
		E_SceneGraphDataType_Reference,
		E_SceneGraphDataType_AftkNode,
		E_SceneGraphDataType_Data,
		E_SceneGraphDataType_Level,
		E_SceneGraphDataType_Unknown
	};

	enum E_SceneGraphItemType
	{
		E_SceneGraphItemType_Generic,
		E_SceneGraphItemType_DgNode,
		E_SceneGraphItemType_Unknown
	};
}

#endif