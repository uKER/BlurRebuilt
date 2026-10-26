#ifndef UIVERTICALMENU_H
#define UIVERTICALMENU_H

#ifdef _MSC_VER
#pragma once
#endif

#include "ui/Game/Gadgets/BaseUIVariableSelect.h"

namespace UI
{

class UIVerticalMenu : public BaseUIVariableSelect
{
public:
	DECLARE_GADGET( E_UIButtonType_VerticalMenu, "VERTICAL_MENU" )

	virtual ~UIVerticalMenu( ) { }

	virtual	void	Init( );
	virtual void	Render( );
	virtual void	Setup( lua_State* pState );
	virtual bool	MessageHandler(const Messaging::C_MessageBase *pMessage);

protected:
	void			MoveUp();
	void			MoveDown();
	void			ResolveAddOpened();

public:	// exposed for Xt
	S32				m_NumRows;
	F32				m_ScrollSpeed;
	bool			m_bQuickNav;

protected:
	F32				mNextPos;
	F32				mPos;
	F32				mWait;
	F32				mOverallSpacing;
	F32				mDropshadow;
	C_UIColour		mOverlayTextureCol;
	bool			mRemoveSpacing;
	F32				mScrollOffset;

	F32				mCatWidth;
};

}; // namespace UI

#endif
