#ifndef UIVARIABLE_SPINNER_H
#define UIVARIABLE_SPINNER_H

#ifdef _MSC_VER
#pragma once
#endif

#include "ui/Game/Gadgets/UIStaticText.h"
#include "ui/Game/Gadgets/BaseUIVariableSelect.h"
#include "ui/Graphics/Localisation/loc.h"

namespace UI
{


class UIVariableSpinner : public BaseUIVariableSelect
{
public:
	DECLARE_GADGET( E_UIButtonType_VsSpinner, "SPINNER_BUTTON" )

	virtual void	Init();
	virtual void	Setup( lua_State* pState );
	virtual void	Render();
	virtual bool	MessageHandler(const Messaging::C_MessageBase *pMessage);

	void			SetTitle( const C_TextStringID& id )
					{
						m_Text = id;
						m_HeaderStoreString.Clear();
					}

protected:
	void			MoveLeft();
	void			MoveRight();
	void			ResolveAddOpened();
	void			SetText( const U32 text )						
					{ 
						m_Text = text; 
					}

public:
	XtLocId			m_TextID;
	C_TextStringID	m_Text;
	C_WideString512	m_HeaderStoreString;


	E_Justify		m_DataJustify;
	bool			m_bAddTitleLength;

protected:
	F32				m_fTitleLength;
};

}; // namespace UI

#endif // UIVARIABLE_SPINNER_H
