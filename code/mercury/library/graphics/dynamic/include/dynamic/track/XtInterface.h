#ifndef RENDER_DYNAMICEFFECT_TRACK_XTINTERFACE_H
#define RENDER_DYNAMICEFFECT_TRACK_XTINTERFACE_H

#ifdef _MSC_VER
#pragma once
#endif


#include "xt\xtgame.h"

namespace DynamicEffect
{
class C_Renderer;

namespace Track
{
class C_TrackRenderer;
class C_Material;

namespace XtInterface
{
	enum E_BlendMode
	{
		E_BlendMode_Multiply = Display::E_BlendMode_Multiply,
		E_BlendMode_Alpha = Display::E_BlendMode_AlphaPreMultiply
	};

	struct Material
	{
		Material() : bInitialised(false), id(0) {}
		bool						bInitialised;
		U32							id;

		DynamicEffect::Track::C_Material*			pMaterial;
		XtSz8						texture;
		XtSz8						normal;
		E_BlendMode					blendMode;
	};
	void		MaterialCallback(XtEventType event, XtRecord* record, Material* pData, XtCallbackContext*);

	// public methods
	void		Initialise(C_TrackRenderer* pTrackRenderer);
	void		Update();
	void		Reset();
	void		Shutdown();

} // XtInterface
} // Track
} // DynamicEffect

#endif // RENDER_DYNAMICEFFECT_TRACK_XTINTERFACE_H
