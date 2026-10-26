#include <stdafx.h>
#include <Portal/RoomGrid.h>
#include <Core/Maths/HVec/Geometry.h>

using namespace Portal;


// Default c'tor for room grid.
C_RoomGrid::C_RoomGrid() :
	m_iNumberOfCellsX(10),
	m_iNumberOfCellsZ(10),
	m_fCellSize(0.0f)
{
	return;
}

// -----------------------------------------------------------------

/// D'tor.
C_RoomGrid::~C_RoomGrid()
{
	// Placeholder method.
	return;
}

// -----------------------------------------------------------------

// Calculate room cell index from world-space position.
NOALIAS const U32 C_RoomGrid::CalculateCellIndexFromPosition(const Maths::HVec3& rkPosition) const __restrict
{
	if(!m_Extents.Inside(rkPosition))
		return C_RoomGrid::InvalidCell;
	
	// TODO: [Stove] Might be good to move to VMX to avoid LHS on PPU.
	F32	fDX = m_Extents.Max().X() - m_Extents.Min().X();
	F32 fDZ = m_Extents.Max().Z() - m_Extents.Min().Z();
	U32 iX = static_cast<U32>(floorf(((rkPosition.X() - m_Extents.Min().X())/fDX) * (m_iNumberOfCellsX) ));
	U32 iZ = static_cast<U32>(floorf(((rkPosition.Z() - m_Extents.Min().Z())/fDZ) * (m_iNumberOfCellsZ) ));
	if(iZ >= m_iNumberOfCellsZ || iX >= m_iNumberOfCellsX)
		return C_RoomGrid::InvalidCell;

	return iX + iZ * m_iNumberOfCellsX;
}

// -----------------------------------------------------------------

// Debug render the room grid.
void C_RoomGrid::DebugRender(DbUtil::I_DebugRenderPtr pDebugRender, const Maths::HMat43& rkCameraMat)
{
	BIZ_WARNING_MSG(pDebugRender, "Debug renderer NULL.");
	if(m_Extents.Volume() == 0.0f)
		return;

	Btl::Vector<Maths::HVec3>	vStartPoints;
	vStartPoints.Reserve(m_iNumberOfCellsX + m_iNumberOfCellsZ);
	Btl::Vector<Maths::HVec3>	vEndPoints;
	vEndPoints.Reserve(m_iNumberOfCellsX + m_iNumberOfCellsZ);
	
	U32 iX=0;
	for(; iX<m_iNumberOfCellsX; ++iX)
	{
		F32 fGap = (m_Extents.Max().X() - m_Extents.Min().X())/(F32)m_iNumberOfCellsX;
		vStartPoints.PushBack(Maths::HVec3(m_Extents.Min().X() + (F32)iX * fGap, 0.0f, m_Extents.Min().Z()));
		vEndPoints.PushBack(Maths::HVec3(m_Extents.Min().X() + (F32)iX * fGap, 0.0f, m_Extents.Max().Z()));
	}

	for(U32 iZ=m_iNumberOfCellsX; iZ<m_iNumberOfCellsX+m_iNumberOfCellsZ; ++iZ)
	{
		F32 fGap = (m_Extents.Max().Z() - m_Extents.Min().Z())/(F32)m_iNumberOfCellsZ;
		vStartPoints.PushBack(Maths::HVec3(m_Extents.Min().X(), 0.0f, m_Extents.Min().Z() + (F32)(iZ-iX) * fGap));
		vEndPoints.PushBack(Maths::HVec3(m_Extents.Max().X(), 0.0f, m_Extents.Min().Z() + (F32)(iZ-iX) * fGap));
	}

	Maths::RGBA colour(1.0f, 1.0f, 1.0f, 1.0f);
 	pDebugRender->Draw3dLines(&vStartPoints[0], &vEndPoints[0], m_iNumberOfCellsX + m_iNumberOfCellsZ, colour);
	return;
}

// -----------------------------------------------------------------

// Return the cell at a specified index.
const C_RoomGridCell* C_RoomGrid::GetCellAtIndex(U32 iIndex) const
{
	BIZ_WARNING_MSG(iIndex < static_cast<U32>(m_vCells.Size()), "Index out of range.");	// Index out of range.
	return &m_vCells[iIndex];
}

// -----------------------------------------------------------------

