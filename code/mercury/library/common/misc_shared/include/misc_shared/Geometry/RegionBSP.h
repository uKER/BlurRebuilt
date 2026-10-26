#ifndef REGIONBSP_H
#define REGIONBSP_H

#ifdef _MSC_VER
#pragma once
#endif

#include <core/core.h>
#include <core/Maths.h>
#include <system/DebugLog.h>
#include <system/debug/I_DebugRender.h>
#include <system/Serialise.h>
#include <misc_shared/geometry/simplePoly.h>
#include <misc_shared/geometry/simplePoly.inl>

namespace RegionBSP
{
extern Debug::Logger &g_Log;

typedef S32 C_RegionId;

class C_SplitPlane;

//// C_ClipPolygon
//
//class C_ClipPolygon
//{
//public:    
//	C_ClipPolygon(){};
//	~C_ClipPolygon(){};
//    
//    void                    operator =( const C_ClipPolygon& other );    
//    
//    void                    NewVertex( const Maths::HVec3& pos );
//    void                    CalculatePlane();
//    const Maths::C_Plane    Plane() const;
//
//	void                    Clip( const Maths::C_Plane& plane, const F32 epsilon, C_ClipPolygon& front, C_ClipPolygon& back );
//
//	void					DebugRender( DbUtil::I_DebugRenderPtr debugRenderPtr, const Maths::RGBA& colour );
//
//public:   
//    Maths::C_Plane				m_Plane;
//#ifdef PLATFORM_PC
//	Ut::UtVector<Ut::DynamicVector<Maths::HVec3, Ut::ForceAlignedAllocator<Maths::HVec3, 16> > > m_VertexList;
//#else
//	Ut::Vector<Maths::HVec3>	m_VertexList;
//#endif
//};


//Mesh constructed from Planes - can be used to view BSP convex volumes

class C_Mesh
{
public:
	C_Mesh(){}
	~C_Mesh(){}

	void AddPlane(Maths::C_Plane& plane);
	void Build();

	Ut::Vector<Maths::C_Plane> m_planes;	//input planes, used to clip polys
	Ut::Vector<SimplePoly::C_Polygon> m_polys;		//output polys

	void DebugRender( DbUtil::I_DebugRenderPtr debugRenderPtr, const Maths::RGBA& colour );
};

// class C_Polyhedron

class C_Polyhedron
{
public:
	typedef Ut::Vector<S32> C_PolyhedronPolygon;
	
	C_Polyhedron();
	~C_Polyhedron();

	S32	AddVertex(const Maths::HVec3& pos);
	C_PolyhedronPolygon& AddPolygon();
	void AddVertexToPolygon(C_PolyhedronPolygon& polygon, const S32 vertexId);

	const bool IsClosed();

	void DebugRender( const Maths::HMat43& mat, DbUtil::I_DebugRenderPtr debugRenderPtr, const Maths::RGBA& colour );

public:
	Ut::Vector<C_PolyhedronPolygon>		m_PolygonList;
	Ut::Vector<Maths::HVec3>			m_VertexList;
};



// C_BuildTree
// The main interface to the region BSP tree builder

class C_BuildRegion;
class C_BuildSplitPlane;

class C_BuildTree
{
friend class C_Tree;
friend class C_BuildRegion;

public:
	C_BuildTree();
	~C_BuildTree();

	void								Initialise( const C_RegionId regionId );
	void								AddPolyhedron( RegionBSP::C_Polyhedron& polyhedron, const C_RegionId regionId );

private:
	C_BuildSplitPlane*					NewSplitPlane();

private:
	C_BuildRegion*						m_pRootRegion;
	Ut::Vector<C_BuildSplitPlane*>		m_pSplitPlanes;
};

class C_RaytraceParameters
{
public:
	C_RaytraceParameters();
	C_RaytraceParameters(	const Maths::C_Vertex&  worldStart,
							const Maths::C_Vertex&  worldEnd,
							const Maths::C_Vector&  worldVector,
							const Maths::C_Normal&  worldNormal,
							const Maths::HF32&      length,
							const C_RegionId		solidRegion);
public:
	Maths::C_Vertex			m_WorldStart;
	Maths::C_Vertex			m_WorldEnd;
	Maths::C_Vector			m_WorldVector;
	Maths::C_Normal			m_WorldNormal;
	Maths::HF32				m_Length;
	C_RegionId				m_SolidRegion;
};

class C_RaytraceResults
{
public:
	C_RaytraceResults();
public:
	bool				m_Hit;
	Maths::C_Normal		m_WorldNormal;
	Maths::C_Vertex		m_WorldPos;
    Maths::HF32			m_Distance;
	C_RegionId			m_FinalRegion;
};

class C_Ray
{
public:
	    // Default Constructor
    C_Ray();
    
