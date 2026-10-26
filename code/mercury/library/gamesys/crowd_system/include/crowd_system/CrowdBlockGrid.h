#ifndef CROWD_BLOCK_GRID_H
#define CROWD_BLOCK_GRID_H

#ifdef _MSC_VER
#pragma once
#endif

#include <btl/containers/Vector.h>

namespace GameResource
{
	class C_CrowdData;
}

namespace AnimatedCrowd
{

//! C_CrowdBlockGridCell
class C_CrowdBlockGridCell
{
public:
	C_CrowdBlockGridCell()
	{
	}

	~C_CrowdBlockGridCell()
	{
		m_BlockIndexes.Destroy();
	}

	const S32 GetNumBlockIndexes() const
	{
		return m_BlockIndexes.Size();
	}

	const U16 GetBlockIndex(const S32 i) const
	{
		BIZ_ASSERT(i > -1 && i < m_BlockIndexes.Size());
		return m_BlockIndexes[i];
	}

#ifdef CFG_CONVERTER
	void AddBlockIndex(const U16 blockIndex)
	{
		m_BlockIndexes.PushBack(blockIndex);
	}
#endif

#ifdef CFG_CONVERTER
	template< typename Archive > 
	void Serialise( Archive& ar )
	{
		ar << (S32)m_BlockIndexes.Size();

		if (m_BlockIndexes.Size() > 0)
		{
			for (S32 i = 0; i < m_BlockIndexes.Size(); ++i)
			{
				ar << m_BlockIndexes[i];
			}
		}
	}
#endif

#ifndef CFG_CONVERTER
	template< typename Archive > 
	void Deserialise( Archive& ar )
	{
		S32 numIndexes = 0;

		ar >> numIndexes;

		if (numIndexes > 0)
		{
			m_BlockIndexes.Resize(numIndexes);

			for (U16 i = 0, count = numIndexes; i < count; ++i)
				ar >> m_BlockIndexes[i];
		}
	}
#endif

private:
	Btl::Vector<U16>	m_BlockIndexes;

	friend class GameResource::C_CrowdData;
};

//! C_CrowdBlockGrid

class C_CrowdBlockGrid
{
public:

	C_CrowdBlockGrid()
		: m_GridDimensions(0),
		m_GridMin(0.f),
		m_GridMax(0.f)
	{
#ifdef CFG_CONVERTER
		m_GridCellSize = 0.f;
#endif
	}

	~C_CrowdBlockGrid()
	{
		m_BlockGridCells.Destroy();
	}

	const C_CrowdBlockGridCell*	GetGridCell(const Maths::HVec3& cameraPosition, S32& cellX, S32& cellY) const
	{
		const Maths::HVec2 camPosXZ(cameraPosition.X(), cameraPosition.Z());

		if (camPosXZ.X() >= m_GridMin.X() && camPosXZ.Y() >= m_GridMin.Y() && camPosXZ.X() < m_GridMax.X() && camPosXZ.Y() < m_GridMax.Y())
		{
			BIZ_ASSERT(m_GridMin.X() != m_GridMax.X());
			BIZ_ASSERT(m_GridMin.Y() != m_GridMax.Y());

			Maths::HVec2 localPosXZ = ((camPosXZ - m_GridMin) / (m_GridMax - m_GridMin)) * static_cast<F32>(m_GridDimensions);
			
			cellX = static_cast<S32>(Maths::Floor(localPosXZ.X()));
			cellY = static_cast<S32>(Maths::Floor(localPosXZ.Y()));

			BIZ_ASSERT(cellX < m_GridDimensions && cellY < m_GridDimensions);

			S32 cellIndex = cellY * m_GridDimensions + cellX;

			return &m_BlockGridCells[cellIndex];
		}

		return NULL;
	}

	const C_CrowdBlockGridCell* GetGridCell(const S32 cellX, const S32 cellY) const
	{
		if (cellX > -1 && cellX < m_GridDimensions && cellY > -1 && cellY < m_GridDimensions)
		{
			return &m_BlockGridCells[cellY * m_GridDimensions + cellX];
		}

		return NULL;
	}

	const C_CrowdBlockGridCell* GetGridCell(const S32 cellIndex) const
	{
		if (cellIndex < m_BlockGridCells.Size())
			return &m_BlockGridCells[cellIndex];
		return NULL;
	}

