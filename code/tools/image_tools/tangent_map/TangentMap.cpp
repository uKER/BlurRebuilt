
#include "stdafx.h"

#include "system\time\Time.h"
#include "system\debuglog.h"
#include <system/debug/log/AutoConfigure.h>

#include "Delaunay2d.h"

// find "Head" by finding any pixel with head colour
// flood fill head with tail colour keeping track of flood fill bounds
// use centre of bounds as vector head position
// flood fill tail with arbitrary colour from original head pixel, keep track of furthest distance from head "position" (centre of bounds position)

namespace TangentMap
{

static Debug::Logger& g_Log = Debug::GetLogger("NewTechSpec");

class C_Bounds
{
public:
	C_Bounds()
		: m_XMin(0)
		, m_XMax(0)
		, m_YMin(0)
		, m_YMax(0)
	{

	}
public:
	S32 m_XMin;
	S32 m_XMax;
	S32 m_YMin;
	S32 m_YMax;
};

class C_HeadInfo
{
public:
	C_HeadInfo() 
		: m_PixelX(0)
		, m_PixelY(0)
	{
	}

public:
	S32 m_PixelX; 
	S32 m_PixelY;

	S32 m_HeadX; // centre of bounds, only set after bounds calculated
	S32 m_HeadY;

	C_Bounds m_Bounds;
};

class C_FlowVector
{
public:
	C_FlowVector()
	{

	}

	~C_FlowVector()
	{

	}

	F32 GetDistance(S32 x, S32 y) const
	{
		F32 dx = (F32)m_HeadX - (F32)x;
		F32 dy = (F32)m_HeadY - (F32)y;
	
		F32 dist = dx * dx + dy * dy;
		if(dist > 0.0f)
			dist = sqrt(dist);
		return dist;
	}

	Maths::HVec3 GetDirection() const
	{
		F32 dx = (F32)m_TailX - (F32)m_HeadX;
		F32 dy = (F32)m_TailY - (F32)m_HeadY;
		F32 dz = 0.0f;
		Maths::HVec3 dir(dx, dy, dz);
		return dir.Normalised().Normal();
	}
	
	Maths::HVec2 GetDirection2d() const
	{
		F32 dx = (F32)m_TailX - (F32)m_HeadX;
		F32 dy = (F32)m_TailY - (F32)m_HeadY;

		Maths::HVec2 dir(dx, dy);
		return dir.Normalised();
	}

	F32 GetLength() const
	{
		F32 dx = (F32)m_TailX - (F32)m_HeadX;
		F32 dy = (F32)m_TailY - (F32)m_HeadY;
		F32 dz = 0.0f;
		Maths::HVec3 dir(dx, dy, dz);
		return dir.Length();
	}

public:
	S32 m_HeadX;
	S32 m_HeadY;

	S32 m_TailX;
	S32 m_TailY;

	S16 m_TriangulationIndex;
};

class C_OuterEdge
{
public:
	C_OuterEdge()
	{
	}
	C_OuterEdge(S16 vertexA, S16 vertexB)
		: m_TriangleRefCount(0)
		, m_VertexA(vertexA)
		, m_VertexB(vertexB)
	{
	}
public:
	S16 m_VertexA;
	S16 m_VertexB;
	S16 m_TriangleRefCount;
};

class C_FlowVectors
{
public:
	C_FlowVectors();
	~C_FlowVectors();

	void Find( Attk::C_Image& image );
	Maths::HVec3 GetLocalDirection(S32 x,S32 y);
	Maths::HVec3 C_FlowVectors::GetClosestLocalDirection(S32 x,S32 y);

private:
	bool FindVector( Attk::C_Image& image );
	bool FindHead(Attk::C_Image& image, C_HeadInfo& headInfo);
	bool FindHeadPixel(Attk::C_Image& image, C_HeadInfo& headInfo);
	void FindTail(Attk::C_Image& image, C_HeadInfo& headInfo, S32& tailX, S32& tailY);
	void Triangulate();
	void BuildOutsideEdges();
	Maths::HVec3 GetLocalDirectionFromEdge(S32 x,S32 y);
	void AddOuterEdge(S16 vertexA, S16 vertexB);

public:
	Ut::Vector<C_FlowVector> m_FlowVectorList;
	Delaunay2d::C_Triangulation m_Triangulation;
	Ut::Vector<C_OuterEdge> m_OuterEdgeList;
};



C_FlowVectors::C_FlowVectors()
{
}

C_FlowVectors::~C_FlowVectors()
{

}

bool ColourEqual(const Attk::C_PixelR32G32B32A32F& x, const Attk::C_PixelR32G32B32A32F& y)
{
	if(x.red != y.red)
		return false;
	if(x.green != y.green)
		return false;
	if(x.blue != y.blue)
		return false;

	return true;
}

Attk::C_PixelR32G32B32A32F headColour(1,0,1,0);
Attk::C_PixelR32G32B32A32F tailColour(0,1,1,0);
Attk::C_PixelR32G32B32A32F unusedColour(0,1,0,0);

bool IsHeadPixel(const Attk::C_PixelR32G32B32A32F& pixel)
{
	return ColourEqual(headColour, pixel);
}

class C_FloodFill
{
public:
	C_FloodFill();
	void Process(Attk::C_Image& image, S32 x, S32 y);

private:
	void FloodFillRecurse(Attk::C_Image& image, S32 x, S32 y);

public:
	Attk::C_PixelR32G32B32A32F m_MatchColour;
	Attk::C_PixelR32G32B32A32F m_FillColour;
	C_Bounds m_Bounds;

