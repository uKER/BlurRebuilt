#pragma once

#include "AFTK\aftk.h"
#include "AFTK\AftkBuildAdjacency.h"
#include <system/Serialise.h>
#include <Btl/Containers/list.h>

namespace NavMesh
{

enum E_FileVersion
{
    E_FileVersion_Initial = 0,
	E_FileVersion_MutipleMeshes,
	E_FileVersion_Sectioned,
	E_FileVersion_Stencils,
	E_FileVersion_StencilsEdged,
    E_FileVersion_Current = E_FileVersion_StencilsEdged
};


// TODO - tie this in with game data somehow
enum E_AICellType
{
	E_AICellType_NotDefined = 0xffff,

	E_AICellType_Generic = 0,
	E_AICellType_Water,
	E_AICellType_Vault,
	E_AICellType_Drop,
	E_AICellType_Landing,
	E_AICellType_Doorway,
	E_AICellType_Blocked,

	E_AICellType_Count
};

class ExportCell;

//////////////////////////
// ExportVertex
//////////////////////////


class ExportVertex
{
public:
	ExportVertex(const Maths::C_Vertex& in)				: m_Position(in)	{ m_OuterPoint = 0; }
	ExportVertex(const F32 x, const F32 y, const F32 z)	: m_Position(x,y,z) { }

	template <typename Archive>
	void Serialise(Archive& ar) const
	{
		ar << m_Position;
		ar << m_OuterPoint;
	}

public:
	Maths::C_Vertex		m_Position;
	U8					m_OuterPoint;
};

struct ExportVertexLink
{
	ExportVertex* m_pLink;
};

//////////////////////////
// PreprocTri
//////////////////////////
class PreprocTri
{
public:
	PreprocTri(const U32 idx) : m_AftkPolygonSourceIdx(idx), m_Stored(false) {}
	const bool							Validate(Aftk::C_SceneMeshNode *pAftkSceneMeshNode);
	void								WarnInvalid();
	
	U32									m_VertexIndex[3];
	U32									m_TextureCoordIndex[3];
	Maths::C_Vertex						m_Position[3];
	Maths::C_Vector						m_Normal;
	U32									m_AftkPolygonSourceIdx;
	U16									m_TextureType;
	U16									m_Stored;
};

//////////////////////////
// PreprocEdge
//////////////////////////
class PreprocEdge
{
public:
	PreprocEdge(const U32 index0, const Maths::C_Vertex &position, const bool diagonal, const Maths::C_Vector &surfaceNormal) :
		m_Diagonal(diagonal),
		m_SurfaceNormal(surfaceNormal),
		m_Split(false)
	{
		m_VertexIndex[0] = index0;
		m_VertexPosition[0] = position;
	}

	PreprocEdge(const PreprocEdge &inEdge) :
		m_Diagonal(inEdge.m_Diagonal),
		m_SurfaceNormal(inEdge.m_SurfaceNormal),
		m_Split(inEdge.m_Split)
	{
		m_VertexIndex[0] = inEdge.m_VertexIndex[0];
		m_VertexPosition[0] = inEdge.m_VertexPosition[0];		
	}

	PreprocEdge(const U32 index0, const U32 index1, const bool diagonal, const U16 textureType, const Maths::C_Vector &surfaceNormal, Aftk::C_SceneMeshNode *pAftkSceneMeshNode) : 
		m_Diagonal(diagonal), 
		m_Split(false),
		m_TextureType(textureType),
		m_SurfaceNormal(surfaceNormal)
	{
		m_VertexIndex[0] = index0;
		m_VertexIndex[1] = index1;

		for(U32 i = 0; i < 2; i++)
		{
			Aftk::Position3 &aftkPos	= pAftkSceneMeshNode->GetVertexPosition(m_VertexIndex[i]);
			m_VertexPosition[i]	= Maths::C_Vertex(aftkPos.X(), aftkPos.Y(), aftkPos.Z());
		}

		m_Normal = (m_VertexPosition[1] - m_VertexPosition[0]).Normalised().Normal() ^ m_SurfaceNormal;
	}
						
public:					
	U32						m_VertexIndex[2];
	Maths::C_Vertex			m_VertexPosition[2];
	bool					m_Diagonal;				// True if the edge is adjacent to another edge
	bool					m_Split;				// True if the diagonal is active is the current pass
	bool					m_DoWeld;
	U16						m_TextureType;			// Parents texture type
	Maths::C_Vector			m_Normal;				// Normal from parent triangle (inverted to point inwards)
	Maths::C_Vector			m_SurfaceNormal;		// Parent triangle's surface normal
	PreprocEdge *			m_pPrevEdge;
	PreprocEdge *			m_pNextEdge;
};

//////////////////////////
// PreprocAdjacentTriangle : For storing triangles and pointers to its neighbours
//////////////////////////
class PreprocAdjacentTriangle
{
public:
	PreprocAdjacentTriangle(PreprocTri *pIn, Aftk::C_BuildAdjacency *pAdjacencyList, Btl::List<PreprocTri*> *pInTriList) : 
		m_pTriangle(pIn), 
		m_pAdjacencyList(pAdjacencyList), 
		m_pTriList(pInTriList)
	{
		for(U32 i = 0; i < 3; i++)
			m_pAdjacent[i] = NULL;
	}

