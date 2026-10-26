#ifndef CROWD_EFFECT_ITEM_H
#define CROWD_EFFECT_ITEM_H

#ifdef _MSC_VER
#pragma once
#endif

namespace AnimatedCrowd
{
	class C_CrowdAnimation;
};

namespace AnimatedCrowd
{

//! EffectSpawnItem
struct ALIGN16 EffectSpawnItem
{
public:
	U32							EventIndex;
	U32							AnimPool	: 2;	// 0 = Invalid, 1 = SharedPool, 2 = UniquePool
	U32							AnimIndex	: 30;
	U32							JointIndex;
	Maths::HMat43				Transform;
	F32							StartTime;
};

//! EffectSpawnData
struct ALIGN16 EffectSpawnData
{
public:
	void AddItem(const EffectSpawnItem& item)
	{
		BIZ_ASSERT(m_NumItems < CROWD_EFFECT_SPAWN_BUFFER_SIZE);

		m_Items[m_NumItems++] = item;
	}

	void RemoveItem(const U32 index)
	{
		BIZ_ASSERT(m_NumItems > 0);
		BIZ_ASSERT(index < m_NumItems);

		m_Items[index] = m_Items[m_NumItems - 1];
		m_NumItems--;
	}

	EffectSpawnItem	m_Items[CROWD_EFFECT_SPAWN_BUFFER_SIZE];
	U32				m_NumItems;
};

}; // namespace AnimatedCrowd

#endif // CROWD_EFFECT_ITEM_H
