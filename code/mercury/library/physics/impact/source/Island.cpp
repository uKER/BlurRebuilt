#include "stdafx.h"

#include "Impact\Island.h"
#include "Impact\Body.h"
#include "Impact\Collision.h"
#include "Impact\CollisionPair.h"
#include "Impact\Constraint.h"
#include "Impact\World.h"
#include "Impact\BroadPhase.h"
#include "Impact\Event.h"
#include "System\Memory\MemoryTracker.h"
#include <algorithm>

//#define PARANOID

using namespace Maths;

namespace Impact
{

Debug::Int g_iIterations("Impact.Iterations", 4, Debug::Range(1,10));
Debug::Bool g_bUseActiveCollision("Impact.Optimizations.UseActivePairsList", false);
Debug::Bool g_bDebugCrash("Impact.DebugCrash", true);
Debug::Bool g_bUpdate("Impact.Update", true);
bool g_bUpdateKey = false;

extern Debug::Logger &g_Log;

// perf statistics
int gnRemovePair = 0;
int gnSplitIsland = 0;
int gnCollisionPairs = 0;
int gnCollisionPairsWithContacts = 0;
int gnSplitIterations = 0;

/// initialise
void Island::Initialise(World *pWorld, int iIsland)
{
	assert(m_bodies.Size() == 0);
	m_bActive = true;
	m_nContinuousCollisionBodies = 0;
	m_iNextFreeIsland = -1;
	m_bFree = false;
	m_pWorld = pWorld;
	m_iIsland = iIsland;
	SetActive(true);
}

/// set island active status
void Island::SetActive(bool bActive)
{
	// if we have gone from active to inactive, free all collision pairs collision cache data
	// this is to save memory for inactive islands
	if (m_bActive == true && bActive == false)
	{
		for (int i=0; i<m_Pairs.Size(); i++)
		{
			if (m_Pairs[i]->GetType() == E_PairType_Collision)
				((CollisionPair*)m_Pairs[i])->FreePersistentData();
		}
	}

	m_bActive = bActive;
	// if we have gone from inactive to active, add us to the world's active island list
	if (m_bInActiveList == false && m_bActive == true)
	{
		m_pWorld->AddActiveIsland(m_iIsland);
		m_bInActiveList = true;
	}
}

/// clear contents
void Island::Clear()
{
	m_Pairs.Clear();
	m_bodies.Clear();
	m_bFree = true;
	m_bActive = false;
}

/// add the given body to the _m_bodies_ list
void Island::AddBody(Body *pBody)
{
	int iBody = m_bodies.Size();
	m_bodies.PushBack(pBody);
	for (int i=0; i<pBody->m_Pairs.Size(); i++)
		AddPair(pBody->m_Pairs[i], iBody);

	if (pBody->ContinuousCollision())
		m_nContinuousCollisionBodies++;
}

/// return the number of bodies in this island
int Island::NumBodies()
{
	return m_bodies.Size();
}

/// perform physics step
void Island::Step(float fTimeStep, const Maths::HVec3 &vGravity)
{
#ifdef PARANOID
	Verify();
#endif

	if (m_nContinuousCollisionBodies>0)
	{
		// if the island contains continuous collision detection objects, run the simulation at 4x the frequency
		fTimeStep /= 4.f;
		for (int i=0; i<4; i++)
		{
			Update(fTimeStep, vGravity);
			if (!m_bActive)
				return;

			NarrowPhase();

			Solver(fTimeStep);
		}
	}
	else
	{
		// normal simulation update
		Update(fTimeStep, vGravity);
		if (!m_bActive)
			return;

		NarrowPhase();

		Solver(fTimeStep);
	}

#ifdef PARANOID
	Verify();
#endif
}


/// add a pair
void Island::AddPair(BodyPair *p, int iBody)
{
	Body *pOurBody = p->m_pBody[iBody];
	Body *pOtherBody = p->m_pBody[iBody^1];

	// add collision pair
	m_Pairs.PushBack(p);

	if (pOtherBody == NULL)
		return;

	// if the other body is already in this island or if it's static we don't need to do anything else
	int iOtherIsland = pOtherBody->m_iIsland;
	if (iOtherIsland == m_iIsland || pOtherBody->IsStatic())
	{
#ifdef PARANOID
		Verify();
#endif
		return;
	}

//	LOGINFO( g_Log, "Merge island %d, %d\n", m_iIsland, iOtherIsland);

	// merge islands
	Island *pOtherIsland = &m_pWorld->m_islands[iOtherIsland];
	// copy bodies and map them to this island
	for (int i=0; i<pOtherIsland->m_bodies.Size(); i++)
	{
		Body *pBody = pOtherIsland->m_bodies[i];
		m_bodies.PushBack(pBody);
		pBody->m_iIsland = m_iIsland;
	}
	// copy collision pairs
	for (int i=0; i<pOtherIsland->m_Pairs.Size(); i++)
		m_Pairs.PushBack(pOtherIsland->m_Pairs[i]);

	// delete the other island
	pOtherIsland->m_Pairs.Clear();
	m_nContinuousCollisionBodies += pOtherIsland->m_nContinuousCollisionBodies;
	m_pWorld->FreeIsland(iOtherIsland);
	SetActive(true);

#ifdef PARANOID
	Verify();
	pOtherIsland->Verify();
#endif
}

/// returns true if this island contains the given body
bool Island::ContainsBody(Body *pBody, bool bSearchPairs)
{
	for (int i=0; i<m_bodies.Size(); i++)
	{
		if (m_bodies[i] == pBody)
			return true;
	}

	if (bSearchPairs)
	{
		for (int i=0; i<m_Pairs.Size(); i++)
		{
			if (m_Pairs[i]->m_pBody[0] == pBody ||
				m_Pairs[i]->m_pBody[1] == pBody)
				return true;
		}
	}

	return false;
}

/// remove a collision pair
void Island::RemovePair(BodyPair *p, int iBody)
{
	Body *pOurBody = p->m_pBody[iBody];
	Body *pOtherBody = p->m_pBody[iBody^1];
	pOurBody->RemovePair(p);
	pOtherBody->RemovePair(p);

	// find the collision pair and delete it
	for (int i=0; i<m_Pairs.Size(); i++)
	{
		if (m_Pairs[i] == p)
		{
			m_Pairs.EraseByIndex(i);
			break;
		}
	}
	// if detached body is static then we're done
	if (pOtherBody->IsStatic())
	{
#ifdef PARANOID
		Verify();
#endif
		return;
	}

	gnRemovePair++;

	// see if _pOurBody_ and _pOtherBody_ are still connected, and tag all bodies connected to _pOtherBody_
	int nTaggedBodies, nTaggedPairs;
	int iTag = m_pWorld->m_iCurTag++;
	if (SearchAndTagConnectedNodes(pOurBody, pOtherBody, nTaggedBodies, nTaggedPairs, iTag))
	{
#ifdef PARANOID
		Verify();
#endif
		return;
	}

	gnSplitIsland++;

	// create a new island and move the tagged bodies and pairs
	// take care: resizing the island array can cause this _this_ pointer to become invalid
	int iThisIsland = m_iIsland;
	World *pWorld = m_pWorld;
	int iNewIsland = pWorld->GetFreeIsland();
	pWorld->m_islands[iThisIsland].SplitIsland(iNewIsland, nTaggedBodies, nTaggedPairs, iTag);
}

/// split an island into two; move all tagged bodies and pairs into the new island
void Island::SplitIsland(int iNewIsland, int nTaggedBodies, int nTaggedPairs, int iTag)
{
	Island *pIsland = &m_pWorld->m_islands[iNewIsland];
	pIsland->m_bodies.Reserve(nTaggedBodies);
	pIsland->m_Pairs.Reserve(nTaggedPairs);

	// move tagged bodies to the new island
	for (int i=0; i<m_bodies.Size(); i++)
	{
		if (m_bodies[i]->m_iTag == iTag)
		{
			Body *pBody = m_bodies[i];
			pIsland->m_bodies.PushBack(pBody);
			m_bodies[i] = m_bodies.Back();
			m_bodies.PopBack();
			pBody->m_iIsland = iNewIsland;
			if (pBody->ContinuousCollision())
			{
				pIsland->m_nContinuousCollisionBodies++;
				m_nContinuousCollisionBodies--;
			}
			i--;
		}
	}
	// move tagged collision pairs to the new island
	for (int i=0; i<m_Pairs.Size(); i++)
	{
		if (m_Pairs[i]->m_iTag == iTag)
		{
			pIsland->m_Pairs.PushBack(m_Pairs[i]);
			m_Pairs[i] = m_Pairs.Back();
			m_Pairs.PopBack();
			i--;
		}
	}

#ifdef PARANOID
	Verify();
	pIsland->Verify();
#endif
}


/// helper struct for SearchAndTagConnectedNodes
struct SortItem
{
	Body *pBody;
	float fDist;
};

/// operator for stl sorting
inline bool operator < (const SortItem &a1, const SortItem &a2)
{
	return a1.fDist > a2.fDist;
}

/// determine whether a split is neccesary, and which bodies to split
/// uses a depth-first search to find a connection between _pOurBody_ and _pOtherBody_
/// returns true if a connection was found
/// tags all bodies and collision pairs connected to _pOtherBody_ so we know which bodies to split
bool Island::SearchAndTagConnectedNodes(Body *pOurBody, Body *pOtherBody, int &nTaggedBodies, int &nTaggedPairs, int iTag)
{
	nTaggedBodies = 0;
	nTaggedPairs = 0;
	Ut::Vector<Body*> stack;
	Ut::Vector<SortItem> sortList;

	// reset tags on all bodies and pairs in the island
	// TODO: PERF: initialise tags properly and keep track of the minimum tag, so that we don't have to reset them everytime
	for (Ut::Vector<BodyPair*>::Iterator iter=m_Pairs.Begin(); iter!=m_Pairs.End(); ++iter)
		(*iter)->m_iTag = iTag-1;
	for (Ut::Vector<Body*>::Iterator iter=m_bodies.Begin(); iter!=m_bodies.End(); ++iter)
		(*iter)->m_iTag = iTag-1;

	// recursively step through all bodies connected to _pOtherBody_
	stack.PushBack(pOtherBody);
	while (stack.Size())
	{
		Body *pBody = stack.Back(); stack.PopBack();
		gnSplitIterations++;
		pBody->m_iTag = iTag;
		nTaggedBodies++;
		sortList.Clear();
		// get a list of all connected bodies
		for (Ut::Vector<BodyPair*>::Iterator iter=pBody->m_Pairs.Begin(); iter!=pBody->m_Pairs.End(); ++iter)
		{
			BodyPair *pPair = *iter;
			Body *pOther;
			if (pPair->m_iTag != iTag)
			{
				pPair->m_iTag = iTag;
				nTaggedPairs++;
				if (pPair->m_pBody[0] == pBody)
					pOther = pPair->m_pBody[1];
				else
					pOther = pPair->m_pBody[0];
				if (pOther && pOther->m_iTag != iTag)
				{
					// check whether this is the source body and if it is, quit because we don't need to split
					if (pOther == pOurBody)
						return true;
					sortList.PushBack();
					sortList.Back().pBody = pOther;
					sortList.Back().fDist = (pOther->m_vPos - pOurBody->m_vPos).Length2();
				}
			}
		}
		int n = sortList.Size();
		if (n > 0)
		{
			// sort the bodies so nearest ones are at the top of the stack
			std::sort(&sortList[0], &sortList[0] + n);
			// add to stack
			for (int i=0; i<n; i++)
				stack.PushBack(sortList[i].pBody);
		}

	}
	return false;
}

/// recursively count the number of connected bodies
int Island::CountBodyRecursive(Body *pBody, int iTag, Body *pFind)
{
	int nBodies = 0;

	if (pBody->IsStatic())
		return -2;

	Ut::Vector<Body*> stack;
	stack.Reserve(128);
	stack.PushBack(pBody);
	while (stack.Size())
	{
		pBody = stack.Back(); stack.PopBack();
		pBody->m_iTag = iTag;
		if (pBody == pFind)
			return -nBodies;
		nBodies++;
		for (Ut::Vector<BodyPair*>::Iterator iter=pBody->m_Pairs.Begin(); iter!=pBody->m_Pairs.End(); ++iter)
		{
			BodyPair *pPair = *iter;
			Body *pOther;
			if (pPair->m_iTag != iTag)
			{
				pPair->m_iTag = iTag;
				if (pPair->m_pBody[0] == pBody)
					pOther = pPair->m_pBody[1];
				else
				{
					pOther = pPair->m_pBody[0];
					assert(pPair->m_pBody[1] == pBody);
				}
				if (pOther->m_iTag != iTag)
				{
					stack.PushBack(pOther);
				}
			}
		}
	}
	return nBodies;
}

/// emit a collision event
void Island::EmitCollisionEvent(CollisionPair *pCP)
{
	Body *pBody0 = pCP->m_pBody[0];
	Body *pBody1 = pCP->m_pBody[1];

	Event event;
	event.type = E_EventType_Collision;
	event.pBody = pBody0;
	event.collisionEvent.pOtherBody = pBody1;
	event.collisionEvent.vPos[0] = pCP->m_Contacts[0].m_vPos.X();
	event.collisionEvent.vPos[1] = pCP->m_Contacts[0].m_vPos.Y();
	event.collisionEvent.vPos[2] = pCP->m_Contacts[0].m_vPos.Z();

	// Relative velocity at contact
	Contact *c = &pCP->m_Contacts[0];
	HVec3 r1 = c->m_vPos - pBody0->m_vPos;
	HVec3 r2 = c->m_vPos - pBody1->m_vPos;
	HVec3 dv = pBody1->m_velocity + (pBody1->m_angularVelocity ^ r2) - pBody0->m_velocity - (pBody0->m_angularVelocity ^ r1);
	event.collisionEvent.fImpulseStrength = dv.Length();

	m_pWorld->GetEventList()->PushBack(event);
}

/// process narrowphase for one island
void Island::NarrowPhase()
{
	// perform collision and make a list of all active collision pairs for the solver
	m_ActivePairs.Clear();

	for (int _i=0; _i<m_Pairs.Size(); _i++)
	{
		BodyPair *i = m_Pairs[_i];
		CollisionPair *iCP = (CollisionPair*)i;
		Body *p1 = i->m_pBody[0], *p2 = i->m_pBody[1];

		if (!i->m_bActive)
		{
			if (i->m_type == E_PairType_Collision && iCP->m_Contacts.Size() > 0)
				m_ActivePairs.PushBack(i);

			continue;
		}

		if (i->m_type == E_PairType_Collision)
		{
			ContactVector contacts;

			// perform collision
			Collide(contacts, p1, p2, &iCP->m_pPersistentData);

			if (contacts.Size() > 0)
			{
				bool bNewCollision = (iCP->m_Contacts.Size() == 0);

				// update arbiter with contacts
				iCP->UpdateContacts(&contacts[0], contacts.Size());

				m_ActivePairs.PushBack(i);

				// these two bodies have just started to collide. register a collision event
				if (bNewCollision && ((p1->m_EventFilter|p2->m_EventFilter) & IMPACT_EVENTFILTER_COLLISION))
					EmitCollisionEvent(iCP);
			}
			else
			{
				// clear the collision pair's contacts
				iCP->UpdateContacts(NULL,0);
			}

			// if both bodies are inactive, deactivate the collisionpair
			if (!p1->IsActive() && !p2->IsActive())
				iCP->m_bActive = false;
		}
		else
			m_ActivePairs.PushBack(i);
	}
}

/// run solver for this island
void Island::Solver(float fTimeStep)
{
	if (!m_ActivePairs.Size())
		return;

	// pre-step
	int n = m_ActivePairs.Size();
	for (int j=0; j<n; j++)
	{
		switch (m_ActivePairs[j]->GetType())
		{
		case E_PairType_Collision:
			((CollisionPair*)m_ActivePairs[j])->PreStep(1.f/fTimeStep);
			break;
		case E_PairType_Constraint:
			((Constraint*)m_ActivePairs[j])->PreStep(1.f/fTimeStep);
			break;
		}
	}

	// apply impulses
	for (int i=0; i<g_iIterations; i++)
	{
		for (int j=0; j<n; j++)
		{
			switch (m_ActivePairs[j]->GetType())
			{
			case E_PairType_Collision:
				((CollisionPair*)m_ActivePairs[j])->Step();
				break;
			case E_PairType_Constraint:
				((Constraint*)m_ActivePairs[j])->Step();
				break;
			}
		}
	}
}

/// run update on the bodies in this island
void Island::Update(float fTimeStep, const HVec3 &vGravity)
{
	bool bActive = false;
	Ut::Vector<Body*> deleteList;

	for (int i=0; i<m_bodies.Size(); i++)
	{
		Body *pBody = m_bodies[i];

		assert(!pBody->IsStatic());

		pBody->UpdateActivationStatus(fTimeStep);
		bActive |= pBody->m_bActive;

		pBody->m_vPrevPos = pBody->m_vPos;
		pBody->m_qPrevRot = pBody->m_rotation;

		if (g_bUpdate || g_bUpdateKey)
		{
			pBody->m_vPos += (HF32)fTimeStep * pBody->m_velocity;
			// convert angular velocity and timestep to quaternion
			C_Quaternion spin(HVec4(-pBody->m_angularVelocity.X(), -pBody->m_angularVelocity.Y(), -pBody->m_angularVelocity.Z(), 0.f));

			pBody->m_rotation = pBody->m_rotation + (spin * pBody->m_rotation ) * 0.5f * fTimeStep;
			pBody->m_rotation.Normalise();
		}
		pBody->m_velocity += (HF32)fTimeStep * vGravity;

		// update new bounding into broadphase
		m_pWorld->UpdateBodyDeferred(pBody, fTimeStep);
	}

	// activate/deactivate island
	SetActive(bActive);
}

/// debug
Util::C_String Island::DebugString()
{
	Util::C_String text, tmp;
	text.Printf("I %d: (%d) ", m_iIsland, m_bodies.Size());
	for (int i=0; i<m_bodies.Size(); i++)
	{
		assert(m_bodies[i]->m_iIsland == m_iIsland);
		tmp.Printf("%d, ",m_bodies[i]->m_iBroadPhaseID);
		text = text + tmp;
	}

	for (int i=0; i<m_Pairs.Size(); i++)
	{
		tmp.Printf("(%d,%d) ",m_Pairs[i]->m_pBody[0]->m_iBroadPhaseID, m_Pairs[i]->m_pBody[1]->m_iBroadPhaseID);
		text = text + tmp;
	}

	return text;
}

/// check that the bodies in the island match the pairs and vice versa
void Island::Verify()
{
	// for each contact pair, check that the body exists in the body list
	for (int i=0; i<m_Pairs.Size(); i++)
	{
		Body *pBody = m_Pairs[i]->m_pBody[0];
		assert(!pBody || pBody->IsStatic() || ContainsBody(pBody, false));
		pBody = m_Pairs[i]->m_pBody[1];
		assert(!pBody || pBody->IsStatic() || ContainsBody(pBody, false));
	}
}



}; // namespace Impact