	void GetGridCellLocation(const Maths::HVec3& cameraPos, S32& cellX, S32& cellY) const
	{
		const Maths::HVec2 camPosXZ(cameraPos.X(), cameraPos.Z());

		if (camPosXZ.X() >= m_GridMin.X() && camPosXZ.Y() >= m_GridMin.Y() && camPosXZ.X() < m_GridMax.X() && camPosXZ.Y() < m_GridMax.Y())
		{
			BIZ_ASSERT(m_GridMin.X() != m_GridMax.X());
			BIZ_ASSERT(m_GridMin.Y() != m_GridMax.Y());

			Maths::HVec2 localPosXZ = ((camPosXZ - m_GridMin) / (m_GridMax - m_GridMin)) * static_cast<F32>(m_GridDimensions);
			
			cellX = static_cast<S32>(Maths::Floor(localPosXZ.X()));
			cellY = static_cast<S32>(Maths::Floor(localPosXZ.Y()));
		}
	}

	const Maths::HVec2& GetGridMin() const
	{
		return m_GridMin;
	}

	const Maths::HVec2& GetGridMax() const
	{
		return m_GridMax;
	}

	const S16 GetGridDimensions() const
	{
		return m_GridDimensions;
	}

	const S32 GetNumGridCells() const
	{
		return m_BlockGridCells.Size();
	}

#ifdef CFG_CONVERTER
	static bool SegmentCircleIntersect(const Maths::HVec2& centre, const F32 radius, const Maths::HVec2& p1, const Maths::HVec2& p2)
	{
		Maths::HVec2 dir = p2 - p1;
		Maths::HVec2 diff = centre - p1;
	    
		F32 t = (diff | dir) / (dir | dir);
	    
		if (t < 0.0f)
			t = 0.0f;
		if (t > 1.0f)
			t = 1.0f;
	    
		Maths::HVec2 closest = p1 + (dir * t);
		Maths::HVec2 d = centre - closest;
	    
		F32 distSqr = (d | d);

		return distSqr <= radius * radius;
	}

	static bool PointInRect(const Maths::HVec2& point, const Maths::HVec2& min, const Maths::HVec2& max)
	{
		if (point.X() >= min.X() && point.Y() >= min.Y() && point.X() <= max.X() && point.Y() <= max.Y())
			return true;
		return false;
	}

	static bool PointInCircle(const Maths::HVec2& circleCentre, const F32 circleRadius, const Maths::HVec2& point)
	{
		const Maths::HVec2 dir = point - circleCentre;
		const F32 distSqr = (dir | dir);
		
		return distSqr <= (circleRadius * circleRadius);
	}

	static bool CircleCircleIntersect(const Maths::HVec2& centre1, const F32 radius1, const Maths::HVec2& centre2, const F32 radius2)
	{
		const Maths::HVec2 dir = centre2 - centre1;
		const F32 distSqr = (dir | dir);
		const F32 radiusSumSqr = (radius1 * radius1) + (radius2 * radius2);

		return distSqr <= radiusSumSqr;
	}

	static bool RectCircleIntersect(const Maths::HVec2& circleCentre, const F32 circleRadius,
							 const Maths::HVec2& rectMin, const Maths::HVec2& rectMax)
	{
		if (PointInRect(circleCentre, rectMin, rectMax))
			return true;

		if (PointInCircle(circleCentre, circleRadius, rectMin))
			return true;

		if (PointInCircle(circleCentre, circleRadius, Maths::HVec2(rectMin.X(), rectMax.Y())))
			return true;

		if (PointInCircle(circleCentre, circleRadius, rectMax))
			return true;

		if (PointInCircle(circleCentre, circleRadius, Maths::HVec2(rectMax.X(), rectMin.Y())))
			return true;

		const Maths::HVec2 boxHalfExtents = (rectMax - rectMin) * 0.5f;
		F32 boxRadius = Maths::Max<F32>(boxHalfExtents.X(), boxHalfExtents.Y()) * 1.4142136f;
		
		if (CircleCircleIntersect(circleCentre, circleRadius, ((rectMin + rectMax) * 0.5f), boxRadius))
		{
			if (SegmentCircleIntersect(circleCentre, circleRadius, rectMin, Maths::HVec2(rectMin.X(), rectMax.Y())))
				return true;

			if (SegmentCircleIntersect(circleCentre, circleRadius, Maths::HVec2(rectMin.X(), rectMax.Y()), rectMax))
				return true;

			if (SegmentCircleIntersect(circleCentre, circleRadius, rectMax, Maths::HVec2(rectMax.X(), rectMin.Y())))
				return true;

			if (SegmentCircleIntersect(circleCentre, circleRadius, Maths::HVec2(rectMax.X(), rectMin.Y()), rectMin))
				return true;
		}

		return false;
	}

