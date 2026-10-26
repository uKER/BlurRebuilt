#ifndef CROWD_CAMERA_FLASHES_H
#define CROWD_CAMERA_FLASHES_H

#ifdef _MSC_VER
#pragma once
#endif

#include <crowd_system/CrowdDefines.h>

#include <Dynamic/Node.h>

#include <btl/containers/Vector.h>
#include <btl/containers/OpenHashSet.h>
#include <btl/containers/BitVector.h>

namespace DynamicEffect
{
	namespace Particle
	{
		class C_Event;
	};

	class C_Node;
};

namespace GameResource
{
	class C_CrowdData;
};

namespace AnimatedCrowd
{
	class C_CrowdVisibleBlockData;
};

namespace AnimatedCrowd
{

//! C_CrowdCameraFlashes

class C_CrowdCameraFlashes
{
public:
	C_CrowdCameraFlashes(DynamicEffect::Particle::C_Event* pEvent);

	~C_CrowdCameraFlashes();

	void	Enable(const U32 spawnRate, const F32 effectWaitTime);
	void	Disable();

	void	EnableBehaviour(const S32 behaviourIndex);

	void	Update(const F32 deltaTime, const C_CrowdVisibleBlockData& visibleBlockData, const GameResource::C_CrowdData* pCrowdData);

	const	CrowdEffectSystemType GetType();

private:
	void	UpdateFirstFree();

	struct Instance
	{
	public:
		Instance()
			: InstanceIndex(-1),
			TimeRemaining(0.f)
		{
		}

		Instance(const U32 instanceIndex, const F32 lifetime)
			: InstanceIndex(instanceIndex),
			TimeRemaining(lifetime)
		{
		}

		void Set(const U32 instanceIndex, const F32 lifetime)
		{
			InstanceIndex = instanceIndex;
			TimeRemaining = lifetime;
		}

		void Update(const F32 deltaTime)
		{
			TimeRemaining -= deltaTime;
		}

		const U32 GetInstanceIndex() const { return InstanceIndex; }
		const bool Finished() const { return TimeRemaining <= 0.f; }

	private:
		U32	InstanceIndex;
		F32	TimeRemaining;
	};

	DynamicEffect::Particle::C_Event*	m_pEvent;
	Btl::Vector<DynamicEffect::C_Node, Btl::VectorParameters_Static<CROWD_MAX_CAMERA_FLASHES> >	m_Nodes;

	Btl::Vector<Instance, Btl::VectorParameters_Array<CROWD_MAX_CAMERA_FLASHES> >
										m_Instances;
	
	Btl::BitVector						m_InstanceBits;
	Btl::OpenHashSet<U32>				m_InstanceHashSet;

	U32									m_FirstFree;

	U32									m_SpawnRate;
	F32									m_WaitTime;

	F32									m_SpawnTime;

	Btl::BitArray128					m_EnabledBehaviours;
};

}; // namespace AnimatedCrowd

#endif // CROWD_CAMERA_FLASHES_H
