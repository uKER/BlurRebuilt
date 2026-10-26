#ifndef UI_AMAX_SLIDER_H
#define UI_AMAX_SLIDER_H

#ifdef _MSC_VER
#pragma once
#endif

#include <ui/Game/Gadgets/UISlider.h>

#include "ui\UIGameDefines.h"

namespace UI
{

class UIAmaxSlider : public UISlider
{
public:
	DECLARE_GADGET( E_UIGameButtonType_AmaxSlider, "AMAX_SLIDER" )

	void Setup( lua_State* pState );
	void PostInit();
	void FrameUpdate();
	void Render();
	bool MessageHandler(const Messaging::C_MessageBase *pMessage);

	F32	GetItemMaxSizeX();
	void UpdateArrows();

private:
	const F32			RenderAnalogue();
	const F32			RenderVariable();
	void				RenderVariableText();
	void				ActivateArrows();

// Exposed for Xt
public:
	XtSz8				m_FontColourName;
	XtSz8				m_SymbolColourName;
	XtSz8				m_BackingColourName;
	C_Vector3Position	m_FontOffset;
	C_Vector3Size		m_SymbolSize;
	C_Vector3Size		m_BackingSize;
	XtF32				m_MinusIntensity;
	XtF32				m_PlusIntensity;
	XtF32				m_ArrowPadding;

	C_FontStylePasses	m_ItemStyle;
	C_FontStylePasses	m_TitleStyle;

private:
	C_UISpriteColours	m_FontColour;
	C_UISpriteColours	m_BackingColour;

	F32					m_MaxItemSizeX;

	C_GadgetBase*		m_pLeftArrow;
	C_GadgetBase*		m_pRightArrow;

	bool				m_bHasResolvedColours;
};

}; // namespace UI

#endif // UI_AMAX_SLIDER_H

