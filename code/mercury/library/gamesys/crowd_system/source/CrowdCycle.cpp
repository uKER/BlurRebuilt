#include "CrowdSystemPch.h"

#include <crowd_system/CrowdCycle.h>

#ifndef PLATFORM_SPU
#include <crowd_system/CrowdDebug.h>

#include <animation/cycle/Cycle.h>
#endif // !PLATFORM_SPU

namespace AnimatedCrowd
{

#ifndef PLATFORM_SPU
C_CrowdCycle::C_CrowdCycle()
	: m_pCycleData(NULL)
{
}

C_CrowdCycle::~C_CrowdCycle()
{
}

void C_CrowdCycle::Create(const Util::C_String& filename, CrowdCycleData* pCycleData)
{
	m_pCycleData = pCycleData;

	MEMORY_TRACKER_PUSH("mercury.crowd");
	m_CycleResourceHandle.Load(filename, this);
	MEMORY_TRACKER_POP("mercury.crowd");
}

void C_CrowdCycle::ResourceNotify(Resource::C_ResourceBase* pResource, const Resource::E_Notify result, void* pData)
{
	switch(result)
	{
	case Resource::E_Notify_DataReady:
		{
			if (m_pCycleData)
				m_pCycleData->m_Valid = true;

			if (GetCycleResource())
			{
				if(GetCycleResource()->GetCycle())
				{
					if (GetCycleResource()->GetCycle()->GetDuration() < 0.01f)
					{
						LOGINFO( g_CrowdLog, "Cycle '%s' has a zero length duration and won't be used.", pResource->GetResourceFilename().Chars() );
						
						if (m_pCycleData)
							m_pCycleData->m_Valid = false;
					}
					else
					{
						if (m_pCycleData)
						{
							m_pCycleData->m_pCycle = GetCycleResource()->GetCycle();
							m_pCycleData->m_Duration = m_pCycleData->m_pCycle->GetDuration();
						}
					}
				}
			}
		}
		break;
		
	default:
		break;
	}
}

const Animation::C_CycleResource* C_CrowdCycle::GetCycleResource() const
{
	if (!m_pCycleData || !m_pCycleData->m_Valid)
		return NULL;

	return static_cast<const Animation::C_CycleResource*>(m_CycleResourceHandle);
}
#endif // !PLATFORM_SPU

///////////////////////////////////////////////////////////////////////////////////////

#ifndef PLATFORM_SPU
C_CrowdCycleGroup::C_CrowdCycleGroup()
{
	m_ValidActions.ClearBits();
}

C_CrowdCycleGroup::~C_CrowdCycleGroup()
{
	Clear();	
}

void C_CrowdCycleGroup::Clear()
{
	for (U32 i = 0, count = static_cast<U32>(m_pCycles.Size()); i < count; ++i)
	{
		if (m_pCycles[i])
			delete m_pCycles[i];
		m_pCycles[i] = NULL;

		BIZ_ASSERT(i < static_cast<U32>(m_CycleDataList.Size()));
		m_CycleDataList[i].Clear();
	}

	m_pCycles.Destroy();
	m_CycleDataList.Destroy();
}

void C_CrowdCycleGroup::AddCycle(const Util::C_String& filename, const F32 weight, const CrowdAction action)
{
	CrowdCycleData cycleData;
	cycleData.Clear();
	cycleData.m_Weight = weight;
	cycleData.m_Action = static_cast<U32>(action);

	m_CycleDataList.PushBack(cycleData);

	C_CrowdCycle* pCycle = new C_CrowdCycle();
	pCycle->Create(filename, &(m_CycleDataList.Back()));

	m_pCycles.PushBack(pCycle);

	BIZ_ASSERT(static_cast<U32>(action) < 32);
	m_ValidActions.SetBit(static_cast<U32>(action));
}
#endif // !PLATFORM_SPU

const CrowdCycleData& C_CrowdCycleGroup::GetCycleData(const S32 index) const
{
	BIZ_ASSERT(index >= 0 && index < m_CycleDataList.Size());
	return m_CycleDataList[index];
}

const S32 C_CrowdCycleGroup::GetNumCycles() const
{
	return m_CycleDataList.Size();
}

const F32 C_CrowdCycleGroup::GetTotalWeight(const CrowdAction action) const
{
	F32 totalWeight = 0.f;

	for (S32 i = 0, count = GetNumCycles(); i < count; ++i)
	{
		if (m_CycleDataList[i].m_Action == static_cast<U32>(action))
			totalWeight += m_CycleDataList[i].m_Weight;
	}

	return totalWeight;
}

const bool C_CrowdCycleGroup::HasCyclesForAction(const CrowdAction action) const
{
	return m_ValidActions.GetBitValue(static_cast<U32>(action));
}

}; // namespace AnimatedCrowd