    // Constructor: Vertex start & end 
    C_Ray( const Maths::C_Vertex& rayWorldStart, const Maths::C_Vertex& rayWorldEnd, const C_RegionId solidRegion );
    
    // Constructor: Vertex start & Vector to end
    C_Ray( const Maths::C_Vertex& rayWorldStart, const Maths::C_Vector& rayWorldVector, const C_RegionId solidRegion );
    
    // Constructor: Vertex start, Normal & Length to end
    C_Ray( const Maths::C_Vertex& rayWorldStart, const Maths::C_Normal& rayWorldNormal, const Maths::HF32& rayLength, const C_RegionId solidRegion );
    
    // Destructor
    ~C_Ray();

public:
	C_RaytraceParameters	m_Parameters;
	C_RaytraceResults		m_Results;
};


// BSP tree defined by a root region optionally containing split planes which subdivide space into two sub-regions
// leaf regions represent a convex region of space
class C_Region
{
friend class C_Tree;

public:
	C_RegionId				GetRegionId();
	C_SplitPlane*			GetSplitPlane();
private:
	C_RegionId				m_RegionId;
	C_SplitPlane*			m_pSplitPlane;
};

// C_SplitPlane

class C_SplitPlane
{
friend class C_Tree;

public:
	Maths::C_Plane			GetPlane();
	C_Region&				GetFront();
	C_Region&				GetBack();
private:
    Maths::C_Plane			m_Plane;
	C_Region				m_Front;
	C_Region				m_Back;
};

// C_Tree
// The main interface to the region BSP tree

class C_Tree
{
public:
	C_Tree();
	~C_Tree();

	const C_RegionId ClassifyPoint( const Maths::HVec3& pos );
	void ClassifyPolygon( SimplePoly::C_Polygon& polygon, Ut::Vector<C_RegionId>& regions);

	void Build( C_BuildTree& buildTree );

	template <typename Archive>
	void Serialise( Archive& ar );

	template <typename Archive>
	void Deserialise( Archive& ar );

	void Raytrace( C_Ray& ray );

	void BuildDebugMesh( const C_RegionId regionId, Ut::Vector<SimplePoly::C_Polygon>& resultPolys );

	/// Return the plane count.
	const U32 GetSplitPlaneCount() const
	{
		return static_cast<U32>(m_SplitPlaneList.Size());
	}

	/// In-order traveral of the tree.
	template <typename _Type>
	void Traverse(_Type& rkFunctor)
	{
		Traverse(&m_RootRegion, rkFunctor);
		return;
	}

	/// Return a pointer to the vector of split planes.
	C_SplitPlane* GetPlanes()
	{
		return &m_SplitPlaneList[0];
	}

	/// Return the index for a pointer to a split plane.
	const S32 GetSplitPlaneIndex( C_SplitPlane* pSplitPlane ) const;

	/// Return a pointer to the root region.
	C_Region* GetRootRegion()
	{
		return &m_RootRegion;
	}
	
	
private:
	const C_RegionId ClassifyPoint( const C_Region& region, const Maths::HVec3& pos );
	void ClassifyPolygon( C_Region& region, SimplePoly::C_Polygon& polygon, Ut::Vector<C_RegionId>& regions);

	void Raytrace( C_Ray& ray, C_Region& region, const Maths::HVec3& startPos, const Maths::HVec3& endPos, const Maths::C_Normal& hitNormal );

	void BuildDebugMesh( const C_RegionId regionId, C_Region& region, Ut::Vector<Maths::C_Plane>& planeList, Ut::Vector<SimplePoly::C_Polygon>& resultPolys );

	/// In-order traveral of the tree.
	template <typename _Type>
	void Traverse(C_Region* pCurrentRegion, _Type& rkFunctor)
	{
		if(!pCurrentRegion->GetSplitPlane())
		{
			rkFunctor(*pCurrentRegion);
			return;
		}

		Traverse(&pCurrentRegion->GetSplitPlane()->GetFront(), rkFunctor);
		rkFunctor(*pCurrentRegion);
		Traverse(&pCurrentRegion->GetSplitPlane()->GetBack(), rkFunctor);
		return;
	}

private:
	C_Region							m_RootRegion;
	Ut::Vector<C_SplitPlane>			m_SplitPlaneList;
};


} // namespace RegionBSP

#endif
