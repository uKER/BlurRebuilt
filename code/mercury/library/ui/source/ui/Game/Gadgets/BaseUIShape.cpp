
#include "stdafx.h"

#include "ui\Game\Gadgets\BaseUIShape.h"

#include "ui\Game\Gadgets\ButtonMessages.h"

#ifdef CFG_ALLOW_OPTIMIZE_OFF_UI
#pragma optimize("", off)
#endif

XT_TYPE( UI::E_StrokeEffect, "UIStrokeEffect" )
	XT_OPTION( UI::E_StrokeEffectIntensityAsAlpha, "IntensityAsAlpha" )
	XT_OPTION( UI::E_StrokeEffectAlpha, "Alpha" )	
	XT_OPTION( UI::E_StrokeEffect_AM_AudioEqAlpha, "AM_AudioEqAlpha" )	
	XT_OPTION( UI::E_StrokeEffect_AM_AudioEqIntensityAsAlpha, "AM_AudioEqIntensityAsAlpha" )	
XT_END_TYPE

// XT Definition
XT_TYPE( UI::BaseUIShape, "UIBaseShape" )
	XT_BASE( UI::C_GadgetBase )
	XT_FIELD( m_SceneName,				"scene_name" )
	XT_FIELD( m_ObjectName,				"object_name" )
	XT_FIELD( m_ShapeJustification,		"shape_justification" )
	XT_FIELD( m_StrokeMinVertexParam,	"min_vertex" )
	XT_FIELD( m_StrokeMaxVertexParam,	"max_vertex" )
	XT_FIELD( m_StrokeEffect,			"stroke_effect" )
	XT_FIELD( m_bFinnedOuter,			"fin_outside" )
	XT_FIELD( m_UseGroupBounds,			"use_group_bounds" )
XT_END_TYPE

