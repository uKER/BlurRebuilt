
#include "Stdafx.h"

#include "UIDynamicShape.h"

#include "ui/illustrator/UIAdobeSceneManager.h"
#include "ui/Illustrator/UIAdobeScene.h"
#include "ui/Illustrator/UIAdobeGroup.h"
#include "ui/Illustrator/UIAdobeShape.h"
#include "ui/Graphics/SimpleTimer.h"

#ifdef CFG_ALLOW_OPTIMIZE_OFF_UI
#pragma optimize("", off)
#endif

XT_TYPE( UI::UIDynamicShape, "UIDynamicShape" )
XT_END_TYPE

namespace UI
{

UIDynamicShape::UIDynamicShape()
:	UIStaticShape(),
	m_CurrentLayer( 0 ),
	m_CurrentGroup( 1 ),
	m_CurrentElement( 0 )
{
}


UIDynamicShape::~UIDynamicShape()
{
}

void UIDynamicShape::Init()
{
	UIStaticShape::Init();
}

void UIDynamicShape::Setup()
{
	UIStaticShape::Setup();

	if ( m_SceneIndex != m_SceneInvalid )
	{
		// Overight the render list with nothing.
		C_AdobeScene* pSceneHandle = C_AdobeSceneManager::Instance().GetScenePtr( m_SceneIndex );

		if ( pSceneHandle )
		{
			m_CurrentLayer = pSceneHandle->GetNumOfLayers()-1;
			m_CurrentGroup = pSceneHandle->GetLayer( m_CurrentLayer )->GetNumOfGroups()-1;

			// Force it to build an empty list.
			pSceneHandle->BuildDynamicRenderList( m_ShapeRenderList, m_CurrentLayer+1, m_CurrentGroup+1, 0 );
		}
		else
		{
			// Force the effect to not run.
			m_CurrentLayer = -1;
			m_CurrentGroup = -1;
		}
	}
	m_RenderEffectTimer = m_RenderEffectSpeed;
}


void UIDynamicShape::FrameUpdate()
{
	if ( m_SceneIndex == m_SceneInvalid )
		return;

	m_RenderEffectTimer += (1.0f / 1000.0f) * theFETimer.FrameTime();
	
	// Check that animation hasnt finished.
	if ( m_CurrentLayer > -1 && m_CurrentGroup > -1 )
	{
		if ( m_RenderEffectTimer > ( m_RenderEffectSpeed - m_RenderEffectModifier ) )
		{
			// Get scene and group details.
			C_AdobeScene* pSceneHandle = C_AdobeSceneManager::Instance().GetScenePtr( m_SceneIndex );
			C_AdobeGroup* pGroup = pSceneHandle->GetLayer( m_CurrentLayer )->GetGroup( m_CurrentGroup );

			if ( pGroup )
			{
				if ( m_CurrentElement >= pGroup->GetNumOfShapes() )
				{
					// If at the last group.
					if ( --m_CurrentGroup < 0 )
					{
						// Make sure were not past the last layer otherwise boom.
						if ( --m_CurrentLayer >= 0 )
							m_CurrentGroup = pSceneHandle->GetLayer( m_CurrentLayer )->GetNumOfGroups()-1;
						else
						{
							m_RenderEffect = E_UiRenderEffect_None; // Turn it off.
							return;
						}
					}
					m_CurrentElement = 0;
				}
				m_RenderEffectElement = m_CurrentElement;
				m_CurrentElement++; // Increase number of shapes to draw.

				// Need to modify the timer based on the size factor
				C_AdobeGroup* pGroup = pSceneHandle->GetLayer( m_CurrentLayer )->GetGroup( m_CurrentGroup );
				C_AdobeShape* pShape = pGroup->GetShape( m_CurrentElement-1 );

				if ( pShape )
				{
					F32 max = pSceneHandle->GetMaxShapeAreaSqrt();
					F32 min = pSceneHandle->GetMinShapeAreaSqrt();
					F32 total_diff = max - min;

					F32 area = sqrt( pShape->m_Bounds.SurfaceArea() );
					F32 area_diff = max - area;

					F32 frac = (area_diff / total_diff); // 1.0f smallest : 0.0f largest

					// This will speed / slow the render effect depending on the scale factor.
					m_RenderEffectModifier = ( m_RenderEffectSpeed * frac );
				}
			}
			// Reset base class variables.
			m_RenderEffectTimer = 0.0f;

			// Update render list.
			pSceneHandle->BuildDynamicRenderList( m_ShapeRenderList, m_CurrentLayer, m_CurrentGroup, m_CurrentElement );
		}
	}
	UIStaticShape::FrameUpdate();
}


void UIDynamicShape::Render()
{
	UIStaticShape::Render();
}

} // namespace UI

#ifdef CFG_ALLOW_OPTIMIZE_OFF_UI
#pragma optimize("", on) 
#endif
