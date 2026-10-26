#ifndef UICOLOURPICKER_H
#define UICOLOURPICKER_H

#ifdef _MSC_VER
#pragma once
#endif

#include <ui\Game\Gadgets\ButtonController.h>
#include <ui/Game/Gadgets/Buttons.h>	// [Blur2PC] C_GadgetBase
#include "Ui\UIGameDefines.h"
#include "ui\UiManager.h"

class C_GameVehicleData;

namespace Vehicle
{
	struct XtPaintJob;
}

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

enum E_Palette
{
	E_Palette_Start = 0,

	E_Palette_Solid = E_Palette_Start,
	E_Palette_Metallic,
	E_Palette_Pearlecent,
	E_Palette_Candy,

	E_Palette_Count,
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

	C_UIColour					GetColour( const S32 sIndex, const S32 sIndexV, const bool bUseRespray = false, const E_Palette ePalette = E_Palette_Solid );
	C_UIColour					GetSorceryColour( const S32 sIndex, const S32 sIndexV, const E_Palette ePalette );
	XtId						GetSorceryId( const S32 sIndex, const S32 sIndexV, const E_Palette ePalette );
	bool						GetFromSorceryId( const XtId id, const S32 sIndex, S32& sPosition, S32& sPositionV, S32& sPalette );

private:
	C_UIColour					CalculateColour( const F32 h, const Maths::HVec2& me );
	C_UIColour					HSVtoRGB( const F32 h, const F32 s, const F32 v );

private:
	S32										m_sBoxSize;
	S32										m_sSizeV;
	S32										m_sSpin;
	S32										m_sTaperSize;
	Btl::Vector< Maths::HVec3 >				m_Positions;
	Btl::Vector< C_UIColour >				m_Colours;

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
	F32					m_fSegmentMultX;
	F32					m_fSegmentMultY;
	bool				m_bDisableAlpha;
	bool				m_bUseRespray;
	bool				m_bDisableAdditionalPalettes;
	bool				m_bDisableResprayMessages;
	F32					m_fMultBacking;
	F32					m_fMultPiece;
	F32					m_fMultPreview;
	F32					m_fPreviewOffsetMultX;
	F32					m_fPreviewOffsetMultY;
	F32					m_fIntensityOffsetMultY;
	F32					m_fAlphaOffsetMultY;
	C_FontStylePasses	m_FontStylePasses;
	F32					m_fMultFont;
	F32					m_fMultButtons;
	F32					m_fMultButtonsWidthIntensity;
	F32					m_fMultButtonsWidthAlpha;
	F32					m_fPaletteTextOffsetMultY;
	XtSz8				m_xtColourLocked;

public:
	void 				Init();
	void 				Setup( lua_State* pState );
	bool 				MessageHandler( const Messaging::C_MessageBase* pMessage );
	void 				Render();

	C_UIColour			GetSelected();
	S32					GetColourIndex();
	S32					GetIntensityIndex();
	XtId				GetSelectedRespray();
	E_Palette			GetPalette();

	void				SetColourIndex( S32 index );
	void				SetIntensityIndex( S32 index );
	void				SetFromRespray( const XtId id, const S32 sIndex, const bool bMultiplayer );

	C_UIColour			GetColour( const S32 sIndex, const S32 sIndexV, const bool bInvert = false );	

	void				SetAlpha( F32 alpha );
	F32					GetAlpha();

	void				SetPaletteLocked( const S32 sPaletteLocked )							{ m_sPaletteLocked = sPaletteLocked; UpdatePaletteLocked(); }

private:
	C_UIColour			GetColour( const S32 sIndex, const bool bInvert = false );

	void				MoveUp();
	void				MoveDown();
	void				MoveLeft();
	void				MoveRight();

	void				MoveLeftV();
	void				MoveRightV();

	void				MoveLeftA();
	void				MoveRightA();

	void				MoveLeftPalette();
	void				MoveRightPalette();

	S32					NumCols( const S32 sRow )												{ return C_UIColourPicker::Instance().NumCols( sRow ); }
	void				GetRowColFromPos( S32& sRow, S32& sCol, S32 sPositionOverride = -1 );
	void				SetPosFromRowCol( const S32 sRow, const S32 sCol );

	S32					NumColours()															{ return C_UIColourPicker::Instance().NumColours(); }

	void				UpdatePaletteLocked()													{ m_bPaletteLocked = ( ( ( ( 1 << m_sPalette ) & m_sPaletteLocked ) != 0 ) || ( m_bDisableAdditionalPalettes && ( m_sPalette != E_Palette_Solid ) ) ); }
	void				PaletteChangedSound();

	void				SendResprayMessage();

private:
	S32					m_sPosition;
	S32					m_sPositionV;
	S32					m_sPositionA;

	S32					m_sBoxSize;
	S32					m_sSizeV;

	F32					m_fSegmentSize;

	S32					m_sPalette;
	S32					m_sPaletteLocked;

	C_UISpriteColours	m_ColourLocked;
	bool				m_bPaletteLocked;
};

}; // namespace UI

#endif // UICOLOURPICKER_H
