#include <stdafx.h>
#include <Portal/RoomContainmentSpace.h>
#include <Portal/Room.h>
#include <Portal/ViewPolygon.h>
#include <Portal/System.h>

using namespace Portal;


// Build the debug mesh.
void C_RoomContainmentSpace::BuildDebugMesh()
{
	// TODO: [Stove] Implement this.
	/*
	// Loop through each space that defines room containment.
	for(U32 iCurrentSubSpace=0; iCurrentSubSpace<static_cast<U32>(m_vSubSpaces.Size()); ++iCurrentSubSpace)
	{
		m_vDebugMeshes.PushBack(RegionBSP::C_Mesh());
		
		for(U32 iCurrentPlane=m_vSubSpaces[iCurrentSubSpace].GetStartIndex(); iCurrentPlane<m_vSubSpaces[iCurrentSubSpace].GetEndIndex()+1; ++iCurrentPlane)
		{
			Maths::C_Plane currentPlane = -m_vPlanes[iCurrentPlane];
			m_vDebugMeshes[iCurrentSubSpace].AddPlane(currentPlane);
		}
		m_vDebugMeshes[iCurrentSubSpace].Build();
	}*/

	return;
}

// -----------------------------------------------------------------

// Return true if space contains specified world-space point.
NOALIAS const bool C_RoomContainmentSpace::Contains(const Maths::HVec3& rkPoint, C_System* pSystem) const __restrict
{
	assert(pSystem);

	for(U32 iCurrentSubSpace=m_iStartSubSpaceIndex; iCurrentSubSpace<m_iEndSubSpaceIndex+1; ++iCurrentSubSpace)
	{
		bool bInSubSpace = true;

		const C_RoomContainmentSubSpace* pCurrentSpace = &pSystem->GetSubSpaceAtIndex(iCurrentSubSpace);
		for(U32 iCurrentPlane = pCurrentSpace->GetStartIndex(); iCurrentPlane<pCurrentSpace->GetEndIndex()+1; ++iCurrentPlane)
		{
			const Maths::C_Plane& currentPlane = pSystem->GetPlaneAtIndex(iCurrentPlane);
			if(currentPlane.Distance(rkPoint) > 0.0f)
			{		
				bInSubSpace = false;
				break;
			}
		}

		if(bInSubSpace)
			return true;
	}

	return false;
}	

// -----------------------------------------------------------------

// Debug render the space.
void C_RoomContainmentSpace::DebugRender(DbUtil::I_DebugRenderPtr pDebugRender, const Maths::HMat43 &rkCameraMat, const Maths::RGBA& rkColour)
{
	for(U32 iCurrentDebugMesh=0; iCurrentDebugMesh<static_cast<U32>(m_vDebugMeshes.Size()); ++iCurrentDebugMesh)
		m_vDebugMeshes[iCurrentDebugMesh].DebugRender(pDebugRender, rkColour);

	return;
}
