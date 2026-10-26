
#include "stdafx.h"

#include "ui\Game\Gadgets\UIStaticScene.h"

#include "ui\Illustrator\UIAdobeShape.h"
#include "ui\Util\WideString.h"

#include "ui\Graphics\Font\Font.h"

#ifdef CFG_ALLOW_OPTIMIZE_OFF_UI
#pragma optimize("", off)
#endif

// XT Definition
XT_TYPE( UI::UIStaticScene, "UIStaticScene" )
	XT_BASE( UI::BaseUIShape )
XT_END_TYPE

namespace UI
{

void UIStaticScene::Init()
{
	BaseUIShape::Init();
}


void UIStaticScene::Setup( lua_State* pState )
{
	BaseUIShape::Setup( pState );
}


void UIStaticScene::Render()
{
	if( !m_Activated || m_pSceneHandle )
		return;

	C_UIAutoTransform transform( this );
	BaseUIShape::Render();

	gSimple2D.AddSprite( m_Layer, m_Position, m_Size.x, m_Size.y, 0x77777777, m_Panel );
	gSimple2D.AddTexUVs( 0, gUITextureManager.GetTextureByName( "null" ) );
	gSimple2D.AddJustification( m_Justify );

	const TransformIndex colourIndex = (U8)gSimple2D.PopColourTransformMatrix();

	TransformIndex ti = 0;
	//CalcTransformHierarchy( ti );

	Maths::HVec3 drawPos = m_Position + ( gSimple2D.GetJustifiedCorner( m_Justify, 0, m_Panel ) * m_Size );

	// Render all of the shapes in the list. Do this backwards!
	for ( S32 shape = m_ShapeRenderList.Size()-1; shape >= 0; shape-- )
	{
		const C_AdobeShape* pShape = m_ShapeRenderList[ shape ];

		if( !pShape )
			continue;

		Maths::HVec3 realPos = drawPos + Maths::HVec3( m_Size.x * pShape->ScalarOffset().X(), ( m_Size.y * pShape->ScalarOffset().Y() ) * Up(), m_Size.z );
		
		const F32 scalarSizeX = m_Size.x * pShape->ScalarValue().X();
		const F32 realSizeX = scalarSizeX + ( scalarSizeX * ( 1.0f / pShape->BoundingBox().Max().X() ) );
		
		const F32 scalarSizeY = m_Size.y * pShape->ScalarValue().Y();
		const F32 realSizeY = scalarSizeY + ( scalarSizeY * ( 1.0f / pShape->BoundingBox().Max().Y() ) );

		// Draw the surface data if one exists.
		if ( pShape->HasFill() )
		{
			m_RenderState.m_EffectParam = colourIndex;
			m_RenderState.m_EffectIndex = m_bFinnedOuter ? E_Effect_PolygonFlatFinnedOuter : E_Effect_PolygonFlat;

			RenderFill( pShape, realPos, realSizeX, realSizeY * Up(), (U8)ti );
		}

		// Draw the outline first if one exists.
		if ( pShape->HasStroke() )
		{				
			m_RenderState.m_EffectParam = colourIndex;
			m_RenderState.m_EffectIndex = m_StrokeEffect;

			RenderStroke( pShape, realPos, realSizeX, realSizeY * Up(), (U8)ti );
		}
	}
}


void UIStaticScene::RenderFill( const C_AdobeShape* pShape, const Maths::HVec3& drawPos, const F32& drawSizeX, const F32& drawSizeY, const U8& transformIndex )
{
	//BaseUIShape::RenderFill( pShape, drawPos, drawSizeX, drawSizeY, transformIndex );
}

void UIStaticScene::RenderStroke( const C_AdobeShape* pShape, const Maths::HVec3& drawPos, const F32& drawSizeX, const F32& drawSizeY, const U8& transformIndex )
{
	//BaseUIShape::RenderStroke( pShape, drawPos, drawSizeX, drawSizeY, transformIndex );
}


}; // namespace UI

#ifdef CFG_ALLOW_OPTIMIZE_OFF_UI
#pragma optimize("", on) 
#endif
