#include <stdafx.h>
#include <Portal/BuilderRoomContainmentSpace.h>
#include <Btl/Containers/algorithm.h>

using namespace Portal;


// Default c'tor.
C_BuilderRoomContainmentSpace::C_BuilderRoomContainmentSpace(C_BuilderRoom* pParent) :
	m_pParentRoom(pParent)
{
	return;
}

// -----------------------------------------------------------------

// D'tor.
C_BuilderRoomContainmentSpace::~C_BuilderRoomContainmentSpace()
{
	return;
}

// -----------------------------------------------------------------

// Return true if room contians point.
const bool C_BuilderRoomContainmentSpace::Contains(const Maths::HVec3 &rkPoint) const
{
	// Check all sub-spaces for one that contains the point.
	Btl::List<C_BuilderRoomContainmentSubSpace>::ConstIterator subSpaceIterator = m_vSubSpaces.Begin();
	for(; subSpaceIterator != m_vSubSpaces.End(); ++subSpaceIterator)
	{
		const C_BuilderRoomContainmentSubSpace* pCurrentSubSpace = NULL;
		pCurrentSubSpace = &(*subSpaceIterator);
		BIZ_WARNING_MSG(pCurrentSubSpace, "Current space pointer is NULL.");
		bool bInSubSpace = true;
	
		// Check the range of planes in the current sub-space.
		for(U32 iCurrentPlane=pCurrentSubSpace->GetStartIndex(); iCurrentPlane<=pCurrentSubSpace->GetEndIndex(); ++iCurrentPlane)
		{
			Btl::List<Maths::C_Plane>::ConstIterator planeIterator = m_vPlanes.Begin();
			Btl::Advance(planeIterator, iCurrentPlane);
			for(U32 iVal=iCurrentPlane; iVal<=pCurrentSubSpace->GetEndIndex(); ++iVal, ++planeIterator);
			{
				if((*planeIterator).Distance(rkPoint) > 0.0f)
				{
					bInSubSpace = false;
					break;	// Break out of sub-space loop.
				}
			}

			if(!bInSubSpace)
				break;
		}
	
		if(bInSubSpace)
			return true;
	}

	// If we got this far we're not inside any of the subspaces.
	return false;
}

// -----------------------------------------------------------------

// Add a plane to the space.
void C_BuilderRoomContainmentSpace::CreatePlane(const Maths::C_Plane& rkPlane)
{
	m_vPlanes.PushBack(rkPlane);
	return;
}

// -----------------------------------------------------------------

// Add a sub-space to the space.
void C_BuilderRoomContainmentSpace::CreateSubSpace(U32 iStartIndex, U32 iEndIndex)
{
	m_vSubSpaces.PushBack(C_BuilderRoomContainmentSubSpace(iStartIndex, iEndIndex));
	return;
}

// -----------------------------------------------------------------

// Return pointer to the room this space defines.
const C_BuilderRoom* C_BuilderRoomContainmentSpace::GetParentRoom() const
{
	return m_pParentRoom;
}

// -----------------------------------------------------------------

// Return the number of planes in the space.
const U32 C_BuilderRoomContainmentSpace::GetPlaneCount() const
{
	return static_cast<U32>(m_vPlanes.Size());
}

// -----------------------------------------------------------------

// Return the plane list.
const Btl::List<Maths::C_Plane>& C_BuilderRoomContainmentSpace::GetPlaneList() const
{
	return m_vPlanes;
}

// -----------------------------------------------------------------

// Return number of sub-spaces.
const U32 C_BuilderRoomContainmentSpace::GetSubSpaceCount() const
{
	return static_cast<U32>(m_vSubSpaces.Size());
}

// -----------------------------------------------------------------

// Return list of sub-spaces.
const Btl::List<C_BuilderRoomContainmentSubSpace>& C_BuilderRoomContainmentSpace::GetSubSpaceList() const
{
	return m_vSubSpaces;
}

// -----------------------------------------------------------------

// Return true if the axis box intersects the room.
const bool C_BuilderRoomContainmentSpace::Intersects(const Maths::C_AxisBox& rkBox) const
{
	Maths::C_AxisBox::C_VertexArray	vBoxPoints;
	rkBox.GetVertexArray(vBoxPoints);
	
	bool bPointInBox = false;
	for(U32 iCurrentVertex=0; iCurrentVertex<8; ++iCurrentVertex)
	{
		if(Contains(vBoxPoints[iCurrentVertex]))
		{
			// Bail out if possible.
			bPointInBox = true;
			break;
		}
	}

	return bPointInBox;
}

