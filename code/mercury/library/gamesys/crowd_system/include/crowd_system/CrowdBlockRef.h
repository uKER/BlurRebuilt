#ifndef CROWD_BLOCK_REF_H
#define CROWD_BLOCK_REF_H

#ifdef _MSC_VER
#pragma once
#endif

#include <crowd_system/CrowdDefines.h>

namespace AnimatedCrowd
{

//! C_CrowdBlockRef

class C_CrowdBlockRef
{
public:

#ifdef CFG_CONVERTER
	struct BlockItem
	{
		U32		InstanceIndex;
		F32		InstanceDistanceSqr;
	};

	enum InstanceInsertResult
	{
		E_RESULT_ADDED_NO_REMOVE	= 0,
		E_RESULT_ADDED_REMOVE,
		E_RESULT_NOT_ADDED
	};
#endif

	C_CrowdBlockRef()
		: m_Route(-1),
		m_NumInstances(0),
#ifdef CFG_CONVERTER
		m_AxisBox(Maths::HVec3(0.f), Maths::HVec3(0.f)),
#endif
		m_BlockType(E_BLOCK_TYPE_LOW_DETAIL)
	{
#ifdef CFG_CONVERTER
		m_pCharacters = NULL;
		m_pInstances = NULL;

		memset(m_InstanceIndexes, 0, sizeof(BlockItem) * CROWD_MAX_INSTANCES_PER_BLOCK);
#else
		m_InstanceIndexes = NULL;
#endif
	}

	~C_CrowdBlockRef()
	{
#ifndef CFG_CONVERTER
		delete [] m_InstanceIndexes;
#endif
	}

#ifdef CFG_CONVERTER
	void UpdateBounds()
	{
		if (m_pCharacters && m_pInstances)
		{
			if (m_NumInstances)
			{
				{
					Maths::C_AxisBox axisBox = (*m_pCharacters)[(*m_pInstances)[m_InstanceIndexes[0].InstanceIndex].GetCharacterIndex()].GetAxisBox();
					
					m_AxisBox.Min(axisBox.Min() + (*m_pInstances)[m_InstanceIndexes[0].InstanceIndex].GetPosition());
					m_AxisBox.Max(axisBox.Max() + (*m_pInstances)[m_InstanceIndexes[0].InstanceIndex].GetPosition());
				}

				for (U8 i = 1; i < m_NumInstances; ++i)
				{
					Maths::C_AxisBox axisBox = (*m_pCharacters)[(*m_pInstances)[m_InstanceIndexes[i].InstanceIndex].GetCharacterIndex()].GetAxisBox();

					axisBox.Min(axisBox.Min() + (*m_pInstances)[m_InstanceIndexes[i].InstanceIndex].GetPosition());
					axisBox.Max(axisBox.Max() + (*m_pInstances)[m_InstanceIndexes[i].InstanceIndex].GetPosition());

					m_AxisBox.UpdateExtents(axisBox);
				}
			}
		}
	}

	const bool CheckBounds(const Maths::C_AxisBox& axisBox) const
	{
		Maths::C_AxisBox testBox;
		testBox.Min(m_AxisBox.Min());
		testBox.Max(m_AxisBox.Max());

		testBox.UpdateExtents(axisBox);

		Maths::HVec3 diagonal = testBox.Diagonal() * 0.5f;

		if (Maths::LengthSquared(diagonal) <= CROWD_MAX_BLOCK_RADIUS_SQR)
			return true;
		return false;
	}

	void Create(const U32 id, const S32 route, const U32 instanceIndex, const Btl::Vector<C_CrowdCharacterRef>* pCharacterList,
		const Btl::Vector<C_CrowdInstance>* pInstanceList)
	{
		m_ID = id;
		m_Route = route;

		m_pCharacters = pCharacterList;
		m_pInstances = pInstanceList;

		memset(m_InstanceIndexes, 0, sizeof(BlockItem) * CROWD_MAX_INSTANCES_PER_BLOCK);

		BlockItem blockItem;
		blockItem.InstanceIndex = instanceIndex;
		blockItem.InstanceDistanceSqr = 0.f;

		m_InstanceIndexes[m_NumInstances++] = blockItem;

		UpdateBounds();
	}

