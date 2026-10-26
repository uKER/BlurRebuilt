#ifndef UIHORIZONTALMENU_H
#define UIHORIZONTALMENU_H

#ifdef _MSC_VER
#pragma once
#endif

#include "ui/Game/Gadgets/BaseUIVariableSelect.h"

namespace UI
{

class UIHorizontalMenu : public BaseUIVariableSelect
{
public:
	DECLARE_GADGET( E_UIButtonType_HorizontalMenu, "HORIZONTAL_MENU" )

	virtual			~UIHorizontalMenu( )	{  }

	virtual void	Init();
	virtual void	Render();
	virtual bool	MessageHandler(const Messaging::C_MessageBase *pMessage);

	virtual void	AddItem( const WCHAR* pText, U32 value );

public:	// exposed for Xt
	bool				m_bDoScroll;

private:
	F32					m_NextPos;
	F32					m_Pos;
	Btl::Vector< F32 >	m_StringSize;
};

}; // namespace UI

#endif
