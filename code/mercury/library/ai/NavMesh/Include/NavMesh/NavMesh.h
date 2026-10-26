#ifndef NAVMESH_H
#define NAVMESH_H

#ifdef _MSC_VER
#pragma once
#endif


#include <btl/Containers/list.h>
#include <System/serialise.h>
namespace DbUtil
{
	class I_DebugRender;
}

namespace AI
{

class C_NavMesh;
class C_NavMeshCell;
class C_NavMeshLink;
class C_NavMeshSection;

// ----------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------

void DebugAIToggleCellAtPoint(void*);


// --------------------------------------------------------------------------
//	- Mesh Import Classes -
// --------------------------------------------------------------------------

enum E_AIMeshLinkType
{
	E_AIMeshLinkType_Normal = 0,
	E_AIMeshLinkType_Barge,
	E_AIMeshLinkType_Drop
};

class C_ImportCellVertex
{
public:
	C_ImportCellVertex()	{}

public:
	Maths::HVec3	m_Position;
};

class C_ImportCellEdge
{
public:
	C_ImportCellEdge( ) :
	  m_NextCellIndex(0xffff),
		  m_NextVertIndex(0xff) 
	  {
	  }

	  template <typename Archive>
	  void Deserialise(Archive& ar)
	  {
		  ar >> m_VertexIndex; 
		  ar >> m_NextCellIndex;
		  ar >> m_NextVertIndex;
		  ar >> m_Normal;
	  }

	  const bool	CellLinked()			{ return (m_NextCellIndex != 0xffff); }

public:
	U32				m_VertexIndex;
	U16				m_NextCellIndex;
	U8				m_NextVertIndex;
	Maths::HVec3	m_Normal;
};

class C_ImportCell
{
public:
	C_ImportCell(): 
	  m_EdgeCount(0),
		  m_pEdge(NULL),
		  m_Type(0),
		  m_Centre(0, 0, 0),
		  m_SectionID(-1),
		  m_CellID(0)
	  {
	  }

	  ~C_ImportCell( )
	  {
		  delete [] m_pEdge;
	  }

	  void	Setup(U32 nEdgeCount)	{ m_EdgeCount = nEdgeCount; m_pEdge = new C_ImportCellEdge[m_EdgeCount]; }


	  template <typename Archive>
	  void Deserialise(Archive& ar)
	  {
		  ar >> m_Type;
		  ar >> m_Centre;

		  C_ImportCellEdge *p(m_pEdge);
		  for(U32 i = 0; i < m_EdgeCount; i++, p++)
		  {
			  ar >> *p;
		  }
	  }

public:
	C_ImportCellEdge*	m_pEdge;
	U16					m_EdgeCount;
	U16					m_Type;
	Maths::HVec3		m_Centre;
	S8					m_SectionID;
	U8					m_CellID;
};

// ----------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------

enum E_AICellType
{
	E_AICellType_Generic = 0,
	E_AICellType_Water,
	E_AICellType_Action,
	E_AICellType_Drop,
	E_AICellType_Landing,
	E_AICellType_Doorway,
	E_AICellType_Blocked
};

enum E_AIMesh_LinkAction
{
	E_AIMesh_LinkAction_Toggle = 0,
	E_AIMesh_LinkAction_Unlock,
	E_AIMesh_LinkAction_Lock
};

// ----------------------------------------------------------------------------------

class C_NavMeshEdge
{
public:

	C_NavMeshEdge() :
	  m_pNext(NULL),
		  m_pLeft(NULL),
		  m_pRight(NULL),
		  m_pBackCell(NULL),
		  m_pBackEdge(NULL)
	  {
	  }

	  ~C_NavMeshEdge()
	  {
		  delete m_pNext;
	  }


public:

	Maths::C_Vertex		m_Start;		///< Start Vertex of Edge
	Maths::C_Vertex		m_End;			///< End Vertex of Edge

	Maths::C_Vertex		m_Middle;		///< Halfway Vertex along the Edge

	Maths::C_Normal		m_Normal;		///< Inward Pointing Normal to the Edge
	Maths::C_Normal		m_Direction;	///< (m_End - m_Start).Normalised()

	C_NavMeshEdge*		m_pNext;

	C_NavMeshEdge*		m_pLeft;
	C_NavMeshEdge*		m_pRight;

