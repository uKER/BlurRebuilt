#ifndef UI_VINYL_STRIP_BAR_H
#define UI_VINYL_STRIP_BAR_H

#include <ui\Game\Gadgets\BaseUIShape.h>

#include "ui\UiManager.h"
#include "VinylList.h"

namespace UI
{

class UIVinylStripBar : public C_GadgetBase
{
public:
	DECLARE_GADGET( E_UIGameButtonType_VinylStripBar, "VINYL_STRIP_BAR" )

	void		Init();
	void		Setup( lua_State* pState );
	void		Render();
	void		FrameUpdate();
	bool		MessageHandler(const Messaging::C_MessageBase *pMessage);

private:
	void		RenderShape( const C_AdobeShape* pShape, const TransformIndex ti, const Maths::HVec3& drawPos, const F32 drawSizeX, const F32 drawSizeY, const C_UIColour& fillColour );
	void		RenderBacking( const bool isSelected, const bool isHighlighted, const bool isGroup, const Maths::HVec3& drawPos, U32 depth );
	void		RenderStripSub(VinylList* list, int root, Maths::HVec3 renderPos, U32 depth );
	void		RenderStrip( VinylList& list, int currentItem, F32 x_start, F32 x_end, U32 depth );
	void		RenderItem( VinylList& list, int item, bool selected, F32 x, U32 depth );

// Exposted for Xt
public:
	C_Vector3Size		m_BackingSize;
	C_Vector3Size		m_IconSize;
	C_Vector3Scale		m_SceneScale;
	C_Vector3Position	m_StepDepth;
	C_Vector3Position	m_IconOffset;
	XtF32				m_StepSize;
	XtF32				m_MoveSpeedMult;
	XtF32				m_UnfoldScale;
	XtF32				m_SplitScale;
	XtU32				m_CurrentShapeIndex;
	XtSz8				m_BackingColourName;
	XtSz8				m_UnavailableColourName;
	XtSz8				m_DesOutlineColourName;
	XtSz8				m_SelOutlineColourName;
	XtSz8				m_IconColourName;

private:
	C_RenderState		m_ShapeRs;
	F32					m_MoveOffset;
	C_UISpriteColours	m_BackingColours;
	C_UISpriteColours	m_UnavailableColours;
	C_UISpriteColours	m_DesOutlineColours;
	C_UISpriteColours	m_SelOutlineColours;
	C_UISpriteColours	m_IconColours;
};

}; // namespace UI

#endif // UI_VINYL_STRIP_BAR_H
