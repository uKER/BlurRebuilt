#ifndef STAGGERMANAGER_H
#define STAGGERMANAGER_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Btl/Containers/vector.h>


namespace Util
{

/// Available frame skips for staggered updates.
enum E_StaggerLevel
{
	E_StaggerLevel_1Frame = 0,	///< every frame
	E_StaggerLevel_2Frame,		///< every 2 frames
	E_StaggerLevel_4Frame,		///< every 4 frames
	E_StaggerLevel_8Frame,		///< every 8 frames
	E_StaggerLevel_16Frame,		///< every 16 frames
	E_StaggerLevel_32Frame,		///< every 32 frames

	E_StaggerLevel_Count,
};


/**
@ingroup actor_group_utility
@brief Manages bitmasks for staggered updates.

Used when we want to update items every N frames, and we want to stagger these
updates to distribute evenly across those frames.

Each item should be added to a group on the desired level using the 
AllocGroup() method. This group identifier can then be used to retrieve a 
bitmask that will set bits for every frame that should be processed.
@code
// construct the bitmask for the current frame...
const U32 bitShift = m_CurrentFrameNumber & 31U; // take the lo 5 bits of the current frame number
const U32 frameMask = 1U << bitShift; // mask a single bit using this value

// now update the objects
for (U32 i = 0; i < m_NumItems; ++i)
{
    const U32 groupMask = m_StaggerManager.GetGroupMask(m_Items[i]->m_StaggerGroup);
    if ((groupMask & frameMask) != 0)
	{
		F32 const timeScale = m_StaggerManager.GetLevelFrameScale(m_Items[i]->m_StaggerLevel);
		m_Items[i]->Update(timeStep * timeScale);
	}
}
@endcode
*/
class C_StaggerManager
{
public:
	C_StaggerManager();
	// auto: ~C_StaggerManager();

	void			Reset();

	inline U32		AllocGroup(const U32 level);
	U32				AllocGroup(const E_StaggerLevel level);
	inline void		FreeGroup(const S32 group);

	inline U32		GetGroupMask(const S32 group) const;
	inline U32		GetGroupAlloc(const S32 group) const;
	inline F32		GetLevelFrameScale(const U32 level) const;

private:
	enum Constants
	{ 
		E_NoStaggerGroups = (1 + 2 + 4 + 8 + 16 + 32), ///< total number of groups needed for all stagger levels
	};
	
	Btl::Vector< U32, Btl::VectorParameters_Array<E_NoStaggerGroups> >		m_GroupAlloc;		///< number of items in each group
	Btl::Vector< U32, Btl::VectorParameters_Array<E_NoStaggerGroups> >		m_GroupMask;		///< bitmask for each group
	Btl::Vector< U32, Btl::VectorParameters_Array<E_StaggerLevel_Count> >	m_LevelStartIndex;	///< first group in each level

	static const F32 s_StaggerLevelFrameScale[];	///< time scale for each stagger level
	static const U32 s_StaggerLevelBaseMask[];		///< base bitmask for each stagger group

	C_StaggerManager(const C_StaggerManager&);				// can not copy
	C_StaggerManager& operator = (const C_StaggerManager&);	// can not copy

};

} // namespace Util


#define STAGGERMANAGER_INL
#include "StaggerManager.inl"
#undef STAGGERMANAGER_INL

#endif
