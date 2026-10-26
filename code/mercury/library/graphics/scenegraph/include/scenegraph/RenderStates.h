#ifndef SCENE_GRAPH_RENDER_STATES_H
#define SCENE_GRAPH_RENDER_STATES_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Fx/RenderFlags.h"
#include "RenderingData/RenderFlags.h"

namespace SceneGraph
{

struct ALIGN16 RenderStates
{
	RenderStates();
	~RenderStates();

	FX::RenderFlags						renderFlagsFX;
	RenderingData::Target::RenderFlags	renderFlagsRD;

	U32									colourWriteEnable;
	Display::C_StencilState				stencilState;

	bool								setBlendMode;
	Display::E_Bool						blendEnable;
	Display::E_Bool						blendHighPrecisionEnable;
	Display::E_Blend					srcBlend;
	Display::E_Blend					destBlend;
	Display::E_BlendOp					blendOp;
	Display::E_Bool						alphaToMaskEnable;
	Display::E_Bool						alphaTestEnable;
	U32									alphaRef;

	bool								setDepth;
	Display::E_Bool						depthEnable;
	Display::E_Bool						depthWriteEnable;
	Display::E_CmpFunc					depthFunc;

	bool								setPixelShader;
	const Display2::C_PixelShader*		pPixelShader;
};

} // namespace SceneGraph

#endif // SCENE_GRAPH_RENDER_STATES_H
