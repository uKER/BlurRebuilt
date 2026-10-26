#ifndef UIGRAPH_H
#define UIGRAPH_H

#ifdef _MSC_VER
#pragma once
#endif

#include "ui/Game/Gadgets/Buttons.h"
#include "ui/Graphics/Font/FontStyle.h"
#include "ui/Graphics/Localisation/Loc.h"

namespace UI
{

// -------
// UIGraph
// -------
class UIGraph : public C_GadgetBase
{
public:
	DECLARE_GADGET( E_UIButtonType_Graph, "GRAPH" )

public: // exposed for Xt
	F32					m_fAxisMinX;
	F32					m_fAxisMaxX;
	F32					m_fAxisMinY;
	F32					m_fAxisMaxY;
	F32					m_fAxisSize;
	F32					m_fPointSize;
	F32					m_fLineSize;
	F32					m_fBarSize;
	F32					m_fAnimTimer;
	bool				m_bPoints;
	bool				m_bLines;
	bool				m_bBars;
	S32					m_sSubDivX;
	S32					m_sSubDivY;
	F32					m_fDivSizeX;
	F32					m_fDivSizeY;
	XtLocId				m_TitleX;
	XtLocId				m_TitleY;
	F32					m_fFontHeight;
	C_FontStylePasses	m_FontStylePasses;
	F32					m_fTitleOffset;
	bool				m_bFlipAxes;

public:
	void 		Init();
	void 		Setup( lua_State* pState );
	void 		Render();

	void		AddData( const F32 x, const F32 y );

private:
	typedef Btl::Pair< F32, F32 >	DataPair;
	Btl::Vector< DataPair >			m_Data;

	C_TextStringID					m_idTitleX;
	C_TextStringID					m_idTitleY;
};

};	// namespace UI

#endif /* UIGRAPH_H */
