
#ifndef UI_ADOBE_BOX_H
#define UI_ADOBE_BOX_H

#include "ui\Game\Gadgets\BaseUIShape.h"

namespace UI
{

class UIAdobeBox : public BaseUIShape
{
public:
	DECLARE_GADGET( E_UIButtonType_AdobeBox, "ADOBE_BOX" )

	virtual void		Init();
	virtual void		Setup( lua_State* pState );
	virtual void 		FrameUpdate();
	virtual void		Render();

private:
	void				FindPieces();
	void				RenderPiece( const C_AdobeShape* pShape, const F32 fX, const F32 fY, const F32 fZ, const F32 fSizeX, const F32 fSizeY, const TransformIndex uColourMatrixIndex );

public:
	F32					m_CornerSize;

private:
	enum
	{
		E_Corner_TopLeft = 0,
		E_Corner_TopCentre,
		E_Corner_TopRight,
		E_Corner_MiddleLeft,
		E_Corner_MiddleCentre,
		E_Corner_MiddleRight,
		E_Corner_BottomLeft,
		E_Corner_BottomCentre,
		E_Corner_BottomRight,

		E_Corner_Count
	};

	const C_AdobeShape*		m_Shapes[E_Corner_Count];

};

}; // namespace UI

#endif // UI_ADOBE_BOX_H
