#ifndef SCENE_GRAPH_VOLATILE_NODE_INFO_H
#define SCENE_GRAPH_VOLATILE_NODE_INFO_H

#ifdef _MSC_VER
#pragma once
#endif

namespace SceneGraph
{

struct VolatileNodeInfo
{
	union
	{
		struct
		{
			U32 distance				: 23;
			U32 distanceSubtechnique	: 1;
			U32 screenSize				: 8;
		};
		U32 data;
	};
};

} // namespace SceneGraph

#endif // SCENE_GRAPH_VIEW_H
