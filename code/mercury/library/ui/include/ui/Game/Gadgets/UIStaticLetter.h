#ifndef UISTATICLETTER_H
#define UISTATICLETTER_H

#ifdef _MSC_VER
#pragma once
#endif

#include "ui/Game/Gadgets/Buttons.h"
#include "ui/Graphics/Font/FontStyle.h"

namespace UI
{

class UIStaticLetter : public C_GadgetBase
{
public:
	DECLARE_GADGET( E_UIButtonType_StaticLetter, "LETTER" )

public:	
	void Init();
	void Setup( lua_State* pState );
	void Render();

public: // exposed for Xt
	C_FontStylePasses		m_FontStylePasses;
	XtSz16					m_TextLetter;
	XtLocId					m_TextId;
	WCHAR					m_Letter;
};

}; // namespace UI

#endif /* UISTATICLETTER_H */
