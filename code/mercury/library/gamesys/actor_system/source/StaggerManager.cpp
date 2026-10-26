#include "ActorSystemPch.h"

#include <actor_system/StaggerManager.h>

#include <limits>


namespace StaggerManagerImpl
{

/// Pow2Series<N>::Sum gives the sum for (i=0,N) of 2^i
template<U32 RadixBase2> struct Pow2Series { enum{ Sum = (1U << RadixBase2) + Pow2Series<RadixBase2 - 1>::Sum }; };
template<> struct Pow2Series<0> { enum{ Sum = 1 }; };

} // namespace StaggerManagerImpl


namespace Util
{

// Stagger Manager
// ---------------
const F32 C_StaggerManager::s_StaggerLevelFrameScale[] = 
{
	1.0f,	// E_StaggerLevel_1Frame
	2.0f,	// E_StaggerLevel_2Frame
	4.0f,	// E_StaggerLevel_4Frame
	8.0f,	// E_StaggerLevel_8Frame
	16.0f,	// E_StaggerLevel_16Frame
	32.0f,	// E_StaggerLevel_32Frame
};

const U32 C_StaggerManager::s_StaggerLevelBaseMask[] =
{
	0xffffffff,	// E_StaggerLevel_1Frame 
	0x55555555,	// E_StaggerLevel_2Frame,
	0x11111111,	// E_StaggerLevel_4Frame,
	0x01010101,	// E_StaggerLevel_8Frame,
	0x00010001,	// E_StaggerLevel_16Frame
	0x00000001,	// E_StaggerLevel_32Frame
};

C_StaggerManager::C_StaggerManager()
{
	// if any of these compile-time asserts fail then check if E_StaggerLevel_Count has been changed without also changing one of these dependent values
	StaticAssert(static_cast<U32>(StaggerManagerImpl::Pow2Series<E_StaggerLevel_Count - 1>::Sum) == static_cast<U32>(E_NoStaggerGroups), E_NoStaggerGroups_must_match_E_StaggerLevel);
	StaticAssert(E_StaggerLevel_Count == lengthof(s_StaggerLevelFrameScale), s_StaggerLevelFrameScale_must_match_E_StaggerLevel);
	StaticAssert(E_StaggerLevel_Count == lengthof(s_StaggerLevelBaseMask), s_StaggerLevelBaseMask_must_match_E_StaggerLevel);

	U32 noInLevel = 1;
	U32 groupNo = 0;

	// initialise levels
	for(S32 levelNo = 0; levelNo < E_StaggerLevel_Count; levelNo++)
	{
		U32 groupMask = s_StaggerLevelBaseMask[levelNo];

		m_LevelStartIndex[levelNo] = groupNo;	// set level start index

		// iterate through levels
		for(U32 levelGroupCount = 0; levelGroupCount < noInLevel; levelGroupCount++)
		{
			//U32 groupMaskHighBit = (groupMask & (1<<31)) ? 1:0;
			U32 groupMaskHighBit = (groupMask >>31) & 1;

			m_GroupMask[groupNo] = groupMask;	// set group mask
			m_GroupAlloc[groupNo] = 0;			// init group alloc

			// update for next group
			groupMask = (groupMask << 1) | groupMaskHighBit;
			groupNo++;
		}

		noInLevel = noInLevel << 1;
	}
}

/**
@brief Reset all stagger groups.
*/
void C_StaggerManager::Reset(void)
{
	for (U32 i = 0; i < E_NoStaggerGroups; ++i)
		m_GroupAlloc[i] = 0;
}

/**
@brief Allocate a stagger group for a given level.
@param level level to assign from.
@returns group identifier, should be relinquished with FreeGroup().
*/
U32 C_StaggerManager::AllocGroup(const E_StaggerLevel level)
{
	U32 levelStartIndex = m_LevelStartIndex[level];
	U32 levelEndIndex = levelStartIndex + (1U << level);
	U32 minVal = std::numeric_limits<U32>::min();
	U32 minDex = levelStartIndex;

	// maybe this loop can be unrolled? (there is only one case when there is 1 iteration)
	for	(U32 i = levelStartIndex; i < levelEndIndex; ++i)
	{
		S32 diff = static_cast<S32>(minVal) - static_cast<S32>(m_GroupAlloc[i]);

		minDex = Maths::Select_GE_Zero(diff, i, minDex);
		minVal = Maths::Select_GE_Zero(diff, m_GroupAlloc[i], minVal);

		// convert these to use integer select
		//bool smaller = (m_GroupAlloc[i] < minVal);
		//minDex = Maths::Select(smaller,i,minDex);
		//minVal = Maths::Select(smaller,m_GroupAlloc[i],minVal);
	}

	m_GroupAlloc[minDex]++;
	return minDex;
}

} // namespace Util
