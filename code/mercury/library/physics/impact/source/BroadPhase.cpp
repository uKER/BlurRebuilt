#include "stdafx.h"

#include "Impact\Body.h"
#include "Impact\BroadPhase.h"
#include "Impact\Collision.h"
#include "Impact\PersistentData.h"
#include "Impact\CollisionPair.h"
#include "Impact\World.h"
#include "System\Memory\MemoryTracker.h"

using namespace Maths;

namespace Impact
{

#define EDGE_TYPE_OPENING 0
#define EDGE_TYPE_CLOSING 1

BroadPhase::BroadPhase(World *pWorld, const C_AxisBox &worldBBox) :
	m_worldBBox( worldBBox ),
	m_pWorld( pWorld )
{
	m_cellSize = HVec3(65535.f) / worldBBox.Diagonal();

	// body -1 is a body that is exactly the size of the world.
	// this is to stop the edge moving code from running off the end without having to do array bounds checks
	AddBody(-1, worldBBox, false);

	// this is to stop any subsequent bodies from being the exact size of the world BBox
	m_worldBBox.Max( m_worldBBox.Max() - (HVec3(1.f)/m_cellSize) );
	m_worldBBox.Min( m_worldBBox.Min() + (HVec3(1.f)/m_cellSize) );
}

BroadPhase::~BroadPhase()
{
}

/// add a body, returns ID to use with UpdateBody
int BroadPhase::AddBody(int iBody, const Maths::C_AxisBox &aabb, bool bSortEdges)
{
	U16Box bbox = GetU16Box(aabb);

	Item item;
	item.m_iBody = iBody;

	// construct the opening and closing edge for this body
	Edge edge;
	edge.m_iItem = m_items.Size();
	for (int iAxis=0; iAxis<3; iAxis++)
	{
		// add opening edge
		item.m_iEdge[EDGE_TYPE_OPENING][iAxis] = m_edges[iAxis].Size();
		edge.m_iPos = bbox.min[iAxis];
		m_edges[iAxis].PushBack(edge);

		// add closing edge
		item.m_iEdge[EDGE_TYPE_CLOSING][iAxis] = m_edges[iAxis].Size();
		edge.m_iPos = bbox.max[iAxis];
		m_edges[iAxis].PushBack(edge);
	}

	m_items.PushBack(item);

	if (bSortEdges)
	{
		for (int iAxis=0; iAxis<3; iAxis++)
		{
			// sort edges into their correct position
			if (iAxis == 2)
			{
				MoveEdgeDown<AddOverlappedPair,EDGE_TYPE_OPENING>(iAxis, item.m_iEdge[EDGE_TYPE_OPENING][iAxis]);
				MoveEdgeDown<RemoveOverlappedPair,EDGE_TYPE_CLOSING>(iAxis, item.m_iEdge[EDGE_TYPE_CLOSING][iAxis]);
			}
			else
			{
				MoveEdgeDown<DoNotUpdate,EDGE_TYPE_OPENING>(iAxis, item.m_iEdge[EDGE_TYPE_OPENING][iAxis]);
				MoveEdgeDown<DoNotUpdate,EDGE_TYPE_CLOSING>(iAxis, item.m_iEdge[EDGE_TYPE_CLOSING][iAxis]);
			}
		}
	}

	return edge.m_iItem;
}

/// remove body
void BroadPhase::RemoveBody(int iBodyID)
{
	g_bPutExtraInfoInDmpFile = true;

	Item *pItem = &m_items[iBodyID];
	assert(pItem->m_iBody != (U16)-1); // already deleted

	// check whether we have any pending updates for this item and remove them if neccesary
	for (int i=0; i<m_updates.Size(); i++)
	{
		UpdateItem *p = &m_updates[i];
		if (p->iBodyID == iBodyID)
			m_updates.EraseNonOrdered(i);
	}

//	Verify();

	// sort edges out to the top and remove them
	for (int iAxis=0; iAxis<3; iAxis++)
	{
		RemoveEdge<DoNotUpdate, EDGE_TYPE_CLOSING>(iAxis, pItem->m_iEdge[EDGE_TYPE_CLOSING][iAxis]);
		RemoveEdge<RemoveOverlappedPair,EDGE_TYPE_OPENING>(iAxis, pItem->m_iEdge[EDGE_TYPE_OPENING][iAxis]);
	}

	// TODO: remove item
	pItem->m_iBody = (U16)-1;

//	Verify();

	g_bPutExtraInfoInDmpFile = false;
}

/// apply all update items to the broadphase
void BroadPhase::Update()
{
	for (int i=0; i<m_updates.Size(); i++)
	{
		UpdateItem *p = &m_updates[i];
		UpdateBody(p->iBodyID, p->aabb);
	}
	m_updates.Clear();
}

/// update body deferred
void BroadPhase::UpdateBodyDeferred(int iBodyID, const C_AxisBox &aabb)
{
	assert(m_items[iBodyID].m_iBody != (U16)-1);

	UpdateItem update;
	update.iBodyID = iBodyID;
	update.aabb = aabb;
	m_updates.PushBack(update);
}

/// update the bounding box of the given body
void BroadPhase::UpdateBody(int iBodyID, const C_AxisBox &aabb)
{
	U16Box bbox = GetU16Box(aabb);

	Item *pItem = &m_items[iBodyID];

	// update edges
	for (int iAxis=0; iAxis<3; iAxis++)
	{
		int iCur[2], d[2];
		U16 *pCurPos[2];

		// sort opening edge
		iCur[EDGE_TYPE_OPENING] = pItem->m_iEdge[EDGE_TYPE_OPENING][iAxis];
		iCur[EDGE_TYPE_CLOSING] = pItem->m_iEdge[EDGE_TYPE_CLOSING][iAxis];

		pCurPos[0] = &m_edges[iAxis][iCur[EDGE_TYPE_OPENING]].m_iPos;
		pCurPos[1] = &m_edges[iAxis][iCur[EDGE_TYPE_CLOSING]].m_iPos;

 		d[EDGE_TYPE_OPENING] = bbox.min[iAxis] - *pCurPos[0];
		d[EDGE_TYPE_CLOSING] = bbox.max[iAxis] - *pCurPos[1];

		*pCurPos[0] = bbox.min[iAxis];
		*pCurPos[1] = bbox.max[iAxis];

		// expand first and then shrink, to prevent temporarily inverting the bounding box
		if (d[EDGE_TYPE_OPENING] < 0)
			MoveEdgeDown<AddOverlappedPair,EDGE_TYPE_OPENING>(iAxis, iCur[EDGE_TYPE_OPENING]);
		if (d[EDGE_TYPE_CLOSING] > 0)
			MoveEdgeUp<AddOverlappedPair,EDGE_TYPE_CLOSING>(iAxis, iCur[EDGE_TYPE_CLOSING]);
		if (d[EDGE_TYPE_OPENING] > 0)
			MoveEdgeUp<RemoveOverlappedPair,EDGE_TYPE_OPENING>(iAxis, iCur[EDGE_TYPE_OPENING]);
		if (d[EDGE_TYPE_CLOSING] < 0)
			MoveEdgeDown<RemoveOverlappedPair,EDGE_TYPE_CLOSING>(iAxis, iCur[EDGE_TYPE_CLOSING]);
	}
}

/// construct a U16Box from a C_AxisBox
BroadPhase::U16Box BroadPhase::GetU16Box(const Maths::C_AxisBox &bbox)
{
	U16Box rtn;

	HVec3 vMin = (bbox.Min() - m_worldBBox.Min()) * m_cellSize;
	HVec3 vMax = (bbox.Max() - m_worldBBox.Min()) * m_cellSize;

	vMin = vMin.Max(HVec3(0.f)).Min(HVec3(65535.f));
	vMax = vMax.Max(HVec3(0.f)).Min(HVec3(65535.f));

	rtn.min[0] = (U16(vMin.X()) & ~1);
	rtn.min[1] = (U16(vMin.Y()) & ~1);
	rtn.min[2] = (U16(vMin.Z()) & ~1);

	rtn.max[0] = U16(vMax.X()) | 1;
	rtn.max[1] = U16(vMax.Y()) | 1;
	rtn.max[2] = U16(vMax.Z()) | 1;

	return rtn;
}

/// test whether the AABBs of two bodies intersect
bool BroadPhase::TestCollision(const Item *pItem1, const Item *pItem2, int iSkipAxis)
{
	for (int iAxis=0; iAxis<3; iAxis++)
	{ 
		if (iAxis != iSkipAxis &&
			(pItem1->m_iEdge[EDGE_TYPE_CLOSING][iAxis] < pItem2->m_iEdge[EDGE_TYPE_OPENING][iAxis] ||
			 pItem2->m_iEdge[EDGE_TYPE_CLOSING][iAxis] < pItem1->m_iEdge[EDGE_TYPE_OPENING][iAxis]) )
				return false; 
	} 

	return true;
}

/// move an edge of _EDGETYPE_ down through the list
/// run the callback _CALL_ when an overlap has been added/removed
template<typename CALL, int EDGETYPE>
void BroadPhase::MoveEdgeDown(int iAxis, U16 iEdge)
{
	Edge *pEdge = &m_edges[iAxis][iEdge];
	Edge *pPrev = &m_edges[iAxis][iEdge-1];
	Item *pItem = &m_items[pEdge->m_iItem];

	while (pEdge->m_iPos < pPrev->m_iPos)
	{
		Item *pPrevItem = &m_items[pPrev->m_iItem];

		int iType = pPrev->EdgeType();
		if (iType == (EDGETYPE^1))
		{
			((CALL*)(this))->Go(iAxis,pItem,pPrevItem);
		}

		// update the other body's edge index
 		pPrevItem->m_iEdge[iType][iAxis]++;
		// update own edge index
		pItem->m_iEdge[EDGETYPE][iAxis]--;

		// swap the contents of the edge structures
		Edge swap = *pEdge;
		*pEdge = *pPrev;
		*pPrev = swap;

		// decrement
		pEdge--;
		pPrev--;
	}
}

/// move an edge of _EDGETYPE_ up through the list
/// run the callback _CALL_ when an overlap has been added/removed
template<typename CALL, int EDGETYPE>
void BroadPhase::MoveEdgeUp(int iAxis, U16 iEdge)
{
	Edge *pEdge = &m_edges[iAxis][iEdge];
	Edge *pNext = &m_edges[iAxis][iEdge+1];
	Item *pItem = &m_items[pEdge->m_iItem];

	while (pEdge->m_iPos > pNext->m_iPos)
	{
		Item *pNextItem = &m_items[pNext->m_iItem];

		int iType = pNext->EdgeType();
		if (iType == (EDGETYPE^1))
		{
			((CALL*)(this))->Go(iAxis,pItem,pNextItem);
		}

		// update the other body's edge index
		pNextItem->m_iEdge[iType][iAxis]--;
		// update own edge index
		pItem->m_iEdge[EDGETYPE][iAxis]++;

		// swap the contents of the edge structures
		Edge swap = *pEdge;
		*pEdge = *pNext;
		*pNext = swap;

		pEdge++;
		pNext++;
	}
}

/// move an edge of _EDGETYPE_ up through to the top of the list and remove it
/// run the callback _CALL_ when an overlap has been added/removed
template<typename CALL, int EDGETYPE>
void BroadPhase::RemoveEdge(int iAxis, U16 iEdge)
{
	Edge *pEdge = &m_edges[iAxis][iEdge];
	Edge *pNext = &m_edges[iAxis][iEdge+1];
	Item *pItem = &m_items[pEdge->m_iItem];

	int n = m_edges[iAxis].Size()-1;
	while (iEdge < n)
	{
		Item *pNextItem = &m_items[pNext->m_iItem];

		int iType = pNext->EdgeType();
		if (iType == (EDGETYPE^1))
		{
			((CALL*)(this))->Go(iAxis,pItem,pNextItem);
		}

		// update the other body's edge index
		pNextItem->m_iEdge[iType][iAxis]--;
		// update own edge index
		pItem->m_iEdge[EDGETYPE][iAxis]++;

		// swap the contents of the edge structures
		Edge swap = *pEdge;
		*pEdge = *pNext;
		*pNext = swap;

		pEdge++;
		pNext++;
		iEdge++;
	}

	m_edges[iAxis].PopBack();
}

// test for collision and add an overlapping pair
void AddOverlappedPair::Go(int iAxis, Item *pBody1, Item *pBody2)
{
	if (TestCollision(pBody1, pBody2, iAxis))
	{
		m_pWorld->AddCollisionPair(pBody1->m_iBody, pBody2->m_iBody);
	}
}

// remove an overlapping pair
void RemoveOverlappedPair::Go(int iAxis, Item *pBody1, Item *pBody2)
{
	m_pWorld->RemoveCollisionPair(pBody1->m_iBody, pBody2->m_iBody);
}

// do nothing
void DoNotUpdate::Go(int iAxis, Item *pBody1, Item *pBody2)
{
}

#pragma optimize("", off)

void BroadPhase::Verify()
{
#ifndef CFG_SPEED
	// assert that edges don't point to invalid bodies
	for (int iAxis=0; iAxis<3; iAxis++)
	{
		for (int i=0; i<m_edges[iAxis].Size(); i++)
		{
			int iItem = m_edges[iAxis][i].m_iItem;
			assert(iItem == 0 || m_items[iItem].m_iBody != (U16)-1);
		}
	}

	// assert that bodies don't point to invalid edges
	for (int iBody=0; iBody<m_items.Size(); iBody++)
	{
		if (m_items[iBody].m_iBody == (U16)-1)
			continue;

		for (int iAxis=0; iAxis<3; iAxis++)
		{
			int iEdge1 = m_items[iBody].m_iEdge[0][iAxis];
			int iEdge2 = m_items[iBody].m_iEdge[1][iAxis];
			assert(m_edges[iAxis][iEdge1].m_iItem == iBody);
			assert(m_edges[iAxis][iEdge2].m_iItem == iBody);
		}
	}
#endif
}

bool g_bPutExtraInfoInDmpFile = false;

/// get pointers to bit of memory that are interesting to dump out
void BroadPhase::GetPointers(Ut::Vector<void*> &pointers, Ut::Vector<int> &sizes)
{
	if (m_items.Size())
	{
		pointers.PushBack(&m_items[0]);
		sizes.PushBack(m_items.Size() * sizeof(m_items[0]));
	}
	for (int i=0; i<3; i++)
	{
		if (m_edges[i].Size())
		{
			pointers.PushBack(&m_edges[i][0]);
			sizes.PushBack(m_edges[i].Size() * sizeof(m_edges[i][0]));
		}
	}
	pointers.PushBack(this);
	sizes.PushBack(sizeof(this));
}

};