	void Build(const Btl::Vector<C_CrowdBlockRef>* pBlocks, const Aftk::C_Bounds& sceneBounds,
		const F32 gridCellSize, const F32 gridCellRange)
	{
		m_BlockGridCells.Clear();

		const Maths::HVec2 worldMin(sceneBounds.GetMin().X(), sceneBounds.GetMin().Z());
		const Maths::HVec2 worldMax(sceneBounds.GetMax().X(), sceneBounds.GetMax().Z());

		m_GridCellSize = gridCellSize;

		Maths::HVec2 dimensions = worldMax - worldMin;
		Maths::HVec2 halfDimensions = dimensions * 0.5f;
		//m_Centre = (worldMax + worldMin) * 0.5f;

		Maths::HVec2 centre = (worldMax + worldMin) * 0.5f;

		F32 halfGridCellSize = Maths::Abs<F32>(m_GridCellSize) * 0.5f;

		//F32 radius = sqrtf((halfGridCellSize * halfGridCellSize) + (halfGridCellSize * halfGridCellSize));
		F32 radius = halfGridCellSize * sqrtf(2.f);
		radius += Maths::Abs<F32>(gridCellRange);

		m_GridDimensions = static_cast<S16>(Maths::Ceil(dimensions.X() / m_GridCellSize));
		m_GridDimensions = Maths::Max<S16>(m_GridDimensions, static_cast<S32>(Maths::Ceil(dimensions.Y() / m_GridCellSize)));

		m_GridMin = centre - Maths::HVec2(m_GridDimensions * halfGridCellSize);
		m_GridMax = centre + Maths::HVec2(m_GridDimensions * halfGridCellSize);

		m_BlockGridCells.Resize(m_GridDimensions * m_GridDimensions);

		const Maths::HVec2 startOffset = m_GridMin + Maths::HVec2(halfGridCellSize);

		for (S32 iBlock = 0; iBlock < pBlocks->Size(); ++iBlock)
		{
			const Maths::HVec2 blockMin((*pBlocks)[iBlock].GetAxisBox().Min().X(), (*pBlocks)[iBlock].GetAxisBox().Min().Z());
			const Maths::HVec2 blockMax((*pBlocks)[iBlock].GetAxisBox().Max().X(), (*pBlocks)[iBlock].GetAxisBox().Max().Z());

			for (S16 j = 0; j < m_GridDimensions; ++j)
			{
				for (S16 i = 0; i < m_GridDimensions; ++i)
				{
					const Maths::HVec2 cellCentre = startOffset + Maths::HVec2(i * m_GridCellSize, j * m_GridCellSize);

					if (RectCircleIntersect(cellCentre, radius, blockMin, blockMax))
						m_BlockGridCells[j * m_GridDimensions + i].AddBlockIndex(iBlock);
				}
			}
		}
	}
#endif

#ifdef CFG_CONVERTER
	template< typename Archive > 
	void Serialise( Archive& ar )
	{
		ar << m_GridDimensions;

		ar << m_GridMin;
		ar << m_GridMax;

		ar << (S32)m_BlockGridCells.Size();

		if (m_BlockGridCells.Size() > 0)
		{
			for (S32 i = 0; i < m_BlockGridCells.Size(); ++i)
			{
				ar << m_BlockGridCells[i];
			}
		}
	}
#endif

#ifndef CFG_CONVERTER
	template< typename Archive > 
	void Deserialise( Archive& ar )
	{
		ar >> m_GridDimensions;

		ar >> m_GridMin;
		ar >> m_GridMax;

		S32 numCells = 0;

		ar >> numCells;

		if (numCells > 0)
		{
			m_BlockGridCells.Resize(numCells);

			for (U32 i = 0, count = numCells; i < count; ++i)
				ar >> m_BlockGridCells[i];
		}
	}
#endif

private:
	Maths::HVec2	m_GridMin;
	Maths::HVec2	m_GridMax;

	Btl::Vector<C_CrowdBlockGridCell>	m_BlockGridCells;

#ifdef CFG_CONVERTER
	F32				m_GridCellSize;
#endif

	S16				m_GridDimensions;

	friend class GameResource::C_CrowdData;
};

}; // namespace AnimatedCrowd

#endif // CROWD_BLOCK_GRID_H
