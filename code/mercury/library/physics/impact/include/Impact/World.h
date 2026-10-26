#ifndef __PHYSICS_WORLD_H__
#define __PHYSICS_WORLD_H__

namespace Impact
{

class Body;
class BroadPhase;
class AddOverlappedPair;
class RemoveOverlappedPair;
class Island;
class EventList;

struct TagJob
{
	int iTag;
	Body *pBody;
};

class World
{
	friend class AddOverlappedPair;
	friend class RemoveOverlappedPair;
	friend class Island;
public:
	/// default constructor
						World(const Maths::C_AxisBox &bbox);

	/// destructor
						~World();

	/// add a new body to the world
	int					AddBody(Body *pBody);

	/// remove a body from the world
	void				RemoveBody(Body *pBody);

	/// return number of bodies; can include deleted 'NULL' bodies
	int					NumBodies();

	/// get pointer to body with given index
	Body				*GetBody(int iBody);

	/// do the physics update
	void				Update(float fTimeStep);

	/// destroy the world
	void				Destroy();

	/// call to notify the broadphase of a changed body position
	void				UpdateBodyDeferred(Body *pBody, float fTimeStep);

	/// get the event list
	EventList			*GetEventList();

protected:
	// -- for use by broadphase
						/// add a collision pair from the set
	void				AddCollisionPair(int iBody1, int iBody2);

						/// remove a collision pair from the set
	void				RemoveCollisionPair(int iBody1, int iBody2);

	/// add an island to the active island list
	void				AddActiveIsland(int i);
private:
	/// free island index
	int					GetFreeIsland();

	/// clear an island and free it
	void				ClearIsland(int i);

	/// free island by index
	void				FreeIsland(int i);

	/// returns the index of the body whose islands has the largest number of bodies in it
	int					GetLargestIslandBodyIdx(CollisionPair *p);

protected:
	Ut::Vector<Body*>	m_bodies;
	Ut::UtSet<CollisionPair>
						m_collisionPairs;

	Maths::HVec3		m_gravity;	// the force of gravity
	int					m_iIterations;

public:
	BroadPhase			*m_pBroadPhase;
protected:

	Ut::Vector<Island>	m_islands;
	Ut::Vector<int>		m_ActiveIslands;
	int					m_iFirstFreeIsland;
	int					m_iCurTag;

	EventList			*m_pEvents;
};

};

#endif
