
#ifndef UI_PATH_H
#define UI_PATH_H

#ifdef _MSC_VER
#pragma once
#endif

#include "ui/Game/Gadgets/Buttons.h"

namespace UI
{

// Forward declarations.
class C_AdobePath;


//-------------------------------------------------------------------
// Class : UIPath
//
// Desc :
//-------------------------------------------------------------------
class UIPath : public C_GadgetBase
{
public:
	DECLARE_GADGET( E_UIButtonType_Path, "PATH" )

	void			Init( );
	void			Setup( lua_State *pState );
	void			Render( );

public:
	XtSz8				m_SceneName;
	XtSz8				m_PathName;
	F32					m_AnimationTime;

private:
	C_AdobePath*		m_Spline;
	C_Vector3Position	m_OriginalPos;
};

} // namespace UI

#endif // UI_SPLINE_H