	C_NavMeshEdge*		m_pBackEdge;	///< Link to adjoining Edge (if one exists)
	C_NavMeshCell*		m_pBackCell;	///< Link to adjoining Cell (if one exists)
};

// ----------------------------------------------------------------------------------

class C_NavMeshCell
{
public:

	C_NavMeshCell();
	~C_NavMeshCell();

	void				ModifyCellLink(const E_AIMesh_LinkAction eAction);

	bool				PointInside(const Maths::C_Vertex& vPosition)	const;
	F32					PointHeightFromCell(const Maths::C_Vertex& vPosition)	const;


public:

	C_NavMeshEdge*		m_pFirst;
	C_NavMeshLink*		m_pLink;

	Maths::C_Vertex		m_Middle;
	F32					m_Distance[4];

	U16					m_Type;
	S32					m_SectionID;


	enum E_Flags {
		E_Flag_Closed = 0,
		E_Flag_Actionable		
	};

	U32	m_Flags;

public:

#define DECL_AINAVCELL_FLAG_ACCESSORS(FLAG_NAME)\
	bool	Get ## FLAG_NAME () const		{ return Maths::BitToBool(m_Flags,	E_Flag_ ## FLAG_NAME); }\
	void	Set ## FLAG_NAME (bool value)	{ U32 mask = 1 << E_Flag_ ## FLAG_NAME; m_Flags = (m_Flags&~mask)|(mask&Maths::BoolToMaskU32(value)); }

	DECL_AINAVCELL_FLAG_ACCESSORS(Closed)
	DECL_AINAVCELL_FLAG_ACCESSORS(Actionable)
};

// ----------------------------------------------------------------------------------

class C_NavMeshSection
{
public:
	C_NavMeshSection()	: m_CellCount(0), m_Cells(NULL), m_SetupCount(0)	{}

	~C_NavMeshSection()	{ SAFE_DELETE_ARY(m_Cells);	}

	void				Setup(const U32 nCellCount)	{m_CellCount = nCellCount; m_Cells = new C_NavMeshCell[m_CellCount]; }

public:

	C_NavMeshCell*					m_Cells;
	U32								m_CellCount;	// Number of Cells in the Navigation Mesh

	U32								m_SetupCount;
};

// ----------------------------------------------------------------------------------

class C_NavMeshLink
{
public:

	C_NavMeshLink()	: m_pCell(NULL), m_pCellLanding(NULL)	{}
	C_NavMeshLink(C_NavMeshCell* pCellLink, const E_AIMeshLinkType eType = E_AIMeshLinkType_Normal) : m_pCell(pCellLink), m_Type(eType)
	{
		SetLinkToA(true);
		SetLinkToB(true);

		m_SectionA = m_pCell->m_SectionID;

		if(m_Type == E_AIMeshLinkType_Drop)
		{
			SetLinkToA(false);
		}
	}
	~C_NavMeshLink()	{}

	S32					GetLinkedID(const S32 nID)	const	{ if(nID == m_SectionA) return m_SectionB; return m_SectionA; }
	S32					GetInitialLinkedID()		const	{ return m_SectionB; }

public:

	U32						m_Index;

	E_AIMeshLinkType		m_Type;

	C_NavMeshCell*			m_pCell;
	const C_NavMeshCell*	m_pCellLanding;

	S32						m_SectionA;
	S32						m_SectionB;

	enum E_Flags {
		E_Flag_LinkToA = 0,
		E_Flag_LinkToB
	};

	U32	m_Flags;

public:

#define DECL_AINAVLINK_FLAG_ACCESSORS(FLAG_NAME)\
	bool	Get ## FLAG_NAME () const		{ return Maths::BitToBool(m_Flags,	E_Flag_ ## FLAG_NAME); }\
	void	Set ## FLAG_NAME (bool value)	{ U32 mask = 1 << E_Flag_ ## FLAG_NAME; m_Flags = (m_Flags&~mask)|(mask&Maths::BoolToMaskU32(value)); }

	DECL_AINAVLINK_FLAG_ACCESSORS(LinkToA)
	DECL_AINAVLINK_FLAG_ACCESSORS(LinkToB)
};


struct C_NavMeshLinkItem
{
	C_NavMeshLinkItem() : m_pLink(NULL)	{}

	C_NavMeshLink* m_pLink;
};


struct C_NavMeshLinkIndex
{
	C_NavMeshLinkIndex(const U32 nIndex) : m_Index(nIndex)	{}

