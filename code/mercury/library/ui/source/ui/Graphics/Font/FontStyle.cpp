#include "stdafx.h"

#include "ui/Graphics/Font/FontStyle.h"
#include "ui/Graphics/Font/UIPolygonFont.h"
#include "ui/Game/UiXt.h"


// Illustrator shape stuff.
#include "ui/Illustrator/UIAdobeShape.h"
#include "ui/Illustrator/UIAdobeSceneManager.h"


#ifdef CFG_ALLOW_OPTIMIZE_OFF_UI
#pragma optimize("", off)
#endif

XT_TYPE( UI::E_XtFontShader, "UIFontShader" )
	XT_OPTION( UI::E_XtFontShader_MultiTexture, "TextureWithAlpha" )
	XT_OPTION( UI::E_XtFontShader_IntensityAsAlpha, "IntensityAsAlpha" )
	XT_OPTION( UI::E_XtFontShader_PolygonFont, "Polygon" )
	XT_OPTION( UI::E_XtFontShader_PolygonFontOutlineFixed, "PolygonOutlineFixed" )
	XT_OPTION( UI::E_XtFontShader_PolygonFontOutlineFade, "PolygonOutlineFade" )
	XT_OPTION( UI::E_XtFontShader_PolygonFontOutlineFixedFade, "PolygonOutlineFixedFade" )
	XT_OPTION( UI::E_XtFontShader_PolygonFontTextured, "PolygonTextured" )
	XT_OPTION( UI::E_XtFontShader_PolygonFontGradient, "PolygonGradient" )
	XT_OPTION( UI::E_XtFontShader_PolygonFontGradient_Param, "PolygonGradientParam" )
	XT_OPTION( UI::E_XtFontShader_PolygonFontGradient_Radial, "PolygonGradientRadial" )
	XT_OPTION( UI::E_XtFontShader_PolygonFontFinned, "PolygonFinned" )
	XT_OPTION( UI::E_XtFontShader_PolygonFontFinnedOuter, "PolygonFinnedOuter" )
XT_END_TYPE

XT_TYPE( UI::C_FontStyle, "UIFontStyle" )
	XT_CALLBACK( UI::C_FontStyle::SorceryCallback )
	XT_FIELD( m_Name, "style_name" )
	XT_FIELD( m_FontName, "font_name" )
	XT_FIELD( m_uFont, "font" )
	XT_FIELD( m_fTrackingOverride, "tracking" )
	XT_FIELD( m_fHeightPercentage, "height_percentage" )
	XT_FIELD( m_bItalics, "italics" )
	XT_FIELD( m_bPixelBoundaryX, "pixel_boundary_x" )
	XT_FIELD( m_bPixelBoundaryY, "pixel_boundary_y" )
	XT_FIELD( m_RenderPassInfo, "render_passes" )
XT_END_TYPE

XT_TYPE( UI::C_FontStylePasses, "UIFontStylePasses" )
	XT_FIELD( m_StyleName, "name" )
XT_END_TYPE

XT_TYPE( UI::C_FontStyleBaseRenderInfo, "UIFontStyleDefault" )
	XT_FIELD( m_ColourStyle, "colour_style" )
	XT_FIELD( m_Alpha, "alpha" )
	XT_FIELD( m_TextureNames, "texture_names" )
	XT_FIELD( m_FontOverideName, "overide_name" )
	XT_FIELD( m_EffectIndex, "render_type" )
	XT_FIELD( m_BlendMode, "blend_mode" )
	XT_FIELD( m_fOffsetX, "offsetx" )
	XT_FIELD( m_fOffsetY, "offsety" )
	XT_FIELD( m_fOffsetZ, "offsetz" )
	XT_FIELD( m_bOffsetProportional, "offset_proportional" )
XT_END_TYPE

XT_TYPE( UI::C_FontStylePolygonOutline, "UIFontStylePolygonOutline" )
	XT_BASE( UI::C_FontStyleBaseRenderInfo )
	XT_FIELD( m_fOutlineAmount, "outline_amount" )
XT_END_TYPE

XT_TYPE( UI::C_FontStylePolygon3D, "UIFontStylePolygon3D" )
	XT_BASE( UI::C_FontStyleBaseRenderInfo )
	XT_FIELD( m_fExtrudeAmount, "extrude_amount" )
XT_END_TYPE

XT_TYPE( UI::C_FontStylePolygonOutline3D, "UIFontStylePolygonOutline3D" )
	XT_BASE( UI::C_FontStyleBaseRenderInfo )
	XT_FIELD( m_fOutlineAmount, "outline_amount" )
	XT_FIELD( m_fExtrudeAmount, "extrude_amount" )
XT_END_TYPE

XT_TYPE( UI::C_FontStylePolygonGradient, "UIFontStylePolygonGradient" )
	XT_BASE( UI::C_FontStyleBaseRenderInfo )
	XT_FIELD( m_uGradientIndex, "GradientIndex" )
	XT_FIELD( m_fGradientAngle, "GradientAngle" )
XT_END_TYPE

XT_TYPE( UI::C_FontStylePolygonBevel3D, "UIFontStylePolygonBevel3D" )
	XT_BASE( UI::C_FontStyleBaseRenderInfo )
	XT_FIELD( m_fOutlineAmount, "outline_amount" )
	XT_FIELD( m_fExtrudeAmount, "extrude_amount" )
	XT_FIELD( m_fExtrudeAmountBevel, "extrude_amount_bevel" )
XT_END_TYPE

