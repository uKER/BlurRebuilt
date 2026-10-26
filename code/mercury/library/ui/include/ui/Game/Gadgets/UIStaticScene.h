
#ifndef UI_STATIC_SCENE_H
#define UI_STATIC_SCENE_H

#include "ui\Game\Gadgets\BaseUIShape.h"

namespace UI
{

class UIStaticScene : public BaseUIShape
{
public:
	DECLARE_GADGET( E_UIButtonType_StaticScene, "STATIC_SCENE" )

	virtual void		Init();
	virtual void		Setup( lua_State* pState );
	virtual void		Render();

	virtual void		RenderFill( const C_AdobeShape* pShape, const Maths::HVec3& drawPos, const F32& drawSizeX, const F32& drawSizeY, const U8& transformIndex );
	virtual void		RenderStroke( const C_AdobeShape* pShape, const Maths::HVec3& drawPos, const F32& drawSizeX, const F32& drawSizeY, const U8& transformIndex );

};

}; // namespace UI

#endif // UI_STATIC_SCENE_H