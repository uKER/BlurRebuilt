
#ifndef UI_UIPROGRESSBAR_H
#define UI_UIPROGRESSBAR_H

#include "ui/Game/Gadgets/Buttons.h"

namespace UI
{

class UIProgressBar : public C_GadgetBase
{
public:
	DECLARE_GADGET( E_UIButtonType_ProgressBar, "PROGRESS_BAR" )

	virtual void			Init();
	virtual void 			Setup( lua_State* pState );
	virtual void 			Render();

	void					SetPosition( const F32 fPos )
							{
								m_CurrentPosition = (S32)((F32)m_Sections * fPos);
							}

public:
	S32						m_Sections;
	S32						m_CurrentPosition;

	F32						m_Spacing;
	F32						m_Padding;
};

}; // namespace UI

#endif // UI_UIPROGRESSBAR_H
