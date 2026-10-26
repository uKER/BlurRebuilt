
#ifndef UI_UIAMAXLIGHT_H
#define UI_UIAMAXLIGHT_H

#include "ui/UIGameDefines.h"
#include "ui/Game/Gadgets/Buttons.h"

namespace UI
{

class UIAmaxLight : public C_GadgetBase
{
public:
	DECLARE_GADGET( E_UIGameButtonType_AmaxLight, "AMAX_LIGHT" )

	virtual void				Init();
	virtual void				Render();
};

}; // namespace UI

#endif // UI_UIAMAXLIGHT_H