	U32	m_Index;
};

struct C_NavMeshLinkSection
{
	~C_NavMeshLinkSection() {for (Btl::List<C_NavMeshLinkIndex*>::Iterator it = m_Links.Begin(); it != m_Links.End(); ++it) delete (*it);};


	U32								m_Index;
	Btl::List<C_NavMeshLinkIndex*>	m_Links;

	U32								m_Checked;
};

// ----------------------------------------------------------------------------------

class C_NavMesh
{
public:

	C_NavMesh() : m_SectionLinkage(NULL), m_Section(NULL), m_SectionCount(0), m_LinkCount(0), m_Links(NULL), m_SectionLinks(0), m_SectionLinkCheck(NULL)	{}
	~C_NavMesh();

	void					Reset();

	const void				LoadMesh(Sys::I_File& file);
	const bool				LoadMeshSingle(Sys::I_File& file);

	void					BuildMeshSection(C_NavMeshSection* pMesh, const U32 nSectionID, const U32 nCellCount, C_ImportCell* cellList, C_ImportCellVertex* vertexList);
	void					LinkCells(const U32 nCellCount, C_ImportCell* cellList, C_ImportCellVertex* vertexList, C_NavMeshCell* pCell);

	void					InitialSetup();
	void					SetupLinks();
	bool					ModifyCell(const C_NavMeshCell* pCell, const U32 eAction);
	bool					ModifyCellAt(const Maths::C_Vertex& vPosition, const U32 eAction = E_AIMesh_LinkAction_Toggle);
	bool					DoorCellSetup(const C_NavMeshCell* pCell, const U32 eType, const bool bOneWay, const Maths::C_Vector& vDirection);
	bool					DoorCellRemoveActionable(const Maths::C_Vertex& vPosition);
	bool					DoorCellClose(const Maths::C_Vertex& vPosition);

	bool					GetLink(const U32 nSectionA, const U32 nSectionB);
	bool					GetSectionLink(const U32 nSection, const U32 nSectionB);
	void					GetCellOnSection(const C_NavMeshCell*& pAINavCell, const Maths::C_Vertex& vPosition, C_NavMeshSection* pSection, F32& fBestHeight);
	void					GetCellAt(const C_NavMeshCell*& pAINavCell, const Maths::C_Vertex& vPosition);
	void					GetCellAt_BruteForce(const C_NavMeshCell*& pAINavCell, const Maths::C_Vertex& vPosition);
	const C_NavMeshCell*	GetLinkedCellAt(const Maths::C_Vertex& vPosition, const S32 nSectionID);
	const C_NavMeshCell*	GetDropCell(const Maths::C_Vertex& vPosition);

	bool					IntersectOutsideEdge(const C_NavMeshCell* pCellReturn, const Maths::C_Line &vLineZeroedY);
	bool					IntersectOutsideEdgeIgnoringAction(const C_NavMeshCell* pCellReturn, const Maths::C_Line &vLineZeroedY, const F32 fHeightEnd, const C_NavMeshCell* pAINavCellEnd = NULL);

	C_NavMeshEdge*			TraceLineSub(C_NavMeshEdge* pFirstEdge, C_NavMeshEdge* pEdge, const Maths::C_Line& vLineZeroedY, bool inside);
	const C_NavMeshCell* 	TraceLine(const C_NavMeshCell* pCell, C_NavMeshEdge* pFirstEdge, const Maths::C_Line& vLineZeroedY);
	bool					TraceCapsule(const C_NavMeshCell* pCellReturn, const Maths::C_Line& vLineZeroedY, float radius);

	void					RenderSection(DbUtil::I_DebugRender* pDebugRender, const C_NavMeshSection *pCellSection);
	void					DebugRender(DbUtil::I_DebugRender* pDebugRender);


public:

	C_NavMeshSection*		m_Section;			// List of Navigation Mesh Sections
	C_NavMeshLinkItem*		m_Links;			// List of Cell Links
	C_NavMeshLinkSection*	m_SectionLinks;		// Cross Linkage Table
	bool*					m_SectionLinkCheck;	// SectionsChecked

	bool*					m_SectionLinkage;	// Current Link Status

	U32						m_SectionCount;		// Total Section Count
	U32						m_LinkCount;		// Total Link Count
};

}
#endif
