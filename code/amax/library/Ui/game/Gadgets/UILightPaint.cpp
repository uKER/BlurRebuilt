
#include "stdafx.h"
#include <ui/Graphics/SimpleTimer.h>	// [Blur2PC]

#include <System/Misc/Random.h>

#include <Ui/Game/Gadgets/ButtonMessages.h>
#include <Ui/Graphics/Simple2D.h>
#include <Ui/LightPaintSystem.h>

#include <Core/Maths/HVec/New/IncludeGeneral.h>
#include <Core/Maths/HVec/New/IncludeGeneral.inl>

#include "GameLogic/ActorStudioActors/UiLocatorActor.h"

#include "UIManager.h"
#include "UILightPaint.h"

// XT Definition
XT_TYPE( UI::UILightPaint, "UILightPaint" )
	XT_BASE( UI::C_GadgetBase )
	XT_FIELD( m_EmitTime, "emit_time" )
	XT_FIELD( m_EmitRate, "emit_rate" )
	XT_FIELD( m_xtLocatorNames, "location_names" )
	XT_FIELD( m_xtPositions, "positions" )
	XT_FIELD( m_ParticleEffect, "particleEffect" )
XT_END_TYPE

#ifndef CFG_GOLD
Debug::Bool gUiLightPaintShowDebug( "ui.LightPaint.ShowDebug", false );
Debug::Bool gUiLightPaintDynamicDebug( "ui.LightPaint.DynamicDebug", false );
#endif

namespace UI
{

void UILightPaint::PostInit()
{
	C_GadgetBase::PostInit();

	m_pControls = NULL;

	m_ControlSize = m_xtLocatorNames.count + m_xtPositions.count;
	if( m_ControlSize <= 1 )
		return;

	m_pControls = new SparkleControl[ m_ControlSize ];

	AddSubscriber(E_UiMsg_FrameUpdate);
}

void UILightPaint::Exit()
{
	SAFE_DELETE_ARY( m_pControls );
}

void UILightPaint::StoreControlPoints()
{
	XtI32 cnt = 0;
	// Copy location actor positions
	for( XtI32 i = 0; i < m_xtLocatorNames.count; i++ )
	{
		Actors::UiLocatorActor *p = Actors::UiLocatorActor::FindByName( m_xtLocatorNames[ i ] );
		if( !p )
		{
			LOGWARN( g_Log, DBSTR "location actor %s not found", m_xtLocatorNames[ i ].string );
			continue;
		}

		Store(Maths::MakeVector3( (F32)p->GetMatrix().AW().X(), (F32)p->GetMatrix().AW().Y(), (F32)p->GetMatrix().AW().Z() ), &m_pControls[ cnt ].position );
		cnt++;
	}

	// Copy direct positions
	Maths::HMat43 mat;
	CalcTransformHierarchy( mat );

	for( XtI32 i = 0; i < m_xtPositions.count; i++ )
	{
		Maths::HVec3 vec( m_xtPositions[ i ].vx, m_xtPositions[ i ].vy, m_xtPositions[ i ].vz );
		vec = vec * mat;

		Store(Maths::MakeVector3( (F32)vec.X(), (F32)vec.Y(), (F32)vec.Z() ), &m_pControls[ cnt ].position );
		cnt++;
	}

	m_ControlSize = cnt;
}

void UILightPaint::FrameUpdate()
{
	C_GadgetBase::FrameUpdate();

	if( !m_Activated )
		return;

	if( !m_ControlSize || !m_pControls )
		return;

	bool doIt = false;

	if( m_EmitRate <= 0.0f )
	{
		// Timed emitter
		doIt = true;
		m_Activated = false;
	}
	else
	if ( (m_EmitTime -= UI::theFETimer.FrameTime()) <= 0.0f )
	{
		// One shot
		doIt = true;
		m_EmitTime += m_EmitRate;
	}

	if( doIt )
	{
		StoreControlPoints();
		LightPaint_Emit( (int)m_SpriteColours[0], GetTextureParam( 0 ).m_TextureID,  m_ParticleEffect, (int)m_Layer, (int)m_Panel, m_Size.x, m_ControlSize, m_pControls);
	}

#ifndef CFG_GOLD
	if( gUiLightPaintShowDebug && m_Panel == E_GameSpritePanel_3DAA_WORLD ) // Only works with game camera
	{
		DbUtil::I_DebugRender* pDebugRenderer = DbUtil::DefaultDebugRenderSys()->Deferred();
		if( pDebugRenderer )
		{
			if( gUiLightPaintDynamicDebug && !doIt )
				StoreControlPoints();

			// Draw the lines
			for( S32 i = 0; i < m_ControlSize-1; i++ )
			{
				Maths::Vector3 vec0( Load( &m_pControls[ i+0 ].position ) );
				Maths::Vector3 vec1( Load( &m_pControls[ i+1 ].position ) );

				U8 v = (i&1);
				U8 r = 255 * ( v);
				U8 g = 255 * (!v);
				U8 b = 255;
		 
				pDebugRenderer->Draw3dLine( Maths::HVec3( vec0 ), Maths::HVec3( vec1 ), Maths::RGBA(r, g, b) );
			}
		}
	}
#endif
}

}; // namespace UI

