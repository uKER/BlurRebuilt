#ifndef __PHYSICS_ISLAND_H__
#define __PHYSICS_ISLAND_H__

namespace Impact
{

class Body;
class BodyPair;
class CollisionPair;
class World;
class Contact;

class Island
{
	friend class World;
public:
	/// initialise
	void Initialise(World *pWorld, int iIsland);

	/// clear contents
	void Clear();

	/// perform physics step (narrowphase, solve, update)
	void Step(float fTimeStep, const Maths::HVec3 &vGravity);

	/// process narrowphase for one island
	void NarrowPhase();

	/// run solver for this island
	void Solver(float fTimeStep);

	/// run update on the bodies in this island
	void Update(float fTimeStep, const Maths::HVec3 &vGravity);

	/// recursively count the number of connected bodies
	static int CountBodyRecursive(Body *pBody, int iTag, Body *pTarget);

	/// add the given body to the _m_bodies_ list
	void AddBody(Body *pBody);

	/// return the number of bodies in this island
	int NumBodies();

	/// add a collision pair
	void AddPair(BodyPair *p, int iBody);

	/// remove a collision pair
	void RemovePair(BodyPair *p, int iBody);

	/// determine whether a split is neccesary, and which bodies to split
	bool SearchAndTagConnectedNodes(Body *pOurBody, Body *pOtherBody, int &nTaggedBodies, int &nTaggedPairs, int iTag);

	/// split an island into two; move all tagged bodies and collisionpairs into the new island
	void SplitIsland(int iNewIsland, int nTaggedBodies, int nTaggedPairs, int iTag);

	/// generate a debug information string
	Util::C_String DebugString();

	/// returns true if this island contains the given body
	bool ContainsBody(Body *pBody, bool bSearchPairs);

	/// check that the bodies in the island match the pairs and vice versa
	void Verify();

private:
	/// set island active status
	void SetActive(bool bActive);

	/// emit a collision event
	void EmitCollisionEvent(CollisionPair *pCP);

protected:
	Ut::Vector<BodyPair*>		m_Pairs;
	Ut::Vector<BodyPair*>		m_ActivePairs;
	Ut::Vector<Body*>			m_bodies;
	int							m_iNextFreeIsland;
	int							m_iIsland;
	World						*m_pWorld;
	bool						m_bFree;
	bool						m_bActive;
	bool						m_bInActiveList;
	int							m_nContinuousCollisionBodies;
};

};


#endif
