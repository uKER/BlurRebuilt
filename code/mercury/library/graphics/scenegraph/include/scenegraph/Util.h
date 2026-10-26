#ifndef SCENE_GRAPH_UTIL_H
#define SCENE_GRAPH_UTIL_H

#ifdef _MSC_VER
#pragma once
#endif

#include <renderingdata/render.h>

namespace SceneGraph
{

// todo: how to map from Generic E_RenderPasses to RenderingData render passes
// eg E_RenderPass_FocusCar to RenderingData::RENDER_PASS_OPAQUE
/*inline RenderingData::RenderPass GetRenderingDataRenderPass(E_RenderPass renderPass)
{
	switch(renderPass)
	{
	case E_RenderPass_Solid: return RenderingData::RENDER_PASS_OPAQUE;
	case E_RenderPass_Punch: return RenderingData::RENDER_PASS_ALPHA_TEST;
	case E_RenderPass_Decal: return RenderingData::RENDER_PASS_OVERLAY;
	case E_RenderPass_Alpha: return RenderingData::RENDER_PASS_BACK_FRONT;
		default: return RenderingData::RENDER_PASS_OPAQUE;
	}
}*/

/*inline U32 GetSceneGraphRenderPass(RenderingData::RenderPass renderPass)
{
	switch(renderPass)
	{
		case RenderingData::RENDER_PASS_OPAQUE: return E_RenderPass_Solid;
		case RenderingData::RENDER_PASS_ALPHA_TEST: return E_RenderPass_Punch;
		case RenderingData::RENDER_PASS_OVERLAY: return E_RenderPass_Decal;
		case RenderingData::RENDER_PASS_BACK_FRONT: return E_RenderPass_Alpha;
		default: return E_RenderPass_Solid;
	}
}*/

}

#endif // SCENE_GRAPH_UTIL_H