	S32 m_OriginX;
	S32 m_OriginY;

	F32 m_FurthestDist;
	S32 m_FurthestX;
	S32 m_FurthestY;
};

C_FloodFill::C_FloodFill()
	: m_OriginX(0)
	, m_OriginY(0)
	, m_FurthestDist(0)
{
}

void C_FloodFill::FloodFillRecurse(Attk::C_Image& image, S32 x, S32 y)
{
	//consider current pixel
	if(x < 0)
		return;
	if(y < 0)
		return;
	if(x >= (S32)image.GetWidth())
		return;
	if(y >= (S32)image.GetHeight())
		return;

	if(!ColourEqual(m_MatchColour, image.GetPixel( x,y )))
		return;

	// fill current pixel
	image.SetPixel(m_FillColour,x,y);

	// update extents
	if( x < m_Bounds.m_XMin )
		m_Bounds.m_XMin = x;
	if( x > m_Bounds.m_XMax )
		m_Bounds.m_XMax = x;
	if( y < m_Bounds.m_YMin )
		m_Bounds.m_YMin = y;
	if( y > m_Bounds.m_YMax )
		m_Bounds.m_YMax = y;

	{
		F32 dx = (F32)x - (F32)m_OriginX;
		F32 dy = (F32)y - (F32)m_OriginY;
		F32 currDist = (dx * dx + dy * dy);
		if(currDist > 0.0f)
			currDist = sqrt(currDist);

		if(currDist > m_FurthestDist)
		{
			m_FurthestDist = currDist;
			m_FurthestX = x;
			m_FurthestY = y;
		}
	}

	FloodFillRecurse( image, x+1, y );
	FloodFillRecurse( image, x-1, y );
	FloodFillRecurse( image, x, y+1 );
	FloodFillRecurse( image, x, y-1 );
}

void C_FloodFill::Process(Attk::C_Image& image, S32 x, S32 y)
{
	m_Bounds.m_XMin = x;
	m_Bounds.m_XMax = x;
	m_Bounds.m_YMin = y;
	m_Bounds.m_YMax = y;

	m_FurthestX = x;
	m_FurthestY = y;

	FloodFillRecurse(image, x, y);
}



bool C_FlowVectors::FindHeadPixel(Attk::C_Image& image, C_HeadInfo& headInfo)
{
	const S32 width = image.GetWidth();
	const S32 height = image.GetHeight();

	for(S32 y=0; y<height; y++)
	{
		for(S32 x=0; x<width; x++)
		{
			Attk::C_PixelR32G32B32A32F srcPixel = image.GetPixel( x,y );

			if(IsHeadPixel(srcPixel))
			{
				headInfo.m_PixelX = x;
				headInfo.m_PixelY = y;
				return true;
			}
		}
	}
	return false;
}

bool C_FlowVectors::FindHead(Attk::C_Image& image, C_HeadInfo& headInfo)
{
	const S32 width = image.GetWidth();
	const S32 height = image.GetHeight();

	bool found = FindHeadPixel(image, headInfo);

	if(!found)
		return false;

	C_FloodFill floodFill;
	floodFill.m_MatchColour = headColour;
	floodFill.m_FillColour = tailColour;
	
	floodFill.Process(image, headInfo.m_PixelX, headInfo.m_PixelY);

	headInfo.m_Bounds = floodFill.m_Bounds;

	headInfo.m_HeadX = (headInfo.m_Bounds.m_XMin + headInfo.m_Bounds.m_XMax) / 2;
	headInfo.m_HeadY = (headInfo.m_Bounds.m_YMin + headInfo.m_Bounds.m_YMax) / 2;

	return true;
}

void C_FlowVectors::FindTail(Attk::C_Image& image, C_HeadInfo& headInfo, S32& tailX, S32& tailY)
{
	C_FloodFill floodFill;
	floodFill.m_MatchColour = tailColour;
	floodFill.m_FillColour = unusedColour;
	
	floodFill.m_OriginX = headInfo.m_HeadX;
	floodFill.m_OriginY = headInfo.m_HeadY;

	floodFill.Process(image, headInfo.m_PixelX, headInfo.m_PixelY);

	tailX = floodFill.m_FurthestX;
	tailY = floodFill.m_FurthestY;
}

bool C_FlowVectors::FindVector( Attk::C_Image& image )
{
	C_HeadInfo headInfo;
	bool found = FindHead(image, headInfo);
	if(!found)
		return false;

	S32 tailX = 0;
	S32 tailY = 0;
	FindTail(image, headInfo, tailX, tailY);

	LOGINFO( g_Log, "Flow Vector[%d] (%d,%d)->(%d %d)", m_FlowVectorList.Size(), headInfo.m_HeadX, headInfo.m_HeadY, tailX, tailY);

	C_FlowVector flowVector;
	flowVector.m_HeadX = headInfo.m_HeadX;
	flowVector.m_HeadY = headInfo.m_HeadY;
	flowVector.m_TailX = tailX;
	flowVector.m_TailY = tailY;
	if(flowVector.GetLength() > 0.0f)
	{
		m_FlowVectorList.PushBack( flowVector );
	}

	return true;
}

//Maths::HVec3 C_FlowVectors::GetLocalDirection(S32 x,S32 y)
//{
//	Maths::HVec3 closestDir;
//	closestDir.X(0);
//	closestDir.Y(0);
//	closestDir.Z(0);
//	F32 closestDist = FLT_MAX;
//
//	for(S32 i=0; i<m_FlowVectorList.Size(); i++)
//	{
//		const C_FlowVector& flowVector = m_FlowVectorList[i];
//		
//		F32 dist = flowVector.GetDistance(x,y);
//		Maths::HVec3 dir = flowVector.GetDirection();
//
//		//closestDir += dir * (1.0f / (dist));
//		
//		if(dist < closestDist)
//		{
//			closestDist = dist;
//			closestDir = dir;
//		}
//		
//	}
//
//	return closestDir.Normalised().Normal();
//}

Maths::HVec3 C_FlowVectors::GetClosestLocalDirection(S32 x,S32 y)
{
	Maths::HVec3 closestDir;
	closestDir.X(0);
	closestDir.Y(0);
	closestDir.Z(0);
	F32 closestDist = FLT_MAX;

	for(S32 i=0; i<m_FlowVectorList.Size(); i++)
	{
		const C_FlowVector& flowVector = m_FlowVectorList[i];

		F32 dist = flowVector.GetDistance(x,y);
		Maths::HVec3 dir = flowVector.GetDirection();

		//closestDir += dir * (1.0f / (dist));

		if(dist < closestDist)
		{
			closestDist = dist;
			closestDir = dir;
		}
	}

	return closestDir.Normalised().Normal();
}

void UpdateClosestEdge(S32 x,S32 y, const C_FlowVector& flowVectorA, const C_FlowVector& flowVectorB, F32& closestEdgeDist, Maths::HVec2& closestEdgeDir)
{
	const Maths::HVec2 p0 = Maths::HVec2((F32)flowVectorA.m_HeadX, (F32)flowVectorA.m_HeadY);
	const Maths::HVec2 p1 = Maths::HVec2((F32)flowVectorB.m_HeadX, (F32)flowVectorB.m_HeadY);
	Maths::HVec2 point = Maths::HVec2((F32)x, (F32)y);

	Maths::HVec2 edgeDir = p1 - p0;	
	F32 edgeLength = edgeDir.Length();
	edgeDir.Normalise();

	F32 t = (point - p0) | edgeDir;

	if( t < 0 )
		t = 0;

	if( t > edgeLength )
		t = edgeLength;

	Maths::HVec2 closestPos = p0 + edgeDir * t;

	F32 dist = (closestPos - point).Length();
	if( dist < closestEdgeDist )
	{
		t = t / edgeLength;
		Maths::HVec2 dir0 = flowVectorA.GetDirection2d();
		Maths::HVec2 dir1 = flowVectorB.GetDirection2d();

		Maths::HVec2 dir = dir0 * (1.0f - t) + dir1 * (t);
		dir = dir.Normalised();	

		closestEdgeDir = dir;
		closestEdgeDist = dist;
	}
}

Maths::HVec3 C_FlowVectors::GetLocalDirectionFromEdge(S32 x,S32 y)
{
	F32 closestEdgeDist = FLT_MAX;
	Maths::HVec2 closestEdgeDir(0.0f, 0.0f);

	for(S32 i=0; i<m_OuterEdgeList.Size(); i++)
	{
		S32 vertexIndexA = m_OuterEdgeList[i].m_VertexA;
		S32 vertexIndexB = m_OuterEdgeList[i].m_VertexB;

		const C_FlowVector& flowVectorA = m_FlowVectorList[vertexIndexA];
		const C_FlowVector& flowVectorB = m_FlowVectorList[vertexIndexB];

		UpdateClosestEdge(x,y, flowVectorA, flowVectorB, closestEdgeDist, closestEdgeDir);
	}
	//Delaunay2d::C_Triangle* currTri( m_Triangulation.m_pFirstUsedTri );
	//for( S32 i = 0 ; i < Delaunay2d::C_Triangulation::E_MaxTris && currTri ; i++, currTri = currTri->m_pNext )
	//{
	//	S32 vertexIndexA = currTri->m_VertexIndex[0];
	//	S32 vertexIndexB = currTri->m_VertexIndex[1];
	//	S32 vertexIndexC = currTri->m_VertexIndex[2];

	//	const C_FlowVector& flowVectorA = m_FlowVectorList[vertexIndexA];
	//	const C_FlowVector& flowVectorB = m_FlowVectorList[vertexIndexB];
	//	const C_FlowVector& flowVectorC = m_FlowVectorList[vertexIndexC];

	//	UpdateClosestEdge(x,y, flowVectorA, flowVectorB, closestEdgeDist, closestEdgeDir);
	//	UpdateClosestEdge(x,y, flowVectorB, flowVectorC, closestEdgeDist, closestEdgeDir);
	//	UpdateClosestEdge(x,y, flowVectorC, flowVectorA, closestEdgeDist, closestEdgeDir);
	//}

	return closestEdgeDir;
}

Maths::HVec3 C_FlowVectors::GetLocalDirection(S32 x,S32 y)
{
	//return GetClosestLocalDirection(x,y);

	Delaunay2d::C_Triangle* currTri( m_Triangulation.m_pFirstUsedTri );
	for( S32 i = 0 ; i < Delaunay2d::C_Triangulation::E_MaxTris && currTri ; i++, currTri = currTri->m_pNext )
	{
		S32 vertexIndexA = currTri->m_VertexIndex[0];
		S32 vertexIndexB = currTri->m_VertexIndex[1];
		S32 vertexIndexC = currTri->m_VertexIndex[2];

		const C_FlowVector& flowVectorA = m_FlowVectorList[vertexIndexA];
		const C_FlowVector& flowVectorB = m_FlowVectorList[vertexIndexB];
		const C_FlowVector& flowVectorC = m_FlowVectorList[vertexIndexC];

		const Maths::HVec2 p0 = Maths::HVec2((F32)flowVectorA.m_HeadX, (F32)flowVectorA.m_HeadY);
		const Maths::HVec2 p1 = Maths::HVec2((F32)flowVectorB.m_HeadX, (F32)flowVectorB.m_HeadY);
		const Maths::HVec2 p2 = Maths::HVec2((F32)flowVectorC.m_HeadX, (F32)flowVectorC.m_HeadY);

		Maths::HVec2 point = Maths::HVec2((F32)x, (F32)y);

		Delaunay2d::C_TriangleBarycentricCoord coord(point, p0, p1, p2 );
		if( coord.IsInside() )
		{
			Maths::HVec2 dir0 = flowVectorA.GetDirection2d();
			Maths::HVec2 dir1 = flowVectorB.GetDirection2d();
			Maths::HVec2 dir2 = flowVectorC.GetDirection2d();

			Maths::HVec2 dir = dir0 * coord.m_Coefs.X() + dir1 * coord.m_Coefs.Y() + dir2 * coord.m_Coefs.Z();
			dir = dir.Normalised();			
			return Maths::HVec3(dir.X(), dir.Y(), 0.0f);
			//return Maths::HVec3(0.0f, 0.0f, 0.0f);
		}
	}

	//return GetClosestLocalDirection(x,y);
	return GetLocalDirectionFromEdge(x,y);
}

void C_FlowVectors::Find( Attk::C_Image& image )
{
	bool done = false;

	while(!done)
	{
		bool found = FindVector( image );
		
		if(!found)
			done = true;
	}

	// add corner vectors

	for(S32 i=0; i<4; i++)
	{
		const S32 width = image.GetWidth();
		const S32 height = image.GetHeight();
		S32 x;
		S32 y;

		switch(i)
		{
			case 0: x= 0; y = 0; break;
			case 1: x= width; y = 0; break;
			case 2: x= 0; y = height; break;
			case 3: x= width; y = height; break;
		}
		Maths::HVec3 dir = GetClosestLocalDirection(x,y);

		C_FlowVector flowVector;
		flowVector.m_HeadX = x;
		flowVector.m_HeadY = y;
		flowVector.m_TailX = (S32)(x + dir.X() * 20.0f);
		flowVector.m_TailY = (S32)(y + dir.Y() * 20.0f);

		m_FlowVectorList.PushBack(flowVector);
	}

	Triangulate();
}

void C_FlowVectors::Triangulate()
{
	LOGINFO( g_Log, "Triangulating");


	for(S32 i=0; i<m_FlowVectorList.Size(); i++)
	{
		C_FlowVector& flowVector = m_FlowVectorList[i];
		Maths::HVec2 pos2d((F32)flowVector.m_HeadX, (F32)flowVector.m_HeadY);
		flowVector.m_TriangulationIndex = m_Triangulation.AddVertex(pos2d);
	}
	
	if(m_Triangulation.m_nVertex < 3)
	{
		LOGERROR( g_Log, "need at least 3 flow vectors" );
	}
	else
	{
		m_Triangulation.Triangulate();
	}
	
	if(m_Triangulation.m_nTris < 1)
	{
		LOGERROR( g_Log, "triangulation gave no triangles?" );
		return;
	}

	if(m_Triangulation.m_nTris > Delaunay2d::C_Triangulation::E_MaxTris)
	{
		LOGERROR( g_Log, "triangulation gave too many triangles (%d/%d)", m_Triangulation.m_nTris, (S32)Delaunay2d::C_Triangulation::E_MaxTris );
		return;
	}

	BuildOutsideEdges();
}

void C_FlowVectors::AddOuterEdge(S16 vertexA, S16 vertexB)
{
	if(vertexB < vertexA)
	{
		S16 temp = vertexA;
		vertexA = vertexB;
		vertexB = temp;
	}

	// see if edge already exists
	S32 found = -1;
	for(S32 i=0;i<m_OuterEdgeList.Size(); i++)
	{
		if((m_OuterEdgeList[i].m_VertexA == vertexA) && (m_OuterEdgeList[i].m_VertexB == vertexB))
		{
			m_OuterEdgeList[i].m_TriangleRefCount++;
			return;
		}
	}

	m_OuterEdgeList.PushBack( C_OuterEdge(vertexA, vertexB) );
}

//build outside edge list
void C_FlowVectors::BuildOutsideEdges()
{
	// add all triangle edges to outer edge list
	Delaunay2d::C_Triangle* currTri( m_Triangulation.m_pFirstUsedTri );
	for( S32 i = 0 ; i < Delaunay2d::C_Triangulation::E_MaxTris && currTri ; i++, currTri = currTri->m_pNext )
	{
		S32 vertexIndexA = currTri->m_VertexIndex[0];
		S32 vertexIndexB = currTri->m_VertexIndex[1];
		S32 vertexIndexC = currTri->m_VertexIndex[2];

		AddOuterEdge(vertexIndexA, vertexIndexB);
		AddOuterEdge(vertexIndexB, vertexIndexC);
		AddOuterEdge(vertexIndexC, vertexIndexA);
	}

	// remove duplicate outer edges

	S32 j=0;
	while(j<m_OuterEdgeList.Size())
	{
		if(m_OuterEdgeList[j].m_TriangleRefCount > 1)
		{
			m_OuterEdgeList.EraseByIndex(i);
		}
		else
		{
			j++;
		}
	}
}

void SetPixel( Attk::C_Image& image, const Attk::C_PixelR32G32B32A32F& colour, S32 x, S32 y )
{
	if( x < 0 )
		return;
	if( y < 0 )
		return;
	if( x >= image.GetWidth() )
		return;
	if( y >= image.GetHeight() )
		return;

	image.SetPixel( colour, x, y );
}

void CircleFill( Attk::C_Image& image, S32 Ox, S32 Oy, S32 radius, const Attk::C_PixelR32G32B32A32F& colour)
{
	for(S32 h = -radius; h<=radius; h++)
	{
		F32 fWidth = radius * radius - h * h;
		if(fWidth > 0.0f)
		{
			fWidth = sqrt(fWidth);
		}
		S32 width = fWidth;
		for(S32 x = -width; x<width; x++)
		{
			SetPixel( image, colour, Ox + x, Oy + h );
		}
	}
}

void DrawLine( Attk::C_Image& image, S32 x0, S32 y0, S32 x1, S32 y1, const Attk::C_PixelR32G32B32A32F& colour)
{
   int Dx = x1 - x0; 
   int Dy = y1 - y0;
   int steep = (abs(Dy) >= abs(Dx));
   if (steep) {
	   Ut::Swap(x0, y0);
       Ut::Swap(x1, y1);
       // recompute Dx, Dy after swap
       Dx = x1 - x0;
       Dy = y1 - y0;
   }
   int xstep = 1;
   if (Dx < 0) {
       xstep = -1;
       Dx = -Dx;
   }
   int ystep = 1;
   if (Dy < 0) {
       ystep = -1;		
       Dy = -Dy; 
   }
   int TwoDy = 2*Dy; 
   int TwoDyTwoDx = TwoDy - 2*Dx; // 2*Dy - 2*Dx
   int E = TwoDy - Dx; //2*Dy - Dx
   int y = y0;
   int xDraw, yDraw;	
   for (int x = x0; x != x1; x += xstep) {		
       if (steep) {			
           xDraw = y;
           yDraw = x;
       } else {			
           xDraw = x;
           yDraw = y;
       }
       // plot
       SetPixel(image, colour, xDraw, yDraw);
       // next
       if (E > 0) {
           E += TwoDyTwoDx; //E += 2*Dy - 2*Dx;
           y = y + ystep;
       } else {
           E += TwoDy; //E += 2*Dy;
       }
   }
}

void DrawThickLine( Attk::C_Image& image, S32 x0, S32 y0, S32 x1, S32 y1, const Attk::C_PixelR32G32B32A32F& colour)
{
	DrawLine( image, x0, y0, x1, y1, colour);

	DrawLine( image, x0-1, y0, x1-1, y1, colour);
	DrawLine( image, x0+1, y0, x1+1, y1, colour);
	DrawLine( image, x0, y0-1, x1, y1-1, colour);
	DrawLine( image, x0, y0+1, x1, y1+1, colour);
}

void WriteFlowImage(C_FlowVectors flowVectors, S32 width, S32 height)
{
	Attk::C_PixelR32G32B32A32F fillColour(0.0f,0.0f,0.0f,0);
	Attk::C_PixelR32G32B32A32F triangleColour(0.2f,0.2f,0.2f,0);
	
	Attk::C_Image destImage;

	destImage.Clear();
	destImage.DeletePixelData();

	destImage.NewPixelData( width, height, 1, Attk::E_PixelFormat_B8G8R8A8U );
	destImage.Fill( fillColour );

	// draw triangulation
	Delaunay2d::C_Triangle* currTri( flowVectors.m_Triangulation.m_pFirstUsedTri );
	for( S32 i = 0 ; i < Delaunay2d::C_Triangulation::E_MaxTris && currTri ; i++, currTri = currTri->m_pNext )
	{
		S32 vertexIndexA = currTri->m_VertexIndex[0];
		S32 vertexIndexB = currTri->m_VertexIndex[1];
		S32 vertexIndexC = currTri->m_VertexIndex[2];

		const C_FlowVector& flowVectorA = flowVectors.m_FlowVectorList[vertexIndexA];
		const C_FlowVector& flowVectorB = flowVectors.m_FlowVectorList[vertexIndexB];
		const C_FlowVector& flowVectorC = flowVectors.m_FlowVectorList[vertexIndexC];

		DrawLine( destImage, flowVectorA.m_HeadX, flowVectorA.m_HeadY, flowVectorB.m_HeadX, flowVectorB.m_HeadY, triangleColour);
		DrawLine( destImage, flowVectorB.m_HeadX, flowVectorB.m_HeadY, flowVectorC.m_HeadX, flowVectorC.m_HeadY, triangleColour);
		DrawLine( destImage, flowVectorC.m_HeadX, flowVectorC.m_HeadY, flowVectorA.m_HeadX, flowVectorA.m_HeadY, triangleColour);
	}


	// draw flow vector positions
	for(S32 i=0; i<flowVectors.m_FlowVectorList.Size(); i++)
	{
		C_FlowVector& flowVector = flowVectors.m_FlowVectorList[i];
		CircleFill( destImage, flowVector.m_HeadX, flowVector.m_HeadY, 3, headColour);
	}

	// draw flow vectors
	for(S32 i=0; i<flowVectors.m_FlowVectorList.Size(); i++)
	{
		C_FlowVector& flowVector = flowVectors.m_FlowVectorList[i];
		DrawThickLine( destImage, flowVector.m_HeadX, flowVector.m_HeadY, flowVector.m_TailX, flowVector.m_TailY, tailColour);
	}

	Util::C_String tempFileName = "_TangentDebug_FlowVectors.tga";
	bool status = destImage.Write( tempFileName, Attk::E_ImageFormat_BestGuess );
	if( !status )
	{
		LOGINFO( g_Log, "   **** Error writing '%s'\n", tempFileName.Chars());
	}
}

void WriteFieldImage(C_FlowVectors flowVectors, S32 width, S32 height)
{
	Attk::C_PixelR32G32B32A32F fillColour(0.0f,0.0f,0.0f,0);
	Attk::C_PixelR32G32B32A32F triangleColour(0.2f,0.2f,0.2f,0);
	
	Attk::C_Image destImage;

	destImage.Clear();
	destImage.DeletePixelData();

	destImage.NewPixelData( width, height, 1, Attk::E_PixelFormat_B8G8R8A8U );
	destImage.Fill( fillColour );

	S32 spacing = 32;
	S32 length = 16;



	for(S32 y=spacing/2; y<height; y+=spacing)
	{
		for(S32 x=spacing/2; x<width; x+=spacing)
		{
			Maths::HVec3 dir = flowVectors.GetLocalDirection(x,y);
			CircleFill( destImage, x, y, 3, headColour);
			DrawLine( destImage, x, y, x + dir.X() * length, y + dir.Y() * length, tailColour);

		}
	}

	Util::C_String tempFileName = "_TangentDebug_FlowField.tga";
	bool status = destImage.Write( tempFileName, Attk::E_ImageFormat_BestGuess );
	if( !status )
	{
		LOGINFO( g_Log, "   **** Error writing '%s'\n", tempFileName.Chars());
	}
}

bool ConvertImage( Attk::C_Image& srcImage, Attk::C_Image& destImage )
{	
	bool bHasAlpha = false;
	//LOGINFO( g_Log, "   Converting\n");

	C_FlowVectors flowVectors;
	flowVectors.Find(srcImage);

	Util::C_String tempFileName = "_TangentDebug_FoundVectors.tga";
	bool status = srcImage.Write( tempFileName, Attk::E_ImageFormat_BestGuess );
	if( !status )
	{
		LOGINFO( g_Log, "   **** Error writing '%s'\n", tempFileName.Chars());
	}

	WriteFlowImage(flowVectors, srcImage.GetWidth(), srcImage.GetHeight());
	WriteFieldImage(flowVectors, srcImage.GetWidth(), srcImage.GetHeight());

	LOGINFO( g_Log, "Creating Image");

	const S32 width = srcImage.GetWidth();
	const S32 height = srcImage.GetHeight();

	const F32 scale = 1.0f;

	for(S32 y=0; y<height; y++)
	{
		if((y % 32) == 0)
		{
			LOGINFO( g_Log, "%d Percent Complete", y * 100 / height);
		}

		for(S32 x=0; x<width; x++)
		{
			Attk::C_PixelR32G32B32A32F destPixel(0,0,0,0);

			Maths::HVec3 dir = flowVectors.GetLocalDirection(x,y);

			destPixel.red = dir.X() * 0.5f + 0.5f;
			destPixel.green = dir.Y() * 0.5f + 0.5f;
			destPixel.blue = dir.Z() * 0.5f + 0.5f;

			destImage.SetPixel( destPixel, x,y );
			//Attk::C_PixelR32G32B32A32F destPixel(0,0,0,0);
			//
			//Attk::C_PixelR32G32B32A32F srcPixel = srcImage.GetPixel( x,y );

			//F32 r = srcPixel.red;
			//F32 g = srcPixel.green;
			//F32 b = srcPixel.blue;
			//F32 a = srcPixel.alpha;

			//F32 redScale = 0.9f;
			//F32 redPower = 0.25f;

			//F32 greenScale = 1.1f;
			//F32 greenPower = 1.0f;

			//F32 blueScale = 0.5f;
			//F32 bluePower = 1.0f;

			//r = pow(r, redPower) * redScale;
			//g = pow(g, greenPower) * greenScale;
			//b = pow(b, bluePower) * blueScale;

			//if( r>1.0f ) r = 1.0f;
			//if( g>1.0f ) g = 1.0f;
			//if( b>1.0f ) b = 1.0f;

			//if( r<0.0f ) r = 0.0f;
			//if( g<0.0f ) g = 0.0f;
			//if( b<0.0f ) b = 0.0f;

			//destPixel.red = r;
			//destPixel.green = g;
			//destPixel.blue = b;
			//destPixel.alpha = a;

			//if( a > 0.0f )
			//{
			//	bHasAlpha = true;
			//}

			//destImage.SetPixel( destPixel, x, y );
		}
	}

	return bHasAlpha;
}


bool ConvertImage( Util::C_String& inFileName, Util::C_String& outFileName )
{
	LOGINFO( g_Log, "   Reading %s\n", inFileName.Chars());
	
	Attk::C_Image srcImage;
	srcImage.Read( inFileName );

	LOGINFO( g_Log, "   Converting %s\n", inFileName.Chars());

	const U32 width = srcImage.GetWidth();
	const U32 height = srcImage.GetHeight();

	//LOGINFO( g_Log, "   Creating destination image\n");
	Attk::C_Image destImage;

	destImage.Clear();
	destImage.DeletePixelData();

	destImage.NewPixelData( width, height, 1, Attk::E_PixelFormat_B8G8R8A8U );
	destImage.Fill( Attk::C_PixelR32G32B32A32F(1.0f, 1.0f, 1.0f, 0.0f) );


	bool bHasAlpha = ConvertImage(srcImage, destImage);

	destImage.ChangePixelFormat( srcImage.GetPixelFormat() );

	LOGINFO( g_Log, "Writing Output file %s", outFileName.Chars());

	bool status = destImage.Write( outFileName, Attk::E_ImageFormat_BestGuess );
	if( !status )
	{
		LOGINFO( g_Log, "   **** Error writing '%s'\n", outFileName.Chars());
		return false;
	}

	//LOGINFO( g_Log, "   Done\n\n", outFileName.Chars());
	return true;
}

bool ProcessFile( Util::C_String& srcFileName, Util::C_String& dstFileName )
{
	bool success = true;
	Util::C_String ext = Util::ExtractFileExtension(srcFileName).Lower();

	if( (ext == "tga") || (ext == "bmp") || (ext == "pic") )
	{
		Sys::C_Fileinfo fileInfo;
		Sys::C_FileSystem::Instance().FileInfo(srcFileName, &fileInfo);

		if( !(fileInfo.m_FileAccess & Sys::E_FileAccess_Read) )
		{
			LOGINFO( g_Log, "   *** No read access for %s - FILE NOT CONVERTED\n", srcFileName.Chars());
		
		}
		else
		{
			success = ConvertImage( srcFileName, dstFileName );			
		}
	}
	return success;
}


} // namespace tangentmap


