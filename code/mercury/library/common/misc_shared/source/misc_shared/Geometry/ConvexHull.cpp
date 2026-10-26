#include "MiscSharedPch.h"

#include <system/debug/I_DebugRender.h>
#include <system/DebugLog.h>
#include <misc_shared/geometry/convexHull.h>

//#define WRITE_CONVEX_HULL_POINTS

#ifdef WRITE_CONVEX_HULL_POINTS
#include <system/serialise.h>
#endif


#define CONVEX_HULL_VERTEX_WELD_DIST			0.01f
#define CONVEX_HULL_VERTEX_COINCIDENT_DIST		0.0001f
#define CONVEX_HULL_PLANE_DIST 					0.001f
#define CONVEX_HULL_DELETE_POLY_PLANE_DIST		0.0001f


namespace ConvexHull
{

// create logger
Debug::Logger &g_Log = Debug::GetLogger("mercury.misc_shared.convexhull");

// C_Polygon

C_Polygon::C_Polygon(C_Mesh* pMesh)
	:	m_pMesh(pMesh),
		m_Building(true),
		m_Index(-1)
{
}

C_Polygon::~C_Polygon()
{
	// reassign source vertices
	while(m_SourceVertexList.Size())
	{
		m_pMesh->m_SourceVertexList.PushBack(m_SourceVertexList.Back());
		m_SourceVertexList.PopBack();
	}

	// remove edge references
	C_EdgeItem* pCurrEdgeItem = m_EdgeItemList.First();
	while(pCurrEdgeItem)
	{
		C_EdgeItem* pNextEdgeItem = pCurrEdgeItem->Next();;
		C_Edge* pCurrEdge = *pCurrEdgeItem;
		pCurrEdge->RemovePolygonRef(this);
		delete pCurrEdgeItem;
		pCurrEdgeItem = pNextEdgeItem;
	}

	// remove vertex references
	C_VertexItem* pCurrVertexItem = m_VertexItemList.First();
	while(pCurrVertexItem)
	{
		C_VertexItem* pNextVertexItem = pCurrVertexItem->Next();
		C_Vertex* pVertex = *pCurrVertexItem;
		pVertex->RemovePolygonRef(this);
		delete pCurrVertexItem;
		pCurrVertexItem = pNextVertexItem;
	}
}


void C_Polygon::AddVertex(C_Vertex* pNewVertex)
{
	assert(m_Building);

	C_VertexItem* pNewVertexItem = new C_VertexItem(pNewVertex);
	m_VertexItemList.PushLast(pNewVertexItem);

	C_PolygonItem* pNewPolygonItem = new C_PolygonItem(this);
	pNewVertex->m_PolygonItemList.PushLast(pNewPolygonItem);
}

void C_Polygon::EndConstruction()
{
	m_Building = false;

	// create edge information for the polygon
	{
		C_VertexItem* pCurrVertexItem = m_VertexItemList.First();
		C_VertexItem* pPrevVertexItem = m_VertexItemList.Last();

		while(pCurrVertexItem)
		{
			C_Edge* pNewEdge = m_pMesh->AddEdge( *pPrevVertexItem, *pCurrVertexItem );

			m_EdgeItemList.PushLast( new C_EdgeItem(pNewEdge) );
			pNewEdge->m_PolygonItemList.PushLast( new C_PolygonItem(this) );

			pPrevVertexItem = pCurrVertexItem;
			pCurrVertexItem = pCurrVertexItem->Next();
		}
	}

	// calculate polygon plane
	const S32 vertexCount = m_VertexItemList.Size();
	if(vertexCount < 3)
	{
		LOGWARN( g_Log, "Convex Hull calculate polygon plane failed, vertex count < 3\n");
		return;
	}

	// use Newell's method to calculate polygon normal

	{
		Maths::HVec3 normal( 0.0f );
		Maths::HVec3 centroid = Maths::HVec3( 0.0f );

		C_VertexItem* pPrevVertexItem = m_VertexItemList.Last();
		C_VertexItem* pCurrVertexItem = m_VertexItemList.First();
		while(pCurrVertexItem)
		{
			const Maths::HVec3 prevPos = (*pPrevVertexItem)->m_Pos;
			const Maths::HVec3 currPos = (*pCurrVertexItem)->m_Pos;

			Maths::HF32 nX = (currPos.Y() - prevPos.Y()) * (currPos.Z() + prevPos.Z());
			Maths::HF32 nY = (currPos.Z() - prevPos.Z()) * (currPos.X() + prevPos.X());
			Maths::HF32 nZ = (currPos.X() - prevPos.X()) * (currPos.Y() + prevPos.Y());

			normal += Maths::HVec3( nX, nY, nZ );

			centroid += currPos;

			pPrevVertexItem = pCurrVertexItem;
			pCurrVertexItem = pCurrVertexItem->Next();
		}

		centroid *= (1.0f / (F32)vertexCount);

		m_Plane = Maths::C_Plane( Maths::C_Normal( normal ), Maths::C_Vertex( centroid ) ); 

	}

	/*
	{
		C_VertexItem* pFirstVertexItem = m_VertexItemList.First();
		C_VertexItem* pAItem = pFirstVertexItem->Next();
		C_VertexItem* pBItem = pAItem->Next();

		bool done = false;
		while(!done)
		{
			Maths::HVec3 OA = (*pAItem)->m_Pos - (*pFirstVertexItem)->m_Pos;
			Maths::HVec3 OB = (*pBItem)->m_Pos - (*pFirstVertexItem)->m_Pos;

			Maths::HVec3 cross = OA ^ OB;
			if( Length(cross) > 0.000001f)
			{
				m_Plane = Maths::C_Plane( (*pFirstVertexItem)->m_Pos, (*pBItem)->m_Pos, (*pAItem)->m_Pos );
				done = true;
			}

			if(!done)
			{
				pAItem = pAItem->Next();
				pBItem = pBItem->Next();

				if(pBItem == NULL)
				{
					done = true;

					LOGWARN( g_Log, "Calculate polygon plane failed\n");
				}
			}
		}
	}

	*/
}

const Maths::HVec3 C_Polygon::GetCentroid()
{
	F32 total = 0.0f;
	Maths::HVec3 result(0.0f, 0.0f, 0.0f);

	C_VertexItem* pCurrVertexItem = m_VertexItemList.First();
	while(pCurrVertexItem)
	{
		result += (*pCurrVertexItem)->m_Pos;
		total += 1.0f;
		pCurrVertexItem = pCurrVertexItem->Next();
	}
	

	return (result * (1.0f / total));
}

S32 C_Polygon::GetEdgeWindingOrder(C_Edge* pEdge)
{
	C_VertexItem* pPrevVertexItem = m_VertexItemList.Last();
	C_VertexItem* pCurrVertexItem = m_VertexItemList.First();

	while(pCurrVertexItem)
	{
		if(((*pPrevVertexItem) == pEdge->m_pVertices[0]) && ((*pCurrVertexItem) == pEdge->m_pVertices[1])) return 1;
		if(((*pPrevVertexItem) == pEdge->m_pVertices[1]) && ((*pCurrVertexItem) == pEdge->m_pVertices[0])) return 0;

		pPrevVertexItem = pCurrVertexItem;
		pCurrVertexItem = pCurrVertexItem->Next();	
	}
	
	LOGFATAL( g_Log, "GetEdgeWindingOrder() - edge not in polygon\n");

	return -1;
}

bool C_Polygon::IsCoplanar(C_Polygon* pOther) const
{
	// check if poly on other side is coplanar to this one

	C_VertexItem* pCurrPolyVertexItem = pOther->m_VertexItemList.First();
	while(pCurrPolyVertexItem)
	{
		C_Vertex* pCurrPolyVertex = *pCurrPolyVertexItem;
		F32 vertexDist = m_Plane.Distance( Maths::C_Vertex(pCurrPolyVertex->m_Pos) );
		if((vertexDist > CONVEX_HULL_PLANE_DIST) || (vertexDist < -CONVEX_HULL_PLANE_DIST))
		{
			return false;
		}
		pCurrPolyVertexItem = pCurrPolyVertexItem->Next();
	}

	return true;
}

S32 C_Polygon::MergeCoplanarNeighbours()
{
	//return 0;

	C_EdgeItem* pCurrEdgeItem = m_EdgeItemList.First();
	while(pCurrEdgeItem)
	{
		C_Edge* pCurrEdge = *pCurrEdgeItem;

		C_PolygonItem* pCurrPolygonItem = pCurrEdge->m_PolygonItemList.First();
		while(pCurrPolygonItem)
		{
			C_Polygon* pOtherPolygon = *pCurrPolygonItem;

			if(pOtherPolygon != this)
			{
				if( IsCoplanar(pOtherPolygon) &&  pOtherPolygon->IsCoplanar(this) )
				{
					// generate a new poly which is the merged result of 'this' and 'poly'

					C_Polygon* pNewPolygon = m_pMesh->NewPolygon();

					// find first clockwise vertex on edge which is on only one polygon
					C_VertexItem* pFirstEdgePrevVertexItem = m_VertexItemList.Last();
					C_VertexItem* pFirstEdgeCurrVertexItem = m_VertexItemList.First();
					C_VertexItem* pFirstVertexItem = NULL;
					while(pFirstEdgeCurrVertexItem && !pFirstVertexItem)
					{
						if(!pOtherPolygon->FindEdge( *pFirstEdgePrevVertexItem, *pFirstEdgeCurrVertexItem ))
						{
							pFirstVertexItem = pFirstEdgeCurrVertexItem;
						}
						
						pFirstEdgePrevVertexItem = pFirstEdgeCurrVertexItem;
						pFirstEdgeCurrVertexItem = pFirstEdgeCurrVertexItem->Next();
					}

					if(pFirstVertexItem == NULL)
					{
						LOGERROR( g_Log, "Problem processing convex hull %s", m_pMesh->m_Name.Chars());
						m_pMesh->m_BuildState = E_BuildState_Error;
						return 0;
					}

					C_Polygon* pCurrPoly = this;
					C_Polygon* pTestPoly = pOtherPolygon;

					C_VertexItem* pCurrVertexItem = pFirstVertexItem;
					C_VertexItem* pNextVertexItem = pCurrVertexItem->Next();
					if(pNextVertexItem == NULL) pNextVertexItem = pCurrPoly->m_VertexItemList.First();

					bool firstTime = true;

					while(( *pCurrVertexItem != *pFirstVertexItem ) || firstTime)
					{
						firstTime = false;

						// output curr vertex
						pNewPolygon->AddVertex( *pCurrVertexItem );

						// check if edge curr->next is on test poly
						if(pTestPoly->FindEdge( *pCurrVertexItem, *pNextVertexItem ))
						{
							// move to test poly
							C_Polygon* pTempPoly = pCurrPoly;
							pCurrPoly = pTestPoly;
							pTestPoly = pTempPoly;

							// set next ptr to curr ptr on new curr_poly
							C_VertexItem* pSearchVertexItem = pCurrPoly->m_VertexItemList.First();
							while( *pSearchVertexItem != *pCurrVertexItem )
							{
								pSearchVertexItem = pSearchVertexItem->Next();
								if(pSearchVertexItem == NULL) pSearchVertexItem = pCurrPoly->m_VertexItemList.First();
							}

							pCurrVertexItem = pSearchVertexItem;

							pNextVertexItem = pCurrVertexItem->Next();
							if(pNextVertexItem == NULL) pNextVertexItem = pCurrPoly->m_VertexItemList.First();
						}

						// advance vertex ptrs
						pCurrVertexItem = pNextVertexItem;

						pNextVertexItem = pCurrVertexItem->Next();
						if(pNextVertexItem == NULL) pNextVertexItem = pCurrPoly->m_VertexItemList.First();
					}


					pNewPolygon->EndConstruction();

					delete pOtherPolygon;
					delete this;

					pNewPolygon->MergeCoplanarNeighbours();

					return 1;
				}
			}


			pCurrPolygonItem = pCurrPolygonItem->Next();
		}

		pCurrEdgeItem = pCurrEdgeItem->Next();
	}
	
	return 0;
}

C_Edge* C_Polygon::FindEdge( C_Vertex* pA, C_Vertex* pB )
{
	C_EdgeItem* pCurrEdgeItem = m_EdgeItemList.First();
	while(pCurrEdgeItem)
	{
		C_Edge* pCurrEdge = *pCurrEdgeItem;

		if( ((pCurrEdge->m_pVertices[0] == pA) && (pCurrEdge->m_pVertices[1] == pB)) ||
			((pCurrEdge->m_pVertices[0] == pB) && (pCurrEdge->m_pVertices[1] == pA)) )
			return pCurrEdge;
		

		pCurrEdgeItem = pCurrEdgeItem->Next();
	}	

	return NULL;
}

// C_Edge

C_Edge::C_Edge(C_Mesh* pMesh, C_Vertex* pA, C_Vertex* pB)
{
	m_Index = -1;

	m_pMesh = pMesh;
	m_pVertices[0] = pA;
	m_pVertices[1] = pB;

	pA->m_EdgeItemList.PushLast(new C_EdgeItem(this));
	pB->m_EdgeItemList.PushLast(new C_EdgeItem(this));
}

C_Edge::~C_Edge()
{
	if(m_PolygonItemList.Elements() != 0)
	{
		LOGERROR( g_Log, "C_Edge::~C_Edge() - polygon list not empty");
	}
	
	m_pVertices[0]->RemoveEdgeRef(this);
	m_pVertices[1]->RemoveEdgeRef(this);

	m_pVertices[0] = NULL;
	m_pVertices[1] = NULL;
}

void C_Edge::RemovePolygonRef(C_Polygon* pPolygon)
{
	C_PolygonItem* pCurrPolygonItem = m_PolygonItemList.First();
	while(pCurrPolygonItem)
	{
		C_Polygon* pCurrPolygon = *pCurrPolygonItem;
		if(pCurrPolygon == pPolygon)
		{
			delete pCurrPolygonItem;

			if(m_PolygonItemList.Elements() == 0)
			{
				delete this;
			}

			return;
		}
				
		pCurrPolygonItem = pCurrPolygonItem->Next();
	}

	LOGERROR( g_Log, "C_Edge::RemovePolygonRef() - polygon not found in edge");
}

const S32 C_Edge::GetVertexIndex(C_Vertex* pVertex)
{
	if(m_pVertices[0] == pVertex) 
		return 0;
	if(m_pVertices[1] == pVertex) 
		return 1;
	return -1;
}

const S32 C_Edge::GetPolygonIndex(C_Polygon* pPolygon)
{
	S32 index = 0;
	C_PolygonItem* pCurrPolygonItem = m_PolygonItemList.First();
	while(pCurrPolygonItem)
	{	
		C_Polygon* pCurrPolygon = *pCurrPolygonItem;
		if(pCurrPolygon == pPolygon)
			return index;

		index++;

		pCurrPolygonItem = pCurrPolygonItem->Next();
	}

	return -1;
}

Maths::HVec3 C_Edge::GetMidPoint()
{
	return ((m_pVertices[0]->m_Pos + m_pVertices[1]->m_Pos) * 0.5f);
}

// C_Vertex

C_Vertex::C_Vertex(C_Mesh* pMesh, const Maths::HVec3& pos)
	:	m_pMesh(pMesh),
		m_Pos(pos),
		m_Index(-1)
{
}

C_Vertex::~C_Vertex()
{
	if(m_PolygonItemList.Elements() != 0)
	{
		LOGERROR( g_Log, "C_Vertex::~C_Vertex() - polygon list not empty");
	}

	if(m_EdgeItemList.Elements() != 0)
	{
		LOGERROR( g_Log, "C_Vertex::~C_Vertex() - edge list not empty");
	}
}

void C_Vertex::RemovePolygonRef(C_Polygon* pPolygon)
{
	C_PolygonItem* pCurrPolygonItem = m_PolygonItemList.First();
	while(pCurrPolygonItem)
	{
		C_Polygon* pCurrPolygon = *pCurrPolygonItem;
		if(pCurrPolygon == pPolygon)
		{
			delete pCurrPolygonItem;

			if(m_PolygonItemList.Elements() == 0)
			{
				if(m_EdgeItemList.Elements() == 0)
				{
					delete this;
				}
			}

			return;
		}
				
		pCurrPolygonItem = pCurrPolygonItem->Next();
	}

	LOGERROR( g_Log, "C_Vertex::RemovePolygonRef() - polygon not found in vertex");

}

void C_Vertex::RemoveEdgeRef(C_Edge* pEdge)
{
	C_EdgeItem* pCurrEdgeItem = m_EdgeItemList.First();
	while(pCurrEdgeItem)
	{
		C_Edge* pCurrEdge = *pCurrEdgeItem;
		if(pCurrEdge == pEdge)
		{
			delete pCurrEdgeItem;

			if(m_EdgeItemList.Elements() == 0)
			{
				if(m_PolygonItemList.Elements() == 0)
				{
					delete this;
				}
			}

			return;
		}
				
		pCurrEdgeItem = pCurrEdgeItem->Next();
	}

	LOGERROR( g_Log, "C_Vertex::RemoveEdgeRef() - edge not found in vertex");
}

// C_Mesh

C_Mesh::C_Mesh()
	:	m_pSelectedPolygon(NULL),
		m_BuildState(E_BuildState_NotBegun),
		m_StepNo(0)
{
}

C_Mesh::~C_Mesh()
{
	m_PolygonList.DeleteAll();
	m_EdgeList.DeleteAll();
	m_VertexList.DeleteAll();
	//m_SourceVertexList.DeleteAll();
}

void C_Mesh::SetName( const Util::C_String& name )
{
	m_Name = name;
}

C_Vertex* C_Mesh::NewVertex(const Maths::HVec3& pos)
{
	// Should we ever get shared vertices as we have already welded the input points and
	// the algorithm reuses existing points?
	C_Vertex* pCurrVertex = m_VertexList.First();
	while(pCurrVertex)
	{		
		if(pos.Distance(pCurrVertex->m_Pos) < (CONVEX_HULL_VERTEX_COINCIDENT_DIST))
			return pCurrVertex;
		
		pCurrVertex = pCurrVertex->Next();
	}

	C_Vertex* pNewVertex = new C_Vertex(this, pos);
	m_VertexList.PushLast(pNewVertex);

	return pNewVertex;
}

C_Polygon* C_Mesh::NewPolygon()
{
	C_Polygon* pNewPolygon = new C_Polygon(this);
	m_PolygonList.PushLast( pNewPolygon );
	return pNewPolygon;
}

void C_Mesh::DeletePolygon(C_Polygon* pPolygon)
{
	delete pPolygon;
}

void C_Mesh::DebugRender( DbUtil::I_DebugRenderPtr debugRenderPtr, Maths::HMat43& m )
{
	DebugRenderMesh( debugRenderPtr, m );
	DebugRenderEdges( debugRenderPtr, m );
	DebugRenderDataStructure( debugRenderPtr, m );
	DebugRenderSourceVertices( debugRenderPtr, m );
}

void C_Mesh::DebugRenderMesh( DbUtil::I_DebugRenderPtr debugRenderPtr, Maths::HMat43& m )
{
	Maths::C_RGBA_F32 baseColour(0.0f, 0.25f, 1.0f, 0.0f );
	Maths::C_RGBA_F32 selectedColour(1.0f, 0.5f, 0.5f, 0.0f );	
	S32 wireframe = 0;

	//Maths::C_RGBA_F32 shadowColour( (U8)(colour.R() >> 2), (U8)(colour.G() >> 2), (U8)(colour.B() >> 2), (U8)colour.A());
	//Maths::C_RGBA_F32 specColour( 0xff, 0xff, 0xff, 0x80);

	Maths::HVec3 camDir(0.0f, 1.0f, 0.0f);//gSceneSettings.camera.world_matrix.GetAW() - m.GetAW();
	camDir.Normalise();

	Maths::HVec3 lightDir(-0.5f, 1.5f, -1.0f);
	lightDir.Normalise();

	Maths::HMat33 rotM(m);
	//Maths::HMat33 invRotM = (rotM).Inverse();

	//lightDir = lightDir * invRotM;
	//camDir = camDir * invRotM;

	Maths::HVec3 halfAngle = lightDir + camDir;
	halfAngle.Normalise();

	bool drawNormals = true;

	C_Polygon* pCurrPolygon = m_PolygonList.First();
	while(pCurrPolygon)
	{
		if((wireframe == 0) || (wireframe == 2))
		{

			Maths::C_RGBA_F32 colour;
			if(pCurrPolygon == m_pSelectedPolygon)
			{
				colour = selectedColour;
			}
			else
			{
				colour = baseColour;
			}

			Maths::HVec3 polyNormal = pCurrPolygon->m_Plane.Normal();
			polyNormal = polyNormal * rotM;
			polyNormal.Normalise();

			Maths::HF32 diffuseIntensity;
			diffuseIntensity = polyNormal | lightDir;
			diffuseIntensity = diffuseIntensity * 0.5f + 0.5f;
			diffuseIntensity = diffuseIntensity.Clamp(0.0f, 1.0f);
			//diffuseIntensity = diffuseIntensity + 0.1f;

			Maths::HF32 specIntensity;
			specIntensity = polyNormal | halfAngle;
			specIntensity = specIntensity.Clamp(0.0f, 1.0f);
			specIntensity = specIntensity * specIntensity * specIntensity * specIntensity * specIntensity;

			Maths::C_RGBA_F32 col = colour * diffuseIntensity + Maths::C_RGBA_F32(1.0f, 1.0f, 1.0f, 0.0f) * specIntensity;
			col = col.Clamp( Maths::C_RGBA_F32(0.0f, 0.0f, 0.0f, 0.0f), Maths::C_RGBA_F32(1.0f, 1.0f, 1.0f, 1.0f) );

			/*
			if(wireframe == 2)
			{
				currColour = RGBA(0x00, 0x00, 0x00, 0x80);
			}
			*/


			Maths::HVec3 firstPos, prevPos, currPos;

			C_VertexItem* pFirstVertexItem = pCurrPolygon->m_VertexItemList.First();
			C_VertexItem* pPrevVertexItem = pFirstVertexItem->Next();
			C_VertexItem* pCurrVertexItem = pPrevVertexItem->Next();

			firstPos = (*pFirstVertexItem)->m_Pos * m;
			prevPos = (*pPrevVertexItem)->m_Pos * m;

			while(pCurrVertexItem)
			{
				currPos = (*pCurrVertexItem)->m_Pos * m;

				debugRenderPtr->Draw3dTriangle(firstPos, currPos, prevPos, (Maths::RGBA)col);

				prevPos = currPos;
				pPrevVertexItem = pCurrVertexItem;
				pCurrVertexItem = pCurrVertexItem->Next();
			}

			if(drawNormals)
			{
				Maths::HVec3 centre(0.0f, 0.0f, 0.0f);
				F32 noPoints = 0.0f;

				C_VertexItem* pCurrVertexItem = pCurrPolygon->m_VertexItemList.First();

				while(pCurrVertexItem)
				{
					currPos = (*pCurrVertexItem)->m_Pos * m;

					centre += currPos;
					noPoints += 1.0f;

					pCurrVertexItem = pCurrVertexItem->Next();
				}
				centre = centre * (1.0f / noPoints);
				debugRenderPtr->Draw3dArrow(centre, polyNormal, 0.1f, 0.02f, Maths::RGBA(255, 255, 255, 255) );
			}
		}

		/*
		if(wireframe)
		{
			C_VertexItem* pPrevVertexItem = pCurrPolygon->m_VertexItemList.Last();
			C_VertexItem* pCurrVertexItem = pCurrPolygon->m_VertexItemList.First();

			Maths::HVec3 prevPos, currPos;

			prevPos = (*pPrevVertexItem)->m_Pos * m;
			while(pCurrVertexItem)
			{
				currPos = (*pCurrVertexItem)->m_Pos * m;

				debugRenderPtr->Draw3dLine(prevPos, currPos, colour);

				prevPos = currPos;
				pPrevVertexItem = pCurrVertexItem;
				pCurrVertexItem = pCurrVertexItem->Next();
			}
		}
		*/

		pCurrPolygon = pCurrPolygon->Next();
	}
}

void C_Mesh::DebugRenderEdges( DbUtil::I_DebugRenderPtr debugRenderPtr, Maths::HMat43& m )
{
	Maths::RGBA openColour(0xff, 0xff, 0x00, 0xff);
	Maths::RGBA closedColour(0xff, 0xff, 0xff, 0xff);

	C_Edge* pCurrEdge = m_EdgeList.First();
	while(pCurrEdge)
	{
		if(pCurrEdge->m_PolygonItemList.Elements() == 1)
		{
			debugRenderPtr->Draw3dLine(pCurrEdge->m_pVertices[0]->m_Pos * m, pCurrEdge->m_pVertices[1]->m_Pos * m, openColour);			
		}
		else
		{
			debugRenderPtr->Draw3dLine(pCurrEdge->m_pVertices[0]->m_Pos * m, pCurrEdge->m_pVertices[1]->m_Pos * m, closedColour);			
		}

		pCurrEdge = pCurrEdge->Next();
	}
}

void C_Mesh::DebugRenderDataStructure( DbUtil::I_DebugRenderPtr debugRenderPtr, Maths::HMat43& m )
{

	//S32 index = 0;

	//C_Vertex* pCurrVertex = m_VertexList.First();
	//while(pCurrVertex)
	//{
	//	//debugRenderPtr->Draw3dPoint( pCurrVertex->m_Pos * m, 0.1f, Maths::RGBA(0xff, 0x00, 0x00, 0xff) );

	//	Util::C_String text;
	//	text.Printf("%d", index);

	//	debugRenderPtr->Draw3dText( pCurrVertex->m_Pos * m, text );

	//	index++;
	//	pCurrVertex = pCurrVertex->Next();
	//}

}

void C_Mesh::DebugRenderSourceVertices( DbUtil::I_DebugRenderPtr debugRenderPtr, Maths::HMat43& m )
{
	C_Polygon* pCurrPolygon = m_PolygonList.First();
	while(pCurrPolygon)
	{
		Maths::RGBA colour;
		F32 size;

		if(pCurrPolygon == m_pSelectedPolygon)
		{
			colour = Maths::RGBA(0xff, 0xff, 0x00, 0xff);
			size = 0.05f;
		}
		else
		{
			colour = Maths::RGBA(0x00, 0xff, 0xff, 0xff);
			size = 0.01f;
		}

		for(S32 i=0; i<pCurrPolygon->m_SourceVertexList.Size(); i++)
		{
			C_SourceVertex& vertex = pCurrPolygon->m_SourceVertexList[i];

			debugRenderPtr->Draw3dPoint( vertex * m, size, colour );
		}


		pCurrPolygon = pCurrPolygon->Next();
	}

	switch(m_BuildState)
	{
		default:
		break;

		case E_BuildState_RemoveSelectedPoly:
		case E_BuildState_CreateNewPolygons:
		case E_BuildState_MergeNeighbourPolygons:
			debugRenderPtr->Draw3dPoint( m_SelectedVertex * m, 0.1f, Maths::RGBA(0xff, 0, 0, 0xff) );
		break;
	}


	{
		for(S32 i=0; i<m_SourceVertexList.Size(); i++)
		{
			C_SourceVertex& vertex = m_SourceVertexList[i];
			debugRenderPtr->Draw3dPoint( vertex * m, 0.05f, Maths::RGBA(0xff, 0xff,0xff, 0xff) );
		}
	}
}

C_Edge* C_Mesh::FindEdge(C_Vertex* pA, C_Vertex* pB)
{
	C_EdgeItem* pCurrEdgeItem = pA->m_EdgeItemList.First();
	while(pCurrEdgeItem)
	{
		C_Edge* pCurrEdge = *pCurrEdgeItem;

		if( ((pCurrEdge->m_pVertices[0] == pA) && (pCurrEdge->m_pVertices[1] == pB)) ||
			((pCurrEdge->m_pVertices[0] == pB) && (pCurrEdge->m_pVertices[1] == pA)) )
			return pCurrEdge;
		

		pCurrEdgeItem = pCurrEdgeItem->Next();
	}	

	return NULL;
}

C_Edge* C_Mesh::AddEdge(C_Vertex* pA, C_Vertex* pB)
{
	C_Edge* pCurrEdge = FindEdge(pA, pB);

	if(pCurrEdge == NULL)
	{
		pCurrEdge = new C_Edge(this, pA, pB);
		m_EdgeList.PushLast(pCurrEdge);
	}

	return pCurrEdge;
}

void C_Mesh::AddSourceVertexArray(Maths::HVec3* posList, S32 posCount)
{
	for(S32 i=0; i<posCount; i++)
	{
		m_SourceVertexList.PushBack( posList[i] );
	}
}

void C_Mesh::Build()
{
	#ifdef WRITE_CONVEX_HULL_POINTS
	{
		Sys::I_File* pFile = Sys::GetDefaultFileSystem()->Open("z:\\ConvexHullPoints.dat", Sys::E_FileAccess_Write );

		if(pFile)
		{
			pFile->SetEndianSwapFlag( true );
			{
				Sys::BinarySerialiser<Ut::DoEndianSwap> ar(*pFile);

				S32 sourceVertexCount = m_SourceVertexList.Size();

				ar << sourceVertexCount;

				for(S32 i=0; i<sourceVertexCount; i++)
				{
					F32 x = m_SourceVertexList[i].X();
					F32 y = m_SourceVertexList[i].Y();
					F32 z = m_SourceVertexList[i].Z();
					ar << x;
					ar << y;
					ar << z;
				}
			}

			SAFE_DELETE_PTR(pFile);
		}		
	}
	#endif
	

	while(!StepBuildingConvexHull())
	{
	}
}

bool C_Mesh::StepBuildingConvexHull()
{
	// find a polygon with a point infront of it

	switch(m_BuildState)
	{
		default:
		break;

		case E_BuildState_NotBegun:
			m_BuildState = E_BuildState_WeldVertices;
		break;			

		case E_BuildState_WeldVertices:
			BuildWeldVertices();
		break;

		case E_BuildState_FindTetrahedron:
			BuildFindMaximalTetrahedron();			
		break;

		case E_BuildState_FindPolyAndVertex:
			BuildFindPolyAndVertex();
		break;

		case E_BuildState_RemoveSelectedPoly:
			BuildRemoveSelectedPoly();
		break;

		case E_BuildState_CreateNewPolygons:
			BuildCreateNewPolygons();
		break;

		case E_BuildState_MergeNeighbourPolygons:
			BuildMergeNeighbourPolygons();
		break;

		case E_BuildState_Enumerate:
			BuildEnumerate();
		break;

		case E_BuildState_Finished:
		break;

		case E_BuildState_Error:
		break;
	}

	if( (m_BuildState == E_BuildState_Finished) || (m_BuildState == E_BuildState_Error) )
	{
		return true;
	}
	else
	{
		m_StepNo++;
		return false;
	}
}

bool C_Mesh::Closed()
{
	U32 noVertices = m_VertexList.Elements();
	U32 noEdges = m_EdgeList.Elements();
	U32 noPolygons = m_PolygonList.Elements();

	// Euler's Formula: V-E+F=2
	if((noVertices - noEdges + noPolygons) != 2)
		return false;
	else
		return true;
}

void C_Mesh::BuildWeldVertices()
{
	S32 noWelds = 0;
	for(S32 currVertexIndex=0; currVertexIndex<m_SourceVertexList.Size(); currVertexIndex++)
	{
		C_SourceVertex& currSourceVertex = m_SourceVertexList[currVertexIndex];

		bool done = false;

		while(!done)
		{
			done = true;

			if((currVertexIndex+1) < m_SourceVertexList.Size())
			{
				for(S32 testVertexIndex=currVertexIndex+1; testVertexIndex<m_SourceVertexList.Size(); testVertexIndex++)
				{
					C_SourceVertex& testSourceVertex = m_SourceVertexList[testVertexIndex];

					if(currSourceVertex.Distance(testSourceVertex) < CONVEX_HULL_VERTEX_WELD_DIST)
					{
						m_SourceVertexList.EraseByIndex(testVertexIndex);
						noWelds++;
						done = false;
						break;
					}
				}
			}
		}
	}

	m_BuildState = E_BuildState_FindTetrahedron;
	//LOGDEBUG( g_Log, "ConvexHull Source Vertex Welds %d\n", noWelds );
}

void C_Mesh::BuildFindMaximalTetrahedron()
{
	C_Tetrahedron tetra;
	bool result;

	// TODO [PaulM] better error checking when tetrahedron not found

	result = FindMaximalTetrahedronAB(tetra);
	if(!result)
	{
		m_BuildState = E_BuildState_Error;
		return;
	}

	result = FindMaximalTetrahedronC(tetra);
	if(!result)
	{
		m_BuildState = E_BuildState_Error;
		return;
	}

	result = FindMaximalTetrahedronD(tetra);
	if(!result)
	{
		m_BuildState = E_BuildState_Error;
		return;
	}

	// add vertices to mesh
	C_Vertex* pVertexA = NewVertex(tetra.m_A);
	C_Vertex* pVertexB = NewVertex(tetra.m_B);
	C_Vertex* pVertexC = NewVertex(tetra.m_C);
	C_Vertex* pVertexD = NewVertex(tetra.m_D);

	// add polygons to mesh
	C_Polygon* pNewPolygon = NULL;
	
	pNewPolygon = NewPolygon();
	pNewPolygon->AddVertex(pVertexA);
	pNewPolygon->AddVertex(pVertexB);
	pNewPolygon->AddVertex(pVertexC);
	pNewPolygon->EndConstruction();
	
	pNewPolygon = NewPolygon();
	pNewPolygon->AddVertex(pVertexC);
	pNewPolygon->AddVertex(pVertexB);
	pNewPolygon->AddVertex(pVertexD);
	pNewPolygon->EndConstruction();			 
	
	pNewPolygon = NewPolygon();
	pNewPolygon->AddVertex(pVertexB);
	pNewPolygon->AddVertex(pVertexA);
	pNewPolygon->AddVertex(pVertexD);
	pNewPolygon->EndConstruction();
	
	pNewPolygon = NewPolygon();
	pNewPolygon->AddVertex(pVertexD);
	pNewPolygon->AddVertex(pVertexA);
	pNewPolygon->AddVertex(pVertexC);
	pNewPolygon->EndConstruction();

	ClassifySourceVertices();

	m_BuildState = E_BuildState_FindPolyAndVertex;
}

void C_Mesh::BuildFindPolyAndVertex()
{
	C_Polygon* pCurrPolygon = m_PolygonList.First();
	while(pCurrPolygon)
	{
		if(pCurrPolygon->m_SourceVertexList.Size())
		{
			break;
		}
		
		pCurrPolygon = pCurrPolygon->Next();
	}

	if(pCurrPolygon == NULL)
	{
		m_BuildState = E_BuildState_MergeNeighbourPolygons;
		return;
	}

	m_pSelectedPolygon = pCurrPolygon;

	// find build vertex furthest from poly
	F32 maxDist = 0.0f;
	S32 maxVertexIndex = -1;

	const S32 sourceVertexCount = m_pSelectedPolygon->m_SourceVertexList.Size();
	for(S32 currVertexIndex=0; currVertexIndex<sourceVertexCount; currVertexIndex++)
	{
		C_SourceVertex& currSourceVertex = m_pSelectedPolygon->m_SourceVertexList[currVertexIndex];

		const F32 currDist = m_pSelectedPolygon->m_Plane.Distance(currSourceVertex);
		if(currDist > maxDist)
		{
			maxVertexIndex = currVertexIndex;
			maxDist = currDist;
		}
	}

	m_SelectedVertex = m_pSelectedPolygon->m_SourceVertexList[maxVertexIndex];
	//m_SelectedPolygon->m_SourceVertexList.Erase(m_SelectedVertex);
	
	m_pSelectedPolygon->m_SourceVertexList.EraseByIndex(maxVertexIndex);

	m_BuildState = E_BuildState_RemoveSelectedPoly;
}

void C_Mesh::BuildRemoveSelectedPoly()
{
	// TODO [PaulM] : very bad if this makes holes, just remove polys connected to polys we have already removed

	/*
	Ut::Vector<C_Polygon*> polygonTestList;
	polygonTestList.PushBack(m_pSelectedPolygon);

	while(polygonTestList.Size())
	{
		C_Polygon* pTestPoly = polygonTestList.Back();
		polygonTestList.PopBack();

		// test poly
		if(pTestPoly->m_Plane.Distance(m_SelectedVertex) > CONVEX_HULL_DELETE_POLY_PLANE_DIST)
		{
			// add neighbour polys to test list
			C_EdgeItem* pCurrEdgeItem = pTestPoly->m_EdgeItemList.First();
			while(pCurrEdgeItem)
			{
				C_Edge* pCurrEdge = *pCurrEdgeItem;

				C_PolygonItem* pCurrPolygonItem = pCurrEdge->m_PolygonItemList.First();
				while(pCurrPolygonItem)
				{
					C_Polygon* pOtherPolygon = *pCurrPolygonItem;

					if(pOtherPolygon != pTestPoly)
					{
						// see if poly is already on list
						bool found = false;
						for(S32 i=0; i<polygonTestList.Size(); i++)
						{
							if(polygonTestList[i] == pOtherPolygon)
								found = true;
						}
						
						if(!found)
						{
							polygonTestList.PushBack(pOtherPolygon);
						}
					}

					pCurrPolygonItem = pCurrPolygonItem->Next();
				}
				pCurrEdgeItem = pCurrEdgeItem->Next();
			}

			delete pTestPoly;
			pTestPoly = NULL;
		}
	}
	*/
	
	// remove all polys which vertex is infront of 
	C_Polygon* pTestPoly = m_PolygonList.First();
	while(pTestPoly)
	{
		C_Polygon* pNextPoly = pTestPoly->Next();

		if(pTestPoly->m_Plane.Distance(m_SelectedVertex) > CONVEX_HULL_DELETE_POLY_PLANE_DIST)
		{
			delete pTestPoly;		  
		}

		pTestPoly = pNextPoly;		
	}
	m_pSelectedPolygon = NULL;

	

	m_BuildState = E_BuildState_CreateNewPolygons;
}

void C_Mesh::BuildCreateNewPolygons()
{
	// create new polygons from broken edges
	Util::C_List<C_EdgeItem> brokenEdgeList;
	C_Edge* pCurrEdge = m_EdgeList.First();
	while(pCurrEdge)
	{
		if(pCurrEdge->m_PolygonItemList.Elements() == 1)
		{
			brokenEdgeList.PushLast( new C_EdgeItem(pCurrEdge) );
		}

		pCurrEdge = pCurrEdge->Next();
	}

	C_Vertex* pNewVertex = NewVertex(m_SelectedVertex);


	C_EdgeItem* pCurrBrokenEdgeItem = brokenEdgeList.First();
	while(pCurrBrokenEdgeItem)
	{
		C_EdgeItem* pNextBrokenEdgeItem = pCurrBrokenEdgeItem->Next();
		
		C_Edge* pCurrBrokenEdge = *pCurrBrokenEdgeItem;

		// get winding order
		C_PolygonItem* pEdgePolygonItem = pCurrBrokenEdge->m_PolygonItemList.First();
		C_Polygon* pEdgePolygon = *pEdgePolygonItem;

		C_Vertex* tri_A = NULL;
		C_Vertex* tri_B = NULL;
		C_Vertex* tri_C = NULL;

		if(pEdgePolygon->GetEdgeWindingOrder( pCurrBrokenEdge ))
		{
			tri_A = pNewVertex;
			tri_B = pCurrBrokenEdge->m_pVertices[1];
			tri_C = pCurrBrokenEdge->m_pVertices[0];
		}
		else
		{
			tri_A = pNewVertex;
			tri_B = pCurrBrokenEdge->m_pVertices[0];
			tri_C = pCurrBrokenEdge->m_pVertices[1];
		}

		// construct polygon
		C_Polygon* pNewPolygon = NewPolygon();
		pNewPolygon->AddVertex(tri_A);
		pNewPolygon->AddVertex(tri_B);
		pNewPolygon->AddVertex(tri_C);
		pNewPolygon->EndConstruction();

		pNewPolygon->MergeCoplanarNeighbours();

		delete pCurrBrokenEdgeItem;
		pCurrBrokenEdgeItem = pNextBrokenEdgeItem;
	}

	if(pNewVertex->m_PolygonItemList.Elements() == 0)
		delete pNewVertex;

	if(!Closed())
	{
		m_BuildState = E_BuildState_Error;
		return;
	}

	ClassifySourceVertices();

	m_BuildState = E_BuildState_FindPolyAndVertex;
}

void C_Mesh::BuildMergeNeighbourPolygons()
{
	//m_BuildState = E_BuildState_Enumerate;
	//return;

	C_Polygon* pCurrPolygon = m_PolygonList.First();
	while(pCurrPolygon)
	{
		if(pCurrPolygon->MergeCoplanarNeighbours())
		{
			pCurrPolygon = m_PolygonList.First();
		}
		else
		{
			pCurrPolygon = pCurrPolygon->Next();
			if(pCurrPolygon == NULL)
			{
				m_BuildState = E_BuildState_Enumerate;
			}
		}
	}

	if(!Closed())
	{
		m_BuildState = E_BuildState_Error;
		return;
	}
}			



const bool C_Mesh::FindMaximalTetrahedronAB(C_Tetrahedron& tetra)
{
	// we just need to find 2 vertices that are seprated really.

	// In fact, we find 2 vertices that are separated by greatest distance in one axis

	F32	maxAxisSeparation = -FLT_MIN;

	S32 AIndex = -1;
	S32 BIndex = -1;

	S32 axis;
	for(axis = 0; axis < 3; axis++)
	{
		Maths::HVec3 axisVec;

		switch(axis)
		{
			case 0: axisVec = Maths::HVec3(1.0f, 0.0f, 0.0f); break;
			case 1: axisVec = Maths::HVec3(0.0f, 1.0f, 0.0f); break;
			case 2: axisVec = Maths::HVec3(0.0f, 0.0f, 1.0f); break;
		}

		S32 axisMinVertexIndex = -1;
		F32	axisMinDist = FLT_MAX;

		S32 axisMaxVertexIndex = -1;
		F32	axisMaxDist = -FLT_MAX;

		S32 sourceVertexCount = m_SourceVertexList.Size();
		for(S32 sourceVertexIndex = 0; sourceVertexIndex < sourceVertexCount; sourceVertexIndex++)
		{
			C_SourceVertex& sourceVertex = m_SourceVertexList[sourceVertexIndex];
			F32 currAxisPos = sourceVertex | axisVec;

			if( currAxisPos < axisMinDist )
			{
				axisMinVertexIndex = sourceVertexIndex;
				axisMinDist = currAxisPos;
			}

			if( currAxisPos > axisMaxDist )
			{
				axisMaxVertexIndex = sourceVertexIndex;
				axisMaxDist = currAxisPos;
			}

			//LOGINFO( g_Log, "Min : %f:%d, Max : %f:%d = currAxisPos %f \n", axisMinDist, axisMinVertexIndex, axisMaxDist, axisMaxVertexIndex, currAxisPos);
		}

		assert(axisMinVertexIndex != -1);
		assert(axisMaxVertexIndex != -1);

		if(axisMinVertexIndex != axisMaxVertexIndex)
		{
			F32 axisSeparation = axisMaxDist - axisMinDist;

			if(axisSeparation > maxAxisSeparation)
			{
				maxAxisSeparation = axisSeparation;
				AIndex = axisMinVertexIndex;
				BIndex = axisMaxVertexIndex;
			}
		}
	}

	if((AIndex == -1) || (BIndex == -1))
	{
		LOGERROR( g_Log, "ConvexHull - Failed to find starting tetrahedron (points AB)\n");
		return false;
	}

	C_SourceVertex& vA = m_SourceVertexList[AIndex];
	C_SourceVertex& vB = m_SourceVertexList[BIndex];

	tetra.m_A = vA;
	tetra.m_B = vB;

	// remove vertices from source list
	if(AIndex > BIndex)
	{
		m_SourceVertexList.EraseByIndex(AIndex);
		m_SourceVertexList.EraseByIndex(BIndex);
	}
	else
	{
		m_SourceVertexList.EraseByIndex(BIndex);
		m_SourceVertexList.EraseByIndex(AIndex);
	}

	return true;
}

const bool C_Mesh::FindMaximalTetrahedronC(C_Tetrahedron& tetra)
{
	// find point furthest from line AB
	F32 maxDist = 0.0f;
	S32 maxVertexIndex = -1;

	Maths::C_Line lineAB( Maths::C_Vertex( tetra.m_A ), Maths::C_Vertex( tetra.m_B ) );

	S32 sourceVertexCount = m_SourceVertexList.Size();
	for(S32 sourceVertexIndex = 0; sourceVertexIndex < sourceVertexCount; sourceVertexIndex++)
	{
		C_SourceVertex& sourceVertex = m_SourceVertexList[sourceVertexIndex];

		F32 currDist = lineAB.Distance( Maths::C_Vertex( sourceVertex ) );

		if(currDist > maxDist)
		{
			maxDist = currDist;
			maxVertexIndex = sourceVertexIndex;
		}
	}

	if(maxVertexIndex == -1)
	{
		LOGERROR( g_Log, "ConvexHull - Failed to find starting tetrahedron (point C)\n");
		return false;
	}

	if(maxDist == 0.0f)
	{
		LOGERROR( g_Log, "ConvexHull - Failed to find starting tetrahedron (point C)\n");
		return false;
	}

	tetra.m_C = m_SourceVertexList[maxVertexIndex];
	m_SourceVertexList.EraseByIndex(maxVertexIndex);

	return true;
}

const bool C_Mesh::FindMaximalTetrahedronD(C_Tetrahedron& tetra)
{
	// TODO [PaulM] : remove vertices from input list when used

	// find d which is furthest from plane of ABC

	F32 maxDist = 0.0f;
	S32 maxVertexIndex = -1;

	Maths::C_Plane planeABC( Maths::C_Vertex( tetra.m_A ), Maths::C_Vertex( tetra.m_B ), Maths::C_Vertex( tetra.m_C ) );

	S32 sourceVertexCount = m_SourceVertexList.Size();
	for(S32 sourceVertexIndex = 0; sourceVertexIndex < sourceVertexCount; sourceVertexIndex++)
	{
		C_SourceVertex& sourceVertex = m_SourceVertexList[sourceVertexIndex];

		F32 currDist = planeABC.Distance( Maths::C_Vertex( sourceVertex ) );
		F32 testDist = fabsf(currDist);

		if(testDist > fabsf(maxDist))
		{
			maxDist = currDist;
			maxVertexIndex = sourceVertexIndex;
		}
	}

	if(maxVertexIndex == -1)
	{
		LOGERROR( g_Log, "ConvexHull - Failed to find starting tetrahedron (point D)\n");
		return false;
	}

	if(maxDist == 0.0f)
	{
		LOGERROR( g_Log, "ConvexHull - Failed to find starting tetrahedron (point D)\n");
		return false;
	}

	tetra.m_D = m_SourceVertexList[maxVertexIndex];
	m_SourceVertexList.EraseByIndex(maxVertexIndex);

	if(maxDist < 0.0f)
	{
		// swap order of points
		Maths::HVec3 tempVertex = tetra.m_A;
		tetra.m_A = tetra.m_B;
		tetra.m_B = tempVertex;
	}

	return true;
}

void C_Mesh::ClassifySourceVertices()
{
	// assign each point to a polygon they are in front of
	// this doesnt need to but it assigns the vertices to the poly they are maximally infront of 
	while(m_SourceVertexList.Size())
	{
		C_SourceVertex vertex = m_SourceVertexList.Back();
		m_SourceVertexList.PopBack();

		C_Polygon* pMaxPoly = NULL;
		F32 maxDist = CONVEX_HULL_PLANE_DIST;

		C_Polygon* pCurrPolygon = m_PolygonList.First();
		while(pCurrPolygon)
		{
			F32 planeDist = pCurrPolygon->m_Plane.Distance(vertex);

			if(planeDist > maxDist)
			{
				pMaxPoly = pCurrPolygon;
				maxDist = planeDist;
			}

			pCurrPolygon = pCurrPolygon->Next();
		}

		if(pMaxPoly)
		{
			pMaxPoly->m_SourceVertexList.PushBack(vertex);
		}
	}
}

void C_Mesh::BuildEnumerate()
{
	S32 index;
	
	index = 0;
	C_Polygon* pCurrPolygon = m_PolygonList.First();
	while(pCurrPolygon)
	{
		pCurrPolygon->m_Index = index++;
		pCurrPolygon = pCurrPolygon->Next();
	}

	index = 0;
	C_Edge* pCurrEdge = m_EdgeList.First();
	while(pCurrEdge)
	{
		pCurrEdge->m_Index = index++;
		pCurrEdge = pCurrEdge->Next();
	}

	index = 0;
	C_Vertex* pCurrVertex = m_VertexList.First();
	while(pCurrVertex)
	{
		pCurrVertex->m_Index = index++;
		pCurrVertex = pCurrVertex->Next();
	}

	m_BuildState = E_BuildState_Finished;
}

}; // namespace ConvexHull

