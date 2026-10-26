
#include "stdafx.h"
#include <System/serialise/C_FileBlock.h>
#include <System/perf.h>
#include <System/debug/I_DebugRender.h>
#include <display.h>

namespace AI
{

static Debug::Logger &g_Logger = Debug::GetLogger("AI.NavMesh");

// ----------------------------------------------------------------------------------
//	- Versioning -
// ----------------------------------------------------------------------------------

enum E_NavMeshFileVersion
{
	E_NavMeshFileVersion_Initial = 0,
	E_NavMeshFileVersion_MutipleMeshes,
	E_NavMeshFileVersion_Sectioned,
	E_NavMeshFileVersion_Stencils,
	E_NavMeshFileVersion_StencilsEdged,
	E_NavMeshFileVersion_RemovedBaseMesh
};


// ----------------------------------------------------------------------------------
//	- Class - C_NavMeshCell -
// ----------------------------------------------------------------------------------

C_NavMeshCell::C_NavMeshCell() :	m_pFirst(NULL), m_pLink(NULL), m_Type(0), m_SectionID(0), m_Flags(0)
{
}

C_NavMeshCell::~C_NavMeshCell()
{
	SAFE_DELETE_PTR(m_pFirst);
	SAFE_DELETE_PTR(m_pLink);
}


void C_NavMeshCell::ModifyCellLink(const E_AIMesh_LinkAction eAction)
{
	if(eAction == E_AIMesh_LinkAction_Toggle)
	{
		SetClosed(!GetClosed());
	}
	else if(eAction == E_AIMesh_LinkAction_Unlock)
	{
		SetClosed(false);
	}
	else if(eAction == E_AIMesh_LinkAction_Lock)
	{
		SetClosed(true);
	}
}

bool C_NavMeshCell::PointInside(const Maths::C_Vertex& vPosition) const
{
	for(C_NavMeshEdge* pEdge = m_pFirst; pEdge; pEdge = pEdge->m_pNext)
	{
		Maths::HVec3 vDelta;

		vDelta.X(-(pEdge->m_Start.Z() - pEdge->m_End.Z()));
		vDelta.Z(pEdge->m_Start.X() - pEdge->m_End.X());

		if((vDelta.X() * (vPosition.X() - pEdge->m_Start.X())) + (vDelta.Z() * (vPosition.Z() - pEdge->m_Start.Z())) < 0.0f)
			return false;
	}

	return true;
}

F32 C_NavMeshCell::PointHeightFromCell(const Maths::C_Vertex& vPosition)	const
{
	if(m_pFirst)
		return Maths::C_Plane(m_pFirst->m_Start, m_pFirst->m_End, m_Middle).Distance(vPosition);

	return -1.0f;
}


// --------------------------------------------------------------------------
//	- Class: C_NavMesh -
// --------------------------------------------------------------------------

C_NavMesh::~C_NavMesh()
{
	SAFE_DELETE_ARY(m_SectionLinkage);
	SAFE_DELETE_ARY(m_Links);
	SAFE_DELETE_ARY(m_SectionLinks);
	SAFE_DELETE_ARY(m_SectionLinkCheck);
	SAFE_DELETE_ARY(m_Section);
}

void C_NavMesh::Reset()
{
	SAFE_DELETE_ARY(m_SectionLinkage);

	SetupLinks();
}

void C_NavMesh::BuildMeshSection(C_NavMeshSection* pMesh, const U32 nSectionID, const U32 nCellCount, C_ImportCell* cellList, C_ImportCellVertex* vertexList)
{
	// Build the list 
	for(U32 nCellCurr = 0; nCellCurr < nCellCount; nCellCurr++)
	{
		C_NavMeshEdge* pEdge = NULL;

		C_ImportCellEdge* pEdgeImported0( cellList[nCellCurr].m_pEdge );
		C_ImportCellEdge* pEdgeImported1( cellList[nCellCurr].m_pEdge );
		for(S32 nEdgeCurr = 0; nEdgeCurr < cellList[nCellCurr].m_EdgeCount; nEdgeCurr++, pEdgeImported0++)
		{
			// If Last Edge, Link with First
			if(nEdgeCurr < cellList[nCellCurr].m_EdgeCount-1)
				pEdgeImported1++;
			else
				pEdgeImported1 = cellList[nCellCurr].m_pEdge;

			// Cross link previous with next
			C_NavMeshEdge *pNextEdge = new C_NavMeshEdge();
			if(pEdge)
			{
				pEdge->m_pRight = pNextEdge;
				pEdge->m_pNext = pNextEdge;

				pNextEdge->m_pLeft = pEdge;
			}
			pEdge = pNextEdge;

			pEdge->m_Start		= vertexList[ pEdgeImported0->m_VertexIndex ].m_Position;
			pEdge->m_End		= vertexList[ pEdgeImported1->m_VertexIndex ].m_Position;
			pEdge->m_Middle	= (pEdge->m_Start + pEdge->m_End) * 0.5f;

			const Maths::C_Normalised nDirection((pEdge->m_End - pEdge->m_Start).Normalised());
			pEdge->m_Direction	= nDirection.Normal();
			pEdge->m_Normal		= pEdgeImported0->m_Normal;

			if(nEdgeCurr == 0)
			{
				pMesh->m_Cells[nCellCurr].m_pFirst = pEdge;
			}
		}

		// Cross link the last edge with the first to complete the circle
		if(pEdge)
		{
			pMesh->m_Cells[nCellCurr].m_pFirst->m_pLeft = pEdge;
			pEdge->m_pRight = pMesh->m_Cells[nCellCurr].m_pFirst;
			pEdge->m_pNext = NULL;
		}

		pMesh->m_Cells[nCellCurr].m_Type = cellList[nCellCurr].m_Type;
		pMesh->m_Cells[nCellCurr].m_Middle = cellList[nCellCurr].m_Centre;
		pMesh->m_Cells[nCellCurr].m_SectionID = nSectionID;

		if(pMesh->m_Cells[nCellCurr].m_Type == E_AICellType_Action)
		{
			pMesh->m_Cells[nCellCurr].m_Type = E_AICellType_Generic;
			pMesh->m_Cells[nCellCurr].SetActionable(true);
		}
		else if(pMesh->m_Cells[nCellCurr].m_Type == E_AICellType_Drop)
		{
			pMesh->m_Cells[nCellCurr].m_pLink = new C_NavMeshLink(&pMesh->m_Cells[nCellCurr], E_AIMeshLinkType_Drop);
			pMesh->m_Cells[nCellCurr].SetActionable(true);
			pMesh->m_Cells[nCellCurr].m_pLink->m_Index = m_LinkCount++;
		}
		else if(pMesh->m_Cells[nCellCurr].m_Type == E_AICellType_Doorway)
		{
			pMesh->m_Cells[nCellCurr].m_pLink = new C_NavMeshLink(&pMesh->m_Cells[nCellCurr]);
			pMesh->m_Cells[nCellCurr].m_pLink->m_Index = m_LinkCount++;
		}
		else if(pMesh->m_Cells[nCellCurr].m_Type == E_AICellType_Blocked)	// TODO: Remove this from Scene Converter
		{
			pMesh->m_Cells[nCellCurr].m_Type = E_AICellType_Generic;
		}
	}
}

void C_NavMesh::LinkCells(const U32 nCellCount, C_ImportCell* cellList, C_ImportCellVertex* vertexList, C_NavMeshCell* pCell)
{
	// Link together
	for(U32 nLinkCellCurr = 0; nLinkCellCurr < nCellCount; nLinkCellCurr++)
	{
		C_ImportCellEdge* pEdgeImported	= cellList[nLinkCellCurr].m_pEdge;
		C_NavMeshEdge* pEdge_0 = pCell[nLinkCellCurr].m_pFirst;

		for(U32 nLinkEdgeCurr = 0; nLinkEdgeCurr < cellList[nLinkCellCurr].m_EdgeCount; nLinkEdgeCurr++, pEdgeImported++)
		{
			if(pEdgeImported->CellLinked())
			{
				if(pEdgeImported->m_NextCellIndex < nCellCount)
				{
					C_NavMeshCell* pCell_1 = &pCell[pEdgeImported->m_NextCellIndex];
					C_NavMeshEdge* pEdge_1 = pCell_1->m_pFirst;

					for(U32 j = 0; j < pEdgeImported->m_NextVertIndex; j++)
					{
						if(pEdge_1)
						{
							pEdge_1 = pEdge_1->m_pNext;
						}
						else
						{
							j = pEdgeImported->m_NextVertIndex;
							LOGERROR(g_Logger, DBSTR "[NavMesh] Cell at X:%.2f Y:%.2f Z:%.2f has anomalies with its Edges.\n", (float)pCell_1->m_Middle.X(), (float)pCell_1->m_Middle.Y(), (float)pCell_1->m_Middle.Z());
						}				
					}

					if(pEdge_0 && pEdge_1)
					{
						pEdge_0->m_pBackCell = pCell_1;
						pEdge_0->m_pBackEdge = pEdge_1;
						pEdge_1->m_pBackCell = &pCell[nLinkCellCurr];
						pEdge_1->m_pBackEdge = pEdge_0;
					}
				}
				else
				{
					LOGERROR(g_Logger, DBSTR "[NavMesh] NextCellIndex out of bounds on Edge with point X:%.2f Y:%.2f Z:%.2f\n", (float)vertexList[ pEdgeImported->m_VertexIndex ].m_Position.X(), (float)vertexList[ pEdgeImported->m_VertexIndex ].m_Position.Y(), (float)vertexList[ pEdgeImported->m_VertexIndex ].m_Position.Z());
				}
			}

			if(pEdge_0)
			{
				pEdge_0 = pEdge_0->m_pNext;
			}
			else
			{
				nLinkEdgeCurr = cellList[nLinkCellCurr].m_EdgeCount;
				LOGERROR(g_Logger, DBSTR "[NavMesh] Cell at X:%.2f Y:%.2f Z:%.2f has anomalies with its Edges.\n", (float)pCell[nLinkCellCurr].m_Middle.X(), (float)pCell[nLinkCellCurr].m_Middle.Y(), (float)pCell[nLinkCellCurr].m_Middle.Z());
			}
		}

		// Check Actionable Drop Cell
		if(pCell[nLinkCellCurr].m_Type == E_AICellType_Drop)
		{
			C_NavMeshEdge* pEdgeCheckDropActionable = pCell[nLinkCellCurr].m_pFirst;
			while(pEdgeCheckDropActionable)
			{
				if(pEdgeCheckDropActionable->m_pBackCell && pEdgeCheckDropActionable->m_pBackCell->m_Type == E_AICellType_Generic)
				{
					pCell[nLinkCellCurr].SetActionable(true);
					pEdgeCheckDropActionable = NULL;
					continue;
				}

				pEdgeCheckDropActionable = pEdgeCheckDropActionable->m_pNext;
			}
		}
	}
}

Debug::Bool	g_AINavMeshDoNotLoad("AI.NavMesh.DoNotLoad", false);

const bool C_NavMesh::LoadMeshSingle(Sys::I_File& file)
{
	if(g_AINavMeshDoNotLoad)
	{
		return false;
	}

	Sys::BinaryDeserialiser<Ut::NoEndianSwap> ar( file );

	C_NavMeshSection* pSection = NULL;
	bool bBase = false;

	Util::C_String32 sLayerName;
	ar >> sLayerName;
	Util::C_String navMeshCheck(sLayerName);
	navMeshCheck = navMeshCheck(0, 9);
	if(navMeshCheck != "_NAVMESH_")
	{
		return false;
	}

	Util::C_String levelName = sLayerName(9, sLayerName.Length());
	if(levelName == "BASE")
	{
		bBase = true;

		pSection = &m_Section[m_SectionCount];
		m_SectionCount++;
	}
	else if(levelName == "ISLAND")
	{
		pSection = &m_Section[m_SectionCount];
		m_SectionCount++;
	}

	Util::C_String32 sName;
	ar >> sName;

	U32	nCellCount = 0;
	ar >> nCellCount;
	if(nCellCount == 0)
	{
		return false;
	}

	// Load the Cell List
	C_ImportCell* cellList = new C_ImportCell[nCellCount];
	for(U32 nCellCurr = 0; nCellCurr < nCellCount; nCellCurr++)
	{
		U16 nEdgeCount;
		ar >> nEdgeCount;

		cellList[nCellCurr].Setup(nEdgeCount);

		ar >> cellList[nCellCurr].m_Type;
		ar >> cellList[nCellCurr].m_Centre;

		for(U32 nEdgeCurr = 0; nEdgeCurr < nEdgeCount; nEdgeCurr++)
		{
			ar >> cellList[nCellCurr].m_pEdge[nEdgeCurr].m_VertexIndex; 
			S8 nDummy;
			ar >> nDummy;
			ar >> cellList[nCellCurr].m_pEdge[nEdgeCurr].m_NextCellIndex;
			ar >> cellList[nCellCurr].m_pEdge[nEdgeCurr].m_NextVertIndex;
			ar >> cellList[nCellCurr].m_pEdge[nEdgeCurr].m_Normal;
		}

		ar >> cellList[nCellCurr].m_SectionID;
	}


	// Load the Vertex List
	U32 nVertexCount = 0;
	ar >> nVertexCount;

	C_ImportCellVertex* vertexList = new C_ImportCellVertex[nVertexCount];
	for(U32 nVertexCurr = 0; nVertexCurr < nVertexCount; nVertexCurr++)
	{
		U8 nDummy;
		ar >> vertexList[nVertexCurr].m_Position;
		ar >> nDummy;
	}

	U32 nSectionCount = 0;
	ar >> nSectionCount;
	if(bBase)
	{
		for(U32 nSectionCurr = 0; nSectionCurr < nSectionCount; nSectionCurr++)
		{
			U32 nDummy;
			ar >> nDummy;
		}
	}

	if(pSection)
	{
		pSection->Setup(nCellCount);
		BuildMeshSection(pSection, m_SectionCount-1, nCellCount, cellList, vertexList);
		LinkCells(nCellCount, cellList, vertexList, pSection->m_Cells);
	}

	SAFE_DELETE_ARY(vertexList);
	SAFE_DELETE_ARY(cellList);

	return true;
}


const void C_NavMesh::LoadMesh(Sys::I_File& file )
{
	Sys::BinaryDeserialiser<Ut::NoEndianSwap> ar( file );

	S32	fileVersion;
	ar >> fileVersion;

	bool bHasMesh = false;
	U32 nDummy;
	ar >> nDummy;
	if(nDummy)
		bHasMesh = true;

	// Setup Nav Mesh Sections
	U32 nCount;
	ar >> nCount;
	if(nCount || bHasMesh)
	{
		if(bHasMesh)
			nCount++;

		bHasMesh = true;
		m_Section = new C_NavMeshSection[nCount];
	}

	// Setup Stencil sections
	ar >> nCount;
	if(nCount)
	{
		bHasMesh = true;
	}

	if(bHasMesh)
	{
		Sys::C_FileBlock rootBlock(&file);

		while(rootBlock.GetPosition() < rootBlock.Length())
		{
			Sys::C_FileBlock block(&rootBlock);

			if(block.HeaderId() ==  "Mesh")
			{
				if(!LoadMeshSingle(block))
				{
					block.Read(NULL, block.GetBlockHeader().m_Size - (U32)block.GetPosition());
				}
			}
			else
			{
				LOGWARN( g_Logger,  "Unknown header '%s'\n", block.HeaderId().Chars());
			}
		}

		InitialSetup();
	}

	return;
}


// Check Section Connectivity
void C_NavMesh::InitialSetup()
{
	m_Links = new C_NavMeshLinkItem[m_LinkCount];
	m_SectionLinks = new C_NavMeshLinkSection[m_SectionCount];
	m_SectionLinkCheck = new bool[m_SectionCount];
	for(U32 nSectionCurr = 0; nSectionCurr < m_SectionCount; nSectionCurr++)
	{
		m_SectionLinks[nSectionCurr].m_Index = nSectionCurr;
	}

	if(m_Section)
	{	
		U32 nLinkCurrID = 0;
		for(U32 nSectionCurr = 0; nSectionCurr < m_SectionCount; nSectionCurr++)
		{
			for(U32 nCellCurr = 0; nCellCurr < m_Section[nSectionCurr].m_CellCount; nCellCurr++)
			{
				C_NavMeshCell* pCurrCell = &m_Section[nSectionCurr].m_Cells[nCellCurr];
				if(pCurrCell && pCurrCell->m_pLink)
				{
					m_Links[nLinkCurrID].m_pLink = pCurrCell->m_pLink;
					C_NavMeshEdge* pEdge = pCurrCell->m_pFirst;
					pCurrCell->m_pLink->m_SectionB = pCurrCell->m_pLink->m_SectionA;
					while(pEdge)
					{
						if(pEdge->m_pBackCell && pEdge->m_pBackCell->m_SectionID != pCurrCell->m_pLink->m_SectionB)
						{
							pCurrCell->m_pLink->m_SectionB = pEdge->m_pBackCell->m_SectionID;
							pEdge = NULL;
						}
						else
						{
							pEdge = pEdge->m_pNext;
						}
					}

					if(pCurrCell->m_SectionID < (S32)m_SectionCount)
					{
						m_SectionLinks[pCurrCell->m_SectionID].m_Links.PushBack(new C_NavMeshLinkIndex(nLinkCurrID));
					}
					else
					{
						LOGERROR(g_Logger, DBSTR "Linked Cell with centre-point at X:%.2f Y:%.2f Z:%.2f has dodgy Section ID.\n", (float)pCurrCell->m_Middle.X(), (float)pCurrCell->m_Middle.Y(), (float)pCurrCell->m_Middle.Z());
					}

					if(pCurrCell->m_pLink->m_SectionB < (S32)m_SectionCount)
						m_SectionLinks[pCurrCell->m_pLink->m_SectionB].m_Links.PushBack(new C_NavMeshLinkIndex(nLinkCurrID));
					else
					{
						LOGERROR(g_Logger, DBSTR "Linked Cell with centre-point at X:%.2f Y:%.2f Z:%.2f has Section B ID larger than Linked Section array size.\n", (float)pCurrCell->m_Middle.X(), (float)pCurrCell->m_Middle.Y(), (float)pCurrCell->m_Middle.Z());
					}

					nLinkCurrID++;
				}
			}
		}
	}

	// TODO: Inital Setup of Doors/Barriers
	for(U32 nLinkCurr = 0; nLinkCurr < m_LinkCount; nLinkCurr++)
	{
		C_NavMeshLink* pLink = m_Links[nLinkCurr].m_pLink;
		if(pLink && pLink->m_Type == E_AIMeshLinkType_Drop)
		{
			const C_NavMeshCell* pAINavCell = GetDropCell(pLink->m_pCell->m_Middle - Maths::C_Vector(0.0f, 0.1f, 0.0f));			
			if(pAINavCell)
			{
				pLink->m_SectionB = pAINavCell->m_SectionID;
				pLink->m_pCellLanding = pAINavCell;
			}
			else
			{
				LOGERROR(g_Logger, DBSTR "Drop Cell with centre-point at X:%.2f Y:%.2f Z:%.2f is not over mesh.\n", (float)pLink->m_pCell->m_Middle.X(), (float)pLink->m_pCell->m_Middle.Y(), (float)pLink->m_pCell->m_Middle.Z());
			}
		}
	}

	SetupLinks();
}


void C_NavMesh::SetupLinks()
{
	m_SectionLinkage = new bool[m_SectionCount * m_SectionCount];
	for(U32 nSectionA = 0; nSectionA < m_SectionCount; nSectionA++)
	{
		for(U32 nSectionB = 0; nSectionB < m_SectionCount; nSectionB++)
		{
			m_SectionLinkage[(nSectionA * m_SectionCount) + nSectionB] = false;
		}
	}

	// TODO: Inital Setup of Doors/Barriers
	if(m_Links)
	{
		for(U32 nLinkCurr = 0; nLinkCurr < m_LinkCount; nLinkCurr++)
		{
			C_NavMeshLink* pLink = m_Links[nLinkCurr].m_pLink;
			if(pLink && pLink->m_Type == E_AIMeshLinkType_Drop)
			{
				m_SectionLinkage[(pLink->m_SectionA * m_SectionCount) + pLink->m_SectionB] = true;
			}
		}
	}
}


bool C_NavMesh::ModifyCell(const C_NavMeshCell* pCell, const U32 eAction)
{
	// TODO: Incoorporate new Cell Linking
	return false;
}

bool C_NavMesh::ModifyCellAt(const Maths::C_Vertex& vPosition, const U32 eAction)
{
	const C_NavMeshCell* pAINavCell = NULL;
	GetCellAt(pAINavCell, vPosition);
	if(pAINavCell)
	{
		return ModifyCell(pAINavCell, eAction);
	}

	return false;
}

bool C_NavMesh::DoorCellSetup(const C_NavMeshCell* pAINavCell, const U32 eType, const bool bOneWay, const Maths::C_Vector& vDirection)
{
	if(pAINavCell && pAINavCell->m_Type == E_AICellType_Doorway && pAINavCell->m_pLink)
	{
		const C_NavMeshCell* pCell = pAINavCell;
		pCell->m_pLink->m_Type = (AI::E_AIMeshLinkType)eType;
		if(bOneWay)
		{
			F32 fDotSectionA = -1.0f;
			F32 fDotSectionB = -1.0f;

			for(C_NavMeshEdge* pEdge = pCell->m_pFirst; pEdge; pEdge = pEdge->m_pNext)
			{
				if(pEdge->m_pBackCell)
				{
					if(pEdge->m_pBackCell->m_SectionID == pCell->m_pLink->m_SectionA)
					{
						fDotSectionA = vDirection | (pEdge->m_Middle - pCell->m_Middle);
					}
					else
					{
						fDotSectionB = vDirection | (pEdge->m_Middle - pCell->m_Middle);
					}
				}
			}

			if(fDotSectionA > fDotSectionB)
			{
				pCell->m_pLink->SetLinkToB(false);
			}
			else
			{
				pCell->m_pLink->SetLinkToA(false);
			}
		}

		if(eType == E_AIMeshLinkType_Barge)
		{
			(const_cast<C_NavMeshCell*>(pCell))->SetActionable(true);
		}

		return ModifyCell(pCell, E_AIMesh_LinkAction_Unlock);
	}

	return false;
}

bool C_NavMesh::DoorCellRemoveActionable(const Maths::C_Vertex& vPosition)
{
	bool bCellNotFound = true;
	for(U32 nLinkCurr = 0; nLinkCurr < m_LinkCount; nLinkCurr++)
	{
		C_NavMeshLink* pLink = m_Links[nLinkCurr].m_pLink;
		if(pLink && pLink->m_pCell && pLink->m_pCell->m_Type == E_AICellType_Doorway)
		{
			if(pLink->m_pCell->PointInside(vPosition))
			{
				if(pLink->m_Type == E_AIMeshLinkType_Barge)
				{
					pLink->m_Type = E_AIMeshLinkType_Normal;
					pLink->m_pCell->SetActionable(false);
				}

				bCellNotFound = false;
			}
		}
	}
	if(bCellNotFound)
	{
		LOGWARN(g_Logger, DBSTR "Door Cell not found under X:%.2f Y:%.2f Z:%.2f.\n", (float)vPosition.X(), (float)vPosition.Y(), (float)vPosition.Z());
	}

	return bCellNotFound;
}


bool C_NavMesh::DoorCellClose(const Maths::C_Vertex& vPosition)
{
	bool bCellNotFound = true;
	for(U32 nLinkCurr = 0; nLinkCurr < m_LinkCount; nLinkCurr++)
	{
		C_NavMeshLink* pLink = m_Links[nLinkCurr].m_pLink;
		if(pLink && pLink->m_pCell)
		{
			if(pLink->m_pCell->PointInside(vPosition))
			{
				pLink->m_pCell->SetClosed(true);
				bCellNotFound = false;
			}
		}
	}
	if(bCellNotFound)
	{
		LOGWARN(g_Logger, DBSTR "Door Cell not found under X:%.2f Y:%.2f Z:%.2f.\n", (float)vPosition.X(), (float)vPosition.Y(), (float)vPosition.Z());
	}

	return bCellNotFound;
}


// Check Section Connectivity
bool C_NavMesh::GetLink(const U32 nSectionA, const U32 nSectionB)
{
	if(nSectionA == nSectionB)
		return true;

	for(U32 nSectionCurr = 0; nSectionCurr < m_SectionCount; nSectionCurr++)
		m_SectionLinkCheck[nSectionCurr] = false;

	return GetSectionLink(nSectionA, nSectionB);
}

bool C_NavMesh::GetSectionLink(const U32 nSection, const U32 nSectionB)
{
	m_SectionLinkCheck[nSection] = true;

	if(m_SectionLinkage[(nSection * m_SectionCount) + nSectionB])
		return true;

	for(U32 nSectionLookup = 0; nSectionLookup < m_SectionCount; nSectionLookup++)
	{
		if(m_SectionLinkage[(nSection * m_SectionCount) + nSectionLookup] && m_SectionLinkCheck[nSectionLookup] == false)
		{
			if(GetSectionLink(nSectionLookup, nSectionB))
			{
				return true;
			}
		}
	}

	return false;
}

const C_NavMeshCell* C_NavMesh::GetDropCell(const Maths::C_Vertex& vPosition)
{
	const C_NavMeshCell* pAINavCell = NULL;
	F32	fBestHeight = FLT_MAX;
	for(U32 nSectionCurr = 0; nSectionCurr < m_SectionCount; nSectionCurr++)
	{
		for(U32 nCellCurr = 0; nCellCurr < m_Section[nSectionCurr].m_CellCount; nCellCurr++)
		{
			C_NavMeshCell* pCurrCell = &m_Section[nSectionCurr].m_Cells[nCellCurr];
			if(pCurrCell->PointInside(vPosition))
			{
				const F32 fHeight = pCurrCell->PointHeightFromCell(vPosition);
				if(fHeight > 0.0f && (!pAINavCell || (fHeight < fBestHeight)))
				{
					pAINavCell = pCurrCell;

					fBestHeight = fHeight;
				}
			}
		}
	}

	return pAINavCell;
}


void C_NavMesh::GetCellOnSection(const C_NavMeshCell*& pAINavCell, const Maths::C_Vertex& vPosition, C_NavMeshSection* pSection, F32& fBestHeight)
{
	if(pSection)
	{
		for(U32 nCellCurr = 0; nCellCurr < pSection->m_CellCount; nCellCurr++)
		{
			C_NavMeshCell* pCurrCell = &pSection->m_Cells[nCellCurr];
			if(pCurrCell->PointInside(vPosition) && pCurrCell->m_pFirst)
			{
				const F32 fHeight = pCurrCell->PointHeightFromCell(vPosition);
				// HACK: THE NAV MESH SHOULD BE LVL WITH OR SLIGHTLY BELOW THE COLLISION MESH - fHeight >= 0.0f
				if(fHeight >= -0.4f && (!pAINavCell || (fHeight < fBestHeight)))
				{
					pAINavCell = pCurrCell;
					fBestHeight = fHeight;
				}
			}
		}
	}
}

// Get Cell At Point
void C_NavMesh::GetCellAt_BruteForce(const C_NavMeshCell*& pAINavCell, const Maths::C_Vertex& vPosition)
{
	PERF_EVENT("Update AI GetCellAt");
	F32	fBestHeight = FLT_MAX;

	if(m_Section)
	{
		for(U32 nSectionCurr = 0; nSectionCurr < m_SectionCount; nSectionCurr++)
		{
			GetCellOnSection(pAINavCell, vPosition, &m_Section[nSectionCurr], fBestHeight);
		}
	}
}

void C_NavMesh::GetCellAt(const C_NavMeshCell*& pAINavCell, const Maths::C_Vertex& vPosition)
{
	const C_NavMeshCell* pCell = pAINavCell;
	if(!pCell) 
	{
		GetCellAt_BruteForce(pAINavCell, vPosition);
		return;
	}

	bool bAmInside=true;

	for(C_NavMeshEdge* pEdge = pCell->m_pFirst; pEdge; pEdge = pEdge->m_pNext)
	{
		Maths::HVec3 vDelta;

		vDelta.X(-(pEdge->m_Start.Z() - pEdge->m_End.Z()));
		vDelta.Z(pEdge->m_Start.X() - pEdge->m_End.X());

		if((vDelta.X() * (vPosition.X() - pEdge->m_Start.X())) + (vDelta.Z() * (vPosition.Z() - pEdge->m_Start.Z())) < 0.0f) 
		{
			//i'm on the wrong side of this *beep*ing edge to still be in this pCell.
			// TODO - path-clip *beep* to pCell-step multiply accurately.
			// first hack is simply check if i'm in the next one.
			// - please keep the *beep*ing comments clean :)
			C_NavMeshCell*	nextCell = pEdge->m_pBackCell;
			if (nextCell) 
			{
				if (nextCell->PointInside(vPosition))
				{
					pAINavCell = nextCell;
					return;	// we found the pCell so we're done.
				}
			}
			bAmInside=false;
		}
	}
	if (!bAmInside) 
	{
		C_NavMeshLink*	link = pCell->m_pLink;
		if (link && link->m_pCellLanding) 
		{
			if (link->m_pCellLanding->PointInside(vPosition))
			{
				pAINavCell = link->m_pCellLanding;
			}
		}

		pAINavCell = NULL;
		GetCellAt_BruteForce(pAINavCell, vPosition);
	}
}


const C_NavMeshCell* C_NavMesh::GetLinkedCellAt(const Maths::C_Vertex& vPosition, const S32 nSectionID)
{
	PERF_EVENT("Update AI GetLinkedCellAt");
	F32	fBestHeight = FLT_MAX;
	const C_NavMeshCell* pAINavCell = NULL;

	if(m_Section && nSectionID > -1 && nSectionID < (S32)m_SectionCount)
	{
		if(m_SectionLinkage)
		{
			for(U32 nSectionCurr = 0; nSectionCurr < m_SectionCount; nSectionCurr++)
			{
				if(GetLink(nSectionID, nSectionCurr))
				{
					GetCellOnSection(pAINavCell, vPosition, &m_Section[nSectionCurr], fBestHeight);
				}
			}
		}
	}

	return pAINavCell;
}


bool C_NavMesh::IntersectOutsideEdge(const C_NavMeshCell* pAINavCell, const Maths::C_Line &vLineZeroedY)
{
	if(pAINavCell)
	{
		// Check Mesh
		C_NavMeshEdge* pFirstEdge = pAINavCell->m_pFirst;
		C_NavMeshEdge* pEdge = pFirstEdge;

		do
		{
			const Maths::C_Line vEdge(pEdge->m_Start.ZeroedY(), pEdge->m_End.ZeroedY());
			const F32 fTA = vLineZeroedY.Intersect(vEdge);
			const F32 fTB = vEdge.Intersect(vLineZeroedY);
			if(fTA > 0.0f && fTA < 1.0f && fTB > 0.0f && fTB < 1.0f)
			{
				if(pEdge->m_pBackCell == NULL || pEdge->m_pBackCell->GetClosed() ||  pEdge->m_pBackCell->GetActionable())
					return true;

				pFirstEdge = pEdge->m_pBackEdge;
				pEdge = pFirstEdge;
			}

			pEdge = pEdge->m_pRight;
		}
		while(pEdge != pFirstEdge);
	}

	return false;
}

bool C_NavMesh::IntersectOutsideEdgeIgnoringAction(const C_NavMeshCell* pAINavCell, const Maths::C_Line &vLineZeroedY, const F32 fHeightEnd, const C_NavMeshCell* pAINavCellEnd)
{
	const C_NavMeshCell* pCellEnd = pAINavCell;
	if(pAINavCell)
	{
		// Check Mesh
		C_NavMeshEdge* pFirstEdge = pAINavCell->m_pFirst;
		C_NavMeshEdge* pEdge = pFirstEdge;

		do
		{
			const Maths::C_Line vEdge(pEdge->m_Start.ZeroedY(), pEdge->m_End.ZeroedY());
			const F32 fTA = vLineZeroedY.Intersect(vEdge);
			const F32 fTB = vEdge.Intersect(vLineZeroedY);
			if(fTA > 0.0f && fTA < 1.0f && fTB > 0.0f && fTB < 1.0f)
			{
				if(pEdge->m_pBackEdge == NULL || pEdge->m_pBackCell->GetClosed())
					return true;

				pCellEnd = pEdge->m_pBackCell;
				pFirstEdge = pEdge->m_pBackEdge;
				pEdge = pFirstEdge;
			}

			pEdge = pEdge->m_pRight;
		}
		while(pEdge != pFirstEdge);
	}

	if(pAINavCellEnd)
	{
		if(pAINavCellEnd != pCellEnd)
			return true;
	}
	else if(pCellEnd)
	{
		const F32 fHeight = pCellEnd->PointHeightFromCell(vLineZeroedY.End() + Maths::C_Vertex(0.0f, fHeightEnd, 0.0f));
		if(fHeight < -0.5f || fHeight > 2.5f)
			return true;
	}

	return false;
}


Debug::Bool g_AIRoutingCapsuleDebug("AI.NavMesh.Capsule.Debug", false);

C_NavMeshEdge* C_NavMesh::TraceLineSub(C_NavMeshEdge* pFirstEdge, C_NavMeshEdge* pEdge, const Maths::C_Line& vLineZeroedY, bool inside)
{
	const float epsilon = 1.0e-6f;

	do
	{
		if ((pEdge->m_pBackCell != 0 && !pEdge->m_pBackCell->GetClosed() && !pEdge->m_pBackCell->GetActionable()) == inside)
		{
			const Maths::C_Line vEdge(pEdge->m_Start.ZeroedY(), pEdge->m_End.ZeroedY());

			const F32 fTA = vLineZeroedY.Intersect(vEdge);
			const F32 fTB = vEdge.Intersect(vLineZeroedY);

			if ((fTA >= -epsilon) && (fTA <= 1.0f + epsilon) && (fTB >= -epsilon) && (fTB <= 1.0f + epsilon))
				return pEdge;
		}

		if (g_AIRoutingCapsuleDebug)
		{
			Maths::C_Vector adj(0.0f, 1.0f, 0.0f);

			DbUtil::DefaultDebugRenderSys()->Primitive()->Draw3dLine(pEdge->m_Start + adj, pEdge->m_End + adj, Maths::RGBA(0xFFFFFF00));
		}

		pEdge = pEdge->m_pRight;
	}
	while(pEdge != pFirstEdge);

	return 0;
}

const C_NavMeshCell* C_NavMesh::TraceLine(const C_NavMeshCell* pCell, C_NavMeshEdge* pFirstEdge, const Maths::C_Line& vLineZeroedY)
{
	C_NavMeshEdge* pEdge = pFirstEdge;

	while(pFirstEdge)
	{
		if (TraceLineSub(pFirstEdge, pEdge, vLineZeroedY, false))
			return 0;

		if ((pFirstEdge = TraceLineSub(pFirstEdge, pEdge, vLineZeroedY, true)) == 0)
			break;

		pCell = pFirstEdge->m_pBackCell;
		pFirstEdge = pFirstEdge->m_pBackEdge;
		pEdge = pFirstEdge->m_pRight;
	}

	return pCell;
}

bool C_NavMesh::TraceCapsule(const C_NavMeshCell* pAINavCell, const Maths::C_Line& vLineZeroedY, float radius)
{
	if(pAINavCell == NULL)
		return true;

	Maths::C_Vector perp(vLineZeroedY.Normal().Z() * radius, 0.0f, -vLineZeroedY.Normal().X() * radius);

	Maths::C_Vertex startLeft	= vLineZeroedY.Start() - perp;
	Maths::C_Vertex startRight	= vLineZeroedY.Start() + perp;

	Maths::C_Vertex endLeft		= vLineZeroedY.End() - perp;
	Maths::C_Vertex endRight	= vLineZeroedY.End() + perp;

	if (g_AIRoutingCapsuleDebug)
	{
		Maths::C_Vector adj(0.0f, 1.0f, 0.0f);

		DbUtil::DefaultDebugRenderSys()->Primitive()->Draw3dLine(startLeft + adj, endLeft + adj, Maths::RGBA(0xFFFF0000));
		DbUtil::DefaultDebugRenderSys()->Primitive()->Draw3dLine(startRight + adj, endRight + adj, Maths::RGBA(0xFF0000FF));
		DbUtil::DefaultDebugRenderSys()->Primitive()->Draw3dLine(vLineZeroedY.Start() + adj, vLineZeroedY.End() + adj, Maths::RGBA(0xFFFFFFFF));
	}

	// TODO: Should do proper capsule trace (someday)

	const C_NavMeshCell* left = TraceLine(pAINavCell, pAINavCell->m_pFirst, Maths::C_Line(vLineZeroedY.Start(), startLeft));
	const C_NavMeshCell* right = TraceLine(pAINavCell, pAINavCell->m_pFirst, Maths::C_Line(vLineZeroedY.Start(), startRight));

	if (!left || !right)
		return true;

	if (TraceLine(left, left->m_pFirst, Maths::C_Line(startLeft, endLeft)) == 0)
		return true;

	if (TraceLine(right, right->m_pFirst, Maths::C_Line(startRight, endRight)) == 0)
		return true;

	return TraceLine(pAINavCell, pAINavCell->m_pFirst, vLineZeroedY) == 0;
}


static Debug::Bool	g_NavMeshRender("AI.NavMesh.Render", false);
static Debug::Float	g_NavMeshRenderDistFromCamera("AI.NavMesh.RenderDistFromCamera", 50.0f, Debug::Range(0.0f, 1000.0f));
static Debug::Bool	g_NavMeshRenderCells("AI.NavMesh.RenderCells", true);
static Debug::Bool	g_NavMeshRenderEdges("AI.NavMesh.RenderEdges", true);
static Debug::Bool	g_NavMeshRenderNormals("AI.NavMesh.RenderNormals", false);
static Debug::Bool	g_NavMeshRenderCellID("AI.NavMesh.RenderID", false);
static Debug::Bool	g_NavMeshRenderCellSection("AI.NavMesh.RenderSection", false);
static Debug::Bool	g_NavMeshRenderCellLinkage("AI.NavMesh.RenderLinkage", false);
static Debug::Float	g_NavMeshRenderRaisedAmount("AI.NavMesh.RaisedAmount", 0.01f, Debug::Range(-2.0f, 4.0f));
static Debug::Float	g_NavMeshRenderOffsetAmount("AI.NavMesh.OffsetAmount", 0.0f, Debug::Range(0.0f, 0.5f));

void C_NavMesh::RenderSection(DbUtil::I_DebugRender* pDebugRender, const C_NavMeshSection* pCellSection)
{
	const Display::C_ViewSettings3D& viewSettings3D = Display::Device()->GetViewSettings3D();
	const Maths::C_Vertex vCameraPos = viewSettings3D.GetCameraPosition();
	const Maths::HVec3 vCameraDir = viewSettings3D.m_CameraToWorld.AZ();

	Maths::C_Vector vOffset(0, (F32)g_NavMeshRenderRaisedAmount, 0);
	Maths::C_RGBA_F32 vCellColour;

	for(U32 nCellCurr = 0; nCellCurr < pCellSection->m_CellCount; nCellCurr++)
	{
		C_NavMeshCell* pCell = &pCellSection->m_Cells[nCellCurr];
		bool bRenderCell = false;
		if((vCameraPos - pCell->m_Middle).Length() < g_NavMeshRenderDistFromCamera)
		{
			for(C_NavMeshEdge* pEdge = pCell->m_pFirst; pEdge; pEdge = pEdge->m_pNext)
			{
				if((vCameraDir | (pEdge->m_Start - vCameraPos).Normalised().Normal()) > 0.0f)
				{
					bRenderCell = true;
					break;
				}
			}
		}

		if(bRenderCell)
		{
			switch(pCell->m_Type)
			{
			case E_AICellType_Generic:
				{
					if(!pCell->GetActionable())
					{
						vCellColour = Maths::RGBA(0, 32, 0);
					}
					else
					{
						vCellColour = Maths::RGBA(32, 0, 0);
					}
				}
				break;
			case E_AICellType_Water:
				{
					vCellColour = Maths::RGBA(0, 0, 32);
				}
				break;
			case E_AICellType_Drop:
				{
					vCellColour = Maths::RGBA(32, 0, 32);
				}
				break;
			case E_AICellType_Doorway:
				{
					if(!pCell->GetActionable())
					{
						vCellColour = Maths::RGBA(128, 128, 128);
					}
					else
					{
						vCellColour = Maths::RGBA(192, 128, 128);
					}

					// Render Direction Arrow
					if(pCell->m_pLink && pCell->m_pLink->GetLinkToA() != pCell->m_pLink->GetLinkToB())
					{
						Maths::C_Vector vDirection;
						for(C_NavMeshEdge* pEdge = pCell->m_pFirst; pEdge; pEdge = pEdge->m_pNext)
						{
							if(pEdge->m_pBackCell)
							{
								if(pCell->m_pLink->GetLinkToA() && pEdge->m_pBackCell->m_SectionID == pCell->m_pLink->m_SectionA)
								{
									vDirection = pEdge->m_Middle - pCell->m_Middle;
								}
								else if(pCell->m_pLink->GetLinkToB() && pEdge->m_pBackCell->m_SectionID == pCell->m_pLink->m_SectionB)
								{
									vDirection = pEdge->m_Middle - pCell->m_Middle;
								}
							}
						}
						pDebugRender->Draw3dArrow(pCell->m_Middle + Maths::C_Vector(0.0f, 1.0f, 0.0f), vDirection.Normalised().Normal(), 0.5f, 0.15f, Maths::RGBA(0xffffffff));
					}
				}
				break;

			default:
				vCellColour = Maths::RGBA(255, 160, 160);
				break;
			}

			if(pCell->GetClosed())
			{
				vCellColour.R(vCellColour.R() * 0.025f);
				vCellColour.G(vCellColour.G() * 0.025f);
				vCellColour.B(vCellColour.B() * 0.025f);
			}

			for(C_NavMeshEdge *pEdge = pCell->m_pFirst; pEdge; pEdge = pEdge->m_pNext)
			{
				const Maths::C_Vertex vStartPos	= pEdge->m_Start + vOffset + Maths::C_Vector(0.0f, 0.002f, 0.0f);
				const Maths::C_Vertex vEndPos	= pEdge->m_End + vOffset + Maths::C_Vector(0.0f, 0.002f, 0.0f);

				// Draw Edges and Normals
				bool bOutsideEdge = false;
				Maths::RGBA vColour(0xffffffff);
				if(pEdge->m_pBackCell == NULL)
				{
					bOutsideEdge = true;
					vColour = Maths::RGBA(0xff0000ff);
				}

				if(g_NavMeshRenderEdges)
				{
					pDebugRender->Draw3dLine(vStartPos, vEndPos, vColour);
				}

				if(g_NavMeshRenderNormals)
				{
					if(bOutsideEdge)
					{
						pDebugRender->Draw3dArrow(Maths::C_Vector((vStartPos + vEndPos) * 0.5f), pEdge->m_Normal, 0.2f, 0.1f, vColour);
					}
					else
					{
						pDebugRender->Draw3dArrow(Maths::C_Vector((vStartPos + vEndPos) * 0.5f), pEdge->m_Normal, 0.05f, 0.025f, vColour);
					}
				}

				if(g_NavMeshRenderCells)
				{
					pDebugRender->Draw3dTriangle(pCell->m_Middle + vOffset, pEdge->m_Start + vOffset, pEdge->m_End + vOffset, vCellColour);
				}				
			}

			vOffset += Maths::C_Vector(0.0f, (F32)g_NavMeshRenderOffsetAmount, 0.0f);
		}
	}
}

static Debug::Int	g_NavMeshSectionDebugOffsetX("AI.NavMesh.SectionDebugOffsetX", 26, Debug::Range(10, 30));
static Debug::Int	g_NavMeshSectionDebugOffsetY("AI.NavMesh.SectionDebugOffsetY", 26, Debug::Range(10, 30));

void C_NavMesh::DebugRender(DbUtil::I_DebugRender* pDebugRender)
{
	if (!g_NavMeshRender)
		return;

	if(!pDebugRender)
		return;

	if(m_Section)
	{
		for(U32 nSectionCurr = 0; nSectionCurr < m_SectionCount; nSectionCurr++)
		{
			RenderSection(pDebugRender, &m_Section[nSectionCurr]);
		}
	}

	for(U32 nSectionCurr = 0; nSectionCurr < m_SectionCount; nSectionCurr++)
	{
		for(U32 nCellCurr = 0; nCellCurr < m_Section[nSectionCurr].m_CellCount; nCellCurr++)
		{
			if(g_NavMeshRenderCellSection)
			{
				pDebugRender->Draw3dText( m_Section[nSectionCurr].m_Cells[nCellCurr].m_Middle, Util::C_String().Printf("%d", nSectionCurr));
			}
			if(g_NavMeshRenderCellID)
			{
				pDebugRender->Draw3dText( m_Section[nSectionCurr].m_Cells[nCellCurr].m_Middle, Util::C_String().Printf("%d", nCellCurr));
			}
		}
	}

	if(g_NavMeshRenderCellLinkage && m_SectionLinkage)
	{
		U32 nYPos = 100, nXPos;

		nXPos = 80 + g_NavMeshSectionDebugOffsetX;
		for(U32 nTopRow = 0; nTopRow < m_SectionCount; nTopRow++)
		{
			pDebugRender->Draw2dText( Maths::HVec2( nXPos, nYPos ), Util::C_String().Printf("%d", nTopRow));
			nXPos += g_NavMeshSectionDebugOffsetX;
		}

		nYPos += g_NavMeshSectionDebugOffsetY;
		for(U32 nY = 0; nY < m_SectionCount; nY++)
		{
			nXPos = 80;
			pDebugRender->Draw2dText( Maths::HVec2( nXPos, nYPos ), Util::C_String().Printf("%d:", nY));

			nXPos += g_NavMeshSectionDebugOffsetX;
			for(U32 nX = 0; nX < m_SectionCount; nX++)
			{
				if(m_SectionLinkage[(nY * m_SectionCount) + nX])
				{
					pDebugRender->Draw2dText( Maths::HVec2( nXPos, nYPos ), Util::C_String().Printf(" 1"));
				}
				else
				{
					pDebugRender->Draw2dText( Maths::HVec2( nXPos, nYPos ), Util::C_String().Printf(" 0"));
				}

				nXPos += g_NavMeshSectionDebugOffsetX;
			}

			nYPos += g_NavMeshSectionDebugOffsetY;
		}
	}

	pDebugRender->Draw2dText( Maths::HVec2( 100, 50 ), Util::C_String().Printf("Sections: %d", m_SectionCount));
}


}
