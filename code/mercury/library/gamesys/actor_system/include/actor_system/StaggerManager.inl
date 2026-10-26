#ifndef STAGGERMANAGER_INL
#error StaggerManager.inl should only be included from StaggerManager.h
#endif


namespace Util
{

/**
@brief Assigns a group from this level.
@param level E_StaggerLevel to assign from.
@returns group identifier, should be relinquished with FreeGroup().
*/
inline U32 C_StaggerManager::AllocGroup(const U32 level)
{
	return AllocGroup(static_cast<E_StaggerLevel>(level));
}

/**
@param group group identifier returned by AllocGroup().
*/
inline void C_StaggerManager::FreeGroup(const S32 group)
{
	if (group != -1)
	{
		BIZ_ASSERT(m_GroupAlloc[group] > 0);
		m_GroupAlloc[group]--;
	}
}

/**
@param group group identifier assigned by AllocGroup().
@returns bitmask to use for items in this group.
*/
inline U32 C_StaggerManager::GetGroupMask(const S32 group) const 
{
	return m_GroupMask[group];
}

/**
@param group group identifier assigned by AllocGroup().
@returns number of items in this group.
*/
inline U32 C_StaggerManager::GetGroupAlloc(const S32 group) const 
{
	return m_GroupAlloc[group];
}

/**
@brief If staggering time-based events, provides a scaling factor for time
steps.
@param level E_StaggerLevel value.
@returns scaling factor for time step.
*/
inline F32 C_StaggerManager::GetLevelFrameScale(const U32 level) const 
{
	BIZ_ASSERT(level < E_StaggerLevel_Count);
	return s_StaggerLevelFrameScale[level]; // TODO: [Oscar] might be better to just return static_cast<float>(1 << level) ? itof isn't lightning quick but could avoid a cache hit
}

} // namespace Util
