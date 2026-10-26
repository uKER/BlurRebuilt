
#include "stdafx.h"

#include "UIVinylPreview.h"
#include "VinylManager.h"

#include <ui/Game/Gadgets/ButtonMessages.h>
#include <ui/Graphics/SimpleTimer.h>
#include <Ui/Illustrator/UIAdobeShape.h>

// XT Definition
XT_TYPE( UI::UIVinylPreview, "UIVinylPreview" )
	XT_BASE( UI::C_GadgetBase )
XT_END_TYPE

namespace UI
{

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void UIVinylPreview::Init()
{
	C_GadgetBase::Init();
}

void UIVinylPreview::Setup( lua_State* pState )
{
	C_GadgetBase::Setup( pState );

	m_RenderState.m_EffectIndex = E_Effect_PolygonFlat;
	m_RenderState.m_EffectParam = 0;
}

void UIVinylPreview::Render()
{

}

}; // namespace UI
