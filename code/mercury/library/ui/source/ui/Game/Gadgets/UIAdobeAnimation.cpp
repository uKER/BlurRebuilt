
#include "stdafx.h"

#include "Ui/Game/Gadgets/UIAdobeAnimation.h"

#include "Ui/Illustrator/UIAdobeGroup.h"
#include "Ui/Illustrator/UIAdobeShape.h"
#include "Ui/Game/Gadgets/ButtonMessages.h"
#include "Ui/Graphics/SimpleTimer.h"

XT_TYPE( UI::UIAdobeAnimation, "UIAdobeAnimation" )
	XT_BASE( UI::UIAdobeTrail )
	XT_FIELD( m_AnimationTime, "animation_time" )
XT_END_TYPE

namespace UI
{

void UIAdobeAnimation::Init()
{
	UIAdobeTrail::Init();

	for ( U32 i = 0; i < m_MaxLayers; i++ )
	{
		m_CurrentFrame[i] = 0;
		m_LayerTimer[i] = 0.0f;
	}
}


void UIAdobeAnimation::Setup( lua_State* pState )
{
	UIAdobeTrail::Setup( pState );

	AddSubscriber( E_UiMsg_FrameUpdate );
}


void UIAdobeAnimation::FrameUpdate()
{
/*
	UIAdobeTrail::FrameUpdate();

	m_pSceneHandle = C_AdobeSceneManager::Instance().GetHandle( m_SceneIndex );
	m_ShapeRenderList.Clear();

	if ( !m_pSceneHandle )
		return;

	for ( U32 layer = 0; layer < (U32)pScene->GetNumOfLayers(); layer++ )
	{
		C_AdobeGroup* pLayer = pScene->GetLayer( layer );

		m_LayerTimer[layer] += theFETimer.FrameTime();

		if ( m_LayerTimer[layer] > ( m_AnimationTime / pLayer->GetNumOfGroups() ) )
		{
			m_CurrentFrame[layer]++;
			m_LayerTimer[layer] = 0.0f;

			if ( m_CurrentFrame[layer] >= (U32)pLayer->GetNumOfGroups() )
			{
				m_CurrentFrame[layer] = 0;
			}
		}

		if ( m_CurrentFrame[layer] < (U32)pLayer->GetNumOfGroups() )
		{
			C_AdobeGroup* pGroup = pLayer->GetGroup( m_CurrentFrame[layer] );

			for ( U32 shape = 0; shape < (U32)pGroup->GetNumOfShapes(); shape++ )
			{
				m_ShapeRenderList.PushBack( pGroup->GetShape( shape ) );
			}
		}
	}
*/
}


}; // namespace UI