namespace UI
{

void BaseUIShape::Init()
{
	C_GadgetBase::Init();

	m_pSceneHandle				= NULL;
	m_StrokeMinVertexParam		= 0.0f;
	m_StrokeMaxVertexParam		= 1.0f;
	m_StrokeEffect				= E_StrokeEffectIntensityAsAlpha;
	m_bFinnedOuter				= false;
}


void BaseUIShape::Setup( lua_State* pState )
{
	C_GadgetBase::Setup( pState );

	ResolveSceneName();
	ResolveObjectName();

	// This cant be set from sorcery unless controlled by the timeline
	m_StrokeMinVertexParam		= 0.0f;
	m_StrokeMaxVertexParam		= 1.0f;

	AddSubscriber( E_UiMsg_FrameUpdate );
}


void BaseUIShape::FrameUpdate()
{
#ifndef CFG_GOLD
	if ( m_pSceneHandle && m_pSceneHandle->IsDirty() )
	{
		ResolveSceneName();
		ResolveObjectName();
	}
#endif // CFG_GOLD
}


const C_AdobeShape* BaseUIShape::FindShape( const char* name ) const
{
	const C_AdobeShape* pShape = NULL;
	S32 size = m_ShapeRenderList.Size();

	for ( S32 i = 0; i < size; i++ )
	{
		pShape = m_ShapeRenderList[i];

		if ( pShape->GetName() == name )
			return pShape;
	}
	return NULL;
}


const bool BaseUIShape::ResolveSceneName()
{
	// Find the scene handle.
	if( m_SceneName && strlen( m_SceneName ) )
	{
		m_pSceneHandle = C_AdobeSceneManager::Instance().GetHandleByName( m_SceneName );
	}

	return ( m_pSceneHandle != NULL );
}


const bool BaseUIShape::ResolveObjectName()
{
	if ( !m_pSceneHandle )
		return false;

	bool result = false;
	if ( m_ObjectName && strlen( m_ObjectName ) )
	{
		// Take an index to a shape.
		if ( strnicmp( m_ObjectName, "shape_index_", 12 ) == 0 )
		{
			const C_AdobeGroup* pLayer = m_pSceneHandle->ScenePtr()->GetLayer( 0 );

			if ( pLayer )
			{
				U32 index = atoi( m_ObjectName + 12 );

				if( pLayer->GetShape( index ) )
				{
					m_ShapeRenderList.Clear();
					m_ShapeRenderList.PushBack( pLayer->GetShape( index ) );
					m_SceneBounds.UpdateExtents( pLayer->GetShape( index )->BoundingBox() );

					result = true;
				}
			}
		}
		else
		{
			// Build scene data containing this shape only.
			result = m_pSceneHandle->ScenePtr()->BuildRenderList( m_ShapeRenderList, m_SceneBounds, Util::C_String( m_ObjectName ).Lower(), m_UseGroupBounds );
		}
	}

	// Didn't find the name we're looking for - show all
	if ( !result )
		m_pSceneHandle->ScenePtr()->BuildRenderList( m_ShapeRenderList, m_SceneBounds, "", m_UseGroupBounds );

	return result;
}


void BaseUIShape::RenderFill( const C_AdobeShape* pShape, const Maths::HVec3& vPos, const F32 fSizeX, const F32 fSizeY, const TransformIndex uColourIndex )
{
	if (!pShape)
		return;

	switch ( pShape->FillStyle().Kind() )
	{
		case C_AdobeFillStyle::E_FillStyle_Kind_Solid :
		{
			m_RenderState.m_EffectParam = uColourIndex;
			m_RenderState.m_EffectIndex = m_bFinnedOuter ? E_Effect_PolygonFlatFinnedOuter : E_Effect_PolygonFlat;
			break;
		}
		case C_AdobeFillStyle::E_FillStyle_Kind_Gradient :
		{
			m_RenderState.m_EffectIndex = E_Effect_PolygonGradient_Param;
			break;
		}
	}

	C_UIColour fillColour = m_SpriteColours[0];
	if( pShape->FillStyle().Kind() == C_AdobeFillStyle::E_FillStyle_Kind_Solid )
	{
		fillColour = fillColour * pShape->FillStyle().Colour();
	}
	// Todo : support other blend modes
	if( pShape->BlendStyle().Active() )
	{
		// Multiply up the alpha
		U32 alpha = (pShape->BlendStyle().Transparency() * fillColour.A()) >> 8;
		fillColour.A( alpha );
	}

	for ( S32 i = 0; i < pShape->Fill().Size(); i++ )
	{
		// Push the sprite onto the stack
		gSimple2D.AddSprite( m_Layer, vPos, fSizeX, fSizeY, fillColour, m_Panel, E_SpriteType_TriList, m_RenderState, (INTPTR)pShape->Fill()[i] );	// [Blur2PC][x64] 

		if( pShape->FillStyle().Kind() == C_AdobeFillStyle::E_FillStyle_Kind_Gradient )
		{
			gSimple2D.AddTexUVs( 0, gSimple2D.GetGradientTextureParam()  );
		}
		else
			gSimple2D.AddTexUVs(0, gUITextureManager.GetTextureByName("null"), 0.0f, 0.0f, 1.0f, 1.0f);
	}
}


void BaseUIShape::RenderStroke( const C_AdobeShape* pShape, const Maths::HVec3& vPos, const F32 fSizeX, const F32 fSizeY, const TransformIndex uColourIndex )
{
	C_UISpriteColours strokeColour = m_SpriteColours;
	strokeColour[0] = strokeColour[0] * pShape->StrokeStyle().Colour();

	// Todo : support other blend modes
	if( pShape->BlendStyle().Active() )
	{
		// Multiply up the alpha
		U32 alpha = (pShape->BlendStyle().Transparency() * strokeColour[0].A()) >> 8;
		strokeColour[0].A( alpha );
	}

	for ( S32 i = 0; i < pShape->Stroke().Size(); i++ )
	{
		U32 minVertex = (U32)((F32)pShape->Stroke()[i]->m_uVertexCount * Maths::Clamp( m_StrokeMinVertexParam, 0.0f, 1.0f ) );
		U32 maxVertex = (U32)((F32)pShape->Stroke()[i]->m_uVertexCount * Maths::Clamp( m_StrokeMaxVertexParam, 0.0f, 1.0f ) );

		// Keep even
		minVertex >>= 1;	minVertex <<= 1;
		maxVertex >>= 1;	maxVertex <<= 1;

		if ( minVertex > maxVertex )
			minVertex = maxVertex;

		// Pass effect settings into the render state structure
		m_RenderState.m_EffectIndex = m_StrokeEffect;
		m_RenderState.m_EffectParam = ( minVertex & 0xffff) | (maxVertex & 0xffff) << 16;

		// Add the user data as a sprite so it can be rendered.
		gSimple2D.AddSprite( m_Layer, vPos, fSizeX, fSizeY, strokeColour, m_Panel, E_SpriteType_TriStrip, m_RenderState, (INTPTR)pShape->Stroke()[i] );	// [Blur2PC][x64] 
		gSimple2D.AddTexUVs( 0, GetTextureParam( 0 ) );

		if ( m_StrokeEffect == E_StrokeEffect_AM_AudioEqAlpha || m_StrokeEffect == E_StrokeEffect_AM_AudioEqIntensityAsAlpha )
			gSimple2D.AddTexUVs( 1, GetTextureParam( 1 ) );	// hijack this texture to pass down some shader params
	}
}


const bool BaseUIShape::HasAlpha() const
{
	return ( m_SpriteColours[0].A() != 0xff || m_SpriteColours[1].A() != 0xff || m_SpriteColours[2].A() != 0xff || m_SpriteColours[3].A() != 0xff );
}


void BaseUIShape::SetBlendMode( const C_AdobeShape* pShape )
{
	switch ( pShape->BlendStyle().BlendMode() )
	{
		case C_AdobeBlendStyle::E_BlendMode_Multiply :
		{
			m_RenderState.m_BlendMode = (U8)Display::E_BlendMode_Multiply; 
			break;
		}
	}
}


const bool BaseUIShape::ChangeSceneName( const char* pName )
{
	if( m_SceneName )
	{
		m_XtAllocator.Delete( const_cast<char *>(m_SceneName.string) );
	}

	m_SceneName = XtSz8( CopyCreateString( pName ) );
	
	return ResolveSceneName();
}


const bool BaseUIShape::ChangeObjectName( const char* pName )
{
	if( m_ObjectName )
	{
		m_XtAllocator.Delete( const_cast<char *>(m_ObjectName.string) );
	}

	m_ObjectName = XtSz8( CopyCreateString( pName ) );

	return ResolveObjectName();
}


}; // namespace UI
