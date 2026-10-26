#include "MiscSharedPch.h"

#include <core/core.h>
#include <system/DebugLog.h>
#include <misc_shared/geometry/SimpleBSP.h>

namespace SimpleBSP
{

Debug::Logger &g_Log = Debug::GetLogger("mercury.misc_shared.bsp");

// C_PolygonVertex

C_PolygonVertex::C_PolygonVertex( const Maths::HVec3& pos ) : m_Pos(pos)
{
}

C_PolygonVertex::~C_PolygonVertex()
{
}
    
// C_Polygon    

C_Polygon::C_Polygon( ) 
{
}
    
C_Polygon::~C_Polygon()
{
}

void C_Polygon::operator =( const C_Polygon& other )
{
    m_Plane = other.m_Plane;
    m_VertexList.DeleteAll();
    
    C_PolygonVertex* pCurrVertex = other.m_VertexList.First();
    while(pCurrVertex)
    {
        m_VertexList.PushLast( new C_PolygonVertex( pCurrVertex->m_Pos ) );
        pCurrVertex = pCurrVertex->Next();
    }    
}

void C_Polygon::NewVertex( const Maths::HVec3& pos )
{
    C_PolygonVertex* pVertex = new C_PolygonVertex(pos);
    m_VertexList.PushLast(pVertex);
}

void C_Polygon::Plane( const Maths::C_Plane& plane )
{
    m_Plane = plane;
}

const Maths::C_Plane C_Polygon::Plane() const
{
    return m_Plane;
}

void C_Polygon::CalculatePlane()
{
    /*
    C_PolygonVertex* pVertexA = m_VertexList.First();
    C_PolygonVertex* pVertexB = pVertexA->Next();
    C_PolygonVertex* pVertexC = pVertexB->Next();
    
    m_Plane = Maths::C_Plane( pVertexA->m_Pos, pVertexB->m_Pos, pVertexC->m_Pos );
    */
    if(m_VertexList.Elements() < 3)
        return;

    // calculate normal
    
    C_PolygonVertex* pCurrVertex = m_VertexList.Last();
    Maths::HVec3 currPos = pCurrVertex->m_Pos;
    
    C_PolygonVertex* pPrevVertex = pCurrVertex->Prev();
    Maths::HVec3 prevPos = pPrevVertex->m_Pos;

    Maths::HVec3 currEdgeDir( (currPos-prevPos).Normalised().Normal() );
        
    // TODO [PaulM] : this is broken, the total magnitude is coming out as 0.5
    // exterior angle should sum to 360            
    Maths::HVec3 normalResult(0.0f);

    Maths::HVec3 prevEdgeDir;
    Maths::HVec3 crossProduct;
    F32 crossLength;
    F32 angle;

    pCurrVertex = m_VertexList.First();
    while(pCurrVertex)
    {
        prevEdgeDir = currEdgeDir;
        prevPos = currPos;
        
        currPos = pCurrVertex->m_Pos;
        
        currEdgeDir = (currPos-prevPos).Normalised().Normal();            
    
        crossProduct = prevEdgeDir ^ currEdgeDir;        
        crossLength = Maths::Length( crossProduct );        
        if(crossLength > 0)
        {
            if(crossLength > 1.2f)
            {
				LOGWARN( g_Log, "Bad angle in portal normal calculation\n");
            }
            
            if(crossLength > 1.0f)
            {
                crossLength = 1.0f;
            }
            
            angle = asinf(crossLength);
            normalResult = normalResult + ((Maths::HVec3)crossProduct.Normalised().Normal() * angle);
        }
            
        pCurrVertex = pCurrVertex->Next();
    }
    
    C_PolygonVertex* pFirstVertex = m_VertexList.First();
    Maths::HVec3 firstPos = pFirstVertex->m_Pos;
    
    m_Plane = Maths::C_Plane( Maths::C_Normal( normalResult ), Maths::C_Vertex( firstPos ) );    
    
}

    
void C_Polygon::Clip( const Maths::C_Plane& plane, const F32 epsilon, C_Polygon& front, C_Polygon& back )
{
    //Maths::C_Normal planeNormal = plane.Normal();
    //Maths::HF32 distToOrigin = plane.DistanceToOrigin();    

    C_PolygonVertex* pPrevVertex = m_VertexList.Last();
    Maths::HVec3 prevPos = pPrevVertex->m_Pos;
    Maths::HF32 prevDist = plane.Distance(prevPos);
    //Maths::HF32 prevDist = (prevPos | planeNormal) + distToOrigin;

    C_PolygonVertex* pCurrVertex = m_VertexList.First();        
    while(pCurrVertex)
    {
        Maths::HVec3 currPos = pCurrVertex->m_Pos;
        Maths::HF32 currDist = plane.Distance(currPos);
        //Maths::HF32 currDist = (currPos | planeNormal) + distToOrigin;

        const Maths::HF32 hfEpsilon( epsilon );
        const Maths::HF32 hfMinusEpsilon( -epsilon );
    
        // FRONT
	    if((prevDist < hfEpsilon) && (currDist < hfEpsilon))
	    {
	        // output nothing
	    }
	    else
	    if((prevDist >= hfEpsilon) && (currDist < hfEpsilon))
	    {
	        // ouput I
            Maths::HF32 t = (prevDist - hfEpsilon) / (prevDist - currDist);
            front.NewVertex( prevPos + (currPos - prevPos) * t );
	    }
	    else
	    if((prevDist < hfEpsilon) && (currDist >= hfEpsilon))
	    {
	        // ouput I, vertex
            Maths::HF32 t = (prevDist - hfEpsilon) / (prevDist - currDist);
            front.NewVertex( prevPos + (currPos - prevPos) * t );
            front.NewVertex( pCurrVertex->m_Pos );
	    }
	    else
	    if((prevDist >= hfEpsilon) && (currDist >= hfEpsilon))
	    {
            // output vertex
            front.NewVertex( pCurrVertex->m_Pos );
	    }

        // BACK                        
	    if((prevDist < hfMinusEpsilon) && (currDist < hfMinusEpsilon))
	    {
            // output vertex
            back.NewVertex( pCurrVertex->m_Pos );
	    }
	    else
	    if((prevDist >= hfMinusEpsilon) && (currDist < hfMinusEpsilon))
	    {
	        // ouput I, vertex
            Maths::HF32 t = (prevDist - hfMinusEpsilon) / (prevDist - currDist);
            back.NewVertex( prevPos + (currPos - prevPos) * t );
            back.NewVertex( pCurrVertex->m_Pos );
	    }
	    else
	    if((prevDist < hfMinusEpsilon) && (currDist >= hfMinusEpsilon))
	    {
	        // ouput I
            Maths::HF32 t = (prevDist - hfMinusEpsilon) / (prevDist - currDist);
            back.NewVertex( prevPos + (currPos - prevPos) * t );
	    }
	    else
	    if((prevDist >= hfMinusEpsilon) && (currDist >= hfMinusEpsilon))
	    {
	        // output nothing
	    }
                
        prevPos = currPos;
        prevDist = currDist;
        pPrevVertex = pCurrVertex;
        pCurrVertex = pCurrVertex->Next();
    }    
    
    front.Plane( Plane() );    
    back.Plane( Plane() );    
}
    
void C_Polygon::DebugRender( DbUtil::I_DebugRenderPtr debugRenderPtr, Maths::RGBA color )
{
    C_PolygonVertex* pPrevVertex = m_VertexList.Last();
    C_PolygonVertex* pCurrVertex = m_VertexList.First();
    while(pCurrVertex)        
    {
        debugRenderPtr->Draw3dLine( pPrevVertex->m_Pos, pCurrVertex->m_Pos, color);
                            
        pPrevVertex = pCurrVertex;
        pCurrVertex = pCurrVertex->Next();
    }
}

void C_Polygon::DebugRender( DbUtil::I_DebugRenderPtr debugRenderPtr )
{
	DebugRender(debugRenderPtr, Maths::RGBA(0xff, 0xff, 0x00, 0xff));
}

void C_Polygon::DebugRenderSolid( DbUtil::I_DebugRenderPtr debugRenderPtr )
{
    C_PolygonVertex* pFirstVertex = m_VertexList.First();
    if(!pFirstVertex) return;        
    C_PolygonVertex* pPrevVertex = pFirstVertex->Next();
    if(!pPrevVertex) return;
    C_PolygonVertex* pCurrVertex = pPrevVertex->Next();
    while(pCurrVertex)        
    {
        debugRenderPtr->Draw3dTriangle( pFirstVertex->m_Pos, pPrevVertex->m_Pos, pCurrVertex->m_Pos, Maths::RGBA(0x00, 0xff, 0xff, 0xff));
                            
        pPrevVertex = pCurrVertex;
        pCurrVertex = pCurrVertex->Next();
    }
}

// C_Node


C_Node::C_Node( const Maths::C_Plane& plane ) : m_SplitPlane( plane )
{
    m_pParent = NULL;
    
    m_pFront = NULL;
    m_pBack = NULL;
}

C_Node::~C_Node()
{
}

void C_Node::DebugRender( DbUtil::I_DebugRenderPtr debugRenderPtr)
{
	DebugRender(debugRenderPtr, Maths::RGBA(0xff, 0xff, 0x00, 0xff));
}

void C_Node::DebugRender( DbUtil::I_DebugRenderPtr debugRenderPtr, Maths::RGBA color)
{
    m_DebugPoly.DebugRender( debugRenderPtr, color );
    
    if(m_pFront)
    {
        m_pFront->DebugRender( debugRenderPtr, Maths::RGBA(0xff, 0x00, 0x00, 0xff));
    }

    if(m_pBack)
    {
        m_pBack->DebugRender( debugRenderPtr, Maths::RGBA(0x00, 0x00, 0xff, 0xff));
    }
}

// C_Tree

 
C_Tree::C_Tree()
{
    m_pRootNode = NULL;
    m_Epsilon = 0.0001f;
}

C_Tree::~C_Tree()
{    
}
        
C_Node* C_Tree::NewNode( const Maths::C_Plane& plane )
{
    C_Node* pNewNode = new C_Node(plane);
    m_NodeList.PushLast(pNewNode);
    return pNewNode;            
}         
        
void C_Tree::Insert(C_Polygon& polygon)
{
    if(m_pRootNode)
    {
        Insert( *m_pRootNode, polygon );    
    }   
    else
    {
        m_pRootNode = CreateNodes( polygon );
    }        
}


//void C_Tree::Insert(C_Node& node, SimplePoly::C_Polygon& polygon)
//{
//	const SimplePoly::E_PolyPlaneTest polyPlaneResult = polygon.Classify( node.m_SplitPlane, m_Epsilon );
//
//	switch(polyPlaneResult)
//	{
//		case SimplePoly::E_PolyPlaneTest_InFront:
//		{
//			if(node.m_pFront)
//			{
//				Insert(*node.m_pFront, polygon);                
//			}
//			else
//			{
//				node.m_pFront = CreateNodes( polygon );                
//				node.m_pFront->m_pParent = &node;
//			}
//		}
//		break;
//
//		case SimplePoly::E_PolyPlaneTest_Behind:
//		{
//			if(node.m_pBack)
//			{
//				Insert(*node.m_pBack, polygon);                
//			}
//			else
//			{
//				node.m_pBack = CreateNodes( polygon );                
//				node.m_pBack->m_pParent = &node;
//			}
//		}
//		break;
//
//		case SimplePoly::E_PolyPlaneTest_Coplanar:
//		{
//			// push down front
//			if(node.m_pFront)
//			{
//				Insert(*node.m_pFront, polygon);                
//			}
//			else
//			{
//				node.m_pFront = CreateNodes( polygon );                
//				node.m_pFront->m_pParent = &node;
//			}
//		}
//		break;
//
//		case SimplePoly::E_PolyPlaneTest_Straddled:
//		{
//			SimplePoly::C_Polygon frontPoly;
//			SimplePoly::C_Polygon backPoly;        
//
//			polygon.Clip( node.m_SplitPlane, m_Epsilon, frontPoly, backPoly );
//		    
//			if(frontPoly.m_VertexList.Size() >= 3)
//			{
//				if(node.m_pFront)
//				{
//					Insert(*node.m_pFront, frontPoly);                
//				}
//				else
//				{
//					node.m_pFront = CreateNodes( frontPoly );                
//					node.m_pFront->m_pParent = &node;
//				}
//			}
//
//			if(backPoly.m_VertexList.Size() >= 3)
//			{
//				if(node.m_pBack)
//				{
//					Insert(*node.m_pBack, backPoly);                
//				}
//				else
//				{
//					node.m_pBack = CreateNodes( backPoly );                
//					node.m_pBack->m_pParent = &node;
//				}
//			}
//		}
//		break;
//	}
//}



void C_Tree::Insert(C_Node& node, C_Polygon& polygon)
{
    C_Polygon frontPoly;
    C_Polygon backPoly;        

    polygon.Clip( node.m_SplitPlane, m_Epsilon, frontPoly, backPoly );
    
    if(frontPoly.m_VertexList.Elements() >= 3)
    {
        if(node.m_pFront)
        {
            Insert(*node.m_pFront, frontPoly);                
        }
        else
        {
            node.m_pFront = CreateNodes( frontPoly );                
            node.m_pFront->m_pParent = &node;
        }
    }

    if(backPoly.m_VertexList.Elements() >= 3)
    {
        if(node.m_pBack)
        {
            Insert(*node.m_pBack, backPoly);                
        }
        else
        {
            node.m_pBack = CreateNodes( backPoly );                
            node.m_pBack->m_pParent = &node;
        }
    }
}

C_Node* C_Tree::CreateNodes(C_Polygon& polygon)
{
    C_Node* pNewNode = NewNode(polygon.Plane());    
    pNewNode->m_DebugPoly = polygon;        
    return pNewNode;       
}
    
void C_Tree::DebugRender( DbUtil::I_DebugRenderPtr debugRenderPtr )
{
    if(m_pRootNode)
    {
        m_pRootNode->DebugRender( debugRenderPtr );
    }
}

const bool C_Tree::Inside( C_Node& node, const Maths::HVec3& pos ) const
{
    F32 distance = node.m_SplitPlane.Distance(pos);
    
    if(distance < 0)
    {
        if(node.m_pBack)
        {
            return Inside(*node.m_pBack, pos);
        }
        else
        {
            return true;
        }            
    }
    else
    {
        if(node.m_pFront)
        {
            return Inside(*node.m_pFront, pos);
        }
        else
        {
            return false;
        }            
    }    
}

const bool C_Tree::Inside( const Maths::HVec3& pos ) const
{
    if(m_pRootNode)
    {
        return Inside( *m_pRootNode, pos );
    }
    else
    {
        return true;
    }
}


C_CellPlane::C_CellPlane( const Maths::C_Plane& plane ) : m_Plane(plane)
{
}

void C_Cell::operator =( const C_Cell& other )
{
    C_CellPlane* pCurrCellPlane = other.m_PlaneList.First();
    while(pCurrCellPlane)
    {
        m_PlaneList.PushLast( new C_CellPlane( pCurrCellPlane->m_Plane ) );
        pCurrCellPlane = pCurrCellPlane->Next();    
    }
}


void C_Containment::BuildContainment( C_Cell& cell, C_Node& node )
{        
    if(node.m_pBack)
    {
        C_CellPlane* pPlane = new C_CellPlane(node.m_SplitPlane);
        cell.m_PlaneList.PushLast(pPlane);

        BuildContainment( cell, *node.m_pBack );

        delete pPlane;
    }
    else
    {   
        // add to containment list
        C_CellPlane* pPlane = new C_CellPlane(node.m_SplitPlane);
        cell.m_PlaneList.PushLast(pPlane);

        CreateCell( cell );

        delete pPlane;
    }

    if(node.m_pFront)
    {
        C_CellPlane* pPlane = new C_CellPlane(-node.m_SplitPlane );
        cell.m_PlaneList.PushLast(pPlane);
        
        BuildContainment( cell, *node.m_pFront );
        
        delete pPlane;
    }
}

C_Containment::C_Containment( C_Tree& tree )
{
    C_Cell tempCell;        
        
    if(tree.m_pRootNode)
    {
        BuildContainment( tempCell, *tree.m_pRootNode );
    }
}

void C_Containment::CreateCell( C_Cell& cell )
{
    C_Cell* pNewCell = new C_Cell;
    *pNewCell = cell;
    m_CellList.PushLast( pNewCell );
}

const bool C_Containment::Inside( Maths::HVec3& pos )
{
    C_Cell* pCurrCell = m_CellList.First();
    while(pCurrCell)
    {
        C_CellPlane* pCurrPlane = pCurrCell->m_PlaneList.First();
        bool fail = false;
        while(!fail && pCurrPlane)
        {
            if( pCurrPlane->m_Plane.Distance( pos ) > 0.0f )
            {
                fail = true;
            }
            pCurrPlane = pCurrPlane->Next();                        
        }
        
        if(!fail) 
            return true;
        
        pCurrCell = pCurrCell->Next();        
    }    
    
    return false;
}


};