int main(int argc, char* argv[])
//int _tmain(int argc, _TCHAR* argv[])
{
	Debug::AutoConfigureDebugLog();
	if((argc < 2) || (argc > 3))
	{
		LOGINFO( TangentMap::g_Log, "%s\nGenerates Tangent Map from source texture\n\n", argv[0]);
		LOGINFO( TangentMap::g_Log, "Usage:\n%s srcTextureFile <dstTextureFile>\n\n", argv[0]);
	}
	else
	{	
		Util::C_String srcFileName = argv[1];
		srcFileName = Util::FormatFileString(srcFileName);

		Util::C_String dstFileName;
		if(argc > 2)
		{
			dstFileName = argv[2];
			dstFileName = Util::FormatFileString(dstFileName);
		}
		else
		{
			Util::C_String directory = Util::ExtractDirectory(srcFileName);
			Util::C_String filename = Util::RemoveFileExtension( Util::ExtractFilename(srcFileName) );
			Util::C_String extension = Util::ExtractFileExtension(srcFileName);
			dstFileName = directory + filename + "_Tangent." +extension;		
		}
		bool success = TangentMap::ProcessFile(srcFileName, dstFileName);

		//LOGINFO( TangentMap::g_Log, "Press any key...\n");
	}

	//getch();

	return 0;
}