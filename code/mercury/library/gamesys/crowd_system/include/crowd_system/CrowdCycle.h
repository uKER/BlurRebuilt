#ifndef CROWD_CYCLE_H
#define CROWD_CYCLE_H

#ifdef _MSC_VER
#pragma once
#endif

#ifndef PLATFORM_SPU
#include <Core/Containers/String.h>
#endif // !PLATFORM_SPU

#include <crowd_system/CrowdDefines.h>

#ifndef PLATFORM_SPU
#include <animation/resource/CycleResource.h>
#endif // !PLATFORM_SPU

#include <btl/containers/Vector.h>
#include <btl/containers/BitVector.h>

namespace Animation
{
	class C_Cycle;
};

namespace AnimatedCrowd
{
	class C_CrowdCycle;
};

namespace AnimatedCrowd
{

//! CrowdCycleData
struct ALIGN16 CrowdCycleData
{
public:
	void Clear()
	{
		m_pCycle	= NULL;
		m_Duration	= 0.f;
		m_Weight	= 0.f;
		m_Action	= static_cast<U32>(E_CROWD_ACTION_UNKNOWN);
		m_Direction	= static_cast<U16>(E_ACTION_DIRECTION_ANY);
		m_Valid		= 0;
	}

	const Animation::C_Cycle*	m_pCycle;
	F32							m_Duration;
	F32							m_Weight;
	
	U32							m_Action		: 4;
	U32							m_Direction		: 2;
	U32							m_Valid			: 26; // Look at all that valid!
};

#ifndef PLATFORM_SPU
//! C_CrowdCycle
class C_CrowdCycle : public Resource::C_Listener
{
public:
	C_CrowdCycle();
	~C_CrowdCycle();

	void				Create(const Util::C_String& filename, CrowdCycleData* pCycleData);
	
	virtual void		ResourceNotify(Resource::C_ResourceBase* pResource, const Resource::E_Notify result, void* pData);

	const Animation::C_CycleResource*
						GetCycleResource() const;

private:
	Animation::C_CycleResourceHandle	m_CycleResourceHandle;
	CrowdCycleData*						m_pCycleData;
};
#endif

//! C_CrowdCycleGroup
class C_CrowdCycleGroup
{
public:

#ifndef PLATFORM_SPU
	C_CrowdCycleGroup();
	~C_CrowdCycleGroup();

	void	Clear();

	void	AddCycle(const Util::C_String& filename, const F32 weight = 1.f, const CrowdAction = E_CROWD_ACTION_IDLE);
#endif // !PLATFORM_SPU

	const CrowdCycleData&	GetCycleData(const S32 index) const;
	const S32				GetNumCycles() const;
	const F32				GetTotalWeight(const CrowdAction action) const;
	const bool				HasCyclesForAction(const CrowdAction action) const;

private:
	Btl::Vector<C_CrowdCycle*, Btl::VectorParameters_Static<16> >	m_pCycles;
	Btl::Vector<CrowdCycleData, Btl::VectorParameters_Static<16> >	m_CycleDataList;

	Btl::BitArray32			m_ValidActions;
};

}; // namespace AnimatedCrowd

#endif // CROWD_CYCLE_H
