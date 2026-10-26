#ifndef CROWD_VISIBLE_BLOCK_DATA_H
#define CROWD_VISIBLE_BLOCK_DATA_H

#ifdef _MSC_VER
#pragma once
#endif

namespace AnimatedCrowd
{

//! C_CrowdPotentiallyVisibleBlock
struct ALIGN16 C_CrowdPotentiallyVisibleBlock
{
public:
	C_CrowdPotentiallyVisibleBlock()
		: m_BlockIndex(0xffffffff),
		m_RouteBlockIndex(0xffffffff)
	{ }

	C_CrowdPotentiallyVisibleBlock(const U32 blockIndex, const U32 routeBlockIndex, const Maths::HVec3& vMin, const Maths::HVec3& vMax)
		: m_BlockIndex(blockIndex),
		m_RouteBlockIndex(routeBlockIndex)
	{
		Maths::Store(vMin, &m_Min);
		Maths::Store(vMax, &m_Max);
	}

	const U32				GetBlockIndex() const		{ return m_BlockIndex; }
	const U32				GetRouteBlockIndex() const	{ return m_RouteBlockIndex; }
	Maths::HVec3			GetMin() const { return Maths::Load<Maths::HVec3, MathsStorage::Float3>(&m_Min); }
	Maths::HVec3			GetMax() const { return Maths::Load<Maths::HVec3, MathsStorage::Float3>(&m_Max); }

private:
	MathsStorage::Float3	m_Min;
	MathsStorage::Float3	m_Max;
	U32						m_BlockIndex;
	U32						m_RouteBlockIndex;
};

//! C_CrowdVisibleBlock

struct C_CrowdVisibleBlock
{
public:
	C_CrowdVisibleBlock()
		: m_BlockRefIndex(0),
		m_Priority(CROWD_BLOCK_CACHE_BUCKET_COUNT),
		m_EventIndex(-1)
	{ }

	C_CrowdVisibleBlock(const U32 blockRefIndex, const U32 priority, const S32 eventIndex)
		: m_BlockRefIndex(blockRefIndex),
		m_Priority(priority),
		m_EventIndex(eventIndex)
	{ }

	const U32	GetBlockRefIndex() const	{ return m_BlockRefIndex; }

	const U32	GetPriority() const			{ return m_Priority; }

	void		SetEventIndex(const S32 eventIndex)	{ m_EventIndex = eventIndex; }
	const S32	GetEventIndex() const		{ return m_EventIndex; }

private:
	U32		m_BlockRefIndex	: 20;
	U32		m_Priority		: 6;
	S32		m_EventIndex	: 6;
};

//! C_CrowdVisibleBlockData

class ALIGN16 C_CrowdVisibleBlockData
{
public:
	C_CrowdVisibleBlockData()
		: m_NumVisibleBlocks(0)
	{ }

	void Clear()
	{
		m_NumVisibleBlocks = 0;
	}

	const C_CrowdVisibleBlock& GetVisibleBlock(const U32 index) const
	{
		BIZ_ASSERT(index < CROWD_MAX_VISIBLE_BLOCKS);
		return m_VisibleBlocks[index];
	}

	void AddVisibleBlock(const C_CrowdVisibleBlock& visibleBlock)
	{
		BIZ_ASSERT(m_NumVisibleBlocks < CROWD_MAX_VISIBLE_BLOCKS);

		if (m_NumVisibleBlocks < CROWD_MAX_VISIBLE_BLOCKS)
			m_VisibleBlocks[m_NumVisibleBlocks++] = visibleBlock;
	}

	const U32 GetNumVisibleBlocks() const
	{
		return m_NumVisibleBlocks;
	}

private:
	C_CrowdVisibleBlock		m_VisibleBlocks[CROWD_MAX_VISIBLE_BLOCKS];
	U32						m_NumVisibleBlocks;
};

struct CrowdDelimiter
{
	U32	m_Data			: 31;
	U32	m_BoundaryType	: 1;
		
	F32	m_Value;
};

inline U32 RadixKey(const CrowdDelimiter& delimiter)
{
	S32 s = ((S32&) delimiter.m_Value);
	if (s >> 31) s = (~s | (1 << 31)) + 1;
	return ((U32&) s) + ((U32)(1 << 31) - 1);
}

struct CrowdDelimiterSort
{
public:
	bool operator()(CrowdDelimiter lhs, CrowdDelimiter rhs)
	{
		return (lhs.m_Value < rhs.m_Value);
	}
};

}; // namespace AnimatedCrowd

#endif // CROWD_VISIBLE_BLOCK_DATA_H
