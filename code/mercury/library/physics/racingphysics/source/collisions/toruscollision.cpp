#include "stdafx.h"
#include "racingphysics/collisions/toruscollision.h"
#include "racingphysics/collisions/raycast.h"
#include "racingphysics/collisions/collisioncache.inl"
#include "racingphysics/collisions/roots3And4.h"
#include "core/maths/utmaths/maths.h"

//#pragma optimize("", off)

///////////////////////////////////////////////////////////////////////////

namespace RacingPhysics
{

//using namespace Maths;
using namespace UtMaths;

TorusCollision::TorusCollision(const Point3R& centre, const Normal3R& normal, float hoopRadius, float tubeRadius)
		:	m_centre(centre), 
			m_normal(normal),
			m_hoopRadius(hoopRadius), 
			m_hoopRadiusSquared(hoopRadius * hoopRadius),
			m_tubeRadius(tubeRadius), 
			m_tubeRadiusSquared(tubeRadius * tubeRadius),	
			//m_oneXZ(1.0, 0.0, 1.0),
			m_fullRadius(hoopRadius + tubeRadius),
			m_fullRadiusSquared(m_fullRadius * m_fullRadius),
			m_boundingBox(m_centre - Vector3R(m_fullRadius, m_fullRadius, m_fullRadius), 
							m_centre + Vector3R(m_fullRadius, m_fullRadius, m_fullRadius))
{
}

// Collide with triangle surface only, edges and vertices are separate.
TorusCollision::BooleanType TorusCollision::CollideFeature(const CollisionCache::Triangle& triangle, CollisionCache::Contact& contact) const
{
	// Find the lowest point and see if it is below the triangle.
	// This is correct as long as the torus does not go under one of the edges without touching it.
	// That case is unlikely but it would still be resolved correctly if the adjacent triangle lies in the same plane.
	Vector3R v = Normalised(m_normal ^ triangle.normal) ^ m_normal;
	Vector3R vs = v * Sign(v | triangle.normal);
	Point3R lowest = m_centre - vs * m_hoopRadius - triangle.normal * m_tubeRadius;
	ScalarType height = ((m_centre - lowest) | triangle.normal) * Maths::Constants<ScalarType>::Two();
	ScalarType intersectionDistance = (triangle.vertex0 - lowest) | triangle.normal;

	if (intersectionDistance > height | intersectionDistance < contact.intersectionDistance)
		return Maths::Constants<BooleanType>::False();
	
	Vector3R l0 = (lowest - triangle.vertex0) ^ triangle.normal;
	ScalarType b1 = (lowest - triangle.vertex1) | l0;
	ScalarType b2 = (lowest - triangle.vertex2) | l0;
	BooleanType reject1 = (b1 * b2 > Maths::Constants<ScalarType>::Zero()); // plane containing the line and one vertex must intersect the opposite edge

	Vector3R l1 = (lowest - triangle.vertex1) ^ triangle.normal;
	ScalarType c1 = (lowest - triangle.vertex0) | l1;
	ScalarType c2 = (lowest - triangle.vertex2) | l1;
	BooleanType reject2 = (c1 * c2 > Maths::Constants<ScalarType>::Zero()); // checking two edges is enough

	if (!reject1 & !reject2)
	{
		contact.collisionType = CollisionCache::COLLISION_TYPE_TRIANGLE_SURFACE;
		contact.intersectionDistance = intersectionDistance;
		contact.intersectionNormal = triangle.normal;
		contact.intersectionPoint = lowest + triangle.normal * intersectionDistance;
		contact.surfaceFlags = triangle.surfaceFlags;

		return Maths::Constants<BooleanType>::True();
	}

	return Maths::Constants<BooleanType>::False();
}

TorusCollision::BooleanType TorusCollision::CollideFeature(const CollisionCache::TriangleEdge& triangleEdge, CollisionCache::Contact& contact) const
{
	// early rejection - sphere distance
	Vector3R v = m_centre - triangleEdge.vertex0;
	ScalarType d = v | triangleEdge.direction.Normal();
	ScalarType s2 = (v | v) - d * d;
	if (s2 > m_fullRadiusSquared)
		return Maths::Constants<BooleanType>::False();

	// using double for accuracy
	typedef MVector<VectorXYZ<double> > MVector3D;
	typedef MPoint<VectorXYZ<double> > MPoint3D;
	typedef MNormal<VectorXYZ<double> > MNormal3D;

	MPoint3D lineStart;
	MPoint3D circleCentre;
	MNormal3D lineDirection;
	MNormal3D circleNormal;

	//MPoint3D lineStart = triangleEdge.vertex0;
	//MPoint3D circleCentre = m_centre;
	//MNormal3D lineDirection = triangleEdge.direction.Normal();
	//MNormal3D circleNormal = m_normal;

	lineStart[0] = triangleEdge.vertex0.X();
	circleCentre[0] = m_centre.X();
	lineDirection[0] = triangleEdge.direction.Normal().X();
	circleNormal[0] = m_normal.X();
	lineStart[1] = triangleEdge.vertex0.Y();
	circleCentre[1] = m_centre.Y();
	lineDirection[1] = triangleEdge.direction.Normal().Y();
	circleNormal[1] = m_normal.Y();
	lineStart[2] = triangleEdge.vertex0.Z();
	circleCentre[2] = m_centre.Z();
	lineDirection[2] = triangleEdge.direction.Normal().Z();
	circleNormal[2] = m_normal.Z();

	double radius = m_hoopRadius;
	double fRSqr = radius * radius;
	double lineLength = triangleEdge.direction.Length();

	MVector3D kDiff = lineStart - circleCentre;
    double fDSqrLen = kDiff | kDiff;
	double fMdM = 1.f;
	double fDdM = kDiff | lineDirection;
	double fNdM = circleNormal | lineDirection;
    double fDdN = kDiff | circleNormal;

	double fA0 = fDdM;
    double fA1 = fMdM;
    double fB0 = fDdM - fNdM*fDdN;
    double fB1 = fMdM - fNdM*fNdM;
    double fC0 = fDSqrLen - fDdN*fDdN;
    double fC1 = fB0;
    double fC2 = fB1;
	
    double fA0Sqr = fA0*fA0;
    double fA1Sqr = fA1*fA1;
    double fTwoA0A1 = 2.0*fA0*fA1;
    double fB0Sqr = fB0*fB0;
    double fB1Sqr = fB1*fB1;
    double fTwoB0B1 = 2.0*fB0*fB1;
    double fTwoC1 = 2.0*fC1;

	// The minimum point B+t*M occurs when t is a root of the quartic
    // equation whose coefficients are defined below.
    double kPoly[5];

    kPoly[0] = fA0Sqr*fC0 - fB0Sqr*fRSqr;
    kPoly[1] = fTwoA0A1*fC0 + fA0Sqr*fTwoC1 - fTwoB0B1*fRSqr;
    kPoly[2] = fA1Sqr*fC0 + fTwoA0A1*fTwoC1 + fA0Sqr*fC2 - fB1Sqr*fRSqr;
    kPoly[3] = fA1Sqr*fTwoC1 + fTwoA0A1*fC2;
    kPoly[4] = fA1Sqr*fC2;

	double afRoot[4];
	int iCount = SolveQuartic(kPoly, afRoot);
	BooleanType found = Maths::Constants<BooleanType>::False();

	for (int i = 0; i < iCount; i++)
	{
		double f = afRoot[i];
		if (f > 0.0 && f < lineLength)
		{
			Point3R p = triangleEdge.vertex0 + triangleEdge.direction.Normal() * (float) f;
			CollisionCache::TriangleVertex tv(p, triangleEdge.surfaceFlags);

			if (CollideFeature(tv, contact))
			{
				found = Maths::Constants<BooleanType>::True();

				Vector3R normal = MakeVector(contact.intersectionNormal);
				normal -= triangleEdge.direction.Normal() * (triangleEdge.direction.Normal() | normal);

				// check against the faces
				Vector3R faceDirection0 = triangleEdge.direction.Normal() ^ triangleEdge.faceNormal0;
				
				if ((normal | faceDirection0) > Maths::Constants<FloatVR>::Zero())
					normal = triangleEdge.faceNormal0;

				Vector3R faceDirection1 = triangleEdge.direction.Normal() ^ triangleEdge.faceNormal1;
				
				if ((normal | faceDirection1) < Maths::Constants<FloatVR>::Zero())
					normal = triangleEdge.faceNormal1;

				contact.intersectionNormal = Normalised(normal);
			}
		}
	}

	if (found)
	{
		contact.collisionType = CollisionCache::COLLISION_TYPE_TRIANGLE_EDGE;
		return Maths::Constants<BooleanType>::True();
	}

	return Maths::Constants<BooleanType>::False();
}

TorusCollision::BooleanType TorusCollision::CollideFeature(const CollisionCache::TriangleVertex& vertex, CollisionCache::Contact& contact) const
{
	Vector3R vc = vertex.position - m_centre;
	ScalarType dn = vc | m_normal;
	Vector3R vp = vc - m_normal * dn;
	ScalarType dp = Sqrt(vp | vp);
	ScalarType dr = Abs(dp - m_hoopRadius);
	ScalarType sign = Sign(dp - m_hoopRadius);
	ScalarType d2 = dn * dn + dr * dr;
	ScalarType d = Sqrt(d2);

	if (d < m_tubeRadius)
	{
		Normal3R normal;
		ScalarType intersectionDistance;
		Point3R closest = m_centre + vp * m_hoopRadius / dp;

		if (sign > Maths::Constants<ScalarType>::Zero()) // outer surface
		{
			intersectionDistance = m_tubeRadius - d;
			normal = Normalised(closest - vertex.position);
		}
		else // inner surface
		{
			// go to nearest convex surface
			Point3R edgePoint = closest + m_tubeRadius * m_normal * Sign(dn);
			Vector3R v = vertex.position - edgePoint;
			intersectionDistance = Magnitude(v);
			normal = MakeNormal(v / intersectionDistance);
		}

		if (intersectionDistance > contact.intersectionDistance)
		{
			contact.intersectionPoint = vertex.position;
			contact.intersectionDistance = intersectionDistance;
			contact.intersectionNormal = normal;
			contact.collisionType = CollisionCache::COLLISION_TYPE_TRIANGLE_VERTEX;
			contact.surfaceFlags = vertex.surfaceFlags;

			return Maths::Constants<BooleanType>::True();
		}
	}

	return Maths::Constants<BooleanType>::False();
}

CollisionCache::BooleanType TorusCollision::CollideFeature(const CollisionCache::VerticalSurface& verticalSurface, CollisionCache::Contact& contact) const
{
	return Maths::Constants<BooleanType>::False();
}

///////////////////////////////////////////////////

CollisionCache::Contact TorusCollision::GetFirstCollision(const CollisionCache& cache, U32 flags) const
{
	return cache.CollideVolume<TorusCollision, BoundingBoxCollision, true>(*this, m_boundingBox, flags);
}

CollisionCache::Contact TorusCollision::GetDeepestCollision(const CollisionCache& cache, U32 flags) const
{
	return cache.CollideVolume<TorusCollision, BoundingBoxCollision, false>(*this, m_boundingBox, flags);
}

}
