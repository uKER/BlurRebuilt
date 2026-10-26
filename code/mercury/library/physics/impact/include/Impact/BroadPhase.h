#ifndef __PHYSICS_BROADPHASE_H__
#define __PHYSICS_BROADPHASE_H__

namespace Impact
{

extern bool g_bPutExtraInfoInDmpFile;
class World;

/// implementation of the sweep-and-prune algorithm to track collisions between axis aligned bounding boxes
class BroadPhase
{
protected:
	/// integer representation of a 3D position
	struct U16Box
	{
		U16 min[3], max[3];
	};

	/// item for updating the bounding box of one body
	struct UpdateItem
	{
		int iBodyID;
		Maths::C_AxisBox aabb;
	};

	/// represent a aabb edge
	struct Edge
	{
		U16 m_iPos;
		U16 m_iItem;

		// the LSB in m_iPos stores whether it's an opening or closing edge
		int EdgeType() const {return (m_iPos & 1);}
	};

	/// stores position of a body's edges in the sorted list
	struct Item
	{
	public:
		U16 m_iEdge[2][3];
		U16 m_iBody;
	};

public:
	/// constructor
				BroadPhase(World *pWorld, const Maths::C_AxisBox &worldBBox);
	/// destructor
				~BroadPhase();
	/// add a body to the list. returns ID to be used with UpdateBody
	int			AddBody(int iBody, const Maths::C_AxisBox &aabb, bool bSortEdges = true);
	/// remove body from broadphase. calls RemoveOverlappedPair
	void		RemoveBody(int iBody);
	/// update the bounding box of the given body
	void		UpdateBody(int iBodyID, const Maths::C_AxisBox &bbox);
	/// apply updates
	void		Update();

	void		UpdateBodyDeferred(int iBodyID, const Maths::C_AxisBox &aabb);

	/// get pointers to bit of memory that are interesting to write into .dmp file
	void		GetPointers(Ut::Vector<void*> &pointers, Ut::Vector<int> &sizes);

protected:
	/// convert a position into a U16Box
	U16Box		GetU16Box(const Maths::C_AxisBox &bbox);
	/// returns true if the two bodies's aabbs intersect
	bool		TestCollision(const Item *pItem1, const Item *pItem2, int iSkipAxis);

	/// check the contents of the broadphase class
	void		Verify();

	// move an edge downwards
	template<typename CALL, int EDGETYPE>
	void		MoveEdgeDown(int axis, U16 edge);
	// move an edge downwards
	template<typename CALL, int EDGETYPE>
	void		MoveEdgeUp(int axis, U16 edge);
	// move an edge up to the top and remove it
	template<typename CALL, int EDGETYPE>
	void		RemoveEdge(int iAxis, U16 iEdge);

	// size of one int (1) measured in floating point world-space units
	Maths::HVec3		m_cellSize;
	// extents of the world aabb
	Maths::C_AxisBox	m_worldBBox;
	// edges
	Ut::Vector<Edge>	m_edges[3];
	// items
	Ut::Vector<Item>	m_items;
	// the world - this keeps track of collision pairs
	World				*m_pWorld;
	// list of updates to make to the broadphase
	Ut::Vector<UpdateItem> m_updates;
};

/// callback class for not updating overlapping pairs
class DoNotUpdate : public BroadPhase
{
public:
	void Go(int iAxis, Item *pBody1, Item *pBody2);
};

/// callback class for adding an overlapped pair
class AddOverlappedPair : public BroadPhase
{
public:
	void Go(int iAxis, Item *pBody1, Item *pBody2);
};

/// callback class for removing an overlapped pair
class RemoveOverlappedPair : public BroadPhase
{
public:
	void Go(int iAxis, Item *pBody1, Item *pBody2);
};

};


#endif
