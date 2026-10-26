#ifndef RP_COLLISION_CACHE_H
#define RP_COLLISION_CACHE_H
#ifdef _MSC_VER
#pragma once
#endif

#include "racingphysics/physics/geometry.h"
#include "racingphysics/physics/axis.h"
#include "racingphysics/collisions/boundingbox.h"
#include "btl/containers/vector.h"
#include "btl/utility/pair.h"

namespace RacingPhysics
{

class CollisionCache
{
	///////////////////////////////////////////////////////
	// Data types

public:
	//class CollideBoundingBox;
	class NullBoundingBox {};

	typedef FloatVR ScalarType;
	typedef BoolVR BooleanType;

	// stores a normalised vector with the original length in the w component
	class NormalLength
	{
	public:
		NormalLength() {}

		FORCE_INLINE NormalLength(const Vector3R& v) 
		{
			FloatVR s = Magnitude(v);
			Set(&m_v, v / s, s);
		}

		NormalLength(const NormalLength& other) {m_v = other.m_v;}
		NormalLength& operator = (const NormalLength& other) {m_v = other.m_v; return *this;}
		
		FloatVR Length() const {return m_v.W();}
		const Normal3R& Normal() const {return (Normal3R&) m_v;}

	private:
		Vector4R m_v;
	};

	// results
	enum CollisionType
	{
		COLLISION_TYPE_NONE,
		COLLISION_TYPE_TRIANGLE_SURFACE,
		COLLISION_TYPE_TRIANGLE_EDGE,
		COLLISION_TYPE_TRIANGLE_VERTEX,
		COLLISION_TYPE_VERTICAL_SURFACE,
		COLLISION_TYPE_VERTICAL_EDGE,
		COLLISION_TYPE_OTHER
	};

	struct Contact
	{
		ScalarType intersectionDistance; // distance from the intersection point to the surface of the object
		Point3R intersectionPoint; // point on the collision mesh
		Normal3R intersectionNormal; // normal pointing away from the mesh
		Vector3R groundVelocity; //ground velocity
		U32 surfaceFlags; // flags on the mesh triangle that was hit
		U32 userData; // userdata on the mesh triangle that was hit
		U32 boundingBoxesHit; // number of objects not eliminated by bounding box check
		CollisionType collisionType; // see above
	};

	const static U32 COLLISION_FLAG_ALL = 0;
	const static U32 COLLISION_FLAG_IGNORE_TRIANGLES = 1<<0;
	const static U32 COLLISION_FLAG_IGNORE_VERTICAL_SURFACES = 1<<1;

	////////////////////////////////////////////////////////
	// General

	CollisionCache();

	enum CacheType
	{
		CACHE_TYPE_SURFACES,
		CACHE_TYPE_FULL,
	};

	void SetCacheType(CacheType level);
	CacheType GetCacheType() const {return m_type;}
	void SetBoundingBoxMinMax(const Point3R& min, const Point3R& max); 
	void Display() const;
	void DisplayTriangles(float collisionMeshBias = 0.0f) const;
	void Finish();
	void Clear();
	bool Empty() const {return m_triangles.Empty() && m_verticalSurfaces.Empty();}

	BooleanType BoundingBoxContains(const Point3R& min, const Point3R& max, const FloatVR& tolerance) const {return m_boundingBox.Contains(min, max, tolerance);}
	BooleanType BoundingBoxContains(const Point3R& min, const Point3R& max) const {return m_boundingBox.Contains(min, max);}

	template <typename T, typename BoundingBox, bool FIRST_ONLY>
	Contact CollideVolume(const T& collisionObject, const BoundingBox& box, U32 collisionFlags) const;

	template <typename T, typename BoundingBox, bool FIRST_ONLY>
	Contact CollideRay(const T& collisionObject, const BoundingBox& box, const ScalarType& maxDistance, U32 collisionFlags) const;

	template <typename T, typename BoundingBox>
	int CountRayCollisions(const T& collisionObject, const BoundingBox& box, const ScalarType& maxDistance, U32 collisionFlags) const;

	template <typename T, typename BoundingBox, bool FIRST_ONLY>
	Contact CollideSweptVolume(const T& collisionObject, const BoundingBox& box, const ScalarType& maxDistance, U32 collisionFlags) const;

	const Point3R& GetCentre() const {return m_boundingBox.GetCentre();}
	const Vector3R& GetSize() const {return m_boundingBox.GetSize();}
	const Point3R& GetMin() const {return m_boundingBox.GetMin();}
	const Point3R& GetMax() const {return m_boundingBox.GetMax();}

	////////////////////////////////////////////////////////
	// Triangles

	struct TriangleVertex
	{
		Point3R position;
		U32 surfaceFlags;

		TriangleVertex() {}
		FORCE_INLINE TriangleVertex(const Point3R& v, U32 flags)
		{
			position = v;
			surfaceFlags = flags;
		}
	};

	struct TriangleEdge
	{
		Point3R vertex0;
		Point3R vertex1;
		Normal3R faceNormal0;
		Normal3R faceNormal1;
		NormalLength direction;
		U32 surfaceFlags;

