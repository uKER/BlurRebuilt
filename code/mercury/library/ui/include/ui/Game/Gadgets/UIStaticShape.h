
#ifndef UI_STATIC_SHAPE_H
#define UI_STATIC_SHAPE_H

#include "ui\Game\Gadgets\BaseUIShape.h"

namespace UI
{

class UIStaticShape : public BaseUIShape
{
public:
	DECLARE_GADGET( E_UIButtonType_StaticShape, "STATIC_SHAPE" )

	virtual void		Init();
	virtual void		Setup( lua_State* pState );
	virtual void		Render();
};

}; // namespace UI

#endif // UI_STATIC_SHAPE_H