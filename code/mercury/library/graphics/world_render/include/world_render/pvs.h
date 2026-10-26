#ifndef WORLD_RENDER_PVS
#define WORLD_RENDER_PVS

#ifdef _MSC_VER
#pragma once
#endif

#include "core/maths/utmaths/maths.h"
#include "btl/containers/bitvector.h"
#include "misc_shared/archive/archive.h"
#include "misc_shared/archive/btl.h"

#ifndef CFG_CONVERTER
extern Debug::Bool g_DebugPVSLoadSamplePoints;
#endif

namespace Pvs
{

typedef UtMaths::VectorXYZ<float> Vector3Base;
typedef UtMaths::VectorXYZW<float> Vector4Base;

typedef UtMaths::MPoint<Vector3Base> Point3;
typedef UtMaths::MVector<Vector3Base> Vector3;
typedef UtMaths::MNormal<Vector3Base> Normal3;
typedef UtMaths::MVector<Vector4Base> Vector4;

class PvsData
{
public:
	FRIEND_SERIALISE(PvsData);

	const MathsStorage::Matrix44& GetCellMatrix(S32 i) const;

	const U32				GetNumSamplePoints(const U32 cell) const;

	const bool				IsSamplePointDirectional(const U32 cell, const U32 samplePoint) const;
	const bool				IsSamplePointOptional(const U32 cell, const U32 samplePoint) const;

	const Maths::HVec3		GetSamplePointPosition(const U32 cell, const U32 samplePoint) const;

protected:
	Btl::Vector<MathsStorage::Matrix44> m_inverseMatrices;

	struct SamplePoint
	{
		Point3 position;
		Normal3 direction;
		float gridSize;
		bool directional;
		bool optional;
	};

	struct Cell
	{
		MathsStorage::Matrix44 matrix;
		Btl::Vector<SamplePoint> samplePoints;
	};

	FRIEND_SERIALISE(SamplePoint);
	FRIEND_SERIALISE(Cell);

	// Cell data as used in PVS calculation (debug only in game).
	Btl::Vector<Cell> m_cells;
};

///////////////////////////////////////////////////////

template <typename Archive>
inline void Serialise(Archive& ar, PvsData& t)
{
	ar & Arch::MakeNvp("InverseMatrices", t.m_inverseMatrices);
	ar & Arch::MakeNvp("Cells", t.m_cells);
}

AUTO_DESERIALISE_FREE(PvsData)

template <typename Archive>
inline void Serialise(Archive& ar, PvsData::SamplePoint& t)
{
	ar & Arch::MakeNvp("Position", t.position);
	ar & Arch::MakeNvp("Direction", t.direction);
	ar & Arch::MakeNvp("Directional", t.directional);
	ar & Arch::MakeNvp("Optional", t.optional);
}

AUTO_DESERIALISE_FREE(PvsData::SamplePoint)

template <typename Archive>
inline void Serialise(Archive& ar, PvsData::Cell& t)
{
	ar & Arch::MakeNvp("Matrix", t.matrix);
	ar & Arch::MakeNvp("SamplePoints", t.samplePoints);
}

template <typename Archive>
inline void Deserialise(Archive& ar, PvsData::Cell& t)
{
	ar & Arch::MakeNvp("Matrix", t.matrix);

#ifndef CFG_CONVERTER
	if(!g_DebugPVSLoadSamplePoints)
	{
		// Skip loading the sample points.
		Btl::Vector<PvsData::SamplePoint> samplePoints;
		ar & Arch::MakeNvp("SamplePoints", samplePoints);
	}
	else
#endif
	{
		ar & Arch::MakeNvp("SamplePoints", t.samplePoints);	
	}
}


inline const MathsStorage::Matrix44& PvsData::GetCellMatrix(S32 i) const
{
	return m_cells[i].matrix;
}

inline const U32 PvsData::GetNumSamplePoints(const U32 cell) const
{
	if (cell < static_cast<U32>(m_cells.Size()))
		return static_cast<U32>(m_cells[cell].samplePoints.Size());
	return 0;
}

inline const bool PvsData::IsSamplePointDirectional(const U32 cell, const U32 samplePoint) const
{
	BIZ_ASSERT(cell < static_cast<U32>(m_cells.Size()) && samplePoint < static_cast<U32>(m_cells[cell].samplePoints.Size()));
	return m_cells[cell].samplePoints[samplePoint].directional;
}

inline const bool PvsData::IsSamplePointOptional(const U32 cell, const U32 samplePoint) const
{
	BIZ_ASSERT(cell < static_cast<U32>(m_cells.Size()) && samplePoint < static_cast<U32>(m_cells[cell].samplePoints.Size()));
	return m_cells[cell].samplePoints[samplePoint].optional;
}

inline const Maths::HVec3 PvsData::GetSamplePointPosition(const U32 cell, const U32 samplePoint) const
{
	if (cell < static_cast<U32>(m_cells.Size()) && samplePoint < static_cast<U32>(m_cells[cell].samplePoints.Size()))
	{
		return Maths::HVec3(
			m_cells[cell].samplePoints[samplePoint].position.X(),
			m_cells[cell].samplePoints[samplePoint].position.Y(),
			m_cells[cell].samplePoints[samplePoint].position.Z()
		);
	}
	
	return Maths::HVec3(0.f, 0.f, 0.f);
}

///////////////////////////////////////////////////////

namespace Target
{

class PvsData : public Pvs::PvsData
{
public:
	S32 GetCell(const Maths::HVec3& camera) const;

	// debug
	static void SetDebugBits(const Btl::BitVector& bits);
	void DebugRender(const Maths::HVec3& camera) const;

private:
	static Btl::BitVector s_debugBits;
};

template <typename Archive>
inline void Serialise(Archive& ar, PvsData& t)
{
	SERIALISE_BASE_FREE(Pvs::PvsData, ar, t);
}

AUTO_DESERIALISE_FREE(PvsData)

}

}

#endif
