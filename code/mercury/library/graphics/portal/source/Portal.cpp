#include <stdafx.h>
#include <Portal/Portal.h>
#include <Portal/Room.h>
#include <Portal/System.h>
#include <Core/containers/StringHelpers.h>

using namespace Portal;


// Default c'tor.
C_Portal::C_Portal() :
	m_iPortalIndex(C_Portal::InvalidPortal),
	m_iRoomIndex(C_Room::InvalidRoom)
{
	return;
}

// -----------------------------------------------------------------

// D'tor.
C_Portal::~C_Portal()
{
	// Placeholder method.
	return;
}

// -----------------------------------------------------------------

// Debug render the portal.
void C_Portal::DebugRender(DbUtil::I_DebugRenderPtr pDebugRender, const C_System& rkSystem, const bool bBlocked, const U32 iPortalIndex)
{
	const Maths::RGBA	red(1.0f, 0.0f, 0.0f, 1.0f);
	const Maths::RGBA	blue(0.0f, 0.0f, 1.0f, 1.0f);
	const Maths::RGBA	green(0.0f, 1.0f, 0.0f, 1.0f);
	static const Debug::Float debugOffsetValue("Graphics.Portal.Debug.ShiftDebugPortalRenderAlongNormal", 0.0f, Debug::Range(0.0f, 1.0f));

	if(!C_System::GetRenderSolidPortals())
	{
		// Draw wireframe of the portal polygon.
		Maths::C_Vertex portalCenter(0.0f, 0.0f, 0.0f);
		Maths::C_Vector offset(m_Plane.Normal().m_V);
		offset *= debugOffsetValue.Value();
		Maths::C_Vertex prevPos = m_vVertexPos.At(m_vVertexPos.Size() - 1) + offset;
		for(U32 iCurrentVertex=0; iCurrentVertex<static_cast<U32>(m_vVertexPos.Size()); ++iCurrentVertex)
		{
			Maths::C_Vertex currPos = m_vVertexPos.At(iCurrentVertex) + offset;
		
			if(!bBlocked)
			{
				if(rkSystem.GetPortalAtIndex(m_iPortalIndex).GetParentRoomIndex() == 0)
					pDebugRender->Draw3dLine(prevPos, currPos, red);
				else
					pDebugRender->Draw3dLine(prevPos, currPos, green);
			}
			else
				pDebugRender->Draw3dLine(prevPos, currPos, blue);
			
			Maths::C_Vector v(currPos);
			portalCenter += v;
			prevPos = currPos;
		}

		// Render normal.
		portalCenter /= static_cast<F32>(m_vVertexPos.Size());
		pDebugRender->Draw3dLine(portalCenter + offset, portalCenter + m_Plane.Normal() + offset, blue);
		pDebugRender->Draw3dText(portalCenter, Util::unsignedIntegerToCString(iPortalIndex));
	}
	else
	{
		U32 iIndex0 = 0;
		U32 iIndex1 = 0;
		U32 iIndex2 = 0;

		for(U32 iCurrentVertex=1; iCurrentVertex<static_cast<U32>(m_vVertexPos.Size())-1; ++iCurrentVertex)
		{
			iIndex1 = iCurrentVertex;
			iIndex2 = iCurrentVertex+1;
			Maths::C_Vertex currPos1 = m_vVertexPos.At(iIndex0);
			Maths::C_Vertex currPos2 = m_vVertexPos.At(iIndex1);
			Maths::C_Vertex currPos3 = m_vVertexPos.At(iIndex2);
			
			if(!bBlocked)
			{
				if(rkSystem.GetPortalAtIndex(m_iPortalIndex).GetParentRoomIndex() == 0)
					pDebugRender->Draw3dTriangle(currPos1, currPos2, currPos3, Maths::RGBA(1.0f, 0.0f, 0.0f, 0.3f));
				else
					pDebugRender->Draw3dTriangle(currPos1, currPos2, currPos3, Maths::RGBA(0.0f, 1.0f, 0.0f, 0.3f));
			}
			else
				pDebugRender->Draw3dTriangle(currPos1, currPos2, currPos3, Maths::RGBA(0.0f, 0.0f, 1.0f, 0.3f));
		}
	}
	return;
}

// -----------------------------------------------------------------

// Return the counterpart portal.
const U32 C_Portal::GetCounterpartPortal() const
{
	return m_iPortalIndex;
}

// -----------------------------------------------------------------

// Return a pointer to room that contains the portal.
const U32 C_Portal::GetParentRoomIndex() const
{
	return m_iRoomIndex;
}

// -----------------------------------------------------------------

// Return the support plane.
const Maths::C_Plane& C_Portal::GetPlane() const
{
	return m_Plane;
}

// -----------------------------------------------------------------

// Compute the screen-space area of the portal's polygon.
const Maths::HF32 C_Portal::GetScreenSpaceArea(const Maths::HMat44& rkViewProj) const
{
	// Stack allocate one vert for each element in view polygon:
	Maths::HVec2* pProjectedVerts = NULL;
	pProjectedVerts = (Maths::HVec2*)alloca((m_vVertexPos.Size() + 2) * sizeof(Maths::HVec2));
	BIZ_WARNING_MSG(pProjectedVerts, "Unable to allocate stack space for the projected verts.");

	// Project all the verts into NDC space:
	for(U32 iCurrentVert=0; iCurrentVert<static_cast<U32>(m_vVertexPos.Size()); ++iCurrentVert)
	{
		Maths::HVec4 projectedVert = Maths::HVec4(m_vVertexPos[iCurrentVert]) * rkViewProj;
		projectedVert /= projectedVert.W();
		pProjectedVerts[iCurrentVert] = Maths::HVec2(projectedVert.X(), projectedVert.Y());
	}							
	pProjectedVerts[m_vVertexPos.Size()]	= pProjectedVerts[0];
	pProjectedVerts[m_vVertexPos.Size()+1] = pProjectedVerts[1];	// Replication for the area calcs.

	// Calculate area for our projected 2D vertices:
	Maths::HF32 fArea = 0.0f;
	for (U32 i=1, j=2, k=0; i<=static_cast<U32>(m_vVertexPos.Size()); ++i, ++j, ++k) 
		fArea = fArea + pProjectedVerts[i].X() * (pProjectedVerts[j].Y() - pProjectedVerts[k].Y());

	return fabs(fArea * 0.5f);
}

// -----------------------------------------------------------------

// Return a vertex of portal polygon at specified index.
const Maths::C_Vertex& C_Portal::GetVertexAtIndex(U32 iIndex) const
{
	BIZ_WARNING_MSG(iIndex < static_cast<U32>(m_vVertexPos.Size()), "Index out of range.");
	return m_vVertexPos[iIndex];
}

// -----------------------------------------------------------------

// Return the number of vertices in the portal polygon.
const U32 C_Portal::GetVertexCount() const
{
	return static_cast<U32>(m_vVertexPos.Size());
}

// -----------------------------------------------------------------

// Return pointer to vertex data.
const Maths::C_Vertex* C_Portal::GetVertexPtr() const
{
	return &m_vVertexPos[0];
}