		TriangleEdge() {}
		FORCE_INLINE TriangleEdge(const Point3R& v0, const Point3R& v1, U32 flags)
		{
			vertex0 = v0;
			vertex1 = v1;
			direction = NormalLength(v1 - v0);
			surfaceFlags = flags;
		}
	};

	struct Triangle
	{
		Point3R vertex0;
		Point3R vertex1;
		Point3R vertex2;
		Normal3R normal;
		U32 surfaceFlags;

		Triangle() {}
		FORCE_INLINE Triangle(const Point3R& v0, const Point3R& v1, const Point3R& v2, U32 flags)
		{
			vertex0 = v0;
			vertex1 = v1;
			vertex2 = v2;
			normal = Normalised((v0 - v1) ^ (v2 - v1));
			surfaceFlags = flags;
		}
	};

	void ReserveTriangles(int n);

	// build 
	void AddTriangle(Point3R p0, Point3R p1, Point3R p2,
						int i0, int i1, int i2,
						int useEdge0, int useEdge1, int useEdge2,
						int useVertex0, int useVertex1, int useVertex2,
						U32 surfaceFlags = 0);
	void AddTriangleForImpactTrimeshAdapter( int i0, int i1, int i2, U32 surfaceFlags);

	// triangles only, no edges or vertices
	void AddTriangleNoSharing(Point3R p0, Point3R p1, Point3R p2, U32 surfaceFlags = 0);

	// access
	int GetNumTriangleVertices() const {return m_triangleVertices.Size();}
	int GetNumTriangleEdges() const {return m_triangleEdges.Size();}
	int GetNumTriangles() const {return m_triangles.Size();}

	int GetMemoryUsage() const;

	const TriangleVertex& GetTriangleVertex(int i) const {return m_triangleVertices[i];}
	const Triangle& GetTriangle(int i) const {return m_triangles[i];}
	const TriangleEdge& GetTriangleEdge(int i) const {return m_triangleEdges[i];}

	////////////////////////////////////////////////////////////////
	// Vertical surfaces

	struct VerticalSurface
	{
		Point3R vertex0;
		Point3R vertex1;
		Normal3R normal;
		NormalLength direction;
		U32 surfaceFlags;

		VerticalSurface() {}
		VerticalSurface(const Point3R& p0, const Point3R& p1, U32 flags = 0) {Initialise(p0, p1, flags);}

		void Initialise(const Point3R& p0, const Point3R& p1, U32 flags = 0)
		{
			vertex0 = p0;
			vertex1 = p1;
			normal = Normalised((p1 - p0) ^ Maths::Constants<Normal3R>::Y());
			Vector3R d = p1 - p0; d.Y(0.f);
			direction = NormalLength(d);
			surfaceFlags = flags;
		}
	};

	struct VerticalEdge
	{
		Point3R vertex0;
		Point3R vertex1;
		NormalLength direction;
		U32 surfaceFlags;

		VerticalEdge() {}
		FORCE_INLINE VerticalEdge(const Point3R& v0, const Point3R& v1, U32 flags)
		{
			vertex0 = v0;
			vertex1 = v1;
			direction = NormalLength(v1 - v0);
			surfaceFlags = flags;
		}
	};

	// triangles
	struct TriangleIndices
	{
		int i0, i1, i2;
	};

	void ReserveVerticalEdges(int n);

	// build
	void AddVerticalEdge(Point3R p0, Point3R p1, 
							int i0, int i1, U32 surfaceFlags);

	// access
	int GetNumVerticalSurfaces() const {return m_verticalSurfaces.Size();}
	const VerticalSurface& GetVerticalSurface(int i) const {return m_verticalSurfaces[i];}

	const Btl::Vector<TriangleIndices> &GetTriangleIndices() const { return m_triangleIndices; }

private:
	template <typename T, typename BoundingBox, bool FIRST_ONLY>
	void CollideTriangleEdges(const T& collisionObject, const BoundingBox& box, Contact& contact) const;

	template <typename T, typename BoundingBox, bool FIRST_ONLY>
	void CollideTriangleVertices(const T& collisionObject, const BoundingBox& box, Contact& contact) const;

	template <typename T, typename BoundingBox, bool FIRST_ONLY>
	void CollideTriangleSurfaces(const T& collisionObject, const BoundingBox& box, Contact& contact) const;

	template <typename T, typename BoundingBox, bool FIRST_ONLY>
	void CollideVerticalSurfaces(const T& collisionObject, const BoundingBox& box, Contact& contact) const;

	enum State
	{
		STATE_EMPTY,
		STATE_BUILDING,
		STATE_READY,
	};

	State m_state;
	CacheType m_type;

	RacingPhysics::BoundingBoxCollision m_boundingBox;

	Btl::Vector<int> m_triangleVertexIndices;
	Btl::Vector<Btl::Pair<int, int> > m_triangleEdgeIndices;
	Btl::Vector<TriangleIndices> m_triangleIndices;

	Btl::Vector<TriangleVertex> m_triangleVertices;
	Btl::Vector<TriangleEdge> m_triangleEdges;
	Btl::Vector<Triangle> m_triangles;

	// vertical surfaces
	Btl::Vector<Btl::Pair<int, int> > m_verticalSurfaceIndices;
	Btl::Vector<VerticalSurface> m_verticalSurfaces;
};

}

#endif
