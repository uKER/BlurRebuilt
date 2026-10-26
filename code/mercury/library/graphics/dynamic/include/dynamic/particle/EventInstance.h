#ifndef RENDER_DYNAMICEFFECT_PARTICLE_EVENTINSTANCE_H
#define RENDER_DYNAMICEFFECT_PARTICLE_EVENTINSTANCE_H

#ifdef _MSC_VER
#pragma once
#endif

// include node here since it is unlikely to change
#include <Dynamic/Node.h>
#include <Dynamic/Handle.h>
#include <Dynamic/Atomic.h>

namespace Curve
{
	class C_ActorCurve3D;
};

namespace DynamicEffect
{
class C_Node;

namespace Particle
{
class C_Random;
class C_ParticleManager;
class C_Event;
class C_Emitter;
class C_SpawnMesh;
class I_Spawner;

class ALIGN16 C_EventInstance : public AtomicPoolElement
{
public:
	C_EventInstance() { assert(0); } // should never happen, needed for UtList though
	C_EventInstance(C_ParticleManager* pRenderer);
	~C_EventInstance();
	void		Destroy() {};

	// setting functions
	void		SetTransform(const C_Node& local, const C_Node* pParent = NULL);
	void		SetLighting(const MathsStorage::Float4A16& vLighting);
	void		SetLighting(const MathsStorage::Float4A16& vLighting, const MathsStorage::Float4A16& vTintColour);
	void		SetLighting(const Maths::C_RGBA_F32& vLighting);
	void		SetLighting(const Maths::C_RGBA_F32& vLighting, const Maths::C_RGBA_F32& vTintColour);
	
	void		SetIntensity(F32 fIntensity);
	void		SetAlpha(F32 fAlpha);
	void		SetLifeScale(F32 fLifeScale);
	void		SetActive(bool bActive);
	void		SetCollisionPlane(const MathsStorage::Float4A16& vCollisionPlanePosition, const MathsStorage::Float4A16& vCollisionPlaneNormal);
	void		SetCollisionPlane(const Maths::HVec3& vCollisionPlanePosition, const Maths::HVec3& vCollisionPlaneNormal);
	void		SetVelocity(const Maths::HVec3& vVelocity);
	void		SetVisibilityFlags(U32 visibilityFlags);

	// custom spawning methods
	void		SetSpawnMesh(const C_SpawnMesh* pSpawnMesh);
	void		SetSpline(const Curve::C_ActorCurve3D* pSpline);
	void		SetSplineUsage(const F32 fBegin, const F32 fEnd);
	void		SetSpawner(I_Spawner* pSpawner);

	void		Spawn(const F32 fFrameTime);
	void		Reset();
	void		Reset(C_Random& random);
	void		Free(); // should be called by client when finished
	void		FreeRemove(); // should be called by client when finished

	bool		Active() const { return m_bActive; }
	bool		Spawning(S32 iIndex) const;
	bool		Repeating() const { return m_bRepeating; }

	Maths::HVec3 GetWorldPosition() const;
	Maths::HVec3 GetWorldNormal() const;

	// get event
	const C_Event*	GetEvent() const { return m_pEvent; }
	// used by handles
	U32			GetSalt() const { return m_iSalt; }
	
	friend class C_Event;
	friend class C_ParticleManager;
	friend class C_Emitter;
private:
	S32			Update(F32 fFrameTime, C_Random& random);
	void		RemoveEmitter(const S32 index);

	// private variables
	C_ParticleManager*								m_pRenderer;
	const C_Event*									m_pEvent;
	Btl::Vector<C_Emitter*>							m_Emitters;
	U32												m_iSalt;
	F32												m_fCycle;
	F32												m_fDuration;
	F32												m_fIntensity;
	F32												m_fAlpha;
	F32												m_fLifeScale;
	bool											m_bActive;
	bool											m_bRepeating;
};

FORCE_INLINE void C_EventInstance::SetLighting(const MathsStorage::Float4A16& vLighting)
{
	SetLighting(Maths::Load<Maths::C_RGBA_F32>(&vLighting));
}

FORCE_INLINE void C_EventInstance::SetLighting(const MathsStorage::Float4A16& vLighting, const MathsStorage::Float4A16& vTintColour)
{
	SetLighting(Maths::Load<Maths::C_RGBA_F32>(&vLighting), Maths::Load<Maths::C_RGBA_F32>(&vTintColour));
}

INLINE void C_EventInstance::RemoveEmitter(const S32 index)
{
	assert(index >= 0 && index < m_Emitters.Size());
	Spu::Set((INTPTR*)&m_Emitters[index], (INTPTR)0);	// [Blur2PC][x64] pointer-wide
}

}
}

#endif // RENDER_DYNAMICEFFECT_PARTICLE_EVENTINSTANCE_H
