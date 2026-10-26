#ifndef __CONVEX_HULL_H__
#define __CONVEX_HULL_H__

#ifdef _MSC_VER
#pragma once
#endif

#include <core/core.h>
#include <system/DebugLog.h>
#include <core/containers/genesis_containers/List.h>
#include <core/containers/genesis_containers/Array.h>

namespace ConvexHull
{

enum E_BuildState
{
	E_BuildState_NotBegun = 0,
	E_BuildState_WeldVertices,
	E_BuildState_FindTetrahedron,	
	E_BuildState_FindPolyAndVertex,	
	E_BuildState_RemoveSelectedPoly,
	E_BuildState_CreateNewPolygons,
	E_BuildState_MergeNeighbourPolygons,
	E_BuildState_Enumerate,
	E_BuildState_Finished,
	E_BuildState_Error
};

class C_Polygon;
class C_Edge;
class C_Vertex;
class C_Mesh;

typedef Util::C_PtrItem<C_Polygon> C_PolygonItem;
typedef Util::C_PtrItem<C_Vertex> C_VertexItem;
typedef Util::C_PtrItem<C_Edge> C_EdgeItem;
		
typedef Maths::HVec3 C_SourceVertex;

class C_Polygon : public Util::C_Item<C_Polygon>
{
public:
	C_Polygon(C_Mesh* pMesh);
	~C_Polygon();

	void									AddVertex(C_Vertex* pNewVertex);
	void									EndConstruction();

	const Maths::HVec3						GetCentroid();

	S32										GetEdgeWindingOrder(C_Edge* pEdge);

	S32										MergeCoplanarNeighbours();
	C_Edge*									FindEdge( C_Vertex* pA, C_Vertex* pB );

	bool									IsCoplanar(C_Polygon* pOther) const;

public:
	Maths::C_Plane							m_Plane; // normal points out of face

	C_Mesh* 								m_pMesh;
	S32										m_Building;

	Util::C_List<C_VertexItem>				m_VertexItemList;
	Util::C_List<C_EdgeItem>				m_EdgeItemList;

	Ut::Vector<C_SourceVertex>				m_SourceVertexList;

	S32										m_Index;
};

class C_Edge : public Util::C_Item<C_Edge>
{
public:
	C_Edge(C_Mesh* pMesh, C_Vertex* pA, C_Vertex* pB);
	~C_Edge();

	void									RemovePolygonRef(C_Polygon* pPolygon);
	const S32								GetVertexIndex(C_Vertex* pVertex); // returns whether this is vertex 0, 1 or -1 of it is not in this edge
	const S32								GetPolygonIndex(C_Polygon* pPolygon); // returns the position in the edge's polygon list

	Maths::HVec3							GetMidPoint();

public:
	C_Mesh* 								m_pMesh;

	Util::C_Array<C_Vertex*,2>				m_pVertices;
	Util::C_List<C_PolygonItem>				m_PolygonItemList;

	S32										m_Index;
};

class C_Vertex : public Util::C_Item<C_Vertex>
{
public:
	C_Vertex(C_Mesh* pMesh, const Maths::HVec3& pos);
	~C_Vertex();

	void									RemovePolygonRef(C_Polygon* pPolygon);
	void									RemoveEdgeRef(C_Edge* pEdge);

public:

	Maths::HVec3							m_Pos;

	C_Mesh* 								m_pMesh;
	Util::C_List<C_PolygonItem>				m_PolygonItemList;
	Util::C_List<C_EdgeItem>				m_EdgeItemList;

	S32										m_Index;
};


class C_Tetrahedron
{
public:
	Maths::HVec3 m_A;
	Maths::HVec3 m_B;
	Maths::HVec3 m_C;
	Maths::HVec3 m_D;
};

class C_Mesh
{
	friend class C_Vertex;
	friend class C_Edge;
	friend class C_Polygon;

public:	
	C_Mesh();
	~C_Mesh();

	void									SetName( const Util::C_String& name );

	C_Vertex*								NewVertex(const Maths::HVec3& pos);
	C_Polygon*								NewPolygon();

	void									DeletePolygon(C_Polygon* pPolygon);

	void									DebugRender( DbUtil::I_DebugRenderPtr debugRenderPtr, Maths::HMat43& m );

	void									DebugRenderMesh( DbUtil::I_DebugRenderPtr debugRenderPtr, Maths::HMat43& m );
	void									DebugRenderEdges( DbUtil::I_DebugRenderPtr debugRenderPtr, Maths::HMat43& m );
	void									DebugRenderDataStructure( DbUtil::I_DebugRenderPtr debugRenderPtr, Maths::HMat43& m );
	void									DebugRenderSourceVertices( DbUtil::I_DebugRenderPtr debugRenderPtr, Maths::HMat43& m );
	
	void									AddSourceVertexArray(C_SourceVertex* posList, S32 posCount);
	void									Build();

	bool									StepBuildingConvexHull();

	C_Edge* 								FindEdge(C_Vertex* pA, C_Vertex* pB);

	bool									Closed();


private:		
	C_Edge*									AddEdge(C_Vertex* pA, C_Vertex* pB);

	void									BuildWeldVertices();
	void									BuildFindMaximalTetrahedron();
	void 									BuildFindPolyAndVertex();
	void									BuildRemoveSelectedPoly();
	void									BuildCreateNewPolygons();
	void									BuildMergeNeighbourPolygons();
	void									BuildEnumerate();

	const bool								FindMaximalTetrahedronAB(C_Tetrahedron& tetra);
	const bool								FindMaximalTetrahedronC(C_Tetrahedron& tetra);
	const bool								FindMaximalTetrahedronD(C_Tetrahedron& tetra);
	
	void 									ClassifySourceVertices();

public:	
	Util::C_String							m_Name;

	Util::C_List<C_Polygon>					m_PolygonList;
	Util::C_List<C_Edge>					m_EdgeList;
	Util::C_List<C_Vertex>					m_VertexList;

	Ut::Vector<C_SourceVertex>				m_SourceVertexList;

	E_BuildState							m_BuildState;

	C_Polygon*								m_pSelectedPolygon;
	C_SourceVertex							m_SelectedVertex;

	S32										m_StepNo;
};


}; // namespace ConvexHull


#endif // header guard