	const InstanceInsertResult AddInstanceIndex(const U32 index, const F32 distanceSqr, U32 &weakerInstance)
	{
		if (m_NumInstances < CROWD_MAX_INSTANCES_PER_BLOCK)
		{
			BlockItem blockItem;
			blockItem.InstanceIndex = index;
			blockItem.InstanceDistanceSqr = distanceSqr;

			m_InstanceIndexes[m_NumInstances++] = blockItem;

			UpdateBounds();

			return E_RESULT_ADDED_NO_REMOVE;
		}
		else
		{
			U32 weakestBlock = 0;
			F32 weakestDistanceSqr = m_InstanceIndexes[weakestBlock].InstanceDistanceSqr;

			for (U32 i = 1; i < CROWD_MAX_INSTANCES_PER_BLOCK; ++i)
			{
				if (m_InstanceIndexes[i].InstanceDistanceSqr > weakestDistanceSqr)
				{
					weakestBlock = i;
					weakestDistanceSqr = m_InstanceIndexes[weakestBlock].InstanceDistanceSqr;
				}
			}

			if (weakestDistanceSqr > distanceSqr)
			{
				weakerInstance = m_InstanceIndexes[weakestBlock].InstanceIndex;

				BlockItem blockItem;
				blockItem.InstanceIndex = index;
				blockItem.InstanceDistanceSqr = distanceSqr;

				m_InstanceIndexes[weakestBlock] = blockItem;

				UpdateBounds();

				return E_RESULT_ADDED_REMOVE;
			}
			else
			{
				return E_RESULT_NOT_ADDED;
			}
		}
		
		return E_RESULT_NOT_ADDED;
	}

	void SetBlockType(const CrowdBlockRefType blockType)
	{
		m_BlockType = blockType;
	}

	/*void SetNearestSamplePoint(const Maths::HVec3& p)
	{
		m_NearestSamplePoint = p;
	}*/

	const U32 GetInstance(const U32 index) const
	{
		BIZ_ASSERT(index < m_NumInstances);

		return m_InstanceIndexes[index].InstanceIndex;
	}
#endif

	const U32 GetID() const
	{
		return m_ID;
	}

	const S32 GetRoute() const
	{
		return m_Route;
	}

	const U8 GetNumInstances() const
	{
		return m_NumInstances;
	}

#ifndef CFG_CONVERTER
	const U32 GetInstanceIndex(const U32 i) const
	{
		BIZ_ASSERT(i < m_NumInstances);

		return m_InstanceIndexes[i];
	}
#endif

#ifdef CFG_CONVERTER
	const Maths::C_AxisBox& GetAxisBox() const
	{
		return m_AxisBox;
	}
#else
	Maths::HVec3 GetMin() const
	{
		return Maths::Load<Maths::HVec3, MathsStorage::Float3>(&m_Min);
	}

	Maths::HVec3 GetMax() const
	{
		return Maths::Load<Maths::HVec3, MathsStorage::Float3>(&m_Max);
	}
#endif

#ifdef CFG_CONVERTER
	const Maths::HVec3 GetPosition() const
	{
		return m_AxisBox.Centre();
	}

	const Maths::HVec3 GetExtents() const
	{
		return ((m_AxisBox.Max() - m_AxisBox.Min()) * 0.5f);
	}
#else
	const Maths::HVec3 GetPosition() const
	{
		return (Maths::Load<Maths::HVec3, MathsStorage::Float3>(&m_Min) + Maths::Load<Maths::HVec3, MathsStorage::Float3>(&m_Max)) * 0.5f;
	}

