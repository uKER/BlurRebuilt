
#include "stdafx.h"

#include "ui\Game\Gadgets\UIStaticShape.h"

#include "ui\Illustrator\UIAdobeShape.h"
#include "ui\Util\WideString.h"

#include "ui\Graphics\Font\Font.h"

#ifdef CFG_ALLOW_OPTIMIZE_OFF_UI
#pragma optimize("", off)
#endif

// XT Definition
XT_TYPE( UI::UIStaticShape, "UIStaticShape" )
	XT_BASE( UI::BaseUIShape )
XT_END_TYPE

#ifndef CFG_GOLD
Debug::Bool		g_RenderShapeBounds( "UI.Shapes.RenderShapeBounds", false );
Debug::Bool		g_RenderShapeName( "UI.Shapes.RenderShapeName", false );
Debug::Bool		g_RenderFileName( "UI.Shapes.RenderFileName", false );
Debug::Float	g_NameSize( "UI.Shapes.NameSize", 2.5, Debug::Range( 0.0f, 64.0f ) );
Debug::Bool		g_LogShapeErrorFullSheet( "UI.Shapes.LogShapeErrorFullSheet", false );
#endif // CFG_GOLD

namespace UI
{

void UIStaticShape::Init()
{
	BaseUIShape::Init();
}


void UIStaticShape::Setup( lua_State* pState )
{
	BaseUIShape::Setup( pState );
}


void UIStaticShape::Render()
{
	if( !m_Activated || !m_pSceneHandle )
		return;

	C_UIAutoTransform transform( this );
	BaseUIShape::Render();

#ifndef CFG_GOLD
	if ( g_RenderShapeBounds )
	{
		gSimple2D.AddSprite( m_Layer, m_Position, m_Size.x, m_Size.y, 0x77777777, m_Panel );
		gSimple2D.AddTexUVs( 0, gUITextureManager.GetTextureByName( "null" ) );
		gSimple2D.AddJustification( m_Justify );
	}
	if ( g_RenderShapeName && m_ObjectName )
	{
		Maths::HVec3 namePos = m_Position + ( gSimple2D.GetJustifiedCorner( m_Justify, 0, m_Panel ) * m_Size );

		C_WideString32 objectName( m_ObjectName );
		gFont.DrawDebugText( m_Layer, namePos + Maths::HVec3( m_Size.x * 0.5f, m_Size.y * Up(), 0.0f ), E_Justify_TopCentre, objectName, 0xff0066ff, g_NameSize, 0.0f, m_Panel );
	}
	if ( g_RenderFileName )
	{
		Maths::HVec3 namePos = m_Position + ( gSimple2D.GetJustifiedCorner( m_Justify, 0, m_Panel ) * m_Size ) + Maths::HVec3( 0.0f, g_NameSize * Up(), 0.0f );

		C_WideString32 objectName( m_pSceneHandle->ScenePtr()->Name() );
		gFont.DrawDebugText( m_Layer, namePos + Maths::HVec3( m_Size.x * 0.5f, m_Size.y * Up(), 0.0f ), E_Justify_TopCentre, objectName, 0xff99ffa1, g_NameSize, 0.0f, m_Panel );
	}
#endif

	const TransformIndex colourIndex = (U8)gSimple2D.PopColourTransformMatrix();

	// Calculate the justified position and then justify the shape to the shape bounds.
	Maths::HVec3 drawPos = m_Position + ( gSimple2D.GetJustifiedCorner( m_Justify, 0, m_Panel ) * m_Size );
	Maths::HVec3 boundsSize = m_Size * Maths::HVec3( m_SceneBounds.Max().X(), m_SceneBounds.Max().Y(), 0.0f );
	drawPos -= ( ( m_Size  - boundsSize ) * gSimple2D.GetJustifiedCorner( m_ShapeJustification, 0, m_Panel ) );

#ifndef CFG_GOLD
	if ( g_LogShapeErrorFullSheet && m_pSceneHandle && m_pSceneHandle->ScenePtr() && ( m_pSceneHandle->ScenePtr()->GetTotalNumberOfShapes() != 1 ) && ( m_pSceneHandle->ScenePtr()->GetTotalNumberOfShapes() == m_ShapeRenderList.Size() ) )
		LOGWARN( g_Log, DBSTR "UIStaticShape - rendering full sheet - %s", m_Name.Text() );
#endif

	// Render all of the shapes in the list. Do this backwards!
	for ( S32 shape = m_ShapeRenderList.Size()-1; shape >= 0; shape-- )
	{
		const C_AdobeShape* pShape = m_ShapeRenderList[ shape ];

		if( !pShape )
			continue;

		SetBlendMode( pShape );
	
		// Draw the fill if one exists.
		if ( pShape->HasFill() )
		{
			RenderFill( pShape, drawPos, m_Size.x, m_Size.y * Up(), colourIndex );
		}
		// Draw the stroke if one exists.
		if ( pShape->HasStroke() )
		{				
			RenderStroke( pShape, drawPos, m_Size.x, m_Size.y * Up(), colourIndex );
		}
	}
}


}; // namespace UI

#ifdef CFG_ALLOW_OPTIMIZE_OFF_UI
#pragma optimize("", on) 
#endif
