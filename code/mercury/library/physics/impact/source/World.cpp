#include "stdafx.h"

#include "Impact\Body.h"
#include "Impact\Collision.h"
#include "Impact\PersistentData.h"
#include "Impact\CollisionPair.h"
#include "Impact\World.h"
#include "Impact\BroadPhase.h"
#include "System\Debug\I_DebugRender.h"
#include "Impact\ConvexHull.h"
#include "Impact\I_TriMesh.h"
#include "Impact\Island.h"
#include "Impact\Event.h"
#include "System\Memory\MemoryTracker.h"

#include "System\Time\Time.h"

using namespace Maths;
using namespace Impact;

Debug::Bool g_bEnable("Impact.Enable", true);
Debug::Bool g_bShowPerformanceGraph("Impact.ShowPerformanceGraph", false);

namespace Impact
{

extern Debug::Logger &g_Log;

extern int gnIntersectEdge;

int nAddCollisionPair = 0;
int nRemoveCollisionPair = 0;
extern int gnRemovePair;
extern int gnSplitIsland;
extern int gnCollisionPairs;
extern int gnCollisionPairsWithContacts;
extern int gnSplitIterations;

/// default constructor
World::World(const C_AxisBox &size) :
	m_gravity(0,-9.0f,0),
	m_iIterations(5),
	m_iCurTag(1)
{
	m_pBroadPhase = new BroadPhase(this, size);
	m_pEvents = new EventList;
	m_iFirstFreeIsland = 0;
}

/// destructor
World::~World()
{
	Destroy();
}

/// return number of bodies; can include deleted 'NULL' bodies
int World::NumBodies()
{
	return m_bodies.Size();
}

/// get pointer to body with given index
Body *World::GetBody(int iBody)
{
	return m_bodies[iBody];
}

/// destroy the world
void World::Destroy()
{
	m_collisionPairs.Clear();
	m_bodies.Clear();
	delete m_pBroadPhase;
	m_pBroadPhase = NULL;
	delete m_pEvents;
	m_pEvents = NULL;
}

/// get the event list
EventList *World::GetEventList()
{
	assert(m_pEvents);
	return m_pEvents;
}

/// call to notify the broadphase of a changed body position
void World::UpdateBodyDeferred(Body *pBody, float fTimeStep)
{
	if (pBody->m_iBroadPhaseID != -1)
	{
		C_AxisBox bbox = pBody->GetAABB();

		// expand the bounding box by the body's movement across the timestep
//		HVec3 vExpand = pBody->m_velocity * fTimeStep;
//		bbox.Min( bbox.Min() + vExpand.Min(HVec3(0,0,0)));
//		bbox.Max( bbox.Max() + vExpand.Max(HVec3(0,0,0)));

		m_pBroadPhase->UpdateBodyDeferred(pBody->m_iBroadPhaseID, pBody->GetAABB());
		pBody->Dirty();
	}
}

/// add a new body to the world
int World::AddBody(Body *pBody)
{
	int i;
	for (i=0; i<m_bodies.Size(); i++)
	{
		if (m_bodies[i] == NULL)
		{
			m_bodies[i] = pBody;
			break;
		}
	}
	if (i>=m_bodies.Size())
		m_bodies.PushBack(pBody);

	if (!pBody->IsStatic())
	{
		// create a new island containing just this body
		int iNewIsland = GetFreeIsland();
		m_islands[iNewIsland].AddBody(pBody);
		pBody->m_iIsland = iNewIsland;
	}
	
	pBody->m_vPrevPos = pBody->m_vPos;
	pBody->m_qPrevRot = pBody->m_rotation;

	pBody->m_iBroadPhaseID = m_pBroadPhase->AddBody(i, pBody->GetAABB());

	return i;
}

/// remove a body from the world
void World::RemoveBody(Body *pBody)
{
	// this will remove the body from the broadphase and remove all it's contact pairs
	m_pBroadPhase->RemoveBody(pBody->m_iBroadPhaseID);
	for (int i=0; i<m_bodies.Size(); i++)
	{
		if (m_bodies[i] == pBody)
			m_bodies[i] = NULL;
	}

	// delete the island that this body is in
	if (pBody->m_iIsland != -1)
	{
		// check that it's the only body left in this island
		assert(m_islands[pBody->m_iIsland].NumBodies() == 1);
		assert(m_islands[pBody->m_iIsland].ContainsBody(pBody, false));

		FreeIsland(pBody->m_iIsland);
	}

	pBody->m_iBroadPhaseID = -1;

#ifndef CFG_SPEED
	// verify that there are no island left that contain this body
	for (int i=0; i<m_islands.Size(); i++)
	{
		assert(!m_islands[i].ContainsBody(pBody, true));
	}
#endif
}

/// returns the index of the body whose islands has the largest number of bodies in it
int World::GetLargestIslandBodyIdx(CollisionPair *p)
{
	// if one of the bodies is static, return the other
	if (p->m_pBody[0]->IsStatic())
		return 1;
	if (p->m_pBody[1]->IsStatic())
		return 0;

	// if one of the islands is bigger, return that one
	int i1 = p->m_pBody[0]->m_iIsland;
	int i2 = p->m_pBody[1]->m_iIsland;
	int c1 = m_islands[i1].NumBodies();
	int c2 = m_islands[i2].NumBodies();
	if (c1 > c2)
		return 0;
	if (c2 > c1)
		return 0;

	// return the body with the most collisionpairs
	if (p->m_pBody[0]->m_Pairs.Size() > p->m_pBody[1]->m_Pairs.Size())
		return 0;
	else
		return 1;
}

/// add a collision pair from the set
void World::AddCollisionPair(int iBody1, int iBody2)
{
	if (iBody1 >= 0xffff || iBody2>=0xffff || iBody1 == iBody2)
		return;

	// don't create collision pairs between static objects
	Body *p[2] = { m_bodies[iBody1], m_bodies[iBody2] };
	if (p[0]->IsStatic() &&
		p[1]->IsStatic())
		return;
	// perform collision filtering
	if ((p[0]->m_iCollisionMask & ((p[1]->m_iCollisionMask>>16) | (p[1]->m_iCollisionMask<<16))) == 0)
		return;

	if (iBody1 > iBody2)
	{
		int iTmp = iBody1;
		iBody1 = iBody2;
		iBody2 = iTmp;
	}
	CollisionPair pair(m_bodies[iBody1], m_bodies[iBody2]);

	// check whether this pair already exists
	Ut::UtSet<CollisionPair>::Iterator i = m_collisionPairs.Find(pair);
	if (i == m_collisionPairs.End())
	{
		// add new pair
		Ut::Pair<Ut::UtSet<CollisionPair>::Iterator,bool> rtn = m_collisionPairs.Insert(pair);
		CollisionPair *p = (CollisionPair*)&rtn.first->GetValue();
		nAddCollisionPair++;
//		LOGINFO( g_Log, "add collision pair (%d, %d)\n", iBody1, iBody2);
		int iBody = GetLargestIslandBodyIdx(p);
		m_bodies[iBody1]->AddPair(p);
		m_bodies[iBody2]->AddPair(p);
		m_islands[p->m_pBody[iBody]->m_iIsland].AddPair(p, iBody);
	}
}

/// remove a collision pair from the set
void World::RemoveCollisionPair(int iBody1, int iBody2)
{
	if (iBody1 >= 0xffff || iBody2>=0xffff || iBody1 == iBody2)
		return;

	// ignore collision pairs between empty objects
	if (m_bodies[iBody1]->IsStatic() &&
		m_bodies[iBody2]->IsStatic())
		return;

	if (iBody1 > iBody2)
	{
		int iTmp = iBody1;
		iBody1 = iBody2;
		iBody2 = iTmp;
	}

	CollisionPair pair(m_bodies[iBody1], m_bodies[iBody2]);
	// find this pair in the set
	Ut::UtSet<CollisionPair>::Iterator i = m_collisionPairs.Find(pair);
	if (i != m_collisionPairs.End())
	{
		// remove this collisionpair from the world
  		CollisionPair *p = (CollisionPair*)&i->GetValue();
		nRemoveCollisionPair++;
//		LOGINFO( g_Log, "remove collision pair (%d, %d)\n", iBody1, iBody2);
		int iBody =	GetLargestIslandBodyIdx(p);
		int iIsland = p->m_pBody[iBody]->m_iIsland;
		m_islands[iIsland].RemovePair(p, iBody);

 		m_collisionPairs.Erase(i);
	}
}

/// return free island index
int World::GetFreeIsland()
{
	int i;
	if (m_iFirstFreeIsland >= m_islands.Size())
	{
		i = m_islands.Size();
		m_islands.PushBack();
		m_islands.Back().m_bInActiveList = false;
		m_iFirstFreeIsland++;
	}
	else
	{
		i = m_iFirstFreeIsland;
		m_iFirstFreeIsland = m_islands[i].m_iNextFreeIsland;
	}

	m_islands[i].Initialise(this, i);

//	LOGINFO( g_Log, "getisland %d\n", i);
	return i;
}

/// free island by index
void World::FreeIsland(int i)
{
	if (i == -1 || m_islands.Size() <= i || m_islands[i].m_bFree)
	{
//		LOGINFO( g_Log, "freeisland %d already free\n", i);
		return; // already freed
	}
//	LOGINFO( g_Log, "freeisland %d\n", i);
	assert(m_islands[i].m_iNextFreeIsland == -1);
	assert(m_iFirstFreeIsland <= m_islands.Size());
	m_islands[i].Clear();
	if (i == m_islands.Size())
	{
		m_islands.PopBack();
		return;
	}

	m_islands[i].m_iNextFreeIsland = m_iFirstFreeIsland;
	m_iFirstFreeIsland = i;
}

/// add an island to the active island list
void World::AddActiveIsland(int i)
{
	for (int j=0; j<m_ActiveIslands.Size(); j++)
	{
		assert(m_ActiveIslands[j] != i);
	}
	m_ActiveIslands.PushBack(i);
}

/// do the physics update
void World::Update(float fTimeStep)
{
#ifdef PLATFORM_PC
//	if (fTimeStep > 0.01f) fTimeStep = 0.01f;
#endif
//	if (fTimeStep > 0.001f) fTimeStep = 0.001f;

	if (!g_bEnable)
		return;

	int nContacts = 0;

	Time::C_Time tUpdate = Time::GetPerformanceCounter();

	// for each island, run the collision, solver and update
	int nBodies = 0;
	for (int iIsland=0; iIsland < m_ActiveIslands.Size(); iIsland++)
	{
		Island *island = &m_islands[m_ActiveIslands[iIsland]];
		if (island->m_bActive)
		{
			nBodies += island->m_bodies.Size();

			island->Step(fTimeStep, m_gravity);
		}
		else
		{
			if (island->m_bInActiveList)
			{
				island->m_bInActiveList = false;
				m_ActiveIslands.EraseByIndex(iIsland);
				iIsland--;
			}
		}
	}

	m_pBroadPhase->Update();
#ifdef PLATFORM_PC
	Util::C_String str;

	tUpdate = Time::GetPerformanceCounter() - tUpdate;
	float f = (float)tUpdate/1000.f;
//	PerformanceGraph::Instance().Update(f);

	int nIslands = 0;
	for (int i=0; i<m_islands.Size(); i++)
		if (!m_islands[i].m_bFree) nIslands++;

	str.Printf("update: %5.2fms", f);
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw2dText(Maths::HVec2(820, 50), str);

	str.Printf("islands: %d (%d)", nIslands, m_ActiveIslands.Size());
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw2dText(Maths::HVec2(50, 85), str);

	str.Printf("Bodies: %d", nBodies);
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw2dText(Maths::HVec2(50, 110), str);

//	str.Printf("ct-pairs: %d", m_CollisionPairVector.Size());
//	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw2dText(Maths::HVec2(50, 135), str);

//	str.Printf("active pairs: %d (%5.2fK)", n, float(n * sizeof(CollisionPair)) / 1024.f);
//	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw2dText(Maths::HVec2(550, 45), str);

//	str.Printf("isectedge: %d", gnIntersectEdge);
//	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw2dText(Maths::HVec2(820, 75), str);

	str.Printf("cp +%d -%d", nAddCollisionPair, nRemoveCollisionPair);
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw2dText(Maths::HVec2(820, 75), str);

	str.Printf("%d/%d %d", gnSplitIsland, gnRemovePair, gnSplitIterations);
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw2dText(Maths::HVec2(820, 110), str);
	gnRemovePair = 0;
	gnSplitIsland = 0;
	gnSplitIterations = 0;

	str.Printf("%d/%d", gnCollisionPairsWithContacts, gnCollisionPairs);
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw2dText(Maths::HVec2(820, 110), str);
	gnCollisionPairs = 0;
	gnCollisionPairsWithContacts = 0;

/*
	int v = 0;
	for (int i=0; i<m_islands.Size(); i++)
	{
		if (m_islands[i].m_bodies.Size() > 0)
		{
			str = m_islands[i].DebugString();
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw2dText(Maths::HVec2(50, 170+v*30), str);
			v++;
		}
	}
/*
	Util::C_String tmp;
	for (int i=0; i<m_bodies.Size(); i++)
	{
		str.Printf("b %d: ", m_bodies[i]->m_iBroadPhaseID);
		for (int x=0; x<m_bodies[i]->m_CollisionPairs.Size(); x++)
		{
			CollisionPair *pCP = m_bodies[i]->m_CollisionPairs[x];
			if (pCP->m_pBody[0] == m_bodies[i])
			{
				tmp.Printf("%d ", pCP->m_pBody[1]->m_iBroadPhaseID);
			}
			else
			{
				tmp.Printf("%d ", pCP->m_pBody[0]->m_iBroadPhaseID);
			}
			str = str + tmp;
		}
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw2dText(Maths::HVec2(550, 170+i*30), str);
	}
*/
	gnIntersectEdge = 0;
	nRemoveCollisionPair = 0;
	nAddCollisionPair = 0;
#endif
}

}; // namespace Physic
