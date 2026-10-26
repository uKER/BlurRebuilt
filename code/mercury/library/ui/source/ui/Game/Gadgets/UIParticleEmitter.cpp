#include "stdafx.h"

#include "ui/Game/Gadgets/UIParticleEmitter.h"
#include "ui/Game/Gadgets/ButtonMessages.h"
#include "ui/Graphics/SimpleTimer.h"
#include "ui/Graphics/UIParticle.h"

#ifdef CFG_ALLOW_OPTIMIZE_OFF_UI
#pragma optimize("", off)
#endif

XT_TYPE( UI::UIParticleEmitter, "UIParticleEmitter" )
	XT_BASE( UI::C_GadgetBase )
	XT_FIELD( m_Time,			"start_time" )
	XT_FIELD( m_EmitRate,		"emit_rate" )
	XT_FIELD( m_CycleType,		"cycle_type" )
	XT_FIELD( m_Duration,		"duration" )

	XT_FIELD( m_AngleInc,		"angle_inc" )
	XT_FIELD( m_ScaleInc,		"scale_inc" )

	XT_FIELD( m_SpawnSize,		"spawn_size" )
	XT_FIELD( m_SpawnScale,		"spawn_scale" )
	XT_FIELD( m_SpawnLife,		"spawn_life" )
	XT_FIELD( m_SpawnAngle,		"spawn_angle" )

	XT_FIELD( m_SpawnMinVel,	"velocity_min" )
	XT_FIELD( m_SpawnMaxVel,	"velocity_max" )
	XT_FIELD( m_SpawnMinAcc,	"acceleration_min" )
	XT_FIELD( m_SpawnMaxAcc,	"acceleration_max" )
XT_END_TYPE

XT_TYPE( UI::UIParticleEmitter::E_CycleType, "UIEmitterCycleType" )
	XT_OPTION( UI::UIParticleEmitter::E_CycleType_Constant, "Constant" )
	XT_OPTION( UI::UIParticleEmitter::E_CycleType_Duration, "Duration" )
XT_END_TYPE


namespace UI
{

UIParticleEmitter::~UIParticleEmitter()
{
	gUIParticleMan.RemovePool( m_PoolId );
}

void UIParticleEmitter::Init( )
{
	C_GadgetBase::Init();

	m_Running = true;
	m_Rand.Seed(77754321);
	m_PoolId = gUIParticleMan.AddPool();

	AddSubscriber( E_UiMsg_FrameUpdate );
}

void UIParticleEmitter::FrameUpdate( )
{
	if ( !m_Activated || !m_Running )
		return;

	// Does the emitter have a run time?
	if ( m_CycleType == E_CycleType_Duration )
	{
		if ( m_Duration <= 0.0f )
		{
			m_Duration = 0.0f;
			m_Running = false;
			return;
		}
		m_Duration -= theFETimer.FrameTime();
	}

	// Emit a new particle if its time.
	m_Time += theFETimer.FrameTime();

	if ( m_Time >= m_EmitRate )
	{
		m_Time = 0.0f;
		EmitParticle();
	}

	C_GadgetBase::FrameUpdate();
}


void UIParticleEmitter::Render( )
{
	C_GadgetBase::Render();
}


void UIParticleEmitter::EmitParticle()
{
	C_UIParticle* pParticle		= gUIParticleMan.Get( m_PoolId );

	if ( !pParticle )
		return;

	pParticle->m_Life			= m_Rand.FRand( m_SpawnLife.mean - m_SpawnLife.d, m_SpawnLife.mean + m_SpawnLife.d );
	pParticle->m_TotalLife		= pParticle->m_Life;

	pParticle->m_Vel			= Maths::HVec3( m_Rand.FRand( m_SpawnMinVel.x, m_SpawnMaxVel.x ), 
												m_Rand.FRand( m_SpawnMinVel.y, m_SpawnMaxVel.y ), 
												m_Rand.FRand( m_SpawnMinVel.z, m_SpawnMaxVel.z ) );

	pParticle->m_Acc			= Maths::HVec3( m_Rand.FRand( m_SpawnMinAcc.x, m_SpawnMaxAcc.x ), 
												m_Rand.FRand( m_SpawnMinAcc.y, m_SpawnMaxAcc.y ), 
												m_Rand.FRand( m_SpawnMinAcc.z, m_SpawnMaxAcc.z ) );

	pParticle->m_Scale			= m_Rand.FRand( m_SpawnScale.mean - m_SpawnScale.d, m_SpawnScale.mean + m_SpawnScale.d );
	pParticle->m_Size			= m_SpawnSize;


	Maths::HVec3 offset = m_Size * Maths::HVec3( m_Rand.FRand( 1.0f ), m_Rand.FRand( 1.0f ), m_Rand.FRand( 1.0f ) );
	pParticle->m_Pos			= offset + ( m_Position - ( m_Size * gSimple2D.GetJustificationOffset( m_Justify, m_Panel ) ) );

	pParticle->m_Angle			= m_Rand.FRand( m_SpawnAngle.mean - m_SpawnAngle.d, m_SpawnAngle.mean + m_SpawnAngle.d );
	pParticle->m_AngleInc		= m_Rand.FRand( m_AngleInc.mean - m_AngleInc.d, m_AngleInc.mean + m_AngleInc.d );
	pParticle->m_ScaleInc		= m_Rand.FRand( m_ScaleInc.mean - m_ScaleInc.d, m_ScaleInc.mean + m_ScaleInc.d );

	pParticle->m_Layer			= m_Layer;
	pParticle->m_Panel			= m_Panel;
	pParticle->m_AlwaysReset	= false;
	pParticle->m_RenderState	= m_RenderState;

	pParticle->m_ColourFrom		= m_SpriteColours;
	pParticle->m_ColourTo		= m_SpriteColours;

	// Note only fade out 
	if( m_RenderState.m_BlendMode == Display::E_BlendMode_Additive )
		pParticle->m_ColourTo.SetIntensity(0);
	else
		pParticle->m_ColourTo.SetAlpha(0);

	// Load up the texture states
	pParticle->m_NumTextureStages = (m_TextureParams.count < C_UIParticle::m_MaxTextureStages) ? m_TextureParams.count : C_UIParticle::m_MaxTextureStages;
	for(S32 i = 0; i < pParticle->m_NumTextureStages; i++)
		pParticle->m_Texture[i] = m_TextureParams[i];
}


}; // namespace UI

#ifdef CFG_ALLOW_OPTIMIZE_OFF_UI
#pragma optimize("", on) 
#endif
