#include "stdafx.h"

#include "Impact\ContactUID.h"
#include "Impact\Body.h"
#include "Impact\Collision.h"
#include "Impact\CollisionUtil.h"

#include "System/Debug/I_DebugRender.h"

#include "System/DebugLog.h"

using namespace Maths;

namespace Impact
{

#ifndef CFG_SPEED
Debug::Bool g_bDrawClippedPoly("Impact.DrawClippedPoly", false);
Debug::Bool g_bClipPolyNoSIMD("Impact.Optimizations.ClipPolySIMD", true);
#else
#define g_bDrawClippedPoly false
#define g_bClipPolyNoSIMD true
#endif

Debug::Logger &g_Log = Debug::GetLogger("mercury.impact");

// classify point p to a plane thickened by a given thickness epsilon
#ifndef _DEBUG
inline
#endif
E_PlaneTest ClassifyPointToPlane(const ClipVertex &p, const C_Plane &plane)
{
	HF32 dist = plane.Distance(p);

	if (dist > HF32(PLANE_THICKNESS_EPSILON))
		return E_PlaneTestFront;
	if (dist < HF32(-PLANE_THICKNESS_EPSILON))
		return E_PlaneTestBack;
	return E_PlaneTestIntersect;
}

int gnIntersectEdge = 0;

/// closest point to line segment
const bool LineClosestPoint( const C_Line &edge, const C_Vertex& point, C_Vertex& out_closest_point, C_Normal& out_normal, HF32& out_distance )
{
    // Point : p
    // Line : x1 + ( x2 - x1 ) t
    // Closest t = ( p - x1 ) . ( x2 - x1 ) / | x2 - x1 | ^2
    //           = ( p - x1 ) . ( x2 - x1 ) / ( x2 - x1 ) . ( x2 - x1 )
    
    const C_Vertex x1( edge.Start() );
    const C_Normal x1x2( edge.Normal() );
    const C_Vector x1p( point - x1 );
    const C_Vector x1p_cross_x1x2( x1p ^ x1x2 );
    const HF32     x1p_dot_x1x2( x1p | x1x2 );
    
    C_Normal normal_x1p_cross_x1x2( UNDEFINED );
    HF32     length_x1p_cross_x1x2( UNDEFINED );
    HF32     length2_x1p_cross_x1x2( UNDEFINED );
    HF32     rcp_length_x1p_cross_x1x2( UNDEFINED );
    NormalLength( x1p_cross_x1x2, normal_x1p_cross_x1x2, length_x1p_cross_x1x2, length2_x1p_cross_x1x2, rcp_length_x1p_cross_x1x2 );
    
    const HF32     t( x1p_dot_x1x2 );
	if (t<HF32(0) || t > edge.Length())
		return false;

    const HF32     distance( length_x1p_cross_x1x2 );
    const C_Vertex closest_point( x1 + ( x1x2 * t ) );
    const C_Normal normal( ( point - closest_point ) * rcp_length_x1p_cross_x1x2, C_Normal::UNIT_LENGTH_VECTOR );
    
    out_closest_point = closest_point;
    out_normal        = normal;
    out_distance      = distance;
    return true;
}


// returns the intersection point of an edge and a plane
ClipVertex IntersectEdgeAgainstPlane(const ClipVertex &a, const ClipVertex &b, const ClipPlane &p)
{
	ClipVertex rtn;

	gnIntersectEdge++;

    // Compute t value at which the directed line ab intersects the plane
    HVec3 ab = b - a;
	float fDist0 = (-p.DistanceToOrigin() - (p.Normal() | a));
	float t = fDist0 / (p.Normal() | ab);

    // If t in [0..1] compute and return intersection point
    if (t >= 0.0f && t <= 1.0f)
	{
		if (fDist0 > 0)
		{
			rtn.uid = a.uid;
			rtn.uid.e.inEdge1 = 0;
			rtn.uid.e.inEdge2 = p.edge;
		}
		else
		{
			rtn.uid = b.uid;
			rtn.uid.e.outEdge1 = 0;
			rtn.uid.e.outEdge2 = p.edge;
		}

		rtn = a + (HF32)t * ab;
		return rtn;
    }

	assert(0);

    // Else t is +INF, -INF, NaN, or not in [0..1], so no intersection
    return rtn;
}

#define RAYSPHERE_ISECT_EPSILON 0.0001

/// find intersection between ray and sphere.
/// returns the distance of intersection from the origin in vHitpos
/// this is used for swept-sphere vs triangle vertex collision
bool IntersectRaySphere(const HVec3 &org, const HVec3 &dir, const HVec3 &pos, float fRadius, float *pfDist, C_Normal &outNormal, float fMin)
{
    // set up quadratic Q(t) = a*t^2 + 2*b*t + c
	HVec3 kDiff;
	float fB,fC;
	float afT[2];
	float fDiscr;
	C_Normal normal;

	kDiff = org - pos;
	// dir has to be normalized!
	fB = dir | kDiff;
	fC = kDiff.Length2()-fRadius*fRadius;

	fDiscr = fB*fB - fC;
    if ( fDiscr < 0.0 )
    {
        return false;
    }
    else if ( fDiscr > 0.0 )
    {
        float fRoot = sqrtf(fDiscr);
        afT[0] = (-fB - fRoot);
        afT[1] = (-fB + fRoot);

		if (afT[0]<afT[1] && afT[0]>fMin)
		{
			*pfDist = afT[0];
			outNormal = kDiff;
			return true;
		}
		else if (afT[1]<afT[0] && afT[1]>fMin)
		{
			*pfDist = afT[1];
			outNormal = kDiff;
			return true;
		}
		else
			return false;
    }
    else
    {
        afT[0] = -fB;
        if ( afT[0] >= fMin )
        {
			*pfDist = afT[0];
			outNormal = kDiff;
            return true;
        }
        else
        {
            return false;
        }
    }
}

/// find the intersection between a ray and a cylinder
/// this is used for swept-sphere vs triangle edge collision
bool CollideEdgeSphereLine(const HVec3 &xs0, const HVec3 &vs, float rad, const HVec3 &v0, const HVec3 &v1, float *hit_time, HVec3 &hit_point, C_Normal &outNormal, float fMinFrac)
{
	HVec3 temp_sphere_hit;
	bool try_vertex = false; // Assume we don't have to try the vertices.
	float delta_dot_ve, delta_dot_vs, delta_sqr, ve_dot_vs;
	float ve_sqr, vs_sqr, temp;
	float A, B, C, root, discriminant;
	float root1 = 0.0f;
	float root2 = 0.0f;
	float s_edge;
	HVec3 t,hit;
	HVec3 ven;

	HVec3 ve,delta;
	ve = v1 - v0;
	delta = xs0 - v0;
	delta_dot_ve = delta | ve;
	delta_dot_vs = delta | vs;
	delta_sqr = delta.Length2();
	ve_dot_vs = ve | vs;
	ve_sqr = ve.Length2();
	vs_sqr = vs.Length2();

	// position of the collision along the edge is given by: xe = v0 + ve*s, where s is
	//  in the range [0,1].  position of sphere along its path is given by: 
	//  xs = xs + vs*t, where t is in the range [0,1].  t is time, but s is arbitrary.
	//
	// solve simultaneous equations
	// (1) distance between edge and sphere center must be sphere radius
	// (2) line between sphere center and edge must be perpendicular to edge
	//
	// (1) (xe - xs)*(xe - xs) = rad*rad
	// (2) (xe - xs) * ve = 0
	//
	// then apply mathematica

	A = ve_dot_vs * ve_dot_vs - ve_sqr * vs_sqr;
	B = 2 * (delta_dot_ve * ve_dot_vs - delta_dot_vs * ve_sqr);
	C = delta_dot_ve * delta_dot_ve + rad * rad * ve_sqr - delta_sqr * ve_sqr;

	if( A > -0.0001f && A < 0.0001f )
	{
		// degenerate case, sphere is traveling parallel to edge
		try_vertex = true;
	} 
	else 
	{
		discriminant = B*B - 4*A*C;
		if( discriminant > 0 ) 
		{
			root = (float)sqrtf(discriminant);
			root1 = (-B + root) / (2 * A);
			root2 = (-B - root) / (2 * A);

			// sort root1 and root2, use the earliest intersection.  the larger root 
			//  corresponds to the final contact of the sphere with the edge on its 
			//  way out.
			if( root2 < root1 ) 
			{
				temp = root1;
				root1 = root2;
				root2 = temp;
			}

			// intersections are both 
			if (root1 >= *hit_time)
				return false; // earliest intersection is further away than previous
			if (root2 < 0)
				return false; // ray starts behind cylinder
			if (root1 < fMinFrac)
				return false;

			// ray segment overlaps with cylinder

			// find sphere and edge positions
			//temp_sphere_hit = xs0 + vs * root1;
			temp_sphere_hit = xs0 + vs * root1;

			// check if hit is between v0 and v1
			s_edge = ((temp_sphere_hit - v0) | ve) / ve_sqr;

			if( (s_edge >= 0) && (s_edge <= 1) )
			{
				// bingo
				*hit_time = root1;
				hit_point = v0 + ve * s_edge;
				// create normal:

				// calculate hitpoint on sphere path
				hit = vs * root1 + xs0;

				outNormal = hit-hit_point;
				return true;
			}
		}
		else
		{
			// discriminant negative, sphere passed edge too far away
			return false;
		}
	}

	return false;
}


// returns the intersection point of an edge and a plane
FORCE_INLINE ClipVertex IntersectEdgeAgainstPlaneFast(const ClipVertex &a, const ClipVertex &b, const ClipPlane &p, const HF32 &fDista, const HF32 &fDistb)
{
	ClipVertex rtn;

	gnIntersectEdge++;

    // Compute t value at which the directed line ab intersects the plane
    HVec3 ab = b - a;
	float x = (fDista - fDistb);
	HF32 t = fDista / x;

    // If t in [0..1] compute and return intersection point
    // Else t is +INF, -INF, NaN, or not in [0..1], so no intersection
    assert (t >= 0.0f && t <= 1.0f);

	if (fDista > HF32(0))
	{
		rtn.uid = a.uid;
		rtn.uid.e.inEdge1 = 0;
		rtn.uid.e.inEdge2 = p.edge;
	}
	else
	{
		rtn.uid = b.uid;
		rtn.uid.e.outEdge1 = 0;
		rtn.uid.e.outEdge2 = p.edge;
	}

	rtn = a + t * ab;
	return rtn;
}

// split a polygon into two along a given plane
void SplitPolygon(const C_ClipPolygon &poly, const ClipPlane &plane, C_ClipPolygon &backPoly)
{
	backPoly.m_Vertices.Resize(0);

	if (poly.m_Vertices.Size() <= 0)
		return;

	// test all edges (a, b) starting with edge from last to first vertex
	int numVerts = poly.m_Vertices.Size();
	ClipVertex a = poly.m_Vertices[numVerts-1];
	E_PlaneTest aSide = ClassifyPointToPlane(a, plane);

	// loop over all edges given by vertex pair (n - 1, n)
	for (int n=0; n<numVerts; n++)
	{
		ClipVertex b = poly.m_Vertices[n];
		E_PlaneTest bSide = ClassifyPointToPlane(b, plane);

		if (bSide == E_PlaneTestFront)
		{
			if (aSide == E_PlaneTestBack)
			{
				// Edge (a, b) straddles, output intersection point to both sides
				ClipVertex i = IntersectEdgeAgainstPlane(a, b, plane);
#ifdef CLASSIFYPOINTTOPLANE_ASSERTS
				assert(ClassifyPointToPlane(i, plane) == E_PlaneTestIntersect);
#endif
				backPoly.AddVertex(i);
			}
		}
		else if (bSide == E_PlaneTestBack)
		{
			if (aSide == E_PlaneTestFront)
			{
				// Edge (a, b) straddles plane, output intersection point to both sides
				ClipVertex i = IntersectEdgeAgainstPlane(a, b, plane);
#ifdef CLASSIFYPOINTTOPLANE_ASSERTS
				assert(ClassifyPointToPlane(i, plane) == E_PlaneTestIntersect);
#endif
				backPoly.AddVertex(i);
			}
			else if (aSide == E_PlaneTestIntersect)
			{
				// output a when edge (a, b) goes from 'on' to 'behind' the plane
				backPoly.AddVertex(a);
			}
			// in all three cases, output b to the back side
			backPoly.AddVertex(b);
		}
		else
		{
			// in one case, als output b to the back side
			if (aSide == E_PlaneTestBack)
			{
				backPoly.AddVertex(b);
			}
		}
		// keep b as starting point of next edge
		a = b;
		aSide = bSide;
	}
}

// split a polygon into two along a given plane
void SplitPolygonFast(const C_ClipPolygon &poly, const ClipPlane &plane, C_ClipPolygon &backPoly)
{
	backPoly.m_Vertices.Resize(0);

	if (poly.m_Vertices.Size() <= 0)
		return;

	// test all edges (a, b) starting with edge from last to first vertex
	int numVerts = poly.m_Vertices.Size();
	ClipVertex a = poly.m_Vertices[numVerts-1];
	E_PlaneTest aSide;
	HF32 aDist = plane.Distance(a);

	if (aDist > HF32(0))
		aSide = E_PlaneTestFront;
	else
		aSide = E_PlaneTestBack;

	// loop over all edges given by vertex pair (n - 1, n)
	for (int n=0; n<numVerts; n++)
	{
		E_PlaneTest bSide;
		ClipVertex b = poly.m_Vertices[n];
		HF32 bDist = plane.Distance(b);
		if (bDist > HF32(0))
		{
			bSide = E_PlaneTestFront;
			if (aSide == E_PlaneTestBack)
			{
				// Edge (a, b) straddles, output intersection point to both sides
				ClipVertex i = IntersectEdgeAgainstPlaneFast(a, b, plane, aDist, bDist);
#ifdef CLASSIFYPOINTTOPLANE_ASSERTS
				assert(ClassifyPointToPlane(i, plane) == E_PlaneTestIntersect);
#endif
				backPoly.AddVertex(i);
			}
		}
		else
		{
			bSide = E_PlaneTestBack;
			if (aSide == E_PlaneTestFront)
			{
				// Edge (a, b) straddles plane, output intersection point to both sides
				ClipVertex i = IntersectEdgeAgainstPlaneFast(a, b, plane, aDist, bDist);
#ifdef CLASSIFYPOINTTOPLANE_ASSERTS
				assert(ClassifyPointToPlane(i, plane) == E_PlaneTestIntersect);
#endif
				backPoly.AddVertex(i);
			}
			// in all three cases, output b to the back side
			backPoly.AddVertex(b);
		}
		// keep b as starting point of next edge
		a = b;
		aSide = bSide;
		aDist = bDist;
	}
}

// split a polygon into two along a given plane
void SplitPolygonFastSIMD(const C_ClipPolygon &poly, const ClipPlane &plane, C_ClipPolygon &backPoly)
{
	if (poly.m_Vertices.Size() > 4 || !g_bClipPolyNoSIMD)
		return SplitPolygonFast(poly, plane, backPoly);

	int n = poly.m_Vertices.Size();
	if (!n)
		return;
	ClipVertex *p = (ClipVertex*)&poly.m_Vertices[0];

	// swizzle in vertices into SOA format
	HMat44 verts;
	HVec4 *_verts((HVec4*)&verts);
	for (int i=0; i<n; i++)
		((HVec4*)&verts)[i] = p[i];
	verts.Transpose();

	HVec4 &_plane = (HVec4&)plane;

	// calculate dot products
	HVec4 _f = _plane.X() * _verts[0] + _plane.Y() * _verts[1] + _plane.Z() * _verts[2] + HVec4(_plane.W());
	float *pf = (float*)&_f;
/*
	assert(n<1 || _f.X() == plane.Distance(poly.m_Vertices[0]));
	assert(n<2 || _f.Y() == plane.Distance(poly.m_Vertices[1]));
	assert(n<3 || _f.Z() == plane.Distance(poly.m_Vertices[2]));
	assert(n<4 || _f.W() == plane.Distance(poly.m_Vertices[3]));
*/
	backPoly.m_Vertices.Resize(0);

	if (poly.m_Vertices.Size() <= 0)
		return;

	// test all edges (a, b) starting with edge from last to first vertex
	int numVerts = poly.m_Vertices.Size();
	ClipVertex a = poly.m_Vertices[numVerts-1];
	E_PlaneTest aSide;
	float aDist = pf[numVerts-1];

	if (aDist > 0)
		aSide = E_PlaneTestFront;
	else
		aSide = E_PlaneTestBack;

	// loop over all edges given by vertex pair (n - 1, n)
	for (int n=0; n<numVerts; n++)
	{
		E_PlaneTest bSide;
		ClipVertex b = poly.m_Vertices[n];
		float bDist = pf[n];
		if (bDist > 0)
		{
			bSide = E_PlaneTestFront;
			if (aSide == E_PlaneTestBack)
			{
				// Edge (a, b) straddles, output intersection point to both sides
				ClipVertex i = IntersectEdgeAgainstPlaneFast(a, b, plane, aDist, bDist);
#ifdef CLASSIFYPOINTTOPLANE_ASSERTS
				assert(ClassifyPointToPlane(i, plane) == E_PlaneTestIntersect);
#endif
				backPoly.AddVertex(i);
			}
		}
		else
		{
			bSide = E_PlaneTestBack;
			if (aSide == E_PlaneTestFront)
			{
				// Edge (a, b) straddles plane, output intersection point to both sides
				ClipVertex i = IntersectEdgeAgainstPlaneFast(a, b, plane, aDist, bDist);
#ifdef CLASSIFYPOINTTOPLANE_ASSERTS
				assert(ClassifyPointToPlane(i, plane) == E_PlaneTestIntersect);
#endif
				backPoly.AddVertex(i);
			}
			// in all three cases, output b to the back side
			backPoly.AddVertex(b);
		}
		// keep b as starting point of next edge
		a = b;
		aSide = bSide;
		aDist = bDist;
	}
}

// create two axis perpendicular to the input normal _n_
void PlaneSpace(const HVec3& n, HVec3& p, HVec3& q)
{
	if (fabs(n.Z()) > 0.7f)
	{
		// choose p in y-z plane
		float a = n[1]*n[1] + n[2]*n[2];
		float k = 1.f/sqrtf(a);
		p[0] = 0;
		p[1] = -n[2]*k;
		p[2] = n[1]*k;
		// set q = n x p
		q[0] = a*k;
		q[1] = -n[0]*p[2];
		q[2] = n[0]*p[1];
	}
	else
	{
		// choose p in x-y plane
		float a = n[0]*n[0] + n[1]*n[1];
		float k = 1.f/sqrtf(a);
		p[0] = -n[1]*k;
		p[1] = n[0]*k;
		p[2] = 0;
		// set q = n x p
		q[0] = -n[2]*p[1];
		q[1] = n[2]*p[0];
		q[2] = a*k;
	}
}

void DrawPoly(const C_ClipPolygon &poly)
{
	// use dumb triangulation
	int n = poly.m_Vertices.Size();
	for (int i=0; i<n; i++)
	{
		int i2 = (i+1)%n;
		int i3 = (i+2)%n;

		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dTriangle(
			poly.m_Vertices[0], poly.m_Vertices[i2], poly.m_Vertices[i3], RGBA(0,128,0,128));
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dTriangle(
			poly.m_Vertices[0], poly.m_Vertices[i3], poly.m_Vertices[i2], RGBA(0,128,0,128));
	}
}

void DrawClipPlanes(C_Plane planes[4], const HVec3 &referencePoint)
{
	for (int i=0; i<4; i++)
	{
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dPlane(planes[i], referencePoint, 1.f, RGBA(128,64,64));
	}
}

/// generate contact points from a contact polygon (scalar fallback for 5+ vertices)
int EmitContactPointsScalar(ContactVector &contacts, const C_ClipPolygon &clippedPoly, const C_Plane &refPlane, const C_Normal &vRefPlaneNormal)
{
	if (g_bDrawClippedPoly)
		DrawPoly(clippedPoly);

	// emit contact points
	int n = clippedPoly.m_Vertices.Size();
	for (int i=0; i<n; i++)
	{
		Contact contact;
		contact.m_vNormal = vRefPlaneNormal;
		contact.m_vPos = clippedPoly.m_Vertices[i];
		contact.m_fSeparation = refPlane.Distance(contact.m_vPos);
		contact.m_uid = clippedPoly.m_Vertices[i].uid.key;

		if (contact.m_fSeparation < 0)
		{
			if (g_bDrawClippedPoly)
				DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dArrow(contact.m_vPos, contact.m_vNormal, contact.m_fSeparation, 0.1f, Maths::RGBA(255,255,255));

			contacts.PushBack(contact);
			if (contacts.Size() == contacts.Capacity())
			{
				//OutputDebugString("HACK\n");
				return 1;	// TEMPORARY HACK
			}
		}
	}

	return contacts.Size();
}

/// generate contact points from a contact polygon
int EmitContactPoints(ContactVector &contacts, const C_ClipPolygon &clippedPoly, const C_Plane &refPlane, int iFlip)
{
	if (clippedPoly.m_Vertices.Size() == 0)
		return 0;

	// optionally flip plane normal
	C_Normal vRefPlaneNormal;
	if (iFlip)
		vRefPlaneNormal = -refPlane.Normal();
	else
		vRefPlaneNormal = refPlane.Normal();

	int n = clippedPoly.m_Vertices.Size();
	if (n > 4 || contacts.Size() + n > contacts.Capacity() || g_bDrawClippedPoly)
		return EmitContactPointsScalar(contacts, clippedPoly, refPlane, vRefPlaneNormal);

	// get vertices in SOA format
	HMat44 verts;
	for (int i=0; i<n; i++)
		((HVec4*)&verts)[i] = clippedPoly.m_Vertices[i];
	verts.Transpose();
	// calculate plane distance for all 4 vertices at once
	HVec4 plane = (HVec4&)refPlane;
	HVec4 fSeparation = ((HVec4*)&verts)[0] * plane.X() + 
						((HVec4*)&verts)[1] * plane.Y() + 
						((HVec4*)&verts)[2] * plane.Z() + 
						HVec4(plane.W());

	// clamp the number of contacts we will add if capacity is full 
	int i = contacts.Size();
	int x = 0;
	// emit contact point 0
	float fSep = fSeparation.X();
	if (fSep < 0)
	{
		contacts.PushBack();
		contacts.Back().m_vNormal = vRefPlaneNormal;
		contacts.Back().m_vPos = clippedPoly.m_Vertices[0];
		contacts.Back().m_fSeparation = fSep;
		contacts.Back().m_uid = clippedPoly.m_Vertices[0].uid.key;
	}
	if (n == 1)
		return contacts.Size();
	// emit contact point 1
	fSep = fSeparation.Y();
	if (fSep < 0)
	{
		contacts.PushBack();
		contacts.Back().m_vNormal = vRefPlaneNormal;
		contacts.Back().m_vPos = clippedPoly.m_Vertices[1];
		contacts.Back().m_fSeparation = fSep;
		contacts.Back().m_uid = clippedPoly.m_Vertices[1].uid.key;
	}
	if (n == 2)
		return contacts.Size();
	// emit contact point 2
	fSep = fSeparation.Z();
	if (fSep < 0)
	{
		contacts.PushBack();
		contacts.Back().m_vNormal = vRefPlaneNormal;
		contacts.Back().m_vPos = clippedPoly.m_Vertices[2];
		contacts.Back().m_fSeparation = fSep;
		contacts.Back().m_uid = clippedPoly.m_Vertices[2].uid.key;
	}
	if (n == 3)
		return contacts.Size();
	// emit contact point 3
	fSep = fSeparation.W();
	if (fSep < 0)
	{
		contacts.PushBack();
		contacts.Back().m_vNormal = vRefPlaneNormal;
		contacts.Back().m_vPos = clippedPoly.m_Vertices[3];
		contacts.Back().m_fSeparation = fSep;
		contacts.Back().m_uid = clippedPoly.m_Vertices[3].uid.key;
	}

	return contacts.Size();
}

/// calculate a world-space a AABB from the local AABB using the body's position and rotation
const C_AxisBox TransformBBox(const Maths::C_AxisBox &AABB, const Maths::HVec3 &vPos, const Maths::HMat33 &mRot)
{
	Maths::HMat33 mRot1 = mRot.Transposed();
	HMat33 mMin( mRot1.AX() * AABB.Min(),
				 mRot1.AY() * AABB.Min(),
				 mRot1.AZ() * AABB.Min());
	HMat33 mMax( mRot1.AX() * AABB.Max(),
				 mRot1.AY() * AABB.Max(),
				 mRot1.AZ() * AABB.Max());
	mMin.Transpose();
	mMax.Transpose();

	HVec3 vMin = vPos + mMin.AX().Min(mMax.AX()) + mMin.AY().Min(mMax.AY()) + mMin.AZ().Min(mMax.AZ());
	HVec3 vMax = vPos + mMin.AX().Max(mMax.AX()) + mMin.AY().Max(mMax.AY()) + mMin.AZ().Max(mMax.AZ());

	return C_AxisBox(vMin,vMax);
}


}; // namespace Impact
