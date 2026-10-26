#ifndef SIMPLE2D_INL
#define SIMPLE2D_INL

#ifdef _MSC_VER
#pragma once
#endif

#include "ui/Game/Gadgets/Buttons.h"

#include "UI/Shaders/Constants.h"

namespace UI
{

inline void C_Simple2D::SetClipSize2D( const F32 fClipWidth, const F32 fClipHeight )
{
	const F32 wBB = fClipWidth;
	const F32 hBB = fClipHeight;
#ifdef PLATFORM_PC
	const F32 wUI = m_ScreenWidth;	// [Blur2PC] retail 0x00799710
#else
	const F32 wUI = m_WideScreen ? UI_VSCREEN_SIZEX_WIDE : UI_VSCREEN_SIZEX;
#endif
	const F32 hUI = UI_VSCREEN_SIZEY;

	m_fClipWidth = fClipWidth;
	m_fClipHeight = fClipHeight;
	m_fNearZ = 0.0f;
	m_fFarZ = 1500.0f;

	const F32 transformUI[ 4 ] =
	{
		wBB / wUI,											// scale x
		hBB / hUI,											// scale y
		( wUI - UI_VSCREEN_SIZEX ) * ( wBB / wUI ) * 0.5f,	// translate x
		0.0f,												// translate y
	};

	// calculate default 2D transform matrix
	CalcTransform2D( transformUI, m_fClipWidth, m_fClipHeight, m_fNearZ, m_fFarZ, m_TransformMatrix2D );

	// set 2D as default
	m_bUsing2D = true;
	m_TransformMatrix = m_TransformMatrix2D; 
	m_ViewPosition = Maths::HVec4( 0.0f, 0.0f, 0.0f, 0.0f );
}

inline void C_Simple2D::SetTransformMatrix2D()
{ 
	m_bUsing2D = true;
	m_TransformMatrix = m_TransformMatrix2D; 
	m_ViewPosition = Maths::HVec4( 0.0f, 0.0f, 0.0f, 0.0f );
	SetTransformMatrixShaderConstant();
}

inline void C_Simple2D::SetTransformMatrix2D( const Maths::HMat44& transformMatrix )
{ 
	m_bUsing2D = true;
	m_TransformMatrix = transformMatrix;
	m_ViewPosition = Maths::HVec4( 0.0f, 0.0f, 0.0f, 0.0f );
	SetTransformMatrixShaderConstant();
}

inline void C_Simple2D::SetTransformMatrix3D( const Maths::HMat44& transformMatrix, const Maths::HVec3& viewPosition )
{
	// this assumes you are setting a 3D transform matrix - so we can zero the z in 2D else it makes the transform go screwy
	m_bUsing2D = false;
	m_TransformMatrix = transformMatrix;
	m_ViewPosition = Maths::HVec4( viewPosition.X(), viewPosition.Y(), viewPosition.Z(), 0.0f );
	SetTransformMatrixShaderConstant();
}

inline void C_Simple2D::SetTransformMatrixShaderConstant()
{
	Display::C_Matrix transformMatrix;
	Maths::Store(m_TransformMatrix, &transformMatrix);
	Display::Device()->SetVertexShaderConstantF( CV_UI_TRANSFORM, (F32*)&transformMatrix, 4 );

	Display::C_Float4 viewPosition;
	Maths::Store(m_ViewPosition, &viewPosition);
	Display::Device()->SetVertexShaderConstantF( CV_UI_VIEW_POSITION, (F32*)&viewPosition, 1 );

#ifdef PLATFORM_PC
	//m_TransformMatrix.Transpose();	// this fixes PC rendering for now - but meh
#endif
	//Display::Device()->SetVertexShaderConstantF( CV_UI_TRANSFORM, (F32*)&m_TransformMatrix, 4 );
}

inline void C_Simple2D::CalcTransform2D( const F32* transformUI, const F32 fClipWidth, const F32 fClipHeight, const F32 fNear, const F32 fFar, Maths::HMat44& dest )
{
	// UI -> backbuffer
	Maths::HMat43 ui( Maths::C_MatrixScale( transformUI[ 0 ], transformUI[ 1 ], 1.0f ) );
	ui.Translation( Maths::HVec3( transformUI[ 2 ], transformUI[ 3 ], 0.0f ) );
	Maths::HMat44 uiMatrix( ui );

	// backbuffer -> device
	Maths::C_Rect shaderBounds( Maths::HVec2( -1.0f, 1.0f ), Maths::HVec2( 1.0f, -1.0f ) );
	Maths::C_Transform2d shaderTransform = Maths::C_Transform2d( shaderBounds );

	Maths::C_Rect view( Maths::HVec2( 0, 0 ), Maths::HVec2( fClipWidth, fClipHeight ) );
	Maths::C_Transform2d viewInv = Maths::C_Transform2d( view );
	viewInv.Invert();

	Maths::C_Transform2d deviceTransform = shaderTransform * viewInv;

	Maths::HMat43 device( Maths::C_MatrixScale( deviceTransform.Scale().X(), deviceTransform.Scale().Y(), 1.0f ) );
	device.Translation( Maths::HVec3( deviceTransform.Offset().X(), deviceTransform.Offset().Y(), 0.0f ) );
	Maths::HMat44 deviceMatrix( device );

	// done
	dest = uiMatrix * deviceMatrix;
	dest.AZ(Maths::HVec4(dest.AZ().X(), dest.AZ().Y(), 1.0f/(fFar - fNear), 0.0f));
	dest.AW(Maths::HVec4(dest.AW().X(), dest.AW().Y(), fNear/(fNear - fFar), 1.0f));
}
 
inline void C_Simple2D::AddWideScreenBorders()
{
	C_UIColour col( 0xff000000 );

	ResetClipWindow( gSimple2D.TopLayer(), 0 );

	AddSprite(gSimple2D.TopLayer(),
		Maths::HVec3( -150, -(UI_VSCREEN_SIZEY*0.125f)-64, 0.0f ),
		UI_VSCREEN_SIZEX+300,
		(UI_VSCREEN_SIZEY*0.125f)+64,
		0xff000000);
	AddTexUVs(0,gUITextureManager.GetTextureByName("null"),0,0,1,1);

	AddSprite(gSimple2D.TopLayer(),
		Maths::HVec3( -150, UI_VSCREEN_SIZEY, 0.0f ),
		UI_VSCREEN_SIZEX+300,
		(UI_VSCREEN_SIZEY*0.125f)+64,
		0xff000000);
	AddTexUVs(0,gUITextureManager.GetTextureByName("null"),0,0,1,1);
}

inline const F32 C_Simple2D::ToScreenX( const F32 x )
{
	if(m_WideScreen)
		return x * UI_VSCREEN_SIZEX_WIDE - ((UI_VSCREEN_SIZEX_WIDE-UI_VSCREEN_SIZEX)*0.5f);

	return x * UI_VSCREEN_SIZEX;
}

inline const F32 C_Simple2D::ToScreenY( const F32 y )
{
	return y * UI_VSCREEN_SIZEY;
}

inline void C_Simple2D::ResetClipWindow( const LayerIndex layer, const PanelIndex panel )
{
	AddSprite( layer, Maths::HVec3( ScreenLeft(), 0.0f, 0.0f ), ScreenWidth(), UI_VSCREEN_SIZEY, 0x00000000, panel, E_SpriteType_SetClipWindow );
}


// [Blur2PC][x64] Clear a newly allocated sprite. Retail clears its 128-byte cache line
// (DCBZ128), which is exactly the sprite on x86; on x64 C_Sprite is 136 bytes, so the
// aligned line would also clear the end of the previous sprite.
#if defined(_WIN64)
#define UI_CLEAR_SPRITE(p) memset( (p), 0, sizeof(C_Sprite) )
#else
#define UI_CLEAR_SPRITE(p) DCBZ128( 0, (p) )
#endif

inline void C_Simple2D::ValidatePanelAndLayer( LayerIndex& uLayer, PanelIndex& uPanel )
{
	// Check Layer is valid
	if ( uLayer >= LayerCount() )
	{
		LOGWARN( g_Log, DBSTR "Invalid sprite layer ( %d )", uLayer );
		uLayer = TopLayer();
	}
	// Check Panel is valid
	if ( uPanel >= PanelCount() )
	{
		LOGWARN( g_Log, DBSTR "Invalid sprite panel ( %d )", uPanel );
		uPanel = PanelCount()-1;
	}
}

inline void C_Simple2D::AddSprite( LayerIndex uLayer, PanelIndex uPanel, const Display::C_StencilState& state )
{
	// Check panel and layer are valid. Otherwise clamp the values.
	ValidatePanelAndLayer( uLayer, uPanel );

	if (m_SpriteCount >= m_MaxSprites)
		return;

	C_Sprite *p = (m_Sprites + m_SpriteCount);
	UI_CLEAR_SPRITE( p );

	m_LayerPanelSprites[ uLayer ][ uPanel ].PushBack( p );

	p->m_Layer				= uLayer;
	p->m_Panel				= uPanel;
	p->m_Type				= E_SpriteType_SetStencilBuffer;
	p->m_Count				= 1;

	p->StencilState( state );

	m_ePreviousSpriteType	= E_SpriteType_SetStencilBuffer;
	m_SpriteCount++;
}

inline void C_Simple2D::AddSprite( LayerIndex uLayer, PanelIndex uPanel, const E_SpriteType type, const INTPTR userData )
{
	if( type == E_SpriteType_Sprite || type == E_SpriteType_TriList || type == E_SpriteType_TriStrip || type == E_SpriteType_Model )
	{
		LOGERROR( g_Log, DBSTR "Invalid control sprite type" );
		return;
	}

	// Check panel and layer are valid. Otherwise clamp the values.
	ValidatePanelAndLayer( uLayer, uPanel );

	if (m_SpriteCount >= m_MaxSprites)
		return;

	C_Sprite *p = (m_Sprites + m_SpriteCount);
	UI_CLEAR_SPRITE( p );

	m_LayerPanelSprites[ uLayer ][ uPanel ].PushBack( p );

	p->m_Layer				= uLayer;
	p->m_Panel				= uPanel;
	p->m_Type				= type;
	p->m_Count				= 1;
	p->m_UserData			= userData;

	m_ePreviousSpriteType	= type;
	m_SpriteCount++;
}

inline void C_Simple2D::AddSprite( LayerIndex uLayer, const Maths::HVec3& pos, const MathsStorage::Float3& size, const C_UISpriteColours& colours, PanelIndex uPanel, const E_SpriteType type, const C_RenderState& renderState, const INTPTR uUserData )
{
	// Check panel and layer are valid. Otherwise clamp the values.
	ValidatePanelAndLayer( uLayer, uPanel );

	if (m_SpriteCount >= m_MaxSprites)
		return;

	C_Sprite * __restrict p = (m_Sprites + m_SpriteCount);
	UI_CLEAR_SPRITE( p );

	p->m_Size				= size;
	p->m_Colours			= colours;
	p->m_Layer				= uLayer;
	p->m_Panel				= uPanel;
	p->m_Type				= type;
	p->m_RenderState		= renderState;
	p->m_UserData			= uUserData;
	p->m_Count				= 1;

	// m_TransformAutoOffset is (0,0,0) when m_TransformIndexAuto is 0.
	p->m_PostTransformIndex = m_TransformIndexAuto;
	Maths::Store<Maths::HVec3>( pos + m_TransformAutoOffset, &p->m_Pos );

	m_LayerPanelSprites[ uLayer ][ uPanel ].PushBack( p );

	m_ePreviousSpriteType = type;
	m_SpriteCount++;
}

inline void C_Simple2D::AddSprite( LayerIndex uLayer, const Maths::HVec3& pos, const F32 fSizeX, const F32 fSizeY, const C_UISpriteColours& colours, PanelIndex uPanel, const E_SpriteType type, const C_RenderState& renderState, const INTPTR uUserData )
{
	// Check panel and layer are valid. Otherwise clamp the values.
	ValidatePanelAndLayer( uLayer, uPanel );

	if (m_SpriteCount >= m_MaxSprites)
		return;

	C_Sprite * __restrict p = (m_Sprites + m_SpriteCount);
	UI_CLEAR_SPRITE( p );

	p->m_Size.x				= fSizeX;
	p->m_Size.y				= fSizeY;
	p->m_Colours			= colours;
	p->m_Layer				= uLayer;
	p->m_Panel				= uPanel;
	p->m_Type				= type;
	p->m_RenderState		= renderState;
	p->m_UserData			= uUserData;
	p->m_Count				= 1;

	// m_TransformAutoOffset is (0,0,0) when m_TransformIndexAuto is 0.
	p->m_PostTransformIndex = m_TransformIndexAuto;
	Maths::Store<Maths::HVec3>( pos + m_TransformAutoOffset, &p->m_Pos );

	m_LayerPanelSprites[ uLayer ][ uPanel ].PushBack( p );

	m_ePreviousSpriteType = type;
	m_SpriteCount++;
}

inline void C_Simple2D::AddTexUVs(const TextureStage stage, const C_TextureParam& param)
{
/*
	if (m_ePreviousSpriteType != E_SpriteType_Sprite && m_ePreviousSpriteType != E_SpriteType_TriStrip && m_ePreviousSpriteType != E_SpriteType_TriList)
		return;

	if (m_pRenderer->ShaderUsage( (E_Effect)CurrentSprite( )->m_RenderState.m_EffectIndex ) & E_ShaderUsageFlags_NotTextured)
		return;
*/

	assert(stage<C_Sprite::m_NumTextureStages);

	C_Simple2DTexture* __restrict p = &CurrentSprite( )->m_Textures[stage];

	p->m_Texture			= param.m_TextureID;
	p->m_pu					= param.m_Position.u;
	p->m_pv					= param.m_Position.v;
	p->m_su					= param.m_Size.u;
	p->m_sv					= param.m_Size.v;
}

inline void C_Simple2D::AddTexUVs(const TextureStage stage, const C_TextureHandleIndex texture, const F32 u, const F32 v, const F32 sizeu, const F32 sizev)
{
/*
	if (m_ePreviousSpriteType != E_SpriteType_Sprite && m_ePreviousSpriteType != E_SpriteType_TriStrip && m_ePreviousSpriteType != E_SpriteType_TriList)
		return;

	if (m_pRenderer->ShaderUsage( (E_Effect)CurrentSprite( )->m_RenderState.m_EffectIndex ) & E_ShaderUsageFlags_NotTextured)
		return;
*/

	assert(stage<C_Sprite::m_NumTextureStages);

	C_Simple2DTexture* __restrict p = &CurrentSprite( )->m_Textures[stage];

	p->m_Texture			= texture;
	p->m_pu					= u;
	p->m_pv					= v;
	p->m_su					= sizeu;
	p->m_sv					= sizev;
}

inline void C_Simple2D::AddRotation(const F32 degX, const F32 degY, const F32 degZ)
{
/*
	if ( m_ePreviousSpriteType != E_SpriteType_Sprite && m_ePreviousSpriteType != E_SpriteType_TriList && m_ePreviousSpriteType != E_SpriteType_TriStrip && m_ePreviousSpriteType != E_SpriteType_Model )
		return;
*/

	if(degX == 0.0f && degY == 0.0f && degZ == 0.0f)
		return;

	C_Sprite* __restrict p = CurrentSprite( );

	if( !p->m_PreTransformIndex )
		p->m_PreTransformIndex = PopPreTransformMatrix();

	Maths::HMat43& mat(m_PreTransformMatrixPool[ p->m_PreTransformIndex ]);

	mat = mat * Maths::HMat43(Maths::C_MatrixRotateRPH( Maths::DegToRad( degZ ), Maths::DegToRad( degX ), Maths::DegToRad( degY ) ));
}

inline void C_Simple2D::AddScale(const F32 x, const F32 y, const F32 z)
{
	if(x == 1.0f && y == 1.0f)
		return;

	C_Sprite* __restrict p = CurrentSprite( );

	// Get matrix
	if( !p->m_PreTransformIndex )
		p->m_PreTransformIndex = PopPreTransformMatrix();

	Maths::HMat43& mat(m_PreTransformMatrixPool[ p->m_PreTransformIndex ]);
	Maths::HMat43 scale(Maths::C_MatrixScale(Maths::HVec3( x, y, z )));

	mat = mat * scale;
}

inline void C_Simple2D::AddPivot(const F32 x, const F32 y, const F32 z)
{
	if(x==0.0f && y == 0.0f && z == 0.0 )
		return;

	AddPivot(Maths::HVec3(x,y,z));
}

inline void C_Simple2D::AddPivot(const Maths::HVec3& trans)
{
	// Get matrix
	C_Sprite* __restrict p = CurrentSprite( );

	if( !p->m_PreTransformIndex )
		p->m_PreTransformIndex = PopPreTransformMatrix();

	Maths::HMat43& mat(m_PreTransformMatrixPool[ p->m_PreTransformIndex ]);

	mat.Translation( mat.Translation() - trans );
}

inline void C_Simple2D::AddOffsetPivot(const F32 x, const F32 y, const F32 z)
{
	Maths::HVec3 pos = Maths::Load<Maths::HVec3>( &CurrentSprite()->m_Pos );
	AddPivot(Maths::HVec3(x,y,z) - pos);
}

inline void C_Simple2D::AddOffsetPivotNeg(const F32 x, const F32 y, const F32 z)
{
	Maths::HVec3 pos = Maths::Load<Maths::HVec3>( &CurrentSprite()->m_Pos );
	AddPivot(pos - Maths::HVec3(x,y,z));
}

inline void C_Simple2D::AddJustification(const E_Justify justify)
{
	CurrentSprite( )->m_Justify = justify;
}

// Pop off the index in the pre transform matrix stack
inline const TransformIndex C_Simple2D::PopPreTransformMatrix()
{
	if( m_PreTransformPoolIndex == m_PreTransformMatrixPool.Size() - 1 )
	{
		LOGWARN( g_Log, DBSTR "PreMatrixPoolSize exceeded");
		return m_PreTransformPoolIndex;
	}

	m_PreTransformPoolIndex++;
	m_PreTransformMatrixPool[ m_PreTransformPoolIndex ].SetIdentity();

	return m_PreTransformPoolIndex;
}

// Pop off the index in the post transform matrix stack
inline const TransformIndex C_Simple2D::PopPostTransformMatrix( )
{
	if( m_PostTransformPoolIndex == m_PostTransformMatrixPool.Size() )
	{
		LOGWARN( g_Log, DBSTR "PostMatrixPoolSize exceeded");
		return m_PostTransformPoolIndex;
	}

	m_PostTransformPoolIndex++;

	// check for auto-transforming
	if ( m_TransformIndexAuto )
		m_PostTransformMatrixPool[ m_PostTransformPoolIndex ] = m_PostTransformMatrixPool[ m_TransformIndexAuto ];
	else
		m_PostTransformMatrixPool[ m_PostTransformPoolIndex ].SetIdentity();

	return m_PostTransformPoolIndex;
}

inline const TransformIndex C_Simple2D::PopColourTransformMatrix()
{
#if 0
	assert(m_ColourMatrixPoolIndex < m_ColourMatrixPoolSize);

	m_ColourMatrixPool[ m_ColourMatrixPoolIndex ].SetIdentity();

	return m_ColourMatrixPoolIndex++;
#else
	m_ColourMatrixPool[ m_ColourMatrixPoolIndex ].SetIdentity();
	if(m_ColourMatrixPoolIndex < (U32)m_ColourMatrixPool.Size()-1)
	{
		// TODO
		return m_ColourMatrixPoolIndex++;
	}

	m_ColourMatrixPool[ m_ColourMatrixPoolIndex ].SetIdentity();
	return m_ColourMatrixPoolIndex;
#endif
}

inline Maths::HMat43& C_Simple2D::GetPreTransformMatrix( const TransformIndex matrix_index, const bool reserved )
{ 
	if( !reserved )
	{
		if( matrix_index == 0 || matrix_index == 1 )
		{
			LOGWARN( g_Log, DBSTR "Attempt to access reserved pre transform");
			return m_PreTransformMatrixPool[ 2 ]; 
		}
	}

	return m_PreTransformMatrixPool[ matrix_index ];
}

inline Maths::HMat43& C_Simple2D::GetPostTransformMatrix( const TransformIndex matrix_index, const bool reserved )
{ 
	if( !reserved )
	{
		if( matrix_index <= 0 )
		{
			LOGWARN( g_Log, DBSTR "Attempt to access reserved post transform");
			return m_PostTransformMatrixPool[ 1 ];
		}
	}
	return m_PostTransformMatrixPool[ matrix_index ];
}

inline Maths::HMat43& C_Simple2D::GetColourTransformMatrix( const TransformIndex matrix_index )
{ 
	if( matrix_index == 0 )
	{
		LOGWARN( g_Log, DBSTR "Attempt to access reserved colour transform");
		return m_ColourMatrixPool[ 1 ];
	}

	return m_ColourMatrixPool[ matrix_index ]; 
}


inline const Maths::HVec3 C_Simple2D::GetJustificationOffset( const E_Justify type, const PanelIndex panel )
{
	return IsPanel3D( panel ) ? m_JustificationOffsets_Flipped[ type ] : m_JustificationOffsets[ type ];
}

inline const Maths::HVec3& C_Simple2D::GetJustifiedCorner( const E_Justify type, const U32 corner, const PanelIndex panel )
{ 
	return GetJustifiedCorner( type, corner, IsPanel3D( panel ) ); 
}

inline const Maths::HVec3& C_Simple2D::GetJustifiedCorner( const E_Justify type, const U32 corner, const bool flipped )
{
	return flipped ? m_JustifiedCorner_Flipped[ type ][ corner ] : m_JustifiedCorner[ type ][ corner ]; 
}

inline void C_Simple2D::PixelBoundary( const bool doX, const bool doY, F32& x, F32& y, F32& sizeX, F32& sizeY )
{
	if( doX )
	{
		const F32 screen_res_x = UI_VSCREEN_SIZEX_WIDE / Display::Device()->GetBackBuffer().GetWidth();

		// Scale to pixel resolution of the screen
		x		/= screen_res_x;
		sizeX	/= screen_res_x;

		// Align to pixel boundary
		x		= floorf(x+0.5f);
		sizeX	= floorf(x+sizeX+0.5f) - x;

		// Scale back to the resolution of the UI
		x		*= screen_res_x;
		sizeX	*= screen_res_x;
	}

	if( doY )
	{
		const F32 screen_res_y = UI_VSCREEN_SIZEY / Display::Device()->GetBackBuffer().GetHeight();

		// Scale to pixel resolution of the screen
		y		/= screen_res_y;
		sizeY	/= screen_res_y;

		// Align to pixel boundary
		y		= floorf(y+0.5f);
		sizeY	= floorf(y+sizeY+0.5f) - y;

		// Scale back to the resolution of the UI
		y		*= screen_res_y;
		sizeY	*= screen_res_y;
	}
}

inline void C_Simple2D::SetZEnable( const bool enable )
{
	m_DefaultStackedRenderState.m_DepthEnabled = (enable ? Display::E_Bool_True : Display::E_Bool_False); 
	SetDepthBufferState( m_DefaultStackedRenderState );
}

inline void C_Simple2D::SetCurrentStackedRenderState()
{
	const C_StackedRenderState& current = CurrentStackedRenderState();

	SetCullMode( current );
	SetClipWindow( current );
	SetColourWriteEnable( current );
	SetDepthBufferState( current );
	SetStencilBufferState( current );
}

inline void C_Simple2D::SetAlphaTestEnable( const Display::E_Bool bEnable )
{
	Display::Device()->SetAlphaTestEnable( bEnable );
}

inline void C_Simple2D::SetCullMode( const C_StackedRenderState& set )
{
#if defined( PLATFORM_PS3 )
	Display::Device()->SetCullMode( Display::FlippedCullMode( set.m_CullMode ) ); // DG - Waaaah?
#else
	Display::Device()->SetCullMode( set.m_CullMode );
#endif
}

inline void C_Simple2D::SetClipWindow( const C_StackedRenderState& set )
{
	if( set.m_ClipEnabled == Display::E_Bool_True )
	{
#if 1
		const Display::C_Viewport& viewport = Display::Device()->GetViewport();
		U32 uWidth = viewport.GetW();
		U32 uHeight = viewport.GetH();
#else
		const Display::C_RenderTarget& rt = Display::Device()->GetRenderTarget();
		U32 uWidth = rt.GetColourWidth();
		U32 uHeight = rt.GetColourHeight();
#endif

		// Scale, Rot, Trans
		Maths::HMat43 bb_matrix( Maths::HMat33(Maths::C_MatrixScale( Maths::HVec3( (F32)uWidth * 0.5f, (F32)uHeight *-0.5f, 0.0f ) )), Maths::HVec3( (F32)uWidth * 0.5f, (F32)uHeight * 0.5f, 0.0f ) );

		Maths::HVec3 clipPos( set.m_ClipPos.vx, set.m_ClipPos.vy, 0.0f );
		Maths::HVec3 clipEnd( set.m_ClipSize.vx + set.m_ClipPos.vx, set.m_ClipSize.vy + set.m_ClipPos.vy, 0.0f );

		clipPos = clipPos * m_TransformMatrix;
		clipEnd = clipEnd * m_TransformMatrix;

		clipPos = clipPos * bb_matrix;
		clipEnd = clipEnd * bb_matrix;

		F32 px = floor( clipPos.X() + 0.5f );
		F32 py = floor( clipPos.Y() + 0.5f );

		F32 ex = floor( clipEnd.X() + 0.5f );
		F32 ey = floor( clipEnd.Y() + 0.5f );

		F32 clip_px = Maths::Clamp<F32>( px, 0, (F32)uWidth );
		F32 clip_py = Maths::Clamp<F32>( py, 0, (F32)uHeight );
		F32 clip_sx = Maths::Clamp<F32>( (ex - px), 0, (F32)uWidth );
		F32 clip_sy = Maths::Clamp<F32>( (ey - py), 0, (F32)uHeight );

		Display::Device()->SetScissorRect( (U32)clip_px, (U32)clip_py, (U32)clip_sx, (U32)clip_sy );
		Display::Device()->SetScissorTest( Display::E_Bool_True );
	}
	else
	{
		Display::Device()->SetScissorTest( Display::E_Bool_False );
	}
}

inline void C_Simple2D::SetColourWriteEnable( const C_StackedRenderState& set )
{
	Display::Device()->SetColourWriteEnable( set.m_ColourWriteEnable );
}

inline void C_Simple2D::SetDepthBufferState( const C_StackedRenderState& set )
{
	Display::Device()->SetDepthEnable( set.m_DepthEnabled );
	Display::Device()->SetDepthWriteEnable( set.m_DepthWriteEnabled );
	Display::Device()->SetDepthFunc( Display::Device()->GetDepthFlip() ? Display::FlippedCmpFunc(set.m_DepthCompareFunc) : set.m_DepthCompareFunc ); // DG - Remove this flip test when it's abstracted in Display by Lloyd
}

inline void C_Simple2D::SetStencilBufferState( const C_StackedRenderState& set )
{
	if( set.m_StencilState.m_StencilEnable == Display::E_Bool_True && Display::Device()->GetRenderTarget().GetDepthStencilSurface() == NULL )
		return;

	Display::Device()->SetStencilState( set.m_StencilState );
}

}

#endif // SIMPLE2D_INL
