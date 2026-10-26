
#ifndef UI_POSTPROCESSTEXTURE_H
#define UI_POSTPROCESSTEXTURE_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Ui/UIGameDefines.h"
#include <ui/Game/Gadgets/Buttons.h>	// [Blur2PC] C_GadgetBase
#include "Ui/UiManager.h"

namespace UI
{

class UIPostProcessTexture : public C_GadgetBase
{
public:
	DECLARE_GADGET( E_UIGameButtonType_PostProcessTexture, "POSTPROCESS_TEXTURE" );

	// Gadget interface
	virtual void Init();
	virtual void Setup( lua_State* pState = NULL );
	virtual void Render();
};

}; // namespace UI

#endif // UI_POSTPROCESSTEXTURE_H
