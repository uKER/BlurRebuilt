#include <stdafx.h>
#include <Portal/ViewPolygon.h>
#include <Portal/Portal.h>

using namespace Portal;

// Default c'tor.
C_ViewPolygon::C_ViewPolygon()
{
	return;
}

// -----------------------------------------------------------------

// D'tor.
C_ViewPolygon::~C_ViewPolygon()
{
	Reset();
	return;
}

// -----------------------------------------------------------------

// Add a new vertex to the polygon.
void C_ViewPolygon::AddVertex(const Maths::HVec3& rkPos)
{
	BIZ_WARNING_MSG(m_vVertexData.Size() < PORTAL_CONFIG_MAXPOLYVERTS, "Too many polygons in view polygon.");	// Don't try to push more than the maximum into the container.
	if(m_vVertexData.Size() >= PORTAL_CONFIG_MAXPOLYVERTS)
		return;

	m_vVertexData.PushBack(rkPos);
	return;
}

// -----------------------------------------------------------------

// Clip the polygon to the specified plane.
void C_ViewPolygon::Clip(C_ViewPolygon& rkResult, const Maths::C_Plane& rkPlane) const
{
	rkResult.Reset();	// Setup the result poly to recieve clipped results.
	BIZ_WARNING_MSG(m_vVertexData.Size() >= 3, "Not enough verts to clip view polygon.");
    if(m_vVertexData.Size() < 3)
        return;
    
    Maths::HVec3	prevPos		= m_vVertexData[m_vVertexData.Size()-1];
    Maths::HF32		fPrevDist	= rkPlane.Distance(prevPos);
	
	for(U32 iCurrentVertex=0; iCurrentVertex<static_cast<U32>(m_vVertexData.Size()); ++iCurrentVertex)
    {        
        Maths::HVec3	currPos		= m_vVertexData[iCurrentVertex];
        Maths::HF32		fCurrDist	= rkPlane.Distance(currPos);

        const Maths::HF32 fZero(0.0f);
    
        if((fPrevDist < fZero) && (fCurrDist < fZero))
	    {
			;	// Doesn't output anything.
	    }
	    else if((fPrevDist >= fZero) && (fCurrDist < fZero))
	    {
	        // Output I.
            Maths::HF32 fT = (fPrevDist - fZero) / (fPrevDist - fCurrDist);
            rkResult.AddVertex(prevPos + (currPos - prevPos) * fT);
	    }
	    else if((fPrevDist < fZero) && (fCurrDist >= fZero))
	    {
	        // Ouput I, Vertex
            Maths::HF32 fT = (fPrevDist - fZero) / (fPrevDist - fCurrDist);
            rkResult.AddVertex(prevPos + (currPos - prevPos) * fT);
            rkResult.AddVertex(currPos);
	    }
	    else if((fPrevDist >= fZero) && (fCurrDist >= fZero))
	    {
            // Output vertex
            rkResult.AddVertex(currPos);
	    }
                
        prevPos = currPos;
        fPrevDist = fCurrDist;
    }        
	return;
}

// -----------------------------------------------------------------

// Clip the view polygon to a specified portal.
void C_ViewPolygon::Clip(C_ViewPolygon& rkResult, const C_Portal* pPortal, const Maths::HVec3& rkCameraPos) const
{
	BIZ_WARNING_MSG(pPortal, "NULL portal pointer.");				// Invalid pointer to portal to clip against.
	BIZ_WARNING_MSG(GetVertexCount(), "No vertices in polygon.");	// No vertices in the model.
	if(!pPortal || !GetVertexCount())
		return;

	C_ViewPolygon result = *this;

	Maths::HF32	fBiggestDistToCamera = 0.0f;
	Maths::HVec3 prevPos = pPortal->GetVertexAtIndex(pPortal->GetVertexCount() - 1);
	for(U32 iCurrentVertex=0; iCurrentVertex<pPortal->GetVertexCount(); ++iCurrentVertex)
	{
		const Maths::HVec3 currPos = pPortal->GetVertexAtIndex(iCurrentVertex);                
        
		Maths::HF32 fVertexToCameraDist = currPos.Distance(rkCameraPos);
		if(fVertexToCameraDist > fBiggestDistToCamera)
			fBiggestDistToCamera = fVertexToCameraDist;

		Maths::C_Plane clipPlane(prevPos, rkCameraPos, currPos);                        
        
		C_ViewPolygon temp = result;
		temp.Clip(result, clipPlane);
        
		if(result.GetVertexCount() < 3)
			break;
        
		prevPos = currPos;            
	}

	rkResult = result;
	return;
}

