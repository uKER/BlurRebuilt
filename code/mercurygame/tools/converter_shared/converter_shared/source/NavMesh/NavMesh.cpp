#include "stdafx.h"

#include <converter_shared/NavMesh/NavMesh.h>
#include <system/misc/StringUtil.h>

#ifdef USER_PETERB
#pragma optimize("", off)
#endif

namespace NavMesh
{


// --------------------------------------------------------------------------
//	- Class: ExportCell -
// --------------------------------------------------------------------------

void ExportCell::LinkAdjacentCells( Btl::List<ExportCell*> &cellList )
{
	ExportCellEdge *src_p0(m_pEdge + 0);
	ExportCellEdge *src_p1(m_pEdge + 1);
	for(U32 i = 0; i < m_EdgeCount; i++, src_p0++, src_p1++)
	{
		// Already linked?
		if(src_p0->m_NextCellIndex != 0xffff)
			continue;

		if((i + 1) == m_EdgeCount)
			src_p1 = m_pEdge;
	
		bool found = false;
		for(Btl::List<ExportCell*>::Iterator it = cellList.Begin(); it!= cellList.End() && !found; ++it)
		{
			if((*it) == this)
				continue;

			ExportCellEdge *dst_p0((*it)->m_pEdge + 0);
			ExportCellEdge *dst_p1((*it)->m_pEdge + 1);

			for(U32 k = 0; k < (*it)->m_EdgeCount; k++, dst_p0++, dst_p1++)
			{
				if((k + 1) == (*it)->m_EdgeCount)
					dst_p1 = (*it)->m_pEdge;

				// Reversed matched?
				if( src_p0->m_VertexIndex == dst_p1->m_VertexIndex && src_p1->m_VertexIndex == dst_p0->m_VertexIndex )
				{
					src_p0->m_NextCellIndex = (*it)->m_Index;
					src_p0->m_NextVertIndex = (U8)k;

					dst_p0->m_NextCellIndex = m_Index;
					dst_p0->m_NextVertIndex = (U8)i;
					
					// Debug Segmentation Info
					src_p0->m_pNextCell = (*it);
					dst_p0->m_pNextCell = this;

					found = true;
					break;
				}
			}
		}
	}
}

bool ExportCell::Valid( Aftk::C_SceneMeshNode *m_pAftkSceneMeshNode )
{
	Maths::C_Vertex vertPos[2];
	Aftk::Position3 aftkPos;

	if(m_EdgeCount < 3)
	{
		if(m_pEdge)
		{
			aftkPos = m_pAftkSceneMeshNode->GetVertexPosition( m_pEdge->m_VertexIndex );
			vertPos[0] = Maths::C_Vertex(aftkPos.X(), aftkPos.Y(), aftkPos.Z());
			LOGWARN( Tools::g_log,  "WARNING : Cell with point (%.3f %.3f %.3f) has only %d sides\n",  (F32)vertPos[0].X(), (F32)vertPos[0].Y(), (F32)vertPos[0].Z(), m_EdgeCount);
		}
		else
		{
			LOGWARN( Tools::g_log,  "WARNING : Cell has only %d sides\n", m_EdgeCount);	
		}
		return false;
	}

	ExportCellEdge *p0(m_pEdge + 0);
	ExportCellEdge *p1(m_pEdge + 1);
	for(U32 i = 0; i < m_EdgeCount; i++, p0++, p1++)
	{
		if( (i + 1) == m_EdgeCount )
			p1 = m_pEdge;

		aftkPos = m_pAftkSceneMeshNode->GetVertexPosition( p0->m_VertexIndex );
		vertPos[0] = Maths::C_Vertex(aftkPos.X(), aftkPos.Y(), aftkPos.Z());

		aftkPos = m_pAftkSceneMeshNode->GetVertexPosition( p1->m_VertexIndex );
		vertPos[1] = Maths::C_Vertex(aftkPos.X(), aftkPos.Y(), aftkPos.Z());

		Maths::C_Vector diff( vertPos[1] - vertPos[0] );

		if(diff.LengthSquared() <= 0.0f)
		{
			LOGDEBUG( Tools::g_log,  "Cell has two verts in the same position (%8.4f %8.4f %8.4f)\n",  (F32)vertPos[0].X(), (F32)vertPos[0].Y(), (F32)vertPos[0].Z());
		}
	}
	return true;
}


// --------------------------------------------------------------------------
//	- Class: PreprocTri -
// --------------------------------------------------------------------------

const bool PreprocTri::Validate( Aftk::C_SceneMeshNode *m_pAftkSceneMeshNode )
{
	for(U32 i = 0; i < 3; i++)
	{
		Aftk::Position3 aftkPos = m_pAftkSceneMeshNode->GetVertexPosition( m_VertexIndex[i] );
		m_Position[i] = Maths::C_Vertex(aftkPos.X(), aftkPos.Y(), aftkPos.Z());
	}
	for(U32 i = 0, j = 1; i < 3; i++, j++)
	{
		if( j == 3 ) j = 0;

		Maths::C_Vector diff( m_Position[j] - m_Position[i]);

		if(diff.LengthSquared() <= 0.0f)
		{
			LOGWARN( Tools::g_log,  "Two points of triangle are the same\n");
			return false;
		}
	}

	Maths::C_Vector vTemp0( m_Position[0] - m_Position[1] );
	Maths::C_Vector vTemp1( m_Position[1] - m_Position[2] );
	Maths::C_Vector vCross = (vTemp0 ^ vTemp1);
	if(vCross.Length() == 0.0f)
	{
		LOGWARN( Tools::g_log,  "Edges of triangle are aligned\n");
		return false;
	}

	m_Normal = vCross.Normalised().Normal() * -1.0f;

	return(true);
}

void PreprocTri::WarnInvalid()
{
	LOGWARN( Tools::g_log,  "Navmesh Problem : Invalid triangle @ (%5.2f, %5.2f, %5.2f - %5.2f, %5.2f, %5.2f - %5.2f, %5.2f, %5.2f\n", 
		(F32)m_Position[0].X(), (F32)m_Position[0].Y(), (F32)m_Position[0].Z(),
		(F32)m_Position[1].X(), (F32)m_Position[1].Y(), (F32)m_Position[1].Z(),
		(F32)m_Position[2].X(), (F32)m_Position[2].Y(), (F32)m_Position[2].Z() );
}


// --------------------------------------------------------------------------
//	- Class: PreprocPolygon -
// --------------------------------------------------------------------------

PreprocPolygon::PreprocPolygon( Aftk::C_SceneMeshNode *pAftkSceneMeshNode, Btl::List<PreprocAdjacentTriangle*> &adjacentList)
	: m_pAftkSceneMeshNode( pAftkSceneMeshNode )
{
	// Clear stored flags
	for(Btl::List<PreprocAdjacentTriangle*>::Iterator it2 = adjacentList.Begin(); it2 != adjacentList.End(); ++it2)
		(*it2)->m_pTriangle->m_Stored = false;

	// Store each triangle as an edge
	Btl::List<PreprocAdjacentTriangle*> workList;

	workList.PushBack( adjacentList.Front() );
	adjacentList.PopFront();
	while(workList.Size()>0)
	{
		S32 recursionDepth = 0;
		PreprocAdjacentTriangle* w = workList.Front();
		w->StoreEdgesRecursive(m_EdgeList, m_pAftkSceneMeshNode, recursionDepth, workList);
		workList.PopFront();
	}

	//adjacentList.First()->StoreEdgesRecursive(m_EdgeList, m_pAftkSceneMeshNode, recursionDepth);
}

PreprocPolygon::~PreprocPolygon()
{
	//Deallocate the list
	for(Btl::List<PreprocEdge*>::Iterator it = m_EdgeList.Begin(); it != m_EdgeList.End(); ++it)
		delete (*it);
}

Btl::List<PreprocEdge*>::Iterator PreprocPolygon::FindSharedEdge(PreprocEdge *pItem)
{
	for(Btl::List<PreprocEdge*>::Iterator it = m_EdgeList.Begin(); it != m_EdgeList.End(); ++it)
	{
		if((*it) == pItem)
			continue;

		if((*it)->m_VertexIndex[0] == pItem->m_VertexIndex[1] && (*it)->m_VertexIndex[1] == pItem->m_VertexIndex[0])
			return it;
	}
	return(m_EdgeList.End());
}

// Return:  1 for C left of the line through A and B
//          0 for C on the line
//         -1 for C right of the line

const S32 PreprocPolygon::AreaSign(const Aftk::Position3 &a, const Aftk::Position3 &b, const Aftk::Position3 &c)
{
	const F32 signEpsilon = 0.001f;

	Maths::HVec2 v0(b.x-a.x, b.z-a.z);
	Maths::HVec2 v1(c.x-a.x, c.z-a.z);

	v0.Normalise();
	v1.Normalise();

	F32 area2 = (v1.X() * v0.Y()) - (v0.X() * v1.Y()) ;
	
    // The area should be an integer
    if      (area2 >  signEpsilon) return  1;
    else if (area2 < -signEpsilon) return -1;
    else return 0;
}

void PreprocPolygon::SplitToConvex( )
{
	for(U32 i = 0; i < 16; i++)
	{
		U32 removedCount = RemoveUnnecessaryDiagonals();
		U32 weldCount = WeldAligned();//0;

		if( !removedCount && !weldCount )
			break;
	}
}

const U32 PreprocPolygon::WeldAligned( )
{
	const F32 weldThreshold = 0.9999f;

	U32 weldedCount = 0;

	// For restarting when a weld is done
	for(bool finished = false; !finished; )
	{
		// Flag all edges as contenders for welding
		for(Btl::List<PreprocEdge*>::Iterator it = m_EdgeList.Begin(); it != m_EdgeList.End(); ++it)
		{
			(*it)->m_DoWeld = true;
		}

		finished = true;
		for(Btl::List<PreprocEdge*>::Iterator it = m_EdgeList.Begin(); it != m_EdgeList.End(); ++it)
		{
			PreprocEdge *pItem = (*it)->m_pNextEdge;

			// Flagged not for welding?
			if(!(*it)->m_DoWeld)
				continue;

			// See if I am aligned with my 'next' edge
			Maths::C_Vertex vec0[2], vec1[2];
			for(U32 j = 0; j < 2; j++)
			{
				Aftk::Position3 temp;
				
				temp = m_pAftkSceneMeshNode->GetVertexPosition( (*it)->m_VertexIndex[j] );
				vec0[j] = Maths::C_Vertex( temp.x, temp.y, temp.z );

				temp = m_pAftkSceneMeshNode->GetVertexPosition( pItem->m_VertexIndex[j] );
				vec1[j] = Maths::C_Vertex( temp.x, temp.y, temp.z );
			}
			Maths::C_Vector diff0(vec0[1] - vec0[0]);
			Maths::C_Vector diff1(vec1[1] - vec1[0]);

			F32 dot = diff0.Normalised().Normal() | diff1.Normalised().Normal();

			if(Maths::Abs(dot) <= weldThreshold)
				continue;

			// Check all others shared with this edge (todo -> store in list and remove all of them at once if valid)
			U32 usedCount = 0;
			for(Btl::List<PreprocEdge*>::Iterator it2 = m_EdgeList.Begin(); it2 != m_EdgeList.End(); ++it2)
			{
				if((*it)->m_VertexIndex[1] == (*it2)->m_VertexIndex[0] || (*it)->m_VertexIndex[1] == (*it2)->m_VertexIndex[1])
				{
					// Check dot for this shared edge
					Maths::C_Vertex vec0[2], vec1[2];	
					for(U32 j = 0; j < 2; j++)
					{
						Aftk::Position3 temp;
						
						temp = m_pAftkSceneMeshNode->GetVertexPosition( (*it)->m_VertexIndex[j] );
						vec0[j] = Maths::C_Vertex( temp.x, temp.y, temp.z );

						temp = m_pAftkSceneMeshNode->GetVertexPosition( (*it2)->m_VertexIndex[j] );
						vec1[j] = Maths::C_Vertex( temp.x, temp.y, temp.z );
					}

					Maths::C_Vector diff2(vec0[1] - vec0[0]);
					Maths::C_Vector diff3(vec1[1] - vec1[0]);

					F32 dot = diff2.Normalised().Normal() | diff3.Normalised().Normal();

					if(Maths::Abs(dot) <= weldThreshold)
					{
						usedCount++;
						if((*it)->m_VertexIndex[1] == (*it2)->m_VertexIndex[1])
							(*it2)->m_DoWeld = false;
					}
				}
			}
					
			if(usedCount)
			{
				(*it)->m_DoWeld = false;
				continue;
			}

			// Delete
			weldedCount++;

			(*it)->m_pNextEdge = pItem->m_pNextEdge;
			pItem->m_pNextEdge->m_pPrevEdge = (*it);
			(*it)->m_VertexIndex[1] = (*it)->m_pNextEdge->m_VertexIndex[0];
			(*it)->m_VertexPosition[1] = (*it)->m_pNextEdge->m_VertexPosition[0];

			// Flag to be tested by RemoveUnnecessaryDiagonals
			(*it)->m_Split = false;

			//Look and delete pItem
			for(Btl::List<PreprocEdge*>::Iterator itItem = m_EdgeList.Begin(); itItem != m_EdgeList.End(); ++itItem)
			{
				if (pItem == (*itItem))
				{
					delete (*itItem);
					m_EdgeList.Erase(itItem);
					break;
				}
			}

			// Start again
			finished = false;
			break;
		}
	}

	return weldedCount;
}

const bool PreprocPolygon::IsConvex( const U16 idx0, const U16 idx1, const U16 idx2)
{
	if(idx1 == idx2)
		return false;

	Aftk::Position3 aftkPosD = m_pAftkSceneMeshNode->GetVertexPosition( idx0 );
	Aftk::Position3 aftkPos0 = m_pAftkSceneMeshNode->GetVertexPosition( idx1 );
	Aftk::Position3 aftkPos1 = m_pAftkSceneMeshNode->GetVertexPosition( idx2 );

	return( AreaSign(aftkPosD, aftkPos0, aftkPos1) <= 0 );
}

const U32 PreprocPolygon::RemoveUnnecessaryDiagonals( )
{
	U32 removedCount = 0;

	for(Btl::List<PreprocEdge*>::Iterator it = m_EdgeList.Begin(); it != m_EdgeList.End(); ++it)
	{
		if(!(*it)->m_Diagonal || (*it)->m_Split)
			continue;

		bool isConvex = false;
		Btl::List<PreprocEdge*>::Iterator itMatchingEdge = FindSharedEdge( *it );
		if(itMatchingEdge == m_EdgeList.End())
			continue;

		PreprocEdge *pMatchingEdge = *itMatchingEdge;

		if(pMatchingEdge->m_TextureType != (*it)->m_TextureType)
			continue;

		if(((*it)->m_SurfaceNormal | pMatchingEdge->m_SurfaceNormal) < 0.99f)
			continue;

        if(	IsConvex( (*it)->m_VertexIndex[0], (*it)->m_pPrevEdge->m_VertexIndex[0], pMatchingEdge->m_pNextEdge->m_VertexIndex[1] )	&&
			IsConvex( pMatchingEdge->m_VertexIndex[0], pMatchingEdge->m_pPrevEdge->m_VertexIndex[0], (*it)->m_pNextEdge->m_VertexIndex[1] ) )
		{
			// Merge 2 polygons into one
			(*it)->m_pPrevEdge->m_pNextEdge = pMatchingEdge->m_pNextEdge;
			(*it)->m_pNextEdge->m_pPrevEdge = pMatchingEdge->m_pPrevEdge;

			pMatchingEdge->m_pPrevEdge->m_pNextEdge = (*it)->m_pNextEdge;
			pMatchingEdge->m_pNextEdge->m_pPrevEdge = (*it)->m_pPrevEdge;

			// Delete the diagonal edges
			delete (*it);
			m_EdgeList.Erase(it);
			delete (*itMatchingEdge);
			m_EdgeList.Erase(itMatchingEdge);
			

			// Reset search
			it = m_EdgeList.Begin();

			removedCount++;
		}
		else
		{
			// Diagonals become real splits
			(*it)->m_Split				= true;
			pMatchingEdge->m_Split	= true;
		}
	}

	return( removedCount );
}

ExportCell *PreprocPolygon::BuildCell( const U32 aftkSourceIndex )
{
	Btl::List<PreprocEdge*> outList;
	PreprocEdge *pSrc( m_EdgeList.Back() );
	m_EdgeList.PopBack();

	outList.PushBack( pSrc );

	PreprocEdge *p = pSrc;

	while(1)
	{
		p = p->m_pNextEdge;
		if(p == pSrc)
			break;
		
		//Remove p from the list
		Btl::List<PreprocEdge*>::Iterator it = m_EdgeList.Begin();
		while (it != m_EdgeList.End())
		{
			if (*it == p)
			{
				m_EdgeList.Erase( it );
				break;
			}
			else
				++it;
		}
		
		outList.PushBack( p );
	}
	
	ExportCell *pCell = new ExportCell( aftkSourceIndex, (U16)outList.Size() );

	ExportCellEdge *pEdge(pCell->m_pEdge);
	for(Btl::List<PreprocEdge*>::Iterator it = outList.Begin(); it != outList.End(); ++it, pEdge++)
	{
		pEdge->m_VertexIndex	= (*it)->m_VertexIndex[0];
		pEdge->m_Normal			= (*it)->m_Normal;
	}
	pCell->m_Type = outList.Front()->m_TextureType;

	// Not really needed as it will be called when it goes out of scope
	for(Btl::List<PreprocEdge*>::Iterator it = outList.Begin(); it != outList.End(); ++it)
		delete (*it);

	outList.Clear(); 

	return pCell;
}

void PreprocPolygon::ExportToCells( Btl::List<ExportCell*> &cellList, const U32 aftkSourceIndex)
{
	// Build cells from the edges which are linked together
	while(m_EdgeList.Size())
	{
		Btl::List<PreprocEdge*> convexEdgeList;

		ExportCell *pCell = BuildCell(aftkSourceIndex);

		if(pCell->Valid(m_pAftkSceneMeshNode))
		{
			pCell->m_Index = cellList.Size();

			cellList.PushBack(pCell);
		}
		else
		{
			delete pCell;
		}
	}
}

// --------------------------------------------------------------------------
//	- Class: PreprocAdjacentTriangle -
// --------------------------------------------------------------------------
void PreprocAdjacentTriangle::StoreEdgesRecursive(Btl::List<PreprocEdge*> &edgeList, Aftk::C_SceneMeshNode *pAftkSceneMeshNode, S32& recursionDepth, Btl::List<PreprocAdjacentTriangle*>& workList)
{
	recursionDepth++;
	if(recursionDepth>16)
	{
		workList.PushBack(this);
		return;
	}

	if(m_pTriangle->m_Stored)
	{
		recursionDepth--;
		return;
	}

	m_pTriangle->m_Stored = true;
	{
		PreprocEdge *pItem = NULL;
		PreprocEdge *pFirst = NULL;
		for(U32 i = 0, j = 1; i < 3; i++, j++)
		{
			if(j == 3)
			{
				j = 0;
			}

			PreprocEdge *pNew = new PreprocEdge( m_pTriangle->m_VertexIndex[i], m_pTriangle->m_VertexIndex[j], (m_pAdjacent[i]!=NULL), m_pTriangle->m_TextureType, m_pTriangle->m_Normal, pAftkSceneMeshNode );

			if(pItem)
			{
				pItem->m_pNextEdge = pNew;
				pNew->m_pPrevEdge = pItem;
			}
			else
			{
				pFirst = pNew;
			}

			pItem = pNew;
			edgeList.PushBack( pItem );
		}

		// Link back to the first
		pItem->m_pNextEdge = pFirst;
		pFirst->m_pPrevEdge = pItem;
	}
	// Store any adjacent triangles
	for(U32 i = 0; i < 3; i++)
	{
		if(m_pAdjacent[i])
		{
			m_pAdjacent[i]->StoreEdgesRecursive(edgeList, pAftkSceneMeshNode, recursionDepth, workList);
		}
	}
	recursionDepth--;
}

void PreprocAdjacentTriangle::FindRecursive( Btl::List<PreprocAdjacentTriangle*> &adjacentList )
{
    U32 aftkLod = 0;
    
	if(m_pTriangle->m_Stored)
		return;

	m_pTriangle->m_Stored = true;
	adjacentList.PushBack(this);

	for(U32 src_v0 = 0, src_v1 = 1, src_v2 = 2; src_v0 < 3; src_v0++, src_v1++, src_v2++)
	{
		if(src_v1 == 3) src_v1 = 0;
		if(src_v2 == 3) src_v2 = 0;
	
		const int ret = m_pAdjacencyList->GetAdjacentPolyIndex(aftkLod, m_pTriangle->m_AftkPolygonSourceIdx, m_pTriangle->m_VertexIndex[src_v0], m_pTriangle->m_VertexIndex[src_v1] );
		if(ret == -1)
			continue;

		// Find adjacent triangle
		bool found = false;
		Btl::List<PreprocTri*>::Iterator it2 = (*m_pTriList).Begin();
		for (; it2 != (*m_pTriList).End(); ++it2 )
		{
			if((*it2)->m_AftkPolygonSourceIdx == ret)
			{
				found = true;
				break;
			}
		}

		if(!found)
			continue;

		m_pAdjacent[src_v0] = new PreprocAdjacentTriangle(*it2, m_pAdjacencyList, m_pTriList);
		m_pAdjacent[src_v0]->FindRecursive(adjacentList);
	}
}


// --------------------------------------------------------------------------
//	- Class: NavMeshNode -
// --------------------------------------------------------------------------

NavMeshNode::NavMeshNode( 
	Aftk::C_SceneMeshNode *pAftkSceneMeshNode) :
	m_pAftkSceneMeshNode( pAftkSceneMeshNode ),
	m_LayerName("Invalid" )
{ 
    const U32 aftkLod = 0;
	m_Name = Util::C_String128( pAftkSceneMeshNode->GetName().c_str() );

	const Aftk::C_LayerNode *pLayer = 0;
	if (pAftkSceneMeshNode->GetNumLayers() > 0)
		//[Ken] Use the first layer we encounter.
		pLayer = pAftkSceneMeshNode->GetLayerPtr(0);

	if(pLayer)
	{
		//m_LayerName = Util::C_String( GetInstanceName( pLayer->GetName().c_str() ) );
		m_LayerName = pLayer->GetName().c_str();
		//m_LayerName = m_LayerName.Upper();
	}

	LOGDEBUG( Tools::g_log,  "---- %s\n", m_Name.Chars() );

	//Convert to world space

	// ARGH! - Freeze transform
	pAftkSceneMeshNode->ApplyMatrix(pAftkSceneMeshNode->GetWorldMatrix());


	// Prepare mesh
	Aftk::C_MeshWeldMemoryManager weldMemoryManager;
	Aftk::C_MeshWeld::WeldVertexPositions(weldMemoryManager, m_pAftkSceneMeshNode, 0.001f * 0.01f);
	Aftk::C_MeshWeld::WeldTextureCoordinates(weldMemoryManager, m_pAftkSceneMeshNode, 0.001f * 0.01f);
	Aftk::C_MeshWeld::WeldPolygonVertices(weldMemoryManager, m_pAftkSceneMeshNode);


	Aftk::C_MeshTriangulate::Triangulate(m_pAftkSceneMeshNode);

	U32 degeneratesRemoved	= Aftk::C_MeshRemoveDegenerates::RemoveDegenerates(m_pAftkSceneMeshNode, aftkLod);
	U32 vertexRemoved		= m_pAftkSceneMeshNode->RepairVertexPositions();	
	U32 coordsRemoved		= m_pAftkSceneMeshNode->RepairTextureCoordinates();

	m_AdjacencyList.GenerateAdjacency( m_pAftkSceneMeshNode );

	CollectTriangles();

	// Generic
	CheckTriangleList(m_GenericTriList);
	BuildGenericCellList();

	U32 genericCells = m_CellList.Size();
	if(genericCells)
	{
		LOGDEBUG( Tools::g_log,  "--- %5d triangles made %d generic cells\n", m_GenericTriList.Size(), genericCells);
	}

	RemapVertices();

	LinkCells();

	CalcCellCentresAndOuterEdges();

	// If this is the Base Mesh, Perform Segmentation
	Util::C_String levelName(m_LayerName);
	const S32 _pos = levelName.Search('_', 9);
	if(_pos == -1)
	{
		levelName = levelName(9,levelName.Length());
		if(levelName == "BASE")
		{
			Segment();
		}
	}

	LOGDEBUG( Tools::g_log,  "--- %5d Total NavMesh Cells\n", m_CellList.Size() );
	LOGDEBUG( Tools::g_log,  "--- %5d NavMesh Vertices\n", m_VertexList.Size() );
}


void NavMeshNode::Segment()
{
	ExportCell* pCellFirst = *(m_CellList.Begin());
	if(pCellFirst)
	{
		// Creation of Open Lists and Insertion of Initial Cell
		Btl::List<C_ExportCellItem*> openDoorList;
		Btl::List<C_ExportCellItem*> openCellList;

		C_ExportCellItem* pCellItemFirst = new C_ExportCellItem(pCellFirst);
		if(pCellFirst->m_Type == E_AICellType_Doorway)
		{
			openDoorList.PushBack(pCellItemFirst);
		}
		else
		{
			openCellList.PushBack(pCellItemFirst);
		}


		U32 nSectionIDCurr = 0, nSectionCellCount = 0;
		while(openCellList.Size() || openDoorList.Size())
		{
			Btl::List<C_ExportCellItem*>::Iterator itCellItemCheck = openCellList.Begin();
			if(itCellItemCheck != openCellList.End())
			{
				C_ExportCellItem* pCellItemCheck = (*itCellItemCheck);

				ExportCell* pCellCheck = pCellItemCheck->m_pCell;
				if(pCellCheck)
				{
					for(U32 nEdgeCurr = 0; nEdgeCurr < pCellCheck->m_EdgeCount; nEdgeCurr++)
					{
						ExportCell* pCellNext = pCellCheck->m_pEdge[nEdgeCurr].m_pNextCell;
						if(pCellNext && pCellNext->m_Segmented == E_CellSegmentationStage_Initial)
						{
							C_ExportCellItem* pCellItemCheckAux = new C_ExportCellItem(pCellNext);
							if(pCellNext->m_Type == E_AICellType_Doorway)
							{
								nSectionCellCount++;
								pCellNext->m_SectionID = nSectionIDCurr;
								openDoorList.PushBack(pCellItemCheckAux);
							}
							else
							{
								openCellList.PushBack(pCellItemCheckAux);
							}

							pCellNext->m_Segmented = E_CellSegmentationStage_Added;
						}
					}

					nSectionCellCount++;
					pCellCheck->m_Segmented = E_CellSegmentationStage_Segmented;
					pCellCheck->m_SectionID = nSectionIDCurr;
					
					
					delete pCellItemCheck;
					openCellList.Erase(openCellList.Begin());
				}
			}


			// Current Section has been Completed
			if(openCellList.Size() == 0)
			{
				// Add count to Section list and Reset values
				if(nSectionCellCount)
				{
					C_SectionCellCount* pSectionCellCount = new C_SectionCellCount;
					pSectionCellCount->m_Count = nSectionCellCount;
					m_SectionCellCount.PushBack(pSectionCellCount);
					nSectionIDCurr++;

					nSectionCellCount = 0;
				}


				// Retrieve First Door in List

				Btl::List<C_ExportCellItem*>::Iterator itDoorItemCheck = openDoorList.Begin();
				
				if(itDoorItemCheck != openDoorList.End())
				{
					C_ExportCellItem* pDoorItemCheck = (*itDoorItemCheck);

					ExportCell* pDoorCheck = pDoorItemCheck->m_pCell;
					if(pDoorCheck)
					{
						U32 nDoorJointCount = 0;
						// Cycle Through Door's Edges to find Unchecked Cell
						bool bWarned = false;
						for(U32 nEdgeCurr = 0; nEdgeCurr < pDoorCheck->m_EdgeCount; nEdgeCurr++)
						{
							ExportCell* pCellNext = pDoorCheck->m_pEdge[nEdgeCurr].m_pNextCell;
							if(pCellNext)
							{
								if(pCellNext->m_Type == E_AICellType_Doorway && bWarned == false)
								{
									bWarned = true;
									LOGWARN(Tools::g_log,  "Navmesh Problem : Door cell @ (%5.2f, %5.2f, %5.2f) is linked to another door cell. This will cause problems!!\n", (float)pDoorCheck->m_Middle.X(), (float)pDoorCheck->m_Middle.Y(), (float)pDoorCheck->m_Middle.Z());
								}
								else if(pCellNext->m_Segmented == E_CellSegmentationStage_Initial)
								{
									C_ExportCellItem* pDoorItemCheckAux = new C_ExportCellItem(pCellNext);
									openCellList.PushBack(pDoorItemCheckAux);

									pCellNext->m_Segmented = E_CellSegmentationStage_Added;
								}

								nDoorJointCount++;
							}
						}

						if(nDoorJointCount != 2)
						{
							LOGWARN(Tools::g_log,  "Navmesh Problem : Door cell @ (%5.2f, %5.2f, %5.2f) should be joined by exactly 2 other cells. This will cause problems!!\n", (float)pDoorCheck->m_Middle.X(), (float)pDoorCheck->m_Middle.Y(), (float)pDoorCheck->m_Middle.Z());
						}

						pDoorCheck->m_Segmented = E_CellSegmentationStage_Segmented;
						delete pDoorItemCheck;
						openDoorList.Erase(openDoorList.Begin());
					}
				}
			}
		}
	
		//Delete
		for(Btl::List<C_ExportCellItem*>::Iterator it = openDoorList.Begin(); it != openDoorList.End(); ++it)
			delete (*it);
		for(Btl::List<C_ExportCellItem*>::Iterator it = openCellList.Begin(); it != openCellList.End(); ++it)
			delete (*it);
	}

	U32* nIDTracker = new U32[m_SectionCellCount.Size()];
	for(S32 i = 0; i < m_SectionCellCount.Size(); i++)
	{
		nIDTracker[i] = 0;
	}
	
	C_CrossLink* nIDCrossLink = new C_CrossLink[m_CellList.Size()];
	Btl::List<ExportCell*>::Iterator it = m_CellList.Begin();
	
	U32 nCurrCellIndex = 0;
	while(it != m_CellList.End())
	{
		ExportCell* pCell = *it;
		nIDCrossLink[nCurrCellIndex].m_SectionID = pCell->m_SectionID;
		nIDCrossLink[nCurrCellIndex].m_CellID = nIDTracker[pCell->m_SectionID]++;

		++it;
		nCurrCellIndex++;
	}
	
	it = m_CellList.Begin();
	while(it != m_CellList.End())
	{
		ExportCell* pCell = *it;
		ExportCellEdge* pEdge(pCell->m_pEdge);
		for(U32 i = 0; i < pCell->m_EdgeCount; i++, pEdge++)
		{
			const U16 nCellIndex = pEdge->m_NextCellIndex;
			if(nCellIndex != 0xffff)
			{
				pEdge->m_NextCellIndex = nIDCrossLink[nCellIndex].m_CellID;
				pEdge->m_NextCellSection = nIDCrossLink[nCellIndex].m_SectionID;
			}
		}

		++it;
	}

	delete[] nIDTracker;
	delete[] nIDCrossLink;
}

void NavMeshNode::CheckTriangleList(Btl::List<PreprocTri*>& myList)
{
	// Print out a list of edges that are shared by 2 triangles (in the same direction)
	for(Btl::List<PreprocTri*>::Iterator it = myList.Begin(); it != myList.End(); ++it)
	{
		for(Btl::List<PreprocTri*>::Iterator it2 = it; it2 != myList.End(); ++it2)
		{
			if(*it == *it2)
				continue;

			bool bClear = true;

			//for(U32 i = 0, j = 1; i < 3 && bClear; i++, j++)
			for(U32 i = 0, j = 1; i < 3; i++, j++)
			{
				U32 idx[4];

				if( j==3 ) j = 0;
				
				idx[0] = (*it)->m_VertexIndex[i];
				idx[1] = (*it)->m_VertexIndex[j];

				for(U32 k = 0, l = 1; k < 3; k++, l++)
				{
					if( l==3 ) l = 0;

					idx[2] = (*it2)->m_VertexIndex[k];
					idx[3] = (*it2)->m_VertexIndex[l];

					if(idx[0] == idx[2] && idx[1] == idx[3])
					{
						Maths::C_Vertex pos[2];

						for(U32 m = 0; m < 2; m++)
						{
							Aftk::Position3 aftkPos = m_pAftkSceneMeshNode->GetVertexPosition( idx[m] );
							pos[m] = Maths::C_Vertex(aftkPos.X(), aftkPos.Y(), aftkPos.Z());
						}

						const Maths::C_Vertex posWorld0 = (Maths::HVec3)pos[0];
						const Maths::C_Vertex posWorld1 = (Maths::HVec3)pos[1];

						LOGWARN(Tools::g_log,  "Navmesh Problem : Edge @ (%5.2f, %5.2f, %5.2f -> %5.2f, %5.2f, %5.2f) has issues!! Check for a duplicated edge or overlapping cells sharing it. \n",
							(F32)posWorld0.X(), (F32)posWorld0.Y(), (F32)posWorld0.Z(),
							(F32)posWorld1.X(), (F32)posWorld1.Y(), (F32)posWorld1.Z());
						
						bClear = false;

						break;
					}
				}
			}
		}
	}
}

void NavMeshNode::CollectTriangles()
{
    U32 aftkLod = 0;
    
	U32 noPolys = m_pAftkSceneMeshNode->GetNumPolygons(aftkLod);

    for(U32 i=0; i<noPolys; i++)
    {
		Aftk::C_Polygon* pPolygon = &m_pAftkSceneMeshNode->GetPolygon(i);
		const U32 noPolyVertices = pPolygon->GetNumPolygonVertices();
		U16 textureType = TypeFromMaterial(pPolygon);

		if(textureType == E_AICellType_NotDefined)
			continue;

		if(noPolyVertices != 3)
		{
			LOGWARN(Tools::g_log,  "Navmesh Problem : Polygon that doesn't have 3 verts error - Please triangulate NavMesh or export as triangles!!!\n");
			continue;
		}

		bool valid = true;
		U32	posIndex[3];
		U32 texCoord[3];

        for(U32 j = 0; j < 3; j++)
        {
            const Aftk::C_PolygonVertex* pPolyVertex = &pPolygon->GetPolygonVertex(j);
			
			posIndex[j] = pPolyVertex->GetPositionIndex();
			texCoord[j] = pPolyVertex->GetTextureCoordinateIndex( 0 );

			Aftk::Position3 aftkPos = m_pAftkSceneMeshNode->GetVertexPosition(posIndex[j]);
			Maths::C_Vertex hPosTmp(aftkPos.X(), aftkPos.Y(), aftkPos.Z());
			if(!hPosTmp.Finite())
			{
				valid = false;
			}
		}

		if(valid)
		{
			PreprocTri *pTri = new PreprocTri( i );

			for ( U32 j = 0; j < 3; j++ )
			{
				pTri->m_VertexIndex[j]	= posIndex[j];
				pTri->m_TextureCoordIndex[j]= texCoord[j];
				pTri->m_TextureType = textureType;
			}
			if(pTri->Validate( m_pAftkSceneMeshNode ))
			{
				m_GenericTriList.PushBack( pTri );
            }
			else
			{
				pTri->WarnInvalid();
				delete pTri;
			}
		}
	}
}

void NavMeshNode::TextureCoordsShiftToZero( Aftk::TextureCoordinate4 *pTextureCoordinate, const U32 count)
{
	U32 smallestIndex_x, smallestIndex_y;
	U32 largestIndex_x, largestIndex_y;
	F32 smallest, largest;

	// X
	smallest = FLT_MAX;
	largest = FLT_MIN;
	for(U32 i = 0; i < count; i++)
	{
		if(pTextureCoordinate[i].X() < smallest)
		{
			smallestIndex_x = i;
			smallest = pTextureCoordinate[i].X();
		}
		if(pTextureCoordinate[i].X() > largest)
		{
			largestIndex_x = i;
			largest = pTextureCoordinate[i].X();
		}
	}
	// Y
	smallest = FLT_MAX;
	largest = FLT_MIN;
	for(U32 i = 0; i < count; i++)
	{
		if(pTextureCoordinate[i].Y() < smallest)
		{
			smallestIndex_y = i;
			smallest = pTextureCoordinate[i].Y();
		}
		if(pTextureCoordinate[i].Y() > largest)
		{
			largestIndex_y = i;
			largest = pTextureCoordinate[i].Y();
		}
	}

	F32 x = pTextureCoordinate[smallestIndex_x].X();
	F32 y = pTextureCoordinate[smallestIndex_y].Y();

	// Shift down
	for(U32 i = 0; i < count; i++)
	{
		pTextureCoordinate[i].x -= x;
		pTextureCoordinate[i].y -= y;
	}
}

// Convex polygons
void NavMeshNode::BuildGenericCellList()
{
	for(Btl::List<PreprocTri*>::Iterator it = m_GenericTriList.Begin(); it != m_GenericTriList.End(); ++it)
	{
		Btl::List<PreprocAdjacentTriangle*>	adjacentList;

		if((*it)->m_Stored)
			continue;

		// Build list of adjacent triangles to me
		PreprocAdjacentTriangle *pHeadNode = new PreprocAdjacentTriangle( *it, &m_AdjacencyList, &m_GenericTriList );
		pHeadNode->FindRecursive( adjacentList );

		PreprocPolygon preprocPolygon( m_pAftkSceneMeshNode, adjacentList );

		// Split polygon in convex parts
		preprocPolygon.SplitToConvex( );

		// Store convex parts in cell list
		preprocPolygon.ExportToCells( m_CellList, (*it)->m_AftkPolygonSourceIdx );
		
		delete pHeadNode;
	}
}

// Build new vertex list to export and remap vertex indices
void NavMeshNode::RemapVertices( )
{
	const U32 referenceUnmapped = 0xffffffff;
	const U32 numVertices = m_pAftkSceneMeshNode->GetNumVertexPositions();

	// Clear reference array
	Btl::Vector<U32> referenceList;
	referenceList.Resize( numVertices );
	for(U32 i = 0; i < numVertices; i++)
	{
		referenceList[i] = referenceUnmapped;
	}

	// Remap and store
	for(Btl::List<ExportCell*>::Iterator it = m_CellList.Begin(); it != m_CellList.End(); ++it)
	{
		for(U32 i = 0; i < (*it)->m_EdgeCount; i++)
		{
			U32 myIndex = (*it)->m_pEdge[i].m_VertexIndex;
			
			if(referenceList[myIndex] != referenceUnmapped)
			{
				(*it)->m_pEdge[i].m_VertexIndex = referenceList[myIndex];
				continue;
			}

			referenceList[myIndex] = m_VertexList.Size();
			(*it)->m_pEdge[i].m_VertexIndex = m_VertexList.Size();

			// Store new vertex
			Aftk::Position3 aftkPos = m_pAftkSceneMeshNode->GetVertexPosition( myIndex );
			Maths::C_Vertex vec( aftkPos.X(), aftkPos.Y(), aftkPos.Z() );

			ExportVertex *pVertex = new ExportVertex( vec );
			

			Aftk::Position3 aftkPos2;
			if( i+1 < (*it)->m_EdgeCount)
			{
				 aftkPos2 = m_pAftkSceneMeshNode->GetVertexPosition( (*it)->m_pEdge[i+1].m_VertexIndex );
			}
			else
			{
				 aftkPos2 = m_pAftkSceneMeshNode->GetVertexPosition( (*it)->m_pEdge[0].m_VertexIndex );
			}
			(*it)->m_pEdge[i].m_Middle = (vec + Maths::C_Vertex(aftkPos2.X(), aftkPos2.Y(), aftkPos2.Z())) * 0.5f;
			
			m_VertexList.PushBack( pVertex );
		}
	}
}

void NavMeshNode::LinkCells()
{
	for ( Btl::List<ExportCell*>::Iterator it = m_CellList.Begin(); it != m_CellList.End(); ++it )
	{
		for( U32 i = 0; i < (*it)->m_EdgeCount; i++)
		{
			(*it)->LinkAdjacentCells(m_CellList);
		}
	}
}

void NavMeshNode::CalcCellCentresAndOuterEdges()
{
	// Eek convert link list into lookup array for random access
	Btl::Vector<ExportVertexLink> vertexList;

	vertexList.Resize(m_VertexList.Size());
	U32 count = 0;
	for(Btl::List<ExportVertex*>::Iterator it = m_VertexList.Begin(); it != m_VertexList.End(); ++it, count++)
	{
		vertexList[count].m_pLink = (*it);
	}

	for(Btl::List<ExportCell*>::Iterator it = m_CellList.Begin(); it != m_CellList.End(); ++it)
	{
		ExportCellEdge *p((*it)->m_pEdge);
		for(U32 i = 0; i < (*it)->m_EdgeCount; i++, p++)
		{
			(*it)->m_Middle = (*it)->m_Middle + vertexList[p->m_VertexIndex].m_pLink->m_Position;
		}

		(*it)->m_Middle = (*it)->m_Middle * (1.0f / (F32)(*it)->m_EdgeCount);
	}

	for(Btl::List<ExportCell*>::Iterator it = m_CellList.Begin(); it != m_CellList.End(); ++it)
	{
		ExportCell* pCell = *it;

		for(U32 nEdgeCurr = 0; nEdgeCurr < pCell->m_EdgeCount; nEdgeCurr++)
		{
			ExportCellEdge* pEdgeCurr = &pCell->m_pEdge[nEdgeCurr];
			if(pEdgeCurr->m_pNextCell == NULL)
			{
				vertexList[pEdgeCurr->m_VertexIndex].m_pLink->m_OuterPoint = 1;
			}
		}
	}
}


static const char *validMaterialName[E_AICellType_Count]=
{
	"GENERIC",		// E_AICellType_Generic
	"WATER",		// E_AICellType_Water
	"VAULT",		// E_AICellType_Vault
	"DROP",			// E_AICellType_Drop
	"LANDING",		// E_AICellType_Landing
	"DOORWAY",		// E_AICellType_Doorway
	"BLOCKED",		// E_AICellType_Blocked
};

const U16 NavMeshNode::TypeFromMaterial(Aftk::C_Polygon* pPolygon)
{
	assert(pPolygon);

	Util::C_String fileName;
	S16 materialIndex = pPolygon->GetMaterialMappingIndex();

	const Aftk::C_SceneMeshNode* pAftkSceneMeshNode = pPolygon->GetMeshNodePtr();
	const Aftk::C_SharedResource<Aftk::C_MaterialNode> *pMaterialResource = pAftkSceneMeshNode->GetPolygonMaterialMappingPtr( materialIndex );
	if(pMaterialResource)
	{
		const Aftk::C_MaterialNode* pAftkMaterialNode = &pMaterialResource->GetContent();
		if(pAftkMaterialNode)
		{
			U32 noShaderImageNodes = pAftkMaterialNode->GetNumShaderImageNodes();
			if(noShaderImageNodes == 1)
			{
				const Aftk::C_ShaderImageNode* pImageNode = pAftkMaterialNode->GetShaderImageNodePtr(0);
				if (pImageNode)
				{
					std::string filename;
					pImageNode->GetFilePlug().GetValue(filename);
					fileName = filename.c_str();

					//fileName = Util::GetInstanceName(fileName.Upper());
					fileName = Util::ExtractFilename(fileName);
					fileName = Util::RemoveFileExtension(fileName);

					for(U32 i = 0; i < E_AICellType_Count; i++)
					{
						if(stricmp(fileName,validMaterialName[i]) == 0)
						{
							return(E_AICellType_Generic + i);
						}
					}
				}
			}
		}
	}

	static S32 errorCount = 0;
	errorCount ++;
	if (errorCount >= 10)
	{
		if (errorCount == 10)
		{
			LOGERROR(Tools::g_log, "More than 10 'Could not validate nav mesh texture' errors detected, error disabled!");
		}
	}
	else
	{
		//Return error and description
		if(pPolygon->GetNumPolygonVertices())
		{
			const Aftk::C_PolygonVertex* pPolyVertex = &pPolygon->GetPolygonVertex(0);
			Aftk::Position3 aftkPos = m_pAftkSceneMeshNode->GetVertexPosition(pPolyVertex->GetPositionIndex());
			Maths::C_Vertex vec = Maths::C_Vertex(aftkPos.X(), aftkPos.Y(), aftkPos.Z());
			LOGERROR(Tools::g_log, "Could not validate nav mesh texture for mesh [%s] with vertex (%.2f, %.2f, %.2f). Check you've assigned the correct texture in Maya.", m_pAftkSceneMeshNode->GetName().c_str(), (F32)vec.X(), (F32)vec.Y(), (F32)vec.Z());
		}
		else
		{
			LOGERROR(Tools::g_log, "Could not validate nav mesh texture for mesh [%s]. Check you've assigned the correct texture in Maya.", m_pAftkSceneMeshNode->GetName().c_str());
		}
	}
	return( E_AICellType_NotDefined );
}

NavMeshNode::~NavMeshNode()
{
	for(Btl::List<ExportVertex*>::Iterator it = m_VertexList.Begin(); it != m_VertexList.End(); ++it)
		delete (*it);

	for(Btl::List<ExportCell*>::Iterator it = m_CellList.Begin(); it != m_CellList.End(); ++it)
		delete (*it);

	for(Btl::List<PreprocTri*>::Iterator it = m_GenericTriList.Begin(); it != m_GenericTriList.End(); ++it)
		delete (*it);

	for(Btl::List<C_SectionCellCount*>::Iterator it = m_SectionCellCount.Begin(); it != m_SectionCellCount.End(); ++it)
		delete (*it);
	
}

} // namespace NavMesh

#ifdef USER_PETERB
#pragma optimize("", on)
#endif
