/////////////////////////////////////////////////////////////
// ParticleTrail
/////////////////////////////////////////////////////////////

#include "stdafx.h"

#include <Graphics/Effects/ParticleTrail.h>

#include <Dynamic/Particle/Generic/XtInterface.h>
#include <Dynamic/Particle/Event.h>
#include <Dynamic/Particle/EventInstance.h>
#include <Dynamic/Common.h>
#include <Dynamic/Renderer.h>
#include <Dynamic/Particle/Manager.h>

#include <GameLogic/GameData/GameEffectData.h>

Debug::Logger &g_ParticleTrailEffectLog = Debug::GetLogger("amax.effects.particletrail");

namespace Game
{

namespace Effects
{

/* 
===============================================================
					class C_ParticleTrail

NOTES: 
===============================================================
*/

/*
===============
NAME: C_ParticleTrail

NOTE: Constructor
===============
*/
C_ParticleTrail::C_ParticleTrail()
{
	Shutdown();
}

/*
===============
NAME: ~C_ParticleTrail

NOTE: Destructor
===============
*/
C_ParticleTrail::~C_ParticleTrail()
{
	Shutdown();
}

/*
===============
NAME: Initialise

NOTE: Initialise the particle trail
===============
*/
void C_ParticleTrail::Initialise(DynamicEffect::C_Node* p_pLocalSpace, float p_fQuality)
{
	int mapping = C_GameEffectData::Instance().mAmaxMiscEffects[0]->effectMapping[m_EffectType];
	if(mapping > -1 && C_GameEffectData::Instance().mAmaxMiscEffects[0]->effects[mapping].particleEvent.Object())
	{
		if(!m_ParticleEvent.Object())
		{
			DynamicEffect::C_Node node;
			node.SetPosition(m_EmitPosition);
			//m_ParticleEvent = C_GameEffectData::Instance().mAmaxMiscEffects[0]->effects[mapping].particleEvent->pEvent->Create(node, &m_LocalSpace);
			m_ParticleEvent = C_GameEffectData::Instance().mAmaxMiscEffects[0]->effects[mapping].particleEvent->pEvent->Create(node, p_pLocalSpace);
		}
	}
	else
	{
		if(DynamicEffect::Particle::C_EventInstance* pEventInstance = m_ParticleEvent.Object())
			pEventInstance->Free();
	}	
}

/*
===============
NAME: Shutdown

NOTE: Shuts down the particle trail
===============
*/
void C_ParticleTrail::Shutdown()
{
	m_EffectType	= E_MiscParticleEventType_RearLightTrail;
	m_PartCode		= Vehicle::E_PartCode_None;
	m_PartSubCode	= Vehicle::E_PartSubCode_None;

	m_EmitPosition	= Maths::HVec3(0.0f, 0.0f, 0.0f);

	if(DynamicEffect::Particle::C_EventInstance* pEventInstance = m_ParticleEvent.Object())
		pEventInstance->Free();
}

/*
===============
NAME: Clear

NOTE: Clears emitters
===============
*/
void C_ParticleTrail::Clear()
{
	if(DynamicEffect::Particle::C_EventInstance* pEventInstance = m_ParticleEvent.Object())
		pEventInstance->Free();
}

/*
===============
NAME: Update

NOTE: Spawns paticles when needed
===============
*/
void C_ParticleTrail::Update(Maths::HMat43 p_Matrix, Maths::C_RGBA_F32 p_Lighting, bool p_Enable, float p_LifeScale, float p_EmitScale)
{
	if(DynamicEffect::Particle::C_EventInstance* pEventInstance = m_ParticleEvent.Object())
	{
		pEventInstance->SetIntensity(p_Enable ? p_EmitScale : 0.0f);
		pEventInstance->SetLifeScale(p_LifeScale);
		pEventInstance->SetLighting(p_Lighting);
	}	
}

/*
===============
NAME: DebugDraw

NOTE: Debug draw emit position
===============
*/
void C_ParticleTrail::DebugDraw(Maths::HMat43 p_Matrix)
{
	DbUtil::I_DebugRender *pDebugRenderer = DbUtil::DefaultDebugRenderSys()->Deferred();
	
	Maths::HVec3 pos = m_EmitPosition;
	pos = pos * p_Matrix;
	pDebugRenderer->Draw3dCross(pos, 0.1f, Maths::RGBA(0.0f, 1.0f, 1.0f));
}

} //namespace Actors

} //namespace Game

