#ifndef RP_COLLISION_MESH_H
#define RP_COLLISION_MESH_H
#ifdef _MSC_VER
#pragma once
#endif

#include "btl/containers/chainedhashmap.h"
#include "racingphysics/physics/geometry.h"
#include "misc_shared/archive/serialise.h"

namespace RacingPhysics
{

const U32 COLLISION_FLAG_ALL = 0;
const U32 COLLISION_FLAG_NOT_GROUND = 1<<0;
const U32 COLLISION_FLAG_NOT_WALL = 1<<1;
const U32 COLLISION_FLAG_NOT_OVERLAY = 1<<2;
const U32 COLLISION_FLAG_NOT_SOLID = 1<<3;

class CollisionMeshSurfaceFlags
{
public:
	enum SurfaceType 
	{
		SURFACE_TYPE_ASPHALT = 0,
		SURFACE_TYPE_GRASS,
		SURFACE_TYPE_GRAVEL,
		SURFACE_TYPE_COBBLES,
		SURFACE_TYPE_PAVING,
		SURFACE_TYPE_CONCRETE,
		SURFACE_TYPE_RUMBLE,
		SURFACE_TYPE_WOOD,
		SURFACE_TYPE_TARMAC,
		SURFACE_TYPE_SANDYROAD,
		SURFACE_TYPE_METALPLATE,
		SURFACE_TYPE_MANHOLES,
		SURFACE_TYPE_RIDGEDGUTTER,
		SURFACE_TYPE_BRIDGEMETALJOINTS,
		SURFACE_TYPE_CATSEYES,
		SURFACE_TYPE_STICKY,
		SURFACE_TYPE_TARMAC_NOT_ON_CIRCUIT,
		SURFACE_TYPE_RUMBLE_LINES,

		SURFACE_TYPE_MAX
	};

	enum WaterType
	{
		WATER_NONE,
		WATER_SHALLOW,
		WATER_DEEP,
	};

	union 
	{
		U8 flags;

		struct
		{
			U8 ice : 1;
			U8 water : 2;
			U8 surfaceType : 5;
		};
	};

	SurfaceType Surface() const {return (SurfaceType) surfaceType;}
	WaterType Water() const {return (WaterType) water;}
	bool Ice() const {return ice != 0;}
	bool AIIgnore() const {return (water || ice) && (surfaceType == 1);}
	U8 RawValue() {return flags;}

	CollisionMeshSurfaceFlags() : flags(0) {}
	explicit CollisionMeshSurfaceFlags(U8 i) : flags(i) {}
};

class CollisionMeshCellIndex
{
public:
	CollisionMeshCellIndex(int i, int j)
	{
		// index range of 32767 = ~1000 km
		assert((i & 0xffff0000) == 0 || (i & 0xffff0000) == 0xffff0000);
		assert((j & 0xffff0000) == 0 || (j & 0xffff0000) == 0xffff0000);
		m_index = ((i & 0xffff) << 16) + (j & 0xffff);
	}

	U32 GetIndex() const {return m_index;}

	bool operator == (const CollisionMeshCellIndex& other) const {return m_index == other.m_index;}

	template <typename Archive>
	void Serialise(Archive& ar) {ar & m_index;}

	AUTO_DESERIALISE_MEMBER()

private:
	U32 m_index;
};

inline U32 Hash(const CollisionMeshCellIndex& index)
{
	return Btl::Hash(index.GetIndex());
}

struct CollisionMeshTriangle
{
	U16 i0, i1, i2;
	U16 flags;
};

template <typename Archive>
void Serialise(Archive& ar, CollisionMeshTriangle& t)
{
	ar & t.i0;
	ar & t.i1;
	ar & t.i2;
}

class CollisionMeshCell
{
public:
	Point3R& GetMin() {return m_min;}
	Point3R& GetMax() {return m_max;}

	//////////////////////////////////////////////////////////////////
	// build
	void SetMin(const Point3R& p) {m_min = p;}
	void SetMax(const Point3R& p) {m_max = p;}

	void AddTriangle(const Point3R& p0, const Point3R& p1, const Point3R& p2);

	template <typename Archive>
	void Serialise(Archive& ar)
	{
		ar & m_min;
		ar & m_max;
		ar & m_vertices;
		ar & m_triangles;
	}

private:
	Point3R m_min;
	Point3R m_max;

	typedef Btl::Vector<Point3R> VertexData;
	VertexData m_vertices;
	Btl::Vector<CollisionMeshTriangle> m_triangles;
};

class CollisionMeshData
{
public:
	template <typename Archive>
	void Serialise(Archive& ar) {ar & m_cellMap;}

protected:
	typedef Btl::ChainedHashMap<CollisionMeshCellIndex, CollisionMeshCell> CellMap;
	CellMap m_cellMap;
};

class Test
{
	template <typename Archive>
	void Serialise(Archive& ar)
	{
		ar & m_index;
	}

	int m_index;
};

}

#endif
