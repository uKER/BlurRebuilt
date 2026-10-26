#include "StdAfx.h"
#include <core/maths.h>

//Project one triangle onto another, this is the maximum number of vertices
const U32 MAX_TRI_TRI_VERTICES = 8;

struct C_TriangleClipResult
{
	Maths::HVec3 m_Vertices[MAX_TRI_TRI_VERTICES];
	U32 m_NumVertices;
};

struct C_Triangle
{
	Maths::HVec3 m_VertexA, m_VertexB, m_VertexC;
};

struct C_Box
{
	Maths::HVec3 m_VertexA, m_VertexB, m_VertexC, m_VertexD;
};

enum E_IntersectionState
{
	E_AllOutsidePlane,
	E_AllInsidePlane,
	E_CrossPlaneEnter,
	E_CrossPlaneExit,
	E_CrossPlane
};

E_IntersectionState IntersectPlaneSegment(const Maths::C_Plane& plane, const Maths::HVec3 PCREF vertA, const Maths::HVec3 PCREF vertB, Maths::HVec3& intersectionPoint, Maths::HF32& ratio)
{
	//Test to see if edge VertA - vertB intersection the plane...if so, calculate the point of intersection
	const Maths::HF32 d0(plane.Distance(vertA));
	const Maths::HF32 d1(plane.Distance(vertB));
	const Maths::HF32 zero(Maths::Constants<Maths::HF32>::Zero());
	if((d0 * d1) >= zero)
	{
		if(d0 >= zero && d1 >= zero)
		{
			return E_AllInsidePlane;
		}
		return E_AllOutsidePlane;
	}

	//Crosses plane, so work out what the intersection point is!
	ratio = (d0/(d0 - d1));
	const Maths::HVec3 d(vertB - vertA);
	intersectionPoint = vertA + (d * ratio);
	return (d0 >= zero) ? E_CrossPlaneExit : E_CrossPlaneEnter;
}

//Returns false if segment outside of triangle!
E_IntersectionState ClipSegment(const Maths::C_Plane& planeA, const Maths::C_Plane& planeB, const Maths::C_Plane& planeC,
				 const Maths::HVec3 PCREF vertA, const Maths::HVec3 PCREF vertB,
				 Maths::HVec3& start, Maths::HVec3& end)
{
	const Maths::HF32 zero(Maths::Constants<Maths::HF32>::Zero());
	const Maths::HF32 one(Maths::Constants<Maths::HF32>::One());
	Maths::HF32 ratioA, ratioB, ratioC;
	Maths::HVec3 intersectA, intersectB, intersectC;

	E_IntersectionState stateA = IntersectPlaneSegment(planeA, vertA, vertB, intersectA, ratioA);
	E_IntersectionState stateB = IntersectPlaneSegment(planeB, vertA, vertB, intersectB, ratioB);
	E_IntersectionState stateC = IntersectPlaneSegment(planeB, vertA, vertB, intersectC, ratioC);

	if(stateA == E_AllOutsidePlane || stateB == E_AllOutsidePlane || stateC == E_AllOutsidePlane)
		return E_AllOutsidePlane;
	if(stateA == E_AllInsidePlane && stateB == E_AllInsidePlane && stateC == E_AllInsidePlane)
	{
		//segment completely inside triangle!
		start = vertA;
		end = vertB;
		return E_AllInsidePlane;
	}

	//Find the largest enter state!

	Maths::HF32 enterDistance = zero;
	Maths::HF32 exitDistance = one;
	start = vertA;
	end = vertB;

	if(stateA == E_CrossPlaneEnter)
	{
		enterDistance = ratioA;
		start = intersectA;
	}
	else if (stateA == E_CrossPlaneExit)
	{
		exitDistance = ratioA;
		end = intersectA;
	}

	if(stateB == E_CrossPlaneEnter)
	{
		if(enterDistance < ratioB)
		{
			enterDistance = ratioB;
			start = intersectB;
		}
	}
	else if (stateB == E_CrossPlaneExit)
	{
		if(exitDistance > ratioB)
		{
			exitDistance = ratioB;
			end = intersectB;
		}
	}

	if(stateC == E_CrossPlaneEnter)
	{
		if(enterDistance < ratioC)
		{
			enterDistance = ratioC;
			start = intersectC;
		}
	}
	else if (stateC == E_CrossPlaneExit)
	{
		if(exitDistance > ratioC)
		{
			exitDistance = ratioC;
			end = intersectC;
		}
	}

	/*//Harder case - not all outside or all inside!
	if(stateA == E_CrossPlane || stateB == E_CrossPlane || stateC == E_CrossPlane)
	{
		DebugBreak();
		printf("Error - seems we are clipping a triangle onto a degenerate triangle");
		return E_AllOutsidePlane;
	}*/

	/*if(stateA == E_CrossPlane)
	{
		if(stateB == E_CrossPlane)
		{
			if(ratioA < ratioB)
			{
				start = intersectA;
				end = intersectB;
			}
			else
			{
				start = intersectB;
				end = intersectA;
			}
		}
		else if (stateC == E_CrossPlane)
		{
			if(ratioA < ratioC)
			{
				start = intersectA;
				end = intersectC;
			}
			else
			{
				start = intersectC;
				end = intersectA;
			}
		}
		else
		{
			if(planeA.Distance(vertA) >= zero)
			{
				start = vertA;
				end = intersectA;
			}
			else
			{
				start = intersectA;
				end = vertB;
			}
		}
	}
	else if (stateB == E_CrossPlane)
	{
		if(stateC == E_CrossPlane)
		{
			if(ratioB < ratioC)
			{
				start = intersectB;
				end = intersectC;
			}
			else
			{
				start = intersectC;
				end = intersectB;
			}
		}
		else
		{
			if(planeB.Distance(vertA) >= zero)
			{
				start = vertA;
				end = intersectB;
			}
			else
			{
				start = intersectB;
				end = vertB;
			}
		}
	}
	else //stateC == E_CrossPlane
	{
		if(planeC.Distance(vertA) >= zero)
		{
			start = vertA;
			end = intersectC;
		}
		else
		{
			start = intersectC;
			end = vertB;
		}
	}*/

	return E_CrossPlane;
}


