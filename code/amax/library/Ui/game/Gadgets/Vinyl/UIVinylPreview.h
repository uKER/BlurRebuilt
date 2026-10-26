#ifndef UI_VINYL_PREVIEW_H
#define UI_VINYL_PREVIEW_H

#include <ui\Game\Gadgets\BaseUIShape.h>
#include <ui/Game/Gadgets/Buttons.h>	// [Blur2PC] C_GadgetBase

#include "ui\UiManager.h"
#include "Ui\UIGameDefines.h"

namespace UI
{

class UIVinylPreview : public C_GadgetBase
{
public:
	DECLARE_GADGET( E_UIGameButtonType_VinylPreview, "VINYL_PREVIEW" )

	void		Init();
	void		Setup( lua_State* pState );
	void		Render();

private:
};

}; // namespace UI

#endif // UI_VINYL_PREVIEW_H