	~PreprocAdjacentTriangle()
	{
		for(U32 i = 0; i < 3; i++)
			if(m_pAdjacent[i])
				delete m_pAdjacent[i];

	}
	void						StoreEdgesRecursive(Btl::List<PreprocEdge*> &edgeList, Aftk::C_SceneMeshNode *pAftkSceneMeshNode, S32& recursionDepth, Btl::List<PreprocAdjacentTriangle*>& workList);
	void						FindRecursive(Btl::List<PreprocAdjacentTriangle*> &list);
	
	PreprocTri*					m_pTriangle;
	PreprocAdjacentTriangle*	m_pAdjacent[3];

private:
	Aftk::C_BuildAdjacency*		m_pAdjacencyList;
	Btl::List<PreprocTri*>*		m_pTriList;
};


class PreprocPolygonBevel;
class PreprocConvexPolygon;

//////////////////////////
// PreprocPolygon : For splitting merging adjacent triangles into a single polygon, plus for splitting into convex edge lists
//////////////////////////
class PreprocPolygon
{
public:
	PreprocPolygon(Aftk::C_SceneMeshNode *pAftkSceneMeshNode, Btl::List<PreprocAdjacentTriangle*> &adjacentList);
	~PreprocPolygon();
	void						SplitToConvex();
	void						ConvexShrink(const F32 amount);
	void						ExportToCells(Btl::List<ExportCell*> &cellList, const U32 aftkSourceIndex);
								
private:						
	Btl::List<PreprocEdge*>::Iterator	FindSharedEdge(PreprocEdge *pItem);
	const S32 					AreaSign(const Aftk::Position3 &a, const Aftk::Position3 &b, const Aftk::Position3 &c);
	const U32					RemoveUnnecessaryDiagonals();
	const U32					WeldAligned();
	const bool					IsConvex(const U16 idx0, const U16 idx1, const U16 idx2);
	ExportCell*					BuildCell(const U32 aftkSourceIndex);

private:
	Aftk::C_SceneMeshNode*		m_pAftkSceneMeshNode;
	Btl::List<PreprocEdge*>		m_EdgeList;
};



//////////////////////////
// ExportCellEdge
//////////////////////////

struct C_CrossLink
{
	U32 m_SectionID;
	U32 m_CellID;
};

class ExportCellEdge
{
public:
	ExportCellEdge() :
		m_VertexIndex(0xffffffff),
		m_Normal(0.0f, 1.0f, 0.0f),
		m_NextCellSection(0),
		m_NextCellIndex(0xffff),
		m_NextVertIndex(0xff),
		m_pNextCell(NULL)
	{
	}

	template <typename Archive>
	void Serialise(Archive& ar) const
	{
		ar << m_VertexIndex;
		ar << m_NextCellSection;
		ar << m_NextCellIndex;
		ar << m_NextVertIndex;
		ar << m_Normal;
	}

public:
	U32				m_VertexIndex;
	S8				m_NextCellSection;
	U16				m_NextCellIndex;
	U8				m_NextVertIndex;
	Maths::C_Vector	m_Normal;

	ExportCell*		m_pNextCell;

