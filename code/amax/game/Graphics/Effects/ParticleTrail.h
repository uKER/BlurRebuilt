/////////////////////////////////////////////////////////////
// ParticleTrail
/////////////////////////////////////////////////////////////

#ifndef __PARTICLE_TRAIL_H__
#define __PARTICLE_TRAIL_H__

namespace WorldRender { class C_LightSampler; }

#include <Graphics/Effects/EffectTypes.h>

#include <Dynamic/Particle/EventInstance.h>
#include <Dynamic/Particle/EventInstanceHandle.h>
#include <Misc_Shared/Curve/HermiteCurve.h>

#include <Vehicle/PartCodes.h>

namespace DynamicEffect { namespace Particle { class C_ParticleManager; }}

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

class C_ParticleTrail
{
public:

	C_ParticleTrail();

	~C_ParticleTrail();

	//! Initialise the particle trail
	void				Initialise(DynamicEffect::C_Node* p_pLocalSpace, float p_fQuality);

	//! Shuts down the particle trail
	void				Shutdown();

	//! Clears emitters
	void				Clear();

	//! Spawns paticles when needed
	void				Update(Maths::HMat43 p_Matrix, Maths::C_RGBA_F32 p_Lighting, bool p_Enable, float p_LifeScale, float p_EmitScale);

	//! Sets the emit position
	void				SetEmitPosition(Maths::HVec3 p_Position) { m_EmitPosition = p_Position; }

	//! Debug draw emit position
	void				DebugDraw(Maths::HMat43 p_Matrix);

public:

	E_MiscParticleEventType							m_EffectType;

	Vehicle::E_PartCode								m_PartCode;
	Vehicle::E_PartSubCode							m_PartSubCode;

	Maths::HVec3									m_EmitPosition;
	DynamicEffect::Particle::C_EventInstanceHandle	m_ParticleEvent;

	Maths::C_RGBA_F32								m_Lighting;
};

} //namespace Effects

} //namespace Game

#endif
