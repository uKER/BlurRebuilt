#ifndef UISLIDER_H
#define UISLIDER_H

#ifdef _MSC_VER
#pragma once
#endif

#include "ui/Game/Gadgets/Buttons.h"
#include "ui/Game/Gadgets/BaseUIVariableSelect.h"

namespace UI
{

class UISlider : public BaseUIVariableSelect
{
public:
	enum E_Type
	{
		E_Type_Analogue = 0,
		E_Type_Variable,
	};

public:
	DECLARE_GADGET( E_UIButtonType_Slider, "SLIDER" )

public:
	virtual void 			Init();
	virtual void 			Setup( lua_State* pState );
	virtual void 			FrameUpdate();
	virtual void 			Render();
	virtual bool 			MessageHandler(const Messaging::C_MessageBase *pMessage);

	const F32				Value()					{ return m_Value; }
	void					Value( F32 fValue )		{ m_Value = fValue; }
	void					ChangeText( const C_TextStringID& number )
							{
								m_Text = number;
							}

public:
	// Exposed for XT
	E_Type					m_SliderType;
	F32						m_Value;
	F32						m_Step;
	F32						m_FontHeight;
	S32						m_Pad;					// Pad controlling to this gadget.
	XtSz8					m_VariableName;
	XtLocId					m_TextID;

protected:
	C_TextStringID			m_Text;

	S32						m_sController;

	bool					m_ButtonDown;
	bool					m_ButtonPrevDown;
	bool					m_MouseDragging;

	// selector
	F32						m_fSelectorTextOffsetX;
	F32						m_fSelectorTextOffsetY;
	F32						m_fSelectorIconWidth;
	F32						m_fSelectorIconHeight;
	E_Justify				m_eSelectorTextJustify;

	S32						m_sAlphaAmount;

	// sounds
	S32						m_sSoundDown;
	S32						m_sSoundUp;
};

} // namespace UI

#endif // UISLIDER_H
