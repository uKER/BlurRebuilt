
#ifndef UI_SCROLLBAR_H
#define UI_SCROLLBAR_H

#include "ui/Game/Gadgets/Buttons.h"

namespace UI
{

enum E_ScrollBarType
{
	E_ScrollBarType_Vertical = 0,
	E_ScrollBarType_Horizontal,
};

/*---------------------------------------------------------------------------------------
	Class : C_ScrollBarStyle
---------------------------------------------------------------------------------------*/
class C_ScrollBarStyle
{
public:
	C_ScrollBarStyle();

	static const C_ScrollBarStyle& Instance()
	{	 
		return m_sInstance; 
	}

// Exposed for XT
public:
	F32							m_EndSizeY;				/// <@ Size of the end piece.
	F32							m_SliderPaddingX;		/// <@ Padding of slider.
	F32							m_SliderPaddingY;		/// <@ Padding of slider.
	F32							m_MinSliderSize;		/// <@ The maximum size the slider can be. Use 0 for not defines.
	F32							m_MaxSliderSize;		/// <@ The maximum size the slider can be. Use 0 for not defines.
	XtRgba						m_EndColour;
	XtRgba						m_SliderColour;
	XtRgba						m_BackgroundColour;
	XtRgba						m_BorderColour;
	F32							m_BorderSize;

private:
	static C_ScrollBarStyle		m_sInstance;
};

/*---------------------------------------------------------------------------------------
	Class : UIScrollBar

	Desc : Scrollbar gadget that supports horizontal and vertical styles.
---------------------------------------------------------------------------------------*/
class UIScrollBar : public C_GadgetBase
{
public:
	UIScrollBar()
		: m_pStyle( NULL )
	{
	}

	DECLARE_GADGET( E_UIButtonType_ScrollBar, "SCROLLBAR" )

	virtual void		Init();
	virtual void		Setup( lua_State* pState );
	virtual void		Render();

	void				SetScrollArea( const F32 size );

	void				SetSliderPosition( const F32 pos )
						{
							m_SliderPosition = pos;
						}

	void				SetBounds( const F32 min, const F32 max )
						{
							m_BoundsMin = min;
							m_BoundsMax = max;
						}

	bool				CanScroll() const
						{
							return ( m_ScrollArea > ( m_BoundsMax - m_BoundsMin ) ) ? true : false;
						}

	E_ScrollBarType		GetScrollType() const
						{
							return m_ScrollType;
						}

	void				Build( const bool bForceBuild = false );

private:
	void				RenderBorder( const F32& x, const F32& y );

// Exposed for xt.
public:
	XtHandle<C_ScrollBarStyle>	m_StyleRef;

public:
	E_ScrollBarType			m_ScrollType;		/// Horizontal or vertical mode.
	F32						m_BoundsMin;		/// Frame bounds.
	F32						m_BoundsMax;
	F32						m_ScrollArea;		/// Total size of the canvas.
	bool					m_bAlwaysShow;		/// Forces the scrollbar to render.

private:
	F32						m_ScrollBarSize;	// Total size of the canvas.
	F32						m_MiddleSize;		// Size of the middle.
	F32						m_SliderPosition;	// Position of the slider.
	
	const C_ScrollBarStyle*	m_pStyle;			// Scrollbar style
};

}; // namespace UI

#endif // UI_SCROLLBAR_H