/*
 * The following code clips triangleA onto triangleB's edge planes
 */
void ClipTriangle(const C_Triangle& triangleA, const C_Triangle& triangleB, C_TriangleClipResult& result)
{
	Maths::C_Plane triPlane(triangleB.m_VertexA, triangleB.m_VertexB, triangleB.m_VertexC);
	//Calculate the 3 planes for triangleB!
	Maths::C_Plane planeAB(triangleB.m_VertexA, triangleB.m_VertexB, Maths::C_Vertex(triangleB.m_VertexB + triPlane.Normal()));
	Maths::C_Plane planeAC(triangleB.m_VertexA, triangleB.m_VertexC, Maths::C_Vertex(triangleB.m_VertexC + triPlane.Normal()));
	Maths::C_Plane planeBC(triangleB.m_VertexB, triangleB.m_VertexC, Maths::C_Vertex(triangleB.m_VertexC + triPlane.Normal()));

	//Now we need to find the line intersection points between triangle A's edges and triangleB's planes!

	Maths::HVec3 intersect[6];

	E_IntersectionState stateAB = ClipSegment(planeAB, planeAC, planeBC, triangleA.m_VertexA, triangleA.m_VertexB,
		intersect[0], intersect[1]);
	E_IntersectionState stateBC = ClipSegment(planeAB, planeAC, planeBC, triangleA.m_VertexB, triangleA.m_VertexC,
		intersect[2], intersect[3]);
	E_IntersectionState stateAC = ClipSegment(planeAB, planeAC, planeBC, triangleA.m_VertexC, triangleA.m_VertexA,
		intersect[4], intersect[5]);

	result.m_NumVertices = 0;
	if(stateAB == E_AllOutsidePlane || stateBC == E_AllOutsidePlane || stateAC == E_AllOutsidePlane)
	{
		return;
	}
	//Work out which vertices constitute the clipped triangle!

	result.m_Vertices[0] = intersect[0];
	result.m_NumVertices = 1;
	for(U32 a = 1; a < 6; ++a)
	{
		if(result.m_Vertices[result.m_NumVertices - 1].Any_NE(intersect[a]))
		{
			result.m_Vertices[result.m_NumVertices++] = intersect[a];
		}
	}
}

void ClipBox(const C_Box& boxA, const C_Triangle& triangleB, C_TriangleClipResult& result)
{
	Maths::C_Plane triPlane(triangleB.m_VertexA, triangleB.m_VertexB, triangleB.m_VertexC);
	//Calculate the 3 planes for triangleB!
	Maths::C_Plane planeAB(triangleB.m_VertexA, triangleB.m_VertexB, Maths::C_Vertex(triangleB.m_VertexB + triPlane.Normal()));
	Maths::C_Plane planeAC(triangleB.m_VertexA, triangleB.m_VertexC, Maths::C_Vertex(triangleB.m_VertexC + triPlane.Normal()));
	Maths::C_Plane planeBC(triangleB.m_VertexB, triangleB.m_VertexC, Maths::C_Vertex(triangleB.m_VertexC + triPlane.Normal()));

	//Now we need to find the line intersection points between triangle A's edges and triangleB's planes!

	Maths::HVec3 intersect[8];

	E_IntersectionState stateAB = ClipSegment(planeAB, planeAC, planeBC, boxA.m_VertexA, boxA.m_VertexB,
		intersect[0], intersect[1]);
	E_IntersectionState stateBC = ClipSegment(planeAB, planeAC, planeBC, boxA.m_VertexB, boxA.m_VertexC,
		intersect[2], intersect[3]);
	E_IntersectionState stateCD = ClipSegment(planeAB, planeAC, planeBC, boxA.m_VertexC, boxA.m_VertexD,
		intersect[4], intersect[5]);
	E_IntersectionState stateAD = ClipSegment(planeAB, planeAC, planeBC, boxA.m_VertexD, boxA.m_VertexA,
		intersect[6], intersect[7]);

	result.m_NumVertices = 0;
	if( stateAB == E_AllOutsidePlane || stateBC == E_AllOutsidePlane || stateCD == E_AllOutsidePlane || 
		stateAD == E_AllOutsidePlane )
	{
		return;
	}
	//Work out which vertices constitute the clipped triangle!

	result.m_Vertices[0] = intersect[0];
	result.m_NumVertices = 1;
	for(U32 a = 1; a < 8; ++a)
	{
		if(result.m_Vertices[result.m_NumVertices - 1].Any_NE(intersect[a]))
		{
			result.m_Vertices[result.m_NumVertices++] = intersect[a];
		}
	}
}
