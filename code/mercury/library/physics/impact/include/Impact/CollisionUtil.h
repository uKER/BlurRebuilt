#ifndef __PHYSICS_COLLISION_UTIL_H__
#define __PHYSICS_COLLISION_UTIL_H__

// needed for ContactVector
#include "Impact\Collision.h"

namespace DbUtil
{
class I_DebugRender;
};

namespace Impact
{

extern Debug::Logger &g_Log;

/// a vertex of a C_ClipPolygon
class ClipVertex : public Maths::C_Vertex
{
public:
	ClipVertex() :
	  Maths::C_Vertex(Maths::UNDEFINED)
	{
	}

	ClipVertex &operator =(const Maths::HVec3 &v)
	{
		*(Maths::C_Vertex*)this = v;
		return *this;
	}

	ContactUID uid;
};

/// plane for use with SplitPolygon function
class ClipPlane : public Maths::C_Plane
{
public:
	ClipPlane()
	{
	}

	ClipPlane(const Maths::C_Plane &plane) :
		Maths::C_Plane(plane)
	{
	}

	ClipPlane(const Maths::C_Normal &normal, const Maths::C_Vertex &vtx) :
		Maths::C_Plane(normal, vtx)
	{
	}

	U8 edge;
};

/// polygon for use with SplitPolygon function
class C_ClipPolygon
{
public:
	void AddVertex(const ClipVertex &v) { m_Vertices.PushBack(v); }

	Ut::UtVector<Ut::StackVector<ClipVertex,10>> m_Vertices	;
};

#define PLANE_THICKNESS_EPSILON 0.0002

/// classify point to be in front, behind or on a plane
Maths::E_PlaneTest ClassifyPointToPlane(const Maths::C_Vertex &p, const Maths::C_Plane &plane);

/// functions for splitting a polygon by a plane. updates uids. safe version
void SplitPolygon(const C_ClipPolygon &poly, const ClipPlane &plane, C_ClipPolygon &backPoly);

/// functions for splitting a polygon by a plane. updates uids. fast version
void SplitPolygonFast(const C_ClipPolygon &poly, const ClipPlane &plane, C_ClipPolygon &backPoly);

/// functions for splitting a polygon by a plane. updates uids. SSE version. calls SplitPolygonFast as a fallback
void SplitPolygonFastSIMD(const C_ClipPolygon &poly, const ClipPlane &plane, C_ClipPolygon &backPoly);

/// generate contact points from a contact polygon
int EmitContactPoints(ContactVector &contacts, const C_ClipPolygon &poly, const Maths::C_Plane &refPlane, int iFlip);

// create two axis perpendicular to the input normal _n_
void PlaneSpace(const Maths::HVec3& n, Maths::HVec3& p, Maths::HVec3& q);

/// draw polygon for debugging purposes
void DrawPoly(const C_ClipPolygon &poly);

/// draw clip planes for debugging purposes
void DrawClipPlanes(Maths::C_Plane planes[4], const Maths::HVec3 &referencePoint);

/// find intersection between ray and sphere.
/// returns the distance of intersection from the origin in vHitpos
/// this is used for swept-sphere vs triangle vertex collision
bool IntersectRaySphere(const Maths::HVec3 &org, const Maths::HVec3 &dir, const Maths::HVec3 &pos, float fRadius, float *pfDist, Maths::C_Normal &outNormal, float fMin);

/// find the intersection between a ray and a cylinder
/// this is used for swept-sphere vs triangle edge collision
/// xs0 is cylinder start, vs is ray direction (normalised), rad is cylinder radius, v0 and v1 are cylinder verts, hit_time is ray length
bool CollideEdgeSphereLine(const Maths::HVec3 &xs0, const Maths::HVec3 &vs, float rad, const Maths::HVec3 &v0, const Maths::HVec3 &v1, float *hit_time, Maths::HVec3 &hit_point, Maths::C_Normal &outNormal, float fMinFrac);

const bool LineClosestPoint( const Maths::C_Line &edge, const Maths::C_Vertex& point, Maths::C_Vertex& out_closest_point, Maths::C_Normal& out_normal, Maths::HF32& out_distance );

/// calculate a world-space a AABB from the local AABB using the body's position and rotation
const Maths::C_AxisBox TransformBBox(const Maths::C_AxisBox &AABB, const Maths::HVec3 &vPos, const Maths::HMat33 &mRot);

// default clipping function
template<typename T>
void ClipPolyToPlanes(C_ClipPolygon &clippedPoly, T *pRefHull, C_ClipPolygon &incidentPoly, int iRefPlane);

};

#endif

#include "CollisionUtil.inl"
