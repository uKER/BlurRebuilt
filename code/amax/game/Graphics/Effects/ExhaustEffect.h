/////////////////////////////////////////////////////////////
// ExhaustFlame
/////////////////////////////////////////////////////////////

#ifndef __EXHAUST_EFFECT_H__
#define __EXHAUST_EFFECT_H__

namespace WorldRender { class C_LightSampler; }

#include <Graphics/Effects/EffectTypes.h>

#include <Dynamic/Particle/EventInstance.h>
#include <Dynamic/Particle/EventInstanceHandle.h>
#include <Misc_Shared/Curve/HermiteCurve.h>

namespace DynamicEffect { namespace Particle { class C_ParticleManager; }}

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

class C_ExhaustEffect
{
public:

	C_ExhaustEffect();

	~C_ExhaustEffect();

	//! Initialises the exhaust effect
	void				Initialise(DynamicEffect::C_Node* p_pLocalSpace);

	//! Shuts down the exhaust effect
	void				Shutdown();

	//! Spawns flame paticles when needed
	void				Update(Maths::HMat43 p_Matrix, F32 p_fTimeStep, Maths::C_RGBA_F32 p_Lighting, float p_fQuality, U32 visibilityFlags);
	
	//! Spawns an effect
	void				SpawnEffect(E_ExhuastEffectType p_Type, float p_fQuality, U32 visibilityFlags);

	//! Sets the left position
	void				AddEmitPosition(E_ExhuastEffectPosition p_Index, Maths::HVec3 p_Position, Maths::HVec3 p_Direction);

	//! Debug draw emit positions
	void				DebugDraw(Maths::HMat43 p_Matrix);

public:

	struct EmitPosition
	{
		Maths::HVec3	m_Position;
		Maths::HVec3	m_Direction;
		Maths::C_Quaternion	m_Rotation;
		bool			m_Exists;
		F32				m_fExhaustFlameIntensity;
		DynamicEffect::Particle::C_EventInstanceHandle	m_Smoke;
	};

	Ut::UtVector<Ut::ArrayVector<EmitPosition, E_ExhuastEffectPosition_Max> > m_EmitPositions;

	//DynamicEffect::C_Node						m_LocalSpace;
	DynamicEffect::C_Node*						m_pLocalSpace;
	Maths::C_RGBA_F32							m_Lighting;
};

} //namespace Effects

} //namespace Game

#endif
