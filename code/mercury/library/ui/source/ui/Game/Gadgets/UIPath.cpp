
#include "stdafx.h"

#include "ui/Game/Gadgets/UIPath.h"

#include "ui/Illustrator/UIAdobePath.h"
#include "ui/Illustrator/UIAdobeScene.h"
#include "ui/Illustrator/UIAdobeSceneManager.h"


#ifdef CFG_ALLOW_OPTIMIZE_OFF_UI
#pragma optimize("", off)
#endif

// XT Definition
XT_TYPE( UI::UIPath, "UIPath" )
	XT_BASE( UI::C_GadgetBase )	
	XT_FIELD( m_SceneName,	"scene_name" )	
	XT_FIELD( m_PathName,	"path_name" )	
	XT_FIELD( m_AnimationTime,	"animation_time" )	
XT_END_TYPE


namespace UI
{

void UIPath::Init()
{
	C_GadgetBase::Init();

	m_Spline = NULL;
	m_AnimationTime = 0.0f;
}


void UIPath::Setup( lua_State *pState )
{
	C_GadgetBase::Setup( pState );

	// Get spline from file.
	if ( m_SceneName && m_PathName )
	{
//		C_AdobeScene* pScene = C_AdobeSceneManager::Instance().GetScenePtrByName( m_SceneName );

//		if ( pScene )
		{
//			m_Spline = pScene->GetPathPtrByName( m_PathName );
		}
	}
	m_OriginalPos = m_Position;
}

void UIPath::Render()
{
	if ( !m_Activated )
		return;

	C_GadgetBase::Render();

	const F32 adobeScale = 1.0f / ILLUSTRATOR_SCALE;
	const F32 drawSizeX = (m_Size.x * adobeScale);
	// Flip the data if its in 3D
	const F32 drawSizeY = ( gSimple2D.IsPanel3D( m_Panel ) )  ? (m_Size.y * adobeScale) * -1.0f : (m_Size.y * adobeScale);
	const F32 offsetY = ( gSimple2D.IsPanel3D( m_Panel ) ) ? ( m_Size.y * 2.0f ) : 0.0f;

	// Draw position
	Maths::HVec3 drawPos = Maths::HVec3( m_OriginalPos.x, m_OriginalPos.y + offsetY, m_OriginalPos.z );

	// justifications
	drawPos += (gSimple2D.GetJustifiedCorner( m_Justify, 0, m_Panel ) * m_Size);

	if ( m_Spline )
	{
		m_Position = m_Spline->GetPositionAt( m_AnimationTime, m_OriginalPos, drawSizeX, drawSizeY, true );
		m_Rotation.z = m_Spline->GetRotationAt( m_AnimationTime, m_OriginalPos, drawSizeX, drawSizeY, true );
/*
#ifndef CFG_GOLD
		if ( g_UiRenderSplineInfo )
			m_Spline->RenderSplineInfo( m_AnimationTime, m_Layer, m_OriginalPos, drawSizeX, drawSizeY, m_Panel, true );
#endif
*/
	}
}


}; // namespace UI


