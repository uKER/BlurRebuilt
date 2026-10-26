#include "StdAfx.h"

#include "UIColourPicker.h"
//#include "UiXtConfig.h"
#include <ui/Game/Gadgets/ButtonMessages.h>

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", off ) 
#endif

XT_TYPE( UI::UIColourPicker, "UIColourPicker" )
	XT_BASE( UI::C_GadgetBase )
	XT_FIELD( m_fSegmentMultX, "segment_mult_x" )
	XT_FIELD( m_fSegmentMultY, "segment_mult_y" )
XT_END_TYPE

namespace UI
{

// ----------------
// C_UIColourPicker
// ----------------
C_UIColourPicker C_UIColourPicker::m_sInstance;

void C_UIColourPicker::BuildColours( const S32 sBoxSize, const S32 sSizeV, const S32 sSpin )
{
	m_sBoxSize = sBoxSize;
	m_sSizeV = sSizeV;
	m_sSpin = sSpin;

	m_Positions.Clear();
	m_Colours.Clear();

	if ( !( m_sBoxSize & 1 ) )
		m_sBoxSize++;

	m_sTaperSize = ( m_sBoxSize + 1 ) / 2;

	S32 sNum = ( m_sTaperSize * m_sTaperSize ) + ( ( m_sBoxSize - m_sTaperSize ) * ( m_sBoxSize - m_sTaperSize + 1 ) ) / 2; // top + middle
	sNum += ( ( m_sTaperSize - 1 ) * ( m_sTaperSize - 1 ) ) + ( ( m_sBoxSize - m_sTaperSize ) * ( m_sBoxSize - m_sTaperSize + 1 ) ) / 2; // bottom

	m_Positions.Reserve( sNum );
	m_Colours.Reserve( sNum );

	for ( S32 i = 0; i < m_sBoxSize; i++ )
	{
		S32 sRowSize = NumCols( i );

		for ( S32 j = 0; j < sRowSize; j++ )
		{
			Maths::HVec3 pos;

			pos.X( pos.X() + j );

			if ( sRowSize != m_sBoxSize )
				pos.X( pos.X() + ( m_sBoxSize - sRowSize - 1 ) * 0.5f + 0.5f );

			pos.Y( pos.Y() + i );

			m_Positions.PushBack( pos );

			//

			C_UIColour col = 0xffffffff;

            Maths::HVec2 me( pos.X(), pos.Y() );

			Maths::HVec2 centre( ( m_sTaperSize - 1 ), ( m_sTaperSize - 1 ) );
			Maths::HVec2 toCentre = centre - me;

			F32 c = Maths::RadToDeg( atan2f( toCentre.Y(), toCentre.X() ) );
            c += 180.0f;

            F32 h = c;
            F32 s = toCentre.Length() / centre.X();
            F32 v = 1.0f;

			m_Colours.PushBack( CalculateColour( h, me ) /*HSVtoRGB( h, s, v )*/ );
		}
	}

	assert( m_Positions.Size() == sNum );
	assert( m_Colours.Size() == sNum );
}

S32 C_UIColourPicker::NumCols( const S32 sRow )
{
	S32 sRowSize = m_sTaperSize;

	for ( S32 i = 0; i < m_sBoxSize; i++ )
	{
		if ( i == sRow )
			return sRowSize;

		sRowSize += ( i >= ( m_sTaperSize - 1 ) ) ? -1 : 1;
	}

	assert( 0 );
	return m_sTaperSize;
}

C_UIColour C_UIColourPicker::CalculateColour( const F32 h, const Maths::HVec2& me )
{
	S32 hi = ( (S32)Maths::Floor( h / 60.0f ) ) % 6;

	static Maths::HVec2 s_ColourPos01[ E_Col_Count ] =
	{
		Maths::HVec2(  1.0f, 0.5f ),	// E_Col_R
		Maths::HVec2( 0.75f, 1.0f ),	// E_Col_Y
		Maths::HVec2( 0.25f, 1.0f ),	// E_Col_G
		Maths::HVec2(  0.0f, 0.5f ),	// E_Col_C
		Maths::HVec2( 0.25f, 0.0f ),	// E_Col_B
		Maths::HVec2( 0.75f, 0.0f ),	// E_Col_M
		Maths::HVec2(  0.5f, 0.5f ),	// E_Col_W
	};

	static C_UIColour s_Colours[ E_Col_Count ] =
	{
		0xffff0000,		// E_Col_R
		0xffffff00,		// E_Col_Y
		0xff00ff00,		// E_Col_G
		0xff00ffff,		// E_Col_C
		0xff0000ff,		// E_Col_B		
		0xffff00ff,		// E_Col_M		
		0xffffffff,		// E_Col_W
	};

	Maths::HVec2 r( me.X() / ( ( m_sBoxSize - 1 ) ), me.Y() / ( ( m_sBoxSize - 1 ) ) );	// 01

	E_Col e1 = E_Col_Count;
	E_Col e2 = E_Col_Count;
	E_Col e3 = E_Col_Count;

	switch ( hi )
	{
		case 0: e1 = E_Col_R; e2 = E_Col_Y; e3 = E_Col_W; break;
		case 1: e1 = E_Col_Y; e2 = E_Col_G; e3 = E_Col_W; break;
		case 2: e1 = E_Col_G; e2 = E_Col_C; e3 = E_Col_W; break;
		case 3: e1 = E_Col_C; e2 = E_Col_B; e3 = E_Col_W; break;
		case 4: e1 = E_Col_B; e2 = E_Col_M; e3 = E_Col_W; break;
		case 5: e1 = E_Col_M; e2 = E_Col_R; e3 = E_Col_W; break;
		default: assert( 0 ); break;
	}

	// use barycentric coordinates
	Maths::HVec2 r1 = s_ColourPos01[ e1 ];
	Maths::HVec2 r2 = s_ColourPos01[ e2 ];
	Maths::HVec2 r3 = s_ColourPos01[ e3 ];

	// 2x2 matrix
	Maths::HVec2 m1( r1.X() - r3.X(), r2.X() - r3.X() );
	Maths::HVec2 m2( r1.Y() - r3.Y(), r2.Y() - r3.Y() );
	// invert
	F32 d = 1.0f / ( ( m1.X() * m2.Y() ) - ( m1.Y() * m2.X() ) );
	Maths::HVec2 m1i(  m2.Y() * d, -m1.Y() * d );
	Maths::HVec2 m2i( -m2.X() * d,  m1.X() * d );

	Maths::HVec2 rn( r - r3 );

	F32 l1 = ( m1i.X() * rn.X() ) + ( m1i.Y() * rn.Y() );
	F32 l2 = ( m2i.X() * rn.X() ) + ( m2i.Y() * rn.Y() );
	F32 l3 = 1 - l1 - l2;

	e1 = (E_Col)( (S32)( e1 + m_sSpin ) % 6 );
	e2 = (E_Col)( (S32)( e2 + m_sSpin ) % 6 );

	C_UIColour c1 = s_Colours[ e1 ];
	C_UIColour c2 = s_Colours[ e2 ];
	C_UIColour c3 = s_Colours[ e3 ];

	C_UIColour c = 0xffffffff;

	c.R01( ( ( c1.R01() * l1 ) + ( c2.R01() * l2 ) + ( c3.R01() * l3 ) ) );
	c.G01( ( ( c1.G01() * l1 ) + ( c2.G01() * l2 ) + ( c3.G01() * l3 ) ) );
	c.B01( ( ( c1.B01() * l1 ) + ( c2.B01() * l2 ) + ( c3.B01() * l3 ) ) );

	return c;
}

C_UIColour C_UIColourPicker::HSVtoRGB( const F32 h, const F32 s, const F32 v )
{
	C_UIColour c;

	F32 f = ( h / 60.0f ) - Maths::Floor( h / 60.0f );
	F32 p = v * ( 1 - s );
	F32 q = v * ( 1 - ( f * s ) );
	F32 t = v * ( 1 - ( ( 1 - f ) * s ) );

	S32 hi = (S32)Maths::Floor( h / 60.0f ) % 6;

	switch ( hi )
	{
		case 0:		c.Set01( 1.0f, v, t, p );	break;
		case 1:		c.Set01( 1.0f, q, v, p );	break;
		case 2:		c.Set01( 1.0f, p, v, t );	break;
		case 3:		c.Set01( 1.0f, p, q, v );	break;
		case 4:		c.Set01( 1.0f, t, p, v );	break;
		case 5:		c.Set01( 1.0f, v, p, q );	break;
		default:	assert( 0 );				break;
	}

	return c;
}

C_UIColour C_UIColourPicker::GetColour( const S32 sIndex, const S32 sVIndex )
{
	if ( sIndex >= NumColours() )
		return 0xffffffff;

	C_UIColour c = m_Colours[ sIndex ];
	F32 v = Maths::Clamp01( (F32)sVIndex / ( m_sSizeV - 1 ) );

	c.R( (U8)( c.R() * v ) );
	c.G( (U8)( c.G() * v ) );
	c.B( (U8)( c.B() * v ) );

	return c;
}

// --------------
// UIColourPicker
// --------------
void UIColourPicker::Init()
{
	C_GadgetBase::Init();

	m_sPosition = 0;
	m_sPositionV = 0;
	m_sPositionA = 0;

	// messages
	AddSubscriber( E_UiMsg_CtrlButton_Up );
	AddSubscriber( E_UiMsg_CtrlButton_Down );
	AddSubscriber( E_UiMsg_CtrlButton_Left );
	AddSubscriber( E_UiMsg_CtrlButton_Right );
	AddSubscriber( E_UiMsg_CtrlButton_LeftTrigger );
	AddSubscriber( E_UiMsg_CtrlButton_RightTrigger );
	AddSubscriber( E_UiMsg_CtrlButton_LeftShoulder );
	AddSubscriber( E_UiMsg_CtrlButton_RightShoulder );
}

void UIColourPicker::Setup( lua_State* pState )
{
	C_GadgetBase::Setup( pState );

	m_sBoxSize = C_UIColourPicker::Instance().BoxSize();
	m_sSizeV = C_UIColourPicker::Instance().SizeV();

	m_sPositionV = m_sSizeV - 1;
	m_sPositionA = m_sSizeV - 1;
	m_fSegmentSize = m_Size.x / m_sBoxSize;
}

bool UIColourPicker::MessageHandler( const Messaging::C_MessageBase* pMessage )
{
	if ( !m_Activated || !m_Selected )
		return C_GadgetBase::MessageHandler( pMessage );

	if ( !pMessage )
		return false;

	switch ( pMessage->Type() )
	{
		case E_UiMsg_CtrlButton_Up:				MoveUp();		break;
		case E_UiMsg_CtrlButton_Down:			MoveDown();		break;
		case E_UiMsg_CtrlButton_Left:			MoveLeft();		break;
		case E_UiMsg_CtrlButton_Right:			MoveRight();	break;
		case E_UiMsg_CtrlButton_LeftTrigger:	MoveLeftV();	break;
		case E_UiMsg_CtrlButton_RightTrigger:	MoveRightV();	break;
		case E_UiMsg_CtrlButton_LeftShoulder:	MoveLeftA();	break;
		case E_UiMsg_CtrlButton_RightShoulder:	MoveRightA();	break;

		default:
			return C_GadgetBase::MessageHandler( pMessage );
	}

	return true;
}

void UIColourPicker::MoveUp()
{
	S32 sRow, sCol;
	GetRowColFromPos( sRow, sCol );

	if ( sRow > 0 )
		sRow--;

	S32 sNumCols = NumCols( sRow );

	if ( sCol > ( sNumCols - 1 ) )
		sCol = sNumCols - 1;

	SetPosFromRowCol( sRow, sCol );
}

void UIColourPicker::MoveDown()
{
	S32 sRow, sCol;
	GetRowColFromPos( sRow, sCol );

	if ( sRow < ( m_sBoxSize - 1 ) )
		sRow++;

	S32 sNumCols = NumCols( sRow );

	if ( sCol > ( sNumCols - 1 ) )
		sCol = sNumCols - 1;

	SetPosFromRowCol( sRow, sCol );
}

void UIColourPicker::MoveLeft()
{
	S32 sRow, sCol;
	GetRowColFromPos( sRow, sCol );

	S32 sNumCols = NumCols( sRow );

	if ( sCol > 0 )
		sCol--;

	SetPosFromRowCol( sRow, sCol );
}

void UIColourPicker::MoveRight()
{
	S32 sRow, sCol;
	GetRowColFromPos( sRow, sCol );

	S32 sNumCols = NumCols( sRow );

	if ( sCol < ( sNumCols - 1 ) )
		sCol++;

	SetPosFromRowCol( sRow, sCol );
}

void UIColourPicker::MoveLeftV()
{
	if ( m_sPositionV > 0 )
		m_sPositionV--;
}

void UIColourPicker::MoveRightV()
{
	if ( m_sPositionV < ( m_sSizeV - 1 ) )
		m_sPositionV++;
}

void UIColourPicker::MoveLeftA()
{
	if ( m_sPositionA > 0 )
		m_sPositionA--;
}

void UIColourPicker::MoveRightA()
{
	if ( m_sPositionA < ( m_sSizeV - 1 ) )
		m_sPositionA++;
}

void UIColourPicker::GetRowColFromPos( S32& sRow, S32& sCol, S32 sPositionOverride )
{
	sRow = 0;
	sCol = 0;

	S32 sColCount = NumCols( sRow );

	for ( S32 i = 0; i < ( ( sPositionOverride == -1 ) ? m_sPosition : sPositionOverride ); i++ )
	{
		if ( sCol == ( sColCount - 1 ) )
		{
			sRow++;
			sCol = 0;
			sColCount = NumCols( sRow );
		}
		else
			sCol++;
	}
}

void UIColourPicker::SetPosFromRowCol( const S32 sRow, const S32 sCol )
{
	m_sPosition = 0;

	for ( S32 i = 0; i < sRow; i++ )
		m_sPosition += NumCols( i );

	m_sPosition += sCol;
}

void UIColourPicker::Render()
{
	if ( !m_Activated )
		return;

	S32 sBoxSizeOld = m_sBoxSize;
	S32 sSizeVOld = m_sSizeV;

	m_sBoxSize = C_UIColourPicker::Instance().BoxSize();
	m_sSizeV = C_UIColourPicker::Instance().SizeV();

	if ( ( m_sBoxSize != sBoxSizeOld ) || ( m_sSizeV != sSizeVOld ) )
	{
		m_sPosition = 0;
		m_sPositionV = m_sSizeV - 1;
		m_sPositionA = m_sSizeV - 1;
		m_fSegmentSize = m_Size.x / m_sBoxSize;
	}	

	static F32 fMultBacking = 1.5f;		// TODO - xt it!
	static F32 fMultPiece = 1.75f;		// TODO - xt it!
	static F32 fMultPreview = 2.25f;	// TODO - xt it!
	
	C_UIColour w = GetColour( m_sPosition, m_sSizeV - 1 );
	C_UIColour b = GetColour( m_sPosition, 0 );

	// auto-transform
	C_UIAutoTransform transform( this );

	C_GadgetBase::Render();

	F32 xxV = 0.0f;
	F32 yyV = 0.0f;
	C_UIColour cV = 0xffffffff;

	F32 xxA = 0.0f;
	F32 yyA = 0.0f;
	C_UIColour cA = 0xffffffff;

	LayerIndex layer1 = ( m_Layer == 0 ) ? 0 : m_Layer - 1;
	LayerIndex layer2 = ( m_Layer == 0 ) ? 1 : m_Layer;

	// picker
	for ( S32 i = 0; i < NumColours(); i++ )
	{
		S32 sRow, sCol;
		GetRowColFromPos( sRow, sCol, i );

		S32 sNumCols = NumCols( sRow );

		F32 y = m_Position.y;
		y += ( m_fSegmentSize * sRow ) * ( 1 - m_fSegmentMultY );

		F32 x = m_Position.x + ( m_Size.x / 2.0f );
		x += ( m_fSegmentSize * sNumCols * 0.5f ) * ( m_fSegmentMultX - 1 );
		x += ( m_fSegmentSize * sCol ) * ( 1 - m_fSegmentMultX );

		// background
		if ( ( sRow == 0 ) || ( sRow == ( m_sBoxSize - 1 ) ) || ( sCol == 0 ) || ( sCol == ( sNumCols - 1 ) ) )
		{
			gSimple2D.AddSprite( layer1, Maths::HVec3( x, y, m_Position.z ), 
				m_fSegmentSize * fMultBacking, m_fSegmentSize * fMultBacking, m_SpriteColours, m_Panel, E_SpriteType_Sprite, m_RenderState );
			gSimple2D.AddTexUVs( 0, GetTextureParam( 1 ) );
			gSimple2D.AddJustification( E_Justify_MiddleCentre );
		}

		if ( i == m_sPosition )
		{
			xxV = x;
			yyV = y;
			continue;
		}

		gSimple2D.AddSprite( layer2, Maths::HVec3( x, y, m_Position.z ), 
			m_fSegmentSize, m_fSegmentSize, GetColour( i, m_sSizeV - 1 ), m_Panel, E_SpriteType_Sprite, m_RenderState );
		gSimple2D.AddTexUVs( 0, GetTextureParam( 0 ) );
		gSimple2D.AddJustification( E_Justify_MiddleCentre );
	}

	// selection
	gSimple2D.AddSprite( layer2, Maths::HVec3( xxV, yyV, m_Position.z ), m_fSegmentSize * fMultPiece, m_fSegmentSize * fMultPiece,
		w, m_Panel, E_SpriteType_Sprite, m_RenderState );
	gSimple2D.AddTexUVs( 0, GetTextureParam( 0 ) );
	gSimple2D.AddJustification( E_Justify_MiddleCentre );

	// v picker
	for ( S32 j = 0; j < 2; j++ )
	{
		for ( S32 i = 0; i < m_sSizeV; i++ )
		{
			const F32 f = (F32)i / ( m_sSizeV - 1 );
			C_UIColour l;
			if( j == 0 )
			{
				l.Lerp( b, w, f );		
			}
			else
			{
				l = w;
				l.A01( f );
			}

			F32 y = m_Position.y + m_Size.x * ( 1 - m_fSegmentMultY ) + ( m_fSegmentSize * (j+1) );
			F32 x = m_Position.x + ( m_Size.x / 2.0f );
			x += ( m_fSegmentSize * m_sSizeV * 0.5f ) * ( m_fSegmentMultX - 1 );
			x += ( m_fSegmentSize * i ) * ( 1 - m_fSegmentMultX );

			if( i == m_sPositionV && j == 0 )
			{
				xxV = x;
				yyV = y;
				cV = l;
				continue;
			}
			if( i == m_sPositionA && j == 1 )
			{
				xxA = x;
				yyA = y;
				cA = l;
				continue;
			}
				
			gSimple2D.AddSprite( layer2, Maths::HVec3( x, y, m_Position.z ), m_fSegmentSize, m_fSegmentSize, 
				l, m_Panel, E_SpriteType_Sprite, m_RenderState );
			gSimple2D.AddTexUVs( 0, GetTextureParam( 0 ) );
			gSimple2D.AddJustification( E_Justify_MiddleCentre );
		}
	}

	// v selection
	gSimple2D.AddSprite( layer2, Maths::HVec3( xxV, yyV, m_Position.z ), m_fSegmentSize * fMultPiece, m_fSegmentSize * fMultPiece, 
		cV, m_Panel, E_SpriteType_Sprite, m_RenderState );
	gSimple2D.AddTexUVs( 0, GetTextureParam( 0 ) );
	gSimple2D.AddJustification( E_Justify_MiddleCentre );

	// a selection
	gSimple2D.AddSprite( layer2, Maths::HVec3( xxA, yyA, m_Position.z ), m_fSegmentSize * fMultPiece, m_fSegmentSize * fMultPiece, 
		cA, m_Panel, E_SpriteType_Sprite, m_RenderState );
	gSimple2D.AddTexUVs( 0, GetTextureParam( 0 ) );
	gSimple2D.AddJustification( E_Justify_MiddleCentre );

	// preview
	gSimple2D.AddSprite( layer2, m_Position, m_fSegmentSize * fMultPreview, m_fSegmentSize * fMultPreview, 
		GetColour( m_sPosition ), m_Panel, E_SpriteType_Sprite, m_RenderState );
	gSimple2D.AddTexUVs( 0, GetTextureParam( 0 ) );
	gSimple2D.AddJustification( E_Justify_MiddleCentre );
}

C_UIColour UIColourPicker::GetColour( const S32 sIndex, const S32 sVIndex, const bool bInvert )
{
	C_UIColour c = C_UIColourPicker::Instance().GetColour( sIndex, sVIndex );

	if ( bInvert )
	{
		c.R( 255 - c.R() );
		c.G( 255 - c.G() );
		c.B( 255 - c.B() );
	}

	return c;
}

C_UIColour UIColourPicker::GetColour( const S32 sIndex, const bool bInvert )
{
	return GetColour( sIndex, m_sPositionV, bInvert );
}

C_UIColour UIColourPicker::GetSelected()
{
	return GetColour( m_sPosition );
}

void UIColourPicker::SetColourIndex( S32 index )
{
	if( index > NumColours() )
		return;

	m_sPosition = index;
}

S32 UIColourPicker::GetColourIndex()
{
	return m_sPosition;
}

void UIColourPicker::SetIntensityIndex( S32 index )
{
	if( index >= m_sSizeV )
		return;

	m_sPositionV = index;
}

S32 UIColourPicker::GetIntensityIndex()
{
	return m_sPositionV;
}

void UIColourPicker::SetAlpha( F32 a )
{
	if( a >= m_sSizeV )
		return;

	m_sPositionA = (S32) ((F32)(m_sSizeV-1) * Maths::Clamp<F32>( a, 0.0f, 1.0f ));
}

F32 UIColourPicker::GetAlpha()
{
	return (F32)m_sPositionA / (F32)(m_sSizeV-1);
}

}; // namespace UI

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", on ) 
#endif
