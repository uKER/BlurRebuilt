#ifndef REGIONBSP_INL
#define REGIONBSP_INL

#ifdef _MSC_VER
#pragma once
#endif

namespace RegionBSP
{

//inline void C_ClipPolygon::operator =( const C_ClipPolygon& other )
//{
//    m_Plane = other.m_Plane;
//	const S32 otherVertexCount = other.m_VertexList.Size();
//    m_VertexList.Resize(otherVertexCount);
//    
//	for(S32 i=0; i<otherVertexCount;i++)
//	{
//		m_VertexList[i] = other.m_VertexList[i];
//	}
//}
//
//inline void C_ClipPolygon::NewVertex( const Maths::HVec3& pos )
//{
//	m_VertexList.PushBack(pos);
//}
//
//inline void C_ClipPolygon::CalculatePlane()
//{
//	const S32 vertexCount = m_VertexList.Size();
//    if(vertexCount < 3)
//        return;
//
//    // calculate normal
//
//	// last vertex pos
//    Maths::HVec3 currPos = m_VertexList[vertexCount - 1];
//	// second to last vertex pos
//    Maths::HVec3 prevPos = m_VertexList[vertexCount - 2];
//
//    Maths::HVec3 currEdgeDir( (currPos-prevPos).Normalised().Normal() );
//        
//    // TODO [PaulM] : is this broken? the total magnitude is coming out as 0.5
//    // exterior angle should sum to 360            
//    Maths::HVec3 normalResult(0.0f);
//
//    Maths::HVec3 prevEdgeDir;
//    Maths::HVec3 crossProduct;
//    F32 crossLength;
//    F32 angle;
//
//	for(S32 vertexNo=0; vertexNo<vertexCount; vertexNo++)
//	{
//        prevEdgeDir = currEdgeDir;
//        prevPos = currPos;
//        
//        currPos = m_VertexList[vertexNo];
//
//        currEdgeDir = (currPos-prevPos).Normalised().Normal();            
//    
//        crossProduct = prevEdgeDir ^ currEdgeDir;        
//        crossLength = crossProduct.Length();        
//        if(crossLength > 0)
//        {
//            if(crossLength > 1.2f)
//            {
//				LOGWARN( g_Log, "Bad angle in polygon normal calculation\n");
//            }
//            
//            if(crossLength > 1.0f)
//            {
//                crossLength = 1.0f;
//            }
//            
//            angle = asinf(crossLength);
//            normalResult = normalResult + ((Maths::HVec3)crossProduct.Normalised().Normal() * angle);
//        }
//	}
//    
//    Maths::HVec3 firstPos = m_VertexList[0];
//    
//    m_Plane = Maths::C_Plane( Maths::C_Normal( -normalResult ), Maths::C_Vertex( firstPos ) );    
//}
//
//inline const Maths::C_Plane C_ClipPolygon::Plane() const
//{
//    return m_Plane;
//}
//
//inline void C_ClipPolygon::Clip( const Maths::C_Plane& plane, const F32 epsilon, C_ClipPolygon& front, C_ClipPolygon& back )
//{
//	const S32 vertexCount = m_VertexList.Size();
//
//	//front.m_VertexList.Clear();
//	//back.m_VertexList.Clear();
//
//	// get last vertex
//    Maths::HVec3 prevPos = m_VertexList[vertexCount - 1];
//    Maths::HF32 prevDist = plane.Distance(prevPos);
//
//	for(S32 vertexNo=0; vertexNo<vertexCount; vertexNo++)
//    {
//		Maths::HVec3 currPos = m_VertexList[vertexNo];
//        Maths::HF32 currDist = plane.Distance(currPos);
//        //Maths::HF32 currDist = (currPos | planeNormal) + distToOrigin;
//
//        const Maths::HF32 hfEpsilon( epsilon );
//        const Maths::HF32 hfMinusEpsilon( -epsilon );
//    
//        // FRONT
//	    if((prevDist < hfEpsilon) && (currDist < hfEpsilon))
//	    {
//	        // output nothing
//	    }
//	    else
//	    if((prevDist >= hfEpsilon) && (currDist < hfEpsilon))
//	    {
//	        // ouput I
//            const Maths::HF32 t = (prevDist - hfEpsilon) / (prevDist - currDist);
//            front.NewVertex( prevPos + (currPos - prevPos) * t );
//	    }
//	    else
//	    if((prevDist < hfEpsilon) && (currDist >= hfEpsilon))
//	    {
//	        // ouput I, vertex
//            const Maths::HF32 t = (prevDist - hfEpsilon) / (prevDist - currDist);
//            front.NewVertex( prevPos + (currPos - prevPos) * t );
//            front.NewVertex( currPos );
//	    }
//	    else
//	    if((prevDist >= hfEpsilon) && (currDist >= hfEpsilon))
//	    {
//            // output vertex
//            front.NewVertex( currPos );
//	    }
//
//        // BACK                        
//	    if((prevDist < hfMinusEpsilon) && (currDist < hfMinusEpsilon))
//	    {
//            // output vertex
//            back.NewVertex( currPos );
//	    }
//	    else
//	    if((prevDist >= hfMinusEpsilon) && (currDist < hfMinusEpsilon))
//	    {
//	        // ouput I, vertex
//            Maths::HF32 t = (prevDist - hfMinusEpsilon) / (prevDist - currDist);
//            back.NewVertex( prevPos + (currPos - prevPos) * t );
//            back.NewVertex( currPos );
//	    }
//	    else
//	    if((prevDist < hfMinusEpsilon) && (currDist >= hfMinusEpsilon))
//	    {
//	        // ouput I
//            Maths::HF32 t = (prevDist - hfMinusEpsilon) / (prevDist - currDist);
//            back.NewVertex( prevPos + (currPos - prevPos) * t );
//	    }
//	    else
//	    if((prevDist >= hfMinusEpsilon) && (currDist >= hfMinusEpsilon))
//	    {
//	        // output nothing
//	    }
//                
//        prevPos = currPos;
//        prevDist = currDist;
//    }    
//    
//    front.m_Plane = m_Plane;
//    back.m_Plane = m_Plane;
//}
//
//inline void C_ClipPolygon::DebugRender( DbUtil::I_DebugRenderPtr debugRenderPtr, const Maths::RGBA& colour )
//{
//	const S32 vertexCount = m_VertexList.Size();
//	Maths::HVec3 prevPos = m_VertexList[vertexCount-1];
//	for(S32 vertexNo=0; vertexNo<vertexCount; vertexNo++)
//	{
//		Maths::HVec3 currPos = m_VertexList[vertexNo];
//
//		debugRenderPtr->Draw3dLine(prevPos, currPos, colour);
//		prevPos = currPos;
//	}
//}


inline C_RegionId C_Region::GetRegionId()
{
	return m_RegionId;
}

inline C_SplitPlane* C_Region::GetSplitPlane()
{
	return m_pSplitPlane;
}

inline Maths::C_Plane C_SplitPlane::GetPlane()
{
	return m_Plane;
}

inline C_Region& C_SplitPlane::GetFront()
{
	return m_Front;
}

inline C_Region& C_SplitPlane::GetBack()
{
	return m_Back;
}

enum E_RegionBSPVersion
{
	E_Version_Initial = 0,
	E_Version_Current = E_Version_Initial
};

template <typename Archive>
inline void C_Tree::Serialise( Archive& ar )
{
	const U32 version = E_Version_Current;
	const S32 planeCount = m_SplitPlaneList.Size();

	ar << version;
	ar << planeCount;

	// write our root region info
	S32 splitIndex = GetSplitPlaneIndex(m_RootRegion.m_pSplitPlane);
	ar << m_RootRegion.m_RegionId;
	ar << splitIndex;

	// write out split plane info
	for(S32 planeNo = 0; planeNo<planeCount; planeNo++)
	{
		const C_SplitPlane& splitPlane = m_SplitPlaneList[planeNo];

		S32 frontIndex = GetSplitPlaneIndex(splitPlane.m_Front.m_pSplitPlane);
		S32 backIndex = GetSplitPlaneIndex(splitPlane.m_Back.m_pSplitPlane);

		ar << splitPlane.m_Plane;

		ar << splitPlane.m_Front.m_RegionId;
		ar << frontIndex;

		ar << splitPlane.m_Back.m_RegionId;
		ar << backIndex;
	}
}

template <typename Archive>
inline void C_Tree::Deserialise( Archive& ar )
{
	U32 version;
	S32 planeCount;

	ar >> version;
	ar >> planeCount;

	if(planeCount)
	{
		m_SplitPlaneList.Resize(planeCount);
	}

	S32 splitIndex;
	ar >> m_RootRegion.m_RegionId;
	ar >> splitIndex;
	if(splitIndex == -1)
	{
		m_RootRegion.m_pSplitPlane = NULL;
	}
	else
	{
		m_RootRegion.m_pSplitPlane = &m_SplitPlaneList[splitIndex];
	}

	for(S32 planeNo = 0; planeNo<planeCount; planeNo++)
	{
		C_SplitPlane& splitPlane = m_SplitPlaneList[planeNo];
		
		ar >> splitPlane.m_Plane;

		S32 frontIndex;
		ar >> splitPlane.m_Front.m_RegionId;
		ar >> frontIndex;

		if(frontIndex == -1)
		{
			splitPlane.m_Front.m_pSplitPlane = NULL;
		}
		else
		{
			splitPlane.m_Front.m_pSplitPlane = &m_SplitPlaneList[frontIndex];
		}

		S32 backIndex;
		ar >> splitPlane.m_Back.m_RegionId;
		ar >> backIndex;

		if(backIndex == -1)
		{
			splitPlane.m_Back.m_pSplitPlane = NULL;
		}
		else
		{
			splitPlane.m_Back.m_pSplitPlane = &m_SplitPlaneList[backIndex];
		}
	}
}



} // namespace RegionBSP

#endif