	const Maths::HVec3 GetExtents() const
	{
		return (Maths::Load<Maths::HVec3, MathsStorage::Float3>(&m_Max) - Maths::Load<Maths::HVec3, MathsStorage::Float3>(&m_Min)) * 0.5f;
	}
#endif

	const CrowdBlockRefType GetBlockType() const
	{
		return static_cast<AnimatedCrowd::CrowdBlockRefType>(m_BlockType);
	}

	/*const Maths::HVec3& GetNearestSamplePoint() const
	{
		return m_NearestSamplePoint;
	}*/

#ifdef CFG_CONVERTER
	static CrowdBlockRefType GetBlockType(const F32 distanceSqr)
	{
		if (distanceSqr <= CROWD_BLOCK_HIGHDETAIL_DISTSQR)
			return E_BLOCK_TYPE_HIGH_DETAIL;
		return E_BLOCK_TYPE_LOW_DETAIL;
	}
#endif

#ifdef CFG_CONVERTER
	template< typename Archive > 
	void Serialise( Archive& ar )
	{
		ar << m_ID;

		ar << m_Route;

		ar << m_NumInstances;

		if (m_NumInstances > 0)
		{
			for (U32 i = 0; i < m_NumInstances; ++i)
			{
				ar << m_InstanceIndexes[i].InstanceIndex;
			}
		}

		ar << m_AxisBox;

		ar << static_cast<S32>(m_BlockType);

		//ar << m_NearestSamplePoint;
	}
#endif

#ifndef CFG_CONVERTER
	template< typename Archive > 
	void Deserialise( Archive& ar )
	{
		ar >> m_ID;

		if (g_CrowdFileVersion > 0)
		{
			S32 route;
			ar >> route;
			m_Route = route;
		}

		ar >> m_NumInstances;

		if (m_NumInstances > 0)
		{
			m_InstanceIndexes = new U32[m_NumInstances];

			for (U32 i = 0, count = m_NumInstances; i < count; ++i)
				ar >> m_InstanceIndexes[i];
		}

		//ar >> m_AxisBox;
		Maths::C_AxisBox axisBox;
		ar >> axisBox;

		Maths::Store(axisBox.Min(), &m_Min);
		Maths::Store(axisBox.Max(), &m_Max);

		S32 blockType = 0;

		if (g_CrowdFileVersion >= 3)
			ar >> blockType;

		m_BlockType = static_cast<CrowdBlockRefType>(blockType);
		m_BlockType = E_BLOCK_TYPE_HIGH_DETAIL;

		//ar >> m_NearestSamplePoint;
	}
#endif

private:

#ifdef CFG_CONVERTER
	const Btl::Vector<C_CrowdCharacterRef>*	m_pCharacters;
	const Btl::Vector<C_CrowdInstance>*		m_pInstances;
#endif

#ifdef CFG_CONVERTER
	Maths::C_AxisBox	m_AxisBox;
#else
	MathsStorage::Float3	m_Min;
	MathsStorage::Float3	m_Max;
#endif

	U32					m_ID;

#ifdef CFG_CONVERTER
	S32					m_Route;
#else
	S16					m_Route;
#endif

	U8					m_NumInstances;

#ifdef CFG_CONVERTER
	CrowdBlockRefType	m_BlockType;
#else
	U8					m_BlockType;
#endif

#ifdef CFG_CONVERTER
	BlockItem			m_InstanceIndexes[CROWD_MAX_INSTANCES_PER_BLOCK];
#else
	U32*				m_InstanceIndexes;
#endif

	// TEMP
	//Maths::HVec3		m_NearestSamplePoint;
};

inline bool operator < (const AnimatedCrowd::C_CrowdBlockRef& blockA, const AnimatedCrowd::C_CrowdBlockRef& blockB)
{
	return blockA.GetRoute() < blockB.GetRoute();
}

}; // namespace AnimatedCrowd

#endif // CROWD_BLOCK_REF_H
