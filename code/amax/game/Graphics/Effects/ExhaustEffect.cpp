/////////////////////////////////////////////////////////////
// VehicleWheel
/////////////////////////////////////////////////////////////

#include "stdafx.h"

#include <Graphics/Effects/ExhaustEffect.h>

#include <world_render/LightSamplePointScene.h>

#include <Dynamic/Particle/Generic/XtInterface.h>
#include <Dynamic/Particle/Event.h>
#include <Dynamic/Particle/EventInstance.h>
#include <Dynamic/Common.h>
#include <Dynamic/Renderer.h>
#include <Dynamic/Particle/Manager.h>

#include <GameLogic/GameData/GameEffectData.h>

Debug::Logger &g_ExhaustEffectLog = Debug::GetLogger("amax.effects");

namespace Game
{

namespace Effects
{

/* 
===============================================================
					class C_ExhaustEffect

NOTES: 
===============================================================
*/

/*
===============
NAME: C_ExhaustEffect

NOTE: Constructor
===============
*/
C_ExhaustEffect::C_ExhaustEffect()
{
	Shutdown();	
}

/*
===============
NAME: ~C_ExhaustEffect

NOTE: Destructor
===============
*/
C_ExhaustEffect::~C_ExhaustEffect()
{
	Shutdown();
}

/*
===============
NAME: Initialise

NOTE: Initialises the exhaust effect
===============
*/
void C_ExhaustEffect::Initialise(DynamicEffect::C_Node* p_pLocalSpace)
{
	m_pLocalSpace		= p_pLocalSpace;
}

/*
===============
NAME: Shutdown

NOTE: Shuts down the exhaust effect
===============
*/
void C_ExhaustEffect::Shutdown()
{
	m_pLocalSpace = NULL;

	for (int i = 0; i < E_ExhuastEffectPosition_Max; i++)
	{
		m_EmitPositions[i].m_Position = Maths::HVec3(0.0f, 0.0f, 0.0f);
		m_EmitPositions[i].m_Direction = Maths::HVec3(0.0f, 0.0f, 0.0f);
		m_EmitPositions[i].m_Exists = false;
		m_EmitPositions[i].m_fExhaustFlameIntensity = 0;

		if(DynamicEffect::Particle::C_EventInstance* pEventInstance = m_EmitPositions[i].m_Smoke.Object())
			pEventInstance->Free();
	}

	//DynamicEffect::C_Renderer::Instance().CleanNode(&m_LocalSpace);
}

/*
===============
NAME: Update

NOTE: Spawns flame paticles when needed
===============
*/
void C_ExhaustEffect::Update(Maths::HMat43 p_Matrix, F32 p_fTimeStep, Maths::C_RGBA_F32 p_Lighting, float p_fQuality, U32 visibilityFlags)
{
	if (!m_pLocalSpace)
		return;

	PERF_EVENT("C_ExhaustEffect::Update");

	m_Lighting = p_Lighting;

	//Check smoke event has been set up and isnt a null record
	int exhaustSmokeMapping = C_GameEffectData::Instance().mAmaxMiscEffects[0]->effectMapping[E_MiscParticleEventType_ExhaustSmoke];
	if(exhaustSmokeMapping > -1 && C_GameEffectData::Instance().mAmaxMiscEffects[0]->effects[exhaustSmokeMapping].particleEvent.Object())
	{
		for(S32 i = 0; i < m_EmitPositions.Size(); ++i)
		{
			if (!m_EmitPositions[i].m_Exists)
				continue;

			if(DynamicEffect::Particle::C_EventInstance* pEventInstance = m_EmitPositions[i].m_Smoke.Object())
			{
				pEventInstance->SetIntensity(p_fQuality);
				pEventInstance->SetLighting(p_Lighting);
				pEventInstance->SetVisibilityFlags(visibilityFlags);
			}
			else
			{
				DynamicEffect::C_Node node;
				node.SetPosition(m_EmitPositions[i].m_Position);
				node.SetRotation(m_EmitPositions[i].m_Rotation);
				m_EmitPositions[i].m_Smoke = C_GameEffectData::Instance().mAmaxMiscEffects[0]->effects[exhaustSmokeMapping].particleEvent->pEvent->Create(node, m_pLocalSpace);
			}
		}
	}
	else
	{
		for(S32 i = 0; i < m_EmitPositions.Size(); ++i)
		{
			if(DynamicEffect::Particle::C_EventInstance* pEventInstance = m_EmitPositions[i].m_Smoke.Object())
				pEventInstance->Free();
		}
	}	
}

/*
===============
NAME: AddEmitPosition

NOTE: Adds emit position to exhaust flame effect
===============
*/
void C_ExhaustEffect::AddEmitPosition(E_ExhuastEffectPosition p_Index, Maths::HVec3 p_Position, Maths::HVec3 p_Direction)
{
	if (p_Index < 0 || p_Index >= E_ExhuastEffectPosition_Max)
		return;

	m_EmitPositions[p_Index].m_Position = p_Position;
	m_EmitPositions[p_Index].m_Direction = p_Direction;
	m_EmitPositions[p_Index].m_Rotation = DynamicEffect::GetQuaternionFromTo<Maths::C_Quaternion>(Maths::Constants<Maths::HVec3>::UnitAxisPositiveZ(), p_Direction);
	m_EmitPositions[p_Index].m_Exists = true;
	m_EmitPositions[p_Index].m_fExhaustFlameIntensity = 0.f;
}

/*
===============
NAME: SpawnEffect

NOTE: Spawns an effect
===============
*/
void C_ExhaustEffect::SpawnEffect(E_ExhuastEffectType p_Type, float p_fQuality, U32 visibilityFlags)
{
	if (p_Type < 0 || p_Type >= E_ExhuastEffectType_Max || C_GameEffectData::Instance().mAmaxMiscEffects.Number() == 0)
		return;

	int index = 0;
	int num = 0;

	E_MiscParticleEventType eventType = E_MiscParticleEventType_ExhaustPop;

	switch (p_Type)
	{
	case E_ExhuastEffectType_ExhaustPop:
		index = 0;
		num = E_ExhuastEffectPosition_Max;
		eventType = E_MiscParticleEventType_ExhaustPop;
		break;
	case E_ExhuastEffectType_ExhaustBang:
		index = 0;
		num = E_ExhuastEffectPosition_Max;
		eventType = E_MiscParticleEventType_ExhaustBang;
		break;
	case E_ExhuastEffectType_ExhaustPopLeft:
		index = E_ExhuastEffectPosition_Left1;
		num = E_ExhuastEffectPosition_Right1;
		eventType = E_MiscParticleEventType_ExhaustPop;
		break;
	case E_ExhuastEffectType_ExhaustBangLeft:
		index = E_ExhuastEffectPosition_Left1;
		num = E_ExhuastEffectPosition_Right1;
		eventType = E_MiscParticleEventType_ExhaustBang;
		break;
	case E_ExhuastEffectType_ExhaustPopRight:
		index = E_ExhuastEffectPosition_Right1;
		num = E_ExhuastEffectPosition_Max;
		eventType = E_MiscParticleEventType_ExhaustPop;
		break;
	case E_ExhuastEffectType_ExhaustBangRight:
		index = E_ExhuastEffectPosition_Right1;
		num = E_ExhuastEffectPosition_Max;
		eventType = E_MiscParticleEventType_ExhaustBang;
		break;
	default:
		return;
		break;	
	}

	int mapping = C_GameEffectData::Instance().mAmaxMiscEffects[0]->effectMapping[eventType];
		
	if (mapping > -1)
	{
		for (index; index < num; index++)
		{
			if (!m_EmitPositions[index].m_Exists)
				continue;

			Maths::HVec3 position = m_EmitPositions[index].m_Position;
			if (eventType == E_MiscParticleEventType_ExhaustBang)
				m_EmitPositions[index].m_fExhaustFlameIntensity = 1.f; // intensity of the light being cast by the exhaust flame

			DynamicEffect::C_Node node;
			node.SetPosition(position);
			node.SetRotation(m_EmitPositions[index].m_Rotation);
			//DynamicEffect::Particle::C_EventInstanceHandle emitter = C_GameEffectData::Instance().mAmaxMiscEffects[0]->effects[mapping].particleEvent->pEvent->Create(node, &m_LocalSpace);
			DynamicEffect::Particle::C_EventInstanceHandle emitter = C_GameEffectData::Instance().mAmaxMiscEffects[0]->effects[mapping].particleEvent->pEvent->Create(node, m_pLocalSpace);
			
			if(DynamicEffect::Particle::C_EventInstance* pEvent = emitter.Object())
			{
				pEvent->SetLighting(m_Lighting);
				pEvent->SetIntensity(p_fQuality);
				pEvent->SetVisibilityFlags(visibilityFlags);
			}
		}
	}
	else
	{
		//LOGERROR(g_VehicleEffectsMessageLogger, "Trying to access particle event that doesn't exist! (%d)", eventType);
		return;
	}
}
/*
===============
NAME: DebugDraw

NOTE: Debug draw emit positions
===============
*/
void C_ExhaustEffect::DebugDraw(Maths::HMat43 p_Matrix)
{
	DbUtil::I_DebugRender *pDebugRenderer = DbUtil::DefaultDebugRenderSys()->Deferred();
	
	for (S32 iPos = 0; iPos < m_EmitPositions.Size(); iPos++)
	{
		if (!m_EmitPositions[iPos].m_Exists)
			continue;

		Maths::HVec3 pos = m_EmitPositions[iPos].m_Position;
		pos = pos * p_Matrix;
		Maths::HVec3 dir = m_EmitPositions[iPos].m_Direction * -1.0f;
		dir = dir * p_Matrix.Rotation();
		pDebugRenderer->Draw3dCross(pos, 0.1f, Maths::RGBA(0.0f, 1.0f, 0.0f));
		pDebugRenderer->Draw3dArrow(pos, dir, 0.5f, 0.1f, Maths::RGBA(0, 255, 0));
	}
}

} //namespace Actors

} //namespace Game