	// Debug
	Maths::C_Vertex	m_Middle;
};

enum E_CellSegmentationStage
{
	E_CellSegmentationStage_Initial = 0,
	E_CellSegmentationStage_Added,
	E_CellSegmentationStage_Segmented
};

//////////////////////////
// ExportCell
//////////////////////////


class C_ExportCellItem
{
public:
	C_ExportCellItem(ExportCell* pCell)	{ m_pCell = pCell; }
	~C_ExportCellItem() {};
public:
	ExportCell* m_pCell;
};

class ExportCell
{
public:
	ExportCell(U32 aftkSourceIndex, U32 capacity) : 
		m_aftkPolygonSourceIdx(aftkSourceIndex),
		m_EdgeCount(capacity),
		m_pEdge(new ExportCellEdge[capacity]),
		m_Type(0),
		m_Index(0xffff),
		m_Middle(0, 0, 0),
		m_SectionID(-1),
		m_Segmented(E_CellSegmentationStage_Initial)
	{
	}

	~ExportCell()
	{
		delete [] m_pEdge;
	}

	bool				Valid(Aftk::C_SceneMeshNode *pAftkSceneMeshNode);
	void				LinkAdjacentCells(Btl::List<ExportCell*> &cellList);
	
	template <typename Archive>
	void Serialise(Archive& ar) const
	{
		ar << m_EdgeCount;
		ar << m_Type;
		ar << m_Middle;

		ExportCellEdge *p(m_pEdge);
		for(U32 i = 0; i < m_EdgeCount; i++, p++)
		{
			ar << *p;
		}

		ar << m_SectionID;

	}

public:
	ExportCellEdge*				m_pEdge;
	U32							m_aftkPolygonSourceIdx;
	U16							m_EdgeCount;
	U16							m_Type;
	U16							m_Index;
	S8							m_SectionID;
	Maths::C_Vertex				m_Middle;
	
	E_CellSegmentationStage		m_Segmented;
};

struct C_SectionCellCount
{
	U32	m_Count;
};

//////////////////////////
// NavMeshNode : Export the entire NavMesh (road and generic cells)
//////////////////////////
class NavMeshNode
{
public:
	NavMeshNode( Aftk::C_SceneMeshNode *pAftkSceneMeshNode);
	~NavMeshNode();

	template <typename Archive>
	void Serialise(Archive& ar) const
	{
		if(m_CellList.Size() == 0)
			LOGWARN(Tools::g_log,  "No cells in NavMesh\n");

		// Write out names
		ar << m_LayerName;
		ar << m_Name;

		// Write out cells
		ar << m_CellList.Size();
		for(Btl::List<ExportCell*>::ConstIterator it = m_CellList.Begin(); it != m_CellList.End(); ++it)
			ar << *(*it);

		// Write out vertex positions
		ar << m_VertexList.Size();
		for(Btl::List<ExportVertex*>::ConstIterator it = m_VertexList.Begin(); it != m_VertexList.End(); ++it)
			ar << *(*it);
		
		// Write out segment counts
		ar << m_SectionCellCount.Size();
		for(Btl::List<C_SectionCellCount*>::ConstIterator it = m_SectionCellCount.Begin(); it != m_SectionCellCount.End(); ++it)
			ar << (*it)->m_Count;
	}

	Util::C_String32			LayerName()	const	{ return m_LayerName; }

private:						
	void						CollectTriangles();
	void						BuildGenericCellList();
	void						LinkCells();
	void						CalcCellCentresAndOuterEdges();
	void						CheckTriangleList(Btl::List<PreprocTri*> &myList);
	void						RemapVertices();
	void						TextureCoordsShiftToZero(Aftk::TextureCoordinate4 *pTextureCoordinate, const U32 count);

	void						Segment();
	const U16					TypeFromMaterial(Aftk::C_Polygon* pPolygon);


public:
	Btl::List<ExportVertex*>	m_VertexList;
	Btl::List<ExportCell*>		m_CellList;

private:
	Util::C_String32			m_Name;
	Util::C_String32			m_LayerName;
	Aftk::C_SceneMeshNode*		m_pAftkSceneMeshNode;
	Aftk::C_BuildAdjacency		m_AdjacencyList;
	Btl::List<PreprocTri*>		m_GenericTriList;

	Btl::List<C_SectionCellCount*>	m_SectionCellCount;
};

} // namespace NavMesh
