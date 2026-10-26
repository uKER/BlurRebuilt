#ifndef UIRACEPREVIEW_H
#define UIRACEPREVIEW_H

#ifdef _MSC_VER
#pragma once
#endif

#include <ui/Game/Gadgets/Buttons.h>
#include "ui/UIGameDefines.h"

namespace UI
{

class UIStaticShape;

// ----------
// UIRacePreview
// ----------
class UIRacePreview : public C_GadgetBase
{
public:
	DECLARE_GADGET( E_UIGameButtonType_RacePreview, "RACE_PREVIEW" )

public:
	void				Init();
	void				Setup( lua_State* pState );
	void				PostInit();
	void				FrameUpdate();
	void				Render();

	void				SendToLua( lua_State* pState );
	bool				ShouldRefresh() const { return m_Refresh; }

public:
	F32					m_VehicleSize;
	XtSz8				m_ColourFirst;
	XtSz8				m_ColourSecond;
	XtSz8				m_ColourThird;
	XtSz8				m_ColourOther;

private:
	enum E_PosColour
	{
		E_PosColour_First = 0,
		E_PosColour_Second,
		E_PosColour_Third,
		E_PosColour_Other,

		E_PosColour_Max,
	};

	Maths::C_AxisBox	m_TrackBounds;
	C_UISpriteColours	m_PosColours[E_PosColour_Max];

	bool				m_Refresh;
	F32					m_Timer;
	F32					m_Scalar;
	F32					m_OffsetX;
	F32					m_OffsetY;

	UIStaticShape*		m_pTrack;
};

};	// namespace UI

#endif // UIRACEPREVIEW_H