// -----------------------------------------------------------------

// Debug render the polygon.
void C_ViewPolygon::DebugRender(DbUtil::I_DebugRenderPtr pDebugRender, const Maths::RGBA& rkColour, bool bSolidPoly) const
{
	BIZ_WARNING_MSG(pDebugRender, "NULL debug renderer pointer.");	// Needs a valid debug render pointer.
	if(!pDebugRender)
		return;

	// Draw each line in the polygon.
	if(!bSolidPoly)
	{	
		for(U32 iCurrent=0; iCurrent<static_cast<U32>(m_vVertexData.Size()); ++iCurrent)
		{
			pDebugRender->SetDepthEnable(false);
			pDebugRender->Draw3dLine(m_vVertexData[iCurrent], 
									 (iCurrent == m_vVertexData.Size()-1)?m_vVertexData[0]:m_vVertexData[iCurrent+1],	// Loop around to start vert. 
									 rkColour);
		}
	}
	else
	{
		pDebugRender->SetBlendMode(DbUtil::E_BlendMode_Alpha);
		U32 iIndex1 = 0;
		U32 iIndex2 = 0;
		U32 iIndex3 = 0;

		for(U32 iCurrent=1; iCurrent<static_cast<U32>(m_vVertexData.Size()-1); ++iCurrent)
		{
			iIndex2 = iCurrent;
			iIndex3 = iCurrent+1;
			pDebugRender->Draw3dTriangleLit(m_vVertexData[iIndex1], m_vVertexData[iIndex2], m_vVertexData[iIndex3], rkColour);
		}
	}

	return;
}

// -----------------------------------------------------------------

// Return the number of vertices in the polygon.
const U32 C_ViewPolygon::GetVertexCount() const
{
	return static_cast<U32>(m_vVertexData.Size());
}

// -----------------------------------------------------------------

// Remove all vertices from the polygon.
void C_ViewPolygon::Reset()
{
	m_vVertexData.Clear();
	return;
}

// -----------------------------------------------------------------

// Assignment of one polygon to another.
void C_ViewPolygon::operator= (const C_ViewPolygon& rkCopy)
{
	m_vVertexData = rkCopy.m_vVertexData;
	return;
}

// -----------------------------------------------------------------

// Compute the screen-space area of the polygon.
const Maths::HF32 C_ViewPolygon::GetScreenSpaceArea(const Maths::HMat44& rkViewProj) const
{
	// Stack allocate one vert for each element in view polygon:
	Maths::HVec2* pProjectedVerts = NULL;
	pProjectedVerts = (Maths::HVec2*)alloca((m_vVertexData.Size() + 2) * sizeof(Maths::HVec2));
	BIZ_WARNING_MSG(pProjectedVerts, "Unable to allocate stack space for the projected verts.");
	
	// Project all the verts into NDC space:
	for(U32 iCurrentVert=0; iCurrentVert<static_cast<U32>(m_vVertexData.Size()); ++iCurrentVert)
	{
		Maths::HVec4 projectedVert = Maths::HVec4(m_vVertexData[iCurrentVert]) * rkViewProj;
		projectedVert /= projectedVert.W();
		pProjectedVerts[iCurrentVert] = Maths::HVec2(projectedVert.X(), projectedVert.Y());
	}							
	pProjectedVerts[m_vVertexData.Size()]	= pProjectedVerts[0];
	pProjectedVerts[m_vVertexData.Size()+1] = pProjectedVerts[1];	// Replication for the area calcs.

	// Calculate area for our projected 2D vertices:
	Maths::HF32 fArea = 0.0f;
	for (U32 i=1, j=2, k=0; i<=static_cast<U32>(m_vVertexData.Size()); ++i, ++j, ++k) 
		fArea = fArea + pProjectedVerts[i].X() * (pProjectedVerts[j].Y() - pProjectedVerts[k].Y());
	
	return fabs(fArea * 0.5f);
}
