#ifndef UI_TOGGLE_H
#define UI_TOGGLE_H

#ifdef _MSC_VER
#pragma once
#endif

#include "ui\UIGameDefines.h"
#include "ui/Game/Gadgets/BaseUIVariableSelect.h"

namespace UI
{

class UIToggle : public I_SelectableGadgetBase
{
public:
	DECLARE_GADGET( E_UIGameButtonType_Toggle, "TOGGLE" )

	void Setup( lua_State* pState );
	void Render();

	void Toggle() { m_Toggle = !m_Toggle; }
	void ChangeTitle( const C_TextStringID& number )				{ m_TitleText = number; }

	// Ive purposely not called this SetColourStyle and didnt want to refactor the base functionality. Can wait till after Blur
	void ChangeColour( const S32 sArg, const char* pStyle );

	// Must supply to implementation for these functions
	const bool	SetCurrentItemByIndex( const S32 index );
	const bool	SetCurrentItemByValue( const F64 value );

	const S32	GetCurrentItemIndex() const							{ return m_Toggle ? 1 : 0; }
	const F64	GetCurrentItemValue( const U32 arg ) const			{ return m_Toggle ? 1.0f : 0.0f; }

public:
	XtLocId				m_TitleID;
	XtLocId				m_ItemAID;
	XtLocId				m_ItemBID;

	C_Vector3Position	m_TitleOffset;
	C_FontStylePasses	m_TitleStyle;
	XtSz8				m_TitleColourName;
	F32					m_TitleSize;

	C_FontStylePasses	m_ItemStyle;
	XtSz8				m_ItemColourName;
	F32					m_ItemSize;

private:
	bool				m_Toggle;

	C_UISpriteColours	m_TitleColour;
	C_UISpriteColours	m_ItemColour;

	C_TextStringID		m_TitleText;
	C_TextStringID		m_ItemAText;
	C_TextStringID		m_ItemBText;

};

}; // namespace UI

#endif // UI_TOGGLE_H

