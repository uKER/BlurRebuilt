#ifndef SIMPLEPOLY_H
#define SIMPLEPOLY_H

#ifdef _MSC_VER
#pragma once
#endif

#include <core/core.h>
#include <core/Maths.h>
#include <system/debug/I_DebugRender.h>

namespace SimplePoly
{

extern Debug::Logger &g_Log;

#ifdef PLATFORM_PC
typedef Ut::UtVector<Ut::DynamicVector<Maths::HVec3, Ut::ForceAlignedAllocator<Maths::HVec3, 16> > > C_VertexList;
typedef Ut::UtVector<Ut::DynamicVector<Maths::C_Plane, Ut::ForceAlignedAllocator<Maths::C_Plane, 16> > > C_PlaneList;
#else
typedef Ut::Vector<Maths::HVec3> C_VertexList;
typedef Ut::Vector<Maths::C_Plane> C_PlaneList;
#endif

enum E_Feature
{
	E_Feature_Vertex = 0,
	E_Feature_Edge,
	E_Feature_Face,
	E_Feature_Count
};

class C_ClosestPointInfo
{
public:
	Maths::HVec3	m_Pos;
	E_Feature		m_Feature;
	S32				m_FeatureIndex;
};

const Maths::HVec3 ClosestPoint( C_VertexList& vertexList, Maths::C_Plane& plane, const Maths::HVec3& pos );

enum E_PolyPlaneTest
{
	E_PolyPlaneTest_Coplanar = 0,
	E_PolyPlaneTest_InFront,
	E_PolyPlaneTest_Behind,
	E_PolyPlaneTest_Straddled
};

class C_Polygon
{
public:    
	C_Polygon();

	~C_Polygon();
    
    void                    operator =( const C_Polygon& other );    
    
    void                    NewVertex( const Maths::HVec3& pos );

    void                    CalculatePlane();

	// Must call calculate plane before GetCentroid
	const Maths::HVec3		Centroid() const;

	// Must call calculate plane before Plane();
	const Maths::C_Plane    Plane() const;

	bool					IsPlaner( const F32 epsilon );

	// Clip with front and back is deprecated, call Split instead
	DEPRECATED void         Clip( const Maths::C_Plane& plane, const F32 epsilon, C_Polygon& front, C_Polygon& back )
	{
		Split(plane, epsilon, front, back);
	}
	
	void					Clip( const Maths::C_Plane& plane, C_Polygon& result );

	void                    Split( const Maths::C_Plane& plane, const F32 epsilon, C_Polygon& front, C_Polygon& back );

	void                    Classify( const Maths::C_Plane& plane, const F32 epsilon, S32& frontCount, S32& backCount );

	const E_PolyPlaneTest	Classify( const Maths::C_Plane& plane, const F32 epsilon );

	// find the closest point to the polygon
	// NOTE : only valid for convex polygons
	void					ClosestPoint( const Maths::HVec3& pos, C_ClosestPointInfo& result );

	void					DebugRender( DbUtil::I_DebugRenderPtr debugRenderPtr, const Maths::RGBA& colour );

	void                    DebugRenderSolid( DbUtil::I_DebugRenderPtr debugRenderPtr, const Maths::RGBA& colour );

	//Create a grid of points in the plane of the polygon, clipped to the edges of the polygon. Optionally align to a specific
	//vector in the plane of the polygon for the U direction, default is the longest edge. V = UxN 
	bool					CreatePointGridOnPoly(C_VertexList& points, F32 gridResolution = 1.0f, Maths::HVec3* pAlignGrid = 0, F32 margin = 0.01f);

	void					CreateEdgePlanes(C_PlaneList& planes); 

	bool					IsPointWithinEdgePlanes(const Maths::HVec3& point, const C_PlaneList& planes, F32 margin = 0.01f); 

	/// Reserve storage for a specified number of polygons.
	void					Reserve(U32 iSize);

	/// Return true if there is a point in the polygon.
	const bool				ContainsPoint(const Maths::HVec3& rkPoint, const F32 fTolerance);

public:   

    Maths::C_Plane			m_Plane;

	Maths::HVec3			m_Centroid;

	C_VertexList			m_VertexList;

};


} // namespace SimplePoly

#endif
