#ifndef UICOLOURPICKER_H
#define UICOLOURPICKER_H

#ifdef _MSC_VER
#pragma once
#endif

#include <ui\Game\Gadgets\ButtonController.h>
//#include "ui\UIGameDefines.h"
#include "ui\UiManager.h"

namespace UI
{

// do _NOT_ change this order!
enum E_Col
{
	E_Col_R = 0,
	E_Col_Y,
	E_Col_G,
	E_Col_C,
	E_Col_B,
	E_Col_M,
	E_Col_W,

	E_Col_Count,
};

// ----------------
// C_UIColourPicker
// ----------------
/// storage for all of the colours
class C_UIColourPicker
{
public:
	void						BuildColours( const S32 sBoxSize, const S32 sSizeV, const S32 sSpin );

	S32							BoxSize()												{ return m_sBoxSize; }
	S32							SizeV()													{ return m_sSizeV; }

	S32							NumCols( const S32 sRow );

	S32							NumColours()											{ return m_Colours.Size(); }	
	C_UIColour					GetColour( const S32 sIndex, const S32 sVPosition );

private:
	C_UIColour					CalculateColour( const F32 h, const Maths::HVec2& me );
	C_UIColour					HSVtoRGB( const F32 h, const F32 s, const F32 v );

private:
	S32							m_sBoxSize;
	S32							m_sSizeV;
	S32							m_sSpin;
	S32							m_sTaperSize;
	Btl::Vector< Maths::HVec3 >	m_Positions;
	Btl::Vector< C_UIColour >	m_Colours;

public:
	static C_UIColourPicker&	Instance()						{ return m_sInstance; }
private:
	static C_UIColourPicker		m_sInstance;
};

// --------------
// UIColourPicker
// --------------
class UIColourPicker : public C_GadgetBase
{
public:
	DECLARE_GADGET( E_UIGameButtonType_ColourPicker, "COLOUR_PICKER" )

public: // exposed for Xt
	F32			m_fSegmentMultX;
	F32			m_fSegmentMultY;

public:
	void 		Init();
	void 		Setup( lua_State* pState );
	bool 		MessageHandler( const Messaging::C_MessageBase* pMessage );
	void 		Render();

	C_UIColour	GetSelected();
	S32			GetColourIndex();
	S32			GetIntensityIndex();

	void		SetColourIndex( S32 index );
	void		SetIntensityIndex( S32 index );
	C_UIColour	GetColour( const S32 sIndex, const S32 sVPosition, const bool bInvert = false );

	void		SetAlpha( F32 alpha );
	F32			GetAlpha();

private:
	C_UIColour	GetColour( const S32 sIndex, const bool bInvert = false );

	void		MoveUp();
	void		MoveDown();
	void		MoveLeft();
	void		MoveRight();

	void		MoveLeftV();
	void		MoveRightV();

	void		MoveLeftA();
	void		MoveRightA();

	S32			NumCols( const S32 sRow )												{ return C_UIColourPicker::Instance().NumCols( sRow ); }
	void		GetRowColFromPos( S32& sRow, S32& sCol, S32 sPositionOverride = -1 );
	void		SetPosFromRowCol( const S32 sRow, const S32 sCol );

	S32			NumColours()															{ return C_UIColourPicker::Instance().NumColours(); }

private:
	S32			m_sPosition;
	S32			m_sPositionV;
	S32			m_sPositionA;

	S32			m_sBoxSize;
	S32			m_sSizeV;

	F32			m_fSegmentSize;
};

}; // namespace UI

#endif // UICOLOURPICKER_H