namespace UI
{
// ----------------------
// C_FontStyleTextureInfo
// ----------------------
bool C_FontStyleTextureInfo::GetUVFromMode( const C_FontSpriteItem& item, F32& _u, F32& _v, F32& _usize, F32& _vsize )
{
	switch ( m_eMode )
	{
		case C_FontStyleTextureInfo::E_FontMappingMode_AsLetter:	// copy UVs from letter texture
			_u		= item.m_T0_px;
			_v		= item.m_T0_py;
			_usize	= item.m_T0_sx;
			_vsize	= item.m_T0_sy;
			break;

		case C_FontStyleTextureInfo::E_FontMappingMode_ZeroToOne:	// map [UV] = 0 [UV]size = 1
			_u		= 0;
			_v		= 0;
			_usize	= 1.0f;
			_vsize	= 1.0f;
			break;

		case C_FontStyleTextureInfo::E_FontMappingMode_Shifted:	// as RndShifted without the randomness
			_u		= item.m_T0_px;
			_v		= 0;
			_usize	= item.m_Sx / item.m_Sy;
			_vsize	= 1;
			break;

		case C_FontStyleTextureInfo::E_FontMappingMode_RndShifted:	// as 01 but shifted in the U random-ish (based on screen coords)
			_u		= item.m_Pos.X() + item.m_T0_px;
			_v		= 0;
			_usize	= item.m_Sx / item.m_Sy;
			_vsize	= 1;
			break;

		default:
			return false;	// not a valid mode
	}

	return true;
}

// -------------------
// C_FontStyleTextures
// -------------------
bool C_FontStyleTextures::GetFromLua( lua_State* pState )
{
	// Check for any extra textures
	if ( Lua::DataMemberType( pState, "textures" ) != LUA_TTABLE )
		return false;

	lua_pushstring( pState, "textures" );
	lua_gettable( pState, -2 );

	U32 uNumTextures = (U32)luaL_getn( pState,-1 );

	if ( uNumTextures > m_uMaxTextures )
	{
		LOGWARN( g_Log, DBSTR "Too many font textures have been given, only the first %d will be used", m_uMaxTextures );
		uNumTextures = m_uMaxTextures;
	}

	for ( U32 i = 1; i <= uNumTextures; i++ )
	{
		lua_rawgeti( pState, -1, i );	// push on index i

		if ( lua_type( pState, - 1) == LUA_TTABLE )	// type check
		{
			C_FontStyleTextureInfo info;
			S32 sTemp;

			if ( Lua::DataMemberType( pState, "mode" ) == LUA_TNUMBER )
			{
				Lua::GetDataMember( pState, "mode", sTemp );
				m_Textures[ i - 1 ].m_eMode = (C_FontStyleTextureInfo::E_FontMappingMode)sTemp;
			}

			if ( Lua::DataMemberType( pState, "name" ) == LUA_TNUMBER )
			{
				Lua::GetDataMember( pState, "name", sTemp );
				m_Textures[ i - 1 ].m_Texture = sTemp;
			}
		}

		lua_pop( pState, 1 ); // pop  off index i
	}

	lua_pop( pState, 1 );	// pop off the textures table

	return true;
}

// -------------------------
// C_FontStyleBaseRenderInfo
// -------------------------
bool C_FontStyleBaseRenderInfo::GetFromLua( lua_State* pState )
{
	// PR - TEMP - we should move the default font style into Sorcery really ...
	m_RenderState.m_EffectIndex = E_Effect_IntensityAsAlpha;

	// additional font textures
	m_FontTextures.GetFromLua( pState );

	if ( Lua::DataMemberType( pState, "colour" ) == LUA_TSTRING )
	{
		// TODO - four colour override?

		C_UIColour col;
		const char* pString;
		Lua::GetDataMember( pState, "colour", pString );
		col.Set( pString );

		m_ColourOverride = col;
		m_bUseColourOverride = true;
	}

	if ( Lua::DataMemberType( pState, "offset" ) == LUA_TNUMBER )
	{
		Lua::GetDataMember( pState, "offset", m_fOffsetX );
		m_fOffsetZ = m_fOffsetY = m_fOffsetX;
	}
	else
	{
		if ( Lua::DataMemberType( pState, "offsetx" ) == LUA_TNUMBER )
			Lua::GetDataMember( pState, "offsetx", m_fOffsetX );

		if ( Lua::DataMemberType( pState, "offsety" ) == LUA_TNUMBER )
			Lua::GetDataMember( pState, "offsety", m_fOffsetY );

		if ( Lua::DataMemberType( pState, "offsetz" ) == LUA_TNUMBER )
			Lua::GetDataMember( pState, "offsetz", m_fOffsetZ );
	}

	if ( Lua::DataMemberType( pState, "offset_proportional" ) == LUA_TBOOLEAN )
		Lua::GetDataMember( pState, "offset_proportional", m_bOffsetProportional );

	// polygon font override
	if ( Lua::DataMemberType( pState, "polygon_font" ) == LUA_TNUMBER )
	{
		S32 sTemp;
		Lua::GetDataMember( pState, "polygon_font", sTemp );

		if ( sTemp > 0 )
			m_pPolygonFontOverride = (C_UIPolygonFont*)gFont.GetFont( sTemp );
	}

	return true;
}

void C_FontStyleBaseRenderInfo::Bind( )
{
	if( !m_bUseColourOverride && m_ColourStyle[0] )
		m_bUseColourOverride = m_ColourOverride.SetByName( m_ColourStyle, m_Alpha );

	if( m_EffectIndex != E_XtFontShader_Null )
		m_RenderState.m_EffectIndex = m_EffectIndex;
	if( m_BlendMode != E_UiBlendMode_Unknown )
		m_RenderState.m_BlendMode = m_BlendMode;

	if( m_FontOverideName && m_FontOverideName[0] )
	{
		S32 sTemp = gFont.FindFontByName( m_FontOverideName, false );
		if( sTemp >= 0 )
			m_pPolygonFontOverride = (C_UIPolygonFont*)gFont.GetFont( sTemp );
	}

	for( S32 i = 0; i < m_TextureNames.count && i < (S32)C_FontStyleTextures::m_uMaxTextures; i++ )
	{
		C_TextureHandleIndex id = gUITextureManager.GetTextureByName( m_TextureNames[ i ] );
		// Check 'alternate' textures
		if( !id.Active() )
		{
			id = gSimple2D.AlternateTextureFindByName( m_TextureNames[ i ] );
		}

		if( id.Active() )
		{
			m_FontTextures.m_Textures[ i ].m_eMode = C_FontStyleTextureInfo::E_FontMappingMode_ZeroToOne; // TODO ... maybe
			m_FontTextures.m_Textures[ i ].m_Texture = id;
		}
	}
}

__forceinline Maths::HVec3 C_FontStyleBaseRenderInfo::GetOffset( const F32 fFontHeight, const PanelIndex panel )
{
	F32 offsetX = m_bOffsetProportional ? ( fFontHeight * m_fOffsetX ) : m_fOffsetX;
	F32 offsetY = m_bOffsetProportional ? ( fFontHeight * m_fOffsetY ) : m_fOffsetY;
	F32 offsetZ = m_bOffsetProportional ? ( fFontHeight * m_fOffsetZ ) : m_fOffsetZ;

	Maths::HVec3 offset( offsetX, offsetY, offsetZ );

	if ( gSimple2D.IsPanel3D( panel ) )
		offset.Y( -offset.Y() );
	else
		offset.Z( 0 );

	return offset;
}

__forceinline C_UISpriteColours C_FontStyleBaseRenderInfo::GetColourOverride( const C_UISpriteColours& original )
{
	if ( !m_bUseColourOverride )
		return original;

	C_UISpriteColours cols = m_ColourOverride;

	for(U32 i = 0; i < cols.m_SpriteColoursSize; i++)
		cols[ i ].A( (U8)( cols[ i ].A() * original[ i ].A01() ) );

	return cols;
}

void C_FontStyleBaseRenderInfo::Render( C_FontSpriteStack& spriteStack, C_FontStyle& style, const F32 fFontHeight, const PanelIndex panel )
{
	// font sequence render state
	C_RenderState fontSeqRenderState = m_RenderState;
	fontSeqRenderState.m_EffectIndex = E_Effect_MultiTexture;

	Maths::HVec3 offset = GetOffset( fFontHeight, panel );

	// render the font 'normal'
	for ( U32 i = spriteStack.DrawStart(); i < spriteStack.DrawEnd(); i++ )
	{
		C_FontSpriteItem item = spriteStack[ i ];

		item.m_Pos += offset;
		item.m_Col = GetColourOverride( item.m_Col );

		if ( item.m_eItemType == C_FontSpriteItem::E_Type_Seq )
			AddCharacter( item, style, fontSeqRenderState, panel );
		else
			AddCharacter( item, style, m_RenderState, panel );
	}
}

void C_FontStyleBaseRenderInfo::AddCharacter( const C_FontSpriteItem& item, C_FontStyle& style, C_RenderState& renderState, const PanelIndex panel )
{
	// TODO - error checking on the shader? e.g. rendering a bitmap with a poly shader - which actually crashes!
	
	const E_SpriteType type = ( item.m_eFontType == I_GlyphAttributes::E_GlyphType_Bitmap ) ? E_SpriteType_Sprite : E_SpriteType_TriList;

	F32 fDrawX = item.m_Pos.X();
	F32 fDrawY = item.m_Pos.Y();

	F32 fScaleX	= item.m_Sx;
	F32 fScaleY = item.m_Sy;

	if( item.m_eFontType == I_GlyphAttributes::E_GlyphType_Polygon )
	{
		fScaleX	/= POLYGON_FONT_HEIGHT;
		fScaleY /= POLYGON_FONT_HEIGHT;

		// Not a typo!
		fScaleX = fScaleY;

		if ( gSimple2D.IsPanel3D( panel ) )
		{
			// flip and shift it
			fScaleY *= -1.0f;

			/*
			fDrawY += item.m_Sy;

			if ( ( gFont.GetCurrentJustify() == E_Justify_TopLeft ) || ( gFont.GetCurrentJustify() == E_Justify_TopCentre ) || ( gFont.GetCurrentJustify() == E_Justify_TopRight ) )
				fDrawY -= item.m_Sy;
			else if ( ( gFont.GetCurrentJustify() == E_Justify_BottomLeft ) || ( gFont.GetCurrentJustify() == E_Justify_BottomCentre ) || ( gFont.GetCurrentJustify() == E_Justify_BottomRight ) )
				fDrawY += item.m_Sy;
			*/
		}
	}

	if ( item.m_eFontType == I_GlyphAttributes::E_GlyphType_Shape && item.m_eItemType == C_FontSpriteItem::E_Type_Seq )
	{
		C_FontSequence* pSeq = (C_FontSequence*)item.m_UserData;

		AddShape( pSeq->m_SceneIndex, pSeq->m_GroupName, Maths::HVec3( fDrawX, fDrawY, item.m_Pos.Z() ), Maths::HVec3( fScaleX, fScaleY, 0.0f ), renderState, item.m_Layer, panel );
	}
	else
	{
		// Note this is a clunky bodge for the font cache where a poly font has changed into a bitmap font
		C_RenderState rs = renderState;		
		if( item.m_eFontType == I_GlyphAttributes::E_GlyphType_Bitmap && gSimple2D.GetRenderer().ShaderUsage( (E_Effect)rs.m_EffectIndex ) & E_ShaderUsageFlags_Polygon )
			rs.m_EffectIndex = E_Effect_IntensityAsAlpha;

		if( style.PixelBoundaryX() || style.PixelBoundaryY() )
			gSimple2D.PixelBoundary( style.PixelBoundaryX(), style.PixelBoundaryY(), fDrawX, fDrawY, fScaleX, fScaleY );

		gSimple2D.AddSprite( item.m_Layer, Maths::HVec3( fDrawX, fDrawY, item.m_Pos.Z()), fScaleX, fScaleY, item.m_Col, panel, type, rs, item.m_UserData );
		gSimple2D.AddPreTransformIndex( item.m_Italics ? gFont.GetItalicsMatrix() : gFont.GetPreTransformMatrix() );
		if( gFont.GetPostTransformMatrix() )
			gSimple2D.AddPostTransformIndex( gFont.GetPostTransformMatrix() );

		// null texture
		//static const U32 nullTex = gUITextureManager.GetTextureByName( "null" );

		// temps for texture uvs
		F32 u, v, usize, vsize;

		S32 sTexCount = 0;

		if ( item.m_Texture.Active() )
		{
			// [Blur2PC] Resolve the input atlas after text layout/caching, on every draw.
			const C_TextureHandleIndex inputTexture = gFont.InputGlyphTexture();
			const C_TextureHandleIndex texture = item.m_InputGlyph && inputTexture.Active() ? inputTexture : item.m_Texture;
			gSimple2D.AddTexUVs( sTexCount++, texture, item.m_T0_px, item.m_T0_py, item.m_T0_sx, item.m_T0_sy );
		}

		// don't add extra textures for font sequences
		if ( item.m_eItemType != C_FontSpriteItem::E_Type_Seq )
		{
			for ( U32 i = 0; i < C_FontStyleTextures::m_uMaxTextures; i++ )
			{
				if ( m_FontTextures.m_Textures[ i ].m_Texture.Active() && m_FontTextures.m_Textures[ i ].GetUVFromMode( item, u, v, usize, vsize ) )
					gSimple2D.AddTexUVs( sTexCount, m_FontTextures.m_Textures[ i ].m_Texture, u, v, usize, vsize );
//				else
//					gSimple2D.AddTexUVs( sTexCount, nullTex, 0.0f, 0.0f, 1.0f, 1.0f );

				sTexCount++;
			}
		}
	}
}

void C_FontStyleBaseRenderInfo::RenderPolygonFontNewUserData( C_UIPolygonFont* pFont, C_FontSpriteStack& spriteStack, C_FontStyle& style, const F32 fFontHeight, const PanelIndex panel )
{
	if ( !pFont )
		return;

	// font sequence render state
	C_RenderState fontSeqRenderState = m_RenderState;
	fontSeqRenderState.m_EffectIndex = E_Effect_MultiTexture;

	Maths::HVec3 offset = GetOffset( fFontHeight, panel );

	// render the polygon font
	for ( U32 i = spriteStack.DrawStart(); i < spriteStack.DrawEnd(); i++ )
	{
		C_FontSpriteItem item = spriteStack[ i ];

		item.m_Pos += offset;
		item.m_Col = GetColourOverride( item.m_Col );

		// new render data from the font
		item.m_UserData = pFont->GetUserData( item.m_Character );

		if ( item.m_eItemType == C_FontSpriteItem::E_Type_Seq )
			AddCharacter( item, style, fontSeqRenderState, panel );
		else
			AddCharacter( item, style, m_RenderState, panel );
	}
}


void C_FontStyleBaseRenderInfo::AddShape( const U32 sceneIndex, const char* groupName, const Maths::HVec3& pos, const Maths::HVec3& size, C_RenderState renderState, const LayerIndex layer, const PanelIndex panel )
{
	if ( sceneIndex == 0xffffffff )
		return;
/*
	C_AdobeScene* pSceneHandle = C_AdobeSceneManager::Instance().GetScenePtr( sceneIndex );

	if ( pSceneHandle )
	{
		const U8 flat_matrix_index		= (U8)gSimple2D.PopColourTransformMatrix();
		C_RenderState tmpRenderState	= renderState;
		F32	scale						= 0.6f; // Change the starting scale.

		// Search for the shape / group defines in the font sequencer.
		C_AdobeScene::AdobeRenderList	shapeRenderList;
		Maths::C_AxisBox				sceneBounds;
		pSceneHandle->BuildRenderList( shapeRenderList, sceneBounds, groupName );

		// Need to scale the draw size based on the illustrator scale.
		const F32 adobeScale = 1.0f / ILLUSTRATOR_SCALE;
		const F32 drawSizeX = ( size.X() * adobeScale );
		const F32 drawSizeY = ( size.Y() * adobeScale );

		// Resize the excluded group / shape so that it fits the font dimensions.
		const F32 shift = ( 1.0f - scale ) * 0.5f;
		Maths::HVec3 drawPos( pos.X() + ( size.X() * shift ), pos.Y() + ( size.Y() * shift ), pos.Z() );
		Maths::HVec3 offset = pSceneHandle->ResizeScene( pos, size, sceneBounds, panel, (S32)E_Justify_MiddleLeft, scale ); 

		for ( S32 shape = shapeRenderList.Size()-1; shape >= 0; shape-- )
		{
			C_AdobeShape* pShape = shapeRenderList[ shape ];

			if( !pShape )
				continue;

			// If their is surface data then render it.
			if ( pShape->m_FillData.Size() && pShape->HasFillData() )
			{
				// Setup blend mode			 
				if( pShape->m_BlendStyle.BlendMode() == C_AdobeBlendStyle::E_BlendMode_Multiply )
					renderState.m_BlendMode = (U8)Display::E_BlendMode_Multiply;
				else
					renderState.m_BlendMode = (U8)Display::E_BlendMode_Solid;

				C_UIColour fillColour = 0xffffffff;
				if( pShape->m_FillStyle.Kind() == C_AdobeFillStyle::E_FillStyle_Kind_Solid )
				{
					fillColour = fillColour * pShape->m_FillStyle.Colour();
				}
				renderState.m_EffectIndex = flat_matrix_index;
				renderState.m_EffectIndex = E_Effect_PolygonFlat;

				for ( S32 i = 0; i < pShape->m_FillData.Size(); i++ )
				{
					gSimple2D.AddSprite( layer, drawPos + offset, drawSizeX * scale, drawSizeY * scale, fillColour, panel, E_SpriteType_TriList, renderState, (INTPTR)pShape->m_FillData[i] );	// [Blur2PC][x64]
					gSimple2D.AddTexUVs(0, gUITextureManager.GetTextureByName("null"), 0.0f, 0.0f, 1.0f, 1.0f);
				}
			}

			// If their is Outline data then render it.
			if ( pShape->m_OutlineData.Size() && pShape->HasOutlineData() )
			{
				C_UIColour strokeColour = 0xffffffff;
				strokeColour = strokeColour * pShape->m_StrokeStyle.Colour();
				// Todo : support other blend modes
				if( pShape->m_BlendStyle.Active() )
				{
					// Multiply up the alpha
					U32 alpha = (pShape->m_BlendStyle.Transparency() * strokeColour.A()) >> 8;
					strokeColour.A( alpha );
				}


				for ( S32 i = 0; i < pShape->m_OutlineData.Size(); i++ )
				{
					renderState.m_EffectParam = ( pShape->m_OutlineData[i]->m_uVertexCount << 8 );
					renderState.m_EffectIndex = E_Effect_PolygonStrokeIntensityAsAlpha;

					gSimple2D.AddSprite( layer, drawPos + offset, drawSizeX * scale, drawSizeY * scale, strokeColour, panel, E_SpriteType_TriStrip, renderState, (INTPTR)pShape->m_OutlineData[i] );	// [Blur2PC][x64]
					gSimple2D.AddTexUVs(0, gUITextureManager.GetTextureByName("null"), 0.0f, 0.0f, 1.0f, 1.0f);
				}
			}
		}
	}
*/
}


// -------------------------
// C_FontStylePolygonOutline
// -------------------------
bool C_FontStylePolygonOutline::GetFromLua( lua_State* pState )
{
	C_FontStyleBaseRenderInfo::GetFromLua( pState );

	if ( Lua::DataMemberType( pState, "outline_amount" ) == LUA_TNUMBER )
		Lua::GetDataMember( pState, "outline_amount", m_fOutlineAmount );

	return true;
}

void C_FontStylePolygonOutline::Bind()
{
	// store outline width in the effect param
	C_PolygonFontEffectParam effectParam;
	effectParam.SetOutlineWidth( m_fOutlineAmount );

	m_RenderState.m_EffectParam = effectParam;

	C_FontStyleBaseRenderInfo::Bind();
}

void C_FontStylePolygonOutline::Render( C_FontSpriteStack& spriteStack, C_FontStyle& style, const F32 fFontHeight, const PanelIndex panel )
{
	if ( m_pPolygonFontOverride )
		RenderPolygonFontNewUserData( m_pPolygonFontOverride, spriteStack, style, fFontHeight, panel );
	else
		C_FontStyleBaseRenderInfo::Render( spriteStack, style, fFontHeight, panel );
}

// --------------------
// C_FontStylePolygon3D
// --------------------
bool C_FontStylePolygon3D::GetFromLua( lua_State* pState )
{
	C_FontStyleBaseRenderInfo::GetFromLua( pState );

	if ( Lua::DataMemberType( pState, "extrude_amount" ) == LUA_TNUMBER )
		Lua::GetDataMember( pState, "extrude_amount", m_fExtrudeAmount );

	Bind();

	return true;
}

void C_FontStylePolygon3D::Bind()
{
	// store extrude amount in the effect param
	C_PolygonFontEffectParam effectParam;
	effectParam.SetExtrusion( m_fExtrudeAmount );

	m_RenderState.m_EffectParam = effectParam;

	C_FontStyleBaseRenderInfo::Bind();
}

void C_FontStylePolygon3D::Render( C_FontSpriteStack& spriteStack, C_FontStyle& style, const F32 fFontHeight, const PanelIndex panel )
{
	if ( m_pPolygonFontOverride )
		RenderPolygonFontNewUserData( m_pPolygonFontOverride, spriteStack, style, fFontHeight, panel );
	else
		C_FontStyleBaseRenderInfo::Render( spriteStack, style, fFontHeight, panel );
}

// ---------------------------
// C_FontStylePolygonOutline3D
// ---------------------------
bool C_FontStylePolygonOutline3D::GetFromLua( lua_State* pState )
{
	C_FontStyleBaseRenderInfo::GetFromLua( pState );

	if ( Lua::DataMemberType( pState, "outline_amount" ) == LUA_TNUMBER )
		Lua::GetDataMember( pState, "outline_amount", m_fOutlineAmount );
	
	if ( Lua::DataMemberType( pState, "extrude_amount" ) == LUA_TNUMBER )
		Lua::GetDataMember( pState, "extrude_amount", m_fExtrudeAmount );

	Bind();

	return true;
}

void C_FontStylePolygonOutline3D::Bind()
{
	// store outline and extrude amount in the effect param
	C_PolygonFontEffectParam effectParam;
	effectParam.SetOutlineWidth( m_fOutlineAmount );
	effectParam.SetExtrusion( m_fExtrudeAmount );

	m_RenderState.m_EffectParam = effectParam;

	C_FontStyleBaseRenderInfo::Bind();
}

void C_FontStylePolygonOutline3D::Render( C_FontSpriteStack& spriteStack, C_FontStyle& style, const F32 fFontHeight, const PanelIndex panel )
{
	if ( m_pPolygonFontOverride )
		RenderPolygonFontNewUserData( m_pPolygonFontOverride, spriteStack, style, fFontHeight, panel );
	else
		C_FontStyleBaseRenderInfo::Render( spriteStack, style, fFontHeight, panel );
}

// --------------------------
// C_FontStylePolygonGradient
// --------------------------
bool C_FontStylePolygonGradient::GetFromLua( lua_State* pState )
{
	C_FontStyleBaseRenderInfo::GetFromLua( pState );

	if ( Lua::DataMemberType( pState, "gradient_index" ) == LUA_TNUMBER )
		Lua::GetDataMember( pState, "gradient_index", m_uGradientIndex );

	if ( Lua::DataMemberType( pState, "gradient_angle" ) == LUA_TNUMBER )
		Lua::GetDataMember( pState, "gradient_angle", m_fGradientAngle );

	Bind();

	return true;
}

void C_FontStylePolygonGradient::Render( C_FontSpriteStack& spriteStack, C_FontStyle& style, const F32 fFontHeight, const PanelIndex panel )
{
	I_FontAttributes* pFont = gFont.GetFont( style.Font() );

	if ( pFont->GetDefaultGlyph()->GetType() != I_GlyphAttributes::E_GlyphType_Polygon || !( m_RenderState.m_EffectIndex & E_ShaderUsageFlags_GradientBounds ) )
		return;

	TransformIndex colourMatrixIndex = 0;

	// calculate bounding area for colour matrix
	Maths::C_AxisBox box;

	for ( U32 i = spriteStack.DrawStart(); i < spriteStack.DrawEnd(); i++ )
	{
		C_FontSpriteItem& item = spriteStack[ i ];
		box.UpdateExtents( item.m_Pos );
		box.UpdateExtents( Maths::C_Vertex( item.m_Pos + Maths::HVec3( item.m_Sx, item.m_Sy, 0.0f ) ) );
	}

	// get a new colour transform matrix
	colourMatrixIndex = gSimple2D.PopColourTransformMatrix();

	Maths::HMat43& colourMatrix = gSimple2D.GetColourTransformMatrix( colourMatrixIndex );
	Maths::HMat43 colourTransform;

	if ( gFont.GetPostTransformMatrix() )
		colourTransform.SetIdentity();
	else
		colourTransform = gSimple2D.GetPostTransformMatrix( gFont.GetPostTransformMatrix() );

	Maths::HVec3 boxSize( box.Max() - box.Min() );

	if ( ( boxSize.X() > 0.0f ) && ( boxSize.Y() > 0.0f ) )
	{
		// inverse rotate the current transform
		colourMatrix = Maths::HMat43( Maths::HMat44( colourMatrix ) * Maths::HMat44( colourTransform ).Inverse() );
		// translate
		colourMatrix.Translation( colourMatrix.Translation() - Maths::HVec3( box.Centre().X(), box.Centre().Y(), 0.0f ) );
		// scale
		colourMatrix = colourMatrix * Maths::HMat43( Maths::C_MatrixScale( Maths::HVec3( 2.0f / boxSize.X(), 2.0f / boxSize.Y(), 0.0f ) ) );
		// rotate
		colourMatrix = colourMatrix * Maths::HMat43( Maths::C_MatrixRotateRPH( Maths::DegToRad( m_fGradientAngle ), 0.0f, 0.0f ) );
	}

	// update the effect param
	m_RenderState.m_EffectParam = colourMatrixIndex | ( m_uGradientIndex << 8 ); // TODO - this should probably be a class

	// now render
	if ( m_pPolygonFontOverride )
		RenderPolygonFontNewUserData( m_pPolygonFontOverride, spriteStack, style, fFontHeight, panel );
	else
		C_FontStyleBaseRenderInfo::Render( spriteStack, style, fFontHeight, panel );
}

// -------------------------
// C_FontStylePolygonBevel3D
// -------------------------
bool C_FontStylePolygonBevel3D::GetFromLua( lua_State* pState )
{
	C_FontStyleBaseRenderInfo::GetFromLua( pState );

	if ( Lua::DataMemberType( pState, "outline_amount" ) == LUA_TNUMBER )
		Lua::GetDataMember( pState, "outline_amount", m_fOutlineAmount );
	
	if ( Lua::DataMemberType( pState, "extrude_amount" ) == LUA_TNUMBER )
		Lua::GetDataMember( pState, "extrude_amount", m_fExtrudeAmount );

	if ( Lua::DataMemberType( pState, "extrude_amount_bevel" ) == LUA_TNUMBER )
		Lua::GetDataMember( pState, "extrude_amount_bevel", m_fExtrudeAmountBevel );

	Bind();

	return true;
}

void C_FontStylePolygonBevel3D::Bind()
{
	// store outline and extrude amount in the effect param
	C_PolygonFontEffectParam effectParam;
	effectParam.SetOutlineWidth( m_fOutlineAmount );
	effectParam.SetExtrusion( m_fExtrudeAmount );
	effectParam.SetExtrusionBevel( m_fExtrudeAmountBevel );

	m_RenderState.m_EffectParam = effectParam;

	C_FontStyleBaseRenderInfo::Bind();
}

void C_FontStylePolygonBevel3D::Render( C_FontSpriteStack& spriteStack, C_FontStyle& style, const F32 fFontHeight, const PanelIndex panel )
{
	if ( m_pPolygonFontOverride )
		RenderPolygonFontNewUserData( m_pPolygonFontOverride, spriteStack, style, fFontHeight, panel );
	else
		C_FontStyleBaseRenderInfo::Render( spriteStack, style, fFontHeight, panel );
}

// -----------
// C_FontStyle
// -----------
// global default instance
C_FontStyle C_FontStyle::m_sInstance;

C_FontStyle::C_FontStyle()
	: m_uFont( 0 ),
	m_fTrackingOverride( 0.0f ),
	m_fHeightPercentage( C_Font::s_fUseDefaultHeightPercentage ),
	m_ManagedByXt( false ),
	m_bItalics( false ),
	m_bPixelBoundaryX( false ),
	m_bPixelBoundaryY( false )
{
	//
}

C_FontStyle::C_FontStyle( const C_FontStyle& o )
{
	// use our own operator =
	*this = o;
}

C_FontStyle::~C_FontStyle()
{
	Cleanup();
}

void C_FontStyle::Cleanup()
{
	if( !m_ManagedByXt )
	{
		if ( m_RenderPassInfo.elements )
		{
			for ( S32 i = 0; i < m_RenderPassInfo.count; i++ )
				SAFE_DELETE_PTR( m_RenderPassInfo[ i ] );

			SAFE_DELETE_ARY( m_RenderPassInfo.elements );
		}
	}

	//m_RenderPassInfo.Clear();
}

void C_FontStyle::Bind()
{
	if( !gFont.GetFont( m_uFont ) || !gFont.GetFont( m_uFont )->GetDefaultGlyph() )
	{
		LOGERROR( g_Log, DBSTR "Invalid font %d\n", m_uFont );
		return;
	}

	// pixel boundaries don't work (and don't seem logical anyway) with polygon fonts
	if ( gFont.GetFont( m_uFont )->GetDefaultGlyph()->GetType() == I_GlyphAttributes::E_GlyphType_Polygon )
		m_bPixelBoundaryX = m_bPixelBoundaryY = false;

	// Check if we've got a name to lookup
	if( m_FontName && m_FontName[0] )
	{
		S32 sTemp = gFont.FindFontByName( m_FontName, false );
		if( sTemp >= 0 )
			m_uFont = (U32)sTemp;
	}

	for ( S32 i = 0; i < m_RenderPassInfo.count; i++ )
	{
		C_FontStyleBaseRenderInfo*p = m_RenderPassInfo[ i ];
		if( !p )
			continue;

		p->Bind();
	}
}

void C_FontStyle::SorceryCallback( XtEventType type, XtRecord* pRecord, C_FontStyle* pFontStyle, XtCallbackContext* pContext )
{
	switch ( type )
	{
	case kXtEtConstruct:
		UiXt::C_ScreenList::Instance().SetDirty( UiXt::C_ScreenList::E_DirtyFlags_Font ); // Dirty screen list so we get the update

		UiXt::C_FontStyleList::Instance().SetDirty();
		UiXt::C_FontStyleList::Instance().GetList().Insert( pFontStyle );
		UiXt::C_FontStyleList::Instance().DefferBind( pFontStyle );

		pFontStyle->XtManaged();
		break;

	case kXtEtDestruct:
		UiXt::C_FontStyleList::Instance().SetDirty();
		UiXt::C_FontStyleList::Instance().GetList().Erase( pFontStyle );
		UiXt::C_FontStyleList::Instance().ClearBind(); // DG - Ag! fix me, move into a link list
		break;
	}
}

void C_FontStyle::InitDefaultStyle( lua_State* pState )
{
	// cleanup any old one - problems when changing sections ...
	C_FontStyle::Instance() = C_FontStyle();

	lua_getglobal( pState, LUA_FONTSTYLE_TABLE_NAME );

	if ( lua_type( pState, -1 ) == LUA_TTABLE )
		C_FontStyle::Instance().GetFromLua( pState );
	else
	{
		// just create one - font 0
		C_FontStyleBaseRenderInfo* pBase = new C_FontStyleBaseRenderInfo;
		assert( pBase );

		pBase->GetRenderState().m_EffectIndex = E_Effect_IntensityAsAlpha;//E_Effect_MultiTexture;

		m_RenderPassInfo.count = 1;
		m_RenderPassInfo.elements = new C_FontStyleBaseRenderInfo*[ m_RenderPassInfo.count ];
		assert( m_RenderPassInfo.elements );

		m_RenderPassInfo[ 0 ] = pBase;
	}

	lua_pop( pState, 1 );
}

bool C_FontStyle::GetFromLua( lua_State* pState, const char* pTableName )
{
	if ( pTableName )
	{
		if ( Lua::DataMemberType( pState, pTableName ) != LUA_TTABLE )
			return false;

		// get the table
		lua_pushstring( pState, pTableName );
		lua_gettable( pState, -2 );
	}

	// get common params

	if ( Lua::DataMemberType( pState, "font" ) == LUA_TNUMBER )
		Lua::GetDataMember( pState, "font", m_uFont );

	if ( Lua::DataMemberType( pState, "tracking" ) == LUA_TNUMBER )
		Lua::GetDataMember( pState, "tracking", m_fTrackingOverride );

	if ( Lua::DataMemberType( pState, "height_percentage" ) == LUA_TNUMBER )
		Lua::GetDataMember( pState, "height_percentage", m_fHeightPercentage );

	if ( Lua::DataMemberType( pState, "italics" ) == LUA_TBOOLEAN )
		Lua::GetDataMember( pState, "italics", m_bItalics );

	if ( Lua::DataMemberType( pState, "pixel_boundary_x" ) == LUA_TBOOLEAN )
		Lua::GetDataMember( pState, "pixel_boundary_x", m_bPixelBoundaryX );

	if ( Lua::DataMemberType( pState, "pixel_boundary_y" ) == LUA_TBOOLEAN )
		Lua::GetDataMember( pState, "pixel_boundary_y", m_bPixelBoundaryY );

	// grab render pass info
	if ( Lua::DataMemberType( pState, "render_passes" ) == LUA_TTABLE )
	{
		lua_pushstring( pState, "render_passes" );
		lua_gettable( pState, -2 );

		S32 sRenderPassCount = luaL_getn( pState, -1 );

//		m_RenderPassInfo.Reserve( sRenderPassCount );
		m_RenderPassInfo.count = sRenderPassCount;
		m_RenderPassInfo.elements = new C_FontStyleBaseRenderInfo*[ m_RenderPassInfo.count ];
		assert( m_RenderPassInfo.elements );

		for ( S32 i = 0; i < sRenderPassCount; i++ )
		{
			lua_rawgeti( pState, -1, i+1 );	// push on index i

			if ( lua_type( pState, -1 ) == LUA_TTABLE ) // type check
			{
				// get type
				E_FontStyleRenderType eType = E_FontStyleRenderType_Default;

				if ( Lua::DataMemberType( pState, "type" ) == LUA_TNUMBER )
				{
					S32 sTemp = 0;
					Lua::GetDataMember( pState, "type", sTemp );
					eType = (E_FontStyleRenderType)sTemp;
				}

				C_FontStyleBaseRenderInfo* pInfo = C_FontStyleCreator::Instance().CreateFontStyle( eType );
				assert( pInfo );

				if ( pInfo )
					pInfo->GetFromLua( pState );

				m_RenderPassInfo[ i ] = pInfo; // Store even if it's NULL
			}

			lua_pop( pState, 1 );	// pop off index i
		}

		// pop off render pass info table
		lua_pop( pState, 1 );
	}

	if ( pTableName )
		lua_pop( pState, 1 );	// pop off style table

	Bind();

	return true;
}

void C_FontStyle::Render( C_FontSpriteStack& spriteStack, const F32 fFontHeight, const PanelIndex panel )
{
	if ( m_RenderPassInfo.elements )
	{
		for ( S32 i = 0; i < m_RenderPassInfo.count; i++ )
		{
			if( m_RenderPassInfo[ i ] )
				m_RenderPassInfo[ i ]->Render( spriteStack, *this, fFontHeight, panel );
		}
	}
}

C_FontStyle& C_FontStyle::operator = ( const C_FontStyle& o )
{
	// free memory
	Cleanup();

	// common params
	Font( o.Font() );
	Tracking( o.Tracking() );
	HeightPercentage( o.HeightPercentage() );
	Italics( o.Italics() );
	PixelBoundaryX( o.PixelBoundaryX() );
	PixelBoundaryY( o.PixelBoundaryY() );

	if( o.m_RenderPassInfo.count )
	{
		m_RenderPassInfo.count = o.m_RenderPassInfo.count;
		m_RenderPassInfo.elements = new C_FontStyleBaseRenderInfo*[ m_RenderPassInfo.count ];
		assert( m_RenderPassInfo.elements );

		// render passes
		for ( S32 i = 0; i < (S32)o.GetRenderPassCount(); i++ )
		{
			C_FontStyleBaseRenderInfo* other = o.m_RenderPassInfo[ i ];
			if( !other )
			{
				m_RenderPassInfo[ i ] = NULL;
				continue;
			}

			// create new
			C_FontStyleBaseRenderInfo* pNew = C_FontStyleCreator::Instance().CreateFontStyle( other->Type(), other );
			assert( pNew );

			m_RenderPassInfo[ i ] = pNew;
	//		AddRenderPassInfo( pNew );
		}
	}

	m_ManagedByXt = false;

	return *this;
}

// ------------------
// C_FontStyleCreator
// ------------------
C_FontStyleCreator C_FontStyleCreator::m_sInstance;

void C_FontStyleCreator::CreateLuaEnums( lua_State* pState )
{
	lua_getglobal( pState, LUA_ENUM_TABLE_NAME );

	if ( lua_isnil( pState, -1 ) )
	{
		LOGFATAL( g_Log, DBSTR "%s table not found", LUA_ENUM_TABLE_NAME );
		lua_pop( pState, 1 );
		return;
	}

	// add in the font styles

	lua_pushstring( pState, "FontStyle" );
	lua_newtable( pState );
		for ( S32 i = 0; i < E_FontStyleRenderType_Count; i++ )
		{
			if ( m_Creators[ i ].m_pCreator )
				Lua::AddDataMember( pState, m_Creators[ i ].m_Name.Chars(), i );
		}
	lua_rawset( pState, -3 );

	lua_pop( pState, 1 );
}

// -----------------
// C_FontStylePasses
// -----------------
C_FontStylePasses::C_FontStylePasses()
	: m_bUseXt( false )
{
	//
}

C_FontStylePasses::~C_FontStylePasses()
{
	m_XtFontStyles.Destroy();
	m_FontStyles.Destroy();
}

void C_FontStylePasses::Setup( lua_State* pState, const char* pLuaName )
{
	// resolve names from sorcery
	if ( ( m_StyleName != NULL ) && ( m_StyleName[ 0 ] != NULL ) )
	{
		m_bUseXt = true;

		char buffer[ 32 ];
		buffer[ 0 ] = '\0';
		S32 sCount = 0;

		// parse tokens from names
		for ( const char* p = m_StyleName.string; *p; )
		{
			if ( *p == ';' )
			{
				// close and trim backwards
				buffer[ sCount ] = '\0';

				for ( S32 i = sCount - 1; i >= 0; i-- )
					if ( buffer[ i ] == ' ' )
						buffer[ i ] = '\0';

				// now resolve
				C_FontStyle* pStyle = const_cast< C_FontStyle* >( UiXt::C_FontStyleList::Instance().FindByName( buffer ) );	// meh

				if ( pStyle )
					m_XtFontStyles.PushBack( pStyle );

				// now trim forwards
				do
				{
					p++;
				} 
				while ( *p == ' ' );

				// reset
				sCount = 0;
			}
			else
				buffer[ sCount++ ] = *p++;
		}

		// close and trim backwards
		buffer[ sCount ] = '\0';

		for ( S32 i = sCount - 1; i >= 0; i-- )
			if ( buffer[ i ] == ' ' )
				buffer[ i ] = '\0';

		// resolve last style
		C_FontStyle* pStyle = const_cast< C_FontStyle* >( UiXt::C_FontStyleList::Instance().FindByName( buffer ) );	// meh

		if ( pStyle )
			m_XtFontStyles.PushBack( pStyle );
	}
	else if ( pState )
	{
		// grab from Lua
		if ( Lua::DataMemberType( pState, pLuaName ) == LUA_TTABLE )
		{
			// get the styles
			lua_pushstring( pState, pLuaName );
			lua_gettable( pState, -2 );

			S32 sFontStyleCount = luaL_getn( pState, -1 );

			if ( sFontStyleCount == 0 )
			{
				m_FontStyles.PushBack( C_FontStyle::Instance() );
			}
			else
			{
				m_FontStyles.Reserve( sFontStyleCount );

				for ( S32 i = 1; i <= sFontStyleCount; i++ )
				{
					lua_rawgeti( pState, -1, i );	// push on index i

					if ( lua_type( pState, -1 ) == LUA_TTABLE ) // type check
					{
						// add a new style
						m_FontStyles.PushBack( C_FontStyle() );
						C_FontStyle& style = m_FontStyles.Back();
						style.GetFromLua( pState );
					}

					lua_pop( pState, 1 );	// pop off index i
				}
			}

			// pop off styles table
			lua_pop( pState, 1 );
		}
	}

	if( m_FontStyles.Size() == 0 && m_XtFontStyles.Size() == 0 )
	{
		m_bUseXt = false;
		// just add in the default style
//		LOGWARN( g_Log, DBSTR "No font styles found" );
		m_FontStyles.PushBack( C_FontStyle::Instance() );
		m_bUseXt = false;
	}
}

U32 C_FontStylePasses::NumFontStyles() const
{
	if ( m_bUseXt )
		return (U32)m_XtFontStyles.Size();

	return (U32)m_FontStyles.Size();
}

C_FontStyle& C_FontStylePasses::GetFontStyle( const U32 uIndex )
{
	if ( m_bUseXt )
		return *m_XtFontStyles[ uIndex ];

	return m_FontStyles[ uIndex ];
}

};	// namespace UI

#ifdef CFG_ALLOW_OPTIMIZE_OFF_UI
#pragma optimize("", on)
#endif
