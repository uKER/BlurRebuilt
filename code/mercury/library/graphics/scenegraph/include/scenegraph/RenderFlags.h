#ifndef SCENE_GRAPH_RENDERFLAGS_H
#define SCENE_GRAPH_RENDERFLAGS_H

#ifdef _MSC_VER
#pragma once
#endif

namespace SceneGraph
{

struct RenderFlags
{
	RenderFlags() { flags = 0xFFFFFFFF; }

    union
    {
        struct
        {
			U32 unused			: 30;
			U32 setColourWrite	: 1;
            U32 setStencilState	: 1;
        };
        U32 flags;
    };
};

} // namespace SceneGraph

#endif // SCENE_GRAPH_RENDERFLAGS_H
